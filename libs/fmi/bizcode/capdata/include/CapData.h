#pragma once
#include "NFmiPoint.h"
#include "xmlliteutils/XMLite.h"
#include <string>
#include <map>
#include <vector>
#include <memory>

namespace Warnings
{
    class WarningMember;

    class CapData
    {
        std::vector<std::shared_ptr<WarningMember>> warnings_;
        std::map<std::string, std::map<std::string, std::vector<std::vector<NFmiPoint>>>> warning_areas_;

    public:
	    CapData();
	    ~CapData();
        bool load(const std::string& urlOrFile, const int useFile);
        void initializeWarnings(XNode &xmlRoot);
        const std::vector<std::shared_ptr<WarningMember>>& warnings() const { return warnings_; }
        std::string& getFirstCapdataPublicationId();
        std::string getOnlineCapdataPublicationId(const std::string& urlOrFile);

    private:
        void convertReferenceToNFmiPoints(std::vector<std::shared_ptr<WarningMember>> &warnings);
        void convertEPSG3067CoordinatesToNFmiPoints(std::vector<std::shared_ptr<WarningMember>> &warnings);
        void simplifyLatLonPoints(std::vector<std::shared_ptr<WarningMember>> &warnings);
        std::vector<NFmiPoint> coordinatesFromStringWithoutConversion(const std::string &coordinateString);
        std::vector<NFmiPoint> parseCoordinatesFromString(const std::string &coordinateStringFromXml);
        void getWarningAreas(std::string url, std::string ref, std::string category);
        bool getWarningsFromUrl(const std::string& url, std::string& theResultStr);
        void setWarningLocation(std::vector<std::shared_ptr<WarningMember>> &warnings);
        double distance(double x, double y, NFmiPoint& loc);
        double distance(NFmiPoint& a, NFmiPoint& b);
        std::vector<std::string> split(const std::string &s, char delim);
        void checkOverlappingWarnings(std::vector<std::shared_ptr<WarningMember>> &warnings);
        void moveIcons(std::shared_ptr<WarningMember> &w1, std::shared_ptr<WarningMember> &w2, double distanceToBeMoved);
        boolean checkIfTimesOverlap(std::shared_ptr<WarningMember> &w1, std::shared_ptr<WarningMember> &w2);
        
    };

}
