#include "CapData.h"
#include "WarningMember.h"
#include "NFmiFileSystem.h"
#include "xmlliteutils/XmlHelperFunctions.h"
#include "NFmiMetTime.h"
#include "NFmiPoint.h"
#include "NFmiYKJArea.h"
#include "NFmiStringTools.h"
#include "NFmiQ2Client.h"
#include "catlog/catlog.h"
#include "NFmiMilliSecondTimer.h"
#include "NFmiValueString.h"

#include <memory>
#include <iostream>
#include <cmath>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>
#include <boost/assign.hpp>
#include <boost/foreach.hpp>

#ifdef _MSC_VER
#pragma warning( disable : 4503 ) // t‰m‰ est‰‰ varoituksen joka tulee VC++ k‰‰nt‰j‰ll‰, kun jonkun boost-luokan nimi merkkein‰ ylitt‰‰ jonkun rajan
#endif

BOOST_GEOMETRY_REGISTER_BOOST_TUPLE_CS(cs::cartesian)

using namespace boost::assign;
using namespace std;
using namespace Warnings;

namespace
{
    // T‰t‰ k‰ytet‰‰n Cap datoista saatujen koordinaattien konvertoimiseen newbase vastaaviksi
    std::unique_ptr<NFmiArea> gYkjAreaCoordinateConverionPtr = std::make_unique<NFmiYKJArea>(NFmiPoint(19, 59), NFmiPoint(32, 70));
}

namespace Warnings
{

    CapData::CapData()
        :warnings_(), warning_areas_()
    {

    }
    CapData::~CapData() = default;

    //Load warnings from url/file according to configurations
    bool CapData::load(const std::string& urlOrFile, const int useFile)
    {
        bool success = false;

        //Read warnings from file/url
        std::string fileContent;
        if(useFile)
        {
            //Get warnings from file
            if(NFmiFileSystem::ReadFile2String(urlOrFile, fileContent)) {
                success = true;
            }
        }
        else
        {
            //Get warnings from url
            if(getWarningsFromUrl(urlOrFile, fileContent)) {
                success = true;
            }
        }

        if(success)
        {
            CString sxmlU_(CA2T(fileContent.c_str()));
            XNode xmlRoot;
            if(xmlRoot.Load(sxmlU_) == false)
            {
                throw std::runtime_error(std::string("CapData::load - xmlRoot.Load(sxmlU_) failed for string: \n") + fileContent);
            }
            //Save warnings as WarningMembers in a (warnings_) vector;
            initializeWarnings(xmlRoot);
            return true;
        }
        return false;
    }

    bool CapData::getWarningsFromUrl(const std::string& url, std::string& theResultStr)
    {
        NFmiMilliSecondTimer timer;
        timer.StartTimer();
        NFmiQ2Client::MakeHTTPRequest(url, theResultStr, true);
        timer.StopTimer();

        std::string logStr = " Reading lasted: ";
        logStr += NFmiValueString::GetStringWithMaxDecimalsSmartWay(timer.TimeDiffInMSeconds() / 1000, 0) + ".";
        logStr += NFmiValueString::GetStringWithMaxDecimalsSmartWay(timer.TimeDiffInMSeconds() % 1000, 0) + " s.";

        CatLog::logMessage("CapData: http request to check warnings." + logStr, CatLog::Severity::Debug, CatLog::Category::NetRequest);
        
        if(theResultStr.length() > 5)
        {
            return true;
        }
        return false;
    }

    //Sorting warnings according to the warning level. This leaves the most intense warning on top of the warnings.
    bool CompareSeverity(const std::shared_ptr<WarningMember> &a, const std::shared_ptr<WarningMember> &b)
    {
        if(a->getSeverity() != b->getSeverity())
            return a->getSeverity() < b->getSeverity();
        else
            return false;
    }

    void CapData::initializeWarnings(XNode &xmlRoot)
    {
        XNodes nodes = xmlRoot.GetChilds(_TEXT("gml:featureMember"));
        for(size_t i = 0; i < nodes.size(); i++)
        {
            LPXNode aNode = nodes[i];
            shared_ptr<WarningMember> member = make_shared<WarningMember>();
            member->Initialize(aNode);
            warnings_.push_back(member);
        }
        convertReferenceToNFmiPoints(warnings_);
        convertEPSG3067CoordinatesToNFmiPoints(warnings_);
        simplifyLatLonPoints(warnings_);
        setWarningLocation(warnings_);
        std::sort(warnings_.begin(), warnings_.end(), CompareSeverity);
        checkOverlappingWarnings(warnings_);
    }

    //Fetch latitude and longitude values according to area reference information.
    void CapData::convertReferenceToNFmiPoints(std::vector<std::shared_ptr<WarningMember>> &warnings)
    {
        for(std::vector<shared_ptr<WarningMember>>::iterator it = warnings.begin(); it != warnings.end(); ++it)
        {
            //Get the url, area and category
            string reference = (*it)->getReference();
            if(reference.empty()) //skip if no reference
            {
                continue;
            }

            std::vector<std::string> urls = split(reference, ',');
            std::vector<std::vector<NFmiPoint>> areaPolygons;

            //Go through all of the areas (in most cases there is just one)
            for(int i = 0; i < urls.size(); ++i)
            {
                string ref = urls[i].substr(urls[i].find("#") + 1);
                string category = ref.substr(0, (ref.find(".")));

                if(warning_areas_.find(category) == warning_areas_.end()) //Save warning areas if category is not yet saved
                {
                    getWarningAreas(urls[i], ref, category);
                }
                for(int i = 0; i < warning_areas_[category][ref].size(); ++i)
                {
                    areaPolygons.push_back(warning_areas_[category][ref][i]);
                }
            }
            (*it)->setWarningAreaPolygons(areaPolygons);
        }
    }

    //Coordinates for freely selected warning areas (EPSG:3067) are converted to NFmiPoints (EPSG:4326).
    void CapData::convertEPSG3067CoordinatesToNFmiPoints(std::vector<std::shared_ptr<WarningMember>> &warnings)
    {
        //Handle WarningMembers one at a time. Only freely selected areas!
        for(std::vector<shared_ptr<WarningMember>>::iterator it = warnings.begin(); it != warnings.end(); ++it)
        {
            std::vector<std::string> coordinates = (*it)->getCoordinates();
            if(!coordinates.empty()) {
                std::vector<std::vector<NFmiPoint>> areaPolygons;
                for(auto &c : coordinates)
                {
                    areaPolygons.push_back(parseCoordinatesFromString(c));
                    (*it)->setWarningAreaPolygons(areaPolygons);
                }
            }
        }
    }

    //Simplify coordinates (boost::simplify) and calculate centroid
    void CapData::simplifyLatLonPoints(std::vector<std::shared_ptr<WarningMember>> &warnings)
    {
        //Handle WarningMembers one at a time
        for(std::vector<shared_ptr<WarningMember>>::iterator it = warnings.begin(); it != warnings.end(); ++it)
        {
            //Test if there are WarningAreaPolygons
            if(!((*it)->getWarningAreaPolygons().empty()))
            {
                std::vector<std::vector<NFmiPoint>> areaPolygons;
                std::vector<vector<NFmiPoint>> inputWarningAreas = (*it)->getWarningAreaPolygons();
                NFmiPoint NFmiCentroid;

                for(std::vector<vector<NFmiPoint>>::iterator i = inputWarningAreas.begin(); i != inputWarningAreas.end(); ++i)
                {
                    vector<NFmiPoint> inputLatLons = *i;
                    vector<NFmiPoint> outputLatLons;

                    boost::geometry::model::d2::point_xy<double> centroid;
                    if(!inputLatLons.empty())
                    {
                        typedef boost::geometry::model::d2::point_xy<double> xy;

                        boost::geometry::model::linestring<xy> input;
                        boost::geometry::model::linestring<xy> output;

                        //Convert NFmiPoints to boost::geometry::model::d2::point_xy's
                        for(vector<NFmiPoint>::iterator i = inputLatLons.begin(); i != inputLatLons.end(); ++i)
                        {
                            input += xy((*i).X(), (*i).Y());
                        }

                        // Simplify points using distance of 0.01 units
                        boost::geometry::simplify(input, output, 0.01);

                        // Define centroid for icon
                        boost::geometry::centroid(input, centroid);

                        //If only one area, centroid is simply that area's centroid
                        if(!NFmiCentroid.X())
                        {
                            NFmiCentroid = NFmiPoint(centroid.x(), centroid.y());
                        }
                        else //otherwise centroid is calculated as a average of all centroids 
                        {
                            //Skip very small areas
                            if(output.size() < 5) continue;
                            NFmiCentroid = NFmiPoint((NFmiCentroid.X() + centroid.x()) / 2, (NFmiCentroid.Y() + centroid.y()) / 2);
                        }

                        //Convert points back to NFmiPoints
                        for(boost::geometry::model::linestring<xy>::iterator j = output.begin(); j != output.end(); ++j)
                        {
                            outputLatLons.push_back(NFmiPoint((*j).x(), (*j).y()));
                        }
                        areaPolygons.push_back(outputLatLons);
                    }

                    //Change warning's polygon points to a simplified set and set initial value for center location
                    (*it)->setWarningAreaPolygons(areaPolygons);
                    if(!(*it)->getCenter().X())
                    {
                        (*it)->setCenter(NFmiCentroid);
                    }
                }
            }
        }
    }

    std::vector<NFmiPoint> CapData::coordinatesFromStringWithoutConversion(const std::string &coordinateString)
    {
        std::stringstream sStream(coordinateString);

        std::vector<NFmiPoint> latLonPoints;
        do
        {
            double longitude = 0, latitude = 0;
            sStream >> longitude >> latitude;
            if(sStream)
            {
                latLonPoints.push_back(NFmiPoint(longitude, latitude));
            }
            else
                break;
        } while(true);

        return latLonPoints;
    }

    std::vector<NFmiPoint> CapData::parseCoordinatesFromString(const std::string &coordinateString)
    {
        std::stringstream sStream(coordinateString);
        std::vector<NFmiPoint> latLonPoints;

        do
        {
            double longitude = 0, latitude = 0;
            char ch = 0;
            sStream >> longitude >> ch >> latitude;
            if(sStream)
            {
                longitude += 3000000; //Fix difference between coordinate systems
                latLonPoints.push_back(gYkjAreaCoordinateConverionPtr->WorldXYToLatLon(NFmiPoint(longitude, latitude)));
            }
            else
                break;
        } while(true);

        return latLonPoints;
    }

    void CapData::getWarningAreas(std::string url, std::string ref, std::string category)
    {
        std::map<std::string, std::vector<std::vector<NFmiPoint>> > data;
        string fileContent;
        url = url.substr(0, (url.find("#")));

        NFmiQ2Client::MakeHTTPRequest(url, fileContent, true);
        if(fileContent.length() > 10)
        {
            CString sxmlU_(CA2T(fileContent.c_str()));
            XNode xmlRoot;
            if(xmlRoot.Load(sxmlU_) == false)
            {
                throw std::runtime_error(std::string("NFmiCapView::Draw - xmlRoot.Load(sxml) failed for string: \n") + fileContent);
            }

            //Areas are saved under correct category  
            XNodes nodes = xmlRoot.GetChilds(_TEXT("gml:surfaceMember"));
            for(size_t i = 0; i < nodes.size(); i++)
            {
                LPXNode aNode = nodes[i];
                LPXNode childNode = XmlHelper::GetChildNode(aNode, 0);
                string area = XmlHelper::AttributeValue(childNode, "gml:id");
                XNodes childs = childNode->GetChilds();

                if(data.find(area) == data.end()) //Area is not yet saved
                {
                    string coordinates = "";
                    std::vector<std::vector<NFmiPoint>> allCoordinates;
                    for(auto j = 0; j < childs.size(); j++)
                    {
                        coordinates = (NFmiStringTools::TrimAll(XmlHelper::GetChildNodeText(childs[j], "gml:posList"), true));
                        allCoordinates.push_back(coordinatesFromStringWithoutConversion(coordinates));
                    }
                    data.emplace(area, allCoordinates);
                }
            }
        }
        else
        {
            CatLog::logMessage("CapData::GetWarningAreas - unable to get area information.", CatLog::Severity::Error, CatLog::Category::NetRequest);
        }
        this->warning_areas_.emplace(category, data);
    }

    //Calculate location for warning symbol using pole of inaccessibility (PIA) calculation method together with initial center (centroid)
    void CapData::setWarningLocation(std::vector<std::shared_ptr<WarningMember>> &warnings)
    {
        //Handle WarningMembers one at a time
        for(std::vector<shared_ptr<WarningMember>>::iterator it = warnings.begin(); it != warnings.end(); ++it)
        {
            //If there is only one set of coordinates, do this. Otherwise center is (already) defined with average of centroids
            if((*it)->getWarningAreaPolygons().size() == 1)
            {
                vector<NFmiPoint> locations = (*it)->getWarningAreaPolygons()[0];
                double maximum_min_distance = 0.0;
                double pia_x = 0.0, pia_y = 0.0;
                double min_x = 180.0, max_x = -180.0, min_y = 90.0, max_y = -90.0;
                NFmiPoint initialPoint = (*it)->getCenter();

                //Find min and max coordinates
                for(auto loc : locations)
                {
                    min_x = (loc.X() < min_x) ? loc.X() : min_x;
                    max_x = (loc.X() > max_x) ? loc.X() : max_x;
                    min_y = (loc.Y() < min_y) ? loc.Y() : min_y;
                    max_y = (loc.Y() > max_y) ? loc.Y() : max_y;
                }

                //Define search locations near initial center (calculated with centroid)
                min_x = initialPoint.X() - (max_x - min_x) / 6;
                max_x = initialPoint.X() + (max_x - min_x) / 6;
                min_y = initialPoint.Y() - (max_y - min_y) / 6;
                max_y = initialPoint.Y() + (max_y - min_y) / 6;

                //begin looping through options for icon location
                for(double x = min_x; x <= max_x; x += (max_x - min_x) / 8)
                {
                    for(double y = min_y; y <= max_y; y += (max_y - min_y) / 8)
                    {
                        double smallest_distance = 9999.9;
                        //loop through vertices and find shortest distance to this point
                        for(auto loc : locations)
                        {
                            if(distance(x, y, loc) < smallest_distance)
                            {
                                smallest_distance = distance(x, y, loc);
                            }
                        }

                        //find the maximum minimum distance through all options around centroid location
                        if(smallest_distance > maximum_min_distance)
                        {
                            maximum_min_distance = smallest_distance;
                            pia_x = x;
                            pia_y = y;
                        }
                    }
                }
                (*it)->setCenter(NFmiPoint(pia_x, pia_y));
            }
        }
    }



    //Check overlapping warnings and move warning icon locations.
    void CapData::checkOverlappingWarnings(std::vector<std::shared_ptr<WarningMember>> &warnings)
    {
        double distanceToBeMoved = 0.6;
        int i = 1;
        for(std::vector<shared_ptr<WarningMember>>::iterator it1 = warnings.begin(); it1 != warnings.end(); ++it1)
        {
            for(std::vector<shared_ptr<WarningMember>>::iterator it2 = warnings.begin() + i; it2 != warnings.end(); ++it2)
            {
                //If warning locations are in the same place and we are not looking at the exact same warning
                if((*it2)->getReference() == (*it1)->getReference() && *it1 != *it2 && (distance((*it1)->getCenter(), (*it2)->getCenter()) < 0.4))
                {
                    //And if warning times are overlapping, move icon locations
                    if(checkIfTimesOverlap(*it1, *it2))
                    {
                        moveIcons(*it1, *it2, distanceToBeMoved);
                    }
                }
            }
            i++; //skip already handled warnings
        }
    }

    boolean CapData::checkIfTimesOverlap(std::shared_ptr<WarningMember> &w1, std::shared_ptr<WarningMember> &w2)
    {
        if(((w1)->getEffectiveFrom() <= (w2)->getEffectiveFrom() && (w2)->getEffectiveFrom() <= (w1)->getEffectiveUntil())
            || ((w1)->getEffectiveFrom() <= (w2)->getEffectiveUntil() && (w2)->getEffectiveUntil() <= (w1)->getEffectiveUntil())
            || ((w2)->getEffectiveFrom() <= (w1)->getEffectiveFrom() && (w1)->getEffectiveFrom() <= (w2)->getEffectiveUntil())
            || ((w2)->getEffectiveFrom() <= (w1)->getEffectiveUntil() && (w1)->getEffectiveUntil() <= (w2)->getEffectiveUntil()))
        {
            return true;
        }
        return false;
    }

    //Move icons according to number of overlapping warnings
    void CapData::moveIcons(std::shared_ptr<WarningMember> &w1, std::shared_ptr<WarningMember> &w2, double distanceToBeMoved)
    {
        double diffX = std::abs(w1->getCenter().X() - w2->getCenter().X());
        if(diffX < distanceToBeMoved)
        {
            w2->setCenter(w2->getCenter() + NFmiPoint(distanceToBeMoved, 0));
        }
    }

    double CapData::distance(double x, double y, NFmiPoint& loc)
    {
        return sqrt(pow(x - loc.X(), 2.0) + pow(y - loc.Y(), 2.0));
    }

    double CapData::distance(NFmiPoint& a, NFmiPoint& b)
    {
        return sqrt(pow(a.X() - b.X(), 2.0) + pow(a.Y() - b.Y(), 2.0));
    }

    std::vector<std::string> CapData::split(const std::string &s, char delim) {
        std::stringstream ss(s);
        std::string item;
        std::vector<std::string> elems;
        while(std::getline(ss, item, delim)) {
            elems.push_back(std::move(item));
        }
        return elems;
    }

    std::string& CapData::getFirstCapdataPublicationId()
    {
        if(warnings_.empty())
        {
            // This way we don't return reference to destroyed string object
            static std::string emptyString;
            return emptyString;
        }

        return warnings_[0]->getPublicationId();
    }

    std::string getPublicationId(XNode &xmlRoot)
    {
        XNodes nodes = xmlRoot.GetChilds(_TEXT("gml:featureMember"));
        if(nodes.empty())
        {
            return "no_warnings";
        }
        LPXNode aNode = nodes[0];
        std::string id = XmlHelper::GetChildNodeText(aNode, "alert:publication_id");
        return id;
    }

    std::string CapData::getOnlineCapdataPublicationId(const std::string& urlOrFile)
    {
        //Check warnings from url
        std::string fileContent;
        getWarningsFromUrl(urlOrFile, fileContent);
        CString sxmlU_(CA2T(fileContent.c_str()));
        XNode xmlRoot;
        xmlRoot.Load(sxmlU_);
        return getPublicationId(xmlRoot);
    }

}