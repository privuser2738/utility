#include "discovery.h"
#include "protocol.h"
#include "settings.h"

#include <QNetworkDatagram>
#include <QNetworkInterface>

Discovery::Discovery(QObject* parent)
    : QObject(parent)
    , m_socket(new QUdpSocket(this))
    , m_broadcastTimer(new QTimer(this))
{
    connect(m_socket, &QUdpSocket::readyRead, this, &Discovery::onReadyRead);
    connect(m_broadcastTimer, &QTimer::timeout, this, &Discovery::onBroadcastTimer);
}

Discovery::~Discovery()
{
    stop();
}

void Discovery::start(int port)
{
    if (m_running) return;

    m_port = port;

    // Bind to the discovery port to receive broadcasts
    if (!m_socket->bind(QHostAddress::Any, m_port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        emit error(QString("Failed to bind discovery socket: %1").arg(m_socket->errorString()));
        return;
    }

    m_running = true;
}

void Discovery::stop()
{
    stopBroadcasting();

    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->close();
    }

    m_running = false;
}

void Discovery::startBroadcasting(const QString& serverName, int serverPort)
{
    if (m_broadcasting) return;

    m_serverName = serverName;
    m_serverPort = serverPort;
    m_broadcasting = true;

    // Broadcast immediately, then every 5 seconds
    onBroadcastTimer();
    m_broadcastTimer->start(5000);
}

void Discovery::stopBroadcasting()
{
    m_broadcastTimer->stop();
    m_broadcasting = false;
}

void Discovery::onReadyRead()
{
    while (m_socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_socket->receiveDatagram();
        QByteArray data = datagram.data();

        QString serverName;
        QString address;
        int port;

        if (Protocol::parseDiscoveryBroadcast(data, serverName, address, port)) {
            // Use the sender's address
            address = datagram.senderAddress().toString();

            // Don't emit our own broadcasts
            bool isOurOwn = false;
            for (const QHostAddress& localAddr : QNetworkInterface::allAddresses()) {
                if (localAddr.toString() == address && port == m_serverPort) {
                    isOurOwn = true;
                    break;
                }
            }

            if (!isOurOwn) {
                emit serverFound(serverName, address, port);
            }
        }
    }
}

void Discovery::onBroadcastTimer()
{
    if (!m_broadcasting) return;

    QByteArray packet = Protocol::createDiscoveryBroadcast(m_serverName, m_serverPort);

    // Broadcast to all network interfaces
    for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
        if (!(iface.flags() & QNetworkInterface::IsUp) ||
            !(iface.flags() & QNetworkInterface::IsRunning) ||
            (iface.flags() & QNetworkInterface::IsLoopBack)) {
            continue;
        }

        for (const QNetworkAddressEntry& entry : iface.addressEntries()) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                QHostAddress broadcast = entry.broadcast();
                if (!broadcast.isNull()) {
                    m_socket->writeDatagram(packet, broadcast, m_port);
                }
            }
        }
    }

    // Also broadcast to 255.255.255.255 as a fallback
    m_socket->writeDatagram(packet, QHostAddress::Broadcast, m_port);
}
