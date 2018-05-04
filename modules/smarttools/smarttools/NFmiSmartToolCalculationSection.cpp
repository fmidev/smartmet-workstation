// ======================================================================
/*!
 * C++ Class Name : NFmiSmartToolCalculationSection
 * ---------------------------------------------------------
 *  Author         : pietarin
 *  Creation Date  : 8.11. 2010
 */
// ======================================================================

#ifdef _MSC_VER
#pragma warning(disable : 4786)  // poistaa n kpl VC++ kääntäjän varoitusta
#endif

#include "NFmiSmartToolCalculationSection.h"
#include "NFmiAreaMaskInfo.h"
#include "NFmiSmartToolCalculation.h"
#include <algorithm>

//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------
NFmiSmartToolCalculationSection::NFmiSmartToolCalculationSection(void) : itsCalculations()
{
}

NFmiSmartToolCalculationSection::NFmiSmartToolCalculationSection(
    const NFmiSmartToolCalculationSection &theOther)
    : itsCalculations(NFmiSmartToolCalculation::DoShallowCopy(theOther.itsCalculations))
{
}

NFmiSmartToolCalculationSection::~NFmiSmartToolCalculationSection(void)
{
}
//--------------------------------------------------------
// Calculate
//--------------------------------------------------------
void NFmiSmartToolCalculationSection::Calculate(const NFmiCalculationParams &theCalculationParams,
                                                NFmiMacroParamValue &theMacroParamValue)
{
  for (size_t i = 0; i < itsCalculations.size(); i++)
    itsCalculations[i]->Calculate(theCalculationParams, theMacroParamValue);
}

void NFmiSmartToolCalculationSection::Calculate_ver2(
    const NFmiCalculationParams &theCalculationParams)
{
  for (size_t i = 0; i < itsCalculations.size(); i++)
    itsCalculations[i]->Calculate_ver2(theCalculationParams);
}

void NFmiSmartToolCalculationSection::AddCalculations(
    const boost::shared_ptr<NFmiSmartToolCalculation> &value)
{
  if (value)
    itsCalculations.push_back(value);
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiSmartToolCalculationSection::FirstVariableInfo(void)
{
  boost::shared_ptr<NFmiFastQueryInfo> info;
  if (itsCalculations.size())
    info = itsCalculations[0]->GetResultInfo();
  return info;
}

// globaali asetus luokka for_each-funktioon
struct TimeSetter
{
  TimeSetter(const NFmiMetTime &theTime) : itsTime(theTime) {}
  void operator()(boost::shared_ptr<NFmiSmartToolCalculation> &theCalculation)
  {
    theCalculation->Time(itsTime);
  }

  const NFmiMetTime &itsTime;
};

void NFmiSmartToolCalculationSection::SetTime(const NFmiMetTime &theTime)
{
  std::for_each(itsCalculations.begin(), itsCalculations.end(), TimeSetter(theTime));
}
