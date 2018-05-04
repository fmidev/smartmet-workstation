// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::Glyph
 */
// ======================================================================

#ifndef TEXTGEN_GLYPH_H
#define TEXTGEN_GLYPH_H

#include <boost/shared_ptr.hpp>
#include <string>

namespace TextGen
{
class Dictionary;
class TextFormatter;

class Glyph
{
 public:
  virtual ~Glyph() {}
  virtual boost::shared_ptr<Glyph> clone() const = 0;
  virtual std::string realize(const Dictionary& theDictionary) const = 0;
  virtual std::string realize(const TextFormatter& theFormatter) const = 0;
  virtual bool isDelimiter() const = 0;

 protected:
  Glyph() {}
};  // class Glyph
}  // namespace TextGen

#endif  // TEXTGEN_GLYPH_H

// ======================================================================
