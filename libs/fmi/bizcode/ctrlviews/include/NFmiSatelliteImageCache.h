#pragma once

#include "NFmiMetTime.h"
#include "NFmiMilliSecondTimer.h"
#include "NFmiSatelliteImageCacheHelpers.h"

#include <future>
#include <mutex> 

#include <boost/noncopyable.hpp>

// T�ss� on yksitt�isen satelliitti kuvan tiedot ja mahdollinen data.
class NFmiSatelliteImageCache : public boost::noncopyable
{
public:

    NFmiSatelliteImageCache(const std::string &fileName, int firstTimeLoadingWaitTimeMs, int imageLoadingFailedWaitTimeMs);

    bool operator<(const NFmiSatelliteImageCache &other) const; // std::set:iin sijoitusta varten

    NFmiImageHolder Image();
    const std::string& ImageFileName() const {return mImageFileName;}
    const NFmiMetTime& ImageTime() const { return mImageHolder->mImageTime; }
    NFmiImageData::ImageStateEnum ImageState() const { return mImageHolder->mState; }
    void SetImageState(NFmiImageData::ImageStateEnum imageState);
    bool CheckOnImageLoading(int waitForMs);
    static NFmiMetTime GetTimeFromFileName(const std::string &fileName);
    void ResetImage();
private:
    NFmiImageHolder StartLoadingImage();

    NFmiImageHolder mImageHolder; // Jos satel kuva luetaan k�ytt��n, se talletetaan t�h�n
	std::string mImageFileName; // Satel kuvan tiedosto polkuineen kaikkineen (esim. P:\meteosat9\HRV\scandinavia_1008x1118\201504091245_8bit-msg-scandinavia_HRV.png)
    mutable std::mutex mImageLoadingMutex;
    std::future<NFmiImageHolder> mImageHolderFuture; // T�m�n futuren avulla ladataan kuva erillisess� task-s�ikeess�
    NFmiMilliSecondTimer mLoadingTimer; // T�m�n avulla mitataan kuvan latauksen kestoa, ett� voidaan tehd� p��t�ksi� eri vaiheiden lopetuksesta.
                                        // Latauksen mahdolliset eri vaiheet: 1. Aletaan lataamaan, t�ll�in SmartMet odottaa tietyn ajan, jos kuva latautuu tarpeeksi nopeasti.
                                        // 2. SmartMet on jatkanut eteenp�in, t�st� eteenp�in annetaan latautumiselle tietty takaraja. Jos kuva ei ole latautunut siin� ajassa, laitetaan kuva errorneus-tilaan.
//    time_t mImageFileTime; // Jos tulevaisuudessa tulee tarve tutkia, onko tullut korjaava kuva vanhan kuvan tilalle, t�m�n avulla voidaan p�ivitt�� kuvia, jos levylt� l�ytyisi kuvalle uudempi aikaleima
    int mFirstTimeLoadingWaitTimeMs; // Kun lataus k�ynnistet��n 1. kerran, kuinka kauan odotetaan siin� paikassa valmistumista, enenen kuin palautetaan tyhj��
    int mImageLoadingFailedWaitTimeMs; // Kuinka kauan yritet��n ladata imagea maksimissaan, ennen kuin sen lataus todetaan lopullisesti virheelliseksi
};
