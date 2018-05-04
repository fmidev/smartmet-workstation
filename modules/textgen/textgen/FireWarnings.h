// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::FireWarnings
 */
// ----------------------------------------------------------------------

#ifndef TEXTGEN_FIREWARNINGS_H
#define TEXTGEN_FIREWARNINGS_H

#include <calculator/TextGenPosixTime.h>
#include <string>
#include <vector>

namespace TextGen
{
class FireWarnings
{
 public:
  enum State
  {
    None = 0,
    GrassFireWarning = 1,
    FireWarning = 2,
    Undefined = 999
  };

  FireWarnings(const std::string& theDirectory, const TextGenPosixTime& theTime);
  State state(int theArea) const;

 private:
  FireWarnings();
  const TextGenPosixTime itsTime;
  std::vector<State> itsWarnings;

};  // class FireWarnings
}  // namespace TextGen

#endif  // TEXTGEN_FIREWARNINGS_H

// ======================================================================
