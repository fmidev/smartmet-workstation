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
// Tehd��n unix dummy toteutu, ett� saadaan k��nnetty� ja linkattua helper ja muut
// helperist� riippuvat kirjastot.



#include "NFmiGlobals.h"
#include "NFmiRect.h"
#include "NFmiDrawingEnvironment.h"
#include <cmath>
#include <list>

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

  bool Convert(NFmiDrawingItem * fmiItem) { return true; }
  bool ConvertShape(NFmiShape * fmiShape) { return true; }

  void  ConvertEnvironment(const NFmiDrawingEnvironment * fmiEnvironment){}
  COLORREF ConvertColor(const FmiRGBColor & fromFmiColor){return COLORREF();}
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

  void DrawEllipse(const NFmiRect & theRelativeSize, NFmiDrawingEnvironment * theEnvi){}
  bool DrawMultiPolygon(std::list<NFmiPolyline*> &thePolyLineList, NFmiDrawingEnvironment * theEnvi, const NFmiPoint &theOffSet){ return true; }
  void DoSymbolBulkDraw(const NFmiSymbolBulkDrawData&, bool) {}
  bool DrawValueLineList(NFmiValueLineList * theLineList, NFmiDrawingEnvironment * theEnvi, const NFmiRect & theRelativeSize){ return true; }
  static std::pair<float, float> GetWsAndWdFromWindVector(float windVector) { return {0.f, 0.f}; }


protected:
  void SetFont(const NFmiDrawingEnvironment * theEnvironment){}
  LPCTSTR ConvertFont(FmiFontType theFont){return LPCTSTR();}
  bool BuildLine(NFmiLine * fmiShape){ return true; }
  bool BuildBitmap(const NFmiBitmap * fmiShape){ return true; }
  bool BuildPolyline(const NFmiPolyline * fmiShape){ return true; }
  bool BuildRectangle(const NFmiRectangle * fmiShape){ return true; }
  bool BuildInvertRectangle(const NFmiRectangle * fmiShape){ return true; }
  bool BuildText(const NFmiText * fmiShape){ return true; }
  void BuildShapeList(const NFmiVoidPtrList * fmiShapeList){}
  bool BuildDrawingItem(const NFmiDrawingItem * fromFmiDrawingItem){ return true; }
  void SelectEnvironment (NFmiDrawingItem * fromFmiDrawingItem){}
  void DeSelectEnvironment (NFmiDrawingItem * fromFmiDrawingItem){}

private:
  NFmiRect mClientRect_;  // Pixel dimensions (width/height stored as NFmiRect)
  NFmiRect itsRelativeClipRect_;
  bool fUseClipping_ = false;

};

// Stub for TurnClippingOffHelper on Linux (no-op since no GDI clipping)
class TurnClippingOffHelper
{
public:
    TurnClippingOffHelper(NFmiToolBox*) {}
    ~TurnClippingOffHelper() {}
};

#endif // WIN32

