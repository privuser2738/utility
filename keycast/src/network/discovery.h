#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QHostAddress>

class Discovery : public QObject
{
    Q_OBJECT

public:
    explicit Discovery(QObject* parent = nullptr);
    ~Discovery();

    bool isRunning() const { return m_running; }

public slots:
    void start(int port);
    void stop();

    // Server: start broadcasting presence
    void startBroadcasting(const QString& serverName, int serverPort);
    void stopBroadcasting();

signals:
    void serverFound(const QString& name, const QString& address, int port);
    void error(const QString& message);

private slots:
    void onReadyRead();
    void onBroadcastTimer();

private:
    QUdpSocket* m_socket;
    QTimer* m_broadcastTimer;

    bool m_running = false;
    bool m_broadcasting = false;

    int m_port = 45678;
    QString m_serverName;
    int m_serverPort = 45679;
};

#endif // DISCOVERY_H
