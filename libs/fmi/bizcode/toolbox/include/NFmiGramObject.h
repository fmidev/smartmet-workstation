
#pragma once

/*-------------------------------------------------------------------------------------*/
//© Ilmatieteenlaitos/Lasse               NFmiBaseGramSeries.h

// Luokan idea: Abstrakti luokka, jolla on Value(), joka tuodaan MetBoxista.

// Muodostettu 30.8.96 NFmiGramSeries:tä uudeksi perusluokaksi
// 4.9.96/LW  Point(),XAxisStepNumber() ja DataNumber()-koodit tuotu tänne (ei virt.)
// 19.8.96/LW SaveIterator() ja RestoreIterator() tehty
// 27.9.96/LW +Init()
// 311096/LW +MakeYScale()
// 060297/LW XAxisStepNumber(void) virtuaaliseksi 
//*-------------------------------------------------------------------------------------*/

#include "NFmiAxis.h"
#include "NFmiDataIdent.h"

class NFmiPoint; 

//----NFmiGramSerie---------------------------------------------------------------
//-----------------------------------------------------------------------------------
class NFmiGramObject
{
 public:
				NFmiGramObject(NFmiAxis *itsYAxis);
	virtual ~NFmiGramObject(void);

	virtual std::ostream& Write(std::ostream &file) const;
	virtual std::istream& Read(std::istream &file);

			  NFmiAxis*  XAxis(void)const {return itsXAxis;};
			  NFmiAxis*  YAxis(bool always = true)const {if (always || itsGiveYAxisAlways)
			                                                       return itsYAxis;
			                                                    return 0;};
   virtual    void       Init(void) {Complete();};                                                                                                               
			  NFmiTitle  Title(void)const{return itsXAxis->Title();}; 
			  void       Title(const NFmiTitle& theTitle){itsXAxis->Title(theTitle);};
              void       XAxisStepNumber(const FmiCounter& theNumber);
	virtual   FmiCounter XAxisStepNumber(void) const {return itsXAxis->StepNumber();};
	virtual   unsigned short DataNumber(void)=0;                                                                 
	virtual	  float      YScaleMean(void);
	virtual	  float      YScaleMaxValue(void);	
	virtual	  float      YScaleMinValue(void);	
	virtual	  float      YScaleSum(void);
			  float      YScaleDeviation(void);
	virtual	  void       YAxisStepNumber(FmiCounter theNumber){itsYAxis->StepNumber(theNumber);};
	virtual	FmiCounter   YAxisStepNumber(void)const {return itsYAxis->StepNumber();};
	virtual void         Reset(void)=0;
	virtual bool   Next(void)=0;
	virtual bool   Previous(void)=0;
	virtual NFmiString   ParamName(void)const=0;//{return itsDataIterator->GetParamDescriptor()->CurrentParamName();};
	virtual NFmiDataIdent Param(void)   const=0;//{return itsDataIterator->Param();};
    virtual void         SaveIterator(void)=0;
	virtual void         RestoreIterator(void)=0;
//	virtual	float        Value(void) const;
//	virtual	float        Value(FmiCounter theSubIndex)  const;
	        NFmiPoint    Point(void) const;
	virtual float        X(void) const=0;
	virtual	float        Y(void) const;
			float        YZero(void) const;
		    bool   YScaleDataOk(void) const;
			void         CompleteYScale (void); //   min/max/both values
	        bool   GiveYAxisAlways(void) const {return itsGiveYAxisAlways;};
	        void         GiveYAxisAlways(bool theGiveYAxisAlways) {itsGiveYAxisAlways = theGiveYAxisAlways;};

protected:
	virtual	float        YScaleValue(void) const=0;
	virtual void         ConstructXAxis(void)=0;   //+011196/LW
		    void         MakeYScaleFromData(void);
	virtual void         MakeYScale(void);
	        void         Complete(void);

	NFmiAxis *itsXAxis; 
	NFmiAxis *itsYAxis;
	NFmiString itsName;
	bool itsGiveYAxisAlways;

private:
};
//@{ \name Globaalit NFmiGramObject-luokan uudelleenohjaus-operaatiot
inline std::ostream& operator<<(std::ostream& os, const NFmiGramObject& item){return item.Write(os);}
inline std::istream& operator>>(std::istream& is, NFmiGramObject& item){return item.Read(is);}
//@}

