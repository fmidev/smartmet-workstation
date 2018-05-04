// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::MySQLDictionary
 */
// ======================================================================

#ifndef TEXTGEN_MYSQLDICTIONARY_H
#define TEXTGEN_MYSQLDICTIONARY_H
#ifdef UNIX

#include "Dictionary.h"
#include <boost/shared_ptr.hpp>
#include <string>

namespace TextGen
{
class MySQLDictionary : public Dictionary
{
 public:
  typedef Dictionary::size_type size_type;

  virtual ~MySQLDictionary();
  MySQLDictionary();
#ifdef NO_COMPILER_OPTIMIZE
  MySQLDictionary(const MySQLDictionary& theDict);
  MySQLDictionary& operator=(const MySQLDictionary& theDict);
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

};  // class MySQLDictionary

}  // namespace TextGen
#endif  // UNIX
#endif  // TEXTGEN_MYSQLDICTIONARY_H

// ======================================================================
