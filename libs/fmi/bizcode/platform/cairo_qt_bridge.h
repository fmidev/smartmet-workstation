#pragma once
// Bridge between Cairo-based rendering (Giza/Imagine2/Trax) and Qt display.
// Weather data is rendered via Cairo to raw ARGB pixels, then blitted into QImage
// for display in the Qt GUI. The QPainter is used for interactive overlays.

#ifdef HAVE_QT6

#include <QImage>
#include <cairo/cairo.h>
#include <string>
#include <vector>
#include <cstring>

namespace CairoQtBridge
{
    // Create a QImage from a Cairo image surface (shares or copies pixel data).
    // The cairo surface must be CAIRO_FORMAT_ARGB32.
    inline QImage cairoSurfaceToQImage(cairo_surface_t* surface)
    {
        cairo_surface_flush(surface);
        int width = cairo_image_surface_get_width(surface);
        int height = cairo_image_surface_get_height(surface);
        int stride = cairo_image_surface_get_stride(surface);
        unsigned char* data = cairo_image_surface_get_data(surface);

        // Cairo ARGB32 and Qt ARGB32_Premultiplied use the same pixel layout
        // on little-endian systems (both are native-endian 0xAARRGGBB).
        // We copy the data so the QImage owns it and the surface can be freed.
        QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
        for(int y = 0; y < height; ++y)
            std::memcpy(image.scanLine(y), data + y * stride, width * 4);

        return image;
    }

    // Create a QImage from raw ARGB pixel array (e.g. from Giza::toargb).
    // Takes ownership of the data by copying it.
    inline QImage argbToQImage(const unsigned int* argb, int width, int height)
    {
        QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
        for(int y = 0; y < height; ++y)
            std::memcpy(image.scanLine(y), argb + y * width, width * 4);
        return image;
    }

    // Create a QImage from a PNG byte string (e.g. from Giza::topng).
    inline QImage pngToQImage(const std::string& pngData)
    {
        QImage image;
        image.loadFromData(reinterpret_cast<const uchar*>(pngData.data()),
                           static_cast<int>(pngData.size()), "PNG");
        return image;
    }

    // Create a Cairo image surface from a QImage (for passing Qt content to Cairo).
    // Caller must call cairo_surface_destroy() when done.
    inline cairo_surface_t* qImageToCairoSurface(const QImage& image)
    {
        QImage argbImage = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        int width = argbImage.width();
        int height = argbImage.height();

        // Cairo needs the data to persist, so we create a surface and copy into it
        cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
        unsigned char* surfData = cairo_image_surface_get_data(surface);
        int surfStride = cairo_image_surface_get_stride(surface);

        cairo_surface_flush(surface);
        for(int y = 0; y < height; ++y)
            std::memcpy(surfData + y * surfStride, argbImage.constScanLine(y), width * 4);
        cairo_surface_mark_dirty(surface);

        return surface;
    }

} // namespace CairoQtBridge

#endif // HAVE_QT6
