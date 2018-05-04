// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::MySQLDictionaries
 */
// ======================================================================

#ifndef TEXTGEN_MYSQLDICTIONARIES_H
#define TEXTGEN_MYSQLDICTIONARIES_H
#ifdef UNIX

#include "Dictionary.h"

#include <boost/shared_ptr.hpp>
#include <string>

namespace TextGen
{
class MySQLDictionaries : public Dictionary
{
 public:
  typedef Dictionary::size_type size_type;

  virtual ~MySQLDictionaries();
  MySQLDictionaries();
#ifdef NO_COMPILER_OPTIMIZE
  MySQLDictionaries(const MySQLDictionaries& theDict);
  MySQLDictionaries& operator=(const MySQLDictionaries& theDict);
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

};  // class MySQLDictionaries

}  // namespace TextGen

#endif // UNIX
#endif  // TEXTGEN_MYSQLDICTIONARIES_H

// ======================================================================
