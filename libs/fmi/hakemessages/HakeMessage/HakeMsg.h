#pragma once


#include "NFmiMetTime.h"
#include "NFmiPoint.h"

namespace HakeMessage {
    class HakeMsg
    {
        NFmiMetTime itsStartTime = NFmiMetTime::gMissingTime; // esim.  <Ilmoitusaika>24.12.2005 12:25:14</Ilmoitusaika>
        NFmiMetTime itsSendingTime = NFmiMetTime::gMissingTime; // esim. <Lahetysaika>24.12.2005 12:35:21</Lahetysaika>
        std::string itsNumber = "Missing"; // esim. <TehtavaNro>05260060</TehtavaNro>
        std::string itsTypeStr; // esim. <Tehtavalaji>461 vahingontorjunta: pieni</Tehtavalaji>
        int itsCategory = 0; // haly viestin kategoorinen id numero esim. 416
        int itsMessageLevel = 1; // 0=ei alustettu, 1=pieni, 2=normaali, 3=suuri
        std::string itsMessageStr; // esimerkissä ei ollut mitään?!?!?
        std::string itsReasonStr; // syy tapahtumalle
        std::string itsMessageCenterId; // esim. <HatakeskusID>6</HatakeskusID>
        std::string itsCountyStr; // esim. <Kunta>HATTULA</Kunta>
        std::string itsCityDistrictStr; // esim. <Kaupunginosa>PAROLA</Kaupunginosa>
        std::string itsAddress; // esim. <Katu>TEOLLISUUSTIE</Katu>
        std::string itsAddress2; // esim. <Katu2>MERVENTIE</Katu2>
        NFmiPoint itsLatlonPoint = NFmiPoint::gMissingLatlon; // otetaan <KoordX>3356592.9</KoordX> ja <KoordY>6775576.18</KoordY> nodeista
        std::string itsTotalMessageStr; // koko message xml löpinä
    public:
        static const HakeMsg unInitialized;

        const NFmiMetTime& StartTime() const;
        void StartTime(const NFmiMetTime &newValue);

        const NFmiMetTime& SendingTime() const;
        void SendingTime(const NFmiMetTime &newValue);

        const std::string& Number() const;
        void Number(const std::string &newValue);

        const std::string& TypeStr() const;
        void TypeStr(const std::string &newValue);

        int Category() const;
        void Category(int newValue);

        int MessageLevel() const;
        void MessageLevel(int newValue);

        const std::string& MessageStr() const;
        void MessageStr(const std::string &newValue);

        const std::string& ReasonStr() const;
        void ReasonStr(const std::string &newValue);

        const std::string& MessageCenterId() const;
        void MessageCenterId(const std::string& newValue);

        const std::string& CountyStr() const;
        void CountyStr(const std::string &newValue);

        const std::string& CityDistrictStr() const;
        void CityDistrictStr(const std::string &newValue);

        const std::string& Address() const;
        void Address(const std::string &newValue);

        const std::string& Address2() const;
        void Address2(const std::string &newValue);

        const NFmiPoint& LatlonPoint() const;
        void LatlonPoint(const NFmiPoint &newValue);

        const std::string& TotalMessageStr() const;
        void TotalMessageStr(const std::string &newValue);
    };

    inline bool operator==(const HakeMsg& msg1, const HakeMsg& msg2)
    {
        return msg1.Number() == msg2.Number();
    }

    inline bool operator<(const HakeMsg& msg1, const HakeMsg& msg2)
    {
        return msg1.StartTime().IsLessThan(msg2.StartTime());
    }
}