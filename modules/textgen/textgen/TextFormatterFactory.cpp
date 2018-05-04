// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace TextGen::TextFormatterFactory
 */
// ======================================================================
/*!
 * \namespace TextGen::TextFormatterFactory
 *
 * \brief TextFormatter creation services
 *
 * The responsibility of the TextFormatterFactory namespace is to provide
 * TextFormatter creation services.
 *
 * For example,
 * \code
 * boost::shared_ptr<TextFormatter> form1(TextFormatterFactory::create("plain"));
 * boost::shared_ptr<TextFormatter> form2(TextFormatterFactory::create("html"));
 * \endcode
 *
 */
// ======================================================================

#include "TextFormatterFactory.h"
#include "DebugTextFormatter.h"
#include "PlainTextFormatter.h"
#include "PlainLinesTextFormatter.h"
#include "SpeechTextFormatter.h"
#include "HtmlTextFormatter.h"
#include "CssTextFormatter.h"
#include "WmlTextFormatter.h"
#include "SoneraTextFormatter.h"
#include <calculator/TextGenError.h>

using namespace boost;

namespace TextGen
{
namespace TextFormatterFactory
{
// ----------------------------------------------------------------------
/*!
 * \brief Create a text formatter of given type
 *
 * The available formatters are
 *
 *    - plain for plain text
 *    - html for html
 *
 * \param theType The type of the text formatter
 * \return An uninitialized text formatter
 */
// ----------------------------------------------------------------------

TextFormatter* create(const std::string& theType)
{
  if (theType == "plain") return new PlainTextFormatter();
  if (theType == "plainlines") return new PlainLinesTextFormatter();
  if (theType == "html") return new HtmlTextFormatter();
  if (theType == "css") return new CssTextFormatter();
  if (theType == "speechtext") return new SpeechTextFormatter();
  if (theType == "wml") return new WmlTextFormatter();
  if (theType == "sonera") return new SoneraTextFormatter();
  if (theType == "debug") return new DebugTextFormatter();
  throw TextGenError("Error: Unknown text formatter type " + theType);
}

}  // namespace TextFormatterFactory
}  // namespace TextGen

// ======================================================================
