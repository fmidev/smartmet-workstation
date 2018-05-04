// ======================================================================
/*!
 * \file
 * \brief Interface of abstract class TextGen::Dictionary
 */
// ======================================================================

#ifndef TEXTGEN_DICTIONARY_H
#define TEXTGEN_DICTIONARY_H

#include <calculator/TextGenError.h>
#include <map>
#include <string>
#include <iostream>
#include <sstream>

namespace TextGen
{
class Dictionary
{
 public:
  typedef unsigned long size_type;

  // Compiler generated:
  //
  // Dictionary();
  // Dictionary(const Dictionary & theDict);
  // Dictionary & operator=(const Dictionary & theDict);

  virtual ~Dictionary() {}
  virtual void init(const std::string& theLanguage) = 0;
  virtual const std::string& language(void) const = 0;
  virtual bool contains(const std::string& theKey) const = 0;
  virtual const std::string& find(const std::string& theKey) const = 0;
  virtual void insert(const std::string& theKey, const std::string& thePhrase) = 0;

  virtual void geoinit(void* theReactor) {}
  virtual bool geocontains(const std::string& theKey) const { return false; }
  virtual bool geocontains(const double& theLongitude,
                           const double& theLatitude,
                           const double& theMaxDistance) const
  {
    return false;
  }
  virtual const std::string& geofind(const std::string& theKey) const
  {
    throw TextGenError("Error: Dictionary::geofind(" + theKey + ") failed");
  }
  virtual const std::string& geofind(const double& theLongitude,
                                     const double& theLatitude,
                                     const double& theMaxDistance) const
  {
    std::stringstream ss;
    ss << "Error: Dictionary::geofind(" << theLongitude << ", " << theLatitude << ", "
       << theMaxDistance << ") failed";

    throw TextGenError(ss.str());
  }

  virtual size_type size(void) const = 0;
  virtual bool empty(void) const = 0;

};  // class Dictionary

}  // namespace TextGen

#endif  // TEXTGEN_DICTIONARY_H

// ======================================================================
