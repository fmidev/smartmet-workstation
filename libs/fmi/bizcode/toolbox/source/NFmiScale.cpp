#include "NFmiScale.h"

#include <algorithm>
#include <iostream>

//______________________________________________________________________________
//inline
NFmiScale :: NFmiScale (void)
			  : itsStartValue (kFloatMissing)
			  , itsEndValue (kFloatMissing)
			  , itsEpsilon(0.f)
			  , itsDataOk(false)
			  , itsLimitCheck(false)
{
}
//______________________________________________________________________________
//inline
NFmiScale :: NFmiScale (float theStartValue, float theEndValue)
			  : itsStartValue (theStartValue)
			  , itsEndValue (theEndValue)
			  , itsEpsilon(0.f)
			  , itsLimitCheck(false)
{

  Check();
}

//______________________________________________________________________________
//inline
NFmiScale :: NFmiScale (const NFmiScale& anOtherScale)
  : itsStartValue (anOtherScale.itsStartValue)
  , itsEndValue (anOtherScale.itsEndValue)
  , itsEpsilon (anOtherScale.itsEpsilon)
  , itsDataOk(anOtherScale.itsDataOk)
  , itsLimitCheck(anOtherScale.itsLimitCheck)
{
}
//______________________________________________________________________________
//inline
NFmiScale :: ~NFmiScale(void)
{
}
//______________________________________________________________________________
//inline
NFmiScale& NFmiScale :: operator= (const NFmiScale& anOtherScale)
{
	itsStartValue = anOtherScale.itsStartValue;
	itsEndValue = anOtherScale.itsEndValue;
	itsDataOk = anOtherScale.itsDataOk;
	itsLimitCheck = anOtherScale.itsLimitCheck;
	itsEpsilon = anOtherScale.itsEpsilon;

	return *this;
}
//______________________________________________________________________________
//inline
NFmiScale& NFmiScale :: operator+= (const NFmiScale& anOtherScale)
{	   // oletetaan että start < end
	if(itsStartValue == kFloatMissing)
	{
	   itsStartValue = anOtherScale.itsStartValue;
	}
	else if (anOtherScale.itsStartValue != kFloatMissing)
	{
	   itsStartValue = std::min(anOtherScale.itsStartValue, itsStartValue);
	}
	if (itsEndValue == kFloatMissing)
	{
	   	itsEndValue = anOtherScale.itsEndValue;
	}
	else if (anOtherScale.itsEndValue != kFloatMissing)
	{
	    itsEndValue = std::max(anOtherScale.itsEndValue, itsEndValue);
	}
	Check();
	return *this;
}
//______________________________________________________________________________
//inline
NFmiScale& NFmiScale :: operator-= (const NFmiScale& anOtherScale)
{	   // oletetaan että start < end
    if(Ok() && anOtherScale.Ok())
	{
	   if(Inside(anOtherScale.itsEndValue))
	   {
	      itsStartValue = anOtherScale.itsEndValue;
	   }
	   else if (Inside(anOtherScale.itsStartValue))
	   {
	      itsEndValue = anOtherScale.itsStartValue;
	   }
	   else if (anOtherScale.Inside(itsStartValue) && anOtherScale.Inside(itsEndValue))
	   {
	      itsStartValue = kFloatMissing;	   // vanha sisältyy täysin vähennettävään
	      itsEndValue = kFloatMissing;     
	   }
	}
	Check();
	return *this;
}
//______________________________________________________________________________
std::ostream& NFmiScale :: Write(std::ostream &file) const
{
	file << itsStartValue << " ";
	file << itsEndValue   << " ";
	return file;
}
//______________________________________________________________________________
std::istream& NFmiScale :: Read(std::istream &file)
{
	file >> itsStartValue;
	file >> itsEndValue;
	Check();
	return file;
}
//______________________________________________________________________________
//inline
void NFmiScale :: StartFromZeroOptionally (float theFactor)
{    // ei toimi laskevalle skaalalle
    if(!IsMissing()) // 041196/LW oli itsDataOk 
	{
	   if ((itsEndValue-itsStartValue)*theFactor > std::min(fabs(itsEndValue), fabs(itsStartValue))
		   && !Inside(0.f))
	   {
		   if(itsStartValue > 0.)
			   itsStartValue = 0.f;
	       else
			   itsEndValue = 0.f;
	   }
//	   Check();
	} 
}
//______________________________________________________________________________
//inline  //+041196/LW
bool NFmiScale :: IsMissing(void) const
{
  return itsStartValue == kFloatMissing ||
		 itsEndValue   == kFloatMissing; 
}
//______________________________________________________________________________
//inline
void NFmiScale :: Check (void)
{
  itsDataOk = itsStartValue != kFloatMissing &&
				  itsEndValue   != kFloatMissing; 
//			   && itsStartValue != itsEndValue;	  //120397 ******
  if (itsDataOk)            //041196/LW
      itsEpsilon = (itsEndValue - itsStartValue) * .001f;
  else 
	  itsEpsilon = .000001f; //041196/LW
}
//______________________________________________________________________________
//inline
void NFmiScale :: ExpandIfNotZero (float theFactor)
{
    if(itsDataOk)
	{
       float expansion = (itsEndValue - itsStartValue) * theFactor;
       if (itsStartValue != 0.)
           itsStartValue += -expansion; 
       if (itsEndValue != 0.)
          itsEndValue += expansion;
//       Check();
    } 
}
//______________________________________________________________________________
//inline
void NFmiScale :: ExpandIfEqual (float theInterval)
{
  if (itsStartValue == itsEndValue)	//041196/LW pois itsDataOk &&
  	  Expand (theInterval);
}
//______________________________________________________________________________
//inline
void NFmiScale :: Expand (float theInterval)
{
    if (!IsMissing()) //041196/LW oli itsDataOk
	{
	   itsStartValue -= theInterval;
	   itsEndValue += theInterval;
	   itsDataOk = true;                                  //041196/LW
       itsEpsilon = (itsEndValue - itsStartValue) * .001f; //041196/LW
	} 
}
//______________________________________________________________________________
//inline
bool NFmiScale :: Inside (float theValue) const
{	// epsilon näyttää tuovan yhden merkitsevän numeron lisää
	return itsDataOk && theValue+itsEpsilon >= itsStartValue && theValue-itsEpsilon <= itsEndValue
			  ? true : false;
}

//______________________________________________________________________________
//inline
float NFmiScale :: RelLocation (float theValue) const
{
  float value = kFloatMissing;
  if (itsDataOk && theValue != kFloatMissing)
	  if(Difference() > 0.f)   // 120397
		  value = (theValue-itsStartValue)/Difference();
      else
		  value = (theValue-itsStartValue)/(24.f*60.f); // tulee ongelmia, jos alku ja loppuaika samoja, teen silloin pakollisen 24h erotuksen

  return  value == kFloatMissing || !Inside(theValue) && itsLimitCheck
			? kFloatMissing : value;
}
//______________________________________________________________________________
//inline
float NFmiScale :: Location (float theRelValue) const
{
  return  itsDataOk ? Difference()*theRelValue + itsStartValue : kFloatMissing;
}








