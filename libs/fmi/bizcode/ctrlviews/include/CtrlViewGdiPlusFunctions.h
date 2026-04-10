#pragma once

#ifndef UNIX

#include "stdafx.h"
#include "NFmiColor.h"
#include "NFmiRect.h"
#include <gdiplus.h>
#include <vector>
#include <list>
#include <boost/shared_ptr.hpp>

class NFmiToolBox;
class NFmiPoint;
class NFmiCtrlView;
class GdiPlusLineInfo;
class NFmiDrawParam;

class NFmiImageAttributes
{
    Gdiplus::Color transparentColor_ = Gdiplus::Color(Gdiplus::Color::PaleGoldenrod);
    Gdiplus::REAL alpha_ = 0.f;
    bool useTransparentColor_ = false;
    bool useAlpha_ = false;
public:
    NFmiImageAttributes();
    NFmiImageAttributes(Gdiplus::REAL alpha);
    NFmiImageAttributes(COLORREF transparentColorRef);
    NFmiImageAttributes(const Gdiplus::Color &transparentColor);
    NFmiImageAttributes(const Gdiplus::Color &transparentColor, Gdiplus::REAL alpha);

    bool IsAnyAttributeSet() const;
    bool IsTransparentColorUsed() const;
    bool IsAlphaUsed() const;
    const Gdiplus::Color& TransparentColor() const;
    Gdiplus::REAL Alpha() const;
};

namespace CtrlView
{
    // Color related functions
    Gdiplus::Color NFmiColor2GdiplusColor(const NFmiColor &theColor);
    // theBrightningFactor on prosentti luku, jolla annettua v�ri� saadaan vaalennettua tai tummennettua.
    // jos prosentti luku on > 0, vaalenee v�ri, jos se on < 0, tummenee v�ri.
    double CalcBrightningFactor(int theStartIndex, int theModelRunCount, int theCurrentIndex);
    NFmiColor ColorRef2Color(COLORREF color);
    COLORREF Color2ColorRef(const NFmiColor &theColor);

    // String manipulations
    std::wstring StringToWString(const std::string& s);
    std::string WStringToString(const std::wstring& s);

    // Relative (toolbox) to Gdi+ coordinate transformation
    const Gdiplus::Rect CRect2GdiplusRect(const CRect &theCRect);
    const Gdiplus::RectF CRect2GdiplusRectF(const CRect &theCRect);
    const Gdiplus::Rect Relative2GdiplusRect(NFmiToolBox *theToolBox, const NFmiRect &theRelativeRect);
    const Gdiplus::RectF Relative2GdiplusRectF(NFmiToolBox *theToolBox, const NFmiRect &theRelativeRect);
    const NFmiRect GdiplusRect2Relative(NFmiToolBox *theToolBox, const Gdiplus::RectF &theGdiplusRect);
    const NFmiRect GdiplusRect2Relative(NFmiToolBox* theToolBox, const Gdiplus::Rect& theGdiplusRect);
    const NFmiPoint GdiplusPoint2Relative(NFmiToolBox *theToolBox, const Gdiplus::Point &theGdiplusPoint);
    const Gdiplus::PointF Relative2GdiplusPoint(NFmiToolBox* theToolBox, const NFmiPoint& theRelativePoint);
    const Gdiplus::Point Relative2GdiplusPointInt(NFmiToolBox *theToolBox, const NFmiPoint &theRelativePoint);
    std::vector<Gdiplus::PointF> Relative2GdiplusPolyLine(NFmiToolBox *theToolBox, const std::vector<NFmiPoint> &theRelativePolyLine, const NFmiPoint &theRelativeOffset);
    NFmiPoint RelativeSizeToPixelSize(const NFmiPoint& relativeSize, NFmiToolBox& theToolBox);
    void MakePathFromRect(Gdiplus::GraphicsPath &thePath, NFmiToolBox *theToolBox, const NFmiRect &theRelativeRect);
    Gdiplus::RectF GetStringBoundingBox(Gdiplus::Graphics& theGdiPlusGraphics, const std::string& theString, float theFontSizeInPixels, const Gdiplus::PointF& theOringinInPixels, const std::wstring& theFontNameStr);
    Gdiplus::RectF GetStringBoundingBox(Gdiplus::Graphics &theGdiPlusGraphics, const std::string &theString, const Gdiplus::PointF &theOringinInPixels, const Gdiplus::Font &theFont);
    Gdiplus::RectF GetWStringBoundingBox(Gdiplus::Graphics& theGdiPlusGraphics, const std::wstring& theString, float theFontSizeInPixels, const Gdiplus::PointF& theOringinInPixels, const std::wstring& theFontNameStr);
    Gdiplus::RectF GetWStringBoundingBox(Gdiplus::Graphics& theGdiPlusGraphics, const std::wstring& theString, const Gdiplus::PointF& theOringinInPixels, const Gdiplus::Font& theFont);

    // Drawing functions
    void DrawBitmapToDC_4(CDC *theDC, Gdiplus::Bitmap &theBitmap, const NFmiRect &theSourcePixels, const Gdiplus::RectF &theDestPixels, bool fDoNearestInterpolation, const NFmiImageAttributes &theImageAttr = NFmiImageAttributes(), Gdiplus::Graphics *theGdiPlusGraphics = 0);
    Gdiplus::Bitmap* CreateBitmapFromFile(const std::string &thePath, const std::string &theFileName);
    Gdiplus::Bitmap* CreateBitmapFromFile(const std::string &thePathAndFileName);
    void SetGdiplusAlignment(FmiDirection theAlignment, Gdiplus::StringFormat &theStringFormat);
    const Gdiplus::PointF ConvertLatlonToGdiPlusPoint(NFmiCtrlView *theView, const NFmiPoint &theLatLon);
    std::vector<Gdiplus::PointF> ConvertLatLon2GdiPoints(NFmiCtrlView *theView, const std::vector<NFmiPoint> &theLatlonPath);
    const NFmiPoint ConvertGdiPlusPointToLatlon(NFmiCtrlView *theView, const Gdiplus::PointF &theGdiPlusPoint);
    void DrawGdiplusCurve(Gdiplus::Graphics &theGraphics, std::vector<Gdiplus::PointF> &thePoints, const GdiPlusLineInfo &theLineInfo, bool fFill, int fillHatchStyle, bool fPrinting, std::vector<Gdiplus::REAL> *theDashPatternVector = 0);
    void DrawGdiplusCurve(Gdiplus::Graphics &theGraphics, std::vector<Gdiplus::PointF> &thePoints, const GdiPlusLineInfo &theLineInfo, bool fFill, const NFmiColor &fillColor, int fillHatchStyle, bool fPrinting, std::vector<Gdiplus::REAL> *theDashPatternVector = 0);
    size_t DrawGdiplusSimpleMultiPolyLine(Gdiplus::Graphics &theGraphics, NFmiToolBox *theToolBox, const std::list<std::vector<NFmiPoint>> &theMultiPolyLine, const NFmiColor &theLineColor, int theLineThickness, const NFmiPoint &theRelativeOffSet);
    std::unique_ptr<Gdiplus::Font> CreateFontPtr(float theFontSizeInPixels, const std::wstring& theFontNameStr, Gdiplus::FontStyle theFontStyle);
    std::unique_ptr<Gdiplus::Font> CreateFontPtr(double theFontSizeInMM, double pixelsPerMM, const std::wstring& theFontNameStr, Gdiplus::FontStyle theFontStyle);
    void DrawTextToRelativeLocation(Gdiplus::Graphics& theGdiPlusGraphics, const NFmiColor& theColor, double theFontSizeInMM, const std::string& theStr, const NFmiPoint& thePlace, double pixelsPerMM, NFmiToolBox* theToolbox, const std::wstring& theFontNameStr, FmiDirection theAlingment, Gdiplus::FontStyle theFontStyle = Gdiplus::FontStyleRegular);
    void DrawTextToRelativeLocation(Gdiplus::Graphics &theGdiPlusGraphics, const NFmiColor &theColor, double theFontSizeInMM, const std::wstring &theStr, const NFmiPoint &thePlace, double pixelsPerMM, NFmiToolBox *theToolbox, const std::wstring &theFontNameStr, FmiDirection theAlingment, Gdiplus::FontStyle theFontStyle = Gdiplus::FontStyleRegular);
    void DrawWrappedTextToRelativeLocation(Gdiplus::Graphics& theGdiPlusGraphics, const NFmiColor& theColor, double theStartFontSizeInMM, const std::string& theStr, double pixelsPerMM, NFmiToolBox* theToolbox, const std::wstring& theFontNameStr, FmiDirection theAlingment, const Gdiplus::RectF &layoutRectInPixels, Gdiplus::FontStyle theFontStyle = Gdiplus::FontStyleRegular);
    void DrawWrappedTextToRelativeLocation(Gdiplus::Graphics& theGdiPlusGraphics, const NFmiColor& theColor, double theStartFontSizeInMM, const std::wstring& theStr, double pixelsPerMM, NFmiToolBox* theToolbox, const std::wstring& theFontNameStr, FmiDirection theAlingment, const Gdiplus::RectF& layoutRectInPixels, Gdiplus::FontStyle theFontStyle = Gdiplus::FontStyleRegular);
    void DrawSimpleText(Gdiplus::Graphics &theGdiPlusGraphics, const NFmiColor &theColor, float theFontSizeInPixels, const std::string &theStr, const NFmiPoint &theAbsPlace, const std::wstring &theFontNameStr, FmiDirection theAlingment, Gdiplus::FontStyle theFontStyle = Gdiplus::FontStyleRegular, const NFmiColor *theBkColor = nullptr);
    void DrawLine(Gdiplus::Graphics &theGdiPlusGraphics, int x1, int y1, int x2, int y2, const NFmiColor &theColor, float thePenWidthInPixels, Gdiplus::DashStyle theDashStyle = Gdiplus::DashStyleSolid);
    void DrawRect(Gdiplus::Graphics &theGdiPlusGraphics, const Gdiplus::Rect &theRectInPixels, const NFmiColor &theRectFrameColor, const NFmiColor &theRectFillColor, bool doFill, bool doFrame, float theRectFrameWidthInPixels, Gdiplus::DashStyle theDashStyle = Gdiplus::DashStyleSolid);
    void DrawPath(Gdiplus::Graphics &theGdiPlusGraphics, const Gdiplus::GraphicsPath &thePathInPixels, const NFmiColor &theFrameColor, const NFmiColor &theFillColor, bool doFill, bool doFrame, float theFrameWidthInPixels);

    void DrawAnimationButton(const NFmiRect &theRect, Gdiplus::Bitmap *theButtonImage, Gdiplus::Graphics *theGdiPlusGraphics, NFmiToolBox &theToolBox, bool isPrinting, const NFmiPoint &theViewSizeInPixels, Gdiplus::REAL theAlpha, bool fForceSize = false, bool fDoNearestInterpolation = true);
    void DrawImageButton(const CRect &theAbsRect, Gdiplus::Bitmap *theButtonImage, Gdiplus::Graphics *theGdiPlusGraphics, Gdiplus::REAL theAlpha, bool isPrinting);
    void DrawBitmap(Gdiplus::Bitmap &theBitmap, const NFmiRect &theSourcePixels, const Gdiplus::RectF &theDestPixels, Gdiplus::REAL theAlpha, Gdiplus::Graphics *theGdiPlusGraphics, bool isPrinting, bool fDoNearestInterpolation);
}
#else  // UNIX - stub implementations for compilation; real UI will use Qt
#include "linux_compat.h"
#include "gdiplus_stub.h"
#include "NFmiColor.h"
#include "NFmiRect.h"
#include "NFmiPoint.h"
#include "NFmiToolBox.h"
#include <string>
#include <vector>
#include <list>
#include <boost/shared_ptr.hpp>

#ifdef HAVE_QT6
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QColor>
#endif

class NFmiCtrlView;
class GdiPlusLineInfo;
class NFmiDrawParam;

class NFmiImageAttributes
{
public:
    NFmiImageAttributes() = default;
    NFmiImageAttributes(Gdiplus::REAL) {}
    NFmiImageAttributes(COLORREF) {}
};

namespace CtrlView
{
    // Color conversions -- NFmiColor uses 0..1 float channels
    inline Gdiplus::Color NFmiColor2GdiplusColor(const NFmiColor& c)
    {
        // NFmiColor: alpha 0 = opaque, 1 = fully transparent (inverted from typical convention)
        unsigned char r = static_cast<unsigned char>(c.Red() * 255.f);
        unsigned char g = static_cast<unsigned char>(c.Green() * 255.f);
        unsigned char b = static_cast<unsigned char>(c.Blue() * 255.f);
        unsigned char a = static_cast<unsigned char>((1.f - c.Alpha()) * 255.f);
        return Gdiplus::Color(a, r, g, b);
    }
    inline double CalcBrightningFactor(int, int, int) { return 0; }
    inline NFmiColor ColorRef2Color(COLORREF color)
    {
        float r = static_cast<float>(color & 0xFF) / 255.f;
        float g = static_cast<float>((color >> 8) & 0xFF) / 255.f;
        float b = static_cast<float>((color >> 16) & 0xFF) / 255.f;
        return NFmiColor(r, g, b);
    }
    inline COLORREF Color2ColorRef(const NFmiColor& c)
    {
        return RGB(static_cast<unsigned char>(c.Red() * 255.f),
                   static_cast<unsigned char>(c.Green() * 255.f),
                   static_cast<unsigned char>(c.Blue() * 255.f));
    }

    // String conversions
    inline std::wstring StringToWString(const std::string& s)
    {
        return std::wstring(s.begin(), s.end());
    }
    inline std::string WStringToString(const std::wstring& s)
    {
        return std::string(s.begin(), s.end());
    }

    // Relative (toolbox 0..1) to pixel coordinate transformations using real HX/HY
    inline const Gdiplus::Rect Relative2GdiplusRect(NFmiToolBox* theToolBox, const NFmiRect& theRelativeRect)
    {
        if(!theToolBox) return {};
        int x = static_cast<int>(theToolBox->HX(theRelativeRect.Left()));
        int y = static_cast<int>(theToolBox->HY(theRelativeRect.Top()));
        int w = static_cast<int>(theToolBox->HX(theRelativeRect.Right())) - x;
        int h = static_cast<int>(theToolBox->HY(theRelativeRect.Bottom())) - y;
        return Gdiplus::Rect(x, y, w, h);
    }
    inline const Gdiplus::RectF Relative2GdiplusRectF(NFmiToolBox* theToolBox, const NFmiRect& theRelativeRect)
    {
        if(!theToolBox) return {};
        float x = static_cast<float>(theToolBox->HXs(theRelativeRect.Left()));
        float y = static_cast<float>(theToolBox->HYs(theRelativeRect.Top()));
        float w = static_cast<float>(theToolBox->HXs(theRelativeRect.Right())) - x;
        float h = static_cast<float>(theToolBox->HYs(theRelativeRect.Bottom())) - y;
        return Gdiplus::RectF(x, y, w, h);
    }
    inline const Gdiplus::PointF Relative2GdiplusPoint(NFmiToolBox* theToolBox, const NFmiPoint& theRelativePoint)
    {
        if(!theToolBox) return {};
        return Gdiplus::PointF(
            static_cast<float>(theToolBox->HXs(theRelativePoint.X())),
            static_cast<float>(theToolBox->HYs(theRelativePoint.Y())));
    }
    inline const Gdiplus::Point Relative2GdiplusPointInt(NFmiToolBox* theToolBox, const NFmiPoint& theRelativePoint)
    {
        if(!theToolBox) return {};
        return Gdiplus::Point(
            static_cast<int>(theToolBox->HX(theRelativePoint.X())),
            static_cast<int>(theToolBox->HY(theRelativePoint.Y())));
    }
    inline void SetGdiplusAlignment(FmiDirection, Gdiplus::StringFormat&) {}
    inline NFmiPoint RelativeSizeToPixelSize(const NFmiPoint& relativeSize, NFmiToolBox& theToolBox)
    {
        return NFmiPoint(theToolBox.HXs(relativeSize.X()), theToolBox.HYs(relativeSize.Y()));
    }

#ifdef HAVE_QT6
    // --- Qt6-backed drawing implementations ---

    inline Qt::PenStyle DashStyleToQtPenStyle(Gdiplus::DashStyle ds)
    {
        switch(ds) {
            case Gdiplus::DashStyleDash:       return Qt::DashLine;
            case Gdiplus::DashStyleDot:        return Qt::DotLine;
            case Gdiplus::DashStyleDashDot:    return Qt::DashDotLine;
            case Gdiplus::DashStyleDashDotDot: return Qt::DashDotDotLine;
            default:                           return Qt::SolidLine;
        }
    }

    inline void DrawLine(Gdiplus::Graphics& theGdiPlusGraphics, int x1, int y1, int x2, int y2,
                         const NFmiColor& theColor, float thePenWidthInPixels,
                         Gdiplus::DashStyle theDashStyle = Gdiplus::DashStyleSolid)
    {
        QPainter* p = theGdiPlusGraphics.painter();
        if(!p) return;
        Gdiplus::Color gc = NFmiColor2GdiplusColor(theColor);
        QPen pen(gc.toQColor(), static_cast<qreal>(thePenWidthInPixels));
        pen.setStyle(DashStyleToQtPenStyle(theDashStyle));
        p->setPen(pen);
        p->drawLine(x1, y1, x2, y2);
    }

    inline void DrawRect(Gdiplus::Graphics& theGdiPlusGraphics, const Gdiplus::Rect& theRectInPixels,
                         const NFmiColor& theRectFrameColor, const NFmiColor& theRectFillColor,
                         bool doFill, bool doFrame, float theRectFrameWidthInPixels,
                         Gdiplus::DashStyle theDashStyle = Gdiplus::DashStyleSolid)
    {
        QPainter* p = theGdiPlusGraphics.painter();
        if(!p) return;
        QRect qr(theRectInPixels.X, theRectInPixels.Y, theRectInPixels.Width, theRectInPixels.Height);
        if(doFill)
        {
            Gdiplus::Color fc = NFmiColor2GdiplusColor(theRectFillColor);
            p->fillRect(qr, fc.toQColor());
        }
        if(doFrame)
        {
            Gdiplus::Color lc = NFmiColor2GdiplusColor(theRectFrameColor);
            QPen pen(lc.toQColor(), static_cast<qreal>(theRectFrameWidthInPixels));
            pen.setStyle(DashStyleToQtPenStyle(theDashStyle));
            p->setPen(pen);
            p->setBrush(Qt::NoBrush);
            p->drawRect(qr);
        }
    }

    inline void DrawSimpleText(Gdiplus::Graphics& theGdiPlusGraphics, const NFmiColor& theColor,
                               float theFontSizeInPixels, const std::string& theStr,
                               const NFmiPoint& theAbsPlace, const std::wstring& theFontNameStr,
                               FmiDirection theAlignment,
                               Gdiplus::FontStyle theFontStyle = Gdiplus::FontStyleRegular,
                               const NFmiColor* theBkColor = nullptr)
    {
        QPainter* p = theGdiPlusGraphics.painter();
        if(!p) return;
        // Set up font
        QString family = QString::fromStdWString(theFontNameStr);
        QFont font(family, static_cast<int>(theFontSizeInPixels));
        font.setPixelSize(static_cast<int>(theFontSizeInPixels));
        font.setBold(theFontStyle & Gdiplus::FontStyleBold);
        font.setItalic(theFontStyle & Gdiplus::FontStyleItalic);
        font.setUnderline(theFontStyle & Gdiplus::FontStyleUnderline);
        font.setStrikeOut(theFontStyle & Gdiplus::FontStyleStrikeout);
        p->setFont(font);

        // Background color
        if(theBkColor)
        {
            Gdiplus::Color bc = NFmiColor2GdiplusColor(*theBkColor);
            p->setBackgroundMode(Qt::OpaqueMode);
            p->setBackground(QBrush(bc.toQColor()));
        }
        else
        {
            p->setBackgroundMode(Qt::TransparentMode);
        }

        // Text color
        Gdiplus::Color tc = NFmiColor2GdiplusColor(theColor);
        p->setPen(QPen(tc.toQColor()));

        // Determine alignment flags
        int flags = 0;
        switch(theAlignment)
        {
            case kCenter:
                flags = Qt::AlignCenter; break;
            case kRight:
            case kTopRight:
            case kUpRight:
                flags = Qt::AlignRight | Qt::AlignVCenter; break;
            default:
                flags = Qt::AlignLeft | Qt::AlignVCenter; break;
        }

        QString text = QString::fromStdString(theStr);
        QPointF pos(theAbsPlace.X(), theAbsPlace.Y());
        // Simple drawText at the given pixel position
        p->drawText(pos, text);
    }

#else
    // --- No-op stub implementations (no Qt6) ---
    inline void DrawLine(Gdiplus::Graphics&, int, int, int, int, const NFmiColor&, float, Gdiplus::DashStyle = Gdiplus::DashStyleSolid) {}
    inline void DrawRect(Gdiplus::Graphics&, const Gdiplus::Rect&, const NFmiColor&, const NFmiColor&, bool, bool, float, Gdiplus::DashStyle = Gdiplus::DashStyleSolid) {}
    inline void DrawSimpleText(Gdiplus::Graphics&, const NFmiColor&, float, const std::string&, const NFmiPoint&, const std::wstring&, FmiDirection, Gdiplus::FontStyle = Gdiplus::FontStyleRegular, const NFmiColor* = nullptr) {}
#endif // HAVE_QT6
}
#endif // UNIX
