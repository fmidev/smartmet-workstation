//**********************************************************
// C++ Class Name :  - (many NFmiDataModifier-child classes)
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: NFmiDataModifierMetEditClasses.cpp 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 2.1.4 ) 
//  - GD System Name    : mask and filter business 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : mask and filter 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Mon - Mar 29, 1999 
// 
// 
//  Description: 
//  T‰h‰n tiedostoon on ker‰tty useita NFmiDataModifier-luokan
//	lapsiluokkia, jotka tekev‰t erilaisia asioita.
// 
//  Change Log: 
// 
//**********************************************************
#include "NFmiDataModifierMetEditClasses.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiAreaMaskList.h"
#include "NFmiGrid.h"
#include "NFmiSoundingFunctions.h"

//************** NFmiDataModifierMissingValueSet ************************

NFmiDataModifierMissingValueSet::NFmiDataModifierMissingValueSet(double theMissingValue, double theValue)
:itsMissingValue(theMissingValue)
,itsValue(theValue)
{
}

NFmiDataModifierMissingValueSet::NFmiDataModifierMissingValueSet(const NFmiDataModifierMissingValueSet &theOther)
:NFmiDataModifier(theOther)
,itsMissingValue(theOther.itsMissingValue)
,itsValue(theOther.itsValue)
{
}

NFmiDataModifier* NFmiDataModifierMissingValueSet::Clone(void) const
{
	return new NFmiDataModifierMissingValueSet(*this);
}


float NFmiDataModifierMissingValueSet::FloatOperation(float theValue)
{
	if(theValue == itsMissingValue)
		return float(itsValue);
	else
		return theValue;
}
//************** NFmiDataModifierMissingValueSet ************************

//************** NFmiDataModifierTimeShifter ************************

NFmiDataModifierTimeShifter::NFmiDataModifierTimeShifter(const NFmiDataModifierTimeShifter &theOther)
:NFmiDataModifier(theOther)
,itsLastTimeIndex(theOther.itsLastTimeIndex)
,itsMaskList(NFmiAreaMaskList::CreateShallowCopy(theOther.itsMaskList))
,itsQueryInfoCopy(NFmiAreaMask::DoShallowCopy(theOther.itsQueryInfoCopy))
,itsTimeShiftFactorTable(theOther.itsTimeShiftFactorTable)
{
}

NFmiDataModifier* NFmiDataModifierTimeShifter::Clone(void) const
{
	return new NFmiDataModifierTimeShifter(*this);
}

float NFmiDataModifierTimeShifter::FloatOperation(float theValue)
{
	int timeIndex = itsQueryInfoCopy->TimeIndex();
	if(itsLastTimeIndex != timeIndex) // t‰m‰ on optimointia, ei p‰ivitet‰ rangeja joka kierroksella
	{
		itsLastTimeIndex = timeIndex;
		SyncronizeMasks();
	}
	if(itsMaskList->IsMasked(itsQueryInfoCopy->LatLon()))
	{
		double timeShift = itsTimeShiftFactorTable[timeIndex];
		float value1 = itsQueryInfoCopy->PeekTimeValue(int(floor(timeShift)));
		float value2 = itsQueryInfoCopy->PeekTimeValue(int(floor(timeShift + 1)));
		if(value1 == kFloatMissing && value2 == kFloatMissing)
			return theValue;
		else if(value1 == kFloatMissing)
			return value2;
		else if(value2 == kFloatMissing)
			return value1;
		else
		{
			float interpolationFactor = float(timeShift - int(timeShift));
			if(interpolationFactor < 0)
				interpolationFactor = 1 + interpolationFactor;
			FmiInterpolationMethod interp = itsQueryInfoCopy->Param().GetParam()->InterpolationMethod();
			if(interp != kNoneInterpolation && interp != kNearestPoint)
			{
				float interpolatedValue = (1 - interpolationFactor)*value1 + interpolationFactor*value2;
				return interpolatedValue;
			}
			else
			{
				if(interpolationFactor < 0.5f)
					return value1;
				else
					return value2;
			}
		}
	}
	return theValue;
}

void NFmiDataModifierTimeShifter::SyncronizeMasks(void)
{
	itsMaskList->SyncronizeMaskTime(itsQueryInfoCopy->Time());
}

void NFmiDataModifierTimeShifter::SetLocationIndex(unsigned long theIndex)
{
	NFmiDataModifier::SetLocationIndex(theIndex);
	if(itsQueryInfoCopy)
		itsQueryInfoCopy->LocationIndex(theIndex);
}

void NFmiDataModifierTimeShifter::SetTimeIndex(unsigned long theIndex)
{
	NFmiDataModifier::SetTimeIndex(theIndex);
	if(itsQueryInfoCopy)
		itsQueryInfoCopy->TimeIndex(theIndex);
}

void NFmiDataModifierTimeShifter::InitLatlonCache(void)
{
	NFmiDataModifier::InitLatlonCache();
	if(itsQueryInfoCopy)
		itsQueryInfoCopy->LatLon();
}

//************** NFmiDataModifierTimeShifter ************************

//************** NFmiDataModifierTimeShifterWithChangingRanges ************************

NFmiDataModifierTimeShifterWithChangingRanges::NFmiDataModifierTimeShifterWithChangingRanges(const NFmiDataModifierTimeShifterWithChangingRanges &theOther)
:NFmiDataModifierTimeShifter(theOther)
,itsNegRange(theOther.itsNegRange)
,itsPosRange(theOther.itsPosRange)
,itsModifier(theOther.itsModifier ? theOther.itsModifier->Clone() : 0)
,itsTimeRangeTable(theOther.itsTimeRangeTable)
{
}

NFmiDataModifier* NFmiDataModifierTimeShifterWithChangingRanges::Clone(void) const
{
	return new NFmiDataModifierTimeShifterWithChangingRanges(*this);
}

NFmiDataModifierTimeShifterWithChangingRanges::~NFmiDataModifierTimeShifterWithChangingRanges(void)
{
	delete itsModifier;
}

float NFmiDataModifierTimeShifterWithChangingRanges::FloatOperation(float theValue)
{
// Hoidetaan maskaus toistaiseksi n‰in: jos paikka on maskattu, tehd‰‰n laskut
// mutta jos ei ole maskattu, palautetaan parametrina annettu arvo takaisin 
// muuttumattomana. N‰in voi pit‰‰ maskit ja maskilistat viel‰ newbase:n ulkopuolella!
//	static int lastTimeIndex = -1;
	int timeIndex = itsQueryInfoCopy->TimeIndex();
	if(itsLastTimeIndex != timeIndex) // t‰m‰ on optimointia, ei p‰ivitet‰ rangeja joka kierroksella
	{
		itsLastTimeIndex = timeIndex;
		SyncronizeMasks();
		itsNegRange = int(itsTimeRangeTable[timeIndex].X());
		itsPosRange = int(itsTimeRangeTable[timeIndex].Y());
	}
	if(itsMaskList->IsMasked(itsQueryInfoCopy->LatLon()))
	{
		itsModifier->Clear();
		for(int i=itsNegRange; i<=itsPosRange; i++)
		{
			itsModifier->Calculate(itsQueryInfoCopy->PeekTimeValue(i));
		}
		if(itsModifier->CalculationResult() != kFloatMissing)
			return itsModifier->CalculationResult();
	}
	return theValue;
}

void NFmiDataModifierTimeShifterWithChangingRanges::SetLocationIndex(unsigned long theIndex)
{
	NFmiDataModifierTimeShifter::SetLocationIndex(theIndex);
	if(itsModifier)
		itsModifier->SetLocationIndex(theIndex);
}

void NFmiDataModifierTimeShifterWithChangingRanges::SetTimeIndex(unsigned long theIndex)
{
	NFmiDataModifierTimeShifter::SetTimeIndex(theIndex);
	if(itsModifier)
		itsModifier->SetTimeIndex(theIndex);
}

void NFmiDataModifierTimeShifterWithChangingRanges::InitLatlonCache(void)
{
	NFmiDataModifierTimeShifter::InitLatlonCache();
	if(itsModifier)
		itsModifier->InitLatlonCache();
}

//************** NFmiDataModifierTimeShifterWithChangingRanges ************************

//************* NFmiDataModifierTimeShifterWithChangingRangesUsingWAvg ************************

NFmiDataModifierTimeShifterWithChangingRangesUsingWAvg::NFmiDataModifierTimeShifterWithChangingRangesUsingWAvg(const NFmiDataModifierTimeShifterWithChangingRangesUsingWAvg &theOther)
:NFmiDataModifierTimeShifterWithChangingRanges(theOther)
{
}

NFmiDataModifier* NFmiDataModifierTimeShifterWithChangingRangesUsingWAvg::Clone(void) const
{
	return new NFmiDataModifierTimeShifterWithChangingRangesUsingWAvg(*this);
}

NFmiDataModifierTimeShifterWithChangingRangesUsingWAvg::~NFmiDataModifierTimeShifterWithChangingRangesUsingWAvg(void)
{
}

float NFmiDataModifierTimeShifterWithChangingRangesUsingWAvg::FloatOperation(float theValue)
{
	float value = NFmiDataModifierTimeShifterWithChangingRanges::FloatOperation(theValue);
	if(value == theValue)
		return value;
	int count = static_cast<int>(itsTimeRangeTable[itsLastTimeIndex].Y() - itsTimeRangeTable[itsLastTimeIndex].X());
	if(count <= 0)
		return value;
	for(int i=0; i<count; i++)
		itsModifier->Calculate(theValue);
	return itsModifier->CalculationResult();
}
//************** NFmiDataModifierTimeShifterWithChangingRangesUsingWAvg ************************

//************** NFmiDataModifierWithModifierWithMasks ************************
float NFmiDataModifierWithModifierWithMasks::FloatOperation(float theValue)
{
// Hoidetaan maskaus toistaiseksi n‰in: jos paikka on maskattu, tehd‰‰n laskut
// mutta jos ei ole maskattu, palautetaan parametrina annettu arvo takaisin 
// muuttumattomana. N‰in voi pit‰‰ maskit ja maskilistat viel‰ newbase:n ulkopuolella!
//	static int lastTimeIndex = -1;
	int timeIndex = itsQueryInfoCopy->TimeIndex();
	if(itsLastTimeIndex != timeIndex) // t‰m‰ on optimointia, ei p‰ivitet‰ maskien aikojajoka kierroksella
	{
		itsLastTimeIndex = timeIndex;
		itsMaskList->SyncronizeMaskTime(itsQueryInfoCopy->Time());
	}
	if(itsMaskList->IsMasked(itsQueryInfoCopy->LatLon()))
	{
		itsModifier->Clear();
		for(int i=itsNegXRange; i<=itsPosXRange; i++)
		{
			for(int j=itsNegYRange; j<=itsPosYRange; j++)
			{
				itsModifier->Calculate(itsQueryInfoCopy->PeekLocationValue(i,j));
			}
		}
		if(itsModifier->CalculationResult() != kFloatMissing)
			return itsModifier->CalculationResult();
	}

	return theValue;
}

NFmiDataModifierWithModifierWithMasks::NFmiDataModifierWithModifierWithMasks(const NFmiDataModifierWithModifierWithMasks &theOther)
:NFmiDataModifier(theOther)
,itsLastTimeIndex(theOther.itsLastTimeIndex)
,itsQueryInfoCopy(NFmiAreaMask::DoShallowCopy(theOther.itsQueryInfoCopy))
,itsModifier(theOther.itsModifier ? theOther.itsModifier->Clone() : 0)
,itsMaskList(NFmiAreaMaskList::CreateShallowCopy(theOther.itsMaskList))
,itsNegXRange(theOther.itsNegXRange)
,itsNegYRange(theOther.itsNegYRange)
,itsPosXRange(theOther.itsPosXRange)
,itsPosYRange(theOther.itsPosYRange)
{
}

NFmiDataModifier* NFmiDataModifierWithModifierWithMasks::Clone(void) const
{
	return new NFmiDataModifierWithModifierWithMasks(*this);
}

NFmiDataModifierWithModifierWithMasks::~NFmiDataModifierWithModifierWithMasks(void)
{
	delete itsModifier;
}

void NFmiDataModifierWithModifierWithMasks::SetLocationIndex(unsigned long theIndex)
{
	NFmiDataModifier::SetLocationIndex(theIndex);
	if(itsQueryInfoCopy)
		itsQueryInfoCopy->LocationIndex(theIndex);
}

void NFmiDataModifierWithModifierWithMasks::SetTimeIndex(unsigned long theIndex)
{
	NFmiDataModifier::SetTimeIndex(theIndex);
	if(itsQueryInfoCopy)
		itsQueryInfoCopy->TimeIndex(theIndex);
}

void NFmiDataModifierWithModifierWithMasks::InitLatlonCache(void)
{
	NFmiDataModifier::InitLatlonCache();
	if(itsQueryInfoCopy)
		itsQueryInfoCopy->LatLon();
}

//************** NFmiDataModifierWithModifierWithMasks ************************

//************** NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime ************************

NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime::NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime(const NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime &theOther)
:NFmiDataModifierWithModifierWithMasks(theOther)
,itsRangeArray(theOther.itsRangeArray)
{
}

NFmiDataModifier* NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime::Clone(void) const
{
	return new NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime(*this);
}

float NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime::FloatOperation(float theValue)
{
	int timeIndex = itsQueryInfoCopy->TimeIndex();
	if(itsLastTimeIndex != timeIndex) // t‰m‰ on optimointia, ei p‰ivitet‰ rangeja joka kierroksella, eik‰ k‰ytet‰ turhaan rect:in funktio kutsuja
								   // ja s‰‰stet‰‰n koodin kirjoitusta!
	{
		itsNegXRange = int(itsRangeArray[timeIndex].Left());
		itsPosXRange = int(itsRangeArray[timeIndex].Right());
		itsNegYRange = int(itsRangeArray[timeIndex].Top());
		itsPosYRange = int(itsRangeArray[timeIndex].Bottom());
	}
	return NFmiDataModifierWithModifierWithMasks::FloatOperation(theValue);
}

NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime::~NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime(void)
{
}

//************** NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime ************************

//************** NFmiDataModifierTimeInterpolationWithAreaModifiers ************************
NFmiDataModifierTimeInterpolationWithAreaModifiers::NFmiDataModifierTimeInterpolationWithAreaModifiers(
						boost::shared_ptr<NFmiFastQueryInfo> theQueryInfoCopy,
						NFmiDataModifier *theDataModifier
						,boost::shared_ptr<NFmiAreaMaskList> &theMaskList
						,const std::vector<NFmiRect> &theRangeArray
						,const NFmiMetTime& theStartTime
						,const NFmiMetTime& theEndTime
						,int theTimeResolutionInMinutes)
:NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime(theQueryInfoCopy, theDataModifier, theMaskList,theRangeArray)
,itsStartTime(theStartTime)
,itsEndTime(theEndTime)
,itsTimeResolutionInMinutes(theTimeResolutionInMinutes)
{
	itsMaxTimeSteps = 1 + itsEndTime.DifferenceInMinutes(itsStartTime) / itsTimeResolutionInMinutes;
	itsCurrentTimeStep = 1;
	CheckIfWDParameter();
}

NFmiDataModifierTimeInterpolationWithAreaModifiers::NFmiDataModifierTimeInterpolationWithAreaModifiers(const NFmiDataModifierTimeInterpolationWithAreaModifiers &theOther)
:NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime(theOther)
,itsStartTime(theOther.itsStartTime)
,itsEndTime(theOther.itsEndTime)
,itsMaxTimeSteps(theOther.itsMaxTimeSteps)
,itsCurrentTimeStep(theOther.itsCurrentTimeStep)
,itsTimeResolutionInMinutes(theOther.itsTimeResolutionInMinutes)
,fFixWDValue(theOther.fFixWDValue)
{
}

NFmiDataModifier* NFmiDataModifierTimeInterpolationWithAreaModifiers::Clone(void) const
{
	return new NFmiDataModifierTimeInterpolationWithAreaModifiers(*this);
}

static float CalcWDInterpolation(float WD1, float factor1, float WD2, float factor2)
{
	// oletus, ett‰ WD1 ja WD2, ja kertoimet eiv‰t ole puuttuvia arvoja
	if(fabs(WD1 - WD2) <= 180.)
	{
		return (WD1*factor1 + WD2*factor2)/(factor1 + factor2);
	}
	else
	{
		double biggerValue = 0.;
		double smallerValue = 0.;
		float biggerFactor = 0.;
		float smallerFactor = 0.;
		if(WD1 > WD2)
		{
			biggerValue = WD1;
			smallerValue = WD2;
			biggerFactor = factor1;
			smallerFactor = factor2;
		}
		else
		{
			biggerValue = WD2;
			smallerValue = WD1;
			biggerFactor = factor2;
			smallerFactor = factor1;
		}
		double sumValue = biggerValue + (360 - biggerValue + smallerValue) * (smallerFactor * (smallerFactor + biggerFactor));
		if(sumValue > 360.)
			sumValue = fmod(sumValue, 360.);
		return static_cast<float>(sumValue);
	}
}

float NFmiDataModifierTimeInterpolationWithAreaModifiers::FloatOperation(float theValue)
{
	int timeIndex = itsQueryInfoCopy->TimeIndex();
	if(itsLastTimeIndex != timeIndex) // t‰m‰ on optimointia, ei p‰ivitet‰ rangeja joka kierroksella, eik‰ k‰ytet‰ turhaan rect:in funktio kutsuja
								   // ja s‰‰stet‰‰n koodin kirjoitusta!
	{
		itsNegXRange = int(itsRangeArray[timeIndex].Left());
		itsPosXRange = int(itsRangeArray[timeIndex].Right());
		itsNegYRange = int(itsRangeArray[timeIndex].Top());
		itsPosYRange = int(itsRangeArray[timeIndex].Bottom());
		itsMaskList->SyncronizeMaskTime(itsQueryInfoCopy->Time());
		itsCurrentTimeStep = itsMaxTimeSteps - itsEndTime.DifferenceInMinutes(itsQueryInfoCopy->Time()) / itsTimeResolutionInMinutes;
		itsLastTimeIndex = timeIndex;
	}
	if(itsCurrentTimeStep == 1 || itsCurrentTimeStep == itsMaxTimeSteps)
		return theValue; // 1. ja viimeinen 'ruutu' pysyy samana
	float startValue = kFloatMissing;
	float endValue = kFloatMissing;
	if(itsMaskList->IsMasked(itsQueryInfoCopy->LatLon()))
	{
		double startOffsetFactor = (itsCurrentTimeStep-1.)/(itsMaxTimeSteps-1.);
		double endOffsetFactor = (itsMaxTimeSteps - (itsCurrentTimeStep-1.))/(itsMaxTimeSteps-1.);
		itsModifier->Clear();
		int i;
		for(i=itsNegXRange; i<=itsPosXRange; i++)
		{
			for(int j=itsNegYRange; j<=itsPosYRange; j++)
			{
				itsModifier->Calculate(itsQueryInfoCopy->PeekValue(1-itsCurrentTimeStep, int(startOffsetFactor * i), int(startOffsetFactor * j)));
			}
		}
		if(itsModifier->CalculationResult() != kFloatMissing)
			startValue = itsModifier->CalculationResult();

		itsModifier->Clear();
		for(i=itsNegXRange; i<=itsPosXRange; i++)
		{
			for(int j=itsNegYRange; j<=itsPosYRange; j++)
			{
				itsModifier->Calculate(itsQueryInfoCopy->PeekValue(itsMaxTimeSteps-itsCurrentTimeStep, int(-endOffsetFactor * i), int(-endOffsetFactor * j)));
			}
		}
		if(itsModifier->CalculationResult() != kFloatMissing)
			endValue = itsModifier->CalculationResult();

		if(startValue == kFloatMissing && endValue == kFloatMissing)
			return kFloatMissing;
		else if(startValue == kFloatMissing)
			return endValue;
		else if(endValue == kFloatMissing)
			return startValue;
		else
		{
			float factor1 = (float(itsMaxTimeSteps) - itsCurrentTimeStep)/itsMaxTimeSteps;
			float factor2 = float(itsCurrentTimeStep)/itsMaxTimeSteps;
			float tempValue = kFloatMissing;
			if(fFixWDValue)
				tempValue = CalcWDInterpolation(startValue, factor1, endValue, factor2);
			else
				tempValue = startValue * ((float(itsMaxTimeSteps) - itsCurrentTimeStep)/itsMaxTimeSteps) + endValue * (float(itsCurrentTimeStep)/itsMaxTimeSteps);
			return tempValue;
		}
	}
	return theValue;
}

// tarkistaa onko resultinfon aktiivinen parametri kuten tuulen suunta
// ja tekee tarvittavat asetukset
void NFmiDataModifierTimeInterpolationWithAreaModifiers::CheckIfWDParameter(void)
{
	fFixWDValue = false;
	if(itsQueryInfoCopy)
	{
		if(itsQueryInfoCopy->Param().GetParamIdent() == kFmiWindDirection)
		{
			fFixWDValue = true;
		}
	}
}

//************** NFmiDataModifierTimeInterpolationWithAreaModifiers ************************


//************** NFmiDataModifierParamCombiner ************************
NFmiDataModifierParamCombiner::NFmiDataModifierParamCombiner(
								 boost::shared_ptr<NFmiFastQueryInfo> theQueryInfoCopy
								,NFmiDataModifier *theDataModifier
								,boost::shared_ptr<NFmiAreaMaskList> &theMaskList
								,const std::vector<NFmiRect> &theRangeArray
								,boost::shared_ptr<NFmiFastQueryInfo> theFirstParamInfo
								,boost::shared_ptr<NFmiFastQueryInfo> theSecondParamInfo
								,const std::vector<float> &theCombineFactorArray
								,float theFirstParamBase
								,float theSecondParamBase
								,float theFirstParamFactor
								,float theSecondParamFactor
								,float theFirstParamAdditionalMissingValue
								,float theSecondParamAdditionalMissingValue)
:NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime(theQueryInfoCopy, theDataModifier, theMaskList,theRangeArray)
,itsRainConstantC(0) // c = log10 * a / b
,itsRainConstantA(200)
,itsRainConstantB(1.6f)
,fFirstParamIsRadar(false)
,fSecondParamIsRadar(false)
,fFirstParamInfoIsSameGridAsModifiedData(false)
,fSecondParamInfoIsSameGridAsModifiedData(false)
,fFirstParamUsed(false)
,fSecondParamUsed(false)
,itsCombineFactorArray(theCombineFactorArray)
,itsFirstParamInfo(theFirstParamInfo)
,itsSecondParamInfo(theSecondParamInfo)
,itsFirstParamBase(theFirstParamBase)
,itsSecondParamBase(theSecondParamBase)
,itsFirstParamFactor(theFirstParamFactor)
,itsSecondParamFactor(theSecondParamFactor)
,itsFirstParamAdditionalMissingValue(theFirstParamAdditionalMissingValue)
,itsSecondParamAdditionalMissingValue(theSecondParamAdditionalMissingValue)
{
	if(theQueryInfoCopy && theFirstParamInfo && theSecondParamInfo)
	{
		itsRainConstantC = float(log10(itsRainConstantA) / itsRainConstantB);
		if(itsFirstParamAdditionalMissingValue == -32)
			fFirstParamIsRadar = true;
		if(itsSecondParamAdditionalMissingValue == -32)
			fSecondParamIsRadar = true;
		if(theFirstParamInfo->ParamIndex() < theFirstParamInfo->SizeParams())
			fFirstParamUsed = true;
		if(theSecondParamInfo->ParamIndex() < theSecondParamInfo->SizeParams())
			fSecondParamUsed = true;
		const NFmiGrid* grid = theQueryInfoCopy->Grid();
		const NFmiGrid* grid1 = theFirstParamInfo->Grid();
		const NFmiGrid* grid2 = theSecondParamInfo->Grid();
		if(grid && grid1 && grid->AreGridsIdentical(*grid1))
			fFirstParamInfoIsSameGridAsModifiedData = true;
		if(grid && grid2 && grid->AreGridsIdentical(*grid2))
			fSecondParamInfoIsSameGridAsModifiedData = true;
	}
}

NFmiDataModifierParamCombiner::NFmiDataModifierParamCombiner(const NFmiDataModifierParamCombiner &theOther)
:NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime(theOther)
,itsRainConstantC(theOther.itsRainConstantC)
,itsRainConstantA(theOther.itsRainConstantA)
,itsRainConstantB(theOther.itsRainConstantB)
,fFirstParamIsRadar(theOther.fFirstParamIsRadar)
,fSecondParamIsRadar(theOther.fSecondParamIsRadar)
,fFirstParamInfoIsSameGridAsModifiedData(theOther.fFirstParamInfoIsSameGridAsModifiedData)
,fSecondParamInfoIsSameGridAsModifiedData(theOther.fSecondParamInfoIsSameGridAsModifiedData)
,fFirstParamUsed(theOther.fFirstParamUsed)
,fSecondParamUsed(theOther.fSecondParamUsed)
,itsCombineFactorArray(theOther.itsCombineFactorArray)
,itsFirstParamInfo(NFmiAreaMask::DoShallowCopy(theOther.itsFirstParamInfo))
,itsSecondParamInfo(NFmiAreaMask::DoShallowCopy(theOther.itsSecondParamInfo))
,itsFirstParamBase(theOther.itsFirstParamBase)
,itsSecondParamBase(theOther.itsSecondParamBase)
,itsFirstParamFactor(theOther.itsFirstParamFactor)
,itsSecondParamFactor(theOther.itsSecondParamFactor)
,itsFirstParamAdditionalMissingValue(theOther.itsFirstParamAdditionalMissingValue)
,itsSecondParamAdditionalMissingValue(theOther.itsSecondParamAdditionalMissingValue)
{
}

NFmiDataModifier* NFmiDataModifierParamCombiner::Clone(void) const
{
	return new NFmiDataModifierParamCombiner(*this);
}

NFmiDataModifierParamCombiner::~NFmiDataModifierParamCombiner(void)
{
}

float NFmiDataModifierParamCombiner::FloatOperation(float theValue)
{
	int timeIndex = itsQueryInfoCopy->TimeIndex();
	if(itsLastTimeIndex != timeIndex) // t‰m‰ on optimointia, ei p‰ivitet‰ rangeja joka kierroksella, eik‰ k‰ytet‰ turhaan rect:in funktio kutsuja
								   // ja s‰‰stet‰‰n koodin kirjoitusta!
	{
		itsLastTimeIndex = timeIndex;
		itsNegXRange = int(itsRangeArray[timeIndex].Left());
		itsPosXRange = int(itsRangeArray[timeIndex].Right());
		itsNegYRange = int(itsRangeArray[timeIndex].Top());
		itsPosYRange = int(itsRangeArray[timeIndex].Bottom());
		itsMaskList->SyncronizeMaskTime(itsQueryInfoCopy->Time());
		itsFirstParamInfo->Time(itsQueryInfoCopy->Time());
		itsSecondParamInfo->Time(itsQueryInfoCopy->Time());
	}

// POISTUU: t‰m‰ on v‰liaikainen viritys, t‰ss‰ oletetaan ett‰ kaikki datat tulevat samanlaisista hiloista
	int locationIndex = itsQueryInfoCopy->LocationIndex();
	if(locationIndex == 0)
	{
		if(fFirstParamInfoIsSameGridAsModifiedData)
			itsFirstParamInfo->ResetLocation();
		if(fSecondParamInfoIsSameGridAsModifiedData)
			itsSecondParamInfo->ResetLocation();
	}
	if(fFirstParamInfoIsSameGridAsModifiedData)
		itsFirstParamInfo->LocationIndex(locationIndex);
	if(fSecondParamInfoIsSameGridAsModifiedData)
		itsSecondParamInfo->LocationIndex(locationIndex);
// POISTUU: t‰m‰ on v‰liaikainen viritys, t‰ss‰ oletetaan ett‰ kaikki datat tulevat samanlaisista hiloista

	if(itsMaskList->IsMasked(itsQueryInfoCopy->LatLon()))
	{
		itsModifier->Clear();
		for(int i=itsNegXRange; i<=itsPosXRange; i++)
		{
			for(int j=itsNegYRange; j<=itsPosYRange; j++)
			{
				float firstParamValue = kFloatMissing;
				float secondParamValue = kFloatMissing;
				if(fFirstParamUsed)
				{
					if(fFirstParamInfoIsSameGridAsModifiedData)
						firstParamValue = itsFirstParamInfo->PeekLocationValue(i,j);
					else
						firstParamValue = itsFirstParamInfo->InterpolatedValue(itsQueryInfoCopy->PeekLocationLatLon(i,j));
					if(fFirstParamIsRadar)
						firstParamValue = Dz2Rain(firstParamValue, itsFirstParamAdditionalMissingValue);
				}
				if(fSecondParamUsed)
				{
					if(fSecondParamInfoIsSameGridAsModifiedData)
						secondParamValue = itsSecondParamInfo->PeekLocationValue(i,j);
					else
						secondParamValue = itsSecondParamInfo->InterpolatedValue(itsQueryInfoCopy->PeekLocationLatLon(i,j));
					if(fSecondParamIsRadar)
						secondParamValue = Dz2Rain(secondParamValue, itsSecondParamAdditionalMissingValue);
				}
// POISTUU: t‰m‰ on v‰liaikainen viritys, t‰ss‰ oletetaan ett‰ kaikki datat tulevat samanlaisista hiloista
// pit‰isi hoitaa itsFirstParamInfo->InterpolatedValue(latlon) avulla, Huom! hidastuu.
// HUOM!! ei toimi ellei tehd‰ metodia GetPeekLatLonPoint(i,j) joka palauttaa latlon-pisteen
//				float firstParamValue = itsFirstParamInfo->PeekLocationValue(i,j);
//				float secondParamValue = itsSecondParamInfo->PeekLocationValue(i,j);
// POISTUU: t‰m‰ on v‰liaikainen viritys, t‰ss‰ oletetaan ett‰ kaikki datat tulevat samanlaisista hiloista

				float newValue1 = kFloatMissing;
				float newValue2 = kFloatMissing;
				if(!fFirstParamIsRadar)
				{
					if(!(firstParamValue == itsFirstParamAdditionalMissingValue || firstParamValue == kFloatMissing))
						newValue1 = itsFirstParamBase + firstParamValue * itsFirstParamFactor;
				}
				else
					newValue1 = firstParamValue;
				if(!fSecondParamIsRadar)
				{
					if(!(secondParamValue == itsSecondParamAdditionalMissingValue || secondParamValue == kFloatMissing))
						newValue2 = itsSecondParamBase + secondParamValue * itsSecondParamFactor;
				}
				else
					newValue2 = secondParamValue;
				if(newValue1 == kFloatMissing && newValue2 == kFloatMissing)
				{// ei tehd‰ mit‰‰n
				}
				else if(newValue1 == kFloatMissing)
				{
					float combinedValue = newValue2;
					itsModifier->Calculate(combinedValue);
				}
				else if(newValue2 == kFloatMissing)
				{
					float combinedValue = newValue1;
					itsModifier->Calculate(combinedValue);
				}
				else // molemmat arvot ok
				{
					float combinedValue = newValue1 * itsCombineFactorArray[timeIndex] + newValue2 * (1.f - itsCombineFactorArray[timeIndex]);
					itsModifier->Calculate(combinedValue);
				}
			}
		}
		if(itsModifier->CalculationResult() != kFloatMissing)
			return itsModifier->CalculationResult();
	}

	return theValue;
}

float NFmiDataModifierParamCombiner::Dz2Rain(float theDz, float& theAdditionalMissingValue)
{
	float value = kFloatMissing;
	if(!(theDz == kFloatMissing || theDz == theAdditionalMissingValue))
	{
		value = float(pow(10.f,(theDz/(10*itsRainConstantB))- itsRainConstantC));
	}
	return value;
}

void NFmiDataModifierParamCombiner::InitLatlonCache(void)
{
	NFmiDataModifier::InitLatlonCache();
	if(itsFirstParamInfo)
		itsFirstParamInfo->LatLon();
	if(itsSecondParamInfo)
		itsSecondParamInfo->LatLon();
}

//************** NFmiDataModifierParamCombiner ************************


//************** NFmiDataModelDataCombiner ************************
NFmiDataModelDataCombiner::NFmiDataModelDataCombiner(
								 boost::shared_ptr<NFmiFastQueryInfo> theQueryInfoCopy
								,NFmiDataModifier *theDataModifier
								,boost::shared_ptr<NFmiAreaMaskList> &theMaskList
								,const std::vector<NFmiRect> &theRangeArray
								,boost::shared_ptr<NFmiFastQueryInfo> theFirstModelInfo
								,boost::shared_ptr<NFmiFastQueryInfo> theSecondModelInfo
								,const std::vector<float> &theCombineFactorArray)
:NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime(theQueryInfoCopy, theDataModifier, theMaskList, theRangeArray)
,fFirstParamInfoIsSameGridAsModifiedData(false)
,fSecondParamInfoIsSameGridAsModifiedData(false)
,fFirstModelInfoHasSameTimeResolutionAsModifiedData(false)
,fSecondModelInfoHasSameTimeResolutionAsModifiedData(false)
,itsCombineFactorArray(theCombineFactorArray)
,itsFirstModelInfo(theFirstModelInfo)
,itsSecondModelInfo(theSecondModelInfo)
,itsCurrentTime()
,itsFirstModelTimeResolution(itsFirstModelInfo->TimeResolution())
,itsSecondModelTimeResolution(itsSecondModelInfo->TimeResolution())
{
	if(theQueryInfoCopy && theFirstModelInfo && theSecondModelInfo)
	{
		const NFmiGrid* grid = theQueryInfoCopy->Grid();
		const NFmiGrid* grid1 = theFirstModelInfo->Grid();
		const NFmiGrid* grid2 = theSecondModelInfo->Grid();
		if(grid && grid1 && grid->AreGridsIdentical(*grid1))
			fFirstParamInfoIsSameGridAsModifiedData = true;
		if(grid && grid2 && grid->AreGridsIdentical(*grid2))
			fSecondParamInfoIsSameGridAsModifiedData = true;

		int modifiedDataTimeResolution = theQueryInfoCopy->TimeResolution();
		if(modifiedDataTimeResolution == itsFirstModelTimeResolution)
			fFirstModelInfoHasSameTimeResolutionAsModifiedData = true;
		if(modifiedDataTimeResolution == itsSecondModelTimeResolution)
			fSecondModelInfoHasSameTimeResolutionAsModifiedData = true;
	}
}

NFmiDataModelDataCombiner::NFmiDataModelDataCombiner(const NFmiDataModelDataCombiner &theOther)
:NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime(theOther)
,fFirstParamInfoIsSameGridAsModifiedData(theOther.fFirstParamInfoIsSameGridAsModifiedData)
,fSecondParamInfoIsSameGridAsModifiedData(theOther.fSecondParamInfoIsSameGridAsModifiedData)
,fFirstModelInfoHasSameTimeResolutionAsModifiedData(theOther.fFirstModelInfoHasSameTimeResolutionAsModifiedData)
,fSecondModelInfoHasSameTimeResolutionAsModifiedData(theOther.fSecondModelInfoHasSameTimeResolutionAsModifiedData)
,itsCombineFactorArray(theOther.itsCombineFactorArray)
,itsFirstModelInfo(NFmiAreaMask::DoShallowCopy(theOther.itsFirstModelInfo))
,itsSecondModelInfo(NFmiAreaMask::DoShallowCopy(theOther.itsSecondModelInfo))
,itsCurrentTime(theOther.itsCurrentTime)
,itsFirstModelTimeResolution(theOther.itsFirstModelTimeResolution)
,itsSecondModelTimeResolution(theOther.itsSecondModelTimeResolution)
{
}

NFmiDataModifier* NFmiDataModelDataCombiner::Clone(void) const
{
	return new NFmiDataModelDataCombiner(*this);
}

NFmiDataModelDataCombiner::~NFmiDataModelDataCombiner(void)
{
}

float NFmiDataModelDataCombiner::FloatOperation(float theValue)
{
	int timeIndex = itsQueryInfoCopy->TimeIndex();
	if(itsLastTimeIndex != timeIndex) // t‰m‰ on optimointia, ei p‰ivitet‰ rangeja joka kierroksella, eik‰ k‰ytet‰ turhaan rect:in funktio kutsuja
								   // ja s‰‰stet‰‰n koodin kirjoitusta!
	{
		itsLastTimeIndex = timeIndex;
		itsNegXRange = int(itsRangeArray[timeIndex].Left());
		itsPosXRange = int(itsRangeArray[timeIndex].Right());
		itsNegYRange = int(itsRangeArray[timeIndex].Top());
		itsPosYRange = int(itsRangeArray[timeIndex].Bottom());
		itsCurrentTime = itsQueryInfoCopy->Time();
		itsMaskList->SyncronizeMaskTime(itsCurrentTime);
		itsFirstModelInfo->Time(itsCurrentTime);
		itsSecondModelInfo->Time(itsCurrentTime);

		// N‰it‰ parametrin asetuksia ei tarvitsisi tehd‰ joka kerta, kun aika muuttuu, mutta menkˆˆn nyt t‰ss‰ vaiheessa
		itsFirstModelInfo->Param(*(itsQueryInfoCopy->Param().GetParam()));
		itsSecondModelInfo->Param(*(itsQueryInfoCopy->Param().GetParam()));
	}

// POISTUU: t‰m‰ on v‰liaikainen viritys, t‰ss‰ oletetaan ett‰ kaikki datat tulevat samanlaisista hiloista
	int locationIndex = itsQueryInfoCopy->LocationIndex();
	if(locationIndex == 0)
	{
		if(fFirstParamInfoIsSameGridAsModifiedData)
			itsFirstModelInfo->ResetLocation();
		if(fSecondParamInfoIsSameGridAsModifiedData)
			itsSecondModelInfo->ResetLocation();
	}
	if(fFirstParamInfoIsSameGridAsModifiedData)
		itsFirstModelInfo->LocationIndex(locationIndex);
	if(fSecondParamInfoIsSameGridAsModifiedData)
		itsSecondModelInfo->LocationIndex(locationIndex);
// POISTUU: t‰m‰ on v‰liaikainen viritys, t‰ss‰ oletetaan ett‰ kaikki datat tulevat samanlaisista hiloista

	if(itsMaskList->IsMasked(itsQueryInfoCopy->LatLon()))
	{
		itsModifier->Clear();
		for(int i=itsNegXRange; i<=itsPosXRange; i++)
		{
			for(int j=itsNegYRange; j<=itsPosYRange; j++)
			{
				float firstParamValue = kFloatMissing;
				float secondParamValue = kFloatMissing;

				if(fFirstParamInfoIsSameGridAsModifiedData)
				{
					if(fFirstModelInfoHasSameTimeResolutionAsModifiedData)
						firstParamValue = itsFirstModelInfo->PeekLocationValue(i,j);
					else // else haara toimii virheellisesti, ottaa aina saman arvon i:st‰ ja j:st‰ riippumatta
						firstParamValue = itsFirstModelInfo->InterpolatedValue(itsCurrentTime, 2 * itsFirstModelTimeResolution);
				}
				else
				{
					if(fFirstModelInfoHasSameTimeResolutionAsModifiedData)
						firstParamValue = itsFirstModelInfo->InterpolatedValue(itsQueryInfoCopy->PeekLocationLatLon(i,j));
					else
						firstParamValue = itsFirstModelInfo->InterpolatedValue(itsQueryInfoCopy->PeekLocationLatLon(i,j), itsCurrentTime, itsFirstModelTimeResolution);
				}

				if(fSecondParamInfoIsSameGridAsModifiedData)
				{
					if(fSecondModelInfoHasSameTimeResolutionAsModifiedData)
						secondParamValue = itsSecondModelInfo->PeekLocationValue(i,j);
					else // else haara toimii virheellisesti, ottaa aina saman arvon i:st‰ ja j:st‰ riippumatta
						secondParamValue = itsSecondModelInfo->InterpolatedValue(itsCurrentTime, 2 * itsSecondModelTimeResolution);
				}
				else
				{
					if(fSecondModelInfoHasSameTimeResolutionAsModifiedData)
						secondParamValue = itsSecondModelInfo->InterpolatedValue(itsQueryInfoCopy->PeekLocationLatLon(i,j));
					else
						secondParamValue = itsSecondModelInfo->InterpolatedValue(itsQueryInfoCopy->PeekLocationLatLon(i,j), itsCurrentTime, itsSecondModelTimeResolution);
				}

// POISTUU: t‰m‰ on v‰liaikainen viritys, t‰ss‰ oletetaan ett‰ kaikki datat tulevat samanlaisista hiloista
// pit‰isi hoitaa itsFirstParamInfo->InterpolatedValue(latlon) avulla, Huom! hidastuu.
// HUOM!! ei toimi ellei tehd‰ metodia GetPeekLatLonPoint(i,j) joka palauttaa latlon-pisteen
//				float firstParamValue = itsFirstParamInfo->PeekLocationValue(i,j);
//				float secondParamValue = itsSecondParamInfo->PeekLocationValue(i,j);
// POISTUU: t‰m‰ on v‰liaikainen viritys, t‰ss‰ oletetaan ett‰ kaikki datat tulevat samanlaisista hiloista

				float newValue1 = firstParamValue;
				float newValue2 = secondParamValue;

				if(newValue1 == kFloatMissing && newValue2 == kFloatMissing)
				{// ei tehd‰ mit‰‰n
				}
				else if(newValue1 == kFloatMissing)
				{
					float combinedValue = newValue2;
					itsModifier->Calculate(combinedValue);
				}
				else if(newValue2 == kFloatMissing)
				{
					float combinedValue = newValue1;
					itsModifier->Calculate(combinedValue);
				}
				else // molemmat arvot ok
				{
					float combinedValue = newValue1 * itsCombineFactorArray[timeIndex] + newValue2 * (1.f - itsCombineFactorArray[timeIndex]);
					itsModifier->Calculate(combinedValue);
				}
			}
		}
		if(itsModifier->CalculationResult() != kFloatMissing)
			return itsModifier->CalculationResult();
	}

	return theValue;
}

void NFmiDataModelDataCombiner::InitLatlonCache(void)
{
	NFmiDataModifier::InitLatlonCache();
	if(itsFirstModelInfo)
		itsFirstModelInfo->LatLon();
	if(itsSecondModelInfo)
		itsSecondModelInfo->LatLon();
}

//************** NFmiDataModelDataCombiner ************************


//************** NFmiDataModifierValidateData_PrForm_T ************************

NFmiDataModifierValidateData_PrForm_T::NFmiDataModifierValidateData_PrForm_T(boost::shared_ptr<NFmiFastQueryInfo> theTemperatureInfo, float theSnowTemperatureLimit, float theRainTemperatureLimit)
:NFmiDataModifier()
,itsTemperatureInfo(theTemperatureInfo)
,itsSnowTemperatureLimit(theSnowTemperatureLimit)
,itsRainTemperatureLimit(theRainTemperatureLimit)
{
}

NFmiDataModifierValidateData_PrForm_T::NFmiDataModifierValidateData_PrForm_T(const NFmiDataModifierValidateData_PrForm_T &theOther)
:NFmiDataModifier(theOther)
,itsTemperatureInfo(new NFmiFastQueryInfo(*theOther.itsTemperatureInfo))
,itsSnowTemperatureLimit(theOther.itsSnowTemperatureLimit)
,itsRainTemperatureLimit(theOther.itsRainTemperatureLimit)
{
}

NFmiDataModifier* NFmiDataModifierValidateData_PrForm_T::Clone(void) const
{
	return new NFmiDataModifierValidateData_PrForm_T(*this);
}

void NFmiDataModifierValidateData_PrForm_T::SetLocationIndex(unsigned long theIndex)
{
	NFmiDataModifier::SetLocationIndex(theIndex);
	if(itsTemperatureInfo)
		itsTemperatureInfo->LocationIndex(theIndex);
}

void NFmiDataModifierValidateData_PrForm_T::SetTimeIndex(unsigned long theIndex)
{
	NFmiDataModifier::SetTimeIndex(theIndex);
	if(itsTemperatureInfo)
		itsTemperatureInfo->TimeIndex(theIndex);
}

void NFmiDataModifierValidateData_PrForm_T::InitLatlonCache(void)
{
	NFmiDataModifier::InitLatlonCache();
	if(itsTemperatureInfo)
		itsTemperatureInfo->LatLon();
}

float NFmiDataModifierValidateData_PrForm_T::FloatOperation(float theValue)
{
	//theValue:n arvot ovat: 0=drizzle, 1=rain, 2=sleet, 3=snow, 4=freezing drizzle, 5=freezing rain, 6=hail
	float temperature = itsTemperatureInfo->FloatValue();
	if(temperature != kFloatMissing)
	{
		if(temperature < itsSnowTemperatureLimit) // itsSnowTemperatureLimit arvon missing arvon tarkistus pit‰‰ tehd‰ ennen modifierin k‰yttˆ‰
		{
			// muutan t‰ss‰ vain sateet ja r‰nn‰t lumeksi, jos raja alittuu (pit‰isikˆ muitakin arvoja muuttaa?)
			if(theValue == 1 || theValue == 2)
				theValue = 3;
		}
		else if(temperature < itsRainTemperatureLimit)
		{
			// muutan t‰ss‰ vain sateet ja lumet r‰nn‰ksi
			if(theValue == 1 || theValue == 3)
				theValue = 2;
		}
		else
		{
			// muutan t‰ss‰ vain lumet ja r‰nn‰t sateeksi
			if(theValue == 3 || theValue == 2)
				theValue = 1;
		}
	}
	return theValue;
}
//************** NFmiDataModifierValidateData_PrForm_T ************************

//************** NFmiDataModifierValidateData_T_DP ************************
// muuttaa kastepistett‰ (DP) l‰mpˆtilan (T) ja suht. kosteuden (RH) mukaan
float NFmiDataModifierValidateData_T_DP::FloatOperation(float theValue)
{
	int timeIndex = itsTemperatureInfo->TimeIndex(); 
	if(itsLastTimeIndex != timeIndex) // t‰m‰ on optimointia
	{
		itsLastTimeIndex = timeIndex;
		itsRHInfo->Time(itsTemperatureInfo->Time());
	}
	float T = itsTemperatureInfo->FloatValue(); // olettaa, ett‰ 'kopio' info on samassa paikassa kuin DP-data (on juoksutettu jo paikankin suhteen)
	float RH = itsRHInfo->InterpolatedValue(itsTemperatureInfo->LatLon());
	if(T == kFloatMissing && RH == kFloatMissing && theValue == kFloatMissing)
		return kFloatMissing;
	else if(T == kFloatMissing || RH == kFloatMissing)
		return theValue; // ei muuteta originaali Td arvoa, jos T tai RH oli puuttuvaa
	else
		theValue = static_cast<float>(NFmiSoundingFunctions::CalcDP(T, RH));
	return theValue;
}
//************** NFmiDataModifierValidateData_T_DP ************************

//************** NFmiDataModifierValidateData_T_DP_ver2 ************************
NFmiDataModifierValidateData_T_DP_ver2::NFmiDataModifierValidateData_T_DP_ver2(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
:NFmiDataModifier()
,itsInfo(theInfo)
,itsTIndex(gMissingIndex)
,itsRHIndex(gMissingIndex)
{
	itsInfo->Param(kFmiTemperature);
	itsTIndex = itsInfo->ParamIndex();
	itsInfo->Param(kFmiHumidity);
	itsRHIndex = itsInfo->ParamIndex();
}

NFmiDataModifierValidateData_T_DP_ver2::NFmiDataModifierValidateData_T_DP_ver2(const NFmiDataModifierValidateData_T_DP_ver2 &theOther)
:NFmiDataModifier(theOther)
,itsInfo(new NFmiFastQueryInfo(*theOther.itsInfo))
,itsTIndex(theOther.itsTIndex)
,itsRHIndex(theOther.itsRHIndex)
{
}

NFmiDataModifier* NFmiDataModifierValidateData_T_DP_ver2::Clone(void) const
{
	return new NFmiDataModifierValidateData_T_DP_ver2(*this);
}

void NFmiDataModifierValidateData_T_DP_ver2::SetLocationIndex(unsigned long theIndex)
{
	NFmiDataModifier::SetLocationIndex(theIndex);
	if(itsInfo)
		itsInfo->LocationIndex(theIndex);
}

void NFmiDataModifierValidateData_T_DP_ver2::SetTimeIndex(unsigned long theIndex)
{
	NFmiDataModifier::SetTimeIndex(theIndex);
	if(itsInfo)
		itsInfo->TimeIndex(theIndex);
}

void NFmiDataModifierValidateData_T_DP_ver2::InitLatlonCache(void)
{
	NFmiDataModifier::InitLatlonCache();
	if(itsInfo)
		itsInfo->LatLon();
}

// Laskee uuden arvon kastepisteelle (Td) k‰ytt‰en apuna l‰mpˆtilaa (T) ja suht. kosteutta (RH).
// itsInfo:n oikea aika (NFmiQueryInfo::ModifyTimesLocationData -metodissa) ja 
// paikka (NFmiQueryInfo::ModifyLocationData2 -metodissa) on varmistettu jo t‰m‰n funktion ulkopuolella em. metodeissa.
float NFmiDataModifierValidateData_T_DP_ver2::FloatOperation(float theValue) // theValue on oikeasti nykyinen Td-arvo datasta
{
	itsInfo->ParamIndex(itsTIndex);
	float T = itsInfo->FloatValue();
	itsInfo->ParamIndex(itsRHIndex);
	float RH = itsInfo->FloatValue();
	if(T == kFloatMissing || RH == kFloatMissing)
		return theValue; // ei muuteta originaali Td arvoa, jos T tai RH oli puuttuvaa
	else
		theValue = static_cast<float>(NFmiSoundingFunctions::CalcDP(T, RH));
	return theValue;
}

//************** NFmiDataModifierValidateData_T_DP_ver2 ************************


//************** NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTimeUsingWAvg ************************

NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTimeUsingWAvg::NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTimeUsingWAvg(const NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTimeUsingWAvg &theOther)
:NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime(theOther)
{
}

NFmiDataModifier* NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTimeUsingWAvg::Clone(void) const
{
	return new NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTimeUsingWAvg(*this);
}

float NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTimeUsingWAvg::FloatOperation(float theValue)
{
	float value = NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime::FloatOperation(theValue);
	if(value == theValue)
		return theValue;
	int count = static_cast<int>(2*((itsRangeArray[itsLastTimeIndex].Width() + 1) * (itsRangeArray[itsLastTimeIndex].Height() + 1)) - 1);
	if(count <= 0)
		return value;
	for(int i=0; i<count; i++)
		itsModifier->Calculate(theValue);
	return itsModifier->CalculationResult();
}

NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTimeUsingWAvg::~NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTimeUsingWAvg(void)
{
}

//************** NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTimeUsingWAvg ************************
