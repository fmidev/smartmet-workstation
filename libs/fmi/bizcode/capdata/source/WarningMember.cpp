#include "WarningMember.h"
#include "NFmiYKJArea.h"
#include "NFmiStringTools.h"
#include "xmlliteutils/XmlHelperFunctions.h"
#include <vector>
#include <memory>
#include <string>

using namespace std;

namespace
{
    // T‰t‰ k‰ytet‰‰n Cap datoista saatujen koordinaattien konvertoimiseen newbase vastaaviksi
    std::unique_ptr<NFmiArea> gYkjAreaCoordinateConverionPtr = std::make_unique<NFmiYKJArea>(NFmiPoint(19, 59), NFmiPoint(32, 70));
}

namespace Warnings
{
    WarningMember::WarningMember()
    {

    }

    WarningMember::~WarningMember() = default;

    void WarningMember::Initialize(LPXNode aNode)
    {
        //Populate (this) with warning data
        this->setPublicationId(XmlHelper::GetChildNodeText(aNode, "alert:publication_id"));
        this->setWarningContext(XmlHelper::GetChildNodeText(aNode, "alert:warning_context"));
        this->setSeverity(XmlHelper::GetChildNodeText(aNode, "alert:severity"));
        this->setCauses(XmlHelper::GetChildNodeText(aNode, "alert:causes"));
        this->setPhysicalReference(XmlHelper::GetChildNodeText(aNode, "alert:physical_reference"));
        std::string value = XmlHelper::GetChildNodeText(aNode, "alert:physical_value");
        this->setPhysicalValue(std::stof(value.empty() ? "0" : value));
        std::string direction = XmlHelper::GetChildNodeText(aNode, "alert:physical_direction");
        this->setPhysicalDirection(std::stof(direction.empty() ? "0" : direction));
        this->setPhysicalUnit(XmlHelper::GetChildNodeText(aNode, "alert:physical_unit"));
        this->setEffectiveFrom(convertTimeToMetTime(XmlHelper::GetChildNodeText(aNode, "alert:effective_from")));
        this->setEffectiveUntil(convertTimeToMetTime(XmlHelper::GetChildNodeText(aNode, "alert:effective_until")));
        this->setInfoEnglish(NFmiStringTools::Trim(XmlHelper::GetChildNodeText(aNode, "alert:info_en")));
        std::string reference = NFmiStringTools::Trim(XmlHelper::GetChildNodeText(aNode, "alert:reference"));
        this->setReference((reference.size() < 10) ? "" : reference); //skip random nonsense in reference
        setCoordinates(aNode);
        setCenter(aNode);
    }

    void WarningMember::setCoordinates(LPXNode aNode)
    {
        auto node = XmlHelper::GetChildNode(aNode, 0);
        if(node->Find(L"gml:MultiPolygon")) //Area consists of multiple polygons
        {
            LPXNode multiPolygonNode = node->Find(L"gml:MultiPolygon");
            XNodes childs = multiPolygonNode->GetChilds(_TEXT("gml:polygonMember"));
            for(size_t i = 0; i < childs.size(); i++)
            {
                LPXNode lpxNode = childs[i];
                std::string s = CT2A(lpxNode->GetText());
                this->coordinates_.push_back(NFmiStringTools::Trim(s));
            }
        } 
        else if(node->Find(L"gml:Polygon"))
        {
            LPXNode polygonNode = node->Find(L"gml:Polygon"); //Only one polygon
            std::string s = CT2A(polygonNode->GetText());
            this->coordinates_.push_back(NFmiStringTools::Trim(s));
        }
    }

    //Works probably only with freely selected areas that have predefined center
    void WarningMember::setCenter(LPXNode aNode)
    {
        auto node = XmlHelper::GetChildNode(aNode, 0);
        if(node->Find(L"alert:representative_x"))
        {
            LPXNode x = node->Find(L"alert:representative_x");
            LPXNode y = node->Find(L"alert:representative_y");

            double longitude = _wtof(x->GetText());
            double latitude = _wtof(y->GetText());

            longitude += 3000000; //Fix difference between coordinate systems
            NFmiPoint point = gYkjAreaCoordinateConverionPtr->WorldXYToLatLon(NFmiPoint(longitude, latitude));

            this->center_ = point;
        }
    }

    //Convert time (e.g. 2017-03-14T10:00:00) to NFmiMetTime
    NFmiMetTime WarningMember::convertTimeToMetTime(std::string timeStr)
    {
        using namespace NFmiStringTools;

        //Split into date and time
        auto subStrings = Split(timeStr, "T");

        //If something is wrong return NFmiMetTime(1900, 0, 0, 0, 0, 0)
        if(subStrings.size() != 2)
        {
            return NFmiMetTime::gMissingTime;
        }

        //Create date substring
        auto dateSubStrings = Split(subStrings[0], "-");
        if(dateSubStrings.size() != 3)
        {
            return NFmiMetTime::gMissingTime;
        }
        auto year = Convert<short>(dateSubStrings[0]);
        auto month = Convert<short>(dateSubStrings[1]);
        auto day = Convert<short>(dateSubStrings[2]);

        auto timeSubStrings = Split(subStrings[1], ".");
        timeSubStrings = Split(timeSubStrings[0], ":");
        if(timeSubStrings.size() != 3)
        {
            return NFmiMetTime::gMissingTime;
        }
        auto hour = Convert<short>(timeSubStrings[0]);
        auto minute = Convert<short>(timeSubStrings[1]);

        return NFmiMetTime(NFmiTime(year, month, day, hour, minute, 0).LocalTime(), 1);
    }

}