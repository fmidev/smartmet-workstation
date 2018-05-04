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
								 ,NFmiDrawingEnvironment * theDrawingEnvi
								 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
								 ,FmiParameterName theParamId
								 ,NFmiPoint theOffSet
								 ,NFmiPoint theSize
                                 ,int theRowIndex
                                 ,int theColumnIndex);
   virtual  ~NFmiStationSimpleWeatherView (void);
   void Draw(NFmiToolBox * theGTB);

protected:
   void DrawData (void);
   float ViewFloatValue(void) override;
   bool PrepareForStationDraw(void) override;
   NFmiPoint GetSpaceOutFontFactor(void); 
   void ModifyTextEnvironment(void);

private:
   void DrawSymbol (void);


};

