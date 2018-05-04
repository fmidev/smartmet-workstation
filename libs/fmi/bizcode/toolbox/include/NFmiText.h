// 2.10.1997/Marko Muutin GetTextLen():iä


#pragma once

#include "NFmiShape.h"

class NFmiString;

//_________________________________________________________ NFmiText
class NFmiText : public NFmiShape
{
 public:

     NFmiText(const NFmiPoint& theStartingPoint
             ,const NFmiString &theText
             ,NFmiView *theEnclosure = 0
             ,NFmiDrawingEnvironment *theEnvironment = 0); 
     NFmiText(const NFmiRect& theRect
             ,const NFmiString &theText
			 ,bool theWrap = true
             ,NFmiView *theEnclosure = 0
             ,NFmiDrawingEnvironment *theEnvironment = 0); 
	 virtual ~NFmiText(void);

              void SetText(const NFmiString &theText);
              char *GetText(void) const;
     unsigned long GetTextLen(void) const;
	     bool IsWrappable(void) const {return fWrap;};

 private:
    NFmiString *itsText;
	bool fWrap;
};

typedef NFmiText* PNFmiText;

