#include "SettingsFunctions.h"
#include "NFmiStringTools.h"
#include "NFmiSettings.h"
#include "NFmiDataMatrix.h"

namespace SettingsFunctions
{
    NFmiColor GetColorFromSettings(const std::string &theSettingKey, const NFmiColor *theOptionalColor)
    {
        bool isOptional = theOptionalColor != 0;
        NFmiColor tColor;
        std::string tmp = (isOptional) ? NFmiSettings::Optional<std::string>(theSettingKey.c_str(), "") : NFmiSettings::Require<std::string>(theSettingKey.c_str());
        if(isOptional && tmp.empty())
            return *theOptionalColor;
        else
        {
            std::stringstream colorStream(tmp);
            colorStream >> tColor;
            return tColor;
        }
    }

    void SetColorToSettings(const std::string &theSettingKey, const NFmiColor &theColor)
    {
        std::stringstream colorStream;
        colorStream << theColor;
        NFmiSettings::Set(theSettingKey.c_str(), colorStream.str(), true);
    }

    // T‰ss‰ point on talletettu "x y" muodossa, eli x-arvo, space, y-arvo.
    NFmiPoint GetPointFromSettings(const std::string &theSettingKey)
    {
        NFmiPoint p;
        std::string tmp = NFmiSettings::Require<std::string>(theSettingKey.c_str());
        std::stringstream stream(tmp);
        stream >> p;
        return p;
    }

    void SetPointToSettings(const std::string &theSettingKey, const NFmiPoint &thePoint)
    {
        std::stringstream stream;
        stream << thePoint.X() << " " << thePoint.Y(); // pakko laittaa n‰in X ja Y erikseen, koska jos kirjoittaa koko NFmiPoint-olion, tulee per‰‰n newline
        NFmiSettings::Set(theSettingKey, stream.str(), true);
    }

    // T‰ss‰ point on talletettu "x,y" muodossa, eli x-arvo, pilkku, y-arvo.
    NFmiPoint GetCommaSeparatedPointFromSettings(const std::string &theKey, const NFmiPoint* theOptionalPoint)
    {
        bool isOptional = theOptionalPoint != 0;
        std::string gridStr = (isOptional) ? NFmiSettings::Optional<std::string>(theKey.c_str(), "") : NFmiSettings::Require<std::string>(theKey.c_str());

        if(isOptional && gridStr.empty())
            return *theOptionalPoint;
        else
        {
            checkedVector<double> gridValues = NFmiStringTools::Split<checkedVector<double> >(gridStr, ",");
            if(gridValues.size() != 2)
                throw std::runtime_error(std::string("GetCommaSeparatedPointFromSettings had invalid setting with key: ") + theKey + "\nand value: " + gridStr + "\n, has to be to numbers (e.g. x,y).");

            return NFmiPoint(gridValues[0], gridValues[1]);
        }
    }

    void SetCommaSeparatedPointToSettings(const std::string &theKey, const NFmiPoint &theGridSize)
    {
        std::string gridStr;
        gridStr += NFmiStringTools::Convert<double>(theGridSize.X());
        gridStr += ",";
        gridStr += NFmiStringTools::Convert<double>(theGridSize.Y());
        NFmiSettings::Set(theKey, gridStr, true);
    }

    // URL:ien kanssa konffeissa on ongelma koska niiss‰ on //-merkit, jotka alkavat
    // kommentti rivin konffitiedostoissa. Siksi konffeihin pit‰‰ //-merkit korvata jollain merkeill‰ etteiv‰t
    // sen j‰lkeiset merkit j‰isi pois url-stringist‰. N‰m‰ apufunktiot tekev‰t tavittavat muutokset
    // haettaessa ja asetettaessa asetuksia. //-merkit korvataan §§-merkeill‰
    // *****

    const std::string urlSlashReplaceStr = "§§";

    std::string GetUrlFromSettings(const std::string &theSettingKey, bool fDoOptional, const std::string &theOptionalValue)
    {
        std::string urlStr;
        if(fDoOptional)
            urlStr = NFmiSettings::Optional<std::string>(theSettingKey.c_str(), theOptionalValue);
        else
            urlStr = NFmiSettings::Require<std::string>(theSettingKey.c_str());
        NFmiStringTools::ReplaceAll(urlStr, urlSlashReplaceStr, "//"); // Url-osoite pit‰‰ tallettaa konffi-tiedostoon ilman //-merkkej‰:
                                                                       // http://fff.fmi.fi sijasta sinne laitetaan http:§§fff.fmi.fi, koska muuten
                                                                       // rivin loppu hyl‰t‰‰n koska ne ovat //-kommenttien takana. T‰ss‰ laitetaan takaisin oikeat merkit.
        return urlStr;
    }

    void SetUrlToSettings(const std::string &theSettingKey, const std::string &theUrlStr)
    {
        std::string tmpStr = theUrlStr;
        NFmiStringTools::ReplaceAll(tmpStr, "//", urlSlashReplaceStr); // Url-osoite pit‰‰ tallettaa konffi-tiedostoon ilman //-merkkej‰:
                                                                       // http://fff.fmi.fi sijasta sinne laitetaan http:§§fff.fmi.fi, koska muuten
                                                                       // rivin loppu hyl‰t‰‰n koska ne ovat //-kommenttien takana. T‰ss‰ laitetaan takaisin oikeat merkit.
        NFmiSettings::Set(theSettingKey, tmpStr, true);
    }

} // namespace CtrlViewUtils
