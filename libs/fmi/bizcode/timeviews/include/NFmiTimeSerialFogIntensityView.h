//**********************************************************
// C++ Class Name : NFmiTimeSerialFogIntensityView 
// ---------------------------------------------------------

//**********************************************************

#pragma once

#include "NFmiTimeSerialDiscreteDataView.h"


class NFmiTimeSerialFogIntensityView : public NFmiTimeSerialDiscreteDataView
{

 public:
    NFmiTimeSerialFogIntensityView (int theMapViewDescTopIndex, const NFmiRect & theRect
										,NFmiToolBox * theToolBox
										,boost::shared_ptr<NFmiDrawParam> &theDrawParam
									    ,int theIndex);
	virtual  ~NFmiTimeSerialFogIntensityView (void);
	virtual void EvaluateValue (double& theValue);

 protected:
	virtual void CreateValueScaleArrays(void);
	void DrawValueGrids (NFmiDrawingEnvironment & envi, double minPos, double maxPos);
	void CreateModifyFactorScaleView(bool fSetScalesDirectlyWithLimits = false, double theValue = kFloatMissing);
	void CreateValueScaleView (void);
};

