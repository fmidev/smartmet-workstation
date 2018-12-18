//© Ilmatieteenlaitos/software by Marko
//  Original 29.09.1998
// 
//                                  
//Ver. xx.xx.xxxx/Marko
//-------------------------------------------------------------------- NFmiStationWindBarbView.h

#pragma once

#include "NFmiStationView.h"
#include "NFmiFastInfoUtils.h"

//_________________________________________________________ NFmiStationWindBarbView

class NFmiToolBox;
class NFmiArea;
class NFmiDrawParam;

class NFmiStationWindBarbView : public NFmiStationView
{

public:
    NFmiStationWindBarbView (int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
							,NFmiToolBox * theToolBox
							,NFmiDrawingEnvironment * theDrawingEnvi
							,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							,FmiParameterName theParamId
							,NFmiPoint theOffSet
							,NFmiPoint theSize
							,int theRowIndex
                            ,int theColumnIndex);
    virtual  ~NFmiStationWindBarbView (void);

   void Draw (NFmiToolBox * theGTB);

protected:
   bool PrepareForStationDraw(void) override;
   void DrawData (void);
   float ViewFloatValue(void) override;
   NFmiPoint GetSpaceOutFontFactor(void);
   void ModifyTextEnvironment(void);
   int GetApproxmationOfDataTextLength(void);
   float InterpolatedToolTipValue(const NFmiMetTime &theUsedTime, const NFmiPoint& theLatlon, boost::shared_ptr<NFmiFastQueryInfo> &theInfo) override;

private:
   void DrawSymbol (void);
   bool GetDataFromLocalInfo() const;


   NFmiFastInfoUtils::MetaWindParamUsage metaWindParamUsage;
};

