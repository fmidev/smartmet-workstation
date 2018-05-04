/*-------------------------------------------------------------------------------------*/
//© Ilmatieteenlaitos/Lasse

//Originaali 11.3.1996

//*-------------------------------------------------------------------------------------*/

#include "NFmiStringTime.h"
									  
const NFmiString weekdays("MaTiKeToPeLaSuMåTiOnToFrLöSöMoTuWeThFrSaSu");

NFmiString NFmiStringTime :: WeekdayName(FmiLanguage theLanguage) const
{
   return weekdays.GetChars((GetWeekday()-1 + (theLanguage-1)*7)*2+1,2); //030497
}
