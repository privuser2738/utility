#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QSharedPointer>

class MainWindow;
class SystemTray;
class Server;
class Client;
class Discovery;
class InputCapture;
class InputInjector;
class ShortcutManager;

class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int& argc, char** argv);
    ~Application();

    static Application* instance();

    MainWindow* mainWindow() const { return m_mainWindow; }
    SystemTray* systemTray() const { return m_systemTray; }
    Server* server() const { return m_server; }
    Client* client() const { return m_client; }
    Discovery* discovery() const { return m_discovery; }
    InputCapture* inputCapture() const { return m_inputCapture; }
    InputInjector* inputInjector() const { return m_inputInjector; }
    ShortcutManager* shortcutManager() const { return m_shortcutManager; }

    bool isBroadcasting() const { return m_broadcasting; }

public slots:
    void startServer();
    void stopServer();
    void connectToServer(const QString& address, int port, const QString& password);
    void disconnectFromServer();

    void startBroadcast();
    void stopBroadcast();
    void toggleBroadcast();

    void startDiscovery();
    void stopDiscovery();

signals:
    void serverStarted();
    void serverStopped();
    void clientConnected();
    void clientDisconnected();
    void broadcastStateChanged(bool active);
    void serverDiscovered(const QString& name, const QString& address, int port);

private:
    void initializeComponents();
    void setupConnections();
    void loadSettings();

    MainWindow* m_mainWindow = nullptr;
    SystemTray* m_systemTray = nullptr;
    Server* m_server = nullptr;
    Client* m_client = nullptr;
    Discovery* m_discovery = nullptr;
    InputCapture* m_inputCapture = nullptr;
    InputInjector* m_inputInjector = nullptr;
    ShortcutManager* m_shortcutManager = nullptr;

    bool m_broadcasting = false;
};

#define keycastApp (static_cast<Application*>(QCoreApplication::instance()))

#endif // APPLICATION_H
