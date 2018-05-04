
#pragma once

#include "NFmiMetTime.h"
#include "NFmiString.h"

 
class NFmiStringTime : public NFmiMetTime 
{ 
// friend ostream& operator<< ( ostream& oStream, const TMetTime& myTime); 
 
 public: 
	NFmiStringTime( void );                              
//    NFmiStrTime( const int timeStepInMinutes )  // cannot access private m.
//     : NFmiMetTime() { ConstructMetTime( timeStepInMinutes ); }
    NFmiStringTime ( const long datePart, const long timePart ) 
    : NFmiMetTime ( datePart,  timePart ) { SetTimeStep( 60 ); }
    NFmiStringTime( const TMetTime& aMetTime ) 
     : NFmiMetTime( aMetTime){};

	NFmiString WeekdayName(FmiLanguage theLanguage=kFinnish)const;	//030497

}; 
 
typedef NFmiStringTime* PNStrTime; 
 
