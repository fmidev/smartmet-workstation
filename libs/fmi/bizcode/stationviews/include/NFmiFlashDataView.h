//© Ilmatieteenlaitos/software by Marko
//  Original 15.01.2004
//
//
//-------------------------------------------------------------------- NFmiFlashDataView.h

#pragma once

#include "NFmiStationView.h"

//_________________________________________________________ NFmiFlashDataView

class NFmiToolBox;
class NFmiLocation;
class NFmiArea;
class NFmiHPlaceDescriptor;
class NFmiQueryInfo;
class NFmiDataModifier;
class NFmiLocationBag;
class NFmiFastQueryInfo;
class NFmiPolyline;

class NFmiFlashDataView : public NFmiStationView
{

public:
   NFmiFlashDataView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
				   ,NFmiToolBox * theToolBox
				   ,NFmiDrawingEnvironment * theDrawingEnvi
				   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
				   ,FmiParameterName theParamId
				   ,NFmiPoint theOffSet
				   ,NFmiPoint theSize
				   ,int theRowIndex
                   ,int theColumnIndex);
   ~NFmiFlashDataView (void);
   void Draw (NFmiToolBox * theGTB);
   std::string ComposeToolTipText(const NFmiPoint& theRelativePoint);
   bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey);
   bool RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey);
   bool MouseMove (const NFmiPoint& thePlace, unsigned long theKey);

protected:
   bool FindNearestFlashInMapTimeSection(const NFmiPoint &theRelativePoint, float &theLat, float &theLon, float &thePow, float &theAcc, float &theMulti, NFmiMetTime &theTime);
   float CalcFlashColorFactor(const NFmiMetTime &theStarTime, const NFmiMetTime &theFlashTime, int theEditorTimeStepInMinutes);
   NFmiColor CalcFlashFillColor(const NFmiColor &theStartColor, const NFmiColor &theEndColor, float theMixRatio);
   void CreateFlashPolyLines(void);
   bool CalcFlashCount(int &theGroundFlashCountOut, int &theCloudFlashCountOut);
   bool PrepareForStationDraw(void) override;
   void DrawFlashes(NFmiFastQueryInfo &theInfo);
   bool GetTimeSpanIndexies(NFmiFastQueryInfo &theInfo, unsigned long &theStartIndexOut, unsigned long &theEndIndexOut);
   bool FindNearestFlash(NFmiFastQueryInfo &theInfo, const NFmiMetTime &theTime, int theUsedTimeStep, const NFmiLocation &theCursorLocation, double &theCurrentMinDist, unsigned long &theMinDistTimeIndex);
   NFmiMetTime CalcFirstTimeOfSpan(void);
	int CalcUsedTimeStepInMinutes(void);

   NFmiPolyline *itsNegativeFlashPolyLine;
   NFmiPolyline *itsPositiveFlashPolyLine;
   NFmiPolyline *itsCloudFlashPolyLine;
};

