//**********************************************************
// C++ Class Name : NFmiTimeSerialDiscreteDataView 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiTimeSerialDiscreteDataView.h 
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

#include "NFmiTimeSerialView.h"

class NFmiTimeSerialDiscreteDataView : public NFmiTimeSerialView
{

 public:
    NFmiTimeSerialDiscreteDataView (const NFmiRect & theRect
								   ,NFmiToolBox * theToolBox
								   ,NFmiDrawingEnvironment * theDrawingEnvi
								   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
								   ,int theIndex
								   ,double theManualModifierLength /* = 1. */);
   virtual  ~NFmiTimeSerialDiscreteDataView (void);

   virtual bool LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey);
   virtual bool LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey);
   virtual bool RightButtonUp(const NFmiPoint & thePlace, unsigned long theKey);
   virtual void DrawLocationInTime(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle);
   virtual void CreateValueScale (void);
   virtual void DrawValueAxis (void);
   using NFmiTimeSerialView::DrawDataLine;
   virtual void DrawDataLine (const NFmiMetTime& theTime1,  const NFmiMetTime& theTime2, double value1, double value2, NFmiDrawingEnvironment & envi);
   virtual void EvaluateValue (double& theValue);
    virtual void EditingMode (int newMode);
	virtual void ChangeTimeSeriesValues(void);

 protected:
	void GetLowAndHighLimits(boost::shared_ptr<NFmiDrawParam> &theDrawParam, float &theLowerLimit, float &theHigherLimit);
	virtual void CreateValueScaleArrays();
	virtual void DrawModifyingUnit(void);
	bool ModifyFactorPointsSetValue(int theValue, int theIndex);
	virtual void ResetModifyFactorValues(void);
	virtual void CreateModifyFactorScaleView(bool fSetScalesDirectlyWithLimits = false, double theValue = kFloatMissing);
	virtual void FixModifyFactorValue (double & theValue);
	virtual void DrawModifyFactorPoints(void);
	virtual void DrawModifyFactorAxis(void);
	virtual void DrawData (void);
	virtual void DrawGrids (NFmiDrawingEnvironment & envi);
	void DrawValueGrids (NFmiDrawingEnvironment & envi, double minPos, double maxPos);
	void DrawModifyFactorPointGrids (void);
	void DrawSelectedStationData (void);
	void DrawBackground (void);
	virtual void CreateValueScaleView (void);
	bool AutoAdjustValueScale(void){return false;};


	 int itsSizeOfArray;
	 NFmiString* itsValueAxisTextArray;
	 double* itsValueAxisNumberArray;


 private:
	 int itsPreviousIndex;
	 bool fCalculateLine;

};

