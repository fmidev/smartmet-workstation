/*-------------------------------------------------------------------------------------*/
//© Ilmatieteen laitos/Lasse

//Originaali 05.1.1995

//Muutettu  xx.1.1995/LW
//*-------------------------------------------------------------------------------------*/

#pragma once

#include "NFmiString.h"

//_________________________________________________________ NFmiTitle

class NFmiTitle
{
 public:
	NFmiTitle (const NFmiString& theText);
	NFmiTitle (const NFmiTitle& anOtherTitle);
	virtual ~NFmiTitle(void);
	NFmiTitle& operator= (const NFmiTitle& anOtherTitle);

	NFmiString Text (void)const {return itsText;};

protected:

private:
	NFmiString itsText;

};

//______________________________________________________________________________
inline
NFmiTitle :: NFmiTitle (const NFmiString& theText)
			  : itsText (theText)
{
}
//______________________________________________________________________________
inline
NFmiTitle :: NFmiTitle (const NFmiTitle& anOtherTitle)
			  : itsText (anOtherTitle.itsText)
{
}
//______________________________________________________________________________
inline
NFmiTitle :: ~NFmiTitle(void)
{
}
//______________________________________________________________________________
inline
NFmiTitle& NFmiTitle :: operator= (const NFmiTitle& anOtherTitle)
{
	itsText = anOtherTitle.itsText;
	return *this;
}

