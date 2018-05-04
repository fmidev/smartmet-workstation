// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::Paragraph
 */
// ======================================================================

#ifndef TEXTGEN_PARAGRAPH_H
#define TEXTGEN_PARAGRAPH_H

#include "GlyphContainer.h"
#include <boost/shared_ptr.hpp>
#include <string>

namespace TextGen
{
class Paragraph : public GlyphContainer
{
 public:
  virtual ~Paragraph() {}
#ifdef NO_COMPILER_GENERATED
  Paragraph();
  Paragraph(const Paragraph& theParagraph);
  Paragraph& operator=(const Paragraph& theParagraph);
#endif

  virtual boost::shared_ptr<Glyph> clone() const;
  virtual std::string realize(const Dictionary& theDictionary) const;
  virtual std::string realize(const TextFormatter& theFormatter) const;
  virtual bool isDelimiter() const;

  Paragraph& operator<<(const Paragraph& theParagraph);
  Paragraph& operator<<(const Glyph& theGlyph);

};  // class Paragraph

}  // namespace TextGen

#endif  // TEXTGEN_PARAGRAPH_H

// ======================================================================
