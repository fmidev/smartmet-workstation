
#include "NFmiIsoLineData.h"

// *********** NFmiUserGridData osio alkaa **************************

void NFmiUserGridData::Init(const NFmiDataMatrix<NFmiPoint>& coordinateMatrix)
{
	coordinateMatrix_ = coordinateMatrix;
	auto totalSize = coordinateMatrix_.NX() * coordinateMatrix_.NY();
	if(totalSize == 0)
	{
		std::string errorMessage = "Error in ";
		errorMessage += __FUNCTION__;
		errorMessage += ": zero size coordinateMatrix was given to do Toolmaster user-grid-draw";
		throw std::runtime_error(errorMessage);
	}
	xCoordinates.resize(totalSize, kFloatMissing);
	yCoordinates.resize(totalSize, kFloatMissing);
	auto gridSizeX = coordinateMatrix_.NX();

	for(size_t yIndex = 0; yIndex < coordinateMatrix_.NY(); yIndex++)
	{
		for(size_t xIndex = 0; xIndex < gridSizeX; xIndex++)
		{
			const auto &coordinate = coordinateMatrix_[xIndex][yIndex];
			auto toolmasterIndex = NFmiIsoLineData::Matrix2ToolmasterIndex(gridSizeX, yIndex, xIndex);
			xCoordinates[toolmasterIndex] = static_cast<float>(coordinate.X());
			yCoordinates[toolmasterIndex] = static_cast<float>(coordinate.Y());
		}
	}
}

bool NFmiUserGridData::UseUserDraw() const
{
	return false;
}

// *********** NFmiUserGridData osio loppuu **************************


// *********** NFmiIsoLineData osio alkaa **************************

static bool IsValuematrixOk(const NFmiDataMatrix<float>& theValueMatrix)
{
	return (theValueMatrix.NX() * theValueMatrix.NY()) != 0;
}

size_t NFmiIsoLineData::Matrix2ToolmasterIndex(size_t gridSizeX, size_t yIndex, size_t xIndex)
{
	return ((yIndex * gridSizeX) + xIndex);
}

bool NFmiIsoLineData::Init(const NFmiDataMatrix<float>& theValueMatrix, int theMaxAllowedIsoLineCount)
{
	if(!::IsValuematrixOk(theValueMatrix))
		return false;

	BaseInitialization(theValueMatrix, 500);

	for(size_t yIndex = 0; yIndex < itsYNumber; yIndex++)
	{
		for(size_t xIndex = 0; xIndex < itsXNumber; xIndex++)
			itsVectorFloatGridData[Matrix2ToolmasterIndex(itsXNumber, yIndex, xIndex)] = itsIsolineData[xIndex][yIndex];
	}

	GetMinAndMaxValues(itsMinValue, itsMaxValue);
	itsIsoLineStatistics.itsMaxValue = itsMaxValue;
	itsIsoLineStatistics.itsMinValue = itsMinValue;
	return true;
}

void NFmiIsoLineData::GetMinAndMaxValues(float& theMinOut, float& theMaxOut) const
{
	theMinOut = 3.4E+38f;
	theMaxOut = -3.4E+38f;
	for(auto value : itsVectorFloatGridData)
	{
		if(value == kFloatMissing)
			continue;
		if(value > theMaxOut)
			theMaxOut = value;
		if(value < theMinOut)
			theMinOut = value;
	}
}

void NFmiIsoLineData::BaseInitialization(const NFmiDataMatrix<float>& theValueMatrix, int theMaxAllowedIsoLineCount)
{
	*this = NFmiIsoLineData();
	SetIsolineData(theValueMatrix);
	itsMaxAllowedIsoLineCount = theMaxAllowedIsoLineCount;

	itsIsoLineColor.resize(itsMaxAllowedIsoLineCount, 0);
	itsIsoLineLabelColor.resize(itsMaxAllowedIsoLineCount, 0);
	itsIsoLineStyle.resize(itsMaxAllowedIsoLineCount, 0);
	itsIsoLineWidth.resize(itsMaxAllowedIsoLineCount, 0.f);
	itsIsoLineAnnonationHeight.resize(itsMaxAllowedIsoLineCount, 0.f);
	itsCustomIsoLineClasses.resize(itsMaxAllowedIsoLineCount, 0.f);
	itsCustomColorContours.resize(itsMaxAllowedIsoLineCount, 0.f);
	itsCustomColorContoursColorIndexies.resize(itsMaxAllowedIsoLineCount, 0);
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
	itsIsolineMinLengthFactor = theOther.itsIsolineMinLengthFactor;
}

void NFmiIsoLineData::InitUserGridCoordinateData(const NFmiDataMatrix<NFmiPoint>& coordinateMatrix)
{
	itsUserGridCoordinateData.Init(coordinateMatrix);
}

bool NFmiIsoLineData::UseUserDraw() const
{
	return itsUserGridCoordinateData.UseUserDraw();
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

// *********** NFmiIsoLineData osio loppuu **************************
