//© Ilmatieteenlaitos/Persa.
//Original 13.10.1995
// 
// 
//                                  
//Ver. xx.xx.xxxx
//--------------------------------------------------------------------------- ndrawite.cpp


#include "NFmiDrawingItem.h"
#include "NFmiDrawingEnvironment.h"

//___________________________________________________________ NFmiDrawingItem
NFmiDrawingItem::NFmiDrawingItem(const NFmiDrawingEnvironment *theEnvironment
                                ,unsigned long theIdent)
         : itsEnvironment(theEnvironment ? new NFmiDrawingEnvironment(*theEnvironment) : 0)
{
  if(theIdent)
    itsIdent = theIdent;
  else
    CreateIdent();
}
//___________________________________________________________ ~NFmiView
NFmiDrawingItem::~NFmiDrawingItem(void)
{
  delete itsEnvironment;
}
//_________________________________________________________________________ CreateIdent

void NFmiDrawingItem::CreateIdent(void)
{
  itsIdent = 0;
}
//_________________________________________________________________________ SetUpEnvironment
void NFmiDrawingItem::SetUpEnvironment(void)
{
  if(!itsEnvironment)
    itsEnvironment = new NFmiDrawingEnvironment;
}

//___________________________________________________________ operator=
//inline
NFmiDrawingItem& NFmiDrawingItem::operator= (const NFmiDrawingItem &theDrawingItem)
{
  itsIdent = theDrawingItem.itsIdent;
  delete itsEnvironment;
  itsEnvironment = new NFmiDrawingEnvironment(*(theDrawingItem.itsEnvironment));
  return *this;
}
