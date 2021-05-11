#include "ForcedIsolineLogging.h"
#include "NFmiDrawParam.h"
#include "NFmiIsoLineData.h"
#include "NFmiFastQueryInfo.h"
#include "CtrlViewFunctions.h"
#include "catlog/catlog.h"

namespace
{
    std::string GetBoolString(bool value)
    {
        return value ? "true" : "false";
    }

    std::string GetSimpleIsolineColorString(boost::shared_ptr<NFmiDrawParam>& theDrawParam)
    {
        std::string colorStr;
        if(theDrawParam->UseSingleColorsWithSimpleIsoLines())
        {
            colorStr += ", lineColor=";
            colorStr += CtrlViewUtils::Color2HtmlColorStr(theDrawParam->IsolineColor());
        }
        else
        {
            colorStr += ", lowLimit=";
            colorStr += std::to_string(theDrawParam->SimpleIsoLineColorShadeLowValue());
            colorStr += ", lowColor=";
            colorStr += CtrlViewUtils::Color2HtmlColorStr(theDrawParam->SimpleIsoLineColorShadeLowValueColor());
            colorStr += ", midLimit=";
            colorStr += std::to_string(theDrawParam->SimpleIsoLineColorShadeMidValue());
            colorStr += ", midColor=";
            colorStr += CtrlViewUtils::Color2HtmlColorStr(theDrawParam->SimpleIsoLineColorShadeMidValueColor());
            colorStr += ", highLimit=";
            colorStr += std::to_string(theDrawParam->SimpleIsoLineColorShadeHighValue());
            colorStr += ", highColor=";
            colorStr += CtrlViewUtils::Color2HtmlColorStr(theDrawParam->SimpleIsoLineColorShadeHighValueColor());
            colorStr += ", topLimit=";
            colorStr += std::to_string(theDrawParam->SimpleIsoLineColorShadeHigh2Value());
            colorStr += ", topColor=";
            colorStr += CtrlViewUtils::Color2HtmlColorStr(theDrawParam->SimpleIsoLineColorShadeHigh2ValueColor());
            colorStr += ", shadingCount=";
            colorStr += std::to_string(theDrawParam->SimpleIsoLineColorShadeClassCount());
        }
        return colorStr;
    }

    std::string GetCommonIsolineSetup2String(boost::shared_ptr<NFmiDrawParam>& theDrawParam)
    {
        std::string commonSetupStr;
        commonSetupStr += ", showLabelBox=";
        commonSetupStr += ::GetBoolString(theDrawParam->ShowSimpleIsoLineLabelBox());
        auto doLabelFilling = !theDrawParam->UseTransparentFillColor();
        commonSetupStr += ", doLabelBoxFill=";
        commonSetupStr += ::GetBoolString(doLabelFilling);
        if(doLabelFilling)
        {
            commonSetupStr += ", labelFillColor=";
            commonSetupStr += CtrlViewUtils::Color2HtmlColorStr(theDrawParam->IsolineLabelBoxFillColor());
        }
        commonSetupStr += ", smoothingFactor=";
        commonSetupStr += std::to_string(theDrawParam->IsoLineSplineSmoothingFactor());
        commonSetupStr += ", doFeathering=";
        commonSetupStr += ::GetBoolString(theDrawParam->UseIsoLineFeathering());
        commonSetupStr += ", drawOnlyOverMasks=";
        commonSetupStr += ::GetBoolString(theDrawParam->DrawOnlyOverMask());
        return commonSetupStr;
    }

    std::string GetCommonSimpleIsolineSetupString(boost::shared_ptr<NFmiDrawParam>& theDrawParam)
    {
        std::string commonSetupStr;
        commonSetupStr += ", gab=";
        commonSetupStr += std::to_string(theDrawParam->IsoLineGab());
        commonSetupStr += ", lineType=";
        commonSetupStr += std::to_string(theDrawParam->SimpleIsoLineLineStyle());
        commonSetupStr += ", lineWidth=";
        commonSetupStr += std::to_string(theDrawParam->SimpleIsoLineWidth());
        commonSetupStr += ", labelHeight=";
        commonSetupStr += std::to_string(theDrawParam->SimpleIsoLineLabelHeight());
        commonSetupStr += ", labelDecimals=";
        commonSetupStr += std::to_string(theDrawParam->IsoLineLabelDigitCount());
        return commonSetupStr;
    }

    std::string GetCommonIsolineSetupString(boost::shared_ptr<NFmiDrawParam>& theDrawParam, NFmiIsoLineData* theIsoLineData, bool stationData, size_t totalCounterForIsolineDrawing, bool mapViewCase)
    {
        std::string commonSetupStr;
        if(theDrawParam->UseSimpleIsoLineDefinitions())
        {
            commonSetupStr += "SimpleIsolineDraw";
            if(theDrawParam->UseSingleColorsWithSimpleIsoLines())
                commonSetupStr += "-basic";
            else
                commonSetupStr += "-multiColor";
        }
        else
        {
            commonSetupStr += "CustomIsolineDraw";
        }

        if(!mapViewCase)
        {
            commonSetupStr += "(crossSection)";
        }

        commonSetupStr += ": totalDrawCounter=";
        commonSetupStr += std::to_string(totalCounterForIsolineDrawing);
        commonSetupStr += ", ";

        auto& infoPtr = theIsoLineData->itsInfo;
        auto macroParamCase = theDrawParam->IsMacroParamCase(true);
        if(macroParamCase)
        {
            commonSetupStr += "macroParam=\"";
            commonSetupStr += theDrawParam->InitFileName();
            commonSetupStr += "\"";
        }
        else
        {
            commonSetupStr += "queryData=\"";
            if(infoPtr)
                commonSetupStr += infoPtr->DataFileName();
            else
                commonSetupStr += "nullptr data given?";
            commonSetupStr += "\"";
        }

        commonSetupStr += ", param=\"";
        commonSetupStr += theDrawParam->ParameterAbbreviation();
        commonSetupStr += ", ";
        commonSetupStr += std::to_string(theDrawParam->Param().GetParamIdent());
        commonSetupStr += "\"";

        if(!macroParamCase && infoPtr)
        {
            commonSetupStr += ", producer=\"";
            auto producer = infoPtr->Producer();
            commonSetupStr += producer->GetName();
            commonSetupStr += ",";
            commonSetupStr += std::to_string(producer->GetIdent());
            commonSetupStr += "\"";
        }

        auto& level = theDrawParam->Level();
        if(level.GetIdent() != 0)
        {
            commonSetupStr += ", level=\"";
            commonSetupStr += std::to_string(level.LevelValue());
            commonSetupStr += ",";
            commonSetupStr += std::to_string(level.GetIdent());
            commonSetupStr += "\"";
        }

        commonSetupStr += ", ";
        if(infoPtr)
        {
            if(stationData)
            {
                commonSetupStr += "station data is gridded with ";
                commonSetupStr += std::to_string(infoPtr->SizeLocations());
                commonSetupStr += " locations";
            }
            else
            {
                commonSetupStr += "original grid data size ";
                commonSetupStr += std::to_string(infoPtr->GridXNumber());
                commonSetupStr += "x";
                commonSetupStr += std::to_string(infoPtr->GridYNumber());
                auto areaPtr = infoPtr->Area();
                if(areaPtr)
                {
                    commonSetupStr += ", data area = ";
                    commonSetupStr += areaPtr->AreaFactoryStr();
                }
            }
        }

        commonSetupStr += ", calculated visualized grid ";
        commonSetupStr += std::to_string(theIsoLineData->itsIsolineData.NX());
        commonSetupStr += "x";
        commonSetupStr += std::to_string(theIsoLineData->itsIsolineData.NY());

        return commonSetupStr;
    }

    template<typename Container>
    std::string MakeContainerToCommaSeparatedStringValues(const Container& container)
    {
        std::string valueStr = "\"";
        size_t index = 0;
        for(const auto& item : container)
        {
            if(index > 0)
                valueStr += ",";
            valueStr += std::to_string(item);
            index++;
        }
        valueStr += "\"";
        return valueStr;
    }
}

namespace ForcedLogging
{
    void IsolineDrawingInfo(boost::shared_ptr<NFmiDrawParam>& theDrawParam, NFmiIsoLineData* theIsoLineData, bool stationData, bool toolMasterUsed, bool mapViewCase)
    {
        if(!toolMasterUsed)
            return;

        static size_t totalCounterForIsolineDrawing = 1;

        std::string logMessage = ::GetCommonIsolineSetupString(theDrawParam, theIsoLineData, stationData, totalCounterForIsolineDrawing, mapViewCase);

        if(theDrawParam->UseSimpleIsoLineDefinitions())
        {
            logMessage += ::GetCommonSimpleIsolineSetupString(theDrawParam);
            logMessage += ::GetSimpleIsolineColorString(theDrawParam);
            logMessage += ::GetCommonIsolineSetup2String(theDrawParam);
        }
        else
        {
            logMessage += ", limits=";
            logMessage += ::MakeContainerToCommaSeparatedStringValues(theDrawParam->SpecialIsoLineValues());
            logMessage += ", lineWidths=";
            logMessage += ::MakeContainerToCommaSeparatedStringValues(theDrawParam->SpecialIsoLineWidth());
            logMessage += ", lineTypes=";
            logMessage += ::MakeContainerToCommaSeparatedStringValues(theDrawParam->SpecialIsoLineStyle());
            logMessage += ", labelHeights=";
            logMessage += ::MakeContainerToCommaSeparatedStringValues(theDrawParam->SpecialIsoLineLabelHeight());
            logMessage += ", lineColorIndexies=";
            logMessage += ::MakeContainerToCommaSeparatedStringValues(theDrawParam->SpecialIsoLineColorIndexies());
            logMessage += ::GetCommonIsolineSetup2String(theDrawParam);
        }

        CatLog::logMessage(logMessage, CatLog::Severity::Debug, CatLog::Category::Visualization, true);

        totalCounterForIsolineDrawing++;
    }
}

