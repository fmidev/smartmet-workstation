//© Ilmatieteenlaitos/software by Marko
//  Original 24.09.1998
// 
//                                  
//Ver. xx.xx.xxxx/Marko
//-------------------------------------------------------------------- NFmiStationSimpleWeatherView.h

#pragma once

#include "NFmiStationView.h"

//_________________________________________________________ NFmiStationSimpleWeatherView

class NFmiToolBox;
class NFmiArea;
class NFmiDrawParam;

class NFmiStationSimpleWeatherView : public NFmiStationView
{

public:
    NFmiStationSimpleWeatherView (int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
								 ,NFmiToolBox * theToolBox
								 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
								 ,FmiParameterName theParamId
								 ,NFmiPoint theOffSet
								 ,NFmiPoint theSize
                                 ,int theRowIndex
                                 ,int theColumnIndex);
   ~NFmiStationSimpleWeatherView (void);
   void Draw(NFmiToolBox * theGTB) override;

protected:
   float ViewFloatValue(bool doTooltipValue) override;
   bool PrepareForStationDraw(void) override;
   NFmiPoint GetSpaceOutFontFactor(void) override;
   void ModifyTextEnvironment(void) override;
   NFmiPoint SbdCalcFixedSymbolSize() const override;
   NFmiSymbolBulkDrawType SbdGetDrawType() const override;
   NFmiSymbolColorChangingType SbdGetSymbolColorChangingType() const override;
   NFmiPoint SbdCalcDrawObjectOffset() const override;
   NFmiPoint SbdCalcFixedRelativeDrawObjectSize() const override;

};

