#include "CtrlViewFastInfoFunctions.h"
#include "NFmiSmartInfo.h"

namespace CtrlViewFastInfoFunctions
{

    unsigned long GetMaskedCount(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiMetEditorTypes::Mask theMask, bool fAllowRightClickSelection)
    {
        if(theMask == NFmiMetEditorTypes::kFmiDisplayedMask && !fAllowRightClickSelection)
            return 0;  // jos hiiren oikealla ei ole sallittua valita pisteitä kartalta, tämä palauttaa 0:aa

        NFmiSmartInfo *info = dynamic_cast<NFmiSmartInfo *>(theInfo.get());
        if(info)
            return info->LocationMaskedCount(theMask);
        else
            return 0;
    }

    // onko kyseinen queryData tyyppi sellainen että sitä pitää seurata kun animaatio on
    // follow observations moodissa.
    bool IsObservationLockModeDataType(NFmiInfoData::Type theDataType)
    {
        if(theDataType == NFmiInfoData::kObservations || theDataType == NFmiInfoData::kFlashData || theDataType == NFmiInfoData::kSingleStationRadarData)
            return true;
        else
            return false;
    }
}
