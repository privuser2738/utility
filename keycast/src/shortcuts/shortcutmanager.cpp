#include "shortcutmanager.h"
#include "actionexecutor.h"
#include <QDebug>

#ifdef Q_OS_WIN
ShortcutManager* ShortcutManager::s_instance = nullptr;
#endif

ShortcutManager::ShortcutManager(QObject* parent)
    : QObject(parent)
    , m_executor(new ActionExecutor(this))
{
#ifdef Q_OS_WIN
    s_instance = this;

    // Create a message-only window to receive hotkey messages
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = HotkeyWndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"KeyCastHotkeyClass";

    RegisterClassEx(&wc);

    m_hwnd = CreateWindowEx(
        0, L"KeyCastHotkeyClass", L"KeyCast Hotkey Window",
        0, 0, 0, 0, 0, HWND_MESSAGE, nullptr,
        GetModuleHandle(nullptr), nullptr
    );
#endif

    connect(m_executor, &ActionExecutor::commandOutput, this, &ShortcutManager::commandExecuted);
    connect(m_executor, &ActionExecutor::commandError, this, &ShortcutManager::commandError);
}

ShortcutManager::~ShortcutManager()
{
    unregisterAllShortcuts();

#ifdef Q_OS_WIN
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
    s_instance = nullptr;
#endif
}

void ShortcutManager::loadShortcuts()
{
    unregisterAllShortcuts();

    QList<ShortcutAction> shortcuts = Settings::instance()->shortcuts();

    for (const ShortcutAction& action : shortcuts) {
        int id = m_nextId++;

        if (registerShortcut(id, action.keySequence)) {
            RegisteredShortcut reg;
            reg.id = id;
            reg.keySequence = action.keySequence;
            reg.action = action;
            m_shortcuts[id] = reg;
        }
    }
}

void ShortcutManager::unloadShortcuts()
{
    unregisterAllShortcuts();
}

bool ShortcutManager::registerShortcut(int id, const QString& keySequence)
{
#ifdef Q_OS_WIN
    auto [modifiers, vk] = parseKeySequence(keySequence);
    if (vk == 0) return false;

    return RegisterHotKey(m_hwnd, id, modifiers, vk);
#else
    Q_UNUSED(id)
    Q_UNUSED(keySequence)
    return false;
#endif
}

void ShortcutManager::unregisterShortcut(int id)
{
#ifdef Q_OS_WIN
    UnregisterHotKey(m_hwnd, id);
#else
    Q_UNUSED(id)
#endif
    m_shortcuts.remove(id);
}

void ShortcutManager::unregisterAllShortcuts()
{
#ifdef Q_OS_WIN
    for (auto it = m_shortcuts.begin(); it != m_shortcuts.end(); ++it) {
        UnregisterHotKey(m_hwnd, it.key());
    }
#endif
    m_shortcuts.clear();
}

void ShortcutManager::executeCommand(const QString& command, const QString& type)
{
    m_executor->execute(command, type);
}

void ShortcutManager::executeShortcut(int id)
{
    if (!m_shortcuts.contains(id)) return;

    const RegisteredShortcut& shortcut = m_shortcuts[id];
    emit shortcutTriggered(id, shortcut.action.name);

    executeCommand(shortcut.action.command, shortcut.action.actionType);
}

QPair<int, int> ShortcutManager::parseKeySequence(const QString& seq)
{
    int modifiers = 0;
    int vk = 0;

    QString s = seq.toUpper();
    QStringList parts = s.split('+', Qt::SkipEmptyParts);

    for (const QString& part : parts) {
        QString p = part.trimmed();

#ifdef Q_OS_WIN
        if (p == "CTRL" || p == "CONTROL") {
            modifiers |= MOD_CONTROL;
        } else if (p == "ALT") {
            modifiers |= MOD_ALT;
        } else if (p == "SHIFT") {
            modifiers |= MOD_SHIFT;
        } else if (p == "WIN" || p == "META") {
            modifiers |= MOD_WIN;
        } else if (p.length() == 1) {
            // Single character
            QChar c = p[0];
            if (c >= 'A' && c <= 'Z') {
                vk = c.unicode();
            } else if (c >= '0' && c <= '9') {
                vk = c.unicode();
            }
        } else if (p.startsWith("F") && p.length() <= 3) {
            // Function keys F1-F24
            bool ok;
            int num = p.mid(1).toInt(&ok);
            if (ok && num >= 1 && num <= 24) {
                vk = VK_F1 + num - 1;
            }
        } else if (p == "SPACE") {
            vk = VK_SPACE;
        } else if (p == "ENTER" || p == "RETURN") {
            vk = VK_RETURN;
        } else if (p == "TAB") {
            vk = VK_TAB;
        } else if (p == "ESCAPE" || p == "ESC") {
            vk = VK_ESCAPE;
        } else if (p == "BACKSPACE") {
            vk = VK_BACK;
        } else if (p == "DELETE" || p == "DEL") {
            vk = VK_DELETE;
        } else if (p == "INSERT" || p == "INS") {
            vk = VK_INSERT;
        } else if (p == "HOME") {
            vk = VK_HOME;
        } else if (p == "END") {
            vk = VK_END;
        } else if (p == "PAGEUP" || p == "PGUP") {
            vk = VK_PRIOR;
        } else if (p == "PAGEDOWN" || p == "PGDN") {
            vk = VK_NEXT;
        } else if (p == "UP") {
            vk = VK_UP;
        } else if (p == "DOWN") {
            vk = VK_DOWN;
        } else if (p == "LEFT") {
            vk = VK_LEFT;
        } else if (p == "RIGHT") {
            vk = VK_RIGHT;
        } else if (p == "NUMLOCK") {
            vk = VK_NUMLOCK;
        } else if (p == "CAPSLOCK") {
            vk = VK_CAPITAL;
        } else if (p == "SCROLLLOCK") {
            vk = VK_SCROLL;
        } else if (p == "PRINTSCREEN" || p == "PRTSC") {
            vk = VK_SNAPSHOT;
        } else if (p == "PAUSE") {
            vk = VK_PAUSE;
        }
#endif
    }

    return {modifiers, vk};
}

#ifdef Q_OS_WIN

LRESULT CALLBACK ShortcutManager::HotkeyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_HOTKEY && s_instance) {
        int id = static_cast<int>(wParam);
        QMetaObject::invokeMethod(s_instance, "executeShortcut", Qt::QueuedConnection, Q_ARG(int, id));
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

#endif
