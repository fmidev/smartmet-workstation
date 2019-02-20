#include "CapabilityTreeParser.h"
#include "QueryBuilder.h"

#include "NFmiParameterName.h"

#include <regex>

namespace Wms
{
    namespace
    {
        void splitToList(const std::string& name, const std::string& delimiter, std::list<std::string>& path)
        {
            auto tmp = cppext::split(name, delimiter);
            std::copy(tmp.begin(), tmp.end(), std::back_inserter(path));
        }

        std::string popLastElementFrom(std::list<std::string>& path)
        {
            auto valueName = path.back();
            path.pop_back();
            return valueName;
        }

        static auto domainRegex = std::regex{ "^http[s]?:\\/\\/.+?(?=\\/)", std::regex::icase };

        std::string parseDomain(std::string& legendUrl)
        {
            auto result = std::smatch{};
            std::regex_search(legendUrl, result, domainRegex);
            return *result.begin();
        }

        std::pair<std::string, std::string> parseLegendUrl(const boost::property_tree::ptree& legendTree)
        {
            auto legendUrlString = legendTree
                .get_child("OnlineResource")
                .get_child("<xmlattr>")
                .get<std::string>("xlink:href");

            auto domainRequest = std::pair<std::string, std::string>{};

            domainRequest.first = parseDomain(legendUrlString);
            domainRequest.second = std::regex_replace(legendUrlString, domainRegex, "");

            return domainRequest;
        }

        Style parseStyle(const boost::property_tree::ptree& styleTree)
        {
            auto domainRequest = parseLegendUrl(styleTree.get_child("LegendURL"));
            return Style
            {
            styleTree.get<std::string>("Name"),
            domainRequest.first,
            domainRequest.second
            };
        }

        std::set<Style> lookForStyles(const boost::property_tree::ptree& layerTree)
        {
            auto styles = std::set<Style>();
            for(const auto& attributeKV : layerTree)
            {
                try
                {
                    if(attributeKV.first == "Style")
                    {
                        styles.insert(parseStyle(attributeKV.second));
                    }
                }
                catch(...)
                {
                    continue;
                }
            }
            if(styles.size() == 1 && styles.begin()->name == "Default")
            {
                return std::set<Style>{};
            }
            else
            {
                return styles;
            }
        }

        bool isTimeTable(const std::string& dimension)
        {
            auto pos = std::min(dimension.length(), static_cast<size_t>(100));
            auto lookEnd = dimension.cbegin() + pos;
            auto res = std::find(dimension.cbegin(), lookEnd, '/');
            return res == lookEnd;
        }

        bool beginningIsSeparated(const std::string& dimension)
        {
            auto res = std::find(dimension.cbegin(), dimension.cend(), ',');
            return res != dimension.cend();
        }

        std::tm parseTime(const std::string& value)
        {
            auto dateAndTime = cppext::split(value, std::string("T"));
            auto date = dateAndTime[0];
            auto time = dateAndTime[1];
            time.pop_back();
            auto YearMonthDay = cppext::split(date, std::string("-"));
            auto HourMinuteSecond = cppext::split(time, std::string(":"));

            return std::tm{
                std::stoi(HourMinuteSecond[2]) ,
                std::stoi(HourMinuteSecond[1]) ,
                std::stoi(HourMinuteSecond[0]) ,
                std::stoi(YearMonthDay[2]) ,
                std::stoi(YearMonthDay[1]) - 1 ,
                std::stoi(YearMonthDay[0]) - 1900
            };
        }

        NFmiMetTime parseMetTime(const std::string& timeStr)
        {
            auto time = parseTime(timeStr);
            return NFmiMetTime{
                static_cast<short>(time.tm_year + 1900),
                static_cast<short>(time.tm_mon + 1),
                static_cast<short>(time.tm_mday),
                static_cast<short>(time.tm_hour),
                static_cast<short>(time.tm_min),
                static_cast<short>(time.tm_sec)
            };
        }

        std::pair<NFmiMetTime, NFmiMetTime> parseDimension(const std::string& dimension)
        {
            auto beginEnd = std::pair<NFmiMetTime, NFmiMetTime>{};
            if(isTimeTable(dimension))
            {
                // Tämä on pilkulla eriteltu lista aikoja, otetaan niistä 1. viimeinen. Esim.
                // 2019-02-13T12:00:55Z, 2019-02-14T11:50:18Z, 2019-02-15T12:02:33Z, 2019-02-16T11:43:52Z, 2019-02-17T11:41:21Z, 2019-02-18T11:13:24Z, 2019-02-19T11:29:36Z, 2019-02-20T11:34:05Z
                auto startFirstTime = dimension.find_first_not_of(' ');
                auto endFirstTime = dimension.find_first_of(',');
                beginEnd.first = parseMetTime(dimension.substr(startFirstTime, endFirstTime));
                auto startLastTime = dimension.find_last_of(',') + 1;
                auto endLastTime = std::string::npos;
                beginEnd.second = parseMetTime(dimension.substr(startLastTime, endLastTime));
            }
            else
            {
                if(beginningIsSeparated(dimension))
                {
                    beginEnd.first = parseMetTime(cppext::split(dimension, ',').front());
                }
                else
                {
                    beginEnd.first = parseMetTime(cppext::split(dimension, '/').front());
                }
                beginEnd.second = parseMetTime(cppext::split(dimension, '/').at(1));
            }

            return beginEnd;
        }

        std::string parseTimeWindow(const boost::property_tree::ptree& layerTree)
        {
            auto dimensionName = layerTree
                .get_child("Dimension")
                .get_child("<xmlattr>")
                .get<std::string>("name");

            if(dimensionName == "time")
            {
                return layerTree.get<std::string>("Dimension");
            }
            return "";
        }
    }

    CapabilityTreeParser::CapabilityTreeParser(NFmiProducer producer, std::string delimiter, std::function<bool(long, const std::string&)> cacheHitCallback)
        :producer_{ producer }
        , delimiter_{ delimiter }
        , cacheHitCallback_{ cacheHitCallback }
    {}

    std::unique_ptr<CapabilityTree> CapabilityTreeParser::parse(const boost::property_tree::ptree& layerTree, std::map<long, std::map<long, LayerInfo>>& hashes, ChangedLayers& changedLayers) const
    {
        auto subTree = std::make_unique<CapabilityNode>();
        subTree->value = Capability{ producer_, kFmiLastParameter, std::string(producer_.GetName()) };
        changedLayers.setProducerId(producer_.GetIdent());
        for(const auto& layerKV : layerTree)
        {
            auto name = std::string{};
            auto timeWindow = std::string{};
            auto startTime = NFmiMetTime{};
            auto endTime = NFmiMetTime{};
            try
            {
                name = layerKV.second.get<std::string>("Name");
                auto tmpTimeWindow = parseTimeWindow(layerKV.second);
                if(cacheHitCallback_(producer_.GetIdent(), name))
                {
                    timeWindow = tmpTimeWindow;
                }
                auto startEnd = parseDimension(tmpTimeWindow);
                startTime = startEnd.first;
                endTime = startEnd.second;
            }
            catch(...)
            {
                continue;
            }
            auto path = std::list<std::string>{};
            splitToList(name, delimiter_, path);

            auto styles = lookForStyles(layerKV.second);
            if(styles.empty())
            {
                auto hashedName = static_cast<long>(std::hash<std::string>{}(name));
                insertLeaf(
                    *subTree,
                    CapabilityLeaf{ Capability{ producer_, hashedName,  popLastElementFrom(path) } },
                    path
                );

                auto layerInfo = LayerInfo{ name };
                layerInfo.startTime = startTime;
                layerInfo.endTime = endTime;
                changedLayers.update(hashedName, layerInfo, timeWindow);
                hashes[producer_.GetIdent()][hashedName] = layerInfo;
            }
            else
            {
                for(const auto& style : styles)
                {
                    auto hashedName = static_cast<long>(std::hash<std::string>{}(name + style.name));

                    auto tmpPath = path;
                    insertLeaf(
                        *subTree,
                        CapabilityLeaf{ Capability{ producer_, hashedName, tmpPath.back() + ":" + style.name } },
                        tmpPath
                    );

                    auto layerInfo = LayerInfo{ name, style };
                    layerInfo.startTime = startTime;
                    layerInfo.endTime = endTime;
                    changedLayers.update(hashedName, layerInfo, timeWindow);
                    hashes[producer_.GetIdent()][hashedName] = layerInfo;
                }
            }

            try
            {
                const auto& subLayerTree = layerKV.second.get_child("Layer");
                parse(subLayerTree, hashes, changedLayers);
            }
            catch(...)
            {
                continue;
            }
        }

        return std::move(subTree);
    }
}