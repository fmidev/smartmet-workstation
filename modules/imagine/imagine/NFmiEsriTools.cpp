#include "NFmiEsriTools.h"
#include "NFmiEsriShape.h"
#include <newbase/NFmiStringTools.h>
#include <list>
#include <stdexcept>

using namespace std;

namespace Imagine
{
namespace NFmiEsriTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Filter a shape based on a condition of form NAME<op>VALUE
 */
// ----------------------------------------------------------------------

NFmiEsriShape* filter(const NFmiEsriShape& theShape, const std::string& theCondition)
{
  // Parse the relevant option

  list<string> comparisons;
  comparisons.push_back("==");
  comparisons.push_back("<=");
  comparisons.push_back(">=");
  comparisons.push_back("<>");
  comparisons.push_back("<");
  comparisons.push_back(">");
  comparisons.push_back("=");

  string fieldname, fieldvalue, fieldcomparison;
  list<string> words;

  for (list<string>::const_iterator it = comparisons.begin(); it != comparisons.end(); ++it)
  {
    string::size_type pos = theCondition.find(*it);
    if (pos != string::npos)
    {
      fieldcomparison = *it;
      fieldname = theCondition.substr(0, pos);
      fieldvalue = theCondition.substr(pos + it->size());
      break;
    }
  }

  if (fieldcomparison.empty()) throw runtime_error("Unable to parse comparison option");

  // Fetch the attribute name

  const NFmiEsriAttributeName* name = theShape.AttributeName(fieldname);
  if (name == 0) throw runtime_error("The shape does not have a field named '" + fieldname + "'");
  // Preparse the desired field value

  const NFmiEsriAttributeType atype = name->Type();

  string svalue = "";
  int ivalue = 0;
  double dvalue = 0;

  switch (atype)
  {
    case kFmiEsriString:
      svalue = fieldvalue;
      break;
    case kFmiEsriInteger:
      ivalue = NFmiStringTools::Convert<int>(fieldvalue);
      break;
    case kFmiEsriDouble:
      dvalue = NFmiStringTools::Convert<double>(fieldvalue);
      break;
    default:
      throw runtime_error("The field '" + fieldname + "' is of unknown type");
  }

  NFmiEsriShape* shape = new NFmiEsriShape(theShape.Type());

  for (NFmiEsriShape::attributes_type::const_iterator ait = theShape.Attributes().begin();
       ait != theShape.Attributes().end();
       ++ait)
  {
    shape->Add(new NFmiEsriAttributeName(**ait));
  }

  for (NFmiEsriShape::const_iterator it = theShape.Elements().begin();
       it != theShape.Elements().end();
       ++it)
  {
    // Skip empty elements
    if (*it == 0) continue;

    // Check if the value is correct

    bool ok = false;
    if (fieldcomparison == "==" || fieldcomparison == "=")
    {
      if (atype == kFmiEsriString)
        ok = ((*it)->GetString(fieldname) == svalue);
      else if (atype == kFmiEsriInteger)
        ok = ((*it)->GetInteger(fieldname) == ivalue);
      else
        ok = ((*it)->GetDouble(fieldname) == dvalue);
    }
    else if (fieldcomparison == "<>")
    {
      if (atype == kFmiEsriString)
        ok = ((*it)->GetString(fieldname) != svalue);
      else if (atype == kFmiEsriInteger)
        ok = ((*it)->GetInteger(fieldname) != ivalue);
      else
        ok = ((*it)->GetDouble(fieldname) != dvalue);
    }
    else if (fieldcomparison == "<")
    {
      if (atype == kFmiEsriString)
        ok = ((*it)->GetString(fieldname) < svalue);
      else if (atype == kFmiEsriInteger)
        ok = ((*it)->GetInteger(fieldname) < ivalue);
      else
        ok = ((*it)->GetDouble(fieldname) < dvalue);
    }
    else if (fieldcomparison == ">")
    {
      if (atype == kFmiEsriString)
        ok = ((*it)->GetString(fieldname) > svalue);
      else if (atype == kFmiEsriInteger)
        ok = ((*it)->GetInteger(fieldname) > ivalue);
      else
        ok = ((*it)->GetDouble(fieldname) > dvalue);
    }
    else if (fieldcomparison == "<=")
    {
      if (atype == kFmiEsriString)
        ok = ((*it)->GetString(fieldname) <= svalue);
      else if (atype == kFmiEsriInteger)
        ok = ((*it)->GetInteger(fieldname) <= ivalue);
      else
        ok = ((*it)->GetDouble(fieldname) <= dvalue);
    }
    else if (fieldcomparison == ">=")
    {
      if (atype == kFmiEsriString)
        ok = ((*it)->GetString(fieldname) >= svalue);
      else if (atype == kFmiEsriInteger)
        ok = ((*it)->GetInteger(fieldname) >= ivalue);
      else
        ok = ((*it)->GetDouble(fieldname) >= dvalue);
    }

    if (!ok) continue;

    NFmiEsriElement* tmp = (*it)->Clone();
    shape->Add(tmp);
  }

  return shape;
}

}  // namespace NFmiEsriTools
}  // namespace Imagine
