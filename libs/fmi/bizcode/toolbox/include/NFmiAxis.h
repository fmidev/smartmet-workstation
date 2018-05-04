/*-------------------------------------------------------------------------------------*/
//© Ilmatieteen laitos/Lasse

//Originaali 20.12.1994

//Muutettu  12.1.1995/LW   peritty skaalasta
//Muutettu  25.10.1995/LW  Iteraattori
//Muutettu  26.10.1995/LW
//Muutettu   2.11.1995/LW  Ei enää skaalan lapsi
//Muutettu   7.11.1995/LW  Vain skaala, ei iteraattoria
//Muutettu  10.11.1995/LW  SecondaryTextLength() ym
//Muutettu  13.11.1995/LW  Scale (NFmiStepScale*)
//Muutettu  14.12.1995/LW  HasTicFormat()
//Muutettu   4.1.1996/LW StepNumber(void)
// 121196/LW +Scale(NFmiStepScale)
//_________________________________________________________ NFmiAxis

#pragma once

#include "NFmiStepScale.h"
#include "NFmiScale.h"
#include "NFmiTitle.h"

//_________________________________________________________ NFmiAxis

class NFmiAxis

{
 public:
	NFmiAxis (void);
	NFmiAxis (const NFmiStepScale& theScale, const NFmiTitle& theTitle);
	NFmiAxis (float theStartValue, float theEndValue, const NFmiTitle& theTitle);
	NFmiAxis (const NFmiAxis& anOtherAxis);
	virtual ~NFmiAxis(void);
	NFmiAxis& operator= (const NFmiAxis& anOtherAxis);

	void           Title(const NFmiTitle& theTitle) {itsTitle=theTitle;};
	const NFmiTitle& Title (void)const         {return itsTitle;};
	NFmiStepScale* Scale (void)const         {return itsScale;};
    void           Scale (NFmiStepScale* theScale) {delete itsScale;
	                                                itsScale = new NFmiStepScale(*theScale);};
    void           Scale (const NFmiStepScale& theScale) {*itsScale = theScale;};	
	void           ConstructTicFormat(unsigned short theNumberOfDecimals);
	const NFmiString& TicFormat(void) const     {return itsTicFormat;};

	bool Next(void)             {return itsScale->Next();};
	virtual void StepNumber(const FmiCounter& theNumber){itsScale->StepNumber(theNumber);};
	virtual FmiCounter StepNumber(void){return itsScale->StepNumber();};
    float      StepValue (void) const  {return itsScale->StepValue();};
	float      StartValue (void) const {return itsScale->StartValue();};
	float      EndValue (void) const   {return itsScale->EndValue();};
	float      Value(void) const       {return itsScale->Value();};
	void       Value(float theValue)   {itsScale->Value(theValue);};
	float      Location(void)const     {return itsScale->Location();};
	float      Location(float aValue)  {return itsScale->Scale()->RelLocation(aValue);};
	void       Reset(void)             {itsScale->Reset();};
	virtual NFmiString Text (void);
	virtual NFmiString SecondaryText (void)const       {return NFmiString("");};
	virtual unsigned short MaxTextLength(void)         {return MaxUniversalTextLength(1);};
	virtual unsigned short MaxSecondaryTextLength(void){return MaxUniversalTextLength(2);};

	virtual std::ostream& Write(std::ostream &file) const;
	virtual std::istream& Read(std::istream &file);

protected:
	virtual unsigned short MaxUniversalTextLength(short theCategory);
	bool HasTicFormat (void) const;

	NFmiTitle itsTitle;
	float itsTicLengthRatio;
	short itsNumberOfSubIntervals;
	NFmiString itsTicFormat;
	NFmiStepScale* itsScale;
private:

};

//@{ \name Globaalit NFmiGramObject-luokan uudelleenohjaus-operaatiot
inline std::ostream& operator<<(std::ostream& os, const NFmiAxis& item){return item.Write(os);}
inline std::istream& operator>>(std::istream& is, NFmiAxis& item){return item.Read(is);}
//@}

