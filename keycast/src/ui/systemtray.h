#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include <QSystemTrayIcon>
#include <QMenu>

class MainWindow;

class SystemTray : public QSystemTrayIcon
{
    Q_OBJECT

public:
    explicit SystemTray(MainWindow* mainWindow, QObject* parent = nullptr);
    ~SystemTray();

    void showMessage(const QString& title, const QString& message,
                     QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information,
                     int msecs = 3000);

public slots:
    void updateBroadcastStatus(bool active);
    void updateServerStatus(bool running);
    void updateClientStatus(bool connected);

private slots:
    void onActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowHideToggled();
    void onToggleBroadcast();
    void onQuit();

private:
    void setupMenu();
    void updateIcon();
    void updateTooltip();

    MainWindow* m_mainWindow;
    QMenu* m_menu;

    QAction* m_showHideAction;
    QAction* m_broadcastAction;
    QAction* m_serverAction;
    QAction* m_clientAction;
    QAction* m_quitAction;

    bool m_broadcasting = false;
    bool m_serverRunning = false;
    bool m_clientConnected = false;
};

#endif // SYSTEMTRAY_H
