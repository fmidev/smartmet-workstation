//© Ilmatieteenlaitos/Persa.
//7.11.1994   Persa
//
//Ver. 30.11.1995/Persa  conversion from TFmiCloud
//--------------------------------------------------------------------------- ncloud.cpp



#include "NFmiCloud.h"

               
NFmiCloud::NFmiCloud (const NFmiRect &theRect
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
void NFmiCloud::Build (void) 
{
   const int kNumberOfPoints = 24;
   float x[kNumberOfPoints] = 
      {0.1f , 0.0f , 0.0f , 0.13f, 0.2f , 0.18f, 0.22f, 0.3f , 0.29f, 0.33f, 0.41f, 0.48f,
       0.5f , 0.65f, 0.76f, 0.81f, 0.9f , 0.96f, 0.98f, 0.9f , 1.0f , 1.0f , 0.9f , 0.8f };
   float y[kNumberOfPoints] = 
      {0.7f , 0.62f, 0.55f, 0.47f, 0.5f , 0.4f , 0.32f, 0.3f , 0.26f, 0.2f , 0.17f, 0.2f ,
       0.1f , 0.05f, 0.1f , 0.2f , 0.16f, 0.2f , 0.3f , 0.4f , 0.48f, 0.58f, 0.68f, 0.7f };
   
   BuildPolyLine(x, y, kNumberOfPoints, GetEnvironment());
}
               
