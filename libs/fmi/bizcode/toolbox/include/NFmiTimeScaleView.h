//© Ilmatieteenlaitos/Persa.
//Original 12.9.1995
// 
// 
//                                  
//Ver. 12.9.1995
//Ver. 24.04.1997/Persa 
//Ver. 23.09.1998/Marko Added GetTime(float)-method (quickfix), should be done otherway
//Ver. 23.09.1998/Marko Changed the SetSelectedScale()-method.
//Ver. 23.09.1998/Marko Added the RelativePoint-method.
//Ver. 23.09.1999/Viljo Added a parameter bool isUpsideDown = true to the constructor
//Ver. 23.09.1999/Viljo Moved the attributes itsBaseScale and fUpsideDown from private to protected section					
//Ver. 01.12.1999/Marko Muutin DrawScale-metodin virtuaaliseksi, ett‰ voin peri‰ uuden luokan.
//--------------------------------------------------------------------------- NFmiTimeScaleView.h

#pragma once

#include "NFmiMetaFileView.h"
#include "NFmiMetTime.h"

#ifdef _MSC_VER
#pragma warning(disable : 4512) // poistaa VC++ k‰‰nt‰j‰n C4512 "assignment operator could not be generated" varoituksen
#endif

class NFmiGramParamDataView;
class NFmiStepTimeScale;

//_________________________________________________________ NFmiTimeScaleView
class NFmiTimeScaleView : public NFmiMetaFileView
{
 public:

                               NFmiTimeScaleView(const NFmiRect &theRect
                                                ,NFmiToolBox *theToolBox
//                                                ,const NFmiStepTimeScale *theBaseScale = 0
//                                                ,const NFmiStepTimeScale *theSelectedScale = 0
                                                ,NFmiStepTimeScale *theBaseScale = 0
                                                ,NFmiStepTimeScale *theSelectedScale = 0
                                                ,NFmiView *theEnclosure = 0
                                                ,NFmiDrawingEnvironment *theEnvironment = 0
                                                ,unsigned long theIdent = 0
												,bool isUpsideDown = true);
                                         
                                 
                      virtual ~NFmiTimeScaleView(void); 


  virtual NFmiVoidPtrList  &CollectOwnDrawingItems(NFmiVoidPtrList  &theDrawingList);
 
        void        SetBaseScale(NFmiStepTimeScale *newScale) {itsBaseScale = newScale;};
        void        SetSelectedScale(NFmiStepTimeScale *newScale);
//        void        SetBaseScale(const NFmiStepTimeScale &newScale) {delete itsBaseScale;
//                                                              itsBaseScale = new NFmiStepTimeScale(newScale);};
//        void        SetSelectedScale(const NFmiStepTimeScale &newScale) {delete itsSelectedScale;
//                                                              itsSelectedScale = new NFmiStepTimeScale(newScale);};

          NFmiRect  CalcScaleArea(void) const;
          NFmiRect  CalcSelectedArea(void);
              void MoveScale(const NFmiPoint &upTo);
              void ResizeScale(const NFmiPoint &upTo);
   virtual NFmiVoidPtrList &DrawScale(NFmiVoidPtrList &theDrawingList
                             ,NFmiStepTimeScale *theScale
                             ,const NFmiRect &theRect);
		NFmiMetTime GetTime(const NFmiPoint &clickedPoint, bool fExact = false); // 23.9.1998/Marko
		NFmiMetTime GetAccurateTime(const NFmiPoint &clickedPoint, int theResolutionInMinutes); // palauttaa ajan minuutin tarkkuudella
		NFmiPoint RelativePoint(const NFmiPoint &theAbsolutePoint); // 23.9.1998/Marko
 private:
         NFmiPoint AdjustToScaleArea(const NFmiPoint &klickedPoint);
 
 protected:
   NFmiStepTimeScale *itsBaseScale;
   bool fUpsideDown; 
   
 private:
   NFmiStepTimeScale *itsSelectedScale;
   double itsVerticalMargin;  
   double itsHorisontalMargin;  
   double itsTickLenght;
};

typedef NFmiTimeScaleView* PNFmiTimeScaleView;

