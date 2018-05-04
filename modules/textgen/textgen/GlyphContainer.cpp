// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::GlyphContainer
 */
// ======================================================================
/*!
 * \class TextGen::GlyphContainer
 *
 * \brief A generic text glyph interface
 *
 */
// ======================================================================

#include "GlyphContainer.h"

using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Return the size of the container
 *
 * \return The size (number of non-recursive glyphs)
 */
// ----------------------------------------------------------------------

GlyphContainer::size_type GlyphContainer::size() const { return itsData.size(); }
// ----------------------------------------------------------------------
/*!
 * \brief Test if the container is empty
 *
 * \return True if the container is empty
 */
// ----------------------------------------------------------------------

bool GlyphContainer::empty() const { return itsData.empty(); }
// ----------------------------------------------------------------------
/*!
 * \brief Clear the container
 */
// ----------------------------------------------------------------------

void GlyphContainer::clear() { itsData.clear(); }
// ----------------------------------------------------------------------
/*!
 * \brief Append a new glyph to the container
 *
 * \param theGlyph The glyph to append
 */
// ----------------------------------------------------------------------

void GlyphContainer::push_back(const Glyph& theGlyph) { itsData.push_back(theGlyph.clone()); }
// ----------------------------------------------------------------------
/*!
 * \brief Append a new glyph to the container
 *
 * \param theGlyph The glyph to append
 */
// ----------------------------------------------------------------------

void GlyphContainer::push_back(const_reference theGlyph) { itsData.push_back(theGlyph); }
// ----------------------------------------------------------------------
/*!
 * \brief Insert a new glyph to the container
 *
 * \param theGlyph The glyph to insert
 */
// ----------------------------------------------------------------------

void GlyphContainer::push_front(const Glyph& theGlyph) { itsData.push_front(theGlyph.clone()); }
// ----------------------------------------------------------------------
/*!
 * \brief Insert a new glyph to the container
 *
 * \param theGlyph The glyph to insert
 */
// ----------------------------------------------------------------------

void GlyphContainer::push_front(const_reference theGlyph) { itsData.push_front(theGlyph); }
// ----------------------------------------------------------------------
/*!
 * \brief Return the begin iterator (const)
 *
 * \return The begin iterator
 */
// ----------------------------------------------------------------------

GlyphContainer::const_iterator GlyphContainer::begin() const { return itsData.begin(); }
// ----------------------------------------------------------------------
/*!
 * \brief Return the end iterator (const)
 *
 * \return The end iterator
 */
// ----------------------------------------------------------------------

GlyphContainer::const_iterator GlyphContainer::end() const { return itsData.end(); }
// ----------------------------------------------------------------------
/*!
 * \brief Return the begin iterator (non-const)
 *
 * \return The begin iterator
 */
// ----------------------------------------------------------------------

GlyphContainer::iterator GlyphContainer::begin() { return itsData.begin(); }
// ----------------------------------------------------------------------
/*!
 * \brief Return the end iterator (non-const)
 *
 * \return The end iterator
 */
// ----------------------------------------------------------------------

GlyphContainer::iterator GlyphContainer::end() { return itsData.end(); }
// ----------------------------------------------------------------------
/*!
 * \brief Return the front element
 *
 * \return The front element
 */
// ----------------------------------------------------------------------

GlyphContainer::const_reference GlyphContainer::front() const { return itsData.front(); }
// ----------------------------------------------------------------------
/*!
 * \brief Return the back element
 *
 * \return The back element
 */
// ----------------------------------------------------------------------

GlyphContainer::const_reference GlyphContainer::back() const { return itsData.back(); }
}  // namespace TextGen

// ======================================================================
