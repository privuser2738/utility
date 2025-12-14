#include "server.h"
#include "protocol.h"
#include "settings.h"
#include "sslconfig.h"
#include "screencapture.h"

#include <QUuid>
#include <QDateTime>
#include <QBuffer>

Server::Server(QObject* parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_pingTimer(new QTimer(this))
{
    connect(m_server, &QTcpServer::newConnection, this, &Server::onNewConnection);
    connect(m_pingTimer, &QTimer::timeout, this, &Server::onPingTimer);
}

Server::~Server()
{
    stop();
}

void Server::start(int port, const QString& password)
{
    if (m_running) return;

    m_port = port;
    m_password = password;

    if (!m_server->listen(QHostAddress::Any, m_port)) {
        emit error(QString("Failed to start server: %1").arg(m_server->errorString()));
        return;
    }

    m_running = true;
    m_pingTimer->start(30000); // Ping every 30 seconds

    emit started();
}

void Server::stop()
{
    if (!m_running) return;

    stopScreenShare();
    m_pingTimer->stop();

    // Disconnect all clients
    QByteArray disconnectPacket = Protocol::createDisconnectPacket();
    for (auto& client : m_clients) {
        if (client.socket && client.socket->isOpen()) {
            client.socket->write(disconnectPacket);
            client.socket->flush();
            client.socket->disconnectFromHost();
        }
    }
    m_clients.clear();
    m_socketToId.clear();

    m_server->close();
    m_running = false;

    emit stopped();
}

void Server::startScreenShare()
{
    if (m_screenSharing) return;

    if (!m_screenCapture) {
        m_screenCapture = new ScreenCapture(this);
        connect(m_screenCapture, &ScreenCapture::frameCaptured, this, &Server::broadcastScreenFrame);
    }

    m_screenCapture->start();
    m_screenSharing = true;
}

void Server::stopScreenShare()
{
    if (!m_screenSharing) return;

    if (m_screenCapture) {
        m_screenCapture->stop();
    }

    m_screenSharing = false;
}

void Server::setupSslSocket(QSslSocket* socket)
{
    if (!m_useSsl) return;

    SslConfig* ssl = SslConfig::instance();
    if (ssl->hasCertificate()) {
        socket->setSslConfiguration(ssl->serverConfiguration());
    }
}

void Server::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket* tcpSocket = m_server->nextPendingConnection();

        // Create SSL socket
        QSslSocket* socket = new QSslSocket(this);
        socket->setSocketDescriptor(tcpSocket->socketDescriptor());
        tcpSocket->setParent(nullptr);
        delete tcpSocket;

        QString clientId = generateClientId();

        ClientConnection client;
        client.id = clientId;
        client.name = "Unknown";
        client.address = socket->peerAddress().toString();
        client.socket = socket;
        client.authenticated = false;
        client.sslEstablished = false;
        client.wantsScreenShare = false;

        m_clients[clientId] = client;
        m_socketToId[socket] = clientId;

        connect(socket, &QSslSocket::readyRead, this, &Server::onClientReadyRead);
        connect(socket, &QSslSocket::disconnected, this, &Server::onClientDisconnected);
        connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
                this, &Server::onClientError);
        connect(socket, QOverload<const QList<QSslError>&>::of(&QSslSocket::sslErrors),
                this, &Server::onSslErrors);
        connect(socket, &QSslSocket::encrypted, this, &Server::onClientEncrypted);

        if (m_useSsl && SslConfig::instance()->hasCertificate()) {
            setupSslSocket(socket);
            socket->startServerEncryption();
        } else {
            // No SSL, mark as established
            m_clients[clientId].sslEstablished = true;
            emit clientConnected(clientId);
        }
    }
}

void Server::onSslErrors(const QList<QSslError>& errors)
{
    QSslSocket* socket = qobject_cast<QSslSocket*>(sender());
    if (!socket) return;

    // Ignore self-signed certificate errors for now
    socket->ignoreSslErrors();
}

void Server::onClientEncrypted()
{
    QSslSocket* socket = qobject_cast<QSslSocket*>(sender());
    if (!socket) return;

    QString clientId = m_socketToId.value(socket);
    if (clientId.isEmpty() || !m_clients.contains(clientId)) return;

    m_clients[clientId].sslEstablished = true;
    emit clientConnected(clientId);
}

void Server::onClientReadyRead()
{
    QSslSocket* socket = qobject_cast<QSslSocket*>(sender());
    if (!socket) return;

    QString clientId = m_socketToId.value(socket);
    if (clientId.isEmpty() || !m_clients.contains(clientId)) return;

    ClientConnection& client = m_clients[clientId];
    client.buffer.append(socket->readAll());

    processClientData(client);
}

void Server::processClientData(ClientConnection& client)
{
    while (client.buffer.size() >= 11) { // Minimum header size
        Protocol::PacketHeader header;
        if (!Protocol::parsePacketHeader(client.buffer, header)) {
            // Invalid packet, clear buffer
            client.buffer.clear();
            return;
        }

        int totalSize = 11 + header.payloadSize;
        if (client.buffer.size() < totalSize) {
            // Wait for more data
            return;
        }

        // Extract complete packet
        QByteArray packet = client.buffer.left(totalSize);
        client.buffer.remove(0, totalSize);

        handlePacket(client, packet);
    }
}

void Server::handlePacket(ClientConnection& client, const QByteArray& packet)
{
    Protocol::PacketHeader header;
    if (!Protocol::parsePacketHeader(packet, header)) return;

    switch (header.type) {
    case Protocol::MessageType::Auth: {
        QString password, clientName;
        if (Protocol::parseAuthPacket(packet, password, clientName)) {
            if (m_password.isEmpty() || password == m_password) {
                client.authenticated = true;
                client.name = clientName;

                QByteArray response = Protocol::createAuthResponsePacket(
                    Protocol::AuthResult::Success,
                    Settings::instance()->computerName()
                );
                client.socket->write(response);

                emit clientAuthenticated(client.id, client.name);
            } else {
                QByteArray response = Protocol::createAuthResponsePacket(
                    Protocol::AuthResult::InvalidPassword
                );
                client.socket->write(response);
                client.socket->flush();
                client.socket->disconnectFromHost();
            }
        }
        break;
    }

    case Protocol::MessageType::Pong:
        // Client responded to ping, connection is alive
        break;

    case Protocol::MessageType::CommandOutput: {
        if (!client.authenticated) return;

        QString output;
        if (Protocol::parseCommandOutputPacket(packet, output)) {
            emit commandOutputReceived(client.id, output);
        }
        break;
    }

    case Protocol::MessageType::ScreenShareRequest:
    case Protocol::MessageType::ScreenShareStart: {
        if (!client.authenticated) return;
        client.wantsScreenShare = true;
        break;
    }

    case Protocol::MessageType::ScreenShareStop: {
        if (!client.authenticated) return;
        client.wantsScreenShare = false;
        break;
    }

    case Protocol::MessageType::ScreenFrameAck: {
        // Client acknowledged frame receipt
        break;
    }

    case Protocol::MessageType::Disconnect:
        if (client.socket) {
            client.socket->disconnectFromHost();
        }
        break;

    default:
        break;
    }
}

void Server::onClientDisconnected()
{
    QSslSocket* socket = qobject_cast<QSslSocket*>(sender());
    if (!socket) return;

    QString clientId = m_socketToId.value(socket);
    if (!clientId.isEmpty()) {
        m_clients.remove(clientId);
        m_socketToId.remove(socket);
        emit clientDisconnected(clientId);
    }

    socket->deleteLater();
}

void Server::onClientError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    QSslSocket* socket = qobject_cast<QSslSocket*>(sender());
    if (socket) {
        socket->disconnectFromHost();
    }
}

void Server::onPingTimer()
{
    QByteArray pingPacket = Protocol::createPingPacket();
    broadcast(pingPacket);
}

void Server::broadcast(const QByteArray& data)
{
    for (auto& client : m_clients) {
        if (client.authenticated && client.socket && client.socket->isOpen()) {
            client.socket->write(data);
        }
    }
}

void Server::broadcastToScreenShareClients(const QByteArray& data)
{
    for (auto& client : m_clients) {
        if (client.authenticated && client.wantsScreenShare && client.socket && client.socket->isOpen()) {
            client.socket->write(data);
        }
    }
}

void Server::sendToClient(const QString& clientId, const QByteArray& data)
{
    if (m_clients.contains(clientId)) {
        ClientConnection& client = m_clients[clientId];
        if (client.authenticated && client.socket && client.socket->isOpen()) {
            client.socket->write(data);
        }
    }
}

void Server::broadcastKeyEvent(int vkCode, bool pressed)
{
    QByteArray packet = Protocol::createKeyEventPacket(vkCode, pressed);
    broadcast(packet);
}

void Server::broadcastMouseEvent(int x, int y, int button, bool pressed)
{
    QByteArray packet = Protocol::createMouseEventPacket(x, y, button, pressed);
    broadcast(packet);
}

void Server::broadcastMouseMove(int x, int y)
{
    QByteArray packet = Protocol::createMouseMovePacket(x, y);
    broadcast(packet);
}

void Server::broadcastCommand(const QString& command, const QString& type)
{
    QByteArray packet = Protocol::createExecuteCommandPacket(command, type);
    broadcast(packet);
}

void Server::broadcastScreenFrame(const QImage& frame)
{
    static quint32 frameId = 0;
    frameId++;

    // Compress as JPEG for smaller size
    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    frame.save(&buffer, "JPEG", 70); // 70% quality
    buffer.close();

    QByteArray packet = Protocol::createScreenFramePacket(imageData, frame.width(), frame.height(), frameId);
    broadcastToScreenShareClients(packet);
}

void Server::sendCommandToClient(const QString& clientId, const QString& command, const QString& type)
{
    QByteArray packet = Protocol::createExecuteCommandPacket(command, type);
    sendToClient(clientId, packet);
}

void Server::sendScreenFrameToClient(const QString& clientId, const QImage& frame)
{
    static quint32 frameId = 0;
    frameId++;

    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    frame.save(&buffer, "JPEG", 70);
    buffer.close();

    QByteArray packet = Protocol::createScreenFramePacket(imageData, frame.width(), frame.height(), frameId);
    sendToClient(clientId, packet);
}

void Server::disconnectClient(const QString& clientId)
{
    if (m_clients.contains(clientId)) {
        ClientConnection& client = m_clients[clientId];
        if (client.socket) {
            QByteArray packet = Protocol::createDisconnectPacket();
            client.socket->write(packet);
            client.socket->flush();
            client.socket->disconnectFromHost();
        }
    }
}

QString Server::generateClientId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
}
