// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::LocationPhrase
 */
// ======================================================================

#ifndef TEXTGEN_LOCATIONPHRASE_H
#define TEXTGEN_LOCATIONPHRASE_H

#include "Glyph.h"
#include <string>

namespace TextGen
{
class Dictionary;

class LocationPhrase : public Glyph
{
 public:
  virtual ~LocationPhrase();
  LocationPhrase(const std::string& theLocation);
#ifdef NO_COMPILER_GENERATED
  LocationPhrase(const LocationPhrase& theLocationPhrase);
  LocationPhrase& operator=(const LocationPhrase& theLocationPhrase);
#endif
  virtual boost::shared_ptr<Glyph> clone() const;
  virtual std::string realize(const Dictionary& theDictionary) const;
  virtual std::string realize(const TextFormatter& theFormatter) const;
  virtual bool isDelimiter() const;

 private:
  LocationPhrase();
  std::string itsLocation;

};  // class LocationPhrase

}  // namespace TextGen

#endif  // TEXTGEN_LOCATIONPHRASE_H

// ======================================================================
