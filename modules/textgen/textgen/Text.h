// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::Text
 */
// ======================================================================

#ifndef TEXTGEN_TEXT_H
#define TEXTGEN_TEXT_H

#include "Glyph.h"
#include <string>

namespace TextGen
{
class Dictionary;
class TextFormatter;

class Text : public Glyph
{
 public:
  virtual ~Text();
  explicit Text(const std::string& theText);

#ifdef NO_COMPILER_GENERATED
  Text(const Text& theText);
  Text& operator=(const Text& theText);
#endif

  virtual boost::shared_ptr<Glyph> clone() const;
  virtual std::string realize(const Dictionary& theDictionary) const;
  virtual std::string realize(const TextFormatter& theFormatter) const;

  virtual bool isDelimiter() const;
  const std::string& value() const;

 private:
  Text();
  const std::string itsText;

};  // class Text
}  // namespace Text

#endif  // TEXTGEN_TEXT_H

// ======================================================================
