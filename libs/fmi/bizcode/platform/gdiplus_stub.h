#pragma once
// Minimal GDI+ type stubs for Linux builds.
// Provides only the types/enums needed so that GDI+-using source files compile.
// All drawing is a no-op on Linux; the real UI will use Qt.

#ifdef UNIX

#include <string>
#include <vector>

using REAL = float;

namespace Gdiplus
{
    using REAL = float;

    enum SmoothingMode
    {
        SmoothingModeDefault = 0,
        SmoothingModeHighSpeed = 1,
        SmoothingModeHighQuality = 2,
        SmoothingModeAntiAlias = 4
    };

    enum DashStyle
    {
        DashStyleSolid = 0,
        DashStyleDash = 1,
        DashStyleDot = 2,
        DashStyleDashDot = 3,
        DashStyleDashDotDot = 4
    };

    enum Unit
    {
        UnitWorld = 0,
        UnitDisplay = 1,
        UnitPixel = 2,
        UnitPoint = 3,
        UnitInch = 4,
        UnitDocument = 5,
        UnitMillimeter = 6
    };

    enum FontStyle
    {
        FontStyleRegular = 0,
        FontStyleBold = 1,
        FontStyleItalic = 2,
        FontStyleBoldItalic = 3,
        FontStyleUnderline = 4,
        FontStyleStrikeout = 8
    };

    enum StringAlignment
    {
        StringAlignmentNear = 0,
        StringAlignmentCenter = 1,
        StringAlignmentFar = 2
    };

    struct PointF
    {
        REAL X = 0;
        REAL Y = 0;
        PointF() = default;
        PointF(REAL x, REAL y) : X(x), Y(y) {}
    };

    struct Point
    {
        int X = 0;
        int Y = 0;
        Point() = default;
        Point(int x, int y) : X(x), Y(y) {}
    };

    struct RectF
    {
        REAL X = 0, Y = 0, Width = 0, Height = 0;
        RectF() = default;
        RectF(REAL x, REAL y, REAL w, REAL h) : X(x), Y(y), Width(w), Height(h) {}
    };

    struct Rect
    {
        int X = 0, Y = 0, Width = 0, Height = 0;
        Rect() = default;
        Rect(int x, int y, int w, int h) : X(x), Y(y), Width(w), Height(h) {}
    };

    struct Color
    {
        unsigned char A = 255, R = 0, G = 0, B = 0;
        Color() = default;
        Color(unsigned char r, unsigned char g, unsigned char b) : R(r), G(g), B(b) {}
        Color(unsigned char a, unsigned char r, unsigned char g, unsigned char b) : A(a), R(r), G(g), B(b) {}
        static const Color PaleGoldenrod;
    };

    inline const Color Color::PaleGoldenrod = Color(238, 232, 170);

    class SolidBrush
    {
    public:
        SolidBrush(const Color&) {}
    };

    class StringFormat
    {
    public:
        void SetAlignment(StringAlignment) {}
        void SetLineAlignment(StringAlignment) {}
    };

    class Font
    {
    public:
        Font(const wchar_t*, REAL, int = FontStyleRegular, int = UnitPixel) {}
    };

    class GraphicsPath
    {
    public:
        void AddRectangle(const RectF&) {}
    };

    class Bitmap {};

    class Graphics
    {
    public:
        void SetClip(const Rect&) {}
        void SetClip(const RectF&) {}
        void ResetClip() {}
        void SetSmoothingMode(SmoothingMode) {}
        SmoothingMode GetSmoothingMode() const { return SmoothingModeDefault; }
        void DrawLine(void*, float, float, float, float) {}
        void DrawString(const wchar_t*, int, const Font*, const PointF&, const StringFormat*, const SolidBrush*) {}
        void DrawCurve(void*, const PointF*, int) {}
    };
}

// Bring PointF and REAL into global scope as NFmiTempView.cpp uses "using namespace Gdiplus;"
// but also has bare PointF/REAL usage
using Gdiplus::PointF;

#endif // UNIX
