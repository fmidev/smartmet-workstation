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
    NFmiTimeSerialDiscreteDataView (int theMapViewDescTopIndex, const NFmiRect & theRect
								   ,NFmiToolBox * theToolBox
								   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
								   ,int theIndex);
   virtual  ~NFmiTimeSerialDiscreteDataView (void);

   bool LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey) override;
   bool LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey) override;
   bool RightButtonUp(const NFmiPoint & thePlace, unsigned long theKey) override;
   void DrawLocationInTime(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle, bool drawModificationLines) override;
   void CreateValueScale (void) override;
   void DrawValueAxis (void) override;
   using NFmiTimeSerialView::DrawDataLine;
   virtual void DrawDataLine (const NFmiMetTime& theTime1,  const NFmiMetTime& theTime2, double value1, double value2, NFmiDrawingEnvironment & envi);
   virtual void EvaluateValue (double& theValue);
   void ChangeTimeSeriesValues(void) override;

 protected:
	void GetLowAndHighLimits(boost::shared_ptr<NFmiDrawParam> &theDrawParam, float &theLowerLimit, float &theHigherLimit);
	virtual void CreateValueScaleArrays();
	void DrawModifyingUnit(void) override;
	bool ModifyFactorPointsSetValue(int theValue, int theIndex);
	void ResetModifyFactorValues(void) override;
	void CreateModifyFactorScaleView(bool fSetScalesDirectlyWithLimits = false, double theValue = kFloatMissing) override;
	void FixModifyFactorValue (float& theValue) override;
	void DrawModifyFactorPoints(void) override;
	void DrawModifyFactorAxis(void) override;
	void DrawData (void) override;
	void DrawGrids (NFmiDrawingEnvironment & envi) override;
	void DrawValueGrids (NFmiDrawingEnvironment & envi, double minPos, double maxPos);
	void DrawModifyFactorPointGrids (void);
	void DrawSelectedStationData (void);
	void DrawBackground (void);
	void CreateValueScaleView (void) override;
	bool AutoAdjustValueScale(void){return false;};
    bool DrawHelperData() const { return false; }


	 int itsSizeOfArray;
	 NFmiString* itsValueAxisTextArray;
	 double* itsValueAxisNumberArray;


 private:
	 int itsPreviousIndex;
	 bool fCalculateLine;

};

