// Weather data renderer using Cairo for rasterization and Trax for contour generation.

#ifdef HAVE_QT6

#include "weather_renderer.h"
#include "cairo_qt_bridge.h"
#include <QPainter>

#include <trax/Contour.h>
#include <trax/IsobandLimits.h>
#include <trax/IsolineValues.h>
#include <trax/Grid.h>

#include <cairo/cairo.h>
#include <cmath>
#include <limits>

namespace
{

// Adapter class to feed grid data to Trax
class SimpleGrid : public Trax::Grid
{
    std::vector<float> values_;
    int width_, height_;
public:
    SimpleGrid(const std::vector<float>& values, int w, int h)
        : values_(values), width_(w), height_(h) {}

    std::size_t width() const override { return width_; }
    std::size_t height() const override { return height_; }

    // Grid coordinates map linearly to image space
    double x(long i, long j) const override
    {
        return static_cast<double>(i);
    }
    double y(long i, long j) const override
    {
        return static_cast<double>(j);
    }

    float operator()(long i, long j) const override
    {
        return values_[j * width_ + i];
    }

    void set(long i, long j, float z) override
    {
        values_[j * width_ + i] = z;
    }

    bool valid(long i, long j) const override
    {
        float v = values_[j * width_ + i];
        return std::isfinite(v);
    }
};

// Scale grid coordinates to image pixel coordinates
double scaleX(double gridX, int gridWidth, int imageWidth)
{
    return gridX * (imageWidth - 1.0) / (gridWidth - 1.0);
}

double scaleY(double gridY, int gridHeight, int imageHeight)
{
    return gridY * (imageHeight - 1.0) / (gridHeight - 1.0);
}

// Draw a polyline on a Cairo context, scaling from grid to image coords
void drawPolyline(cairo_t* cr,
                  const std::vector<double>& xcoords,
                  const std::vector<double>& ycoords,
                  int gridWidth, int gridHeight,
                  int imageWidth, int imageHeight)
{
    if(xcoords.size() < 2) return;

    cairo_move_to(cr,
        scaleX(xcoords[0], gridWidth, imageWidth),
        scaleY(ycoords[0], gridHeight, imageHeight));

    for(size_t i = 1; i < xcoords.size(); ++i)
    {
        cairo_line_to(cr,
            scaleX(xcoords[i], gridWidth, imageWidth),
            scaleY(ycoords[i], gridHeight, imageHeight));
    }
}

// Extract ARGB components
void argbComponents(unsigned int argb, double& r, double& g, double& b, double& a)
{
    a = ((argb >> 24) & 0xFF) / 255.0;
    r = ((argb >> 16) & 0xFF) / 255.0;
    g = ((argb >> 8) & 0xFF) / 255.0;
    b = (argb & 0xFF) / 255.0;
}

} // anonymous namespace


namespace WeatherRenderer
{

QImage renderIsolines(
    const std::vector<float>& gridValues,
    int gridWidth, int gridHeight,
    int imageWidth, int imageHeight,
    const std::vector<double>& isoValues,
    unsigned int lineColor,
    double lineWidth)
{
    // Create Cairo surface
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, imageWidth, imageHeight);
    cairo_t* cr = cairo_create(surface);

    // Clear to transparent
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    // Set line style
    double r, g, b, a;
    argbComponents(lineColor, r, g, b, a);
    cairo_set_source_rgba(cr, r, g, b, a);
    cairo_set_line_width(cr, lineWidth);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

    // Generate isolines with Trax
    SimpleGrid grid(gridValues, gridWidth, gridHeight);
    Trax::Contour contour;
    contour.interpolation(Trax::InterpolationType::Linear);

    Trax::IsolineValues values;
    for(double v : isoValues)
        values.add(v);

    auto results = contour.isolines(grid, values);

    // Draw each isoline collection
    for(const auto& collection : results)
    {
        for(const auto& polyline : collection.polylines())
        {
            auto xcoords = polyline.xcoordinates();
            auto ycoords = polyline.ycoordinates();
            drawPolyline(cr, xcoords, ycoords, gridWidth, gridHeight, imageWidth, imageHeight);
            cairo_stroke(cr);
        }
    }

    cairo_destroy(cr);

    // Convert to QImage
    QImage result = CairoQtBridge::cairoSurfaceToQImage(surface);
    cairo_surface_destroy(surface);
    return result;
}

QImage renderIsobands(
    const std::vector<float>& gridValues,
    int gridWidth, int gridHeight,
    int imageWidth, int imageHeight,
    const std::vector<std::pair<double, double>>& limits,
    const std::vector<unsigned int>& colors)
{
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, imageWidth, imageHeight);
    cairo_t* cr = cairo_create(surface);

    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);

    SimpleGrid grid(gridValues, gridWidth, gridHeight);
    Trax::Contour contour;
    contour.interpolation(Trax::InterpolationType::Linear);

    Trax::IsobandLimits bandLimits;
    for(const auto& [lo, hi] : limits)
        bandLimits.add(lo, hi);

    auto results = contour.isobands(grid, bandLimits);

    // Draw each isoband with its color
    for(size_t i = 0; i < results.size() && i < colors.size(); ++i)
    {
        double r, g, b, a;
        argbComponents(colors[i], r, g, b, a);
        cairo_set_source_rgba(cr, r, g, b, a);

        for(const auto& polygon : results[i].polygons())
        {
            // Exterior ring
            auto ext = polygon.exterior();
            auto xcoords = ext.xcoordinates();
            auto ycoords = ext.ycoordinates();
            if(xcoords.size() >= 3)
            {
                cairo_move_to(cr,
                    scaleX(xcoords[0], gridWidth, imageWidth),
                    scaleY(ycoords[0], gridHeight, imageHeight));
                for(size_t k = 1; k < xcoords.size(); ++k)
                    cairo_line_to(cr,
                        scaleX(xcoords[k], gridWidth, imageWidth),
                        scaleY(ycoords[k], gridHeight, imageHeight));
                cairo_close_path(cr);
            }

            // Holes (subtract from fill)
            for(const auto& hole : polygon.holes())
            {
                auto hx = hole.xcoordinates();
                auto hy = hole.ycoordinates();
                if(hx.size() >= 3)
                {
                    cairo_move_to(cr,
                        scaleX(hx[0], gridWidth, imageWidth),
                        scaleY(hy[0], gridHeight, imageHeight));
                    for(size_t k = 1; k < hx.size(); ++k)
                        cairo_line_to(cr,
                            scaleX(hx[k], gridWidth, imageWidth),
                            scaleY(hy[k], gridHeight, imageHeight));
                    cairo_close_path(cr);
                }
            }

            cairo_fill(cr);
        }
    }

    cairo_destroy(cr);
    QImage result = CairoQtBridge::cairoSurfaceToQImage(surface);
    cairo_surface_destroy(surface);
    return result;
}

QImage renderColorGrid(
    const std::vector<float>& gridValues,
    int gridWidth, int gridHeight,
    int imageWidth, int imageHeight,
    std::function<unsigned int(float)> colorFunc)
{
    QImage image(imageWidth, imageHeight, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    // Simple nearest-neighbor mapping from image pixels to grid values
    for(int iy = 0; iy < imageHeight; ++iy)
    {
        auto* scanline = reinterpret_cast<unsigned int*>(image.scanLine(iy));
        for(int ix = 0; ix < imageWidth; ++ix)
        {
            // Map image pixel to grid coordinate
            int gx = static_cast<int>(std::round(static_cast<double>(ix) * (gridWidth - 1) / (imageWidth - 1)));
            int gy = static_cast<int>(std::round(static_cast<double>(iy) * (gridHeight - 1) / (imageHeight - 1)));
            gx = std::clamp(gx, 0, gridWidth - 1);
            gy = std::clamp(gy, 0, gridHeight - 1);

            float value = gridValues[gy * gridWidth + gx];
            if(std::isfinite(value))
                scanline[ix] = colorFunc(value);
        }
    }

    return image;
}

QImage compositeLayers(int width, int height, const std::vector<QImage>& layers)
{
    QImage result(width, height, QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    for(const auto& layer : layers)
    {
        if(!layer.isNull())
            painter.drawImage(0, 0, layer.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
    painter.end();

    return result;
}

} // namespace WeatherRenderer

#endif // HAVE_QT6
