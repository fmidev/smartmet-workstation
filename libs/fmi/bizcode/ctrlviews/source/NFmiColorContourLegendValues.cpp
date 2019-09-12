#include "stdafx.h"
#include "NFmiColorContourLegendValues.h"
#include "NFmiDrawParam.h"
#include "NFmiFastQueryInfo.h"
#include "ToolMasterColorCube.h"
#include "NFmiValueString.h"
#include "CtrlViewFunctions.h"
#include "boost/range/adaptor/reversed.hpp"

namespace
{
    bool acceptableDataTypeForNormalLegendDraw(NFmiInfoData::Type dataType)
    {
        if(dataType == NFmiInfoData::kNoDataType || dataType == NFmiInfoData::kSatelData || dataType == NFmiInfoData::kConceptualModelData)
            return false;
        if(dataType == NFmiInfoData::kCapData || dataType == NFmiInfoData::kWmsData)
            return false;
        if(dataType == NFmiInfoData::kAnyData || dataType >= NFmiInfoData::kSoundingParameterData)
            return false;

        return true;
    }
}

NFmiColorContourLegendValues::NFmiColorContourLegendValues(const boost::shared_ptr<NFmiDrawParam>& drawParam, boost::shared_ptr<NFmiFastQueryInfo>& info)
{
    init(drawParam, info);
}

void NFmiColorContourLegendValues::init(const boost::shared_ptr<NFmiDrawParam>& drawParam, boost::shared_ptr<NFmiFastQueryInfo>& info)
{
    clear();
    if(drawParam && drawParam->ShowColorLegend())
    {
        if(::acceptableDataTypeForNormalLegendDraw(drawParam->DataType()))
        {
            isStationData_ = isStationDataType(drawParam, info);
            auto viewType = drawParam->GetViewType(isStationData_);
            if(NFmiDrawParam::IsColorContourType(viewType))
            {
                name_ = drawParam->ParameterAbbreviation();

                if(drawParam->UseSimpleIsoLineDefinitions())
                    FillSimpleColorContourValues(drawParam);
                else
                    FillCustomColorContourValues(drawParam);
            }
        }
    }
}

void NFmiColorContourLegendValues::clear()
{
    useLegend_ = false;
    isStationData_ = false;
    classLimitValues_.clear();
    classLimitTexts_.clear();
    classColors_.clear();
    name_.clear();
}

bool NFmiColorContourLegendValues::isStationDataType(const boost::shared_ptr<NFmiDrawParam>& drawParam, boost::shared_ptr<NFmiFastQueryInfo>& info)
{
    // MacroParamit ovat hiladatoja
    if(!drawParam->IsMacroParamCase(true))
    {
        if(info)
            return !info->IsGrid();
    }

    // Jos info on nullptr, oletus on että kyse on hiladatasta
    return false;
}

static int CalcActualLimitCount(float bottomLimit, float topLimit, float contourStep)
{
    return boost::math::iround(((topLimit - bottomLimit) / contourStep) + 1.f);
}

static bool CheckOriginalLimits(const std::vector<float>& originalClasses)
{
    if(originalClasses.size() < 4)
        return false;
    float currentValue = originalClasses.front();
    for(auto index = 1; index < originalClasses.size(); index++)
    {
        if(currentValue > originalClasses[index])
            return false;
    }
    return true;
}

const int g_MinimumLimitCountRequired = 10;
const int g_MaximumLimitCountAllowed = 40;

// Simple color contour juttu on tehty aikoinaan pieleen. Siinä on 4 rajaa ja niille 4 väriä, vaikka värejä pitäisi
// olla yksi enemmän kuin rajoja. Seurauksena värit ja rajat alkavat toimimaan vasta kun käytetään tarpeeksi
// pientä steppiä, jolloin tehdyt laskut alkavat menemään järkevästi kohdilleen. 
// Siksi pitää tulla tarpeeksi laskettuja rajoja (> 10 ?), ennen kuin tämä hyväksytään visualisoitavaksi legendaksi. 
// Jos taas steppi on niin pieni että tulee n. >30 rajaa, pitää alkaa harventamaan sopivasti tehtyä legendaa.
void NFmiColorContourLegendValues::FillSimpleColorContourValues(const boost::shared_ptr<NFmiDrawParam>& drawParam)
{
    std::vector<float> originalClasses{ drawParam->ColorContouringColorShadeLowValue(),
    drawParam->ColorContouringColorShadeMidValue(),
    drawParam->ColorContouringColorShadeHighValue(),
    drawParam->ColorContouringColorShadeHigh2Value() };
    if(::CheckOriginalLimits(originalClasses))
    {
        std::vector<NFmiColor> originalColors{ drawParam->ColorContouringColorShadeLowValueColor(),
        drawParam->ColorContouringColorShadeMidValueColor(),
        drawParam->ColorContouringColorShadeHighValueColor(),
        drawParam->ColorContouringColorShadeHigh2ValueColor() };

        auto contourStep = static_cast<float>(drawParam->ContourGab());
        if(contourStep > 0) // pitää olla positiivinen luku
        {
            auto actualLimitCount = ::CalcActualLimitCount(originalClasses.front(), originalClasses.back(), contourStep);
            if(actualLimitCount >= g_MinimumLimitCountRequired)
            {
                MakeFinalSimpleColorContourValues(originalClasses, originalColors, contourStep, actualLimitCount);
            }
        }
    }
}

const float g_UsedEps = 0.000001f;

static std::pair<float, int> CalcUsedContourStep(float bottomLimit, float topLimit, float originalContourStep)
{
    float usedContourStep = originalContourStep;
    int usedLimitCount = 0;
    for(int counter = 0; counter < 2000; counter++)
    {
        usedLimitCount = ::CalcActualLimitCount(bottomLimit, topLimit, usedContourStep);
        if(usedLimitCount < g_MinimumLimitCountRequired)
            break; // ei saatu sopivaa määrää askelia millään, lopetetaan
        if(usedLimitCount <= g_MaximumLimitCountAllowed)
        {
            // uudella stepillä pitää tulla oikean topValue
            auto newTopLimit = bottomLimit + ((usedLimitCount -1) * usedContourStep);
            if(CtrlViewUtils::IsEqualEnough(topLimit, newTopLimit, g_UsedEps))
            {
                return std::make_pair(usedContourStep, usedLimitCount);
            }
        }
        usedContourStep += originalContourStep;
    }

    return std::make_pair(-1.f, 0); // negatiivinen steppi kertoo että homma ei onnistu
}

static std::vector<float> MakeSimpleLimitVector(float bottomLimit, float contourStep, int limitCount)
{
    std::vector<float> limits;
    for(int counter = 0; counter < limitCount; counter++)
    {
        limits.push_back(bottomLimit + (counter * contourStep));
    }
    return limits;
}

static float CheckForEqualEnoughLimits(float classLimit, const std::vector<float>& originalClasses)
{
    for(size_t index = 0; index < originalClasses.size(); index++)
    {
        if(CtrlViewUtils::IsEqualEnough(classLimit, originalClasses[index], g_UsedEps))
        {
            return static_cast<float>(index);
        }
    }
    return -1;
}

// Etsi classLimit:iä vastaava indeksi-kerroin eli annetun luvun sijainti originalClasses -vectorissa.
// Jos originalClasses:in sisältö on esim. 10,15,20,25, saadaan seuraavia arvoja eri classLimit arvoilla:
// 9.9 => -1 (0 ei ole arvoalueella 10-25 ja negatiivinen paluuarvo on virheen merkki)
// 10 => 0.0
// 12.5 => 0.5
// 15 => 1.0
// 20 => 2.0
// 21 => 2.2
// 24 => 2.8
// 25 => 3.0
// 25.1 => -1
// Eli kokonaisosio kertoo indeksin vector:issa siihen väliin, mihin annettu luku osuu ja 
// desimaali osio keroo suhteellisen kertoimen sijainnista kahden ympäröivän luvun välissä.
static float CalcIndexFactor(float classLimit, const std::vector<float>& originalClasses)
{
    auto matchingIndex = ::CheckForEqualEnoughLimits(classLimit, originalClasses);
    if(matchingIndex >= 0)
        return matchingIndex;
    if(classLimit < originalClasses.front() || classLimit > originalClasses.back())
        return -1;
    for(size_t index = 0; index < originalClasses.size() - 1; index++)
    {
        auto lowLimit = originalClasses[index];
        auto highLimit = originalClasses[index+1];
        if(lowLimit <= classLimit && classLimit <= highLimit)
        {
            return static_cast<float>(index) + (classLimit - lowLimit) / (highLimit - lowLimit);
        }
    }
    return -1; // ei saisi oikeastaan mennä tähän (koodi tai data virhe), mutta pakko palauttaa vain virhe
}

static bool IsIndexFactorInteger(float indexFactor)
{
    return std::floor(indexFactor) == indexFactor;
}

static NFmiColor MakeSimpleColorBlend(float indexFactor, const std::vector<NFmiColor>& originalColors)
{
    NFmiColor usedColor;
    if(::IsIndexFactorInteger(indexFactor))
        usedColor = originalColors[boost::math::iround(indexFactor)];
    else
    {
        auto lowIndexReal = std::floor(indexFactor);
        auto fractionPart = indexFactor - lowIndexReal;
        auto lowIndex = static_cast<int>(lowIndexReal);
        usedColor = originalColors[lowIndex];
        usedColor.Mix(originalColors[lowIndex + 1], fractionPart);
    }
    usedColor.Alpha(1.f); // has to fix the default 0-alpha away, otherwise there will be problems later
    return usedColor;
}

static std::vector<NFmiColor> MakeSimpleColorVector(const std::vector<float>& usedClassLimitValues, const std::vector<float>& originalClasses, const std::vector<NFmiColor>& originalColors)
{
    std::vector<NFmiColor> colors;
    for(auto classLimit : usedClassLimitValues)
    {
        auto indexFactor = ::CalcIndexFactor(classLimit, originalClasses);
        if(indexFactor < 0)
            return std::vector<NFmiColor>(); // Jos tulee yksikin virhe, palautetaan tyhjä vector, joka estää legendan käytön
        colors.push_back(::MakeSimpleColorBlend(indexFactor, originalColors));
    }
    return colors;
}

void NFmiColorContourLegendValues::MakeFinalSimpleColorContourValues(const std::vector<float>& originalClasses, const std::vector<NFmiColor>& originalColors, float contourStep, int actualLimitCount)
{
    auto contourStepInfo = ::CalcUsedContourStep(originalClasses.front(), originalClasses.back(), contourStep);
    if(contourStepInfo.first > 0)
    {
        isSimpleContour_ = true;
        classLimitValues_ = ::MakeSimpleLimitVector(originalClasses.front(), contourStepInfo.first, contourStepInfo.second);
        classColors_ = ::MakeSimpleColorVector(classLimitValues_, originalClasses, originalColors);
        FinalizeFillingValues();
    }
}

void NFmiColorContourLegendValues::FillCustomColorContourValues(const boost::shared_ptr<NFmiDrawParam>& drawParam)
{
    classLimitValues_ = drawParam->SpecialContourValues();
    auto colorIndices = drawParam->SpecialContourColorIndexies();
    for(auto colorIndex : colorIndices)
        classColors_.push_back(ToolMasterColorCube::ColorIndexToRgb(colorIndex));

    FinalizeFillingValues();
}

void NFmiColorContourLegendValues::FinalizeFillingValues()
{
    if(hasEmptyValues())
        return;

    if(isSimpleContour_)
    {
        // Pitää olla sama määrä rajoja ja värejä
        if(classLimitValues_.size() > classColors_.size())
            classLimitValues_.resize(classColors_.size());
        else if(classLimitValues_.size() < classColors_.size())
            classColors_.resize(classLimitValues_.size());
    }
    else
    {
        // Rajoja pitää olla yksi vähemmän kuin värejä
        if(classLimitValues_.size() > classColors_.size() - 1)
            classLimitValues_.resize(classColors_.size() - 1);
        else if(classLimitValues_.size() < classColors_.size() - 1)
            classColors_.resize(classLimitValues_.size() + 1);
    }
    RemoveTransparentColorsFromEdges();
    if(hasEmptyValues())
        return;

    FillClassLimitTextsVector();
    useLegend_ = true;
}

bool NFmiColorContourLegendValues::hasEmptyValues()
{
    return classLimitValues_.empty() || classColors_.empty();
}

static bool IsTransparent(const NFmiColor& color)
{
    return color.Alpha() == 0;
}

template<typename Container>
static void RemoveItemsFromStart(Container& container, size_t removedCount)
{
    if(removedCount)
    {
        if(removedCount == container.size())
            container.clear();
        else
            container = Container(container.begin() + removedCount, container.end());
    }
}

void NFmiColorContourLegendValues::RemoveTransparentColorsFromEdges()
{
    // Ensin tutkitaan kuinka monta peräkkäistä läpinäkyvää väriä on alussa
    size_t counter = 0;
    for(const auto& color : classColors_)
    {
        if(::IsTransparent(color))
            counter++;
        else
            break;
    }
    if(counter > 1)
    {
        // Jos läpinäkyviä on enemmän kuin 1, poistetaan värejä ja rajoja n-1 kpl
        auto actuallyRemovedCount = counter - 1;
        ::RemoveItemsFromStart(classColors_, actuallyRemovedCount);
        ::RemoveItemsFromStart(classLimitValues_, actuallyRemovedCount);
    }

    // Sitten tutkitaan kuinka monta peräkkäistä läpinäkyvää väriä on lopussa
    counter = 0;
    for(const auto& color : boost::adaptors::reverse(classColors_))
    {
        if(::IsTransparent(color))
            counter++;
        else
            break;
    }
    if(counter > 1)
    {
        // Jos läpinäkyviä on enemmän kuin 1, poistetaan värejä ja rajoja n-1 kpl
        auto actuallyRemovedCount = counter - 1;
        classColors_.resize(classColors_.size() - actuallyRemovedCount);
        classLimitValues_.resize(classLimitValues_.size() - actuallyRemovedCount);
    }
}

void NFmiColorContourLegendValues::FillClassLimitTextsVector()
{
    for(auto classLimitValue : classLimitValues_)
        classLimitTexts_.push_back(std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(classLimitValue, 2)));
}
