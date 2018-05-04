/*-------------------------------------------------------------------------------------*/
//© Ilmatieteenlaitos/Lasse

//Originaali 8. 12.1994

//Muutettu   13.10.1995/LW
//Muutettu   19.10.1995/LW  GetTime(float), ValueToTime(float)
//Muutettu    6.11.1995
//Muutettu   10.11.1995/LW Expand()
//060697/LW TFmiTime->NFmiTime

//*-------------------------------------------------------------------------------------*/

#include "NFmiTimeScale.h"

#include <stdlib.h>

//______________________________________________________________________________
//inline
NFmiTimeScale :: NFmiTimeScale (void)
					: NFmiScale()
					, itsStartTime(NFmiTime(1996,1,1,12))
{
}
//______________________________________________________________________________
//inline
NFmiTimeScale :: NFmiTimeScale (const NFmiTime& theStartTime
										 ,const NFmiTime& theEndTime
										 ,unsigned long theExpansionInMinutes)
					: NFmiScale (0.f, kFloatMissing)  // missing ylikirjoitetaan alla
					, itsStartTime(theStartTime)
{
	long tempShort = static_cast<long>(theExpansionInMinutes); // 11.1.2001/MArko Muutin short:in longiksi
	itsStartTime.ChangeByMinutes(-tempShort);
	itsEndValue = RelTime(theEndTime) + theExpansionInMinutes;
	Check();
}
//______________________________________________________________________________
//inline
NFmiTimeScale :: NFmiTimeScale (const NFmiTimeScale& anOtherScale)
					: NFmiScale (anOtherScale)
{
	itsStartTime = anOtherScale.itsStartTime;
}
//______________________________________________________________________________
//inline
NFmiTimeScale :: ~NFmiTimeScale(void)
{
}
//______________________________________________________________________________
//inline
NFmiTimeScale& NFmiTimeScale :: operator= (const NFmiTimeScale& anOtherScale)
{
	NFmiScale :: operator=(anOtherScale);
	itsStartTime = anOtherScale.itsStartTime;
	return *this;
}

//____________ei inline koska some return statement__________________________________________________________________
NFmiTime NFmiTimeScale :: EndTime (void)const
{
	return ValueToTime(itsEndValue);
}

//______________________________________________________________________________
NFmiTime NFmiTimeScale :: GetTime (float theRelLocation) const
{									//Time
	return ValueToTime(Location(theRelLocation));
}
//______________________________________________________________________________

//inline

float NFmiTimeScale :: RelTimeLocation (const NFmiTime& theTime) const

{

	return RelLocation(RelTime(theTime));

}
//______________________________________________________________________________
//inline
void NFmiTimeScale :: Expand (float theInterval)
{
	itsStartTime.ChangeByMinutes(static_cast<long>(-theInterval)); // 11.1.2001/MArko Poistin short castin
	itsEndValue += 2.f*theInterval;
}
//______________________________________________________________________________
//inline
void NFmiTimeScale :: ExpandIfEqual (float theInterval)
{
	if(itsStartValue == itsEndValue)
		Expand(theInterval);
}
//______________________________________________________________________________
NFmiTime NFmiTimeScale :: ValueToTime (float theLocation) const
{
	NFmiTime time(itsStartTime);
//	time.ChangeByMinutes((short)theLocation);
	time.ChangeByMinutes(static_cast<long>(theLocation)); // 11.1.2001/MArko Poistin short castin
	return time;
}
//______________________________________________________________________________
//inline
void NFmiTimeScale :: SetTimes (const NFmiTime& theStartTime, const NFmiTime& theEndTime)
{
	SetStartTime(theStartTime);
	SetEndTime(theEndTime);
}
//______________________________________________________________________________
//inline
void NFmiTimeScale :: SetStartTime (const NFmiTime& theTime)
{        // loppuaika pysyy ennallaan
	NFmiTime endTime = EndTime();
	itsStartTime = theTime;
	SetEndTime(endTime);
//	SetEndValue (EndValue() - RelTime(theTime));
}
//______________________________________________________________________________
//inline
void NFmiTimeScale :: SetEndTime (const NFmiTime& theTime)
{
	SetEndValue(RelTime(theTime));
}
//______________________________________________________________________________
//inline
NFmiTime NFmiTimeScale :: StartTime (void)const
{
	return itsStartTime;
}

//______________________________________________________________________________
//inline
float NFmiTimeScale :: RelTime (const NFmiTime& theTime) const
{
	return static_cast<float>(theTime.DifferenceInMinutes(itsStartTime));
}

//______________________________________________________________________________
//inline
bool  NFmiTimeScale :: TimeInside (const NFmiTime& theTime)const
{
	return Inside(RelTime(theTime));
}
//______________________________________________________________________________
void NFmiTimeScale :: Move (const NFmiTime &newTime)
{
  long differenceFromEnd =  newTime.DifferenceInMinutes (EndTime()); 
  long differenceFromStart =  newTime.DifferenceInMinutes (StartTime());
   
  NFmiTime startTime(StartTime());
  NFmiTime endTime(EndTime());
  if(abs(int(differenceFromEnd)) > abs(int(differenceFromStart)))
  {
    startTime.ChangeByMinutes(differenceFromStart); // 11.1.2001/MArko Poistin short castin
    endTime.ChangeByMinutes(differenceFromStart); // 11.1.2001/MArko Poistin short castin
  }
  else
  {
    startTime.ChangeByMinutes(differenceFromEnd); // 11.1.2001/MArko Poistin short castin
    endTime.ChangeByMinutes(differenceFromEnd); // 11.1.2001/MArko Poistin short castin
  }
    SetStartTime(startTime);
    SetEndTime(endTime);
}
//______________________________________________________________________________
//______________________________________________________________________________
void NFmiTimeScale :: Resize (const NFmiTime &newTime)
{
  long differenceFromEnd =  newTime.DifferenceInMinutes (EndTime()); 
  long differenceFromStart =  newTime.DifferenceInMinutes (StartTime()); 
  if(abs(int(differenceFromEnd)) > abs(int(differenceFromStart)))
  {
    NFmiTime startTime(StartTime());
    startTime.ChangeByMinutes(differenceFromStart); // 11.1.2001/MArko Poistin short castin
    SetStartTime(startTime);
  }
  else
  {
    NFmiTime endTime(EndTime());
    endTime.ChangeByMinutes(differenceFromEnd); // 11.1.2001/MArko Poistin short castin
    SetEndTime(endTime);
  }
}
//______________________________________________________________________________
//______________________________________________________________________________
void NFmiTimeScale :: MoveLeft (const NFmiTime &newTime)
{
  long differenceFromStart =  newTime.DifferenceInMinutes (StartTime());
   
  NFmiTime startTime(StartTime());
  startTime.ChangeByMinutes(differenceFromStart); // 11.1.2001/MArko Poistin short castin
  SetStartTime(startTime);
  
  NFmiTime endTime(EndTime());
  endTime.ChangeByMinutes(differenceFromStart); // 11.1.2001/MArko Poistin short castin
  SetEndTime(endTime);
}
//______________________________________________________________________________
//______________________________________________________________________________
void NFmiTimeScale :: MoveRight (const NFmiTime &newTime)
{
  long differenceFromEnd =  newTime.DifferenceInMinutes (EndTime());

  NFmiTime startTime(StartTime());
  startTime.ChangeByMinutes(differenceFromEnd); // 11.1.2001/MArko Poistin short castin
  SetStartTime(startTime);
  
  NFmiTime endTime(EndTime());
  endTime.ChangeByMinutes(differenceFromEnd); // 11.1.2001/MArko Poistin short castin
  SetEndTime(endTime);
}
