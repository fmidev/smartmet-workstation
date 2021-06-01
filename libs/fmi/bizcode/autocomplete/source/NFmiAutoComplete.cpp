//© Ilmatieteenlaitos/Marko.
//Original 6.10.2009
//
// Luokka tiet‰‰ brainstorm autocomplete-pluginin toiminnosta.
// Luojkan avulla sielt‰ haetaan paikkahaku tietoa paikannimien 
// alun perusteella.
//---------------------------------------------------------- NFmiAutoComplete.cpp

/*
Haku esimerkki:

http://brainstormgw.fmi.fi/autocomplete?keyword=ylewww_fi&max=8&pattern=kok

ja vastaus edelliseen:

{"autocomplete": {
  "result": [
     { "id": 651943,"name": "Kokkola","country": "FI","feature": "PPL","area": "","lon": "23.1167","lat": "63.8333","timezone": "Europe/Helsinki" }
,     { "id": 651980,"name": "Kokem‰ki","country": "FI","feature": "PPL","area": "","lon": "22.35","lat": "61.25","timezone": "Europe/Helsinki" }
,     { "id": 651979,"name": "Kokinkyl‰","country": "FI","feature": "PPL","area": "Espoo","lon": "24.7589","lat": "60.1831","timezone": "Europe/Helsinki" }
,     { "id": 840742,"name": "Kokkila","country": "FI","feature": "PPL","area": "Siuntio","lon": "24.0303","lat": "60.1856","timezone": "Europe/Helsinki" }
,     { "id": 651982,"name": "Kokem‰enkartano","country": "FI","feature": "PPL","area": "Kokem‰ki","lon": "22.3","lat": "61.2333","timezone": "Europe/Helsinki" }
,     { "id": 651976,"name": "Kokkihenna","country": "FI","feature": "PPL","area": "Orimattila","lon": "25.5","lat": "60.8167","timezone": "Europe/Helsinki" }
,     { "id": 651972,"name": "Kokkila","country": "FI","feature": "PPL","area": "H‰meenlinna","lon": "24.6667","lat": "61.1333","timezone": "Europe/Helsinki" }
,     { "id": 651973,"name": "Kokkila","country": "FI","feature": "PPL","area": "Lappeenranta","lon": "28.55","lat": "60.9833","timezone": "Europe/Helsinki" }
 ],
 "found-results": 8,
 "max-results": 8
 }
}

*/

#include "stdafx.h"
#include "NFmiAutoComplete.h"
#include "NFmiQ2Client.h"
#include "NFmiSettings.h"
#include "SettingsFunctions.h"
#include "Utf8ConversionFunctions.h"
#include "catlog/catlog.h"
#include "CtrlViewGdiPlusFunctions.h"

#include "json_spirit.h"
#include <boost/algorithm/string.hpp>

NFmiPoint NFmiACLocationInfo::gMissingLatlon = NFmiPoint(kFloatMissing, kFloatMissing);

static std::wstring g_TestAutoCompleteString = L"{\"autocomplete\": {  \"result\": [     { \"id\": 657157,\"name\": \"Pariisi\",\"country\": \"FR\",\"feature\": \"PPL\",\"area\": \"\",\"lon\": \"2.53333\",\"lat\": \"49.0167\",\"timezone\": \"Europe/Paris\" },     { \"id\": 651980,\"name\": \"Kokem‰ki\",\"country\": \"FI\",\"feature\": \"PPL\",\"area\": \"\",\"lon\": \"22.35\",\"lat\": \"61.25\",\"timezone\": \"Europe/Helsinki\" },     { \"id\": 651979,\"name\": \"Kokinkyl‰\",\"country\": \"FI\",\"feature\": \"PPL\",\"area\": \"Espoo\",\"lon\": \"24.7589\",\"lat\": \"60.1831\",\"timezone\": \"Europe/Helsinki\" },     { \"id\": 840742,\"name\": \"Kokkila\",\"country\": \"FI\",\"feature\": \"PPL\",\"area\": \"Siuntio\",\"lon\": \"24.0303\",\"lat\": \"60.1856\",\"timezone\": \"Europe/Helsinki\" },     { \"id\": 651982,\"name\": \"Kokem‰enkartano\",\"country\": \"FI\",\"feature\": \"PPL\",\"area\": \"Kokem‰ki\",\"lon\": \"22.3\",\"lat\": \"61.2333\",\"timezone\": \"Europe/Helsinki\" },     { \"id\": 651976,\"name\": \"Kokkihenna\",\"country\": \"FI\",\"feature\": \"PPL\",\"area\": \"Orimattila\",\"lon\": \"25.5\",\"lat\": \"60.8167\",\"timezone\": \"Europe/Helsinki\" },     { \"id\": 651972,\"name\": \"Kokkila\",\"country\": \"FI\",\"feature\": \"PPL\",\"area\": \"H‰meenlinna\",\"lon\": \"24.6667\",\"lat\": \"61.1333\",\"timezone\": \"Europe/Helsinki\" },     { \"id\": 651973,\"name\": \"Kokkila\",\"country\": \"FI\",\"feature\": \"PPL\",\"area\": \"Lappeenranta\",\"lon\": \"28.55\",\"lat\": \"60.9833\",\"timezone\": \"Europe/Helsinki\" } ], \"found-results\": 8, \"max-results\": 8 }}";
static bool g_DoAutoCompleteTest = false;

NFmiACLocationInfo::NFmiACLocationInfo(void)
:itsId(-1)
,itsName()
,itsCountry()
,itsFeature()
,itsArea()
,itsLatlon(gMissingLatlon)
,itsTimeZone()
{
}


NFmiAutoComplete::NFmiAutoComplete(void)
:itsBaseUrl()
,itsMaxResults(5)
,fAutoCompleteDialogOn(false)
,fUse(false)
,itsKeyWordList()
,itsNameTextColor()
,itsRectangleFillColor()
,itsRectangleFrameColor()
,itsRectangleOutOfMapFillColor()
,itsRectangleOutOfMapFrameColor()
,itsRectangleSelectedFillColor()
,itsRectangleSelectedFrameColor()
,itsMarkerFrameColor()
,itsMarkerFillColor()
,itsOutOfMapArroyHeadColor()
,itsOutOfMapArroyHeadInMM(1)
,itsMarkerSizeInMM(1)
,itsFontSizeInMM(2)
,itsRectangleFramePenSizeInMM(0.1)
,itsConnectingLinePenSizeInMM(0.1)
,itsFontName()
,itsBaseNameSpace()
{
}

NFmiAutoComplete::~NFmiAutoComplete(void)
{
}

void NFmiAutoComplete::InitFromSettings(const std::string &theBaseNameSpace)
{
	itsBaseNameSpace = theBaseNameSpace;

	itsBaseUrl = SettingsFunctions::GetUrlFromSettings(itsBaseNameSpace + "::BaseUrl", true, "");
	itsMaxResults = NFmiSettings::Require<int>(std::string(itsBaseNameSpace + "::MaxResults").c_str());
	fUse = NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::Use").c_str());
	itsOutOfMapArroyHeadInMM = NFmiSettings::Require<double>(std::string(itsBaseNameSpace + "::OutOfMapArroyHeadInMM").c_str());
	itsMarkerSizeInMM = NFmiSettings::Require<double>(std::string(itsBaseNameSpace + "::MarkerSizeInMM").c_str());
	itsFontSizeInMM = NFmiSettings::Require<double>(std::string(itsBaseNameSpace + "::FontSizeInMM").c_str());
	itsRectangleFramePenSizeInMM = NFmiSettings::Require<double>(std::string(itsBaseNameSpace + "::RectangleFramePenSizeInMM").c_str());
	itsConnectingLinePenSizeInMM = NFmiSettings::Require<double>(std::string(itsBaseNameSpace + "::ConnectingLinePenSizeInMM").c_str());
	itsFontName = NFmiSettings::Require<std::string>(std::string(itsBaseNameSpace + "::FontName").c_str());

	std::string keyWordListStr = NFmiSettings::Require<std::string>(std::string(itsBaseNameSpace + "::KeyWordList").c_str());
	itsKeyWordList = NFmiStringTools::Split(keyWordListStr, ","); // pit‰isi olla pilkulla erotettu lista avainsanoja, mit‰ voidaan k‰ytt‰‰ haussa

	itsNameTextColor = SettingsFunctions::GetColorFromSettings(itsBaseNameSpace + "::NameTextColor");
	itsRectangleFillColor = SettingsFunctions::GetColorFromSettings(itsBaseNameSpace + "::RectangleFillColor");
	itsRectangleFrameColor = SettingsFunctions::GetColorFromSettings(itsBaseNameSpace + "::RectangleFrameColor");
	itsRectangleOutOfMapFillColor = SettingsFunctions::GetColorFromSettings(itsBaseNameSpace + "::RectangleOutOfMapFillColor");
	itsRectangleOutOfMapFrameColor = SettingsFunctions::GetColorFromSettings(itsBaseNameSpace + "::RectangleOutOfMapFrameColor");
	itsRectangleSelectedFillColor = SettingsFunctions::GetColorFromSettings(itsBaseNameSpace + "::RectangleSelectedFillColor");
	itsRectangleSelectedFrameColor = SettingsFunctions::GetColorFromSettings(itsBaseNameSpace + "::RectangleSelectedFrameColor");
	itsMarkerFrameColor = SettingsFunctions::GetColorFromSettings(itsBaseNameSpace + "::MarkerFrameColor");
	itsMarkerFillColor = SettingsFunctions::GetColorFromSettings(itsBaseNameSpace + "::MarkerFillColor");
	itsOutOfMapArroyHeadColor = SettingsFunctions::GetColorFromSettings(itsBaseNameSpace + "::OutOfMapArroyHeadColor");
	invertColorAlphaValues_ = NFmiSettings::Optional<bool>(itsBaseNameSpace + "::invertColorAlphaValues", invertColorAlphaValues_);
	doColorAlphaFixes();

	if(itsBaseUrl.empty()) // jos ei ollut annettu url:ia, ei sitten k‰ytet‰ ollenkaan t‰t‰ systeemi‰
		fUse = false;
}

void NFmiAutoComplete::doColorAlphaFixes()
{
	if(invertColorAlphaValues_)
	{
		itsNameTextColor.InvertAlphaChannel();
		itsRectangleFillColor.InvertAlphaChannel();
		itsRectangleFrameColor.InvertAlphaChannel();
		itsRectangleOutOfMapFillColor.InvertAlphaChannel();
		itsRectangleOutOfMapFrameColor.InvertAlphaChannel();
		itsRectangleSelectedFillColor.InvertAlphaChannel();
		itsRectangleSelectedFrameColor.InvertAlphaChannel();
		itsMarkerFrameColor.InvertAlphaChannel();
		itsMarkerFillColor.InvertAlphaChannel();
		itsOutOfMapArroyHeadColor.InvertAlphaChannel();
	}
}

/*  // StoreSettings-runko on t‰ss‰ vain malliksi, ei ole ehk‰ tarkoitustakaan tehd‰ talletuksia
void NFmiAutoComplete::StoreToSettings(void)
{
	if(itsBaseNameSpace.empty() == false)
	{
//		NFmiSettings::Set(std::string(itsBaseNameSpace + "::Function").c_str(), NFmiStringTools::Convert(itsFunction));
	}
	else
		throw std::runtime_error("Error in NFmiAutoComplete::StoreToSettings, unable to store setting.");
}
*/

static void MakeErrorLogsFromGetRespond(const std::string &functionName, const std::string &baseError, const std::string &requestString)
{
    std::string logStr = functionName;
    logStr += baseError;
    CatLog::logMessage(logStr, CatLog::Severity::Error, CatLog::Category::NetRequest);

    logStr = "With following search: ";
    logStr += requestString;
    CatLog::logMessage(logStr, CatLog::Severity::Error, CatLog::Category::NetRequest);
}

std::wstring NFmiAutoComplete::GetAutoCompleteRespond(NFmiQ2Client &theHttpClient, const std::string &theWord, bool logEvents)
{
    std::string wantedUrlStr;
	try
	{
		if(g_DoAutoCompleteTest)
			return g_TestAutoCompleteString;
		else
		{
			std::string wantedParams;
			wantedParams += "keyword=";
			if(itsKeyWordList.size() == 0)
				wantedParams += "ylewww_fi"; // mit‰ vaihtoehtoja t‰h‰n lˆytyy?
			else
				wantedParams += itsKeyWordList[0]; // otetaan t‰ss‰ vaiheessa vain 1. listasta

			wantedParams += "&";

			wantedParams += "max=";
			wantedParams += NFmiStringTools::Convert(itsMaxResults);
			wantedParams += "&";

			wantedParams += "pattern=";
			std::string utf8Word = fromLocaleStringToUtf8(theWord);
			wantedParams += utf8Word;

			std::string narrowResultStr;
			wantedUrlStr = itsBaseUrl + "?" + wantedParams;
			if(logEvents)
			{
				std::string logStr("AutoComplete GET str was:\n");
				logStr += wantedUrlStr;
				CatLog::logMessage(logStr, CatLog::Severity::Debug, CatLog::Category::NetRequest);
			}

			theHttpClient.MakeHTTPRequest(wantedUrlStr, narrowResultStr, true);
			return CtrlView::StringToWString(narrowResultStr);
		}
	}
	catch(std::exception &e)
	{
        std::string baseError = " caused error: ";
        baseError += e.what();
        ::MakeErrorLogsFromGetRespond(__FUNCTION__, baseError, wantedUrlStr);
    }
	catch(...)
	{
        std::string baseError = " caused unknown error";
        ::MakeErrorLogsFromGetRespond(__FUNCTION__, baseError, wantedUrlStr);
    }
	return std::wstring();
}

static void ParseLocationObject(json_spirit::wObject &theLocationObject, std::vector<NFmiACLocationInfo> &theLocationsOut)
{
	NFmiACLocationInfo locationInfo;
	for(auto it = theLocationObject.begin(); it != theLocationObject.end(); ++it)
	{
		auto valuePair = *it;

		if(boost::iequals(valuePair.name_, L"id"))
		{
			if(valuePair.value_.type() == json_spirit::int_type)
				locationInfo.itsId = valuePair.value_.get_int();
		}
		else if(boost::iequals(valuePair.name_, L"name"))
			locationInfo.itsName = valuePair.value_.get_str();
		else if(boost::iequals(valuePair.name_, L"country"))
			locationInfo.itsCountry = valuePair.value_.get_str();
		else if(boost::iequals(valuePair.name_, L"feature"))
			locationInfo.itsFeature = valuePair.value_.get_str();
		else if(boost::iequals(valuePair.name_, L"area"))
			locationInfo.itsArea = valuePair.value_.get_str();
		else if(boost::iequals(valuePair.name_, L"lon"))
		{
			if(valuePair.value_.type() == json_spirit::real_type)
				locationInfo.itsLatlon.X(valuePair.value_.get_real());
		}
		else if(boost::iequals(valuePair.name_, L"lat"))
		{
			if(valuePair.value_.type() == json_spirit::real_type)
				locationInfo.itsLatlon.Y(valuePair.value_.get_real());
		}
		else if(boost::iequals(valuePair.name_, L"timezone"))
			locationInfo.itsTimeZone = valuePair.value_.get_str();
	}
	theLocationsOut.push_back(locationInfo);
}

static void ParseLocationArray(json_spirit::wArray &theLocationArray, std::vector<NFmiACLocationInfo> &theLocationsOut)
{
	for(auto it = theLocationArray.begin(); it != theLocationArray.end(); ++it)
	{
		if(it->type() == json_spirit::obj_type)
		{
			auto location = it->get_obj();
			::ParseLocationObject(location, theLocationsOut);
		}
	}
}

static void ParseJsonLocations(json_spirit::wValue &theValue, std::vector<NFmiACLocationInfo> &theLocationsOut)
{
	if(theValue.type() == json_spirit::obj_type)
	{
		// autoCompleteObject:in nimi on "autocomplete", en tarkista, koska jos joku joskus muuttaa sit‰ homma ei tulevaisuudessa pelit‰ en‰‰
		auto autoCompleteObject = theValue.get_obj();
		if(autoCompleteObject.size() == 0)
			throw std::runtime_error("Auto Complete metadata was empty.");
		else
		{
			// autoCompletePair:in nimi on "autocomplete", en tarkista, koska jos joku joskus muuttaa sit‰ homma ei tulevaisuudessa pelit‰ en‰‰
			auto autoCompletePair = autoCompleteObject[0];
			if(autoCompletePair.value_.type() == json_spirit::obj_type)
			{
				// resultObject:in nimi on "result", en tarkista, koska jos joku joskus muuttaa sit‰ homma ei tulevaisuudessa pelit‰ en‰‰
				auto resultObject = autoCompletePair.value_.get_obj();
				for(auto it = resultObject.begin(); it != resultObject.end(); ++it)
				{
					// t‰‰ll‰ etsit‰‰n sit‰ paria miss‰ on "result" nimen‰, koska on muitakin pareja resultObject-vektorissa
					auto resultPairValue = *it;
					if(boost::iequals(resultPairValue.name_, L"result"))
					{
						if(resultPairValue.value_.type() == json_spirit::array_type)
							::ParseLocationArray(resultPairValue.value_.get_array(), theLocationsOut);
					}
				}
			}
		}
	}
}

static void LogSearchAndResultStrings(const std::string &searchWord, const std::wstring &respondStr, CatLog::Severity logSeverity)
{
    std::string logStr("LocationFinder-dialog, search: '");
    logStr += searchWord;
    logStr += "', respond string was:\n";
    logStr += CtrlView::WStringToString(respondStr);
    CatLog::logMessage(logStr, logSeverity, CatLog::Category::NetRequest);
}

// hakee s‰‰detyst‰ auto complete palvelusta annettua sanaa vastaavat t‰rkeimpien paikkojen
// tiedot ja purkaa ja palauttaa NFmiACLocationInfo-vektoriin.
std::vector<NFmiACLocationInfo> NFmiAutoComplete::DoAutoComplete(NFmiQ2Client &theHttpClient, const std::string &theWord, bool logEvents)
{
	std::vector<NFmiACLocationInfo> resultLocations;
	if(theWord.empty() == false)
	{
		auto respondStr = GetAutoCompleteRespond(theHttpClient, theWord, logEvents);
		if(respondStr.empty() == false)
		{
            std::string errorString = __FUNCTION__;
            bool reportError = false;

			json_spirit::wValue metaDataValue;
            try
            {
                if(json_spirit::read(respondStr, metaDataValue))
                    ::ParseJsonLocations(metaDataValue, resultLocations);
            }
            catch(std::exception &e)
            {
                reportError = true;
                errorString += ": auto-complete request caused error: ";
                errorString += e.what();
                errorString += ", with search:";
            }
            catch(...)
            {
                reportError = true;
                errorString += ": auto-complete request caused unknown error with search:";
            }

            if(reportError)
            {
                CatLog::logMessage(errorString, CatLog::Severity::Error, CatLog::Category::NetRequest);
            }
            if(logEvents || reportError)
                ::LogSearchAndResultStrings(theWord, respondStr, reportError ? CatLog::Severity::Error : CatLog::Severity::Debug);
        }
	}
	return resultLocations;
}
