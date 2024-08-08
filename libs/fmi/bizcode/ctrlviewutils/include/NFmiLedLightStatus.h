#pragma once

#include "NFmiMilliSecondTimer.h"
#include "catlog/catlog.h"
#include <mutex> 


enum class NFmiLedColor
{
	None,
	Blue,
	Gray,
	Green,
	LightBlue,
	LightGreen,
	Orange,
	Purple,
	Red,
	Yellow
};

enum class NFmiLedAction
{
	Off, // => harmaa ledi p‰‰lle
	On, // Jonkun v‰rinen ledi palaa koko ajan
	Blink,
	SlowBlink,
};

// Eri ledit s‰‰det‰‰n kuuntelemaan erilaisten 'kanavien' (channel) viestityksi‰.
enum class NFmiLedChannel
{
	None,
	// QueryDatojen luku serverilt‰, combined-datojen teko ja soundingIndex datojen tekoon liittyvi‰ sanomia
	QueryData, 
	// Eri Wms servereiden getcapabilities haut, wms layerien haut, jne.
	WmsData, 
	// Yleiskanava kaikille operatiivisille ongelmille, mist‰ k‰ytt‰j‰‰ pit‰isi varoittaa
	OperationalInfo, 
	// Jokaiselle queryDatalle voidaan antaa konffeissa myˆh‰stymisaika ja k‰ytt‰j‰lle raportoidaan jos uutta 
	// dataa ei ole tullut kyseisen aikarajan sis‰ll‰. Kyse on vain siit‰ ett‰ joku data luetaan klo xx.xx,
	// Jos kyseiselle datalle ei ole tullut uudempaa dataa klo yy.yy menness‰, siit‰ aloitetaan varoittamaan k‰ytt‰j‰‰.
	DataIsLate 
};

// Jos tarvitaan enemm‰n ledej‰, pit‰‰ niit‰ varten tehd‰ uusia 
// ID_INDICATOR_LED_CONTROL_1-6 tyyppisi‰ indikaattoreita 
// SmartMet projektin resursseihin ja lis‰t‰ ne indicatorVector:iin 
// CMainFrame::MakeStatusBarIndicators metodissa
const int g_maximumNumberOfLedsInStatusbar = 6;

class NFmiLedLightStatus
{
	int statusBarPaneIndex_ = 0;
	NFmiLedAction ledAction_ = NFmiLedAction::Off;
	NFmiLedAction lastBlinkAction_ = NFmiLedAction::Off;
	NFmiLedColor ledColor_ = NFmiLedColor::Gray;
	NFmiLedChannel ledChannel_ = NFmiLedChannel::None;
	NFmiMilliSecondTimer lastBlinkChangeTimer_;
	bool ledColorSeverityBased_ = false;
	bool actionChanged_ = true;
public:
	NFmiLedLightStatus();
	NFmiLedLightStatus(int statusBarPaneIndex, NFmiLedAction ledAction, NFmiLedColor ledColor, NFmiLedChannel ledChannel, bool ledColorSeverityBased);

	int statusBarPaneIndex() const { return statusBarPaneIndex_; }
	// Palauttaa true ja k‰ytetyn v‰rin, jos on aika vaihtaa ledin v‰ri‰,
	// muuten palauttaa (false,gray) parin
	std::pair<bool, NFmiLedColor> hasLedColorChanged();
	NFmiLedChannel ledChannel() const { return ledChannel_; }
	void ledAction(NFmiLedAction newValue);
	void ledColor(NFmiLedColor newColor);
	bool ledColorSeverityBased() const { return ledColorSeverityBased_; }

private:

	bool isBlinking() const;
	std::pair<bool, NFmiLedColor> actionChangedColor();
	std::pair<bool, NFmiLedColor> hasColorChanged();
	int blinkTimeLimitsInMs() const;
	bool timeToChangeBlinkingColor() const;
	std::pair<bool, NFmiLedColor> colorChanged(bool setModeOn);
};

struct NFmiLedChannelInitializer
{
	NFmiLedChannel ledChannel_ = NFmiLedChannel::None;
	NFmiLedColor ledColor_ = NFmiLedColor::Gray;
	// T‰m‰ tulee aina tooltipin alkuun otsikoksi
	std::string channelTopic_;
	// Jos kaikki kanavan eri l‰hteist‰ tulleet sanomat ovat tyhji‰, laitetaan t‰m‰ teksti tooltippiin sen merkiksi
	std::string emptyReportBaseMessage_;
	bool ledColorSeverityBased_ = false;
};

struct ChannelReport
{
	// Jostain l‰hetetty raportti, joka voi olla myˆs tyhj‰ string:i (jolloin ignoorataan tooltip teksti‰ tehdess‰)
	std::string message_;
	CatLog::Severity severity_ = CatLog::Severity::NoSeverity;
};

class NFmiLedLightStatusSystem
{
	std::vector<NFmiLedChannelInitializer> channelInitializers_;
	std::vector<NFmiLedLightStatus> ledLightStatusVector_;
	int startingLedPaneIndex_ = 0;
	int mapViewTextStatusbarPaneIndex_ = 0;
	// Ensin sanomat jaetaan channel:eihin, joka channe:lilla voi olla eri m‰‰r‰ raportoijia, 
	// ja niist‰ s‰ilytet‰‰n aina viimeinen p‰‰ll‰ oleva viesti (joka voi olla tyhj‰)
	std::map<NFmiLedChannel, std::map<std::string, ChannelReport>> channelReports_;
	// channelReports rakennetta pit‰‰ suojella mutex:illa ja lukoilla, koska raportteja tulee eri s‰ikeist‰ rinnakkain
	mutable std::mutex channelReportsMutex_;
	static NFmiLedLightStatusSystem* staticInstance_;
	bool isInitilized_ = false;
	// T‰m‰ flagi kertoo ett‰ halutaanko Smartmetia jo sulkea. Jos se on true
	// t‰llˆin estet‰‰n tiettyjen operaatioiden tekeminen, jotta ohjelma ei kaatuisi niihin
	// jos tiettyjen working-threadien sulkeminen kest‰‰ kauan ja niit‰ kutsutaan liian myˆh‰‰n.
	// Crashrpt raporteissa on n‰kynyt ett‰ Smartmetia suljettaessa on NFmiLedLightStatusBlockReporter:in
	// destructor on tehnyt kutsuja jo lopetettuun NFmiLedLightStatusSystem:iin.
	static bool programWantsToStop_;
public:
	NFmiLedLightStatusSystem();
	void Initialize(const std::vector<NFmiLedChannelInitializer>& channelInitializers, bool mapViewTextStatusbarPaneIsAfterLeds);
	static void InitializeStaticInstance(NFmiLedLightStatusSystem* staticInstance);

	std::vector<NFmiLedLightStatus>& LedLightStatusVector() { return ledLightStatusVector_; }
	int StartingLedPaneIndex() const { return startingLedPaneIndex_; }
	int MapViewTextStatusbarPaneIndex() const { return mapViewTextStatusbarPaneIndex_; }
	size_t LedPaneCount() const { return ledLightStatusVector_.size(); }
	std::string GetTooltipStatusMessage(int ledPaneIndex);
	bool ReportToChannel(NFmiLedChannel ledChannel, const std::string& reporterName, const std::string& message, CatLog::Severity messageSeverity);
	bool StopReportToChannel(NFmiLedChannel ledChannel, const std::string& reporterName);
	bool StopReportToChannelWithFileFilter(NFmiLedChannel ledChannel, const std::string& fileFilter);
	bool IsInitilized() const { return isInitilized_; }
	static bool ReportToChannelFromThread(NFmiLedChannel ledChannel, const std::string& reporterName, const std::string& message, CatLog::Severity messageSeverity);
	static bool StopReportToChannelFromThread(NFmiLedChannel ledChannel, const std::string& reporterName);
	static void ProgramStopsNow();
	static bool ProgramWantsToStop() { return programWantsToStop_; }
private:
	std::vector<ChannelReport> getChannelReports(NFmiLedChannel ledChannel) const;
	void SetLedChannelActionMode(NFmiLedChannel ledChannel);
	std::string MakeNonEmptyStatusMessageFromChannel(NFmiLedChannel ledChannel);
	bool ClearChannelReport(NFmiLedChannel ledChannel, const std::string& reportStr);
	void SetColorByMaxSeverity(NFmiLedChannel ledChannel, NFmiLedLightStatus& ledLightStatus);
	NFmiLedColor GetLedColor(CatLog::Severity severity, bool warningsOrHigher);
	std::string MakeSeverityString(CatLog::Severity severity);
	CatLog::Severity GetChannelMaxSeverity(NFmiLedChannel ledChannel);
	NFmiLedLightStatus* GetLedLightStatus(NFmiLedChannel ledChannel);
};

// T‰m‰n apuluokan tarkoitus on tehd‰ raportti halutulle kanavalle ja halutulta worker-threadilta.
// Sitten kun toiminta loppuu hallitusti tai hallitsemattomasti, destruktori lopettaa sanoman (sammuttaa kanavaan liittyv‰n viestin).
class NFmiLedLightStatusBlockReporter
{
	NFmiLedChannel usedLedChannel_ = NFmiLedChannel::None;
	std::string reporterName_;
public:
	NFmiLedLightStatusBlockReporter(NFmiLedChannel usedLedChannel, const std::string& reporterName, const std::string& reportMessage);
	~NFmiLedLightStatusBlockReporter();
};
