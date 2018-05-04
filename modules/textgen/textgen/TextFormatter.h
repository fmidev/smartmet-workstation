// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::TextFormatter
 */
// ======================================================================

#ifndef TEXTGEN_TEXTFORMATTER_H
#define TEXTGEN_TEXTFORMATTER_H

#include <boost/shared_ptr.hpp>
#include <string>

namespace TextGen
{
class Dictionary;
class Document;
class Glyph;
class GlyphContainer;
class Header;
class Integer;
class Real;
class IntegerRange;
class Paragraph;
class Sentence;
class TimePeriod;
class WeatherTime;

class SectionTag;
class StoryTag;

class TextFormatter
{
 public:
  virtual ~TextFormatter() {}
#ifdef NO_COMPILER_GENERATED
  TextFormatter() {}
#endif

  virtual void dictionary(const boost::shared_ptr<Dictionary>& theDict) = 0;

  virtual std::string format(const Glyph& theGlyph) const = 0;

  // override for all composites
  virtual std::string visit(const Glyph& theGlyph) const = 0;
  virtual std::string visit(const Integer& theInteger) const = 0;
  virtual std::string visit(const Real& theReal) const = 0;
  virtual std::string visit(const IntegerRange& theRange) const = 0;
  virtual std::string visit(const Sentence& theSentence) const = 0;
  virtual std::string visit(const Paragraph& theParagraph) const = 0;
  virtual std::string visit(const Header& theHeader) const = 0;
  virtual std::string visit(const Document& theDocument) const = 0;
  virtual std::string visit(const WeatherTime& theTime) const = 0;
  virtual std::string visit(const TimePeriod& thePeriod) const = 0;

  virtual std::string visit(const SectionTag& theSectionTag) const = 0;
  virtual std::string visit(const StoryTag& theStoryTag) const = 0;

  void setProductName(const std::string& theProductName) { itsProductName = theProductName; }
 protected:
  std::string itsProductName;

};  // class TextFormatter
}  // namespace TextGen

#endif  // TEXTGEN_TEXTFORMATTER_H

// ======================================================================
