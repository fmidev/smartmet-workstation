//© Ilmatieteenlaitos/Persa.
//Original 6.11.1995
// 
// 
//                                  
//Ver. 19.01.1995
// 170297/LW DrawTickTexts(+theTextHeigh)
// 130397/LW +itsFontType
//Ver. 11.10.1997/Persa Value(NFmiPoint clickPoint) method added
//Ver. 10.12.1999/Marko Muutin DrawAll-metodin virtuaaliseksi.
//--------------------------------------------------------------------------- NFmiAxisView.h

#pragma once

#include "NFmiMetaFileView.h"
#include "NFmiDrawingEnvironment.h"

#ifdef _MSC_VER
#pragma warning(disable : 4512) // poistaa VC++ k‰‰nt‰j‰n C4512 "assignment operator could not be generated" varoituksen
#endif

class NFmiAxis;
class NFmiString;

//_________________________________________________________ NFmiAxisView
class NFmiAxisView : public NFmiMetaFileView
{
 public:

                               NFmiAxisView(const NFmiRect &theRect
                                           ,NFmiToolBox *theToolBox
                                           ,NFmiAxis *theAxis
                                           ,FmiDirection theOrientation
                                           ,FmiDirection theRotation = kRight
                                           ,bool isAxis = true
                                           ,bool isPrimaryText = true
                                           ,bool isSecondaryText = false
                                           ,float theTickLenght = 0.2
										   ,FmiFontType = kTimesNewRoman
                                           ,NFmiView *theEnclosure = 0
                                           ,NFmiDrawingEnvironment *theEnvironment = 0
                                           ,unsigned long theIdent = 0);
                                         
                                 
                      virtual ~NFmiAxisView(void); 


//  NFmiVoidPtrList  &CollectOwnDrawingItems(NFmiVoidPtrList  &theDrawingList);
 
        void        SetAxis(NFmiAxis *newAxis) {itsAxis = newAxis;};

          NFmiRect  CalcScaleArea(void);

       virtual void DrawAll(void);
               void DrawHeader(void);
               void DrawAxisBase(void);
               void DrawTicks(void);
               void DrawTickTexts(double theTextHeigh);
          NFmiPoint HeaderPlace(void);
           NFmiRect CalcAxisArea(void);
           NFmiRect CalcTickTextArea(void);
          NFmiPoint TickStart(const NFmiRect &theRect
                             ,double theRelativeLocation);
          NFmiPoint TickEnd(const NFmiRect &theRect
                           ,double theRelativeLocation);
          NFmiPoint TickTextStart(const NFmiRect &theRect
                                 ,double theRelativeLocation
                                 ,const NFmiString &theText);
             double CalcTickLength(const NFmiRect &theRect);
             double RelativeTickPlace(void);
             double NumberOfTextLines(void);

               void SetUpEnvironment(void);
			 double Value(const NFmiPoint& theClickPoint); //Gets the value of the scale
													//in a given point. The point
													//is in the axisViews coordinate system.
 protected:

// private:
   NFmiAxis *itsAxis;
   FmiFontType itsFontType; //130397

//   float itsVerticalMargin;  
//   float itsHorisontalMargin;  
   float itsTickLenght;
   FmiDirection itsOrientation;  
   FmiDirection itsRotation;  
   bool fItsAxis;
   bool fItsPrimaryText;
   bool fItsSecondaryText;
   short itsCurrentTextLine;
};

typedef NFmiAxisView* PNFmiAxisView;


