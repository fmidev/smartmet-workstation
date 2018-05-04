// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::Sentence
 */
// ======================================================================

#ifndef TEXTGEN_SENTENCE_H
#define TEXTGEN_SENTENCE_H

#include "GlyphContainer.h"
#include <boost/shared_ptr.hpp>
#include <string>

namespace TextGen
{
class Sentence : public GlyphContainer
{
 public:
  virtual ~Sentence() {}
#ifdef NO_COMPILER_GENERATED
  Sentence();
  Sentence(const Sentence& theSentence);
  Sentence& operator=(const Sentence& theSentence);
#endif

  virtual boost::shared_ptr<Glyph> clone() const;
  virtual std::string realize(const Dictionary& theDictionary) const;
  virtual std::string realize(const TextFormatter& theFormatter) const;
  virtual bool isDelimiter() const;

  Sentence& operator<<(const Sentence& theSentence);
  Sentence& operator<<(const Glyph& theGlyph);
  Sentence& operator<<(const std::string& thePhrase);
  Sentence& operator<<(int theNumber);

};  // class Sentence

}  // namespace TextGen

#endif  // TEXTGEN_SENTENCE_H

// ======================================================================
