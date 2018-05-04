// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::Phrase
 */
// ======================================================================

#ifndef TEXTGEN_PHRASE_H
#define TEXTGEN_PHRASE_H

#include "Glyph.h"
#include <string>

namespace TextGen
{
class Dictionary;

class Phrase : public Glyph
{
 public:
  virtual ~Phrase();
  Phrase(const std::string& theWord);
#ifdef NO_COMPILER_GENERATED
  Phrase(const Phrase& thePhrase);
  Phrase& operator=(const Phrase& thePhrase);
#endif
  virtual boost::shared_ptr<Glyph> clone() const;
  virtual std::string realize(const Dictionary& theDictionary) const;
  virtual std::string realize(const TextFormatter& theFormatter) const;
  virtual bool isDelimiter() const;

 private:
  Phrase();
  std::string itsWord;

};  // class Phrase

}  // namespace TextGen

#endif  // TEXTGEN_PHRASE_H

// ======================================================================
