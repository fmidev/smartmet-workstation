#include "NFmiLedLightStatus.h"
#include "NFmiFileString.h"
#include "NFmiColor.h"
#include "ColorStringFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ***************************************************
// ******** NFmiLedLightStatus starts ****************
// ***************************************************

NFmiLedLightStatus::NFmiLedLightStatus() = default;

NFmiLedLightStatus::NFmiLedLightStatus(int statusBarPaneIndex, NFmiLedAction ledAction, NFmiLedColor ledColor, NFmiLedChannel ledChannel, bool ledColorSeverityBased)
	:statusBarPaneIndex_(statusBarPaneIndex)
	, ledAction_(ledAction)
	, ledColor_(ledColor)
	, ledChannel_(ledChannel)
	,ledColorSeverityBased_(ledColorSeverityBased)
{
}

std::pair<bool, NFmiLedColor> NFmiLedLightStatus::hasLedColorChanged()
{
	if(actionChanged_)
	{
		return actionChangedColor();
	}
	else
	{
		return hasColorChanged();
	}
	return std::make_pair(false, NFmiLedColor::Gray);
}

bool NFmiLedLightStatus::isBlinking() const
{
	return ledAction_ == NFmiLedAction::Blink || ledAction_ == NFmiLedAction::SlowBlink;
}

std::pair<bool, NFmiLedColor> NFmiLedLightStatus::actionChangedColor()
{
	actionChanged_ = false;
	if(ledAction_ != NFmiLedAction::Off)
	{
		return colorChanged(true);
	}
	else
	{
		return colorChanged(false);
	}
}

std::pair<bool, NFmiLedColor> NFmiLedLightStatus::hasColorChanged()
{
	if(timeToChangeBlinkingColor())
	{
		if(lastBlinkAction_ == NFmiLedAction::On)
		{
			return colorChanged(false);
		}
		else
		{
			return colorChanged(true);
		}
	}
	return std::make_pair(false, NFmiLedColor::Gray);
}

int NFmiLedLightStatus::blinkTimeLimitsInMs() const
{
	if(isBlinking())
	{
		auto lastActionOn = (lastBlinkAction_ == NFmiLedAction::On);
		switch(ledAction_)
		{
		case NFmiLedAction::Blink:
			return lastActionOn ? 370 : 180;
		case NFmiLedAction::SlowBlink:
			return lastActionOn ? 560 : 180;
		default:
			break;
		}
	}
	return 0;
}

bool NFmiLedLightStatus::timeToChangeBlinkingColor() const
{
	if(isBlinking())
	{
		auto limitInMs = blinkTimeLimitsInMs();
		if(limitInMs <= lastBlinkChangeTimer_.CurrentTimeDiffInMSeconds())
			return true;
	}
	return false;
}

std::pair<bool, NFmiLedColor> NFmiLedLightStatus::colorChanged(bool setModeOn)
{
	lastBlinkAction_ = setModeOn ? NFmiLedAction::On : NFmiLedAction::Off;
	lastBlinkChangeTimer_.StartTimer();
	return std::make_pair(true, setModeOn ? ledColor_ : NFmiLedColor::Gray);
}

void NFmiLedLightStatus::ledAction(NFmiLedAction newValue) 
{ 
	actionChanged_ = true;
	ledAction_ = newValue; 
}

void NFmiLedLightStatus::ledColor(NFmiLedColor newColor)
{
	actionChanged_ = true;
	ledColor_ = newColor;
}

// ***************************************************
// ****** NFmiLedLightStatusSystem starts ************
// ***************************************************

bool NFmiLedLightStatusSystem::programWantsToStop_ = false;
void NFmiLedLightStatusSystem::ProgramStopsNow()
{
	programWantsToStop_ = true;
}

NFmiLedLightStatusSystem* NFmiLedLightStatusSystem::staticInstance_ = nullptr;

NFmiLedLightStatusSystem::NFmiLedLightStatusSystem() = default;

void NFmiLedLightStatusSystem::Initialize(const std::vector<NFmiLedChannelInitializer>& channelInitializers, bool mapViewTextStatusbarPaneIsAfterLeds)
{
	channelInitializers_ = channelInitializers;
	int actualUsedLedCount = (int)channelInitializers_.size();
	if(actualUsedLedCount > g_maximumNumberOfLedsInStatusbar)
	{
		actualUsedLedCount = g_maximumNumberOfLedsInStatusbar;
		std::string warningString = __FUNCTION__;
		warningString += ": wanted led status number exceeds maximum ";
		warningString += std::to_string(g_maximumNumberOfLedsInStatusbar);
		warningString += ", using only the first inside allowed number, logical error in SmartMet code.";
		CatLog::logMessage(warningString, CatLog::Severity::Warning, CatLog::Category::Configuration, true);
	}

	// Vain kaksi konfiguraatiota sallittu:
	if(mapViewTextStatusbarPaneIsAfterLeds)
	{
		// 1. Ensin on kaikki ledit, sen j‰lkeen status-teksti-paneeli ja sen j‰lkeen loput
		startingLedPaneIndex_ = 0;
		mapViewTextStatusbarPaneIndex_ = actualUsedLedCount;
	}
	else
	{
		// 2. Ensin on status-teksti-paneeli, sen j‰lkeen kaikki ledit ja sitten loput
		startingLedPaneIndex_ = 1;
		mapViewTextStatusbarPaneIndex_ = 0;
	}
	for(int index = 0; index < actualUsedLedCount; index++)
	{
		auto paneIndex = startingLedPaneIndex_ + index;
		const auto& channelInitializer = channelInitializers_[index];
		ledLightStatusVector_.push_back(NFmiLedLightStatus(paneIndex, NFmiLedAction::Off, channelInitializer.ledColor_, channelInitializer.ledChannel_, channelInitializer.ledColorSeverityBased_));
	}
	isInitilized_ = true;
}

void NFmiLedLightStatusSystem::InitializeStaticInstance(NFmiLedLightStatusSystem* staticInstance)
{
	staticInstance_ = staticInstance;
}

static std::string GetFontColor(CatLog::Severity severity)
{
	switch(severity)
	{
	case CatLog::Severity::Critical:
		return "magenta"; // magenta teksti n‰kyy purplea parammin tooltipin kelta pohjaisessa bubblessa
	case CatLog::Severity::Error:
		return "red";
	case CatLog::Severity::Warning:
		return ColorString::Color2HtmlColorStr(NFmiColor(1.f, 0.37f, 0.f)); // punertavampi oranssi n‰kyy paremmin tooltipissa
	default:
		return "black";
	}
}

// ledPaneIndex alkaa 0:sta ja osoittaa ledLightStatusVector_:iin
std::string NFmiLedLightStatusSystem::GetTooltipStatusMessage(int ledPaneIndex)
{
	if(ledPaneIndex >= ledLightStatusVector_.size())
	{
		std::string statusMessage = "Internal error in SmartMet code when requesting\nled-control tooltip status message with ledPaneIndex ";
		statusMessage += std::to_string(ledPaneIndex);
		statusMessage += ",\nwhich was over actual led-controls count limit";
		return statusMessage;
	}

	auto usedLedChannel = ledLightStatusVector_[ledPaneIndex].ledChannel();
	std::string nonEmptyStatusMessages = MakeNonEmptyStatusMessageFromChannel(usedLedChannel);
	std::string finalStatusMessage = channelInitializers_[ledPaneIndex].channelTopic_;
	finalStatusMessage += "<br><hr color=red><br>";
	if(nonEmptyStatusMessages.empty())
		finalStatusMessage += channelInitializers_[ledPaneIndex].emptyReportBaseMessage_;
	else
		finalStatusMessage += nonEmptyStatusMessages;
	return finalStatusMessage;
}

// Eri Led-kanaville tulee raportteja erilaisista worker-threadeista, jolloin raportoinnista pit‰‰ tehd‰ thread turvallista.
// Lis‰ksi eri l‰hteist‰ (threadeista) halutaan s‰ilytt‰‰ erilliset viestit, jotka liitet‰‰n listoina tooltippiin.
// Sanomilla on severity (h‰lytystaso), jotta eri viestej‰ voidaan v‰ritt‰‰ eri v‰rein tooltippiin.
bool NFmiLedLightStatusSystem::ReportToChannel(NFmiLedChannel ledChannel, const std::string& reporterName, const std::string& message, CatLog::Severity messageSeverity)
{
	try
	{
		{
			// Tehd‰‰n koodi blokki, jotta mutex vapautuu heti kun on mahdollista
			std::lock_guard<std::mutex> lock(channelReportsMutex_);
			channelReports_[ledChannel][reporterName] = ChannelReport{ message, messageSeverity };
		}
		SetLedChannelActionMode(ledChannel);
		return true;
	}
	catch(...)
	{ }
	return false;
}

bool NFmiLedLightStatusSystem::StopReportToChannel(NFmiLedChannel ledChannel, const std::string& reporterName)
{
	try
	{
		{
			// Tehd‰‰n koodi blokki, jotta mutex vapautuu heti kun on mahdollista
			std::lock_guard<std::mutex> lock(channelReportsMutex_);
			channelReports_[ledChannel][reporterName] = ChannelReport{ "", CatLog::Severity::Info };
		}
		SetLedChannelActionMode(ledChannel);
		return true;
	}
	catch(...)
	{
	}
	return false;
}

bool NFmiLedLightStatusSystem::StopReportToChannelWithFileFilter(NFmiLedChannel ledChannel, const std::string& fileFilter)
{
	const std::string searchWord1 = "combined";
	auto channelReports = getChannelReports(ledChannel);
	NFmiFileString fileStr(fileFilter);
	auto searchedFileFilterPart = fileStr.FileName();
	// Jos jostain channel report rivist‰ lˆytyy sek‰ sana 'combined' ett‰ annettu filefilter, t‰llˆin sen rivin raportti poistetaan
	for(const auto &channelReport : channelReports)
	{
		const auto& reportStr = channelReport.message_;
		auto position1 = reportStr.find(searchWord1);
		auto position2 = reportStr.find(searchedFileFilterPart);
		if(position1 != std::string::npos && position2 != std::string::npos)
		{
			return ClearChannelReport(ledChannel, reportStr);
		}
	}
	return false;
}

bool NFmiLedLightStatusSystem::ClearChannelReport(NFmiLedChannel ledChannel, const std::string& reportStr)
{
	try
	{
		bool removedMessage = false;
		// Laitetaan mutex-lukko erilliseen blokkiin, jotta se purkautuu
		{
			std::lock_guard<std::mutex> lock(channelReportsMutex_);
			auto& wantedChannelReport = channelReports_.at(ledChannel);
			for(auto iter = wantedChannelReport.begin(); iter != wantedChannelReport.end(); ++iter)
			{
				if(iter->second.message_ == reportStr)
				{
					iter = wantedChannelReport.erase(iter);
					removedMessage = true;
					break; // Turha jatkaa poiston j‰lkeen, on myˆs kaatunut iteraattorin juoksutukseen
				}
			}
		} // mutex-lukon purku

		if(removedMessage)
		{
			// T‰t‰ saa kutsua vasta kun edell‰ ollut mutex-lukko on purettu
			SetLedChannelActionMode(ledChannel);
		}
		return removedMessage;
	}
	catch(...)
	{
	}
	return false;
}

bool NFmiLedLightStatusSystem::ReportToChannelFromThread(NFmiLedChannel ledChannel, const std::string& reporterName, const std::string& message, CatLog::Severity messageSeverity)
{
	if(staticInstance_)
	{
		return staticInstance_->ReportToChannel(ledChannel, reporterName, message, messageSeverity);
	}
	return false;
}

bool NFmiLedLightStatusSystem::StopReportToChannelFromThread(NFmiLedChannel ledChannel, const std::string& reporterName)
{
	if(staticInstance_)
	{
		return staticInstance_->StopReportToChannel(ledChannel, reporterName);
	}
	return false;
}

std::vector<ChannelReport> NFmiLedLightStatusSystem::getChannelReports(NFmiLedChannel ledChannel) const
{
	try
	{
		std::lock_guard<std::mutex> lock(channelReportsMutex_);
		const auto &wantedChannelReport = channelReports_.at(ledChannel);
		std::vector<ChannelReport> nonEmptyReports;
		for(const auto& channelReport : wantedChannelReport)
		{
			if(!channelReport.second.message_.empty())
				nonEmptyReports.push_back(channelReport.second);
		}
		return nonEmptyReports;
	}
	catch(...)
	{
	}
	return std::vector<ChannelReport>();
}

void NFmiLedLightStatusSystem::SetLedChannelActionMode(NFmiLedChannel ledChannel)
{
	auto ledLightStatus = GetLedLightStatus(ledChannel);
	if(ledLightStatus)
	{
		auto nonEmptyStatusMessages = MakeNonEmptyStatusMessageFromChannel(ledChannel);
		auto newLedAction = nonEmptyStatusMessages.empty() ? NFmiLedAction::Off : NFmiLedAction::Blink;
		ledLightStatus->ledAction(newLedAction);
		if(ledLightStatus->ledColorSeverityBased())
		{
			SetColorByMaxSeverity(ledChannel, *ledLightStatus);
		}
	}
}

void NFmiLedLightStatusSystem::SetColorByMaxSeverity(NFmiLedChannel ledChannel, NFmiLedLightStatus& ledLightStatus)
{
	auto maxSeverity = GetChannelMaxSeverity(ledChannel);
	ledLightStatus.ledColor(GetLedColor(maxSeverity, true));
}

CatLog::Severity NFmiLedLightStatusSystem::GetChannelMaxSeverity(NFmiLedChannel ledChannel)
{
	CatLog::Severity maxFoundSeverity = CatLog::Severity::Trace;
	try
	{
		std::lock_guard<std::mutex> lock(channelReportsMutex_);
		const auto& wantedChannelReport = channelReports_.at(ledChannel);
		std::vector<ChannelReport> nonEmptyReports;
		for(const auto& channelReport : wantedChannelReport)
		{
			if(!channelReport.second.message_.empty())
			{
				auto currentSeverity = channelReport.second.severity_;
				if(maxFoundSeverity < currentSeverity)
					maxFoundSeverity = currentSeverity;
			}
		}
	}
	catch(...)
	{
	}
	return maxFoundSeverity;
}

NFmiLedColor NFmiLedLightStatusSystem::GetLedColor(CatLog::Severity severity, bool warningsOrHigher)
{
	if(warningsOrHigher && severity <= CatLog::Severity::Info)
		return NFmiLedColor::Gray;
	switch(severity)
	{
	case CatLog::Severity::Critical:
		return NFmiLedColor::Purple;
	case CatLog::Severity::Error:
		return NFmiLedColor::Red;
	case CatLog::Severity::Warning:
		return NFmiLedColor::Orange;
	case CatLog::Severity::Info:
		return NFmiLedColor::Green;
	case CatLog::Severity::Debug:
		return NFmiLedColor::LightGreen;
	case CatLog::Severity::Trace:
		return NFmiLedColor::LightBlue;
	default:
		return NFmiLedColor::Gray;
	}
}

std::string NFmiLedLightStatusSystem::MakeSeverityString(CatLog::Severity severity)
{
	switch(severity)
	{
	case CatLog::Severity::Critical:
		return "#CRITICAL#";
	case CatLog::Severity::Error:
		return "ERROR";
	case CatLog::Severity::Warning:
		return "Warning";
	case CatLog::Severity::Info:
		return "info";
	case CatLog::Severity::Debug:
		return "debug";
	case CatLog::Severity::Trace:
		return "trace";
	default:
		return "???";
	}
}

std::string NFmiLedLightStatusSystem::MakeNonEmptyStatusMessageFromChannel(NFmiLedChannel ledChannel)
{
	auto ledLightStatus = GetLedLightStatus(ledChannel);
	if(!ledLightStatus)
		return "";

	auto channelReports = getChannelReports(ledChannel);
	std::string nonEmptyStatusMessages;
	for(const auto& channelReport : channelReports)
	{
		if(!channelReport.message_.empty())
		{
			if(!nonEmptyStatusMessages.empty())
			{
				nonEmptyStatusMessages += "<br><hr color=skyblue><br>";
			}
			nonEmptyStatusMessages += "<b><font color=";
			nonEmptyStatusMessages += ::GetFontColor(channelReport.severity_);
			nonEmptyStatusMessages += ">";
			if(ledLightStatus->ledColorSeverityBased())
			{
				nonEmptyStatusMessages += "(";
				nonEmptyStatusMessages += MakeSeverityString(channelReport.severity_);
				nonEmptyStatusMessages += ") ";
			}
			nonEmptyStatusMessages += channelReport.message_;
			nonEmptyStatusMessages += "</font></b>";
		}
	}
	return nonEmptyStatusMessages;
}

NFmiLedLightStatus* NFmiLedLightStatusSystem::GetLedLightStatus(NFmiLedChannel ledChannel)
{
	auto iter = std::find_if(ledLightStatusVector_.begin(), ledLightStatusVector_.end(),
		[=](const NFmiLedLightStatus& ledLightStatus) {return ledLightStatus.ledChannel() == ledChannel; }
	);

	if(iter != ledLightStatusVector_.end())
		return &(*iter);
	return nullptr;
}

// ********************************************************
// ****** NFmiLedLightStatusBlockReporter starts **********
// ********************************************************

NFmiLedLightStatusBlockReporter::NFmiLedLightStatusBlockReporter(NFmiLedChannel usedLedChannel, const std::string& reporterName, const std::string& reportMessage)
	:usedLedChannel_(usedLedChannel)
	, reporterName_(reporterName)
{
	NFmiLedLightStatusSystem::ReportToChannelFromThread(usedLedChannel_, reporterName_, reportMessage, CatLog::Severity::Info);
}

NFmiLedLightStatusBlockReporter::~NFmiLedLightStatusBlockReporter()
{
	// Eli StopReportToChannelFromThread kutsu tehd‰‰n vain jos ohjelmaa ei ole viel‰ haluttu jo sulkea!
	if(!NFmiLedLightStatusSystem::ProgramWantsToStop())
	{
		NFmiLedLightStatusSystem::StopReportToChannelFromThread(usedLedChannel_, reporterName_);
	}
}
