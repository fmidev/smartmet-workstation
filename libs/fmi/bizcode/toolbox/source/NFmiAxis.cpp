/*-------------------------------------------------------------------------------------*/
//© Ilmatieteenlaitos/Lasse

//Originaali 21. 12.1994

//Muutettu   2.11.1995/LW  Ei en‰‰ skaalan lapsi
//Muutettu   7.11.1995/LW  Vain skaala, ei iteraattoria
//Muutettu  10.11.1995/LW  SecondaryTextLength()
//Muutettu  14.12.1995/LW  HasTicFormat()
///*-------------------------------------------------------------------------------------*/

#include "NFmiAxis.h"
#include "NFmiValueString.h"
#include <algorithm>

//______________________________________________________________________________
//inline
NFmiAxis :: NFmiAxis (void)
			 : itsTitle(NFmiString("NFmiAxis"))
			 , itsTicLengthRatio(.2f)
			 , itsNumberOfSubIntervals(2)
			 , itsTicFormat(NFmiString())
			 , itsScale(0)
{
}
//______________________________________________________________________________
//inline
NFmiAxis :: NFmiAxis (const NFmiStepScale& theScale, const NFmiTitle& theTitle)
				:itsTitle (theTitle)
				,itsTicLengthRatio(.2f)
				,itsNumberOfSubIntervals(2)
				,itsTicFormat(NFmiString())
			 ,  itsScale(new NFmiStepScale (theScale))
{
   Reset();
}
//______________________________________________________________________________
//inline
NFmiAxis :: NFmiAxis (float theStartValue
                     ,float theEndValue 
                     ,const NFmiTitle& theTitle)
			 :itsTitle (theTitle)
			 ,itsTicLengthRatio(.2f)
			 ,itsNumberOfSubIntervals(2)
			 ,itsTicFormat(NFmiString())
			 ,itsScale(new NFmiStepScale (theStartValue, theEndValue))
{
   Reset();
}
//______________________________________________________________________________
//inline
NFmiAxis :: NFmiAxis (const NFmiAxis& anOtherAxis)
  : itsTitle (anOtherAxis.itsTitle)
  , itsTicLengthRatio(anOtherAxis.itsTicLengthRatio)
  , itsNumberOfSubIntervals(anOtherAxis.itsNumberOfSubIntervals)
  , itsTicFormat(anOtherAxis.itsTicFormat)
  , itsScale(new NFmiStepScale(*anOtherAxis.itsScale))
{
}
//______________________________________________________________________________
//inline
NFmiAxis :: ~NFmiAxis(void)
{
    delete itsScale;
}
//______________________________________________________________________________
NFmiAxis& NFmiAxis :: operator= (const NFmiAxis& anOtherAxis)
{
    if(itsScale)
	   delete itsScale;
	itsScale = new NFmiStepScale(*anOtherAxis.itsScale);

	itsTitle = anOtherAxis.itsTitle;
	itsTicLengthRatio = anOtherAxis.itsTicLengthRatio;
	itsNumberOfSubIntervals = anOtherAxis.itsNumberOfSubIntervals;
	itsTicFormat = anOtherAxis.itsTicFormat;
	return *this;
}
//______________________________________________________________________________
void NFmiAxis :: ConstructTicFormat(unsigned short theDecimals)
{
	itsTicFormat = NFmiString("%.");
	itsTicFormat += NFmiString(NFmiValueString(static_cast<int>(theDecimals), "%hd"));
	itsTicFormat += "f";
}
//______________________________________________________________________________
unsigned short NFmiAxis :: MaxUniversalTextLength(short theCategory)
{
	// itsCounter:kin pit‰isi s‰ilytt‰‰
	float tempValue = Value();
	Reset();
	unsigned short num = 0;
	unsigned short apu;
	while(Next())
	{
		if(theCategory == 1) apu = static_cast<unsigned short>(Text().GetLen()); // ei mene suoraan makroon
		if(theCategory == 2) apu = static_cast<unsigned short>(SecondaryText().GetLen());
		num = std::max(num, apu);
	}
	Value(tempValue);
	return num;
}
//______________________________________________________________________________
NFmiString NFmiAxis :: Text (void)
{
    if(!HasTicFormat())
	{
	   NFmiValueString stepString(itsScale->StepValue());
	   ConstructTicFormat(stepString.SignificantDecimals());
	}
	NFmiValueString string(Value(), TicFormat().CharPtr());
	return string;
}
//______________________________________________________________________________
bool NFmiAxis :: HasTicFormat (void) const
{
    return 	itsTicFormat.GetLen() ? true : false;
}

std::ostream& NFmiAxis::Write(std::ostream &file) const
{
	return file;
}

std::istream& NFmiAxis::Read(std::istream &file)
{
	return file;
}
