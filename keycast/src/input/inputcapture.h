#ifndef INPUTCAPTURE_H
#define INPUTCAPTURE_H

#include <QObject>
#include <QThread>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class InputCaptureWorker;

class InputCapture : public QObject
{
    Q_OBJECT

public:
    explicit InputCapture(QObject* parent = nullptr);
    ~InputCapture();

    bool isCapturing() const { return m_capturing; }

public slots:
    void start();
    void stop();

signals:
    void keyEvent(int vkCode, bool pressed);
    void mouseEvent(int x, int y, int button, bool pressed);
    void mouseMove(int x, int y);

private:
    QThread* m_workerThread;
    InputCaptureWorker* m_worker;
    bool m_capturing = false;
};

#ifdef Q_OS_WIN

class InputCaptureWorker : public QObject
{
    Q_OBJECT

public:
    explicit InputCaptureWorker(QObject* parent = nullptr);
    ~InputCaptureWorker();

    static InputCaptureWorker* instance() { return s_instance; }

public slots:
    void startCapture();
    void stopCapture();

signals:
    void keyEvent(int vkCode, bool pressed);
    void mouseEvent(int x, int y, int button, bool pressed);
    void mouseMove(int x, int y);
    void captureStarted();
    void captureStopped();

private:
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

    void processMessages();

    HHOOK m_keyboardHook = nullptr;
    HHOOK m_mouseHook = nullptr;
    bool m_running = false;

    static InputCaptureWorker* s_instance;
};

#elif defined(Q_OS_LINUX)

// Forward declare X11 types
typedef struct _XDisplay Display;

class InputCaptureWorker : public QObject
{
    Q_OBJECT

public:
    explicit InputCaptureWorker(QObject* parent = nullptr);
    ~InputCaptureWorker();

    static InputCaptureWorker* instance() { return s_instance; }

public slots:
    void startCapture();
    void stopCapture();

signals:
    void keyEvent(int vkCode, bool pressed);
    void mouseEvent(int x, int y, int button, bool pressed);
    void mouseMove(int x, int y);
    void captureStarted();
    void captureStopped();

private:
    void processMessages();

    Display* m_display = nullptr;
    int m_xiOpcode = 0;
    bool m_running = false;

    static InputCaptureWorker* s_instance;
};

#else

// Stub for other platforms (macOS, etc.)
class InputCaptureWorker : public QObject
{
    Q_OBJECT

public:
    explicit InputCaptureWorker(QObject* parent = nullptr) : QObject(parent) {}
    ~InputCaptureWorker() = default;

    static InputCaptureWorker* instance() { return nullptr; }

public slots:
    void startCapture() { emit captureStarted(); }
    void stopCapture() { emit captureStopped(); }

signals:
    void keyEvent(int vkCode, bool pressed);
    void mouseEvent(int x, int y, int button, bool pressed);
    void mouseMove(int x, int y);
    void captureStarted();
    void captureStopped();
};

#endif

#endif // INPUTCAPTURE_H
