// ======================================================================
/*!
 * \file NFmiToolBox.h
 * \brief Interface of class NFmiToolBox
 */
// ======================================================================

#pragma once

#ifdef _MSC_VER

#include "stdafx.h"

#include "NFmiGlobals.h"
#include "NFmiRect.h"
#include "NFmiDrawingEnvironment.h"

#include <list>
#include <vector>

enum FmiCtrlKeys
{
  kCtrlKey = 1,
  kAltKey = 2,
  kShiftKey = 4,
  kEscKey = 8,
  kCapsLockKey = 16,
  kSpaceKey = 32,
  AltGrKey = 64
};

class NFmiMetaFileView;
class NFmiValueLineList;
class NFmiText;
class NFmiPolyline;
class NFmiRectangle;
class NFmiBitmap;
class NFmiLine;
class NFmiShape;
class NFmiDrawingItem;
class NFmiDrawingEnvironment;
class NFmiVoidPtrList;
class NFmiView;
class NFmiColor;
class NFmiString;
class NFmiSymbolBulkDrawData;

// Yksinkertainen pisteen k��nt� origon suhteen funktio.
// Kulma annetaan asteina. 0 astetta on pohjoisessa ja kulma kiert�� my�t�p�iv��n.
inline const NFmiPoint RotatePoint(const NFmiPoint &thePoint, double alfa)
{
	double alfaInRadians = alfa * 2*kPii/360.;
	double X = thePoint.X() * ::cos(alfaInRadians) - thePoint.Y() * ::sin(alfaInRadians);
	double Y = thePoint.X() * ::sin(alfaInRadians) + thePoint.Y() * ::cos(alfaInRadians);
	return NFmiPoint(X, Y);
}

inline void RotatePoint(CPoint& mfcPoint, double alfa)
{
	const double alfaInRadians = alfa * 2 * kPii / 360.;
	auto X = mfcPoint.x * ::cos(alfaInRadians) - mfcPoint.y * ::sin(alfaInRadians);
	auto Y = mfcPoint.x * ::sin(alfaInRadians) + mfcPoint.y * ::cos(alfaInRadians);
	mfcPoint = CPoint(boost::math::iround(X), boost::math::iround(Y));
}

class NFmiToolBox
{

public:

  NFmiToolBox (CView * theEnclosure, bool isScrollView = false);
  NFmiToolBox(const CRect & clientRect);
  ~NFmiToolBox(void);

  void Draw(NFmiMetaFileView * theView);

  unsigned long ConvertCtrlKey (UINT theHardCtrl) const;
  void ConvertPoint(const NFmiPoint& relativePoint, CPoint& absolutePoint);
  void ConvertPoint(const CPoint & absolutePoint, NFmiPoint & relativePoint);
  void ConvertPoint(const NFmiRect & theFrame, const NFmiPoint & relativePoint, CPoint & absolutePoint);
  void ConvertRect(const NFmiRect & relativeRect, CRect & absoluteRect);
  void ConvertRect(const CRect & absoluteRect, NFmiRect & relativeRect);
  void UpdateClientRect(void)
  { itsEnclosure->GetClientRect(mClientRect); }

  bool Convert(const NFmiDrawingItem * fmiItem);
  bool ConvertShape(const NFmiShape * fmiShape);

  void  ConvertEnvironment(const NFmiDrawingEnvironment * fmiEnvironment);
  COLORREF ConvertColor(const FmiRGBColor & fromFmiColor);
  void ConvertPointList(const std::vector<NFmiPoint>& fmiPoints, std::vector<CPoint> &theMFCPoints);
  void ConvertPointList(const std::vector<NFmiPoint>& fmiPoints, std::vector<CPoint> &theMFCPoints, const CPoint &MFCOffSet, double xScale, double yScale, double rotationAlfa);

  // muutos pikseli-maailmasta toolboxin suhteelliseen maailmaan
  NFmiPoint ToViewPoint(long xPix, long yPix);
  // muutos toolboxin suhteellisesta maailmasta n�yt�n pikseli-maailmaan
  NFmiPoint ToScreenPoint(double x, double y);

  NFmiRect ToFmiRect(const CRect & MacRect);
  double WidthPerHeight(void) const
  {
	CRect rect;
	itsEnclosure->GetClientRect(rect);
	return (double)rect.Width() / (double)rect.Height();
  }

  NFmiRect BuildFrame(const NFmiView * viewToFrame,
					  const NFmiColor & theFrameColor,
					  const NFmiColor & theFillColor,
					  NFmiVoidPtrList & theDrawingList);

  CDC * GetDC (void) const
  { return pDC; }
  void SetDC(CDC * pmyDC, bool fReCalcClientRect = true);
  void GetPrintInfo(CPrintInfo * pPrintInfo);
  // Koska asetus metodi on nimetty GetPrintInfo-nimiseksi, laitoin t�m�n itse palautus metodin nimeksi t�ll�isen.
  CPrintInfo* ReturnPrintInfo(void){ return pItsPrintInfo; }

  double MeasureText(const NFmiString& theText);
  NFmiPoint MeasureTextCorrect(const NFmiText & theText);
  void SetupFont(CFont& theFont, NFmiDrawingEnvironment* fmiEnvironment);

  long HX(double sX) const { return (long)(sX * mClientRect.Width()); }
  long HY(double sY) const { return (long)(sY * mClientRect.Height()); }
  double HXs(double sX) const { return sX * mClientRect.Width(); }
  double HYs(double sY) const { return sY * mClientRect.Height(); }
  double SX(long hX) const { return mClientRect.Width() ? (double)hX / mClientRect.Width() : 0; } // laitoin 0-jako eston \Marko
  double SY(long hY) const { return mClientRect.Height() ? (double)hY / mClientRect.Height() : 0; } // laitoin 0-jako eston \Marko
  double SXs(double hX) const { return mClientRect.Width() ? hX / mClientRect.Width() : 0; } // laitoin 0-jako eston \Marko
  double SYs(double hY) const { return mClientRect.Height() ? hY / mClientRect.Height() : 0; } // laitoin 0-jako eston \Marko

  void SetTextAlignment(FmiDirection theAlignment);
  FmiDirection GetTextAlignment(void);

  bool SetXDirection(FmiDirection newDirection);
  bool SetYDirection(FmiDirection newDirection);
  int GetHatchPattern(void) const {return itsHatchPattern;}
  void SetHatchPattern(int newValue) {itsHatchPattern = newValue;}
  CRect GetDravingRect();
  void DrawBitmap(CBitmap * theBitmap, const NFmiRect & theDrawedBitmapPortion,const NFmiRect & theRelativeSize);
  void DrawDC(CDC * theDC, const NFmiRect & theRelativeSize);
  void DrawDC(CDC * theDC, const NFmiRect & theRelativeSize, const NFmiRect & theRelativeSourceStart);

  void DrawEllipse(const NFmiRect & theRelativeSize, NFmiDrawingEnvironment * theEnvi);
  bool DrawValueLineList(NFmiValueLineList * theLineList, NFmiDrawingEnvironment * theEnvi, const NFmiRect & theRelativeSize);
  bool DrawMultiPolygon(std::list<NFmiPolyline*> &thePolyLineList, NFmiDrawingEnvironment * theEnvi, const NFmiPoint &theOffSet);
  bool DrawPolyline(NFmiPolyline * fmiShape, const NFmiPoint &theOffSet, const NFmiPoint &theScale, double rotationAlfa = 0);

  const NFmiRect& RelativeClipRect(void) const {return itsRelativeClipRect;}
  void RelativeClipRect(const NFmiRect& theRect, bool newState = true) {itsRelativeClipRect=theRect; fUseClipping = newState;}
  bool UseClipping(void) const {return fUseClipping;}
  void UseClipping(bool newState) {fUseClipping = newState;}
  void SetUpClipping();
  void EndClipping(void);
  void SetClientRect(const CRect &theClientRect){mClientRect = theClientRect;} // �l� k�yt� t�t� jos et tied� mit� teet!
  const CRect& GetClientRect(void){return mClientRect;}
  void DoSymbolBulkDraw(const NFmiSymbolBulkDrawData &sbdData, bool doStationPlotOnly);
  static std::pair<float, float> GetWsAndWdFromWindVector(float windVector);

protected:
  void SetFont(const NFmiDrawingEnvironment * theEnvironment);
  LPCTSTR ConvertFont(FmiFontType theFont);
  bool BuildLine(const NFmiLine * fmiShape);
  bool BuildBitmap(const NFmiBitmap * fmiShape);
  bool BuildPolyline(const NFmiPolyline * fmiShape);
  bool BuildRectangle(const NFmiRectangle * fmiShape);
  bool BuildInvertRectangle(const NFmiRectangle * fmiShape);
  bool BuildText(const NFmiText * fmiShape);
  void BuildShapeList(const NFmiVoidPtrList * fmiShapeList);
  bool BuildDrawingItem(const NFmiDrawingItem * fromFmiDrawingItem);
  void SelectEnvironment (const NFmiDrawingItem * fromFmiDrawingItem);
  void DeSelectEnvironment (const NFmiDrawingItem * fromFmiDrawingItem);

  void FillPolyPolygonPoints(std::list<NFmiPolyline*> &thePolyLineList, std::vector<CPoint> &thePoints, std::vector<int> &thePolygonPointCounts, int &thePolygonCount, const NFmiPoint &theOffSet);
  void DoTextBulkDraw(const NFmiSymbolBulkDrawData& sbdData);
  void DoArroyBulkDraw(const NFmiSymbolBulkDrawData& sbdData);
  void DoBasicArroyBulkDraw(const NFmiSymbolBulkDrawData& sbdData, const std::vector<CPoint>& mfcArrowPositions);
  void DoChangingColorArroyBulkDraw(const NFmiSymbolBulkDrawData& sbdData, const std::vector<CPoint>& mfcArrowPositions);
  void DoWindBarbBulkDraw(const NFmiSymbolBulkDrawData& sbdData);
  void DoChangingColorWindBarbBulkDraw(const NFmiSymbolBulkDrawData& sbdData);
  void DoSimpleWeatherBulkkDraw(const NFmiSymbolBulkDrawData& sbdData);
  void DoStationPlotkDraw(const NFmiSymbolBulkDrawData& sbdData);
  
private:

  CView * itsEnclosure;
  NFmiDrawingEnvironment * itsBaseEnvironment;
  bool fFilled;
  bool fFramed;
  bool fInvert; // draws with inverted colors (works only with NFmiRectangle)
  COLORREF itsFrameColor;
  COLORREF itsFillColor;
  CBrush * pItsFillPattern;
  CSize itsPenSize;
  int itsPenStyle; // 0=solid, 1=dash jne. Ei toimi kuin 1 paksuisille viivoille (GDI puute)
  CRect mClientRect;
  CDC * pDC;
  CPen * pItsPen;
  CPrintInfo * pItsPrintInfo;
  CRgn itsClipRegion; // k�ytet��n clippauksen yhteydess�

  short itsXDirection;
  short itsYDirection;
  bool fIsScrollView;

  CBitmap itsMemBitmap;
  CDC itsdcMem;

  NFmiRect itsRelativeClipRect;
  bool fUseClipping;

  /*
   * Hatch Styles
   * no hatch           -1
   * HS_HORIZONTAL       0       -----
   * HS_VERTICAL         1       |||||
   * HS_FDIAGONAL        2       \\\\\
   * HS_BDIAGONAL        3       /////
   * HS_CROSS            4       +++++
   * HS_DIAGCROSS        5       xxxxx
   */

  int itsHatchPattern;

};

class TurnClippingOffHelper
{
	NFmiToolBox* toolbox_ = nullptr;
	bool oldClippingMode_ = false;
public:
	TurnClippingOffHelper(NFmiToolBox* toolbox);
	~TurnClippingOffHelper();
};

#else // (defined) UNIX
// Linux implementation: when HAVE_QT6 is defined, the Build* methods draw via
// the QPainter wrapped inside a Gdiplus::Graphics object. Without Qt6, they are no-ops.

#include "NFmiGlobals.h"
#include "NFmiRect.h"
#include "NFmiDrawingEnvironment.h"
#include <cmath>
#include <list>
#include <vector>

#ifdef HAVE_QT6
#include "gdiplus_stub.h"          // Gdiplus::Graphics wraps QPainter
#include "NFmiLine.h"
#include "NFmiText.h"
#include "NFmiRectangle.h"
#include "NFmiPolyline.h"
#include "NFmiBitmap.h"
#include "NFmiString.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QColor>
#include <QPolygon>
#endif

inline const NFmiPoint RotatePoint(const NFmiPoint &thePoint, double alfa)
{
    double alfaInRadians = alfa * 2*kPii/360.;
    double X = thePoint.X() * ::cos(alfaInRadians) - thePoint.Y() * ::sin(alfaInRadians);
    double Y = thePoint.X() * ::sin(alfaInRadians) + thePoint.Y() * ::cos(alfaInRadians);
    return NFmiPoint(X, Y);
}

enum FmiCtrlKeys
{
  kCtrlKey = 1,
  kAltKey = 2,
  kShiftKey = 4,
  kEscKey = 8,
  kCapsLockKey = 16,
  kSpaceKey = 32,
  AltGrKey = 64
};

class NFmiMetaFileView;
class NFmiValueLineList;
class NFmiText;
class NFmiPolyline;
class NFmiRectangle;
class NFmiBitmap;
class NFmiLine;
class NFmiShape;
class NFmiDrawingItem;
class NFmiDrawingEnvironment;
class NFmiVoidPtrList;
class NFmiView;
class NFmiColor;
class NFmiString;
class NFmiSymbolBulkDrawData;

// unix dummy esittelyt joillekin Windows jutuille
#include "linux_compat.h"
using CRect = NFmiRect;
class CView;
class CPrintInfo;

class NFmiToolBox
{

public:

  NFmiToolBox (CView * theEnclosure, bool isScrollView = false)
    : mClientRect_(NFmiPoint(0,0), NFmiPoint(800, 600)) {}
  NFmiToolBox(const CRect & clientRect)
    : mClientRect_(clientRect) {}
  NFmiToolBox(int width, int height)
    : mClientRect_(NFmiPoint(0,0), NFmiPoint(width, height)) {}
  ~NFmiToolBox(void){}

  void Draw(NFmiMetaFileView * theView){}

  unsigned long ConvertCtrlKey (UINT theHardCtrl) const{return 0;}
  void ConvertPoint(const NFmiPoint & relativePoint, CPoint & absolutePoint){}
  void ConvertPoint(const NFmiRect & theFrame, const NFmiPoint & relativePoint, CPoint & absolutePoint){}
  void ConvertRect(const NFmiRect & relativeRect, CRect & absoluteRect){}
  void UpdateClientRect(void){}

  // ---- Convert: main drawing dispatch ----
  bool Convert(NFmiDrawingItem * fmiItem)
  {
#ifdef HAVE_QT6
    if(!fmiItem) return false;
    switch(fmiItem->GetIdentifier())
    {
      case kLineShape:      return BuildLine(static_cast<NFmiLine*>(fmiItem));
      case kPolylineShape:  return BuildPolyline(static_cast<const NFmiPolyline*>(fmiItem));
      case kRectangleShape: return BuildRectangle(static_cast<const NFmiRectangle*>(fmiItem));
      case kTextShape:      return BuildText(static_cast<const NFmiText*>(fmiItem));
      case kBitmap:         return BuildBitmap(static_cast<const NFmiBitmap*>(fmiItem));
      default: break;
    }
#endif
    return true;
  }

  bool ConvertShape(NFmiShape * fmiShape) { return Convert(static_cast<NFmiDrawingItem*>(fmiShape)); }

  // ---- ConvertEnvironment: extract pen/brush state from drawing environment ----
  void ConvertEnvironment(const NFmiDrawingEnvironment * fmiEnvironment)
  {
#ifdef HAVE_QT6
    if(!fmiEnvironment) return;
    itsFrameColor_ = ConvertColor(fmiEnvironment->GetFrameColor());
    itsFillColor_  = ConvertColor(fmiEnvironment->GetFillColor());
    fFramed_ = fmiEnvironment->IsFramed();
    fFilled_ = fmiEnvironment->IsFilled();
    fInvert_ = fmiEnvironment->IsInverted();
    itsPenWidth_ = static_cast<int>(fmiEnvironment->GetPenSize().X());
    if(itsPenWidth_ < 1) itsPenWidth_ = 1;
    itsPenStyle_ = static_cast<int>(fmiEnvironment->GetLineStyle());
#endif
  }

  // ---- ConvertColor: FmiRGBColor (0-1 floats) to COLORREF ----
  COLORREF ConvertColor(const FmiRGBColor & c)
  {
    return RGB(static_cast<int>(c.red * 255),
               static_cast<int>(c.green * 255),
               static_cast<int>(c.blue * 255));
  }

  unsigned short ConvertPointList(NFmiVoidPtrList * fmiPointList, CPoint ** MFCPoints){return 0;}

  NFmiPoint ToViewPoint(long xPix, long yPix){ return NFmiPoint(SX(xPix), SY(yPix)); }

  NFmiRect ToFmiRect(const CRect & MacRect){return NFmiRect();}
  double WidthPerHeight(void) const
  {
    return mClientRect_.Height() != 0 ? mClientRect_.Width() / mClientRect_.Height() : 0;
  }

  NFmiRect BuildFrame(const NFmiView * viewToFrame,
					  const NFmiColor & theFrameColor,
					  const NFmiColor & theFillColor,
					  NFmiVoidPtrList & theDrawingList){ return NFmiRect(); }

  CDC * GetDC (void) const{ return 0;}
  void SetDC(CDC * pmyDC){}
  void GetPrintInfo(CPrintInfo * pPrintInfo){}

#ifdef HAVE_QT6
  // Set the Gdiplus::Graphics wrapper (which holds the QPainter*)
  void SetGraphics(Gdiplus::Graphics* g) { graphics_ = g; }
  Gdiplus::Graphics* GetGraphics() const { return graphics_; }
#endif

  double MeasureText(const NFmiString & theText){ return 0;}
  NFmiPoint MeasureTextCorrect(const NFmiText &){ return NFmiPoint(); }
  void DrawPolyline(const NFmiPolyline*, const NFmiPoint&, const NFmiPoint&, double = 0) {}

  // Relative-to-pixel coordinate conversions (match Windows CRect-based version)
  long HX(double sX) const { return static_cast<long>(sX * mClientRect_.Width()); }
  long HY(double sY) const { return static_cast<long>(sY * mClientRect_.Height()); }
  double HXs(double sX) const { return sX * mClientRect_.Width(); }
  double HYs(double sY) const { return sY * mClientRect_.Height(); }
  // Pixel-to-relative coordinate conversions
  double SX(long hX) const { return mClientRect_.Width() ? static_cast<double>(hX) / mClientRect_.Width() : 0; }
  double SY(long hY) const { return mClientRect_.Height() ? static_cast<double>(hY) / mClientRect_.Height() : 0; }
  double SXs(double hX) const { return mClientRect_.Width() ? hX / mClientRect_.Width() : 0; }
  double SYs(double hY) const { return mClientRect_.Height() ? hY / mClientRect_.Height() : 0; }

  void SetClientRect(const CRect &theClientRect){ mClientRect_ = theClientRect; }
  const CRect& GetClientRect() const { return mClientRect_; }

  const NFmiRect& RelativeClipRect(void) const { return itsRelativeClipRect_; }
  void RelativeClipRect(const NFmiRect& theRect, bool newState = true)
  {
    itsRelativeClipRect_ = theRect;
    fUseClipping_ = newState;
  }
  bool UseClipping(void) const { return fUseClipping_; }
  void UseClipping(bool newState) { fUseClipping_ = newState; }
  void SetUpClipping() {}
  void EndClipping(void) {}

  void SetTextAlignment(FmiDirection theAlignment){}
  FmiDirection GetTextAlignment(void) { return kCenter; }

  bool SetXDirection(FmiDirection newDirection){ return true; }
  bool SetYDirection(FmiDirection newDirection){ return true; }
  CRect GetDravingRect(){ return mClientRect_; }
  void DrawBitmap(CBitmap * theBitmap, const NFmiRect & theDrawedBitmapPortion,const NFmiRect & theRelativeSize){}
  void DrawDC(CDC * theDC, const NFmiRect & theRelativeSize){}
  void DrawDC(CDC * theDC, const NFmiRect & theRelativeSize, const NFmiRect & theRelativeSourceStart){}

  // ---- DrawEllipse: draw/fill an ellipse within theRelativeSize ----
  void DrawEllipse(const NFmiRect & theRelativeSize, NFmiDrawingEnvironment * theEnvi)
  {
#ifdef HAVE_QT6
    if(!graphics_ || !graphics_->painter()) return;
    QPainter* p = graphics_->painter();
    ConvertEnvironment(theEnvi);
    int x = HX(theRelativeSize.Left());
    int y = HY(theRelativeSize.Top());
    int w = HX(theRelativeSize.Width());
    int h = HY(theRelativeSize.Height());
    if(fFilled_)
      p->setBrush(QBrush(colorrefToQColor(itsFillColor_)));
    else
      p->setBrush(Qt::NoBrush);
    if(fFramed_)
      p->setPen(QPen(colorrefToQColor(itsFrameColor_), itsPenWidth_));
    else
      p->setPen(Qt::NoPen);
    p->drawEllipse(x, y, w, h);
#endif
  }

  bool DrawMultiPolygon(std::list<NFmiPolyline*> &thePolyLineList, NFmiDrawingEnvironment * theEnvi, const NFmiPoint &theOffSet){ return true; }
  void DoSymbolBulkDraw(const NFmiSymbolBulkDrawData&, bool) {}
  bool DrawValueLineList(NFmiValueLineList * theLineList, NFmiDrawingEnvironment * theEnvi, const NFmiRect & theRelativeSize){ return true; }
  static std::pair<float, float> GetWsAndWdFromWindVector(float windVector) { return {0.f, 0.f}; }


protected:
  void SetFont(const NFmiDrawingEnvironment * theEnvironment){}
  LPCTSTR ConvertFont(FmiFontType theFont){return LPCTSTR();}

  // ---- BuildLine: draw a line from start to end point ----
  bool BuildLine(NFmiLine * fmiShape)
  {
#ifdef HAVE_QT6
    if(!graphics_ || !graphics_->painter() || !fmiShape) return false;
    QPainter* p = graphics_->painter();
    SelectEnvironment(fmiShape);
    NFmiPoint sp = fmiShape->GetStartingPoint();
    NFmiPoint ep = fmiShape->GetEndingPoint();
    int x1 = HX(sp.X()), y1 = HY(sp.Y());
    int x2 = HX(ep.X()), y2 = HY(ep.Y());
    p->setPen(QPen(colorrefToQColor(itsFrameColor_), itsPenWidth_));
    p->drawLine(x1, y1, x2, y2);
    DeSelectEnvironment(fmiShape);
#endif
    return true;
  }

  bool BuildBitmap(const NFmiBitmap * fmiShape){ return true; }

  // ---- BuildPolyline: draw/fill a polyline ----
  bool BuildPolyline(const NFmiPolyline * fmiShape)
  {
#ifdef HAVE_QT6
    if(!graphics_ || !graphics_->painter() || !fmiShape) return false;
    QPainter* p = graphics_->painter();
    SelectEnvironment(const_cast<NFmiDrawingItem*>(static_cast<const NFmiDrawingItem*>(fmiShape)));
    const auto& pts = fmiShape->GetPoints();
    if(pts.size() < 2) { DeSelectEnvironment(const_cast<NFmiDrawingItem*>(static_cast<const NFmiDrawingItem*>(fmiShape))); return true; }
    QPolygon poly(static_cast<int>(pts.size()));
    for(int i = 0; i < static_cast<int>(pts.size()); ++i)
      poly.setPoint(i, HX(pts[i].X()), HY(pts[i].Y()));
    if(fFilled_)
    {
      p->setPen(Qt::NoPen);
      p->setBrush(QBrush(colorrefToQColor(itsFillColor_)));
      p->drawPolygon(poly);
    }
    if(fFramed_)
    {
      p->setBrush(Qt::NoBrush);
      p->setPen(QPen(colorrefToQColor(itsFrameColor_), itsPenWidth_));
      p->drawPolyline(poly);
    }
    DeSelectEnvironment(const_cast<NFmiDrawingItem*>(static_cast<const NFmiDrawingItem*>(fmiShape)));
#endif
    return true;
  }

  // ---- BuildRectangle: draw/fill a rectangle ----
  bool BuildRectangle(const NFmiRectangle * fmiShape)
  {
#ifdef HAVE_QT6
    if(!graphics_ || !graphics_->painter() || !fmiShape) return false;
    QPainter* p = graphics_->painter();
    SelectEnvironment(const_cast<NFmiDrawingItem*>(static_cast<const NFmiDrawingItem*>(fmiShape)));
    const NFmiRect& fr = fmiShape->GetFrame();
    int x = HX(fr.Left()), y = HY(fr.Top());
    int w = HX(fr.Width()), h = HY(fr.Height());
    if(fFilled_)
    {
      p->setPen(Qt::NoPen);
      p->setBrush(QBrush(colorrefToQColor(itsFillColor_)));
      p->drawRect(x, y, w, h);
    }
    if(fFramed_)
    {
      p->setBrush(Qt::NoBrush);
      p->setPen(QPen(colorrefToQColor(itsFrameColor_), itsPenWidth_));
      p->drawRect(x, y, w, h);
    }
    DeSelectEnvironment(const_cast<NFmiDrawingItem*>(static_cast<const NFmiDrawingItem*>(fmiShape)));
#endif
    return true;
  }

  bool BuildInvertRectangle(const NFmiRectangle * fmiShape){ return true; }

  // ---- BuildText: draw text at the shape's top-left position ----
  bool BuildText(const NFmiText * fmiShape)
  {
#ifdef HAVE_QT6
    if(!graphics_ || !graphics_->painter() || !fmiShape) return false;
    QPainter* p = graphics_->painter();
    SelectEnvironment(const_cast<NFmiDrawingItem*>(static_cast<const NFmiDrawingItem*>(fmiShape)));
    NFmiPoint tl = fmiShape->GetFrame().TopLeft();
    int x = HX(tl.X()), y = HY(tl.Y());
    // Set font from environment
    auto* env = fmiShape->GetEnvironment();
    if(env)
    {
      int fh = env->GetFontHeight();
      if(fh < 1) fh = 12;
      QFont qf;
      qf.setPixelSize(fh);
      qf.setBold(env->BoldFont());
      p->setFont(qf);
    }
    p->setPen(QPen(colorrefToQColor(itsFrameColor_)));
    const char* text = fmiShape->GetText();
    if(text)
      p->drawText(x, y, QString::fromUtf8(text));
    DeSelectEnvironment(const_cast<NFmiDrawingItem*>(static_cast<const NFmiDrawingItem*>(fmiShape)));
#endif
    return true;
  }

  void BuildShapeList(const NFmiVoidPtrList * fmiShapeList){}
  bool BuildDrawingItem(const NFmiDrawingItem * fromFmiDrawingItem){ return true; }

  void SelectEnvironment(NFmiDrawingItem * fromFmiDrawingItem)
  {
    if(fromFmiDrawingItem && fromFmiDrawingItem->GetEnvironment())
      ConvertEnvironment(fromFmiDrawingItem->GetEnvironment());
  }
  void DeSelectEnvironment(NFmiDrawingItem * fromFmiDrawingItem){}

private:
  NFmiRect mClientRect_;  // Pixel dimensions (width/height stored as NFmiRect)
  NFmiRect itsRelativeClipRect_;
  bool fUseClipping_ = false;

#ifdef HAVE_QT6
  Gdiplus::Graphics* graphics_ = nullptr;
  // Drawing state extracted from NFmiDrawingEnvironment
  COLORREF itsFrameColor_ = 0;
  COLORREF itsFillColor_ = 0;
  bool fFramed_ = false;
  bool fFilled_ = false;
  bool fInvert_ = false;
  int itsPenWidth_ = 1;
  int itsPenStyle_ = 0;

  // Helper: COLORREF (0x00BBGGRR) to QColor
  static QColor colorrefToQColor(COLORREF c)
  {
    return QColor(static_cast<int>(c & 0xFF),
                  static_cast<int>((c >> 8) & 0xFF),
                  static_cast<int>((c >> 16) & 0xFF));
  }
#endif
};

// Stub for TurnClippingOffHelper on Linux (no-op since no GDI clipping)
class TurnClippingOffHelper
{
public:
    TurnClippingOffHelper(NFmiToolBox*) {}
    ~TurnClippingOffHelper() {}
};

#endif // WIN32

