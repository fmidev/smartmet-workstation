// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::AnalysisSources
 */
// ======================================================================

#ifndef TEXTGEN_ANALYSISSOURCES_H
#define TEXTGEN_ANALYSISSOURCES_H

#include <boost/shared_ptr.hpp>

namespace TextGen
{
class MaskSource;
class WeatherSource;

class AnalysisSources
{
 public:
  boost::shared_ptr<WeatherSource> getWeatherSource() const;
  boost::shared_ptr<MaskSource> getMaskSource() const;
  boost::shared_ptr<MaskSource> getLandMaskSource() const;
  boost::shared_ptr<MaskSource> getCoastMaskSource() const;
  boost::shared_ptr<MaskSource> getInlandMaskSource() const;
  boost::shared_ptr<MaskSource> getNorthernMaskSource() const;
  boost::shared_ptr<MaskSource> getSouthernMaskSource() const;
  boost::shared_ptr<MaskSource> getEasternMaskSource() const;
  boost::shared_ptr<MaskSource> getWesternMaskSource() const;

  void setWeatherSource(const boost::shared_ptr<WeatherSource>& theSource);
  void setMaskSource(const boost::shared_ptr<MaskSource>& theSource);
  void setLandMaskSource(const boost::shared_ptr<MaskSource>& theSource);
  void setCoastMaskSource(const boost::shared_ptr<MaskSource>& theSource);
  void setInlandMaskSource(const boost::shared_ptr<MaskSource>& theSource);
  void setNorthernMaskSource(const boost::shared_ptr<MaskSource>& theSource);
  void setSouthernMaskSource(const boost::shared_ptr<MaskSource>& theSource);
  void setEasternMaskSource(const boost::shared_ptr<MaskSource>& theSource);
  void setWesternMaskSource(const boost::shared_ptr<MaskSource>& theSource);

 private:
  boost::shared_ptr<WeatherSource> itsWeatherSource;
  boost::shared_ptr<MaskSource> itsMaskSource;
  boost::shared_ptr<MaskSource> itsLandMaskSource;
  boost::shared_ptr<MaskSource> itsCoastMaskSource;
  boost::shared_ptr<MaskSource> itsInlandMaskSource;
  boost::shared_ptr<MaskSource> itsNorthernMaskSource;
  boost::shared_ptr<MaskSource> itsSouthernMaskSource;
  boost::shared_ptr<MaskSource> itsEasternMaskSource;
  boost::shared_ptr<MaskSource> itsWesternMaskSource;

};  // class AnalysisSources

}  // namespace TextGen

#endif  // TEXTGEN_ANALYSISSOURCES_H

// ======================================================================
