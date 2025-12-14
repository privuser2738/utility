#include "settings.h"
#include <QStandardPaths>
#include <QDir>

Settings* Settings::m_instance = nullptr;

Settings* Settings::instance()
{
    if (!m_instance) {
        m_instance = new Settings();
    }
    return m_instance;
}

Settings::Settings(QObject* parent)
    : QObject(parent)
    , m_settings(QSettings::IniFormat, QSettings::UserScope, "KeyCast", "KeyCast")
{
}

// Window settings
QByteArray Settings::windowGeometry() const
{
    return m_settings.value("window/geometry").toByteArray();
}

void Settings::setWindowGeometry(const QByteArray& geometry)
{
    m_settings.setValue("window/geometry", geometry);
}

QByteArray Settings::windowState() const
{
    return m_settings.value("window/state").toByteArray();
}

void Settings::setWindowState(const QByteArray& state)
{
    m_settings.setValue("window/state", state);
}

// Tray settings
bool Settings::minimizeToTray() const
{
    return m_settings.value("tray/minimizeToTray", true).toBool();
}

void Settings::setMinimizeToTray(bool enabled)
{
    m_settings.setValue("tray/minimizeToTray", enabled);
    emit settingsChanged();
}

bool Settings::closeToTray() const
{
    return m_settings.value("tray/closeToTray", true).toBool();
}

void Settings::setCloseToTray(bool enabled)
{
    m_settings.setValue("tray/closeToTray", enabled);
    emit settingsChanged();
}

bool Settings::startMinimized() const
{
    return m_settings.value("tray/startMinimized", false).toBool();
}

void Settings::setStartMinimized(bool enabled)
{
    m_settings.setValue("tray/startMinimized", enabled);
    emit settingsChanged();
}

bool Settings::showTrayNotifications() const
{
    return m_settings.value("tray/showNotifications", true).toBool();
}

void Settings::setShowTrayNotifications(bool enabled)
{
    m_settings.setValue("tray/showNotifications", enabled);
    emit settingsChanged();
}

// Network settings
int Settings::serverPort() const
{
    return m_settings.value("network/serverPort", 45679).toInt();
}

void Settings::setServerPort(int port)
{
    m_settings.setValue("network/serverPort", port);
    emit settingsChanged();
}

int Settings::discoveryPort() const
{
    return m_settings.value("network/discoveryPort", 45678).toInt();
}

void Settings::setDiscoveryPort(int port)
{
    m_settings.setValue("network/discoveryPort", port);
    emit settingsChanged();
}

QString Settings::serverPassword() const
{
    return m_settings.value("network/password", "").toString();
}

void Settings::setServerPassword(const QString& password)
{
    m_settings.setValue("network/password", password);
    emit settingsChanged();
}

bool Settings::autoStartServer() const
{
    return m_settings.value("network/autoStartServer", false).toBool();
}

void Settings::setAutoStartServer(bool enabled)
{
    m_settings.setValue("network/autoStartServer", enabled);
    emit settingsChanged();
}

bool Settings::enableDiscovery() const
{
    return m_settings.value("network/enableDiscovery", true).toBool();
}

void Settings::setEnableDiscovery(bool enabled)
{
    m_settings.setValue("network/enableDiscovery", enabled);
    emit settingsChanged();
}

// Mode settings
bool Settings::serverModeEnabled() const
{
    return m_settings.value("mode/serverEnabled", true).toBool();
}

void Settings::setServerModeEnabled(bool enabled)
{
    m_settings.setValue("mode/serverEnabled", enabled);
    emit settingsChanged();
}

bool Settings::clientModeEnabled() const
{
    return m_settings.value("mode/clientEnabled", true).toBool();
}

void Settings::setClientModeEnabled(bool enabled)
{
    m_settings.setValue("mode/clientEnabled", enabled);
    emit settingsChanged();
}

// Shortcut settings
QList<ShortcutAction> Settings::shortcuts() const
{
    QList<ShortcutAction> list;
    int size = m_settings.beginReadArray("shortcuts");
    for (int i = 0; i < size; ++i) {
        m_settings.setArrayIndex(i);
        ShortcutAction action;
        action.name = m_settings.value("name").toString();
        action.keySequence = m_settings.value("keySequence").toString();
        action.actionType = m_settings.value("actionType").toString();
        action.command = m_settings.value("command").toString();
        action.targetClients = m_settings.value("targetClients", "local").toString();
        list.append(action);
    }
    m_settings.endArray();
    return list;
}

void Settings::setShortcuts(const QList<ShortcutAction>& shortcuts)
{
    m_settings.beginWriteArray("shortcuts");
    for (int i = 0; i < shortcuts.size(); ++i) {
        m_settings.setArrayIndex(i);
        m_settings.setValue("name", shortcuts[i].name);
        m_settings.setValue("keySequence", shortcuts[i].keySequence);
        m_settings.setValue("actionType", shortcuts[i].actionType);
        m_settings.setValue("command", shortcuts[i].command);
        m_settings.setValue("targetClients", shortcuts[i].targetClients);
    }
    m_settings.endArray();
    emit settingsChanged();
}

void Settings::addShortcut(const ShortcutAction& shortcut)
{
    QList<ShortcutAction> list = shortcuts();
    list.append(shortcut);
    setShortcuts(list);
}

void Settings::removeShortcut(int index)
{
    QList<ShortcutAction> list = shortcuts();
    if (index >= 0 && index < list.size()) {
        list.removeAt(index);
        setShortcuts(list);
    }
}

// Server list
QList<ServerInfo> Settings::savedServers() const
{
    QList<ServerInfo> list;
    int size = m_settings.beginReadArray("servers");
    for (int i = 0; i < size; ++i) {
        m_settings.setArrayIndex(i);
        ServerInfo info;
        info.name = m_settings.value("name").toString();
        info.address = m_settings.value("address").toString();
        info.port = m_settings.value("port", 45679).toInt();
        info.autoConnect = m_settings.value("autoConnect", false).toBool();
        list.append(info);
    }
    m_settings.endArray();
    return list;
}

void Settings::setSavedServers(const QList<ServerInfo>& servers)
{
    m_settings.beginWriteArray("servers");
    for (int i = 0; i < servers.size(); ++i) {
        m_settings.setArrayIndex(i);
        m_settings.setValue("name", servers[i].name);
        m_settings.setValue("address", servers[i].address);
        m_settings.setValue("port", servers[i].port);
        m_settings.setValue("autoConnect", servers[i].autoConnect);
    }
    m_settings.endArray();
    emit settingsChanged();
}

void Settings::addServer(const ServerInfo& server)
{
    QList<ServerInfo> list = savedServers();
    list.append(server);
    setSavedServers(list);
}

void Settings::removeServer(int index)
{
    QList<ServerInfo> list = savedServers();
    if (index >= 0 && index < list.size()) {
        list.removeAt(index);
        setSavedServers(list);
    }
}

// Broadcast settings
bool Settings::broadcastKeyboard() const
{
    return m_settings.value("broadcast/keyboard", true).toBool();
}

void Settings::setBroadcastKeyboard(bool enabled)
{
    m_settings.setValue("broadcast/keyboard", enabled);
    emit settingsChanged();
}

bool Settings::broadcastMouse() const
{
    return m_settings.value("broadcast/mouse", false).toBool();
}

void Settings::setBroadcastMouse(bool enabled)
{
    m_settings.setValue("broadcast/mouse", enabled);
    emit settingsChanged();
}

QString Settings::broadcastToggleHotkey() const
{
    return m_settings.value("broadcast/toggleHotkey", "Ctrl+Shift+B").toString();
}

void Settings::setBroadcastToggleHotkey(const QString& hotkey)
{
    m_settings.setValue("broadcast/toggleHotkey", hotkey);
    emit settingsChanged();
}

// Computer name
QString Settings::computerName() const
{
    QString defaultName = qEnvironmentVariable("COMPUTERNAME");
    if (defaultName.isEmpty()) {
        defaultName = qEnvironmentVariable("HOSTNAME", "KeyCast-Client");
    }
    return m_settings.value("general/computerName", defaultName).toString();
}

void Settings::setComputerName(const QString& name)
{
    m_settings.setValue("general/computerName", name);
    emit settingsChanged();
}

void Settings::sync()
{
    m_settings.sync();
}
