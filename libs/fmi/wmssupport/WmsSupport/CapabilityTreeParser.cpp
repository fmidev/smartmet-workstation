#include "CapabilityTreeParser.h"
#include "QueryBuilder.h"
#include "NFmiParameterName.h"
#include "xmlliteutils/XmlHelperFunctions.h"

#include <regex>
#include <algorithm>

using namespace boost::property_tree;

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

        std::pair<std::string, std::string> parseLegendUrl(const ptree& legendTree)
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

        Style parseStyle(const ptree& styleTree)
        {
            auto domainRequest = parseLegendUrl(styleTree.get_child("LegendURL"));
            return Style
            {
            styleTree.get<std::string>("Name"),
            domainRequest.first,
            domainRequest.second
            };
        }

        std::set<Style> lookForStyles(const ptree& layerTree)
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

		ptree lookForSubLayer(const ptree& layerTree)
		{
			auto pTree = ptree{};
			for (const auto& attributeKV : layerTree)
			{
				try
				{
					if (attributeKV.first == "Layer")
					{
						pTree = attributeKV.second;
						return pTree.get_child("Layer");
					}
				}
				catch (...)
				{
					continue;
				}
			}
			return pTree;
		}

		//Checks whether given time is a table and not range ( 2019-06-06T13:00:00.000Z/2019-06-06T13:00:00.000Z/PT1H )
        bool isTimeTable(const std::string& dimension) 
        {
			auto pos = min(dimension.length(), static_cast<size_t>(100));
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
			
			if (dateAndTime.size() < 2) //If no time, use current time
			{
				std::time_t t = std::time(0);  
				std::tm* currentTime = std::localtime(&t);
				return *currentTime;
			}

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
			if (dimension.empty())
				return beginEnd;

            if(isTimeTable(dimension))
            {
                // Tämä on pilkulla eritelty lista aikoja, otetaan niistä 1. ja viimeinen. Esim.
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

        std::string parseTimeWindow(const ptree& layerTree)
        {
			try
			{
				auto dimensionName = layerTree
					.get_child("Dimension")
					.get_child("<xmlattr>")
					.get<std::string>("name");

				if (dimensionName == "time")
				{
					return layerTree.get<std::string>("Dimension");
				}
				return "";
			}
			catch (...)
			{
				return "";
			}            
        }

		std::string getNameOrTitle(const std::pair<const std::string, ptree>& layerKV)
		{
			auto name = layerKV.second.get_optional<std::string>("Name");
			auto title = layerKV.second.get_optional<std::string>("Title");

			if (name)
			{
				return layerKV.second.get<std::string>("Name");
			}
			else if (title)
			{
				return layerKV.second.get<std::string>("Title");
			}
			return std::string{};
		}
    }

    CapabilityTreeParser::CapabilityTreeParser(NFmiProducer producer, std::string delimiter, std::function<bool(long, const std::string&)> cacheHitCallback)
        :producer_{ producer }
        , delimiter_{ delimiter }
        , cacheHitCallback_{ cacheHitCallback }
    {}

	std::unique_ptr<CapabilityTree> CapabilityTreeParser::parse(const ptree& layerTree, std::map<long, std::map<long, LayerInfo>>& hashes
		, ChangedLayers& changedLayers) const
	{
		auto subTree = std::make_unique<CapabilityNode>();
		subTree->value = Capability{ producer_, kFmiLastParameter, std::string(producer_.GetName()) };
		changedLayers.setProducerId(producer_.GetIdent());
		auto path = std::list<std::string>{};

		for (const auto& layerKV : layerTree)
		{
			parseNodes(subTree, layerKV, path, hashes, changedLayers);
		}

		return std::move(subTree);
	}

	std::unique_ptr<CapabilityTree> CapabilityTreeParser::parseXml(std::string& xml, std::map<long, std::map<long, LayerInfo>>& hashes, ChangedLayers& changedLayers)
	{
		auto subTree = std::make_unique<CapabilityNode>();
		subTree->value = Capability{ producer_, kFmiLastParameter, std::string(producer_.GetName()) };
		changedLayers.setProducerId(producer_.GetIdent());
		auto path = std::list<std::string>{};

		parseXMLtoNodes(xml);

		for (size_t i = 0; i < nodes_.size(); i++)
		{
			LPXNode aNode = nodes_[i];
			parseNodes(subTree, aNode, path, hashes, changedLayers);
		}

		return std::move(subTree);
	}

	void CapabilityTreeParser::parseXMLtoNodes(const std::string& xml)
	{
		try
		{
			XNode xmlRoot;
			CString sxmlU_(CA2T(xml.c_str()));
			if (xmlRoot.Load(sxmlU_) == false)
			{
				throw std::runtime_error(std::string("CapabilitiesHandler::startFetchingCapabilitiesInBackground - xmlRoot.Load(sxmlU_) failed for string: \n") + xml);
			}
			nodes_ = xmlRoot.GetChilds(_TEXT("Capability"));
		}
		catch (...)
		{
		}
	}

	void CapabilityTreeParser::parseNodes(std::unique_ptr<Wms::CapabilityNode> &subTree, const std::pair<const std::string, ptree>& layerKV
		, std::list<std::string> &path, std::map<long, std::map<long, LayerInfo>>& hashes, ChangedLayers& changedLayers) const
	{
		if (layerKV.first != "Layer") return;

		std::list<std::string> layerPath = path;
		auto name = getNameOrTitle(layerKV);
		auto timeWindow = std::string{};

		auto tmpTimeWindow = parseTimeWindow(layerKV.second); 
		if (cacheHitCallback_(producer_.GetIdent(), name))
		{
			timeWindow = tmpTimeWindow;
		}
		auto startEnd = parseDimension(tmpTimeWindow);

		splitToList(name, delimiter_, layerPath);

		// If timeWindow is empty, check child layers, but still add this name to the path.
		if (tmpTimeWindow.empty()) {
			try
			{
				ptree& subLayerTree = lookForSubLayer(layerKV.second);
				for (const auto& layer : subLayerTree)
				{
					parseNodes(subTree, layerKV, layerPath, hashes, changedLayers);
				}
			}
			catch (...)
			{
				return;
			}
		}
		else
		{
			addWithPossibleStyles(layerKV, subTree, layerPath, timeWindow, changedLayers, hashes, startEnd, name);
		}	
	}

	void CapabilityTreeParser::parseNodes(std::unique_ptr<Wms::CapabilityNode>& subTree, const LPXNode& aNode
		, std::list<std::string>& path, std::map<long, std::map<long, LayerInfo>>& hashes, ChangedLayers& changedLayers) const
	{
		if (XmlHelper::GetChildNodeText(aNode, "alert:publication_id") != "Layer") return;
		
		std::list<std::string> layerPath = path;
// 		auto name = getNameOrTitle(aNode);
		
// 		std::list<std::string> layerPath = path;
// 		auto name = getNameOrTitle(aNode);
// 		auto timeWindow = std::string{};
// 
// 		auto tmpTimeWindow = parseTimeWindow(aNode.second);
// 		if (cacheHitCallback_(producer_.GetIdent(), name))
// 		{
// 			timeWindow = tmpTimeWindow;
// 		}
// 		auto startEnd = parseDimension(tmpTimeWindow);
// 
// 		splitToList(name, delimiter_, layerPath);
// 
// 		// If timeWindow is empty, check child layers, but still add this name to the path.
// 		if (tmpTimeWindow.empty()) {
// 			try
// 			{
// 				ptree& subLayerTree = lookForSubLayer(aNode.second);
// 				for (const auto& layer : subLayerTree)
// 				{
// 					parseNodes(subTree, aNode, layerPath, hashes, changedLayers);
// 				}
// 			}
// 			catch (...)
// 			{
// 				return;
// 			}
// 		}
// 		else
// 		{
// 			addWithPossibleStyles(aNode, subTree, layerPath, timeWindow, changedLayers, hashes, startEnd, name);
// 		}
	}

	void CapabilityTreeParser::addWithPossibleStyles(const std::pair<const std::string, ptree> &layerKV, std::unique_ptr<CapabilityNode> &subTree, 
		std::list<std::string> &path, std::string &timeWindow, ChangedLayers &changedLayers, std::map<long, std::map<long, LayerInfo>> &hashes
		, std::pair<NFmiMetTime, NFmiMetTime> &startEnd, std::string& name) const
	{
		//Check if layer has multiple styles (then add all possibilities)
		auto styles = lookForStyles(layerKV.second);
		if (styles.empty())
		{
			auto hashedName = static_cast<long>(std::hash<std::string>{}(name));
			insertLeaf(
				*subTree,
				CapabilityLeaf{ Capability{ producer_, hashedName,  popLastElementFrom(path) } },
				path
			);

			auto layerInfo = LayerInfo{ name };
			layerInfo.startTime = startEnd.first;
			layerInfo.endTime = startEnd.second;
			changedLayers.update(hashedName, layerInfo, timeWindow);
			hashes[producer_.GetIdent()][hashedName] = layerInfo;
		}
		else
		{
			for (const auto& style : styles)
			{
				auto hashedName = static_cast<long>(std::hash<std::string>{}(name + style.name));

				auto tmpPath = path;
				insertLeaf(
					*subTree,
					CapabilityLeaf{ Capability{ producer_, hashedName, tmpPath.back() + ":" + style.name } },
					tmpPath
				);

				auto layerInfo = LayerInfo{ name, style };
				layerInfo.startTime = startEnd.first;
				layerInfo.endTime = startEnd.second;
				changedLayers.update(hashedName, layerInfo, timeWindow);
				hashes[producer_.GetIdent()][hashedName] = layerInfo;
			}
		}
	}
}