/*-------------------------------------------------------------------------------------*/
//© Ilmatieteenlaitos/Lasse

//Originaali 21.10.1997
//Muutettu 021297/LW +GetStartStartPoint()
//Muutettu 101297/LW +SetXStartScale() ja muut +MoveXEndScale() 
//Muutettu 090398/LW +GetXStartScale() ja muut
//Muutettu 121098/LW +UnScale() ja muut
///*-------------------------------------------------------------------------------------*/

#include "NFmiRectScale.h"
#include <fstream>

//______________________________________________________________________________
//inline
NFmiRectScale :: NFmiRectScale (void)
{
	SetScales(NFmiRect(0.,0.,1.,1.), NFmiRect(0.,0.,1.,1.));
}
//______________________________________________________________________________
//inline
NFmiRectScale :: NFmiRectScale (const NFmiRect &theStartRect, const NFmiRect &theEndRect)
  : itsXStartScale (static_cast<float>(theStartRect.Left()),static_cast<float>(theStartRect.Right()))
  , itsYStartScale (static_cast<float>(theStartRect.Top()),static_cast<float>(theStartRect.Bottom()))
  , itsXEndScale (static_cast<float>(theEndRect.Left()),static_cast<float>(theEndRect.Right()))
  , itsYEndScale (static_cast<float>(theEndRect.Top()),static_cast<float>(theEndRect.Bottom()))
{
}

//______________________________________________________________________________
//inline
NFmiRectScale :: ~NFmiRectScale(void)
{
}
//______________________________________________________________________________
void NFmiRectScale :: SetScales(const NFmiRect& theStartRect, const NFmiRect& theEndRect)
{
	SetStartScales(theStartRect);
	SetEndScales(theEndRect);
}
//______________________________________________________________________________
void NFmiRectScale :: SetXStartScale(const NFmiScale& theScale)
{
	itsXStartScale = theScale;
}
//______________________________________________________________________________
void NFmiRectScale :: SetYStartScale(const NFmiScale& theScale)
{
	itsYStartScale = theScale;
}
//______________________________________________________________________________
void NFmiRectScale :: SetXEndScale(const NFmiScale& theScale)
{
	itsXEndScale = theScale;
}
//______________________________________________________________________________
void NFmiRectScale :: SetYEndScale(const NFmiScale& theScale)
{
	itsYEndScale = theScale;
}
//______________________________________________________________________________
void NFmiRectScale :: SetStartScales(const NFmiRect& theStartRect)
{
	itsXStartScale.Set(static_cast<float>(theStartRect.Left()),
					   static_cast<float>(theStartRect.Right()));
	itsYStartScale.Set(static_cast<float>(theStartRect.Top()),
					   static_cast<float>(theStartRect.Bottom()));
}
//______________________________________________________________________________
void NFmiRectScale :: SetEndScales(const NFmiRect& theStartRect)
{
	itsXEndScale.Set(static_cast<float>(theStartRect.Left()),
					 static_cast<float>(theStartRect.Right()));
	itsYEndScale.Set(static_cast<float>(theStartRect.Top()),
					 static_cast<float>(theStartRect.Bottom()));
}
//______________________________________________________________________________
void NFmiRectScale :: MoveXEndScale(double theDelta)
{
	SetXEndScale(NFmiScale(static_cast<float>(itsXEndScale.StartValue()+ theDelta)
		                  ,static_cast<float>(itsXEndScale.EndValue()+ theDelta)));
}
/*
//______________________________________________________________________________
//inline
NFmiScale& NFmiScale :: operator= (const NFmiScale& anOtherScale)
{
	itsStartValue = anOtherScale.itsStartValue;
	itsEndValue = anOtherScale.itsEndValue;
	itsDataOk = anOtherScale.itsDataOk;
	itsLimitCheck = anOtherScale.itsLimitCheck;
	itsEpsilon = anOtherScale.itsEpsilon;

	return *this;
}
*/
//______________________________________________________________________________
//______________________________________________________________________________
std::ostream& NFmiRectScale :: Write(std::ostream &file)
{
	file << itsXStartScale << " ";
	file << itsYStartScale << " ";
	file << itsXEndScale << " ";
	file << itsYEndScale << " ";
	return file;
}
//______________________________________________________________________________
std::istream& NFmiRectScale :: Read(std::istream &file)
{
	file >> itsXStartScale;
	file >> itsYStartScale;
	file >> itsXEndScale;
	file >> itsYEndScale;
//	Check();
	return file;
}
/*
//______________________________________________________________________________
//
bool NFmiScale :: IsMissing(void) const
{
  return itsStartValue == kFloatMissing ||
		 itsEndValue   == kFloatMissing; 
}
//______________________________________________________________________________
//inline
void NFmiScale :: Check (void)
{
  itsDataOk = itsStartValue != kFloatMissing &&
				  itsEndValue   != kFloatMissing; 
//			   && itsStartValue != itsEndValue;	  //120397 ******
  if (itsDataOk)            //041196/LW
      itsEpsilon = (itsEndValue - itsStartValue) * .001f;
  else 
	  itsEpsilon = .000001f; //041196/LW
}
*/
/*
//______________________________________________________________________________
//inline
bool NFmiScale :: Inside (float theValue) const
{	// epsilon näyttää tuovan yhden merkitsevän numeron lisää
	return itsDataOk && theValue+itsEpsilon >= itsStartValue && theValue-itsEpsilon <= itsEndValue
			  ? true : false;
}
 */
/*
//______________________________________________________________________________
//inline
float NFmiScale :: RelLocation (float theValue) const
{
  float value = kFloatMissing;
  if (itsDataOk && theValue != kFloatMissing)
	  if(Difference() > 0.f)   // 120397
		  value = (theValue-itsStartValue)/Difference();
      else
		  value = 0.f;

  return  value == kFloatMissing || !Inside(theValue) && itsLimitCheck
			? kFloatMissing : value;
}
//______________________________________________________________________________
//inline
float NFmiScale :: Location (float theRelValue) const
{
  return  itsDataOk ? Difference()*theRelValue + itsStartValue : kFloatMissing;
}
*/
//9.3.98______________________________________________________________________________
NFmiRect NFmiRectScale :: GetStartScales (void) const
{
	return NFmiRect (NFmiPoint(itsXStartScale.StartValue(),itsYStartScale.StartValue())
		            ,NFmiPoint(itsXStartScale.EndValue(),itsYStartScale.EndValue()));	
}
//9.3.98______________________________________________________________________________
NFmiRect NFmiRectScale :: GetEndScales (void) const
{
	return NFmiRect (NFmiPoint(itsXEndScale.StartValue(),itsYEndScale.StartValue())
		            ,NFmiPoint(itsXEndScale.EndValue(),itsYEndScale.EndValue()));	
}
//9.3.98______________________________________________________________________________
NFmiScale NFmiRectScale :: GetXEndScale (void) const
{
	return itsXEndScale;	
}
//9.3.98______________________________________________________________________________
NFmiScale NFmiRectScale :: GetYEndScale (void) const
{
	return itsYEndScale;	
}
//9.3.98______________________________________________________________________________
NFmiScale NFmiRectScale :: GetXStartScale (void) const
{
	return itsXStartScale;	
}
//9.3.98______________________________________________________________________________
NFmiScale NFmiRectScale :: GetYStartScale (void) const
{
	return itsYStartScale;	
}
//2.12.97______________________________________________________________________________
NFmiPoint NFmiRectScale :: GetStartStartPoint (void) const
{
	return NFmiPoint(itsXStartScale.StartValue(),itsYStartScale.StartValue());	
}
//______________________________________________________________________________
NFmiPoint NFmiRectScale :: GetEndStartPoint (void) const
{
	return NFmiPoint(itsXEndScale.StartValue(),itsYEndScale.StartValue());	
}
//______________________________________________________________________________
NFmiPoint NFmiRectScale :: GetStartCenter (void) const
{
	return NFmiPoint(itsXStartScale.CenterValue(),itsYStartScale.CenterValue());	
}
//______________________________________________________________________________
NFmiPoint NFmiRectScale :: GetEndCenter (void) const
{
	return NFmiPoint(itsXEndScale.CenterValue(),itsYEndScale.CenterValue());	
}
//______________________________________________________________________________
double NFmiRectScale :: GetXScaling (void) const
{
	return itsXEndScale.Difference() / itsXStartScale.Difference();	
}
//______________________________________________________________________________
double NFmiRectScale :: GetYScaling (void) const
{
	return itsYEndScale.Difference() / itsYStartScale.Difference();	
}
//______________________________________________________________________________
NFmiPoint NFmiRectScale :: GetScaling (void) const
{
	 return NFmiPoint(GetXScaling(),GetYScaling());
}
//______________________________________________________________________________
NFmiPoint NFmiRectScale :: Scale (const NFmiPoint& thePoint) const
{
	 return  NFmiPoint(ScaleX(thePoint.X()),ScaleY(thePoint.Y()));
}
//______________________________________________________________________________
NFmiRect NFmiRectScale :: ScaleRect (const NFmiRect& theRect) const
{
	 return  NFmiRect(Scale(theRect.TopLeft()), Scale(theRect.BottomRight()));
}
//______________________________________________________________________________
double NFmiRectScale :: ScaleX (double value) const
{
	 return  itsXEndScale.Location(itsXStartScale.RelLocation(static_cast<float>(value)));
}
//______________________________________________________________________________
double NFmiRectScale :: ScaleY (double value) const
{
	 return  itsYEndScale.Location(itsYStartScale.RelLocation(static_cast<float>(value)));
}
//______________________________________________________________________________
double NFmiRectScale :: UnScaleX (double value) const
{
	 return  itsXStartScale.Location(itsXEndScale.RelLocation(static_cast<float>(value)));
}
//______________________________________________________________________________
double NFmiRectScale :: UnScaleY (double value) const
{
	 return  itsYStartScale.Location(itsYEndScale.RelLocation(static_cast<float>(value)));
}
//______________________________________________________________________________
NFmiPoint NFmiRectScale :: UnScale (const NFmiPoint& thePoint) const
{
	 return  NFmiPoint(UnScaleX(thePoint.X()),UnScaleY(thePoint.Y()));
}
//______________________________________________________________________________
NFmiRect NFmiRectScale :: UnScaleRect (const NFmiRect& theRect) const
{
	 return  NFmiRect(UnScale(theRect.TopLeft()), UnScale(theRect.BottomRight()));
}







