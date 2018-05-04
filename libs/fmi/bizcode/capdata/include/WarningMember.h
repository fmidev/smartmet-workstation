#pragma once

#include "NFmiMetTime.h"
#include "NFmiPoint.h"
#include "xmlliteutils/XMLite.h"
#include <string>
#include <vector>

namespace Warnings
{
    //Warnings are handled as WarningMembers
    class WarningMember
    {
        std::string publicationId_;
        std::string warningContext_;
        std::string severity_;
        std::string causes_;
        std::string physicalReference_;
        float physicalValue_ = kFloatMissing;
        float physicalDirection_ = kFloatMissing;
        std::string physicalUnit_;
        NFmiMetTime effectiveFrom_;
        NFmiMetTime effectiveUntil_;
        std::string infoEnglish_;
        std::string reference_; //Regional polygon info can be found in different urls. After # is the exact region name (http://gml.fmi.fi/static/2016/FI/sea_region.xml#sea_region.B1N).  
        std::vector<std::string> coordinates_; //Coordinates from freely selected warning area (uses EPSG : 3067, so these still need a conversion to latlons!)
        std::vector<std::vector<NFmiPoint>> warningAreaPolygons_; //Contains all areas (EPSG:4326), some might have just one polygon
        std::string representativeX;
        std::string representativeY;
        NFmiPoint center_; //Can be used to draw symbol in the middle of the warning area

    public:
        WarningMember();
        ~WarningMember();
        void Initialize(LPXNode aNode);

        //Setters
        void setPublicationId(std::string &publicationId) { publicationId_ = publicationId; }
        void setWarningContext(std::string &warningContext) { warningContext_ = warningContext; }
        void setSeverity(std::string &severity) { severity_ = severity; }
        void setCauses(std::string &causes) { causes_ = causes; }
        void setPhysicalReference(std::string &physicalReference) { physicalReference_ = physicalReference; }
        void setPhysicalValue(float physicalValue) { physicalValue_ = physicalValue; }
        void setPhysicalDirection(float physicalDirection) { physicalDirection_ = physicalDirection; }
        void setPhysicalUnit(std::string &physicalUnit) { physicalUnit_ = physicalUnit; }
        void setEffectiveFrom(NFmiMetTime &effectiveFrom) { effectiveFrom_ = effectiveFrom; }
        void setEffectiveUntil(NFmiMetTime &effectiveUntil) { effectiveUntil_ = effectiveUntil; }
        void setInfoEnglish(std::string &infoEnglish) { infoEnglish_ = infoEnglish; }
        void setReference(std::string &reference) { reference_ = reference; }
        void setWarningAreaPolygons(std::vector<std::vector<NFmiPoint>> &warningAreaPolygons) { warningAreaPolygons_ = warningAreaPolygons; }
        void setCenter(NFmiPoint &center) { center_ = center; }

        //Getters
        std::string& getPublicationId() { return publicationId_; }
        std::string& getWarningContext() { return warningContext_; }
        std::string& getSeverity() { return severity_; }
        std::string& getCauses() { return causes_; }
        std::string& getReference() { return reference_; }
        std::vector<std::string>& getCoordinates() { return coordinates_; }
        std::string& getInfo() { return infoEnglish_; }
        std::vector<std::vector<NFmiPoint>> getWarningAreaPolygons() { return warningAreaPolygons_; }
        NFmiPoint getCenter() { return center_; }
        NFmiMetTime& getEffectiveFrom() { return effectiveFrom_; }
        NFmiMetTime& getEffectiveUntil() { return effectiveUntil_; }
        float& getPhysicalValue() { return physicalValue_; }
        float& getPhysicalDirection() { return physicalDirection_; }

    private:
        void setCoordinates(LPXNode aNode);
        void setCenter(LPXNode aNode);
        NFmiMetTime convertTimeToMetTime(std::string timeStr);
    };
}
