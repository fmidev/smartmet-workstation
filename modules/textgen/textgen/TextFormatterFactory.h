// ======================================================================
/*!
 * \file
 * \brief Interface of namespace TextGen::TextFormatterFactory
 */
// ======================================================================

#ifndef TEXTGEN_TEXTFORMATTERFACTORY_H
#define TEXTGEN_TEXTFORMATTERFACTORY_H

#include "TextFormatter.h"

namespace TextGen
{
namespace TextFormatterFactory
{
TextFormatter* create(const std::string& theType);

}  // namespace TextFormatterFactory
}  // namespace TextGen

#endif  // TEXTGEN_TEXTFORMATTERFACTORY_H

// ======================================================================
