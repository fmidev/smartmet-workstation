//© Ilmatieteenlaitos/Persa.
//25.12.1994   Persa
//
//Ver. 25.12.1994
//--------------------------------------------------------------------------- NFmiSun.h


#pragma once

#include "NFmiSymbol.h"


//_________________________________________________________ NFmiSun
class NFmiSun : public NFmiSymbol
{
 public:

   NFmiSun(const NFmiRect &theRect
          ,NFmiToolBox *theToolBox
          ,NFmiView *theEnclosure = 0
          ,NFmiDrawingEnvironment *theEnvironment = 0
          ,ShapeIdentifier theIdent = kSymbol); 

    virtual void Build(void); 
 private:
 };

typedef NFmiSun* PNFmiSun;

