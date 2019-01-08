
#include "NFmiSatelliteImageCache.h"
#include "NFmiFileString.h"
#include "NFmiStringTools.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "catlog/catlog.h"


static NFmiMetTime ConvertStampToTime(const std::string &theTimeStamp)
{
    if(theTimeStamp.size() == 12)
    {
        NFmiMetTime aTime(1);
        aTime.FromStr(theTimeStamp, kYYYYMMDDHHMM);
        return aTime;
    }
    else
        return NFmiMetTime::gMissingTime;
}

// Etsii tarpeeksi pitk‰n numero stringin tiedosto nimest‰, hajottaa ensin filenamen '_'-merkeill‰ osiin.
// timestampissa pit‰‰ olla muotoa YYYYMMDDHHMM eli 12 merkki‰.
static std::string GetTimeStamp(const std::string &theFileName)
{
	NFmiFileString fileString(theFileName);
	std::string fileBaseName(fileString.Header());
	std::vector<std::string> parts2 = NFmiStringTools::Split(fileBaseName, "_"); // kokeillaan ensin _-merkill‰ eroteltua aikaleimaa
	if(parts2.size() <= 1)
        parts2 = NFmiStringTools::Split(fileBaseName, "."); // kokeillaan sitten .-merkill‰ eroteltua aikaleimaa
	if(parts2.size() > 1) // tiedoston nimess‰ pit‰‰ olla _-merkill‰ erotettu timestamp-osio
	{
		for(unsigned int i=0; i<parts2.size(); i++)
		{
			if(parts2[i].size() == 12)
			{
				try
				{
					/* double tmp = */ NFmiStringTools::Convert<double>(parts2[i]);
					return parts2[i]; // jos double konversio onnistui, oletetaan ett‰ kyseess‰ on numeroita sis‰lt‰v‰ stringi
				}
				catch(std::exception & /* e */ )
				{
				}
			}
		}
	}
	return std::string();
}

// *********************************************************
// ************  NFmiSatelliteImageCache  ******************
// *********************************************************

NFmiSatelliteImageCache::NFmiSatelliteImageCache(const std::string &fileName, int firstTimeLoadingWaitTimeMs, int imageLoadingFailedWaitTimeMs)
:mImageHolder(std::make_shared<NFmiImageData>())
,mImageFileName(fileName)
,mImageLoadingMutex()
,mImageHolderFuture()
,mLoadingTimer()
,mFirstTimeLoadingWaitTimeMs(firstTimeLoadingWaitTimeMs)
,mImageLoadingFailedWaitTimeMs(imageLoadingFailedWaitTimeMs)
{
    mImageHolder->mFilePath = fileName;
    mImageHolder->mImageTime = NFmiSatelliteImageCache::GetTimeFromFileName(fileName); // Alustamattomassa imageHolderissa pit‰‰ olla aikatieto mukana
}

NFmiMetTime NFmiSatelliteImageCache::GetTimeFromFileName(const std::string &fileName)
{
    NFmiMetTime aTime = ::ConvertStampToTime(::GetTimeStamp(fileName));
    if(aTime != NFmiMetTime::gMissingTime)
        return aTime;
    else
        throw std::runtime_error(std::string("Error in NFmiSatelliteImageCache::GetTimeFromFileName: invalid timestamp in filename\n") + fileName);
}

bool NFmiSatelliteImageCache::operator<(const NFmiSatelliteImageCache &other) const
{
    return mImageHolder->mImageTime < other.mImageHolder->mImageTime;
}

NFmiImageHolder NFmiSatelliteImageCache::Image() 
{
    if(mImageHolder->mState == NFmiImageData::kUninitialized)
        return StartLoadingImage();
    else
        return mImageHolder;
}

static NFmiImageHolder LoadImageTask(std::string imageFileName)
{
    NFmiImageHolder imageHolder = std::make_shared<NFmiImageData>();
    try
    {
        // Latdattava ImageHolder pit‰‰ alustaa ajalla ja statuksella
        imageHolder->mFilePath = imageFileName;
        imageHolder->mImageTime = NFmiSatelliteImageCache::GetTimeFromFileName(imageFileName); // imageHolderissa pit‰‰ olla aikatieto mukana
        imageHolder->mState = NFmiImageData::kLoading;

//        std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // Testausta varten hidastus imagen lukuun
        imageHolder->mImage = std::shared_ptr<Gdiplus::Bitmap>(CtrlView::CreateBitmapFromFile(imageFileName));
        return imageHolder;
    }
    catch(std::exception &e)
    {
        imageHolder->mErrorMessage = e.what();
    }
    catch(...)
    {
        imageHolder->mErrorMessage = "Unknown error in NFmiSatelliteImageCache - LoadImageTask -function";
    }
    return imageHolder;
}

// T‰t‰ funktiota saa kutsua vain NFmiSatelliteImageCache::Image -metodista ja silloin vain kun ollaan uninitialized -tilassa.
// Kuvan lataus laitetaan erilliseen threadiin ja mImageState kLoading -tilaan. 
// Jos se valmistuu tarpeeksi nopeasti, t‰m‰ metodi odottaa ja palauttaa ladatun kuvan.
// Jos lataus kest‰‰ liikaa, palautetaan tyhj‰‰, mutta kun threadi lopulta valmistuu, 
// asetetaan kuva mImage:en ja laitetaan mImageState kJustLoaded -tilaan.
NFmiImageHolder NFmiSatelliteImageCache::StartLoadingImage()
{
    try
    {
        SetImageState(NFmiImageData::kLoading);
        mLoadingTimer.StartTimer();
        mImageHolderFuture = std::async(std::launch::async, ::LoadImageTask, mImageFileName);
        if(CheckOnImageLoading(mFirstTimeLoadingWaitTimeMs)) // Odotetaan v‰h‰n aikaa josko kuva latautuu
            return mImageHolder; // palautetaan se sitten v‰littˆm‰sti
    }
    catch(...)
    {
        SetImageState(NFmiImageData::kErrorneus);
    }

    return mImageHolder; // No k‰vi miten k‰vi, niin palautetaan imageholder joka tapauksessa
}

// Tarkistetaan onko latautumassa oleva image latautunut. 
// Palauttaa true jos lataus valmistunut, oli se onnistunut tai ei, muuten palautetaan aina false.
bool NFmiSatelliteImageCache::CheckOnImageLoading(int waitForMs)
{
    if(!mImageHolder)
    {
        std::string errorMessage = __FUNCTION__;
        errorMessage += ", mImageHolder was nullptr, this is logical error in program, notify developer...";
        CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Data, true);
    }
    else
    {
        if(mImageHolder->mState == NFmiImageData::kLoading)
        {
            if(mImageHolderFuture.wait_for(std::chrono::milliseconds(waitForMs)) == std::future_status::ready)
            {
                std::lock_guard<std::mutex> lock(mImageLoadingMutex);
                mImageHolder = mImageHolderFuture.get();
                // HUOM! Ei saa k‰ytt‰‰ SetImageState-metodia statuksen asetuksessa, koska siell‰kin on lukitus
                if(!mImageHolder->mErrorMessage.empty() || mImageHolder->mImage == nullptr)
                    mImageHolder->mState = NFmiImageData::kErrorneus; // Jos image holderin error-text osiossa on jotain tai image-ptr oli nullptr, ep‰onnistui lataus
                else
                    mImageHolder->mState = NFmiImageData::kOk; // Laitetaan status ok:ksi
                return true;
            }

            if(mLoadingTimer.CurrentTimeDiffInMSeconds() > mImageLoadingFailedWaitTimeMs)
            { // Kuvaa on latailtu tuloksetta tarpeeksi kauan, laitetaan error -tila p‰‰lle ja laitetaan imageHolderille error teksti
                std::lock_guard<std::mutex> lock(mImageLoadingMutex);
                mImageHolder->mErrorMessage = std::string("Image loading took too long time, loading failed for image-file:") + "\n" + mImageFileName;
                mImageHolder->mState = NFmiImageData::kErrorLoadingTookTooLong;
                mImageHolderFuture = std::future<NFmiImageHolder>(); // nollataan t‰ss‰ tilanteessa future
                return true; // Palautetaan true, koska saatiin joku ratkaisu aikaiseksi (loading lasted too long error)
            }
        }
    }
    return false;
}

// T‰m‰ on threadi turvallinen tapa muuttaa imagen statusta.
void NFmiSatelliteImageCache::SetImageState(NFmiImageData::ImageStateEnum imageState)
{
    std::lock_guard<std::mutex> lock(mImageLoadingMutex);
    mImageHolder->mState = imageState;
}

// Tyhjennet‰‰n imageHolder, jolloin kuva joudutaan lataamaan uudestaan, jos sit‰ k‰ytet‰‰n.
void NFmiSatelliteImageCache::ResetImage()
{
    std::lock_guard<std::mutex> lock(mImageLoadingMutex);
    mImageHolder = std::make_shared<NFmiImageData>(ImageTime()); // originaali aika otetaan talteen
}

