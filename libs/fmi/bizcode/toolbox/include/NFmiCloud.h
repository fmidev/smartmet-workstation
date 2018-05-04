#pragma once

#include "NFmiSymbol.h"


//_________________________________________________________ NFmiCloud
class NFmiCloud : public NFmiSymbol
{
 public:

   NFmiCloud(const NFmiRect &theRect
            ,NFmiToolBox *theToolBox
            ,NFmiView *theEnclosure = 0
            ,NFmiDrawingEnvironment *theEnvironment = 0
            ,ShapeIdentifier theIdent = kShapeList); 

    virtual void Build(void); 
 private:
};

typedef NFmiCloud* PNFmiCloud;


