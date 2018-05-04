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
        , NFmiDrawingEnvironment * theDrawingEnvi
        , boost::shared_ptr<NFmiDrawParam> &theDrawParam
        , FmiParameterName theParamId
        , NFmiPoint theOffSet
        , NFmiPoint theSize
        , int theRowIndex
        , int theColumnIndex);
   virtual  ~NFmiStationTextView (void);

   bool IsSingleSymbolView(void) { return false; }; // tulostaa kartalle tekstin pätkän, joten false
protected:
   void			DrawData (void);
   void			DrawText2(void);
   virtual NFmiColor	CalcTextColor (void);
   void			ModifyTextEnvironment (void);
   NFmiPoint GetSpaceOutFontFactor(void);

};

