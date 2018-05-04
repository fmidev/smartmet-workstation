//**********************************************************
// C++ Class Name : NFmiSmartToolCalculationSectionInfo
// ---------------------------------------------------------
//  Author         : pietarin
//  Creation Date  : 8.11. 2010
//**********************************************************
#ifdef _MSC_VER
#pragma warning(disable : 4786)  // poistaa n kpl VC++ kääntäjän varoitusta
#endif

#include "NFmiSmartToolCalculationSectionInfo.h"
#include "NFmiAreaMaskInfo.h"
#include "NFmiSmartToolCalculationInfo.h"
//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------
NFmiSmartToolCalculationSectionInfo::NFmiSmartToolCalculationSectionInfo(void)
{
}
NFmiSmartToolCalculationSectionInfo::~NFmiSmartToolCalculationSectionInfo(void)
{
}

void NFmiSmartToolCalculationSectionInfo::AddCalculationInfo(
    boost::shared_ptr<NFmiSmartToolCalculationInfo> &value)
{
  if (value)
    itsSmartToolCalculationInfoVector.push_back(value);
}

// Lisätään set:iin kaikki parametrit, joita tässä sectioniossa voidaan muokata.
void NFmiSmartToolCalculationSectionInfo::AddModifiedParams(std::map<int, std::string> &theModifiedParams)
{
  checkedVector<boost::shared_ptr<NFmiSmartToolCalculationInfo> >::size_type ssize =
      itsSmartToolCalculationInfoVector.size();
  for (size_t i = 0; i < ssize; i++)
  {
    theModifiedParams.emplace(itsSmartToolCalculationInfoVector[i]->GetResultDataInfo()->GetDataIdent().GetParamIdent(), itsSmartToolCalculationInfoVector[i]->GetResultDataInfo()->GetDataIdent().GetParamName());
  }
}
