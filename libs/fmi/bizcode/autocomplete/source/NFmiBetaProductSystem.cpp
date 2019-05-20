#ifdef _MSC_VER
#pragma warning( disable : 4800 ) // t�m� est�� pitk�n varoituksen joka tulee kun k�ytet��n CachedRegBool -> registry_int<bool> -tyyppist� dataa, siell� operator T -metodissa DWORD muuttuu bool:iksi (Huom! static_cast ei auta)
#endif

#include "NFmiBetaProductSystem.h"
#include "NFmiBetaProductHelperFunctions.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiValueString.h"
#include "NFmiFileString.h"
#include "NFmiFileSystem.h"
#include "NFmiPathUtils.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "json_spirit_writer.h"
#include "json_spirit_reader.h"

#include <fstream>
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
            objectOut = T(); // Objekti pit�� nollata oletus arvoilla ennen parsimista
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
,itsTimeBoxLocation(kBottomLeft)
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
,itsSelectedViewIndex(0)
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

// K�y l�pi kaikki tarkastelut ja tekee virheilmoituksia
bool NFmiBetaProduct::CheckTimeRelatedInputs(const NFmiMetTime &theStartingTime, const std::string &theTimeLengthInHoursString, const std::string &theTimeStepInMinutesString, bool theUseUtcTimesInTimeBox, FmiDirection theTimeBoxLocation)
{
    // Nollataan status dataosiot
    fTimeInputOk = false;
    itsTimeInputErrorString.clear();

    // Alustetaan ns. transientit dataosiot
    itsStartingTime = theStartingTime;

    // Alustetaan pysyv�t dataosiot
    itsTimeLengthInHoursString = theTimeLengthInHoursString;
    itsTimeStepInMinutesString = theTimeStepInMinutesString;
    fUseUtcTimesInTimeBox = theUseUtcTimesInTimeBox;
    itsTimeBoxLocation = theTimeBoxLocation;
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
            // auto on tyyppi� validTime_obs/for_stationid.png
            return NFmiBetaProductionSystem::FileNameTemplateValidTimeStamp() + "_" + NFmiBetaProductionSystem::BetaAutomationAutoFileNameTypeStamp() + "_" + NFmiBetaProductionSystem::FileNameTemplateStationIdStamp() + ".png";
        }
        else
        {
            // auto on tyyppi� validTime_obs/for.png
            return NFmiBetaProductionSystem::FileNameTemplateValidTimeStamp() + "_" + NFmiBetaProductionSystem::BetaAutomationAutoFileNameTypeStamp() + ".png";
        }
    }
    else
        return itsFileNameTemplate;
}

const double gMissingTimeRelatedValue = -99999.; // Kaikki aika kontrolleihin liittyv�t arvot ovat positiivisia

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
    if(tmpTimeLengthInHours < 0) // Formaalisesti virheellisen inputin paluuarvo on my�s negatiivinen, joten sit� ei tarvitse tarkastella erikseen
    {
        itsTimeInputErrorString = ::MakeInvalidInputString(itsTimeLengthLabel, "Give positive real value like: 15", itsTimeLengthInHoursString);
        return NFmiMetTime::gMissingTime;
    }
    else
        itsTimeLengthInHours = tmpTimeLengthInHours;

    int tmpTimeStepInMinutes = GetTimeControlRelatedValueFromString<int>(itsTimeStepInMinutesString);
    if(tmpTimeStepInMinutes < 1) // Formaalisti virheellisen paluu arvo on negatiivinen, eik� saa my�sk��n olla 0, koska t�ll�in tulisi 0:lla jako
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
    else if(!::CheckUsedBoxLocation(itsTimeBoxLocation))
    {
        infoStr = ::GetDictionaryString("Timebox location was illegal.");
        infoStr += "\n";
        infoStr += ::GetDictionaryString("Make correct selection from dropdown list.");
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

// Oletus: theRowIndexListStr on jo tarkastettu, ett� se ei ole tyhj� ja siit� on poistettu kaikki whitespacet ja
// on tarkastettu, ett� indexRangeString:ista l�ytyy '-' merkki (ja indeksi ei saanut olla negatiivinen, joten esim. "-12" ei kelpaa)
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

// Muuttujassa itsRowIndexListStringU_ pit�� olla jotain seuraavaa:
// 1. tyhj�
// 2. int1,int2,int3-int4,... eli pilkuilla erotettuja positiivisia lukuja (0 ei kelpaa) listassa joissa voi olla yksitt�isi� lukuja tai lukuv�lej�
// 2.1. Jos kyse lukuv�list�, on mukana kaksi lukua, miss� niiden erottimena on '-' -merkki
// Esim. "1,3,6-8,12,..."
// Heitt�� poikkeuksia, jos virheellinen input.
static std::vector<int> MakeIndexList(std::string indexListString)
{
    indexListString = ::RemoveAllWhiteSpaces(indexListString); // Poistetaan varmuuden vuoksi kaikki white-spacet input stringist�
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

// Oletus: t�nne tullaan vain jos itsRowIndexList EI ole tyhj�.
// 1. Jos itsRowSubdirectoryTemplateStringU_ -muuttujassa on jotain, siit� pit�� l�yty� '#' -markki, muuten se ei kelpaa
// 2. Jos t�m� on tyhj�, mutta itsRowIndexList -muuttujassa on arvoja, pit�� itsFileNameTemplateU_ -muuttujassa olla "rowIndex" -leima
std::string NFmiBetaProduct::CheckUsedRowSubdirectoryTemplate()
{
    std::string subdirTemplateStr = itsRowSubdirectoryTemplate;
    subdirTemplateStr = ::RemoveAllWhiteSpaces(subdirTemplateStr); // Poistetaan varmuuden vuoksi kaikki white-spacet input stringist�
    if(subdirTemplateStr.empty())
    {
            itsRowInputErrorString = ::GetDictionaryString("Multiple rows were selected but no row subdirectory template.");
            itsRowInputErrorString += "\n";
            itsRowInputErrorString += ::GetDictionaryString("Unable to generate images.");
            return "";
    }
    else
    {
        std::string::size_type pos = subdirTemplateStr.find('#');
        if(pos != std::string::npos)
            return subdirTemplateStr; // templaatista l�ytyi # -merkki, templaatti kunnossa
        else
        {
            itsRowInputErrorString = ::GetDictionaryString("Multiple rows were selected but invalid input in row subdirectory template");
            itsRowInputErrorString += ": \"";
            itsRowInputErrorString += subdirTemplateStr;
            itsRowInputErrorString += "\"\n";
            itsRowInputErrorString += ::GetDictionaryString("There must be somthing with # -character in template. Unable to generate images.");
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
    if(itsRowInputErrorString.empty() && !itsRowIndexies.empty()) // Jos CalcRowIndexList:issa ei virheit� ja palautettu itsRowIndexList ei ollut tyhj�, tutkitaan row-subdirectory-template:ia
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

// K�y l�pi kaikki tarkastelut ja tekee virheilmoituksia
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

// K�y l�pi kaikki tarkastelut ja tekee status asetuksia.
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
        // Jos k�ytt�j� ei ole antanut listaa, tehd��n sellainen k�yt�ss� olevasta theOrigRowIndex:ist�
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

        // Tutkitaan l�ytyyk� annettua viewMacroa ollenkaan
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
static const std::string gJsonName_TimeboxLocation = "TimeboxLocation";
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

static void AddNonEmptyStringJsonPair(const std::string &value, const std::string &valueJsonName, json_spirit::Object &jsonObject)
{
    if(!value.empty())
        jsonObject.push_back(json_spirit::Pair(valueJsonName, value));
}

json_spirit::Object NFmiBetaProduct::MakeJsonObject(const NFmiBetaProduct &betaProduct)
{
    static NFmiBetaProduct defaultBetaProduct;

    json_spirit::Object jsonObject;
    ::AddNonEmptyStringJsonPair(betaProduct.ImageStoragePath(), gJsonName_ImageStoragePath, jsonObject);
    ::AddNonEmptyStringJsonPair(betaProduct.FileNameTemplate(), gJsonName_FileNameTemplate, jsonObject);
    if(defaultBetaProduct.UseAutoFileNames() != betaProduct.UseAutoFileNames())
        jsonObject.push_back(json_spirit::Pair(gJsonName_UseAutoFileNames, betaProduct.UseAutoFileNames()));
    ::AddNonEmptyStringJsonPair(betaProduct.TimeLengthInHoursString(), gJsonName_TimeLengthInHours, jsonObject);
    ::AddNonEmptyStringJsonPair(betaProduct.TimeStepInMinutesString(), gJsonName_TimeStepInMinutes, jsonObject);
    if(defaultBetaProduct.UseUtcTimesInTimeBox() != betaProduct.UseUtcTimesInTimeBox())
        jsonObject.push_back(json_spirit::Pair(gJsonName_UseUtcInTimebox, betaProduct.UseUtcTimesInTimeBox()));
    if(defaultBetaProduct.TimeBoxLocation() != betaProduct.TimeBoxLocation())
        jsonObject.push_back(json_spirit::Pair(gJsonName_TimeboxLocation, betaProduct.TimeBoxLocation()));
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
    jsonObject.push_back(json_spirit::Pair(gJsonName_SelectedViewIndex, betaProduct.SelectedViewIndex()));
    ::AddNonEmptyStringJsonPair(betaProduct.SynopStationIdListString(), gJsonName_SynopStationIdList, jsonObject);

    return jsonObject;
}

void NFmiBetaProduct::InitFromJsonRead(const NFmiMetTime &theStartingTime)
{
    bool timeStatus = CheckTimeRelatedInputs(theStartingTime, itsTimeLengthInHoursString, itsTimeStepInMinutesString, fUseUtcTimesInTimeBox, itsTimeBoxLocation);
    bool rowStatus = CheckRowRelatedInputs(itsRowIndexListString, itsRowSubdirectoryTemplate, itsFileNameTemplate, fUseAutoFileNames, itsParamBoxLocation);
    MakeViewMacroInfoText(itsViewMacroPath);
}

static void FixPathDirectorySeparatorSlashes(std::string &thePath)
{
    NFmiStringTools::ReplaceChars(thePath, '/', '\\'); // muutetaan polku niin ett� kenoviivat on oikein p�in
}

void NFmiBetaProduct::ParseJsonPair(json_spirit::Pair &thePair)
{
    // T�ss� puret��n NFmiBetaProduct luokan p��tason pareja.
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
    else if(thePair.name_ == gJsonName_TimeboxLocation)
        itsTimeBoxLocation = static_cast<FmiDirection>(thePair.value_.get_int());
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
        itsSelectedViewIndex = thePair.value_.get_int();
    else if(thePair.name_ == gJsonName_SynopStationIdList)
        itsSynopStationIdListString = thePair.value_.get_str();
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

// T�m� on suoraa NFmiTimeModeInfo -luokan k�ytt��, jolloin ei tehd� sen kummempia tarkasteluja parametreille.
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

    // Tietyiss� dataosissa vain default arvosta poikkeavat arvot talletetaan json objektiin.
    if(defaultTimeModeObject.itsWallClockOffsetInHoursString != timeModeInfo.itsWallClockOffsetInHoursString)
        jsonObject.push_back(json_spirit::Pair(gJsonName_WallClockOffsetInHours, timeModeInfo.itsWallClockOffsetInHoursString));

    return jsonObject;
}

void NFmiBetaProductAutomation::NFmiTimeModeInfo::ParseJsonPair(json_spirit::Pair &thePair)
{
    // T�ss� puret��n NFmiTimeModeInfo luokan p��tason pareja.
    if(thePair.name_ == gJsonName_TimeMode)
        itsTimeMode = static_cast<NFmiBetaProductAutomation::TimeMode>(thePair.value_.get_int());
    else if(thePair.name_ == gJsonName_WallClockOffsetInHours)
        itsWallClockOffsetInHoursString = thePair.value_.get_str();
}

// Testataanvain todellisia k�ytt�j�n antamia arvoja, ei testata mit��n statuksia, 
// tai status stringeja, koska verrokkia ei v�ltt�m�tt� p�ivitet�.
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
, itsTriggerData()
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
    itsTriggerModeInfoStatusString = ::GetDictionaryString("Fixed time list is not implemented yet");
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

void NFmiBetaProductAutomation::NFmiTriggerModeInfo::CheckTriggerData()
{
    fTriggerModeInfoStatus = false;
    itsTriggerModeInfoStatusString = ::GetDictionaryString("Data event is not implemented yet");
}

// T�m� toimii vain itsTriggerMode == kFmiTimeStep -moodissa!!!!
NFmiMetTime NFmiBetaProductAutomation::NFmiTriggerModeInfo::MakeFirstRunTimeOfGivenDay(const NFmiMetTime &theTime) const
{
    NFmiMetTime aTime(theTime.GetYear(), theTime.GetMonth(), theTime.GetDay(), 0, 0, 0, 1);
    aTime.ChangeByMinutes(itsFirstRunTimeOffsetInMinutes);
    return aTime;
}

// Lasketaan milloin automaatio pit�� suorittaa seuraavan kerrran, kun tiedet��n milloin se 
// on viimeksi ajettu ja mik� on nykyhetki
NFmiMetTime NFmiBetaProductAutomation::NFmiTriggerModeInfo::CalcNextDueTime(const NFmiMetTime &theLastRunTime) const
{
    if(itsTriggerMode == kFmiTimeStep)
    {
        NFmiMetTime aTime = MakeFirstRunTimeOfGivenDay(theLastRunTime);
        for(;; aTime.ChangeByMinutes(boost::math::iround(itsRunTimeStepInHours * 60)))
        {
            if(aTime > theLastRunTime)
            {
                if(aTime.GetDay() != theLastRunTime.GetDay())
                    aTime = MakeFirstRunTimeOfGivenDay(aTime); // jos ollaan menty jo toiseen p�iv��n, pit�� sille laskea uusi alkuaika!
                return aTime;
            }

            // Loput tarkastelut ovat ns sanity checkej�, jos on v��ri� l�ht�arvoja datassta, ja niit� ei ole tarkasteltu kunnolla, jotta koodi ei j�� ikilooppiin.
            if(aTime.GetDay() != theLastRunTime.GetDay())
                break;
            if(itsRunTimeStepInHours <= 0)
                break;
        }
    }

    return NFmiMetTime::gMissingTime; // virhetilanne, palautetaan puutt1uva aika
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

    // Tietyiss� dataosissa vain default arvosta poikkeavat arvot talletetaan json objektiin.
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
    // T�ss� puret��n NFmiTriggerModeInfo luokan p��tason pareja.
    if(thePair.name_ == gJsonName_TriggerMode)
        itsTriggerMode = static_cast<NFmiBetaProductAutomation::TriggerMode>(thePair.value_.get_int());
    else if(thePair.name_ == gJsonName_FixedRunTimes)
    {
        itsFixedRunTimesString = thePair.value_.get_str();
        // HUOM! Laske ajoajat t�ss�
    }
    else if(thePair.name_ == gJsonName_RunTimeStepInHours)
        itsRunTimeStepInHoursString = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_FirstRunTimeOfDay)
        itsFirstRunTimeOfDayString = thePair.value_.get_str();
    else if(thePair.name_ == gJsonName_TriggerData)
    {
        itsTriggerDataString = thePair.value_.get_str();
        // HUOM! Laske trigger data lista t�ss�
    }
}

// Testataanvain todellisia k�ytt�j�n antamia arvoja, ei testata mit��n statuksia, 
// tai status stringeja, koska verrokkia ei v�ltt�m�tt� p�ivitet�.
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
    if(itsTriggerData != other.itsTriggerData)
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


std::function<std::string(bool)> NFmiBetaProductAutomation::itsBetaProductionBaseDirectoryGetter;

void NFmiBetaProductAutomation::SetBetaProductionBaseDirectoryGetter(std::function<std::string(bool)> &getterFunction)
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
    itsBetaProductPath = PathUtils::getTrueFilePath(theBetaProductPath, NFmiBetaProductAutomation::itsBetaProductionBaseDirectoryGetter(true), NFmiBetaProductionSystem::BetaProductFileExtension(), &extensionAdded);

    if(itsBetaProductPath.empty())
        itsBetaProductPathStatusString = ::GetDictionaryString("No Beta product in use");
    else
    {
        NFmiFileString fileString(itsBetaProductPath);

        // Tutkitaan l�ytyyk� annettua viewMacroa ollenkaan
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
// T�m� on hyv� tehd� kun ajetaan automaatioita ja joku on saattanut tehd� muutoksia tuotteeseen.
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
    // T�ss� puret��n NFmiTriggerModeInfo luokan p��tason pareja.
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

// T�t� kutsutaan kun esim. luetaan data tiedostosta ja tehd��n t�isi tarkistus kaikille osille
void NFmiBetaProductAutomation::DoFullChecks()
{
    CheckBetaProductPath(itsOriginalBetaProductPath);
    CheckTriggerModeInfo(itsTriggerModeInfo.itsTriggerMode, itsTriggerModeInfo.itsFixedRunTimesString, itsTriggerModeInfo.itsRunTimeStepInHoursString, itsTriggerModeInfo.itsFirstRunTimeOfDayString, itsTriggerModeInfo.itsTriggerDataString);
    CheckStartTimeModeInfo(itsStartTimeModeInfo.itsTimeMode, itsStartTimeModeInfo.itsWallClockOffsetInHoursString);
    CheckEndTimeModeInfo(itsEndTimeModeInfo.itsTimeMode, itsEndTimeModeInfo.itsWallClockOffsetInHoursString);
}

// T�ss� tehd��n beta automation listassa oleville Beta-automaation tilaa kuvaava satus info teksti.
// Kyseess� on siis vain parilla sanalla sanottu tilanne, jotta se mahtuisi dialogissa olevaan sarakkeeseen.
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

// Testataanvain todellisia k�ytt�j�n antamia arvoja, ei testata mit��n statuksia, 
// tai status stringeja, koska verrokkia ei v�ltt�m�tt� p�ivitet�.
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

// T�t� kutsutaan kun esim. luetaan data tiedostosta ja tehd��n t�ysi tarkistus kaikille osille
void NFmiBetaProductAutomationListItem::DoFullChecks(bool fAutomationModeOn)
{
    itsStatus = kFmiListItemOk;
    if(itsBetaProductAutomation)
    {
        itsBetaProductAutomation->DoFullChecks();
        if(GetErrorStatus() == kFmiListItemOk) // Jos automaatio tuote on ok, lasketaan aina seuraava ajoaika valmiiksi
        {
            if(fAutomationModeOn)
                itsNextRunTime = itsBetaProductAutomation->TriggerModeInfo().CalcNextDueTime(itsLastRunTime);
            else
                itsNextRunTime = NFmiMetTime::gMissingTime;
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

// T�m� tallentaa json objektiin vain enable -lipun ja polun k�ytettyyn Beta-automaatioon. 
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
// *******  NFmiBetaProductAutomationList osio alkaa  *****************
// ********************************************************************

std::function<std::string(bool)> NFmiBetaProductAutomationList::itsBetaProductionBaseDirectoryGetter;

void NFmiBetaProductAutomationList::SetBetaProductionBaseDirectoryGetter(std::function<std::string(bool)> &getterFunction)
{
    NFmiBetaProductAutomationList::itsBetaProductionBaseDirectoryGetter = getterFunction;
}

NFmiBetaProductAutomationList::NFmiBetaProductAutomationList()
:itsAutomationVector()
{
}

// Oletus: theListItem.itsBetaProductAutomationPath:iin on jo laitettu k�ytetty polku.
// 1. Tarkistaa onko annettu polku absoluuttinen vai suhteellinen
// 2. Jos suhteellinen, laske absoluuttinen polku beta-product-base-directoryn mukaan
// 3. Jos absoluuttinen polku, laske suhteellinen polku em. base-directoryn mukaan.
// 4. Kokeillaan l�ytyyk� saatu absoluuttinen polku
bool NFmiBetaProductAutomationList::MakeListItemPathSettingsCheck(NFmiBetaProductAutomationListItem &theListItem, const std::string &theCheckedPath)
{
    std::string givenPath = theCheckedPath;
    if(givenPath.empty())
        return false;
    if(NFmiFileSystem::IsAbsolutePath(givenPath))
    {
        theListItem.itsBetaProductAutomationAbsolutePath = givenPath;
        theListItem.itsBetaProductAutomationPath = PathUtils::getRelativePathIfPossible(givenPath, NFmiBetaProductAutomationList::itsBetaProductionBaseDirectoryGetter(true));
    }
    else
    {
        theListItem.itsBetaProductAutomationAbsolutePath = PathUtils::getTrueFilePath(givenPath, NFmiBetaProductAutomationList::itsBetaProductionBaseDirectoryGetter(true), NFmiBetaProductionSystem::BetaAutomationFileExtension());
        theListItem.itsBetaProductAutomationPath = givenPath;
    }
    return NFmiFileSystem::FileExists(theListItem.itsBetaProductAutomationAbsolutePath);
}

bool NFmiBetaProductAutomationList::MakeListItemPathSettings(NFmiBetaProductAutomationListItem &theListItem)
{
    NFmiBetaProductAutomationListItem tmpItem = theListItem;
    // Kokeillaan ensin l�ytyyk� automaatio-tiedosto k�ytt�en mahdollista suhteellista polkua vastaan (k�ytt�j�n antama polku, joka voi olla suhteellinen)
    if(!MakeListItemPathSettingsCheck(tmpItem, tmpItem.itsBetaProductAutomationPath))
    { // jos ei l�ytynyt, kokeillaan viel� l�ytyyk� absoluuttisella polulla mit��n
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
    if(PrepareListItemAfterJsonRead(*listItem)) // Voidaan k�ytt�� t�t� metodia, vaikka listItemia ei olekaan luettu json-tiedostosta
    {
        listItem->fEnable = true;
        listItem->itsNextRunTime = listItem->itsBetaProductAutomation->TriggerModeInfo().CalcNextDueTime(NFmiMetTime(1));
        itsAutomationVector.push_back(listItem);
        return true;
    }

    return false;
}

// Rivi indeksi pit�� antaa 0-kantaisena indeksin�, oikeasti rivit grid-controllissa alkavat yhdest�, koska siin� on mukana otsikko rivi.
bool NFmiBetaProductAutomationList::Remove(size_t theZeroBasedRowIndex)
{
    if(theZeroBasedRowIndex < itsAutomationVector.size())
    {
        itsAutomationVector.erase(itsAutomationVector.begin() + theZeroBasedRowIndex);
        return true;
    }
    else
        return false; // indeksi ei osoittanut mihink��n vektorin olioon
}

static NFmiBetaProductAutomationListItem gDummyBetaProductAutomationListItem;

// Rivi indeksi pit�� antaa 0-kantaisena indeksin�, oikeasti rivit grid-controllissa alkavat yhdest�, koska siin� on mukana otsikko rivi.
NFmiBetaProductAutomationListItem& NFmiBetaProductAutomationList::Get(size_t theZeroBasedRowIndex)
{
    if(theZeroBasedRowIndex < itsAutomationVector.size())
        return *itsAutomationVector[theZeroBasedRowIndex];
    else
        return gDummyBetaProductAutomationListItem; // indeksi ei osoittanut mihink��n vektorin olioon
}

// Rivi indeksi pit�� antaa 0-kantaisena indeksin�, oikeasti rivit grid-controllissa alkavat yhdest�, koska siin� on mukana otsikko rivi.
const NFmiBetaProductAutomationListItem& NFmiBetaProductAutomationList::Get(size_t theZeroBasedRowIndex) const
{
    if(theZeroBasedRowIndex < itsAutomationVector.size())
        return *itsAutomationVector[theZeroBasedRowIndex];
    else
        return gDummyBetaProductAutomationListItem; // indeksi ei osoittanut mihink��n vektorin olioon
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
    { // t�ytet��n objekti vain jos l�ytyi yht��n talletettavaa dataa
        jsonObject.push_back(json_spirit::Pair(gJsonName_BetaAutomationListHeader, dataArray));
    }
    return jsonObject;
}

// ListItem on luettu json-tiedostosta NFmiBetaProductAutomationList -luvun yhteydess�, 
// sille pit�� tehd� seuraavia asioita ennen kuin se voidaan lis�t� itsAutomationVector:iin:
// 1. S��d� polut (abs vs relative)
// 2. Luo dynaamisesti oletus Beta-automation olio
// 3. Lue Beta-automation olio k�ytt��n
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
                itsAutomationVector.push_back(listItem); // Lis�t��n virheellisestikin luetut listItemit, jotta k�ytt�j� saisi palautetta
            }
        }
    }
}

// T�t� kutsutaan kun esim. luetaan data tiedostosta ja tehd��n t�ysi tarkistus kaikille osille
void NFmiBetaProductAutomationList::DoFullChecks(bool fAutomationModeOn)
{
    // Tutkitaan ensin erikseen jokainen automaatio-olio
    for(auto &listItem : itsAutomationVector)
        listItem->DoFullChecks(fAutomationModeOn);

    // Sitten tehd��n yhteis tarkastuksia: 1. onko samoja automaatioita useita listassa
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
    if(PrepareListItemAfterJsonRead(*listItemFromFile)) // Voidaan k�ytt�� t�t� metodia, vaikka listItemia ei olekaan luettu json-tiedostosta
    {
        // Jos luku meni hyvin, sijoitetaan annettu beta-automaatio p�ivitett�v��n otukseen
        automationListItem->itsBetaProductAutomation.swap(listItemFromFile->itsBetaProductAutomation);
        // p�ivitet��n viel� seuraava ajoaika
        automationListItem->itsNextRunTime = automationListItem->itsBetaProductAutomation->TriggerModeInfo().CalcNextDueTime(NFmiMetTime(1));
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

std::vector<std::shared_ptr<NFmiBetaProductAutomationListItem>> NFmiBetaProductAutomationList::GetDueAutomations(const NFmiMetTime &theCurrentTime)
{
    std::vector<std::shared_ptr<NFmiBetaProductAutomationListItem>> dueAutomations;
    for(auto &listItem : itsAutomationVector)
    {
        if(listItem->fEnable)
        {
            if(listItem->GetErrorStatus() == NFmiBetaProductAutomationListItem::kFmiListItemOk)
            {
                NFmiMetTime nextRuntime = listItem->itsBetaProductAutomation->TriggerModeInfo().CalcNextDueTime(listItem->itsLastRunTime);
                if(nextRuntime > listItem->itsLastRunTime && nextRuntime <= theCurrentTime)
                    dueAutomations.push_back(listItem);
            }
        }
    }

    return dueAutomations;
}

static const std::string gBetaAutomationListName = "Beta-automation list";

bool NFmiBetaProductAutomationList::StoreInJsonFormat(const NFmiBetaProductAutomationList &theBetaProductAutomationList, const std::string &theFilePath, std::string &theErrorStringOut)
{
    return ::StoreObjectInJsonFormat(theBetaProductAutomationList, theFilePath, gBetaAutomationListName, theErrorStringOut);
}

bool NFmiBetaProductAutomationList::ReadInJsonFormat(NFmiBetaProductAutomationList &theBetaProductAutomationList, const std::string &theFilePath, std::string &theErrorStringOut)
{
    bool status = ::ReadObjectInJsonFormat(theBetaProductAutomationList, theFilePath, gBetaAutomationListName, theErrorStringOut);
    theBetaProductAutomationList.DoFullChecks(true); // Tehd��n t��ll� tarkastelut automaatiomoodi p��ll�, my�hemmin (t�t� funktiota kutsuvassa systeemiss�) tarkastelut on teht�v� uudestaan kun oikeasti tiedet��n miss� moodissa ollaan
    return status;
}


// ********************************************************************
// *********  NFmiBetaProductSystem osio alkaa  ***********************
// ********************************************************************

std::string NFmiBetaProductionSystem::itsRunTimeTitleString = ::GetDictionaryString("Run");
std::string NFmiBetaProductionSystem::itsRunTimeFormatString = ::GetDictionaryString("DDMMYY-HHz");

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

NFmiBetaProductionSystem::NFmiBetaProductionSystem()
: fBetaProductGenerationRunning(false)
, itsUsedAutomationList()
, itsBetaProductionBaseDirectory()
, itsBetaProductionBaseCacheDirectory()
, fDoCacheSyncronization(false)
, mBaseRegistryPath()
, mBetaProductTimeStepInMinutes()
, mBetaProductTimeLengthInHours()
, mBetaProductUseUtcTimesInTimeBox()
, mBetaProductTimeBoxLocation()
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
{
}

static std::string MakeBaseCacheDirectory(const std::string &theLocalCacheBaseDirectory, const std::string &theOriginalBaseDirectory)
{
    std::string path = theLocalCacheBaseDirectory;
    NFmiStringTools::ReplaceAll(path, "/", "\\"); // K��nnet��n kenoviivat windows tyyliin '/' -> '\'
    NFmiFileString fileStr(theOriginalBaseDirectory);
    fileStr.NormalizeDelimiter();
    path += fileStr.Path();
    NFmiStringTools::ReplaceAll(path, "\\\\", "\\"); // Mahdolliset tupla kenoviivat pit�� korvata yks�is kenoilla "\\" -> "\"
    return path;
}

bool NFmiBetaProductionSystem::Init(const std::string &theBaseRegistryPath, const std::string &theLocalCacheBaseDirectory)
{
    // N�m� alustetaan vain ja ainoastaan konffeista ainakin toistaiseksi
    itsBetaProductionBaseDirectory = NFmiSettings::Optional<std::string>("BetaProduction::BaseDirectory", "C:\\smartmet\\BetaProducts\\");
    fDoCacheSyncronization = NFmiSettings::Optional<bool>("BetaProduction::DoCacheSyncronization", false);

    itsBetaProductionBaseCacheDirectory = itsBetaProductionBaseDirectory;
    if(fDoCacheSyncronization)
        itsBetaProductionBaseCacheDirectory = ::MakeBaseCacheDirectory(theLocalCacheBaseDirectory, itsBetaProductionBaseDirectory);

    mBaseRegistryPath = theBaseRegistryPath;
    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;
    // Beta product section
    std::string betaProductSectionName = "\\BetaProduct";
    mBetaProductTimeStepInMinutes = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, betaProductSectionName, "\\TimeStepInMinutes", usedKey, 60);
    mBetaProductTimeLengthInHours = ::CreateRegValue<CachedRegDouble>(mBaseRegistryPath, betaProductSectionName, "\\TimeLengthInHours", usedKey, 15);
    mBetaProductUseUtcTimesInTimeBox = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, betaProductSectionName, "\\UseUtcTimesInTimeBox", usedKey, false);
    mBetaProductTimeBoxLocation = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, betaProductSectionName, "\\TimeBoxLocation", usedKey, kBottomLeft);
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

    mAutomationModeOn = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, betaProductSectionName, "\\AutomationModeOn", usedKey, false, "");
    mUsedAutomationListPathString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\UsedAutomationListPath", usedKey, "");
    mBetaProductPath = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\BetaProductPath", usedKey, "");
    mBetaProductTabControlIndex = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, betaProductSectionName, "\\BetaProductTabControlIndex", usedKey, 0);

    mBetaProductSaveInitialPath = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\SaveInitialPath", usedKey, itsBetaProductionBaseCacheDirectory); // otetaan Beta-product base-directory oletus arvoksi t�h�n

    mTriggerModeIndex = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, betaProductSectionName, "\\TriggerModeIndex", usedKey, 0);
    mFixedTimesString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\FixedTimes", usedKey, "");
    mAutomationTimeStepInHoursString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\AutomationTimeStepInHours", usedKey, "");
    mFirstRunTimeOfDayString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\FirstRunTimeOfDayString", usedKey, "");
    mStartTimeModeIndex = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, betaProductSectionName, "\\StartTimeModeIndex", usedKey, 0);
    mEndTimeModeIndex = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, betaProductSectionName, "\\EndTimeModeIndex", usedKey, 0);
    mStartTimeClockOffsetInHoursString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\StartTimeClockOffsetInHours", usedKey, "");
    mEndTimeClockOffsetInHoursString = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, betaProductSectionName, "\\EndTimeClockOffsetInHours", usedKey, "");

    std::function<std::string(bool)> getterFunction = std::bind(&NFmiBetaProductionSystem::GetBetaProductionBaseDirectory, this, std::placeholders::_1);
    NFmiBetaProductAutomation::SetBetaProductionBaseDirectoryGetter(getterFunction);
    NFmiBetaProductAutomationList::SetBetaProductionBaseDirectoryGetter(getterFunction);

    LoadUsedAutomationList(UsedAutomationListPathString());

    return true;
}

// SmartMetin CMainFrm::OnTimer kutsuu t�t� funktiota kerran minuutissa ja p��ttelee onko teht�v� mit��n 
// itsUsedAutomationList:alla olevaa tuotantoa.
bool NFmiBetaProductionSystem::DoNeededBetaAutomation()
{
    NFmiMetTime currentTime(1); // Otetaan talteen nyky UTC hetki minuutin tarkkuudella
    if(AutomationModeOn())
    {
        std::vector<std::shared_ptr<NFmiBetaProductAutomationListItem>> dueAutomations = itsUsedAutomationList.GetDueAutomations(currentTime);
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


// Lataa k�ytt��n annetusta polusta automaatio-listan.
// Asettaa polun my�s mUsedAutomationListPathString:in arvoksi.
bool NFmiBetaProductionSystem::LoadUsedAutomationList(const std::string &thePath)
{
    UsedAutomationListPathString(thePath);
    std::string fullPath = PathUtils::getTrueFilePath(thePath, GetBetaProductionBaseDirectory(true), NFmiBetaProductionSystem::BetaAutomationListFileExtension());
    std::string errorString;
    itsUsedAutomationList = NFmiBetaProductAutomationList(); // Nollataan k�yt�ss� ollut lista ennen uuden lukua
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

FmiDirection NFmiBetaProductionSystem::BetaProductTimeBoxLocation()
{
    int tmpValue = *mBetaProductTimeBoxLocation;
    return static_cast<FmiDirection>(tmpValue);
}

void NFmiBetaProductionSystem::BetaProductTimeBoxLocation(FmiDirection newValue)
{
    *mBetaProductTimeBoxLocation = newValue;
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

std::string NFmiBetaProductionSystem::BetaProductSaveInitialPath()
{
    return *mBetaProductSaveInitialPath;
}

void NFmiBetaProductionSystem::BetaProductSaveInitialPath(const std::string &newValue)
{
    *mBetaProductSaveInitialPath = newValue;
}

bool NFmiBetaProductionSystem::AutomationModeOn()
{
    return *mAutomationModeOn;
}

void NFmiBetaProductionSystem::AutomationModeOn(bool newValue)
{
    *mAutomationModeOn = newValue;
}

// fGetUserPath = true tarkoittaa ett� halutaan polku mit� oikeasti k�ytet��n 'ty�ss�' eli 
// se on lokaali cache-polku jos niin asetuksissa on m��r�tty. Jos lokaali cachea ei k�ytet�
// palautetaan originaali(server)-polku.
// fGetUserPath = false tarkoittaa ett� halutaan polku 'server' osoitteeseen, t�m� voi 
// siis olla vain 'originaali' polku.
std::string NFmiBetaProductionSystem::GetBetaProductionBaseDirectory(bool fGetUserPath)
{
    if(fGetUserPath && fDoCacheSyncronization)
        return itsBetaProductionBaseCacheDirectory;
    else
        return itsBetaProductionBaseDirectory;
}

//void NFmiBetaProductionSystem::SetBetaProductionBaseDirectory(const std::string &newValue)
//{
//    *mBetaProductionBaseDirectory = newValue;
//}

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
