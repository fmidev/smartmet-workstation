
#pragma once

#include "NFmiMetTime.h"
#include "NFmiDataIdent.h"

#include <list>

typedef std::list<std::pair<NFmiDataIdent, NFmiMetTime>> ImageCacheUpdateData; // Tällä tietorakenteella siirretään päivitys tarvetietoja imageCachen ja clientin (SmartMet) välillä

namespace Gdiplus
{
    class Bitmap;
}

// NFmiImageHolder:issa on ladattavasta kuvasta oleellisimmat tiedot:
// 1. Mahdollinen virheilmoitus, jos kuvan lataus on syystä tai toisesta epäonnistunut.
// 2. Itse Gdiplus::Bitmap image-pointteri
// 3. Kuvan sen hetkinen statustila
// 4. Kuvan UTC aika. Esim. NOAA satelliiteilla kuvaa saatetaan hakea 12:00 aikaan, mutta itse kuva on 12:07 (+- 30 minuutin aika haarukka sallittu), tällöin on hyvä  olla mukana kuvan originaali aika
struct NFmiImageData
{
    enum ImageStateEnum
    { // cachessa olevien kuvien tila vaihtelee ja niiden käyttö sen mukaan
        kUninitialized = 0, // Kuvalle ei ole vielä tehty mitään, vain tiedoston nimi on tiedosssa.
        kLoading,       // Kuvaa on alettu lukemaan, joskus tässä voi kestää kauan, jopa minuutin.
        kOk,            // Kuvaa on käytetty ja se on todettu toimivaksi.
        kErrorneus = 100,     // Kuvan kanssa oli jotain ongelmia, sitä ei saa käyttää missään piirrossa tai muussa.
        kErrorLoadingTookTooLong,  // Kuvan lataus oli liian hidasta, julistetaan kuva vialliseksi.
    };

    NFmiImageData();
    NFmiImageData(const NFmiMetTime &time);
    bool IsImageUsable() const;
    
    std::string mErrorMessage;
    std::shared_ptr<Gdiplus::Bitmap> mImage;
    ImageStateEnum mState; // Kuvan luku/käyttö tila.
    NFmiMetTime mImageTime; // Kuvan kartta/valid -aika UTC (esim. 2015 4.9. 12:45)
    std::string mFilePath; // Kuvan tiedostopolku mm. tooltippia ja debuggausta varten
};

typedef std::shared_ptr<NFmiImageData> NFmiImageHolder;
