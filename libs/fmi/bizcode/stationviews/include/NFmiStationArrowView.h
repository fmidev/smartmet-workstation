//© Ilmatieteenlaitos/software by Marko
//  Original 27.06.2006
// 
//                                  
//Ver. xx.xx.xxxx/Marko
//-------------------------------------------------------------------- NFmiStationArrowView.h

#pragma once

#include "NFmiStationView.h"

//_________________________________________________________ NFmiStationArrowView

class NFmiToolBox;
class NFmiArea;
class NFmiDrawParam;
class NFmiPolyline;

class NFmiStationArrowView : public NFmiStationView
{
public:
	NFmiStationArrowView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
							,NFmiToolBox * theToolBox
							,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							,FmiParameterName theParamId
							,NFmiPoint theOffSet
							,NFmiPoint theSize
							,int theRowIndex
                            ,int theColumnIndex);
    ~NFmiStationArrowView(void);

protected:
	NFmiPoint GetSpaceOutFontFactor(void) override;
	void ModifyTextEnvironment(void) override;
	int GetApproxmationOfDataTextLength(std::vector<float>* sampleValues = nullptr) override;
	NFmiPoint SbdCalcFixedSymbolSize() const override;
	int SbdCalcFixedPenSize() const override;
	NFmiSymbolBulkDrawType SbdGetDrawType() const override;
	float ViewFloatValue(bool doTooltipValue) override;
	NFmiPoint SbdCalcDrawObjectOffset() const override;
	NFmiSymbolColorChangingType SbdGetSymbolColorChangingType() const override;

};

