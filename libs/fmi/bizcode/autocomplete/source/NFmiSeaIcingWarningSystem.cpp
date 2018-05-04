#include "NFmiSeaIcingWarningSystem.h"
#include "NFmiDataStoringHelpers.h"
#include "xmlliteutils/XmlNodeHelpers.h"
#include "NFmiSettings.h"
#include "boost\algorithm\string\predicate.hpp"
#include "NFmiQueryDataUtil.h"
#include "NFmiFileSystem.h"
#include "xmlliteutils/UtfConverter.h"
#include "NFmiFileString.h"


// *****************************************************
// *******  NFmiSeaIcingWarningMessage  ****************
// *****************************************************

NFmiSeaIcingWarningMessage::NFmiSeaIcingWarningMessage(void)
:itsTime()
,itsMessageStr()
,itsIdStr()
,itsSprayStr()
,itsIntensity1Str()
,itsIntensity2Str()
,itsAirKJStr()
,itsVisKMStr()
,itsWSStr()
,itsIceSpeedStr()
,itsTwStr()
,itsTemperatureStr()
,itsWaveStr()
,itsLatlonPoint()
,itsTotalMessageStr()
{
}

NFmiSeaIcingWarningMessage::~NFmiSeaIcingWarningMessage(void)
{
}

void NFmiSeaIcingWarningMessage::Clear(void)
{
	itsTime = NFmiMetTime::gMissingTime;
	itsMessageStr.clear();
	itsIdStr.clear();
	itsSprayStr.clear();
	itsIntensity1Str.clear();
	itsIntensity2Str.clear();
	itsAirKJStr.clear();
	itsVisKMStr.clear();
	itsWSStr.clear();
	itsIceSpeedStr.clear();
	itsTwStr.clear();
	itsTemperatureStr.clear();
	itsWaveStr.clear();
	itsLatlonPoint = NFmiPoint::gMissingLatlon;
	itsTotalMessageStr.clear();
}

static NFmiMetTime GetSeaIcingTime(const std::string &thePvmStr, const std::string &theUtcStr, const std::string &theOrigMsgStr)
{
// theUtcStr on muotoa: 2008-01-02
	std::vector<short> pvmVec = NFmiStringTools::Split<std::vector<short> >(thePvmStr, "-");
// theUtcStr on muotoa: 23:00
    std::vector<short> utcVec = NFmiStringTools::Split<std::vector<short> >(theUtcStr, ":");

	if(pvmVec.size() == 3 && utcVec.size() == 2)
	{
		return NFmiMetTime(pvmVec[0], pvmVec[1], pvmVec[2], utcVec[0], utcVec[1], 0, 1);
	}
	else
		throw std::runtime_error(std::string("Error in message's pvm or utc time string in GetSeaIcingTime-function:\n") + theOrigMsgStr);
}

/*
<?xml version="1.0" encoding="UTF-8"?>
<form1>
<data>
<lat>064.25</lat>
<long>021.30</long>
<pvm>2008-01-02</pvm>
<utc>23:00</utc>
<tunnus/>
<parskej>1</parskej>
<intens1/>
<intens2>102</intens2>
<ilmakj>0</ilmakj>
<naky_km/>
<tuuli>10.00</tuuli>
<jaa_nop>IB</jaa_nop>
<merivesi/>
<ilma>-4.00</ilma>
<aallokko/>
<kert_j/>
<time_1>23:00</time_1>
<time_2/>
<viesti>Hiljennetty nopeutta heti ja j√Ä√Ätyminen jatkuu edelleen Selk√Ämerell√Ä ja jatkuu todenn√Äk√∂isesti ensi y√∂n.Lisatietoja saa tali@eslshipping.fi</viesti>
<TextField1>A L U K S E L T A :</TextField1>
</data>
</form1>
*/
void NFmiSeaIcingWarningMessage::InitializeFromFormNode(LPXNode theNode)
{
	Clear();
	itsTotalMessageStr = CT2A(theNode->GetXML());
	std::string pvmStr = XmlLiteUtils::ChildNodeValue(theNode, "pvm"); // <pvm>2008-01-02</pvm>
	std::string utcStr = XmlLiteUtils::ChildNodeValue(theNode, "utc"); // <utc>23:00</utc>
	itsTime = ::GetSeaIcingTime(pvmStr, utcStr, itsTotalMessageStr);
	std::string lonStr = XmlLiteUtils::ChildNodeValue(theNode, "long");
	std::string latStr = XmlLiteUtils::ChildNodeValue(theNode, "lat");
	NFmiStringTools::ReplaceChars(lonStr, ',', '.'); // jos lat/lon arvot annettu ',' desimaali merkill‰, muutetaan se ensin '.'-merkiksi
	NFmiStringTools::ReplaceChars(latStr, ',', '.'); // jos lat/lon arvot annettu ',' desimaali merkill‰, muutetaan se ensin '.'-merkiksi
	itsLatlonPoint = NFmiPoint(NFmiStringTools::Convert<double>(lonStr), NFmiStringTools::Convert<double>(latStr));
	itsMessageStr = XmlLiteUtils::ChildNodeValue(theNode, "viesti");
	itsIdStr = XmlLiteUtils::ChildNodeValue(theNode, "tunnus");
	itsSprayStr = XmlLiteUtils::ChildNodeValue(theNode, "parskej");
	itsIntensity1Str = XmlLiteUtils::ChildNodeValue(theNode, "intens1");
	itsIntensity2Str = XmlLiteUtils::ChildNodeValue(theNode, "intens2");
	itsAirKJStr = XmlLiteUtils::ChildNodeValue(theNode, "ilmakj");
	itsVisKMStr = XmlLiteUtils::ChildNodeValue(theNode, "naky_km");
	itsWSStr = XmlLiteUtils::ChildNodeValue(theNode, "tuuli");
	itsIceSpeedStr = XmlLiteUtils::ChildNodeValue(theNode, "jaa_nop");
	itsTwStr = XmlLiteUtils::ChildNodeValue(theNode, "merivesi");
	itsTemperatureStr = XmlLiteUtils::ChildNodeValue(theNode, "ilma");
	itsWaveStr = XmlLiteUtils::ChildNodeValue(theNode, "aallokko");
}

// **** NFmiSeaIcingWarningSystem ***********************************

NFmiSeaIcingWarningSystem::NFmiSeaIcingWarningSystem(void)
:itsMessages()
,itsMessageFilePattern()
,itsLatestReadFileTimeStamp()
,itsUpdateTimeStepInMinutes(60)
,fViewVisible(false)
,itsDefaultSymbolInfo()
,itsLastErrorWhenReadingMessages()
,itsErrorFilePath()
,fIsHidden(true)
{
}

NFmiSeaIcingWarningSystem::~NFmiSeaIcingWarningSystem(void)
{
}

void NFmiSeaIcingWarningSystem::Clear(bool fClearOnlyMessages)
{
	itsMessages.clear();
	if(fClearOnlyMessages == false)
		itsLatestReadFileTimeStamp = 0;
}

int NFmiSeaIcingWarningSystem::UpdateTimeStepInMinutes(void) const
{
	if(fIsHidden)
		return -1; // jos piilotettu, palautetaan negatiivinen arvo, jolloin haku rutiineja ei k‰ynnistet‰
	else
		return itsUpdateTimeStepInMinutes;
}

void NFmiSeaIcingWarningSystem::Add(std::set<NFmiSeaIcingWarningMessage> &theNewMessages, time_t theTimeStamp)
{
	itsMessages.insert(theNewMessages.begin(), theNewMessages.end());
	itsLatestReadFileTimeStamp = theTimeStamp;
}

void NFmiSeaIcingWarningSystem::InitializeFromSettings(const std::string &theSettingsKeyBaseStr)
{
	Clear(false);

    std::string filePatternStr = NFmiSettings::Optional<std::string>(theSettingsKeyBaseStr + "FilePattern", "");
    if(filePatternStr.empty())
        return;
    else
    {
        MessageFilePattern(filePatternStr);
        MakeErrorPath();
        UpdateTimeStepInMinutes(NFmiSettings::Optional<int>(theSettingsKeyBaseStr + "UpdateTimeStepInMinutes", 5));

        // symbolinfo stringit ovat muotoa:
        // v‰ri:symbolID:kokoPikseleiss‰_x_y:haly_luokka:k‰yt‰_n‰yt‰:min_shown_level
        // 128,0,255:2:12,14:413:1:2
        std::string symbolInfoDefault = NFmiSettings::Optional<std::string>(theSettingsKeyBaseStr + "SymbolInfoDefault", "128,0,255:2:12,14:413:1:2");
        if(!symbolInfoDefault.empty())
            DefaultSymbolInfo(HakeLegacySupport::WarningSymbolInfo::GetWarningSymbolInfo(symbolInfoDefault));

        // Default behaviour is to show the control when the corresponding configuration option is missing.
        std::string value = NFmiSettingsImpl::Instance().Value("MetEditor::Toolbar::SeaIcingWarningsDlg", "Show");
        fIsHidden = boost::iequals(value, "hide");
    }
}

bool NFmiSeaIcingWarningSystem::DoMessageSearching() const
{
    // K‰ynnistet‰‰nkˆ sanomien haku threadi vai ei. Vain Suomessa k‰ytˆss‰, joten muissa maissa turhaa. 
    // itsMessageFilePattern arvo on optionaalinen (InitializeFromSettings metodissa), jos tyhj‰, ei tehd‰ hakuja.
    return !itsMessageFilePattern.empty(); 
}

// t‰ss‰ asetellaan l‰hinn‰ piirto-ominasuuksia
void NFmiSeaIcingWarningSystem::Init(const NFmiSeaIcingWarningSystem &theData, bool doFullInit)
{
	itsUpdateTimeStepInMinutes = theData.itsUpdateTimeStepInMinutes;
	fViewVisible = theData.fViewVisible;
	itsDefaultSymbolInfo = theData.itsDefaultSymbolInfo;

	if(doFullInit)
	{
		itsMessages = theData.itsMessages;
		itsMessageFilePattern = theData.itsMessageFilePattern;
		itsLatestReadFileTimeStamp = theData.itsLatestReadFileTimeStamp;
		itsLastErrorWhenReadingMessages = theData.itsLastErrorWhenReadingMessages;
		itsErrorFilePath = theData.itsErrorFilePath;
		fIsHidden = theData.fIsHidden;
	}
}

void NFmiSeaIcingWarningSystem::StoreSettings(const std::string &theSettingsKeyBaseStr)
{
	NFmiSettings::Set(theSettingsKeyBaseStr + "FilePattern", MessageFilePattern(), true);
	NFmiSettings::Set(theSettingsKeyBaseStr + "UpdateTimeStepInMinutes", NFmiStringTools::Convert(UpdateTimeStepInMinutes()), true);

	// symbolinfo stringi on muotoa:
	// v‰ri:symbolID:kokoPikseleiss‰_x_y:haly_luokka:k‰yt‰_n‰yt‰:min_shown_level
	// 128,0,255:2:12,14:413:1:2
	NFmiSettings::Set(theSettingsKeyBaseStr + "SymbolInfoDefault", HakeLegacySupport::WarningSymbolInfo::MakeWarningSymbolInfoStr(DefaultSymbolInfo()), true);
}

void NFmiSeaIcingWarningSystem::DecodeMessages(const std::string &theXMLMessageStr, const std::string &theFileName)
{
	if(theXMLMessageStr.empty() == false)
	{
        CString sxmlU_(CA2T(theXMLMessageStr.c_str()));
		XNode xmlRoot;
		PARSEINFO pi;
        if(xmlRoot.Load(sxmlU_, &pi) == false)
		{
			if(pi.erorr_occur)
			{
				std::string errMsg("NFmiSeaIcingWarningMessage::DecodeMessage - xmlRoot.Load(sxml) failed:\n");
				errMsg += CT2A(pi.error_string);
				errMsg += "\nFailed for message:\n";
				errMsg += theXMLMessageStr;
				throw std::runtime_error(errMsg);
			}
			else
				throw std::runtime_error(std::string("NFmiSeaIcingWarningMessage::DecodeMessage - xmlRoot.Load(sxml) failed for string: \n") + theXMLMessageStr + "In file: " + theFileName);
		}

		XNodes nodes = xmlRoot.GetChilds(_TEXT("data"));
		for(size_t i=0; i<nodes.size(); i++)
		{
			LPXNode aNode = nodes[i];

			NFmiSeaIcingWarningMessage tmpData;
			tmpData.InitializeFromFormNode(aNode);
			itsMessages.insert(tmpData);
		}
	}
}

struct FindTimeGreaterOrEqualSeaIceMsg
{
	NFmiMetTime itsWantedTime;

	FindTimeGreaterOrEqualSeaIceMsg(const NFmiMetTime &theWantedTime)
	:itsWantedTime(theWantedTime)
	{
	}

	bool operator()(const NFmiSeaIcingWarningMessage &theMessage)
	{
		if(theMessage.Time() >= itsWantedTime)
			return true;
		else
			return false;
	}
};


// etsii 1. sanoman, jonka aloitus aika on >= kuin annettu aika tai setin lopun
std::set<NFmiSeaIcingWarningMessage>::iterator NFmiSeaIcingWarningSystem::Find(const NFmiMetTime &theStartTime)
{
	return std::find_if(itsMessages.begin(), itsMessages.end(), FindTimeGreaterOrEqualSeaIceMsg(theStartTime));
}

typedef std::list<std::pair<std::string, std::time_t> > FileListWithTimes;

// oletus theErrorPath p‰‰ttyy / tai \ merkkiin.
static void MoveFileToDirectory(const NFmiFileString &theFileNameWithPath, const std::string &theErrorPath)
{
    NFmiFileSystem::CreateDirectory(theErrorPath);
    std::string errFileName(theErrorPath);
    errFileName += static_cast<char*>(theFileNameWithPath.FileName());
    NFmiFileSystem::RenameFile(static_cast<char*>(theFileNameWithPath), errFileName);
}

bool NFmiSeaIcingWarningSystem::CheckForNewMessages(NFmiStopFunctor *theStopFunctor)
{
	NFmiQueryDataUtil::CheckIfStopped(theStopFunctor);
	itsLastErrorWhenReadingMessages = "";
	bool status = false; // palauttaa true, jos lˆytyi uusia HALY viestej‰
	FileListWithTimes files = NFmiFileSystem::PatternFiles(itsMessageFilePattern, itsLatestReadFileTimeStamp);

	NFmiQueryDataUtil::CheckIfStopped(theStopFunctor);
	time_t tmpTimeStamp = itsLatestReadFileTimeStamp;
	FileListWithTimes::iterator it = files.begin();
	FileListWithTimes::iterator endIt = files.end();
	for( ; it != endIt; ++it)
	{
		try
		{
			if(itsLatestReadFileTimeStamp < (*it).second) // ei lueta viesti‰, jos viimeisin aikaleima on tuoreempi kuin kyseisen tiedoston
			{
				std::string fileName = (*it).first;
				std::string messageStr;
				NFmiFileSystem::ReadFile2String(fileName, messageStr);
            	std::string asciiMessageStr = UtfConverter::ConvertUtf_8ToString(messageStr);
				NFmiStringTools::TrimAll(asciiMessageStr, true); // trimmataan mahdolliset rivin vaihdot pois xml tageist‰, ett‰ XML-lite selviytyy parseroinnista
				NFmiQueryDataUtil::CheckIfStopped(theStopFunctor);

				DecodeMessages(asciiMessageStr, fileName); // purkaa ja lis‰‰ message-listoihin sanomat
				if(tmpTimeStamp < (*it).second)
					tmpTimeStamp = (*it).second; // otetaan myˆh‰isin aika talteen v‰liaikaiseen aikaleimaan
				status = true;
			}
		}
		catch(std::exception &e)
		{ // t‰ll‰ lastError virityksell‰ p‰‰st‰‰n jatkamaan lukua, vaikka yksi tai useampi viesteist‰ olisikin virheellinen
			itsLastErrorWhenReadingMessages = e.what();
			::MoveFileToDirectory(NFmiFileString((*it).first), itsErrorFilePath);
			itsLastErrorWhenReadingMessages += "\nMoving errorneus file '";
			itsLastErrorWhenReadingMessages += (*it).first;
			itsLastErrorWhenReadingMessages += "' to error-directory: ";
			itsLastErrorWhenReadingMessages += itsErrorFilePath;
		}
	}
	itsLatestReadFileTimeStamp = tmpTimeStamp; // talletetaan lopuksi temppi muuttujaan talletettu aikaleima muistiin
	return status;
}


// tehd‰‰n virhehakemisto kyseiseen viesti hakemistoon error-nimisen‰
void NFmiSeaIcingWarningSystem::MakeErrorPath(void)
{
	itsErrorFilePath = "";
	NFmiFileString fileString(itsMessageFilePattern);
	if(fileString.IsAbsolutePath())
		itsErrorFilePath += fileString.Device();
	itsErrorFilePath += fileString.Path();
	itsErrorFilePath += "error";
	itsErrorFilePath += kFmiDirectorySeparator;
}

std::vector<NFmiSeaIcingWarningMessage*> NFmiSeaIcingWarningSystem::GetWantedWarningMessages(const NFmiMetTime &theLastTime, int theStepInMinutes, const boost::shared_ptr<NFmiArea> &theArea)
{
	std::vector<NFmiSeaIcingWarningMessage*> messages;

	// Lasketaan time1 ja time2, jotka m‰‰rittelev‰t aikav‰lin, jonka v‰liselt‰ ajalta n‰ytet‰‰n kartalla HALY-viestien symbolit
	NFmiMetTime time2(theLastTime);
	NFmiMetTime time1(time2);
	time1.SetTimeStep(1);
	time1.ChangeByMinutes(-theStepInMinutes);

	std::set<NFmiSeaIcingWarningMessage>::iterator it = Find(time1);
	std::set<NFmiSeaIcingWarningMessage>::iterator endIt = GetMessages().end();

	for( ; it != endIt; ++it)
	{
		if((*it).Time() > time2)
			break; // ei menn‰ pitemm‰lle kuin time2
		if(theArea->IsInside((*it).LatlonPoint()) == false)
			continue;
		// const_cast on MSVC++ 2010 Beta 2 k‰‰nt‰j‰n bugin pakottama kasti (olettaa ett‰ it-olio palauttaa const-olion, vaikka ei ole)
		messages.push_back(const_cast<NFmiSeaIcingWarningMessage*>(&(*it)));
	}
	return messages;
}
