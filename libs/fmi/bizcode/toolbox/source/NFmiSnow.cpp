//© Ilmatieteenlaitos/Lasse.
//9.12.1996   Persa
//
//Ver. 9.12.1996/Lasse	Conversion from NFmiDrop
//--------------------------------------------------------------------------- ndrop.cp

#include "NFmiSnow.h"
               
NFmiSnow::NFmiSnow (const NFmiRect &theRect
                   ,NFmiToolBox *theToolBox
                   ,NFmiView *theEnclosure
                   ,NFmiDrawingEnvironment *theEnvironment
                   ,ShapeIdentifier theIdent) 
        : NFmiSymbol(theRect
                    ,theToolBox
                    ,theEnclosure
                    ,theEnvironment
                    ,theIdent)
{
}
void NFmiSnow::Build (void) 
{
   const int kNumberOfPoints = 2;
   float x1[kNumberOfPoints] = {.75f, .25f};
   float y1[kNumberOfPoints] = {.933f, .067f};
   float x2[kNumberOfPoints] = { 1.f, 0.f};
   float y2[kNumberOfPoints] = {.5f, .5f};
   float x3[kNumberOfPoints] = {.75f, .25f};
   float y3[kNumberOfPoints] = {.067f, .933f};
  
   BuildPolyLine(x1, y1, kNumberOfPoints, GetEnvironment());
   BuildPolyLine(x2, y2, kNumberOfPoints, GetEnvironment());
   BuildPolyLine(x3, y3, kNumberOfPoints, GetEnvironment());
}
               
