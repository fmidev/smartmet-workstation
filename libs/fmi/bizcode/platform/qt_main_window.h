#pragma once
// Minimal Qt6 main window for SmartMet Linux builds.
// Provides a QWidget with a QImage backing store and QPainter access.

#ifdef HAVE_QT6

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QKeyEvent>

class SmartMetMainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SmartMetMainWindow(QWidget* parent = nullptr);
    ~SmartMetMainWindow() override;

    // Access the backing QImage (for off-screen rendering)
    QImage& backingImage() { return backingImage_; }
    const QImage& backingImage() const { return backingImage_; }

    // Begin a drawing cycle: returns a QPainter* targeting the backing image.
    // Caller must call endDrawing() when done.
    QPainter* beginDrawing();
    void endDrawing();

    // Current pixel dimensions of the widget
    int pixelWidth() const { return backingImage_.width(); }
    int pixelHeight() const { return backingImage_.height(); }

signals:
    void resized(int width, int height);
    void keyPressed(int key, int modifiers);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void recreateBackingImage(int width, int height);

    QImage backingImage_;
    QPainter* activePainter_ = nullptr;
};

#endif // HAVE_QT6
