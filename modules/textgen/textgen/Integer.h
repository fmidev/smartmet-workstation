// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::Integer
 */
// ======================================================================

#ifndef TEXTGEN_INTEGER_H
#define TEXTGEN_INTEGER_H

#include "Glyph.h"
#include <string>

namespace TextGen
{
class Dictionary;
class TextFormatter;

class Integer : public Glyph
{
 public:
  virtual ~Integer();
  explicit Integer(int theInteger);

#ifdef NO_COMPILER_GENERATED
  Integer(const Integer& theInteger);
  Integer& operator=(const Integer& theInteger);
#endif

  virtual boost::shared_ptr<Glyph> clone() const;
  virtual std::string realize(const Dictionary& theDictionary) const;
  virtual std::string realize(const TextFormatter& theFormatter) const;

  virtual bool isDelimiter() const;
  int value() const;

 private:
  Integer();
  int itsInteger;

};  // class Integer
}  // namespace Integer

#endif  // TEXTGEN_INTEGER_H

// ======================================================================
