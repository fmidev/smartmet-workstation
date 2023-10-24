#include "NFmiTempDataGenerator.h"
#include "NFmiQueryData.h"
#include "NFmiTEMPCode.h"
#include "NFmiSoundingData.h"
#include "NFmiAviationStationInfoSystem.h"
#include "NFmiTimeList.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiFastInfoUtils.h"
#include "NFmiDictionaryFunction.h"
#include <boost/algorithm/string.hpp>

namespace
{
    class SoundingLevelValues
    {
    public:
        NFmiMetTime time = NFmiMetTime::gMissingTime;
        NFmiPoint latlon = NFmiPoint::gMissingLatlon;
        float P = kFloatMissing;
        float geop = kFloatMissing;
        float T = kFloatMissing;
        float Td = kFloatMissing;
        float RH = kFloatMissing;
        float WD = kFloatMissing;
        float WS = kFloatMissing;

        bool operator==(const SoundingLevelValues& other) const
        {
            if(time != other.time)
                return false;
            if(latlon != other.latlon)
                return false;
            if(P != other.P)
                return false;
            if(geop != other.geop)
                return false;
            if(T != other.T)
                return false;
            if(Td != other.Td)
                return false;
            if(RH != other.RH)
                return false;
            if(WD != other.WD)
                return false;
            if(WS != other.WS)
                return false;

            return true;
        }

        // Onko yht‰‰n ei-puuttuvaa arvoa datassa
        bool HasAnyData() const
        {
            return !(*this == SoundingLevelValues());
        }

        void SetParameterValue(FmiParameterName parameter, float value)
        {
            switch(parameter)
            {
            case kFmiLongitude:
                latlon.X(value);
                break;
            case kFmiLatitude:
                latlon.Y(value);
                break;
            case kFmiPressure:
                P = value;
                break;
            case kFmiGeopHeight:
                geop = value;
                break;
            case kFmiTemperature:
                T = value;
                break;
            case kFmiDewPoint:
                Td = value;
                break;
            case kFmiHumidity:
                RH = value;
                break;
            case kFmiWindDirection:
                WD = value;
                break;
            case kFmiWindSpeedMS:
                WS = value;
                break;
            default:
                break;
            }
        }
    };


    // Wyomingin CSV datassa on seuraavat parametrikent‰t (t‰ll‰ hetkell‰).
    // Ne joiden FmiParameterName arvo on kFmiBadParameter, j‰tet‰‰n huomioimatta, 
    // koska niit‰ ei meid‰n systeemit ainakaan viel‰ k‰sittele luotausdatan kanssa.
    std::map<std::string, FmiParameterName> gKnownDescriptionParameterMappings =
    {
        {"time", kFmiUTCTime},
        {"longitude", kFmiLongitude},
        {"latitude", kFmiLatitude},
        {"pressure", kFmiPressure},
        {"geopotential height", kFmiGeopHeight},
        {"temperature", kFmiTemperature},
        {"dew point temperature", kFmiDewPoint},
        {"ice point temperature", kFmiBadParameter},
        {"relative humidity", kFmiHumidity},
        {"humidity wrt ice", kFmiBadParameter},
        {"mixing ratio", kFmiBadParameter},
        {"wind direction", kFmiWindDirection},
        {"wind speed", kFmiWindSpeedMS}
    };

    std::vector<FmiParameterName> GenerateParameterListFromParameterDescriptionLine(const std::string& thePossibleParameterDescriptionLine)
    {
        bool foundAnyParameters = false;
        std::vector<FmiParameterName> parameters;
        std::vector<std::string> parameterParts;
        boost::split(parameterParts, thePossibleParameterDescriptionLine, boost::is_any_of(","));
        // Parametrin nimet otetaan tarkoituksella omaan kopioon, koska sille tehd‰‰n sitten to_lower operaatio
        for(auto parameterStr : parameterParts)
        {
            // Varmistetaan ett‰ kaikki toiminta tehd‰‰n aina lower case stringeill‰
            boost::algorithm::to_lower(parameterStr);
            auto iter = gKnownDescriptionParameterMappings.find(parameterStr);
            if(iter != gKnownDescriptionParameterMappings.end())
            {
                foundAnyParameters = true;
                parameters.push_back(iter->second);
            }
            else
            {
                parameters.push_back(kFmiBadParameter);
            }
        }

        if(foundAnyParameters)
        {
            return parameters;
        }

        throw std::runtime_error("No matching parameters found from given string");
    }

    bool IsNextLineTrimmedEmpty(std::istream& input, std::string &nextLineStrOut)
    {
        if(!input)
        {
            return false; // Jos on luettu input loppuun, palautetaan false
        }
        std::getline(input, nextLineStrOut); // Read line
        boost::trim(nextLineStrOut); // Siivotaan mahdolliset whitespacet pois rivilt‰
        return nextLineStrOut.empty();
    }

    std::string GetFirstNoneEmptyLine(std::istream& input)
    {
        // Skipataan kaikki mahdolliset whitespace rivit ensin
        std::string line;
        for(; ::IsNextLineTrimmedEmpty(input, line); )
        {
        }

        if(input)
        {
            return line;
        }
        else
        {
            // Syˆte oli jo lopussa, palautetaan vain tyhj‰‰
            return "";
        }
    }

    NFmiMetTime ParseTime(const std::string& timeStr)
    {
        // Aika on Wyomingin CSV datassa seuraavaa formaattia:
        // 2023-03-02 11:30:12
        // YYYY-MM-DD HH:mm:SS

        try
        {
            std::vector<std::string> timeStringParts;
            boost::split(timeStringParts, timeStr, boost::is_any_of(" "));
            if(timeStringParts.size() == 2)
            {
                std::vector<std::string> dateParts;
                boost::split(dateParts, timeStringParts[0], boost::is_any_of("-"));
                std::vector<std::string> timeParts;
                boost::split(timeParts, timeStringParts[1], boost::is_any_of(":"));
                if(dateParts.size() == 3 && timeParts.size() == 3)
                {
                    auto year = std::stoi(dateParts[0]);
                    auto month = std::stoi(dateParts[1]);
                    auto day = std::stoi(dateParts[2]);
                    auto hour = std::stoi(timeParts[0]);
                    auto minute = std::stoi(timeParts[1]);
                    auto second = std::stoi(timeParts[2]);
                    return NFmiMetTime(year, month, day, hour, minute, second, 1);
                }
            }
        }
        catch(...)
        { }
        
        return NFmiMetTime::gMissingTime;
    }

    float ParseValue(const std::string& valueStr)
    {
        try
        {
            return std::stof(valueStr);
        }
        catch(...)
        { }
        return kFloatMissing;
    }

    void ParseValueAndSetData(FmiParameterName parameter, const std::string& valueStr, SoundingLevelValues& soundingLevelValues)
    {
        switch(parameter)
        {
        case kFmiUTCTime:
        {
            soundingLevelValues.time = ::ParseTime(valueStr);
            break;
        }
        default:
            soundingLevelValues.SetParameterValue(parameter, ::ParseValue(valueStr));
            break;
        }

    }

    std::pair<bool, SoundingLevelValues> ParseSoundingLevelValues(const std::string& levelValuesStr, const std::vector<FmiParameterName> &parameters)
    {
        try
        {
            std::vector<std::string> valueParts;
            boost::split(valueParts, levelValuesStr, boost::is_any_of(","));
            // Minimiss‰‰n arvorivin pit‰‰ saman m‰‰r‰ arvoja kuin parameters vectorissa on parametreja
            if(valueParts.size() >= parameters.size())
            {
                SoundingLevelValues soundingLevelValues;
                for(size_t index = 0; index < parameters.size(); index++)
                {
                    ::ParseValueAndSetData(parameters[index], valueParts[index], soundingLevelValues);
                }
                return std::make_pair(soundingLevelValues.HasAnyData(), soundingLevelValues);
            }
        }
        catch(...)
        { }

        // Jos ei saatu purettua mielekk‰‰sti rivi‰, palautetaan sen merkiksi false ja missing arvoilla oleva data
        return std::make_pair(false, SoundingLevelValues());
    }

    NFmiMetTime DeduceSoundingDataTime(const std::vector<SoundingLevelValues>& soundingLevelValuesVector)
    {
        // Haetaan ensimm‰inen ei-puuttuva aika listasta ja pyˆristet‰‰n se seuraavaan tasatuntiin.
        for(const auto& levelValues : soundingLevelValuesVector)
        {
            if(levelValues.time != NFmiMetTime::gMissingTime)
            {
                auto finalTime = levelValues.time;
                finalTime.SetTimeStep(60, true, kForward);
                return finalTime;
            }
        }
        // Jos luotaus datasta ei lˆydy mit‰‰n j‰rkev‰‰ aikaa, ei kannata tehd‰ sit‰ ollenkaan
        throw std::runtime_error("Couldn't find any non-missing time from this sounding data");
    }

    NFmiPoint GetLowestLevelLatlon(const std::vector<SoundingLevelValues>& soundingLevelValuesVector)
    {
        // Haetaan ensimm‰inen ei-puuttuva latlon paikka
        for(const auto& levelValues : soundingLevelValuesVector)
        {
            if(levelValues.latlon != NFmiPoint::gMissingLatlon)
            {
                return levelValues.latlon;
            }
        }
        // Jos luotaus datasta ei lˆydy mit‰‰n j‰rkev‰‰ aikaa, ei kannata tehd‰ sit‰ ollenkaan
        throw std::runtime_error("Couldn't find any non-missing latlon location from this sounding data");
    }

    template<typename StationMapKey>
    std::pair<double, NFmiAviationStation> FindNearestAviationStation(const NFmiPoint& latlon, const std::map<StationMapKey,NFmiAviationStation> & aviationStationMap)
    {
        double minDistance = std::numeric_limits<double>::max();
        const NFmiAviationStation *minDistanceStation = nullptr;
        for(const auto& aviationStationPair : aviationStationMap)
        {
            auto& aviationStation = aviationStationPair.second;
            auto currentDistance = aviationStation.Distance(latlon);
            if(minDistance > currentDistance)
            {
                minDistance = currentDistance;
                minDistanceStation = &aviationStation;
            }
        }
        if(minDistanceStation)
            return std::make_pair(minDistance, *minDistanceStation);
        else
            return std::make_pair(minDistance, NFmiAviationStation());
    }

    NFmiLocation FindNearestSoundingLocation(const NFmiPoint& latlon, NFmiAviationStationInfoSystem& tempStations)
    {
        const auto& nearestIcaoStationPair = ::FindNearestAviationStation(latlon, tempStations.IcaoStations());
        const auto& nearestWmoStationPair = ::FindNearestAviationStation(latlon, tempStations.WmoStations());
        if(nearestIcaoStationPair.first < nearestWmoStationPair.first)
            return nearestIcaoStationPair.second;
        else
            return nearestWmoStationPair.second;
    }

    std::unique_ptr<NFmiSoundingData> CreateSoundingDataFromLevels(const std::vector<SoundingLevelValues>& soundingLevelValuesVector, NFmiAviationStationInfoSystem& tempStations)
    {
        if(!soundingLevelValuesVector.empty())
        {
            auto soundingDataPtr = std::make_unique<NFmiSoundingData>();
            soundingDataPtr->Time(::DeduceSoundingDataTime(soundingLevelValuesVector));
            soundingDataPtr->Location(::FindNearestSoundingLocation(::GetLowestLevelLatlon(soundingLevelValuesVector), tempStations));
            auto& P_values = soundingDataPtr->GetParamData(kFmiPressure);
            auto& geom_values = soundingDataPtr->GetParamData(kFmiGeomHeight);
            auto& T_values = soundingDataPtr->GetParamData(kFmiTemperature);
            auto& Td_values = soundingDataPtr->GetParamData(kFmiDewPoint);
            auto& RH_values = soundingDataPtr->GetParamData(kFmiHumidity);
            auto& WS_values = soundingDataPtr->GetParamData(kFmiWindSpeedMS);
            auto& WD_values = soundingDataPtr->GetParamData(kFmiWindDirection);
            for(const auto& soundingLevelValues : soundingLevelValuesVector)
            {
                P_values.push_back(soundingLevelValues.P);
                geom_values.push_back(soundingLevelValues.geop);
                T_values.push_back(soundingLevelValues.T);
                Td_values.push_back(soundingLevelValues.Td);
                RH_values.push_back(soundingLevelValues.RH);
                WS_values.push_back(soundingLevelValues.WS);
                WD_values.push_back(soundingLevelValues.WD);
            }

            // Tuulidatan eri parametrit pit‰‰ t‰ytt‰‰ seuraavaksi
            NFmiFastInfoUtils::MetaWindParamUsage metaWindParamUsage(false, false, true, false);
            soundingDataPtr->FillRestOfWindData(metaWindParamUsage);
            boost::shared_ptr<NFmiFastQueryInfo> emptyDummyInfoPtr;
            soundingDataPtr->MakeFillDataPostChecks(emptyDummyInfoPtr);
            return soundingDataPtr;
        }
        return nullptr;
    }

    std::unique_ptr<NFmiSoundingData> GenerateSingleSoundingData(std::istream &input, NFmiAviationStationInfoSystem& tempStations)
    {
        auto parameterLineStr = ::GetFirstNoneEmptyLine(input);
        if(!parameterLineStr.empty())
        {
            try
            {
                auto parameters = ::GenerateParameterListFromParameterDescriptionLine(parameterLineStr);
                std::string parameterValueLine;
                std::vector<SoundingLevelValues> soundingLevelValuesVector;
                for(; !::IsNextLineTrimmedEmpty(input, parameterValueLine); )
                {
                    auto soundingLevelValuesPair = ::ParseSoundingLevelValues(parameterValueLine, parameters);
                    if(soundingLevelValuesPair.first)
                    {
                        soundingLevelValuesVector.push_back(soundingLevelValuesPair.second);
                    }
                }
                return ::CreateSoundingDataFromLevels(soundingLevelValuesVector, tempStations);
            }
            catch(...)
            {

            }
        }

        return nullptr;
    }

    NFmiParamDescriptor MakeParamDescriptor(const NFmiProducer& wantedProducer)
    {
        NFmiParamBag params;
        params.Add(NFmiDataIdent(NFmiParam(kFmiPressure, "P"), wantedProducer));
        params.Add(NFmiDataIdent(NFmiParam(kFmiGeomHeight, "geomheight"), wantedProducer));
        params.Add(NFmiDataIdent(NFmiParam(kFmiTemperature, "T"), wantedProducer));
        params.Add(NFmiDataIdent(NFmiParam(kFmiDewPoint, "Td"), wantedProducer));
        params.Add(NFmiDataIdent(NFmiParam(kFmiHumidity, "RH"), wantedProducer));
        params.Add(NFmiDataIdent(NFmiParam(kFmiWindSpeedMS, "WS"), wantedProducer));
        params.Add(NFmiDataIdent(NFmiParam(kFmiWindDirection, "WD"), wantedProducer));

        return NFmiParamDescriptor(params);
    }

    NFmiVPlaceDescriptor MakeVPlaceDescriptor(const std::list<std::unique_ptr<NFmiSoundingData>>& soundingDataList)
    {
        float maxLevelSize = 0;
        for(const auto &sounding : soundingDataList)
        {  
            // Etsit‰‰n se luotaus miss‰ on eniten levelej‰, ja otetaan se koko 'pohjaksi' levelbagiin
            auto levelCount = (float)sounding->GetParamData(kFmiPressure).size();
            if(maxLevelSize < levelCount)
            {
                maxLevelSize = levelCount;
            }
        }

        return NFmiVPlaceDescriptor(NFmiLevelBag(kFmiSoundingLevel, 1, maxLevelSize, 1));
    }

    struct TimeListCollector
    {
        template<typename SoundingDataPtr>
        void operator()(SoundingDataPtr& soundingData) 
        { 
            timeSet.insert(soundingData->Time()); 
        }

        std::set<NFmiMetTime> timeSet;
    };

    struct LocationCollector
    {
        template<typename SoundingDataPtr>
        void operator()(const SoundingDataPtr& soundingData) { locationSet.insert(soundingData->Location()); }
        std::set<NFmiLocation> locationSet;
    };

    NFmiTimeDescriptor MakeTimeDescriptor(const std::list<std::unique_ptr<NFmiSoundingData>>& soundingDataList)
    {
        NFmiMetTime originTime(60);
        auto timeListCollector = std::for_each(soundingDataList.begin(), soundingDataList.end(), TimeListCollector());
        NFmiTimeList timeList;
        for(const auto& time : timeListCollector.timeSet)
        {
            // Huom! NFmiTimeList::Add metodi ei tee Clone:a annetuista ajoista, vaan ottaa ne sellaisenaan omistukseensa.
            // Lis‰t‰‰n ajat vain loppuun (3. parametri true), koska on vain erilaisia aikoja aikaj‰rjestyksess‰.
            timeList.Add(new NFmiMetTime(time), false, true);
        }
        return NFmiTimeDescriptor(originTime, timeList);
    }

    NFmiHPlaceDescriptor MakeHPlaceDescriptor(const std::list<std::unique_ptr<NFmiSoundingData>>& soundingDataList)
    {
        auto locationCollector = std::for_each(soundingDataList.begin(), soundingDataList.end(), LocationCollector());
        NFmiLocationBag locationBag;
        for(const auto& location : locationCollector.locationSet)
        {
            locationBag.AddLocation(location, false);
        }
        return NFmiHPlaceDescriptor(locationBag);
    }

    void FillSoundingData(NFmiQueryData& qdata, const std::list<std::unique_ptr<NFmiSoundingData>>& soundingDataList)
    {
        NFmiFastQueryInfo fastInfo(&qdata);
        for(const auto& soundingDataPtr : soundingDataList)
        {
            if(fastInfo.Time(soundingDataPtr->Time()))
            {
                if(fastInfo.Location(soundingDataPtr->Location().GetIdent()))
                {
                    auto& P_values = soundingDataPtr->GetParamData(kFmiPressure);
                    auto& geom_values = soundingDataPtr->GetParamData(kFmiGeomHeight);
                    auto& T_values = soundingDataPtr->GetParamData(kFmiTemperature);
                    auto& Td_values = soundingDataPtr->GetParamData(kFmiDewPoint);
                    auto& RH_values = soundingDataPtr->GetParamData(kFmiHumidity);
                    auto& WS_values = soundingDataPtr->GetParamData(kFmiWindSpeedMS);
                    auto& WD_values = soundingDataPtr->GetParamData(kFmiWindDirection);
                    int levelIndex = 0;
                    for(fastInfo.ResetLevel(); fastInfo.NextLevel(); levelIndex++)
                    {
                        if(levelIndex < P_values.size())
                        {
                            fastInfo.Param(kFmiPressure);
                            fastInfo.FloatValue(P_values[levelIndex]);
                            fastInfo.Param(kFmiGeomHeight);
                            fastInfo.FloatValue(geom_values[levelIndex]);
                            fastInfo.Param(kFmiTemperature);
                            fastInfo.FloatValue(T_values[levelIndex]);
                            fastInfo.Param(kFmiDewPoint);
                            fastInfo.FloatValue(Td_values[levelIndex]);
                            fastInfo.Param(kFmiHumidity);
                            fastInfo.FloatValue(RH_values[levelIndex]);
                            fastInfo.Param(kFmiWindSpeedMS);
                            fastInfo.FloatValue(WS_values[levelIndex]);
                            fastInfo.Param(kFmiWindDirection);
                            fastInfo.FloatValue(WD_values[levelIndex]);
                        }
                    }
                }
            }
        }
    }

    std::unique_ptr<NFmiQueryData> CreateQDataFromSoundings(const std::list<std::unique_ptr<NFmiSoundingData>>& soundingDataList, const NFmiProducer& wantedProducer)
    {
        auto paramDescriptor = ::MakeParamDescriptor(wantedProducer);
        auto vPlaceDescriptor = ::MakeVPlaceDescriptor(soundingDataList);
        auto timeDescriptor = ::MakeTimeDescriptor(soundingDataList);
        auto hPlaceDescriptor = ::MakeHPlaceDescriptor(soundingDataList);

        NFmiQueryInfo metaInfo(paramDescriptor, timeDescriptor, hPlaceDescriptor, vPlaceDescriptor);
        std::unique_ptr<NFmiQueryData> qdataPtr(NFmiQueryDataUtil::CreateEmptyData(metaInfo));
        if(qdataPtr)
        {
            ::FillSoundingData(*qdataPtr, soundingDataList);
        }

        return qdataPtr;
    }

    std::string MakeTimeString(const NFmiMetTime& atime)
    {
        return std::string(atime.ToStr("YYYY.MM.DD HH:mm"));
    }

    static std::string MakeCheckReport(NFmiQueryData* qdata, int theSoundingCount)
    {
        if(qdata && theSoundingCount)
        {
            NFmiFastQueryInfo info(qdata);
            if(info.SizeLevels() && info.SizeLocations() && info.SizeParams() && info.SizeTimes())
            {  // dataan oli saatu varmasti jotain, kun kaikki size:t ovat > 0
                std::string str;
                str += ::GetDictionaryString("TempCodeInsertDlgSoundingStr");
                str += ":";
                str += NFmiStringTools::Convert<int>(theSoundingCount);
                str += " ";
                str += ::GetDictionaryString("TempCodeInsertDlgPiecesStr");
                str += ", ";
                str += ::GetDictionaryString("TempCodeInsertDlgStationStr");
                str += ":";
                str += NFmiStringTools::Convert<int>(info.SizeLocations());
                str += " ";
                str += ::GetDictionaryString("TempCodeInsertDlgPiecesStr");
                str += ", ";
                str += ::GetDictionaryString("TempCodeInsertDlgTimeStr");
                str += ":";
                str += NFmiStringTools::Convert<int>(info.SizeTimes());
                str += " ";
                str += ::GetDictionaryString("TempCodeInsertDlgPiecesStr");
                str += ".\n";
                str += ::GetDictionaryString("Times: ");
                for(info.ResetTime(); info.NextTime(); )
                    str += ::MakeTimeString(info.Time()) + " ";
                return str;
            }
        }

        return ::GetDictionaryString("Unable to genereate any TEMP data from give input string.");
    }

    // Pseudo koodi ja ajatuksia miten sanomien k‰sittely pit‰isi tehd‰
    // 1. Jokainen yksitt‰inen luotaus on per‰kk‰isill‰ riveill‰
    // 2. Luotaukset erotetaan toisistaan yhdell‰ tai useammalla tyhj‰ll‰ rivill‰
    // 3. Jokainen luotaus alkaa parametrien nimi rivill‰
    // 4. Sit‰ seuraa 0-n kpl arvorivej‰ per‰kk‰in
    // 5. Joka kohdassa kysyt‰‰n onko nyt tyhj‰ rivi vai ei ja tehd‰‰n p‰‰tˆksi‰ sen mukaan
    // 6. Tyhj‰n rivin tarkastus tehd‰‰n seuraavalla tavalla
    //   auto len = fs.tellg();             // Get current position
    //   getline(fs, line);                 // Read line
    //   fs.seekg(len, std::ios_base::beg); // Return to position before "Read line".

    std::unique_ptr<NFmiQueryData> GenerateDataFromWyomingCsvText(const std::string& possibleCsvStr,
        std::string& checkReportStr,
        NFmiAviationStationInfoSystem& tempStations,
        const NFmiPoint& unknownStationLocation,
        const NFmiProducer& wantedProducer,
        bool roundTimesToNearestSynopticTimes)
    {
        std::stringstream input(possibleCsvStr);
        std::list<std::unique_ptr<NFmiSoundingData>> soundingDataList;
        do
        {
            auto soundingDataPtr = ::GenerateSingleSoundingData(input, tempStations);
            if(soundingDataPtr)
                soundingDataList.emplace_back(std::move(soundingDataPtr));
        } while(input);

        auto qdataPtr = ::CreateQDataFromSoundings(soundingDataList, wantedProducer);
        checkReportStr = ::MakeCheckReport(qdataPtr.get(), (int)soundingDataList.size());
        return qdataPtr;
    }

}

namespace NFmiTempDataGenerator
{

    std::unique_ptr<NFmiQueryData> GenerateDataFromText(const std::string& tempRelatedStr,
        std::string& checkReportStr,
        NFmiAviationStationInfoSystem& tempStations,
        const NFmiPoint& unknownStationLocation,
        const NFmiProducer& wantedProducer,
        bool roundTimesToNearestSynopticTimes)
    {
        try
        {
            // Tutkitaan ensin onko tekstidata vanhaa TEMP formaattia
            std::unique_ptr<NFmiQueryData> newDataPtr(DecodeTEMP::MakeNewDataFromTEMPStr(tempRelatedStr, checkReportStr, tempStations, unknownStationLocation, wantedProducer, roundTimesToNearestSynopticTimes));
            if(newDataPtr)
            {
                // Jos oli TEMP formaattia ja data luotiin, palautetaan se
                return newDataPtr;
            }

            // Kokeillaan onko tekstidata uutta CSV formaattia luotausdatalle, mit‰ Wyomingin yliopisto k‰ytt‰‰ palveluissaan
            return GenerateDataFromWyomingCsvText(tempRelatedStr, checkReportStr, tempStations, unknownStationLocation, wantedProducer, roundTimesToNearestSynopticTimes);
        }
        catch(std::exception& e)
        {
            checkReportStr = "Failed to create sounding data: ";
            checkReportStr += e.what();
        }
        return nullptr;
    }


}
