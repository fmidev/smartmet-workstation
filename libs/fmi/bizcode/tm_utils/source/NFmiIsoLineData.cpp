
#include "NFmiIsoLineData.h"

void NFmiIsoLineData::Init(int xCount, int yCount, int theMaxAllowedIsoLineCount, int theParamFieldCount)
{
    // Flow-char piirrossa annetaan 2 tai useampia parametri kenttiä, joten itsVectorFloatGridData pitää alustaa tarpeeksi isoksi myös niitä tilanteita varten
    itsVectorFloatGridData.resize(xCount * yCount * theParamFieldCount);
	itsXNumber = xCount;
	itsYNumber = yCount;
	itsMaxAllowedIsoLineCount = theMaxAllowedIsoLineCount;
	itsIsolineData = 0;

	fUseIsoLines = 0;
	fUseColorContours = 0;
	itsTrueIsoLineCount = 0;
	itsTrueColorContoursCount = 0;
	fUseLabelBox = false;
	fUseIsoLineFeathering = false;

	itsIsoLineColor.resize(itsMaxAllowedIsoLineCount, 0);
	itsIsoLineLabelColor.resize(itsMaxAllowedIsoLineCount, 0);
	itsIsoLineStyle.resize(itsMaxAllowedIsoLineCount, 0);
	itsIsoLineWidth.resize(itsMaxAllowedIsoLineCount, 0.f);
	itsIsoLineAnnonationHeight.resize(itsMaxAllowedIsoLineCount, 0.f);
	fUseCustomIsoLineClasses = false;
	fUseCustomColorContoursClasses = false;
	fUseSeparatorLinesBetweenColorContourClasses = false;
	fUseIsoLineGabWithCustomContours = false;
	itsColorIndexCount = 0;
	itsIsoLineStep = 1.f;
	itsColorContoursStep = 1.f;
	itsCustomIsoLineClasses.resize(itsMaxAllowedIsoLineCount, 0.f);
	itsCustomColorContours.resize(itsMaxAllowedIsoLineCount, 0.f);
	itsCustomColorContoursColorIndexies.resize(itsMaxAllowedIsoLineCount, 0);

	itsIsoLineSplineSmoothingFactor = 1;
	itsIsoLineLabelDecimalsCount = 0;
	itsIsoLineZeroClassValue = 0;
	itsMinValue = kFloatMissing;
	itsMaxValue = kFloatMissing;
	fUseSingleColorsWithSimpleIsoLines = true; // yksiväriset 'simppelit' isoviivat

	itsIsoLineStatistics.Init();
	itsColorContoursStatistics.Init();
	itsIsoLineDrawSettings.Init();
	itsHatch1.Init();
	itsHatch2.Init();

	itsDefRGBRowSize = 0; // muuta tähän taulukkoon liitttyvää ei 'nollata'
}

// tässä ei tehdä mitää itse datalle, vain eri piirto-optiot kopioidaan
void NFmiIsoLineData::InitDrawOptions(const NFmiIsoLineData &theOther)
{
	itsParam = theOther.itsParam;
	itsTime = theOther.itsTime;

	fUseIsoLines = theOther.fUseIsoLines;
	fUseColorContours = theOther.fUseColorContours;
	itsTrueIsoLineCount = theOther.itsTrueIsoLineCount;
	itsTrueColorContoursCount = theOther.itsTrueColorContoursCount;

	itsIsoLineColor = theOther.itsIsoLineColor;
	itsIsoLineLabelColor = theOther.itsIsoLineLabelColor;
	itsIsoLineStyle = theOther.itsIsoLineStyle;
	itsIsoLineWidth = theOther.itsIsoLineWidth;
	itsIsoLineAnnonationHeight = theOther.itsIsoLineAnnonationHeight;
	fUseLabelBox = theOther.fUseLabelBox;
	fUseSingleColorsWithSimpleIsoLines = theOther.fUseSingleColorsWithSimpleIsoLines;
	fUseIsoLineFeathering = theOther.fUseIsoLineFeathering;
	fUseCustomIsoLineClasses = theOther.fUseCustomIsoLineClasses;
	fUseCustomColorContoursClasses = theOther.fUseCustomColorContoursClasses;
	fUseSeparatorLinesBetweenColorContourClasses = theOther.fUseSeparatorLinesBetweenColorContourClasses;
	fUseIsoLineGabWithCustomContours = theOther.fUseIsoLineGabWithCustomContours;
	itsColorIndexCount = theOther.itsColorIndexCount;
	itsIsoLineStep = theOther.itsIsoLineStep;
	itsColorContoursStep = theOther.itsColorContoursStep;
	itsCustomIsoLineClasses = theOther.itsCustomIsoLineClasses;
	itsCustomColorContours = theOther.itsCustomColorContours;
	itsCustomColorContoursColorIndexies = theOther.itsCustomColorContoursColorIndexies;

	itsIsoLineSplineSmoothingFactor = theOther.itsIsoLineSplineSmoothingFactor;
	itsIsoLineLabelDecimalsCount = theOther.itsIsoLineLabelDecimalsCount;
	itsIsoLineZeroClassValue = theOther.itsIsoLineZeroClassValue;
	itsIsoLineStartClassValue = theOther.itsIsoLineStartClassValue;
	itsMinValue = theOther.itsMinValue;
	itsMaxValue = theOther.itsMaxValue;

	itsIsoLineStatistics = theOther.itsIsoLineStatistics;
	itsColorContoursStatistics = theOther.itsColorContoursStatistics;
	itsIsoLineDrawSettings = theOther.itsIsoLineDrawSettings;

	itsInfo = theOther.itsInfo;

	itsHatch1 = theOther.itsHatch1;
	itsHatch2 = theOther.itsHatch2;

	itsDefRGBRowSize = theOther.itsDefRGBRowSize;
	itsUsedColorsCube = theOther.itsUsedColorsCube;
    itsIsoLineBoxFillColorIndex = theOther.itsIsoLineBoxFillColorIndex;
}

void IsoLineStatistics_::Init(void)
{
	itsMissingValue = 0.f;
	itsMinValue = 0.f;
	itsMinClassValue = 0.f;
	itsMaxValue = 0.f;
	itsMaxClassValue = 0.f;
	itsMeanValue = 0.f;
}

void IsoLineDrawingSettings_::Init(void)
{
	itsStartValue = 0.f;
	itsEndValue = 1.f;
	itsIsoLineStep = 1.f;
	itsNumberOfIsoLines = 0;
}

// Tämä funktio laskee interpoloidun arvon itsCrossSectionData-datasta.
// Oletus annettu piste on aina 0,0  -  1,1 maailmassa ja lasketaan siihen halutut indeksit.
float NFmiIsoLineData::InterpolatedValue(const NFmiPoint &thePoint)
{
	float value = kFloatMissing;
		// pitää tehdä pieni huijaus pisteen kanssa, jos se tulee ulko rajalle eli x = 1 tai y = 1
//		NFmiPoint usedPoint(thePoint);
//		if(usedPoint.X() >= 1.)
//			usedPoint.X(0.9999999); // tällöin pitää laittaa hieman ykköstä pienempi luku, että koodi toimii ulko rajalla
//		if(usedPoint.Y() >= 1.)
//			usedPoint.Y(0.9999999); // tällöin pitää laittaa hieman ykköstä pienempi luku, että koodi toimii ulko rajalla
	double xInd = (itsXNumber-1) * thePoint.X();
	double yInd = (itsYNumber-1) * (1 - thePoint.Y()); // taas pakko tehdä y-akseli kääntö temppu
	int x1 = static_cast<int>(std::floor(xInd));
	int y1 = static_cast<int>(std::floor(yInd));
	int x2 = x1 + 1;
	int y2 = y1 + 1;
	if(x1 >= 0 && x2 < static_cast<int>(itsIsolineData.NX()) && y1 >= 0 && y2 < static_cast<int>(itsIsolineData.NY()))
	{ // lasketaan tulos vain jos ollaan matriisin sisällä, tähän voisi reunoille laskea erikois arvoja jos haluaa
		double xFraction = xInd - x1;
		double yFraction = yInd - y1;
		value = static_cast<float>(NFmiInterpolation::BiLinear(xFraction,
											yFraction,
										itsIsolineData.At(x1, y2),
										itsIsolineData.At(x2, y2),
										itsIsolineData.At(x1, y1),
										itsIsolineData.At(x2, y1)));
	}
	return value;
}

void NFmiIsoLineData::SetIsolineData(const NFmiDataMatrix<float> &isolineData)
{
    itsIsolineData = isolineData;
    itsXNumber = static_cast<int>(itsIsolineData.NX());
    itsYNumber = static_cast<int>(itsIsolineData.NY());
    itsVectorFloatGridData.resize(itsXNumber * itsYNumber, kFloatMissing);
}

void NFmiIsoLineData::ResizeDefRGBArray(size_t newRowSize)
{
	itsDefRGBRowSize = newRowSize;
	if(itsDefRGBRowSize > s_DefRGBRowCapasity)
		itsDefRGBRowSize = s_DefRGBRowCapasity;
}

