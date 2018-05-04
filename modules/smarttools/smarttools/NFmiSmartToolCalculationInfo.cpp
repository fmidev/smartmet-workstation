//**********************************************************
// C++ Class Name : NFmiSmartToolCalculationInfo
// ---------------------------------------------------------
//  Author         : pietarin
//  Creation Date  : 8.11. 2010
//**********************************************************
#ifdef _MSC_VER
#pragma warning(disable : 4786)  // poistaa n kpl VC++ kääntäjän varoitusta
#endif

#include "NFmiSmartToolCalculationInfo.h"
#include "NFmiAreaMaskInfo.h"
//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------
NFmiSmartToolCalculationInfo::NFmiSmartToolCalculationInfo(void)
    : itsResultDataInfo(), itsCalculationOperandInfoVector(), fAllowMissingValueAssignment(false)
{
}

NFmiSmartToolCalculationInfo::~NFmiSmartToolCalculationInfo(void)
{
}

void NFmiSmartToolCalculationInfo::AddCalculationInfo(
    boost::shared_ptr<NFmiAreaMaskInfo> &theAreaMaskInfo)
{
  if (theAreaMaskInfo)
    itsCalculationOperandInfoVector.push_back(theAreaMaskInfo);
}

/*! tarkistaa onko lause muotoa:
 * par = MISS
 * vain tälläinen lauseke sallii puuttuvan arvon sijoituksen dataan skripteillä.
 * Pitää tehdä speciaali asetus systeemi, sillä muuten tulee ongelmia sijotusten kanssa,
 * koska muuten puuttuvia arvoja tulee sijoitetuksi aina kun jotain dataa puuttuu,
 * tai se ei kata koko aluetta tai aika-skaalaa.
 */
void NFmiSmartToolCalculationInfo::CheckIfAllowMissingValueAssignment(void)
{
  fAllowMissingValueAssignment = false;
  size_t size = itsCalculationOperandInfoVector.size();
  if (size == 1)
  {
    if (itsCalculationOperandInfoVector[0]->GetOperationType() == NFmiAreaMask::Constant)
    {
      if (itsCalculationOperandInfoVector[0]->GetMaskCondition().LowerLimit() == kFloatMissing)
        fAllowMissingValueAssignment = true;
    }
  }
}
