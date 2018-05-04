#pragma once

#include "NFmiColor.h"
#include "NFmiDrawingEnvironment.h"
#include "NFmiDataStoringHelpers.h"

class NFmiTempLineInfo
{
public:
    NFmiTempLineInfo(void)
		:itsColor(0)
		,itsThickness(1)
		,itsLineType(FMI_SOLID)
		,fDrawLine(true)
	{}

    NFmiTempLineInfo(const NFmiColor &theColor, int theThickness, FmiPattern theLineType, bool drawLine)
		:itsColor(theColor)
		,itsThickness(theThickness)
		,itsLineType(theLineType)
		,fDrawLine(drawLine)
	{}

	const NFmiColor& Color(void) const {return itsColor;}
	void Color(const NFmiColor &newValue) {itsColor = newValue;}
	int Thickness(void) const {return itsThickness;}
	void Thickness(int newValue) {itsThickness = newValue;}
	FmiPattern LineType(void) const {return itsLineType;}
	void LineType(FmiPattern newValue) {itsLineType = newValue;}
	bool DrawLine(void) const {return fDrawLine;}
	void DrawLine(bool newValue) {fDrawLine = newValue;}
    NFmiDataStoringHelpers::NFmiExtraDataStorage FillExtraData() const;

    void Write(std::ostream& os) const;
    void WriteBasicData(std::ostream& os) const;
	void Read(std::istream& is);
private:
	NFmiColor itsColor;
	int itsThickness; // viivan paksuus pikseleinä
	FmiPattern itsLineType; // ks. FmiPattern-enum (FMI_SOLID = 0, FMI_DASH = 1, FMI_DOT = 2, FMI_DASHDOT = 3, FMI_DASHDOTDOT = 4)
	bool fDrawLine; // piirretäänkö kyseinen viiva vai ei
};

inline std::ostream& operator<<(std::ostream& os, const NFmiTempLineInfo& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiTempLineInfo& item){item.Read(is); return is;}



