
#pragma once

#include "NFmiSymbol.h"


//_________________________________________________________ NFmiDrop
class NFmiDrop : public NFmiSymbol
{
 public:

   NFmiDrop(const NFmiRect &theRect
          ,NFmiToolBox *theToolBox
          ,NFmiView *theEnclosure = 0
          ,NFmiDrawingEnvironment *theEnvironment = 0
          ,ShapeIdentifier theIdent = kSymbol); 

    virtual void Build(void); 
 private:
};

typedef NFmiDrop* PNFmiDrop;


