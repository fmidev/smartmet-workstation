#include "NFmiStepScale.h"
#include "NFmiScale.h"

#include <algorithm>
#include <iostream>

//______________________________________________________________________________
NFmiStepScale :: NFmiStepScale (const NFmiScale& theScale, float theStep, float theStartGap
								,FmiDirection theDirection, bool areStepsOnBorders)
					: itsScale (new NFmiScale(theScale))
					, itsDirection(theDirection)
					, fStepsOnBorders(areStepsOnBorders)
					, itsStepValue(theStep)
					, itsMinStepValue(1.f)
					, itsStartGap(theStartGap)
					, itsCounter(0)
					, itsCounterLimit(5000)
{
	if (itsStepValue == kFloatMissing) StepNumber(5);	// 21.12/näin gappi myös järkeväksi
	SetStepsOnBorders();
	Reset();
}
//______________________________________________________________________________
NFmiStepScale :: NFmiStepScale (float theMinStepValue 
                               ,const NFmiScale& theScale
                               ,float theStep
                               ,float theStartGap
							   ,FmiDirection theDirection
							   ,bool areStepsOnBorders)
					: itsScale (new NFmiScale(theScale))
					, itsDirection(theDirection)
					, fStepsOnBorders(areStepsOnBorders)
					, itsStepValue(theStep)
					, itsMinStepValue(theMinStepValue)
					, itsStartGap(theStartGap)
					, itsCounter(0)
					, itsCounterLimit(5000)
{
	if (itsStepValue == kFloatMissing) 
		 StepNumber(static_cast<short>(std::max(5.0f, itsMinStepValue)));
	
	if(fStepsOnBorders) SetStepsOnBorders();
	Reset();
}
//______________________________________________________________________________
NFmiStepScale :: NFmiStepScale (const float& theStartValue
                               ,const float& theEndValue
                               ,float theStep
                               ,float theStartGap
                               ,FmiDirection theDirection
							   ,bool areStepsOnBorders
							   ,int theMaxStepNumber)
					: itsScale (new NFmiScale(theStartValue, theEndValue))
					, itsDirection(theDirection)
					, fStepsOnBorders(areStepsOnBorders)
					, itsStepValue(theStep)
					, itsMinStepValue(1.f)
					, itsStartGap(theStartGap)
					, itsCounter(0)
					, itsCounterLimit(5000)
{
	if (itsStepValue == kFloatMissing) 
		StepNumber(theMaxStepNumber);	// 21.12/näin gappi myös järkeväksi
	if(fStepsOnBorders) SetStepsOnBorders();
	Reset();
}
//______________________________________________________________________________
//inline
NFmiStepScale :: NFmiStepScale (const NFmiStepScale& anOtherScale)
			  : itsScale(new NFmiScale(*anOtherScale.Scale()))
				,itsDirection(anOtherScale.itsDirection)
				,fStepsOnBorders(anOtherScale.fStepsOnBorders)
				,itsStepValue (anOtherScale.itsStepValue)
				,itsMinStepValue(anOtherScale.itsMinStepValue)
				,itsStartGap (anOtherScale.itsStartGap)
				,itsCounter(anOtherScale.itsCounter)
				,itsCounterLimit(anOtherScale.itsCounterLimit)
{
//	Reset();
}
//______________________________________________________________________________
//inline
NFmiStepScale :: ~NFmiStepScale(void)
{
	delete itsScale;
}
//______________________________________________________________________________
std::ostream& NFmiStepScale :: Write(std::ostream &file) const
{
	itsScale->Write(file);
	file << itsStepValue << " ";
	file << itsMinStepValue << " ";
	return file;
}
//______________________________________________________________________________
std::istream& NFmiStepScale :: Read(std::istream &file)
{
	itsScale->Read(file);
	file >> itsStepValue;
	file >> itsMinStepValue;
	return file;
}
//______________________________________________________________________________
//inline
NFmiStepScale& NFmiStepScale :: operator= (const NFmiStepScale& anOtherScale)
{
	delete itsScale;
	itsScale = new NFmiScale(*anOtherScale.Scale());
	itsStepValue = anOtherScale.itsStepValue;
	itsStartGap = anOtherScale.itsStartGap;
	itsDirection = anOtherScale.itsDirection;
	itsValue = anOtherScale.itsValue;
	itsMinStepValue = anOtherScale.itsMinStepValue;
	itsCounter = anOtherScale.itsCounter;
	itsCounterLimit = anOtherScale.itsCounterLimit;

	return *this;
}
//______________________________________________________________________________
//inline
NFmiStepScale& NFmiStepScale :: operator+= (const NFmiScale& aScale)
{
    *itsScale += aScale;
	if(Scale()->Ok()) 
	{
	   if(itsStepValue == kFloatMissing)  // toimiiko?,  10000?
	            StepNumber(5);
 	   CalculateStartGap();
	}

	return *this;
}
//______________________________________________________________________________
//inline
NFmiStepScale& NFmiStepScale :: operator+= (const NFmiStepScale& aScale)
{
    return operator +=(*aScale.Scale()); 
 //   *itsScale += *aScale.Scale();
//	CalculateStartGap();

//	return *this;
}
//______________________________________________________________________________
void NFmiStepScale :: SetScale(float theStartValue, float theEndValue
							   ,bool setToBorders)  
{
	itsScale->Set(theStartValue,theEndValue);
	CalculateStartGap();	

	fStepsOnBorders = setToBorders;   //ei oikeastaan vielä tarvita
	if(setToBorders)
	{
		SetStepsOnBorders();
	}
}
//______________________________________________________________________________
float NFmiStepScale :: FirstValue(void) const //ei siis StartValue()
{
	//jos menee ulos voisi palauttaa missing
	return StartValue() + itsStartGap;
}
//______________________________________________________________________________
float NFmiStepScale :: LastValue(void) const
{	// tarvittaisko epsilonia
	return static_cast<float>(floor((EndValue() - FirstValue())/itsStepValue) * itsStepValue + FirstValue());
}
//301198______________________________________________________________________________
void NFmiStepScale :: SetStepsOnBorders(void) 
{
	fStepsOnBorders = true;
	double eps = (EndValue() - FirstValue()) * .001;

	NFmiScale* scale = Scale();

	if(scale->EndValue()-LastValue() > eps)   //ennen alkupistettä
		scale->SetEndValue(LastValue()+itsStepValue);

	if(FirstValue() - scale->StartValue() > eps)
		scale->SetStartValue(FirstValue()-itsStepValue);
 
	itsStartGap = 0.;
	Reset();
}
//______________________________________________________________________________
void NFmiStepScale :: StepNumber(const FmiCounter& theMaxNumber)
{            // with rounding
    if(Scale()->Ok())
	{
	   itsStepValue = Round(itsScale->Difference() / std::max(1, static_cast<int>(theMaxNumber)));
	   itsStepValue = std::max(itsStepValue, itsMinStepValue);

	   Update(fStepsOnBorders);
	}
}
//1.12.98______________________________________________________________________________
void NFmiStepScale :: Update(bool stepsOnBorders) 
{   //tarvitaan mm jos skaalaan (NFmiScale) suoraan tehdään muutoksia, esim arvovälin laajennus							  
	fStepsOnBorders = stepsOnBorders;
	CalculateStartGap();
	if(fStepsOnBorders)
		SetStepsOnBorders();
	Reset();      
}
//______________________________________________________________________________
FmiCounter NFmiStepScale :: StepNumber(void) const
{   							   //061196/LW
   return FmiCounter((EndValue() - FirstValue()) / itsStepValue + .0001) + 1;     
}
//______________________________________________________________________________
void NFmiStepScale :: CalculateStartGap(void)
{
  itsStartGap = (static_cast<float>(floor ((StartValue()-.0001f*(EndValue()-StartValue()))/itsStepValue))+1)
	* itsStepValue - StartValue();
}

//______________________________________________________________________________
//inline
float NFmiStepScale :: StepValue(void)const
{
	return itsStepValue;
}
//______________________________________________________________________________
float NFmiStepScale :: Round(float theValue) const
{                       // to 1, 2 or 5 * 10**n
	float aLog = static_cast<float>(log10(theValue));
	float exponent = floor(aLog);
	float mantissa = aLog - exponent;
	if (mantissa < 0.f) mantissa = 1.f - mantissa;
	float value;
	if (mantissa > .7)
	{
		value = 1.f;
		exponent++;
	}
	else if (mantissa > .302)
	{
		value = 5.f;
	}
	else if (mantissa == 0.)
	{
		value = 1.f;
	}
	else
	{
		value = 2.f;
	}
	return static_cast<float>(value * pow(10.f, exponent));
}
//______________________________________________________________________________
FmiCounter NFmiStepScale :: operator% (FmiCounter theFactor)
{
	return (theFactor == 0) ? kShortMissing : itsCounter % theFactor;
}
//______________________________________________________________________________
//inline
void NFmiStepScale :: Reset (void)
{
	itsValue = (itsDirection == kForward) ?   //else backward
		 StartValue() + itsStartGap - itsStepValue
	  : EndValue() - itsStartGap + itsStepValue;
												 // startgap loppuun kun toisinpäin!!
	itsCounter = 0;
}

//______________________________________________________________________________
//inline
void NFmiStepScale :: Step (void)
{
	itsValue = (itsDirection == kForward) ?   //else backward
					itsValue + itsStepValue
				  :itsValue - itsStepValue;
	itsCounter++;
}
//______________________________________________________________________________
//inline
void NFmiStepScale :: StepBack (void)
{
	itsValue = (itsDirection == kBackward) ?
					itsValue + itsStepValue
				  :itsValue - itsStepValue;
	itsCounter--;
}
//______________________________________________________________________________
//inline
bool NFmiStepScale :: Next (float &theValue)
{
	Step();
	theValue = itsValue;
	return IsInside();
}
//______________________________________________________________________________
//inline
bool NFmiStepScale :: Next (void)
{
	Step();
	return IsInside();
}
//______________________________________________________________________________
//inline
bool NFmiStepScale :: Value (float &theValue)
{   // ei kyllä toimi täydellisesti: itsCounter ja alkuGappi!!

	 // IsInside jätetty pois koska resetin jälkeinen -startgap ei hyväksytty

//	if (IsInside(theValue))
//	{
		itsValue = theValue;
		return isTrue;
//	}
//	else
//	{
//		return isFalse;
//	}
}
//______________________________________________________________________________
//inline
float NFmiStepScale :: Value (void) const
{
	return itsValue;
}
//______________________________________________________________________________
//inline
float NFmiStepScale :: Location (void) const
{
	return itsScale->RelLocation(itsValue);
}
//______________________________________________________________________________
//inline
bool NFmiStepScale :: IsInside (void) const
{
	if(itsCounterLimit > 0 && itsCounter > itsCounterLimit) // jos itsCounterLimit on 0 tai negatiivinen, ei tarkastusta. Jos itsCounter on mennyt rajan yli, palautetaan vain false, joka lopettaa loopit
		return false;
	return itsScale->Inside(itsValue); // ettei oikealta mene yli
}

//______________________________________________________________________________
//inline
bool NFmiStepScale :: IsInside (float theValue) const
{
	return itsScale->Inside(theValue);
}
//______________________________________________________________________________
//inline
bool NFmiStepScale :: NextInside (void)
{
	Step();
	bool nextInside = IsInside();
	StepBack();
	return nextInside;
}
//______________________________________________________________________________
//inline
bool NFmiStepScale :: LastInside (void) //before present
{
	StepBack();
	bool lastInside = IsInside();
	Step();
	return lastInside;
}
//______________________________________________________________________________
//inline
bool NFmiStepScale :: IsLast (void)
{
	return IsInside() && !NextInside();
}
//______________________________________________________________________________
//inline
bool NFmiStepScale :: IsFirst (void)
{
	return IsInside() && !LastInside();
}
//______________________________________________________________________________
//inline
bool NFmiStepScale :: IsMidst (void) const
{
	float midstCounter = (float(StepNumber())+1.f) / 2.f;
	return (itsCounter >= midstCounter - .6f
		&& itsCounter < midstCounter +.1f) ;
}
//______________________________________________________________________________

float NFmiStepScale::StartValue(void) const
{
	return itsScale->StartValue();
}

float NFmiStepScale::EndValue(void) const
{
	return itsScale->EndValue();
}
