#pragma once

#include "NFmiMetEditorTypes.h"
#include "NFmiInfoData.h"
#include <boost/shared_ptr.hpp>

class NFmiFastQueryInfo;

namespace CtrlViewFastInfoFunctions
{
    unsigned long GetMaskedCount(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiMetEditorTypes::Mask theMask, bool fAllowRightClickSelection);
    bool IsObservationLockModeDataType(NFmiInfoData::Type theDataType);
}
