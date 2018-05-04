// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::Header
 */
// ======================================================================

#ifndef TEXTGEN_HEADER_H
#define TEXTGEN_HEADER_H

#include "GlyphContainer.h"
#include <boost/shared_ptr.hpp>
#include <string>

namespace TextGen
{
class Header : public GlyphContainer
{
 public:
  virtual ~Header() {}
#ifdef NO_COMPILER_GENERATED
  Header();
  Header(const Header& theHeader);
  Header& operator=(const Header& theHeader);
#endif

  virtual boost::shared_ptr<Glyph> clone() const;
  virtual std::string realize(const Dictionary& theDictionary) const;
  virtual std::string realize(const TextFormatter& theFormatter) const;
  virtual bool isDelimiter() const;

  Header& operator<<(const Glyph& theGlyph);
  Header& operator<<(const std::string& thePhrase);
  Header& operator<<(int theNumber);

};  // class Header

}  // namespace TextGen

#endif  // TEXTGEN_HEADER_H

// ======================================================================
