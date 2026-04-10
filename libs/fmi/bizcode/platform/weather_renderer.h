#pragma once
// Weather data renderer using Cairo/Trax/Giza for the heavy lifting.
// Produces QImage tiles that the Qt GUI composites for display.
//
// Architecture:
//   NFmiFastQueryInfo (data) → Trax (contour geometry) → Cairo (rasterize) → QImage
//   The QPainter-based GUI then overlays interactive elements on top.

#ifdef HAVE_QT6

#include "cairo_qt_bridge.h"
#include <cairo/cairo.h>

#include <QImage>
#include <vector>
#include <string>
#include <cmath>
#include <functional>

// Forward declarations for smartmet types
class NFmiArea;
class NFmiRect;

namespace WeatherRenderer
{

// A rendered map layer as a QImage with its geographic bounds.
struct RenderedLayer
{
    QImage image;
    double west = 0, south = 0, east = 0, north = 0;  // geographic bounds
};

// Color definition for isolines/isobands
struct IsoColor
{
    double value;   // data value
    unsigned char r, g, b, a;
};

// Render isolines from grid data onto a QImage.
// gridValues: row-major grid data (width * height)
// isoValues: data values at which to draw contour lines
// colors: ARGB color for each isoline
// lineWidth: line thickness in pixels
// Returns a QImage with transparent background and isolines drawn.
QImage renderIsolines(
    const std::vector<float>& gridValues,
    int gridWidth, int gridHeight,
    int imageWidth, int imageHeight,
    const std::vector<double>& isoValues,
    unsigned int lineColor = 0xFF000000,  // ARGB black
    double lineWidth = 1.0);

// Render filled isobands (contour fills) from grid data.
// limits: pairs of (lo, hi) values defining each band
// colors: ARGB fill color for each band
QImage renderIsobands(
    const std::vector<float>& gridValues,
    int gridWidth, int gridHeight,
    int imageWidth, int imageHeight,
    const std::vector<std::pair<double, double>>& limits,
    const std::vector<unsigned int>& colors);  // ARGB for each band

// Render a color-mapped raster image from grid data.
// colorFunc: maps a float data value to ARGB color
QImage renderColorGrid(
    const std::vector<float>& gridValues,
    int gridWidth, int gridHeight,
    int imageWidth, int imageHeight,
    std::function<unsigned int(float)> colorFunc);

// Composite multiple rendered layers into a single image.
QImage compositeLayers(
    int width, int height,
    const std::vector<QImage>& layers);

} // namespace WeatherRenderer

#endif // HAVE_QT6
