/*-------------------------------------------------------------------------------------*/
//© Ilmatieteenlaitos/Lasse

//Originaali 07. 1.1995

//Muutettu   25.10.1995/LW  Pirintä NFmiStepTimeScale:istä eikä  NFmiTimeScale:istä
//Muutettu   26.10.1995/LW  SecondaryText()
//Muutettu    1.11.1995/LW
//Muutettu    7.11.1995/LW  Vain skaala, ei iteraattoria
//Muutettu   10.11.1995/LW  CurrentTime
//Muutettu   15.11.1995/LW  Uusi konstruktori
//Muutettu   15.11.1995/LW  Uusi konstruktori
//Muutettu   12.3.1996/LW   itsDayTextAsWeekday
// 030497/LW +itsLanguage
// 220597/LW saa kellonajan myös tekstein yö/aamu/päivä/ilta
// 060697/LW TFmiTime->NFmiTime
///*-------------------------------------------------------------------------------------*/


#pragma once

#include "NFmiAxis.h"

class NFmiStepTimeScale;
class NFmiTime;
//_________________________________________________________ NFmiTimeAxis

class NFmiTimeAxis : public  NFmiAxis 
{
 public:
	NFmiTimeAxis (void);
	NFmiTimeAxis (const NFmiStepTimeScale& theScale, const NFmiTitle& theTitle);
	NFmiTimeAxis (const NFmiTime& theStartTime, const NFmiTime& theEndTime, const NFmiTitle& theTitle);
	NFmiTimeAxis (const NFmiTimeAxis& anOtherAxis);
	virtual ~NFmiTimeAxis(void);
	NFmiTimeAxis& operator= (const NFmiTimeAxis& anOtherAxis);

	NFmiString Text (void);
	NFmiString SecondaryText (void)const;
	NFmiString MinuteText (void)const;
	NFmiString HourText (void)const;
	NFmiString DayText (void)const;
	NFmiString DayName (void)const;
    NFmiString DayTimeName (void)const; //220597
	NFmiTime CurrentTime (void) const;
    NFmiTime StartTime (void) const;
    NFmiTime EndTime (void) const;
    void DayTextAsWeekday(bool boolean, FmiLanguage theLanguage=kFinnish){itsDayTextAsWeekday=boolean; //030497
	                                                                            itsLanguage = theLanguage;};
    void HourTextAsDayTime(bool boolean, FmiLanguage theLanguage=kFinnish){itsHourTextAsDayTime=boolean; //220597
	                                                                            itsLanguage = theLanguage;};
    bool DayTextAsWeekday(void)const {return itsDayTextAsWeekday;};
    bool HourTextAsDayTime(void)const {return itsHourTextAsDayTime;};	//220597
protected:
private:
	bool itsDayTextAsWeekday;
	bool itsHourTextAsDayTime; //220597
	FmiLanguage itsLanguage;
};

