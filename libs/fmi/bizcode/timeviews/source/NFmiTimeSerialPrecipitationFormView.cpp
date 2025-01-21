//**********************************************************
// C++ Class Name : NFmiTimeSerialPrecipitationFormView 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiTimeSerialPrecipitationFormView.cpp 
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
#include "NFmiTimeSerialPrecipitationFormView.h"

#include "NFmiDrawParam.h"
#include "NFmiTitle.h"
#include "NFmiAxis.h"
#include "NFmiAxisView.h"

//--------------------------------------------------------
// Constructor/Destructor 
//--------------------------------------------------------
NFmiTimeSerialPrecipitationFormView::NFmiTimeSerialPrecipitationFormView
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

NFmiTimeSerialPrecipitationFormView::~NFmiTimeSerialPrecipitationFormView (void)
{
}

//--------------------------------------------------------
// DrawValueGrids 
//--------------------------------------------------------
void NFmiTimeSerialPrecipitationFormView::DrawValueGrids (NFmiDrawingEnvironment & envi, double minPos, double maxPos)
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
void NFmiTimeSerialPrecipitationFormView::EvaluateValue (double& theValue)
{
   return;
}

//--------------------------------------------------------
// CreateValueScaleArrays 
//--------------------------------------------------------
void NFmiTimeSerialPrecipitationFormView::CreateValueScaleArrays(void)
{
	if(itsValueAxisNumberArray)
		delete [] itsValueAxisNumberArray;
	if(itsValueAxisTextArray)
		delete [] itsValueAxisTextArray;

	itsValueAxisNumberArray = new double[7];
	itsValueAxisTextArray = new NFmiString[7];
	
	for(int i = 0; i < 7; i++)
		itsValueAxisNumberArray[i] = i;

	// piti korjata hard koodattuja taulukoita kun fontti muuttui
	itsValueAxisTextArray[0] = NFmiString("Ï");
	itsValueAxisTextArray[1] = NFmiString("Ù");
	itsValueAxisTextArray[2] = NFmiString("á");
	itsValueAxisTextArray[3] = NFmiString("ã");
	itsValueAxisTextArray[4] = NFmiString("Õ");
	itsValueAxisTextArray[5] = NFmiString("ß");
	itsValueAxisTextArray[6] = NFmiString("6");

	itsSizeOfArray = 7;

	return;
}

//--------------------------------------------------------
// CreateModifyFactorScaleView 
//--------------------------------------------------------
void NFmiTimeSerialPrecipitationFormView::CreateModifyFactorScaleView(bool /* fSetScalesDirectlyWithLimits */, double /* theValue */)
{
	delete itsModifyFactorAxis;
	delete itsModifyFactorView;
	float lowerLimit= float(-1.)
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
	return;
}

//--------------------------------------------------------
// CreateValueScaleView 
//--------------------------------------------------------
void NFmiTimeSerialPrecipitationFormView::CreateValueScaleView (void)
{
	delete itsValueAxis;
	delete itsValueView;
	float lowLimit = -1.;
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
