// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::GlyphContainer
 */
// ======================================================================

#ifndef TEXTGEN_GLYPHCONTAINER_H
#define TEXTGEN_GLYPHCONTAINER_H

#include "Glyph.h"
#include <list>

namespace TextGen
{
class GlyphContainer : public Glyph
{
 public:
  virtual ~GlyphContainer() {}
  GlyphContainer() {}
  virtual boost::shared_ptr<Glyph> clone() const = 0;
  virtual std::string realize(const Dictionary& theDictionary) const = 0;
  virtual std::string realize(const TextFormatter& theFormatter) const = 0;
  virtual bool isDelimiter() const = 0;

  typedef boost::shared_ptr<Glyph> value_type;
  typedef const value_type& const_reference;
  typedef value_type& reference;
  typedef std::list<value_type> storage_type;
  typedef storage_type::size_type size_type;
  typedef storage_type::difference_type difference_type;
  typedef storage_type::const_iterator const_iterator;
  typedef storage_type::iterator iterator;

  size_type size() const;
  bool empty() const;
  void clear();

  void push_back(const Glyph& theGlyph);
  void push_back(const_reference theGlyph);
  void push_front(const Glyph& theGlyph);
  void push_front(const_reference theGlyph);

  const_iterator begin() const;
  const_iterator end() const;

  iterator begin();
  iterator end();

  const_reference front() const;
  const_reference back() const;

 protected:
  storage_type itsData;

};  // class GlyphContainer
}  // namespace TextGen

#endif  // TEXTGEN_GLYPHCONTAINER_H

// ======================================================================
