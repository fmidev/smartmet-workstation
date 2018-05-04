// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::InlandMaskSource
 */
// ======================================================================

#ifndef TEXTGEN_INLANDMASKSOURCE_H
#define TEXTGEN_INLANDMASKSOURCE_H

#include <calculator/MaskSource.h>

namespace TextGen
{
class InlandMaskSource : public MaskSource
{
 public:
  typedef MaskSource::mask_type mask_type;
  typedef MaskSource::masks_type masks_type;

  InlandMaskSource(const WeatherArea& theCoast);

  virtual mask_type mask(const WeatherArea& theArea,
                         const std::string& theData,
                         const WeatherSource& theWeatherSource) const;

  virtual masks_type masks(const WeatherArea& theArea,
                           const std::string& theData,
                           const WeatherSource& theWeatherSource) const;

 private:
  InlandMaskSource();

  class Pimple;
  boost::shared_ptr<Pimple> itsPimple;

};  // class InlandMaskSource

}  // namespace TextGen

#endif  // TEXTGEN_INLANDMASKSOURCE_H

// ======================================================================
