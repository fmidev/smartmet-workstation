#pragma once
// The map canvas: a QWidget with a QImage backing store that draws the current
// WeatherDataModel contents through WeatherRenderer, plus a colour legend overlay.
//
// Reports the cursor position so the main window can show a lat/lon and value readout.

#ifdef HAVE_QT6

#include <QImage>
#include <QPainter>
#include <QWidget>

#include <functional>

class WeatherDataModel;

class SmartMetMapView : public QWidget
{
    Q_OBJECT

public:
    explicit SmartMetMapView(WeatherDataModel& model, QWidget* parent = nullptr);
    ~SmartMetMapView() override;

    // Redraws the backing image from the model. Call after the model changes.
    void refresh();

    // Draw the built-in sample field instead of the model (no data was loaded).
    void setDemoMode(bool demo) { demoMode_ = demo; }
    bool demoMode() const { return demoMode_; }

    void setLegendVisible(bool visible);
    bool legendVisible() const { return legendVisible_; }

    // Lets SmartMet's real map view draw the canvas instead of the standalone renderer.
    // The callback gets the painter and the canvas size and returns true if it drew.
    // Kept as a callback so this widget does not have to know about the document stack.
    void setDocumentRenderer(std::function<bool(QPainter&, int, int)> renderer)
    {
        documentRenderer_ = std::move(renderer);
    }

signals:
    // Formatted "64.20°N 27.50°E   1013.40 Pressure" readout, empty when off the data.
    void cursorReadout(const QString& text);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void recreateBackingImage(int width, int height);
    void drawModel(QPainter& painter);
    void drawDemo(QPainter& painter);
    void drawLegend(QPainter& painter, float dataMin, float dataMax);

    WeatherDataModel& model_;
    QImage backingImage_;
    std::function<bool(QPainter&, int, int)> documentRenderer_;
    bool demoMode_ = false;
    bool legendVisible_ = true;
};

#endif // HAVE_QT6
