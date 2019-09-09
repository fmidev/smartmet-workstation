#include "stdafx.h"
#include "NFmiColorContourLegendValues.h"
#include "NFmiDrawParam.h"
#include "NFmiFastQueryInfo.h"
#include "ToolMasterColorCube.h"
#include "NFmiValueString.h"

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
    if(drawParam && !drawParam->ShowColorLegend())
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

void NFmiColorContourLegendValues::FillSimpleColorContourValues(const boost::shared_ptr<NFmiDrawParam>& drawParam)
{

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
    if(classLimitValues_.empty() || classColors_.empty())
        return;

    if(classLimitValues_.size() > classColors_.size() - 1)
        classLimitValues_.resize(classColors_.size() - 1);
    else if(classLimitValues_.size() < classColors_.size() - 1)
        classColors_.resize(classLimitValues_.size() + 1);
    FillClassLimitTextsVector();
    useLegend_ = true;
}

void NFmiColorContourLegendValues::FillClassLimitTextsVector()
{
    for(auto classLimitValue : classLimitValues_)
        classLimitTexts_.push_back(std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(classLimitValue, 2)));
}
