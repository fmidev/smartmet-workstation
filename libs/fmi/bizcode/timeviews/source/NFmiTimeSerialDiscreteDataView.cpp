//**********************************************************
// C++ Class Name : NFmiTimeSerialDiscreteDataView 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiTimeSerialDiscreteDataView.cpp 
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
#include "NFmiTimeSerialDiscreteDataView.h"

#include "NFmiDrawParam.h"
#include "NFmiDrawParamList.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiLine.h"
#include "NFmiToolBox.h"
#include "NFmiGlobals.h"
#include "NFmiAxisView.h"
#include "NFmiStepScale.h"
#include "NFmiAxis.h"
#include "CtrlViewDocumentInterface.h"
#include "NFmiRectangle.h"
#include "NFmiText.h"
#include "CtrlViewFastInfoFunctions.h"
#include "EditedInfoMaskHandler.h"

#include <boost\math\special_functions\round.hpp>

//--------------------------------------------------------
// Constructor/Destructor 
//--------------------------------------------------------
NFmiTimeSerialDiscreteDataView::NFmiTimeSerialDiscreteDataView (int theMapViewDescTopIndex, const NFmiRect & theRect
															   ,NFmiToolBox * theToolBox
															   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
															   ,int theIndex)
:NFmiTimeSerialView(theMapViewDescTopIndex, theRect
				  ,theToolBox
				  ,theDrawParam
				  ,theIndex)
,itsValueAxisTextArray(0)
,itsValueAxisNumberArray(0)
,fCalculateLine(false)
{
}

NFmiTimeSerialDiscreteDataView::~NFmiTimeSerialDiscreteDataView (void)
{
	if(itsValueAxisNumberArray)
		delete [] itsValueAxisNumberArray;
	
	if(itsValueAxisTextArray)
		delete [] itsValueAxisTextArray;
}

//--------------------------------------------------------
// LeftButtonUp 
//--------------------------------------------------------
bool NFmiTimeSerialDiscreteDataView::LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey)
{
	if(IsIn(thePlace))
	{
		itsCtrlViewDocumentInterface->TimeSerialViewDirty(true);
		// kun monta alinäyttöä yhtäaikaa, pitää ensimmäisellä klikkauksella asettaa kyseinen näyttö 'editointitilaan'
		if(itsDrawParam && (!itsDrawParam->IsParamEdited()))
		{
            itsCtrlViewDocumentInterface->TimeSerialViewDrawParamList()->DisableEditing();
			itsDrawParam->EditParam(true);
			return true;
		}
		{
			// kuinka läheltä pitää aikaakselia klikata ennenkuin ohjelma suostuu 'löytämään' klikkauksen paikan
			const double proximityFactor = 0.2; 
			int index;
			if(FindTimeIndex(thePlace.X(), proximityFactor, index))
			{
				auto value = Position2ModifyFactor(thePlace);
				FixModifyFactorValue(value);

				return ModifyFactorPointsSetValue(boost::math::iround(value), index) == true;	// Vain yksi tapa käsitellä aikasarjaa. M.K.
			}
		}
	}
	return false;
}
bool NFmiTimeSerialDiscreteDataView::LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey)
{ // tämä pitää määrittää uusien hiiri ominaisuuksien takia tänne paaluttamaan falsea
	return false;
}

//--------------------------------------------------------
// ModifyFactorPointsSetValue 
//--------------------------------------------------------
bool NFmiTimeSerialDiscreteDataView::ModifyFactorPointsSetValue(int theValue, int theIndex)
{
	if(fCalculateLine)
	{
		int index = theIndex;
		if(itsPreviousIndex > index)
		{
			index = itsPreviousIndex;
			itsPreviousIndex = theIndex;
		}

		for(int i = itsPreviousIndex; i <= index; i++)
			itsModificationFactorCurvePoints[i] = (float)theValue;		// 0 = drizzle, 1 = rain, 2 = sleet, 3 =snow, 4 = freezing
																 
		fCalculateLine = false;
	}
	else
	{
		fCalculateLine = true;
		itsPreviousIndex = theIndex;
	}
	return true;
}

//--------------------------------------------------------
// RightButtonUp 
//--------------------------------------------------------
bool NFmiTimeSerialDiscreteDataView::RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey)
{
	if(IsIn(thePlace))
	{
        itsCtrlViewDocumentInterface->TimeSerialViewDirty(true);
		// kun monta alinäyttöä yhtäaikaa, pitää ensimmäisellä klikkauksella asettaa kyseinen näyttö 'editointitilaan'
		if(itsDrawParam && (!itsDrawParam->IsParamEdited()))
		{
            itsCtrlViewDocumentInterface->TimeSerialViewDrawParamList()->DisableEditing();
			itsDrawParam->EditParam(true);
			return true;
		}
	}
	return false;
}

//--------------------------------------------------------
// ResetModifyFactorValues 
//--------------------------------------------------------
void NFmiTimeSerialDiscreteDataView::ResetModifyFactorValues(void)
{
	for(unsigned int i=0; i < itsModificationFactorCurvePoints.size(); i++)
		itsModificationFactorCurvePoints[i] = -1;
	return;
}

void NFmiTimeSerialDiscreteDataView::GetLowAndHighLimits(boost::shared_ptr<NFmiDrawParam> &theDrawParam, float &theLowerLimit, float &theHigherLimit)
{
	theLowerLimit = -1;
	FmiParameterName parName = static_cast<FmiParameterName>(theDrawParam->Param().GetParamIdent());
	if(parName == kFmiPrecipitationForm)
		theHigherLimit = 8;
    else if(parName == kFmiPotentialPrecipitationForm)
        theHigherLimit = 8;
    else if(parName == kFmiFogIntensity)
		theHigherLimit = 3;
	else if(parName == kFmiPrecipitationType)
		theHigherLimit = 3;
	else
		theHigherLimit = 10;
}

//--------------------------------------------------------
// CreateModifyFactorScaleView 
//--------------------------------------------------------
void NFmiTimeSerialDiscreteDataView::CreateModifyFactorScaleView(bool /* fSetScalesDirectlyWithLimits */, double /* theValue */)
{
	delete itsModifyFactorAxis;
	delete itsModifyFactorView;
	float lowerLimit=0
		, upperLimit=1;
	GetLowAndHighLimits(itsDrawParam, lowerLimit, upperLimit);
	NFmiStepScale scale(lowerLimit,upperLimit, 1./*,testScale.StepValue()*/);	// Askelväli 1. M.K. 10.5.99
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
// FixModifyFactorValue 
//--------------------------------------------------------
void NFmiTimeSerialDiscreteDataView::FixModifyFactorValue (float& theValue)
{
	if(theValue > itsDrawParam->AbsoluteMaxValue())
		theValue = (float)itsDrawParam->AbsoluteMaxValue();
	if(theValue < itsDrawParam->AbsoluteMinValue())
		theValue = (float)itsDrawParam->AbsoluteMinValue();
	return;
}

//--------------------------------------------------------
// DrawModifyFactorPoints 
//--------------------------------------------------------
void NFmiTimeSerialDiscreteDataView::DrawModifyFactorPoints(void)
{
	DrawModifyFactorPointsManual();		// Vain yksi tapa muokata dataa.
	return;
}


//--------------------------------------------------------
// ChangeTimeSeriesValues 
//--------------------------------------------------------
void NFmiTimeSerialDiscreteDataView::ChangeTimeSeriesValues(void)
// HUOM! POIKKEAVATKO NÄMÄ VIRTUAALI FUNKTIOT TOISISTAAN
{
	if(IsModifyFactorValuesNonZero())
	{
// HUOM!! muuta docin funktion rajapintaa!!!!!
        bool status = itsCtrlViewDocumentInterface->DoTimeSeriesValuesModifying(itsDrawParam, NFmiMetEditorTypes::kFmiSelectionMask, itsEditedDataTimeDescriptor, itsModificationFactorCurvePoints, NFmiMetEditorTypes::kFmiTimeSerialModification, true, -1);
		if(status)
			ResetModifyFactorValues();
	}
}


//--------------------------------------------------------
// DrawData 
//--------------------------------------------------------
void NFmiTimeSerialDiscreteDataView::DrawData (void)
{
 	DrawSelectedStationData();
	DrawModifyFactorPoints();
	DrawTimeLine();
	return;
}

//--------------------------------------------------------
// DrawGrids 
//--------------------------------------------------------
void NFmiTimeSerialDiscreteDataView::DrawGrids (NFmiDrawingEnvironment & envi)
{
	NFmiRect valueRect = CalcValueAxisRect();
	CalcTimeAxisRect();
	double pixels = itsToolBox->GetDravingRect().Height();
	double pixelsperline = 15;
	double lineperview = pixels/pixelsperline;
	double lineperunit = lineperview/itsRect.Height();
	envi.SetSubLinePerUnit(lineperunit);
	DrawTimeGrids(envi,valueRect.Bottom(),valueRect.Top());
	pixels = itsToolBox->GetDravingRect().Width();
	lineperview = pixels/pixelsperline;
	lineperunit = lineperview/itsRect.Width();
	envi.SetSubLinePerUnit(lineperunit);
    return;
}

//--------------------------------------------------------
// DrawValueGrids 
//--------------------------------------------------------
void NFmiTimeSerialDiscreteDataView::DrawValueGrids (NFmiDrawingEnvironment & envi, double minPos, double maxPos)
{
   return;
}
//--------------------------------------------------------
// DrawModifyFactorPointGrids 
//--------------------------------------------------------
void NFmiTimeSerialDiscreteDataView::DrawModifyFactorPointGrids (void)
{
	NFmiDrawingEnvironment envi;
	envi.DisableFill();
	envi.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	NFmiTimeBag timeBag = ZoomedTimeBag();
	NFmiPoint point1(CalcRelativeModifyFactorPosition(timeBag.FirstTime(),0))
			 ,point2(CalcRelativeModifyFactorPosition(timeBag.LastTime(),0));
	NFmiLine line(point1, point2, 0, &envi);
	itsToolBox->Convert(&line);
	return;
}

//--------------------------------------------------------
// DrawSelectedStationData 
//--------------------------------------------------------
void NFmiTimeSerialDiscreteDataView::DrawSelectedStationData (void)
{
    NFmiTimeSerialView::DrawSelectedStationData();
}

//--------------------------------------------------------
// DrawBackground 
//--------------------------------------------------------

void NFmiTimeSerialDiscreteDataView::DrawBackground (void)
{
	NFmiDrawingEnvironment envi;
	envi.EnableFill();
	envi.EnableFrame();
	envi.SetFillColor(NFmiColor(1.f,1.f,0.97f));
	if(itsDrawParam && itsDrawParam->IsParamEdited())
	{
		envi.SetPenSize(NFmiPoint(2.,2.));
		envi.SetFrameColor(NFmiColor(1.f,0.f,0.f));
	}
	else
	{
		envi.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	}
	NFmiRectangle rect(itsRect.TopLeft()
					  ,itsRect.BottomRight()
					  ,0
					  ,&envi);
	itsToolBox->Convert(&rect);
	return;
}

//--------------------------------------------------------
// CreateValueScaleView 
//--------------------------------------------------------
void NFmiTimeSerialDiscreteDataView::CreateValueScaleView (void)
{
	delete itsValueAxis;
	delete itsValueView;
	float lowLimit = -1;
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

//void NFmiTimeSerialDiscreteDataView::DrawLocationInTime()
void NFmiTimeSerialDiscreteDataView::DrawLocationInTime(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle, bool drawModificationLines)
{
	double realValue;
	NFmiMetTime time1, time;

	int step = (int)(ZoomedTimeBag().Resolution())/2;

	itsInfo->ResetTime();
	itsInfo->NextTime();
	time1 = itsInfo->Time();

	//erikseen eka aika
	time = itsInfo->Time();
	time.SetTimeStep(step);
	time.ChangeByMinutes(step);
	realValue = itsInfo->InterpolatedValue(theLatLonPoint);
	DrawDataLine(time1, time, realValue, realValue, theCurrentDataLineStyle);

	long timeCount = itsInfo->SizeTimes();
	for(long i=1; i < timeCount-1; i++) 
	{
		time1 = time;
		itsInfo->NextTime();
		time = itsInfo->Time();
		time.SetTimeStep(step);
		time.ChangeByMinutes(step);
		realValue = itsInfo->InterpolatedValue(theLatLonPoint);
		DrawDataLine(time1, time, realValue, realValue, theCurrentDataLineStyle);
	}

	//viimeinen aika
	time1 = time;
	itsInfo->NextTime();
	time = itsInfo->Time();
	time.SetTimeStep(step);
	realValue = itsInfo->InterpolatedValue(theLatLonPoint);
	DrawDataLine(time1, time, realValue, realValue, theCurrentDataLineStyle);
	return;
}
//--------------------------------------------------------
// CreateValueScale 
//--------------------------------------------------------
void NFmiTimeSerialDiscreteDataView::CreateValueScale (void)
{
	CreateValueScaleView();
}

//--------------------------------------------------------
// DrawValueAxis 
//--------------------------------------------------------
void NFmiTimeSerialDiscreteDataView::DrawValueAxis (void)
{
	bool oldClippingState = itsToolBox->UseClipping();
	itsToolBox->UseClipping(false);
	NFmiDrawingEnvironment envi;

	envi.SetFrameColor(NFmiColor(0.5f,0.5f,0.5f));
	envi.DisableFrame();
	NFmiRect cat = CalcValueAxisRect();
	cat.Inflate(0.,-itsToolBox->SY(1));
	NFmiRectangle rectangle(cat.TopLeft()
					  ,cat.BottomRight()
					  ,0
					  ,&envi);
	itsToolBox->Convert(&rectangle);
	itsValueView->DrawAll();

	envi.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	envi.EnableFrame();
	envi.SetFontType(kSynop);
	envi.BoldFont(true);
	envi.SetFontSize(NFmiPoint(36, 36));
	itsToolBox->SetTextAlignment(kTopLeft);
	double fontHeight = itsToolBox->SY(envi.GetFontHeight());
	for(int i = 0; i < itsSizeOfArray && i <= 10; i++) // Kirjoittaa arvoakselin tekstit
	{
		NFmiPoint place = NFmiPoint((cat.Left() + 0), Value2AxisPosition(float(itsValueAxisNumberArray[i])) - fontHeight/2.); 
		NFmiText text(place, itsValueAxisTextArray[i], false, 0, &envi);
		itsToolBox->Convert(&text);
	}	
	
	itsToolBox->UseClipping(oldClippingState);

	return;
}

//--------------------------------------------------------
// DrawModifyFactorAxis 
//--------------------------------------------------------
void NFmiTimeSerialDiscreteDataView::DrawModifyFactorAxis(void)
{
	NFmiDrawingEnvironment envi;

	envi.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	envi.DisableFrame();
	NFmiRect cat = CalcModifyFactorAxisRect();
	cat.Inflate(0.,-itsToolBox->SY(1));
	NFmiRectangle rectangle(cat.TopLeft()
					  ,cat.BottomRight()
					  ,0
					  ,&envi);
	itsToolBox->Convert(&rectangle);
	itsModifyFactorView->DrawAll();
	envi.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	envi.EnableFrame();
	envi.SetFontType(kSynop);
	envi.BoldFont(true);
	envi.SetFontSize(NFmiPoint(36, 36));
	double moveFont = itsToolBox->SX(10);
	itsToolBox->SetTextAlignment(kTopLeft);
	double fontHeight = itsToolBox->SY(envi.GetFontHeight());

	for(int i = 0; i < itsSizeOfArray && i <= 10; i++) // Kirjoittaa arvoakselin tekstit
	{
		NFmiPoint place = NFmiPoint(cat.Left() - moveFont, Value2AxisPosition(float(itsValueAxisNumberArray[i])) - fontHeight/2.); 
		NFmiText text(place, itsValueAxisTextArray[i], false, 0, &envi);
		itsToolBox->Convert(&text);
	}	

	return;
}

//--------------------------------------------------------
// DrawModifyingUnit
//--------------------------------------------------------
void NFmiTimeSerialDiscreteDataView::DrawModifyingUnit(void)
{
	return;
}

void NFmiTimeSerialDiscreteDataView::DrawDataLine (const NFmiMetTime& theTime1,  const NFmiMetTime& theTime2, double value1, double /* value2 */, NFmiDrawingEnvironment & envi)
{
	if(value1 == kFloatMissing)	
		return;
	
	bool isInAxis = ValueInsideValueAxis(value1);
	NFmiPoint point1 = CalcRelativeValuePosition(theTime1, value1)
			 ,point2 = CalcRelativeValuePosition(theTime2, value1);
	if(!isInAxis)
	{
		if(!CheckIsPointsDrawable(point1, isInAxis, point2, isInAxis, true))
			return;
	}

	NFmiLine line(point1, point2, 0, &envi);
	itsToolBox->Convert(&line);
}

//--------------------------------------------------------
// EvaluateValue 
//--------------------------------------------------------

//   Asettaa uuden arvon datalle muutoskäyrää 
//   piirrettäessä. Esim. kok.pilv. 90 + 10 -> 
//   100
//   ja WD 350 + 20 -> 10, jne.
void NFmiTimeSerialDiscreteDataView::EvaluateValue (double& theValue)
{
   return;
}

//--------------------------------------------------------
// CreateValueScaleArrays 
//--------------------------------------------------------
void NFmiTimeSerialDiscreteDataView::CreateValueScaleArrays(void)
{
	return;	
}
