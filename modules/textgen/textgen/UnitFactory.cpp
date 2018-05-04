// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace TextGen::UnitFactory
 */
// ======================================================================
/*!
 * \namespace TextGen::UnitFactory
 *
 * \brief Provides unit phrase creation services.
 *
 * The purpose of the factory is to decouple the formatting
 * of units from their creation. The desired formatting is
 * specified by a global Settings variable for each unit.
 *
 * Note that we return always on purpose a Sentence so that
 * we may append the phrase to a sentence.
 *
 * The variables controlling each unit are listed below.
 * Note that the default is always SI.
 *
 * <table>
 * <tr>
 * <th>Unit</th>
 * <th>Variable</th>
 * <th>Values</th>
 * </tr>
 * <tr>
 * <td>DegreesCelsius</td>
 * <td>textgen::units::celsius::format</td>
 * <td>phrase/SI/none</td>
 * </tr>
 * <tr>
 * <td>MetersPerSecond</td>
 * <td>textgen::units::meterspersecond::format</td>
 * <td>phrase/SI/none</td>
 * </tr>
 * <td>Millimeters</td>
 * <td>textgen::units::millimeters::format</td>
 * <td>phrase/SI/none</td>
 * </tr>
 * <tr>
 * <td>Percent</td>
 * <td>textgen::units::percent::format</td>
 * <td>phrase/SI/none</td>
 * </tr>
 * <tr>
 * <td>HectoPascal</td>
 * <td>textgen::units::hectopascal::format</td>
 * <td>phrase/SI/none</td>
 * </tr>
 * <tr>
 * <td>Meters</td>
 * <td>textgen::units::meters::format</td>
 * <td>phrase/SI/none</td>
 * </tr>
 * </table>
 */
// ======================================================================

#include "UnitFactory.h"
#include "Delimiter.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include "Integer.h"
#include <calculator/TextGenError.h>

#include <string>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;

namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Return the DegreesCelsius sentence
 *
 * \return The sentence
 */
// ----------------------------------------------------------------------

boost::shared_ptr<TextGen::Sentence> degrees_celsius()
{
  using namespace TextGen;

  const string var = "textgen::units::celsius::format";
  const string opt = Settings::optional_string(var, "SI");

  boost::shared_ptr<Sentence> sentence(new Sentence);

  if (opt == "SI")
    *sentence << Delimiter("\xc2\xb0" + string("C"));
  else if (opt == "phrase")
    *sentence << "astetta";
  else if (opt == "none")
    ;
  else
    throw TextGenError("Unknown format " + opt + " in variable " + var);

  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the DegreesCelsius sentence
 *
 * \param value The value
 * \return The sentence
 */
// ----------------------------------------------------------------------

boost::shared_ptr<TextGen::Sentence> degrees_celsius(int value, bool isInterval = false)
{
  using namespace TextGen;

  const string var = "textgen::units::celsius::format";
  const string opt = Settings::optional_string(var, "SI");

  boost::shared_ptr<Sentence> sentence(new Sentence);

  if (opt == "SI")
  {
    *sentence << Delimiter("\xc2\xb0" + string("C"));
  }
  else if (opt == "phrase")
  {
    string degrees_string("");

    if (abs(value) <= 4)
    {
      if (isInterval)
        degrees_string =
            string("astetta (n..." + boost::lexical_cast<std::string>(abs(value)) + ")");
      else
        degrees_string = string("astetta (" + boost::lexical_cast<std::string>(abs(value)) + ")");
    }
    else if (abs(value) % 10 == 1 && abs(value) != 11)
    {
      if (isInterval)
        degrees_string = "astetta (n...(mod 10=1))";
      else
        degrees_string = "astetta (mod 10=1)";
    }
    else
    {
      if (isInterval)
        degrees_string = "astetta (m...n)";
      else
        degrees_string = "astetta (n)";
    }

    *sentence << degrees_string;
  }
  else if (opt == "none")
    ;
  else
    throw TextGenError("Unknown format " + opt + " in variable " + var);

  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the MetersPerSecond sentence
 *
 * \return The sentence
 */
// ----------------------------------------------------------------------

boost::shared_ptr<TextGen::Sentence> meters_per_second()
{
  using namespace TextGen;

  const string var = "textgen::units::meterspersecond::format";
  const string opt = Settings::optional_string(var, "SI");

  boost::shared_ptr<Sentence> sentence(new Sentence);

  if (opt == "SI")
    *sentence << "m/s";
  else if (opt == "phrase" || opt == "textphrase")
    *sentence << "metria sekunnissa";
  else if (opt == "none")
    ;
  else
    throw TextGenError("Unknown format " + opt + " in variable " + var);

  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the MetersPerSecond sentence
 *
 * \param value The value
 * \return The sentence
 */
// ----------------------------------------------------------------------

boost::shared_ptr<TextGen::Sentence> meters_per_second(int value, bool withoutNumber = false)
{
  using namespace TextGen;

  const string var = "textgen::units::meterspersecond::format";
  const string opt = Settings::optional_string(var, "SI");

  boost::shared_ptr<Sentence> sentence(new Sentence);

  if (opt == "SI")
  {
    if (withoutNumber)
      *sentence << "m/s";
    else
      *sentence << TextGen::Integer(value) << "m/s";
  }
  else if (opt == "phrase")
  {
    if (withoutNumber)
    {
      if (value == 0)
        *sentence << "metria sekunnissa";
      else if (value == 1)
        *sentence << "metri sekunnissa";
      else if (abs(value) % 10 == 1 && abs(value) != 11)
        *sentence << "metria sekunnissa (mod 10=1)";
      else
        *sentence << "metria sekunnissa";
    }
    else
    {
      if (value == 0)
        *sentence << "0 metria sekunnissa";
      else if (value == 1)
        *sentence << "1 metri sekunnissa";
      else if (abs(value) % 10 == 1 && abs(value) != 11)
        *sentence << TextGen::Integer(value) << "metria sekunnissa (mod 10=1)";
      else
        *sentence << TextGen::Integer(value) << "metria sekunnissa";
    }
  }
  else if (opt == "textphrase")
  {
    if (value >= 1 && value <= 3)
    {
      *sentence << "heikkoa";
    }
    else if (value >= 4 && value <= 7)
    {
      *sentence << "kohtalaista";
    }
    else if (value >= 8 && value <= 13)
    {
      *sentence << "navakkaa";
    }
    else if (value >= 14 && value <= 20)
    {
      *sentence << "kovaa";
    }
    else if (value >= 21 && value <= 32)
    {
      *sentence << "myrskya";
    }
    else if (value > 32)
    {
      *sentence << "hirmumyrskya";
    }
  }
  else if (opt == "none")
    ;
  else
    throw TextGenError("Unknown format " + opt + " in variable " + var);

  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the Millimeters sentence
 *
 * \return The sentence
 */
// ----------------------------------------------------------------------

boost::shared_ptr<TextGen::Sentence> millimeters()
{
  using namespace TextGen;

  const string var = "textgen::units::millimeters::format";
  const string opt = Settings::optional_string(var, "SI");

  boost::shared_ptr<Sentence> sentence(new Sentence);

  if (opt == "SI")
    *sentence << Delimiter("mm");
  else if (opt == "phrase")
    *sentence << "millimetria";
  else if (opt == "none")
    ;
  else
    throw TextGenError("Unknown format " + opt + " in variable " + var);

  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the Millimeters sentence
 *
 * \param value The value
 * \return The sentence
 */
// ----------------------------------------------------------------------

boost::shared_ptr<TextGen::Sentence> millimeters(int value, bool withoutNumber = false)
{
  using namespace TextGen;

  const string var = "textgen::units::millimeters::format";
  const string opt = Settings::optional_string(var, "SI");

  boost::shared_ptr<Sentence> sentence(new Sentence);

  if (opt == "SI")
  {
    if (withoutNumber)
      *sentence << Delimiter("mm");
    else
      *sentence << TextGen::Integer(value) << Delimiter("mm");
  }
  else if (opt == "phrase")
  {
    if (withoutNumber)
    {
      if (value == 0)
        *sentence << "millimetria";
      else if (value == 1)
        *sentence << "millimetri";
      else if (abs(value) % 10 == 1 && abs(value) != 11)
        *sentence << "millimetria (mod 10=1)";
      else
        *sentence << "millimetria";
    }
    else
    {
      if (value == 0)
        *sentence << "0 millimetria";
      else if (value == 1)
        *sentence << "1 millimetri";
      else if (abs(value) % 10 == 1 && abs(value) != 11)
        *sentence << TextGen::Integer(value) << "millimetria (mod 10=1)";
      else
        *sentence << TextGen::Integer(value) << "millimetria";
    }
  }
  else if (opt == "none")
    ;
  else
    throw TextGenError("Unknown format " + opt + " in variable " + var);

  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the Meters sentence
 *
 * \return The sentence
 */
// ----------------------------------------------------------------------

boost::shared_ptr<TextGen::Sentence> meters()
{
  using namespace TextGen;

  const string var = "textgen::units::meters::format";
  const string opt = Settings::optional_string(var, "SI");

  boost::shared_ptr<Sentence> sentence(new Sentence);

  if (opt == "SI")
    *sentence << Delimiter("m");
  else if (opt == "phrase")
    *sentence << "metria";
  else if (opt == "none")
    ;
  else
    throw TextGenError("Unknown format " + opt + " in variable " + var);

  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the Meters sentence
 *
 * \param value The value
 * \return The sentence
 */
// ----------------------------------------------------------------------

boost::shared_ptr<TextGen::Sentence> meters(int value, bool withoutNumber = false)
{
  using namespace TextGen;

  const string var = "textgen::units::meters::format";
  const string opt = Settings::optional_string(var, "SI");

  boost::shared_ptr<Sentence> sentence(new Sentence);

  if (opt == "SI")
  {
    if (withoutNumber)
      *sentence << Delimiter("m");
    else
      *sentence << TextGen::Integer(value) << Delimiter("m");
  }
  else if (opt == "phrase")
  {
    if (withoutNumber)
    {
      if (value == 0)
        *sentence << "metria";
      else if (value == 1)
        *sentence << "metri";
      else if (abs(value) % 10 == 1 && abs(value) != 11)
        *sentence << "metri (mod 10=1)";
      else
        *sentence << "metria";
    }
    else
    {
      if (value == 0)
        *sentence << "0 metria";
      else if (value == 1)
        *sentence << "1 metri";
      else if (abs(value) % 10 == 1 && abs(value) != 11)
        *sentence << TextGen::Integer(value) << "metria (mod 10=1)";
      else
        *sentence << TextGen::Integer(value) << "metria";
    }
  }
  else if (opt == "none")
    ;
  else
    throw TextGenError("Unknown format " + opt + " in variable " + var);

  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the Percent sentence
 *
 * \return The sentence
 */
// ----------------------------------------------------------------------

boost::shared_ptr<TextGen::Sentence> percent()
{
  using namespace TextGen;

  const string var = "textgen::units::percent::format";
  const string opt = Settings::optional_string(var, "SI");

  boost::shared_ptr<Sentence> sentence(new Sentence);

  if (opt == "SI")
    *sentence << Delimiter("%");
  else if (opt == "phrase")
    *sentence << "prosenttia";
  else if (opt == "none")
    ;
  else
    throw TextGenError("Unknown format " + opt + " in variable " + var);
  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the Percent sentence
 *
 * \param value The value
 * \return The sentence
 */
// ----------------------------------------------------------------------

boost::shared_ptr<TextGen::Sentence> percent(int value, bool withoutNumber = false)
{
  using namespace TextGen;

  const string var = "textgen::units::percent::format";
  const string opt = Settings::optional_string(var, "SI");

  boost::shared_ptr<Sentence> sentence(new Sentence);

  if (opt == "SI")
  {
    if (withoutNumber)
      *sentence << Delimiter("%");
    else
      *sentence << TextGen::Integer(value) << Delimiter("%");
  }
  else if (opt == "phrase")
  {
    if (withoutNumber)
    {
      if (value == 0)
        *sentence << "prosenttia";
      else if (value == 1)
        *sentence << "prosentti";
      else if (abs(value) % 10 == 1 && abs(value) != 11)
        *sentence << "prosenttia (mod 10=1)";
      else
        *sentence << "prosenttia";
    }
    else
    {
      if (value == 0)
        *sentence << "0 prosenttia";
      else if (value == 1)
        *sentence << "1 prosentti";
      else if (abs(value) % 10 == 1 && abs(value) != 11)
        *sentence << TextGen::Integer(value) << "prosenttia (mod 10=1)";
      else
        *sentence << TextGen::Integer(value) << "prosenttia";
    }
  }
  else if (opt == "none")
    ;
  else
    throw TextGenError("Unknown format " + opt + " in variable " + var);
  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the HectoPascal sentence
 *
 * \return The sentence
 */
// ----------------------------------------------------------------------

boost::shared_ptr<TextGen::Sentence> hectopascal()
{
  using namespace TextGen;

  const string var = "textgen::units::hectopascal::format";
  const string opt = Settings::optional_string(var, "SI");

  boost::shared_ptr<Sentence> sentence(new Sentence);

  if (opt == "SI")
    *sentence << Delimiter("hPa");
  else if (opt == "phrase")
    *sentence << "hehtopascalia";
  else if (opt == "none")
    ;
  else
    throw TextGenError("Unknown format " + opt + " in variable " + var);
  return sentence;
}

// ----------------------------------------------------------------------
/*!§
 * \brief Return the HectoPascal sentence
 *
 * \param value The value
 * \return The sentence
 */
// ----------------------------------------------------------------------

boost::shared_ptr<TextGen::Sentence> hectopascal(int value, bool withoutNumber = false)
{
  using namespace TextGen;

  const string var = "textgen::units::hectopascal::format";
  const string opt = Settings::optional_string(var, "SI");

  boost::shared_ptr<Sentence> sentence(new Sentence);

  if (opt == "SI")
  {
    if (withoutNumber)
      *sentence << Delimiter("hPa");
    else
      *sentence << TextGen::Integer(value) << Delimiter("hPa");
  }
  else if (opt == "phrase")
  {
    if (withoutNumber)
    {
      if (value == 0)
        *sentence << "hehtopascalia";
      else if (value == 1)
        *sentence << "hehtopascal";
      else if (abs(value) % 10 == 1 && abs(value) != 11)
        *sentence << "hehtopascalia (mod 10=1)";
      else
        *sentence << "hehtopascalia";
    }
    else
    {
      if (value == 0)
        *sentence << "0 hehtopascalia";
      else if (value == 1)
        *sentence << "1 hehtopascal";
      else if (abs(value) % 10 == 1 && abs(value) != 11)
        *sentence << TextGen::Integer(value) << "hehtopascalia (mod 10=1)";
      else
        *sentence << TextGen::Integer(value) << "hehtopascalia";
    }
  }
  else if (opt == "none")
    ;
  else
    throw TextGenError("Unknown format " + opt + " in variable " + var);
  return sentence;
}
}

namespace TextGen
{
namespace UnitFactory
{
// ----------------------------------------------------------------------
/*!
 * \brief Return the formatted sentence for the given unit
 *
 * \param theUnit The desired unit
 * \param value The value
 * \return The sentence
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Sentence> create(Units theUnit)
{
  switch (theUnit)
  {
    case DegreesCelsius:
      return degrees_celsius();
    case MetersPerSecond:
      return meters_per_second();
    case Millimeters:
      return millimeters();
    case Percent:
      return percent();
    case HectoPascal:
      return hectopascal();
    case Meters:
      return meters();
  }

  throw TextGenError("UnitFactory::create failed - unknown unit");
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the formatted sentence for the given unit and value
 *
 * \param theUnit The desired unit
 * \return The sentence
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Sentence> create(Units theUnit, int value)
{
  switch (theUnit)
  {
    case DegreesCelsius:
      return degrees_celsius(value);
    case MetersPerSecond:
      return meters_per_second(value);
    case Millimeters:
      return millimeters(value);
    case Percent:
      return percent(value);
    case HectoPascal:
      return hectopascal(value);
    case Meters:
      return meters(value);
  }

  throw TextGenError("UnitFactory::create failed - unknown unit");
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the formatted sentence for the given unit and value
 *
 * \param theUnit The desired unit
 * \return The sentence
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Sentence> create_unit(Units theUnit, int value, bool isInterval /* = false*/)
{
  switch (theUnit)
  {
    case DegreesCelsius:
      return degrees_celsius(value, isInterval);
    case MetersPerSecond:
      return meters_per_second(value, true);
    case Millimeters:
      return millimeters(value, true);
    case Percent:
      return percent(value, true);
    case HectoPascal:
      return hectopascal(value, true);
    case Meters:
      return meters(value, true);
  }

  throw TextGenError("UnitFactory::create failed - unknown unit");
}

}  // namespace UnitFactory
}  // namespace TextGen

// ======================================================================
