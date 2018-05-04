// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::SpecialStory::none
 */
// ======================================================================

#include "SpecialStory.h"
#include "MessageLogger.h"
#include "Paragraph.h"

using namespace TextGen;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Generate story on none
 *
 * \return The story
 */
// ----------------------------------------------------------------------

Paragraph SpecialStory::none() const
{
  MessageLogger log("SpecialStory::none");

  Paragraph paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
