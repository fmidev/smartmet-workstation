//© Ilmatieteenlaitos/Persa.
//23.12.1994   Persa
//
//Ver. 30.11.1995       Portattu TFmiFlash'ista
//--------------------------------------------------------------------------- nflash.cpp


#include "NFmiFlash.h"

               
NFmiFlash::NFmiFlash (const NFmiRect &theRect
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
void NFmiFlash::Build (void) 
{
   const int kNumberOfPoints = 7;
   float x[kNumberOfPoints] = 
      {0.4f , 0.2f , 0.4f , 0.1f , 0.8f , 0.6f , 0.7f };
   float y[kNumberOfPoints] = 
      {0.0f , 0.4f , 0.4f , 1.0f , 0.2f , 0.2f , 0.0f };
   
   BuildPolyLine(x, y, kNumberOfPoints, GetEnvironment());
}
               
