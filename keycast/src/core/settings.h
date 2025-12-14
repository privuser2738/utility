#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QVariant>

struct ShortcutAction {
    QString name;
    QString keySequence;
    QString actionType;  // "binary" or "terminal"
    QString command;
    QString targetClients;  // comma-separated client IDs or "all"
};

struct ServerInfo {
    QString name;
    QString address;
    int port;
    bool autoConnect;
};

class Settings : public QObject
{
    Q_OBJECT

public:
    static Settings* instance();

    // Window settings
    QByteArray windowGeometry() const;
    void setWindowGeometry(const QByteArray& geometry);
    QByteArray windowState() const;
    void setWindowState(const QByteArray& state);

    // Tray settings
    bool minimizeToTray() const;
    void setMinimizeToTray(bool enabled);
    bool closeToTray() const;
    void setCloseToTray(bool enabled);
    bool startMinimized() const;
    void setStartMinimized(bool enabled);
    bool showTrayNotifications() const;
    void setShowTrayNotifications(bool enabled);

    // Network settings
    int serverPort() const;
    void setServerPort(int port);
    int discoveryPort() const;
    void setDiscoveryPort(int port);
    QString serverPassword() const;
    void setServerPassword(const QString& password);
    bool autoStartServer() const;
    void setAutoStartServer(bool enabled);
    bool enableDiscovery() const;
    void setEnableDiscovery(bool enabled);

    // Mode settings
    bool serverModeEnabled() const;
    void setServerModeEnabled(bool enabled);
    bool clientModeEnabled() const;
    void setClientModeEnabled(bool enabled);

    // Shortcut settings
    QList<ShortcutAction> shortcuts() const;
    void setShortcuts(const QList<ShortcutAction>& shortcuts);
    void addShortcut(const ShortcutAction& shortcut);
    void removeShortcut(int index);

    // Server list
    QList<ServerInfo> savedServers() const;
    void setSavedServers(const QList<ServerInfo>& servers);
    void addServer(const ServerInfo& server);
    void removeServer(int index);

    // Broadcast settings
    bool broadcastKeyboard() const;
    void setBroadcastKeyboard(bool enabled);
    bool broadcastMouse() const;
    void setBroadcastMouse(bool enabled);
    QString broadcastToggleHotkey() const;
    void setBroadcastToggleHotkey(const QString& hotkey);

    // Computer name
    QString computerName() const;
    void setComputerName(const QString& name);

    void sync();

signals:
    void settingsChanged();

private:
    explicit Settings(QObject* parent = nullptr);
    ~Settings() = default;

    static Settings* m_instance;
    mutable QSettings m_settings;
};

#endif // SETTINGS_H
