#include "CtrlViewTimeConsumptionReporter.h"
#include "NFmiCtrlView.h"
#include "catlog/catlog.h"
#include "SpecialDesctopIndex.h"
#include "NFmiDrawParam.h"
#include "CtrlViewFunctions.h"
#include "NFmiDictionaryFunction.h"
#include "CtrlViewDocumentInterface.h"

namespace CtrlViewUtils
{
    size_t CtrlViewTimeConsumptionReporter::currentUpdateId_ = 1001;

    CtrlViewTimeConsumptionReporter::CtrlViewTimeConsumptionReporter(NFmiCtrlView *ctrlView, const std::string &eventName)
        :ctrlView_(ctrlView)
        , eventName_(eventName)
        , identifier_()
        , startTime_(std::chrono::system_clock::now())
    {
        if(CatLog::doTraceLevelLogging())
        {
            identifier_ = makeCtrlViewIdentifier(ctrlView_);
            std::string message = makeCurrentUpdateIdString() + " " + eventName_;
            if(ctrlView_)
                message += " starts for: " + identifier_;
            else
                message += " starts";
            CatLog::logMessage(message, CatLog::Severity::Trace, CatLog::Category::Visualization, true);
        }
    }

    CtrlViewTimeConsumptionReporter::~CtrlViewTimeConsumptionReporter()
    {
        if(CatLog::doTraceLevelLogging())
        {
            auto endTime(std::chrono::system_clock::now());
            auto durationValue = endTime - startTime_;
            std::string message = makeCurrentUpdateIdString() + " Operation lasted ";
            message += makeReadableDurationString(durationValue);
            message += " for " + eventName_;
            if(ctrlView_)
                message += " by: " + identifier_;
            CatLog::logMessage(message, CatLog::Severity::Trace, CatLog::Category::Visualization, true);
        }
    }

    std::string CtrlViewTimeConsumptionReporter::makeReadableDurationString(const std::chrono::duration<float> &durationValue)
    {
        std::string durationString;
        auto secondsValue = std::chrono::duration_cast<std::chrono::seconds>(durationValue);
        durationString += std::to_string(secondsValue.count());
        durationString += ".";
        auto milliSecondsPart = std::chrono::duration_cast<std::chrono::milliseconds>(durationValue).count() % 1000;
        if(milliSecondsPart < 10)
            durationString += "00";
        else if(milliSecondsPart < 100)
            durationString += "0";
        durationString += std::to_string(milliSecondsPart) + "s";

        return durationString;
    }

    // Identifier is: map-view=x, row=y, column=z
    std::string CtrlViewTimeConsumptionReporter::makeCtrlViewIdentifier(NFmiCtrlView *ctrlView)
    {
        if(ctrlView == nullptr)
            return "";

        bool hasActualDataWithName = ctrlView->DrawParam() ? ctrlView->DrawParam()->DataType() != NFmiInfoData::kNoDataType : false;
        std::string identifier;
        if(hasActualDataWithName)
        {
            bool crossSectionView = false;
            identifier += "'";
            identifier += CtrlViewUtils::GetParamNameString(ctrlView->DrawParam(), ctrlView->GetCtrlViewDocumentInterface(), ::GetDictionaryString("MapViewToolTipOrigTimeNormal"), ::GetDictionaryString("MapViewToolTipOrigTimeMinute"), crossSectionView, false, false, 0, ctrlView->IsTimeSerialView());
            identifier += "' ";
        }

        identifier += "[";
        if(ctrlView->MapViewDescTopIndex() <= CtrlViewUtils::kFmiMaxMapDescTopIndex)
        {
            identifier += "map-view=";
            identifier += std::to_string(ctrlView->MapViewDescTopIndex() + 1);
        }
        else if(ctrlView->MapViewDescTopIndex() == CtrlViewUtils::kFmiCrossSectionView)
        {
            identifier += "cross-section";
        }
        else if(ctrlView->MapViewDescTopIndex() == CtrlViewUtils::kFmiTimeSerialView)
        {
            identifier += "time-serial";
        }
        identifier += ", row=";
        int absoluteRowNumber = ctrlView->ViewGridRowNumber() + ctrlView->GetCtrlViewDocumentInterface()->MapRowStartingIndex(ctrlView->MapViewDescTopIndex()) - 1;
        identifier += std::to_string(absoluteRowNumber);
        if(ctrlView->ViewGridColumnNumber() >= 1)
        {
            identifier += ", column=";
            identifier += std::to_string(ctrlView->ViewGridColumnNumber());
        }
        // Only views with reasonable dataType are meaningful to include leyer-numbers
        if(hasActualDataWithName && ctrlView->ViewRowLayerNumber() >= 1)
        {
            identifier += ", layer=";
            identifier += std::to_string(ctrlView->ViewRowLayerNumber());
        }
        identifier += "]";
        return identifier;
    }

    void CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(const std::string &message, NFmiCtrlView *ctrlView)
    {
        if(CatLog::doTraceLevelLogging())
        {
            std::string identifierString = " ";
            identifierString += CtrlViewTimeConsumptionReporter::makeCtrlViewIdentifier(ctrlView);
            CatLog::logMessage(makeCurrentUpdateIdString() + " " + message + identifierString, CatLog::Severity::Trace, CatLog::Category::Visualization, true);
        }
    }

    void CtrlViewTimeConsumptionReporter::setCurrentUpdateId(size_t currentUpdateId)
    {
        currentUpdateId_ = currentUpdateId;
    }

    std::string CtrlViewTimeConsumptionReporter::makeCurrentUpdateIdString()
    {
        return std::string("[") + std::to_string(currentUpdateId_) + "]";
    }

    void CtrlViewTimeConsumptionReporter::increaseCurrentUpdateId()
    {
        currentUpdateId_++;
    }

}
