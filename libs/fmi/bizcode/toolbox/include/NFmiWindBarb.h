//130597/LW originaali

#pragma once

#include "NFmiSymbol.h"

#ifdef _MSC_VER
#pragma warning(disable : 4512) // poistaa VC++ k‰‰nt‰j‰n C4512 "assignment operator could not be generated" varoituksen
#endif

//_________________________________________________________ NFmiWindBarbSymbol
class NFmiWindBarb : public NFmiSymbol
{
 public:

   NFmiWindBarb(float theSpeed
	           ,float theDirection
               ,const NFmiRect &theRect
               ,NFmiToolBox *theToolBox
               ,bool onSouthernHemiSphere
			   ,double theBarbLength=.4f
			   ,double theFlagLength=.3f
               ,NFmiView *theEnclosure = 0
               ,NFmiDrawingEnvironment *theEnvironment = 0
               ,ShapeIdentifier theIdent = kSymbol); 

    virtual void Build(void);
     
 protected:
   NFmiPoint ConvertFromThisToToolBoxWorld(const NFmiPoint &thePoint);
 
   float itsSpeed;
   float itsDirection;

   double itsBarbLength;
   double itsFlagLength;
   bool fOnSouthernHemiSphere; // etel‰isell‰ pallonpuoliskolla tuulinuolien v‰k‰set piirret‰‰n toiselle puolelle eli matalapainetta kohden (kuten pohjoisellakin)

 private:

};
typedef NFmiWindBarb* PNFmiWindBarb;

