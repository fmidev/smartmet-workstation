#include "stdafx.h"
#include "IsolineVizualizationData.h"
#include "ToolMasterColorCube.h"
#include "NFmiDrawParam.h"
#include "SpecialDesctopIndex.h"
#include "CtrlViewFunctions.h"
#include "catlog/catlog.h"

#include <numeric>
#include <math.h>

#ifdef max
#undef max
#undef min
#endif // max

namespace
{
	// Kuinka monta eriarvoista isoviivaa sallitaan maksimissaan oletuksena. 
// Mit‰ isompi luku, sit‰ kauemmin voi tietyiss‰ tilanteissa 'mustan mˆssˆn' piirto kest‰‰ visualisoinneissa.
// T‰ll‰ siis yritet‰‰n est‰‰ ett‰ v‰‰rin asetetuilla piirtoasetuksilla kone jumittaa mahdottoman kauan 
// ja piirt‰‰ ruudulle tuhansittain isoviivoja vieri-viereen.
	const int g_MaxAllowedIsoLineCount = 500;

	// Oletus container ei saa olla tyhj‰
	template<typename Container>
	void padContainerEndWithLastValue(Container& container, int wantedSize)
	{
		auto lastValue = container.back();
		for(auto index = container.size(); index < wantedSize; index++)
			container.push_back(lastValue);
	}

	int getLabelBoxFillColorIndex(const NFmiColor& color, bool doTransparentColor)
	{
		if(doTransparentColor)
			return ToolMasterColorCube::UsedHollowColorIndex();
		else
			return ToolMasterColorCube::RgbToColorIndex(color);
	}

} // nameless namespace

IsolineVizualizationData::IsolineVizualizationData() = default;

bool IsolineVizualizationData::initialize(const ContouringJobData& isolineJobData, const boost::shared_ptr<NFmiDrawParam>& drawParam, float dataMin, float dataMax, bool stationData)
{
	isolineJobData_ = isolineJobData;
	initFromDrawParam(drawParam);
	dataMin_ = dataMin;
	dataMax_ = dataMax;
	stationData_ = stationData;
	usedViewType_ = drawParam->GetViewType(stationData);
	if(useSimpleDefinitions_)
	{
		customCase_ = false;
		auto simpleColorDefinitions = getSimpleColors(drawParam);
		isCorrectlyInitialized_ = initialize(simpleColorDefinitions.first, simpleColorDefinitions.second);
	}
	else
	{
		// Custom contour case
		customCase_ = true;
		auto customColorDefinitions = getCustomColors(drawParam);
		isCorrectlyInitialized_ = initialize(customColorDefinitions.first, customColorDefinitions.second);
	}
	calcFixedToolmasterContourLimits();
	makePossibleErrorLogging();
	return isCorrectlyInitialized_;
}

std::pair<std::vector<float>, std::vector<NFmiColor>> IsolineVizualizationData::getSimpleColors(const boost::shared_ptr<NFmiDrawParam>& drawParam)
{
	std::vector<float> classLimits;
	std::vector<NFmiColor> colors;
	if(useSingleColor_)
	{
		// Simple case with single color
		classLimits.push_back(0); // laitetaan dummy arvo t‰h‰n kun kyse yksiv‰risest‰ tapauksesta
		colors.push_back(drawParam->IsolineColor());
	}
	else
	{
		// Simple case with multiple colors
		classLimits.push_back(drawParam->SimpleIsoLineColorShadeLowValue());
		classLimits.push_back(drawParam->SimpleIsoLineColorShadeMidValue());
		classLimits.push_back(drawParam->SimpleIsoLineColorShadeHighValue());
		classLimits.push_back(drawParam->SimpleIsoLineColorShadeHigh2Value());
		colors.push_back(drawParam->SimpleIsoLineColorShadeLowValueColor());
		colors.push_back(drawParam->SimpleIsoLineColorShadeMidValueColor());
		colors.push_back(drawParam->SimpleIsoLineColorShadeHighValueColor());
		colors.push_back(drawParam->SimpleIsoLineColorShadeHigh2ValueColor());
		colors.push_back(drawParam->SimpleIsoLineColorShadeHigh3ValueColor());
	}
	return std::make_pair(classLimits, colors);
}

// Ennen smartmet versio 5.14.5.0 pidettiin custom isoviiva ja contour arvot samoina.
// T‰llˆin ei siis kannattanut piirt‰‰ isoviivoja niiden custom v‰reill‰, koska ne
// hukkuisivat contour v‰reihin ja ne korvattiin valitulla (yhdell‰) isoviiva v‰rill‰.
// N‰m‰ samat custom v‰rit ja luokkarajat talletettiin n‰yttˆmakroihin sek‰ isoviivan 
// ett‰ contour arvojen kohdalle, joten taaksep‰in yhteensopivuuden nimiss‰ t‰llˆin
// isoviivav‰ri pit‰‰ korvata sill‰ yhdell‰ valitulla isoviivav‰rill‰.
static bool IsJointIsolineContourCase(const boost::shared_ptr<NFmiDrawParam>& drawParam)
{
	if(drawParam->SpecialIsoLineValues() != drawParam->SpecialContourValues())
		return false;
	if(drawParam->SpecialIsoLineColorIndexies() != drawParam->SpecialContourColorIndexies())
		return false;
	return true;
}

std::pair<std::vector<float>, std::vector<int>> IsolineVizualizationData::getCustomColors(const boost::shared_ptr<NFmiDrawParam>& drawParam)
{
	const auto& classLimits = drawParam->SpecialIsoLineValues();
	std::vector<int> colorIndexies = drawParam->SpecialIsoLineColorIndexies();
	if(usedViewType_ == NFmiMetEditorTypes::View::kFmiColorContourIsoLineView && ::IsJointIsolineContourCase(drawParam))
	{
		// Jos colorcontour ja isoviivat piirret‰‰n yht‰aikaa, piirret‰‰n isoviivat yhdell‰ 
		// v‰rill‰, koska muuten isoviivat h‰vi‰vat saman v‰risten color-contourien sekaan.
		colorIndexies.clear();
		colorIndexies.push_back(ToolMasterColorCube::RgbToColorIndex(drawParam->IsolineColor()));
	}
	return std::make_pair(classLimits, colorIndexies);
}

void IsolineVizualizationData::initFromDrawParam(const boost::shared_ptr<NFmiDrawParam>& drawParam)
{
	useSimpleDefinitions_ = drawParam->UseSimpleIsoLineDefinitions();
	useSingleColor_ = drawParam->UseSingleColorsWithSimpleIsoLines();
	useColorBlending_ = drawParam->DoIsoLineColorBlend();
	usedStep_ = static_cast<float>(drawParam->IsoLineGab());
	usedZeroLimitValue_ = drawParam->SimpleIsoLineZeroValue();
	splineSmoothingFactor_ = drawParam->IsoLineSplineSmoothingFactor();
	labelTextColorIndex_ = ToolMasterColorCube::RgbToColorIndex(drawParam->IsolineTextColor());
	labelBoxFillColorIndex_ = ::getLabelBoxFillColorIndex(drawParam->IsolineLabelBoxFillColor(), drawParam->UseTransparentFillColor());
	labelDecimalsCount_ = drawParam->IsoLineLabelDigitCount();
	useFeathering_ = drawParam->UseIsoLineFeathering();
	useLabelBox_ = drawParam->ShowSimpleIsoLineLabelBox();

	if(useSimpleDefinitions_)
	{
		lineWidths_.push_back(drawParam->SimpleIsoLineWidth());
		lineStyles_.push_back(drawParam->SimpleIsoLineLineStyle());
		labelHeights_.push_back(drawParam->SimpleIsoLineLabelHeight());
	}
	else
	{
		lineWidths_ = drawParam->SpecialIsoLineWidth();
		if(lineWidths_.empty())
			lineWidths_.push_back(drawParam->SimpleIsoLineWidth());

		lineStyles_ = drawParam->SpecialIsoLineStyle();
		if(lineStyles_.empty())
			lineStyles_.push_back(drawParam->SimpleIsoLineLineStyle());

		labelHeights_ = drawParam->SpecialIsoLineLabelHeight();
		if(labelHeights_.empty())
			labelHeights_.push_back(3.f); // drawParam->SimpleIsoLineLabelHeight());
	}
}

bool IsolineVizualizationData::calculateIsolineStartAndCount()
{
	if(!customCase_)
	{
		if(dataMin_ == kFloatMissing || dataMax_ == kFloatMissing)
		{
			initializationErrorMessage_ = "IsolineVizualizationData: given dataMin and/or dataMax was missing value, nothing to be done";
			return false;
		}

		if(usedStep_ <= 0)
		{
			initializationErrorMessage_ = "IsolineVizualizationData: given isoline step was zero or negative value, nothing to be done";
			return false;
		}

		startLimitValue_ = usedZeroLimitValue_ + ((int(dataMin_ / usedStep_) - 0) * usedStep_);
		float endValue = usedZeroLimitValue_ + ((int(dataMax_ / usedStep_) + 1) * usedStep_);
		usedIsolineCount_ = static_cast<int>(((endValue - startLimitValue_) / usedStep_) + 1);
		if(usedIsolineCount_ > g_MaxAllowedIsoLineCount)
			usedIsolineCount_ = g_MaxAllowedIsoLineCount;
	}
	return true;
}

bool IsolineVizualizationData::initialize(const std::vector<float>& classLimits, const std::vector<NFmiColor>& colors)
{
	originalClassLimits_ = classLimits;
	originalColors_ = colors;
	return doSimpleIsolineInitialization();
}

bool IsolineVizualizationData::initialize(const std::vector<float>& classLimits, const std::vector<int>& colorIndexies)
{
	originalClassLimits_ = classLimits;
	originalColorIndexies_ = colorIndexies;
	return doCustomIsolineInitialization();
}

bool IsolineVizualizationData::doSimpleIsolineInitialization()
{
	missingLimitCleanUp();
	if(!doFinalLimitsAndColorsChecks())
		return false;

	originalColorIndexies_ = ColorContouringData::calcDefaultColorTableIndexies(originalColors_);
	if(useSingleColor_)
	{
		justCopyFinalValues();
	}
	else
	{
		if(useColorBlending_)
		{
			calculateBlendedColors();
		}
		else
		{
			calculateNonBlendingColors();
		}
	}

	return true;
}

void IsolineVizualizationData::calculateBlendedColors()
{
	float limitValue = startLimitValue_;
	for(int limitCounter = 0; limitCounter < usedIsolineCount_; limitCounter++, limitValue += usedStep_)
	{
		finalClassLimits_.push_back(limitValue);
		auto colorValues = getBlendedColorValues(limitValue);
		finalColorIndexies_.push_back(colorValues.first);
		finalColors_.push_back(colorValues.second);
	}
}

void IsolineVizualizationData::calculateNonBlendingColors()
{
	float limitValue = startLimitValue_;
	for(int limitCounter = 0; limitCounter < usedIsolineCount_; limitCounter++, limitValue += usedStep_)
	{
		finalClassLimits_.push_back(limitValue);
		auto colorValues = getMultiColorValues(limitValue);
		finalColorIndexies_.push_back(colorValues.first);
		finalColors_.push_back(colorValues.second);
	}
}

std::pair<int, NFmiColor> IsolineVizualizationData::getBlendedColorValues(float limitValue)
{
	// 1. Haetaan limitValue:ta l‰himm‰n classLimitin elementin indeksi.
	auto closestValueIndex = CtrlViewUtils::GetClosestValueIndex(limitValue, originalClassLimits_);
	auto closestClassLimitValue = originalClassLimits_[closestValueIndex];
	if(closestValueIndex == 0 && limitValue <= closestClassLimitValue)
		return std::make_pair(originalColorIndexies_[0], originalColors_[0]);
	else if(closestValueIndex == (originalClassLimits_.size() - 1) && limitValue >= closestClassLimitValue)
		return std::make_pair(originalColorIndexies_[closestValueIndex], originalColors_[closestValueIndex]);
	else if(limitValue == closestClassLimitValue)
		return std::make_pair(originalColorIndexies_[closestValueIndex], originalColors_[closestValueIndex]);
	else
	{
		// Lopuksi pit‰‰ sitten tehd‰ v‰rien blendailuja:

		// 1. Haetaan annetun limitValue:n ymp‰rilt‰ olevien arvojen indeksit taulukkoon
		auto firstIndex = closestValueIndex;
		auto secondIndex = closestValueIndex;
		if(limitValue < closestClassLimitValue)
			firstIndex = closestValueIndex - 1;
		else
			secondIndex = closestValueIndex + 1;

		// 2. Haetaan indeksien avulla limitValue:n ymp‰rilt‰ olevat arvot
		auto firstLimitValue = originalClassLimits_[firstIndex];
		auto secondLimitValue = originalClassLimits_[secondIndex];

		// 3. Lasketaan miksauskerroin NFmiColor::Mix funktiota varten
		auto mixFactor = (secondLimitValue - limitValue) / (secondLimitValue - firstLimitValue);

		// 4. Miksataan uusi v‰ri
		auto blendedColor = originalColors_[secondIndex];
		blendedColor.Mix(originalColors_[firstIndex], mixFactor);

		// 5. Haetaan uuden v‰rin indeksi default color-cubesta
		auto blendedColorIndex = ToolMasterColorCube::RgbToColorIndex(blendedColor);

		return std::make_pair(blendedColorIndex, blendedColor);
	}
}

// Haetaan annettua limitValue arvoa pienempi tai yht‰suuren classLimitin indeksi.
// Jos arvo menee yli kaikkien classLimittien, palautetaan indeksill‰ limitCount + 1.
std::pair<int, NFmiColor> IsolineVizualizationData::getMultiColorValues(float limitValue)
{
	size_t usedColorIndex = 0;
	if(limitValue <= originalClassLimits_.front())
	{
		usedColorIndex = 0;
	}
	else if(limitValue > originalClassLimits_.back())
	{
		// T‰m‰ menee tarkoituksella yhden yli limit taulukon (on samalla v‰ri taulukon viimeinen indeksi)
		usedColorIndex = originalClassLimits_.size();
	}
	else
	{
		for(size_t index = originalClassLimits_.size() - 2; index >= 0; index--)
		{
			if(limitValue > originalClassLimits_[index])
			{
				usedColorIndex = index + 1;
				break;
			}
		}
	}
	return std::make_pair(originalColorIndexies_[usedColorIndex], originalColors_[usedColorIndex]);
}

bool IsolineVizualizationData::doCustomIsolineInitialization()
{
	originalColors_ = ColorContouringData::calcDefaultColorTableColors(originalColorIndexies_);
	missingLimitCleanUp();
	if(!doFinalLimitsAndColorsChecks())
		return false;

	justCopyFinalValues();
	usedIsolineCount_ = static_cast<int>(finalClassLimits_.size());
	padCustomAttributes();
	return true;
}

// Tietyille custom isoviiva asetuksille pit‰‰ lis‰t‰ arvoja vektorien loppuun.
// usedIsolineCount_ on lopullinen m‰‰r‰ isoviivoja ja t‰m‰ m‰‰r‰ pit‰‰ saada 
// vektoreihin arvoja. Lis‰ykset tehd‰‰n lis‰‰m‰ll‰ viimeist‰ arvoa loppuun.
void IsolineVizualizationData::padCustomAttributes()
{
	// finalClassLimits_ datalla pit‰isi olla jo oikea m‰‰r‰ arvoja, muihin
	// pit‰‰ tarvittaessa lis‰t‰ viimeist‰ arvoa loppuun, ett‰ saadaan samat vektorikoot.
	::padContainerEndWithLastValue(finalColorIndexies_, usedIsolineCount_);
	::padContainerEndWithLastValue(lineWidths_, usedIsolineCount_);
	::padContainerEndWithLastValue(lineStyles_, usedIsolineCount_);
	::padContainerEndWithLastValue(labelHeights_, usedIsolineCount_);
}

// Tutkitaan onko rajoja ja v‰reja tarpeeksi. 
// Niiden m‰‰r‰ riippuu siit‰ k‰ytet‰‰nkˆ color blendausta vai ei.
// Palauttaa false, jos jokin on pieless‰ ja raportoi siit‰.
// Kutsutaan missingLimitCleanUp- ja checkIfColorBlendingIsUsed metodi kutsujen j‰lkeen. 
bool IsolineVizualizationData::doFinalLimitsAndColorsChecks()
{
	if(!calculateIsolineStartAndCount())
		return false;

	auto limitSize = originalClassLimits_.size();
	if(limitSize == 0)
	{
		initializationErrorMessage_ = "IsolineVizualizationData: no original class limits given, nothing to be done";
		return false;
	}

	auto colorSize = originalColors_.size();
	if(colorSize == 0)
	{
		initializationErrorMessage_ = "IsolineVizualizationData: no original colors given, nothing to be done";
		return false;
	}

	if(!std::is_sorted(originalClassLimits_.begin(), originalClassLimits_.end()))
	{
		initializationErrorMessage_ = "IsolineVizualizationData: given limits were not in ascending order, nothing to be done";
		return false;
	}

	if(customCase_)
	{
		// Varmistetaan ett‰ custom tapauksessa ei menn‰ yli ‰‰rirajojen
		if(limitSize > g_MaxAllowedIsoLineCount)
			originalClassLimits_.resize(g_MaxAllowedIsoLineCount);
		if(colorSize > g_MaxAllowedIsoLineCount)
			originalColors_.resize(g_MaxAllowedIsoLineCount);
	}
	else
	{
		// Single color tapausta ei tarvitse tarkistaa t‰ss‰
		if(!useSingleColor_)
		{
			// Vain simple tapauksen useColorBlending tapauksessa pit‰‰ olla limit+v‰ri m‰‰r‰t samoja
			if(useColorBlending_)
			{
				if(limitSize != colorSize)
				{
					auto minSize = std::min(limitSize, colorSize);
					minSize = std::min(minSize, static_cast<size_t>(g_MaxAllowedIsoLineCount));
					originalColors_.resize(minSize);
					originalClassLimits_.resize(minSize);
				}
			}
			else
			{
				// Ei color-blend tapauksessa v‰rej‰ oltava 1 enemm‰n kuin rajoja
				if((limitSize + 1) != colorSize)
				{
					if(colorSize < 2)
					{
						initializationErrorMessage_ = "IsolineVizualizationData: In non color blending case the color size was <= 1, nothing to be done";
						return false;
					}
					else
					{
						auto newLimitSize = (limitSize >= colorSize) ? (colorSize - 1) : limitSize;
						newLimitSize = std::min(newLimitSize, static_cast<size_t>(g_MaxAllowedIsoLineCount));
						originalColors_.resize(newLimitSize);
						originalClassLimits_.resize(newLimitSize + 1);
					}
				}
			}
		}
	}

	return true;
}

// T‰ss‰ poistetaan kaikki missing limit arvot (originalClassLimits_ vektorista) ja 
// niihin liittyv‰t v‰rit (originalColors_ vektorista).
// Poistetaan myˆs limit+v‰ri, jos kahden per‰kk‰isen limitin arvo on sama (tyhj‰ v‰li, jolloin j‰lkimm‰inen raja+v‰ri poistuu).
// Oletus: Kun t‰t‰ kutsutaan custom-contour alustuksesta, pit‰‰ originalColors_ olla alustettuna jo.
void IsolineVizualizationData::missingLimitCleanUp()
{
	auto limitsSize = originalClassLimits_.size();
	auto colorsSize = originalColors_.size();
	if(limitsSize && colorsSize)
	{
		std::vector<float> tmpLimits;
		std::vector<NFmiColor> tmpColors;

		// limit:ien ja color:ien lukum‰‰r‰t voivat olla erisuuria, k‰yd‰‰n niit‰ l‰pi vain limitsSize:iin asti
		for(size_t index = 0; index < limitsSize; index++)
		{
			// 1. Siivotaan pois mahdolliset missing limitit ja niihin liittyv‰t v‰rit.
			if(originalClassLimits_[index] != kFloatMissing)
			{
				// 2. Siivotaan myˆs pois sellaiset rajat (ja v‰rit), miss‰ per‰kk‰iset limitit ovat samoja (et‰isyys on 0)
				if(index > 0 && originalClassLimits_[index] == originalClassLimits_[index - 1])
					continue;

				tmpLimits.push_back(originalClassLimits_[index]);
				if(index < colorsSize)
					tmpColors.push_back(originalColors_[index]);
			}
		}

		if(!useColorBlending_ && tmpLimits.size() < colorsSize)
		{
			// V‰ri-blendaus on false tapauksissa otetaan mukaan yksi v‰ri enemm‰n kuin on rajoja, jos mahdollista
			tmpColors.push_back(originalColors_[tmpLimits.size()]);
		}

		originalClassLimits_ = tmpLimits;
		originalColors_ = tmpColors;
	}
}

void IsolineVizualizationData::makePossibleErrorLogging() const
{
	ColorContouringData::makePossibleErrorLogging(initializationErrorMessage_, isCorrectlyInitialized_, isolineJobData_);
}

void IsolineVizualizationData::justCopyFinalValues()
{
	finalClassLimits_ = originalClassLimits_;
	finalColors_ = originalColors_;
	finalColorIndexies_ = originalColorIndexies_;
}

void IsolineVizualizationData::calcFixedToolmasterContourLimits()
{
	finalToolmasterFixedClassLimits_ = finalClassLimits_;
	std::transform(finalToolmasterFixedClassLimits_.begin(), finalToolmasterFixedClassLimits_.end(), finalToolmasterFixedClassLimits_.begin(),
		[&](auto value) {return ColorContouringData::GetToolMasterContourLimitChangeValue(value); });
}
