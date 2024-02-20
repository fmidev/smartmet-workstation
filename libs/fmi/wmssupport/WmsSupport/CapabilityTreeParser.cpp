#include "wmssupport/CapabilityTreeParser.h"
#include "wmssupport/QueryBuilder.h"
#include "NFmiParameterName.h"
#include "xmlliteutils/XmlHelperFunctions.h"
#include "catlog/catlog.h"

#include <regex>
#include <algorithm>
#include <codecvt>
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace Wms
{
	namespace
	{

		void splitToList(const std::string& name, const std::string& delimiter, std::list<std::string>& path)
		{
			auto tmp = cppext::split(name, delimiter);
			std::copy(tmp.begin(), tmp.end(), std::back_inserter(path));
		}

		void addToList(const std::string& name, std::list<std::string>& path)
		{
			path.emplace_back(name);
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

		std::string wstring2string(const std::wstring& wstr)
		{
			using convert_typeX = std::codecvt_utf8<wchar_t>;
			std::wstring_convert<convert_typeX, wchar_t> converterX;

			return converterX.to_bytes(wstr);
		}

		std::pair<std::string, std::string> parseLegendUrl(LPXNode legendNode)
		{
			auto domainRequest = std::pair<std::string, std::string>{};
			if(legendNode)
			{
				auto legendUrlNode = legendNode->GetChild(_TEXT("OnlineResource"));
				if(legendUrlNode)
				{
					auto legendUrl = legendUrlNode->GetAttrValue(_TEXT("xlink:href"));
					std::wstring ws(legendUrl);
					std::string legendUrlString(wstring2string(ws));

					domainRequest.first = parseDomain(legendUrlString);
					domainRequest.second = std::regex_replace(legendUrlString, domainRegex, "");
				}
			}
			return domainRequest;
		}

		Style parseStyle(LPXNode styleNode)
		{
			if(styleNode)
			{
				auto legendUrlNode = styleNode->GetChild(_TEXT("LegendURL"));
				if(legendUrlNode)
				{
					auto domainRequest = parseLegendUrl(legendUrlNode);
					std::string name = XmlHelper::ChildNodeValue(styleNode, "Name");

					return Style
					{
						name,
						domainRequest.first,
						domainRequest.second
					};
				}
			}

			return Style{};
		}

		std::set<Style> lookForStyles(LPXNode layerNode, bool &useAlsoAlternateFmiDefaultLayerNameOut)
		{
			auto styles = std::set<Style>();
			if(layerNode == nullptr)
				return styles;

			for(size_t i = 0; i < layerNode->GetChilds().size(); i++)
			{
				try
				{
					auto childNode = layerNode->GetChild(static_cast<int>(i));
					if(childNode && childNode->name == "Style")
					{
						styles.insert(parseStyle(childNode));
					}
				}
				catch(...)
				{
					continue;
				}

			}
			if(styles.size() == 1 && (styles.begin()->name == "Default" || styles.begin()->name == "default"))
			{
				useAlsoAlternateFmiDefaultLayerNameOut = (styles.begin()->name == "default");
				return std::set<Style>{};
			}
			else
			{
				return styles;
			}
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

			if(dateAndTime.size() >= 2)
			{
				auto date = dateAndTime[0];
				auto time = dateAndTime[1];
				time.pop_back();
				auto YearMonthDay = cppext::split(date, std::string("-"));
				auto HourMinuteSecond = cppext::split(time, std::string(":"));
				// Joskus time-osio voi olla vajaa, esim. "12:00:", tällöin haluamme että sekunnit annetaan 0:na kuitenkin.
				std::string secondsString = "0";
				if(HourMinuteSecond.size() >= 3)
					secondsString = HourMinuteSecond[2];

				if(HourMinuteSecond.size() >= 2 && YearMonthDay.size() >= 3)
				{
					return std::tm{
						std::stoi(secondsString) ,
						std::stoi(HourMinuteSecond[1]) ,
						std::stoi(HourMinuteSecond[0]) ,
						std::stoi(YearMonthDay[2]) ,
						std::stoi(YearMonthDay[1]) - 1 ,
						std::stoi(YearMonthDay[0]) - 1900
					};
				}
			}

			//If no time, use current time
			std::time_t t = std::time(0);
			std::tm* currentTime = std::localtime(&t);
			return *currentTime;
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
				static_cast<short>(time.tm_sec),
				1l
			};
		}

		const std::regex g_UniversalTimeStringRegex(std::string("[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}Z.*"));

		bool containsUniversalTimeString(const std::string& timeStringProspect)
		{
			return std::regex_match(timeStringProspect, g_UniversalTimeStringRegex);
		}

		std::string getLastSuitableTimeString(const std::vector<std::string>& timeStringProspects)
		{
			for(auto reverseIter = timeStringProspects.rbegin(); reverseIter != timeStringProspects.rend(); ++reverseIter)
			{
				if(containsUniversalTimeString(*reverseIter))
					return *reverseIter;
			}
			// jos ei löytynyt mitään, palautetaan vain viimeinen stringi
			return timeStringProspects.back();
		}

		std::pair<NFmiMetTime, NFmiMetTime> parseDimension(const std::string& dimension)
		{
			auto beginEnd = std::pair<NFmiMetTime, NFmiMetTime>{};

			if(dimension.empty())
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
					auto dimensionSplit = cppext::split(dimension, '/');
					beginEnd.second = parseMetTime(getLastSuitableTimeString(dimensionSplit));
				}
				else
				{
					beginEnd.first = parseMetTime(cppext::split(dimension, '/').front());
					beginEnd.second = parseMetTime(cppext::split(dimension, '/').at(1));
				}
			}

			return beginEnd;
		}

		bool IsNameStringNumeric(const std::string& nameString)
		{
			try
			{
				auto numericValue = boost::lexical_cast<double>(nameString);
				return true;
			}
			catch(...)
			{
				return false;
			}
		}

		std::string GetLastPartOfName(const std::string& nameString, const std::string& delimiter)
		{
			std::string finalString = nameString;
			if(!finalString.empty())
			{
				if(delimiter != "0")
				{
					auto nameSplit = cppext::split(finalString, delimiter);
					finalString = nameSplit.back();
				}
			}
			return finalString;
		}

		std::string GetStrippedSubPathName(LPXNode layerNode, const std::string& delimiter, bool checkTitleFirst)
		{
			std::string titleStr = XmlHelper::GetChildNodeText(layerNode, "Title");
			auto finalTitleString = GetLastPartOfName(titleStr, delimiter);
			std::string nameStr = XmlHelper::GetChildNodeText(layerNode, "Name");
			auto finalNameString = GetLastPartOfName(nameStr, delimiter);
			if(finalTitleString.empty())
				return finalNameString;
			if(finalNameString.empty())
				return finalTitleString;

			if(IsNameStringNumeric(finalTitleString))
				return finalNameString;
			if(IsNameStringNumeric(finalNameString))
				return finalTitleString;

			if(checkTitleFirst)
				return finalTitleString;
			else
				return finalNameString;
		}

	} // end of unnamed namespace


	bool DimensionTimeData::hasTimeDimension() const
	{
		if(timeWindow_.empty())
			return false;
		return (startTime_ != NFmiMetTime::gMissingTime) && (endTime_ != NFmiMetTime::gMissingTime);
	}


	CapabilityTreeParser::CapabilityTreeParser(const NFmiProducer& producer, const std::string& delimiter, std::function<bool(long, const std::string&)>& cacheHitCallback, bool acceptTimeDimensionalLayersOnly)
		:producer_{ producer }
		, delimiter_{ delimiter }
		, cacheHitCallback_{ cacheHitCallback }
		,acceptTimeDimensionalLayersOnly_(acceptTimeDimensionalLayersOnly)
	{}

	std::unique_ptr<CapabilityTree> CapabilityTreeParser::parseXmlGeneral(std::string& xml, std::map<long, std::map<long, LayerInfo>>& hashes, ChangedLayers& changedLayers)
	{
		auto subTree = std::make_unique<CapabilityNode>();
		subTree->value = Capability{ producer_, kFmiLastParameter, std::string(producer_.GetName()) };
		changedLayers.setProducerId(producer_.GetIdent());
		auto path = std::list<std::string>{};

		XNode xmlRoot;
		CString sxmlU_(CA2T(xml.c_str()));
		if(xmlRoot.Load(sxmlU_) == false)
		{
			throw std::runtime_error(std::string("CapabilityTreeParser::parseXml - xmlRoot.Load(sxmlU_) failed for string: \n") + xml);
		}

		try
		{

			auto capabilityNodes = xmlRoot.GetChilds(_TEXT("Capability"));
			for(const auto& capabilityNode : capabilityNodes)
			{
				ParseCapability(subTree, capabilityNode, path, hashes, changedLayers);
			}
		}
		catch(...)
		{
		}

		return std::move(subTree);
	}

	void CapabilityTreeParser::ParseCapability(std::unique_ptr<Wms::CapabilityNode>& subTree, LPXNode capabilityNode
		, std::list<std::string>& path, std::map<long, std::map<long, LayerInfo>>& hashes, ChangedLayers& changedLayers)
	{
		if(capabilityNode)
		{
			auto layerNodes = capabilityNode->GetChilds(_TEXT("Layer"));
			for(const auto& layerNode : layerNodes)
			{
				std::list<std::string> path;
				ParseLayer(subTree, layerNode, path, hashes, changedLayers);
			}
		}
	}

	void CapabilityTreeParser::ParseLayer(std::unique_ptr<CapabilityNode>& subTree, LPXNode layerNode, std::list<std::string>& path,
		std::map<long, std::map<long, LayerInfo>>& hashes, ChangedLayers& changedLayers)
	{
		if(layerNode)
		{
			if(DoPossibleSubLayerParsing(subTree, layerNode, path, hashes, changedLayers))
				return;

			ParseLeafLayer(subTree, layerNode, path, hashes, changedLayers);
		}
	}

	bool CapabilityTreeParser::DoPossibleSubLayerParsing(std::unique_ptr<CapabilityNode>& subTree, LPXNode layerNode, std::list<std::string>& path,
		std::map<long, std::map<long, LayerInfo>>& hashes, ChangedLayers& changedLayers)
	{
		auto subLayerNodes = layerNode->GetChilds(_TEXT("Layer"));
		if(!subLayerNodes.empty())
		{
			for(const auto& subLayerNode : subLayerNodes)
			{
				auto subNodePath = path;
				std::string title = GetStrippedSubPathName(layerNode, delimiter_, true);
				addToList(title, subNodePath);
				ParseLayer(subTree, subLayerNode, subNodePath, hashes, changedLayers);
			}
			return true;
		}
		return false;
	}

	DimensionTimeData CapabilityTreeParser::getDimensionTimeData(LPXNode layerNode)
	{
		DimensionTimeData dimensionTimeData;
		auto dimensionNode = layerNode->GetChilds(_TEXT("Dimension"));
		if(!dimensionNode.empty())
		{
			auto dNode = dimensionNode.at(0);
			dimensionTimeData.nodeName_ = XmlHelper::AttributeValue(dNode, "name");
			if(dimensionTimeData.nodeName_ == "time")
			{
				dimensionTimeData.timeWindow_ = XmlHelper::ChildNodeValue(layerNode, "Dimension");
				auto startEnd = parseDimension(dimensionTimeData.timeWindow_);
				dimensionTimeData.startTime_ = startEnd.first;
				dimensionTimeData.endTime_ = startEnd.second;
			}
		}
		return dimensionTimeData;
	}

	void CapabilityTreeParser::ParseLeafLayer(std::unique_ptr<CapabilityNode>& subTree, LPXNode layerNode, std::list<std::string>& path,
		std::map<long, std::map<long, LayerInfo>>& hashes, ChangedLayers& changedLayers)
	{
		if(layerNode)
		{
			auto dimensionTimeData = getDimensionTimeData(layerNode);
			if(!acceptTimeDimensionalLayersOnly_ || dimensionTimeData.hasTimeDimension())
			{
				auto hasFlatFmiLayerStructure = DoGeneralPathHandling(layerNode, path);
				auto name = XmlHelper::GetChildNodeText(layerNode, "Name");
				cacheHitCallback_(producer_.GetIdent(), name);
				addWithPossibleStyles(layerNode, subTree, path, changedLayers, hashes, dimensionTimeData, name, hasFlatFmiLayerStructure);
			}
		}
	}


	bool CapabilityTreeParser::DoGeneralPathHandling(LPXNode layerNode, std::list<std::string>& path)
	{
		if(HasFlatWmsStructure(layerNode, path))
		{
			// Fmi flat structure handling
			std::string pathName = XmlHelper::GetChildNodeText(layerNode, "Name");
			splitToList(pathName, delimiter_, path);
			return true;
		}
		else
		{
			// Normal recursive structure handling
			std::string title = GetStrippedSubPathName(layerNode, delimiter_, false);
			addToList(title, path);
			return false;
		}
	}

	bool CapabilityTreeParser::HasFlatWmsStructure(LPXNode layerNode, std::list<std::string>& path)
	{
		if(path.size() <= 1 && delimiter_ != "0")
		{
			std::string title = XmlHelper::GetChildNodeText(layerNode, "Name");
			auto splitName = cppext::split(title, delimiter_);
			if(splitName.size() > 1)
				return true;
		}
		return false;
	}

	void CapabilityTreeParser::addWithPossibleStyles(LPXNode layerNode, std::unique_ptr<CapabilityNode>& subTree,
		std::list<std::string>& path, ChangedLayers& changedLayers, std::map<long, std::map<long, LayerInfo>>& hashes,
		DimensionTimeData& dimensionTimeData, std::string& name, bool hasFlatFmiLayerStructure) const
	{
		if(layerNode)
		{
			bool useAlsoAlternateFmiDefaultLayerName = false;
			// Check if layer has multiple styles (then add all possibilities)
			auto styles = lookForStyles(layerNode, useAlsoAlternateFmiDefaultLayerName);
			addWithStyles(subTree, path, changedLayers, hashes, dimensionTimeData, name, styles, hasFlatFmiLayerStructure, useAlsoAlternateFmiDefaultLayerName);
		}
	}

	void traceLogLayerInfo(const LayerInfo& layerInfo, std::string serverName)
	{
		if(CatLog::logLevel() <= CatLog::Severity::Debug)
		{
			if(layerInfo.hasTimeDimension && (layerInfo.endTime.DifferenceInHours(layerInfo.startTime) <= 3))
			{
				std::string logMessage = "Short time-window with Wms server ";
				logMessage += serverName;
				logMessage += ": layerName = ";
				logMessage += layerInfo.name;
				logMessage += " startTime = ";
				logMessage += layerInfo.startTime.ToStr("YYYY MM.DD. HH:mm:SS");
				logMessage += " endTime = ";
				logMessage += layerInfo.endTime.ToStr("YYYY MM.DD. HH:mm:SS");
				CatLog::logMessage(logMessage, CatLog::Severity::Debug, CatLog::Category::NetRequest);
			}
		}
	}

	void CapabilityTreeParser::addWithStyles(std::unique_ptr<CapabilityNode>& subTree,
		std::list<std::string>& path, ChangedLayers& changedLayers, std::map<long, std::map<long, LayerInfo>>& hashes,
		DimensionTimeData& dimensionTimeData, std::string& name, std::set<Wms::Style>& styles, bool hasFlatFmiLayerStructure, bool useAlsoAlternateFmiDefaultLayerName) const
	{
		if (styles.empty())
		{
			auto hashedName = static_cast<long>(std::hash<std::string>{}(name));
			insertLeaf(
				*subTree,
				CapabilityLeaf{ Capability{ producer_, hashedName,  popLastElementFrom(path), dimensionTimeData.hasTimeDimension()}},
				path
			);

			auto layerInfo = LayerInfo{ name };
			layerInfo.setTimeDimensions(dimensionTimeData.startTime_, dimensionTimeData.endTime_);
			if(dimensionTimeData.hasTimeDimension())
			{
				changedLayers.update(hashedName, layerInfo, dimensionTimeData.timeWindow_);
			}
			hashes[producer_.GetIdent()][hashedName] = layerInfo;
			if(hasFlatFmiLayerStructure && useAlsoAlternateFmiDefaultLayerName)
			{
				// hashed-name pitää rakentaa ilman ':' erotinta!
				auto nameWithDefaultStyle = name + "default";
				auto hashedNameWithDefaultStyle = static_cast<long>(std::hash<std::string>{}(nameWithDefaultStyle));
				hashes[producer_.GetIdent()][hashedNameWithDefaultStyle] = layerInfo;
			}
		}
		else
		{
			for (const auto& style : styles)
			{
				auto hashedName = static_cast<long>(std::hash<std::string>{}(name + style.name));

				auto tmpPath = path;
				insertLeaf(
					*subTree,
					CapabilityLeaf{ Capability{ producer_, hashedName, tmpPath.back() + ":" + style.name, dimensionTimeData.hasTimeDimension() } },
					tmpPath
				);

				auto layerInfo = LayerInfo{ name, style };
				layerInfo.setTimeDimensions(dimensionTimeData.startTime_, dimensionTimeData.endTime_);
				if(dimensionTimeData.hasTimeDimension())
				{
					changedLayers.update(hashedName, layerInfo, dimensionTimeData.timeWindow_);
				}
				//else
				//{
				//	std::string msg = "Timeless dimension layer: " + layerInfo.name + " from " + producer_.GetName();
				//	CatLog::logMessage(msg, CatLog::Severity::Debug, CatLog::Category::NetRequest);
				//}
				hashes[producer_.GetIdent()][hashedName] = layerInfo;
				traceLogLayerInfo(layerInfo, std::string(producer_.GetName()));
			}
		}
	}
}