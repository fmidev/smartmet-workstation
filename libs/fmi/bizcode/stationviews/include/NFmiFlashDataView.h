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
				   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
				   ,FmiParameterName theParamId
				   ,NFmiPoint theOffSet
				   ,NFmiPoint theSize
				   ,int theRowIndex
                   ,int theColumnIndex);
   ~NFmiFlashDataView (void);
   void Draw (NFmiToolBox * theGTB) override;
   std::string ComposeToolTipText(const NFmiPoint& theRelativePoint) override;
   bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
   bool RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
   bool MouseMove (const NFmiPoint& thePlace, unsigned long theKey) override;

protected:
   bool FindNearestFlashInMapTimeSection(const NFmiPoint &theRelativePoint, float &theLat, float &theLon, float &thePow, float &theAcc, float &theMulti, NFmiMetTime &theTime);
   float CalcFlashColorFactor(const NFmiMetTime &theStarTime, const NFmiMetTime &theFlashTime, int theEditorTimeStepInMinutes);
   NFmiColor CalcFlashFillColor(const NFmiColor &theStartColor, const NFmiColor &theEndColor, float theMixRatio);
   void CreateFlashPolyLines(void);
   bool CalcFlashCount(int &theGroundFlashCountOut, int &theCloudFlashCountOut);
   bool PrepareForStationDraw(void) override;
   void DrawFlashes(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);

   NFmiPolyline *itsNegativeFlashPolyLine;
   NFmiPolyline *itsPositiveFlashPolyLine;
   NFmiPolyline *itsCloudFlashPolyLine;
};

