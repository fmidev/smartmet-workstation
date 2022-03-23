
#include "NFmiIsoLineData.h"

namespace
{
	size_t Matrix2ToolmasterIndex(size_t gridSizeX, size_t yIndex, size_t xIndex)
	{
		return ((yIndex * gridSizeX) + xIndex);
	}

	void GetMinAndMaxValues(const std::vector<float>& values, float& minOut, float& maxOut)
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

	void DoMatrixDataInitialization(const NFmiDataMatrix<float>& valuesMatrix, std::vector<float>& values, float& minOut, float& maxOut)
	{
		values.resize(valuesMatrix.NX() * valuesMatrix.NY());
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

bool NFmiIsoLineData::InitIsoLineData(const NFmiDataMatrix<float>& theValueMatrix, NFmiIsoLineData* otherIsoLineData)
{
	if(!::IsValuematrixOk(theValueMatrix))
		return false;

	BaseInitialization(theValueMatrix);
	::DoMatrixDataInitialization(itsIsolineData, itsVectorFloatGridData, itsDataMinValue, itsDataMaxValue);
	if(otherIsoLineData)
	{
		InitDrawOptions(*otherIsoLineData);
		itsInfo = otherIsoLineData->itsInfo;
	}
	return true;
}

void NFmiIsoLineData::BaseInitialization(const NFmiDataMatrix<float>& theValueMatrix)
{
	DoBaseInitializationReset();
	SetIsolineData(theValueMatrix);
}

// T‰m‰ resetoi kaiken muun paitsi itsInfo, itsParam, itsTime, itsIsolineMinLengthFactor ja itsContourUserDrawData dataosat.
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

// T‰ss‰ ei tehd‰ mit‰‰n itse datalle, vain tietyt eri piirto-optiot kopioidaan.
// Tietyt dataosiot on jo kopioitu NFmiIsoLineData::InitIsoLineData metodissa 
// kutsutuissa metodeissa ennen t‰m‰n metodin kutsua.
// T‰t‰ k‰ytet‰‰n kun originaali hiladataa pit‰‰ harventaa (koskee nyt vain isoviivapiirtoa).
void NFmiIsoLineData::InitDrawOptions(const NFmiIsoLineData &theOther)
{
	fUseIsoLines = theOther.fUseIsoLines;
	fUseColorContours = theOther.fUseColorContours;
	itsTrueIsoLineCount = theOther.itsTrueIsoLineCount;
	itsTrueColorContoursCount = theOther.itsTrueColorContoursCount;

	itsColorContouringData = theOther.itsColorContouringData;
	itsIsolineVizualizationData = theOther.itsIsolineVizualizationData;

	fUseLabelBox = theOther.fUseLabelBox;
	itsIsoLineBoxFillColorIndex = theOther.itsIsoLineBoxFillColorIndex;
	fUseSingleColorsWithSimpleIsoLines = theOther.fUseSingleColorsWithSimpleIsoLines;
	fUseIsoLineFeathering = theOther.fUseIsoLineFeathering;
	fUseCustomIsoLineClasses = theOther.fUseCustomIsoLineClasses;
	fUseCustomColorContoursClasses = theOther.fUseCustomColorContoursClasses;
	fUseSeparatorLinesBetweenColorContourClasses = theOther.fUseSeparatorLinesBetweenColorContourClasses;
	fUseIsoLineGabWithCustomContours = theOther.fUseIsoLineGabWithCustomContours;
	fDrawLabelsOverContours = theOther.fDrawLabelsOverContours;

	itsDataMinValue = theOther.itsDataMinValue;
	itsDataMaxValue = theOther.itsDataMaxValue;
	itsClassMinValue = theOther.itsClassMinValue;
	itsClassMaxValue = theOther.itsClassMaxValue;

	itsHatch1 = theOther.itsHatch1;
	itsHatch2 = theOther.itsHatch2;

	// Huom! itsDefRGB -taulukkoa ei tarvitse kopioida t‰ss‰, se alustetaan vasta myˆhemmin
	itsDefRGBRowSize = theOther.itsDefRGBRowSize;
	itsUsedColorsCube = theOther.itsUsedColorsCube;

	itsSingleSubMapViewHeightInMillimeters = theOther.itsSingleSubMapViewHeightInMillimeters;
	itsDataGridToViewHeightRatio = theOther.itsDataGridToViewHeightRatio;
	itsColorContouringData = theOther.itsColorContouringData;
}

void NFmiIsoLineData::InitContourUserDrawData(const NFmiDataMatrix<float>& valueMatrix, const NFmiDataMatrix<NFmiPoint>& coordinateMatrix)
{
	itsContourUserDrawData.Init(valueMatrix, coordinateMatrix);
}

bool NFmiIsoLineData::UseContourUserDraw() const
{
	return itsContourUserDrawData.UseUserDraw();
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
