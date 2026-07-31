#pragma once
// The map canvas: a QWidget with a QImage backing store that draws the current
// WeatherDataModel contents through WeatherRenderer, plus a colour legend overlay.
//
// Reports the cursor position so the main window can show a lat/lon and value readout.

#ifdef HAVE_QT6

#include "weather_renderer.h"

#include <QImage>
#include <QPainter>
#include <QPoint>
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

    // ---- zoom and pan ----
    // Mouse wheel zooms around the cursor, dragging with the left button pans, and a
    // double click or resetZoom() goes back to the whole grid.
    void zoomAt(const QPointF& widgetPos, double factor);
    void resetZoom();
    bool isZoomed() const;
    double zoomFactor() const;

signals:
    // Formatted "64.20°N 27.50°E   1013.40 Pressure" readout, empty when off the data.
    void cursorReadout(const QString& text);
    // Emitted whenever the visible area changes, so the window can show the zoom level.
    void viewChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void recreateBackingImage(int width, int height);
    void drawModel(QPainter& painter);
    void drawDemo(QPainter& painter);
    void drawLegend(QPainter& painter, float dataMin, float dataMax);

    // Widget pixel -> normalized grid coordinates inside the current visible rectangle
    void widgetToGrid(const QPointF& widgetPos, double& u, double& v) const;
    // Keeps the visible rectangle inside the grid and no larger than the whole grid
    void clampView();
    void emitCursorReadout(const QPointF& widgetPos);

    WeatherDataModel& model_;
    QImage backingImage_;
    std::function<bool(QPainter&, int, int)> documentRenderer_;
    WeatherRenderer::GridView view_;
    bool demoMode_ = false;
    bool legendVisible_ = true;

    // Panning state
    bool panning_ = false;
    QPointF panStartPos_;
    WeatherRenderer::GridView panStartView_;
};

#endif // HAVE_QT6
