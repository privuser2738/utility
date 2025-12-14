#include "settingsdialog.h"
#include "settings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Settings");
    setMinimumSize(450, 400);

    setupUi();
    loadSettings();
}

void SettingsDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    m_tabWidget = new QTabWidget();

    // === General Tab ===
    QWidget* generalTab = new QWidget();
    QVBoxLayout* generalLayout = new QVBoxLayout(generalTab);

    QGroupBox* behaviorGroup = new QGroupBox("Application Behavior");
    QVBoxLayout* behaviorLayout = new QVBoxLayout(behaviorGroup);

    m_startMinimizedCheck = new QCheckBox("Start minimized to system tray");
    behaviorLayout->addWidget(m_startMinimizedCheck);

    m_minimizeToTrayCheck = new QCheckBox("Minimize to system tray instead of taskbar");
    behaviorLayout->addWidget(m_minimizeToTrayCheck);

    m_closeToTrayCheck = new QCheckBox("Close to system tray (keep running)");
    behaviorLayout->addWidget(m_closeToTrayCheck);

    m_showNotificationsCheck = new QCheckBox("Show tray notifications");
    behaviorLayout->addWidget(m_showNotificationsCheck);

    generalLayout->addWidget(behaviorGroup);

    QGroupBox* identityGroup = new QGroupBox("Identity");
    QFormLayout* identityLayout = new QFormLayout(identityGroup);

    m_computerNameEdit = new QLineEdit();
    m_computerNameEdit->setPlaceholderText("Computer name shown to other clients");
    identityLayout->addRow("Computer Name:", m_computerNameEdit);

    generalLayout->addWidget(identityGroup);
    generalLayout->addStretch();

    m_tabWidget->addTab(generalTab, "General");

    // === Network Tab ===
    QWidget* networkTab = new QWidget();
    QVBoxLayout* networkLayout = new QVBoxLayout(networkTab);

    QGroupBox* portsGroup = new QGroupBox("Ports");
    QFormLayout* portsLayout = new QFormLayout(portsGroup);

    m_serverPortSpin = new QSpinBox();
    m_serverPortSpin->setRange(1024, 65535);
    portsLayout->addRow("Server Port (TCP):", m_serverPortSpin);

    m_discoveryPortSpin = new QSpinBox();
    m_discoveryPortSpin->setRange(1024, 65535);
    portsLayout->addRow("Discovery Port (UDP):", m_discoveryPortSpin);

    networkLayout->addWidget(portsGroup);

    QGroupBox* securityGroup = new QGroupBox("Security");
    QFormLayout* securityLayout = new QFormLayout(securityGroup);

    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Required for clients to connect");
    securityLayout->addRow("Server Password:", m_passwordEdit);

    networkLayout->addWidget(securityGroup);

    QGroupBox* autoGroup = new QGroupBox("Automatic");
    QVBoxLayout* autoLayout = new QVBoxLayout(autoGroup);

    m_autoStartServerCheck = new QCheckBox("Auto-start server on application launch");
    autoLayout->addWidget(m_autoStartServerCheck);

    m_enableDiscoveryCheck = new QCheckBox("Enable network discovery (UDP broadcast)");
    autoLayout->addWidget(m_enableDiscoveryCheck);

    networkLayout->addWidget(autoGroup);
    networkLayout->addStretch();

    m_tabWidget->addTab(networkTab, "Network");

    // === Mode Tab ===
    QWidget* modeTab = new QWidget();
    QVBoxLayout* modeLayout = new QVBoxLayout(modeTab);

    QGroupBox* modeGroup = new QGroupBox("Operating Modes");
    QVBoxLayout* modeGroupLayout = new QVBoxLayout(modeGroup);

    m_serverModeCheck = new QCheckBox("Enable Server Mode");
    modeGroupLayout->addWidget(m_serverModeCheck);

    QLabel* serverDesc = new QLabel("Server mode allows this computer to broadcast input\n"
                                     "and commands to connected clients.");
    serverDesc->setStyleSheet("color: gray; margin-left: 20px;");
    modeGroupLayout->addWidget(serverDesc);

    m_clientModeCheck = new QCheckBox("Enable Client Mode");
    modeGroupLayout->addWidget(m_clientModeCheck);

    QLabel* clientDesc = new QLabel("Client mode allows this computer to receive input\n"
                                     "and commands from a connected server.");
    clientDesc->setStyleSheet("color: gray; margin-left: 20px;");
    modeGroupLayout->addWidget(clientDesc);

    modeLayout->addWidget(modeGroup);
    modeLayout->addStretch();

    m_tabWidget->addTab(modeTab, "Mode");

    // === Broadcast Tab ===
    QWidget* broadcastTab = new QWidget();
    QVBoxLayout* broadcastLayout = new QVBoxLayout(broadcastTab);

    QGroupBox* hotkeyGroup = new QGroupBox("Hotkeys");
    QFormLayout* hotkeyLayout = new QFormLayout(hotkeyGroup);

    m_broadcastHotkeyEdit = new QLineEdit();
    m_broadcastHotkeyEdit->setPlaceholderText("e.g., Ctrl+Shift+B");
    hotkeyLayout->addRow("Toggle Broadcast:", m_broadcastHotkeyEdit);

    broadcastLayout->addWidget(hotkeyGroup);
    broadcastLayout->addStretch();

    m_tabWidget->addTab(broadcastTab, "Broadcast");

    mainLayout->addWidget(m_tabWidget);

    // Button box
    QDialogButtonBox* buttonBox = new QDialogButtonBox();
    QPushButton* applyBtn = buttonBox->addButton(QDialogButtonBox::Apply);
    buttonBox->addButton(QDialogButtonBox::Ok);
    buttonBox->addButton(QDialogButtonBox::Cancel);

    connect(applyBtn, &QPushButton::clicked, this, &SettingsDialog::onApply);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox);
}

void SettingsDialog::loadSettings()
{
    Settings* settings = Settings::instance();

    // General
    m_startMinimizedCheck->setChecked(settings->startMinimized());
    m_minimizeToTrayCheck->setChecked(settings->minimizeToTray());
    m_closeToTrayCheck->setChecked(settings->closeToTray());
    m_showNotificationsCheck->setChecked(settings->showTrayNotifications());
    m_computerNameEdit->setText(settings->computerName());

    // Network
    m_serverPortSpin->setValue(settings->serverPort());
    m_discoveryPortSpin->setValue(settings->discoveryPort());
    m_passwordEdit->setText(settings->serverPassword());
    m_autoStartServerCheck->setChecked(settings->autoStartServer());
    m_enableDiscoveryCheck->setChecked(settings->enableDiscovery());

    // Mode
    m_serverModeCheck->setChecked(settings->serverModeEnabled());
    m_clientModeCheck->setChecked(settings->clientModeEnabled());

    // Broadcast
    m_broadcastHotkeyEdit->setText(settings->broadcastToggleHotkey());
}

void SettingsDialog::saveSettings()
{
    Settings* settings = Settings::instance();

    // General
    settings->setStartMinimized(m_startMinimizedCheck->isChecked());
    settings->setMinimizeToTray(m_minimizeToTrayCheck->isChecked());
    settings->setCloseToTray(m_closeToTrayCheck->isChecked());
    settings->setShowTrayNotifications(m_showNotificationsCheck->isChecked());
    settings->setComputerName(m_computerNameEdit->text());

    // Network
    settings->setServerPort(m_serverPortSpin->value());
    settings->setDiscoveryPort(m_discoveryPortSpin->value());
    settings->setServerPassword(m_passwordEdit->text());
    settings->setAutoStartServer(m_autoStartServerCheck->isChecked());
    settings->setEnableDiscovery(m_enableDiscoveryCheck->isChecked());

    // Mode
    settings->setServerModeEnabled(m_serverModeCheck->isChecked());
    settings->setClientModeEnabled(m_clientModeCheck->isChecked());

    // Broadcast
    settings->setBroadcastToggleHotkey(m_broadcastHotkeyEdit->text());

    settings->sync();
}

void SettingsDialog::onAccept()
{
    saveSettings();
    accept();
}

void SettingsDialog::onApply()
{
    saveSettings();
}
