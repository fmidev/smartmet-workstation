
#pragma once

#include "NFmiSymbol.h"


//_________________________________________________________ NFmiSnow
class NFmiSnow : public NFmiSymbol
{
 public:

   NFmiSnow(const NFmiRect &theRect
          ,NFmiToolBox *theToolBox
          ,NFmiView *theEnclosure = 0
          ,NFmiDrawingEnvironment *theEnvironment = 0
          ,ShapeIdentifier theIdent = kSymbol); 

    virtual void Build(void); 
 private:
};

typedef NFmiSnow* PNFmiSnow;

