// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::CoastMaskSource
 */
// ======================================================================

#ifndef TEXTGEN_COASTMASKSOURCE_H
#define TEXTGEN_COASTMASKSOURCE_H

#include <calculator/MaskSource.h>

namespace TextGen
{
class CoastMaskSource : public MaskSource
{
 public:
  typedef MaskSource::mask_type mask_type;
  typedef MaskSource::masks_type masks_type;

  CoastMaskSource(const WeatherArea& theCoast);

  virtual mask_type mask(const WeatherArea& theArea,
                         const std::string& theData,
                         const WeatherSource& theWeatherSource) const;

  virtual masks_type masks(const WeatherArea& theArea,
                           const std::string& theData,
                           const WeatherSource& theWeatherSource) const;

 private:
  CoastMaskSource();

  class Pimple;
  boost::shared_ptr<Pimple> itsPimple;

};  // class CoastMaskSource

}  // namespace TextGen

#endif  // TEXTGEN_COASTMASKSOURCE_H

// ======================================================================
