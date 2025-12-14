#ifndef SCREENCAPTURE_H
#define SCREENCAPTURE_H

#include <QObject>
#include <QImage>
#include <QTimer>
#include <QScreen>

class ScreenCapture : public QObject
{
    Q_OBJECT

public:
    explicit ScreenCapture(QObject* parent = nullptr);
    ~ScreenCapture();

    bool isCapturing() const { return m_capturing; }
    int frameRate() const { return m_frameRate; }
    void setFrameRate(int fps);

    int quality() const { return m_quality; }
    void setQuality(int quality);

    QSize captureSize() const { return m_captureSize; }
    void setCaptureSize(const QSize& size);

    int screenIndex() const { return m_screenIndex; }
    void setScreenIndex(int index);

    QRect captureRegion() const { return m_captureRegion; }
    void setCaptureRegion(const QRect& region);

public slots:
    void start();
    void stop();
    void captureFrame();

signals:
    void frameCaptured(const QImage& frame);
    void error(const QString& message);

private:
    QImage captureScreen();
    QImage scaleImage(const QImage& image);

    QTimer* m_captureTimer;
    bool m_capturing = false;

    int m_frameRate = 15;       // FPS
    int m_quality = 70;         // JPEG quality
    QSize m_captureSize;        // Output size (empty = original)
    int m_screenIndex = 0;      // Which screen to capture
    QRect m_captureRegion;      // Region to capture (empty = full screen)
};

#endif // SCREENCAPTURE_H
