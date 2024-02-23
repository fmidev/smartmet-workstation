#ifdef _MSC_VER
#pragma warning( disable : 4800 ) // tämä estää pitkän varoituksen joka tulee kun käytetään CachedRegBool -> registry_int<bool> -tyyppistä dataa, siellä operator T -metodissa DWORD muuttuu bool:iksi (Huom! static_cast ei auta)
#endif

#include "NFmiBetaProductSystem.h"
#include "NFmiBetaProductHelperFunctions.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiValueString.h"
#include "NFmiFileString.h"
#include "NFmiFileSystem.h"
#include "NFmiPathUtils.h"
#include "MakeOptionalExePath.h"
#include "NFmiSmartToolIntepreter.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiInfoOrganizer.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "json_spirit_writer.h"
#include "json_spirit_reader.h"

#include <fstream>
#include <algorithm>
#include "boost/math/special_functions/round.hpp"
#include "json_spirit_writer_options.h"


static std::string MakeErrorString(const std::string &startStr, const std::string &endStr, const std::string &objectName, const std::string &endCharacters)
{
    std::string str = ::GetDictionaryString(startStr.c_str());
    str += " ";
    str += objectName;
    str += " ";
    str += ::GetDictionaryString(endStr.c_str());
    str += endCharacters;

    return str;
}

template<typename T>
static void ParseJsonValue(T &object, json_spirit::Value &theValue)
{
    if(theValue.type() == json_spirit::obj_type)
    {
        json_spirit::Object tmpObj = theValue.get_obj();
        for(json_spirit::Object::iterator it = tmpObj.begin(); it != tmpObj.end(); ++it)
        {
            object.ParseJsonPair(*it);
        }
    }
}

template<typename T>
bool StoreObjectInJsonFormat(const T &object, const std::string &theFilePath, const std::string &theObjectName, std::string &theErrorStringOut)
{
    json_spirit::Object jsonObject = T::MakeJsonObject(object);
    if(jsonObject.size() == 0)
    {
        theErrorStringOut = ::MakeErrorString("The given", theObjectName, "was completely empty, nothing to store.", "");
        return false;
    }

    std::stringstream outStream;
    json_spirit::write(jsonObject, outStream, json_spirit::pretty_print);

    try
    {
        NFmiFileSystem::SafeFileSave(theFilePath, outStream.str());
        return true;
    }
    catch(std::exception &e)
    {
        theErrorStringOut = ::MakeErrorString("Error while trying to save given", theObjectName, "to file", ":\n");
        theErrorStringOut += theFilePath;
        theErrorStringOut += "\n";
        theErrorStringOut += e.what();
    }
    catch(...)
    {
        theErrorStringOut = ::MakeErrorString("Unknown error while trying to save given", theObjectName, "to file", ":\n");
        theErrorStringOut += theFilePath;
    }
    return false;
}

template<typename T>
bool ReadObjectInJsonFormat(T &objectOut, const std::string &theFilePath, const std::string &theObjectName, std::string &theErrorStringOut)
{
    if(theFilePath.empty())
    {
        theErrorStringOut = ::MakeErrorString("Given", theObjectName, "file name was empty, you must provide absolute path and filename for data.\nE.g.C:\\data\\beta1.bpr", "");
        return false;
    }

    if(NFmiFileSystem::FileExists(theFilePath) == false)
    {
        theErrorStringOut = ::MakeErrorString("Given", theObjectName, "file doesn't exist", ":\n");
        theErrorStringOut += theFilePath;
        return false;
    }

    std::ifstream in(theFilePath.c_str(), std::ios_base::in | std::ios_base::binary);
    if(!in)
    {
        theErrorStringOut = ::MakeErrorString("Unable to open given", theObjectName, "file", ":\n");
        theErrorStringOut += theFilePath;
        return false;
    }

    try
    {
        json_spirit::Value jsonValue;
        if(json_spirit::read(in, jsonValue))
        {
            objectOut = T(); // Objekti pitää nollata oletus arvoilla ennen parsimista
            ::ParseJsonValue(objectOut, jsonValue);
            return true;
        }
        else
        {
            theErrorStringOut = ::MakeErrorString("Unable to read", theObjectName, "(malformatted json?) from file", ":\n");
            theErrorStringOut += theFilePath;
        }
    }
    catch(std::exception &e)
    {
        theErrorStringOut = ::MakeErrorString("Unable to read", theObjectName, "from file", ":\n");
        theErrorStringOut += theFilePath;
        theErrorStringOut += "\n";
        theErrorStringOut += ::GetDictionaryString("Reason");
        theErrorStringOut += ": ";
        theErrorStringOut += e.what();
    }
    catch(...)
    {
        theErrorStringOut = ::MakeErrorString("Unknown error while reading", theObjectName, "from file", ":\n");
        theErrorStringOut += theFilePath;
    }

    return false;
}

// **************************************************************
// ***********  FixedRunTime osio alkaa  ************************
// **************************************************************

FixedRunTime::FixedRunTime() = default;

FixedRunTime::FixedRunTime(short hour, short minute)
    :itsHour(hour),
    itsMinute(minute)
{}

bool FixedRunTime::operator==(const FixedRunTime& other) const
{
    return (itsHour == other.itsHour) && (itsMinute == other.itsMinute);
}

// Jos annetun NFmiMetTime olion tunti + minuutti lukema on isompi kuin this olion, palauta true.
bool FixedRunTime::operator<(const NFmiMetTime& metTime) const
{
    if(itsHour < metTime.GetHour())
        return true;
    if(itsHour > metTime.GetHour())
        return false;

    return itsMinute < metTime.GetMin();
}

bool FixedRunTime::operator>(const NFmiMetTime& metTime) const
{
    if(itsHour > metTime.GetHour())
        return true;
    if(itsHour < metTime.GetHour())
        return false;

    return itsMinute > metTime.GetMin();
}

bool FixedRunTime::IsValid() const
{
    if(itsHour >= 0 && itsHour < 24 && itsMinute >= 0 && itsMinute < 60)
        return true;
    else
        return false;
}

NFmiMetTime FixedRunTime::MakeMetTime() const
{
    NFmiMetTime aTime(1);
    aTime.SetHour(itsHour);
    aTime.SetMin(itsMinute);
    return aTime;
}

// fixedRunTimeString pitää olla muotoa HH:mm esim. 4:15 tai 04:15
// Oletus: fixedRunTimeString on jo trimmattu whitespace:n suhteen.
FixedRunTime FixedRunTime::ParseFixedRunTimeString(const std::string& fixedRunTimeString, std::string& possibleErrorString)
{
    std::vector<std::string> parts;
    boost::split(parts, fixedRunTimeString, boost::is_any_of(":"));
    try
    {
        if(parts.size() == 2)
        {
            short hour = boost::lexical_cast<short>(parts[0]);
            short minute = boost::lexical_cast<short>(parts[1]);
            FixedRunTime fixedRunTime(hour, minute);
            if(fixedRunTime.IsValid())
                return fixedRunTime;
            else
            {
                possibleErrorString = ::GetDictionaryString("Invalid Fixed time hour or minute") + ": ";
                possibleErrorString += fixedRunTimeString;
                return FixedRunTime();
            }
        }
    }
    catch(...)
    {
    }

    possibleErrorString = ::GetDictionaryString("Invalid Fixed time") + ": ";
    possibleErrorString += fixedRunTimeString;
    return FixedRunTime();
}

std::vector<FixedRunTime> FixedRunTime::ParseFixedRunTimesString(const std::string& fixedRunTimesString, std::string& possibleErrorString)
{
    // Need to clear possible white space from string first
    std::string usedStr = fixedRunTimesString;
    usedStr.erase(std::remove_if(usedStr.begin(), usedStr.end(), ::isspace), usedStr.end());
    if(usedStr.empty())
    {
        possibleErrorString = ::GetDictionaryString("Fixed run times were empty");
        return std::vector<FixedRunTime>(); // virhetilanteessa palkautetaan tyhjä vector
    }

    std::vector<std::string> fixedRunTimeParts;
    boost::split(fixedRunTimeParts, usedStr, boost::is_any_of(","));

    std::vector<FixedRunTime> fixedRunTimes;
    for(const auto& fixedRunTimePart : fixedRunTimeParts)
    {
        auto fixedRunTime = ParseFixedRunTimeString(fixedRunTimePart, possibleErrorString);
        if(fixedRunTime.IsValid())
            fixedRunTimes.push_back(fixedRunTime);
        else
            return std::vector<FixedRunTime>(); // virhetilanteessa palkautetaan tyhjä vector
    }

    return fixedRunTimes;
}


// **************************************************************
// *********  NFmiBetaProduct osio alkaa  ***********************
// **************************************************************

const std::string NFmiBetaProduct::itsTimeLengthLabel = ::GetDictionaryString("Time length [hours]");
const std::string NFmiBetaProduct::itsTimeStepLabel = ::GetDictionaryString("Time step [minutes]");
std::string NFmiBetaProduct::itsRootViewMacroPath = "";
FmiLanguage NFmiBetaProduct::itsLanguage = kEnglish;

NFmiBetaProduct::NFmiBetaProduct()
:itsImageStoragePath()
,itsFileNameTemplate()
,fUseAutoFileNames(false)
,itsTimeLengthInHoursString()
,itsTimeLengthInHours(0)
,itsTimeStepInMinutesString()
,itsTimeStepInMinutes(0)
,fUseUtcTimesInTimeBox(false)
,itsParamBoxLocation(kNoDirection)
,fTimeInputOk(false)
,itsTimeInputErrorString()
,itsTimeRangeInfoText()
,itsRowIndexListString()
,itsRowIndexies()
,itsRowSubdirectoryTemplate()
,fRowIndexInputOk(false)
,itsRowInputErrorString()
,itsRowIndexListInfoText()
,itsSelectedViewIndex(BetaProductViewIndex::MainMapView)
,itsViewMacroPath()
,fGivenViewMacroOk(false)
,itsWebSiteTitleString()
,itsWebSiteDescriptionString()
,itsCommandLineString()
,fDisplayRunTimeInfo(false)
,fShowModelOriginTime(false)
,itsSynopStationIdListString()
,fSynopStationIdListInputOk(false)
,itsSynopStationIdList()
{
}

// Käy läpi kaikki tarkastelut ja tekee virheilmoituksia
bool NFmiBetaProduct::CheckTimeRelatedInputs(const NFmiMetTime &theStartingTime, const std::string &theTimeLengthInHoursString, const std::string &theTimeStepInMinutesString, bool theUseUtcTimesInTimeBox)
{
    // Nollataan status dataosiot
    fTimeInputOk = false;
    itsTimeInputErrorString.clear();

    // Alustetaan ns. transientit dataosiot
    itsStartingTime = theStartingTime;

    // Alustetaan pysyvät dataosiot
    itsTimeLengthInHoursString = theTimeLengthInHoursString;
    itsTimeStepInMinutesString = theTimeStepInMinutesString;
    fUseUtcTimesInTimeBox = theUseUtcTimesInTimeBox;
    itsTimeRangeInfoText = MakeTimeRangeInfoString();

    return InputWasGood();
}

// Palauttaa vain tiedon onko viimeinen tarkastelu mennyt hyvin vai ei
bool NFmiBetaProduct::InputWasGood()
{
    return fTimeInputOk && fRowIndexInputOk && fGivenViewMacroOk && fSynopStationIdListInputOk;
}

std::string NFmiBetaProduct::GetUsedFileNameTemplate(bool addStationIdMarker) const
{
    if(fUseAutoFileNames)
    {
        if(addStationIdMarker)
        {
            // auto on tyyppiä validTime_obs/for_stationid.png
            return NFmiBetaProductionSystem::FileNameTemplateValidTimeStamp() + "_" + NFmiBetaProductionSystem::BetaAutomationAutoFileNameTypeStamp() + "_" + NFmiBetaProductionSystem::FileNameTemplateStationIdStamp() + ".png";
        }
        else
        {
            // auto on tyyppiä validTime_obs/for.png
            return NFmiBetaProductionSystem::FileNameTemplateValidTimeStamp() + "_" + NFmiBetaProductionSystem::BetaAutomationAutoFileNameTypeStamp() + ".png";
        }
    }
    else
        return itsFileNameTemplate;
}

const double gMissingTimeRelatedValue = -99999.; // Kaikki aika kontrolleihin liittyvät arvot ovat positiivisia

template<typename T>
static T GetTimeControlRelatedValueFromString(const std::string &theValueString)
{
    try
    {
        return boost::lexical_cast<T>(theValueString);
    }
    catch(...)
    {
        return static_cast<T>(gMissingTimeRelatedValue);
    }
}

static std::string MakeInvalidInputString(const std::string &theStartPartDictionaryString, const std::string &theHelpLineDictionaryString, const std::string &theInputString)
{
    std::string str = ::GetDictionaryString(theStartPartDictionaryString.c_str());
    str += " ";
    str += ::GetDictionaryString("had invalid value");
    str += ":\n";
    if(theInputString.empty())
        str += ::GetDictionaryString("<empty>");
    else
        str += theInputString;
    str += "\n";
    str += ::GetDictionaryString(theHelpLineDictionaryString.c_str());

    return str;
}

NFmiMetTime NFmiBetaProduct::CalcEndTime()
{
    double tmpTimeLengthInHours = ::GetTimeControlRelatedValueFromString<double>(itsTimeLengthInHoursString);
    if(tmpTimeLengthInHours < 0) // Formaalisesti virheellisen inputin paluuarvo on myös negatiivinen, joten sitä ei tarvitse tarkastella erikseen
    {
        itsTimeInputErrorString = ::MakeInvalidInputString(itsTimeLengthLabel, "Give positive real value like: 15", itsTimeLengthInHoursString);
        return NFmiMetTime::gMissingTime;
    }
    else
        itsTimeLengthInHours = tmpTimeLengthInHours;

    int tmpTimeStepInMinutes = GetTimeControlRelatedValueFromString<int>(itsTimeStepInMinutesString);
    if(tmpTimeStepInMinutes < 1) // Formaalisti virheellisen paluu arvo on negatiivinen, eikä saa myöskään olla 0, koska tällöin tulisi 0:lla jako
    {
        itsTimeInputErrorString = ::MakeInvalidInputString(itsTimeStepLabel, "Give positive integer value like: 60", itsTimeStepInMinutesString);
        return NFmiMetTime::gMissingTime;
    }
    else
        itsTimeStepInMinutes = tmpTimeStepInMinutes;

    int usedTimeSteps = boost::math::iround(itsTimeLengthInHours * 60. / itsTimeStepInMinutes);
    int usedTimeLengthInMinutes = usedTimeSteps * itsTimeStepInMinutes;
    NFmiMetTime endTime(itsStartingTime);
    endTime.ChangeByMinutes(usedTimeLengthInMinutes);
    return endTime;
}

static std::string MakeFullTimeString(const NFmiMetTime &theTime, FmiLanguage theLanguage)
{
    NFmiString timeStr = theTime.ToStr(::GetDictionaryString("TempViewLegendTimeFormat"), theLanguage);
    return std::string(timeStr);
}

static bool CheckUsedBoxLocation(FmiDirection theDirection)
{
    switch(theDirection)
    {
    case kTopLeft:
    case kTopRight:
    case kBottomLeft:
    case kBottomRight:
    case kTopCenter:
    case kBottomCenter:
    case kNoDirection:
        return true;
    default:
        return false;
    }
}

std::string NFmiBetaProduct::MakeTimeRangeInfoString()
{
    std::string infoStr;
    NFmiMetTime endTime = CalcEndTime();
    if(endTime == NFmiMetTime::gMissingTime)
    {
        infoStr = itsTimeInputErrorString;
    }
    else
    {
        long actualLengthInMinutes = endTime.DifferenceInMinutes(itsStartingTime);
        itsTimeStepCount = (actualLengthInMinutes / itsTimeStepInMinutes) + 1;
        infoStr = ::GetDictionaryString("Start time");
        infoStr += ":\t";
        infoStr += ::MakeFullTimeString(itsStartingTime, itsLanguage);
        infoStr += "\n";
        infoStr += ::GetDictionaryString("End time");;
        infoStr += ":\t";
        infoStr += MakeFullTimeString(endTime, itsLanguage);
        infoStr += "\n";
        infoStr += ::GetDictionaryString("Time step count: ");;
        infoStr += boost::lexical_cast<std::string>(itsTimeStepCount);
        infoStr += " (";
        infoStr += ::GetDictionaryString("total images");;
        infoStr += ": ";
        infoStr += boost::lexical_cast<std::string>(itsTimeStepCount * GetUsedRowIndexies(1).size());
        infoStr += ")\n";
        infoStr += ::GetDictionaryString("actual length [h]: ");;
        infoStr += NFmiValueString::GetStringWithMaxDecimalsSmartWay(actualLengthInMinutes / 60., 2);
        fTimeInputOk = true;
    }
    return infoStr;
}

// Oletus: theRowIndexListStr on jo tarkastettu, että se ei ole tyhjä ja siitä on poistettu kaikki whitespacet ja
// on tarkastettu, että indexRangeString:ista löytyy '-' merkki (ja indeksi ei saanut olla negatiivinen, joten esim. "-12" ei kelpaa)
static void AddPossibleRangeToList(std::vector<int> &indexListInOut, const std::string &indexRangeString)
{
    try
    {
        // Kokeillaan onko kyse '-' erotetusta rangesta (esim. 2-4)
        std::vector<int> tmpVec = NFmiStringTools::Split<std::vector<int>>(indexRangeString, "-");
        if(tmpVec.size() != 2)
        {
            std::string errorString = ::GetDictionaryString("Invalid row range given, must be two numbers (e.g. 2-4)");
            errorString += ": ";
            errorString += indexRangeString;
            throw std::runtime_error(errorString);
        }
        else if(tmpVec[0] >= tmpVec[1])
        {
            std::string errorString = ::GetDictionaryString("Invalid row range given (a-b where a >= b)");
            errorString += ": ";
            errorString += indexRangeString;
            throw std::runtime_error(errorString);
        }
        else
        {
            for(int i = tmpVec[0]; i <= tmpVec[1]; i++)
                indexListInOut.push_back(i);
        }
    }
    catch(std::exception &e)
    {
        std::string errorString = ::GetDictionaryString("Invalid range with error");
        errorString += ": ";
        errorString += e.what();
        errorString += " (";
        errorString += indexRangeString;
        errorString += ")";
        throw std::runtime_error(errorString);
    }
}

static std::string RemoveAllWhiteSpaces(const std::string &theString)
{
    std::string str(theString);
    str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());
    return str;
}

const char g_RangeCharacter = '-';
const std::string g_RangeCharacterString = "-";

static bool IsRangeCase(const std::string &indexString)
{
    return indexString.find(g_RangeCharacter) != std::string::npos;
}

static void AddPossibleIndexToList(std::vector<int> &indexListInOut, const std::string &indexString)
{
    indexListInOut.push_back(boost::lexical_cast<int>(indexString));
}

static void AddPossibleIndicesToList(std::vector<int> &indexListInOut, const std::string &indexString)
{
    if(!indexString.empty())
    {
        if(IsRangeCase(indexString))
            AddPossibleRangeToList(indexListInOut, indexString);
        else
            AddPossibleIndexToList(indexListInOut, indexString);
    }
}

// Muuttujassa itsRowIndexListStringU_ pitää olla jotain seuraavaa:
// 1. tyhjä
// 2. int1,int2,int3-int4,... eli pilkuilla erotettuja positiivisia lukuja (0 ei kelpaa) listassa joissa voi olla yksittäisiä lukuja tai lukuvälejä
// 2.1. Jos kyse lukuvälistä, on mukana kaksi lukua, missä niiden erottimena on '-' -merkki
// Esim. "1,3,6-8,12,..."
// Heittää poikkeuksia, jos virheellinen input.
static std::vector<int> MakeIndexList(std::string indexListString)
{
    indexListString = ::RemoveAllWhiteSpaces(indexListString); // Poistetaan varmuuden vuoksi kaikki white-spacet input stringistä
    if(indexListString.empty())
        return std::vector<int>();
    else
    {
        std::vector<std::string> indexParts;
        boost::split(indexParts, indexListString, boost::is_any_of(","));
        std::vector<int> indexList;
        for(const auto &indexString : indexParts)
        {
            ::AddPossibleIndicesToList(indexList, indexString);
        }
        return indexList;
    }
}

std::vector<int> NFmiBetaProduct::CalcRowIndexies()
{
    try
    {
        return ::MakeIndexList(itsRowIndexListString);
    }
    catch(std::exception &e)
    {
        itsRowInputErrorString = ::GetDictionaryString("Row index input error");
        itsRowInputErrorString += ":\n";
        itsRowInputErrorString += e.what();
    }
    return std::vector<int>();
}

bool NFmiBetaProduct::ContainsStringCaseInsensitive(const std::string &searchThis, const std::string &findThis)
{
    auto findRange = boost::algorithm::ifind_first(searchThis, findThis);
    return !findRange.empty();
}

// Oletus: tänne tullaan vain jos itsRowIndexList EI ole tyhjä.
// 1. Jos itsRowSubdirectoryTemplateStringU_ -muuttujassa on jotain, siitä pitää löytyä '#' -markki, muuten se ei kelpaa
// 2. Jos tämä on tyhjä, mutta itsRowIndexList -muuttujassa on arvoja, pitää itsFileNameTemplateU_ -muuttujassa olla "rowIndex" -leima
std::string NFmiBetaProduct::CheckUsedRowSubdirectoryTemplate()
{
    std::string subdirTemplateStr = itsRowSubdirectoryTemplate;
    subdirTemplateStr = ::RemoveAllWhiteSpaces(subdirTemplateStr); // Poistetaan varmuuden vuoksi kaikki white-spacet input stringistä
    if(subdirTemplateStr.empty())
    {
            itsRowInputErrorString = ::GetDictionaryString("Multiple rows were selected but no row subdirectory template.");
            itsRowInputErrorString += "\n";
            itsRowInputErrorString += ::GetDictionaryString("Unable to generate images");
            return "";
    }
    else
    {
        std::string::size_type pos = subdirTemplateStr.find('#');
        if(pos != std::string::npos)
            return subdirTemplateStr; // templaatista löytyi # -merkki, templaatti kunnossa
        else
        {
            itsRowInputErrorString = ::GetDictionaryString("Multiple rows were selected but invalid input in row subdirectory template");
            itsRowInputErrorString += ": \"";
            itsRowInputErrorString += subdirTemplateStr;
            itsRowInputErrorString += "\"\n";
            itsRowInputErrorString += ::GetDictionaryString("There must be something with # -character in template. Unable to generate images");
            return subdirTemplateStr;
        }
    }
}

static bool CheckForOnlyPositiveNumbers(const std::vector<int> &theRowIndexies)
{
    auto iter = std::find_if(theRowIndexies.begin(), theRowIndexies.end(), [](int value){return value < 1; });
    return iter == theRowIndexies.end();
}

const std::string gSupportedRowIndexFormats = "<empty>  OR  1,3,4  OR  2-4";
const std::string gSupportedRowSubdirectoryFormats = "<empty>  OR  Row#  OR  #  OR  dir#";

std::string NFmiBetaProduct::MakeRowIndexInfoString()
{
    itsRowIndexies = CalcRowIndexies();
    if(itsRowInputErrorString.empty() && !itsRowIndexies.empty()) // Jos CalcRowIndexList:issa ei virheitä ja palautettu itsRowIndexList ei ollut tyhjä, tutkitaan row-subdirectory-template:ia
        itsRowSubdirectoryTemplate = CheckUsedRowSubdirectoryTemplate();

    std::string infoStr;
    if(!itsRowInputErrorString.empty())
    {
        infoStr = ::GetDictionaryString("Error with row index input");
        infoStr += ":\n";
        infoStr += itsRowInputErrorString;
        infoStr += ":\n";
        infoStr += ::GetDictionaryString("Try");
        infoStr += ": ";
        if(itsRowIndexies.empty())
            infoStr += gSupportedRowIndexFormats;
        else
            infoStr += gSupportedRowSubdirectoryFormats;
    }
    else if(!::CheckUsedBoxLocation(itsParamBoxLocation))
    {
        infoStr = ::GetDictionaryString("Parambox location was illegal.");
        infoStr += "\n";
        infoStr += ::GetDictionaryString("Make correct selection from dropdown list.");
    }
    else
    {
        if(!::CheckForOnlyPositiveNumbers(itsRowIndexies))
        {
            infoStr = ::GetDictionaryString("There were 0 or negative values in given row index list");
            infoStr += ":\n";
            infoStr += itsRowIndexListString;
            infoStr += "\n";
            infoStr += ::GetDictionaryString("Make sure they are all positive integer values like 1,2,3");
        }
        else
        {
            if(itsRowIndexies.empty())
                infoStr = ::GetDictionaryString("No row index list given, only visible row used");
            else
            {
                infoStr = ::GetDictionaryString("Following rows");
                infoStr += " (";
                infoStr += boost::lexical_cast<std::string>(itsRowIndexies.size());
                infoStr += " ";
                infoStr += ::GetDictionaryString("pcs");
                infoStr += ") ";
                infoStr += ::GetDictionaryString("used to generate images");
                infoStr += ":\n";
                infoStr += itsRowIndexListString;
            }
            fRowIndexInputOk = true;
        }
    }

    return infoStr;
}

// Käy läpi kaikki tarkastelut ja tekee virheilmoituksia
bool NFmiBetaProduct::CheckRowRelatedInputs(const std::string &theRowIndexListString, const std::string &theRowSubdirectoryTemplate, const std::string &theFileNameTemplate, bool useAutoFileNames, FmiDirection theParamBoxLocation)
{
    fRowIndexInputOk = false;
    itsRowInputErrorString.clear();

    itsRowIndexListString = theRowIndexListString;
    itsRowSubdirectoryTemplate = theRowSubdirectoryTemplate;
    itsFileNameTemplate = theFileNameTemplate;
    fUseAutoFileNames = useAutoFileNames;
    itsParamBoxLocation = theParamBoxLocation;

    itsRowIndexListInfoText = MakeRowIndexInfoString();

    return InputWasGood();
}

// Käy läpi kaikki tarkastelut ja tekee status asetuksia.
bool NFmiBetaProduct::CheckSynopStationIdListRelatedInputs(const std::string &theSynopStationIdListString)
{
    fSynopStationIdListInputOk = false;
    try
    {
        itsSynopStationIdListString = theSynopStationIdListString;
        itsSynopStationIdList = ::MakeIndexList(theSynopStationIdListString);
        fSynopStationIdListInputOk = true;
    }
    catch(std::exception & /* e */ )
    {

    }
    return InputWasGood();
}

std::vector<int> NFmiBetaProduct::GetUsedRowIndexies(int theOrigRowIndex) const
{
    if(itsRowIndexies.size())
        return itsRowIndexies;
    else
    {
        // Jos käyttäjä ei ole antanut listaa, tehdään sellainen käytössä olevasta theOrigRowIndex:istä
        return std::vector<int>{theOrigRowIndex};
    }
}

static std::string GetFileNameHeaderInHyphens(const NFmiFileString &theFileString, bool includeExtension)
{
    std::string str = "'";
    str += theFileString.Header();
    if(includeExtension)
    {
        if(theFileString.Extension().GetLen())
        {
            str += ".";
            str += theFileString.Extension();
        }
    }
    str += "'";
    return str;
}

const std::string& NFmiBetaProduct::MakeViewMacroInfoText(const std::string &theOriginalPath)
{
    fGivenViewMacroOk = true;
    itsViewMacroInfoText.clear();
    itsOriginalViewMacroPath = theOriginalPath;

    bool extensionAdded = false;
    itsViewMacroPath = PathUtils::getTrueFilePath(theOriginalPath, itsRootViewMacroPath, "vmr", &extensionAdded);

    if(itsViewMacroPath.empty())
        itsViewMacroInfoText = ::GetDictionaryString("No viewMacro used");
    else
    {
        NFmiFileString fileString(itsViewMacroPath);

        // Tutkitaan löytyykö annettua viewMacroa ollenkaan
        if(NFmiFileSystem::FileExists(itsViewMacroPath))
        {
            itsViewMacroInfoText = ::GetDictionaryString("Using viewMacro");
            itsViewMacroInfoText += ": ";
            itsViewMacroInfoText += ::GetFileNameHeaderInHyphens(fileString, !extensionAdded);
        }
        else
        {
            fGivenViewMacroOk = false;
            itsViewMacroInfoText = ::GetDictionaryString("Unable to locate given viewMacro");
            itsViewMacroInfoText += ": ";
            itsViewMacroInfoText += ::GetFileNameHeaderInHyphens(fileString, !extensionAdded);
        }
    }
    return itsViewMacroInfoText;
}

static const std::string gJsonName_ImageStoragePath = "ImageStoragePath";
static const std::string gJsonName_FileNameTemplate = "FileNameTemplate";
static const std::string gJsonName_UseAutoFileNames = "UseAutoFileNames";
static const std::string gJsonName_TimeLengthInHours = "TimeLengthInHours";
static const std::string gJsonName_TimeStepInMinutes = "TimeStepInMinutes";
static const std::string gJsonName_UseUtcInTimebox = "UseUtcInTimebox";
static const std::string gJsonName_ParamboxLocation = "ParamboxLocation";
static const std::string gJsonName_RowIndexList = "RowIndexList";
static const std::string gJsonName_RowSubdirectoryTemplate = "RowSubdirectoryTemplate";
static const std::string gJsonName_SelectedViewIndex = "SelectedViewIndex";
static const std::string gJsonName_ViewMacroPath = "ViewMacroPath";
static const std::string gJsonName_WebSiteTitle = "WebSiteTitle";
static const std::string gJsonName_WebSiteDescription = "WebSiteDescription";
static const std::string gJsonName_CommandLine = "CommandLine";
static const std::string gJsonName_DisplayRuntimeInfo = "DisplayRuntimeInfo";
static const std::string gJsonName_ShowModelOriginTime = "ShowModelOriginTime";
static const std::string gJsonName_SynopStationIdList = "SynopStationIdList";
static const std::string gJsonName_PackImages = "PackImages";
static const std::string gJsonName_EnsureCurveVisibility = "EnsureCurveVisibility";

static void AddNonEmptyStringJsonPair(const std::string &value, const std::string &valueJsonName, json_spirit::Object &jsonObject)
{
    if(!value.empty())
        jsonObject.push_back(json_spirit::Pair(valueJsonName, value));
}

json_spirit::Object NFmiBetaProduct::MakeJsonObject(const NFmiBetaProduct &betaProduct)
{
    static const NFmiBetaProduct defaultBetaProduct;

    json_spirit::Object jsonObject;
    ::AddNonEmptyStringJsonPair(betaProduct.ImageStoragePath(), gJsonName_ImageStoragePath, jsonObject);
    ::AddNonEmptyStringJsonPair(betaProduct.FileNameTemplate(), gJsonName_FileNameTemplate, jsonObject);
    if(defaultBetaProduct.UseAutoFileNames() != betaProduct.UseAutoFileNames())
        jsonObject.push_back(json_spirit::Pair(gJsonName_UseAutoFileNames, betaProduct.UseAutoFileNames()));
    ::AddNonEmptyStringJsonPair(betaProduct.TimeLengthInHoursString(), gJsonName_TimeLengthInHours, jsonObject);
    ::AddNonEmptyStringJsonPair(betaProduct.TimeStepInMinutesString(), gJsonName_TimeStepInMinutes, jsonObject);
    if(defaultBetaProduct.UseUtcTimesInTimeBox() != betaProduct.UseUtcTimesInTimeBox())
        jsonObject.push_back(json_spirit::Pair(gJsonName_UseUtcInTimebox, betaProduct.UseUtcTimesInTimeBox()));
    if(defaultBetaProduct.ParamBoxLocation() != betaProduct.ParamBoxLocation())
        jsonObject.push_back(json_spirit::Pair(gJsonName_ParamboxLocation, betaProduct.ParamBoxLocation()));
    if(defaultBetaProduct.DisplayRunTimeInfo() != betaProduct.DisplayRunTimeInfo())
        jsonObject.push_back(json_spirit::Pair(gJsonName_DisplayRuntimeInfo, betaProduct.DisplayRunTimeInfo()));
    if(defaultBetaProduct.ShowModelOriginTime() != betaProduct.ShowModelOriginTime())
        jsonObject.push_back(json_spirit::Pair(gJsonName_ShowModelOriginTime, betaProduct.ShowModelOriginTime()));
    ::AddNonEmptyStringJsonPair(betaProduct.RowIndexListString(), gJsonName_RowIndexList, jsonObject);
    ::AddNonEmptyStringJsonPair(betaProduct.RowSubdirectoryTemplate(), gJsonName_RowSubdirectoryTemplate, jsonObject);
    ::AddNonEmptyStringJsonPair(betaProduct.OriginalViewMacroPath(), gJsonName_ViewMacroPath, jsonObject);
    ::AddNonEmptyStringJsonPair(betaProduct.WebSiteTitleString(), gJsonName_WebSiteTitle, jsonObject);
    ::AddNonEmptyStringJsonPair(betaProduct.WebSiteDescriptionString(), gJsonName_WebSiteDescription, jsonObject);
    ::AddNonEmptyStringJsonPair(betaProduct.CommandLineString(), gJsonName_CommandLine, jsonObject);
    jsonObject.push_back(json_spirit::Pair(gJsonName_SelectedViewIndex, static_cast<int>(betaProduct.SelectedViewIndex())));
    ::AddNonEmptyStringJsonPair(betaProduct.SynopStationIdListString(), gJsonName_SynopStationIdList, jsonObject);
    if(defaultBetaProduct.PackImages() != betaProduct.PackImages())
        jsonObject.push_back(json_spirit::Pair(gJsonName_PackImages, betaProduct.PackImages()));
    if(defaultBetaProduct.EnsureCurveVisibility() != betaProduct.EnsureCurveVisibility())
        jsonObject.push_back(json_spirit::Pair(gJsonName_EnsureCurveVisibility, betaProduct.EnsureCurveVisibility()));

    return jsonObject;
}

void NFmiBetaProduct::InitFromJsonRead(const NFmiMetTime &theStartingTime)
{
    auto timeStatus = CheckTimeRelatedInputs(theStartingTime, itsTimeLengthInHoursString, itsTimeStepInMinutesString, fUseUtcTimesInTimeBox);
    auto rowStatus = CheckRowRelatedInputs(itsRowIndexListString, itsRowSubdirectoryTemplate, itsFileNameTemplate, fUseAutoFileNames, itsParamBoxLocation);
    auto  stationIdStatus = CheckSynopStationIdListRelatedInputs(itsSynopStationIdListString);
    MakeViewMacroInfoText(itsViewMacroPath);
}

static void FixPathDirectorySeparatorSlashes(std::string &thePath)
{
    NFmiStringTools::ReplaceChars(thePath, '/', '\\'); // muutetaan polku niin että kenoviivat on oikein päin
}

void NFmiBetaProduct::ParseJsonPair(json_spirit::Pair &thePair)
{
    // Tässä puretään NFmiBetaProduct luokan päätason pareja.
    if(thePair.name_ == gJsonName_ImageStoragePath)
    {
        itsImageStoragePath = thePair.value_.get_str();
        ::FixPathDirectorySeparatorSlashes(itsImageStoragePath);
    }
    else if(thePair.name_ == gJsonName_FileNameTemplate)
        itsFileNameTemplate = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_UseAutoFileNames)
        fUseAutoFileNames = thePair.value_.get_bool();
    else if(thePair.name_ == gJsonName_TimeLengthInHours)
        itsTimeLengthInHoursString = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_TimeStepInMinutes)
        itsTimeStepInMinutesString = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_UseUtcInTimebox)
        fUseUtcTimesInTimeBox = thePair.value_.get_bool();
    else if(thePair.name_ == gJsonName_ParamboxLocation)
        itsParamBoxLocation = static_cast<FmiDirection>(thePair.value_.get_int());
    else if(thePair.name_ == gJsonName_DisplayRuntimeInfo)
        fDisplayRunTimeInfo = thePair.value_.get_bool();
    else if(thePair.name_ == gJsonName_ShowModelOriginTime)
        fShowModelOriginTime = thePair.value_.get_bool();
    else if(thePair.name_ == gJsonName_RowIndexList)
        itsRowIndexListString = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_RowSubdirectoryTemplate)
        itsRowSubdirectoryTemplate = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_ViewMacroPath)
    {
        itsOriginalViewMacroPath = itsViewMacroPath = thePair.value_.get_str();
        ::FixPathDirectorySeparatorSlashes(itsOriginalViewMacroPath);
        ::FixPathDirectorySeparatorSlashes(itsViewMacroPath);
    }
    else if(thePair.name_ == gJsonName_WebSiteTitle)
        itsWebSiteTitleString = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_WebSiteDescription)
        itsWebSiteDescriptionString = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_CommandLine)
        itsCommandLineString = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_SelectedViewIndex)
        itsSelectedViewIndex = static_cast<BetaProductViewIndex>(thePair.value_.get_int());
    else if(thePair.name_ == gJsonName_SynopStationIdList)
        itsSynopStationIdListString = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_PackImages)
        fPackImages = thePair.value_.get_bool();
    else if(thePair.name_ == gJsonName_EnsureCurveVisibility)
        fEnsureCurveVisibility = thePair.value_.get_bool();
}

bool NFmiBetaProduct::StoreInJsonFormat(const NFmiBetaProduct &betaProduct, const std::string &theFilePath, std::string &theErrorStringOut)
{
    return ::StoreObjectInJsonFormat(betaProduct, theFilePath, "Beta-product", theErrorStringOut);
}

bool NFmiBetaProduct::ReadInJsonFormat(NFmiBetaProduct &betaProductOut, const std::string &theFilePath, std::string &theErrorStringOut)
{
    return ::ReadObjectInJsonFormat(betaProductOut, theFilePath, "Beta-product", theErrorStringOut);
}


// ********************************************************************
// *** NFmiBetaProductAutomation::NFmiTimeModeInfo osio alkaa *********
// ********************************************************************
NFmiBetaProductAutomation::NFmiTimeModeInfo::NFmiTimeModeInfo()
:itsTimeMode(kFmiBetaProductTime)
,itsWallClockOffsetInHoursString()
,itsWallClockOffsetInHours(0)
,fTimeModeInfoStatus(false)
,itsTimeModeInfoStatusString()
{
}

// Tämä on suoraa NFmiTimeModeInfo -luokan käyttöä, jolloin ei tehdä sen kummempia tarkasteluja parametreille.
NFmiBetaProductAutomation::NFmiTimeModeInfo::NFmiTimeModeInfo(TimeMode theTimeMode, double theWallClockOffsetInHours)
:itsTimeMode(theTimeMode)
, itsWallClockOffsetInHoursString()
, itsWallClockOffsetInHours(theWallClockOffsetInHours)
, fTimeModeInfoStatus(true)
, itsTimeModeInfoStatusString()
{
}

void NFmiBetaProductAutomation::NFmiTimeModeInfo::CheckTimeModeInfo(TimeMode theTimeMode, const std::string &theWallClockOffsetInHoursString)
{
    fTimeModeInfoStatus = true;
    itsTimeModeInfoStatusString.clear();
    itsTimeMode = theTimeMode;
    itsWallClockOffsetInHoursString = theWallClockOffsetInHoursString;

    if(itsTimeMode == kFmiWallClockOffsetTime)
    {
        try
        {
            itsWallClockOffsetInHours = boost::lexical_cast<double>(itsWallClockOffsetInHoursString);
        }
        catch(...)
        {
            fTimeModeInfoStatus = false;
            itsTimeModeInfoStatusString = ::GetDictionaryString("Illegal offset value");
        }
    }
}

static const std::string gJsonName_TimeMode = "TimeMode";
static const std::string gJsonName_WallClockOffsetInHours = "WallClockOffsetInHours";

json_spirit::Object NFmiBetaProductAutomation::NFmiTimeModeInfo::MakeJsonObject(const NFmiTimeModeInfo &timeModeInfo)
{
    static NFmiTimeModeInfo defaultTimeModeObject;

    json_spirit::Object jsonObject;
    jsonObject.push_back(json_spirit::Pair(gJsonName_TimeMode, timeModeInfo.itsTimeMode));

    // Tietyissä dataosissa vain default arvosta poikkeavat arvot talletetaan json objektiin.
    if(defaultTimeModeObject.itsWallClockOffsetInHoursString != timeModeInfo.itsWallClockOffsetInHoursString)
        jsonObject.push_back(json_spirit::Pair(gJsonName_WallClockOffsetInHours, timeModeInfo.itsWallClockOffsetInHoursString));

    return jsonObject;
}

void NFmiBetaProductAutomation::NFmiTimeModeInfo::ParseJsonPair(json_spirit::Pair &thePair)
{
    // Tässä puretään NFmiTimeModeInfo luokan päätason pareja.
    if(thePair.name_ == gJsonName_TimeMode)
        itsTimeMode = static_cast<NFmiBetaProductAutomation::TimeMode>(thePair.value_.get_int());
    else if(thePair.name_ == gJsonName_WallClockOffsetInHours)
        itsWallClockOffsetInHoursString = thePair.value_.get_str();
}

// Testataanvain todellisia käyttäjän antamia arvoja, ei testata mitään statuksia, 
// tai status stringeja, koska verrokkia ei välttämättä päivitetä.
bool NFmiBetaProductAutomation::NFmiTimeModeInfo::operator==(const NFmiBetaProductAutomation::NFmiTimeModeInfo &other) const
{
    if(itsTimeMode != other.itsTimeMode)
        return false;
    if(itsWallClockOffsetInHours != other.itsWallClockOffsetInHours)
        return false;

    return true;
}

bool NFmiBetaProductAutomation::NFmiTimeModeInfo::operator!=(const NFmiBetaProductAutomation::NFmiTimeModeInfo &other) const
{
    return !(*this == other);
}

// ********************************************************************
// *** NFmiBetaProductAutomation::NFmiTriggerModeInfo osio alkaa ******
// ********************************************************************
NFmiBetaProductAutomation::NFmiTriggerModeInfo::NFmiTriggerModeInfo()
:itsTriggerMode(kFmiFixedTimes)
, itsFixedRunTimesString()
, itsFixedRunTimes()
, itsRunTimeStepInHoursString()
, itsRunTimeStepInHours(0)
, itsFirstRunTimeOfDayString()
, itsFirstRunTimeOffsetInMinutes(0)
, itsTriggerDataString()
, itsTriggerDataList()
, fTriggerModeInfoStatus(false)
, itsTriggerModeInfoStatusString()
{
}

void NFmiBetaProductAutomation::NFmiTriggerModeInfo::CheckTriggerModeInfo(int theTriggerMode, const std::string &theFixedRunTimesString, const std::string &theRunTimeStepInHoursString, const std::string &theFirstRunTimeOfDayString, const std::string &theTriggerDataString)
{
    // Alustetaan arvot
    fTriggerModeInfoStatus = false;
    itsTriggerModeInfoStatusString.clear();
    itsTriggerMode = static_cast<NFmiBetaProductAutomation::TriggerMode>(theTriggerMode);
    itsFixedRunTimesString = theFixedRunTimesString;
    itsRunTimeStepInHoursString = theRunTimeStepInHoursString;
    itsFirstRunTimeOfDayString = theFirstRunTimeOfDayString;
    itsTriggerDataString = theTriggerDataString;

    // Kutsutaan tarvittavaa tarkastus funtiota triggerModesta riippuen
    if(itsTriggerMode == kFmiFixedTimes)
        CheckFixedRunTimes();
    else if(itsTriggerMode == kFmiTimeStep)
        CheckTimeStep();
    else if(itsTriggerMode == kFmiDataTrigger)
        CheckTriggerData();
    else
        itsTriggerModeInfoStatusString = ::GetDictionaryString("Unknown trigger mode") + ": " + boost::lexical_cast<std::string>(theTriggerMode);
}

void NFmiBetaProductAutomation::NFmiTriggerModeInfo::CheckFixedRunTimes()
{
    std::string possibleErrorMessage;
    itsFixedRunTimes = FixedRunTime::ParseFixedRunTimesString(itsFixedRunTimesString, possibleErrorMessage);
    if(itsFixedRunTimes.empty())
    {
        fTriggerModeInfoStatus = false;
        if(!possibleErrorMessage.empty())
            itsTriggerModeInfoStatusString = possibleErrorMessage;
        else
            itsTriggerModeInfoStatusString = ::GetDictionaryString("Fixed time list was empty");
    }
    else
    {
        fTriggerModeInfoStatus = true;
        itsTriggerModeInfoStatusString = ::GetDictionaryString("Fixed time list used with");
        itsTriggerModeInfoStatusString += " " + std::to_string(itsFixedRunTimes.size()) + " " + ::GetDictionaryString("times");
    }
}

static std::string MakeDoubleCheckErrorString(const std::string &theValueName, const std::string &theValueString, const std::string &theExplanationString)
{
    std::string str = theValueName;
    str += " ('";
    str += theValueString;
    str += "') ";
    str += theExplanationString;

    return str;
}

static bool StringToDoubleConversion(const std::string &theValueName, const std::string &theValueString, double &theValueOut, std::string &theErrorStringOut)
{
    try
    {
        theValueOut = boost::lexical_cast<double>(theValueString);
        return true;
    }
    catch(std::exception &e)
    {
        theErrorStringOut = ::MakeDoubleCheckErrorString(theValueName, theValueString, ::GetDictionaryString("was not a real number"));
        theErrorStringOut += ":\n";
        theErrorStringOut += e.what();
    }
    return false;
}

static bool CheckTimeOfDayInput(const std::string &theTimeOfDayStringIn, const std::string &theVariableNameIn, int &theTimeOfDayOffsetInMinutesOut, std::string &theErrorStringOut)
{
    try
    {
        std::vector<std::string> splitValues = NFmiStringTools::Split(theTimeOfDayStringIn, ":");
        if(splitValues.size() == 2 && splitValues[1].size() == 2)
        {
            int hours = boost::lexical_cast<int>(splitValues[0]);
            int minutes = boost::lexical_cast<int>(splitValues[1]);
            if(hours >= 0 && hours < 24 && minutes >= 0 && minutes < 60)
            {
                theTimeOfDayOffsetInMinutesOut = hours * 60 + minutes;
                return true;
            }
        }
    }
    catch(...)
    {
    }

    theErrorStringOut = theVariableNameIn;
    theErrorStringOut += " ";
    theErrorStringOut += ::GetDictionaryString("input was illegal, use 'HH:mm' - format");
    return false;
}

void NFmiBetaProductAutomation::NFmiTriggerModeInfo::CheckTimeStep()
{
    std::string timeStepVariableName = ::GetDictionaryString(NFmiBetaProductAutomation::RunTimeStepInHoursTitle().c_str());
    if(::StringToDoubleConversion(timeStepVariableName, itsRunTimeStepInHoursString, itsRunTimeStepInHours, itsTriggerModeInfoStatusString))
    {
        if(itsRunTimeStepInHours <= 0)
            itsTriggerModeInfoStatusString = ::MakeDoubleCheckErrorString(timeStepVariableName, itsRunTimeStepInHoursString, ::GetDictionaryString("must be positive value"));
        else if(itsRunTimeStepInHours > 24)
            itsTriggerModeInfoStatusString = ::MakeDoubleCheckErrorString(timeStepVariableName, itsRunTimeStepInHoursString, ::GetDictionaryString("must be <= 24 h"));
        else
        {
            std::string firstRunTimeOfDayVariableName = ::GetDictionaryString(NFmiBetaProductAutomation::FirstRunTimeOfDayTitle().c_str());
            if(::CheckTimeOfDayInput(itsFirstRunTimeOfDayString, firstRunTimeOfDayVariableName, itsFirstRunTimeOffsetInMinutes, itsTriggerModeInfoStatusString))
            {
                fTriggerModeInfoStatus = true;
                itsTriggerModeInfoStatusString = ::GetDictionaryString("Using time step mode with values");
                itsTriggerModeInfoStatusString += ":\n";
                itsTriggerModeInfoStatusString += ::GetDictionaryString(NFmiBetaProductAutomation::RunTimeStepInHoursTitle().c_str());
                itsTriggerModeInfoStatusString += " = ";
                itsTriggerModeInfoStatusString += itsRunTimeStepInHoursString;
                itsTriggerModeInfoStatusString += ", ";
                itsTriggerModeInfoStatusString += ::GetDictionaryString(NFmiBetaProductAutomation::FirstRunTimeOfDayTitle().c_str());
                itsTriggerModeInfoStatusString += " = ";
                itsTriggerModeInfoStatusString += itsFirstRunTimeOfDayString;
            }
        }
    }
}

static std::string MakeTriggerDataErrorLine(const std::string possibleParameterErrorListStr)
{
    return ::GetDictionaryString("Error with these") + ": " + possibleParameterErrorListStr;
}

static void AddParameterToErrorString(const std::string &parameterStr, std::string& errorStr)
{
    if(!errorStr.empty())
    {
        errorStr += ", ";
    }
    errorStr += parameterStr;
}


void NFmiBetaProductAutomation::NFmiTriggerModeInfo::CheckTriggerData()
{
    itsTriggerDataList.clear();
    std::string possibleParameterErrorListStr;
    std::vector<std::string> parameterStringList;
    boost::split(parameterStringList, itsTriggerDataString, boost::is_any_of(","));
    for(std::string parameterStr : parameterStringList)
    {
        boost::trim(parameterStr);
        if(!parameterStr.empty())
        {
            try
            {
                auto dataInfo = NFmiSmartToolIntepreter::CheckForVariableDataType(parameterStr);
                if(dataInfo.first)
                {
                    itsTriggerDataList.push_back(dataInfo.second);
                }
                else
                {
                    AddParameterToErrorString(parameterStr, possibleParameterErrorListStr);
                }
            }
            catch(std::exception& )
            {
                AddParameterToErrorString(parameterStr, possibleParameterErrorListStr);
            }
        }
    }

    if(!itsTriggerDataList.empty())
    {
        itsTriggerModeInfoStatusString = ::GetDictionaryString("Found");
        itsTriggerModeInfoStatusString += " " + std::to_string(itsTriggerDataList.size()) + " " + ::GetDictionaryString("trigger data");
        if(possibleParameterErrorListStr.empty())
        {
            fTriggerModeInfoStatus = true;
        }
        else
        {
            fTriggerModeInfoStatus = false;
            itsTriggerModeInfoStatusString += "\n" + ::MakeTriggerDataErrorLine(possibleParameterErrorListStr);
        }
    }
    else
    {
        fTriggerModeInfoStatus = false;
        itsTriggerModeInfoStatusString = ::GetDictionaryString("Found no trigger data");
        if(!possibleParameterErrorListStr.empty())
        {
            itsTriggerModeInfoStatusString += "\n" + ::MakeTriggerDataErrorLine(possibleParameterErrorListStr);
        }
    }
}

// Tämä toimii vain itsTriggerMode == kFmiTimeStep -moodissa!!!!
NFmiMetTime NFmiBetaProductAutomation::NFmiTriggerModeInfo::MakeFirstRunTimeOfGivenDay(const NFmiMetTime &theTime) const
{
    NFmiMetTime aTime(theTime.GetYear(), theTime.GetMonth(), theTime.GetDay(), 0, 0, 0, 1);
    aTime.ChangeByMinutes(itsFirstRunTimeOffsetInMinutes);
    return aTime;
}

NFmiMetTime NFmiBetaProductAutomation::NFmiTriggerModeInfo::CalcNextDueTimeWithFixedTimes(const NFmiMetTime& theLastRunTime) const
{
    // Etsitään fixed listasta seuraava theLastRunTime:ia suurempi aika (HH:mm tasolla suurempi)
    auto iter = std::find_if(itsFixedRunTimes.begin(), itsFixedRunTimes.end(), [&](const auto& fixedTime) {return fixedTime > theLastRunTime; });
    if(iter != itsFixedRunTimes.end())
    {
        return iter->MakeMetTime();
    }
    else
    {
        // Jos ei löydy tälle päivälle enää aikoja, tehdään listan 1. ajasta seuraavan päivän aika
        auto firstIter = itsFixedRunTimes.begin();
        if(firstIter != itsFixedRunTimes.end())
        {
            auto firstNextDayTime = firstIter->MakeMetTime();
            firstNextDayTime.ChangeByDays(1);
            return firstNextDayTime;
        }
    }

    return NFmiMetTime::gMissingTime; // Mahdollinen virhetilanne ja jos aikalista oli tyhjä, palautetaan puuttuva aika
}

NFmiMetTime NFmiBetaProductAutomation::NFmiTriggerModeInfo::CalcNextDueTimeWithTimeSteps(const NFmiMetTime& theLastRunTime) const
{
    NFmiMetTime aTime = MakeFirstRunTimeOfGivenDay(theLastRunTime);
    for(;; aTime.ChangeByMinutes(boost::math::iround(itsRunTimeStepInHours * 60)))
    {
        if(aTime > theLastRunTime)
        {
            if(aTime.GetDay() != theLastRunTime.GetDay())
                aTime = MakeFirstRunTimeOfGivenDay(aTime); // jos ollaan menty jo toiseen päivään, pitää sille laskea uusi alkuaika!
            return aTime;
        }

        // Loput tarkastelut ovat ns sanity checkejä, jos on vääriä lähtöarvoja datassta, ja niitä ei ole tarkasteltu kunnolla, jotta koodi ei jää ikilooppiin.
        if(aTime.GetDay() != theLastRunTime.GetDay())
            break;
        if(itsRunTimeStepInHours <= 0)
            break;
    }

    // Jos ei löytynyt enää tälle päivälle aikoja, tehdään seuraavan päivän 1. aika tässä
    NFmiMetTime firstNextDayTime = MakeFirstRunTimeOfGivenDay(theLastRunTime);
    firstNextDayTime.ChangeByDays(1);
    return firstNextDayTime;
}

// Lasketaan milloin automaatio pitää suorittaa seuraavan kerrran, kun tiedetään milloin se 
// on viimeksi ajettu ja mikä on nykyhetki
NFmiMetTime NFmiBetaProductAutomation::NFmiTriggerModeInfo::CalcNextDueTime(const NFmiMetTime &theLastRunTime, bool automationModeOn) const
{
    if(automationModeOn)
    {
        if(itsTriggerMode == kFmiFixedTimes)
        {
            return CalcNextDueTimeWithFixedTimes(theLastRunTime);
        }
        else if(itsTriggerMode == kFmiTimeStep)
        {
            return CalcNextDueTimeWithTimeSteps(theLastRunTime);
        }
    }

    return NFmiMetTime::gMissingTime; // virhetilanne, palautetaan puuttuva aika
}

static bool CheckIfInfoWasOnTriggerList(boost::shared_ptr<NFmiFastQueryInfo> &info, const std::vector<std::string>& loadedDataTriggerList, const std::string &automationName, int dataTriggerWaitForMinutes)
{
    if(info)
    {
        auto filePatternIter = std::find(loadedDataTriggerList.begin(), loadedDataTriggerList.end(), info->DataFilePattern());
        if(filePatternIter != loadedDataTriggerList.end())
        {
            std::string debugTriggerMessage = "Beta automation '";
            debugTriggerMessage += automationName;
            debugTriggerMessage += "' was triggered by loading data: ";
            debugTriggerMessage += info->DataFileName();
            if(dataTriggerWaitForMinutes > 0)
            {
                debugTriggerMessage += ", but trigger will be delayed by ";
                debugTriggerMessage += std::to_string(dataTriggerWaitForMinutes);
                debugTriggerMessage += " minutes";
            }
            CatLog::logMessage(debugTriggerMessage, CatLog::Severity::Debug, CatLog::Category::Operational);
            return true;
        }
    }
    return false;
}

static std::string MakeLevelString(const NFmiLevel& level)
{
    std::string str;
    switch(level.LevelType())
    {
    case kFmiPressureLevel:
        str += "pressure ";
        break;
    case kFmiHybridLevel:
        str += "hybrid lev";
        break;
    case kFmiHeight:
        str += "height z";
        break;
    case kFmiFlightLevel:
        str += "flight-level fl";
        break;
    default:
        str += "unknown level type ";
        break;
    }

    str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(level.LevelValue(), 1);
    return str;
}

static void DoPossibleLogWarningAboutNonExistingLevel(NFmiInfoOrganizer& infoOrganizer, const NFmiDefineWantedData& triggerData, const std::vector<std::string>& loadedDataTriggerList, const std::string& automationName)
{
    auto* level = triggerData.UsedLevel();
    if(level)
    {
        for(const auto& loadedDataFilenameFilter : loadedDataTriggerList)
        {
            auto infos = infoOrganizer.GetInfos(loadedDataFilenameFilter);
            for(auto& info : infos)
            {
                if(info->SizeLevels() > 1)
                {
                    if(*(info->Producer()) == triggerData.producer_)
                    {
                        if(info->LevelType() == level->LevelType())
                        {
                            std::string debugTriggerMessage = "Beta automation '";
                            debugTriggerMessage += automationName;
                            debugTriggerMessage += "' has data trigger level (";
                            debugTriggerMessage += ::MakeLevelString(*level);
                            debugTriggerMessage += ") which was not found in loaded level data of wanted type: ";
                            debugTriggerMessage += info->DataFileName();
                            debugTriggerMessage += ", maybe beta-automation trigger data should be fixed?";
                            CatLog::logMessage(debugTriggerMessage, CatLog::Severity::Warning, CatLog::Category::Operational);
                        }
                    }
                }
            }
        }
    }
}

bool NFmiBetaProductAutomation::NFmiTriggerModeInfo::HasDataTriggerBeenLoaded(const std::vector<std::string>& loadedDataTriggerList, NFmiInfoOrganizer& infoOrganizer, const std::string& automationName, bool automationModeOn, int& postponeTriggerInMinutesOut) const
{
    if(automationModeOn)
    {
        if(itsTriggerMode == kFmiDataTrigger)
        {
            if(!itsTriggerDataList.empty())
            {
                for(const auto& triggerData : itsTriggerDataList)
                {
                    auto wantedProducerId = triggerData.producer_.GetIdent();
                    if(wantedProducerId == kFmiSYNOP || wantedProducerId == kFmiFlashObs)
                    {
                        // Ainakin Suomessa Synop ja Flash datat ovat multi-data juttuja, jolloin
                        // pitää tutkia että onko mikään kyseisen tuottajan datoista juuri nyt ladattu
                        auto infoList = infoOrganizer.GetInfos(wantedProducerId);
                        for(auto& info : infoList)
                        {
                            if(::CheckIfInfoWasOnTriggerList(info, loadedDataTriggerList, automationName, triggerData.dataTriggerRelatedWaitForMinutes_))
                            {
                                postponeTriggerInMinutesOut = triggerData.dataTriggerRelatedWaitForMinutes_;
                                return true;
                            }
                        }
                    }
                    else
                    {
                        // NFmiExtraMacroParamData::FindWantedInfo metodille pitää sallia etsiä myös asemadatoja (3. parametri true).
                        auto wantedInfoData = NFmiExtraMacroParamData::FindWantedInfo(infoOrganizer, triggerData, true);
                        if(::CheckIfInfoWasOnTriggerList(wantedInfoData.foundInfo_, loadedDataTriggerList, automationName, triggerData.dataTriggerRelatedWaitForMinutes_))
                        {
                            postponeTriggerInMinutesOut = triggerData.dataTriggerRelatedWaitForMinutes_;
                            return true;
                        }
                        else if(triggerData.IsParamProducerLevel())
                        {
                            ::DoPossibleLogWarningAboutNonExistingLevel(infoOrganizer, triggerData, loadedDataTriggerList, automationName);
                        }
                    }
                }
            }
        }
    }
    return false;
}

static const std::string gJsonName_TriggerMode = "TriggerMode";
static const std::string gJsonName_FixedRunTimes = "FixedRunTimes";
static const std::string gJsonName_RunTimeStepInHours = "RunTimeStepInHours";
static const std::string gJsonName_FirstRunTimeOfDay = "FirstRunTimeOfDay";
static const std::string gJsonName_TriggerData = "TriggerData";

json_spirit::Object NFmiBetaProductAutomation::NFmiTriggerModeInfo::MakeJsonObject(const NFmiBetaProductAutomation::NFmiTriggerModeInfo &triggerModeInfo)
{
    static NFmiTriggerModeInfo defaultTriggerModeInfo;

    json_spirit::Object jsonObject;
    jsonObject.push_back(json_spirit::Pair(gJsonName_TriggerMode, triggerModeInfo.itsTriggerMode));

    // Tietyissä dataosissa vain default arvosta poikkeavat arvot talletetaan json objektiin.
    if(defaultTriggerModeInfo.itsFixedRunTimesString != triggerModeInfo.itsFixedRunTimesString)
        jsonObject.push_back(json_spirit::Pair(gJsonName_FixedRunTimes, triggerModeInfo.itsFixedRunTimesString));
    if(defaultTriggerModeInfo.itsRunTimeStepInHoursString != triggerModeInfo.itsRunTimeStepInHoursString)
        jsonObject.push_back(json_spirit::Pair(gJsonName_RunTimeStepInHours, triggerModeInfo.itsRunTimeStepInHoursString));
    if(defaultTriggerModeInfo.itsFirstRunTimeOfDayString != triggerModeInfo.itsFirstRunTimeOfDayString)
        jsonObject.push_back(json_spirit::Pair(gJsonName_FirstRunTimeOfDay, triggerModeInfo.itsFirstRunTimeOfDayString));
    if(defaultTriggerModeInfo.itsTriggerDataString != triggerModeInfo.itsTriggerDataString)
        jsonObject.push_back(json_spirit::Pair(gJsonName_TriggerData, triggerModeInfo.itsTriggerDataString));

    return jsonObject;
}

void NFmiBetaProductAutomation::NFmiTriggerModeInfo::ParseJsonPair(json_spirit::Pair &thePair)
{
    // Tässä puretään NFmiTriggerModeInfo luokan päätason pareja.
    if(thePair.name_ == gJsonName_TriggerMode)
        itsTriggerMode = static_cast<NFmiBetaProductAutomation::TriggerMode>(thePair.value_.get_int());
    else if(thePair.name_ == gJsonName_FixedRunTimes)
    {
        itsFixedRunTimesString = thePair.value_.get_str();
        // HUOM! Laske ajoajat tässä
    }
    else if(thePair.name_ == gJsonName_RunTimeStepInHours)
        itsRunTimeStepInHoursString = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_FirstRunTimeOfDay)
        itsFirstRunTimeOfDayString = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_TriggerData)
    {
        itsTriggerDataString = thePair.value_.get_str();
        CheckTriggerData();
    }
}

// Testataanvain todellisia käyttäjän antamia arvoja, ei testata mitään statuksia, 
// tai status stringeja, koska verrokkia ei välttämättä päivitetä.
bool NFmiBetaProductAutomation::NFmiTriggerModeInfo::operator==(const NFmiBetaProductAutomation::NFmiTriggerModeInfo &other) const
{
    if(itsTriggerMode != other.itsTriggerMode)
        return false;
    if(itsFixedRunTimes != other.itsFixedRunTimes)
        return false;
    if(itsRunTimeStepInHours != other.itsRunTimeStepInHours)
        return false;
    if(itsFirstRunTimeOffsetInMinutes != other.itsFirstRunTimeOffsetInMinutes)
        return false;
    if(itsTriggerDataList != other.itsTriggerDataList)
        return false;

    return true;
}

bool NFmiBetaProductAutomation::NFmiTriggerModeInfo::operator!=(const NFmiBetaProductAutomation::NFmiTriggerModeInfo &other) const
{
    return !(*this == other);
}

// ********************************************************************
// *********  NFmiBetaProductAutomation osio alkaa  *******************
// ********************************************************************

const std::string NFmiBetaProductAutomation::itsFirstRunTimeOfDayTitle = "First run of day [HH:mm]";
const std::string NFmiBetaProductAutomation::itsRunTimeStepInHoursTitle = "Time step [h]";


BaseDirectoryGetterFunctionType NFmiBetaProductAutomation::itsBetaProductionBaseDirectoryGetter;

void NFmiBetaProductAutomation::SetBetaProductionBaseDirectoryGetter(BaseDirectoryGetterFunctionType &getterFunction)
{
    NFmiBetaProductAutomation::itsBetaProductionBaseDirectoryGetter = getterFunction;
}

NFmiBetaProductAutomation::NFmiBetaProductAutomation()
: itsBetaProductPath()
, itsOriginalBetaProductPath()
, fBetaProductPathStatus(false)
, itsBetaProductPathStatusString()
, itsTriggerModeInfo()
, itsStartTimeModeInfo()
, itsEndTimeModeInfo()
{
}

bool NFmiBetaProductAutomation::InputWasGood()
{
    return fBetaProductPathStatus && itsTriggerModeInfo.fTriggerModeInfoStatus && itsStartTimeModeInfo.fTimeModeInfoStatus && itsEndTimeModeInfo.fTimeModeInfoStatus;
}

void NFmiBetaProductAutomation::CheckBetaProductPath(const std::string &theBetaProductPath)
{
    fBetaProductPathStatus = false;
    itsBetaProductPathStatusString.clear();
    itsOriginalBetaProductPath = theBetaProductPath;

    bool extensionAdded = false;
    itsBetaProductPath = PathUtils::getTrueFilePath(theBetaProductPath, NFmiBetaProductAutomation::itsBetaProductionBaseDirectoryGetter(), NFmiBetaProductionSystem::BetaProductFileExtension(), &extensionAdded);

    if(itsBetaProductPath.empty())
        itsBetaProductPathStatusString = ::GetDictionaryString("No Beta product in use");
    else
    {
        NFmiFileString fileString(itsBetaProductPath);

        // Tutkitaan löytyykö annettua viewMacroa ollenkaan
        if(NFmiFileSystem::FileExists(itsBetaProductPath))
        {
            fBetaProductPathStatus = true;
            itsBetaProductPathStatusString = ::GetDictionaryString("Using Beta product");
            itsBetaProductPathStatusString += ": ";
            itsBetaProductPathStatusString += ::GetFileNameHeaderInHyphens(fileString, !extensionAdded);
        }
        else
        {
            itsBetaProductPathStatusString = ::GetDictionaryString("Unable to locate given Beta product");
            itsBetaProductPathStatusString += ": ";
            itsBetaProductPathStatusString += ::GetFileNameHeaderInHyphens(fileString, !extensionAdded);
        }
    }
}

void NFmiBetaProductAutomation::CheckTriggerModeInfo(int theTriggerMode, const std::string &theFixedRunTimesString, const std::string &theRunTimeStepInHoursString, const std::string &theRunTimeOffsetInHoursString, const std::string &theTriggerDataString)
{
    itsTriggerModeInfo.CheckTriggerModeInfo(theTriggerMode, theFixedRunTimesString, theRunTimeStepInHoursString, theRunTimeOffsetInHoursString, theTriggerDataString);
}

bool NFmiBetaProductAutomation::TriggerModeInfoStatus() const 
{ 
    return itsTriggerModeInfo.fTriggerModeInfoStatus;
}

const std::string& NFmiBetaProductAutomation::TriggerModeInfoStatusString() const 
{ 
    return  itsTriggerModeInfo.itsTriggerModeInfoStatusString; 
}

void NFmiBetaProductAutomation::CheckStartTimeModeInfo(TimeMode theTimeMode, const std::string &theWallClockOffsetInHoursString)
{
    itsStartTimeModeInfo.CheckTimeModeInfo(theTimeMode, theWallClockOffsetInHoursString);
}

void NFmiBetaProductAutomation::CheckEndTimeModeInfo(TimeMode theTimeMode, const std::string &theWallClockOffsetInHoursString)
{
    itsEndTimeModeInfo.CheckTimeModeInfo(theTimeMode, theWallClockOffsetInHoursString);
}

bool NFmiBetaProductAutomation::StartTimeModeInfoStatus() const 
{ 
    return itsStartTimeModeInfo.fTimeModeInfoStatus;
}

const std::string& NFmiBetaProductAutomation::StartTimeModeInfoStatusString() const 
{ 
    return  itsStartTimeModeInfo.itsTimeModeInfoStatusString;
}

bool NFmiBetaProductAutomation::EndTimeModeInfoStatus() const 
{ 
    return itsEndTimeModeInfo.fTimeModeInfoStatus;
}

const std::string& NFmiBetaProductAutomation::EndTimeModeInfoStatusString() const 
{ 
    return  itsEndTimeModeInfo.itsTimeModeInfoStatusString;
}

// Jos fReloadFromFile on tosi, pakotetaan Beta-product luettavaksi tiedostosta aina.
// Tämä on hyvä tehdä kun ajetaan automaatioita ja joku on saattanut tehdä muutoksia tuotteeseen.
std::shared_ptr<NFmiBetaProduct> NFmiBetaProductAutomation::GetBetaProduct(bool fReloadFromFile)
{
    if(fReloadFromFile || !(itsBetaProduct && itsBetaProductPath == itsLoadedBetaProductAbsolutePath))
    {
        itsBetaProduct = std::make_shared<NFmiBetaProduct>(); // nollataan varmuuden vuoksi itsBetaProduct, johon tiedostosta sitten luetaan uusi otus
        if(BetaProduct::LoadObjectFromKnownFileInJsonFormat(*itsBetaProduct, itsBetaProductPath, "Beta-product", true))
        {
            itsBetaProduct->InitFromJsonRead(NFmiMetTime(1));
            itsLoadedBetaProductAbsolutePath = itsBetaProductPath;
        }
        else
            itsBetaProduct.reset(); // luvun virhetilanteessa nollataan olio
    }
    return itsBetaProduct;
}

static const std::string gJsonName_BetaProductPath = "BetaProductPath";
static const std::string gJsonName_OriginalBetaProductPath = "OriginalBetaProductPath";
static const std::string gJsonName_TriggerModeInfo = "TriggerModeInfo";
static const std::string gJsonName_StartTimeModeInfo = "StartTimeModeInfo";
static const std::string gJsonName_EndTimeModeInfo = "EndTimeModeInfo";

json_spirit::Object NFmiBetaProductAutomation::MakeJsonObject(const NFmiBetaProductAutomation &betaProductAutomation)
{
    json_spirit::Object jsonObject;
    ::AddNonEmptyStringJsonPair(betaProductAutomation.BetaProductPath(), gJsonName_BetaProductPath, jsonObject);
    ::AddNonEmptyStringJsonPair(betaProductAutomation.OriginalBetaProductPath(), gJsonName_OriginalBetaProductPath, jsonObject);
    json_spirit::Object triggerModeInfoJsonObject = NFmiTriggerModeInfo::MakeJsonObject(betaProductAutomation.TriggerModeInfo());
    if(!triggerModeInfoJsonObject.empty())
        jsonObject.push_back(json_spirit::Pair(gJsonName_TriggerModeInfo, triggerModeInfoJsonObject));
    json_spirit::Object startTimeModeInfoJsonObject = NFmiTimeModeInfo::MakeJsonObject(betaProductAutomation.StartTimeModeInfo());
    if(!startTimeModeInfoJsonObject.empty())
        jsonObject.push_back(json_spirit::Pair(gJsonName_StartTimeModeInfo, startTimeModeInfoJsonObject));
    json_spirit::Object endTimeModeInfoJsonObject = NFmiTimeModeInfo::MakeJsonObject(betaProductAutomation.EndTimeModeInfo());
    if(!endTimeModeInfoJsonObject.empty())
        jsonObject.push_back(json_spirit::Pair(gJsonName_EndTimeModeInfo, endTimeModeInfoJsonObject));

    return jsonObject;
}

void NFmiBetaProductAutomation::ParseJsonPair(json_spirit::Pair &thePair)
{
    // Tässä puretään NFmiTriggerModeInfo luokan päätason pareja.
    if(thePair.name_ == gJsonName_BetaProductPath)
        itsBetaProductPath = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_OriginalBetaProductPath)
        itsOriginalBetaProductPath = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_TriggerModeInfo)
        ::ParseJsonValue(itsTriggerModeInfo, thePair.value_);
    else if(thePair.name_ == gJsonName_StartTimeModeInfo)
        ::ParseJsonValue(itsStartTimeModeInfo, thePair.value_);
    else if(thePair.name_ == gJsonName_EndTimeModeInfo)
        ::ParseJsonValue(itsEndTimeModeInfo, thePair.value_);
}

// Tätä kutsutaan kun esim. luetaan data tiedostosta ja tehdään täisi tarkistus kaikille osille
void NFmiBetaProductAutomation::DoFullChecks()
{
    CheckBetaProductPath(itsOriginalBetaProductPath);
    CheckTriggerModeInfo(itsTriggerModeInfo.itsTriggerMode, itsTriggerModeInfo.itsFixedRunTimesString, itsTriggerModeInfo.itsRunTimeStepInHoursString, itsTriggerModeInfo.itsFirstRunTimeOfDayString, itsTriggerModeInfo.itsTriggerDataString);
    CheckStartTimeModeInfo(itsStartTimeModeInfo.itsTimeMode, itsStartTimeModeInfo.itsWallClockOffsetInHoursString);
    CheckEndTimeModeInfo(itsEndTimeModeInfo.itsTimeMode, itsEndTimeModeInfo.itsWallClockOffsetInHoursString);
}

// Tässä tehdään beta automation listassa oleville Beta-automaation tilaa kuvaava satus info teksti.
// Kyseessä on siis vain parilla sanalla sanottu tilanne, jotta se mahtuisi dialogissa olevaan sarakkeeseen.
std::string NFmiBetaProductAutomation::MakeShortStatusErrorString()
{
    if(!fBetaProductPathStatus)
        return "Beta-prod path error";
    else if(!itsTriggerModeInfo.fTriggerModeInfoStatus)
        return "Trigger mode error";
    else if(!itsStartTimeModeInfo.fTimeModeInfoStatus)
        return "Start time mode error";
    else if(!itsEndTimeModeInfo.fTimeModeInfoStatus)
        return "End time mode error";
    else
        return "Automation ok";
}

bool NFmiBetaProductAutomation::StoreInJsonFormat(const NFmiBetaProductAutomation &betaProductAutomation, const std::string &theFilePath, std::string &theErrorStringOut)
{
    return ::StoreObjectInJsonFormat(betaProductAutomation, theFilePath, "Beta-product-automation", theErrorStringOut);
}

bool NFmiBetaProductAutomation::ReadInJsonFormat(NFmiBetaProductAutomation &betaProductAutomation, const std::string &theFilePath, std::string &theErrorStringOut)
{
    bool status = ::ReadObjectInJsonFormat(betaProductAutomation, theFilePath, "Beta-product-automation", theErrorStringOut);
    betaProductAutomation.DoFullChecks();
    return status;
}

// Testataanvain todellisia käyttäjän antamia arvoja, ei testata mitään statuksia, 
// tai status stringeja, koska verrokkia ei välttämättä päivitetä.
bool NFmiBetaProductAutomation::operator==(const NFmiBetaProductAutomation &other) const
{
    if(itsBetaProductPath != other.itsBetaProductPath)
        return false;
    if(itsOriginalBetaProductPath != other.itsOriginalBetaProductPath)
        return false;
    if(itsTriggerModeInfo != other.itsTriggerModeInfo)
        return false;
    if(itsStartTimeModeInfo != other.itsStartTimeModeInfo)
        return false;
    if(itsEndTimeModeInfo != other.itsEndTimeModeInfo)
        return false;

    return true;
}

bool NFmiBetaProductAutomation::operator!=(const NFmiBetaProductAutomation &other) const
{
    return !(*this == other);
}

// ********************************************************************
// ***  NFmiBetaProductAutomationListItem osio alkaa  *****************
// ********************************************************************

NFmiBetaProductAutomationListItem::NFmiBetaProductAutomationListItem()
:fEnable(false)
, itsBetaProductAutomationPath()
, itsBetaProductAutomationAbsolutePath()
, itsBetaProductAutomation()
, itsStatus(kFmiListItemOk)
, itsLastRunTime(1) // Otetaan viime ajoajaksi nyky utc hetki 1 minuutin tarkkuudella
, fProductsHaveBeenGenerated(false)
, itsNextRunTime(NFmiMetTime::gMissingTime)
{
}

NFmiBetaProductAutomationListItem::NFmiBetaProductAutomationListItem(const std::string &theBetaAutomationPath)
:fEnable(false)
, itsBetaProductAutomationPath(theBetaAutomationPath)
, itsBetaProductAutomationAbsolutePath()
, itsBetaProductAutomation()
, itsStatus(kFmiListItemOk)
, itsLastRunTime(1) // Otetaan viime ajoajaksi nyky utc hetki 1 minuutin tarkkuudella
, fProductsHaveBeenGenerated(false)
, itsNextRunTime(NFmiMetTime::gMissingTime)
{
}

// Tätä kutsutaan kun esim. luetaan data tiedostosta ja tehdään täysi tarkistus kaikille osille
void NFmiBetaProductAutomationListItem::DoFullChecks(bool automationModeOn)
{
    itsStatus = kFmiListItemOk;
    if(itsBetaProductAutomation)
    {
        itsBetaProductAutomation->DoFullChecks();
        if(GetErrorStatus() == kFmiListItemOk) // Jos automaatio tuote on ok, lasketaan aina seuraava ajoaika valmiiksi
        {
            itsNextRunTime = itsBetaProductAutomation->TriggerModeInfo().CalcNextDueTime(itsLastRunTime, automationModeOn);
        }
    }
    else
        itsStatus = kFmiListItemReadError;

}

bool NFmiBetaProductAutomationListItem::IsEmpty() const
{
    if(itsBetaProductAutomation)
        return false;
    else
        return true;
}

std::string NFmiBetaProductAutomationListItem::AutomationName() const
{
    try
    {
        boost::filesystem::path p(itsBetaProductAutomationPath);
        return p.stem().string();
    }
    catch(...)
    {
        return "FileName-error";
    }
}

std::string NFmiBetaProductAutomationListItem::ShortStatusText() const
{
    if(itsStatus == kFmiListItemReadError)
        return "Automation read error";
    else if(itsStatus == kFmiListItemSameAutomationsInList)
        return "Same automation multiple times";
    else if(itsBetaProductAutomation)
        return itsBetaProductAutomation->MakeShortStatusErrorString();
    else
        return "Empty-Automation-error";
}

NFmiBetaProductAutomationListItem::ErrorStatus NFmiBetaProductAutomationListItem::GetErrorStatus() const
{
    if(itsStatus == kFmiListItemReadError)
        return itsStatus;
    else if(itsBetaProductAutomation)
    {
        if(!itsBetaProductAutomation->InputWasGood())
            return kFmiListItemAutomationError;
    }
        
    return itsStatus;
}

std::string NFmiBetaProductAutomationListItem::FullAutomationPath() const
{
    return itsBetaProductAutomationAbsolutePath;
}


static const std::string gJsonName_BetaAutomationListItemEnable = "Enable";
static const std::string gJsonName_BetaAutomationListItemPath = "Beta-automation path";
static const std::string gJsonName_BetaAutomationListItemAbsolutePath = "Beta-automation absolute path";

// Tämä tallentaa json objektiin vain enable -lipun ja polun käytettyyn Beta-automaatioon. 
// Mutta ei itse Beta-automaatio otusta!!
json_spirit::Object NFmiBetaProductAutomationListItem::MakeJsonObject(const NFmiBetaProductAutomationListItem &listItem)
{
    NFmiBetaProductAutomationListItem defaultListItem;

    json_spirit::Object jsonObject;
    if(defaultListItem.fEnable != listItem.fEnable)
        jsonObject.push_back(json_spirit::Pair(gJsonName_BetaAutomationListItemEnable, listItem.fEnable));
    ::AddNonEmptyStringJsonPair(listItem.itsBetaProductAutomationPath, gJsonName_BetaAutomationListItemPath, jsonObject);
    ::AddNonEmptyStringJsonPair(listItem.itsBetaProductAutomationAbsolutePath, gJsonName_BetaAutomationListItemAbsolutePath, jsonObject);

    return jsonObject;
}

void NFmiBetaProductAutomationListItem::ParseJsonPair(json_spirit::Pair &thePair)
{
    if(thePair.name_ == gJsonName_BetaAutomationListItemEnable)
        fEnable = thePair.value_.get_bool();
    else if(thePair.name_ == gJsonName_BetaAutomationListItemPath)
        itsBetaProductAutomationPath = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_BetaAutomationListItemAbsolutePath)
        itsBetaProductAutomationAbsolutePath = thePair.value_.get_str();
}

// ********************************************************************
// *******  NFmiPostponedBetaAutomation osio alkaa  *******************
// ********************************************************************

NFmiPostponedBetaAutomation::NFmiPostponedBetaAutomation(std::shared_ptr<NFmiBetaProductAutomationListItem> &postponedDataTriggeredAutomation, int postponeTimeInMinutes)
:itsPostponeTimer()
,itsPostponedDataTriggeredAutomation(postponedDataTriggeredAutomation)
,itsPostponeTimeInMinutes(postponeTimeInMinutes)
{}

bool NFmiPostponedBetaAutomation::IsPostponeTimeOver()
{
    return itsPostponeTimer.CurrentTimeDiffInMSeconds() >= (itsPostponeTimeInMinutes * 1000 * 60);
}


// ********************************************************************
// *******  NFmiBetaProductAutomationList osio alkaa  *****************
// ********************************************************************

BaseDirectoryGetterFunctionType NFmiBetaProductAutomationList::itsBetaProductionBaseDirectoryGetter;

void NFmiBetaProductAutomationList::SetBetaProductionBaseDirectoryGetter(BaseDirectoryGetterFunctionType &getterFunction)
{
    NFmiBetaProductAutomationList::itsBetaProductionBaseDirectoryGetter = getterFunction;
}

NFmiBetaProductAutomationList::NFmiBetaProductAutomationList()
:itsAutomationVector()
{
}

// Oletus: theListItem.itsBetaProductAutomationPath:iin on jo laitettu käytetty polku.
// 1. Tarkistaa onko annettu polku absoluuttinen vai suhteellinen
// 2. Jos suhteellinen, laske absoluuttinen polku beta-product-base-directoryn mukaan
// 3. Jos absoluuttinen polku, laske suhteellinen polku em. base-directoryn mukaan.
// 4. Kokeillaan löytyykö saatu absoluuttinen polku
bool NFmiBetaProductAutomationList::MakeListItemPathSettingsCheck(NFmiBetaProductAutomationListItem &theListItem, const std::string &theCheckedPath)
{
    std::string givenPath = theCheckedPath;
    if(givenPath.empty())
        return false;
    if(NFmiFileSystem::IsAbsolutePath(givenPath))
    {
        theListItem.itsBetaProductAutomationAbsolutePath = givenPath;
        theListItem.itsBetaProductAutomationPath = PathUtils::getRelativePathIfPossible(givenPath, NFmiBetaProductAutomationList::itsBetaProductionBaseDirectoryGetter());
    }
    else
    {
        theListItem.itsBetaProductAutomationAbsolutePath = PathUtils::getTrueFilePath(givenPath, NFmiBetaProductAutomationList::itsBetaProductionBaseDirectoryGetter(), NFmiBetaProductionSystem::BetaAutomationFileExtension());
        theListItem.itsBetaProductAutomationPath = givenPath;
    }
    return NFmiFileSystem::FileExists(theListItem.itsBetaProductAutomationAbsolutePath);
}

bool NFmiBetaProductAutomationList::MakeListItemPathSettings(NFmiBetaProductAutomationListItem &theListItem)
{
    NFmiBetaProductAutomationListItem tmpItem = theListItem;
    // Kokeillaan ensin löytyykö automaatio-tiedosto käyttäen mahdollista suhteellista polkua vastaan (käyttäjän antama polku, joka voi olla suhteellinen)
    if(!MakeListItemPathSettingsCheck(tmpItem, tmpItem.itsBetaProductAutomationPath))
    { // jos ei löytynyt, kokeillaan vielä löytyykö absoluuttisella polulla mitään
        if(theListItem.itsBetaProductAutomationAbsolutePath.empty())
        {
            theListItem = tmpItem; // Jos jsonista ei saatu ollenkaan absoluuttista polkua, palautetaan suhteellisen polun tarkastelun tilanne ja palautetaan false
            return false;
        }
        else
            return MakeListItemPathSettingsCheck(theListItem, theListItem.itsBetaProductAutomationAbsolutePath);
    }
    theListItem = tmpItem;
    return true;
}

bool NFmiBetaProductAutomationList::Add(const std::string &theBetaAutomationPath)
{
    std::shared_ptr<NFmiBetaProductAutomationListItem> listItem = std::make_shared<NFmiBetaProductAutomationListItem>(theBetaAutomationPath);
    if(PrepareListItemAfterJsonRead(*listItem)) // Voidaan käyttää tätä metodia, vaikka listItemia ei olekaan luettu json-tiedostosta
    {
        listItem->fEnable = true;
        // Lisättäessä listaan laitetaan CalcNextDueTime funktiolla true optio päälle.
        listItem->itsNextRunTime = listItem->itsBetaProductAutomation->TriggerModeInfo().CalcNextDueTime(NFmiMetTime(1), true);
        itsAutomationVector.push_back(listItem);
        return true;
    }

    return false;
}

// Rivi indeksi pitää antaa 0-kantaisena indeksinä, oikeasti rivit grid-controllissa alkavat yhdestä, koska siinä on mukana otsikko rivi.
bool NFmiBetaProductAutomationList::Remove(size_t theZeroBasedRowIndex)
{
    if(theZeroBasedRowIndex < itsAutomationVector.size())
    {
        itsAutomationVector.erase(itsAutomationVector.begin() + theZeroBasedRowIndex);
        return true;
    }
    else
        return false; // indeksi ei osoittanut mihinkään vektorin olioon
}

static NFmiBetaProductAutomationListItem gDummyBetaProductAutomationListItem;

// Rivi indeksi pitää antaa 0-kantaisena indeksinä, oikeasti rivit grid-controllissa alkavat yhdestä, koska siinä on mukana otsikko rivi.
NFmiBetaProductAutomationListItem& NFmiBetaProductAutomationList::Get(size_t theZeroBasedRowIndex)
{
    if(theZeroBasedRowIndex < itsAutomationVector.size())
        return *itsAutomationVector[theZeroBasedRowIndex];
    else
        return gDummyBetaProductAutomationListItem; // indeksi ei osoittanut mihinkään vektorin olioon
}

// Rivi indeksi pitää antaa 0-kantaisena indeksinä, oikeasti rivit grid-controllissa alkavat yhdestä, koska siinä on mukana otsikko rivi.
const NFmiBetaProductAutomationListItem& NFmiBetaProductAutomationList::Get(size_t theZeroBasedRowIndex) const
{
    if(theZeroBasedRowIndex < itsAutomationVector.size())
        return *itsAutomationVector[theZeroBasedRowIndex];
    else
        return gDummyBetaProductAutomationListItem; // indeksi ei osoittanut mihinkään vektorin olioon
}

static const std::string gJsonName_BetaAutomationListHeader = "Beta-automation list";

json_spirit::Object NFmiBetaProductAutomationList::MakeJsonObject(const NFmiBetaProductAutomationList &theBetaProductAutomationList)
{
    json_spirit::Array dataArray;
    const NFmiBetaProductAutomationList::AutomationContainer &dataVector = theBetaProductAutomationList.AutomationVector();
    for(size_t i = 0; i < dataVector.size(); i++)
    {
        json_spirit::Object tmpObject = NFmiBetaProductAutomationListItem::MakeJsonObject(*dataVector[i]);
        if(tmpObject.size())
        {
            json_spirit::Value tmpVal(tmpObject);
            dataArray.push_back(tmpVal);
        }
    }

    json_spirit::Object jsonObject; // luodaan ns. null-objekti
    if(dataArray.size())
    { // täytetään objekti vain jos löytyi yhtään talletettavaa dataa
        jsonObject.push_back(json_spirit::Pair(gJsonName_BetaAutomationListHeader, dataArray));
    }
    return jsonObject;
}

// ListItem on luettu json-tiedostosta NFmiBetaProductAutomationList -luvun yhteydessä, 
// sille pitää tehdä seuraavia asioita ennen kuin se voidaan lisätä itsAutomationVector:iin:
// 1. Säädä polut (abs vs relative)
// 2. Luo dynaamisesti oletus Beta-automation olio
// 3. Lue Beta-automation olio käyttöön
bool NFmiBetaProductAutomationList::PrepareListItemAfterJsonRead(NFmiBetaProductAutomationListItem &theListItem)
{
    MakeListItemPathSettings(theListItem);
    theListItem.itsBetaProductAutomation = std::make_shared<NFmiBetaProductAutomation>();
    std::string errorString;
    return NFmiBetaProductAutomation::ReadInJsonFormat(*theListItem.itsBetaProductAutomation, theListItem.itsBetaProductAutomationAbsolutePath, errorString);
}

void NFmiBetaProductAutomationList::ParseJsonPair(json_spirit::Pair &thePair)
{
    if(thePair.name_ == gJsonName_BetaAutomationListHeader)
    {
        json_spirit::Array dataFileArray = thePair.value_.get_array();
        if(dataFileArray.size())
        {
            for(json_spirit::Array::iterator it = dataFileArray.begin(); it != dataFileArray.end(); ++it)
            {
                std::shared_ptr<NFmiBetaProductAutomationListItem> listItem = std::make_shared<NFmiBetaProductAutomationListItem>();
                ::ParseJsonValue(*listItem, *it);
                if(!PrepareListItemAfterJsonRead(*listItem))
                    listItem->itsStatus = NFmiBetaProductAutomationListItem::kFmiListItemReadError;
                itsAutomationVector.push_back(listItem); // Lisätään virheellisestikin luetut listItemit, jotta käyttäjä saisi palautetta
            }
        }
    }
}

// Tätä kutsutaan kun esim. luetaan data tiedostosta ja tehdään täysi tarkistus kaikille osille
void NFmiBetaProductAutomationList::DoFullChecks(bool fAutomationModeOn)
{
    // Tutkitaan ensin erikseen jokainen automaatio-olio
    for(auto &listItem : itsAutomationVector)
        listItem->DoFullChecks(fAutomationModeOn);

    // Sitten tehdään yhteis tarkastuksia: 1. onko samoja automaatioita useita listassa
    if(itsAutomationVector.size() > 1)
    {
        for(size_t j = 0; j < itsAutomationVector.size() - 1; j++)
        {
            for(size_t i = j+1; i < itsAutomationVector.size(); i++)
            {
                if(itsAutomationVector[j]->FullAutomationPath() == itsAutomationVector[i]->FullAutomationPath() && !itsAutomationVector[j]->FullAutomationPath().empty())
                {
                    itsAutomationVector[j]->itsStatus = NFmiBetaProductAutomationListItem::kFmiListItemSameAutomationsInList;
                }
            }
        }
    }
}

void NFmiBetaProductAutomationList::RefreshAutomationList()
{
    for(auto &betaAutomation : itsAutomationVector)
    {
        RefreshAutomationIfNeeded(betaAutomation);
    }
}

void NFmiBetaProductAutomationList::RefreshAutomationIfNeeded(std::shared_ptr<NFmiBetaProductAutomationListItem> &automationListItem)
{
    // Lue annettu beta-automaatio uudestaan tiedostosta uuteen olioon
    std::shared_ptr<NFmiBetaProductAutomationListItem> listItemFromFile = std::make_shared<NFmiBetaProductAutomationListItem>(automationListItem->itsBetaProductAutomationAbsolutePath);
    if(PrepareListItemAfterJsonRead(*listItemFromFile)) // Voidaan käyttää tätä metodia, vaikka listItemia ei olekaan luettu json-tiedostosta
    {
        // Jos luku meni hyvin, sijoitetaan annettu beta-automaatio päivitettävään otukseen
        automationListItem->itsBetaProductAutomation.swap(listItemFromFile->itsBetaProductAutomation);
        // päivitetään vielä seuraava ajoaika
        automationListItem->itsNextRunTime = automationListItem->itsBetaProductAutomation->TriggerModeInfo().CalcNextDueTime(NFmiMetTime(1), true);
    }
}

bool NFmiBetaProductAutomationList::IsOk() const
{
    for(auto &listItem : itsAutomationVector)
    {
        if(listItem->GetErrorStatus() != NFmiBetaProductAutomationListItem::kFmiListItemOk)
            return false;
    }
    return true;
}

std::set<std::string> NFmiBetaProductAutomationList::GetUniqueFilePathSet() const
{
    std::set<std::string> uniqueFilePaths;
    for(auto &listItem : itsAutomationVector)
        uniqueFilePaths.insert(listItem->FullAutomationPath());
    return uniqueFilePaths;
}

bool NFmiBetaProductAutomationList::ContainsAutomationMoreThanOnce() const
{
    std::set<std::string> uniqueFilePaths = GetUniqueFilePathSet();
    return itsAutomationVector.size() > uniqueFilePaths.size();
}

bool NFmiBetaProductAutomationList::HasAutomationAlready(const std::string &theFullFilePath) const
{
    std::set<std::string> uniqueFilePaths = GetUniqueFilePathSet();
    std::set<std::string>::iterator pos = uniqueFilePaths.find(theFullFilePath);
    return pos != uniqueFilePaths.end();
}

static void LogBetaAutomationTrigger(NFmiBetaProductAutomationListItem & automationListItem, const NFmiMetTime &nextRuntime)
{
    std::string debugTriggerMessage = "Beta automation '";
    debugTriggerMessage += automationListItem.AutomationName();
    debugTriggerMessage += "' was triggered by ";
    const auto& triggerModeInfo = automationListItem.itsBetaProductAutomation->TriggerModeInfo();
    if(triggerModeInfo.itsTriggerMode == NFmiBetaProductAutomation::kFmiFixedTimes)
        debugTriggerMessage += "fixed time";
    else
        debugTriggerMessage += "time step";
    debugTriggerMessage += " at ";
    debugTriggerMessage += nextRuntime.ToStr("HH:mm Utc", kEnglish);
    CatLog::logMessage(debugTriggerMessage, CatLog::Severity::Debug, CatLog::Category::Operational);
}

static void AddPostponedAutomationsToDueList(std::vector<std::shared_ptr<NFmiBetaProductAutomationListItem>>& dueAutomationsInOut, std::list<NFmiPostponedBetaAutomation>& postponedDataTriggeredAutomations)
{
    for(auto it = postponedDataTriggeredAutomations.begin(); it != postponedDataTriggeredAutomations.end(); )
    {
        // Check condition for removal
        if(it->IsPostponeTimeOver())
        {
            std::string debugTriggerMessage = "Postponed beta-automation '";
            debugTriggerMessage += it->itsPostponedDataTriggeredAutomation->AutomationName();
            debugTriggerMessage += "' is now triggered after ";
            debugTriggerMessage += std::to_string(it->itsPostponeTimeInMinutes);
            debugTriggerMessage += " minutes";
            CatLog::logMessage(debugTriggerMessage, CatLog::Severity::Debug, CatLog::Category::Operational);
            // Kopsataan myöhästetty automaatio ajettavien listalle
            dueAutomationsInOut.push_back(it->itsPostponedDataTriggeredAutomation);
            // Erase the current element and get the iterator to the next element
            it = postponedDataTriggeredAutomations.erase(it);
        }
        else
        {
            // Move iterator to the next element
            ++it; 
        }
    }
}

std::vector<std::shared_ptr<NFmiBetaProductAutomationListItem>> NFmiBetaProductAutomationList::GetDueAutomations(const NFmiMetTime &theCurrentTime, const std::vector<std::string>& loadedDataTriggerList, NFmiInfoOrganizer& infoOrganizer, bool automationModeOn)
{
    std::vector<std::shared_ptr<NFmiBetaProductAutomationListItem>> dueAutomations;
    for(auto& listItem : itsAutomationVector)
    {
        if(listItem->fEnable)
        {
            if(listItem->GetErrorStatus() == NFmiBetaProductAutomationListItem::kFmiListItemOk)
            {
                const auto& triggerModeInfo = listItem->itsBetaProductAutomation->TriggerModeInfo();
                if(triggerModeInfo.itsTriggerMode == NFmiBetaProductAutomation::kFmiDataTrigger)
                {
                    int postponeTriggerInMinutes = 0;
                    if(triggerModeInfo.HasDataTriggerBeenLoaded(loadedDataTriggerList, infoOrganizer, listItem->AutomationName(), automationModeOn, postponeTriggerInMinutes))
                    {
                        if(postponeTriggerInMinutes <= 0)
                            dueAutomations.push_back(listItem);
                        else
                            itsPostponedDataTriggeredAutomations.push_back(NFmiPostponedBetaAutomation(listItem, postponeTriggerInMinutes));
                    }
                }
                else
                {
                    NFmiMetTime nextRuntime = triggerModeInfo.CalcNextDueTime(listItem->itsLastRunTime, automationModeOn);
                    if(nextRuntime > listItem->itsLastRunTime && nextRuntime <= theCurrentTime)
                    {
                        ::LogBetaAutomationTrigger(*listItem, nextRuntime);
                        dueAutomations.push_back(listItem);
                    }
                }
            }
        }
    }

    ::AddPostponedAutomationsToDueList(dueAutomations, itsPostponedDataTriggeredAutomations);

    return dueAutomations;
}

// Käyttäjällä on nykyään kolme on-demand -nappia, joista voi käynnistää halutun setin automaatioita työstettäväksi:
// 1. Jos selectedAutomationIndex:issä on positiivinen numero, ajetaan vain sen osoittama automaatio.
// 2. Jos selectedAutomationIndex on -1 ja doOnlyEnabled on true, ajetaan kaikki listalle olevat enbloidut automaatiot.
// 3. Jos selectedAutomationIndex on -1 ja doOnlyEnabled on false, ajetaan kaikki listalle olevat automaatiot.
// selectedAutomationIndex -parametri on 1:stä alkava indeksi ja -1 tarkoitti siis että käydään koko listaa läpi.
std::vector<std::shared_ptr<NFmiBetaProductAutomationListItem>> NFmiBetaProductAutomationList::GetOnDemandAutomations(int selectedAutomationIndex, bool doOnlyEnabled)
{
    std::vector<std::shared_ptr<NFmiBetaProductAutomationListItem>> onDemandAutomations;
    if(selectedAutomationIndex > 0)
    {
        auto actualIndex = selectedAutomationIndex - 1;
        if(actualIndex < itsAutomationVector.size())
        {
            const auto &automationItem = itsAutomationVector[actualIndex];
            // Automaation pitää kuitenkin olla ilman virheitä, että se kelpuutetaan ajettavaksi
            if(automationItem->GetErrorStatus() == NFmiBetaProductAutomationListItem::kFmiListItemOk)
                onDemandAutomations.push_back(automationItem);
        }
    }
    else
    {
        for(const auto& listItem : itsAutomationVector)
        {
            // Automaation pitää kuitenkin olla ilman virheitä, että se kelpuutetaan ajettavaksi
            if(listItem->GetErrorStatus() == NFmiBetaProductAutomationListItem::kFmiListItemOk)
            {
                if(!doOnlyEnabled || listItem->fEnable)
                    onDemandAutomations.push_back(listItem);
            }
        }
    }

    return onDemandAutomations;
}


static const std::string gBetaAutomationListName = "Beta-automation list";

bool NFmiBetaProductAutomationList::StoreInJsonFormat(const NFmiBetaProductAutomationList &theBetaProductAutomationList, const std::string &theFilePath, std::string &theErrorStringOut)
{
    return ::StoreObjectInJsonFormat(theBetaProductAutomationList, theFilePath, gBetaAutomationListName, theErrorStringOut);
}

bool NFmiBetaProductAutomationList::ReadInJsonFormat(NFmiBetaProductAutomationList &theBetaProductAutomationList, const std::string &theFilePath, std::string &theErrorStringOut)
{
    bool status = ::ReadObjectInJsonFormat(theBetaProductAutomationList, theFilePath, gBetaAutomationListName, theErrorStringOut);
    theBetaProductAutomationList.DoFullChecks(true); // Tehdään täällä tarkastelut automaatiomoodi päällä, myöhemmin (tätä funktiota kutsuvassa systeemissä) tarkastelut on tehtävä uudestaan kun oikeasti tiedetään missä moodissa ollaan
    return status;
}


// ********************************************************************
// *********  NFmiBetaProductSystem osio alkaa  ***********************
// ********************************************************************

std::string NFmiBetaProductionSystem::itsRunTimeTitleString = ::GetDictionaryString("Run");
std::string NFmiBetaProductionSystem::itsRunTimeFormatString = ::GetDictionaryString("DDMMYY-HH");

std::function<void(std::vector<std::shared_ptr<NFmiBetaProductAutomationListItem>> &, const NFmiMetTime &)> NFmiBetaProductionSystem::itsGenerateBetaProductsCallback;

const std::string NFmiBetaProductionSystem::itsBetaProductFileExtension = "BetaProd";
const std::string NFmiBetaProductionSystem::itsBetaProductFileFilter = "Beta-product Files (*." + NFmiBetaProductionSystem::itsBetaProductFileExtension + ")|*." + NFmiBetaProductionSystem::itsBetaProductFileExtension + "|All Files (*.*)|*.*||";
const std::string NFmiBetaProductionSystem::itsBetaAutomationFileExtension = "BetaAutom";
const std::string NFmiBetaProductionSystem::itsBetaAutomationFileFilter = "Beta-automation Files (*." + NFmiBetaProductionSystem::itsBetaAutomationFileExtension + ")|*." + NFmiBetaProductionSystem::itsBetaAutomationFileExtension + "|All Files (*.*)|*.*||";
const std::string NFmiBetaProductionSystem::itsBetaAutomationListFileExtension = "BetaList";
const std::string NFmiBetaProductionSystem::itsBetaAutomationListFileFilter = "Beta-automation-list Files (*." + NFmiBetaProductionSystem::itsBetaAutomationListFileExtension + ")|*." + NFmiBetaProductionSystem::itsBetaAutomationListFileExtension + "|All Files (*.*)|*.*||";
const std::string NFmiBetaProductionSystem::itsBetaAutomationAutoFileNameTypeStamp = "obsOrForType";
const std::string NFmiBetaProductionSystem::itsFileNameTemplateValidTimeStamp = "validtime";
const std::string NFmiBetaProductionSystem::itsFileNameTemplateStationIdStamp = "stationid";
const std::string NFmiBetaProductionSystem::itsFileNameTemplateOrigTimeStamp = "origtime";
const std::string NFmiBetaProductionSystem::itsFileNameTemplateMakeTimeStamp = "maketime";

NFmiBetaProductionSystem::NFmiBetaProductionSystem()
: fBetaProductGenerationRunning(false)
, itsUsedAutomationList()
, itsBetaProductionBaseDirectory()
, mBaseRegistryPath()
, mBetaProductTimeStepInMinutes()
, mBetaProductTimeLengthInHours()
, mBetaProductUseUtcTimesInTimeBox()
, mBetaProductParamBoxLocation()
, mBetaProductStoragePath()
, mBetaProductFileNameTemplate()
, mBetaProductUseAutoFileNames()
, mBetaProductRowIndexListString()
, mBetaProductRowSubDirectoryTemplate()
, mBetaProductSelectedViewIndex()
, mBetaProductViewMacroPath()
, mBetaProductWebSiteTitle()
, mBetaProductWebSiteDescription()
, mBetaProductCommandLine()
, mBetaProductDisplayRuntime()
, mBetaProductShowModelOriginTime()
, mBetaProductSaveInitialPath()
, mBetaProductSynopStationIdListString()
, mBetaProductPackImages()
, mBetaProductEnsureCurveVisibility()
, mAutomationModeOn()
, mUsedAutomationListPathString()
, mBetaProductTabControlIndex()
, mBetaProductPath()
, mTriggerModeIndex()
, mFixedTimesString()
, mAutomationTimeStepInHoursString()
, mFirstRunTimeOfDayString()
, mStartTimeModeIndex()
, mEndTimeModeIndex()
, mStartTimeClockOffsetInHoursString()
, mEndTimeClockOffsetInHoursString()
, mAutomationPath()
, mTriggerDataString()
{
}

bool NFmiBetaProductionSystem::Init(const std::string &theBaseRegistryPath, const std::string& theAbsoluteWorkingDirectory, const std::string& possibleStartingBetaAutomationListPath)
{
    // Nämä alustetaan vain ja ainoastaan konffeista ainakin toistaiseksi
    itsBetaProductionBaseDirectory = NFmiSettings::Optional<std::string>("BetaProduction::BaseDirectory", "C:\\smartmet\\BetaProducts\\");
    itsBetaProductionBaseDirectory = PathUtils::makeFixedAbsolutePath(itsBetaProductionBaseDirectory, theAbsoluteWorkingDirectory);
    CatLog::logMessage(std::string("BetaProduction::BaseDirectory = ") + itsBetaProductionBaseDirectory, CatLog::Severity::Info, CatLog::Category::Configuration);

    mBaseRegistryPath = theBaseRegistryPath;
    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;
    // Beta product section
    std::string betaProductSectionName = "\\BetaProduct";
    mBetaProductTimeStepInMinutes = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, betaProductSectionName, "\\TimeStepInMinutes", usedKey, 60);
    mBetaProductTimeLengthInHours = ::CreateRegValue<CachedRegDouble>(mBaseRegistryPath, betaProductSectionName, "\\TimeLengthInHours", usedKey, 15);
    mBetaProductUseUtcTimesInTimeBox = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, betaProductSectionName, "\\UseUtcTimesInTimeBox", usedKey, false);
    mBetaProductParamBoxLocation = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, betaProductSectionName, "\\ParamBoxLocation", usedKey, kNoDirection);
    mBetaProductStoragePath = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\StoragePath", usedKey, "");
    mBetaProductFileNameTemplate = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\FileNameTemplate", usedKey, "product1_validTime.png");
    mBetaProductUseAutoFileNames = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, betaProductSectionName, "\\UseAutoFileNames", usedKey, false);
    mBetaProductRowIndexListString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\RowIndexListString", usedKey, "");
    mBetaProductRowSubDirectoryTemplate = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\RowSubDirectoryTemplate", usedKey, "");
    mBetaProductSelectedViewIndex = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, betaProductSectionName, "\\SelectedViewIndex", usedKey, 0);
    mBetaProductViewMacroPath = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\ViewMacroPath", usedKey, "");
    mBetaProductWebSiteTitle = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\WebSiteTitle", usedKey, "");
    mBetaProductWebSiteDescription = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\WebSiteDescription", usedKey, "");
    mBetaProductCommandLine = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\CommandLine", usedKey, "");
    mBetaProductDisplayRuntime = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, betaProductSectionName, "\\DisplayRuntime", usedKey, false);
    mBetaProductShowModelOriginTime = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, betaProductSectionName, "\\ShowModelOriginTime", usedKey, false);
    mBetaProductSynopStationIdListString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\SynopStationIdList", usedKey, "");
    mBetaProductPackImages = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, betaProductSectionName, "\\PackImages", usedKey, false);
    mBetaProductEnsureCurveVisibility = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, betaProductSectionName, "\\EnsureCurveVisibility", usedKey, false);

    mAutomationModeOn = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, betaProductSectionName, "\\AutomationModeOn", usedKey, false, "");
    mUsedAutomationListPathString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\UsedAutomationListPath", usedKey, "");
    SetupFinalBataAutomationListPath(possibleStartingBetaAutomationListPath);
    mBetaProductPath = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\BetaProductPath", usedKey, "");
    mBetaProductTabControlIndex = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, betaProductSectionName, "\\BetaProductTabControlIndex", usedKey, 0);

    // otetaan Beta-product base-directory oletus arvoksi näihin kolmeen polkuun
    mBetaProductSaveInitialPath = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\BetaProductSaveInitialPath", usedKey, itsBetaProductionBaseDirectory); 
    mBetaAutomationSaveInitialPath = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\BetaAutomationSaveInitialPath", usedKey, itsBetaProductionBaseDirectory);
    mBetaAutomationListSaveInitialPath = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\BetaAutomationListSaveInitialPath", usedKey, itsBetaProductionBaseDirectory);

    mTriggerModeIndex = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, betaProductSectionName, "\\TriggerModeIndex", usedKey, 0);
    mFixedTimesString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\FixedTimes", usedKey, "");
    mAutomationTimeStepInHoursString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\AutomationTimeStepInHours", usedKey, "");
    mFirstRunTimeOfDayString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\FirstRunTimeOfDayString", usedKey, "");
    mStartTimeModeIndex = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, betaProductSectionName, "\\StartTimeModeIndex", usedKey, 0);
    mEndTimeModeIndex = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, betaProductSectionName, "\\EndTimeModeIndex", usedKey, 0);
    mStartTimeClockOffsetInHoursString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\StartTimeClockOffsetInHours", usedKey, "");
    mEndTimeClockOffsetInHoursString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\EndTimeClockOffsetInHours", usedKey, "");
    mAutomationPath = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\AutomationPath", usedKey, "");
    mTriggerDataString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\TriggerData", usedKey, "");

    std::function<std::string()> getterFunction = [this]() {return this->GetBetaProductionBaseDirectory(); };
    NFmiBetaProductAutomation::SetBetaProductionBaseDirectoryGetter(getterFunction);
    NFmiBetaProductAutomationList::SetBetaProductionBaseDirectoryGetter(getterFunction);
    InitImagePackingExe(theAbsoluteWorkingDirectory);

    LoadUsedAutomationList(UsedAutomationListPathString());

    return true;
}

static void LogUsedBetaAutomationListPath(const std::string& usedCaseText, const std::string& usedPath)
{
    CatLog::Severity severity = NFmiFileSystem::FileExists(usedPath) ? CatLog::Severity::Info : CatLog::Severity::Error;
    std::string finalLogMessage = usedCaseText;
    finalLogMessage += ", final path was " + usedPath;
    if(severity == CatLog::Severity::Error)
    {
        finalLogMessage += ", error: file doesn't exist";
    }
    CatLog::logMessage(finalLogMessage, severity, CatLog::Category::Configuration, true);
}

void NFmiBetaProductionSystem::SetupFinalBataAutomationListPath(const std::string& possibleStartingBetaAutomationListPath)
{
    if(!possibleStartingBetaAutomationListPath.empty())
    {
        *mUsedAutomationListPathString = PathUtils::getTrueFilePath(possibleStartingBetaAutomationListPath, itsBetaProductionBaseDirectory, itsBetaAutomationListFileExtension);
        std::string caseText = "BetaAutomationList path was given from command line arguments with \"-b ";
        caseText += possibleStartingBetaAutomationListPath;
        caseText += "\" -option";
        ::LogUsedBetaAutomationListPath(caseText, *mUsedAutomationListPathString);
    }
    else
    {
        ::LogUsedBetaAutomationListPath("BetaAutomationList path was retrieved from Windows registry", *mUsedAutomationListPathString);
    }
}

std::string NFmiBetaProductionSystem::AddQuotationMarksToString(std::string paddedString)
{
    paddedString = "\"" + paddedString + "\"";
    return paddedString;
}

bool NFmiBetaProductionSystem::InitImagePackingExe(const std::string& theAbsoluteWorkingDirectory)
{
    std::string exeNameInErrorMessages = "pngquant";
    itsImagePackingExePath = ExePathHelper::MakeOptionalExePath(theAbsoluteWorkingDirectory, "SmartMet::OptionalImagePackingExePath", exeNameInErrorMessages);
    if(itsImagePackingExePath.empty())
    {
        itsImagePackingExePath = ExePathHelper::MakeHardCodedExePath(theAbsoluteWorkingDirectory, "\\utils\\pngquant\\pngquant.exe", exeNameInErrorMessages);
        // Jos ei ole tullut tähän mennessä järkevää arvoa exelle, ollaan jo tehty virheilmoituksia lokiin
    }

    if(itsImagePackingExePath.empty())
        return false;
    else
    {
        std::string usedPathString = "For ";
        usedPathString += exeNameInErrorMessages;
        usedPathString += " executable following path is used: ";
        usedPathString += itsImagePackingExePath;
        CatLog::logMessage(usedPathString, CatLog::Severity::Info, CatLog::Category::Configuration);
    }

    itsImagePackingExeCommandLine = NFmiSettings::Optional<std::string>("SmartMet::OptionalImagePackingExeCommandLine", "");
    if(itsImagePackingExeCommandLine.empty())
    {
        itsImagePackingExeCommandLine = "--force --verbose --nofs --speed=1 --ext=.png 256 *.png";
    }

    return true;
}

// SmartMetin CMainFrm::OnTimer kutsuu tätä funktiota kerran minuutissa ja päättelee onko tehtävä mitään 
// itsUsedAutomationList:alla olevaa tuotantoa.
bool NFmiBetaProductionSystem::DoNeededBetaAutomation(const std::vector<std::string>& loadedDataTriggerList, NFmiInfoOrganizer& infoOrganizer)
{
    NFmiMetTime currentTime(1); // Otetaan talteen nyky UTC hetki minuutin tarkkuudella
    bool automationModeOn = AutomationModeOn();
    if(automationModeOn)
    {
        std::vector<std::shared_ptr<NFmiBetaProductAutomationListItem>> dueAutomations = itsUsedAutomationList.GetDueAutomations(currentTime, loadedDataTriggerList, infoOrganizer, automationModeOn);
        if(!dueAutomations.empty())
        {
            // Joku callback funktio kutsu CFmiBetaProductDialog-luokalle
            if(itsGenerateBetaProductsCallback)
            {
                itsGenerateBetaProductsCallback(dueAutomations, currentTime);
                return true;
            }
        }
    }
    return false;
}

// Käyttäjällä on nykyään kolme on-demand -nappia, joista voi käynnistää halutun setin automaatioita työstettäväksi:
// 1. Jos selectedAutomationIndex:issä on positiivinen numero, ajetaan vain sen osoittama automaatio.
// 2. Jos selectedAutomationIndex on -1 ja doOnlyEnabled on true, ajetaan kaikki listalle olevat enbloidut automaatiot.
// 3. Jos selectedAutomationIndex on -1 ja doOnlyEnabled on false, ajetaan kaikki listalle olevat automaatiot.
// selectedAutomationIndex -parametri on 1:stä alkava indeksi ja -1 tarkoitti siis että käydään koko listaa läpi.
bool NFmiBetaProductionSystem::DoOnDemandBetaAutomations(int selectedAutomationIndex, bool doOnlyEnabled)
{
    NFmiMetTime currentTime(1); // Otetaan talteen nyky UTC hetki minuutin tarkkuudella
    std::vector<std::shared_ptr<NFmiBetaProductAutomationListItem>> onDemandAutomations = itsUsedAutomationList.GetOnDemandAutomations(selectedAutomationIndex, doOnlyEnabled);
    if(!onDemandAutomations.empty())
    {
        // Joku callback funktio kutsu CFmiBetaProductDialog-luokalle
        if(itsGenerateBetaProductsCallback)
        {
            itsGenerateBetaProductsCallback(onDemandAutomations, currentTime);
            return true;
        }
    }
    return false;
}


// Lataa käyttöön annetusta polusta automaatio-listan.
// Asettaa polun myös mUsedAutomationListPathString:in arvoksi.
bool NFmiBetaProductionSystem::LoadUsedAutomationList(const std::string &thePath)
{
    UsedAutomationListPathString(thePath);
    std::string fullPath = PathUtils::getTrueFilePath(thePath, GetBetaProductionBaseDirectory(), NFmiBetaProductionSystem::BetaAutomationListFileExtension());
    std::string errorString;
    itsUsedAutomationList = NFmiBetaProductAutomationList(); // Nollataan käytössä ollut lista ennen uuden lukua
    bool status = NFmiBetaProductAutomationList::ReadInJsonFormat(itsUsedAutomationList, fullPath, errorString);
    UsedAutomationList().DoFullChecks(AutomationModeOn());
    return status;
}

int NFmiBetaProductionSystem::BetaProductTimeStepInMinutes()
{
    return *mBetaProductTimeStepInMinutes;
}

void NFmiBetaProductionSystem::BetaProductTimeStepInMinutes(int newValue)
{
    *mBetaProductTimeStepInMinutes = newValue;
}

double NFmiBetaProductionSystem::BetaProductTimeLengthInHours()
{
    return *mBetaProductTimeLengthInHours;
}

void NFmiBetaProductionSystem::BetaProductTimeLengthInHours(double newValue)
{
    *mBetaProductTimeLengthInHours = newValue;
}

bool NFmiBetaProductionSystem::BetaProductUseUtcTimesInTimeBox()
{
    return *mBetaProductUseUtcTimesInTimeBox;
}

void NFmiBetaProductionSystem::BetaProductUseUtcTimesInTimeBox(bool newValue)
{
    *mBetaProductUseUtcTimesInTimeBox = newValue;
}

FmiDirection NFmiBetaProductionSystem::BetaProductParamBoxLocation()
{
    int tmpValue = *mBetaProductParamBoxLocation;
    return static_cast<FmiDirection>(tmpValue);
}

void NFmiBetaProductionSystem::BetaProductParamBoxLocation(FmiDirection newValue)
{
    *mBetaProductParamBoxLocation = newValue;
}

std::string NFmiBetaProductionSystem::BetaProductStoragePath()
{
    return *mBetaProductStoragePath;
}

void NFmiBetaProductionSystem::BetaProductStoragePath(const std::string &newValue)
{
    *mBetaProductStoragePath = newValue;
}

std::string NFmiBetaProductionSystem::BetaProductFileNameTemplate()
{
    return *mBetaProductFileNameTemplate;
}

void NFmiBetaProductionSystem::BetaProductFileNameTemplate(const std::string &newValue)
{
    *mBetaProductFileNameTemplate = newValue;
}

bool NFmiBetaProductionSystem::BetaProductUseAutoFileNames()
{
    return *mBetaProductUseAutoFileNames;
}

void NFmiBetaProductionSystem::BetaProductUseAutoFileNames(bool newValue)
{
    *mBetaProductUseAutoFileNames = newValue;
}

std::string NFmiBetaProductionSystem::BetaProductRowIndexListString()
{
    return *mBetaProductRowIndexListString;
}

void NFmiBetaProductionSystem::BetaProductRowIndexListString(const std::string &newValue)
{
    *mBetaProductRowIndexListString = newValue;
}

std::string NFmiBetaProductionSystem::BetaProductRowSubDirectoryTemplate()
{
    return *mBetaProductRowSubDirectoryTemplate;
}

void NFmiBetaProductionSystem::BetaProductRowSubDirectoryTemplate(const std::string &newValue)
{
    *mBetaProductRowSubDirectoryTemplate = newValue;
}

int NFmiBetaProductionSystem::BetaProductSelectedViewIndex()
{
    return *mBetaProductSelectedViewIndex;
}

void NFmiBetaProductionSystem::BetaProductSelectedViewIndex(int newValue)
{
    *mBetaProductSelectedViewIndex = newValue;
}

std::string NFmiBetaProductionSystem::BetaProductViewMacroPath()
{
    return *mBetaProductViewMacroPath;
}

void NFmiBetaProductionSystem::BetaProductViewMacroPath(const std::string &newValue)
{
    *mBetaProductViewMacroPath = newValue;
}

std::string NFmiBetaProductionSystem::BetaProductWebSiteTitle()
{
    return *mBetaProductWebSiteTitle;
}

void NFmiBetaProductionSystem::BetaProductWebSiteTitle(const std::string &newValue)
{
    *mBetaProductWebSiteTitle = newValue;
}

std::string NFmiBetaProductionSystem::BetaProductWebSiteDescription()
{
    return *mBetaProductWebSiteDescription;
}

void NFmiBetaProductionSystem::BetaProductWebSiteDescription(const std::string &newValue)
{
    *mBetaProductWebSiteDescription = newValue;
}

std::string NFmiBetaProductionSystem::BetaProductCommandLine()
{
    return *mBetaProductCommandLine;
}

void NFmiBetaProductionSystem::BetaProductCommandLine(const std::string &newValue)
{
    *mBetaProductCommandLine = newValue;
}

bool NFmiBetaProductionSystem::BetaProductDisplayRuntime()
{
    return *mBetaProductDisplayRuntime;
}

void NFmiBetaProductionSystem::BetaProductDisplayRuntime(bool newValue)
{
    *mBetaProductDisplayRuntime = newValue;
}

bool NFmiBetaProductionSystem::BetaProductShowModelOriginTime()
{
    return *mBetaProductShowModelOriginTime;
}

void NFmiBetaProductionSystem::BetaProductShowModelOriginTime(bool newValue)
{
    *mBetaProductShowModelOriginTime = newValue;
}

std::string NFmiBetaProductionSystem::BetaProductSynopStationIdListString()
{
    return *mBetaProductSynopStationIdListString;
}

void NFmiBetaProductionSystem::BetaProductSynopStationIdListString(const std::string &newValue)
{
    *mBetaProductSynopStationIdListString = newValue;
}

bool NFmiBetaProductionSystem::BetaProductPackImages()
{
    return *mBetaProductPackImages;
}

void NFmiBetaProductionSystem::BetaProductPackImages(bool newValue)
{
    *mBetaProductPackImages = newValue;
}

bool NFmiBetaProductionSystem::BetaProductEnsureCurveVisibility()
{
    return *mBetaProductEnsureCurveVisibility;
}

void NFmiBetaProductionSystem::BetaProductEnsureCurveVisibility(bool newValue)
{
    *mBetaProductEnsureCurveVisibility = newValue;
}

std::string NFmiBetaProductionSystem::BetaProductSaveInitialPath()
{
    return *mBetaProductSaveInitialPath;
}

void NFmiBetaProductionSystem::BetaProductSaveInitialPath(const std::string &newValue)
{
    *mBetaProductSaveInitialPath = newValue;
}

std::string NFmiBetaProductionSystem::BetaAutomationSaveInitialPath()
{
    return *mBetaAutomationSaveInitialPath;
}

void NFmiBetaProductionSystem::BetaAutomationSaveInitialPath(const std::string& newValue)
{
    *mBetaAutomationSaveInitialPath = newValue;
}

std::string NFmiBetaProductionSystem::BetaAutomationListSaveInitialPath()
{
    return *mBetaAutomationListSaveInitialPath;
}

void NFmiBetaProductionSystem::BetaAutomationListSaveInitialPath(const std::string& newValue)
{
    *mBetaAutomationListSaveInitialPath = newValue;
}

bool NFmiBetaProductionSystem::AutomationModeOn()
{
    return *mAutomationModeOn;
}

void NFmiBetaProductionSystem::AutomationModeOn(bool newValue)
{
    *mAutomationModeOn = newValue;
}

std::string NFmiBetaProductionSystem::GetBetaProductionBaseDirectory() const
{
    return itsBetaProductionBaseDirectory;
}

std::string NFmiBetaProductionSystem::UsedAutomationListPathString()
{
    return *mUsedAutomationListPathString;
}

void NFmiBetaProductionSystem::UsedAutomationListPathString(const std::string &newValue)
{
    *mUsedAutomationListPathString = newValue;
}

std::string NFmiBetaProductionSystem::BetaProductPath()
{
    return *mBetaProductPath;
}

void NFmiBetaProductionSystem::BetaProductPath(const std::string &newValue)
{
    *mBetaProductPath = newValue;
}

int NFmiBetaProductionSystem::BetaProductTabControlIndex()
{
    return *mBetaProductTabControlIndex;
}

void NFmiBetaProductionSystem::BetaProductTabControlIndex(int newValue)
{
    *mBetaProductTabControlIndex = newValue;
}

int NFmiBetaProductionSystem::TriggerModeIndex()
{
    return *mTriggerModeIndex;
}

void NFmiBetaProductionSystem::TriggerModeIndex(int newValue)
{
    *mTriggerModeIndex = newValue;
}

std::string NFmiBetaProductionSystem::FixedTimesString()
{
    return *mFixedTimesString;
}

void NFmiBetaProductionSystem::FixedTimesString(const std::string &newValue)
{
    *mFixedTimesString = newValue;
}

std::string NFmiBetaProductionSystem::AutomationTimeStepInHoursString()
{
    return *mAutomationTimeStepInHoursString;
}

void NFmiBetaProductionSystem::AutomationTimeStepInHoursString(const std::string &newValue)
{
    *mAutomationTimeStepInHoursString = newValue;
}

std::string NFmiBetaProductionSystem::FirstRunTimeOfDayString()
{
    return *mFirstRunTimeOfDayString;
}

void NFmiBetaProductionSystem::FirstRunTimeOfDayString(const std::string &newValue)
{
    *mFirstRunTimeOfDayString = newValue;
}

int NFmiBetaProductionSystem::StartTimeModeIndex()
{
    return *mStartTimeModeIndex;
}

void NFmiBetaProductionSystem::StartTimeModeIndex(int newValue)
{
    *mStartTimeModeIndex = newValue;
}

int NFmiBetaProductionSystem::EndTimeModeIndex()
{
    return *mEndTimeModeIndex;
}

void NFmiBetaProductionSystem::EndTimeModeIndex(int newValue)
{
    *mEndTimeModeIndex = newValue;
}

std::string NFmiBetaProductionSystem::StartTimeClockOffsetInHoursString()
{
    return *mStartTimeClockOffsetInHoursString;
}

void NFmiBetaProductionSystem::StartTimeClockOffsetInHoursString(const std::string &newValue)
{
    *mStartTimeClockOffsetInHoursString = newValue;
}

std::string NFmiBetaProductionSystem::EndTimeClockOffsetInHoursString()
{
    return *mEndTimeClockOffsetInHoursString;
}

void NFmiBetaProductionSystem::EndTimeClockOffsetInHoursString(const std::string &newValue)
{
    *mEndTimeClockOffsetInHoursString = newValue;
}

std::string NFmiBetaProductionSystem::AutomationPath()
{
    return *mAutomationPath;
}

void NFmiBetaProductionSystem::AutomationPath(const std::string& newValue)
{
    *mAutomationPath = newValue;
}

std::string NFmiBetaProductionSystem::TriggerDataString()
{
    return *mTriggerDataString;
}

void NFmiBetaProductionSystem::TriggerDataString(const std::string& newValue)
{
    *mTriggerDataString = newValue;
}

