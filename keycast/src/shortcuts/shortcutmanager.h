#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include <QObject>
#include <QMap>
#include <QKeySequence>
#include "settings.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class ActionExecutor;

class ShortcutManager : public QObject
{
    Q_OBJECT

public:
    explicit ShortcutManager(QObject* parent = nullptr);
    ~ShortcutManager();

    void loadShortcuts();
    void unloadShortcuts();

    bool registerShortcut(int id, const QString& keySequence);
    void unregisterShortcut(int id);
    void unregisterAllShortcuts();

public slots:
    void executeCommand(const QString& command, const QString& type);
    void executeShortcut(int id);

signals:
    void shortcutTriggered(int id, const QString& name);
    void commandExecuted(const QString& command, const QString& output);
    void commandError(const QString& command, const QString& error);

private:
#ifdef Q_OS_WIN
    bool nativeEventFilter(void* message);
    static LRESULT CALLBACK HotkeyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd = nullptr;
    static ShortcutManager* s_instance;
#endif

    struct RegisteredShortcut {
        int id;
        QString keySequence;
        ShortcutAction action;
    };

    QMap<int, RegisteredShortcut> m_shortcuts;
    ActionExecutor* m_executor;
    int m_nextId = 1;

    QPair<int, int> parseKeySequence(const QString& seq);
};

#endif // SHORTCUTMANAGER_H
