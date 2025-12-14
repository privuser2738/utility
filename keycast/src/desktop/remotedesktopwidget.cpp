#include "remotedesktopwidget.h"
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>

RemoteDesktopWidget::RemoteDesktopWidget(QWidget* parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setMinimumSize(320, 240);

    // Dark background when not connected
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(30, 30, 30));
    setPalette(pal);
}

void RemoteDesktopWidget::setConnected(bool connected)
{
    m_connected = connected;
    if (!connected) {
        clear();
    }
    update();
}

void RemoteDesktopWidget::setControlEnabled(bool enabled)
{
    m_controlEnabled = enabled;
    setCursor(enabled ? Qt::CrossCursor : Qt::ArrowCursor);
}

void RemoteDesktopWidget::setScaleToFit(bool scale)
{
    m_scaleToFit = scale;
    updateScaledFrame();
    update();
}

void RemoteDesktopWidget::updateFrame(const QImage& frame)
{
    m_frame = frame;
    m_remoteSize = frame.size();
    updateScaledFrame();
    update();
}

void RemoteDesktopWidget::clear()
{
    m_frame = QImage();
    m_scaledFrame = QImage();
    m_remoteSize = QSize();
    update();
}

void RemoteDesktopWidget::updateScaledFrame()
{
    if (m_frame.isNull()) {
        m_scaledFrame = QImage();
        m_displayRect = QRect();
        m_scale = 1.0;
        return;
    }

    QSize widgetSize = size();

    if (m_scaleToFit) {
        // Calculate scale to fit while maintaining aspect ratio
        qreal scaleX = static_cast<qreal>(widgetSize.width()) / m_frame.width();
        qreal scaleY = static_cast<qreal>(widgetSize.height()) / m_frame.height();
        m_scale = qMin(scaleX, scaleY);

        int scaledWidth = static_cast<int>(m_frame.width() * m_scale);
        int scaledHeight = static_cast<int>(m_frame.height() * m_scale);

        // Center the image
        int x = (widgetSize.width() - scaledWidth) / 2;
        int y = (widgetSize.height() - scaledHeight) / 2;

        m_displayRect = QRect(x, y, scaledWidth, scaledHeight);
        m_scaledFrame = m_frame.scaled(scaledWidth, scaledHeight,
                                        Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        // 1:1 scale, centered
        m_scale = 1.0;
        int x = (widgetSize.width() - m_frame.width()) / 2;
        int y = (widgetSize.height() - m_frame.height()) / 2;
        m_displayRect = QRect(x, y, m_frame.width(), m_frame.height());
        m_scaledFrame = m_frame;
    }
}

QPoint RemoteDesktopWidget::mapToRemote(const QPoint& localPos) const
{
    if (m_frame.isNull() || m_displayRect.isEmpty()) {
        return QPoint(-1, -1);
    }

    // Check if point is within display area
    if (!m_displayRect.contains(localPos)) {
        return QPoint(-1, -1);
    }

    // Map to remote coordinates
    int remoteX = static_cast<int>((localPos.x() - m_displayRect.x()) / m_scale);
    int remoteY = static_cast<int>((localPos.y() - m_displayRect.y()) / m_scale);

    // Clamp to valid range
    remoteX = qBound(0, remoteX, m_remoteSize.width() - 1);
    remoteY = qBound(0, remoteY, m_remoteSize.height() - 1);

    return QPoint(remoteX, remoteY);
}

void RemoteDesktopWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    if (!m_connected) {
        // Draw "Not Connected" message
        painter.setPen(QColor(100, 100, 100));
        QFont font = painter.font();
        font.setPointSize(14);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, "Not Connected");
        return;
    }

    if (m_scaledFrame.isNull()) {
        // Draw "Waiting for frames" message
        painter.setPen(QColor(100, 100, 100));
        QFont font = painter.font();
        font.setPointSize(12);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, "Waiting for screen data...");
        return;
    }

    // Draw the frame
    painter.drawImage(m_displayRect.topLeft(), m_scaledFrame);

    // Draw border if focused
    if (m_hasFocus && m_controlEnabled) {
        painter.setPen(QPen(QColor(0, 120, 215), 2));
        painter.drawRect(m_displayRect.adjusted(-1, -1, 1, 1));
    }
}

void RemoteDesktopWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)
    updateScaledFrame();
}

void RemoteDesktopWidget::keyPressEvent(QKeyEvent* event)
{
    if (!m_connected || !m_controlEnabled) {
        QWidget::keyPressEvent(event);
        return;
    }

    emit keyPressed(event->key(), event->modifiers());
    event->accept();
}

void RemoteDesktopWidget::keyReleaseEvent(QKeyEvent* event)
{
    if (!m_connected || !m_controlEnabled) {
        QWidget::keyReleaseEvent(event);
        return;
    }

    emit keyReleased(event->key(), event->modifiers());
    event->accept();
}

void RemoteDesktopWidget::mousePressEvent(QMouseEvent* event)
{
    if (!m_connected || !m_controlEnabled) {
        QWidget::mousePressEvent(event);
        return;
    }

    QPoint remotePos = mapToRemote(event->pos());
    if (remotePos.x() >= 0) {
        emit mousePressed(remotePos.x(), remotePos.y(), event->button());
    }
    event->accept();
}

void RemoteDesktopWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_connected || !m_controlEnabled) {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    QPoint remotePos = mapToRemote(event->pos());
    if (remotePos.x() >= 0) {
        emit mouseReleased(remotePos.x(), remotePos.y(), event->button());
    }
    event->accept();
}

void RemoteDesktopWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_connected || !m_controlEnabled) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    QPoint remotePos = mapToRemote(event->pos());
    if (remotePos.x() >= 0) {
        emit mouseMoved(remotePos.x(), remotePos.y());
    }
    event->accept();
}

void RemoteDesktopWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (!m_connected || !m_controlEnabled) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }

    QPoint remotePos = mapToRemote(event->pos());
    if (remotePos.x() >= 0) {
        emit mouseDoubleClicked(remotePos.x(), remotePos.y(), event->button());
    }
    event->accept();
}

void RemoteDesktopWidget::wheelEvent(QWheelEvent* event)
{
    if (!m_connected || !m_controlEnabled) {
        QWidget::wheelEvent(event);
        return;
    }

    QPoint remotePos = mapToRemote(event->position().toPoint());
    if (remotePos.x() >= 0) {
        emit wheelScrolled(remotePos.x(), remotePos.y(), event->angleDelta().y());
    }
    event->accept();
}

void RemoteDesktopWidget::focusInEvent(QFocusEvent* event)
{
    m_hasFocus = true;
    update();
    QWidget::focusInEvent(event);
}

void RemoteDesktopWidget::focusOutEvent(QFocusEvent* event)
{
    m_hasFocus = false;
    update();
    QWidget::focusOutEvent(event);
}
