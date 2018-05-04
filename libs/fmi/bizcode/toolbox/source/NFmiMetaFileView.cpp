//© Ilmatieteenlaitos/Persa.
//04. 5.1995   Persa
//
//Ver. 4. 5.1995
//--------------------------------------------------------------------------- nmfview.cpp



#include "NFmiMetaFileView.h"


//___________________________________________________________ NFmiMetaFileView
NFmiMetaFileView::NFmiMetaFileView(const NFmiRect &theRect
                                  ,NFmiToolBox *theToolBox
                                  ,const NFmiView *theEnclosure
                                  ,NFmiDrawingEnvironment *theEnvironment
                                  ,unsigned long theIdent)
         : NFmiView(theRect,
                    theToolBox,
                    theEnclosure,
                    theEnvironment, theIdent)
         , itsOwnDrawingItems(0)
{
  SetDirty();
}
//___________________________________________________________ NFmiMetFileView
NFmiMetaFileView::~NFmiMetaFileView(void)
{
}
//_________________________________________________________________________ CollectDrawingItems

NFmiVoidPtrList &NFmiMetaFileView::CollectDrawingItems(NFmiVoidPtrList &theDrawingList)
{
  CollectOwnDrawingItems(theDrawingList);
  CollectSubDrawingItems(theDrawingList);
  
  return theDrawingList;
}
//_________________________________________________________________________ CollectSubDrawingItems

NFmiVoidPtrList &NFmiMetaFileView::CollectSubDrawingItems(NFmiVoidPtrList &theDrawingList)
{
  return theDrawingList;
  
}
//_________________________________________________________________________ CollectOwnDrawingItems

NFmiVoidPtrList &NFmiMetaFileView::CollectOwnDrawingItems(NFmiVoidPtrList &theDrawingList)
{
  //TŠhŠn metodiin koodia perityssŠ luokassa. Muista kuitenkin aina kutsua emon metodia
  // ensimmŠiseksi.

  if(IsDirty())
  {
/////////    itsOwnDrawingItems->Clear(kDelete);   //Mahtaakohan delete toimia
//    return theDrawingList;
  }

//  if(itsFrame)
//    itsDrawingArea = BuildFrame(TFmiColor(0.7,0.7,0.9)

    BuildFrame(*itsBackgroundColor
              ,*itsBackgroundColor
              ,theDrawingList);										

  return theDrawingList;  //+= itsOwnDrawingList;
}
//_________________________________________________________________________ CollectDrawingItems
NFmiVoidPtrList &NFmiMetaFileView::GetDrawingItems(void)
{
/****
  if(!IsDirty())
    return itsDrawingList;
  
  ClearDrawingList();
  
//  SetDirty(false);
//  itsDrawingList.Clear(kDelete);
****/
  CollectDrawingItems(itsDrawingList);
  return itsDrawingList;
}
/*
//___________________________________________________________ SetRect
void NFmiMetaFileView::SetRect(const NFmiRect &theRect)
{
  if(*itsRect != theRect)
  {
    SetDirty();
    delete itsRect;
    itsRect = new NFmiRect(theRect);
    if(itsSubVIews)
    {
       NFmiVoidPtrIterator theIterator(itsSubVIews);
      void *theItem;
      while(theIterator.Next(theItem))
        ((NFmiView *)theItem)->SetRect(theRect);
     }  
  }
}
*/
//_________________________________________________________________________ CollectDrawingItems
void NFmiMetaFileView::ClearDrawingList(void)
{
  NFmiVoidPtrIterator shapeIterator(itsDrawingList);
  void *aDrawingItem;
  while(shapeIterator.Next(aDrawingItem))
    delete static_cast<NFmiDrawingItem *>(aDrawingItem);

  itsDrawingList.Clear();
  
  SetDirty(true);
}
