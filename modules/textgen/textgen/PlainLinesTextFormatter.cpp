// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::PlainTextFormatter
 */
// ======================================================================
/*!
 * \class TextGen::PlainLinesTextFormatter
 *
 * \brief Glyph visitor generating normal ASCII output
 */
// ======================================================================

#include "PlainLinesTextFormatter.h"
#include "Paragraph.h"
#include "TextFormatterTools.h"

using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Visit a paragraph
 */
// ----------------------------------------------------------------------

string PlainLinesTextFormatter::visit(const Paragraph& theParagraph) const
{
  string ret =
      TextFormatterTools::realize(theParagraph.begin(), theParagraph.end(), *this, "\n", "");
  return ret;
}

}  // namespace TextGen

// ======================================================================
