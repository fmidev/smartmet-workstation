// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::SectionTag
 */
// ======================================================================

#ifndef TEXTGEN_SECTIONTAG_H
#define TEXTGEN_SECTIONTAG_H

#include "Glyph.h"
#include <string>

namespace TextGen
{
class Dictionary;
class TextFormatter;

class SectionTag : public Glyph
{
 public:
  virtual ~SectionTag();
  SectionTag(const std::string& theName, const bool& prefixTag = true);
#ifdef NO_COMPILER_GENERATED
  SectionTag(const SectionTag& theSectionTag);
  SectionTag& operator=(const SectionTag& theSectionTag);
#endif

  virtual boost::shared_ptr<Glyph> clone() const;
  virtual std::string realize(const Dictionary& theDictionary) const;
  virtual std::string realize(const TextFormatter& theFormatter) const;

  virtual bool isDelimiter() const;
  virtual bool isPrefixTag() const;

 private:
  SectionTag();
  std::string itsName;
  bool itsPrefixTag;

};  // class SectionTag
}  // namespace SectionTag

#endif  // TEXTGEN_SECTIONTAG_H

// ======================================================================
