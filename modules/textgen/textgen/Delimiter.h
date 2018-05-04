// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::Delimiter
 */
// ======================================================================

#ifndef TEXTGEN_DELIMITER_H
#define TEXTGEN_DELIMITER_H

#include "Glyph.h"
#include <string>

namespace TextGen
{
class Delimiter : public Glyph
{
 public:
  virtual ~Delimiter();
  Delimiter(const std::string& theSeparator);
#ifdef NO_COMPILER_GENERATED
  Delimiter(const Delimiter& thePhrase);
  Delimiter& operator=(const Delimiter& thePhrase);
#endif

  virtual boost::shared_ptr<Glyph> clone() const;
  virtual std::string realize(const Dictionary& theDictionary) const;
  virtual std::string realize(const TextFormatter& theFormatter) const;
  virtual bool isDelimiter() const;

  const std::string& value() const;

 private:
  Delimiter();

  std::string itsSeparator;

};  // class Delimiter

}  // namespace TextGen

#endif  // TEXTGEN_DELIMITER_H

// ======================================================================
