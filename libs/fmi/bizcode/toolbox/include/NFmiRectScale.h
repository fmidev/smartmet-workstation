/*-------------------------------------------------------------------------------------*/

//© Ilmatieteenlaitos/Lasse

//Originaali 21.10. 1997
//Muutettu 021297/LW +GetStartStartPoint() 
//Muutettu 101297/LW SetXStartScale() ja muut
//Muutettu 090398/LW +GetXStartScale() ja muut
//Muutettu 121098/LW +UnScale().....
//*-------------------------------------------------------------------------------------*/

#pragma once

#include "NFmiScale.h"
#include "NFmiRect.h"

//_________________________________________________________ 


class NFmiRectScale 
{
  public:

	NFmiRectScale (void);
	NFmiRectScale (const NFmiRect &theStartRect, const NFmiRect &theEndRect);
//	NFmiRectScale (const NFmiScale& anOtherScale);

	virtual ~NFmiRectScale(void);
	virtual std::ostream& Write(std::ostream &file);
	virtual std::istream& Read(std::istream &file);
//	NFmiScale& operator=  (const NFmiScale& anOtherScale);
//	NFmiScale& operator+= (const NFmiScale& anOtherScale);
// 	NFmiScale& operator-= (const NFmiScale& anOtherScale);

	void SetStartScales(const NFmiRect& startRect);
	void SetXStartScale(const NFmiScale& scale); //101297
	void SetYStartScale(const NFmiScale& scale);//101297
	void SetXEndScale(const NFmiScale& scale);//101297
	void SetYEndScale(const NFmiScale& scale);//101297
	void MoveXEndScale (double theDelta);      //101297
	void SetEndScales(const NFmiRect& endRect);
	void SetScales(const NFmiRect& startRect, const NFmiRect& endRect);
	double GetXScaling(void)const;
	double GetYScaling(void)const;
    NFmiRect GetStartScales (void) const;
    NFmiRect GetEndScales (void) const;
    NFmiScale GetXStartScale (void) const;
    NFmiScale GetYStartScale (void) const;
    NFmiScale GetXEndScale (void) const;
    NFmiScale GetYEndScale (void) const;
	NFmiPoint GetEndStartPoint(void) const;
	NFmiPoint GetStartStartPoint(void) const;
	NFmiPoint GetStartCenter(void) const;
	NFmiPoint GetEndCenter(void) const;
	NFmiPoint GetScaling(void) const;
	NFmiPoint Scale(const NFmiPoint& fromPoint) const;
   NFmiRect ScaleRect (const NFmiRect& fromRect) const;
   double ScaleX (double fromValue) const;
   double ScaleY (double fromValue) const;
   NFmiPoint UnScale(const NFmiPoint& fromPoint) const;
   NFmiRect UnScaleRect (const NFmiRect& fromRect) const;
   double UnScaleX (double fromValue) const;
   double UnScaleY (double fromValue) const;

protected:

private:
	NFmiScale itsXStartScale; 
	NFmiScale itsYStartScale; 
	NFmiScale itsXEndScale; 
	NFmiScale itsYEndScale; 
};

