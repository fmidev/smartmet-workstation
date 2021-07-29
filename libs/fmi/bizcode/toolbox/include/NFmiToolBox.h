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

// Yksinkertainen pisteen k‰‰ntˆ origon suhteen funktio.
// Kulma annetaan asteina. 0 astetta on pohjoisessa ja kulma kiert‰‰ myˆt‰p‰iv‰‰n.
inline const NFmiPoint RotatePoint(const NFmiPoint &thePoint, double alfa)
{
	double alfaInRadians = alfa * 2*kPii/360.;
	double X = thePoint.X() * ::cos(alfaInRadians) - thePoint.Y() * ::sin(alfaInRadians);
	double Y = thePoint.X() * ::sin(alfaInRadians) + thePoint.Y() * ::cos(alfaInRadians);
	return NFmiPoint(X, Y);
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
  void ConvertPointList(const std::vector<NFmiPoint>& fmiPoints, std::vector<CPoint> &theMFCPoints, CPoint &MFCOffSet, double xScale, double yScale, double rotationAlfa);

  // muutos pikseli-maailmasta toolboxin suhteelliseen maailmaan
  NFmiPoint ToViewPoint(long xPix, long yPix);
  // muutos toolboxin suhteellisesta maailmasta n‰ytˆn pikseli-maailmaan
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
  // Koska asetus metodi on nimetty GetPrintInfo-nimiseksi, laitoin t‰m‰n itse palautus metodin nimeksi t‰ll‰isen.
  CPrintInfo* ReturnPrintInfo(void){ return pItsPrintInfo; }

  double MeasureText(const NFmiString & theText);

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
  void SetClientRect(const CRect &theClientRect){mClientRect = theClientRect;} // ‰l‰ k‰yt‰ t‰t‰ jos et tied‰ mit‰ teet!
  const CRect& GetClientRect(void){return mClientRect;}

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

  void SetUpClipping();
  void EndClipping(void);
  void FillPolyPolygonPoints(std::list<NFmiPolyline*> &thePolyLineList, std::vector<CPoint> &thePoints, std::vector<int> &thePolygonPointCounts, int &thePolygonCount, const NFmiPoint &theOffSet);

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
  CRgn itsClipRegion; // k‰ytet‰‰n clippauksen yhteydess‰

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

//---------------------------------------------------------------------ConvertPoint
inline
void NFmiToolBox::ConvertPoint(const NFmiPoint &relativePoint, CPoint &absolutePoint)
{
  absolutePoint.y = itsYDirection * int(relativePoint.Y() * mClientRect.Height());
  absolutePoint.x = itsXDirection * int(relativePoint.X() * mClientRect.Width());
}
//---------------------------------------------------------------------ConvertPoint
inline
void NFmiToolBox::ConvertPoint(const NFmiRect &theFrame, const NFmiPoint &relativePoint, CPoint &absolutePoint)
{
  absolutePoint.y = itsYDirection * int(relativePoint.Y() * theFrame.Height());
  absolutePoint.x = itsXDirection * int(relativePoint.X() * theFrame.Width());
}
//---------------------------------------------------------------------ConvertRect
inline
void NFmiToolBox::ConvertRect(const NFmiRect &relativeRect, CRect &absoluteRect)
{
  absoluteRect.top = itsYDirection * int(relativeRect.Top() * mClientRect.Height());
  absoluteRect.left = itsXDirection * int(relativeRect.Left() * mClientRect.Width());
  absoluteRect.bottom = itsYDirection * int(relativeRect.Bottom() * mClientRect.Height());
  absoluteRect.right = itsXDirection * int(relativeRect.Right() * mClientRect.Width());
}

//---------------------------------------------------------------------ToViewPoint
// muutos pikseli-maailmasta toolboxin suhteelliseen maailmaan
inline NFmiPoint NFmiToolBox::ToViewPoint(long xPix, long yPix)
{
  return NFmiPoint(static_cast<float>(xPix) / mClientRect.Width(), static_cast<float>(yPix) / mClientRect.Height());
}

// muutos toolboxin suhteellisesta maailmasta n‰ytˆn pikseli-maailmaan
inline NFmiPoint NFmiToolBox::ToScreenPoint(double x, double y)
{
  return NFmiPoint(static_cast<long>(x * mClientRect.Width()), static_cast<long>(y * mClientRect.Height()));
}

#else // (defined) UNIX
// Tehd‰‰n unix dummy toteutu, ett‰ saadaan k‰‰nnetty‰ ja linkattua helper ja muut
// helperist‰ riippuvat kirjastot.



#include "NFmiGlobals.h"
#include "NFmiRect.h"
#include "NFmiDrawingEnvironment.h"

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

// unix dummy esittelyt joillekin Windows jutuille
typedef unsigned int UINT;
typedef int COLORREF;
typedef const char * LPCSTR;
typedef const char * PCSTR;
typedef LPCSTR PCTSTR, LPCTSTR, PCUTSTR, LPCUTSTR;
typedef NFmiRect CRect;
class CView;
class CPoint;
class CPrintInfo;
class CBitmap;
class CDC;

class NFmiToolBox
{

public:

  NFmiToolBox (CView * theEnclosure, bool isScrollView = false){};
  NFmiToolBox(const CRect & clientRect){};
  ~NFmiToolBox(void){};

  void Draw(NFmiMetaFileView * theView){};

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

  NFmiPoint ToViewPoint(long xPix, long yPix){ return NFmiPoint(); }

  NFmiRect ToFmiRect(const CRect & MacRect){return NFmiRect();}
  double WidthPerHeight(void) const { return 0; }

  NFmiRect BuildFrame(const NFmiView * viewToFrame,
					  const NFmiColor & theFrameColor,
					  const NFmiColor & theFillColor,
					  NFmiVoidPtrList & theDrawingList){ return NFmiRect(); }

  CDC * GetDC (void) const{ return 0;}
  void SetDC(CDC * pmyDC){}
  void GetPrintInfo(CPrintInfo * pPrintInfo){}

  double MeasureText(const NFmiString & theText){ return 0;}

  long HX(double sX) const { return 0L; }
  long HY(double sY) const { return 0L; }
  double SX(long hX) const { return 0; }
  double SY(long hY) const { return 0; }

  void SetTextAlignment(FmiDirection theAlignment){}
  FmiDirection GetTextAlignment(void);

  bool SetXDirection(FmiDirection newDirection){ return true; }
  bool SetYDirection(FmiDirection newDirection){ return true; }
  CRect GetDravingRect(){ return CRect();}
  void DrawBitmap(CBitmap * theBitmap, const NFmiRect & theDrawedBitmapPortion,const NFmiRect & theRelativeSize){}
  void DrawDC(CDC * theDC, const NFmiRect & theRelativeSize){}
  void DrawDC(CDC * theDC, const NFmiRect & theRelativeSize, const NFmiRect & theRelativeSourceStart){}

  void DrawEllipse(const NFmiRect & theRelativeSize, NFmiDrawingEnvironment * theEnvi){}
  bool DrawValueLineList(NFmiValueLineList * theLineList, NFmiDrawingEnvironment * theEnvi, const NFmiRect & theRelativeSize){ return true; }


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
/*
  CView * itsEnclosure;
  NFmiDrawingEnvironment * itsBaseEnvironment;
  bool fFilled;
  bool fFramed;
  bool fInvert; // draws with inverted colors (works only with NFmiRectangle)
  COLORREF itsFrameColor;
  COLORREF itsFillColor;
  CBrush * pItsFillPattern;
  CSize itsPenSize;
  CRect mClientRect;
  CDC * pDC;
  CPen * pItsPen;
  CPrintInfo * pItsPrintInfo;
  short itsXDirection;
  short itsYDirection;
  bool fIsScrollView;

  CBitmap itsMemBitmap;
  CDC itsdcMem;
*/

};



#endif // WIN32

