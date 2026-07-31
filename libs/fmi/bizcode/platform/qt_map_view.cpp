#ifdef HAVE_QT6

#include "qt_map_view.h"

#include "weather_data_model.h"
#include "weather_renderer.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>

#include <algorithm>
#include <cmath>

namespace
{
    // Maps a value to a blue -> white -> red gradient across the given range.
    unsigned int adaptiveColor(float value, float dataMin, float dataMax)
    {
        if(!std::isfinite(value))
            return 0x00000000; // transparent for missing

        float range = dataMax - dataMin;
        if(range < 0.001f) range = 1.0f;
        float normalized = std::clamp((value - dataMin) / range, 0.0f, 1.0f);

        int r, g, b;
        if(normalized < 0.5f)
        {
            float f = normalized * 2.0f;
            r = static_cast<int>(f * 255);
            g = static_cast<int>(f * 255);
            b = 255;
        }
        else
        {
            float f = (normalized - 0.5f) * 2.0f;
            r = 255;
            g = static_cast<int>((1.0f - f) * 255);
            b = static_cast<int>((1.0f - f) * 255);
        }
        return 0xC0000000u | (r << 16) | (g << 8) | b; // semi-transparent
    }

    unsigned int temperatureColor(float value)
    {
        return adaptiveColor(value, -20.0f, 30.0f);
    }

    // Isoline interval rounded to a "nice" number, aiming for about 10 isolines.
    double niceIsolineInterval(float dataMin, float dataMax)
    {
        double range = dataMax - dataMin;
        if(range <= 0) return 1.0;

        double rawInterval = range / 10.0;
        double magnitude = std::pow(10.0, std::floor(std::log10(rawInterval)));
        double residual = rawInterval / magnitude;
        double niceResidual;
        if(residual <= 1.5)
            niceResidual = 1.0;
        else if(residual <= 3.5)
            niceResidual = 2.0;
        else if(residual <= 7.5)
            niceResidual = 5.0;
        else
            niceResidual = 10.0;

        return niceResidual * magnitude;
    }

    // Sample temperature field for the no-data demo screen.
    std::vector<float> generateSampleGrid(int width, int height)
    {
        std::vector<float> grid(width * height);
        for(int j = 0; j < height; ++j)
        {
            for(int i = 0; i < width; ++i)
            {
                double x = static_cast<double>(i) / (width - 1);
                double y = static_cast<double>(j) / (height - 1);
                double cx = 0.55, cy = 0.45;
                double dist = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
                double temp = 20.0 - 30.0 * dist
                    + 3.0 * std::sin(x * 8.0) * std::cos(y * 6.0)
                    + 2.0 * std::sin((x + y) * 5.0);
                grid[j * width + i] = static_cast<float>(temp);
            }
        }
        return grid;
    }
}

SmartMetMapView::SmartMetMapView(WeatherDataModel& model, QWidget* parent)
    : QWidget(parent), model_(model)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    resize(1024, 768);
    recreateBackingImage(1024, 768);
}

SmartMetMapView::~SmartMetMapView() = default;

void SmartMetMapView::setLegendVisible(bool visible)
{
    if(legendVisible_ == visible) return;
    legendVisible_ = visible;
    refresh();
}

void SmartMetMapView::refresh()
{
    if(backingImage_.isNull()) return;

    QPainter painter(&backingImage_);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.fillRect(backingImage_.rect(), QColor(240, 240, 245));

    if(demoMode_)
        drawDemo(painter);
    else
        drawModel(painter);

    painter.end();
    update();
}

void SmartMetMapView::drawModel(QPainter& painter)
{
    const int w = backingImage_.width();
    const int h = backingImage_.height();
    if(w < 10 || h < 10) return;

    if(!model_.hasData())
        return;

    if(model_.validValueCount() == 0)
    {
        painter.setPen(Qt::black);
        painter.setFont(QFont("Sans", 11));
        painter.drawText(20, 30, "No values for this parameter / level / time step");
        return;
    }

    const float dataMin = model_.dataMin();
    const float dataMax = model_.dataMax();
    auto colorFunc = [dataMin, dataMax](float v) { return adaptiveColor(v, dataMin, dataMax); };

    const auto& values = model_.gridValues();
    const int gw = model_.gridWidth();
    const int gh = model_.gridHeight();

    auto colorLayer = WeatherRenderer::renderColorGrid(values, gw, gh, w, h, colorFunc);

    const double interval = niceIsolineInterval(dataMin, dataMax);
    std::vector<double> isoValues;
    for(double v = std::ceil(dataMin / interval) * interval; v <= dataMax; v += interval)
        isoValues.push_back(v);

    auto isolineLayer = WeatherRenderer::renderIsolines(
        values, gw, gh, w, h, isoValues, 0xFF000000, 1.5);

    const double refValue = std::round(((dataMin + dataMax) / 2.0) / interval) * interval;
    auto refLine = WeatherRenderer::renderIsolines(
        values, gw, gh, w, h, {refValue}, 0xFF0000FF, 3.0);

    painter.drawImage(0, 0, WeatherRenderer::compositeLayers(w, h,
        {colorLayer, isolineLayer, refLine}));

    if(legendVisible_)
        drawLegend(painter, dataMin, dataMax);
}

void SmartMetMapView::drawDemo(QPainter& painter)
{
    const int w = backingImage_.width();
    const int h = backingImage_.height();
    if(w < 10 || h < 10) return;

    const int gridW = 50, gridH = 40;
    auto gridData = generateSampleGrid(gridW, gridH);

    auto colorLayer = WeatherRenderer::renderColorGrid(
        gridData, gridW, gridH, w, h, temperatureColor);

    std::vector<double> isoValues;
    for(double t = -15.0; t <= 25.0; t += 5.0)
        isoValues.push_back(t);

    auto isolineLayer = WeatherRenderer::renderIsolines(
        gridData, gridW, gridH, w, h, isoValues, 0xFF000000, 1.5);
    auto zeroline = WeatherRenderer::renderIsolines(
        gridData, gridW, gridH, w, h, {0.0}, 0xFF0000FF, 3.0);

    painter.drawImage(0, 0, WeatherRenderer::compositeLayers(w, h,
        {colorLayer, isolineLayer, zeroline}));

    painter.setPen(Qt::black);
    painter.setFont(QFont("Sans", 14, QFont::Bold));
    painter.drawText(20, 30, "Temperature demo - no querydata loaded");

    if(legendVisible_)
        drawLegend(painter, -20.0f, 30.0f);
}

void SmartMetMapView::drawLegend(QPainter& painter, float dataMin, float dataMax)
{
    const int w = backingImage_.width();
    const int entries = 10;
    const int legendX = w - 170;
    const int legendY = 30;
    const int legendHeight = 40 + entries * 16;

    painter.setFont(QFont("Sans", 10));
    painter.setPen(Qt::black);
    painter.fillRect(legendX - 10, legendY - 15, 160, legendHeight, QColor(255, 255, 255, 200));
    painter.drawRect(legendX - 10, legendY - 15, 160, legendHeight);
    painter.drawText(legendX, legendY,
        demoMode_ ? QString("Temperature (C)") : QString::fromStdString(model_.paramName()));

    for(int i = 0; i <= entries; ++i)
    {
        const float val = dataMin + i * (dataMax - dataMin) / entries;
        const unsigned int c = adaptiveColor(val, dataMin, dataMax);
        painter.fillRect(legendX, legendY + 15 + i * 16, 20, 14,
                         QColor((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF));
        painter.drawText(legendX + 25, legendY + 27 + i * 16,
                         QString::number(static_cast<double>(val), 'f', 1));
    }
}

void SmartMetMapView::paintEvent(QPaintEvent* /*event*/)
{
    QPainter widgetPainter(this);
    widgetPainter.drawImage(0, 0, backingImage_);
}

void SmartMetMapView::mouseMoveEvent(QMouseEvent* event)
{
    if(demoMode_ || !model_.hasData())
    {
        emit cursorReadout(QString());
        return;
    }

    const double relX = static_cast<double>(event->position().x()) / std::max(1, width() - 1);
    const double relY = static_cast<double>(event->position().y()) / std::max(1, height() - 1);

    double lat = 0, lon = 0;
    float value = 0;
    if(!model_.valueAt(relX, relY, lat, lon, value))
    {
        emit cursorReadout(QString());
        return;
    }

    emit cursorReadout(QString("%1°%2  %3°%4     %5 %6")
        .arg(std::abs(lat), 0, 'f', 2).arg(lat >= 0 ? "N" : "S")
        .arg(std::abs(lon), 0, 'f', 2).arg(lon >= 0 ? "E" : "W")
        .arg(static_cast<double>(value), 0, 'f', 2)
        .arg(QString::fromStdString(model_.paramName())));
}

void SmartMetMapView::leaveEvent(QEvent* event)
{
    emit cursorReadout(QString());
    QWidget::leaveEvent(event);
}

void SmartMetMapView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    const QSize& newSize = event->size();
    if(newSize.width() > 0 && newSize.height() > 0)
    {
        recreateBackingImage(newSize.width(), newSize.height());
        refresh();
    }
}

void SmartMetMapView::recreateBackingImage(int width, int height)
{
    QImage newImage(width, height, QImage::Format_ARGB32_Premultiplied);
    newImage.fill(Qt::white);
    backingImage_ = std::move(newImage);
}

#endif // HAVE_QT6
