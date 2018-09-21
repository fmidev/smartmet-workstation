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
    // 1. Mikä on sallittu aikahaarukka? esim. 10.00 - 10.20 Utc
    // 2. Mikä on maksimi etäisyys CP-pisteestä lähimpään havaintoasemaan
    // 3. Hae arvot sallituilta asemilta sallituilta ajoilta, jos ei arvoa, muutos kyseisessä CP-pisteessä on 0 (merkitään missing arvolla).
    // 3.1. Hae datat sortattuna niin että se, missä on uusin sallittu aika, tulee ensimmäisenä
    // 3.2. Käy kaikki datat läpi
    // 3.3. Käy läpi datan sallitut ajat uusimmasta taaksepäin
    // 3.4. Käy läpi CP-pisteet ja etsi jokaiseen lähin asema. 
    // 3.5. Laita aseman etäisyys pisteeseen joka CP-pisteeseen talteen, jotta voidaan etsiä toisista datoista lähempiä asemia
    // 3.6. Miten CP-pisteiden arvoja arvotetaan, esim. toisen aseman aika on uudempi, mutta toinen asema on lähempänä
    // 4. Täydennä CP-pisteiden arvoja seuraavasti, jos CP-pisteessä on puuttuva, otetaan editoidusta datasta 0-hetkeltä arvo siihen (0-muutos)
    // 5. Laske CP-pisteiden avulla 0-hetken 'analyysikenttä'
    // 6. Laske analyysikentän ja editoidun datan 0-hetken kentän avulla muutoskenttä
    // 7. Blendaa muutoskenttä editoituun dataan liu'uttamalla
    // 8. Lasketaanko 'analyysit' ja niiden sijoitus editoituun dataan myös 0-hetkeä edeltäville editoidun datan ajoille?
}
