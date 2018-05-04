//Ver. 28.2.1996/Lasse  conversion from NFmiCloud

#pragma once

#include "NFmiSymbol.h"

typedef enum
{
	kCalm = 0
   ,kWeak
   ,kModerate
   ,kGale
   ,kNearGale
   ,kStorm
   ,kSevereStorm
} FmiWindClass;

//_________________________________________________________ NFmiArrow
class NFmiArrow : public NFmiSymbol
{
 public:

   NFmiArrow(float theValue
	        ,const NFmiRect &theRect
            ,NFmiToolBox *theToolBox
            ,NFmiView *theEnclosure = 0
            ,NFmiDrawingEnvironment *theEnvironment = 0
            ,ShapeIdentifier theIdent = kShapeList); 

    virtual void Build(void);
	FmiWindClass Class(void);

 protected:
    float itsValue;

 private:

};

typedef NFmiArrow* PNFmiArrow;


