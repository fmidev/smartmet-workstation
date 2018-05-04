#pragma once

#include "NFmiMetTime.h"
#include "NFmiMilliSecondTimer.h"
#include "NFmiSatelliteImageCacheHelpers.h"

#include <future>
#include <mutex> 

#include <boost/noncopyable.hpp>

// Tässä on yksittäisen satelliitti kuvan tiedot ja mahdollinen data.
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

    NFmiImageHolder mImageHolder; // Jos satel kuva luetaan käyttöön, se talletetaan tähän
	std::string mImageFileName; // Satel kuvan tiedosto polkuineen kaikkineen (esim. P:\meteosat9\HRV\scandinavia_1008x1118\201504091245_8bit-msg-scandinavia_HRV.png)
    mutable std::mutex mImageLoadingMutex;
    std::future<NFmiImageHolder> mImageHolderFuture; // Tämän futuren avulla ladataan kuva erillisessä task-säikeessä
    NFmiMilliSecondTimer mLoadingTimer; // Tämän avulla mitataan kuvan latauksen kestoa, että voidaan tehdä päätöksiä eri vaiheiden lopetuksesta.
                                        // Latauksen mahdolliset eri vaiheet: 1. Aletaan lataamaan, tällöin SmartMet odottaa tietyn ajan, jos kuva latautuu tarpeeksi nopeasti.
                                        // 2. SmartMet on jatkanut eteenpäin, tästä eteenpäin annetaan latautumiselle tietty takaraja. Jos kuva ei ole latautunut siinä ajassa, laitetaan kuva errorneus-tilaan.
//    time_t mImageFileTime; // Jos tulevaisuudessa tulee tarve tutkia, onko tullut korjaava kuva vanhan kuvan tilalle, tämän avulla voidaan päivittää kuvia, jos levyltä löytyisi kuvalle uudempi aikaleima
    int mFirstTimeLoadingWaitTimeMs; // Kun lataus käynnistetään 1. kerran, kuinka kauan odotetaan siinä paikassa valmistumista, enenen kuin palautetaan tyhjää
    int mImageLoadingFailedWaitTimeMs; // Kuinka kauan yritetään ladata imagea maksimissaan, ennen kuin sen lataus todetaan lopullisesti virheelliseksi
};
