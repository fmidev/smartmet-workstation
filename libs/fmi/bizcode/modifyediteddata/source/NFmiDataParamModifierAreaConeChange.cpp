//**********************************************************
// C++ Class Name : NFmiDataParamModifierAreaConeChange 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiDataParamModifierAreaConeChange.cpp 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 2.1.4 ) 
//  - GD System Name    : Met-editor Plan 2 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : dataparam modification and masks 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Fri - Feb 26, 1999 
// 
// 
//  Description: 
// 
//  Change Log: 
// 
//**********************************************************
#include "NFmiDataParamModifierAreaConeChange.h"

#include "NFmiSmartInfo.h"
#include "NFmiArea.h"
#include "NFmiDrawParam.h"
#include "NFmiAreaMaskList.h"

//--------------------------------------------------------
// Constructor/Destructor 
//--------------------------------------------------------
NFmiDataParamModifierAreaConeChange::NFmiDataParamModifierAreaConeChange(boost::shared_ptr<NFmiFastQueryInfo> theInfo
																	,boost::shared_ptr<NFmiDrawParam> &theDrawParam
																	,boost::shared_ptr<NFmiAreaMaskList> &theMaskList
																	,const boost::shared_ptr<NFmiArea> &theZoomedArea
																	,const boost::shared_ptr<NFmiArea> &theOriginalArea
																	,const NFmiPoint& theLatLon
																	,double theRadius, double theFactor
																	,NFmiMetEditorTypes::Mask theAreaMask
																	,int theLimitingOption
																	,float theLimitngValue)
:NFmiDataParamModifier(theInfo, theDrawParam, theMaskList, theAreaMask, NFmiRect())
,itsModifyingCenter(theLatLon)
,itsModificationRange(theRadius)
,itsModifyFactor(theFactor)
,itsZoomedArea(theZoomedArea)
,itsOriginalArea(theOriginalArea)
,itsRelativeCheckRect()
,itsLimitingOption(theLimitingOption)
,itsLimitngValue(theLimitngValue)
,fCircularValue(false)
,itsCircularValueModulor(kFloatMissing)
{
	if(itsZoomedArea)
		itsModifyingCenterInXY = itsZoomedArea->ToXY(itsModifyingCenter);
}

NFmiDataParamModifierAreaConeChange::~NFmiDataParamModifierAreaConeChange (void)
{
}
//--------------------------------------------------------
// Calculate 
//--------------------------------------------------------
double NFmiDataParamModifierAreaConeChange::Calculate (const double& theValue)
{
	double value;
	
	double distance = CalcDistance();

	if(distance < itsModificationRange)
	{
		if(theValue != kFloatMissing)
			value = theValue + itsModifyFactor * (1 - (distance/itsModificationRange));
		else
			value = itsModifyFactor * (1 - (distance/itsModificationRange));
	}
	else
	{
		return theValue;
	}

	return FmiMin(FmiMax(value, itsDrawParam->AbsoluteMinValue()), itsDrawParam->AbsoluteMaxValue());
}

double NFmiDataParamModifierAreaConeChange::CalcDistance(void)
{
	NFmiPoint currentXYPosition = itsZoomedArea->ToXY(itsInfo->LatLon());
	return itsModifyingCenterInXY.Distance(currentXYPosition);
}

// 1999.11.17/Marko ModifyData2 k‰ytt‰‰
// laskee rectin, jonka sis‰‰n pensseli mahtuu ja muuttaa sen dimensiot vastaamaan 0,0 - 1,1 aluetta
// ottaen huomioon mm. zooomauksen ja monen ruudun n‰ytˆn,
// koska queryinfon relativepoint-metodi palauttaa relatiivisen paikan 0,0 - 1,1 avaruudessa
void NFmiDataParamModifierAreaConeChange::PrepareFastIsInsideData(void)
{
	// itsOriginalArea pit‰‰ tarkistaa, jos muokataan piste dataa, on original area 0-pointteri
    NFmiRect zoomedRect(itsOriginalArea ? itsOriginalArea->XYArea(itsZoomedArea.get()) : NFmiRect(0,0,1,1));
	NFmiPoint size(itsModificationRange*2/itsZoomedArea->Width() * zoomedRect.Width(), itsModificationRange*2/itsZoomedArea->Height() * zoomedRect.Height());
	itsRelativeCheckRect.Size(size);
	NFmiPoint center((itsModifyingCenterInXY.X() - itsZoomedArea->Left())/itsZoomedArea->Width(), (itsModifyingCenterInXY.Y() - itsZoomedArea->Top())/itsZoomedArea->Height());
	center.X(center.X() * zoomedRect.Width() + zoomedRect.Left());
	center.Y(center.Y() * zoomedRect.Height() + zoomedRect.Top());
	itsRelativeCheckRect.Center(center);

	// viritys tuulen suunnalle t‰h‰n, pit‰isi olla oma virtuaali funktio
	fCircularValue = itsDrawParam->Param().GetParamIdent() == kFmiWindDirection;
	itsCircularValueModulor = fCircularValue ? 360 : kFloatMissing;

}
// 1999.11.17/Marko ModifyData2 k‰ytt‰‰ 
bool NFmiDataParamModifierAreaConeChange::IsPossibleInside(const NFmiPoint& theRelativePlace)
{
	return itsRelativeCheckRect.IsInside(NFmiPoint(theRelativePlace.X(), 1 - theRelativePlace.Y()));
}

double NFmiDataParamModifierAreaConeChange::FixCircularValues(double theValue)
{
	if(fCircularValue && theValue != kFloatMissing)
	{
		if(theValue < 0)
			return itsCircularValueModulor - fmod(-theValue, itsCircularValueModulor);
		else
			return fmod(theValue, itsCircularValueModulor);
	}
	return theValue;
}

// 1999.11.17/Marko ModifyData2 k‰ytt‰‰
double NFmiDataParamModifierAreaConeChange::Calculate2(const double& theValue)
{
	double value;
	
	double distance = CalcDistance();

	if(distance < itsModificationRange)
	{
		if(itsParamMaskList->IsMasked(itsInfo->LatLon()))
		{
			if(theValue != kFloatMissing)
				value = theValue + itsModifyFactor * (1 - (distance/itsModificationRange));
			else
				value = itsModifyFactor * (1 - (distance/itsModificationRange));
		}
		else
			return theValue;
	}
	else
		return theValue;

	value = FixCircularValues(value); // ensin tehd‰‰n circular (esim. tuulen suunta) tarkistus ja sitten vasta min/max

	if(itsLimitingOption == 1) // ei alle
		value = FmiMax(value, static_cast<double>(itsLimitngValue));
	else if(itsLimitingOption == 2) // ei yli
		value = FmiMin(value, static_cast<double>(itsLimitngValue));
	
	return FmiMin(FmiMax(value, itsDrawParam->AbsoluteMinValue()), itsDrawParam->AbsoluteMaxValue());
}


// ***********************************************************************************
// *********** NFmiDataParamModifierAreaCircleSetValue *******************************
// ***********************************************************************************

NFmiDataParamModifierAreaCircleSetValue::NFmiDataParamModifierAreaCircleSetValue(
											 boost::shared_ptr<NFmiFastQueryInfo> theInfo 
											,boost::shared_ptr<NFmiDrawParam> &theDrawParam
											,boost::shared_ptr<NFmiAreaMaskList> &theMaskList
											,boost::shared_ptr<NFmiArea> &theArea
											,const NFmiPoint& theLatLon
											,double theRadius, double theValue
											,NFmiMetEditorTypes::Mask theAreaMask)
:NFmiDataParamModifierAreaConeChange(theInfo
									,theDrawParam
									,theMaskList
									,theArea
									,boost::shared_ptr<NFmiArea>()
									,theLatLon
									,theRadius
									,kFloatMissing
									,theAreaMask
									,0,0) // 0,0 ei rajoitus asetusta ja raja
,itsModifyValue(theValue)
{}

double NFmiDataParamModifierAreaCircleSetValue::Calculate(const double& theValue)
{
	double value = theValue;
	double distance = CalcDistance();
	if(distance < itsModificationRange)
		value = itsModifyValue;
	else
		return theValue;

	return FmiMin(FmiMax(value, itsDrawParam->AbsoluteMinValue()), itsDrawParam->AbsoluteMaxValue());
}
