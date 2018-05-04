// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::NullDictionary
 */
// ======================================================================

#ifndef TEXTGEN_NULLDICTIONARY_H
#define TEXTGEN_NULLDICTIONARY_H

#include "Dictionary.h"

#include <string>

namespace TextGen
{
class NullDictionary : public Dictionary
{
 public:
  typedef Dictionary::size_type size_type;

  // Compiler generated:
  //
  // NullDictionary();
  // NullDictionary(const NullDictionary & theDict);
  // NullDictionary & operator=(const NullDictionary & theDict);

  virtual ~NullDictionary() {}
  virtual void init(const std::string& theLanguage) { itsLanguage = theLanguage; }
  virtual const std::string& language(void) const { return itsLanguage; }
  virtual bool contains(const std::string& theKey) const { return false; }
  virtual const std::string& find(const std::string& theKey) const;
  virtual void insert(const std::string& theKey, const std::string& thePhrase);

  virtual size_type size(void) const { return 0; }
  virtual bool empty(void) const { return true; }
 private:
  std::string itsLanguage;

};  // class NullDictionary

}  // namespace TextGen

#endif  // TEXTGEN_NULLDICTIONARY_H

// ======================================================================
