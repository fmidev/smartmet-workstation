#include "HakeMessage\XmlHakeParser.h"
#include "HakeMessage\HakeMsg.h"

#include "NFmiMetTime.h"
#include "NFmiPoint.h"
#include "NFmiStringTools.h"
#include "NFmiYKJArea.h"

#include "xmlliteutils/XMLite.h"
#include <memory>

using namespace std;

namespace
{
    // Tätä käytetään HAKE datopista saatujen koordinaattien konvertoimiseen newbase vastaaviksi
    std::unique_ptr<NFmiArea> gYkjAreaCoordinateConverionPtr = std::make_unique<NFmiYKJArea>(NFmiPoint(19, 59), NFmiPoint(32, 70));
}

namespace HakeMessage
{
    namespace
    {
        NFmiMetTime GetTimeFromXmlMessageString(const std::string &timeStr)
        {
            std::string tmpStr(timeStr);
            NFmiStringTools::Trim(tmpStr);
            std::vector<std::string> subStrings = NFmiStringTools::Split(tmpStr, " ");
            if(subStrings.size() != 2)
                throw std::runtime_error(std::string("GetTimeFromHALYMessageString - must be 2 'parts' in string:\n") + tmpStr);

            std::vector<std::string> dateSubStrings = NFmiStringTools::Split(subStrings[0], ".");
            if(dateSubStrings.size() != 3)
                throw std::runtime_error(std::string("GetTimeFromHALYMessageString - must be 3 date 'parts' in string:\n") + subStrings[0]);
            std::vector<std::string> timeSubStrings = NFmiStringTools::Split(subStrings[1], ":");
            if(timeSubStrings.size() != 3)
                throw std::runtime_error(std::string("GetTimeFromHALYMessageString - must be 3 time 'parts' in string:\n") + subStrings[1]);

            short year = NFmiStringTools::Convert<short>(dateSubStrings[2]);
            short month = NFmiStringTools::Convert<short>(dateSubStrings[1]);
            short day = NFmiStringTools::Convert<short>(dateSubStrings[0]);

            short hour = NFmiStringTools::Convert<short>(timeSubStrings[0]);
            short minute = NFmiStringTools::Convert<short>(timeSubStrings[1]);
            short second = NFmiStringTools::Convert<short>(timeSubStrings[2]);

            NFmiTime saTime(year, month, day, hour, minute, second);
            return NFmiMetTime(saTime.UTCTime(), 1);
        }

        std::string GetXMLChildNodeString(XNode &xml, const std::string &theChildNodeName, const std::string &theXMLMessageStr)
        {
            LPXNode node = xml.Find(CA2T(theChildNodeName.c_str()));
            if(node == 0)
                throw std::runtime_error(std::string("GetXMLChildNodeString - wanted child node '") + theChildNodeName + "' could not be found from xml string:\n" + theXMLMessageStr);
            std::string tmp = CT2A(node->GetText());
            return tmp;
        }

        int GetMessageLevel(const std::string &theStr)
        {
            static bool firstTime = true;
            static std::map<std::string, int> possibleLevels;
            if(firstTime)
            {
                firstTime = false;
                possibleLevels.insert(std::make_pair("pieni", 1));
                possibleLevels.insert(std::make_pair("normaali", 2));
                possibleLevels.insert(std::make_pair("suuri", 3));
            }
            std::string tmpStr(theStr);
            std::map<std::string, int>::iterator it = possibleLevels.find(NFmiStringTools::LowerCase(tmpStr));
            if(it != possibleLevels.end())
                return (*it).second;
            return 0;
        }
    }

    HakeMsg XmlHakeParser::parse(const std::string &xmlString)
    {
        auto xmlMessage = HakeMsg{};
        xmlMessage.IsFromXmlFormat(true);

        CString sxmlU_(CA2T(xmlString.c_str()));
        XNode xml;
        PARSEINFO pi;
        if(xml.Load(sxmlU_, &pi) == false)
        {
            if(pi.erorr_occur)
            {
                std::string errMsg("XmlHakeMessage::DecodeMessage - xml.Load(sxml) failed:\n");
                errMsg += CT2A(pi.error_string);
                errMsg += "\nFailed for message:\n";
                errMsg += xmlString;
                throw std::runtime_error(errMsg);
            }
            else
                throw std::runtime_error(std::string("XmlHakeMessage::DecodeMessage - xml.Load(sxml) failed for string: \n") + xmlString);
        }

        std::string childNodeStr = GetXMLChildNodeString(xml, "HatakeskusID", xmlString);
        xmlMessage.MessageCenterId(std::to_string(NFmiStringTools::Convert<int>(childNodeStr)));
        childNodeStr = GetXMLChildNodeString(xml, "TehtavaNro", xmlString);
        try
        {
            xmlMessage.Number(std::to_string(NFmiStringTools::Convert<size_t>(childNodeStr)));
        }
        catch(...)
        {
            xmlMessage.Number("0");
        }
        childNodeStr = GetXMLChildNodeString(xml, "Tehtavalaji", xmlString);
        xmlMessage.TypeStr(childNodeStr);

        try
        {
            // puretaan kategoorinen id numero ja suuruusluokka irti type-stringistä, joka on siis muotoa:
            // 461 vahingontorjunta: pieni
            std::string tmpStr(childNodeStr);
            NFmiStringTools::Trim(tmpStr);
            std::vector<std::string> categoryParts = NFmiStringTools::Split(tmpStr, " ");
            if(categoryParts.size() < 3)
                throw std::runtime_error(std::string("XmlHakeMessage::DecodeMessage - Tehtavalaji-node was invalid, it has to have atleast 3 words:\n") + tmpStr);
            try
            {
                xmlMessage.Category(NFmiStringTools::Convert<int>(categoryParts[0])); // categori on 1. osa
            }
            catch(...)
            { // luultavasti category stringissä on merkki perässä kuten 403B, joka on yleinen juttu, yritetään irroittaa tälläisessä tapauksessa kategoria stringistä
                if(categoryParts[0].size() > 2)
                {
                    // rakennetaan uusi stringi, missä perästä on otettu pois viimeinen merkki, ja kokeillaan konversiota uudestaan
                    std::string categoryTempStr(categoryParts[0].begin(), categoryParts[0].end() - 1);
                    xmlMessage.Category(NFmiStringTools::Convert<int>(categoryTempStr)); // categori on 1. osa
                }
                else
                    throw;
            }
        }
        catch(const exception&)
        {
            xmlMessage.Category(0);
        }
        xmlMessage.MessageLevel(1); //GetMessageLevel(categoryParts[categoryParts.size()-1]); // messagelevel on viimeinen osa
        xmlMessage.MessageStr(childNodeStr); // laitetaan typeStr messageksikin

        childNodeStr = GetXMLChildNodeString(xml, "Kunta", xmlString);
        xmlMessage.CountyStr(childNodeStr);
        try
        {
            childNodeStr = GetXMLChildNodeString(xml, "Seloste", xmlString);
            xmlMessage.ReasonStr(childNodeStr);
        }
        catch(...)
        { // ei löytynyt viestistä katu osoitetta, laitetaan ??? tilalle, koska tämä näyttää puuttuvan usein viesteistä
            xmlMessage.ReasonStr("???");
        }

        try
        {
            childNodeStr = GetXMLChildNodeString(xml, "Katu", xmlString);
        }
        catch(...)
        { // ei löytynyt viestistä katu osoitetta, laitetaan ??? tilalle, koska tämä näyttää puuttuvan usein viesteistä
            childNodeStr = "???";
        }
        xmlMessage.Address(childNodeStr);

        try
        {
            childNodeStr = GetXMLChildNodeString(xml, "KoordX", xmlString);
            double worldX = NFmiStringTools::Convert<double>(childNodeStr);
            childNodeStr = GetXMLChildNodeString(xml, "KoordY", xmlString);
            double worldY = NFmiStringTools::Convert<double>(childNodeStr);
            // PITÄÄ TEHDÄ world xy muunnos YKJ-arean avulla latlon koordinaatistoon
            xmlMessage.LatlonPoint(gYkjAreaCoordinateConverionPtr->WorldXYToLatLon(NFmiPoint(worldX, worldY)));
        }
        catch(const exception &)
        {
            xmlMessage.LatlonPoint(NFmiPoint::gMissingLatlon);
        }

        bool ilmoitusaikaOk = true;
        try
        {
            childNodeStr = GetXMLChildNodeString(xml, "Ilmoitusaika", xmlString);
            xmlMessage.StartTime(GetTimeFromXmlMessageString(childNodeStr));
        }
        catch(...)
        {
            ilmoitusaikaOk = false;
        }
        bool lahetysaikaOk = true;
        try
        {
            childNodeStr = GetXMLChildNodeString(xml, "Lahetysaika", xmlString);
            xmlMessage.SendingTime(GetTimeFromXmlMessageString(childNodeStr));
        }
        catch(...)
        {
            lahetysaikaOk = false;
        }

        if(ilmoitusaikaOk == true || lahetysaikaOk == true)
        {
            if(ilmoitusaikaOk == false)
                xmlMessage.StartTime(xmlMessage.SendingTime()); // korvataan puuttuva aika toisella
            else if(lahetysaikaOk == false)
                xmlMessage.SendingTime(xmlMessage.StartTime()); // korvataan puuttuva aika toisella
        }

        xmlMessage.TotalMessageStr(xmlString);

        return xmlMessage;
    }
}