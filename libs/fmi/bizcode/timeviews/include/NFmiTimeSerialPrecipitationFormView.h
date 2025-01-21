//**********************************************************
// C++ Class Name : NFmiTimeSerialPrecipitationFormView 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiTimeSerialPrecipitationFormView.h 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 2.1.4 ) 
//  - GD System Name    : Met-editor Plan 3 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : 2. uusi aikasarja viritys 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Tues - Apr 6, 1999 
// 
// 
//  Description: 
// 
//  Change Log: 
// 
//**********************************************************

#pragma once

#include "NFmiTimeSerialDiscreteDataView.h"


class NFmiTimeSerialPrecipitationFormView : public NFmiTimeSerialDiscreteDataView
{

 public:
    NFmiTimeSerialPrecipitationFormView (int theMapViewDescTopIndex, const NFmiRect & theRect
										,NFmiToolBox * theToolBox
										,boost::shared_ptr<NFmiDrawParam> &theDrawParam
									    ,int theIndex);
	virtual  ~NFmiTimeSerialPrecipitationFormView (void);
	virtual void EvaluateValue (double& theValue);

 protected:
	virtual void CreateValueScaleArrays(void);
	void DrawValueGrids (NFmiDrawingEnvironment & envi, double minPos, double maxPos);
	void CreateModifyFactorScaleView(bool fSetScalesDirectlyWithLimits = false, double theValue = kFloatMissing);
	void CreateValueScaleView (void);
};

