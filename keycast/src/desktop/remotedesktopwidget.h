#ifndef REMOTEDESKTOPWIDGET_H
#define REMOTEDESKTOPWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPoint>
#include <QTimer>

class RemoteDesktopWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RemoteDesktopWidget(QWidget* parent = nullptr);
    ~RemoteDesktopWidget() = default;

    bool isConnected() const { return m_connected; }
    bool isControlEnabled() const { return m_controlEnabled; }
    void setControlEnabled(bool enabled);

    bool scaleToFit() const { return m_scaleToFit; }
    void setScaleToFit(bool scale);

    QSize remoteSize() const { return m_remoteSize; }

public slots:
    void setConnected(bool connected);
    void updateFrame(const QImage& frame);
    void clear();

signals:
    void keyPressed(int key, Qt::KeyboardModifiers modifiers);
    void keyReleased(int key, Qt::KeyboardModifiers modifiers);
    void mousePressed(int x, int y, Qt::MouseButton button);
    void mouseReleased(int x, int y, Qt::MouseButton button);
    void mouseMoved(int x, int y);
    void mouseDoubleClicked(int x, int y, Qt::MouseButton button);
    void wheelScrolled(int x, int y, int delta);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    QPoint mapToRemote(const QPoint& localPos) const;
    void updateScaledFrame();

    QImage m_frame;
    QImage m_scaledFrame;
    QSize m_remoteSize;

    bool m_connected = false;
    bool m_controlEnabled = true;
    bool m_scaleToFit = true;
    bool m_hasFocus = false;

    QRect m_displayRect;
    qreal m_scale = 1.0;
};

#endif // REMOTEDESKTOPWIDGET_H
