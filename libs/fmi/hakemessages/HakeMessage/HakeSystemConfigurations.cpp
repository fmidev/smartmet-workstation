#include "HakeSystemConfigurations.h"
#include "NFmiSettings.h"
#include "NFmiDataStoringHelpers.h"
#include "NFmiStringTools.h"
#include "HakeMessage/HakeMsg.h"

#include "boost/algorithm/string/predicate.hpp"
#include "boost/math/special_functions/round.hpp"
#include <boost/algorithm/string.hpp>

namespace
{
    // Siivotaan p‰ist‰ white space:t. Tarkistetaan sitten ett‰ ei ole j‰‰nyt tyhj‰‰ string:ia.
    bool cleanAndCheckMessageTypeInfoInputString(std::string &messageTypeInfoString)
    {
        NFmiStringTools::TrimAll(messageTypeInfoString);
        return !messageTypeInfoString.empty();
    }

    // Tehd‰‰n stringi, jossa eri MessageTypeInfo -> string osiot on eroteltu '|' merkill‰.
    std::string makeMessageTypeInfoVectorString(const std::vector<HakeLegacySupport::MessageTypeInfo> &messageTypeInfos)
    {
        std::string str;
        for(const auto &messageTypeInfo : messageTypeInfos)
        {
            if(!str.empty())
                str += HakeLegacySupport::g_OuterSeparatorString;
            str += messageTypeInfo.to_string();
        }
        return str;
    }

    std::vector<HakeLegacySupport::MessageTypeInfo> makeMessageTypeInfoVectorFromString(const std::string &messageTypeInfosString)
    {
        std::vector<HakeLegacySupport::MessageTypeInfo> messageTypeInfos;
        std::vector<std::string> stringParts;
        boost::split(stringParts, messageTypeInfosString, boost::is_any_of(HakeLegacySupport::g_OuterSeparatorString));
        for(auto &messageString : stringParts)
        {
            if(::cleanAndCheckMessageTypeInfoInputString(messageString))
            {
                HakeLegacySupport::MessageTypeInfo messageTypeInfo;
                messageTypeInfo.from_string(messageString);
                messageTypeInfos.push_back(messageTypeInfo);
            }
        }

        return messageTypeInfos;
    }

}

namespace HakeLegacySupport
{

    HakeSystemConfigurations::HakeSystemConfigurations(void)
        : itsUpdateTimeStepInMinutes(-1)
        , fWarningCenterViewOn(false)
        , fShowAllMessages(false)
        , itsSymbolInfos()
        , itsDefaultSymbolInfo()
        , itsMessageTypeInfos()
    {
    }

    HakeSystemConfigurations::~HakeSystemConfigurations(void)
    {
    }

    double HakeSystemConfigurations::UpdateTimeStepInMinutes(void) const
    {
        if(fIsHidden)
            return -1.; // jos piilotettu, palautetaan negatiivinen arvo, jolloin haku rutiineja ei k‰ynnistet‰
        else
            return itsUpdateTimeStepInMinutes;
    }

    // t‰m‰ palauttaa default symbolInfon, jos ei muuta lˆydy
    WarningSymbolInfo* HakeSystemConfigurations::FindSymbolInfo(int theCategory)
    {
        std::vector<WarningSymbolInfo>::iterator it = FindNormalSymbolInfo(theCategory);
        if(it != itsSymbolInfos.end())
            return &(*it);
        else
            return &itsDefaultSymbolInfo;
    }

    std::vector<WarningSymbolInfo>::iterator HakeSystemConfigurations::FindNormalSymbolInfo(int theCategory)
    {
        return std::find_if(itsSymbolInfos.begin(), itsSymbolInfos.end(), [category=theCategory](const auto &symbolInfo) {return symbolInfo.MessageCategory() == category; });
    }

    void HakeSystemConfigurations::Add(const WarningSymbolInfo &theSymbolInfo)
    {
        itsSymbolInfos.push_back(theSymbolInfo);
    }

    void HakeSystemConfigurations::ClearSymbolInfos(void)
    {
        itsSymbolInfos.clear();
    }

    void HakeSystemConfigurations::InitializeFromSettings(const std::string &theBaseNamespace)
    {
        itsBaseNamespace = theBaseNamespace;
        ClearSymbolInfos();

        ShowAllMessages(NFmiSettings::Optional<bool>(itsBaseNamespace + "ShowAllMessages", true));

        std::string defaultSymbolInfoString = "128,0,255:2:12,14:413:1:2";

        std::string symbolInfoDefault = NFmiSettings::Optional<std::string>(itsBaseNamespace + "SymbolInfoDefault", defaultSymbolInfoString);
        DefaultSymbolInfo(WarningSymbolInfo::GetWarningSymbolInfo(symbolInfoDefault));
        // symbolinfo stringit ovat muotoa:
        // v‰ri:symbolID:kokoPikseleiss‰_x_y:haly_luokka:k‰yt‰_n‰yt‰:min_shown_level
        // 128,0,255:2:12,14:413:1:2
        // symboliinfoja on m‰‰r‰ttym‰‰r‰ (nyt 10)
        std::string symbolInfo1 = NFmiSettings::Optional<std::string>(itsBaseNamespace + "SymbolInfo1", defaultSymbolInfoString);
        Add(WarningSymbolInfo::GetWarningSymbolInfo(symbolInfo1));
        std::string symbolInfo2 = NFmiSettings::Optional<std::string>(itsBaseNamespace + "SymbolInfo2", defaultSymbolInfoString);
        Add(WarningSymbolInfo::GetWarningSymbolInfo(symbolInfo2));
        std::string symbolInfo3 = NFmiSettings::Optional<std::string>(itsBaseNamespace + "SymbolInfo3", defaultSymbolInfoString);
        Add(WarningSymbolInfo::GetWarningSymbolInfo(symbolInfo3));
        std::string symbolInfo4 = NFmiSettings::Optional<std::string>(itsBaseNamespace + "SymbolInfo4", defaultSymbolInfoString);
        Add(WarningSymbolInfo::GetWarningSymbolInfo(symbolInfo4));
        std::string symbolInfo5 = NFmiSettings::Optional<std::string>(itsBaseNamespace + "SymbolInfo5", defaultSymbolInfoString);
        Add(WarningSymbolInfo::GetWarningSymbolInfo(symbolInfo5));
        std::string symbolInfo6 = NFmiSettings::Optional<std::string>(itsBaseNamespace + "SymbolInfo6", defaultSymbolInfoString);
        Add(WarningSymbolInfo::GetWarningSymbolInfo(symbolInfo6));
        std::string symbolInfo7 = NFmiSettings::Optional<std::string>(itsBaseNamespace + "SymbolInfo7", defaultSymbolInfoString);
        Add(WarningSymbolInfo::GetWarningSymbolInfo(symbolInfo7));
        std::string symbolInfo8 = NFmiSettings::Optional<std::string>(itsBaseNamespace + "SymbolInfo8", defaultSymbolInfoString);
        Add(WarningSymbolInfo::GetWarningSymbolInfo(symbolInfo8));
        std::string symbolInfo9 = NFmiSettings::Optional<std::string>(itsBaseNamespace + "SymbolInfo9", defaultSymbolInfoString);
        Add(WarningSymbolInfo::GetWarningSymbolInfo(symbolInfo9));
        std::string symbolInfo10 = NFmiSettings::Optional<std::string>(itsBaseNamespace + "SymbolInfo10", defaultSymbolInfoString);
        Add(WarningSymbolInfo::GetWarningSymbolInfo(symbolInfo10));

        // Default behaviour is to show the control when the corresponding configuration option is missing.
        std::string value = NFmiSettings::Optional<std::string>("MetEditor::Toolbar::WarningCenterDlg", "Hide");
        fIsHidden = boost::iequals(value, "hide");

        std::string messagesInfoStr = NFmiSettings::Optional<std::string>(itsBaseNamespace + "WarningMessagesInfo", "");
        itsMessageTypeInfos = ::makeMessageTypeInfoVectorFromString(messagesInfoStr);
    }

    // Huom! t‰m‰ asettaa kaikki talletettavat asiat settingseihin, mutta ei tee save:a.
    void HakeSystemConfigurations::StoreSettings(void)
    {
        NFmiSettings::Set(itsBaseNamespace +"ShowAllMessages", NFmiStringTools::Convert(static_cast<int>(ShowAllMessages())), true);

        // symbolinfo stringit ovat muotoa:
        // v‰ri:symbolID:kokoPikseleiss‰_x_y:haly_luokka:k‰yt‰_n‰yt‰:min_shown_level
        // 128,0,255:2:12,14:413:1:2
        NFmiSettings::Set(itsBaseNamespace + "SymbolInfoDefault", WarningSymbolInfo::MakeWarningSymbolInfoStr(DefaultSymbolInfo()), true);
        std::string settingNameBase(itsBaseNamespace + "SymbolInfo");
        for(unsigned int i = 0; i < itsSymbolInfos.size(); i++)
        {
            std::string settingName(settingNameBase);
            settingName += NFmiStringTools::Convert<int>(i + 1);
            NFmiSettings::Set(settingName, WarningSymbolInfo::MakeWarningSymbolInfoStr(itsSymbolInfos[i]), true);
        }

        std::string hakeMessagesInfoStr = ::makeMessageTypeInfoVectorString(itsMessageTypeInfos);
        NFmiSettings::Set(itsBaseNamespace + "WarningMessagesInfo", hakeMessagesInfoStr, true);
    }

    // t‰ss‰ asetellaan l‰hinn‰ piirto-ominasuuksia
    // eli samat jutut kuin Read/Write metodeissa
    void HakeSystemConfigurations::Init(const HakeSystemConfigurations &theData, bool doFullInit)
    {
        itsUpdateTimeStepInMinutes = theData.itsUpdateTimeStepInMinutes;
        fWarningCenterViewOn = theData.fWarningCenterViewOn;
        fShowAllMessages = theData.fShowAllMessages;
        itsSymbolInfos = theData.itsSymbolInfos;
        itsDefaultSymbolInfo = theData.itsDefaultSymbolInfo;

        if(doFullInit)
        {
            fIsHidden = theData.fIsHidden;
        }
    }

    NFmiMetTime HakeSystemConfigurations::MakeStartTimeForHakeMessages(const NFmiMetTime &theMapTime, int theMapTimeStepInMinutes)
    {
        NFmiMetTime startTime(theMapTime);
        startTime.ChangeByMinutes(-theMapTimeStepInMinutes);
        return startTime;
    }

    // HUOM!! T‰m‰ laittaa kommentteja mukaan!
    void HakeSystemConfigurations::Write(std::ostream& os) const
    {
        os << "// HakeSystemConfigurations::Write..." << std::endl;

        os << "// UpdateTimeStepInMinutes + WarningCenterViewOn + ShowAllMessages" << std::endl;
        os << boost::math::iround(itsUpdateTimeStepInMinutes) << " " << fWarningCenterViewOn << " " << fShowAllMessages << std::endl;

        os << "// Container<SymbolInfos>" << std::endl;
        NFmiDataStoringHelpers::WriteContainer(itsSymbolInfos, os, std::string("\n"));

        os << "// DefaultSymbolInfo" << std::endl;
        os << itsDefaultSymbolInfo << std::endl;

        NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
                                                                // Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
                                                                // edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.
        extraData.Add(::makeMessageTypeInfoVectorString(itsMessageTypeInfos)); // 1. extra string parametri
        os << "// possible extra data" << std::endl;
        os << extraData;

        if(os.fail())
            throw std::runtime_error("NFmiWarningCenterSystem::Write failed");
    }

    // HUOM!! ennen kuin t‰m‰ luokka luetaan sis‰‰n tiedostosta, poista kommentit
    // NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
    // sekaan. Eli ‰l‰ k‰yt‰ suoraan t‰t‰ metodia, vaan Init(filename)-metodia!!!!
    void HakeSystemConfigurations::Read(std::istream& is)
    {
        is >> itsUpdateTimeStepInMinutes >> fWarningCenterViewOn >> fShowAllMessages;

        if(is.fail())
            throw std::runtime_error("HakeSystemConfigurations::Read failed");
        NFmiDataStoringHelpers::ReadContainer(itsSymbolInfos, is);

        if(is.fail())
            throw std::runtime_error("HakeSystemConfigurations::Read failed");
        is >> itsDefaultSymbolInfo;

        if(is.fail())
            throw std::runtime_error("HakeSystemConfigurations::Read failed");

        NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
        is >> extraData;
        // T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
        // eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.
        if(extraData.itsStringValues.size() > 0)
            itsMessageTypeInfos = ::makeMessageTypeInfoVectorFromString(extraData.itsStringValues[0]); // 1. extra string parametri

        if(is.fail())
            throw std::runtime_error("HakeSystemConfigurations::Read failed");
    }

    // Lis‰t‰‰n annettu messageTypeInfo sille kuuluvaan vektoriin ja sitten sortataan sanoma infot viel‰ j‰rjestykseen.
    void HakeSystemConfigurations::AddMessageTypeInfo(const MessageTypeInfo &messageTypeInfo)
    {
        itsMessageTypeInfos.push_back(messageTypeInfo);
        SortMessageTypeInfoVector();
    }

    MessageTypeInfo* HakeSystemConfigurations::GetMessageTypeInfo(size_t index)
    {
        if(index < itsMessageTypeInfos.size())
        {
            return &itsMessageTypeInfos[index];
        }
        return nullptr;
    }


    void HakeSystemConfigurations::RemoveMessageTypeInfo(size_t index)
    {
        if(index < itsMessageTypeInfos.size())
        {
            itsMessageTypeInfos.erase(itsMessageTypeInfos.begin() + index);
        }
    }

    void HakeSystemConfigurations::SortMessageTypeInfoVector()
    {
        std::sort(itsMessageTypeInfos.begin(), itsMessageTypeInfos.end());
    }

    bool HakeSystemConfigurations::IsMessageShown(const HakeMessage::HakeMsg &message)
    {
        if(!ShowAllMessages())
        {
            auto iter = std::find_if(itsMessageTypeInfos.begin(), itsMessageTypeInfos.end(), [&message](const MessageTypeInfo &info) 
            {
                return info.messageCategory() == message.Category();
            });
            if(iter != itsMessageTypeInfos.end())
                return iter->show();
        }
        return true;
    }

} // namespace HakeLegacySupport
