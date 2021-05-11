#include "stdafx.h"
#include "ColorContouringData.h"
#include "ToolMasterColorCube.h"
#include "NFmiDrawParam.h"
#include "SpecialDesctopIndex.h"
#include "CtrlViewFunctions.h"
#include "catlog/catlog.h"

#include <agX/agx.h>

#include <numeric>
#include <math.h>

#ifdef max
#undef max
#undef min
#endif // max

namespace
{
	const NFmiColor g_DefaultValueRangeColor;

	// Kun toolmaster piirt‰‰ isoviivan discreetist‰ datasta tulokset voivat olla hieman yll‰tt‰vi‰.
	// Esim. kokonaispilvisyys (10% tarkkuudella) voi menn‰ pieleen jos joillain alueilla on paljon
	// samoja arvoja (esim. 90% ja 80% sekaisin) ja raja menee siin‰ 'v‰liss‰' eli 90%.
	// Ongelma voidaan kiert‰‰, kun esim. kaikkia rajoja pienennet‰‰n sis‰isesti hieman, jolloin
	// 90% rajasta tuleekin oikeasti 89.99999%. T‰llˆin isoviivat kiertavat kauniisti 90% arvot.
	static const float gToolMasterContourLimitChangeValue = std::numeric_limits<float>::epsilon() * 3; // t‰m‰ pit‰‰ olla pieni arvo (~epsilon) koska muuten pienet rajat eiv‰t toimi, mutta pelkk‰ epsilon on liian pieni


	// Custom color laskuissa tulee mukaan transparenttien v‰rien yhdist‰minen toisiin v‰reihin.
	// Jos l‰pik‰yt‰v‰ v‰ri on transparentti, pit‰‰ se yhdist‰‰ seuraavaan v‰riin, jos sellainen lˆytyy.
	// Jos seuraava v‰ri-indeksi on myˆs transparentti, ei tehd‰ yhdistely‰.
	// Jos viimeinen v‰ri-indeksi on transparentti, mutta sit‰ edellt‰v‰ ei ole, pit‰‰ nekin yhdist‰‰.
	std::vector<NFmiColor> calcDefaultColorTableColorsFromCustomColorIndexies(const std::vector<int>& colorIndexies)
	{
		std::vector<NFmiColor> colors;
		for(size_t index = 0; index < colorIndexies.size(); index++)
		{
			auto colorIndex = colorIndexies[index];
			auto lastColorIndexCheckedNow = (index >= colorIndexies.size() - 1);
			if(lastColorIndexCheckedNow)
			{
				auto lastColorIsTransparent = (colorIndex == ToolMasterColorCube::UsedHollowColorIndex());
				if(lastColorIsTransparent)
				{
					// Jos viimeinen v‰ri oli l‰pin‰kyv‰, pit‰‰ se yhdist‰‰ listalle viimeiseksi lis‰ttyyn v‰riin
					// laittamalla kyseisen v‰rin alpha t‰ysin l‰pin‰kyv‰ksi.
					if(!colors.empty())
						colors.back().Alpha(1.f);
				}
				else
				{
					// Jos viimeinen v‰ri ei ollut transparentti, lis‰t‰‰n se vain listaan.
					colors.push_back(ToolMasterColorCube::ColorIndexToRgb(colorIndex));
				}
			}
			else
			{
				auto nextColorIndex = colorIndexies[index + 1];
				auto color1IsTransparent = (colorIndex == ToolMasterColorCube::UsedHollowColorIndex());
				auto color2IsTransparent = (nextColorIndex == ToolMasterColorCube::UsedHollowColorIndex());
				if(color1IsTransparent || color2IsTransparent)
				{
					if(color1IsTransparent && color2IsTransparent)
					{
						// Kaksi per‰kk‰ist‰ l‰pin‰kyv‰‰ v‰ri‰, laitetaan nykyinen sellaisenaan v‰rilistaan
						colors.push_back(ToolMasterColorCube::ColorIndexToRgb(colorIndex));
					}
					else if(color1IsTransparent)
					{
						// Nykyinen on transparentti ja seuraava ei ole ==>
						// Otetaan itse v‰ri seuraavasta indeksist‰
						auto combinedTransparentColor = ToolMasterColorCube::ColorIndexToRgb(nextColorIndex);
						// Laitetaan opaciteetti t‰ysin pois p‰‰lt‰
						combinedTransparentColor.Alpha(1.f);
						colors.push_back(combinedTransparentColor);
						// Hyp‰t‰‰n seuraava v‰ri yli
						++index;
					}
					else
					{
						colors.push_back(ToolMasterColorCube::ColorIndexToRgb(colorIndex));
					}
				}
				else
				{
					colors.push_back(ToolMasterColorCube::ColorIndexToRgb(colorIndex));
				}
			}
		}
		return colors;
	}

	std::vector<int> calcNewColorTableIndexies(const std::vector<NFmiColor>& colors)
	{
		std::vector<int> colorIndexies;
		int colorIndex = ToolMasterColorCube::SpecialColorCountInColorTableStart();
		for(const auto& color : colors)
		{
			if(ToolMasterColorCube::IsColorFullyOpaque(color))
				colorIndexies.push_back(colorIndex++);
			else
				colorIndexies.push_back(ToolMasterColorCube::UsedHollowColorIndex());
		}
		return colorIndexies;
	}

	// On havaittu, ett‰ jos lasketaan originaali stepeill‰ blendausrajat ja v‰rit, niit‰
	// tulee niin paljon ett‰ ihmissilm‰ ei en‰‰ erota v‰rej‰ toisistaan ja piirto vain
	// turhan takia hidastuu, t‰llˆin halutaan etsia sopiva steppi, mill‰ saadaan riitt‰v‰n
	// hyv‰‰ j‰lke‰, ilman ett‰ contouraus kuitenkaan hidastuu liikaa.
	// Esim. On haluttu piirt‰‰ pintapaineen arvoja v‰lill‰ 980 - 1010 (limitRange = 30) 
	// ja haluttu tehd‰ se 0.001 v‰lein, tulisi eri contouraus rajoja ja v‰rej‰  30000 kpl. 
	// Mik‰‰n kone ei voisi visualisoida v‰rien siirtym‰ tuolla tarkkuudella (10000 s‰vyn 
	// muutos yhdest‰ v‰rist‰ toiseen), eik‰ mik‰‰n olento pystyisi niit‰ silmill‰‰n erottelemaan.
	// Nyt halutaan laskea sellainen steppi, mik‰ tuottaa maksimissaan ehk‰ n. 300-500 v‰ri‰ koko skaalaan.
	float calcSuitableBlendingStep(float originalStep, float limitRange, float maximumAllowedLimitCount)
	{
		auto originalBlendedColorCount = (limitRange / originalStep) + 2.f;
		if(originalBlendedColorCount < maximumAllowedLimitCount)
			return originalStep;

		// V‰rej‰ on tulossa liikaa, pit‰‰ alkaa harventamaan
		float sparsingFactor = originalBlendedColorCount / maximumAllowedLimitCount;
		auto sparsedRawStep = originalStep * sparsingFactor;
		// P‰‰tell‰‰n uuden stepin suuruusluokka
		auto log10Value = std::log10f(sparsedRawStep);
		auto roundedLog10ValueInteger = std::roundf(log10Value);
		auto normilizedSparsedRawStep = sparsedRawStep / std::pow(10.f, roundedLog10ValueInteger);
		// T‰ss‰ on listattu kaikki halutut 'j‰rkev‰t' stepit v‰lill‰ [1,10[ (>=1 ja <10).
		// Lopullinen steppi laitetaan sitten vain oikeaan koko luokkaan 
		// (esim. 2:n tapauksessa ...,0.02, 0.2, 2, 20, 200,...).
		std::vector<float> suitablePowerOfOneSteps{0.5f, 1.f, 2.f, 2.5f, 5.f, 10.f };
		auto suitableStepsPos = std::find_if(suitablePowerOfOneSteps.begin(), suitablePowerOfOneSteps.end(),
			[=](auto value) {return (value + std::numeric_limits<float>::epsilon()) >= normilizedSparsedRawStep; });
		if(suitableStepsPos != suitablePowerOfOneSteps.end())
		{
			auto finalSparsedStep = *suitableStepsPos * std::pow(10.f, float(roundedLog10ValueInteger));
			return finalSparsedStep;
		}
		else
			return kFloatMissing;
	}

	// T‰ss‰ lasketaan kuinka monta v‰ri‰ voidaan blendata annettujen v‰rien v‰lille 
	// k‰ytt‰m‰ll‰ oletus v‰rikuutiota, jossa 8x8x8 v‰ri‰. 
	// Suurin v‰riskaala muutos saadaan, jos color1:n jokin kanava eroaa color2:n 
	// vastaavasta 1:ll‰. T‰llˆin n‰iden kahden v‰rin v‰liin mahtuu 6 v‰ri‰ v‰rikuutiosta,
	// t‰llˆin funktio palauttaa maksimi arvon 6.
	// Jos v‰rien maksimi kanava ero on 0, t‰llˆin v‰rit ovat k‰yt‰nnˆss‰ samoja ja 
	// mahdollisten steppien m‰‰r‰ on 0 mik‰ on minimi. 
	// Jos maksimi kanava ero on luokkaa 1/(8-1) eli ~0.143, ovat annetut v‰rit yhden stepin verran erilaisia,
	// mutta niiden v‰liin ei voi laskea yht‰‰n uutta v‰ri‰, t‰llˆin steppien m‰‰r‰ on myˆs 0.
	int calcPossibleColorStepsBetweenColors(const NFmiColor& color1, const NFmiColor& color2)
	{
		auto actualColorCubeColor1 = ToolMasterColorCube::ColorToActualCubeColor(color1);
		auto actualColorCubeColor2 = ToolMasterColorCube::ColorToActualCubeColor(color2);
		float colorChannelDifference1 = std::fabs(actualColorCubeColor1.Red() - actualColorCubeColor2.Red());
		float colorChannelDifference2 = std::fabs(actualColorCubeColor1.Green() - actualColorCubeColor2.Green());
		float colorChannelDifference3 = std::fabs(actualColorCubeColor1.Blue() - actualColorCubeColor2.Blue());
		float maxDifference = std::max(colorChannelDifference1, colorChannelDifference2);
		maxDifference = std::max(maxDifference, colorChannelDifference3);
		// Varmistetaan viel‰ ett‰ maxDifference arvo on 0 - 1
		maxDifference = std::min(maxDifference, 1.f);
		maxDifference = std::max(maxDifference, 0.f);
		// colorStepLimit saadaan kun 1 jaetaan v‰rikuution sivun koolla miinus 1
		float colorStepLimit = 1.f / (ToolMasterColorCube::ColorCubeColorChannelSize() - 1);
		// V‰hennet‰‰n lopputuloksesta 1, jotta lopputulos on v‰lill‰ -1 - 6
		int possibleColorSteps = boost::math::iround(maxDifference / colorStepLimit) - 1;
		// Arvo ei saa kuitenkaan olla negatiivinen, joten minimiss‰‰n arvoksi 0
		if(possibleColorSteps < 0)
			possibleColorSteps = 0;
		return possibleColorSteps;
	}

	bool isQueryDataRelated(NFmiInfoData::Type dataType)
	{
		if(NFmiDrawParam::IsModelRunDataType(dataType))
			return true;
		if(dataType == NFmiInfoData::kObservations || dataType == NFmiInfoData::kAnalyzeData || 
			dataType == NFmiInfoData::kEditable || dataType == NFmiInfoData::kCopyOfEdited || 
			dataType == NFmiInfoData::kEditingHelpData || dataType == NFmiInfoData::kSingleStationRadarData)
			return true;

		return false;
	}

	void addColorWithPossibleTransparency(std::vector<NFmiColor>& colors, const NFmiColor& color, bool isTransparent)
	{
		if(isTransparent)
		{
			auto transparentColor = color;
			transparentColor.Alpha(1.f);
			colors.push_back(transparentColor);
		}
		else
			colors.push_back(color);
	}

	std::vector<float> createBlendedClassLimits(const std::vector<float>& originalClassLimits, float usedStep)
	{
		std::vector<float> blendedClassLimits;
		// Loopissa menn‰‰n viimeist‰ edelliseen limittiin asti
		for(size_t originalLimitsIndex = 0; originalLimitsIndex < originalClassLimits.size() - 1; originalLimitsIndex++)
		{
			float currentBaseLimit = originalClassLimits[originalLimitsIndex];
			blendedClassLimits.push_back(currentBaseLimit);
			float nextBaseLimit = originalClassLimits[originalLimitsIndex + 1];
			for(float currentLimit = currentBaseLimit + usedStep; currentLimit < nextBaseLimit; currentLimit += usedStep)
			{
				blendedClassLimits.push_back(currentLimit);
			}
		}
		// Viimeinen limiitti lis‰t‰‰n vain suoraan per‰‰n
		blendedClassLimits.push_back(originalClassLimits.back());

		return blendedClassLimits;
	}

	NFmiColor MakeBlendedColor(const NFmiColor &color1, const NFmiColor& color2, float usedRangeWidthForBlending, float distanceFromColor1, bool clearAlpha)
	{
		float mixingRatio = distanceFromColor1 / usedRangeWidthForBlending;
		auto mixedColor = color1;
		mixedColor.Mix(color2, mixingRatio, true);
		if(clearAlpha)
			mixedColor.Alpha(0);
		return mixedColor;
	}

	// Oletus: kun tullaan t‰nne, on kaikki eri vektorit tarkastettu, niin ett‰ niiss‰ on oikeat
	// koko suhteet toisiinsa n‰hden ja ett‰ niiss‰ on tarpeeksi arvoja talletettuina.
	// Seuraavat laskut vaativat tietyt alku koot ja koko suhteet...
	std::vector<NFmiColor> createBlendedColors(const std::vector<NFmiColor>& originalColors, const std::vector<float>& originalClassLimits, const std::vector<float>& finalClassLimits, float usedStep)
	{
		std::vector<NFmiColor> finalColors;
		finalColors.push_back(originalColors.front());
		// Loopissa menn‰‰n viimeist‰ edelliseen limittiin asti
		for(size_t originalLimitsIndex = 0; originalLimitsIndex < originalClassLimits.size() - 1; originalLimitsIndex++)
		{
			bool lastLimitRangeCase = originalLimitsIndex == originalClassLimits.size() - 2;
			float limitRangeExtra = lastLimitRangeCase ? usedStep : 0;
			auto currentLimitValue = originalClassLimits[originalLimitsIndex];
			auto currentLimitValuePos = std::find(finalClassLimits.begin(), finalClassLimits.end(), currentLimitValue);
			const auto& currentLimitValueColor = originalColors[originalLimitsIndex];
			auto nextLimitValue = originalClassLimits[originalLimitsIndex + 1];
			bool rangeIsStepSize = (nextLimitValue - currentLimitValue) <= (usedStep + std::numeric_limits<float>::epsilon());
			//if(rangeIsStepSize)
			//	limitRangeExtra = 0;
			auto usedRangeWidthForBlending = (nextLimitValue - currentLimitValue + limitRangeExtra);
			auto nextLimitValuePos = std::find(currentLimitValuePos, finalClassLimits.end(), nextLimitValue);
			const auto& nextLimitValueColor = originalColors[originalLimitsIndex + 1];
			bool opaqueToTransparentChange = currentLimitValueColor.Alpha() <= 0 && nextLimitValueColor.IsFullyTransparent();
			if(currentLimitValuePos != finalClassLimits.end() && nextLimitValuePos != finalClassLimits.end())
			{
				auto iter = ++currentLimitValuePos;
				for(; iter != nextLimitValuePos && iter != finalClassLimits.end(); ++iter)
				{
					auto currentBlendedValue = *iter;
					auto distanceFromCurrentColor = currentBlendedValue - currentLimitValue;
					auto clearAlpha = opaqueToTransparentChange && lastLimitRangeCase;
					finalColors.push_back(::MakeBlendedColor(currentLimitValueColor, nextLimitValueColor, usedRangeWidthForBlending, distanceFromCurrentColor, clearAlpha));
				}
			}
			if(lastLimitRangeCase)
			{
				// Viimeisin v‰lin viimeinen v‰ri pit‰‰ blendata erikseen listaan
				auto distanceFromCurrentColor = rangeIsStepSize ? usedRangeWidthForBlending : (usedRangeWidthForBlending - usedStep);
				auto clearAlpha = opaqueToTransparentChange;
				finalColors.push_back(::MakeBlendedColor(currentLimitValueColor, nextLimitValueColor, usedRangeWidthForBlending, distanceFromCurrentColor, clearAlpha));
			}
			else
			{
				// Ollaan p‰‰sty normiv‰lin seuraavaan limittiin asti, jolloin lis‰t‰‰n sen v‰ri listaan suoraan
				finalColors.push_back(nextLimitValueColor);
			}
		}

		// Viimeinen v‰ri lopusta lis‰t‰‰n vain suoraan per‰‰n
		finalColors.push_back(originalColors.back());
		return finalColors;
	}

} // nameless namespace

// ContouringJobData implementations
// =================================

ContouringJobData::ContouringJobData() = default;

std::string ContouringJobData::makeJobDataString() const
{
	std::string jobDataString = nameAbbreviation_;
	if(::isQueryDataRelated(dataType_))
	{
		jobDataString += " {par: ";
		const auto& dataIdent = dataIdent_;
		jobDataString += dataIdent.GetParamName();
		jobDataString += " (id=";
		jobDataString += std::to_string(dataIdent.GetParamIdent());
		jobDataString += "), prod: ";
		const auto* producer = dataIdent.GetProducer();
		jobDataString += producer->GetName();
		jobDataString += " (id=";
		jobDataString += std::to_string(producer->GetIdent());
		jobDataString += ")";
		if(level_.GetIdent() != 0)
		{
			jobDataString += ", level: type=";
			const auto& level = level_;
			jobDataString += std::to_string(level.GetIdent());
			jobDataString += ", value=";
			jobDataString += std::to_string(int(level.LevelValue()));
			jobDataString += ")";
		}
		jobDataString += "}";
	}
	else if(NFmiDrawParam::IsMacroParamCase(dataType_))
	{
		jobDataString += " {calculated macroParam}";
	}

	jobDataString += ", in: ";
	if(viewIndex_ <= CtrlViewUtils::kFmiMaxMapDescTopIndex)
	{
		jobDataString += "map-view=";
		jobDataString += std::to_string(viewIndex_ + 1);
	}
	else if(viewIndex_ == CtrlViewUtils::kFmiCrossSectionView)
	{
		jobDataString += "cross-section-view=1";
	}

	jobDataString += ", row=";
	jobDataString += std::to_string(rowIndex_);

	jobDataString += ", layer=";
	jobDataString += std::to_string(layerIndex_);

	return jobDataString;
}

// ColorContouringData implementations
// ===================================

ColorContouringData::ColorContouringData() = default;

bool ColorContouringData::initialize(const ContouringJobData& contouringJobData, const boost::shared_ptr<NFmiDrawParam>& drawParam)
{
	contouringJobData_ = contouringJobData;
	if(drawParam->UseSimpleIsoLineDefinitions())
	{
		// Simple contour case
		doSimpleContourChecks_ = true;
		std::vector<float> classLimits;
		classLimits.push_back(drawParam->ColorContouringColorShadeLowValue());
		classLimits.push_back(drawParam->ColorContouringColorShadeMidValue());
		classLimits.push_back(drawParam->ColorContouringColorShadeHighValue());
		classLimits.push_back(drawParam->ColorContouringColorShadeHigh2Value());
		std::vector<NFmiColor> colors;
		::addColorWithPossibleTransparency(colors, drawParam->ColorContouringColorShadeLowValueColor(), drawParam->SimpleColorContourTransparentColor1());
		::addColorWithPossibleTransparency(colors, drawParam->ColorContouringColorShadeMidValueColor(), drawParam->SimpleColorContourTransparentColor2());
		::addColorWithPossibleTransparency(colors, drawParam->ColorContouringColorShadeHighValueColor(), drawParam->SimpleColorContourTransparentColor3());
		::addColorWithPossibleTransparency(colors, drawParam->ColorContouringColorShadeHigh2ValueColor(), drawParam->SimpleColorContourTransparentColor4());
		::addColorWithPossibleTransparency(colors, drawParam->ColorContouringColorShadeHigh3ValueColor(), drawParam->SimpleColorContourTransparentColor5());
		auto step = static_cast<float>(drawParam->ContourGab());
		auto lineStyle = drawParam->SimpleIsoLineLineStyle();
		isCorrectlyInitialized_ = initialize(classLimits, colors, step, lineStyle);
	}
	else
	{
		// Custom contour case
		doSimpleContourChecks_ = false;
		const auto& classLimits = drawParam->SpecialContourValues();
		const auto& colorIndexies = drawParam->SpecialContourColorIndexies();
		auto step = static_cast<float>(drawParam->ContourGab());
		if(!drawParam->UseContourGabWithCustomContours())
			step = 0;
		isCorrectlyInitialized_ = initialize(classLimits, colorIndexies, step);
	}
	calcFixedToolmasterContourLimits();
	makePossibleErrorLogging();
	return isCorrectlyInitialized_;
}

bool ColorContouringData::initializeForLegendCalculations(const boost::shared_ptr<NFmiDrawParam>& drawParam, int maxAllowedLimits)
{
	// Legendoja varten pit‰‰ rajoittaa laskettuja limittej‰ rankemmalla k‰dell‰
	maximumAllowedLimitCount_ = maxAllowedLimits;
	// Tehd‰‰n dummy job-data objekti
	ContouringJobData contouringJobData; 
	return initialize(contouringJobData, drawParam);
}

bool ColorContouringData::initialize(const std::vector<float>& classLimits, const std::vector<NFmiColor>& colors, float step, int separationLineStyle)
{
	originalClassLimits_ = classLimits;
	originalColors_ = colors;
	originalBlendingStep_ = step;
	separationLineStyle_ = separationLineStyle;
	return doSimpleContourInitialization();
}

bool ColorContouringData::initialize(const std::vector<float>& classLimits, const std::vector<int>& colorIndexies, float step)
{
	originalClassLimits_ = classLimits;
	originalColorIndexies_ = colorIndexies;
	originalBlendingStep_ = step;
	return doCustomContourInitialization();
}

bool ColorContouringData::doSimpleContourInitialization()
{
	missingLimitCleanUp();
	useColorBlending_ = checkIfColorBlendingIsUsed();
	if(!doFinalLimitsAndColorsChecks())
		return false;
	if(useColorBlending())
		return doSimpleContourBlendingSetups();
	else
		return doSimpleContourNonBlendingSetups();
}

bool ColorContouringData::doCustomContourInitialization()
{
	useColorBlending_ = checkIfColorBlendingIsUsed();
	// Lasketaan ensin originaali v‰rit indeksien avulla, color blendauksen yhteydess‰ 
	// transparentti v‰rit pit‰‰ hanskata erilailla kuin ilman.
	if(useColorBlending())
		originalColors_ = ::calcDefaultColorTableColorsFromCustomColorIndexies(originalColorIndexies_);
	else
		originalColors_ = ColorContouringData::calcDefaultColorTableColors(originalColorIndexies_);
	// Sen j‰lkeen voidaan k‰ytt‰‰ simple-contour juttuja sellaisenaan alustamaan systeemi
	return doSimpleContourInitialization();
}

bool ColorContouringData::doSimpleContourBlendingSetups()
{
	neededStepsBetweenLimits_ = calcNeededStepsBetweenLimits();
	colorStepsBetweenLimitColors_ = calcColorStepsBetweenLimitColors();
	if(!blendingValuesCalculatedCorrectly())
	{
		initializationErrorMessage_ = "ColorContouringData::doSimpleContourBlendingSetups calculated steps and needed blending colors are errorneous";
		return false;
	}

	if(canBlendingBeDoneWithinColorCubeColors())
		return doBlendingWithinColorCubeColors();
	else
		return doBlendingWithNewColorTable();
}

bool ColorContouringData::blendingValuesCalculatedCorrectly() const
{
	if(neededStepsBetweenLimits_.size() && neededStepsBetweenLimits_.size() == colorStepsBetweenLimitColors_.size())
		return true;
	else
		return false;
}

// Oletus: blendingValuesCalculatedCorrectly tarkastus on jo tehty aiemmin
bool ColorContouringData::canBlendingBeDoneWithinColorCubeColors()
{
	for(size_t index = 0; index < neededStepsBetweenLimits_.size(); index++)
	{
		int finalNeededStepsBetweenLimits = boost::math::iround(neededStepsBetweenLimits_[index]);
		auto finalColorStepsBetweenLimitColors = colorStepsBetweenLimitColors_[index];
		if(finalColorStepsBetweenLimitColors >= 1)
		{
			if(finalNeededStepsBetweenLimits > finalColorStepsBetweenLimitColors)
				return false;
		}
	}
	return true;
}

// Oletus: blendingValuesCalculatedCorrectly tarkastus on jo tehty aiemmin
bool ColorContouringData::doBlendingWithinColorCubeColors()
{
	finalBlendingStep_ = originalBlendingStep_;
	finalClassLimits_ = ::createBlendedClassLimits(originalClassLimits_, originalBlendingStep_);
	finalColors_ = ::createBlendedColors(originalColors_, originalClassLimits_, finalClassLimits_, finalBlendingStep_);
	finalColorIndexies_ = ColorContouringData::calcDefaultColorTableIndexies(finalColors_);
	return true;
}

bool ColorContouringData::doBlendingWithNewColorTable()
{
	// Kun t‰nne asti on tultu, pit‰‰ olla v‰hint‰in 2 limit:ia
	auto limitRange = originalClassLimits_.back() - originalClassLimits_.front();
	auto possibleSparsedStep = calcSuitableBlendingStep(originalBlendingStep_, limitRange, float(maximumAllowedLimitCount_));
	if(possibleSparsedStep == kFloatMissing)
	{
		initializationErrorMessage_ = "ColorContouringData: Unable to calculate sparsed contour step for unknown reasons";
		return false;
	}

	finalBlendingStep_ = possibleSparsedStep;
	finalClassLimits_ = ::createBlendedClassLimits(originalClassLimits_, possibleSparsedStep);
	finalColors_ = ::createBlendedColors(originalColors_, originalClassLimits_, finalClassLimits_, finalBlendingStep_);
	if(finalColors_.size() > maximumAllowedLimitCount_)
	{
		initializationErrorMessage_ = "ColorContouringData: sparsed contour step still produced contours over maximum limit for unknown reasons";
		return false;
	}

	if(originalBlendingStep_ != possibleSparsedStep)
	{
		std::string msg = "ColorContouringData::doBlendingWithNewColorTable - used contouring step changed from ";
		msg += std::to_string(originalBlendingStep_);
		msg += " to ";
		msg += std::to_string(possibleSparsedStep);
		msg += " due that original step would produce way too many blended colors (steps)";
		CatLog::logMessage(msg, CatLog::Severity::Debug, CatLog::Category::Visualization);
	}

	finalColorIndexies_ = ::calcNewColorTableIndexies(finalColors_);
	useDefaultColorTable_ = false;

	return true;
}

std::vector<float> ColorContouringData::calcNeededStepsBetweenLimits()
{
	// Laske limit-v‰lien tarve stepeille, lasketaan se reaalilukuna ainakin aluksi
	std::vector<float> neededStepsBetweenLimits;
	for(size_t index = 0; index < originalClassLimits_.size() - 1; index++)
	{
		auto limitDifference = originalClassLimits_[index + 1] - originalClassLimits_[index];
		// Huom! jakolaskusta pit‰‰ viel‰ v‰hent‰‰ 1:n, jotta saadaan v‰liaskelten oikea lukum‰‰r‰
		neededStepsBetweenLimits.push_back((limitDifference / originalBlendingStep_) - 1.f);
	}
	return neededStepsBetweenLimits;
}

std::vector<int> ColorContouringData::calcColorStepsBetweenLimitColors()
{
	// Laske eri limitv‰lien v‰rien v‰lille laskettavat steppim‰‰r‰t
	// Koska t‰nne tultaessa on selv‰‰ ett‰ tehd‰‰n blendausta, on color:ien m‰‰r‰
	// sama kuin limittien m‰‰r‰ ja tarkastelu alkaa 1. v‰rist‰ alkaen.
	std::vector<int> colorStepsBetweenLimitColors;
	for(size_t index = 0; index < originalColors_.size() - 1; index++)
	{
		colorStepsBetweenLimitColors.push_back(::calcPossibleColorStepsBetweenColors(originalColors_[index], originalColors_[index + 1]));
	}
	return colorStepsBetweenLimitColors;
}

bool ColorContouringData::doSimpleContourNonBlendingSetups()
{
	finalClassLimits_ = originalClassLimits_;
	finalColors_ = originalColors_;
	finalBlendingStep_ = originalBlendingStep_;
	finalColorIndexies_ = ColorContouringData::calcDefaultColorTableIndexies(finalColors_);
	return true;
}

// T‰t‰ kutsutaan vasta sen j‰lkeen kun missingLimitCleanUp -metodia on kutsuttu.
bool ColorContouringData::checkIfColorBlendingIsUsed() const
{
	// Jos steppi on 0 tai pienempi, ei tehd‰ blendausta ollenkaan
	if(originalBlendingStep_ <= 0)
		return false;

	// Jos luokka rajoja on alle 2 kpl, ei tehd‰ blendausta ollenkaan
	if(originalClassLimits_.size() < 2)
		return false;

	// Jos steppi on ~ >= kuin kaikki arvov‰lit, ei tehd‰ blendausta ollenkaan
	for(size_t index = 0; index < originalClassLimits_.size() - 1; index++)
	{
		auto limitDifference = originalClassLimits_[index + 1] - originalClassLimits_[index];
		if(limitDifference > originalBlendingStep_ && !CtrlViewUtils::IsEqualEnough(limitDifference, originalBlendingStep_, std::numeric_limits<float>::epsilon()))
			return true;
	}

	// Steppi oli isompi kuin mik‰‰n luokkarajav‰leist‰, ei voi tehd‰ blendausta
	return false;
}

// Tutkitaan onko rajoja ja v‰reja tarpeeksi. 
// Niiden m‰‰r‰ riippuu siit‰ k‰ytet‰‰nkˆ color blendausta vai ei.
// Palauttaa false, jos jokin on pieless‰ ja raportoi siit‰.
// Kutsutaan missingLimitCleanUp- ja checkIfColorBlendingIsUsed metodi kutsujen j‰lkeen. 
bool ColorContouringData::doFinalLimitsAndColorsChecks()
{
	auto limitSize = originalClassLimits_.size();
	if(limitSize == 0)
	{
		initializationErrorMessage_ = "ColorContouringData: original class limit size was 0, nothing to be done";
		return false;
	}

	auto colorSize = originalColors_.size();
	if(colorSize <= 1)
	{
		initializationErrorMessage_ = "ColorContouringData: original colors size was 1 or under, nothing to be done";
		return false;
	}

	if(useColorBlending_)
	{
		// color blending tapauksessa tarvitaan sama m‰‰r‰ limit ja color:eja
		auto minSize = std::min(limitSize, colorSize);
		originalColors_.resize(minSize);
		originalClassLimits_.resize(minSize);
	}
	else
	{
		if(limitSize != colorSize - 1)
		{
			if(limitSize < colorSize)
				originalColors_.resize(limitSize + 1);
			else if(limitSize >= colorSize)
				originalClassLimits_.resize(colorSize - 1);
		}
	}

	return true;
}

// T‰ss‰ poistetaan kaikki missing limit arvot (originalClassLimits_ vektorista) ja 
// niihin liittyv‰t v‰rit (originalColors_ vektorista kohdasta index+1).
// Poistetaan myˆs limit+v‰ri, jos kahden per‰kk‰isen limitin arvo on sama (tyhj‰ v‰li, jolloin j‰lkimm‰inen raja+v‰ri poistuu).
// Oletus: Kun t‰t‰ kutsutaan custom-contour alustuksesta, pit‰‰ originalColors_ olla alustettuna jo.
void ColorContouringData::missingLimitCleanUp()
{
	// Tehd‰‰n n‰m‰ tarkastelut vain simple tapaukselle, koska kukaan ei annna custom osiossa rajoiksi puuttuvia arvoja
	if(!doSimpleContourChecks_)
		return;

	auto limitsSize = originalClassLimits_.size();
	auto colorsSize = originalColors_.size();
	if(limitsSize && colorsSize)
	{
		std::vector<float> tmpLimits;
		std::vector<NFmiColor> tmpColors;
		// 1. v‰ri laitetaan aina mukaan, koska siihen ei liity rajaa, joka voisi olla missing arvoinen
		tmpColors.push_back(originalColors_.front());

		for(size_t index = 0; index < limitsSize && index < colorsSize - 1; index++)
		{
			// 2. Siivotaan pois missing limitit ja niihin liittyv‰t v‰rit.
			if(originalClassLimits_[index] != kFloatMissing)
			{
				// 3. Siivotaan myˆs pois sellaiset rajat (ja v‰rit), miss‰ per‰kk‰iset limitit ovat samoja (et‰isyys on 0)
				if(index > 0 && originalClassLimits_[index] == originalClassLimits_[index - 1])
					continue;

				tmpLimits.push_back(originalClassLimits_[index]);
				tmpColors.push_back(originalColors_[index + 1]);
			}
		}

		originalClassLimits_ = tmpLimits;
		originalColors_ = tmpColors;
	}
}

bool ColorContouringData::createNewToolMasterColorTable(int colorTableIndex)
{
	if(useDefaultColorTable_)
		return false;

	int createdColorTableSize = int(finalColorIndexies_.size()) + ToolMasterColorCube::SpecialColorCountInColorTableStart();
	XuColorTableCreate(colorTableIndex, createdColorTableSize, XuLOOKUP, XuRGB, 255);
	XuColorTableActivate(colorTableIndex);
	XuClasses(finalToolmasterFixedClassLimits_.data(), int(finalToolmasterFixedClassLimits_.size()));
	XuShadingColorIndices(finalColorIndexies_.data(), int(finalColorIndexies_.size()));

	float colorRGB[3], hatchArrayDummy[5];

	ToolMasterColorCube::SetupSpecialColorsForActiveColorTable();

	int index = ToolMasterColorCube::SpecialColorCountInColorTableStart(); // aletaan rakentaan v‰ri taulukkoa hollow v‰rin j‰lkeen
	for(size_t i = 0; i < finalColors_.size(); i++)
	{
		const auto& currentColor = finalColors_[i];
		if(ToolMasterColorCube::IsColorFullyOpaque(currentColor))
		{
			colorRGB[0] = static_cast<float>(int(255 * currentColor.GetRed()));
			colorRGB[1] = static_cast<float>(int(255 * currentColor.GetGreen()));
			colorRGB[2] = static_cast<float>(int(255 * currentColor.GetBlue()));
			XuColor(XuCOLOR, index++, colorRGB, hatchArrayDummy);
		}
	}

	return true;
}

void ColorContouringData::makePossibleErrorLogging() const
{
	ColorContouringData::makePossibleErrorLogging(initializationErrorMessage_, isCorrectlyInitialized_, contouringJobData_);
}

void ColorContouringData::makePossibleErrorLogging(const std::string& initializationErrorMessage, bool wasCorrectlyInitialized, const ContouringJobData& contouringJobData)
{
	if(!wasCorrectlyInitialized)
	{
		std::string logMessage = initializationErrorMessage;
		logMessage += " (with ";
		logMessage += contouringJobData.makeJobDataString();
		logMessage += ")";
		CatLog::logMessage(logMessage, CatLog::Severity::Error, CatLog::Category::Visualization, true);
	}
}

// Jos ei joudu blendaamaan v‰rej‰, k‰ytet‰‰n custom v‰ri-indeksi konversiota sellaisenaan, 
// kun lasketaan k‰ytettyj‰ v‰rej‰.
std::vector<NFmiColor> ColorContouringData::calcDefaultColorTableColors(const std::vector<int>& colorIndexies)
{
	std::vector<NFmiColor> colors;
	for(auto colorIndex : colorIndexies)
	{
		colors.push_back(ToolMasterColorCube::ColorIndexToRgb(colorIndex));
	}
	return colors;
}

std::vector<int> ColorContouringData::calcDefaultColorTableIndexies(const std::vector<NFmiColor>& colors)
{
	std::vector<int> colorIndexies;
	for(const auto& color : colors)
	{
		colorIndexies.push_back(ToolMasterColorCube::RgbToColorIndex(color));
	}
	return colorIndexies;
}

const NFmiColor& ColorContouringData::getValueRangeColor(float value1, float value2) const
{
	if(finalClassLimits_.empty())
		return g_DefaultValueRangeColor;

	auto index1 = ::getClosestValueIndex(value1, finalClassLimits_);
	auto index2 = ::getClosestValueIndex(value2, finalClassLimits_);
	if(index1 == 0 && index2 == 0)
		return finalColors_.front();
	auto lastIndexValue = finalClassLimits_.size() - 1;
	if(index1 == lastIndexValue && index2 == lastIndexValue)
		return finalColors_.back();
	else
		return finalColors_[index2];
}

// T‰m‰n gToolMasterContourLimitChangeValue- globaalin muuttujan k‰yttˆ on j‰‰nyt minulle nyt hieman h‰m‰r‰n peittoon,
// Mutta luulen ett‰ se liittyy enemm‰nkin diskreettien parametrien piirtoon. 
// Se kuitenkin pienent‰‰ rajoja hyvin pienell‰ luvulla ,jolloin tarkoitu oli saada aikaan efekti ett‰ isoviivat tai contourit 
// kiersiv‰t tietyn arvoiset alueet paremmin. T‰st‰ rajojen pikkuriikkisest‰ muutoksesta ei ole haittaa kun operoidaan normaalin kokoisien
// luku arvojen kanssa esim. n. 0.00001 - 10000000000000.
// MUTTA kun arvo joukko on tarpeeksi pient‰ eli ollaan tarpeeksi l‰hell‰ gToolMasterContourLimitChangeValue, alkaa vaikutus h‰iritsem‰‰n.
// Sen takia pit‰‰ tarkastella onko annettu raja tarpeeksi l‰hell‰ 'epsilonia' ja jos on, ei rajaa siirret‰ ollenkaan.
float ColorContouringData::GetToolMasterContourLimitChangeValue(float theValue)
{
	if(::fabs(theValue / gToolMasterContourLimitChangeValue) < 1000.f)
		return theValue; // jos theValue:n ja 'epsilonin' kokoero on alle 1000x, ei rajaa siirret‰ en‰‰
	else
		return theValue - gToolMasterContourLimitChangeValue;
}

void ColorContouringData::calcFixedToolmasterContourLimits()
{
	finalToolmasterFixedClassLimits_ = finalClassLimits_;
	std::transform(finalToolmasterFixedClassLimits_.begin(), finalToolmasterFixedClassLimits_.end(), finalToolmasterFixedClassLimits_.begin(),
		[&](auto value) {return ColorContouringData::GetToolMasterContourLimitChangeValue(value); });
}
