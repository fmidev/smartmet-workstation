//© Ilmatieteenlaitos/Persa.
//18.12.1994   Persa
//
//Ver. 18.12.1994
//--------------------------------------------------------------------------- nsubview.cp


#include "NFmiSubView.h"
#include "NFmiVoidPtrList.h"

//___________________________________________________________ NFmiSubView
NFmiSubView::NFmiSubView(const NFmiRect &theRect
                        ,NFmiToolBox *theToolBox
                        ,NFmiSubView *theEnclosure
                        ,NFmiDrawingEnvironment *theEnvironment
                        ,unsigned long theIdent)
         : NFmiView(theRect, theToolBox, theEnclosure, theEnvironment)
         , itsSubViews(0)
{
  itsBaseViewFlag = false;
  
  // NFmiPoint theOffset(theRect.TopLeft());
  // NFmiPoint theScale(theRect.Size());

//////  SetMap(NFmiMap(theEnclosure->GetMap()->Transform(theOffset)
//////                ,theEnclosure->GetMap()->Transform(theScale)));
  
//////  SetRect(NFmiRect(GetMap()->Transform(theRect.TopLeft())
//////                  ,GetMap()->Transform(theRect.BottomRight())));
}
//_________________________________________________________________________ CollectDrawingItems

NFmiVoidPtrList &NFmiSubView::CollectDrawingItems(NFmiVoidPtrList &theDrawingList)
{
  CollectOwnDrawingItems(theDrawingList);
  CollectSubDrawingItems(theDrawingList);
  
  return theDrawingList;
}
//_________________________________________________________________________ CollectOwnDrawingItems

NFmiVoidPtrList &NFmiSubView::CollectOwnDrawingItems(NFmiVoidPtrList &theDrawingList)
{
  //Tähän metodiin koodia perityssä luokassa. Muista kuitenkin aina kutsua emon metodia
  // ensimmäiseksi.
// NFmiView::CollectOwnDrawingItems(theDrawingList);

  return theDrawingList;
}
//_________________________________________________________________________ CollectSubDrawingItems

NFmiVoidPtrList &NFmiSubView::CollectSubDrawingItems(NFmiVoidPtrList &theDrawingList)
{
/*
  NFmiListIterator<void> toDo(itsSubViews);
  NFmiView *theSubView;
  while(toDo.Next(theSubView))
  {
    ((NFmiSubView *)theSubView)->CollectDrawingItems(theDrawingList);
  }
*/
  return theDrawingList;
  
}
