// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::NorthernMaskSource
 */
// ======================================================================

#ifndef TEXTGEN_NORTHERNMASKSOURCE_H
#define TEXTGEN_NORTHERNMASKSOURCE_H

#include <calculator/MaskSource.h>

namespace TextGen
{
class NorthernMaskSource : public MaskSource
{
 public:
  typedef MaskSource::mask_type mask_type;
  typedef MaskSource::masks_type masks_type;

  NorthernMaskSource(const WeatherArea& theArea);

  virtual mask_type mask(const WeatherArea& theArea,
                         const std::string& theData,
                         const WeatherSource& theWeatherSource) const;

  virtual masks_type masks(const WeatherArea& theArea,
                           const std::string& theData,
                           const WeatherSource& theWeatherSource) const;

 private:
  NorthernMaskSource();

  class Pimple;
  boost::shared_ptr<Pimple> itsPimple;

};  // class NorthernMaskSource

}  // namespace TextGen

#endif  // TEXTGEN_NORTHERNMASKSOURCE_H

// ======================================================================
