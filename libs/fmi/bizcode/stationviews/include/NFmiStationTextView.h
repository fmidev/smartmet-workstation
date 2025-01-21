#pragma once

#include "NFmiStationView.h"

class NFmiToolBox;
class NFmiLocation;
class NFmiArea;
class NFmiDrawParam;

class NFmiStationTextView : public NFmiStationView
{

public:
    NFmiStationTextView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
        , NFmiToolBox * theToolBox
        , boost::shared_ptr<NFmiDrawParam> &theDrawParam
        , FmiParameterName theParamId
        , NFmiPoint theOffSet
        , NFmiPoint theSize
        , int theRowIndex
        , int theColumnIndex);
   virtual  ~NFmiStationTextView (void);

   bool IsSingleSymbolView(void) override { return false; }; // tulostaa kartalle tekstin pätkän, joten false
protected:
   void			ModifyTextEnvironment (void) override;
   NFmiPoint GetSpaceOutFontFactor(void) override;
   NFmiPoint SbdCalcFixedSymbolSize() const override;

};

