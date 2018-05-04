// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::MaskSource
 */
// ======================================================================

#ifndef TEXTGEN_MASKSOURCE_H
#define TEXTGEN_MASKSOURCE_H

#include <boost/shared_ptr.hpp>
#include <string>

class NFmiIndexMask;
class NFmiIndexMaskSource;
class NFmiPoint;

namespace TextGen
{
class MapSource;
class WeatherArea;
class WeatherSource;

class MaskSource
{
 public:
  typedef boost::shared_ptr<NFmiIndexMask> mask_type;
  typedef boost::shared_ptr<NFmiIndexMaskSource> masks_type;

  virtual ~MaskSource() {}
  virtual mask_type mask(const WeatherArea& theArea,
                         const std::string& theData,
                         const WeatherSource& theWeatherSource) const = 0;

  virtual masks_type masks(const WeatherArea& theArea,
                           const std::string& theData,
                           const WeatherSource& theWeatherSource) const = 0;

};  // class MaskSource

}  // namespace TextGen

#endif  // TEXTGEN_MASKSOURCE_H

// ======================================================================
