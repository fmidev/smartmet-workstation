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
							,NFmiDrawingEnvironment * theDrawingEnvi
							,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							,FmiParameterName theParamId
							,NFmiPoint theOffSet
							,NFmiPoint theSize
							,int theRowIndex
                            ,int theColumnIndex);
    virtual ~NFmiStationArrowView(void);

	void Draw(NFmiToolBox * theGTB);

protected:
	void DrawData(void);
	NFmiPoint GetSpaceOutFontFactor(void); 
	void ModifyTextEnvironment(void);
	int GetApproxmationOfDataTextLength(void);

private:
	void DrawSymbol(void);

	NFmiPolyline *itsArrowSymbol;
};

