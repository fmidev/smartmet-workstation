#include  "NFmiTextTime.h" 
 
 
static 		//220597/LW
const char 
    *dayTimes [] = 
	{ " Yö  ", "Aamu ", "Päivä", "Ilta ", 	//230597/LW 5merkkiä, oli 7

	  "Natt ", "Morg.", " Dag ", "Afton",

	  "Night", "Morn.", " Day ", "Even."}; 

static 
const char 
    *weekdays [] = 
	{"Maanantai", "Tiistai", "Keskiviikko", "Torstai", 
                   "Perjantai", "Lauantai", "Sunnuntai",

	  "Måndag", "Tisdag", "Onsdag", "Torsdag", 
                   "Fredag", "Lördag", "Söndag",

	  "Monday", "Tuesday", "Wednesday", "Thursday", 
                   "Friday", "Saturday", "Sunday"};

static
const char 
    *months [] = 
	{"Tammikuu", "Helmikuu", "Maaliskuu", "Huhtikuu", 
     "Toukokuu", "Kesäkuu", "Heinäkuu", "Elokuu",
     "Syyskuu", "Lokakuu", "Marraskuu", "Joulukuu",

	 "Januari", "Februari", "Mars", "April", 
     "Maj", "Juni", "Juli", "Augusti",
     "September", "Oktober", "November", "December",


	 "January", "February", "March", "April", 
     "May", "June", "July", "August",
     "September", "October", "November", "December"};
//__________________________________________________________________ 
NFmiTextTime :: NFmiTextTime (void) 
              : NFmiMetTime()
{
}
//100697/LW__________________________________________________________________ 
NFmiTextTime ::NFmiTextTime(const NFmiTime& aTime)
              :NFmiMetTime()
{
	SetDate (aTime.GetYear(), aTime.GetMonth(), aTime.GetDay()); 
    SetTime( aTime.GetHour(), aTime.GetMin(), aTime.GetSec()); 
}
//220597/LW__________________________________________________________________ 
NFmiString NFmiTextTime :: DayTimeName(const FmiLanguage theLanguage) const 
{ 
   return NFmiString( dayTimes[(theLanguage-1)*4+GetHour()/6]);
   
} 
//__________________________________________________________________ 
NFmiString NFmiTextTime :: MonthName(const FmiLanguage theLanguage) const 
{ 
   return NFmiString( months[(theLanguage-1)*12+GetMonth() - 1]); //120597/LW
} 

//__________________________________________________________________ 
NFmiString NFmiTextTime :: Weekday(const FmiLanguage theLanguage) const 
{ 
   return NFmiString( weekdays[(theLanguage-1)*7+GetWeekday() - 1]);
} 
