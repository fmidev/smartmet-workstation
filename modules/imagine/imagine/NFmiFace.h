// ======================================================================
/*!
 * \file
 * \brief Interface of class Imagine::NFmiFace
 */
// ======================================================================

#ifdef UNIX  // En saanut viel‰ Imagine::NFmiFace-luokkaa toimimaan VC++ 2012 k‰‰nt‰j‰ss‰
#pragma once

#include "imagine-config.h"

#ifdef IMAGINE_WITH_CAIRO
#error "Either Cairo or us"
#endif

#include "NFmiAlignment.h"
#include "NFmiColorTools.h"

#include <string>

namespace Imagine
{
class NFmiFreeType;
class NFmiImage;

class NFmiFace
{
 public:
  ~NFmiFace();
  NFmiFace(const std::string& theFontSpec);
  NFmiFace(const std::string& theFile, int theWidth, int theHeight);
#ifndef NO_COMPILER_GENERATED
  NFmiFace(const NFmiFace& theFace);
  NFmiFace& operator=(const NFmiFace& theFace);
#endif

  void Background(bool theMode);
  void BackgroundMargin(int theWidth, int theHeight);
  void BackgroundColor(NFmiColorTools::Color theColor);
  void BackgroundRule(NFmiColorTools::NFmiBlendRule theRule);

  void Draw(NFmiImage& theImage,
            int theX,
            int theY,
            const std::string& theText,
            NFmiAlignment theAlignment = kFmiAlignNorthWest,
            NFmiColorTools::Color theColor = NFmiColorTools::Black,
            NFmiColorTools::NFmiBlendRule theRule = NFmiColorTools::kFmiColorOnOpaque) const;

 private:
  NFmiFace();

  std::string itsFile;
  int itsWidth;
  int itsHeight;

  bool itsBackgroundOn;
  int itsBackgroundWidth;
  int itsBackgroundHeight;
  NFmiColorTools::Color itsBackgroundColor;
  NFmiColorTools::NFmiBlendRule itsBackgroundRule;

};  // class NFmiFace
}  // namespace Imagine

#endif  // IMAGINE_NFMIFACE_H

// ======================================================================
