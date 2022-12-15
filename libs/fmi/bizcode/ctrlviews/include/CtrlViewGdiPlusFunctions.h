#pragma once

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
    Gdiplus::Color transparentColor_ = Gdiplus::Color(Gdiplus::Color::Black);
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
    // theBrightningFactor on prosentti luku, jolla annettua väriä saadaan vaalennettua tai tummennettua.
    // jos prosentti luku on > 0, vaalenee väri, jos se on < 0, tummenee väri.
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
    size_t DrawGdiplusSimpleMultiPolyLineInPixelCoordinates(Gdiplus::Graphics &theGraphics, NFmiToolBox *theToolBox, const std::list<std::vector<Gdiplus::PointF>> &theMultiPolyLineInPixelCoordinates, const NFmiColor &theLineColor, int theLineThickness);
    std::unique_ptr<Gdiplus::Font> CreateFontPtr(float theFontSizeInPixels, const std::wstring& theFontNameStr, Gdiplus::FontStyle theFontStyle);
    std::unique_ptr<Gdiplus::Font> CreateFontPtr(double theFontSizeInMM, double pixelsPerMM, const std::wstring& theFontNameStr, Gdiplus::FontStyle theFontStyle);
    void DrawTextToRelativeLocation(Gdiplus::Graphics& theGdiPlusGraphics, const NFmiColor& theColor, double theFontSizeInMM, const std::string& theStr, const NFmiPoint& thePlace, double pixelsPerMM, NFmiToolBox* theToolbox, const std::wstring& theFontNameStr, FmiDirection theAlingment, Gdiplus::FontStyle theFontStyle = Gdiplus::FontStyleRegular);
    void DrawTextToRelativeLocation(Gdiplus::Graphics &theGdiPlusGraphics, const NFmiColor &theColor, double theFontSizeInMM, const std::wstring &theStr, const NFmiPoint &thePlace, double pixelsPerMM, NFmiToolBox *theToolbox, const std::wstring &theFontNameStr, FmiDirection theAlingment, Gdiplus::FontStyle theFontStyle = Gdiplus::FontStyleRegular);
    void DrawSimpleText(Gdiplus::Graphics &theGdiPlusGraphics, const NFmiColor &theColor, float theFontSizeInPixels, const std::string &theStr, const NFmiPoint &theAbsPlace, const std::wstring &theFontNameStr, FmiDirection theAlingment, Gdiplus::FontStyle theFontStyle = Gdiplus::FontStyleRegular, const NFmiColor *theBkColor = nullptr);
    void DrawLine(Gdiplus::Graphics &theGdiPlusGraphics, int x1, int y1, int x2, int y2, const NFmiColor &theColor, float thePenWidthInPixels, Gdiplus::DashStyle theDashStyle = Gdiplus::DashStyleSolid);
    void DrawRect(Gdiplus::Graphics &theGdiPlusGraphics, const Gdiplus::Rect &theRectInPixels, const NFmiColor &theRectFrameColor, const NFmiColor &theRectFillColor, bool doFill, bool doFrame, float theRectFrameWidthInPixels, Gdiplus::DashStyle theDashStyle = Gdiplus::DashStyleSolid);
    void DrawPath(Gdiplus::Graphics &theGdiPlusGraphics, const Gdiplus::GraphicsPath &thePathInPixels, const NFmiColor &theFrameColor, const NFmiColor &theFillColor, bool doFill, bool doFrame, float theFrameWidthInPixels);

    void DrawAnimationButton(const NFmiRect &theRect, Gdiplus::Bitmap *theButtonImage, Gdiplus::Graphics *theGdiPlusGraphics, NFmiToolBox &theToolBox, bool isPrinting, const NFmiPoint &theViewSizeInPixels, Gdiplus::REAL theAlpha, bool fForceSize = false, bool fDoNearestInterpolation = true);
    void DrawImageButton(const CRect &theAbsRect, Gdiplus::Bitmap *theButtonImage, Gdiplus::Graphics *theGdiPlusGraphics, Gdiplus::REAL theAlpha, bool isPrinting);
    void DrawBitmap(Gdiplus::Bitmap &theBitmap, const NFmiRect &theSourcePixels, const Gdiplus::RectF &theDestPixels, Gdiplus::REAL theAlpha, Gdiplus::Graphics *theGdiPlusGraphics, bool isPrinting, bool fDoNearestInterpolation);
}