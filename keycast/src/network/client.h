#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QSslSocket>
#include <QTimer>
#include <QImage>

class Client : public QObject
{
    Q_OBJECT

public:
    explicit Client(QObject* parent = nullptr);
    ~Client();

    bool isConnected() const { return m_connected; }
    bool isAuthenticated() const { return m_authenticated; }
    QString serverName() const { return m_serverName; }
    QString serverAddress() const { return m_serverAddress; }

    bool useSsl() const { return m_useSsl; }
    void setUseSsl(bool use) { m_useSsl = use; }

public slots:
    void connectToServer(const QString& address, int port, const QString& password);
    void disconnect();

    void sendCommandOutput(const QString& output);
    void requestScreenShare(bool start);
    void sendScreenFrameAck(quint32 frameId);

signals:
    void connected();
    void disconnected();
    void sslEstablished();
    void authenticated(const QString& serverName);
    void authenticationFailed(const QString& reason);

    void keyEventReceived(int vkCode, bool pressed);
    void mouseEventReceived(int x, int y, int button, bool pressed);
    void mouseMoveReceived(int x, int y);
    void executeCommandReceived(const QString& command, const QString& type);
    void screenFrameReceived(const QImage& frame, quint32 frameId);
    void clipboardReceived(const QString& mimeType, const QByteArray& data);

    void error(const QString& message);
    void reconnecting(int attempt, int maxAttempts);

private slots:
    void onConnected();
    void onDisconnected();
    void onEncrypted();
    void onSslErrors(const QList<QSslError>& errors);
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);
    void onReconnectTimer();

private:
    void processData();
    void handlePacket(const QByteArray& packet);
    void sendAuthentication();

    QSslSocket* m_socket;
    QTimer* m_reconnectTimer;

    bool m_connected = false;
    bool m_authenticated = false;
    bool m_useSsl = true;
    bool m_sslEstablished = false;

    QString m_serverAddress;
    int m_serverPort = 45679;
    QString m_password;
    QString m_serverName;

    QByteArray m_buffer;

    bool m_autoReconnect = true;
    int m_reconnectAttempts = 0;
    static const int MAX_RECONNECT_ATTEMPTS = 5;
};

#endif // CLIENT_H
