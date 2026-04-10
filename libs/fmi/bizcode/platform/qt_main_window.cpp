#ifdef HAVE_QT6

#include "qt_main_window.h"

SmartMetMainWindow::SmartMetMainWindow(QWidget* parent)
    : QWidget(parent)
{
    // Start with a reasonable default size
    resize(1024, 768);
    recreateBackingImage(1024, 768);
}

SmartMetMainWindow::~SmartMetMainWindow()
{
    if(activePainter_)
    {
        activePainter_->end();
        delete activePainter_;
        activePainter_ = nullptr;
    }
}

QPainter* SmartMetMainWindow::beginDrawing()
{
    if(activePainter_)
        return activePainter_;

    activePainter_ = new QPainter(&backingImage_);
    activePainter_->setRenderHint(QPainter::Antialiasing, true);
    activePainter_->setRenderHint(QPainter::TextAntialiasing, true);
    return activePainter_;
}

void SmartMetMainWindow::endDrawing()
{
    if(activePainter_)
    {
        activePainter_->end();
        delete activePainter_;
        activePainter_ = nullptr;
    }
    update(); // Schedule a repaint to display the backing image
}

void SmartMetMainWindow::paintEvent(QPaintEvent* /*event*/)
{
    QPainter widgetPainter(this);
    widgetPainter.drawImage(0, 0, backingImage_);
}

void SmartMetMainWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    const QSize& newSize = event->size();
    if(newSize.width() > 0 && newSize.height() > 0)
    {
        recreateBackingImage(newSize.width(), newSize.height());
        emit resized(newSize.width(), newSize.height());
    }
}

void SmartMetMainWindow::recreateBackingImage(int width, int height)
{
    // End any active drawing before replacing the image
    if(activePainter_)
    {
        activePainter_->end();
        delete activePainter_;
        activePainter_ = nullptr;
    }

    QImage newImage(width, height, QImage::Format_ARGB32_Premultiplied);
    newImage.fill(Qt::white);

    // If old image had content, copy it over (partial resize preservation)
    if(!backingImage_.isNull())
    {
        QPainter copyPainter(&newImage);
        copyPainter.drawImage(0, 0, backingImage_);
        copyPainter.end();
    }

    backingImage_ = std::move(newImage);
}

#endif // HAVE_QT6
