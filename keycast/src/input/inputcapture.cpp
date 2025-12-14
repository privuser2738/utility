#include "inputcapture.h"
#include <QCoreApplication>

InputCapture::InputCapture(QObject* parent)
    : QObject(parent)
    , m_workerThread(new QThread(this))
    , m_worker(new InputCaptureWorker())
{
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &InputCaptureWorker::keyEvent, this, &InputCapture::keyEvent);
    connect(m_worker, &InputCaptureWorker::mouseEvent, this, &InputCapture::mouseEvent);
    connect(m_worker, &InputCaptureWorker::mouseMove, this, &InputCapture::mouseMove);

    m_workerThread->start();
}

InputCapture::~InputCapture()
{
    stop();
    m_workerThread->quit();
    m_workerThread->wait();
}

void InputCapture::start()
{
    if (m_capturing) return;
    m_capturing = true;
    QMetaObject::invokeMethod(m_worker, "startCapture", Qt::QueuedConnection);
}

void InputCapture::stop()
{
    if (!m_capturing) return;
    m_capturing = false;
    QMetaObject::invokeMethod(m_worker, "stopCapture", Qt::QueuedConnection);
}

#ifdef Q_OS_WIN

InputCaptureWorker* InputCaptureWorker::s_instance = nullptr;

InputCaptureWorker::InputCaptureWorker(QObject* parent)
    : QObject(parent)
{
    s_instance = this;
}

InputCaptureWorker::~InputCaptureWorker()
{
    stopCapture();
    s_instance = nullptr;
}

void InputCaptureWorker::startCapture()
{
    if (m_running) return;

    // Install low-level keyboard hook
    m_keyboardHook = SetWindowsHookEx(
        WH_KEYBOARD_LL,
        LowLevelKeyboardProc,
        GetModuleHandle(nullptr),
        0
    );

    // Install low-level mouse hook
    m_mouseHook = SetWindowsHookEx(
        WH_MOUSE_LL,
        LowLevelMouseProc,
        GetModuleHandle(nullptr),
        0
    );

    if (m_keyboardHook && m_mouseHook) {
        m_running = true;
        emit captureStarted();

        // Process messages to keep hooks active
        processMessages();
    }
}

void InputCaptureWorker::stopCapture()
{
    m_running = false;

    if (m_keyboardHook) {
        UnhookWindowsHookEx(m_keyboardHook);
        m_keyboardHook = nullptr;
    }

    if (m_mouseHook) {
        UnhookWindowsHookEx(m_mouseHook);
        m_mouseHook = nullptr;
    }

    emit captureStopped();
}

void InputCaptureWorker::processMessages()
{
    MSG msg;
    while (m_running) {
        // Process Windows messages (required for hooks to work)
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Process Qt events
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);

        // Small sleep to prevent high CPU usage
        Sleep(1);
    }
}

LRESULT CALLBACK InputCaptureWorker::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && s_instance) {
        KBDLLHOOKSTRUCT* kbd = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

        bool pressed = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        bool released = (wParam == WM_KEYUP || wParam == WM_SYSKEYUP);

        if (pressed || released) {
            // Emit signal (thread-safe via queued connection)
            QMetaObject::invokeMethod(s_instance, "keyEvent", Qt::QueuedConnection,
                Q_ARG(int, kbd->vkCode),
                Q_ARG(bool, pressed));
        }
    }

    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT CALLBACK InputCaptureWorker::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && s_instance) {
        MSLLHOOKSTRUCT* mouse = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);

        int x = mouse->pt.x;
        int y = mouse->pt.y;

        switch (wParam) {
        case WM_LBUTTONDOWN:
            QMetaObject::invokeMethod(s_instance, "mouseEvent", Qt::QueuedConnection,
                Q_ARG(int, x), Q_ARG(int, y), Q_ARG(int, 1), Q_ARG(bool, true));
            break;
        case WM_LBUTTONUP:
            QMetaObject::invokeMethod(s_instance, "mouseEvent", Qt::QueuedConnection,
                Q_ARG(int, x), Q_ARG(int, y), Q_ARG(int, 1), Q_ARG(bool, false));
            break;
        case WM_RBUTTONDOWN:
            QMetaObject::invokeMethod(s_instance, "mouseEvent", Qt::QueuedConnection,
                Q_ARG(int, x), Q_ARG(int, y), Q_ARG(int, 2), Q_ARG(bool, true));
            break;
        case WM_RBUTTONUP:
            QMetaObject::invokeMethod(s_instance, "mouseEvent", Qt::QueuedConnection,
                Q_ARG(int, x), Q_ARG(int, y), Q_ARG(int, 2), Q_ARG(bool, false));
            break;
        case WM_MBUTTONDOWN:
            QMetaObject::invokeMethod(s_instance, "mouseEvent", Qt::QueuedConnection,
                Q_ARG(int, x), Q_ARG(int, y), Q_ARG(int, 3), Q_ARG(bool, true));
            break;
        case WM_MBUTTONUP:
            QMetaObject::invokeMethod(s_instance, "mouseEvent", Qt::QueuedConnection,
                Q_ARG(int, x), Q_ARG(int, y), Q_ARG(int, 3), Q_ARG(bool, false));
            break;
        case WM_MOUSEMOVE:
            QMetaObject::invokeMethod(s_instance, "mouseMove", Qt::QueuedConnection,
                Q_ARG(int, x), Q_ARG(int, y));
            break;
        }
    }

    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

#endif // Q_OS_WIN

#ifdef Q_OS_LINUX

#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <X11/keysym.h>

InputCaptureWorker* InputCaptureWorker::s_instance = nullptr;

InputCaptureWorker::InputCaptureWorker(QObject* parent)
    : QObject(parent)
    , m_display(nullptr)
    , m_xiOpcode(0)
{
    s_instance = this;
}

InputCaptureWorker::~InputCaptureWorker()
{
    stopCapture();
    s_instance = nullptr;
}

void InputCaptureWorker::startCapture()
{
    if (m_running) return;

    // Open X11 display
    m_display = XOpenDisplay(nullptr);
    if (!m_display) {
        emit captureStarted(); // Emit anyway for consistency
        return;
    }

    // Check for XInput2 extension
    int event, error;
    if (!XQueryExtension(m_display, "XInputExtension", &m_xiOpcode, &event, &error)) {
        XCloseDisplay(m_display);
        m_display = nullptr;
        emit captureStarted();
        return;
    }

    // Check XInput2 version
    int major = 2, minor = 0;
    if (XIQueryVersion(m_display, &major, &minor) != Success) {
        XCloseDisplay(m_display);
        m_display = nullptr;
        emit captureStarted();
        return;
    }

    // Select input events on root window
    Window root = DefaultRootWindow(m_display);

    XIEventMask evmask;
    unsigned char mask[(XI_LASTEVENT + 7) / 8] = {0};

    evmask.deviceid = XIAllMasterDevices;
    evmask.mask_len = sizeof(mask);
    evmask.mask = mask;

    XISetMask(mask, XI_RawKeyPress);
    XISetMask(mask, XI_RawKeyRelease);
    XISetMask(mask, XI_RawButtonPress);
    XISetMask(mask, XI_RawButtonRelease);
    XISetMask(mask, XI_RawMotion);

    XISelectEvents(m_display, root, &evmask, 1);
    XSync(m_display, False);

    m_running = true;
    emit captureStarted();

    // Process X11 events
    processMessages();
}

void InputCaptureWorker::stopCapture()
{
    m_running = false;

    if (m_display) {
        XCloseDisplay(m_display);
        m_display = nullptr;
    }

    emit captureStopped();
}

void InputCaptureWorker::processMessages()
{
    while (m_running && m_display) {
        while (XPending(m_display)) {
            XEvent event;
            XNextEvent(m_display, &event);

            if (event.xcookie.type == GenericEvent &&
                event.xcookie.extension == m_xiOpcode) {

                if (XGetEventData(m_display, &event.xcookie)) {
                    XIRawEvent* rawEvent = static_cast<XIRawEvent*>(event.xcookie.data);

                    switch (event.xcookie.evtype) {
                    case XI_RawKeyPress:
                        emit keyEvent(rawEvent->detail, true);
                        break;
                    case XI_RawKeyRelease:
                        emit keyEvent(rawEvent->detail, false);
                        break;
                    case XI_RawButtonPress:
                        if (rawEvent->detail >= 1 && rawEvent->detail <= 3) {
                            // Get cursor position
                            Window root, child;
                            int rootX, rootY, winX, winY;
                            unsigned int mask;
                            XQueryPointer(m_display, DefaultRootWindow(m_display),
                                          &root, &child, &rootX, &rootY, &winX, &winY, &mask);
                            emit mouseEvent(rootX, rootY, rawEvent->detail, true);
                        }
                        break;
                    case XI_RawButtonRelease:
                        if (rawEvent->detail >= 1 && rawEvent->detail <= 3) {
                            Window root, child;
                            int rootX, rootY, winX, winY;
                            unsigned int mask;
                            XQueryPointer(m_display, DefaultRootWindow(m_display),
                                          &root, &child, &rootX, &rootY, &winX, &winY, &mask);
                            emit mouseEvent(rootX, rootY, rawEvent->detail, false);
                        }
                        break;
                    case XI_RawMotion: {
                        Window root, child;
                        int rootX, rootY, winX, winY;
                        unsigned int mask;
                        XQueryPointer(m_display, DefaultRootWindow(m_display),
                                      &root, &child, &rootX, &rootY, &winX, &winY, &mask);
                        emit mouseMove(rootX, rootY);
                        break;
                    }
                    }

                    XFreeEventData(m_display, &event.xcookie);
                }
            }
        }

        // Process Qt events
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);

        // Small sleep to prevent high CPU usage
        usleep(1000);
    }
}

#endif // Q_OS_LINUX
