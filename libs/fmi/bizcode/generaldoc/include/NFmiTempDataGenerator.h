#pragma once

#include <memory>
#include <string>

class NFmiQueryData;
class NFmiAviationStationInfoSystem;
class NFmiPoint;
class NFmiProducer;

namespace NFmiTempDataGenerator
{

std::unique_ptr<NFmiQueryData> GenerateDataFromText(const std::string &tempRelatedStr,
                                      std::string &checkReportStr,
                                      NFmiAviationStationInfoSystem &tempStations,
                                      const NFmiPoint &unknownStationLocation,
                                      const NFmiProducer &wantedProducer,
                                      bool roundTimesToNearestSynopticTimes);

}
