																																																//© Ilmatieteenlaitos/Persa.
//13.11.1995   Persa
//
//Ver. 00.00.1995
// 061196/LW
//--------------------------------------------------------------------------- nsun.cpp

#include "NFmiSun.h"

               
NFmiSun::NFmiSun (const NFmiRect &theRect
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
void NFmiSun::Build (void) 
{
   const int kStarPoints = 8;
   float itsStarX[kStarPoints] = {0.50f, 0.85f, 0.00f, 0.85f, 0.50f, 0.15f, 1.00f, 0.15f};
   float itsStarY[kStarPoints] = {0.00f, 0.85f, 0.50f, 0.15f, 1.00f, 0.15f, 0.50f, 0.85f};

   BuildPolyLine(itsStarX, itsStarY, kStarPoints, GetEnvironment());

   const int kCirclePoints = 8;
   float itsCircleX[kCirclePoints] = {0.38f, 0.62f, 0.78f, 0.78f, 0.62f, 0.38f, 0.22f, 0.22f};
   float itsCircleY[kCirclePoints] = {0.22f, 0.22f, 0.38f, 0.62f, 0.78f, 0.78f, 0.62f, 0.38f};
  
   //pitäisi palauttaa ennalleen
 //  GetEnvironment()->DisableFrame(); //061196/LW tuli kuitenkin, ja mustalla
   BuildPolyLine(itsCircleX, itsCircleY, kCirclePoints, GetEnvironment());
}
               
