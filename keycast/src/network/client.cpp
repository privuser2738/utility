#include "client.h"
#include "protocol.h"
#include "settings.h"
#include "sslconfig.h"

#include <QBuffer>

Client::Client(QObject* parent)
    : QObject(parent)
    , m_socket(new QSslSocket(this))
    , m_reconnectTimer(new QTimer(this))
{
    connect(m_socket, &QSslSocket::connected, this, &Client::onConnected);
    connect(m_socket, &QSslSocket::disconnected, this, &Client::onDisconnected);
    connect(m_socket, &QSslSocket::encrypted, this, &Client::onEncrypted);
    connect(m_socket, QOverload<const QList<QSslError>&>::of(&QSslSocket::sslErrors),
            this, &Client::onSslErrors);
    connect(m_socket, &QSslSocket::readyRead, this, &Client::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &Client::onError);
    connect(m_reconnectTimer, &QTimer::timeout, this, &Client::onReconnectTimer);
}

Client::~Client()
{
    disconnect();
}

void Client::connectToServer(const QString& address, int port, const QString& password)
{
    if (m_connected) {
        disconnect();
    }

    m_serverAddress = address;
    m_serverPort = port;
    m_password = password;
    m_reconnectAttempts = 0;
    m_sslEstablished = false;

    if (m_useSsl) {
        m_socket->setSslConfiguration(SslConfig::instance()->clientConfiguration());
        m_socket->connectToHostEncrypted(address, port);
    } else {
        m_socket->connectToHost(address, port);
    }
}

void Client::disconnect()
{
    m_autoReconnect = false;
    m_reconnectTimer->stop();

    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        // Send disconnect packet
        if (m_authenticated) {
            QByteArray packet = Protocol::createDisconnectPacket();
            m_socket->write(packet);
            m_socket->flush();
        }
        m_socket->disconnectFromHost();
    }

    m_connected = false;
    m_authenticated = false;
    m_sslEstablished = false;
    m_buffer.clear();
}

void Client::onConnected()
{
    m_connected = true;
    m_reconnectAttempts = 0;

    emit connected();

    // If not using SSL, send auth immediately
    if (!m_useSsl) {
        sendAuthentication();
    }
    // If using SSL, wait for encrypted() signal
}

void Client::onEncrypted()
{
    m_sslEstablished = true;
    emit sslEstablished();

    // Now send authentication
    sendAuthentication();
}

void Client::onSslErrors(const QList<QSslError>& errors)
{
    // Ignore self-signed certificate errors for now
    // In production, you might want to verify the server certificate
    m_socket->ignoreSslErrors();
}

void Client::sendAuthentication()
{
    QString clientName = Settings::instance()->computerName();
    QByteArray authPacket = Protocol::createAuthPacket(m_password, clientName);
    m_socket->write(authPacket);
}

void Client::onDisconnected()
{
    m_connected = false;
    m_authenticated = false;
    m_sslEstablished = false;
    m_buffer.clear();

    emit disconnected();

    // Auto-reconnect if enabled
    if (m_autoReconnect && m_reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
        m_reconnectTimer->start(5000); // Retry in 5 seconds
    }
}

void Client::onReadyRead()
{
    m_buffer.append(m_socket->readAll());
    processData();
}

void Client::processData()
{
    while (m_buffer.size() >= 11) { // Minimum header size
        Protocol::PacketHeader header;
        if (!Protocol::parsePacketHeader(m_buffer, header)) {
            // Invalid packet, clear buffer
            m_buffer.clear();
            return;
        }

        int totalSize = 11 + header.payloadSize;
        if (m_buffer.size() < totalSize) {
            // Wait for more data
            return;
        }

        // Extract complete packet
        QByteArray packet = m_buffer.left(totalSize);
        m_buffer.remove(0, totalSize);

        handlePacket(packet);
    }
}

void Client::handlePacket(const QByteArray& packet)
{
    Protocol::PacketHeader header;
    if (!Protocol::parsePacketHeader(packet, header)) return;

    switch (header.type) {
    case Protocol::MessageType::AuthResponse: {
        Protocol::AuthResult result;
        QString serverName;
        if (Protocol::parseAuthResponsePacket(packet, result, serverName)) {
            if (result == Protocol::AuthResult::Success) {
                m_authenticated = true;
                m_serverName = serverName;
                m_autoReconnect = true;
                emit authenticated(serverName);
            } else {
                QString reason;
                switch (result) {
                case Protocol::AuthResult::InvalidPassword:
                    reason = "Invalid password";
                    break;
                case Protocol::AuthResult::ServerFull:
                    reason = "Server is full";
                    break;
                case Protocol::AuthResult::VersionMismatch:
                    reason = "Protocol version mismatch";
                    break;
                default:
                    reason = "Unknown error";
                }
                m_autoReconnect = false;
                emit authenticationFailed(reason);
            }
        }
        break;
    }

    case Protocol::MessageType::KeyEvent: {
        if (!m_authenticated) return;

        int vkCode;
        bool pressed;
        if (Protocol::parseKeyEventPacket(packet, vkCode, pressed)) {
            emit keyEventReceived(vkCode, pressed);
        }
        break;
    }

    case Protocol::MessageType::MouseEvent: {
        if (!m_authenticated) return;

        int x, y, button;
        bool pressed;
        if (Protocol::parseMouseEventPacket(packet, x, y, button, pressed)) {
            emit mouseEventReceived(x, y, button, pressed);
        }
        break;
    }

    case Protocol::MessageType::MouseMove: {
        if (!m_authenticated) return;

        int x, y;
        if (Protocol::parseMouseMovePacket(packet, x, y)) {
            emit mouseMoveReceived(x, y);
        }
        break;
    }

    case Protocol::MessageType::ExecuteCommand: {
        if (!m_authenticated) return;

        QString command, type;
        if (Protocol::parseExecuteCommandPacket(packet, command, type)) {
            emit executeCommandReceived(command, type);
        }
        break;
    }

    case Protocol::MessageType::ScreenFrame: {
        if (!m_authenticated) return;

        QByteArray imageData;
        int width, height;
        quint32 frameId;
        if (Protocol::parseScreenFramePacket(packet, imageData, width, height, frameId)) {
            QImage image;
            if (image.loadFromData(imageData, "JPEG")) {
                emit screenFrameReceived(image, frameId);
                // Auto-ack
                sendScreenFrameAck(frameId);
            }
        }
        break;
    }

    case Protocol::MessageType::ClipboardData: {
        if (!m_authenticated) return;

        QString mimeType;
        QByteArray clipData;
        if (Protocol::parseClipboardDataPacket(packet, mimeType, clipData)) {
            emit clipboardReceived(mimeType, clipData);
        }
        break;
    }

    case Protocol::MessageType::Ping: {
        QByteArray pong = Protocol::createPongPacket();
        m_socket->write(pong);
        break;
    }

    case Protocol::MessageType::Disconnect:
        m_autoReconnect = false;
        m_socket->disconnectFromHost();
        break;

    default:
        break;
    }
}

void Client::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    emit error(m_socket->errorString());
}

void Client::onReconnectTimer()
{
    m_reconnectTimer->stop();

    if (!m_autoReconnect || m_connected) return;

    m_reconnectAttempts++;
    emit reconnecting(m_reconnectAttempts, MAX_RECONNECT_ATTEMPTS);

    if (m_reconnectAttempts <= MAX_RECONNECT_ATTEMPTS) {
        if (m_useSsl) {
            m_socket->connectToHostEncrypted(m_serverAddress, m_serverPort);
        } else {
            m_socket->connectToHost(m_serverAddress, m_serverPort);
        }
    }
}

void Client::sendCommandOutput(const QString& output)
{
    if (!m_authenticated || !m_connected) return;

    QByteArray packet = Protocol::createCommandOutputPacket(output);
    m_socket->write(packet);
}

void Client::requestScreenShare(bool start)
{
    if (!m_authenticated || !m_connected) return;

    QByteArray packet = Protocol::createScreenShareRequestPacket(start);
    m_socket->write(packet);
}

void Client::sendScreenFrameAck(quint32 frameId)
{
    if (!m_authenticated || !m_connected) return;

    QByteArray packet = Protocol::createScreenFrameAckPacket(frameId);
    m_socket->write(packet);
}
