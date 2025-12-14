#include "inputinjector.h"

#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_LINUX)
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#endif

InputInjector::InputInjector(QObject* parent)
    : QObject(parent)
{
#ifdef Q_OS_LINUX
    m_display = XOpenDisplay(nullptr);
#endif
}

#ifdef Q_OS_LINUX
InputInjector::~InputInjector()
{
    if (m_display) {
        XCloseDisplay(m_display);
        m_display = nullptr;
    }
}
#endif

void InputInjector::injectKeyEvent(int vkCode, bool pressed)
{
#ifdef Q_OS_WIN
    sendKeyInput(vkCode, !pressed);
#elif defined(Q_OS_LINUX)
    if (!m_display) return;
    XTestFakeKeyEvent(m_display, vkCode, pressed ? True : False, CurrentTime);
    XFlush(m_display);
#else
    Q_UNUSED(vkCode)
    Q_UNUSED(pressed)
#endif
}

void InputInjector::injectMouseEvent(int x, int y, int button, bool pressed)
{
#ifdef Q_OS_WIN
    // Move mouse to position first
    injectMouseMove(x, y);

    unsigned long flags = 0;

    if (button == 1) { // Left button
        flags = pressed ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
    } else if (button == 2) { // Right button
        flags = pressed ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
    } else if (button == 3) { // Middle button
        flags = pressed ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
    }

    if (flags != 0) {
        sendMouseInput(x, y, flags);
    }
#elif defined(Q_OS_LINUX)
    if (!m_display) return;

    // Move mouse first
    injectMouseMove(x, y);

    // Send button event
    unsigned int xButton = button; // X11 uses 1=left, 2=middle, 3=right
    if (button == 2) xButton = 3; // Swap middle and right for X11
    if (button == 3) xButton = 2;

    XTestFakeButtonEvent(m_display, xButton, pressed ? True : False, CurrentTime);
    XFlush(m_display);
#else
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(button)
    Q_UNUSED(pressed)
#endif
}

void InputInjector::injectMouseMove(int x, int y)
{
#ifdef Q_OS_WIN
    // Convert to absolute coordinates
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Absolute coordinates use 0-65535 range
    int absX = (x * 65535) / screenWidth;
    int absY = (y * 65535) / screenHeight;

    sendMouseInput(absX, absY, MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE);
#elif defined(Q_OS_LINUX)
    if (!m_display) return;
    XTestFakeMotionEvent(m_display, -1, x, y, CurrentTime);
    XFlush(m_display);
#else
    Q_UNUSED(x)
    Q_UNUSED(y)
#endif
}

void InputInjector::pressKey(int vkCode)
{
    injectKeyEvent(vkCode, true);
}

void InputInjector::releaseKey(int vkCode)
{
    injectKeyEvent(vkCode, false);
}

void InputInjector::typeKey(int vkCode)
{
    pressKey(vkCode);
    releaseKey(vkCode);
}

void InputInjector::clickMouse(int button)
{
#ifdef Q_OS_WIN
    POINT pt;
    GetCursorPos(&pt);
    injectMouseEvent(pt.x, pt.y, button, true);
    injectMouseEvent(pt.x, pt.y, button, false);
#elif defined(Q_OS_LINUX)
    if (!m_display) return;

    // Get current mouse position
    Window root, child;
    int rootX, rootY, winX, winY;
    unsigned int mask;
    XQueryPointer(m_display, DefaultRootWindow(m_display),
                  &root, &child, &rootX, &rootY, &winX, &winY, &mask);

    injectMouseEvent(rootX, rootY, button, true);
    injectMouseEvent(rootX, rootY, button, false);
#else
    Q_UNUSED(button)
#endif
}

void InputInjector::moveMouse(int x, int y)
{
    injectMouseMove(x, y);
}

#ifdef Q_OS_WIN

void InputInjector::sendKeyInput(int vkCode, bool keyUp)
{
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = static_cast<WORD>(vkCode);
    input.ki.wScan = static_cast<WORD>(MapVirtualKey(vkCode, MAPVK_VK_TO_VSC));

    if (keyUp) {
        input.ki.dwFlags = KEYEVENTF_KEYUP;
    }

    // Handle extended keys
    switch (vkCode) {
    case VK_INSERT:
    case VK_DELETE:
    case VK_HOME:
    case VK_END:
    case VK_PRIOR: // Page Up
    case VK_NEXT:  // Page Down
    case VK_UP:
    case VK_DOWN:
    case VK_LEFT:
    case VK_RIGHT:
    case VK_NUMLOCK:
    case VK_RCONTROL:
    case VK_RMENU: // Right Alt
    case VK_LWIN:
    case VK_RWIN:
        input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
        break;
    }

    SendInput(1, &input, sizeof(INPUT));
}

void InputInjector::sendMouseInput(int x, int y, unsigned long flags, unsigned long mouseData)
{
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dx = x;
    input.mi.dy = y;
    input.mi.dwFlags = flags;
    input.mi.mouseData = mouseData;

    SendInput(1, &input, sizeof(INPUT));
}

#endif // Q_OS_WIN
