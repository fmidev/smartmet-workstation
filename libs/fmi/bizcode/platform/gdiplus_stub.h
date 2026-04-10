#pragma once
// Minimal GDI+ type stubs for Linux builds.
// Provides only the types/enums needed so that GDI+-using source files compile.
// When HAVE_QT6 is defined, the Graphics class wraps a QPainter for real drawing.

#ifdef UNIX

#include <string>
#include <vector>

#ifdef HAVE_QT6
#include <QPainter>
#include <QPainterPath>
#include <QImage>
#include <QFont>
#include <QColor>
#include <QPen>
#include <QBrush>
#endif

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

#ifdef HAVE_QT6
        QColor toQColor() const { return QColor(R, G, B, A); }
#endif
    };

    inline const Color Color::PaleGoldenrod = Color(238, 232, 170);

    class SolidBrush
    {
    public:
        Color color_;
        SolidBrush(const Color& c) : color_(c) {}
    };

    class StringFormat
    {
    public:
        StringAlignment alignment_ = StringAlignmentNear;
        StringAlignment lineAlignment_ = StringAlignmentNear;
        void SetAlignment(StringAlignment a) { alignment_ = a; }
        void SetLineAlignment(StringAlignment a) { lineAlignment_ = a; }
    };

    class Font
    {
    public:
        std::wstring familyName_;
        REAL size_;
        int style_;
        Font(const wchar_t* family, REAL size, int style = FontStyleRegular, int /*unit*/ = UnitPixel)
            : familyName_(family ? family : L""), size_(size), style_(style) {}
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
#ifdef HAVE_QT6
        // QPainter-backed implementation
        Graphics() : painter_(nullptr) {}
        explicit Graphics(QPainter* p) : painter_(p) {}

        void setPainter(QPainter* p) { painter_ = p; }
        QPainter* painter() const { return painter_; }

        void SetClip(const Rect& r)
        {
            if(painter_)
                painter_->setClipRect(QRect(r.X, r.Y, r.Width, r.Height));
        }
        void SetClip(const RectF& r)
        {
            if(painter_)
                painter_->setClipRect(QRectF(r.X, r.Y, r.Width, r.Height));
        }
        void ResetClip()
        {
            if(painter_)
                painter_->setClipping(false);
        }
        void SetSmoothingMode(SmoothingMode mode)
        {
            smoothingMode_ = mode;
            if(painter_)
            {
                bool antialias = (mode == SmoothingModeAntiAlias || mode == SmoothingModeHighQuality);
                painter_->setRenderHint(QPainter::Antialiasing, antialias);
            }
        }
        SmoothingMode GetSmoothingMode() const { return smoothingMode_; }
        void DrawLine(void*, float x1, float y1, float x2, float y2)
        {
            if(painter_)
                painter_->drawLine(QPointF(x1, y1), QPointF(x2, y2));
        }
        void DrawString(const wchar_t* str, int len, const Font* font, const PointF& origin,
                        const StringFormat* /*fmt*/, const SolidBrush* brush)
        {
            if(!painter_) return;
            if(font)
            {
                QFont qf(QString::fromStdWString(font->familyName_), static_cast<int>(font->size_));
                qf.setBold(font->style_ & FontStyleBold);
                qf.setItalic(font->style_ & FontStyleItalic);
                painter_->setFont(qf);
            }
            if(brush)
                painter_->setPen(QPen(brush->color_.toQColor()));
            QString text = str ? QString::fromWCharArray(str, len >= 0 ? len : -1) : QString();
            painter_->drawText(QPointF(origin.X, origin.Y), text);
        }
        void DrawCurve(void*, const PointF* pts, int count)
        {
            if(!painter_ || !pts || count < 2) return;
            QPainterPath path;
            path.moveTo(pts[0].X, pts[0].Y);
            for(int i = 1; i < count; ++i)
                path.lineTo(pts[i].X, pts[i].Y);
            painter_->drawPath(path);
        }

    private:
        QPainter* painter_ = nullptr;
        SmoothingMode smoothingMode_ = SmoothingModeDefault;

#else
        // No-op stub implementation (no Qt6)
        void SetClip(const Rect&) {}
        void SetClip(const RectF&) {}
        void ResetClip() {}
        void SetSmoothingMode(SmoothingMode) {}
        SmoothingMode GetSmoothingMode() const { return SmoothingModeDefault; }
        void DrawLine(void*, float, float, float, float) {}
        void DrawString(const wchar_t*, int, const Font*, const PointF&, const StringFormat*, const SolidBrush*) {}
        void DrawCurve(void*, const PointF*, int) {}
#endif // HAVE_QT6
    };
}

// Bring PointF and REAL into global scope as NFmiTempView.cpp uses "using namespace Gdiplus;"
// but also has bare PointF/REAL usage
using Gdiplus::PointF;

#endif // UNIX
