// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::RealRange
 */
// ======================================================================

#ifndef TEXTGEN_REALRANGE_H
#define TEXTGEN_REALRANGE_H

#include "Glyph.h"
#include "TextFormatter.h"
#include <string>

namespace TextGen
{
class Dictionary;

class RealRange : public Glyph
{
 public:
  virtual ~RealRange();
  explicit RealRange(float theStartValue,
                     float theEndValue,
                     int thePrecision = 1,
                     const std::string& theSeparator = "-");
#ifdef NO_COMPILER_GENERATED
  RealRange(const RealRange& theRealRange);
  RealRange& operator=(const RealRange& theRealRange);
#endif

  virtual boost::shared_ptr<Glyph> clone() const;
  virtual std::string realize(const Dictionary& theDictionary) const;
  virtual std::string realize(const TextFormatter& theFormatter) const;

  virtual bool isDelimiter() const;

  const std::string& rangeSeparator() const;
  float startValue() const;
  float endValue() const;
  int precision() const;

 protected:
  std::string itsRangeSeparator;
  float itsStartValue;
  float itsEndValue;
  int itsPrecision;

 private:
  RealRange();

};  // class RealRange
}  // namespace RealRange

#endif  // TEXTGEN_REAL_H

// ======================================================================
