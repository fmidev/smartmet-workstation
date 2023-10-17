#include "stdafx.h"
#include "NFmiColorContourLegendValues.h"
#include "NFmiDrawParam.h"
#include "NFmiFastQueryInfo.h"
#include "ToolMasterColorCube.h"
#include "NFmiValueString.h"
#include "CtrlViewFunctions.h"
#include "ColorContouringData.h"
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
                FillColorContourValues(drawParam);
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

void NFmiColorContourLegendValues::FillColorContourValues(const boost::shared_ptr<NFmiDrawParam>& drawParam)
{
    ColorContouringData colorContouringData;
    // Testeissä 40 on osoittautunut liian isoksi max-luvuksi
    auto maxAllowedLimitCount = 35; 
    colorContouringData.initializeForLegendCalculations(drawParam, maxAllowedLimitCount);
    if(colorContouringData.isCorrectlyInitialized())
    {
        classLimitValues_ = colorContouringData.finalClassLimits();
        classColors_ = colorContouringData.finalColors();
        FinalizeFillingValues(drawParam->Alpha());
    }
}

void NFmiColorContourLegendValues::FinalizeFillingValues(float layerAlphaProcent)
{
    if(hasEmptyValues())
        return;

    // Rajoja pitää olla yksi vähemmän kuin värejä
    if(classLimitValues_.size() > classColors_.size() - 1)
        classLimitValues_.resize(classColors_.size() - 1);
    else if(classLimitValues_.size() < classColors_.size() - 1)
        classColors_.resize(classLimitValues_.size() + 1);

    RemoveTransparentColorsFromEdges();
    RemoveConsecutiveTransparentColorsFromMiddle();
    SetLayerAlphaForColors(layerAlphaProcent);
    if(hasEmptyValues())
        return;

    FillClassLimitTextsVector();
    useLegend_ = true;
}

// Jos layerilla on alpha kerroin, laitetaan se tässä joka värille käyttöön
// jotta legendaan piirrossa voidaan 'haalentaa' väreja alpha kertoimen avulla.
void NFmiColorContourLegendValues::SetLayerAlphaForColors(float layerAlphaProcent)
{
    if(layerAlphaProcent < 100)
    {
        float usedAlpha = 1.f - (layerAlphaProcent / 100.f);
        for(auto& color : classColors_)
        {
            // Läpinäkyvään väriin ei saa asettaa kuitenkaan layer-alphaa
            if(!ToolMasterColorCube::IsColorFullyTransparent(color))
            {
                color.Alpha(usedAlpha);
            }
        }
    }
}

bool NFmiColorContourLegendValues::hasEmptyValues()
{
    return classLimitValues_.empty() || classColors_.empty();
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
        if(!ToolMasterColorCube::IsColorFullyOpaque(color))
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
        if(!ToolMasterColorCube::IsColorFullyOpaque(color))
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

// Oletus: alusta on jo poistettu peräkkäiset transparentit värit ja niiden rajat.
void NFmiColorContourLegendValues::RemoveConsecutiveTransparentColorsFromMiddle()
{
    // Käydään värejä läpi 2. alkaen ja etsitään peräkkäisiä transparentteja värejä.
    // Poistetaan tällöin aina jälkimmäiset värit ja rajat, kunnes tulee opaque väri tai loppu
    // (tämä algoritmi poistaa siten myös lopusta peräkkäiset värit)
    for(size_t colorIndex = 1; colorIndex < classColors_.size() - 1; )
    {
        if(!ToolMasterColorCube::IsColorFullyOpaque(classColors_[colorIndex]) && !ToolMasterColorCube::IsColorFullyOpaque(classColors_[colorIndex + 1]))
        {
            // jos esim. 2. ja 3. väri ovat läpinäkyviä, halutaan tällöin poistaa jälkimmäinen
            // eli 3. väri ja siihen kuuluu 2. raja joka poistetaan myös
            classColors_.erase(classColors_.begin() + colorIndex + 1);
            classLimitValues_.erase(classLimitValues_.begin() + colorIndex);
        }
        else
            colorIndex++;
    }
}

void NFmiColorContourLegendValues::FillClassLimitTextsVector()
{
    for(auto classLimitValue : classLimitValues_)
        classLimitTexts_.push_back(std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(classLimitValue, 2)));
}
