#include "application.h"
#include "settings.h"
#include "mainwindow.h"
#include "systemtray.h"
#include "server.h"
#include "client.h"
#include "discovery.h"
#include "inputcapture.h"
#include "inputinjector.h"
#include "shortcutmanager.h"

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{
    setApplicationName("KeyCast");
    setApplicationVersion("1.0.0");
    setOrganizationName("KeyCast");
    setQuitOnLastWindowClosed(false);

    initializeComponents();
    setupConnections();
    loadSettings();
}

Application::~Application()
{
    Settings::instance()->sync();

    delete m_shortcutManager;
    delete m_inputInjector;
    delete m_inputCapture;
    delete m_discovery;
    delete m_client;
    delete m_server;
    delete m_systemTray;
    delete m_mainWindow;
}

Application* Application::instance()
{
    return keycastApp;
}

void Application::initializeComponents()
{
    // Create network components
    m_server = new Server(this);
    m_client = new Client(this);
    m_discovery = new Discovery(this);

    // Create input components
    m_inputCapture = new InputCapture(this);
    m_inputInjector = new InputInjector(this);

    // Create shortcut manager
    m_shortcutManager = new ShortcutManager(this);

    // Create UI components
    m_mainWindow = new MainWindow();
    m_systemTray = new SystemTray(m_mainWindow);
}

void Application::setupConnections()
{
    // Server connections
    connect(m_server, &Server::started, this, &Application::serverStarted);
    connect(m_server, &Server::stopped, this, &Application::serverStopped);
    connect(m_server, &Server::clientConnected, this, [this](const QString& clientId) {
        if (m_systemTray && Settings::instance()->showTrayNotifications()) {
            m_systemTray->showMessage("Client Connected", clientId);
        }
    });

    // Client connections
    connect(m_client, &Client::connected, this, &Application::clientConnected);
    connect(m_client, &Client::disconnected, this, &Application::clientDisconnected);

    // Discovery connections
    connect(m_discovery, &Discovery::serverFound, this, &Application::serverDiscovered);

    // Input capture to server broadcast
    connect(m_inputCapture, &InputCapture::keyEvent, this, [this](int vkCode, bool pressed) {
        if (m_broadcasting && m_server->isRunning()) {
            m_server->broadcastKeyEvent(vkCode, pressed);
        }
    });

    connect(m_inputCapture, &InputCapture::mouseEvent, this, [this](int x, int y, int button, bool pressed) {
        if (m_broadcasting && m_server->isRunning() && Settings::instance()->broadcastMouse()) {
            m_server->broadcastMouseEvent(x, y, button, pressed);
        }
    });

    // Client receives input and injects it
    connect(m_client, &Client::keyEventReceived, m_inputInjector, &InputInjector::injectKeyEvent);
    connect(m_client, &Client::mouseEventReceived, m_inputInjector, &InputInjector::injectMouseEvent);
    connect(m_client, &Client::executeCommandReceived, this, [this](const QString& command, const QString& type) {
        m_shortcutManager->executeCommand(command, type);
    });
}

void Application::loadSettings()
{
    Settings* settings = Settings::instance();

    // Auto-start server if configured
    if (settings->autoStartServer() && settings->serverModeEnabled()) {
        startServer();
    }

    // Auto-start discovery if configured
    if (settings->enableDiscovery()) {
        startDiscovery();
    }

    // Load shortcuts
    m_shortcutManager->loadShortcuts();

    // Show main window unless start minimized
    // Always use showNormal() to ensure window is visible and not minimized
    if (!settings->startMinimized()) {
        m_mainWindow->showNormal();
        m_mainWindow->activateWindow();
        m_mainWindow->raise();
    }

    // Always show system tray
    m_systemTray->show();
}

void Application::startServer()
{
    Settings* settings = Settings::instance();
    m_server->start(settings->serverPort(), settings->serverPassword());
}

void Application::stopServer()
{
    m_server->stop();
}

void Application::connectToServer(const QString& address, int port, const QString& password)
{
    m_client->connectToServer(address, port, password);
}

void Application::disconnectFromServer()
{
    m_client->disconnect();
}

void Application::startBroadcast()
{
    if (!m_broadcasting) {
        m_broadcasting = true;
        m_inputCapture->start();
        emit broadcastStateChanged(true);

        if (m_systemTray && Settings::instance()->showTrayNotifications()) {
            m_systemTray->showMessage("Broadcast Started", "Input is now being broadcast to connected clients");
        }
    }
}

void Application::stopBroadcast()
{
    if (m_broadcasting) {
        m_broadcasting = false;
        m_inputCapture->stop();
        emit broadcastStateChanged(false);

        if (m_systemTray && Settings::instance()->showTrayNotifications()) {
            m_systemTray->showMessage("Broadcast Stopped", "Input broadcast has been stopped");
        }
    }
}

void Application::toggleBroadcast()
{
    if (m_broadcasting) {
        stopBroadcast();
    } else {
        startBroadcast();
    }
}

void Application::startDiscovery()
{
    m_discovery->start(Settings::instance()->discoveryPort());
}

void Application::stopDiscovery()
{
    m_discovery->stop();
}
