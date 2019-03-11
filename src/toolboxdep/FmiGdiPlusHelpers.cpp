

#ifdef _MSC_VER
#pragma warning (disable:4996) // poistaa widen- ja narrow ctype-metodien varoitukset
#endif



#include "stdafx.h"
#include "FmiGdiPlusHelpers.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiFileString.h"
#include "NFmiRect.h"
#include "NFmiToolBox.h"
#include "NFmiFileSystem.h"
#include "NFmiStationView.h"
#include "NFmiMapViewdescTop.h"
#include "xmlliteutils/UtfConverter.h"
#include "FmiWin32Helpers.h"
#include "NFmiColorSpaces.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "NFmiApplicationWinRegistry.h"
#include "boost/math/special_functions/round.hpp"

std::string CFmiGdiPlusHelpers::StrToUtf8(const std::string &theString)
{
    std::wstring wStr = CtrlView::StringToWString(theString);
    std::string utf8Str = UtfConverter::ToUtf8(wStr);
    size_t endPos = strlen(&utf8Str[0]);
    if(endPos < utf8Str.size())
        utf8Str.resize(endPos); // pit�� tehd� resize, koska muuten loppuun j�� paljon 0:aa
    return utf8Str;
}

static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

    Gdiplus::GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;  // Failure

    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j = 0; j < num; ++j)
	{
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

static std::string GetFinalFilePath(CFileDialog &fileSaveDlg, NFmiApplicationWinRegistry &applicationWinRegistry)
{
    std::string fileName = CT2A(fileSaveDlg.GetPathName());
    if(!fileName.empty())
    {
        applicationWinRegistry.SetSaveImageExtensionFilterIndex(fileSaveDlg.m_pOFN->nFilterIndex);
        auto defaultFileExtension = applicationWinRegistry.GetCurrentSaveImageFileFilterExtension();
        std::string fileExtension = CT2A(fileSaveDlg.GetFileExt());
        if(fileExtension.empty() && defaultFileExtension.empty() == false)
        {
            if(fileName.back() != '.')
                fileName.push_back('.');
            fileName += defaultFileExtension;
        }
        return fileName;
    }

    return "";
}

// filterIndex parametri alkaa 1:st�.
static std::string GetDefaultFilename(NFmiApplicationWinRegistry &applicationWinRegistry)
{
    std::string filename = "image1.";
    filename += applicationWinRegistry.GetCurrentSaveImageFileFilterExtension();
    return filename;
}


bool CFmiGdiPlusHelpers::SaveMfcBitmapToFile(const std::string &theCallingFunctionName, CBitmap *bm, SmartMetDocumentInterface *smartMetDocumentInterface, const NFmiRect *theRelativeOutputArea)
{
	if(bm)
	{
        auto &applicationWinRegistry = smartMetDocumentInterface->ApplicationWinRegistry();
		CFileDialog myDlg(FALSE,
							NULL,
							CA2T(::GetDefaultFilename(applicationWinRegistry).c_str()),
							OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                            g_SaveImageFileFilter,
							NULL );
        myDlg.m_ofn.lpstrInitialDir = CA2T(smartMetDocumentInterface->FileDialogDirectoryMemory().c_str());
        myDlg.m_ofn.nFilterIndex = applicationWinRegistry.SaveImageExtensionFilterIndex();

		if (myDlg.DoModal()==IDOK)
		{
			std::string fileName = ::GetFinalFilePath(myDlg, applicationWinRegistry);
            smartMetDocumentInterface->MakeAndStoreFileDialogDirectoryMemory(fileName);
			return CFmiGdiPlusHelpers::SaveMfcBitmapToFile(theCallingFunctionName, bm, fileName, theRelativeOutputArea);
		}
	}
	else // bm oli 0-pointteri
	{
		std::string errStr("Stored bitmap given to function CFmiGdiPlusHelpers::SaveMfcBitmapToFile was NULL, error in application.");
		CatLog::logMessage(errStr, CatLog::Severity::Error, CatLog::Category::Operational);
        ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(errStr.c_str()), _TEXT("File saving warning!"), MB_OK | MB_ICONWARNING);
	}

	return false;
}

static Gdiplus::Rect CalcGdiWantedSourcePixelArea(const Gdiplus::Size &theTotalSourceSize, const NFmiRect *theRelativeOutputArea)
{
    Gdiplus::Size wantedSize = theTotalSourceSize;
    Gdiplus::Point startPoint(0, 0);
	if(theRelativeOutputArea)
	{
        wantedSize.Width = boost::math::iround(wantedSize.Width * theRelativeOutputArea->Width());
        wantedSize.Height = boost::math::iround(wantedSize.Height * theRelativeOutputArea->Height());
        startPoint.X = boost::math::iround(theTotalSourceSize.Width * theRelativeOutputArea->Left());
        startPoint.Y = boost::math::iround(theTotalSourceSize.Height * theRelativeOutputArea->Top());
	}
    return Gdiplus::Rect(startPoint, wantedSize);
}

static std::string GetGdiStatusMessage(Gdiplus::Status theStatus)
{
    static std::vector<std::string> statusMessages = {
        "Ok",
        "GenericError",
        "InvalidParameter",
        "OutOfMemory",
        "ObjectBusy",
        "InsufficientBuffer",
        "NotImplemented",
        "Win32Error",
        "WrongState",
        "Aborted",
        "FileNotFound",
        "ValueOverflow",
        "AccessDenied",
        "UnknownImageFormat",
        "FontFamilyNotFound",
        "FontStyleNotFound",
        "NotTrueTypeFont",
        "UnsupportedGdiplusVersion",
        "GdiplusNotInitialized",
        "PropertyNotFound",
        "PropertyNotSupported",
        "ProfileNotFound"
    };
      
    size_t index = static_cast<size_t>(theStatus);
    if(index < statusMessages.size())
        return statusMessages[index];
    else
        return "UnknownGdiError";
}

static void ReportImageCroppingError(const std::string &theCallingFunctionName, const std::string &theFileName, Gdiplus::Rect wantedRectInPixels, const Gdiplus::Size &theTotalSourceSize)
{
    std::string errorString = theCallingFunctionName;
    errorString += " failed to crop and save image '";
    errorString += theFileName;
    errorString += "', orig image size in pixels (w,h): ";
    errorString += std::to_string(theTotalSourceSize.Width);
    errorString += ",";
    errorString += std::to_string(theTotalSourceSize.Height);
    errorString += ", crop area in pixels (x,y,w,h): ";
    errorString += std::to_string(wantedRectInPixels.X);
    errorString += ",";
    errorString += std::to_string(wantedRectInPixels.Y);
    errorString += ",";
    errorString += std::to_string(wantedRectInPixels.Width);
    errorString += ",";
    errorString += std::to_string(wantedRectInPixels.Height);
    CatLog::logMessage(errorString, CatLog::Severity::Error, CatLog::Category::Visualization);
}

bool CFmiGdiPlusHelpers::SaveMfcBitmapToFile(const std::string &theCallingFunctionName, CBitmap *bm, const std::string &theFileName, const NFmiRect *theRelativeOutputArea, bool throwError)
{
	if(bm)
	{
		CLSID  encoderClsid;
		INT    result;

		NFmiFileString fileString(theFileName);
		std::string fileExtension = fileString.Extension();
		NFmiStringTools::LowerCase(fileExtension);
		// GdiPlus Image osaa tallettaa seuraavissa formaateissa
		// image/bmp
		// image/jpeg
		// image/gif
		// image/tiff
		// image/png

		if(fileExtension == "bmp")
			result = GetEncoderClsid(L"image/bmp", &encoderClsid);
		else if(fileExtension == "jpeg" || fileExtension == "jpg")
			result = GetEncoderClsid(L"image/jpeg", &encoderClsid);
		else if(fileExtension == "gif")
			result = GetEncoderClsid(L"image/gif", &encoderClsid);
		else if(fileExtension == "tiff")
			result = GetEncoderClsid(L"image/tiff", &encoderClsid);
		else // if(fileExtension == "png") tehd��n defaulttina png kuva
			result = GetEncoderClsid(L"image/png", &encoderClsid);

		try
		{
			std::wstring fileName2(CtrlView::StringToWString(theFileName));
			Gdiplus::Bitmap aBitmap(*bm, 0);
            Gdiplus::Status gdiStatus = Gdiplus::Ok;

			if(theRelativeOutputArea)
			{
				// Clone a portion of the bitmap.
                Gdiplus::Size totalSourceSizeInPixels = Gdiplus::Size(aBitmap.GetWidth(), aBitmap.GetHeight());
                Gdiplus::Rect wantedRect = ::CalcGdiWantedSourcePixelArea(totalSourceSizeInPixels, theRelativeOutputArea);
				Gdiplus::Bitmap* clone = aBitmap.Clone(wantedRect, PixelFormatDontCare);
                if(clone)
                {
                    gdiStatus = clone->Save(fileName2.c_str(), &encoderClsid);
                    delete clone;
                }
                else
                    ::ReportImageCroppingError(theCallingFunctionName, theFileName, wantedRect, totalSourceSizeInPixels);
			}
			else
                gdiStatus = aBitmap.Save(fileName2.c_str(), &encoderClsid);

            if(gdiStatus != Gdiplus::Ok)
            {
                std::string errorStr = "Gdiplus system error: ";
                errorStr += ::GetGdiStatusMessage(gdiStatus);
                throw std::runtime_error(errorStr);
            }

            return true;
		}
		catch(std::exception &e)
		{
			std::string errStr("Cannot save image to file:\n");
			errStr += theFileName;
			errStr += "\nReason: ";
			errStr += e.what();
            if(throwError)
                throw std::runtime_error(errStr);
            else
                ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(errStr.c_str()), _TEXT("File saving warning!"), MB_OK | MB_ICONWARNING);
		}
		catch(...) // en tied� mit� GdiPlus systeemit heitt��
		{
			std::string errStr("Cannot save image to file:\n");
			errStr += theFileName;
			errStr += "\nFor unknown reason: ";
            if(throwError)
                throw std::runtime_error(errStr);
            else
                ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(errStr.c_str()), _TEXT("File saving warning!"), MB_OK | MB_ICONWARNING);
		}
	}
	else // bm oli 0-pointteri
	{
		std::string errStr("Stored bitmap given to function CFmiGdiPlusHelpers::SaveMfcBitmapToFile was NULL, error in application.");
        if(throwError)
            throw std::runtime_error(errStr);
        else
            ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(errStr.c_str()), _TEXT("File saving warning!"), MB_OK | MB_ICONWARNING);
	}

	return false;
}

// K�ytet��n kuvan tallennuksessa "store tmpFile -> rename to finalName" -toimintoa.
// throwError -parametri, jos true => heitt�� std::runtime_error:in, jos false => avaa messageboxin
bool CFmiGdiPlusHelpers::SafelySaveMfcBitmapToFile(const std::string &theCallingFunctionName, CBitmap *bm, const std::string &theFileName, const NFmiRect *theRelativeOutputArea, bool throwError)
{
    std::string temporaryFileName = theFileName;
    temporaryFileName += "_TMP";
    if(CFmiGdiPlusHelpers::SaveMfcBitmapToFile(theCallingFunctionName, bm, temporaryFileName, theRelativeOutputArea, throwError))
    {
        MoveFileEx(CA2T(temporaryFileName.c_str()), CA2T(theFileName.c_str()), MOVEFILE_REPLACE_EXISTING);
//        return NFmiFileSystem::RenameFile(temporaryFileName, theFileName); // T�m� funktio ottaa jostain syyst� vanhasta (korvattavasta) tiedostosta aikaleimat ja laittaa ne uuteen, paitsi jos k�y koodia step-by-step:illa l�pi
    }
    return false;
}

