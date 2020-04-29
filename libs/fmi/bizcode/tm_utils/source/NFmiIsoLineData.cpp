
#include "NFmiIsoLineData.h"

namespace
{
	size_t Matrix2ToolmasterIndex(size_t gridSizeX, size_t yIndex, size_t xIndex)
	{
		return ((yIndex * gridSizeX) + xIndex);
	}

	void GetMinAndMaxValues(const checkedVector<float>& values, float& minOut, float& maxOut)
	{
		minOut = 3.4E+38f;
		maxOut = -3.4E+38f;
		for(auto value : values)
		{
			if(value == kFloatMissing)
				continue;
			if(value > maxOut)
				maxOut = value;
			if(value < minOut)
				minOut = value;
		}
	}

	void DoMatrixDataInitialization(const NFmiDataMatrix<float>& valuesMatrix, checkedVector<float>& values, float& minOut, float& maxOut)
	{
		values.resize(valuesMatrix.NX()* valuesMatrix.NY());
		auto gridSizeX = valuesMatrix.NX();
		for(size_t yIndex = 0; yIndex < valuesMatrix.NY(); yIndex++)
		{
			for(size_t xIndex = 0; xIndex < gridSizeX; xIndex++)
				values[::Matrix2ToolmasterIndex(gridSizeX, yIndex, xIndex)] = valuesMatrix[xIndex][yIndex];
		}

		::GetMinAndMaxValues(values, minOut, maxOut);
	}
}

// *********** NFmiContourUserDrawData osio alkaa **************************

void NFmiContourUserDrawData::Init(const NFmiDataMatrix<float>& valueMatrix, const NFmiDataMatrix<NFmiPoint>& coordinateMatrix)
{
	if(!(valueMatrix.NX() && valueMatrix.NY()))
		throw std::runtime_error(std::string("Empty valueMatrix given as parameter in ") + __FUNCTION__);
	if(!(coordinateMatrix.NX() && coordinateMatrix.NY()))
		throw std::runtime_error(std::string("Empty coordinateMatrix given as parameter in ") + __FUNCTION__);
	if(coordinateMatrix.NX() != valueMatrix.NX() || coordinateMatrix.NY() != valueMatrix.NY())
		throw std::runtime_error(std::string("Different size matrices (coordinateMatrix and valueMatrix) given as parameter in ") + __FUNCTION__);

	// Ensin alustetaan value datarakenteet
	itsUserDrawValuesMatrix = valueMatrix;
	itsXNumber = static_cast<int>(itsUserDrawValuesMatrix.NX());
	itsYNumber = static_cast<int>(itsUserDrawValuesMatrix.NY());
	::DoMatrixDataInitialization(itsUserDrawValuesMatrix, itsUserDrawValues, itsDataMinValue, itsDataMaxValue);

	// Sitten alustetaan coordinate jutut
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
			auto toolmasterIndex = ::Matrix2ToolmasterIndex(gridSizeX, yIndex, xIndex);
			xCoordinates[toolmasterIndex] = static_cast<float>(coordinate.X());
			yCoordinates[toolmasterIndex] = static_cast<float>(coordinate.Y());
		}
	}
}

bool NFmiContourUserDrawData::UseUserDraw() const
{
	return !xCoordinates.empty() && !yCoordinates.empty();
}

// *********** NFmiContourUserDrawData osio loppuu **************************


// *********** NFmiIsoLineData osio alkaa **************************

static bool IsValuematrixOk(const NFmiDataMatrix<float>& theValueMatrix)
{
	return (theValueMatrix.NX() * theValueMatrix.NY()) != 0;
}

bool NFmiIsoLineData::Init(const NFmiDataMatrix<float>& theValueMatrix, int theMaxAllowedIsoLineCount)
{
	if(!::IsValuematrixOk(theValueMatrix))
		return false;

	BaseInitialization(theValueMatrix, 500);
	::DoMatrixDataInitialization(itsIsolineData, itsVectorFloatGridData, itsDataMinValue, itsDataMaxValue);
	return true;
}

void NFmiIsoLineData::BaseInitialization(const NFmiDataMatrix<float>& theValueMatrix, int theMaxAllowedIsoLineCount)
{
	DoBaseInitializationReset();
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

// Tämä resetoi kaiken muun paitsi itsInfo, itsParam, itsTime, itsIsolineMinLengthFactor ja itsContourUserDrawData dataosat.
void NFmiIsoLineData::DoBaseInitializationReset()
{
	NFmiIsoLineData tmpIsoLineData;
	tmpIsoLineData.itsInfo = this->itsInfo;
	tmpIsoLineData.itsTime = this->itsTime;
	tmpIsoLineData.itsParam = this->itsParam;
	tmpIsoLineData.itsIsolineMinLengthFactor = this->itsIsolineMinLengthFactor;
	tmpIsoLineData.itsContourUserDrawData = std::move(this->itsContourUserDrawData);
	*this = tmpIsoLineData;
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
	fDrawLabelsOverContours = theOther.fDrawLabelsOverContours;
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
	itsDataMinValue = theOther.itsDataMinValue;
	itsDataMaxValue = theOther.itsDataMaxValue;
	itsClassMinValue = theOther.itsClassMinValue;
	itsClassMaxValue = theOther.itsClassMaxValue;

	itsInfo = theOther.itsInfo;

	itsHatch1 = theOther.itsHatch1;
	itsHatch2 = theOther.itsHatch2;
	itsSingleSubMapViewHeightInMillimeters = theOther.itsSingleSubMapViewHeightInMillimeters;

	itsDefRGBRowSize = theOther.itsDefRGBRowSize;
	itsUsedColorsCube = theOther.itsUsedColorsCube;
    itsIsoLineBoxFillColorIndex = theOther.itsIsoLineBoxFillColorIndex;
	itsIsolineMinLengthFactor = theOther.itsIsolineMinLengthFactor;
}

bool NFmiIsoLineData::UseContourDraw() const
{
	return fUseColorContours != 0;
}

void NFmiIsoLineData::InitContourUserDrawData(const NFmiDataMatrix<float>& valueMatrix, const NFmiDataMatrix<NFmiPoint>& coordinateMatrix)
{
	itsContourUserDrawData.Init(valueMatrix, coordinateMatrix);
}

bool NFmiIsoLineData::UseContourUserDraw() const
{
	return itsContourUserDrawData.UseUserDraw();
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
