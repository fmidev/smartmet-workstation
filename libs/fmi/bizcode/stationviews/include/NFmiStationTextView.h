#pragma once

#include "NFmiStationView.h"

class NFmiToolBox;
class NFmiLocation;
class NFmiArea;
class NFmiDrawParam;

class NFmiStationTextView : public NFmiStationView
{

public:
    NFmiStationTextView(int theMapViewDescTopIndex, std::shared_ptr<NFmiArea> &theArea
        , NFmiToolBox * theToolBox
        , std::shared_ptr<NFmiDrawParam> &theDrawParam
        , FmiParameterName theParamId
        , NFmiPoint theOffSet
        , NFmiPoint theSize
        , int theRowIndex
        , int theColumnIndex);
   virtual  ~NFmiStationTextView (void);

   bool IsSingleSymbolView(void) override { return false; }; // tulostaa kartalle tekstin pätkän, joten false
protected:
   void			ModifyTextEnvironment (void) override;
   NFmiPoint SbdCalcFixedSymbolSize() const override;

};

