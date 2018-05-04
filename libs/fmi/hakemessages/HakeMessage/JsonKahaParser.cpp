#include "HakeMessage/JsonKahaParser.h"
#include "HakeMessage/KahaMsg.h"

#include "NFmiMetTime.h"

#include "json.hpp"

#include <cppext/split.h>

namespace HakeMessage
{
    namespace
    {
        using Json = nlohmann::json;

        NFmiMetTime kahaTimeToMetTime(const std::string& timeStr)
        {
            //"2017-06-20 15:15:00"
            auto dateTime = cppext::split(timeStr, ' ');

            auto dateSubStrings = cppext::split(dateTime[0], '-');
            if(dateSubStrings.size() != 3)
            {
                return NFmiMetTime::gMissingTime;
            }
            auto year = std::stoi(dateSubStrings[0]);
            auto month = std::stoi(dateSubStrings[1]);
            auto day = std::stoi(dateSubStrings[2]);

            auto hourMinSec = cppext::split(dateTime[1], ':');
            auto hour = std::stoi(hourMinSec[0]);
            auto min = std::stoi(hourMinSec[1]);
            auto sec = std::stoi(hourMinSec[2]);

            return NFmiMetTime{ NFmiTime(year, month, day, hour, min, sec), 1 };
        }

        int calcKahaCategoryIdForRain(KahaMsg &kahaMsg, const Json &properties)
        {
            if(properties.find("smartmet") != properties.end())
            {
                const auto& smartmet = properties["smartmet"];
                if(smartmet.is_array())
                {
                    // ei sadetta tapausksessa on taulukossa 1 kohta 
                    if(smartmet.size() == 1)
                        return 1100; // ei-sadetta koodi on 1100
                    else if(smartmet.size() == 2)
                    {
                        // Taulukon 1. osassa on olomuoto
                        auto choiceLevel1 = smartmet[0];
                        int precipitationForm = std::stoi(choiceLevel1["choice_id"].get<std::string>());
                        // Taulukon 2. osassa on intensiteetti
                        auto choiceLevel2 = smartmet[1];
                        int precipitationIntensity = std::stoi(choiceLevel2["choice_id"].get<std::string>());
                        // uudeksi categoriaksi tehd‰‰n 4 numeroinen luku 11fi, miss‰ f on olomuoto*10 ja i on intensiteetti
                        return kahaMsg.Category() * 100 + precipitationForm * 10 + precipitationIntensity;
                    }
                }
            }

            return 0;
        }

        // Kaha sanomilla on ik‰v‰ tapa laittaa samalla id:lla erilaisia sataa/ei sada tilanteita.
        // Meteorologisesti eri tapaukset pit‰‰ erotella omiksi id:ksi riippuen sataako vai ei. Ja jos sataa, kuinka rankasti.
        // Lis‰ksi uudemmassa Weather applikaatiossa on viel‰ mukana sateen olomuoto, joka pit‰‰ myˆs erotella.
        void fixHakaCategoryId(KahaMsg &kahaMsg, const Json &properties)
        {
            if(kahaMsg.Category() == 11)
                kahaMsg.Category(calcKahaCategoryIdForRain(kahaMsg, properties));
        }
    }

    KahaMsg JsonKahaParser::parseKaha(const std::string& jsonString)
    {
        auto kahaMsg = KahaMsg{};

        auto json = Json::parse(jsonString);
        const auto& properties = json["properties"];

        kahaMsg.Number(std::to_string(properties["id"].get<int>()));
        kahaMsg.MessageStr(properties["title"]);
        kahaMsg.StartTime(kahaTimeToMetTime(properties["utctime"]));

        kahaMsg.Category(properties["question_id"].get<int>());

        const auto& coordinates = json["geometry"]["coordinates"];

        kahaMsg.LatlonPoint(NFmiPoint(coordinates[0], coordinates[1]));
        fixHakaCategoryId(kahaMsg, properties);

        return kahaMsg;
    }
}