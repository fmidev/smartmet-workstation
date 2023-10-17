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

    // Jos info on nullptr, oletus on ett� kyse on hiladatasta
    return false;
}

void NFmiColorContourLegendValues::FillColorContourValues(const boost::shared_ptr<NFmiDrawParam>& drawParam)
{
    ColorContouringData colorContouringData;
    // Testeiss� 40 on osoittautunut liian isoksi max-luvuksi
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

    // Rajoja pit�� olla yksi v�hemm�n kuin v�rej�
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

// Jos layerilla on alpha kerroin, laitetaan se t�ss� joka v�rille k�ytt��n
// jotta legendaan piirrossa voidaan 'haalentaa' v�reja alpha kertoimen avulla.
void NFmiColorContourLegendValues::SetLayerAlphaForColors(float layerAlphaProcent)
{
    if(layerAlphaProcent < 100)
    {
        float usedAlpha = 1.f - (layerAlphaProcent / 100.f);
        for(auto& color : classColors_)
        {
            // L�pin�kyv��n v�riin ei saa asettaa kuitenkaan layer-alphaa
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
    // Ensin tutkitaan kuinka monta per�kk�ist� l�pin�kyv�� v�ri� on alussa
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
        // Jos l�pin�kyvi� on enemm�n kuin 1, poistetaan v�rej� ja rajoja n-1 kpl
        auto actuallyRemovedCount = counter - 1;
        ::RemoveItemsFromStart(classColors_, actuallyRemovedCount);
        ::RemoveItemsFromStart(classLimitValues_, actuallyRemovedCount);
    }

    // Sitten tutkitaan kuinka monta per�kk�ist� l�pin�kyv�� v�ri� on lopussa
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
        // Jos l�pin�kyvi� on enemm�n kuin 1, poistetaan v�rej� ja rajoja n-1 kpl
        auto actuallyRemovedCount = counter - 1;
        classColors_.resize(classColors_.size() - actuallyRemovedCount);
        classLimitValues_.resize(classLimitValues_.size() - actuallyRemovedCount);
    }
}

// Oletus: alusta on jo poistettu per�kk�iset transparentit v�rit ja niiden rajat.
void NFmiColorContourLegendValues::RemoveConsecutiveTransparentColorsFromMiddle()
{
    // K�yd��n v�rej� l�pi 2. alkaen ja etsit��n per�kk�isi� transparentteja v�rej�.
    // Poistetaan t�ll�in aina j�lkimm�iset v�rit ja rajat, kunnes tulee opaque v�ri tai loppu
    // (t�m� algoritmi poistaa siten my�s lopusta per�kk�iset v�rit)
    for(size_t colorIndex = 1; colorIndex < classColors_.size() - 1; )
    {
        if(!ToolMasterColorCube::IsColorFullyOpaque(classColors_[colorIndex]) && !ToolMasterColorCube::IsColorFullyOpaque(classColors_[colorIndex + 1]))
        {
            // jos esim. 2. ja 3. v�ri ovat l�pin�kyvi�, halutaan t�ll�in poistaa j�lkimm�inen
            // eli 3. v�ri ja siihen kuuluu 2. raja joka poistetaan my�s
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
