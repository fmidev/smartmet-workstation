// ======================================================================
/*!
 * \file
 * \brief Implementation of class MessageLogger
 */
// ======================================================================
/*!
 * \class MessageLogger
 *
 * \brief Generic debugging message logger
 *
 * Sample use in a main program:
 * \code
 * int main()
 * {
 *   MessageLogger::open("my.log");
 *   MessageLogger log("main");
 *   log << "Starting the work" << std::endl;
 *   ...
 *
 * }
 * \endcode
 *
 * Sample use in a function:
 * \code
 * void myfunction()
 * {
 *   MessageLogger log("myfunction()");
 *   log << "calculating some result " << 10 << std::endl;
 * }
 * \endcode

 */
// ======================================================================

#include "MessageLogger.h"

#include "DebugTextFormatter.h"
#include <newbase/NFmiTime.h>

#include <fstream>
#include <iomanip>
#include <ctime>

using namespace std;

unsigned long MessageLogger::itsDepth = 0;
ostream* MessageLogger::itsOutput = 0;
char MessageLogger::itsIndentChar = ' ';
unsigned int MessageLogger::itsIndentStep = 2;
bool MessageLogger::itsTimeStampOn = false;

namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Output timestamp if flag is true
 */
// ----------------------------------------------------------------------

void output_timestamp(bool theFlag, ostream* theOutput)
{
  if (theOutput != 0 && theFlag)
  {
    NFmiTime now;
    *theOutput << now.ToStr(kYYYYMMDDHHMMSS).CharPtr() << ' ';
  }
}
}

// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

MessageLogger::~MessageLogger()
{
  --itsDepth;
  output_timestamp(itsTimeStampOn, itsOutput);
  if (itsOutput != 0)
    *itsOutput << string(itsIndentStep * itsDepth, itsIndentChar) << "[Leaving " << itsFunction
               << ']' << endl;
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * \param theFunction The function name
 */
// ----------------------------------------------------------------------

MessageLogger::MessageLogger(const string& theFunction) : itsFunction(theFunction)
{
  output_timestamp(itsTimeStampOn, itsOutput);

  if (itsOutput != 0)
    *itsOutput << string(itsIndentStep * itsDepth, itsIndentChar) << "[Entering " << itsFunction
               << ']' << endl;
  ++itsDepth;
}

// ----------------------------------------------------------------------
/*!
 * \brief Write a new message when flush occurs
 *
 * \param theMessage The message to write
 */
// ----------------------------------------------------------------------

void MessageLogger::onNewMessage(const string_type& theMessage)
{
  output_timestamp(itsTimeStampOn, itsOutput);

  if (itsOutput != 0) *itsOutput << string(itsIndentStep * itsDepth, itsIndentChar) << theMessage;
}

// ----------------------------------------------------------------------
/*!
 * \brief Open a messagelog output stream
 *
 * \param theFilename The filename for the log
 */
// ----------------------------------------------------------------------

void MessageLogger::open(const string& theFilename)
{
  delete itsOutput;
  itsOutput = 0;

  if (theFilename.empty()) return;

  itsOutput = new ofstream(theFilename.c_str(), ios::out);
  if (itsOutput == 0)
    throw std::runtime_error("MessageLogger could not allocate a new output stream");
  if (!(*itsOutput))
    throw std::runtime_error("MessageLogger failed to open '" + theFilename + "' for writing");
}

// ----------------------------------------------------------------------
/*!
 * \brief Convenience operator for textgen library
 *
 * This really should not be here, it couples the generic MessageLogger
 * class to the DebugTextFormatter and Glyph classes. However, any
 * other solution seems inconvenient for the user.
 *
 */
// ----------------------------------------------------------------------

MessageLogger& MessageLogger::operator<<(const TextGen::Glyph& theGlyph)
{
  static TextGen::DebugTextFormatter formatter;
  if (itsOutput != 0) *this << "Return: " << formatter.format(theGlyph) << endl;
  return *this;
}

// ======================================================================
