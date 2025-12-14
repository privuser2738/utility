#include "systemtray.h"
#include "mainwindow.h"
#include "application.h"
#include "settings.h"

#include <QApplication>
#include <QPainter>
#include <QPixmap>

SystemTray::SystemTray(MainWindow* mainWindow, QObject* parent)
    : QSystemTrayIcon(parent)
    , m_mainWindow(mainWindow)
{
    setupMenu();
    updateIcon();
    updateTooltip();

    connect(this, &QSystemTrayIcon::activated, this, &SystemTray::onActivated);

    // Connect to application signals
    connect(keycastApp, &Application::broadcastStateChanged, this, &SystemTray::updateBroadcastStatus);
    connect(keycastApp, &Application::serverStarted, this, [this]() { updateServerStatus(true); });
    connect(keycastApp, &Application::serverStopped, this, [this]() { updateServerStatus(false); });
    connect(keycastApp, &Application::clientConnected, this, [this]() { updateClientStatus(true); });
    connect(keycastApp, &Application::clientDisconnected, this, [this]() { updateClientStatus(false); });
}

SystemTray::~SystemTray()
{
    delete m_menu;
}

void SystemTray::setupMenu()
{
    m_menu = new QMenu();

    m_showHideAction = m_menu->addAction("Show Window");
    connect(m_showHideAction, &QAction::triggered, this, &SystemTray::onShowHideToggled);

    m_menu->addSeparator();

    m_broadcastAction = m_menu->addAction("Start Broadcast");
    connect(m_broadcastAction, &QAction::triggered, this, &SystemTray::onToggleBroadcast);

    m_serverAction = m_menu->addAction("Server: Stopped");
    m_serverAction->setEnabled(false);

    m_clientAction = m_menu->addAction("Client: Disconnected");
    m_clientAction->setEnabled(false);

    m_menu->addSeparator();

    m_quitAction = m_menu->addAction("Quit");
    connect(m_quitAction, &QAction::triggered, this, &SystemTray::onQuit);

    setContextMenu(m_menu);
}

void SystemTray::updateIcon()
{
    // Create a simple icon programmatically
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw base circle
    if (m_broadcasting) {
        painter.setBrush(QColor(0, 200, 0));  // Green when broadcasting
    } else if (m_serverRunning || m_clientConnected) {
        painter.setBrush(QColor(100, 150, 255));  // Blue when connected
    } else {
        painter.setBrush(QColor(128, 128, 128));  // Gray when idle
    }

    painter.setPen(QPen(Qt::white, 2));
    painter.drawEllipse(2, 2, 28, 28);

    // Draw "K" for KeyCast
    painter.setPen(QPen(Qt::white, 3));
    QFont font("Arial", 14, QFont::Bold);
    painter.setFont(font);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, "K");

    painter.end();

    setIcon(QIcon(pixmap));
}

void SystemTray::updateTooltip()
{
    QString tooltip = "KeyCast";

    if (m_broadcasting) {
        tooltip += " - Broadcasting";
    }

    if (m_serverRunning) {
        tooltip += QString(" | Server: Running (Port %1)").arg(Settings::instance()->serverPort());
    }

    if (m_clientConnected) {
        tooltip += " | Client: Connected";
    }

    setToolTip(tooltip);
}

void SystemTray::showMessage(const QString& title, const QString& message,
                             QSystemTrayIcon::MessageIcon icon, int msecs)
{
    if (Settings::instance()->showTrayNotifications()) {
        QSystemTrayIcon::showMessage(title, message, icon, msecs);
    }
}

void SystemTray::updateBroadcastStatus(bool active)
{
    m_broadcasting = active;

    if (active) {
        m_broadcastAction->setText("Stop Broadcast");
    } else {
        m_broadcastAction->setText("Start Broadcast");
    }

    updateIcon();
    updateTooltip();
}

void SystemTray::updateServerStatus(bool running)
{
    m_serverRunning = running;

    if (running) {
        m_serverAction->setText(QString("Server: Running (Port %1)").arg(Settings::instance()->serverPort()));
    } else {
        m_serverAction->setText("Server: Stopped");
    }

    updateIcon();
    updateTooltip();
}

void SystemTray::updateClientStatus(bool connected)
{
    m_clientConnected = connected;

    if (connected) {
        m_clientAction->setText("Client: Connected");
    } else {
        m_clientAction->setText("Client: Disconnected");
    }

    updateIcon();
    updateTooltip();
}

void SystemTray::onActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        onShowHideToggled();
        break;
    default:
        break;
    }
}

void SystemTray::onShowHideToggled()
{
    if (m_mainWindow->isVisible()) {
        m_mainWindow->hide();
        m_showHideAction->setText("Show Window");
    } else {
        m_mainWindow->show();
        m_mainWindow->raise();
        m_mainWindow->activateWindow();
        m_showHideAction->setText("Hide Window");
    }
}

void SystemTray::onToggleBroadcast()
{
    keycastApp->toggleBroadcast();
}

void SystemTray::onQuit()
{
    Settings::instance()->sync();
    QApplication::quit();
}
