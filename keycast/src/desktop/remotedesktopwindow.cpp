#include "remotedesktopwindow.h"
#include "remotedesktopwidget.h"
#include "client.h"
#include "protocol.h"

#include <QVBoxLayout>
#include <QDateTime>
#include <QCloseEvent>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

RemoteDesktopWindow::RemoteDesktopWindow(Client* client, QWidget* parent)
    : QMainWindow(parent)
    , m_client(client)
{
    setWindowTitle("Remote Desktop - KeyCast");
    setMinimumSize(800, 600);

    setupUi();
    setupToolbar();

    // Status bar
    m_statusLabel = new QLabel("Disconnected");
    m_fpsLabel = new QLabel("0 FPS");
    statusBar()->addWidget(m_statusLabel);
    statusBar()->addPermanentWidget(m_fpsLabel);

    // Connect to client signals
    connect(m_client, &Client::connected, this, &RemoteDesktopWindow::onConnected);
    connect(m_client, &Client::disconnected, this, &RemoteDesktopWindow::onDisconnected);
    connect(m_client, &Client::screenFrameReceived, this, &RemoteDesktopWindow::onScreenFrame);

    // Connect input signals from widget
    connect(m_desktopWidget, &RemoteDesktopWidget::keyPressed, this, &RemoteDesktopWindow::onKeyPressed);
    connect(m_desktopWidget, &RemoteDesktopWidget::keyReleased, this, &RemoteDesktopWindow::onKeyReleased);
    connect(m_desktopWidget, &RemoteDesktopWidget::mousePressed, this, &RemoteDesktopWindow::onMousePressed);
    connect(m_desktopWidget, &RemoteDesktopWidget::mouseReleased, this, &RemoteDesktopWindow::onMouseReleased);
    connect(m_desktopWidget, &RemoteDesktopWidget::mouseMoved, this, &RemoteDesktopWindow::onMouseMoved);

    updateStatusBar();
}

RemoteDesktopWindow::~RemoteDesktopWindow()
{
    stopViewing();
}

void RemoteDesktopWindow::setupUi()
{
    m_desktopWidget = new RemoteDesktopWidget(this);
    setCentralWidget(m_desktopWidget);
}

void RemoteDesktopWindow::setupToolbar()
{
    m_toolbar = addToolBar("Remote Desktop");
    m_toolbar->setMovable(false);

    m_controlAction = m_toolbar->addAction("Control: ON");
    m_controlAction->setCheckable(true);
    m_controlAction->setChecked(true);
    connect(m_controlAction, &QAction::triggered, this, &RemoteDesktopWindow::onToggleControl);

    m_scaleAction = m_toolbar->addAction("Scale to Fit");
    m_scaleAction->setCheckable(true);
    m_scaleAction->setChecked(true);
    connect(m_scaleAction, &QAction::triggered, this, &RemoteDesktopWindow::onToggleScaling);

    m_toolbar->addSeparator();

    m_fullscreenAction = m_toolbar->addAction("Fullscreen");
    connect(m_fullscreenAction, &QAction::triggered, this, &RemoteDesktopWindow::onFullscreen);
}

void RemoteDesktopWindow::startViewing()
{
    if (m_viewing) return;

    m_viewing = true;
    m_frameCount = 0;
    m_lastFpsTime = QDateTime::currentMSecsSinceEpoch();

    if (m_client->isAuthenticated()) {
        m_client->requestScreenShare(true);
    }

    m_desktopWidget->setConnected(true);
    updateStatusBar();
}

void RemoteDesktopWindow::stopViewing()
{
    if (!m_viewing) return;

    m_viewing = false;

    if (m_client->isAuthenticated()) {
        m_client->requestScreenShare(false);
    }

    m_desktopWidget->setConnected(false);
    m_desktopWidget->clear();
    updateStatusBar();
}

void RemoteDesktopWindow::closeEvent(QCloseEvent* event)
{
    stopViewing();
    event->accept();
}

void RemoteDesktopWindow::onConnected()
{
    if (m_viewing) {
        m_client->requestScreenShare(true);
    }
    updateStatusBar();
}

void RemoteDesktopWindow::onDisconnected()
{
    m_desktopWidget->setConnected(false);
    m_desktopWidget->clear();
    updateStatusBar();
}

void RemoteDesktopWindow::onScreenFrame(const QImage& frame, quint32 frameId)
{
    Q_UNUSED(frameId)

    if (!m_viewing) return;

    m_desktopWidget->updateFrame(frame);

    // Calculate FPS
    m_frameCount++;
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_lastFpsTime >= 1000) {
        int fps = m_frameCount * 1000 / (now - m_lastFpsTime);
        m_fpsLabel->setText(QString("%1 FPS").arg(fps));
        m_frameCount = 0;
        m_lastFpsTime = now;
    }
}

void RemoteDesktopWindow::onToggleControl()
{
    bool enabled = m_controlAction->isChecked();
    m_desktopWidget->setControlEnabled(enabled);
    m_controlAction->setText(enabled ? "Control: ON" : "Control: OFF");
}

void RemoteDesktopWindow::onToggleScaling()
{
    bool scale = m_scaleAction->isChecked();
    m_desktopWidget->setScaleToFit(scale);
}

void RemoteDesktopWindow::onFullscreen()
{
    if (isFullScreen()) {
        showNormal();
        m_toolbar->show();
        statusBar()->show();
        m_fullscreenAction->setText("Fullscreen");
    } else {
        m_toolbar->hide();
        statusBar()->hide();
        showFullScreen();
        m_fullscreenAction->setText("Exit Fullscreen");
    }
}

void RemoteDesktopWindow::updateStatusBar()
{
    if (m_client->isAuthenticated()) {
        m_statusLabel->setText(QString("Connected to %1").arg(m_client->serverName()));
    } else if (m_client->isConnected()) {
        m_statusLabel->setText("Authenticating...");
    } else {
        m_statusLabel->setText("Disconnected");
    }
}

void RemoteDesktopWindow::onKeyPressed(int key, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers)

    if (!m_client->isAuthenticated()) return;

    int vk = qtKeyToVk(key);
    if (vk > 0) {
        // Send key press to server which will broadcast to clients
        // For now, we use the protocol directly
        QByteArray packet = Protocol::createKeyEventPacket(vk, true);
        // TODO: Send via client
    }
}

void RemoteDesktopWindow::onKeyReleased(int key, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers)

    if (!m_client->isAuthenticated()) return;

    int vk = qtKeyToVk(key);
    if (vk > 0) {
        QByteArray packet = Protocol::createKeyEventPacket(vk, false);
        // TODO: Send via client
    }
}

void RemoteDesktopWindow::onMousePressed(int x, int y, Qt::MouseButton button)
{
    if (!m_client->isAuthenticated()) return;

    int btn = qtButtonToButton(button);
    QByteArray packet = Protocol::createMouseEventPacket(x, y, btn, true);
    // TODO: Send via client
}

void RemoteDesktopWindow::onMouseReleased(int x, int y, Qt::MouseButton button)
{
    if (!m_client->isAuthenticated()) return;

    int btn = qtButtonToButton(button);
    QByteArray packet = Protocol::createMouseEventPacket(x, y, btn, false);
    // TODO: Send via client
}

void RemoteDesktopWindow::onMouseMoved(int x, int y)
{
    if (!m_client->isAuthenticated()) return;

    QByteArray packet = Protocol::createMouseMovePacket(x, y);
    // TODO: Send via client
}

int RemoteDesktopWindow::qtKeyToVk(int key) const
{
#ifdef Q_OS_WIN
    // Map Qt keys to Windows virtual key codes
    switch (key) {
    case Qt::Key_Escape: return VK_ESCAPE;
    case Qt::Key_Tab: return VK_TAB;
    case Qt::Key_Backspace: return VK_BACK;
    case Qt::Key_Return:
    case Qt::Key_Enter: return VK_RETURN;
    case Qt::Key_Insert: return VK_INSERT;
    case Qt::Key_Delete: return VK_DELETE;
    case Qt::Key_Home: return VK_HOME;
    case Qt::Key_End: return VK_END;
    case Qt::Key_Left: return VK_LEFT;
    case Qt::Key_Up: return VK_UP;
    case Qt::Key_Right: return VK_RIGHT;
    case Qt::Key_Down: return VK_DOWN;
    case Qt::Key_PageUp: return VK_PRIOR;
    case Qt::Key_PageDown: return VK_NEXT;
    case Qt::Key_Shift: return VK_SHIFT;
    case Qt::Key_Control: return VK_CONTROL;
    case Qt::Key_Alt: return VK_MENU;
    case Qt::Key_CapsLock: return VK_CAPITAL;
    case Qt::Key_NumLock: return VK_NUMLOCK;
    case Qt::Key_ScrollLock: return VK_SCROLL;
    case Qt::Key_F1: return VK_F1;
    case Qt::Key_F2: return VK_F2;
    case Qt::Key_F3: return VK_F3;
    case Qt::Key_F4: return VK_F4;
    case Qt::Key_F5: return VK_F5;
    case Qt::Key_F6: return VK_F6;
    case Qt::Key_F7: return VK_F7;
    case Qt::Key_F8: return VK_F8;
    case Qt::Key_F9: return VK_F9;
    case Qt::Key_F10: return VK_F10;
    case Qt::Key_F11: return VK_F11;
    case Qt::Key_F12: return VK_F12;
    case Qt::Key_Space: return VK_SPACE;
    default:
        // For letters and numbers, Qt key codes match ASCII
        if (key >= Qt::Key_A && key <= Qt::Key_Z) {
            return key; // 'A' to 'Z'
        }
        if (key >= Qt::Key_0 && key <= Qt::Key_9) {
            return key; // '0' to '9'
        }
        return 0;
    }
#else
    return key;
#endif
}

int RemoteDesktopWindow::qtButtonToButton(Qt::MouseButton button) const
{
    switch (button) {
    case Qt::LeftButton: return 1;
    case Qt::RightButton: return 2;
    case Qt::MiddleButton: return 3;
    default: return 0;
    }
}
