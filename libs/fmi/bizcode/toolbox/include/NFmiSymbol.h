//© Ilmatieteenlaitos/Persa.
//13.11.1995   Persa
//
//Ver. 00.00.1995
//--------------------------------------------------------------------------- NFmiSymbol.h


#pragma once

#include "NFmiMetaFileView.h"

#ifdef _MSC_VER
#pragma warning(disable : 4512) // poistaa VC++ k‰‰nt‰j‰n C4512 "assignment operator could not be generated" varoituksen
#endif

class NFmiShape;

//_________________________________________________________ NFmiSymbol
class NFmiSymbol : public NFmiMetaFileView   //public NFmiShapeList
{
 public:

   NFmiSymbol(const NFmiRect &theRect
             ,NFmiToolBox *theToolBox
             ,NFmiView *theEnclosure = 0
             ,NFmiDrawingEnvironment *theEnvironment = 0
             ,ShapeIdentifier theIdent = kSymbol); 


    virtual void Build(void)=0; 
            void BuildMultiLine (const float *xPoints,
                                 const float *yPoints,
                                 const int numberOfPoints,
                                 NFmiDrawingEnvironment *theEnvironment = 0); 
            void BuildPolyLine (const float *xPoints,
                                const float *yPoints,
                                const int numberOfPoints,
                                NFmiDrawingEnvironment *theEnvironment = 0); 

      NFmiShape *BuildRectangle (const float top,
                                 const float left,
                                 const float bottom,
                                 const float right,
                                 NFmiDrawingEnvironment *theEnvironment = 0); 
      NFmiShape *BuildTriangle  (const float top,
                                 const float left,
                                 const float bottom,
                                 const float right,
                                 NFmiDrawingEnvironment *theEnvironment = 0); 
      NFmiShape *BuildSnowFlake (const float top,
                                 const float left,
                                 const float bottom,
                                 const float right,
                                 NFmiDrawingEnvironment *theEnvironment = 0);
                                  
      NFmiShape *BuildRainDrop  (const NFmiPoint &thePlace,
                                 const NFmiPoint &theSize,
                                 NFmiDrawingEnvironment *theEnvironment = 0); 



 private:
};

typedef NFmiSymbol* PNFmiSymbol;

