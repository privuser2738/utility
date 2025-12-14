#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QSslSocket>
#include <QMap>
#include <QTimer>
#include <QImage>

class ScreenCapture;

struct ClientConnection {
    QString id;
    QString name;
    QString address;
    QSslSocket* socket;
    bool authenticated;
    bool sslEstablished;
    bool wantsScreenShare;
    QByteArray buffer;
};

class Server : public QObject
{
    Q_OBJECT

public:
    explicit Server(QObject* parent = nullptr);
    ~Server();

    bool isRunning() const { return m_running; }
    bool isListening() const { return m_running; }
    bool isScreenSharing() const { return m_screenSharing; }
    int clientCount() const { return m_clients.size(); }
    QStringList clientIds() const { return m_clients.keys(); }

public slots:
    void start(int port, const QString& password);
    void stop();
    void startScreenShare();
    void stopScreenShare();

    void broadcastKeyEvent(int vkCode, bool pressed);
    void broadcastMouseEvent(int x, int y, int button, bool pressed);
    void broadcastMouseMove(int x, int y);
    void broadcastCommand(const QString& command, const QString& type);
    void broadcastScreenFrame(const QImage& frame);

    void sendCommandToClient(const QString& clientId, const QString& command, const QString& type);
    void sendScreenFrameToClient(const QString& clientId, const QImage& frame);
    void disconnectClient(const QString& clientId);

    bool useSsl() const { return m_useSsl; }
    void setUseSsl(bool use) { m_useSsl = use; }

signals:
    void started();
    void stopped();
    void clientConnected(const QString& clientId);
    void clientDisconnected(const QString& clientId);
    void clientAuthenticated(const QString& clientId, const QString& clientName);
    void commandOutputReceived(const QString& clientId, const QString& output);
    void error(const QString& message);

private slots:
    void onNewConnection();
    void onClientReadyRead();
    void onClientDisconnected();
    void onClientError(QAbstractSocket::SocketError socketError);
    void onSslErrors(const QList<QSslError>& errors);
    void onClientEncrypted();
    void onPingTimer();

private:
    void processClientData(ClientConnection& client);
    void handlePacket(ClientConnection& client, const QByteArray& packet);
    void broadcast(const QByteArray& data);
    void broadcastToScreenShareClients(const QByteArray& data);
    void sendToClient(const QString& clientId, const QByteArray& data);
    QString generateClientId();
    void setupSslSocket(QSslSocket* socket);

    QTcpServer* m_server;
    QTimer* m_pingTimer;
    ScreenCapture* m_screenCapture = nullptr;
    QMap<QString, ClientConnection> m_clients;
    QMap<QSslSocket*, QString> m_socketToId;

    bool m_running = false;
    bool m_useSsl = true;
    bool m_screenSharing = false;
    QString m_password;
    int m_port = 45679;
};

#endif // SERVER_H
