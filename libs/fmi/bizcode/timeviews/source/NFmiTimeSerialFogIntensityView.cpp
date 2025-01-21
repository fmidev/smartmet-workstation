//**********************************************************
// C++ Class Name : NFmiTimeSerialFogIntensityView 
// ---------------------------------------------------------

//**********************************************************
#include "NFmiTimeSerialFogIntensityView.h"

#include "NFmiDrawParam.h"
#include "NFmiAxis.h"
#include "NFmiAxisView.h"

//--------------------------------------------------------
// Constructor/Destructor 
//--------------------------------------------------------
NFmiTimeSerialFogIntensityView::NFmiTimeSerialFogIntensityView(int theMapViewDescTopIndex, const NFmiRect & theRect
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

NFmiTimeSerialFogIntensityView::~NFmiTimeSerialFogIntensityView (void)
{
}

//--------------------------------------------------------
// DrawValueGrids 
//--------------------------------------------------------
void NFmiTimeSerialFogIntensityView::DrawValueGrids (NFmiDrawingEnvironment & envi, double minPos, double maxPos)
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
void NFmiTimeSerialFogIntensityView::EvaluateValue (double& theValue)
{
   return;
}

//--------------------------------------------------------
// CreateValueScaleArrays 
//--------------------------------------------------------
void NFmiTimeSerialFogIntensityView::CreateValueScaleArrays(void)
{
	if(itsValueAxisNumberArray)
		delete [] itsValueAxisNumberArray;
	if(itsValueAxisTextArray)
		delete [] itsValueAxisTextArray;

	itsValueAxisNumberArray = new double[3];
	itsValueAxisTextArray = new NFmiString[3];
	
	for(int i = 0; i < 3; i++)
		itsValueAxisNumberArray[i] = i;

	// piti korjata hard koodattuja taulukoita kun fontti muuttui
	itsValueAxisTextArray[0] = NFmiString("");
	itsValueAxisTextArray[1] = NFmiString("©");
	itsValueAxisTextArray[2] = NFmiString("2");

	itsSizeOfArray = 3;

	return;
}

//--------------------------------------------------------
// CreateModifyFactorScaleView 
//--------------------------------------------------------
void NFmiTimeSerialFogIntensityView::CreateModifyFactorScaleView(bool /* fSetScalesDirectlyWithLimits */, double /* theValue */)
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
void NFmiTimeSerialFogIntensityView::CreateValueScaleView (void)
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
