// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::BasicDictionary
 */
// ======================================================================

#ifndef TEXTGEN_BASICDICTIONARY_H
#define TEXTGEN_BASICDICTIONARY_H

#include "Dictionary.h"

#include <map>
#include <string>

namespace TextGen
{
class BasicDictionary : public Dictionary
{
 public:
  typedef Dictionary::size_type size_type;

  // Compiler generated:
  //
  // BasicDictionary();
  // BasicDictionary(const BasicDictionary & theDict);
  // BasicDictionary & operator=(const BasicDictionary & theDict);

  virtual ~BasicDictionary();
  virtual void init(const std::string& theLanguage);
  virtual const std::string& language(void) const;
  virtual bool contains(const std::string& theKey) const;
  virtual const std::string& find(const std::string& theKey) const;
  virtual void insert(const std::string& theKey, const std::string& thePhrase);

  virtual size_type size(void) const;
  virtual bool empty(void) const;

 private:
  typedef std::map<std::string, std::string> StorageType;
  StorageType itsData;
  std::string itsLanguage;

};  // class BasicDictionary

}  // namespace TextGen

#endif  // TEXTGEN_BASICDICTIONARY_H

// ======================================================================
