// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::Real
 */
// ======================================================================

#ifndef TEXTGEN_REAL_H
#define TEXTGEN_REAL_H

#include "Glyph.h"
#include <string>

namespace TextGen
{
class Dictionary;
class TextFormatter;

class Real : public Glyph
{
 public:
  virtual ~Real();
  explicit Real(float theReal, int thePrecision = 1, bool theComma = true);

#ifdef NO_COMPILER_GENERATED
  Real(const Real& theReal);
  Real& operator=(const Real& theReal);
#endif

  virtual boost::shared_ptr<Glyph> clone() const;
  virtual std::string realize(const Dictionary& theDictionary) const;
  virtual std::string realize(const TextFormatter& theFormatter) const;
  virtual bool isDelimiter() const;
  float value() const;
  int precision() const;
  bool comma() const;

 private:
  Real();
  float itsReal;
  int itsPrecision;
  bool itsComma;

};  // class Real
}  // namespace Real

#endif  // TEXTGEN_REAL_H

// ======================================================================
