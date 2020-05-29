#include "CtrlViewGdiPlusFunctions.h"
#include "NFmiToolBox.h"
#include "NFmiFileString.h"
#include "NFmiFileSystem.h"
#include "NFmiCtrlView.h"
#include "CtrlViewDocumentInterface.h"
#include "GdiPlusLineInfo.h"
#include "CtrlViewFunctions.h"
#include "NFmiColorSpaces.h"
#include "NFmiDrawParam.h"

namespace CtrlView
{
    Gdiplus::Color NFmiColor2GdiplusColor(const NFmiColor &theColor, bool fUseAlpha)
    {
        if(fUseAlpha)
            return Gdiplus::Color(static_cast<BYTE>(theColor.Alpha() * 255), static_cast<BYTE>(theColor.GetRed() * 255), static_cast<BYTE>(theColor.GetGreen() * 255), static_cast<BYTE>(theColor.GetBlue() * 255));
        else
            return Gdiplus::Color(static_cast<BYTE>(theColor.GetRed() * 255), static_cast<BYTE>(theColor.GetGreen() * 255), static_cast<BYTE>(theColor.GetBlue() * 255));
    }

    int RgbToColorIndex(Matrix3D<std::pair<int, COLORREF> > &theUsedColorsCube, const NFmiColor& color)
    {
        int k = boost::math::iround(color.Red()*(theUsedColorsCube.zDIM() - 1));
        int j = boost::math::iround(color.Green()*(theUsedColorsCube.Columns() - 1));
        int i = boost::math::iround(color.Blue()*(theUsedColorsCube.Rows() - 1));

        return theUsedColorsCube[k][j][i].first;
    }

    int GetBrighterTMColorIndex(Matrix3D<std::pair<int, COLORREF> > &theUsedColorsCube, int theTMColorIndex, double theBrightningFactor, int theSpecialColorEndIndex)
    {
        if(theTMColorIndex <= theSpecialColorEndIndex)
            return theTMColorIndex; // ei vaalenneta foreground, background eik‰ transparent v‰rej‰
        else
        {

            int tmpColorIndex = theTMColorIndex - theSpecialColorEndIndex - 1;
            size_t xInd = tmpColorIndex % theUsedColorsCube.Rows();
            size_t yInd = (tmpColorIndex / theUsedColorsCube.Rows()) % theUsedColorsCube.Columns();
            size_t zInd = (tmpColorIndex / (theUsedColorsCube.Rows()*theUsedColorsCube.Columns())) % theUsedColorsCube.zDIM();

            NFmiColor aColor(ColorRef2Color(theUsedColorsCube[zInd][yInd][xInd].second));
            NFmiColor modifiedColor = NFmiColorSpaces::GetBrighterColor(aColor, theBrightningFactor);

            return RgbToColorIndex(theUsedColorsCube, modifiedColor);
        }
    }

    void ModifyColors(boost::shared_ptr<NFmiDrawParam> &theDrawParam, double theBrightningFactor, bool fDoIsolineColors, bool fDoContourColors, bool fDosymbolColors, Matrix3D<std::pair<int, COLORREF> > &theUsedColorsCube, int theSpecialColorEndIndex)
    {
        if(fDoIsolineColors)
        {
            NFmiColor newColor = NFmiColorSpaces::GetBrighterColor(theDrawParam->IsolineColor(), theBrightningFactor);
            theDrawParam->IsolineColor(newColor);

            newColor = NFmiColorSpaces::GetBrighterColor(theDrawParam->IsolineTextColor(), theBrightningFactor);
            theDrawParam->IsolineTextColor(newColor);

            newColor = NFmiColorSpaces::GetBrighterColor(theDrawParam->SimpleIsoLineColorShadeLowValueColor(), theBrightningFactor);
            theDrawParam->SimpleIsoLineColorShadeLowValueColor(newColor);
            newColor = NFmiColorSpaces::GetBrighterColor(theDrawParam->SimpleIsoLineColorShadeMidValueColor(), theBrightningFactor);
            theDrawParam->SimpleIsoLineColorShadeMidValueColor(newColor);
            newColor = NFmiColorSpaces::GetBrighterColor(theDrawParam->SimpleIsoLineColorShadeHighValueColor(), theBrightningFactor);
            theDrawParam->SimpleIsoLineColorShadeHighValueColor(newColor);
            newColor = NFmiColorSpaces::GetBrighterColor(theDrawParam->SimpleIsoLineColorShadeHigh2ValueColor(), theBrightningFactor);
            theDrawParam->SimpleIsoLineColorShadeHigh2ValueColor(newColor);

            std::vector<int> colIndexies = theDrawParam->SpecialIsoLineColorIndexies();
            std::vector<int> newColIndexies;
            for(size_t i = 0; i < colIndexies.size(); i++)
                newColIndexies.push_back(GetBrighterTMColorIndex(theUsedColorsCube, colIndexies[i], theBrightningFactor, theSpecialColorEndIndex));
            theDrawParam->SetSpecialIsoLineColorIndexies(newColIndexies); // HUOM! t‰m‰ aettaa myˆs SpecialContourColorIndexies samalla, joten viel‰ ei s‰‰ tehd‰ mit‰‰n myˆhemmin contour osiossa niille.
        }

        if(fDoContourColors)
        {
            NFmiColor newColor = NFmiColorSpaces::GetBrighterColor(theDrawParam->ColorContouringColorShadeLowValueColor(), theBrightningFactor);
            theDrawParam->ColorContouringColorShadeLowValueColor(newColor);
            newColor = NFmiColorSpaces::GetBrighterColor(theDrawParam->ColorContouringColorShadeMidValueColor(), theBrightningFactor);
            theDrawParam->ColorContouringColorShadeMidValueColor(newColor);
            newColor = NFmiColorSpaces::GetBrighterColor(theDrawParam->ColorContouringColorShadeHighValueColor(), theBrightningFactor);
            theDrawParam->ColorContouringColorShadeHighValueColor(newColor);
            newColor = NFmiColorSpaces::GetBrighterColor(theDrawParam->ColorContouringColorShadeHigh2ValueColor(), theBrightningFactor);
            theDrawParam->ColorContouringColorShadeHigh2ValueColor(newColor);

            // HUOM! N‰m‰ asetetaan isoline kohdassa samalla kuin custom isoline v‰ri indeksit, joten ei saa vaalentaa n‰it‰ toistaiseksi uudestaan.
            /*
            std::vector<int> colIndexies = theDrawParam->SpecialContourColorIndexies();
            std::vector<int> newColIndexies;
            for(size_t i = 0; i < colIndexies.size(); i++)
            newColIndexies.push_back(::GetBrighterTMColorIndex(theUsedColorsCube, colIndexies[i], theBrightningFactor, theSpecialColorEndIndex));
            theDrawParam->SetSpecialContourColorIndexies(newColIndexies);
            */
        }
    }

    // Lasketaan ns. vaalennus kerroin, kun piirret‰‰n eri m‰‰r‰ malliajoja p‰‰llekk‰in.
    // Jos theCurrentIndex on sama kuin theStartIndex, palautetaan 0. Kun theCurrentIndex kasvaa, kasvaa vaalennus kerroin.
    double CalcBrightningFactor(int theStartIndex, int theModelRunCount, int theCurrentIndex)
    {
        if(theStartIndex == theCurrentIndex)
            return 0;
        else
        {
            double brightningFactor = 20. + (60. / theModelRunCount)*(-theCurrentIndex + theStartIndex); // mit‰ isompi luku, sit‰ enemm‰n vaalenee (0-100), vanhemmat malliajot vaaleammalla
            return brightningFactor;
        }
    }

    NFmiColor ColorRef2Color(COLORREF color)
    {
        NFmiColor tColor;
        tColor.SetRGB((float(GetRValue(color)) / float(255.0)),
            (float(GetGValue(color)) / float(255.0)),
            (float(GetBValue(color)) / float(255.0)));
        return tColor;
    }

    COLORREF Color2ColorRef(const NFmiColor &theColor)
    {
        return RGB(theColor.Red() * 255, theColor.Green() * 255, theColor.Blue() * 255);
    }

    std::wstring StringToWString(const std::string& s)
    {
        std::locale loc;
        std::wstring ws;
        ws.resize(s.size());
        std::use_facet<std::ctype<wchar_t> >(loc).widen(&s[0], &s[0] + s.size(), &ws[0]);
        return ws;
    }

    std::string WStringToString(const std::wstring& s)
    {
        std::locale loc;
        std::string ns;
        ns.resize(s.size());
        std::use_facet<std::ctype<wchar_t> >(loc).narrow(&s[0], &s[0] + s.size(), '?', &ns[0]);
        return ns;
    }

    const Gdiplus::PointF Relative2GdiplusPoint(NFmiToolBox *theToolBox, const NFmiPoint &theRelativePoint)
    {
        CPoint absolutePoint;
        theToolBox->ConvertPoint(theRelativePoint, absolutePoint);
        return Gdiplus::PointF(static_cast<Gdiplus::REAL>(absolutePoint.x), static_cast<Gdiplus::REAL>(absolutePoint.y));
    }

    std::vector<Gdiplus::PointF> Relative2GdiplusPolyLine(NFmiToolBox *theToolBox, const std::vector<NFmiPoint> &theRelativePolyLine, const NFmiPoint &theRelativeOffset)
    {
        bool useOffset = theRelativeOffset != NFmiPoint();
        std::vector<Gdiplus::PointF> gdiplusPolyLine(theRelativePolyLine.size());
        for(size_t i = 0; i < gdiplusPolyLine.size(); i++)
        {
            if(useOffset)
                gdiplusPolyLine[i] = Relative2GdiplusPoint(theToolBox, theRelativePolyLine[i] + theRelativeOffset);
            else
                gdiplusPolyLine[i] = Relative2GdiplusPoint(theToolBox, theRelativePolyLine[i]);
        }

        return gdiplusPolyLine;
    }

    const Gdiplus::Rect CRect2GdiplusRect(const CRect &theCRect)
    {
        return Gdiplus::Rect(theCRect.left, theCRect.top, theCRect.Width(), theCRect.Height());
    }

    const Gdiplus::RectF CRect2GdiplusRectF(const CRect &theCRect)
    {
        return Gdiplus::RectF(static_cast<Gdiplus::REAL>(theCRect.left), static_cast<Gdiplus::REAL>(theCRect.top),
            static_cast<Gdiplus::REAL>(theCRect.Width()), static_cast<Gdiplus::REAL>(theCRect.Height()));
    }

    const Gdiplus::Rect Relative2GdiplusRect(NFmiToolBox *theToolBox, const NFmiRect &theRelativeRect)
    {
        CRect absoluteRect;
        theToolBox->ConvertRect(theRelativeRect, absoluteRect);
        return Gdiplus::Rect(absoluteRect.left, absoluteRect.top, absoluteRect.Width(), absoluteRect.Height());
    }

    const Gdiplus::RectF Relative2GdiplusRectF(NFmiToolBox *theToolBox, const NFmiRect &theRelativeRect)
    {
        CRect absoluteRect;
        theToolBox->ConvertRect(theRelativeRect, absoluteRect);
        return Gdiplus::RectF(static_cast<Gdiplus::REAL>(absoluteRect.left), static_cast<Gdiplus::REAL>(absoluteRect.top), static_cast<Gdiplus::REAL>(absoluteRect.Width()), static_cast<Gdiplus::REAL>(absoluteRect.Height()));
    }

    const NFmiRect GdiplusRect2Relative(NFmiToolBox *theToolBox, const Gdiplus::RectF &theGdiplusRect)
    {
        CRect absoluteRect;
        absoluteRect.top = boost::math::iround(theGdiplusRect.Y);
        absoluteRect.left = boost::math::iround(theGdiplusRect.X);
        absoluteRect.right = boost::math::iround(theGdiplusRect.X + theGdiplusRect.Width);
        absoluteRect.bottom = boost::math::iround(theGdiplusRect.Y + theGdiplusRect.Height);
        NFmiRect relativeRect;
        theToolBox->ConvertRect(absoluteRect, relativeRect);
        return relativeRect;
    }

    const NFmiRect GdiplusRect2Relative(NFmiToolBox *theToolBox, const Gdiplus::Rect &theGdiplusRect)
    {
        CRect absoluteRect;
        absoluteRect.top = theGdiplusRect.Y;
        absoluteRect.left = theGdiplusRect.X;
        absoluteRect.right = theGdiplusRect.X + theGdiplusRect.Width;
        absoluteRect.bottom = theGdiplusRect.Y + theGdiplusRect.Height;
        NFmiRect relativeRect;
        theToolBox->ConvertRect(absoluteRect, relativeRect);
        return relativeRect;
    }

    const NFmiPoint GdiplusPoint2Relative(NFmiToolBox* theToolBox, const Gdiplus::Point& theGdiplusPoint)
    {
        CPoint abolutePoint(theGdiplusPoint.X, theGdiplusPoint.Y);
        NFmiPoint relativePoint;
        theToolBox->ConvertPoint(abolutePoint, relativePoint);
        return relativePoint;
    }

    void MakePathFromRect(Gdiplus::GraphicsPath &thePath, NFmiToolBox *theToolBox, const NFmiRect &theRelativeRect)
    {
        thePath.Reset();
        thePath.AddRectangle(Relative2GdiplusRect(theToolBox, theRelativeRect));
        thePath.CloseFigure();
    }

    Gdiplus::RectF GetStringBoundingBox(Gdiplus::Graphics &theGdiPlusGraphics, const std::string &theString, float theFontSizeInPixels, const Gdiplus::PointF &theOringinInPixels, const std::wstring &theFontNameStr)
    {
        Gdiplus::Font aFont(theFontNameStr.c_str(), theFontSizeInPixels, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
        return GetStringBoundingBox(theGdiPlusGraphics, theString, theOringinInPixels, aFont);
    }

    Gdiplus::RectF GetStringBoundingBox(Gdiplus::Graphics& theGdiPlusGraphics, const std::string& theString, const Gdiplus::PointF& theOringinInPixels, const Gdiplus::Font& theFont)
    {
        std::wstring wideStr = StringToWString(theString);
        Gdiplus::RectF boundinBox;
        theGdiPlusGraphics.MeasureString(wideStr.c_str(), static_cast<INT>(wideStr.size()), &theFont, theOringinInPixels, &boundinBox);
        return boundinBox;
    }

    // theAlpha: 0 on t‰ysin l‰pin‰kyv‰, 0.5 = semi transparent ja 1.0 = opaque
    // HUOM! CDC voi olla 0-pointteri.
    void DrawBitmapToDC(CDC *theDC, Gdiplus::Bitmap &theBitmap, const NFmiRect &theSourcePixels, const Gdiplus::RectF &theDestPixels, Gdiplus::REAL theAlpha, bool fDoNearestInterpolation, Gdiplus::Graphics *theGdiPlusGraphics)
    {
        Gdiplus::ImageAttributes imageAttr;
        Gdiplus::ColorMatrix colorMatrix = {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, theAlpha, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f
        };
        imageAttr.SetColorMatrix(&colorMatrix);
        DrawBitmapToDC(theDC, theBitmap, theSourcePixels, theDestPixels, fDoNearestInterpolation, &imageAttr, theGdiPlusGraphics);
    }

    void DrawBitmapToDC(CDC *theDC, Gdiplus::Bitmap &theBitmap, const NFmiRect &theSourcePixels, const Gdiplus::RectF &theDestPixels, const Gdiplus::Color &theTransparentColor, Gdiplus::REAL theAlpha, bool fDoNearestInterpolation)
    {
        Gdiplus::ImageAttributes imageAttr;
        Gdiplus::ColorMatrix colorMatrix = {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, theAlpha, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f
        };
        imageAttr.SetColorMatrix(&colorMatrix);
        imageAttr.SetColorKey(theTransparentColor, theTransparentColor);
        DrawBitmapToDC(theDC, theBitmap, theSourcePixels, theDestPixels, fDoNearestInterpolation, &imageAttr);
    }

    void DrawBitmapToDC(CDC *theDC, Gdiplus::Bitmap &theBitmap, const NFmiRect &theSourcePixels, const Gdiplus::RectF &theDestPixels, const Gdiplus::Color &theTransparentColor, bool fDoNearestInterpolation)
    {
        Gdiplus::ImageAttributes imageAttr;
        imageAttr.SetColorKey(theTransparentColor, theTransparentColor);
        DrawBitmapToDC(theDC, theBitmap, theSourcePixels, theDestPixels, fDoNearestInterpolation, &imageAttr);
    }

    static void DrawBitmapToDC(Gdiplus::Graphics *theGdiPlusGraphics, bool isPrinting, Gdiplus::Bitmap &theBitmap, const NFmiRect &theSourcePixels, const Gdiplus::RectF &theDestPixels, Gdiplus::ImageAttributes *theImageAttr, bool fDoNearestInterpolation)
    {
        if(isPrinting)
            theGdiPlusGraphics->SetPageUnit(Gdiplus::UnitPixel); // t‰h‰n asti on pelattu printatessa aina pikseli maailmassa, joten gdiplus:in pit‰‰ laittaa siihen

                                                                 // T‰ss‰ on kaikki eri mahd. interpolaatio tavat kuvapiirrossa
                                                                 // InterpolationModeInvalid, InterpolationModeDefault, InterpolationModeLowQuality, InterpolationModeHighQuality, InterpolationModeBilinear, InterpolationModeBicubic, InterpolationModeNearestNeighbor, InterpolationModeHighQualityBilinear, InterpolationModeHighQualityBicubic
                                                                 // HUOM! Vain seuraavat toimivat satel-kuvien kanssa kunnolla, kun alpha-kerroin on alle 90% eli eiv‰t tee 'reiki‰' eik‰ muitakaan v‰rimuunnoksia 
                                                                 // InterpolationModeHighQuality, InterpolationModeBicubic, InterpolationModeHighQualityBicubic
        if(fDoNearestInterpolation)
            theGdiPlusGraphics->SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor); // t‰m‰ ei taas sotke kuvia tietyiss‰ tapauksissa, kuten tekee InterpolationModeHighQuality, esim. karttakuvien p‰‰lle heitetyt layerit eiv‰t sotkeennu t‰ll‰
        else
            theGdiPlusGraphics->SetInterpolationMode(Gdiplus::InterpolationModeHighQuality); // pakko k‰ytt‰‰ hitaampia interpolaatio menetelmi‰, koska ne eiv‰t 'sotke' satel-kuvien piirtoa, kun mukana on l‰pin‰kyvyys kerroin joka on alle 90%

        Gdiplus::Status status = theGdiPlusGraphics->DrawImage(&theBitmap,
            theDestPixels,
            static_cast<Gdiplus::REAL>(theSourcePixels.Place().X()),
            static_cast<Gdiplus::REAL>(theSourcePixels.Place().Y()),
            static_cast<Gdiplus::REAL>(theSourcePixels.Width()),
            static_cast<Gdiplus::REAL>(theSourcePixels.Height()),
            Gdiplus::UnitPixel,
            theImageAttr
        );
        if(status != Gdiplus::Ok)
        {
            DWORD lastErr = GetLastError();
            if(lastErr && status)
                status = status;
        }
    }


    void DrawBitmapToDC(CDC *theDC, Gdiplus::Bitmap &theBitmap, const NFmiRect &theSourcePixels, const Gdiplus::RectF &theDestPixels, bool fDoNearestInterpolation, Gdiplus::ImageAttributes *theImageAttr, Gdiplus::Graphics *theGdiPlusGraphics)
    {
        if(theGdiPlusGraphics)
            DrawBitmapToDC(theGdiPlusGraphics, (theDC->IsPrinting() == TRUE), theBitmap, theSourcePixels, theDestPixels, theImageAttr, fDoNearestInterpolation);
        else
        {
            Gdiplus::Graphics graphics(theDC->GetSafeHdc());
            DrawBitmapToDC(&graphics, (theDC->IsPrinting() == TRUE), theBitmap, theSourcePixels, theDestPixels, theImageAttr, fDoNearestInterpolation);
        }
    }

    Gdiplus::Bitmap* CreateBitmapFromFile(const std::string &thePath, const std::string &theFileName)
    {
        if(theFileName.empty())
        {
            std::string errMsg("CreateBitmapFromFile - given image file name was empty.");
            throw std::runtime_error(errMsg);
        }
        std::string finalFileName;
        NFmiFileString fileString(theFileName);
        if(fileString.IsAbsolutePath() == false)
        {
            if(!thePath.empty())
            {
                if(NFmiFileSystem::DirectoryExists(thePath) == false)
                {
                    std::string errMsg(std::string("CreateBitmapFromFile - given path/directory doesn't exist:\n") + thePath);
                    throw std::runtime_error(errMsg);
                }
                finalFileName = thePath;
                finalFileName += kFmiDirectorySeparator; // pakko lis‰t‰ editorin tyˆhakemisto suhteelliseen tiedostonimeen, koska oletus hakemisto on saattanut muuttua esim. makro tallennuksissa jne.
            }
        }
        finalFileName += theFileName;
        return CreateBitmapFromFile(finalFileName);
    }

    Gdiplus::Bitmap* CreateBitmapFromFile(const std::string &thePathAndFileName)
    {
        if(thePathAndFileName.empty())
        {
            std::string errMsg("CreateBitmapFromFile - given image file name was empty.");
            throw std::runtime_error(errMsg);
        }
        if(NFmiFileSystem::FileExists(thePathAndFileName) == false)
        {
            std::string errMsg(std::string("CreateBitmapFromFile - given image file name doesn't exist: \n") + thePathAndFileName);
            throw std::runtime_error(errMsg);
        }

        try
        {
            CString filePathU_ = CA2T(thePathAndFileName.c_str());

            //Gdiplus::Bitmap *aBitmap = Gdiplus::Bitmap::FromFile(filePathU_.AllocSysString()); // T‰m‰ tapa lukea kuvatiedostoa johtaa siihen ett‰ kuvatiedosto on lukossa niin kauan kuin SmartMet pit‰‰ kuvaa muistissaan

            CComPtr<IStream> stream;
            HRESULT hr = SHCreateStreamOnFileEx(filePathU_, STGM_READ | STGM_SHARE_DENY_WRITE | STGM_FAILIFTHERE, FILE_ATTRIBUTE_NORMAL, FALSE, NULL, &stream); // T‰m‰ lukitsee kuvatiedoston vain kirjoitukselta (esim. deletointi) ja sekin lukko vapautuu kun tiedosto on luettu muistiin.
            if(hr != S_OK)
                throw std::runtime_error(std::string("SHCreateStreamOnFileEx '") + thePathAndFileName + "' - failed");
            Gdiplus::Bitmap *aBitmap = Gdiplus::Bitmap::FromStream(stream);

            if(aBitmap == 0)
                throw std::runtime_error(std::string("Gdiplus::Bitmap::FromFile '") + thePathAndFileName + "' - failed");
            return aBitmap;
        }
        catch(...)
        {
            // ei onnistunut tiedoston luku, heit‰ poikkeus
            std::string errMsg("CreateBitmapFromFile - cannot read the image file:\n");
            errMsg += thePathAndFileName;
            throw std::runtime_error(errMsg);
        }
    }

    void SetGdiplusAlignment(FmiDirection theAlignment, Gdiplus::StringFormat &theStringFormat)
    {
        if(theAlignment == kLeft)
        {
            theStringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
            theStringFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
        }
        else if(theAlignment == kRight)
        {
            theStringFormat.SetAlignment(Gdiplus::StringAlignmentFar);
            theStringFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
        }
        else if(theAlignment == kCenter)
        {
            theStringFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
            theStringFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
        }
        else if(theAlignment == kTopLeft)
        {
            theStringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
            theStringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);
        }
        else if(theAlignment == kTopRight)
        {
            theStringFormat.SetAlignment(Gdiplus::StringAlignmentFar);
            theStringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);
        }
        else if(theAlignment == kTopCenter)
        {
            theStringFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
            theStringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);
        }
        else if(theAlignment == kBottomLeft)
        {
            theStringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
            theStringFormat.SetLineAlignment(Gdiplus::StringAlignmentFar);
        }
        else if(theAlignment == kBottomRight)
        {
            theStringFormat.SetAlignment(Gdiplus::StringAlignmentFar);
            theStringFormat.SetLineAlignment(Gdiplus::StringAlignmentFar);
        }
        else if(theAlignment == kBottomCenter)
        {
            theStringFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
            theStringFormat.SetLineAlignment(Gdiplus::StringAlignmentFar);
        }
    }

    const Gdiplus::PointF ConvertLatlonToGdiPlusPoint(NFmiCtrlView *theView, const NFmiPoint &theLatLon)
    {
        const NFmiPoint &viewSizeInPix = theView->GetCtrlViewDocumentInterface()->MapViewSizeInPixels(theView->MapViewDescTopIndex());
        NFmiPoint xyPoint = theView->LatLonToViewPoint(theLatLon);
        return Gdiplus::PointF(static_cast<Gdiplus::REAL>(xyPoint.X()*viewSizeInPix.X()), static_cast<Gdiplus::REAL>(xyPoint.Y()*viewSizeInPix.Y()));
    }

    std::vector<Gdiplus::PointF> ConvertLatLon2GdiPoints(NFmiCtrlView *theView, const std::vector<NFmiPoint> &theLatlonPath)
    {
        std::vector<Gdiplus::PointF> gdiPoints;
        for(size_t i = 0; i < theLatlonPath.size(); i++)
        {
            gdiPoints.push_back(CtrlView::ConvertLatlonToGdiPlusPoint(theView, theLatlonPath[i]));
        }
        return gdiPoints;
    }

    const NFmiPoint ConvertGdiPlusPointToLatlon(NFmiCtrlView *theView, const Gdiplus::PointF &theGdiPlusPoint)
    {
        const NFmiPoint &viewSizeInPix = theView->GetCtrlViewDocumentInterface()->MapViewSizeInPixels(theView->MapViewDescTopIndex());
        NFmiPoint xyPoint(theGdiPlusPoint.X / viewSizeInPix.X(), theGdiPlusPoint.Y / viewSizeInPix.Y());
        return theView->ViewPointToLatLon(xyPoint);
    }

    // fillHatchStyle arvolla -1 tarkoittaa ett‰ fillauksen yhteydess‰ ei k‰ytet‰ hatchi‰.
    void DrawGdiplusCurve(Gdiplus::Graphics &theGraphics, std::vector<Gdiplus::PointF> &thePoints, const GdiPlusLineInfo &theLineInfo, bool fFill, int fillHatchStyle, bool fPrinting, std::vector<Gdiplus::REAL> *theDashPatternVector)
    {
        DrawGdiplusCurve(theGraphics, thePoints, theLineInfo, fFill, theLineInfo.Color(), fillHatchStyle, fPrinting, theDashPatternVector);
    }

    void DrawGdiplusCurve(Gdiplus::Graphics &theGraphics, std::vector<Gdiplus::PointF> &thePoints, const GdiPlusLineInfo &theLineInfo, bool fFill, const NFmiColor &fillColor, int fillHatchStyle, bool fPrinting, std::vector<Gdiplus::REAL> *theDashPatternVector)
    {
        if(thePoints.size() > 1)
        {
            bool useAlpha = (theLineInfo.Color().Alpha() > 0); // jos alpha on suurempi kuin 0, k‰ytet‰‰n sen arvoa. NFmiColor luokan default alpha 0 pit‰‰ ohittaa (ei ole mielest‰ni j‰rke‰ piirt‰‰ n‰kym‰ttˆmi‰ viivoja)
            
            if(fFill)
            {
                Gdiplus::Color gdiFillColor = NFmiColor2GdiplusColor(theLineInfo.Color(), useAlpha);
                if(fillHatchStyle >= 0 && fPrinting)
                {
                    // GDI+ on bugi eli jos printtaa hatch fillattua juttua, se on t‰ysin opaque v‰litt‰m‰tt‰ mist‰‰n v‰ri-asetuksista
                    // eli jos printtaus p‰‰ll‰ ja k‰ytet‰‰n hatchi‰, otetaan hatchays pois ja lis‰t‰‰n fillauksessa alphaa, jolloin v‰rityksest‰ tulee 'vaaleampi'
                    Gdiplus::Color fadedUsedColor(static_cast<BYTE>(gdiFillColor.GetAlpha()*0.35), gdiFillColor.GetRed(), gdiFillColor.GetGreen(), gdiFillColor.GetBlue());
                    Gdiplus::SolidBrush brush(fadedUsedColor);
                    theGraphics.FillClosedCurve(&brush, &thePoints[0], static_cast<INT>(thePoints.size()),
                        Gdiplus::FillModeWinding, theLineInfo.Tension());
                }
                else if(fillHatchStyle < 0)
                {
                    Gdiplus::SolidBrush brush(gdiFillColor);
                    theGraphics.FillClosedCurve(&brush, &thePoints[0], static_cast<INT>(thePoints.size()),
                        Gdiplus::FillModeWinding, theLineInfo.Tension());
                }
                else
                {
                    Gdiplus::Color backColor(0, 128, 128, 128); // argb
                    Gdiplus::HatchBrush brush(static_cast<Gdiplus::HatchStyle>(fillHatchStyle), gdiFillColor, backColor); // hatch-kuviota ei k‰ytet‰ ja siin‰ n‰kyy olevan
                                                                                                                       // printatessa bugi, eli backColor l‰pin‰kyvyys ei toimi (ruudulla ok)
                    theGraphics.FillClosedCurve(&brush, &thePoints[0], static_cast<INT>(thePoints.size()),
                        Gdiplus::FillModeWinding, theLineInfo.Tension());
                }

            }
            else
            {
                Gdiplus::Color lineColor = NFmiColor2GdiplusColor(theLineInfo.Color(), useAlpha);
                Gdiplus::Pen pen(lineColor, static_cast<Gdiplus::REAL>(theLineInfo.Thickness()));
                pen.SetDashStyle(static_cast<Gdiplus::DashStyle>(theLineInfo.LineType()));
                if(theLineInfo.LineType() == 5)
                { // tehd‰‰n custom tyyli
                    if(theDashPatternVector)
                        pen.SetDashPattern(&(*theDashPatternVector)[0], static_cast<INT>(theDashPatternVector->size()));
                    else
                    {
                        // Create and set an array of real numbers. (teen nelj‰ll‰ luvulla mutta tasa v‰lisen, ett‰ n‰kee ett‰ voi olla pitempikin taulukko)
                        const Gdiplus::REAL dashLen = 7.f;
                        const int dashVecSize = 4;
                        Gdiplus::REAL dashVals[dashVecSize] = {
                            dashLen,   // dash length 7
                            dashLen,   // space length 7
                            dashLen,  // dash length 7
                            dashLen };  // space length 7

                                        // Set the dash pattern for the custom dashed line.
                        pen.SetDashPattern(dashVals, dashVecSize);
                    }
                }
                pen.SetLineJoin(theLineInfo.GetLineJoin());
                // SetLineCap-metodi asettaa samalla sek‰ start, end ja dash capit
                pen.SetLineCap(theLineInfo.GetLineCap(), theLineInfo.GetLineCap(), Gdiplus::DashCapFlat);
                theGraphics.SetSmoothingMode(theLineInfo.GetSmoothingMode());
                // T‰m‰ outline koodi ei toimi kuten olisi odottanut. Joudun tekem‰‰n oman viivan outline-koodin jet-olioita varten
                if(theLineInfo.UseOutLine())
                {
                    Gdiplus::GraphicsPath aPath;
                    aPath.AddPolygon(&thePoints[0], static_cast<INT>(thePoints.size()));
                    aPath.Outline();

                    theGraphics.DrawPath(&pen, &aPath);
                }
                else

                    theGraphics.DrawCurve(&pen, &thePoints[0], static_cast<INT>(thePoints.size()), theLineInfo.Tension()); // tension-kerroin, eli k‰yr‰n piirtoa hieman pyˆristet‰‰n ns. "cardinal curve" omnaisuudella
            }
        }
    }

    // Super simppeli optimoitu multi polyline piirto:
    // 1. Kaikki viivat piirret‰‰n samalla v‰rill‰ ja viiva paksuudella (luodaan vain yksi kyn‰)
    // 2. Ei mink‰‰nlaista polygon fill tukea, eik‰ muita viiva tyyppej‰ kuin normaali yhten‰inen viiva (ei tarvitse kikkailla mm. printtauksien kanssa).
    // 3. Ei alpha optiota.
    // Palauttaa kaikkien piirrettyjen pisteiden lukum‰‰r‰n (ainoastaan debug tietona asiakkaalle)
    size_t DrawGdiplusSimpleMultiPolyLine(Gdiplus::Graphics &theGraphics, NFmiToolBox *theToolBox, const std::list<std::vector<NFmiPoint>> &theMultiPolyLine, const NFmiColor &theLineColor, int theLineThickness, const NFmiPoint &theRelativeOffSet)
    {
        size_t totalDrawedPointCount = 0;
        Gdiplus::Color lineColor = NFmiColor2GdiplusColor(theLineColor);
        Gdiplus::Pen pen(lineColor, static_cast<Gdiplus::REAL>(theLineThickness));
        pen.SetLineJoin(Gdiplus::LineJoinRound);
        for(const auto &polyLine : theMultiPolyLine)
        {
            totalDrawedPointCount += polyLine.size();
            auto gdiplusPolyLine = Relative2GdiplusPolyLine(theToolBox, polyLine, theRelativeOffSet);
            theGraphics.DrawCurve(&pen, &gdiplusPolyLine[0], static_cast<INT>(gdiplusPolyLine.size()), 0.f);
        }
        return totalDrawedPointCount;
    }

    std::unique_ptr<Gdiplus::Font> CreateFontPtr(float theFontSizeInPixels, const std::wstring& theFontNameStr, Gdiplus::FontStyle theFontStyle)
    {
        return std::make_unique<Gdiplus::Font>(theFontNameStr.c_str(), theFontSizeInPixels, theFontStyle, Gdiplus::UnitPixel);
    }

    std::unique_ptr<Gdiplus::Font> CreateFontPtr(double theFontSizeInMM, double pixelsPerMM, const std::wstring& theFontNameStr, Gdiplus::FontStyle theFontStyle)
    {
        float usedFontSizeInPixels = static_cast<float>(theFontSizeInMM * pixelsPerMM);
        return CreateFontPtr(usedFontSizeInPixels, theFontNameStr, theFontStyle);
    }


    void DrawTextToRelativeLocation(Gdiplus::Graphics &theGdiPlusGraphics, const NFmiColor &theColor, double theFontSizeInMM, const std::string &theStr, const NFmiPoint &thePlace, double pixelsPerMM, NFmiToolBox *theToolbox, const std::wstring &theFontNameStr, FmiDirection theAlingment, Gdiplus::FontStyle theFontStyle)
    {
        Gdiplus::Color usedColor(NFmiColor2GdiplusColor(theColor));
        Gdiplus::SolidBrush aBrush(usedColor);
        Gdiplus::StringFormat stringFormat;
        SetGdiplusAlignment(theAlingment, stringFormat);
        auto aFont = CreateFontPtr(theFontSizeInMM, pixelsPerMM, theFontNameStr, theFontStyle);

        std::wstring wideStr = StringToWString(theStr);

        Gdiplus::PointF aPlace = Relative2GdiplusPoint(theToolbox, thePlace);
        theGdiPlusGraphics.DrawString(wideStr.c_str(), static_cast<INT>(wideStr.size()), aFont.get(), aPlace, &stringFormat, &aBrush);
    }

    void DrawSimpleText(Gdiplus::Graphics &theGdiPlusGraphics, const NFmiColor &theColor, float theFontSizeInPixels, const std::string &theStr, const NFmiPoint &theAbsPlace, const std::wstring &theFontNameStr, FmiDirection theAlingment, const NFmiColor *theBkColor)
    {
        Gdiplus::Color usedColor(NFmiColor2GdiplusColor(theColor));
        Gdiplus::SolidBrush aBrush(usedColor);
        Gdiplus::StringFormat stringFormat;
        SetGdiplusAlignment(theAlingment, stringFormat);
        auto aFont = CreateFontPtr(theFontSizeInPixels, theFontNameStr, Gdiplus::FontStyleRegular);

        std::wstring wideStr = StringToWString(theStr);

        Gdiplus::PointF aPlace(static_cast<Gdiplus::REAL>(theAbsPlace.X()), static_cast<Gdiplus::REAL>(theAbsPlace.Y()));
        if(theBkColor)
        {
            Gdiplus::RectF boundingRect;
            theGdiPlusGraphics.MeasureString(wideStr.c_str(), static_cast<INT>(wideStr.size()), aFont.get(), aPlace, &stringFormat, &boundingRect);
            Gdiplus::Color usedBkColor(NFmiColor2GdiplusColor(*theBkColor));
            Gdiplus::SolidBrush bkFillBrush(usedBkColor);
            theGdiPlusGraphics.FillRectangle(&bkFillBrush, boundingRect);
        }
        theGdiPlusGraphics.DrawString(wideStr.c_str(), static_cast<INT>(wideStr.size()), aFont.get(), aPlace, &stringFormat, &aBrush);
    }

    void DrawLine(Gdiplus::Graphics &theGdiPlusGraphics, int x1, int y1, int x2, int y2, const NFmiColor &theColor, float thePenWidthInPixels, Gdiplus::DashStyle theDashStyle)
    {
        Gdiplus::Pen aPen(NFmiColor2GdiplusColor(theColor), thePenWidthInPixels);
        aPen.SetDashStyle(theDashStyle);
        theGdiPlusGraphics.DrawLine(&aPen, x1, y1, x2, y2);
    }

    void DrawRect(Gdiplus::Graphics &theGdiPlusGraphics, const Gdiplus::Rect &theRectInPixels, const NFmiColor &theRectFrameColor, const NFmiColor &theRectFillColor, bool doFill, bool doFrame, bool useAlphaFill, float theRectFrameWidthInPixels, Gdiplus::DashStyle theDashStyle)
    {
        Gdiplus::GraphicsPath aPath;
        aPath.AddRectangle(theRectInPixels);
        aPath.CloseFigure();

        if(doFill)
        {
            Gdiplus::SolidBrush aBrush(NFmiColor2GdiplusColor(theRectFillColor, useAlphaFill));
            theGdiPlusGraphics.FillPath(&aBrush, &aPath);
        }
        if(doFrame)
        {
            Gdiplus::Pen aPen(NFmiColor2GdiplusColor(theRectFrameColor), theRectFrameWidthInPixels);
            aPen.SetDashStyle(theDashStyle);
            theGdiPlusGraphics.DrawPath(&aPen, &aPath);
        }
    }

    void DrawPath(Gdiplus::Graphics &theGdiPlusGraphics, const Gdiplus::GraphicsPath &thePathInPixels, const NFmiColor &theFrameColor, const NFmiColor &theFillColor, bool doFill, bool doFrame, bool useAlphaFill, float theFrameWidthInPixels)
    {
        if(doFill)
        {
            Gdiplus::SolidBrush aBrush(NFmiColor2GdiplusColor(theFillColor, useAlphaFill));
            theGdiPlusGraphics.FillPath(&aBrush, &thePathInPixels);
        }
        if(doFrame)
        {
            Gdiplus::Pen aPen(NFmiColor2GdiplusColor(theFrameColor), theFrameWidthInPixels);
            theGdiPlusGraphics.DrawPath(&aPen, &thePathInPixels);
        }
    }

    void DrawAnimationButton(const NFmiRect &theRect, Gdiplus::Bitmap *theButtonImage, Gdiplus::Graphics *theGdiPlusGraphics, NFmiToolBox &theToolBox, bool isPrinting, const NFmiPoint &theViewSizeInPixels, Gdiplus::REAL theAlpha, bool fForceSize, bool fDoNearestInterpolation)
    {
        if(theButtonImage == 0)
        {
            Gdiplus::GraphicsPath aPath;
            NFmiColor buttonRectColor(0.f, 0.f, 0.0f);
            Gdiplus::Pen aPen(NFmiColor2GdiplusColor(buttonRectColor), 1);
            MakePathFromRect(aPath, &theToolBox, theRect);
            theGdiPlusGraphics->DrawPath(&aPen, &aPath);
        }
        else
        {
            NFmiPoint bitmapSize(theButtonImage->GetWidth(), theButtonImage->GetHeight());
            NFmiPoint destBitmapSize(bitmapSize);
            if(isPrinting || fForceSize)
            {
                destBitmapSize.X(theToolBox.HX(theRect.Width()));
                destBitmapSize.Y(theToolBox.HY(theRect.Height()));
            }
            NFmiPoint startPoint(CtrlViewUtils::ConvertPointFromRect1ToRect2(theRect.TopLeft(), NFmiRect(0, 0, 1, 1), NFmiRect(NFmiPoint(0, 0), theViewSizeInPixels)));
            Gdiplus::RectF destRect(static_cast<Gdiplus::REAL>(startPoint.X()), static_cast<Gdiplus::REAL>(startPoint.Y()), static_cast<Gdiplus::REAL>(destBitmapSize.X()), static_cast<Gdiplus::REAL>(destBitmapSize.Y()));
            NFmiRect sourceRect(0, 0, bitmapSize.X(), bitmapSize.Y());
            DrawBitmapToDC(theToolBox.GetDC(), *theButtonImage, sourceRect, destRect, theAlpha, fDoNearestInterpolation, theGdiPlusGraphics);
        }
    }

    void DrawImageButton(const CRect &theAbsRect, Gdiplus::Bitmap *theButtonImage, Gdiplus::Graphics *theGdiPlusGraphics, Gdiplus::REAL theAlpha, bool isPrinting)
    {
        if(theButtonImage == 0)
        {
            NFmiColor buttonRectColor(0.f, 0.f, 0.0f);
            Gdiplus::Pen aPen(NFmiColor2GdiplusColor(buttonRectColor), 1);
            theGdiPlusGraphics->DrawRectangle(&aPen, CRect2GdiplusRect(theAbsRect));
        }
        else
        {
            NFmiPoint bitmapSize(theButtonImage->GetWidth(), theButtonImage->GetHeight());
            NFmiPoint destBitmapSize(theAbsRect.Width(), theAbsRect.Height());

            NFmiPoint startPoint(theAbsRect.left, theAbsRect.top);
            Gdiplus::RectF destRect(static_cast<Gdiplus::REAL>(startPoint.X()), static_cast<Gdiplus::REAL>(startPoint.Y()), static_cast<Gdiplus::REAL>(destBitmapSize.X()), static_cast<Gdiplus::REAL>(destBitmapSize.Y()));
            NFmiRect sourceRect(0, 0, bitmapSize.X(), bitmapSize.Y());
            DrawBitmap(*theButtonImage, sourceRect, destRect, theAlpha, theGdiPlusGraphics, isPrinting, true);
        }
    }


    void DrawBitmap(Gdiplus::Bitmap &theBitmap, const NFmiRect &theSourcePixels, const Gdiplus::RectF &theDestPixels, Gdiplus::REAL theAlpha, Gdiplus::Graphics *theGdiPlusGraphics, bool isPrinting, bool fDoNearestInterpolation)
    {
        Gdiplus::ImageAttributes imageAttr;
        Gdiplus::ColorMatrix colorMatrix = {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, theAlpha, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f
        };
        imageAttr.SetColorMatrix(&colorMatrix);
        DrawBitmapToDC(theGdiPlusGraphics, isPrinting, theBitmap, theSourcePixels, theDestPixels, &imageAttr, fDoNearestInterpolation);
    }

    // Piirt‰‰ 3D-laatkon. Laatikon vasen ja yl‰ reunat piirret‰‰n vaaleammalla ja oikea ja ala reunat piirret‰‰n tummemmalla.
    // Keskiosa piirret‰‰n halutulla v‰rill‰. Reunoihin saa halutun paksuuden. Tummennus ja vaalennus tapahtuu halutulla prosentilla.
    // Testattu vain 2 pikselin paksuisilla reunoilla.
    // HUOM! kun theEdgeWidth = 1, GDI+ -piirto toimi mielest‰ni v‰‰rin ja jouduin tekem‰‰n pariin kohtaan yhden pikselin siito viilauluja.
    void Draw3DRect(Gdiplus::Graphics *theGdiPlusGraphics, const Gdiplus::RectF &theRect, const NFmiColor &theColor, float theEdgeColorFactor, float theEdgeWidth)
    {
        Gdiplus::RectF theRectX(theRect);
        if(theEdgeWidth == 1.f)  // "yhden levyiset reunat aiheuttivat ongelmia" -fixi
            theRectX.Width -= 1.f;

        NFmiColor darkColor = NFmiColorSpaces::GetBrighterColor(theColor, -theEdgeColorFactor);
        Gdiplus::Pen darkPen(NFmiColor2GdiplusColor(darkColor), theEdgeWidth);
        darkPen.SetAlignment(Gdiplus::PenAlignmentInset);
        NFmiColor lightColor = NFmiColorSpaces::GetBrighterColor(theColor, theEdgeColorFactor);
        Gdiplus::Pen lightPen(NFmiColor2GdiplusColor(lightColor), theEdgeWidth);
        lightPen.SetAlignment(Gdiplus::PenAlignmentInset);
        Gdiplus::SolidBrush normalBrush(NFmiColor2GdiplusColor(theColor, false));

        // Piirret‰‰n koko alueen ymp‰rˆiv‰ laatikko tummalla v‰rill‰ pohjalle.
        // T‰ll‰ saadaan oikea ja alareuna tummemmalla v‰rill‰.
        Gdiplus::RectF usedRect = theRectX;
        theGdiPlusGraphics->DrawRectangle(&darkPen, usedRect);

        // Pienennet‰‰n laatikkoa reunuksen verran oikeasta ja ala reunoista.
        // Piirret‰‰n t‰m‰ laatikko vaalealla v‰rilla, jolloin saadaan vaalea reuna vasen ja yl‰ reunoista..
        usedRect.Width -= theEdgeWidth;
        usedRect.Height -= theEdgeWidth;
        theGdiPlusGraphics->DrawRectangle(&lightPen, usedRect);

        // T‰m‰ on kikka vitonen. Haluan piirt‰‰ laatikon l‰pi kulmasta kulmaan tietyst‰ kohtaa vaalealla v‰rille 
        // antialisoidun viivan. T‰m‰n takoitus on antaa laatikon vaalean ja tumman risteykseen pieni vino muutos alue.
        // T‰t‰ aluetta ei juuri erota pienill‰ reuna leveyksill‰, mutta isommilla siit‰ pit‰isi tulla 'kiva' valaistus efekti.
        {
            // otetaan originaali laatikon kulmapisteet ja siirret‰‰n niit‰ hieman sivulle. Piirret‰‰n niiden avulla 
            // viiva vaalealla v‰rill‰ antialisoitu viiva poikki laatikon.
            Gdiplus::PointF bottomLeftPoint(theRectX.GetLeft(), theRectX.GetBottom());
            Gdiplus::PointF topRightPoint(theRectX.GetRight(), theRectX.GetTop());
            float usedPenWidth = theEdgeWidth;
            if(theEdgeWidth > 1.f) // t‰m‰ ei ollut yhden yhden levyiset reunat aiheuttivat ongelmia -fixi, vaan t‰m‰n viilauksen ymm‰rr‰n viivan piirto geometrioista
            {
                bottomLeftPoint.Y -= theEdgeWidth;
                topRightPoint.X -= theEdgeWidth;
                usedPenWidth -= 1.f; // poikkiviivasta pit‰‰ tehd‰ yhden ohuempi kuin laatikon reunoista, paitsi kun se on jo 1
            }
            Gdiplus::Pen lightPen2(NFmiColor2GdiplusColor(lightColor), usedPenWidth);
            lightPen2.SetAlignment(Gdiplus::PenAlignmentCenter);
            lightPen2.SetStartCap(Gdiplus::LineCapTriangle);
            lightPen2.SetEndCap(Gdiplus::LineCapTriangle);
            Gdiplus::SmoothingMode oldMode = theGdiPlusGraphics->GetSmoothingMode();
            theGdiPlusGraphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
            theGdiPlusGraphics->DrawLine(&lightPen2, bottomLeftPoint, topRightPoint);
            theGdiPlusGraphics->SetSmoothingMode(oldMode);
        }

        // Lopuksi fillataan laatikko sis‰ alueelle halutulla perusv‰rill‰.
        usedRect = theRectX; // sijoitetaan originaal laatikko, ett‰ saadaan helpommat laskut
        usedRect.Inflate(-theEdgeWidth, -theEdgeWidth);
        if(theEdgeWidth == 1.f)  // "yhden levyiset reunat aiheuttivat ongelmia" -fixi
        {
            usedRect.Width += 1.f;
            usedRect.Height += 1.f;
        }
        theGdiPlusGraphics->FillRectangle(&normalBrush, usedRect); // fillataan normaali v‰rill‰ laatikon keskus

    }
}
