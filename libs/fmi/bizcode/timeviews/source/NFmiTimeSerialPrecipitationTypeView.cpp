//**********************************************************
// C++ Class Name : NFmiTimeSerialPrecipitationTypeView 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiTimeSerialPrecipitationTypeView.cpp 
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
#include "NFmiTimeSerialPrecipitationTypeView.h"

#include "NFmiDrawParam.h"
#include "NFmiTitle.h"
#include "NFmiAxis.h"
#include "NFmiAxisView.h"

//--------------------------------------------------------
// Constructor/Destructor 
//--------------------------------------------------------
NFmiTimeSerialPrecipitationTypeView::NFmiTimeSerialPrecipitationTypeView
															   (int theMapViewDescTopIndex, const NFmiRect & theRect
															   ,NFmiToolBox * theToolBox
															   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
															   ,int theIndex)
:NFmiTimeSerialDiscreteDataView(theMapViewDescTopIndex, theRect
							   ,theToolBox
							   ,theDrawParam
							   ,theIndex)
{
	ResetModifyFactorValues();
}
 NFmiTimeSerialPrecipitationTypeView::~NFmiTimeSerialPrecipitationTypeView (void)
{
}

//--------------------------------------------------------
// DrawValueGrids 
//--------------------------------------------------------
void NFmiTimeSerialPrecipitationTypeView::DrawValueGrids (NFmiDrawingEnvironment & envi, double minPos, double maxPos)
{
   return;
}
//--------------------------------------------------------
// EvaluateValue 
//--------------------------------------------------------

//   Asettaa uuden arvon datalle muutoskäyrää 
//   piirrettäessä. Esim. kok.pilv. 90 + 10 -> 
//   100
//   ja WD 350 + 20 -> 10, jne.
void NFmiTimeSerialPrecipitationTypeView::EvaluateValue (double& theValue)
{
   return;
}

//--------------------------------------------------------
// CreateValueScaleArrays 
//--------------------------------------------------------
void NFmiTimeSerialPrecipitationTypeView::CreateValueScaleArrays(void)
{
	if(itsValueAxisNumberArray)
		delete [] itsValueAxisNumberArray;
	if(itsValueAxisTextArray)
		delete [] itsValueAxisTextArray;

	itsValueAxisNumberArray = new double[2];
	itsValueAxisTextArray = new NFmiString[2];
	
	for(int i = 0; i < 2; i++)
		itsValueAxisNumberArray[i] = i + 1;

	// piti korjata hard koodattuja taulukoita kun fontti muuttui
	itsValueAxisTextArray[0] = NFmiString("Ù");
	itsValueAxisTextArray[1] = NFmiString("í");

	itsSizeOfArray = 2;

	return;
}


//--------------------------------------------------------
// CreateModifyFactorScaleView 
//--------------------------------------------------------
void NFmiTimeSerialPrecipitationTypeView::CreateModifyFactorScaleView(bool /* fSetScalesDirectlyWithLimits */, double /* theValue */)
{
	delete itsModifyFactorAxis;
	delete itsModifyFactorView;
	float lowerLimit= float(0.)
		, upperLimit= 1;
	GetLowAndHighLimits(itsDrawParam, lowerLimit, upperLimit);
	NFmiStepScale scale(lowerLimit,upperLimit, 1.);	// Askelväli 1.
	itsModifyFactorAxis = new NFmiAxis(scale, NFmiTitle("Y"));

	itsModifyFactorView = new NFmiAxisView(CalcModifyFactorAxisRect()
                                    ,itsToolBox
                                    ,itsModifyFactorAxis
                                    ,kDown
                                    ,kLeft
                                    ,true
                                    ,false
                                    ,false
                                    ,0.1f);

	CreateValueScaleArrays();

	return;
}

//--------------------------------------------------------
// CreateValueScaleView 
//--------------------------------------------------------
void NFmiTimeSerialPrecipitationTypeView::CreateValueScaleView (void)
{
	delete itsValueAxis;
	delete itsValueView;
	float lowLimit = 0.;
	float highLimit = 1;
	GetLowAndHighLimits(itsDrawParam, lowLimit, highLimit);

	NFmiStepScale scale((float)lowLimit,(float)highLimit, 1.);	// Askelväli 1.
	itsValueAxis = new NFmiAxis(scale, NFmiTitle("Y"));
	itsValueView = new NFmiAxisView(CalcValueAxisRect()
                                    ,itsToolBox
                                    ,itsValueAxis
                                    ,kDown
                                    ,kLeft
                                    ,true
                                    ,false
                                    ,false
                                    ,0.1f);
	CreateValueScaleArrays();
	return;
}
