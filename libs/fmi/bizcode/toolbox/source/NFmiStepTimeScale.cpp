#include "NFmiStepTimeScale.h"
#include "NFmiTimeScale.h"
#include "NFmiTimeBag.h"

#include <stdio.h>
#include <algorithm>

//______________________________________________________________________________
//______________________________________________________________________________
//inline
NFmiStepTimeScale :: NFmiStepTimeScale (void)
						 : NFmiStepScale()
{
    itsMinStepValue = 1.f; // koska pienempiä aikayksiköitä ei ole
}
//______________________________________________________________________________
//inline
NFmiStepTimeScale :: NFmiStepTimeScale (unsigned long theMinStepValue
                                       ,const NFmiTimeScale& theScale
									   ,unsigned long theStepInMinutes
									   ,unsigned long theStartGapInMinutes
									   ,FmiDirection theDirection)
{
    itsMinStepValue = static_cast<float>(theMinStepValue);
	itsScale = new NFmiTimeScale(theScale);
	itsStepValue = static_cast<float>(theStepInMinutes);
	itsDirection =  theDirection;
	if (itsStepValue <= 0.f) itsStepValue = (itsScale->EndValue()-itsScale->StartValue())/5.f;
	itsStepValue = std::max(itsStepValue, itsMinStepValue);
	itsStartGap = static_cast<float>(theStartGapInMinutes);
	Reset();
}

//______________________________________________________________________________
//inline
NFmiStepTimeScale :: NFmiStepTimeScale (const NFmiTimeScale& theScale
													,unsigned long theStepInMinutes
													,unsigned long theStartGapInMinutes
													,FmiDirection theDirection)
{
    itsMinStepValue = 1.f;
	itsScale = new NFmiTimeScale(theScale);
	itsStepValue = static_cast<float>(theStepInMinutes);
	itsDirection =  theDirection;
	if (itsStepValue <= 0.f) itsStepValue = (itsScale->EndValue()-itsScale->StartValue())/5.f;
	itsStartGap = static_cast<float>(theStartGapInMinutes);
	Reset();
}

//______________________________________________________________________________
//inline
NFmiStepTimeScale :: NFmiStepTimeScale (const NFmiTime& theStartTime
	                                   ,const NFmiTime& theEndTime
									   ,unsigned long theStepInMinutes
									   ,unsigned long theStartGapInMinutes
									   ,FmiDirection theDirection)
{
    itsMinStepValue = 1.f;
	itsScale = new NFmiTimeScale(theStartTime, theEndTime);
	itsStepValue = static_cast<float>(theStepInMinutes);
	itsDirection =  theDirection;
	if (itsStepValue <= 0.f) itsStepValue = (itsScale->EndValue()-itsScale->StartValue())/5.f;
	itsStartGap = static_cast<float>(theStartGapInMinutes);
	Reset();
}
//______________________________________________________________________________
//inline
NFmiStepTimeScale :: NFmiStepTimeScale (const NFmiTimeBag& theTimeBag)
{
    itsMinStepValue = 1.f;
	itsScale = new NFmiTimeScale(theTimeBag.FirstTime(), theTimeBag.LastTime());
	itsStepValue = static_cast<float>(theTimeBag.Resolution());
	itsStartGap = 0.f;
	itsDirection = kForward;
	Reset();
}
//______________________________________________________________________________
//inline
NFmiStepTimeScale :: NFmiStepTimeScale (const NFmiStepTimeScale& anOtherScale)
						 : NFmiStepScale () //anOtherScale)
{
//	((NFmiTimeScale*)Scale())->SetStartTime(((NFmiTimeScale*)anOtherScale.Scale())->StartTime());
	itsScale = new NFmiTimeScale(*static_cast<NFmiTimeScale *>(anOtherScale.Scale()));
	itsStepValue = anOtherScale.itsStepValue;
	itsStartGap  = anOtherScale.itsStartGap;
	itsDirection = anOtherScale.itsDirection;
    itsMinStepValue = anOtherScale.itsMinStepValue;
}
//______________________________________________________________________________
//inline
NFmiStepTimeScale :: ~NFmiStepTimeScale(void)
{
}
//______________________________________________________________________________
//inline
NFmiStepTimeScale& NFmiStepTimeScale :: operator= (const NFmiStepTimeScale& anOtherScale)
{
//	NFmiStepScale :: operator=(anOtherScale);
	delete itsScale;
	itsScale = new NFmiTimeScale(*static_cast<NFmiTimeScale *>(anOtherScale.Scale()));

    itsMinStepValue = anOtherScale.itsMinStepValue;
	itsDirection = anOtherScale.itsDirection;
	itsValue = anOtherScale.itsValue;
	return *this;
}
//_____________________________________________________________________________________
float NFmiStepTimeScale :: Round(float theValue)const
{    // pitäisi saada Aikaluokasta
	float newValue;
	if(theValue > 14401)
		newValue = 28800.f;
	else if(theValue > 7201)
		newValue = 14400.f;
	else if(theValue > 5761)
		newValue = 7200.f;
	else if(theValue > 4321)
		newValue = 5760.f;
	else if(theValue > 2881)
		newValue = 4320.f;
	else if(theValue > 1441)
		newValue = 2880.f;
	else if(theValue > 721)
		newValue = 1440.f;
	else if(theValue > 361)
		newValue = 720.f;
	else if(theValue > 181)
		newValue = 360.f;
	else if(theValue > 121)
		newValue = 180.f;
	else if(theValue > 60.5)
		newValue = 120.f;
	else if(theValue > 30.5)
		newValue = 60.f;
	else if(theValue > 10.5)
		newValue = 30.f;
	else if(theValue > 5.1)
		newValue = 10.f;
	else if(theValue > 2.01)
		newValue = 5.f;
	else if(theValue > 1.01)
		newValue = 2.f;
	else
		newValue = 1.f;
	return newValue;
}
//______________________________________________________________________________
void NFmiStepTimeScale :: CalculateStartGap(void)
{
		//voivat olla negatiivisia
	float absStart = static_cast<float>((static_cast<NFmiTimeScale *>(Scale()))->StartTime().DifferenceInMinutes(NFmiTime(1996,1,1,12)));
	float absEnd = absStart + Scale()->EndValue();
	itsStartGap = static_cast<float>(floor((absStart-.0001f*(absEnd-absStart))/itsStepValue)+1)
					  * itsStepValue - absStart;
}
/*
//_____________________________________________________________________________________
void NFmiStepTimeScale :: TimeStartGap(const NFmiTime& theTime)
{
	StartGap(theTime.DifferenceInMinutes(itsStartTime));
}
*/
//______________________________________________________________________________
//inline

bool NFmiStepTimeScale :: NextTime (NFmiTime &theValue)
{
	Step();

	theValue = (static_cast<NFmiTimeScale *>(Scale()))->StartTime();
	theValue.ChangeByMinutes(static_cast<long>(itsValue)); // 11.1.2001/MArko Poistin short castin

	return itsScale->Inside(itsValue);
}
//______________________________________________________________________________
//inline
TFmiTime NFmiStepTimeScale :: CurrentTime (void)const
{
	TFmiTime aTime = TFmiTime((static_cast<NFmiTimeScale *>(Scale()))->StartTime());
 // TÄMÄ PITÄISI SAADA AIKALUOKKAAN:
	short days = short(itsValue/1440);
	long minutes = long(itsValue) % 1440;
	aTime.ChangeByDays(days);
	aTime.ChangeByMinutes(minutes);
	return aTime;
}
//______________________________________________________________________________
 // Marko/1998.12.3
NFmiMetTime NFmiStepTimeScale::ValueToTime(const float& theValue)const 
{
	return (static_cast<NFmiTimeScale *>(itsScale))->GetTime(theValue);
}

NFmiTime NFmiStepTimeScale::StartTime(void) const 
{
	return (static_cast<NFmiTimeScale *>(itsScale))->StartTime();
}
 
NFmiTime NFmiStepTimeScale::EndTime(void) const 
{
	return (static_cast<NFmiTimeScale *>(itsScale))->EndTime();
}
 
NFmiTime NFmiStepTimeScale::FirstTime(void) const 
{
	return (static_cast<NFmiTimeScale *>(itsScale))->ValueToTime(FirstValue());
} //eka steppiaika //061196

NFmiTime NFmiStepTimeScale::LastTime(void) const 
{
	return (static_cast<NFmiTimeScale *>(itsScale))->ValueToTime(LastValue());
} //vika steppiaika //061196

