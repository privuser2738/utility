#ifndef REMOTEDESKTOPWINDOW_H
#define REMOTEDESKTOPWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>

class RemoteDesktopWidget;
class Client;

class RemoteDesktopWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit RemoteDesktopWindow(Client* client, QWidget* parent = nullptr);
    ~RemoteDesktopWindow();

public slots:
    void startViewing();
    void stopViewing();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onConnected();
    void onDisconnected();
    void onScreenFrame(const QImage& frame, quint32 frameId);
    void onToggleControl();
    void onToggleScaling();
    void onFullscreen();

    // Forward input to server
    void onKeyPressed(int key, Qt::KeyboardModifiers modifiers);
    void onKeyReleased(int key, Qt::KeyboardModifiers modifiers);
    void onMousePressed(int x, int y, Qt::MouseButton button);
    void onMouseReleased(int x, int y, Qt::MouseButton button);
    void onMouseMoved(int x, int y);

private:
    void setupUi();
    void setupToolbar();
    void updateStatusBar();
    int qtKeyToVk(int key) const;
    int qtButtonToButton(Qt::MouseButton button) const;

    Client* m_client;
    RemoteDesktopWidget* m_desktopWidget;

    QToolBar* m_toolbar;
    QAction* m_controlAction;
    QAction* m_scaleAction;
    QAction* m_fullscreenAction;

    QLabel* m_statusLabel;
    QLabel* m_fpsLabel;

    bool m_viewing = false;
    int m_frameCount = 0;
    qint64 m_lastFpsTime = 0;
};

#endif // REMOTEDESKTOPWINDOW_H
