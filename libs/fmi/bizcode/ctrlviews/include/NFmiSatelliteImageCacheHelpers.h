
#pragma once

#include "NFmiMetTime.h"
#include "NFmiDataIdent.h"

#include <list>

typedef std::list<std::pair<NFmiDataIdent, NFmiMetTime>> ImageCacheUpdateData; // T�ll� tietorakenteella siirret��n p�ivitys tarvetietoja imageCachen ja clientin (SmartMet) v�lill�

namespace Gdiplus
{
    class Bitmap;
}

// NFmiImageHolder:issa on ladattavasta kuvasta oleellisimmat tiedot:
// 1. Mahdollinen virheilmoitus, jos kuvan lataus on syyst� tai toisesta ep�onnistunut.
// 2. Itse Gdiplus::Bitmap image-pointteri
// 3. Kuvan sen hetkinen statustila
// 4. Kuvan UTC aika. Esim. NOAA satelliiteilla kuvaa saatetaan hakea 12:00 aikaan, mutta itse kuva on 12:07 (+- 30 minuutin aika haarukka sallittu), t�ll�in on hyv�  olla mukana kuvan originaali aika
struct NFmiImageData
{
    enum ImageStateEnum
    { // cachessa olevien kuvien tila vaihtelee ja niiden k�ytt� sen mukaan
        kUninitialized = 0, // Kuvalle ei ole viel� tehty mit��n, vain tiedoston nimi on tiedosssa.
        kLoading,       // Kuvaa on alettu lukemaan, joskus t�ss� voi kest�� kauan, jopa minuutin.
        kOk,            // Kuvaa on k�ytetty ja se on todettu toimivaksi.
        kErrorneus = 100,     // Kuvan kanssa oli jotain ongelmia, sit� ei saa k�ytt�� miss��n piirrossa tai muussa.
        kErrorLoadingTookTooLong,  // Kuvan lataus oli liian hidasta, julistetaan kuva vialliseksi.
    };

    NFmiImageData();
    NFmiImageData(const NFmiMetTime &time);
    bool IsImageUsable() const;
    
    std::string mErrorMessage;
    std::shared_ptr<Gdiplus::Bitmap> mImage;
    ImageStateEnum mState; // Kuvan luku/k�ytt� tila.
    NFmiMetTime mImageTime; // Kuvan kartta/valid -aika UTC (esim. 2015 4.9. 12:45)
    std::string mFilePath; // Kuvan tiedostopolku mm. tooltippia ja debuggausta varten
};

typedef std::shared_ptr<NFmiImageData> NFmiImageHolder;
