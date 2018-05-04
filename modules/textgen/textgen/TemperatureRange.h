// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::TemperatureRange
 */
// ======================================================================

#ifndef TEXTGEN_TEMPERATURERANGE_H
#define TEXTGEN_TEMPERATURERANGE_H

#include "IntegerRange.h"
#include "TextFormatter.h"
#include <string>

namespace TextGen
{
class Dictionary;

class TemperatureRange : public IntegerRange
{
 public:
  explicit TemperatureRange(int theStartValue,
                            int theEndValue,
                            const std::string& theSeparator = "-");

  virtual boost::shared_ptr<Glyph> clone() const;
  virtual std::string realize(const Dictionary& theDictionary) const;
  virtual std::string realize(const TextFormatter& theFormatter) const;

};  // class TemperatureRange

}  // namespace Textgen

#endif  // TEXTGEN_TEMPERATURERANGE_H

// ======================================================================
