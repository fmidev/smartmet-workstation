// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::SpecialStory::text
 */
// ======================================================================

#include "SpecialStory.h"
#include "MessageLogger.h"
#include "Paragraph.h"
#include <calculator/Settings.h>
#include "Text.h"

#include <boost/filesystem.hpp>

#include <cstdio>
#include <fstream>
#include <sstream>

#ifdef UNIX
#include <unistd.h>
#else
#define R_OK    4
#include <io.h>
#endif

using namespace TextGen;
using namespace std;
using namespace boost;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Read file contents
 */
// ----------------------------------------------------------------------

string read_file(const string& filename)
{
  stringstream ss;
  ss << ifstream(filename.c_str(), ios::binary).rdbuf();
  return ss.str();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if a file is executable
 */
// ----------------------------------------------------------------------

bool is_executable(const string& filename) 
{
#ifdef UNIX
    return !access(filename.c_str(), X_OK);
#else
    return !access(filename.c_str(), R_OK);
#endif
}
// ----------------------------------------------------------------------
/*!
 * \brief Execute command and return stdout
 *
 * Note: To catch stderr too append 2>&1 to the command
 */
// ----------------------------------------------------------------------

string execute(const string& cmd)
{
#ifdef UNIX
    FILE* pipe = popen(cmd.c_str(), "r");
#else
    FILE* pipe = _popen(cmd.c_str(), "r");
#endif
  if (!pipe) throw runtime_error("Could not execute command '" + cmd + "'");

  char buffer[128];
  std::string result = "";
  while (!feof(pipe))
  {
    if (fgets(buffer, 128, pipe) != NULL) result += buffer;
  }
#ifdef UNIX
  pclose(pipe);
#else
  _pclose(pipe);
#endif
  return result;
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate story on text
 *
 * Variables:
 *
 *     value = "Text to be inserted into paragraph as is."
 *     value = @filename
 *     value = @filename.php
 *
 * Product specific variables are possible:
 *
 *     value::en_html = "<underline>Text to be underlined.</underline>"
 *
 * \return The story
 */
// ----------------------------------------------------------------------

Paragraph SpecialStory::text() const
{
  MessageLogger log("SpecialStory::text");

  // Get the options

  using namespace Settings;

  const string default_text = Settings::optional_string(itsVar + "::value", "");

  Paragraph paragraph;

  if (default_text.empty())
  {
  }
  else if (default_text[0] != '@')
  {
    // text is set in formatter, since you must be able to give format-specific text
    //	paragraph << Text(default_text);
  }
  else
  {
    string filename = default_text.substr(1, string::npos);
    log << "File to be included: " << filename << endl;
    if (!boost::filesystem::exists(filename))
    {
      log << "The file does not exist!" << endl;
      throw runtime_error("File '" + filename + "' is not readable");
    }

    // Execute and catch stdout if the file is executable

    if (is_executable(filename))
    {
      string txt = execute(filename);
      paragraph << Text(txt);
    }
    else
    {
      string txt = read_file(filename);
      paragraph << Text(txt);
    }
  }

  return paragraph;
}

}  // namespace TextGen

// ======================================================================
