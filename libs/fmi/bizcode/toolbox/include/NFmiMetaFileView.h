//© Ilmatieteenlaitos/Persa.
//Original  4.5.1995
// 
// 
//                                  
//Ver. 04.05.1995
//--------------------------------------------------------------------------- NFmiMetaFileView.h

#pragma once

#include "NFmiView.h"
#include "NFmiVoidPtrList.h"

//_________________________________________________________ NFmiMetaFileView
class NFmiMetaFileView : public NFmiView
{

 public:

                               NFmiMetaFileView(const NFmiRect &theRect
                                               ,NFmiToolBox *theToolBox
                                               ,const NFmiView *theEnclosure = 0
                                               ,NFmiDrawingEnvironment *theEnvironment = 0
                                               ,unsigned long theIdent = 0);
                                 
                      virtual ~NFmiMetaFileView(void); 


            NFmiVoidPtrList  &GetDrawingItems(void);
    virtual NFmiVoidPtrList  &CollectOwnDrawingItems(NFmiVoidPtrList  &theDrawingList);
    virtual NFmiVoidPtrList  &CollectDrawingItems(NFmiVoidPtrList  &theDrawingList);
    virtual NFmiVoidPtrList  &CollectSubDrawingItems(NFmiVoidPtrList  &theDrawingList);
 
                        void  SetDirty(const bool theFlag = true) {itsDirtyFlag = theFlag;};
                  bool  IsDirty(void) const {  return itsDirtyFlag;};
                        void  ClearDrawingList(void);

 private:
  
   NFmiVoidPtrList *itsOwnDrawingItems;
   NFmiVoidPtrList itsDrawingList;
   
   bool itsDirtyFlag;
};

typedef NFmiMetaFileView* PNFmiMetaFileView;

