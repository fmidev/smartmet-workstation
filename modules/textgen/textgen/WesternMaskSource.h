// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::WesternMaskSource
 */
// ======================================================================

#ifndef TEXTGEN_WESTERNMASKSOURCE_H
#define TEXTGEN_WESTERNMASKSOURCE_H

#include <calculator/MaskSource.h>

namespace TextGen
{
class WesternMaskSource : public MaskSource
{
 public:
  typedef MaskSource::mask_type mask_type;
  typedef MaskSource::masks_type masks_type;

  WesternMaskSource(const WeatherArea& theArea);

  virtual mask_type mask(const WeatherArea& theArea,
                         const std::string& theData,
                         const WeatherSource& theWeatherSource) const;

  virtual masks_type masks(const WeatherArea& theArea,
                           const std::string& theData,
                           const WeatherSource& theWeatherSource) const;

 private:
  WesternMaskSource();

  class Pimple;
  boost::shared_ptr<Pimple> itsPimple;

};  // class WesternMaskSource

}  // namespace TextGen

#endif  // TEXTGEN_WESTERNMASKSOURCE_H

// ======================================================================
