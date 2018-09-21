#include "NFmiControlPointObservationBlender.h"

NFmiControlPointObservationBlender::NFmiControlPointObservationBlender(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiDrawParam> &theDrawParam, boost::shared_ptr<NFmiAreaMaskList> &theMaskList,
    unsigned long theAreaMask, boost::shared_ptr<NFmiEditorControlPointManager> &theCPManager, const NFmiRect &theCPGridCropRect,
    bool theUseGridCrop, const NFmiPoint &theCropMarginSize, checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> &observationInfos, const NFmiMetTime &actualFirstTime)
    :NFmiDataParamControlPointModifier(theInfo, theDrawParam, theMaskList,
        theAreaMask, theCPManager, theCPGridCropRect,
        theUseGridCrop, theCropMarginSize)
    , itsObservationInfos(observationInfos)
    , itsActualFirstTime(actualFirstTime)
{
}

void NFmiControlPointObservationBlender::DoTimeSerialModifications(NFmiTimeDescriptor& theActiveTimes, NFmiThreadCallBacks *theThreadCallBacks)
{
    // 1. Mik� on sallittu aikahaarukka? esim. 10.00 - 10.20 Utc
    // 2. Mik� on maksimi et�isyys CP-pisteest� l�himp��n havaintoasemaan
    // 3. Hae arvot sallituilta asemilta sallituilta ajoilta, jos ei arvoa, muutos kyseisess� CP-pisteess� on 0 (merkit��n missing arvolla).
    // 3.1. Hae datat sortattuna niin ett� se, miss� on uusin sallittu aika, tulee ensimm�isen�
    // 3.2. K�y kaikki datat l�pi
    // 3.3. K�y l�pi datan sallitut ajat uusimmasta taaksep�in
    // 3.4. K�y l�pi CP-pisteet ja etsi jokaiseen l�hin asema. 
    // 3.5. Laita aseman et�isyys pisteeseen joka CP-pisteeseen talteen, jotta voidaan etsi� toisista datoista l�hempi� asemia
    // 3.6. Miten CP-pisteiden arvoja arvotetaan, esim. toisen aseman aika on uudempi, mutta toinen asema on l�hemp�n�
    // 4. T�ydenn� CP-pisteiden arvoja seuraavasti, jos CP-pisteess� on puuttuva, otetaan editoidusta datasta 0-hetkelt� arvo siihen (0-muutos)
    // 5. Laske CP-pisteiden avulla 0-hetken 'analyysikentt�'
    // 6. Laske analyysikent�n ja editoidun datan 0-hetken kent�n avulla muutoskentt�
    // 7. Blendaa muutoskentt� editoituun dataan liu'uttamalla
    // 8. Lasketaanko 'analyysit' ja niiden sijoitus editoituun dataan my�s 0-hetke� edelt�ville editoidun datan ajoille?
}
