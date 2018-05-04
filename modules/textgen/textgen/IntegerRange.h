// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::IntegerRange
 */
// ======================================================================

#ifndef TEXTGEN_INTEGERRANGE_H
#define TEXTGEN_INTEGERRANGE_H

#include "Glyph.h"
#include "TextFormatter.h"
#include <string>

namespace TextGen
{
class Dictionary;

class IntegerRange : public Glyph
{
 public:
  virtual ~IntegerRange();
  explicit IntegerRange(int theStartValue, int theEndValue, const std::string& theSeparator = "-");
#ifdef NO_COMPILER_GENERATED
  IntegerRange(const IntegerRange& theIntegerRange);
  IntegerRange& operator=(const IntegerRange& theIntegerRange);
#endif

  virtual boost::shared_ptr<Glyph> clone() const;
  virtual std::string realize(const Dictionary& theDictionary) const;
  virtual std::string realize(const TextFormatter& theFormatter) const;

  virtual bool isDelimiter() const;

  const std::string& rangeSeparator() const;
  int startValue() const;
  int endValue() const;

 protected:
  std::string itsRangeSeparator;
  int itsStartValue;
  int itsEndValue;

 private:
  IntegerRange();

};  // class IntegerRange
}  // namespace IntegerRange

#endif  // TEXTGEN_INTEGER_H

// ======================================================================
