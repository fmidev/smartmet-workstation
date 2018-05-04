/*-------------------------------------------------------------------------------------*/
//© Ilmatieteenlaitos/Lasse

//Originaali 12. 1.1995

//Muutettu   xx.12.1994/LW
//Muutettu   26.10.1995/LW  mm konstruktoriin lis‰tty NFmiScale(..)
//Muutettu    1.11.1995/LW
//Muutettu    7.11.1995/LW  Vain skaala, ei iteraattoria
//Muutettu    9.11.1995/LW
//Muutettu   15.11.1995/LW  Uusi konstruktori
//Muutettu   14.12.1995/LW  Uusi konstruktori
//Muutettu   12.3.1996/LW  itsDayTextAsWeekday
//251096/LW SecondaryText():iin lis‰tty kk.yy mahdollisuus, samoin pelkk‰ yy  
// HUOM Aikaluokkaan pit‰isi saada ChangeByMinutes(long) tai vastaava
// 030497/LW +itsLanguage
// 120597/LW
// 220597/LW "p‰iv‰naika tekstill‰" -optio
// 230597/LW
// 300597/LW
// 060697/LW NFmiTextTime sinne miss‰ tarvitaan
//           TFmiTime->NFmiTime 
// 090697/LW apu-NFmiTimet NMetTimeksi jotta ei paikallisaika-konversiota
// 100697/LW ed. peruttu, huomioitu textTimen konstr.
///*-------------------------------------------------------------------------------------*/

#include "NFmiTimeAxis.h"
#include "NFmiTextTime.h"
#include "NFmiValueString.h"
#include "NFmiStepTimeScale.h"

//______________________________________________________________________________
//inline
NFmiTimeAxis :: NFmiTimeAxis (void)
				  : NFmiAxis()
{
	itsDayTextAsWeekday = false;
	itsHourTextAsDayTime = false;
}
//______________________________________________________________________________
//inline
NFmiTimeAxis :: NFmiTimeAxis (const NFmiStepTimeScale& theScale, const NFmiTitle& theTitle)
				  : NFmiAxis()
{
	itsScale = new NFmiStepTimeScale(theScale);
	itsTitle = theTitle;
	itsDayTextAsWeekday = false;
	itsHourTextAsDayTime = false;
	Reset();
}
//______________________________________________________________________________
//inline
NFmiTimeAxis :: NFmiTimeAxis (const NFmiTime& theStartTime
                             ,const NFmiTime& theEndTime
                             ,const NFmiTitle& theTitle)
				  : NFmiAxis()
{
	itsScale = new NFmiStepTimeScale(theStartTime, theEndTime);
	itsTitle = theTitle;
	itsDayTextAsWeekday = false;
	itsHourTextAsDayTime = false;
	Reset();
}//______________________________________________________________________________
//inline
NFmiTimeAxis :: NFmiTimeAxis (const NFmiTimeAxis& anOtherAxis)
				 :  NFmiAxis() //anOtherAxis)
{
	itsScale = new NFmiStepTimeScale(*static_cast<NFmiStepTimeScale *>(anOtherAxis.itsScale));
	itsDayTextAsWeekday = anOtherAxis.itsDayTextAsWeekday;
	itsHourTextAsDayTime = anOtherAxis.itsHourTextAsDayTime;
	itsLanguage = anOtherAxis.itsLanguage;			  //030497
}
//______________________________________________________________________________
//inline
NFmiTimeAxis :: ~NFmiTimeAxis(void)
{
}
//______________________________________________________________________________
//inline
NFmiTimeAxis& NFmiTimeAxis :: operator= (const NFmiTimeAxis& anOtherAxis)
{
//	NFmiAxis:: operator= (anOtherAxis);
// eri tyyppi‰ kuin Axis:ssa :
	delete itsScale;
	itsScale = new NFmiStepTimeScale(*static_cast<NFmiStepTimeScale *>(anOtherAxis.itsScale));

	itsTitle = anOtherAxis.itsTitle;
	itsTicLengthRatio = anOtherAxis.itsTicLengthRatio;
	itsNumberOfSubIntervals = anOtherAxis.itsNumberOfSubIntervals;
	itsTicFormat = anOtherAxis.itsTicFormat;
	itsDayTextAsWeekday = anOtherAxis.itsDayTextAsWeekday;
	itsHourTextAsDayTime = anOtherAxis.itsHourTextAsDayTime;
	itsLanguage = anOtherAxis.itsLanguage; //030497
	return *this;
}
//______________________________________________________________________________
NFmiString NFmiTimeAxis ::Text (void)
{
	float step = StepValue();
	NFmiString string;
	if(step > 23*60)
		if(itsDayTextAsWeekday)
		    string = DayName();
	    else
		    string = DayText();
	else if(step > 50.)
		if(itsHourTextAsDayTime)	//220597
		    string = DayTimeName();
	    else
		    string = HourText();
	else
		string = MinuteText();

	return string;
}
//______________________________________________________________________________
NFmiString NFmiTimeAxis :: MinuteText (void)const
{
	NFmiValueString string(CurrentTime().GetMin(), "%hd");
	return string;
}
//______________________________________________________________________________
NFmiString NFmiTimeAxis :: HourText (void)const
{
	NFmiValueString string(CurrentTime().GetHour(), "%hd");
	return string;
}
//______________________________________________________________________________
NFmiString NFmiTimeAxis :: DayName (void)const
{ //040497 korvaa eka rivi tokalla+kolmannella kun Vili palauttaa myˆs ruotsin
  // korvattu 220597, toimiko?
	//   return NFmiStringTime(CurrentTime()).WeekdayName(itsLanguage); //030497
   NFmiTextTime kissa(CurrentTime());//100697	// ei onnaa yhdell‰ rivill‰
   return kissa.Weekday(itsLanguage).GetChars(1,2);
}
//220597______________________________________________________________________________
NFmiString NFmiTimeAxis :: DayTimeName (void)const
{ 
    NFmiTextTime textTime(CurrentTime());//100697	// ei onnaa yhdell‰ rivill‰
//	NFmiTextTime textTime = CurrentTime();
//	NFmiTextTime textTime(time.GetYear(),time.GetMonth(), time.GetDay(), time.GetHour());;
	return textTime.DayTimeName(itsLanguage).GetChars(1,5);	 //230597
}
//______________________________________________________________________________
NFmiString NFmiTimeAxis :: DayText (void)const
{
	NFmiValueString string;
    string.SetValue(CurrentTime().GetDay(), "%hd");  //090697
  	return string;
}
//______________________________________________________________________________
NFmiString NFmiTimeAxis :: SecondaryText (void)const
{
	NFmiValueString str("");
	NFmiTextTime time(CurrentTime());
	if(itsScale->StepValue() < 50.)
	{
		if (time.GetMin() == 30)
			str = NFmiValueString(time.GetHour(), "%hd");
	}
	else if(itsScale->StepValue() < 1400.)
	{
		if (time.GetHour() == 12)
		{
			str.SetValue(time.GetDay(), "%hd");
			str += ".";
			str += NFmiValueString(time.GetMonth(), "%hd");
		}
	}
//	else if(itsScale->StepValue() < 40000.)  //251096/LW lis‰tty
	else if(itsScale->IsMidst())               //120597/LW 
	{
//		if (time.GetDay()%6 == 2) // 26., 2., 8. pv jne
		{
//			str.SetValue(time.GetMonth(), "%hd");
			str=time.MonthName(); //120597/LW
			str += " ";//.";
			str += NFmiValueString(time.GetYear(), "%hd");
		}
	}
	else // eka kk, t‰m‰ vuosi 251096/LW, EI TESTATTU 
	{
	/* 300597 tuli p‰‰llekk‰in edellisen kanssa
 		if (time.GetMonth()== 6) 
		{
			str.SetValue(time.GetYear(), "%hd");
		}
	*/
	}

	return str;
}

NFmiTime NFmiTimeAxis::CurrentTime(void) const 
{
	return (static_cast<NFmiStepTimeScale *>(itsScale))->CurrentTime();
}

NFmiTime NFmiTimeAxis::StartTime(void) const 
{
	return (static_cast<NFmiStepTimeScale *>(itsScale))->StartTime();
}

NFmiTime NFmiTimeAxis::EndTime(void) const 
{
	return (static_cast<NFmiStepTimeScale *>(itsScale))->EndTime();
}

