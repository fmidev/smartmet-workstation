// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::PlainLinesTextFormatter
 */
// ======================================================================

#ifndef TEXTGEN_PLAINLINESTEXTFORMATTER_H
#define TEXTGEN_PLAINLINESTEXTFORMATTER_H

#include "PlainTextFormatter.h"

namespace TextGen
{
class PlainLinesTextFormatter : public PlainTextFormatter
{
 public:
  PlainLinesTextFormatter() {}
  virtual ~PlainLinesTextFormatter() {}
  using PlainTextFormatter::visit;
  virtual std::string visit(const Paragraph& theParagraph) const;

  std::string name() const { return "plainlines"; }
};  // class PlainLinesTextFormatter
}  // namespace TextGen

#endif  // TEXTGEN_PLAINLINESTEXTFORMATTER_H

// ======================================================================
