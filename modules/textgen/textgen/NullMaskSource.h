// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::NullMaskSource
 */
// ======================================================================

#ifndef TEXTGEN_NULLMASKSOURCE_H
#define TEXTGEN_NULLMASKSOURCE_H

#include <calculator/MaskSource.h>

namespace TextGen
{
class NullMaskSource : public MaskSource
{
 public:
  typedef MaskSource::mask_type mask_type;
  typedef MaskSource::masks_type masks_type;

  NullMaskSource();

  virtual mask_type mask(const WeatherArea& theArea,
                         const std::string& theData,
                         const WeatherSource& theWeatherSource) const;

  virtual masks_type masks(const WeatherArea& theArea,
                           const std::string& theData,
                           const WeatherSource& theWeatherSource) const;

 private:
  boost::shared_ptr<NFmiIndexMask> itsData;

};  // class NullMaskSource

}  // namespace TextGen

#endif  // TEXTGEN_NULLMASKSOURCE_H

// ======================================================================
