//© Ilmatieteenlaitos/Persa.
//23.12.1994   Persa
//
//Ver. 4.12.1995/Persa	Conversion from NFmiDrop
//--------------------------------------------------------------------------- ndrop.cp



#include "NFmiDrop.h"
               
NFmiDrop::NFmiDrop (const NFmiRect &theRect
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
void NFmiDrop::Build (void) 
{
   const int kNumberOfPoints = 16;
   float x[kNumberOfPoints] = 
      {0.76f, 0.62f, 0.4f, 0.22f, 0.17f, 0.13f, 0.2f, 0.38f, 0.53f, 0.7f, 0.75f, 0.78f,
       0.77f, 0.74f, 0.75f, 0.76f};
   float y[kNumberOfPoints] = 
      {0.0f, 0.2f, 0.39f, 0.5f, 0.6f, 0.78f, 0.94f, 1.0f, 0.98f, 0.9f, 0.8f, 0.7f,
       0.5f, 0.3f, 0.15f, 0.0f };
   
   BuildPolyLine(x, y, kNumberOfPoints, GetEnvironment());
}
               
