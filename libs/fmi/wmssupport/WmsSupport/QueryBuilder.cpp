#include "wmssupport/QueryBuilder.h"
#include "wmssupport/EpsgParser.h"

#include <cpprest/asyncrt_utils.h>
#undef U // This fixes cpprest's U -macro clash with boost library move code (really dangerous to give macro name like U !!!!)

#include <iterator>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace Wms
{
    namespace
    {
        string stringify(const vector<string> &layers)
        {
            ostringstream out;
            if(!layers.empty())
            {
                copy(layers.cbegin(), layers.cend() - 1, ostream_iterator<string>(out, ","));
                out << layers.back();
            }
            return out.str();
        }

        string stringifyWithLeadingZeros(short number)
        {
            return number < 10 ? to_string(0) + to_string(number) : to_string(number);
        }

        string stringify(const NFmiMetTime& time)
        {
            auto year = time.GetYear();
            auto month = time.GetMonth();
            auto day = time.GetDay();
            auto hour = time.GetHour();
            auto minutes = time.GetMin();
            auto seconds = time.GetSec();
            auto yearStr = to_string(year);
            auto monthStr = stringifyWithLeadingZeros(month);
            auto dayStr = stringifyWithLeadingZeros(day);
            auto hourStr = stringifyWithLeadingZeros(hour);
            auto minStr = stringifyWithLeadingZeros(minutes);
            auto secStr = stringifyWithLeadingZeros(seconds);
            return yearStr
                + "-" + monthStr
                + "-" + dayStr
                + "T" + hourStr
                + ":" + minStr 
                + ":" + secStr
                + "Z";
        }
    }

    QueryBuilder::QueryBuilder()
        :epsgParser_{"","",""}
    {}

    QueryBuilder::QueryBuilder(const std::string& stereo00, const std::string& stereo10, const std::string& stereo20, bool useCrs)
        : epsgParser_{stereo00, stereo10, stereo20}
        ,useCrs_(useCrs)
    {
    }

    WmsQuery QueryBuilder::build() const
    {
        return query_;
    }

    QueryBuilder& QueryBuilder::clear()
    {
        query_ = WmsQuery();
        return *this;
    }

    QueryBuilder& QueryBuilder::setScheme(const std::string& scheme)
    {
        query_.scheme = scheme;
        return *this;
    }

    QueryBuilder& QueryBuilder::setHost(const std::string& host)
    {
        query_.host = host;
        return *this;
    }

    QueryBuilder& QueryBuilder::setPath(const std::string& path)
    {
        query_.path = path;
        return *this;
    }

    QueryBuilder& QueryBuilder::setService(const std::string& service)
    {
        query_.query["SERVICE"] = service;
        return *this;
    }

    QueryBuilder& QueryBuilder::setVersion(const std::string& version)
    {
        query_.query["VERSION"] = version;
        return *this;
    }

    QueryBuilder& QueryBuilder::setRequest(const std::string& request)
    {
        query_.query["REQUEST"] = request;
        return *this;
    }

    QueryBuilder& QueryBuilder::setFormat(const std::string& format)
    {
        query_.query["FORMAT"] = format;
        return *this;
    }

    QueryBuilder& QueryBuilder::setLayers(const std::vector<std::string>& layers)
    {
        query_.query["LAYERS"] = stringify(layers);
        return *this;
    }

    QueryBuilder& QueryBuilder::setLayers(const std::string& layer)
    {
        query_.query["LAYERS"] = layer;
        return *this;
    }

    QueryBuilder& QueryBuilder::setWidth(unsigned int width)
    {
        query_.query["WIDTH"] = std::to_string(width);
        return *this;
    }

    QueryBuilder& QueryBuilder::setHeight(unsigned int height)
    {
        query_.query["HEIGHT"] = std::to_string(height);
        return *this;
    }

    QueryBuilder& QueryBuilder::setTransparency(bool on)
    {
        query_.query["TRANSPARENT"] = on ? "true" : "false";
        return *this;
    }

    QueryBuilder& QueryBuilder::setCrsAndBbox(const NFmiArea& area)
    {
        if(useCrs_)
            query_.query["CRS"] = epsgParser_.parse(area);
        else
            query_.query["sRS"] = epsgParser_.parse(area);

        auto bbox = std::string{};
        if(area.ClassId() == kNFmiLatLonArea)
        {
            bbox = to_string(area.BottomLeftLatLon().Y()) + ","
                + to_string(area.BottomLeftLatLon().X()) + ","
                + to_string(area.TopRightLatLon().Y()) + ","
                + to_string(area.TopRightLatLon().X());
        }
        else if (area.ClassId() == kNFmiStereographicArea)
        {
            auto bottomLeft = area.WorldRect().TopLeft();
            auto topRight = area.WorldRect().BottomRight();
           bbox =to_string(bottomLeft.X()) + ","
                + to_string(bottomLeft.Y()) + ","
                + to_string(topRight.X()) + ","
                + to_string(topRight.Y());
        }
        query_.query["BBOX"] = bbox;

        return *this;
    }

    QueryBuilder& QueryBuilder::setStyles(const std::string& styles)
    {
        query_.query["STYLES"] = styles;
        return *this;
    }

    QueryBuilder& QueryBuilder::setExceptions(const string& exceptions)
    {
        query_.query["EXCEPTIONS"] = exceptions;
        return *this;
    }

    QueryBuilder& QueryBuilder::setTime(const NFmiMetTime& time, bool hasTimeDimension)
    {
        if(hasTimeDimension)
        {
            query_.query["TIME"] = stringify(time);
        }
        return *this;
    }

    QueryBuilder& QueryBuilder::setMap(const std::string& map)
    {
        if(!map.empty())
        {
            query_.query["MAP"] = map;
        }
        return *this;
    }

    QueryBuilder& QueryBuilder::setToken(const std::string& token)
    {
        if(!token.empty())
        {
            query_.query["TOKEN"] = token;
        }
        return *this;
    }

    QueryBuilder& QueryBuilder::useProxy(const std::string& url)
    {
        query_.proxy = url;
        return *this;
    }
}
