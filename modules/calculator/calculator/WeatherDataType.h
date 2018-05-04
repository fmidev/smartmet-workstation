// ======================================================================
/*!
 * \file
 * \brief Definition of enumeration TextGen::WeatherDataType
 */
// ======================================================================

#ifndef TEXTGEN_WEATHERDATATYPE_H
#define TEXTGEN_WEATHERDATATYPE_H

namespace TextGen
{
enum WeatherDataType
{
  Forecast,
  Observation,
  Climatology
};

}  // namespace TextGen

const char* data_type_name(const TextGen::WeatherDataType& theDataType);

#endif  // TEXTGEN_WEATHERDATATYPE_H

// ======================================================================
