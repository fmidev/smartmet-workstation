//© Ilmatieteenlaitos/Persa.
//Original 12.5.1995
// 
// 
//                                  
//Ver. 12.5.1995
//--------------------------------------------------------------------------- NFmiGramParamView.h

#pragma once

#include "NFmiMetaFileView.h"

class NFmiGramParamDataView;
class NFmiTimeScaleView;
//_________________________________________________________ NFmiGramParamView
class NFmiGramParamView : public NFmiMetaFileView
{
 public:

                               NFmiGramParamView(const NFmiRect &theRect
                                                ,NFmiToolBox *theToolBox
                                                ,NFmiView *theEnclosure = 0
                                                ,NFmiDrawingEnvironment *theEnvironment = 0
                                                ,unsigned long theIdent = 0);
                                         
                               NFmiGramParamView(const NFmiGramParamView &theView);
                                 
                      virtual ~NFmiGramParamView(void); 

        NFmiGramParamView &operator= (const NFmiGramParamView &theView);

  NFmiVoidPtrList  &CollectOwnDrawingItems(NFmiVoidPtrList  &theDrawingList);
 
 private:

//  NFmiGramParam *itsParam;
  NFmiRect *itsDataArea;
  NFmiGramParamDataView *itsDataView;
  NFmiRect *itsTimeArea;
  NFmiTimeScaleView *itsTimeView;
  
};

typedef NFmiGramParamView* PNFmiGramParamView;





//Inlines
