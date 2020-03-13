#include "CombinedMapHandlerInterface.h"
#include "NFmiDrawParam.h"
#include "NFmiFastQueryInfo.h"
#include "ApplicationInterface.h"
#include "catlog/catlog.h"

namespace
{
	bool verboseLogging_ = false;

	// Lokitetaan annettu viesti vain jos SmartMet on verbose tilassa (-v optio käynnistyksessä)
	void doVerboseLogReporting(const std::string& logMessage)
	{
		if(verboseLogging_)
			CatLog::logMessage(logMessage, CatLog::Severity::Debug, CatLog::Category::Configuration);
	}
}

CombinedMapHandlerInterface::GetCombinedMapHandlerInterfaceImplementationCallBackType CombinedMapHandlerInterface::GetCombinedMapHandlerInterfaceImplementation;

bool CombinedMapHandlerInterface::isDrawParamForecast(boost::shared_ptr<NFmiDrawParam>& drawParam)
{
	if(drawParam)
	{
		switch(drawParam->DataType())
		{
		case NFmiInfoData::kViewable:
		case NFmiInfoData::kHybridData:
		case NFmiInfoData::kKepaData:
		case NFmiInfoData::kEditable:
		case NFmiInfoData::kCopyOfEdited:
		case NFmiInfoData::kMacroParam:
		case NFmiInfoData::kCrossSectionMacroParam:
		case NFmiInfoData::kQ3MacroParam:
		case NFmiInfoData::kModelHelpData:
		case NFmiInfoData::kConceptualModelData:
		case NFmiInfoData::kEditingHelpData:
		case NFmiInfoData::kTrajectoryHistoryData:
			return true;
		}
	}
	return false;
}

bool CombinedMapHandlerInterface::isDrawParamObservation(boost::shared_ptr<NFmiDrawParam>& drawParam)
{
	if(drawParam)
	{
		switch(drawParam->DataType())
		{
		case NFmiInfoData::kObservations:
		case NFmiInfoData::kClimatologyData:
		case NFmiInfoData::kAnalyzeData:
		case NFmiInfoData::kFlashData:
		case NFmiInfoData::kSatelData:
			return true;
		}
	}
	return false;
}

void CombinedMapHandlerInterface::activeEditedParameterMayHaveChangedViewUpdateFlagSetting(int mapViewDescTopIndex)
{
	auto usedViewUpdateFlag = ::GetWantedMapViewIdFlag(mapViewDescTopIndex) | SmartMetViewId::DataFilterToolDlg | SmartMetViewId::BrushToolDlg;
	ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(usedViewUpdateFlag);
}

std::string CombinedMapHandlerInterface::getSelectedParamInfoString(const NFmiDataIdent* dataIdent, const NFmiLevel* level)
{
	std::string str("Param:");
	str += NFmiStringTools::Convert<unsigned long>(dataIdent->GetParamIdent());
	str += " ('";
	str += dataIdent->GetParamName();
	str += "')";

	str += ", Producer: ";
	str += NFmiStringTools::Convert<unsigned long>(dataIdent->GetProducer()->GetIdent());
	str += " ('";
	str += dataIdent->GetProducer()->GetName();
	str += "')";

	if(level)
	{
		str += ", Level: ";
		str += NFmiStringTools::Convert<unsigned long>(level->LevelTypeId());
		str += " ('";
		str += level->GetName();
		str += ", ";
		str += NFmiStringTools::Convert<float>(level->LevelValue());
		str += "')";
	}
	str += ".";
	return str;
}

// Funktio, joka palauttaa annetusta drawparamista stringin ,joka kertoo:
// Param: id (name) Prod: id (name) Level: id (name, value)
std::string CombinedMapHandlerInterface::getSelectedParamInfoString(boost::shared_ptr<NFmiFastQueryInfo>& fastInfo, bool ignoreLevel)
{
	const NFmiDataIdent& dataIdent = fastInfo->Param();
	const NFmiLevel* level = 0;
	if(ignoreLevel == false)
	{
		if(fastInfo->SizeLevels() > 1)
			if(fastInfo->LevelIndex() < fastInfo->SizeLevels()) // pitää pitää huoli että level index osoittaa johonkin, tässä ei ruveta kuitenkaan säätämään leveliä esim. 1. leveliin
				level = fastInfo->Level();
	}
	return getSelectedParamInfoString(&dataIdent, level);
}

void CombinedMapHandlerInterface::verboseLogging(bool newValue)
{
	verboseLogging_ = newValue;
}

bool CombinedMapHandlerInterface::verboseLogging()
{
	return verboseLogging_;
}

void CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(const std::string& functionName)
{
	::doVerboseLogReporting(std::string("Starting ") + functionName + " -function");
}
