// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::FileDictionary
 */
// ======================================================================

#ifndef TEXTGEN_FILEDICTIONARY_H
#define TEXTGEN_FILEDICTIONARY_H

#include "Dictionary.h"

#include <boost/shared_ptr.hpp>
#include <string>

namespace TextGen
{
class FileDictionary : public Dictionary
{
 public:
  typedef Dictionary::size_type size_type;

  virtual ~FileDictionary();
  FileDictionary();
#ifdef NO_COMPILER_OPTIMIZE
  FileDictionary(const FileDictionary& theDict);
  FileDictionary& operator=(const FileDictionary& theDict);
#endif

  virtual void init(const std::string& theLanguage);
  virtual const std::string& language(void) const;
  virtual bool contains(const std::string& theKey) const;
  virtual const std::string& find(const std::string& theKey) const;
  virtual void insert(const std::string& theKey, const std::string& thePhrase);

  virtual size_type size(void) const;
  virtual bool empty(void) const;

 private:
  class Pimple;
  boost::shared_ptr<Pimple> itsPimple;

};  // class FileDictionary

}  // namespace TextGen

#endif  // TEXTGEN_FILEDICTIONARY_H

// ======================================================================
