/*-------------------------------------------------------------------------------------*/
//© Ilmatieteenlaitos/Lasse

//Originaali 28.12.1994

//Muutettu   10.1.1995/LW  NFmiTimeScale, NFmiStepScale virtuaalisiksi
//Muutettu   06.11.1995/LW
//Muutettu   15.11.1995/LW Uusi konstruktori
//Muutettu   18.1.1996/LW itsMinStepValue
// 061196/LW FirstTime(), LastTime()
// 060697/LW TFmiTime->NFmiTime
// 031298/Marko Added ValueToTime()-method.
//*-------------------------------------------------------------------------------------*/

#pragma once

#include "NFmiStepScale.h"
#include "NFmiMetTime.h"


class NFmiTimeScale;
class NFmiTimeBag;
//_________________________________________________________ NFmiStepTimeScale

class NFmiStepTimeScale : public  NFmiStepScale //virtual
//								, public virtual NFmiScale // Stroustrup
{
//friend class NFmiTimeScaleIterator;
public:
	NFmiStepTimeScale(void);
	NFmiStepTimeScale (const NFmiTimeScale& theScale
	                  ,unsigned long theStepInMinutes=0
					  ,unsigned long theStartGapInMinutes=0
					  ,FmiDirection theDirection = kForward);
	NFmiStepTimeScale (unsigned long theMinStepValue 
	                  ,const NFmiTimeScale& theScale 
	                  ,unsigned long theStepInMinutes=0
					  ,unsigned long theStartGapInMinutes=0
					  ,FmiDirection theDirection = kForward);
	NFmiStepTimeScale (const NFmiTime& theStartTime
	                  ,const NFmiTime& theEndTime
	                  ,unsigned long theStepInMinutes=0
					  ,unsigned long theStartGapInMinutes=0
					  ,FmiDirection theDirection = kForward);
	NFmiStepTimeScale (const NFmiTimeBag& theTimeBag);
	NFmiStepTimeScale (const NFmiStepTimeScale& anOtherScale);
	virtual ~NFmiStepTimeScale(void);
	NFmiStepTimeScale& operator= (const NFmiStepTimeScale& anOtherScale);
//	void TimeStepValue (const float theTimeStep)	{itsStepValue = Location(theTimeStep);};

	void TimeStartGap(const NFmiTime& theTime);

	bool NextTime (NFmiTime& theValue);
//	bool NextTime (void);
	NFmiTime CurrentTime (void)const;
	NFmiTime StartTime(void) const; 
 	NFmiTime EndTime  (void) const; 
	NFmiTime FirstTime(void) const;
	NFmiTime LastTime (void) const;
	NFmiMetTime ValueToTime(const float& theValue) const; // Marko/1998.12.3

protected:
	float Round(float theValue)const;
	void  CalculateStartGap(void);

private:
//	NFmiTime itsStartTime;

};

