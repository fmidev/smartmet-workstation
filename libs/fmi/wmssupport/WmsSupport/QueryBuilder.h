#pragma once

#include "wmssupport/WmsQuery.h"
#include "wmssupport/EpsgParser.h"

#include <NFmiArea.h>
#include <NFmiRect.h>
#include <NFmiMetTime.h>

#include <string>
#include <memory>

namespace Wms
{
    class QueryBuilder
    {
        WmsQuery query_;
        EpsgParser epsgParser_;
        bool useCrs_ = true;
    public:
        QueryBuilder();
        QueryBuilder(const std::string& stereo00, const std::string& stereo10, const std::string& stereo20, bool useCrs);

        WmsQuery build() const;
        QueryBuilder& clear();
        bool useCrs() const {return useCrs_;}

        QueryBuilder& setScheme(const std::string& scheme);
        QueryBuilder& setHost(const std::string& host);
        QueryBuilder& setPath(const std::string& path);
        QueryBuilder& setService(const std::string& service);
        QueryBuilder& setVersion(const std::string& version);
        QueryBuilder& setRequest(const std::string& request);
        QueryBuilder& setFormat(const std::string& format);
        QueryBuilder& setLayers(const std::vector<std::string>& layers);
        QueryBuilder& setLayers(const std::string& layer);
        QueryBuilder& setWidth(unsigned int width);
        QueryBuilder& setHeight(unsigned int height);
        QueryBuilder& setTransparency(bool on);
        QueryBuilder& setCrsAndBbox(const NFmiArea& area);
        QueryBuilder& setStyles(const std::string& styles);
        QueryBuilder& setExceptions(const std::string& exceptions);
        QueryBuilder& setTime(const NFmiMetTime& time, bool hasTimeDimension);
        QueryBuilder& setMap(const std::string& map);
        QueryBuilder& setToken(const std::string& token);
        QueryBuilder& useProxy(const std::string& url);
    };
}

