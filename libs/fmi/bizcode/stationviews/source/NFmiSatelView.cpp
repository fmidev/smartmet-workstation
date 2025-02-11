#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ k‰‰nt‰j‰n varoitusta (liian pitk‰ nimi >255 merkki‰ joka johtuu 'puretuista' STL-template nimist‰)
#endif

#include "NFmiSatelView.h"
#include "NFmiToolBox.h"
#include "NFmiDrawParam.h"
#include "NFmiSatelliteImageCacheSystem.h"
#include "NFmiDictionaryFunction.h"
#include "CtrlViewDocumentInterface.h"
#include "NFmiQueryDataUtil.h"
#include "CtrlViewFunctions.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "CtrlViewKeyboardFunctions.h"
#include "CtrlViewTimeConsumptionReporter.h"

#include <boost/filesystem.hpp> 

#include <boost/math/special_functions/round.hpp>

using namespace std;


NFmiSatelView::NFmiSatelView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
								,NFmiToolBox *theToolBox
								,boost::shared_ptr<NFmiDrawParam> &theDrawParam
								,FmiParameterName theParamId
								,int theRowIndex
                                ,int theColumnIndex)
: NFmiStationView(theMapViewDescTopIndex, theArea
			  ,theToolBox
			  ,theDrawParam
			  ,theParamId
			  ,NFmiPoint(0, 0)
			  ,NFmiPoint(1, 1)
			  ,theRowIndex
              ,theColumnIndex)
,itsSatelImageTime()
{
}

NFmiSatelView::~NFmiSatelView(void)
{
}

// Joillakin satelliiteilla on ep‰m‰‰r‰iset ajat kuvilleen (esim. NOAA:n ei-geostationaarinen). 
// Sen takia pit‰‰ antaa jokin aikaraja, jonka sis‰ll‰ tietty kuva sallitaan piirrett‰v‰ksi karttan‰ytˆlle.
// Normaaleille geostationaarisille satelliitti kuville offset on 0 (ne tuottavat kuvia m‰‰r‰tyn v‰liajoin).
long NFmiSatelView::ImagesOffsetInMinutes(const NFmiDataIdent &theDataIdent)
{
    if(theDataIdent.GetProducer()->GetIdent() == 3041)
        return 30; // jos NOAA tuottaja (3041), aikahaarukka on +- 30 minuuttia
    else
        return 0;
}

NFmiImageHolder NFmiSatelView::GetImageFromCache()
{
    const NFmiDataIdent &dataIdent = itsDrawParam->Param();
    auto & satelliteImageCacheSystem = itsCtrlViewDocumentInterface->SatelliteImageCacheSystem();
    if(!itsCtrlViewDocumentInterface->BetaProductGenerationRunning())
        return satelliteImageCacheSystem.FindImage(dataIdent, itsTime, NFmiSatelView::ImagesOffsetInMinutes(dataIdent));
    else
    {
        // Nyt ollaan Beta-tuotannossa, t‰llˆin pit‰‰ kuvien latausta odottaa loppuun asti loopissa eli kunnes ei olla en‰‰ loading-moodissa
        NFmiImageHolder imageHolder = satelliteImageCacheSystem.FindImage(dataIdent, itsTime, NFmiSatelView::ImagesOffsetInMinutes(dataIdent));
        for(; imageHolder && imageHolder->mState == NFmiImageData::kLoading;)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            imageHolder = satelliteImageCacheSystem.FindImage(dataIdent, itsTime, NFmiSatelView::ImagesOffsetInMinutes(dataIdent)); // Haetaan holder-olio aina m‰‰r‰ ajoin uudestaan tarkasteltavaksi
        }
        return imageHolder;
    }
}

void NFmiSatelView::Draw(NFmiToolBox *theGTB)
{
    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, __FUNCTION__);
    if(!theGTB)
		return;
	itsToolBox = theGTB;
	if(!IsParamDrawn())
		return;

    NFmiImageHolder imageHolder = GetImageFromCache();
    boost::shared_ptr<NFmiArea> imageArea = itsCtrlViewDocumentInterface->SatelliteImageCacheSystem().ImageChannelArea(itsDrawParam->Param());
    if(imageHolder && imageArea && imageHolder->IsImageUsable())
//    if(testBitmap)
    {
        if(!NFmiQueryDataUtil::AreAreasSameKind(itsArea.get(), imageArea.get()))
            DrawImageOnDifferentProjection(imageArea, imageHolder);
        else
        {
            std::shared_ptr<Gdiplus::Bitmap> usedImage = imageHolder->mImage;
//            std::shared_ptr<Gdiplus::Bitmap> usedImage = testBitmap;
            NFmiPoint bitmapSize(itsCtrlViewDocumentInterface->ActualMapBitmapSizeInPixels(itsMapViewDescTopIndex));
            NFmiPoint startPoint(CtrlViewUtils::ConvertPointFromRect1ToRect2(itsRect.TopLeft(), NFmiRect(0, 0, 1, 1), NFmiRect(NFmiPoint(0, 0), itsCtrlViewDocumentInterface->MapViewSizeInPixels(itsMapViewDescTopIndex))));
            Gdiplus::RectF destRect(static_cast<Gdiplus::REAL>(startPoint.X()), static_cast<Gdiplus::REAL>(startPoint.Y()), static_cast<Gdiplus::REAL>(bitmapSize.X()), static_cast<Gdiplus::REAL>(bitmapSize.Y()));
            NFmiRect sourceRect(imageArea->XYArea(itsArea.get()));
            sourceRect.Left(sourceRect.Left() * usedImage->GetWidth());
            sourceRect.Top(sourceRect.Top() * usedImage->GetHeight());
            sourceRect.Right(sourceRect.Right() * usedImage->GetWidth());
            sourceRect.Bottom(sourceRect.Bottom() * usedImage->GetHeight());
            Gdiplus::REAL alpha = itsDrawParam->Alpha() / 100.f; // 0 on t‰ysin l‰pin‰kyv‰, 0.5 = semi transparent ja 1.0 = opaque
            bool doNearestInterpolation = alpha >= 1.f ? true : false;
            CtrlView::DrawBitmapToDC_4(itsToolBox->GetDC(), *usedImage, sourceRect, destRect, doNearestInterpolation, NFmiImageAttributes(alpha), itsGdiPlusGraphics);
        }
    }
}

static Gdiplus::Color GetColor(Gdiplus::Bitmap &theImage, const NFmiPoint &theGridPoint, const Gdiplus::Color &theTransparentColor)
{
   try
   {
       int x = boost::math::iround(theGridPoint.X());
       int y = boost::math::iround(theImage.GetHeight() - theGridPoint.Y() - 1); // aina pit‰‰ k‰‰nt‰‰ y-akseli kun kyseess‰ on newbase vs. gdiplus bitmap
       Gdiplus::Color aColor;
       if(x >= 0 && x < static_cast<int>(theImage.GetWidth()) && y >= 0 && y < static_cast<int>(theImage.GetHeight()))
       {
           theImage.GetPixel(x, y, &aColor);
           return aColor;
       }
   }
   catch(...)
   {
   }
   return theTransparentColor;
}

Gdiplus::Bitmap* CreateProjectedImage(Gdiplus::Bitmap &theSourceImage, const boost::shared_ptr<NFmiArea> &theSourceArea, const boost::shared_ptr<NFmiArea> &theDestArea, const NFmiPoint &theDestImageSize, const Gdiplus::Color &theTransparentColor)
{
    NFmiGrid grid1(theSourceArea.get(), theSourceImage.GetWidth(), theSourceImage.GetHeight());
    NFmiGrid grid2(theDestArea.get(), static_cast<unsigned long>(theDestImageSize.X()), static_cast<unsigned long>(theDestImageSize.Y()));
   Gdiplus::Bitmap *img2 = new Gdiplus::Bitmap(grid2.XNumber(), grid2.YNumber(), PixelFormat24bppRGB); // HUOM! SetPixel ei toimi monocrome tai grayscale kuville, pit‰‰ olla 24ppb
   if(img2)
   {
       for(grid2.Reset(); grid2.Next(); )
       {
           NFmiPoint latlonPoint2(grid2.LatLon());
           NFmiPoint gridPoint1(grid1.LatLonToGrid(latlonPoint2));
           Gdiplus::Color color1(::GetColor(theSourceImage, gridPoint1, theTransparentColor));
           NFmiPoint gridPoint2(grid2.GridPoint());
           int xInd2 = boost::math::iround(gridPoint2.X());
           int yInd2 = img2->GetHeight() - boost::math::iround(gridPoint2.Y()) - 1; // pit‰‰ k‰‰nt‰‰ y-akseli
           img2->SetPixel(xInd2, yInd2, color1);
       }

   }
   return img2;
}

// yritet‰‰n piirt‰‰ kuva erilaisen projektion p‰‰lle kuin miss‰ alkuper‰inen
// kuva on. T‰m‰ vaati kuvan konvertointia haluttuun projektioon.
// Oletus: theImageArea ja theImageHolder on jo tarkistettu.
void NFmiSatelView::DrawImageOnDifferentProjection(boost::shared_ptr<NFmiArea> &theImageArea, NFmiImageHolder &theImageHolder)
{
    CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(std::string(__FUNCTION__) + ": must project satel image to different map projection", this);
    Gdiplus::Color transColor(0, 1, 2, 5); // ((static_cast<Gdiplus::ARGB>(Gdiplus::Color::Purple));
    // nyt pit‰‰ tehd‰ kuvasta lennossa konversio haluttuun projektioon
    NFmiPoint bitmapSize(itsCtrlViewDocumentInterface->ActualMapBitmapSizeInPixels(itsMapViewDescTopIndex));
    Gdiplus::Bitmap* projectedBitmap = ::CreateProjectedImage(*theImageHolder->mImage, theImageArea, itsArea, bitmapSize, transColor);
    if(projectedBitmap)
    {
        NFmiPoint startPoint(CtrlViewUtils::ConvertPointFromRect1ToRect2(itsRect.TopLeft(), NFmiRect(0, 0, 1, 1), NFmiRect(NFmiPoint(0, 0), itsCtrlViewDocumentInterface->MapViewSizeInPixels(itsMapViewDescTopIndex))));
        Gdiplus::RectF destRect(static_cast<Gdiplus::REAL>(startPoint.X()), static_cast<Gdiplus::REAL>(startPoint.Y()), static_cast<Gdiplus::REAL>(bitmapSize.X()), static_cast<Gdiplus::REAL>(bitmapSize.Y()));
        NFmiRect sourceRect;
        sourceRect.Size(bitmapSize);

        Gdiplus::REAL alpha = itsDrawParam->Alpha() / 100.f; // 0 on t‰ysin l‰pin‰kyv‰, 0.5 = semi transparent ja 1.0 = opaque
        bool doNearestInterpolation = alpha >= 1.f ? true : false;
        CtrlView::DrawBitmapToDC_4(itsToolBox->GetDC(), *projectedBitmap, sourceRect, destRect, doNearestInterpolation, NFmiImageAttributes(transColor, alpha), itsGdiPlusGraphics);
        delete projectedBitmap; // t‰m‰ kuva pit‰‰ tuhota, mutta ei satelImagea
    }
}

std::string NFmiSatelView::ComposeToolTipText(const NFmiPoint& /* theRelativePoint */ )
{
	string str = itsDrawParam->ParameterAbbreviation();
    auto fontColor = CtrlViewUtils::GetParamTextColor(itsDrawParam->DataType(), itsDrawParam->UseArchiveModelData());
    str = AddColorTagsToString(str, fontColor, true);
    str += " ";

    NFmiImageHolder imageHolder = GetImageFromCache();
    if(!imageHolder)
    {
        str += "(";
        str += ::GetDictionaryString("No image at this time");
        str += ")";
        return str;
    }
    else
    {
        NFmiString timeStr(imageHolder->mImageTime.ToStr(NFmiString("YYYY.MM.DD HH:mm [utc]")));
        str += static_cast<char*>(timeStr);
        str += " (";
        if(imageHolder->mState == NFmiImageData::kLoading)
            str += ::GetDictionaryString("Loading...");
        else if(imageHolder->mState == NFmiImageData::kErrorneus)
            str += ::GetDictionaryString("Error occured");
        else if(imageHolder->mState == NFmiImageData::kErrorLoadingTookTooLong)
            str += ::GetDictionaryString("Error: loading took too long");
        else if(imageHolder->mState == NFmiImageData::kOk)
            str += ::GetDictionaryString("image ok");
        else if(imageHolder->mState == NFmiImageData::kUninitialized)
            str += ::GetDictionaryString("Uninitialized, report developer");
        else
            str += ::GetDictionaryString("Unknown image status, report developer");
        str += ")";
        
        // Jos lataus virhe, laitetaan virhe viesti, koska siin‰ on mukana koko kuvan polku, muuten vain kuvan nimi/polku
        if(imageHolder->mState == NFmiImageData::kErrorLoadingTookTooLong || imageHolder->mState == NFmiImageData::kErrorneus)
            str += "\n" + imageHolder->mErrorMessage;
        else
        {
            bool showExtraInfo = CtrlView::IsKeyboardKeyDown(VK_CONTROL); // jos CTRL-n‰pp‰in on pohjassa, laitetaan lis‰‰ infoa n‰kyville

            // Laitetaan tooltippiin viel‰ sulkuihin seuraavalle riville kuvan polku
            str += "\n(";
            if(showExtraInfo) // joko koko polku tai vain tiedosto nimi
                str += imageHolder->mFilePath;
            else
            {
                boost::filesystem::path aPath = imageHolder->mFilePath;
                str += aPath.filename().string();
            }
            str += ")";
        }

        return str;
    }
}
