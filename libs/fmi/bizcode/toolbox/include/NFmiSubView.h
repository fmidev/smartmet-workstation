//© Ilmatieteenlaitos/Persa.
//Original 18.12.1994
// 
// 
//                                  
//Ver. 18.12.1994
//--------------------------------------------------------------------------- NFmiSubView.h

#pragma once

#include "NFmiView.h"

class NFmiVoidPtrList;

//_________________________________________________________ NFmiSubView
class NFmiSubView : public NFmiView
{
 public:

                               NFmiSubView(const NFmiRect &theRect
                                          ,NFmiToolBox *theToolBox
                                          ,NFmiSubView *theEnclosure = 0
                                          ,NFmiDrawingEnvironment *theEnvironment = 0
                                          ,unsigned long theIdent = 0);
                                         
//                               NFmiSubView(const NFmiSubView &theView){};
                                 
                      virtual ~NFmiSubView(void){}; 

  virtual NFmiVoidPtrList &CollectOwnDrawingItems(NFmiVoidPtrList &theDrawingList);
  virtual NFmiVoidPtrList &CollectDrawingItems(NFmiVoidPtrList &theDrawingList);
  virtual NFmiVoidPtrList &CollectSubDrawingItems(NFmiVoidPtrList &theDrawingList);


 private:

  bool itsBaseViewFlag;

  NFmiVoidPtrList *itsSubViews;
};

typedef NFmiSubView* PNFmiSubView;


