//© Ilmatieteenlaitos/Persa. 
//27.5.1994   Persa
//030397/LW +FmiFont.type
//130397/LW lis‰‰ fontteja
// 1997.09.02/Marko New fill pattern and its relative value
// 1998.10.06/Marko Inverted colors drawing possibility enabled (works 
//					only with NFmiRectTangle)
// 1999.06.15/Marko Font can be bold now.

#pragma once

#include "NFmiColor.h"
#include "NFmiPoint.h"
#include "NFmiString.h"
#include "NFmiViewingConstants.h"

typedef enum {
   kArcl,
   kArial,
   kBodub,
   kCourier,
   kCourierNew,
   kGillcdb,
   kKlang,
   kLucidaConsole,
   kModern,
   kSansSerif,
   kSerif,
   kNewber,
   kOld,
   kRoman,
   kScrib,
   kScript,
   kSymbols,	    // kSymbol varattu
   kTimesNewRoman,
   kWingDings,
   kSynop,
   kSyntax,			//2.5.01
   kMirri,
   kNoneFont           //9.12.99/Lasse
} FmiFontType;

typedef struct {
   unsigned short height;
   unsigned short width;
   FmiFontType    type;
} FmiFont;


typedef enum {
   FMI_SOLID = 0,
   FMI_DASH = 1, /* -------  */
   FMI_DOT = 2,  /* .......  */
   FMI_DASHDOT = 3, /* _._._._  */
   FMI_DASHDOTDOT = 4, /* _.._.._  */
   FMI_RELATIVE_FILL = 20 // 1997.09.02/Marko Used to draw dashed lines with relative breaks
} FmiPattern;


class NFmiDrawingEnvironment
{

public:
		  NFmiDrawingEnvironment (void);
		  NFmiDrawingEnvironment (const NFmiDrawingEnvironment &anOther);

   const NFmiPoint&         GetPenSize(void) const {return fPenSize;};
   const NFmiPoint&         GetShadowSize(void) const {return fShadowSize;};
   const NFmiPoint&         GetShadowDisplacement(void) const {return itsShadowDisplacement;};
   const NFmiPoint&			GetGradientCenter(void) const  {return fGradientCenter;};
   const FmiRGBColor&       GetForegroundColor(void) const {return fForegroundColor;};
   const FmiRGBColor&       GetBackgroundColor(void) const {return fBackgroundColor;};
   const FmiRGBColor&       GetFillColor(void) const {return fFillColor;};
   const FmiRGBColor&       GetFrameColor(void) const {return fFrameColor;};
   const FmiRGBColor&       GetShadowColor(void) const {return fShadowColor;};
   unsigned short           GetFontHeight(void) const {return fFont.height;};
   unsigned short           GetFontWidth(void) const {return fFont.width;};
   unsigned short			GetNumberOfGradients(void) const  {return fNumberOfGradients;};
   NFmiPoint                GetFontSize(void) const {return NFmiPoint(fFont.width, fFont.height);};
   FmiFontType              GetFontType(void) const {return fFont.type;};
   FmiLineStyle             GetLineStyle(void) const {return itsLineStyle;}; 
   NFmiString               GetFontName(void) const {return itsFontName;}; 

   // 1997.09.02/Marko
   FmiPattern				GetFillPattern(void) const {return fFillPattern;};
   double					GetRelativeFill(void) const {return fRelativeFill;};
   double					GetSubLinePerUnit(void) const {return fSubLinesPerUnit;};

   bool               IsFramed(void) const {return fHasFrame;};
   bool               IsFilled(void) const {return fHasFill;};
   bool               IsShadowed(void) const {return fHasShadow;}; // draws with inverted colors (works only with NFmiRectTangle)
   bool               IsInverted(void) const {return fInvert;}; // draws with inverted colors (works only with NFmiRectTangle)
   bool               IsGradient(void) const {return fHasGradient;};

   void                     SetGradientCenter(const NFmiPoint &newCenter);
   void                     SetNumberOfGradients(const unsigned short theNumber);
   void                     SetLineStyle(FmiLineStyle lineStyle){itsLineStyle = lineStyle;}; //Lasse/1.12.99

   void                     SetPenSize(const NFmiPoint &newPenSize);
   void                     SetShadowSize(const NFmiPoint &newShadowSize);

   void                     SetForegroundColor(const FmiRGBColor &newRGBColor);
   void                     SetForegroundColor(const NFmiColor &newFmiColor);

   void                     SetBackgroundColor(const FmiRGBColor &newRGBColor);
   void                     SetBackgroundColor(const NFmiColor &newFmiColor);

   void                     SetFillColor(const FmiRGBColor &newRGBColor);
   void                     SetFillColor(const NFmiColor &newFmiColor);

   void                     SetFrameColor(const FmiRGBColor &newRGBColor);
   void                     SetFrameColor(const NFmiColor &newFmiColor);

   void                     SetShadowColor(const FmiRGBColor &newRGBColor);
   void                     SetShadowColor(const NFmiColor &newFmiColor);

   void                     SetShadowDisplacement(const NFmiPoint &displacement);

   void                     SetFontSize(const NFmiPoint &newFontSize);
   void                     SetFontType(const FmiFontType& newType);
   void                     SetFontName(const NFmiString& font){itsFontName=font;};

   // 1997.09.02/Marko
   void						SetFillPattern(FmiPattern newPattern){fFillPattern=newPattern;};
   void						SetRelativeFill(double newRelFill)
							{
							 if(newRelFill < 0.) {fRelativeFill=0;}
							 else if(newRelFill > 1.) {fRelativeFill=1.;}
							 else {fRelativeFill = newRelFill;}
							};
   void						SetSubLinePerUnit(double newValue){fSubLinesPerUnit = newValue;};
   
   void                     EnableFrame(void) {fHasFrame = true;};
   void                     EnableFill(void) {fHasFill = true;};
   void                     EnableShadow(void) {fHasShadow = true;};
   void                     EnableGradient(void) {fHasGradient = true;};
   void                     DisableFrame(void) {fHasFrame = false;};
   void                     DisableFill(void) {fHasFill = false;};
   void                     DisableShadow(void) {fHasShadow = false;};
   void                     EnableInvert(void) {fInvert = true;}; // draws with inverted colors (works only with NFmiRectTangle)
   void                     DisableInvert(void) {fInvert = false;};
   void                     DisableGradient(void) {fHasGradient = false;};
   bool				BoldFont(void){return fBoldFont;}; // Marko/15.6.1999
   void						BoldFont(bool newValue){fBoldFont = newValue;}; // Marko/15.6.1999
  int GetHatchPattern(void) const {return itsHatchPattern;}
  void SetHatchPattern(int newValue) {itsHatchPattern = newValue;}

protected:

private:
   FmiRGBColor fForegroundColor;
   FmiRGBColor fBackgroundColor;
   FmiRGBColor fFrameColor;
   FmiRGBColor fFillColor;
   FmiRGBColor fShadowColor;
   NFmiPoint    fPenSize;
   NFmiPoint    fShadowSize;
   FmiLineStyle itsLineStyle;  
   NFmiPoint   itsShadowDisplacement; //7.12.99/Lasse, vai onko edellinen t‰t‰
   NFmiPoint   fGradientCenter;
   unsigned short fNumberOfGradients;
   FmiFont     fFont;
   NFmiString  itsFontName;   //1.12.99/Lasse vaihtoehto edelliselle
   FmiPattern  fFillPattern;
   FmiPattern  fShadowPattern;
   double	   fRelativeFill; // 1997.09.02/Marko Used with new fillpattern. Value <= 0 means
							  // no line, >= 1 means full line and 0.5 means 50% of line is
							  // drawn like here - - - - and 0.1 would be drawn like ......
   double	   fSubLinesPerUnit;// 1997.09.02/Marko Used with new fillpattern
		// when line is drawn, start and end points are given. Their distance is calculated
		// and with this attribute is calculated that how many 'sub' lines is draw to complete
	    // the whole line from start to end
   
   bool fHasFill;
   bool fHasFrame;
   bool fHasShadow;
   bool fHasGradient;
   bool fInvert; // draws with inverted colors (works only with NFmiRectTangle)

   bool fBoldFont; // Marko/15.6.1999

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

//--------------------------------------------------------
inline void NFmiDrawingEnvironment::SetNumberOfGradients(const unsigned short theNumber)
{
    fNumberOfGradients = theNumber;
}

//--------------------------------------------------------SetPenSize
inline void NFmiDrawingEnvironment::SetGradientCenter(const NFmiPoint &newCenter)
{
    fGradientCenter = newCenter;
}
//--------------------------------------------------------SetPenSize
inline void NFmiDrawingEnvironment::SetPenSize(const NFmiPoint &newPenSize)
{
    fPenSize = newPenSize;
}

//--------------------------------------------------------SetShadowSize
inline void NFmiDrawingEnvironment::SetShadowSize(const NFmiPoint &newShadowSize)
{
    fShadowSize = newShadowSize;
}
//--------------------------------------------------------SetShadowSize
inline void NFmiDrawingEnvironment::SetShadowDisplacement(const NFmiPoint &newShadowDisp)
{
    itsShadowDisplacement = newShadowDisp;
}
//--------------------------------------------------------SetFontSize
inline void NFmiDrawingEnvironment::SetFontSize(const NFmiPoint &newFontSize)
{
    fFont.height = static_cast<unsigned short>(FmiMax(1., newFontSize.Y()));
    fFont.width = static_cast<unsigned short>(FmiMax(1., newFontSize.X()));

}
//--------------------------------------------------------SetFontSize
inline void NFmiDrawingEnvironment::SetFontType(const FmiFontType& newFontType)
{
    fFont.type = newFontType;
 }

//--------------------------------------------------------SetFillColor
inline void NFmiDrawingEnvironment::SetForegroundColor(const FmiRGBColor &newRGBColor)
{
    fForegroundColor.red = FmiMin(1.f, FmiMax(0.f, newRGBColor.red));
    fForegroundColor.green = FmiMin(1.f, FmiMax(0.f, newRGBColor.green));
    fForegroundColor.blue = FmiMin(1.f, FmiMax(0.f, newRGBColor.blue));
}
//--------------------------------------------------------SetFrameColor
inline void NFmiDrawingEnvironment::SetForegroundColor(const NFmiColor &newFmiColor)
{
    fForegroundColor.red = newFmiColor.Red();
    fForegroundColor.green = newFmiColor.Green();
    fForegroundColor.blue = newFmiColor.Blue();
}

//--------------------------------------------------------SetFillColor
inline void NFmiDrawingEnvironment::SetBackgroundColor(const FmiRGBColor &newRGBColor)
{
    fBackgroundColor.red = FmiMin(1.f, FmiMax(0.f, newRGBColor.red));
    fBackgroundColor.green = FmiMin(1.f, FmiMax(0.f, newRGBColor.green));
    fBackgroundColor.blue = FmiMin(1.f, FmiMax(0.f, newRGBColor.blue));
}
//--------------------------------------------------------SetFrameColor
inline void NFmiDrawingEnvironment::SetBackgroundColor(const NFmiColor &newFmiColor)
{
    fBackgroundColor.red = newFmiColor.Red();
    fBackgroundColor.green = newFmiColor.Green();
    fBackgroundColor.blue = newFmiColor.Blue();
}

//--------------------------------------------------------SetFrameColor
inline void NFmiDrawingEnvironment::SetFrameColor(const FmiRGBColor &newRGBColor)
{
    fFrameColor.red = FmiMin(1.f, FmiMax(0.f, newRGBColor.red));
    fFrameColor.green = FmiMin(1.f, FmiMax(0.f, newRGBColor.green));
    fFrameColor.blue = FmiMin(1.f, FmiMax(0.f, newRGBColor.blue));
}

//--------------------------------------------------------SetFrameColor
inline void NFmiDrawingEnvironment::SetFrameColor(const NFmiColor &newFmiColor)
{
    fFrameColor.red = newFmiColor.Red();
    fFrameColor.green = newFmiColor.Green();
    fFrameColor.blue = newFmiColor.Blue();
}

//--------------------------------------------------------SetFillColor
inline void NFmiDrawingEnvironment::SetFillColor(const FmiRGBColor &newRGBColor)
{
    fFillColor.red = FmiMin(1.f, FmiMax(0.f, newRGBColor.red));
    fFillColor.green = FmiMin(1.f, FmiMax(0.f, newRGBColor.green));
    fFillColor.blue = FmiMin(1.f, FmiMax(0.f, newRGBColor.blue));
}
//--------------------------------------------------------SetFrameColor
inline void NFmiDrawingEnvironment::SetFillColor(const NFmiColor &newFmiColor)
{
    fFillColor.red = newFmiColor.Red();
    fFillColor.green = newFmiColor.Green();
    fFillColor.blue = newFmiColor.Blue();
}


//--------------------------------------------------------SetShadowColor
inline void NFmiDrawingEnvironment::SetShadowColor(const FmiRGBColor &newRGBColor)
{
    fShadowColor.red = FmiMin(1.f, FmiMax(0.f, newRGBColor.red));
    fShadowColor.green = FmiMin(1.f, FmiMax(0.f, newRGBColor.green));
    fShadowColor.blue = FmiMin(1.f, FmiMax(0.f, newRGBColor.blue));
}
//--------------------------------------------------------SetFrameColor
inline void NFmiDrawingEnvironment::SetShadowColor(const NFmiColor &newFmiColor)
{
    fShadowColor.red = newFmiColor.Red();
    fShadowColor.green = newFmiColor.Green();
    fShadowColor.blue = newFmiColor.Blue();
}


