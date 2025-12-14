#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QComboBox>

class RemoteDesktopWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public slots:
    void updateServerStatus(bool running);
    void updateClientStatus(bool connected);
    void updateBroadcastStatus(bool active);
    void addDiscoveredServer(const QString& name, const QString& address, int port);
    void addConnectedClient(const QString& clientId, const QString& clientName);
    void removeConnectedClient(const QString& clientId);
    void appendTerminalOutput(const QString& text);

protected:
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;

private slots:
    void onStartServerClicked();
    void onStopServerClicked();
    void onConnectClicked();
    void onDisconnectClicked();
    void onToggleBroadcastClicked();
    void onAddShortcutClicked();
    void onRemoveShortcutClicked();
    void onEditShortcutClicked();
    void onSendTerminalCommand();
    void onSettingsClicked();
    void onServerSelectionChanged();
    void onOpenRemoteDesktopClicked();
    void onToggleScreenShareClicked();

private:
    void setupUi();
    void setupBroadcastTab();
    void setupShortcutsTab();
    void setupConnectionsTab();
    void setupTerminalTab();
    void setupRemoteDesktopTab();
    void setupToolbar();
    void loadSettings();
    void saveSettings();
    void refreshShortcutsList();
    void refreshServerList();

    QTabWidget* m_tabWidget;

    // Broadcast tab
    QWidget* m_broadcastTab;
    QPushButton* m_toggleBroadcastBtn;
    QCheckBox* m_broadcastKeyboardCheck;
    QCheckBox* m_broadcastMouseCheck;
    QLabel* m_broadcastStatusLabel;
    QListWidget* m_connectedClientsList;

    // Shortcuts tab
    QWidget* m_shortcutsTab;
    QTableWidget* m_shortcutsTable;
    QPushButton* m_addShortcutBtn;
    QPushButton* m_editShortcutBtn;
    QPushButton* m_removeShortcutBtn;

    // Connections tab
    QWidget* m_connectionsTab;
    QListWidget* m_serverList;
    QLineEdit* m_serverAddressEdit;
    QSpinBox* m_serverPortSpin;
    QLineEdit* m_passwordEdit;
    QPushButton* m_connectBtn;
    QPushButton* m_disconnectBtn;
    QPushButton* m_startServerBtn;
    QPushButton* m_stopServerBtn;
    QLabel* m_serverStatusLabel;
    QLabel* m_clientStatusLabel;

    // Terminal tab
    QWidget* m_terminalTab;
    QTextEdit* m_terminalOutput;
    QLineEdit* m_terminalInput;
    QPushButton* m_sendCommandBtn;
    QComboBox* m_targetClientCombo;

    // Remote Desktop tab
    QWidget* m_remoteDesktopTab;
    QPushButton* m_openRemoteDesktopBtn;
    QPushButton* m_toggleScreenShareBtn;
    QLabel* m_screenShareStatusLabel;
    RemoteDesktopWindow* m_remoteDesktopWindow = nullptr;
};

#endif // MAINWINDOW_H
