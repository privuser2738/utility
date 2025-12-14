#include "screencapture.h"
#include <QGuiApplication>
#include <QScreen>
#include <QPixmap>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

ScreenCapture::ScreenCapture(QObject* parent)
    : QObject(parent)
    , m_captureTimer(new QTimer(this))
{
    connect(m_captureTimer, &QTimer::timeout, this, &ScreenCapture::captureFrame);
}

ScreenCapture::~ScreenCapture()
{
    stop();
}

void ScreenCapture::setFrameRate(int fps)
{
    m_frameRate = qBound(1, fps, 60);
    if (m_capturing) {
        m_captureTimer->setInterval(1000 / m_frameRate);
    }
}

void ScreenCapture::setQuality(int quality)
{
    m_quality = qBound(10, quality, 100);
}

void ScreenCapture::setCaptureSize(const QSize& size)
{
    m_captureSize = size;
}

void ScreenCapture::setScreenIndex(int index)
{
    QList<QScreen*> screens = QGuiApplication::screens();
    if (index >= 0 && index < screens.size()) {
        m_screenIndex = index;
    }
}

void ScreenCapture::setCaptureRegion(const QRect& region)
{
    m_captureRegion = region;
}

void ScreenCapture::start()
{
    if (m_capturing) return;

    m_capturing = true;
    m_captureTimer->start(1000 / m_frameRate);
}

void ScreenCapture::stop()
{
    if (!m_capturing) return;

    m_capturing = false;
    m_captureTimer->stop();
}

void ScreenCapture::captureFrame()
{
    QImage frame = captureScreen();
    if (!frame.isNull()) {
        if (!m_captureSize.isEmpty()) {
            frame = scaleImage(frame);
        }
        emit frameCaptured(frame);
    }
}

QImage ScreenCapture::captureScreen()
{
    QList<QScreen*> screens = QGuiApplication::screens();
    if (screens.isEmpty()) {
        emit error("No screens available");
        return QImage();
    }

    QScreen* screen = nullptr;
    if (m_screenIndex >= 0 && m_screenIndex < screens.size()) {
        screen = screens[m_screenIndex];
    } else {
        screen = QGuiApplication::primaryScreen();
    }

    if (!screen) {
        emit error("Failed to get screen");
        return QImage();
    }

#ifdef Q_OS_WIN
    // Use Windows API for faster capture
    HDC hdcScreen = GetDC(nullptr);
    if (!hdcScreen) {
        emit error("Failed to get screen DC");
        return QImage();
    }

    int width, height, x, y;
    if (m_captureRegion.isValid()) {
        x = m_captureRegion.x();
        y = m_captureRegion.y();
        width = m_captureRegion.width();
        height = m_captureRegion.height();
    } else {
        x = 0;
        y = 0;
        width = GetSystemMetrics(SM_CXSCREEN);
        height = GetSystemMetrics(SM_CYSCREEN);
    }

    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    HGDIOBJ hOld = SelectObject(hdcMem, hBitmap);

    BitBlt(hdcMem, 0, 0, width, height, hdcScreen, x, y, SRCCOPY);

    SelectObject(hdcMem, hOld);

    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height; // Negative for top-down
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    QImage image(width, height, QImage::Format_RGB32);
    GetDIBits(hdcMem, hBitmap, 0, height, image.bits(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);

    return image;
#else
    // Use Qt for other platforms
    QRect grabRect = m_captureRegion.isValid() ? m_captureRegion : screen->geometry();
    QPixmap pixmap = screen->grabWindow(0, grabRect.x(), grabRect.y(),
                                         grabRect.width(), grabRect.height());
    return pixmap.toImage();
#endif
}

QImage ScreenCapture::scaleImage(const QImage& image)
{
    if (image.isNull() || m_captureSize.isEmpty()) return image;

    return image.scaled(m_captureSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
