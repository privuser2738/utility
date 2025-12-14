#ifndef INPUTINJECTOR_H
#define INPUTINJECTOR_H

#include <QObject>

#ifdef Q_OS_LINUX
// Forward declare X11 types
typedef struct _XDisplay Display;
#endif

class InputInjector : public QObject
{
    Q_OBJECT

public:
    explicit InputInjector(QObject* parent = nullptr);
#ifdef Q_OS_LINUX
    ~InputInjector();
#else
    ~InputInjector() = default;
#endif

public slots:
    void injectKeyEvent(int vkCode, bool pressed);
    void injectMouseEvent(int x, int y, int button, bool pressed);
    void injectMouseMove(int x, int y);

    // Convenience methods
    void pressKey(int vkCode);
    void releaseKey(int vkCode);
    void typeKey(int vkCode);
    void clickMouse(int button);
    void moveMouse(int x, int y);

private:
#ifdef Q_OS_WIN
    void sendKeyInput(int vkCode, bool keyUp);
    void sendMouseInput(int x, int y, unsigned long flags, unsigned long mouseData = 0);
#elif defined(Q_OS_LINUX)
    Display* m_display = nullptr;
#endif
};

#endif // INPUTINJECTOR_H
