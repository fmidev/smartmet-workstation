// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::SouthernMaskSource
 */
// ======================================================================

#ifndef TEXTGEN_SOUTHERNMASKSOURCE_H
#define TEXTGEN_SOUTHERNMASKSOURCE_H

#include <calculator/MaskSource.h>

namespace TextGen
{
class SouthernMaskSource : public MaskSource
{
 public:
  typedef MaskSource::mask_type mask_type;
  typedef MaskSource::masks_type masks_type;

  SouthernMaskSource(const WeatherArea& theArea);

  virtual mask_type mask(const WeatherArea& theArea,
                         const std::string& theData,
                         const WeatherSource& theWeatherSource) const;

  virtual masks_type masks(const WeatherArea& theArea,
                           const std::string& theData,
                           const WeatherSource& theWeatherSource) const;

 private:
  SouthernMaskSource();

  class Pimple;
  boost::shared_ptr<Pimple> itsPimple;

};  // class SouthernMaskSource

}  // namespace TextGen

#endif  // TEXTGEN_SOUTHERNMASKSOURCE_H

// ======================================================================
