//© Ilmatieteenlaitos/software by Marko
//  Original 29.09.1998
// 
//                                  
//Ver. xx.xx.xxxx/Marko
//-------------------------------------------------------------------- NFmiStationWindBarbView.h

#pragma once

#include "NFmiStationView.h"

//_________________________________________________________ NFmiStationWindBarbView

class NFmiToolBox;
class NFmiArea;
class NFmiDrawParam;

class NFmiStationWindBarbView : public NFmiStationView
{

public:
    NFmiStationWindBarbView (int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
							,NFmiToolBox * theToolBox
							,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							,FmiParameterName theParamId
							,NFmiPoint theOffSet
							,NFmiPoint theSize
							,int theRowIndex
                            ,int theColumnIndex);
   ~NFmiStationWindBarbView (void);

   void Draw (NFmiToolBox * theGTB) override;

protected:
   bool PrepareForStationDraw(void) override;
   NFmiPoint GetSpaceOutFontFactor(void) override;
   void ModifyTextEnvironment(void) override;
   int GetApproxmationOfDataTextLength(std::vector<float>* sampleValues = nullptr) override;
   float InterpolatedToolTipValue(const NFmiMetTime &theUsedTime, const NFmiPoint& theLatlon, boost::shared_ptr<NFmiFastQueryInfo> &theInfo) override;
   NFmiPoint SbdCalcFixedSymbolSize() const override;
   NFmiSymbolBulkDrawType SbdGetDrawType() const override;
   NFmiPoint SbdCalcFixedRelativeDrawObjectSize() const override;
   int SbdCalcFixedPenSize() const override;
   float ViewFloatValue(bool doTooltipValue) override;
   NFmiPoint SbdCalcDrawObjectOffset() const override;
   NFmiSymbolColorChangingType SbdGetSymbolColorChangingType() const override;

};

