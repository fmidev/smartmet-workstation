// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::PlainTextFormatter
 */
// ======================================================================

#ifndef TEXTGEN_PLAINTEXTFORMATTER_H
#define TEXTGEN_PLAINTEXTFORMATTER_H

#include "TextFormatter.h"

namespace TextGen
{
class PlainTextFormatter : public TextFormatter
{
 public:
  PlainTextFormatter() {}
  virtual ~PlainTextFormatter() {}
  virtual void dictionary(const boost::shared_ptr<Dictionary>& theDict);

  virtual std::string format(const Glyph& theGlyph) const;

  // override for all composites
  virtual std::string visit(const Glyph& theGlyph) const;
  virtual std::string visit(const Integer& theInteger) const;
  virtual std::string visit(const Real& theReal) const;
  virtual std::string visit(const IntegerRange& theRange) const;
  virtual std::string visit(const Sentence& theSentence) const;
  virtual std::string visit(const Paragraph& theParagraph) const;
  virtual std::string visit(const Header& theHeader) const;
  virtual std::string visit(const Document& theDocument) const;
  virtual std::string visit(const WeatherTime& theTime) const;
  virtual std::string visit(const TimePeriod& theTimePeriod) const;

  virtual std::string visit(const SectionTag& theSectionTag) const;
  virtual std::string visit(const StoryTag& theStoryTag) const;

  std::string name() const { return "plain"; }
 private:
  boost::shared_ptr<Dictionary> itsDictionary;
  mutable std::string itsSectionVar;
  mutable std::string itsStoryVar;

};  // class PlainTextFormatter
}  // namespace TextGen

#endif  // TEXTGEN_PLAINTEXTFORMATTER_H

// ======================================================================
