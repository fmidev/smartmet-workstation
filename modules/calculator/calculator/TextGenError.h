// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::TextGenError
 */
// ======================================================================

#ifndef TEXTGEN_TEXTGENERROR_H
#define TEXTGEN_TEXTGENERROR_H

#include <stdexcept>
#include <string>

namespace TextGen
{
class TextGenError : public std::exception
{
 public:
  ~TextGenError() throw() {}
  TextGenError(const std::string& theError);
  virtual const char* what(void) const throw();

 private:
  std::string itsError;

};  // class TextGenError
}  // namespace TextGen

#endif  // TEXTGEN_TEXTGENERROR_H

// ======================================================================
