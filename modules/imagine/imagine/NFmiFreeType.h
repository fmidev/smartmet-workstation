// ======================================================================
/*!
 * \file
 * \brief Interface of singleton class Imagine::NFmiFreeType
 */
// ======================================================================

#ifdef UNIX  // En saanut viel‰ Imagine::NFmiFreeType -luokkaa toimimaan VC++ 2012 k‰‰nt‰j‰ss‰

#pragma once

#include "imagine-config.h"

#ifdef IMAGINE_WITH_CAIRO
#error "Either Cairo or us"
#endif

#include "NFmiAlignment.h"
#include "NFmiColorTools.h"

#include <boost/shared_ptr.hpp>
#include <string>

namespace Imagine
{
class NFmiImage;

class NFmiFreeType
{
 public:
  static NFmiFreeType& Instance();

  void Draw(
      NFmiImage& theImage,
      const std::string& theFont,
      int theWidth,
      int theHeight,
      int theX,
      int theY,
      const std::string& theText,
      NFmiAlignment theAlignment = kFmiAlignNorthWest,
      NFmiColorTools::Color theColor = NFmiColorTools::Black,
      NFmiColorTools::NFmiBlendRule theRule = NFmiColorTools::kFmiColorOnOpaque,
      bool theBackgroundOn = false,
      int theBackgroundWidth = 3,
      int theBackgroundHeight = 3,
      NFmiColorTools::Color theBackgroundColor = NFmiColorTools::MakeColor(180, 180, 180, 32),
      NFmiColorTools::NFmiBlendRule theBackgroundRule = NFmiColorTools::kFmiColorOnOpaque) const;

 private:
  class Pimple;
  boost::shared_ptr<Pimple> itsPimple;

  // Private - only NFmiFreeType itself is allowed to call these
  ~NFmiFreeType();
  NFmiFreeType();

  // Disabled intentionally:

  NFmiFreeType(const NFmiFreeType& theOb);
  NFmiFreeType& operator=(const NFmiFreeType& theOb);

};  // class NFmiFreeType
}  // namespace Imagine

#endif  // IMAGINE_NFMIFREETYPE_H

// ======================================================================
