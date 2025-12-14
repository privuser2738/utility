#include "mainwindow.h"
#include "settingsdialog.h"
#include "application.h"
#include "settings.h"
#include "server.h"
#include "client.h"
#include "shortcutmanager.h"
#include "remotedesktopwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QToolBar>
#include <QAction>
#include <QCloseEvent>
#include <QMessageBox>
#include <QComboBox>
#include <QHeaderView>
#include <QInputDialog>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("KeyCast - Input Broadcast & Remote Control");
    setMinimumSize(800, 600);

    setupUi();
    setupToolbar();
    loadSettings();

    // Connect to application signals
    connect(keycastApp, &Application::serverStarted, this, [this]() {
        updateServerStatus(true);
    });
    connect(keycastApp, &Application::serverStopped, this, [this]() {
        updateServerStatus(false);
    });
    connect(keycastApp, &Application::clientConnected, this, [this]() {
        updateClientStatus(true);
    });
    connect(keycastApp, &Application::clientDisconnected, this, [this]() {
        updateClientStatus(false);
    });
    connect(keycastApp, &Application::broadcastStateChanged, this, &MainWindow::updateBroadcastStatus);
    connect(keycastApp, &Application::serverDiscovered, this, &MainWindow::addDiscoveredServer);
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::setupUi()
{
    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);

    setupBroadcastTab();
    setupShortcutsTab();
    setupConnectionsTab();
    setupTerminalTab();
    setupRemoteDesktopTab();
}

void MainWindow::setupBroadcastTab()
{
    m_broadcastTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_broadcastTab);

    // Status group
    QGroupBox* statusGroup = new QGroupBox("Broadcast Status");
    QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);

    m_broadcastStatusLabel = new QLabel("Broadcast: Inactive");
    m_broadcastStatusLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    statusLayout->addWidget(m_broadcastStatusLabel);

    m_toggleBroadcastBtn = new QPushButton("Start Broadcast");
    m_toggleBroadcastBtn->setMinimumHeight(50);
    m_toggleBroadcastBtn->setStyleSheet("font-size: 14px;");
    connect(m_toggleBroadcastBtn, &QPushButton::clicked, this, &MainWindow::onToggleBroadcastClicked);
    statusLayout->addWidget(m_toggleBroadcastBtn);

    layout->addWidget(statusGroup);

    // Options group
    QGroupBox* optionsGroup = new QGroupBox("Broadcast Options");
    QVBoxLayout* optionsLayout = new QVBoxLayout(optionsGroup);

    m_broadcastKeyboardCheck = new QCheckBox("Broadcast Keyboard Input");
    m_broadcastKeyboardCheck->setChecked(Settings::instance()->broadcastKeyboard());
    connect(m_broadcastKeyboardCheck, &QCheckBox::toggled, Settings::instance(), &Settings::setBroadcastKeyboard);
    optionsLayout->addWidget(m_broadcastKeyboardCheck);

    m_broadcastMouseCheck = new QCheckBox("Broadcast Mouse Input");
    m_broadcastMouseCheck->setChecked(Settings::instance()->broadcastMouse());
    connect(m_broadcastMouseCheck, &QCheckBox::toggled, Settings::instance(), &Settings::setBroadcastMouse);
    optionsLayout->addWidget(m_broadcastMouseCheck);

    layout->addWidget(optionsGroup);

    // Connected clients group
    QGroupBox* clientsGroup = new QGroupBox("Connected Clients");
    QVBoxLayout* clientsLayout = new QVBoxLayout(clientsGroup);

    m_connectedClientsList = new QListWidget();
    clientsLayout->addWidget(m_connectedClientsList);

    layout->addWidget(clientsGroup);

    m_tabWidget->addTab(m_broadcastTab, "Broadcast");
}

void MainWindow::setupShortcutsTab()
{
    m_shortcutsTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_shortcutsTab);

    m_shortcutsTable = new QTableWidget();
    m_shortcutsTable->setColumnCount(4);
    m_shortcutsTable->setHorizontalHeaderLabels({"Name", "Hotkey", "Type", "Command"});
    m_shortcutsTable->horizontalHeader()->setStretchLastSection(true);
    m_shortcutsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_shortcutsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(m_shortcutsTable);

    QHBoxLayout* btnLayout = new QHBoxLayout();

    m_addShortcutBtn = new QPushButton("Add");
    connect(m_addShortcutBtn, &QPushButton::clicked, this, &MainWindow::onAddShortcutClicked);
    btnLayout->addWidget(m_addShortcutBtn);

    m_editShortcutBtn = new QPushButton("Edit");
    connect(m_editShortcutBtn, &QPushButton::clicked, this, &MainWindow::onEditShortcutClicked);
    btnLayout->addWidget(m_editShortcutBtn);

    m_removeShortcutBtn = new QPushButton("Remove");
    connect(m_removeShortcutBtn, &QPushButton::clicked, this, &MainWindow::onRemoveShortcutClicked);
    btnLayout->addWidget(m_removeShortcutBtn);

    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    m_tabWidget->addTab(m_shortcutsTab, "Shortcuts");

    refreshShortcutsList();
}

void MainWindow::setupConnectionsTab()
{
    m_connectionsTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_connectionsTab);

    // Server mode group
    QGroupBox* serverGroup = new QGroupBox("Server Mode");
    QVBoxLayout* serverLayout = new QVBoxLayout(serverGroup);

    QHBoxLayout* serverStatusLayout = new QHBoxLayout();
    m_serverStatusLabel = new QLabel("Server: Stopped");
    serverStatusLayout->addWidget(m_serverStatusLabel);
    serverStatusLayout->addStretch();

    m_startServerBtn = new QPushButton("Start Server");
    connect(m_startServerBtn, &QPushButton::clicked, this, &MainWindow::onStartServerClicked);
    serverStatusLayout->addWidget(m_startServerBtn);

    m_stopServerBtn = new QPushButton("Stop Server");
    m_stopServerBtn->setEnabled(false);
    connect(m_stopServerBtn, &QPushButton::clicked, this, &MainWindow::onStopServerClicked);
    serverStatusLayout->addWidget(m_stopServerBtn);

    serverLayout->addLayout(serverStatusLayout);
    layout->addWidget(serverGroup);

    // Client mode group
    QGroupBox* clientGroup = new QGroupBox("Client Mode");
    QVBoxLayout* clientLayout = new QVBoxLayout(clientGroup);

    QLabel* serversLabel = new QLabel("Discovered Servers:");
    clientLayout->addWidget(serversLabel);

    m_serverList = new QListWidget();
    connect(m_serverList, &QListWidget::itemSelectionChanged, this, &MainWindow::onServerSelectionChanged);
    connect(m_serverList, &QListWidget::itemDoubleClicked, this, &MainWindow::onConnectClicked);
    clientLayout->addWidget(m_serverList);

    QFormLayout* connectForm = new QFormLayout();

    m_serverAddressEdit = new QLineEdit();
    m_serverAddressEdit->setPlaceholderText("Server IP or hostname");
    connectForm->addRow("Address:", m_serverAddressEdit);

    m_serverPortSpin = new QSpinBox();
    m_serverPortSpin->setRange(1, 65535);
    m_serverPortSpin->setValue(Settings::instance()->serverPort());
    connectForm->addRow("Port:", m_serverPortSpin);

    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Server password");
    connectForm->addRow("Password:", m_passwordEdit);

    clientLayout->addLayout(connectForm);

    QHBoxLayout* clientBtnLayout = new QHBoxLayout();

    m_clientStatusLabel = new QLabel("Not connected");
    clientBtnLayout->addWidget(m_clientStatusLabel);
    clientBtnLayout->addStretch();

    m_connectBtn = new QPushButton("Connect");
    connect(m_connectBtn, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    clientBtnLayout->addWidget(m_connectBtn);

    m_disconnectBtn = new QPushButton("Disconnect");
    m_disconnectBtn->setEnabled(false);
    connect(m_disconnectBtn, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);
    clientBtnLayout->addWidget(m_disconnectBtn);

    clientLayout->addLayout(clientBtnLayout);
    layout->addWidget(clientGroup);

    m_tabWidget->addTab(m_connectionsTab, "Connections");
}

void MainWindow::setupTerminalTab()
{
    m_terminalTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_terminalTab);

    // Target selection
    QHBoxLayout* targetLayout = new QHBoxLayout();
    targetLayout->addWidget(new QLabel("Target:"));

    m_targetClientCombo = new QComboBox();
    m_targetClientCombo->addItem("All Clients", "all");
    m_targetClientCombo->addItem("Local", "local");
    targetLayout->addWidget(m_targetClientCombo);
    targetLayout->addStretch();

    layout->addLayout(targetLayout);

    // Output
    m_terminalOutput = new QTextEdit();
    m_terminalOutput->setReadOnly(true);
    m_terminalOutput->setStyleSheet("font-family: Consolas, monospace; background-color: #1e1e1e; color: #d4d4d4;");
    layout->addWidget(m_terminalOutput);

    // Input
    QHBoxLayout* inputLayout = new QHBoxLayout();

    m_terminalInput = new QLineEdit();
    m_terminalInput->setPlaceholderText("Enter command...");
    connect(m_terminalInput, &QLineEdit::returnPressed, this, &MainWindow::onSendTerminalCommand);
    inputLayout->addWidget(m_terminalInput);

    m_sendCommandBtn = new QPushButton("Send");
    connect(m_sendCommandBtn, &QPushButton::clicked, this, &MainWindow::onSendTerminalCommand);
    inputLayout->addWidget(m_sendCommandBtn);

    layout->addLayout(inputLayout);

    m_tabWidget->addTab(m_terminalTab, "Terminal");
}

void MainWindow::setupToolbar()
{
    QToolBar* toolbar = addToolBar("Main");
    toolbar->setMovable(false);

    QAction* settingsAction = toolbar->addAction("Settings");
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettingsClicked);

    toolbar->addSeparator();

    QAction* aboutAction = toolbar->addAction("About");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "About KeyCast",
            "KeyCast v1.0.0\n\n"
            "Cross-platform input broadcast and remote control utility.\n\n"
            "Features:\n"
            "- Broadcast keyboard/mouse to connected clients\n"
            "- Execute programs via keyboard shortcuts\n"
            "- Send terminal commands to remote machines");
    });
}

void MainWindow::loadSettings()
{
    Settings* settings = Settings::instance();

    if (!settings->windowGeometry().isEmpty()) {
        restoreGeometry(settings->windowGeometry());
    }
    if (!settings->windowState().isEmpty()) {
        restoreState(settings->windowState());
    }

    refreshServerList();
}

void MainWindow::saveSettings()
{
    Settings* settings = Settings::instance();
    settings->setWindowGeometry(saveGeometry());
    settings->setWindowState(saveState());
    settings->sync();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (Settings::instance()->closeToTray()) {
        hide();
        event->ignore();
    } else {
        saveSettings();
        event->accept();
        QApplication::quit();
    }
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized() && Settings::instance()->minimizeToTray()) {
            hide();
            event->ignore();
            return;
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::updateServerStatus(bool running)
{
    if (running) {
        m_serverStatusLabel->setText("Server: Running on port " +
            QString::number(Settings::instance()->serverPort()));
        m_serverStatusLabel->setStyleSheet("color: green;");
        m_startServerBtn->setEnabled(false);
        m_stopServerBtn->setEnabled(true);
    } else {
        m_serverStatusLabel->setText("Server: Stopped");
        m_serverStatusLabel->setStyleSheet("color: red;");
        m_startServerBtn->setEnabled(true);
        m_stopServerBtn->setEnabled(false);
    }
}

void MainWindow::updateClientStatus(bool connected)
{
    if (connected) {
        m_clientStatusLabel->setText("Connected to server");
        m_clientStatusLabel->setStyleSheet("color: green;");
        m_connectBtn->setEnabled(false);
        m_disconnectBtn->setEnabled(true);
    } else {
        m_clientStatusLabel->setText("Not connected");
        m_clientStatusLabel->setStyleSheet("color: red;");
        m_connectBtn->setEnabled(true);
        m_disconnectBtn->setEnabled(false);
    }
}

void MainWindow::updateBroadcastStatus(bool active)
{
    if (active) {
        m_broadcastStatusLabel->setText("Broadcast: ACTIVE");
        m_broadcastStatusLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: green;");
        m_toggleBroadcastBtn->setText("Stop Broadcast");
        m_toggleBroadcastBtn->setStyleSheet("font-size: 14px; background-color: #ff6b6b;");
    } else {
        m_broadcastStatusLabel->setText("Broadcast: Inactive");
        m_broadcastStatusLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: gray;");
        m_toggleBroadcastBtn->setText("Start Broadcast");
        m_toggleBroadcastBtn->setStyleSheet("font-size: 14px;");
    }
}

void MainWindow::addDiscoveredServer(const QString& name, const QString& address, int port)
{
    QString itemText = QString("%1 (%2:%3)").arg(name, address).arg(port);

    // Check if already in list
    for (int i = 0; i < m_serverList->count(); ++i) {
        if (m_serverList->item(i)->data(Qt::UserRole).toString() == address) {
            m_serverList->item(i)->setText(itemText);
            return;
        }
    }

    QListWidgetItem* item = new QListWidgetItem(itemText);
    item->setData(Qt::UserRole, address);
    item->setData(Qt::UserRole + 1, port);
    m_serverList->addItem(item);
}

void MainWindow::addConnectedClient(const QString& clientId, const QString& clientName)
{
    QListWidgetItem* item = new QListWidgetItem(QString("%1 (%2)").arg(clientName, clientId));
    item->setData(Qt::UserRole, clientId);
    m_connectedClientsList->addItem(item);

    // Update target combo
    m_targetClientCombo->addItem(clientName, clientId);
}

void MainWindow::removeConnectedClient(const QString& clientId)
{
    for (int i = 0; i < m_connectedClientsList->count(); ++i) {
        if (m_connectedClientsList->item(i)->data(Qt::UserRole).toString() == clientId) {
            delete m_connectedClientsList->takeItem(i);
            break;
        }
    }

    // Update target combo
    for (int i = 0; i < m_targetClientCombo->count(); ++i) {
        if (m_targetClientCombo->itemData(i).toString() == clientId) {
            m_targetClientCombo->removeItem(i);
            break;
        }
    }
}

void MainWindow::appendTerminalOutput(const QString& text)
{
    m_terminalOutput->append(text);
}

void MainWindow::onStartServerClicked()
{
    keycastApp->startServer();
}

void MainWindow::onStopServerClicked()
{
    keycastApp->stopServer();
}

void MainWindow::onConnectClicked()
{
    QString address = m_serverAddressEdit->text().trimmed();
    int port = m_serverPortSpin->value();
    QString password = m_passwordEdit->text();

    if (address.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a server address");
        return;
    }

    keycastApp->connectToServer(address, port, password);
}

void MainWindow::onDisconnectClicked()
{
    keycastApp->disconnectFromServer();
}

void MainWindow::onToggleBroadcastClicked()
{
    keycastApp->toggleBroadcast();
}

void MainWindow::onAddShortcutClicked()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Add Shortcut", "Shortcut name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;

    QString hotkey = QInputDialog::getText(this, "Add Shortcut", "Hotkey (e.g., Ctrl+Shift+A):", QLineEdit::Normal, "", &ok);
    if (!ok || hotkey.isEmpty()) return;

    QStringList types = {"binary", "terminal"};
    QString type = QInputDialog::getItem(this, "Add Shortcut", "Action type:", types, 0, false, &ok);
    if (!ok) return;

    QString command = QInputDialog::getText(this, "Add Shortcut", "Command or path:", QLineEdit::Normal, "", &ok);
    if (!ok || command.isEmpty()) return;

    ShortcutAction action;
    action.name = name;
    action.keySequence = hotkey;
    action.actionType = type;
    action.command = command;
    action.targetClients = "local";

    Settings::instance()->addShortcut(action);
    keycastApp->shortcutManager()->loadShortcuts();
    refreshShortcutsList();
}

void MainWindow::onEditShortcutClicked()
{
    int row = m_shortcutsTable->currentRow();
    if (row < 0) return;

    QList<ShortcutAction> shortcuts = Settings::instance()->shortcuts();
    if (row >= shortcuts.size()) return;

    ShortcutAction& action = shortcuts[row];
    bool ok;

    QString name = QInputDialog::getText(this, "Edit Shortcut", "Shortcut name:", QLineEdit::Normal, action.name, &ok);
    if (!ok) return;
    action.name = name;

    QString hotkey = QInputDialog::getText(this, "Edit Shortcut", "Hotkey:", QLineEdit::Normal, action.keySequence, &ok);
    if (!ok) return;
    action.keySequence = hotkey;

    QStringList types = {"binary", "terminal"};
    int typeIndex = types.indexOf(action.actionType);
    QString type = QInputDialog::getItem(this, "Edit Shortcut", "Action type:", types, typeIndex, false, &ok);
    if (!ok) return;
    action.actionType = type;

    QString command = QInputDialog::getText(this, "Edit Shortcut", "Command or path:", QLineEdit::Normal, action.command, &ok);
    if (!ok) return;
    action.command = command;

    Settings::instance()->setShortcuts(shortcuts);
    keycastApp->shortcutManager()->loadShortcuts();
    refreshShortcutsList();
}

void MainWindow::onRemoveShortcutClicked()
{
    int row = m_shortcutsTable->currentRow();
    if (row < 0) return;

    if (QMessageBox::question(this, "Remove Shortcut",
        "Are you sure you want to remove this shortcut?") == QMessageBox::Yes) {
        Settings::instance()->removeShortcut(row);
        keycastApp->shortcutManager()->loadShortcuts();
        refreshShortcutsList();
    }
}

void MainWindow::onSendTerminalCommand()
{
    QString command = m_terminalInput->text().trimmed();
    if (command.isEmpty()) return;

    QString target = m_targetClientCombo->currentData().toString();

    appendTerminalOutput("> " + command);
    m_terminalInput->clear();

    if (target == "local") {
        keycastApp->shortcutManager()->executeCommand(command, "terminal");
    } else if (target == "all") {
        keycastApp->server()->broadcastCommand(command, "terminal");
    } else {
        keycastApp->server()->sendCommandToClient(target, command, "terminal");
    }
}

void MainWindow::onSettingsClicked()
{
    SettingsDialog dialog(this);
    dialog.exec();
}

void MainWindow::onServerSelectionChanged()
{
    QListWidgetItem* item = m_serverList->currentItem();
    if (item) {
        m_serverAddressEdit->setText(item->data(Qt::UserRole).toString());
        m_serverPortSpin->setValue(item->data(Qt::UserRole + 1).toInt());
    }
}

void MainWindow::refreshShortcutsList()
{
    m_shortcutsTable->setRowCount(0);

    QList<ShortcutAction> shortcuts = Settings::instance()->shortcuts();
    for (const ShortcutAction& action : shortcuts) {
        int row = m_shortcutsTable->rowCount();
        m_shortcutsTable->insertRow(row);

        m_shortcutsTable->setItem(row, 0, new QTableWidgetItem(action.name));
        m_shortcutsTable->setItem(row, 1, new QTableWidgetItem(action.keySequence));
        m_shortcutsTable->setItem(row, 2, new QTableWidgetItem(action.actionType));
        m_shortcutsTable->setItem(row, 3, new QTableWidgetItem(action.command));
    }
}

void MainWindow::refreshServerList()
{
    m_serverList->clear();

    QList<ServerInfo> servers = Settings::instance()->savedServers();
    for (const ServerInfo& server : servers) {
        addDiscoveredServer(server.name, server.address, server.port);
    }
}

void MainWindow::setupRemoteDesktopTab()
{
    m_remoteDesktopTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_remoteDesktopTab);

    // Status group
    QGroupBox* statusGroup = new QGroupBox("Screen Sharing Status");
    QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);

    m_screenShareStatusLabel = new QLabel("Screen Sharing: Inactive");
    m_screenShareStatusLabel->setStyleSheet("font-size: 14px;");
    statusLayout->addWidget(m_screenShareStatusLabel);

    m_toggleScreenShareBtn = new QPushButton("Start Screen Share (Server)");
    m_toggleScreenShareBtn->setMinimumHeight(40);
    connect(m_toggleScreenShareBtn, &QPushButton::clicked, this, &MainWindow::onToggleScreenShareClicked);
    statusLayout->addWidget(m_toggleScreenShareBtn);

    layout->addWidget(statusGroup);

    // Remote Desktop Viewer group
    QGroupBox* viewerGroup = new QGroupBox("Remote Desktop Viewer");
    QVBoxLayout* viewerLayout = new QVBoxLayout(viewerGroup);

    QLabel* infoLabel = new QLabel(
        "Connect to a server and open the Remote Desktop viewer to see and control the remote screen.\n"
        "Make sure you are connected to a server in the Connections tab first."
    );
    infoLabel->setWordWrap(true);
    viewerLayout->addWidget(infoLabel);

    m_openRemoteDesktopBtn = new QPushButton("Open Remote Desktop Viewer");
    m_openRemoteDesktopBtn->setMinimumHeight(50);
    m_openRemoteDesktopBtn->setStyleSheet("font-size: 14px;");
    connect(m_openRemoteDesktopBtn, &QPushButton::clicked, this, &MainWindow::onOpenRemoteDesktopClicked);
    viewerLayout->addWidget(m_openRemoteDesktopBtn);

    layout->addWidget(viewerGroup);

    // Instructions
    QGroupBox* helpGroup = new QGroupBox("Instructions");
    QVBoxLayout* helpLayout = new QVBoxLayout(helpGroup);

    QLabel* helpLabel = new QLabel(
        "<b>As Server (share your screen):</b><br>"
        "1. Start the server in the Connections tab<br>"
        "2. Click 'Start Screen Share' above<br>"
        "3. Connected clients can view your screen<br><br>"
        "<b>As Client (view remote screen):</b><br>"
        "1. Connect to a server in the Connections tab<br>"
        "2. Click 'Open Remote Desktop Viewer'<br>"
        "3. The server's screen will appear in the viewer<br>"
        "4. Mouse and keyboard input will be sent to the server"
    );
    helpLabel->setWordWrap(true);
    helpLayout->addWidget(helpLabel);

    layout->addWidget(helpGroup);
    layout->addStretch();

    m_tabWidget->addTab(m_remoteDesktopTab, "Remote Desktop");
}

void MainWindow::onOpenRemoteDesktopClicked()
{
    Client* client = keycastApp->client();

    if (!client->isConnected()) {
        QMessageBox::warning(this, "Not Connected",
            "Please connect to a server first in the Connections tab.");
        return;
    }

    if (!m_remoteDesktopWindow) {
        m_remoteDesktopWindow = new RemoteDesktopWindow(client, this);
    }

    m_remoteDesktopWindow->show();
    m_remoteDesktopWindow->raise();
    m_remoteDesktopWindow->activateWindow();
    m_remoteDesktopWindow->startViewing();
}

void MainWindow::onToggleScreenShareClicked()
{
    Server* server = keycastApp->server();

    if (!server->isListening()) {
        QMessageBox::warning(this, "Server Not Running",
            "Please start the server first in the Connections tab.");
        return;
    }

    if (server->isScreenSharing()) {
        server->stopScreenShare();
        m_toggleScreenShareBtn->setText("Start Screen Share (Server)");
        m_screenShareStatusLabel->setText("Screen Sharing: Inactive");
        m_screenShareStatusLabel->setStyleSheet("font-size: 14px; color: gray;");
    } else {
        server->startScreenShare();
        m_toggleScreenShareBtn->setText("Stop Screen Share (Server)");
        m_screenShareStatusLabel->setText("Screen Sharing: ACTIVE");
        m_screenShareStatusLabel->setStyleSheet("font-size: 14px; color: green; font-weight: bold;");
    }
}
