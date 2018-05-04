//© Ilmatieteenlaitos/Persa.
//16.9.1995   Persa
//
//Ver. 16.9.1995

#include "NFmiText.h"
#include "NFmiString.h"


//------------------------------------------------------------------
NFmiText::NFmiText (const NFmiPoint& theAlignedPoint
                   ,const NFmiString &theText
                   ,NFmiView *theEnclosure
                   ,NFmiDrawingEnvironment *theEnvironment) 
             : NFmiShape(NFmiRect(theAlignedPoint, theAlignedPoint)
                        ,theEnclosure
                        ,theEnvironment
                        ,kTextShape)
              , itsText(new NFmiString(theText))
			  , fWrap(false)
{
}
//------------------------------------------------------------------
NFmiText::NFmiText (const NFmiRect& theRect
                   ,const NFmiString &theText
				   ,bool theWrap
                   ,NFmiView *theEnclosure
                   ,NFmiDrawingEnvironment *theEnvironment) 
             : NFmiShape(theRect
                        ,theEnclosure
                        ,theEnvironment
                        ,kTextShape)
              , itsText(new NFmiString(theText))
			  , fWrap(theWrap)
{
}

NFmiText::~NFmiText(void)
{
	delete itsText;
}
               
char* NFmiText::GetText(void) const 
{
	return const_cast<char *>(reinterpret_cast<const char *>(itsText->GetCharPtr()));
}

unsigned long NFmiText::GetTextLen(void) const 
{
	return itsText->GetLen();
}

void NFmiText::SetText(const NFmiString &theText)
{
	delete itsText;
	itsText = 0;
	itsText = new NFmiString(theText);
}
