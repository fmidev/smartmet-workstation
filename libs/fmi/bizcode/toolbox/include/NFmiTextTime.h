/*---------------------------------------------------------------------------*/ 
//© Ilmatieteenlaitos/Lasse.               nfmitexttime.h 
// 
//Originaali 6. 6.1997 
// 
// 060697/LW string-muotoiset vastaukset (kk, viikonpäivä, vrk.aika) siirretty tänne NFmiMetTimestä
// 090697/LW uusi konstruktori
// 100697/LW NFmiTextTime (NFmiTime&) muutettu
/*---------------------------------------------------------------------------*/
 
#pragma once
 
#include  "NFmiMetTime.h" 
 
class NFmiTextTime : public  NFmiMetTime 
{ 
 public: 
    NFmiTextTime (void);
    NFmiTextTime (const NFmiTime& aTime);// :NFmiMetTime(aTime){;}
    NFmiTextTime (const NFmiMetTime& aMetTime )  //090697
     : NFmiMetTime( aMetTime){;};
 //   NFmiTime ( const long datePart, const long timePart ); 
 //   NFmiTime (const short year, const short month, const short day) 
 //   : NFmiStaticTime (year, month, day) {SetZoneDifferenceHour(); }
 //   NFmiTime (const short year, const short month, const short day 
//	     ,const short hour, const short minute=0, const short sec=0) 
 //   : NFmiStaticTime (year, month, day, hour, minute, sec) { SetZoneDifferenceHour();}
 
    NFmiString	Weekday(const FmiLanguage theLanguage = kFinnish) const;
	NFmiString	MonthName(const FmiLanguage theLanguage = kFinnish) const;
	NFmiString	DayTimeName(const FmiLanguage theLanguage = kFinnish) const;
  //  void		PrintWeekday(void) const; 

 protected: 
 private:
}; 
 
