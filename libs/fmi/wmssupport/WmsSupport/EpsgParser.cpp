#include "EpsgParser.h"

#include <NFmiArea.h>
#include <NFmiAzimuthalArea.h>

#include <cpprest/asyncrt_utils.h>
#undef U // This fixes cpprest's U -macro clash with boost library move code (really dangerous to give macro name like U !!!!)

#include <string>

using AreaWithOrientation = NFmiAzimuthalArea;

namespace Wms
{
    namespace
    {
        int getOrientationOf(const AreaWithOrientation& area)
        {
            return static_cast<int>(area.Orientation());
        }
    }

    EpsgParser::EpsgParser(const std::string& stereo00, const std::string& stereo10, const std::string& stereo20)
        : stereo00_{ stereo00 }
        , stereo10_{ stereo10 }
        , stereo20_{ stereo20 }
    {
    }

    std::string EpsgParser::parse(const NFmiArea& area) const
    {
        auto classId = static_cast<unsigned int>(area.ClassId());

        switch(classId)
        {
        case kNFmiStereographicArea:
            return stereographicToEpsg(area);
        case kNFmiLatLonArea:
            return "EPSG:4326";
        case kNFmiYKJArea:
            return "EPSG:2393";
        case kNFmiMercatorArea:
            return "EPSG:900913";
        default:
            throw std::runtime_error("Area type not supported.");
        }
    }

    std::string EpsgParser::stereographicToEpsg(const NFmiArea& area) const
    {
        decltype(auto) areaWithOrientation = static_cast<const AreaWithOrientation&>(area);
        auto orientation = getOrientationOf(areaWithOrientation);
        auto epsg = std::string{};
        if(orientation == 0)
        {
            epsg = stereo00_;
        }
        else if (orientation == 10)
        {
            epsg = stereo10_;
        }
        else if(orientation == 20)
        {
            epsg = stereo20_;
        }
        return epsg;
    }
}