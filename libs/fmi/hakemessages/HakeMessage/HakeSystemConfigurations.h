#pragma once

#include <vector>
#include "WarningSymbolInfo.h"
#include "NFmiMetTime.h"
#include "MessageTypeInfo.h"

namespace HakeMessage
{
    class HakeMsg;
}

namespace HakeLegacySupport
{

    class HakeSystemConfigurations
    {
    public:
        HakeSystemConfigurations(void);
        ~HakeSystemConfigurations(void);
        void InitializeFromSettings(const std::string &theBaseNamespace);
        void StoreSettings(void);
        void Init(const HakeSystemConfigurations &theData, bool doFullInit = false);
        static NFmiMetTime MakeStartTimeForHakeMessages(const NFmiMetTime &theMapTime, int theMapTimeStepInMinutes);

        double UpdateTimeStepInMinutes(void) const;
        void UpdateTimeStepInMinutes(double newValue) { itsUpdateTimeStepInMinutes = newValue; }
        bool WarningCenterViewOn(void) const { return fWarningCenterViewOn; }
        void WarningCenterViewOn(bool newState) { fWarningCenterViewOn = newState; }
        void Add(const WarningSymbolInfo &theSymbolInfo);
        std::vector<WarningSymbolInfo>& SymbolInfos(void) { return itsSymbolInfos; }
        WarningSymbolInfo* FindSymbolInfo(int theCategory);
        WarningSymbolInfo& DefaultSymbolInfo(void) { return itsDefaultSymbolInfo; }
        void DefaultSymbolInfo(const WarningSymbolInfo &newValue) { itsDefaultSymbolInfo = newValue; }
        bool ShowAllMessages(void) const { return fShowAllMessages; }
        void ShowAllMessages(bool newValue) { fShowAllMessages = newValue; }
        bool IsHidden(void) const { return fIsHidden; }
        void IsHidden(bool newState) { fIsHidden = newState; }
        void ClearSymbolInfos(void);
        std::vector<MessageTypeInfo>& MessageTypeInfos() { return itsMessageTypeInfos; }
        void SetMessageTypeInfos(const std::vector<MessageTypeInfo> &messageTypeInfos) { itsMessageTypeInfos = messageTypeInfos; }
        void AddMessageTypeInfo(const MessageTypeInfo &messageTypeInfo);
        MessageTypeInfo* GetMessageTypeInfo(size_t index);
        void RemoveMessageTypeInfo(size_t index);
        void SortMessageTypeInfoVector();
        bool IsMessageShown(const HakeMessage::HakeMsg &message);

        // HUOM!! Tämä laittaa kommentteja mukaan!
        void Write(std::ostream& os) const;
        // HUOM!! ennen kuin tämä luokka luetaan sisään tiedostosta, poista kommentit
        // NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
        // sekaan. Eli älä käytä suoraan tätä metodia, vaan Init(filename)-metodia!!!!
        void Read(std::istream& is);

    private:
        std::vector<WarningSymbolInfo>::iterator FindNormalSymbolInfo(int theCategory);

        double itsUpdateTimeStepInMinutes; // kuinka usein tarkastetaan, onko tullut uusia sanomia. Jos <= 0, ei tehdä tarkistuksia ollenkaan
        bool fWarningCenterViewOn;
        bool fShowAllMessages; // tämä asetus laittaa kaikki mahdolliset varoitukset näkyviin niin kartalla kuin taulukko näytössä
        std::vector<WarningSymbolInfo> itsSymbolInfos; // miten eri haly-kategorioiden symbolit laitetaan kartalle/taulukkoon
        WarningSymbolInfo itsDefaultSymbolInfo; // jos halutaan katsoa kaikkia mahdollisia vatoituksia, käytetään silloin piirrossta näitä asetuksia
        bool fIsHidden; // jos toiminto on piilotettu konfiguraatioilla, laitetaan palauttamaan negatiivista timesteppiä, että SmartMet
        std::vector<MessageTypeInfo> itsMessageTypeInfos; // Tähän laitetaan tiedot kaikista käytössä olleista Hake + Kaha tyyppisistä sanomista (mm. on/off tieto)

        std::string itsBaseNamespace; // konffien pohja namespace, esim. "SmartMet::HakeMessages::"
    };

}

inline std::ostream& operator<<(std::ostream& os, const HakeLegacySupport::HakeSystemConfigurations& item) { item.Write(os); return os; }
inline std::istream& operator >> (std::istream& is, HakeLegacySupport::HakeSystemConfigurations& item) { item.Read(is); return is; }
