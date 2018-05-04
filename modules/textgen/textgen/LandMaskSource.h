// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::LandMaskSource
 */
// ======================================================================

#ifndef TEXTGEN_LANDMASKSOURCE_H
#define TEXTGEN_LANDMASKSOURCE_H

#include <calculator/MaskSource.h>

namespace TextGen
{
class LandMaskSource : public MaskSource
{
 public:
  typedef MaskSource::mask_type mask_type;
  typedef MaskSource::masks_type masks_type;

  LandMaskSource(const WeatherArea& theLand);

  virtual mask_type mask(const WeatherArea& theArea,
                         const std::string& theData,
                         const WeatherSource& theWeatherSource) const;

  virtual masks_type masks(const WeatherArea& theArea,
                           const std::string& theData,
                           const WeatherSource& theWeatherSource) const;

 private:
  LandMaskSource();

  class Pimple;
  boost::shared_ptr<Pimple> itsPimple;

};  // class LandMaskSource

}  // namespace TextGen

#endif  // TEXTGEN_LANDMASKSOURCE_H

// ======================================================================
