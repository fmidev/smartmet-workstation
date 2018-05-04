#pragma once

#include "NFmiColor.h"

class NFmiMapViewTimeLabelInfo
{
public:
	class TimeStringInfo
	{
	public:
		TimeStringInfo(void)
		:itsTimeFormat()
		,itsFontName("arial")
		,itsFontSizeInMM(2.5)
		,itsColor(0,0,0)
		,fBold(false)
		{
		}

		std::string itsTimeFormat; // NFmiTime-luokan ToStr-metodin format stringi
		std::string itsFontName;
		double itsFontSizeInMM;
		NFmiColor itsColor;
		bool fBold;
	};


	NFmiMapViewTimeLabelInfo();

	void InitFromSettings(const std::string &theInitNameSpace);

	bool UseOldTimeLabel() const {return fUseOldTimeLabel;}
	void UseOldTimeLabel(bool newValue) {fUseOldTimeLabel = newValue;}
	const TimeStringInfo& TimeStringInfo1() const {return itsTimeStringInfo1;}
	void TimeStringInfo1(const TimeStringInfo &newValue) {itsTimeStringInfo1 = newValue;}
	const TimeStringInfo& TimeStringInfo2() const {return itsTimeStringInfo2;}
	void TimeStringInfo2(const TimeStringInfo &newValue) {itsTimeStringInfo2 = newValue;}
	const NFmiColor& BoxFillColor() const {return itsBoxFillColor;}
	void BoxFillColor(const NFmiColor &newValue) {itsBoxFillColor = newValue;}
	const NFmiColor& BoxFrameColor() const {return itsBoxFrameColor;}
	void BoxFrameColor(const NFmiColor &newValue) {itsBoxFrameColor = newValue;}
	double BoxFrameThicknessInMM() const {return itsBoxFrameThicknessInMM;}
	void BoxFrameThicknessInMM(double newValue) {itsBoxFrameThicknessInMM = newValue;}
	double SizeFactor1() const {return itsSizeFactor1;}
	void SizeFactor1(double newValue) {itsSizeFactor1 = newValue;}
	double SizeFactor2() const {return itsSizeFactor2;}
	void SizeFactor2(double newValue) {itsSizeFactor2 = newValue;}
	double ViewSize1() const {return itsViewSize1;}
	void ViewSize1(double newValue) {itsViewSize1 = newValue;}
	double ViewSize2() const {return itsViewSize2;}
	void ViewSize2(double newValue) {itsViewSize2 = newValue;}

	double MinFactor() const {return itsMinFactor;}
	void MinFactor(double newValue) {itsMinFactor = newValue;}
	double MaxFactor() const {return itsMaxFactor;}
	void MaxFactor(double newValue) {itsMaxFactor = newValue;}
	double AbsoluteMinFontSizeInMM() const {return itsAbsoluteMinFontSizeInMM;}
	void AbsoluteMinFontSizeInMM(double newValue) {itsAbsoluteMinFontSizeInMM = newValue;}
	double AbsoluteMaxFontSizeInMM() const {return itsAbsoluteMaxFontSizeInMM;}
	void AbsoluteMaxFontSizeInMM(double newValue) {itsAbsoluteMaxFontSizeInMM = newValue;}

private:

bool fUseOldTimeLabel;

TimeStringInfo itsTimeStringInfo1;
TimeStringInfo itsTimeStringInfo2;

NFmiColor itsBoxFillColor;
NFmiColor itsBoxFrameColor;
double itsBoxFrameThicknessInMM;

/*
 * Following setting are for adjusting the time string
 * and time label box size according to actual map view size.
 * So that in small view time label is smaller and in big
 * view they are larger.
 * This is done by givin two linear points in size-factor and
 * view size space. E.g. you can say that with mapView that
 * is 140 mm height has size factor 0.8 and mapView with height
 * 600 mm has factor 1.4. With these two points are all different
 * mapView heights calculated linearly. So if you have font size
 * 3.0 mm and map view height gives factor 1.2 the final font size
 * will be 3.6.
 * You can also set the absolute minimum and maximum size for fonts.
*/

double itsSizeFactor1;
double itsSizeFactor2;
double itsViewSize1;
double itsViewSize2;
double itsMinFactor;
double itsMaxFactor;
double itsAbsoluteMinFontSizeInMM;
double itsAbsoluteMaxFontSizeInMM;

};
