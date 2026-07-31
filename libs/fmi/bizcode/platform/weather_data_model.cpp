#ifdef UNIX

#include "weather_data_model.h"

#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiDataMatrix.h>
#include <newbase/NFmiGlobals.h>
#include <newbase/NFmiLevel.h>

#include <algorithm>
#include <limits>

namespace
{
    std::string paramNameOf(const NFmiFastQueryInfo& theInfo)
    {
        return std::string(theInfo.Param().GetParamName().CharPtr());
    }

    std::string timeStringOf(NFmiFastQueryInfo& theInfo)
    {
        return std::string(theInfo.Time().ToStr(NFmiString("YYYY.MM.DD HH:mm")).CharPtr());
    }

    // Querydata level names are usually just "None", so build the label from the level
    // type instead - "700 hPa" rather than "None 700".
    std::string levelStringOf(const NFmiFastQueryInfo& theInfo)
    {
        const NFmiLevel* level = theInfo.Level();
        if(!level)
            return "";

        const std::string value = std::to_string(static_cast<long>(level->LevelValue()));
        switch(level->LevelType())
        {
            case kFmiPressureLevel:  return value + " hPa";
            case kFmiHybridLevel:    return "Hybrid " + value;
            case kFmiHeight:         return value + " m";
            case kFmiDepth:          return value + " m depth";
            case kFmiGroundSurface:  return "Surface";
            case kFmiMeanSeaLevel:   return "Mean sea level";
            default: break;
        }

        const std::string name(level->GetName().CharPtr());
        if(name.empty() || name == "None")
            return "Level " + value;
        return name + " " + value;
    }
}

bool WeatherDataModel::refreshGridData()
{
    itsValidValueCount = 0;
    if(!itsQueryInfo) return false;
    try
    {
        itsGridWidth = static_cast<int>(itsQueryInfo->GridXNumber());
        itsGridHeight = static_cast<int>(itsQueryInfo->GridYNumber());
        if(itsGridWidth <= 0 || itsGridHeight <= 0)
            return false;

        itsParamName = ::paramNameOf(*itsQueryInfo);
        itsTimeString = ::timeStringOf(*itsQueryInfo);
        itsLevelString = ::levelStringOf(*itsQueryInfo);

        // NFmiDataMatrix is indexed as [x][y], the renderer expects grid[y * width + x]
        NFmiDataMatrix<float> matrix = itsQueryInfo->Values();
        itsGridValues.resize(itsGridWidth * itsGridHeight);
        itsDataMin = std::numeric_limits<float>::max();
        itsDataMax = std::numeric_limits<float>::lowest();

        for(int y = 0; y < itsGridHeight; ++y)
            for(int x = 0; x < itsGridWidth; ++x)
            {
                float val = matrix[x][y];
                // Replace missing values with NaN so the renderer skips them
                if(val == kFloatMissing || val >= 32000.0f)
                    itsGridValues[y * itsGridWidth + x] = std::numeric_limits<float>::quiet_NaN();
                else
                {
                    itsGridValues[y * itsGridWidth + x] = val;
                    itsDataMin = std::min(itsDataMin, val);
                    itsDataMax = std::max(itsDataMax, val);
                    ++itsValidValueCount;
                }
            }

        if(itsValidValueCount == 0)
        {
            itsDataMin = 0;
            itsDataMax = 0;
        }
        return true;
    }
    catch(...) { return false; }
}

bool WeatherDataModel::nextTime()  { return itsQueryInfo && itsQueryInfo->NextTime() && refreshGridData(); }
bool WeatherDataModel::prevTime()  { return itsQueryInfo && itsQueryInfo->PreviousTime() && refreshGridData(); }
bool WeatherDataModel::nextParam() { return itsQueryInfo && itsQueryInfo->NextParam() && refreshGridData(); }
bool WeatherDataModel::prevParam() { return itsQueryInfo && itsQueryInfo->PreviousParam() && refreshGridData(); }
bool WeatherDataModel::nextLevel() { return itsQueryInfo && itsQueryInfo->NextLevel() && refreshGridData(); }
bool WeatherDataModel::prevLevel() { return itsQueryInfo && itsQueryInfo->PreviousLevel() && refreshGridData(); }

bool WeatherDataModel::selectData(int theIndex)
{
    const auto& datas = itsLoader.loadedData();
    if(datas.empty()) return false;

    const int count = static_cast<int>(datas.size());
    theIndex = ((theIndex % count) + count) % count;

    const auto& selected = datas[theIndex];
    itsCurrentDataIndex = theIndex;
    itsDataName = selected.name;
    itsQueryInfo = selected.info.get();
    itsQueryInfo->FirstParam();
    itsQueryInfo->FirstLevel();
    itsQueryInfo->FirstTime();
    return refreshGridData();
}

bool WeatherDataModel::selectFirstDataWithValues()
{
    const int count = dataCount();
    for(int i = 0; i < count; ++i)
    {
        if(!selectData(i))
            continue;
        if(itsValidValueCount > 0)
            return true;
        while(itsQueryInfo->NextParam())
        {
            if(refreshGridData() && itsValidValueCount > 0)
                return true;
        }
    }
    // Nothing had values - still show the first data rather than the demo screen.
    return selectData(0);
}

bool WeatherDataModel::selectParam(int theIndex)
{
    if(!itsQueryInfo || theIndex < 0) return false;
    if(!itsQueryInfo->ParamIndex(static_cast<unsigned long>(theIndex))) return false;
    return refreshGridData();
}

bool WeatherDataModel::selectLevel(int theIndex)
{
    if(!itsQueryInfo || theIndex < 0) return false;
    if(!itsQueryInfo->LevelIndex(static_cast<unsigned long>(theIndex))) return false;
    return refreshGridData();
}

bool WeatherDataModel::selectTime(int theIndex)
{
    if(!itsQueryInfo || theIndex < 0) return false;
    if(!itsQueryInfo->TimeIndex(static_cast<unsigned long>(theIndex))) return false;
    return refreshGridData();
}

std::vector<std::string> WeatherDataModel::dataNames() const
{
    std::vector<std::string> names;
    for(const auto& data : itsLoader.loadedData())
        names.push_back(data.name);
    return names;
}

std::vector<std::string> WeatherDataModel::paramNames() const
{
    std::vector<std::string> names;
    if(!itsQueryInfo) return names;

    // Iterating moves the info, so put it back where it was afterwards.
    const unsigned long saved = itsQueryInfo->ParamIndex();
    for(unsigned long i = 0; i < itsQueryInfo->SizeParams(); ++i)
    {
        if(itsQueryInfo->ParamIndex(i))
            names.push_back(::paramNameOf(*itsQueryInfo));
    }
    itsQueryInfo->ParamIndex(saved);
    return names;
}

std::vector<std::string> WeatherDataModel::levelNames() const
{
    std::vector<std::string> names;
    if(!itsQueryInfo) return names;

    const unsigned long saved = itsQueryInfo->LevelIndex();
    for(unsigned long i = 0; i < itsQueryInfo->SizeLevels(); ++i)
    {
        if(itsQueryInfo->LevelIndex(i))
            names.push_back(::levelStringOf(*itsQueryInfo));
    }
    itsQueryInfo->LevelIndex(saved);
    return names;
}

std::vector<std::string> WeatherDataModel::timeNames() const
{
    std::vector<std::string> names;
    if(!itsQueryInfo) return names;

    const unsigned long saved = itsQueryInfo->TimeIndex();
    for(unsigned long i = 0; i < itsQueryInfo->SizeTimes(); ++i)
    {
        if(itsQueryInfo->TimeIndex(i))
            names.push_back(::timeStringOf(*itsQueryInfo));
    }
    itsQueryInfo->TimeIndex(saved);
    return names;
}

int WeatherDataModel::currentParamIndex() const
{
    return itsQueryInfo ? static_cast<int>(itsQueryInfo->ParamIndex()) : -1;
}

int WeatherDataModel::currentLevelIndex() const
{
    return itsQueryInfo ? static_cast<int>(itsQueryInfo->LevelIndex()) : -1;
}

int WeatherDataModel::currentTimeIndex() const
{
    return itsQueryInfo ? static_cast<int>(itsQueryInfo->TimeIndex()) : -1;
}

bool WeatherDataModel::valueAt(double theRelX, double theRelY,
                               double& theLatOut, double& theLonOut, float& theValueOut) const
{
    if(!itsQueryInfo || itsGridWidth <= 0 || itsGridHeight <= 0)
        return false;
    if(theRelX < 0.0 || theRelX > 1.0 || theRelY < 0.0 || theRelY > 1.0)
        return false;

    // Grid row 0 is the southernmost, the map is drawn with north up.
    const int gx = std::clamp(static_cast<int>(std::lround(theRelX * (itsGridWidth - 1))),
                              0, itsGridWidth - 1);
    const int gyFromTop = std::clamp(static_cast<int>(std::lround(theRelY * (itsGridHeight - 1))),
                                     0, itsGridHeight - 1);
    const int gy = itsGridHeight - 1 - gyFromTop;

    const NFmiPoint latLon = itsQueryInfo->LatLon(static_cast<unsigned long>(gy * itsGridWidth + gx));
    theLatOut = latLon.Y();
    theLonOut = latLon.X();

    theValueOut = itsGridValues[gy * itsGridWidth + gx];
    return std::isfinite(theValueOut);
}

void WeatherDataModel::clear()
{
    itsQueryInfo = nullptr;
    itsCurrentDataIndex = -1;
    itsGridValues.clear();
    itsLoader.clear();
}

#endif // UNIX
