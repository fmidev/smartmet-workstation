/*-------------------------------------------------------------------------------------*/
//© Ilmatieteenlaitos/Lasse

//Originaali . 12.1994

//Muutettu   03.11.1995/LW
//Muutettu    7.12.1995/LW  Operator +=
//Muutettu   21.12.1995/LW  Konstruktoreihin stepValue=kFloatMissing
//Muutettu   18.1.1996/LW  itsMinStepValue
// 061196/LW FirsValue(), LastValue()
// 120697/LW +IsMidst()
// 011298/LW SetScale(),SetStepsOnBorders(),fStepsOnBorders, Update() 
//           oletus ett‰ stepit osuvat alku- ja loppupisteeseen
// 080699/Marko		fStepsOnBorders asetukset konstruktoreihin, oletus arvo true

//*-------------------------------------------------------------------------------------*/

#pragma once

#include "NFmiGlobals.h"
#include <iosfwd>

class NFmiScale;

//_________________________________________________________ NFmiStepScale

class NFmiStepScale
{
//friend class NFmiScaleIterator;
public:
	NFmiStepScale (void){itsScale = 0;
								itsStepValue = kFloatMissing;
								itsStartGap = 0.f;
								itsDirection = kForward;
								itsMinStepValue = 0.f;
								fStepsOnBorders = true;
								itsCounter=0;
								itsCounterLimit=5000;
						};
	NFmiStepScale (const NFmiScale& theScale, float theStep=kFloatMissing, float theStartGap=0.
															,FmiDirection theDirection = kForward, bool areStepsOnBorders = true);
	NFmiStepScale (const float& theStartValue, const float& theEndValue, float theStep=kFloatMissing, float theStartGap=0.
															,FmiDirection theDirection = kForward, bool areStepsOnBorders = true, int theMaxStepNumber = 5);
	NFmiStepScale (float theMinStepValue, const NFmiScale& theScale, float theStep=kFloatMissing, float theStartGap=0.
															,FmiDirection theDirection = kForward, bool areStepsOnBorders = true);
// stepNumber ei onnistu konstruktorissa koska k‰ytet‰‰n virtuaalifunktioita
//	NFmiStepScale (float theStartValue, float theEndValue, FmiCounter theMaxStepNumber=5);
	NFmiStepScale (const NFmiStepScale& anOtherScale);
	virtual ~NFmiStepScale(void);
	NFmiStepScale& operator= (const NFmiStepScale& anOtherStepScale);
	NFmiStepScale& operator+= (const NFmiScale& aScale);
	NFmiStepScale& operator+= (const NFmiStepScale& anOtherScale);
	virtual std::ostream& Write(std::ostream &file) const;
	virtual std::istream& Read(std::istream &file);

	NFmiScale* Scale(void) const {return itsScale;};
	void SetScale (float theStartValue, float theEndValue,bool setToBorders=true); //011298
	void StartGap (const float theGap){itsStartGap = theGap;};
	void StepValue (const float theStep){itsStepValue = theStep;};
	virtual void StepNumber(const FmiCounter& theNumber);
    virtual FmiCounter StepNumber(void) const;
	virtual void  Update(bool stepsOnBorders=true);    //011298 

	float StepValue(void)const;
	float StartGap(void){return itsStartGap;};
//	void Rounding(bool theRounding) {itsRounding = theRounding;};
	FmiCounter operator% (FmiCounter theDivisionFactor);

	float      StartValue (void) const;
	float      EndValue (void) const;
	float      FirstValue (void) const; //eka steppiarvo //061196
	float      LastValue (void) const; //vika steppiarvo //061196

	bool Next      (float& theValue);
	bool Next      (void);
	bool Value     (float& theValue); //=Set
	float      Value     (void) const;
	float      Location  (void) const;
	bool IsLast    (void);
	bool IsFirst   (void);
	bool IsInside  (void) const;           //current
	bool IsInside  (float theValue) const; //given
	bool IsMidst   (void) const;  //120597
	void       Reset     (void);
//	virtual NFmiString Text (void)const;
	FmiCounter CounterLimit(void) const {return itsCounterLimit;}
	void CounterLimit(FmiCounter newValue) {itsCounterLimit = newValue;}

protected:
	void       Step      (void);
	void       StepBack  (void);
	bool NextInside(void);
	bool LastInside(void);
	void  SetStepsOnBorders(void); //011298 

	NFmiScale     *itsScale;
	float          itsValue;
	FmiDirection   itsDirection;
	FmiCounter     itsCounter;
	FmiCounter     itsCounterLimit; // kuinka pitk‰lle sallitaan ett‰ Next-metodin looppausta jatketaan. Tein systeemin niin ett‰ on maksimi
									// lukum‰‰r‰ askelluksia, koska ohjelma j‰i jumiin, kun t‰m‰ luokka rupesi piirt‰m‰‰n miljardia tick-markkia.
									// Vaihtoehdot on ett‰ heitet‰‰n poikkeus tai lopetetaan virhetilanne hiljaisesti. Valitsin
									// hiljaisen vaihtoehdon. Defaultti raja-arvo on 5000. Jos joku piirt‰‰ mihin tahansa asteikon
									// jossa on niin paljon tai enemm‰n tick-markkeja, tulos on vain puuroa.


protected:
	bool  fStepsOnBorders;
	virtual float Round(float theValue) const;
	virtual void CalculateStartGap(void);

	float itsStepValue;
	float itsMinStepValue;
	float itsStartGap;

private:

};
//@{ \name Globaalit NFmiStepScale-luokan uudelleenohjaus-operaatiot
inline std::ostream& operator<<(std::ostream& os, const NFmiStepScale& item){return item.Write(os);}
inline std::istream& operator>>(std::istream& is, NFmiStepScale& item){return item.Read(is);}
//@}

