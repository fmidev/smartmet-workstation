/*-------------------------------------------------------------------------------------*/
//© Ilmatieteenlaitos/Lasse               ngrbaser.cp

//Muodostettu 19.9.96/ NFmiBaseGramSeriest‰
// 301096/LW
// 011196/LW XAkseli tehd‰‰n Complete():n sis‰ll‰ 
/*-------------------------------------------------------------------------------------*/

#include "NFmiGramObject.h"
#include "NFmiPoint.h"

#include <algorithm>
#include <iostream>


//-------------------------------------------------------------------KFmiTxtParam----
//-----------------------------------------------------------------------------------
NFmiGramObject::NFmiGramObject(NFmiAxis *theYScale)
					  :itsYAxis(theYScale ==0 ? 0 : new NFmiAxis(*theYScale))
					  , itsGiveYAxisAlways(true)
{
	itsXAxis = 0;
}
//-----------------------------------------------------------------------------------
NFmiGramObject:: ~NFmiGramObject(void)
{
	if(itsYAxis)
	   delete itsYAxis;
	if(itsXAxis)
	   delete itsXAxis;
}
//______________________________________________________________________________
std::ostream& NFmiGramObject:: Write(std::ostream &file) const
{
	file << *itsYAxis << " ";
	file << *itsXAxis << " ";
	return file;
}
//______________________________________________________________________________
std::istream& NFmiGramObject:: Read(std::istream &file)
{
	file >> *itsYAxis;
	file >> *itsXAxis;
	return file;
}
/*
//-----------------------------------------------------------------------------------
float NFmiGramObject::Value(FmiCounter theSubIndex)const
{   
  return itsDataIterator->CurrentValue();
}
//-----------------------------------------------------------------------------------
float NFmiGramObject::Value(void)const
{
  return itsDataIterator->CurrentValue();
}
*/
//-----------------------------------------------------------------------------------
float NFmiGramObject::YScaleDeviation(void)
{
  return YScaleMaxValue() - YScaleMinValue();
}

//-----------------------------------------------------------------------------------
float NFmiGramObject::YScaleMaxValue(void)
{ 	
 float theMax;
 SaveIterator();
//** TMetTime saveTime(itsDataIterator->CurrentTime());
// NFmiMetBoxIterator tempIterator = NFmiMetBoxIterator(*itsDataIterator);
 Reset();
 theMax = -kFloatMissing;
 while(Next())
 {
		float theValue = YScaleValue();
		if(theValue != kFloatMissing && theMax < theValue)
		  theMax = theValue;
 }
 // delete itsDataIterator;
 // itsDataIterator = new NFmiMetBoxIterator(tempIterator);
 
//** itsDataIterator->CurrentTime(saveTime);
 RestoreIterator();
 return theMax > -kFloatMissing ? theMax : kFloatMissing;
}
//-----------------------------------------------------------------------------------
float NFmiGramObject::YScaleMinValue(void)
{	 
 float theMin =kFloatMissing;
 SaveIterator();
// TMetTime saveTime(itsDataIterator->CurrentTime());
 Reset();
 while(Next())
 {
		float theValue = YScaleValue();
		if(theValue != kFloatMissing && theMin > theValue)
			theMin = theValue;
 }
//  itsDataIterator->CurrentTime(saveTime);
 RestoreIterator();
 return theMin;
}
//-----------------------------------------------------------------------------------
float NFmiGramObject::YScaleSum(void)
{
  float theSum = 0.f;
  SaveIterator();
//  TMetTime saveTime(itsDataIterator->CurrentTime());
  Reset();
  while(Next())
  {
	 float theValue = YScaleValue();
	 if(theValue != kFloatMissing)
			 theSum += YScaleValue();
  }
   RestoreIterator();
//   itsDataIterator->CurrentTime(saveTime);
   return theSum;
}
//-----------------------------------------------------------------------------------
float NFmiGramObject::YScaleMean(void)
{
  float theSum = 0.f;
  short number = 0;
  SaveIterator();
//  TMetTime saveTime (itsDataIterator->CurrentTime());
  Reset();
  while(Next())
  {
	 float theValue = YScaleValue();
	 if(theValue != kFloatMissing)
	 {
			 theSum += theValue;
			 number++;
	 }
  }
  RestoreIterator();
//  itsDataIterator->CurrentTime(saveTime);
  
  return number>0 ? theSum/number : kFloatMissing;
}
//-----------------------------------------------------------------------------------
void NFmiGramObject::Complete(void)
{
   //oletetaan ett‰ X aina olemassa
	if (!itsYAxis)
	{
		MakeYScale(); //311096/LW oli ..FromData()
	}
	itsYAxis->Scale()->Scale()->LimitCheck(true);
	itsYAxis->StepNumber(5);

	ConstructXAxis(); //011196/LW
}
//-----------------------------------------------------------------------------------
void NFmiGramObject::MakeYScale(void)
{ //T‰m‰ virt.menetelm‰ jotta voidaan haarautua lapsessa.
  MakeYScaleFromData();
}
//-----------------------------------------------------------------------------------
void NFmiGramObject::CompleteYScale(void)
{
	if (itsYAxis->StartValue() == kFloatMissing)
											itsYAxis->Scale()->Scale()->SetStartValue(YScaleMinValue());
	if (itsYAxis->EndValue() == kFloatMissing)
										 itsYAxis->Scale()->Scale()->SetEndValue(YScaleMaxValue());
	itsYAxis->Scale()->Scale()->ExpandIfEqual(1.f);
}
//-----------------------------------------------------------------------------------
void NFmiGramObject::MakeYScaleFromData(void)
{
	if(itsYAxis)  //190397
	   delete itsYAxis;
	NFmiScale scale(YScaleMinValue(), YScaleMaxValue());
	scale.ExpandIfEqual(1.f);
	scale.StartFromZeroOptionally(2.f);	   
	scale.ExpandIfNotZero(.05f);

	NFmiStepScale stepScale(scale);
	stepScale.StepNumber(5);
	itsYAxis = new NFmiAxis(stepScale, NFmiTitle("MakeYScaleFromData"));

}
//-----------------------------------------------------------------------------------
bool NFmiGramObject::YScaleDataOk(void) const
{
	return YScaleValue() == kFloatMissing ? false : true;
}
//-----------------------------------------------------------------------------------
void NFmiGramObject::XAxisStepNumber(const FmiCounter& theStepNumber)
{
   itsXAxis->StepNumber(std::min(DataNumber(), theStepNumber));
}
//-----------------------------------------------------------------------------------
float NFmiGramObject::Y(void)const
{
	 return itsYAxis->Scale()->Scale()->RelLocation(YScaleValue());
}
//-----------------------------------------------------------------------------------
NFmiPoint NFmiGramObject::Point(void)const
{
    return NFmiPoint(X(), Y());  
}
//-----------------------------------------------------------------------------------
float NFmiGramObject::YZero(void)const
{
	 NFmiScale scale(*itsYAxis->Scale()->Scale());
	 scale.LimitCheck(false);

     float loc = scale.RelLocation(0.f);
	 loc = std::min(1.f, loc);
	 loc = std::max(0.f, loc);
	 return loc;
}
