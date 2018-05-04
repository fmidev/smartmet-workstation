// ======================================================================
/*!
 * \file
 * \brief Implementation of abstract class TextGen::ParameterAnalyzer
 */
// ======================================================================
/*!
 * \class TextGen::ParameterAnalyzer
 *
 * \brief Parameter analysis functions
 *
 */
// ======================================================================

#include "ParameterAnalyzer.h"
#include "TextGenError.h"

namespace TextGen
{
void ParameterAnalyzer::getParameterStrings(const WeatherParameter& theParameter,
                                            std::string& theParameterName,
                                            std::string& theDataName)
{
  switch (theParameter)
  {
    case WindDirection:
    {
      theDataName = "textgen::winddirection";
      theParameterName = "WindDirection";
    }
    break;
    case WaveDirection:
    {
      theDataName = "textgen::wavedirection";
      theParameterName = "WaveDirection";
    }
    break;
    case Temperature:
    {
      theDataName = "textgen::temperature";
      theParameterName = "Temperature";
    }
    break;
    case Precipitation:
    {
      theDataName = "textgen::precipitation";
      theParameterName = "Precipitation1h";
    }
    break;
    case Cloudiness:
    {
      theDataName = "textgen::cloudiness";
      theParameterName = "TotalCloudCover";
    }
    break;
    case Frost:
    {
      theDataName = "textgen::frost";
      theParameterName = "FrostProbability";
    }
    break;
    case SevereFrost:
    {
      theDataName = "textgen::severefrost";
      theParameterName = "SevereFrostProbability";
    }
    break;
    case RelativeHumidity:
    {
      theDataName = "textgen::relativehumidity";
      theParameterName = "Humidity";
    }
    break;
    case WindSpeed:
    {
      theDataName = "textgen::windspeed";
      theParameterName = "WindSpeedMS";
    }
    break;
    case Thunder:
    {
      theDataName = "textgen::thunder";
      theParameterName = "ProbabilityThunderstorm";
    }
    break;
    case PrecipitationType:
    {
      theDataName = "textgen::precipitationtype";
      theParameterName = "PrecipitationType";
    }
    break;
    case PrecipitationForm:
    {
      theDataName = "textgen::precipitationform";
      theParameterName = "PrecipitationForm";
    }
    break;
    case PrecipitationProbability:
    {
      theDataName = "textgen::precipitationprobability";
      theParameterName = "PoP";
    }
    break;
    case RoadTemperature:
    {
      theDataName = "textgen::roadtemperature";
      theParameterName = "RoadTemperature";
    }
    break;
    case RoadCondition:
    {
      theDataName = "textgen::roadcondition";
      theParameterName = "RoadCondition";
    }
    break;
    case WaveHeight:
    {
      theDataName = "textgen::waveheight";
      theParameterName = "SigWaveHeight";
    }
    break;
    case RoadWarning:
    {
      theDataName = "textgen::roadwarning";
      theParameterName = "RoadWarning";
    }
    break;
    case ForestFireIndex:
    {
      theDataName = "textgen::forestfireindex";
      theParameterName = "ForestFireWarning";
    }
    break;
    case Evaporation:
    {
      theDataName = "textgen::evaporation";
      theParameterName = "Evaporation";
    }
    break;
    case DewPoint:
    {
      theDataName = "textgen::dewpoint";
      theParameterName = "DewPoint";
    }
    break;
    case GustSpeed:
    {
      theDataName = "textgen::gustspeed";
      theParameterName = "HourlyMaximumGust";
    }
    break;
    case Pressure:
    {
      theDataName = "textgen::pressure";
      theParameterName = "Pressure";
    }
    break;
    case Fog:
    {
      theDataName = "textgen::fog";
      theParameterName = "FogIntensity";
    }
    break;
    case MaximumWind:
    {
      theDataName = "textgen::maximumwind";
      theParameterName = "HourlyMaximumWindSpeed";
    }
    break;
    case EffectiveTemperatureSum:
    {
      theDataName = "textgen::effectivetemperaturesum";
      theParameterName = "EffectiveTemperatureSum";
    }
    break;
    case PrecipitationRate:
    {
      theDataName = "textgen::precipitationrate";
      theParameterName = "PrecipitationRate";
    }
    break;
    case WaterEquivalentOfSnow:
    {
      theDataName = "textgen::waterequivalentofsnow";
      theParameterName = "WaterEquivalentOfSnow";
    }
    break;
    case NormalMaxTemperatureF02:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMaxTemperatureF02";
    }
    break;
    case NormalMaxTemperatureF12:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMaxTemperatureF12";
    }
    break;
    case NormalMaxTemperatureF37:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMaxTemperatureF37";
    }
    break;
    case NormalMaxTemperatureF50:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMaxTemperatureF50";
    }
    break;
    case NormalMaxTemperatureF63:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMaxTemperatureF63";
    }
    break;
    case NormalMaxTemperatureF88:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMaxTemperatureF88";
    }
    break;
    case NormalMaxTemperatureF98:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMaxTemperatureF98";
    }
    break;

    case NormalMeanTemperatureF02:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMeanTemperatureF02";
    }
    break;
    case NormalMeanTemperatureF12:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMeanTemperatureF12";
    }
    break;
    case NormalMeanTemperatureF37:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMeanTemperatureF37";
    }
    break;
    case NormalMeanTemperatureF50:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMeanTemperatureF50";
    }
    break;
    case NormalMeanTemperatureF63:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMeanTemperatureF63";
    }
    break;
    case NormalMeanTemperatureF88:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMeanTemperatureF88";
    }
    break;
    case NormalMeanTemperatureF98:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMeanTemperatureF98";
    }
    break;

    case NormalMinTemperatureF02:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMinTemperatureF02";
    }
    break;
    case NormalMinTemperatureF12:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMinTemperatureF12";
    }
    break;
    case NormalMinTemperatureF37:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMinTemperatureF37";
    }
    break;
    case NormalMinTemperatureF50:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMinTemperatureF50";
    }
    break;
    case NormalMinTemperatureF63:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMinTemperatureF63";
    }
    break;
    case NormalMinTemperatureF88:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMinTemperatureF88";
    }
    break;
    case NormalMinTemperatureF98:
    {
      theDataName = "textgen::fractiles";
      theParameterName = "NormalMinTemperatureF98";
    }
    break;
    case WindChill:
    {
      theDataName = "textgen::windchill";
      theParameterName = "WindChill";
    }
    break;
    case GrowthPeriodOnOff:
    {
      theDataName = "textgen::effectivetemperaturesum";
      theParameterName = "GrowthPeriodOnOff";
    }
    break;
  }
}

const char* getDataTypeName(const TextGen::WeatherDataType& theDataType)
{
  switch (theDataType)
  {
    case Forecast:
      return "forecast";
    case Observation:
      return "observation";
    case Climatology:
      return "climatology";
  }

  throw TextGenError("Unrecognized WeatherDataType in ParameterAnalyzer::getDataTypeName");
}

}  // namespace TextGen
