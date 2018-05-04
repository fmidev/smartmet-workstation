//© Ilmatieteenlaitos/Persa.
//27.5.1994   Persa
//
//Ver. 26.10.1994
// 27.9.96/LW
//030397/LW +font.type
// 1997.09.02/Marko New fill pattern and its relative value
// 1998.10.06/Marko Inverted colors drawing possibility enabled.
// 1999.06.15/Marko Font can be bold now.


#include "NFmiDrawingEnvironment.h"


NFmiDrawingEnvironment::NFmiDrawingEnvironment (void)
{
   NFmiPoint defaulSize(1, 1); 
   NFmiPoint gradientCenter(.5, .5);
   SetNumberOfGradients(5);
   SetPenSize(defaulSize);
   SetLineStyle(kSolid);       

   defaulSize.X(2); 
   defaulSize.Y(2); 
   SetShadowSize(defaulSize);

   defaulSize.X(6); 
   defaulSize.Y(6); 
   SetShadowDisplacement(defaulSize);

  
   defaulSize.X(8); 
   defaulSize.Y(12); 
   SetFontSize(defaulSize);
   SetFontType(kCourierNew);

   FmiRGBColor defaultColor;
   defaultColor.red = 0.0f;
   defaultColor.green = 0.0f;
   defaultColor.blue = 0.0f;
   
   SetForegroundColor(defaultColor);
   SetFrameColor(defaultColor);

   defaultColor.red = 0.6f;    
   defaultColor.green = 0.6f;
   defaultColor.blue = 0.6f;
   SetShadowColor(defaultColor);

   defaultColor.red = 1.0f;
   defaultColor.green = 1.0f;
   defaultColor.blue = 1.0f;

   SetBackgroundColor(defaultColor);
   SetFillColor(defaultColor);
   
   EnableFrame();
   DisableFill();
   DisableShadow();
   DisableGradient();
   fFillPattern=FMI_SOLID; // 1997.09.02/Marko
   fRelativeFill=1.; // 1997.09.02/Marko
   fSubLinesPerUnit=1.; // 1997.09.02/Marko
   fShadowPattern=FMI_SOLID;
   DisableInvert(); // 1998.10.06/Marko
   fBoldFont = false; // 1999.06.15/Marko
   itsHatchPattern = -1;
}
NFmiDrawingEnvironment::NFmiDrawingEnvironment (const NFmiDrawingEnvironment &anOther)
{
   fForegroundColor = anOther.fForegroundColor;
   fBackgroundColor = anOther.fBackgroundColor;
//   fFrameColor = anOther.fFrameColor;
   fFrameColor.red = anOther.fFrameColor.red;
   fFrameColor.green = anOther.fFrameColor.green;
   fFrameColor.blue = anOther.fFrameColor.blue;
   fFillColor.red = anOther.fFillColor.red;
   fFillColor.green = anOther.fFillColor.green;
   fFillColor.blue = anOther.fFillColor.blue;
   fShadowColor = anOther.fShadowColor;
   fPenSize = anOther.fPenSize;
   itsLineStyle = anOther.itsLineStyle;      //Lasse/1.12.99
   fGradientCenter = anOther.fGradientCenter;
   fNumberOfGradients = anOther.fNumberOfGradients;
   fShadowSize = anOther.fShadowSize;
   itsShadowDisplacement = anOther.itsShadowDisplacement; //27.12.99/Lasse
   fShadowSize = anOther.fShadowSize;
   fFont.height = anOther.fFont.height;
   fFont.width = anOther.fFont.width;
   fFont.type = anOther.fFont.type;
   itsFontName = anOther.itsFontName;
   fFillPattern = anOther.fFillPattern;
   fShadowPattern = anOther.fShadowPattern;
   fRelativeFill=anOther.fRelativeFill; // 1997.09.02/Marko
   fSubLinesPerUnit=anOther.fSubLinesPerUnit; // 1997.09.02/Marko
   
   fHasFill = anOther.fHasFill;
   fHasFrame = anOther.fHasFrame;
   fHasShadow = anOther.fHasShadow;
   fHasGradient = anOther.fHasGradient;
   fInvert = anOther.fInvert; // 1998.10.06/Marko
   fBoldFont = anOther.fBoldFont; // 1999.06.15/Marko
   itsHatchPattern = anOther.itsHatchPattern;
}

