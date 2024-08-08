#include "CtrlViewWin32Functions.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "NFmiToolBox.h"
#include "ToolMasterHelperFunctions.h"
#include "CtrlViewDocumentInterface.h"
#include "NFmiMouseClickUrlActionData.h"
#include "CtrlViewKeyboardFunctions.h"
#include "NFmiApplicationWinRegistry.h"

#include <stdexcept>

namespace
{
    CButton g_DummyButton;

    void DrawColorButtonBitmap(CDC &dc, int x, int y, const NFmiColorButtonDrawingData& drawingData)
    {
        bool colorIsTransparent = drawingData.colorOptions_.first;
        bool colorIsdisabled = drawingData.colorOptions_.second;
        auto whiteColorRef = RGB(255, 255, 255);
        auto redColorRef = RGB(255, 0, 0);
        CRect frameRect(0, 0, x, y);
        if(colorIsdisabled) // Disablointi on prioreteettill‰ 1.
        {
            // Jos v‰ri oli disabloitu, piirret‰‰n valkoinen pohja ja siihen
            // p‰‰lle punaisilla viivoilla piirretty rasti (X).
            dc.FillSolidRect(&frameRect, whiteColorRef);
            CPen redPen(PS_SOLID, 2, redColorRef);
            auto oldPen = dc.SelectObject(&redPen);
            dc.MoveTo(1, 1);
            dc.LineTo(x-1, y-1);
            dc.MoveTo(x-1, 1);
            dc.LineTo(1, y-1);
            dc.SelectObject(oldPen);
        }
        else if(colorIsTransparent) // L‰pin‰kyvyys on prioreteettill‰ 2.
        {
            // Jos v‰ri oli l‰pin‰kyv‰, piirret‰‰n valkoinen pohja ja siihen
            // p‰‰lle katkoviivoilla piirretty punainen laatikko.
            // Sen j‰lkeen piirret‰‰n v‰littu v‰ri viel‰ pikkuisella laatikolla 
            // (Transparentin v‰ri‰ k‰ytet‰‰n kuitenkin v‰rien blendailuissa)

            // 1. Valkoinen pohja
            dc.FillSolidRect(&frameRect, whiteColorRef);
            // 2. Punainen katkoviiva laatikko
            CPen dashedRedPen(PS_DOT, 1, redColorRef);
            auto oldPen = dc.SelectObject(&dashedRedPen);
            dc.Rectangle(&frameRect);
            // 3. Valitun v‰rinen fillattu pienempi laatikko
            frameRect.InflateRect(-2, -2);
            dc.FillSolidRect(&frameRect, drawingData.color_);
            dc.SelectObject(oldPen);
        }
        else // Normipiirto (fillattu laatikko) on prioriteetill‰ 3.
            dc.FillSolidRect(&frameRect, drawingData.color_);
    }

    NFmiPoint GetWantedRelativePlace(const NFmiRect& theViewFrameRelative, FmiDirection theWantedLocation)
    {
        switch(theWantedLocation)
        {
        case kBottomLeft:
            return theViewFrameRelative.BottomLeft();
        case kBottomRight:
            return theViewFrameRelative.BottomRight();
        case kTopLeft:
            return theViewFrameRelative.TopLeft();
        case kTopRight:
            return theViewFrameRelative.TopRight();
        case kBottomCenter:
            return NFmiPoint(theViewFrameRelative.Center().X(), theViewFrameRelative.Bottom());
        case kTopCenter:
            return NFmiPoint(theViewFrameRelative.Center().X(), theViewFrameRelative.Top());
        default:
            return theViewFrameRelative.BottomLeft();
        }
    }
}

NFmiColorButtonDrawingData::NFmiColorButtonDrawingData(CWnd* view, COLORREF& color, CBitmap** bitmap, CRect& rect, CButton& button, const std::pair<bool, bool>& colorOptions)
:view_(view)
,nfmiColor_()
,color_(color)
,bitmap_(bitmap)
,rect_(rect)
,button_(button)
,colorOptions_(colorOptions)
{
}

NFmiColorButtonDrawingData::NFmiColorButtonDrawingData(CWnd* view, const NFmiColor& nfmiColor, COLORREF& color, CBitmap** bitmap, CRect& rect, CButton& button, const std::pair<bool, bool>& colorOptions)
:view_(view)
,nfmiColor_(nfmiColor)
,isNfmiColorUsed_(true)
,color_(color)
,bitmap_(bitmap)
,rect_(rect)
,button_(button)
,colorOptions_(colorOptions)
{
}

NFmiColorButtonDrawingData::NFmiColorButtonDrawingData(CWnd* view, const NFmiColor& nfmiColor, COLORREF& color, CBitmap** bitmap, CRect& rect, CButton& button)
:view_(view)
,nfmiColor_(nfmiColor)
,isNfmiColorUsed_(true)
,color_(color)
,bitmap_(bitmap)
,rect_(rect)
,button_(button)
{
}

NFmiColorButtonDrawingData::NFmiColorButtonDrawingData(CWnd* view, COLORREF& color, CBitmap** bitmap, CRect& rect, CButton& button)
:view_(view)
,nfmiColor_()
,color_(color)
,bitmap_(bitmap)
,rect_(rect)
,button_(button)
{
}

NFmiColorButtonDrawingData::NFmiColorButtonDrawingData(CWnd* view, COLORREF& color, CRect& rect)
:view_(view)
,nfmiColor_()
,color_(color)
,rect_(rect)
,button_(g_DummyButton)
{
}

void NFmiColorButtonDrawingData::SetNfmiColor(const NFmiColor& nfmiColor)
{
    nfmiColor_ = nfmiColor;
    isNfmiColorUsed_ = true;
}

namespace CtrlView
{
    void DestroyBitmap(CBitmap **bitmap, bool deleteOldBitmap)
    {
        CBitmap *singlePointerBitmap = *bitmap;
        if(singlePointerBitmap)
        {
            singlePointerBitmap->DeleteObject();
            if(deleteOldBitmap)
            {
                delete singlePointerBitmap;
            }
            singlePointerBitmap = nullptr;
        }
    }

    void MakeCombatibleBitmap(CWnd *theView, CBitmap **theMemoryBitmap, int cx, int cy)
    {
        if(theView == 0)
            throw std::runtime_error("Error in CtrlView::MakeCombatibleBitmap, theView was NULL pointer.");
        if(*theMemoryBitmap)
            (*theMemoryBitmap)->DeleteObject();
        else
            *theMemoryBitmap = new CBitmap;
        // Muista ett‰ CClientDC:n kanssa ei tehd‰ lopuksi DeleteDC -kutsua kuten muiden Device Context otusten kanssa tehd‰‰n.
        CClientDC dc(theView);
        CRect clientArea(0, 0, cx, cy);
        if(cx == 0 && cy == 0)
            theView->GetClientRect(&clientArea);

        (*theMemoryBitmap)->CreateCompatibleBitmap(&dc, clientArea.Width()
            , clientArea.Height());
        ASSERT((*theMemoryBitmap)->m_hObject != 0);
    }

    CBitmap* CreateColorBitmap(int x, int y, const NFmiColorButtonDrawingData& drawingData)
    {
        CClientDC clientDC(drawingData.view_);
        CDC dc;
        dc.CreateCompatibleDC(&clientDC);

        CBitmap *bitmap = new CBitmap;
        bitmap->CreateCompatibleBitmap(&clientDC, x, y);
        CBitmap *tmpBitmap = dc.SelectObject(bitmap);
        ::DrawColorButtonBitmap(dc, x, y, drawingData);
        bitmap = dc.SelectObject(tmpBitmap);
        dc.DeleteDC();
        return bitmap;
    }

    void ColorButtonDraw(NFmiColorButtonDrawingData& drawingData)
    {
        DestroyBitmap(drawingData.bitmap_, true);
        const auto& rect = drawingData.rect_;
        *drawingData.bitmap_ = CreateColorBitmap(rect.Width() - 6, rect.Height() - 6, drawingData);
        drawingData.button_.SetBitmap(**drawingData.bitmap_);
    }

    void ColorButtonPressed(NFmiColorButtonDrawingData& drawingData)
    {
        CColorDialog dlg(drawingData.color_, 0, drawingData.view_);
        if(dlg.DoModal() == IDOK)
        {
            drawingData.color_ = dlg.GetColor();
            ColorButtonDraw(drawingData);
        }
    }

    void InitialButtonColorUpdate(NFmiColorButtonDrawingData& drawingData)
    {
        if(drawingData.isNfmiColorUsed_)
        {
            drawingData.color_ = CtrlView::Color2ColorRef(drawingData.nfmiColor_);
        }
        drawingData.button_.GetWindowRect(drawingData.rect_);
        ColorButtonDraw(drawingData);
    }

    // asettaa toolmasterin ja toolboxin DC:t
    // esim. ennen piirto tai ennen kuin tarkastellaan hiiren klikkausta ruudulta
    // HUOM!! printtauksen yhteydess‰ kutsu ensin DC:n asetus ja sitten printinfon!!!
    void SetToolsDCs(CDC* theDC, CWnd *theView, NFmiToolBox *theToolBox, bool fToolMasterAvailable)
    {
        SetToolBoxsDC(theDC, theToolBox);
        SetToolMastersDC(theDC, theView, fToolMasterAvailable);
    }

    // toolboxin DC:n
    void SetToolBoxsDC(CDC* theDC, NFmiToolBox *theToolBox)
    {
        if(theToolBox)
            theToolBox->SetDC(theDC);
    }
    // toolmasterin DC:n
    void SetToolMastersDC(CDC* theDC, CWnd *theView, bool fToolMasterAvailable)
    {
        if(fToolMasterAvailable)
        {
            CRect clientRect;
            theView->GetClientRect(&clientRect);
            SetToolMastersDC(theDC, clientRect);
        }
    }

    // uudet versiot SetToolsDC:st‰, jossa haluttu piirtoalue annetaan parametrina, t‰llˆin
    // printti alueen erilainen k‰sittely ei j‰‰ huomiotta
    void SetToolsDCs(CDC* theDC, NFmiToolBox *theToolBox, const CRect &theClientRect, bool fToolMasterAvailable) // t‰m‰ asettaa kaikki kerralla
    {
        SetToolBoxsDC(theDC, theToolBox, theClientRect);
        if(fToolMasterAvailable)
            SetToolMastersDC(theDC, theClientRect);
    }

    void SetToolBoxsDC(CDC* theDC, NFmiToolBox *theToolBox, const CRect &theClientRect)
    {
        if(theToolBox)
        {
            theToolBox->SetDC(theDC, false);
            theToolBox->SetClientRect(theClientRect);
        }
    }

    void SetToolMastersDC(CDC* theDC, const CRect &theClientRect)
    {
        Toolmaster::SetToolMastersDC(theDC, theClientRect);
    }

    void OpenWantedUrlInBrowser(SmartMetOpenUrlAction currentOpenUrlAction, int mapViewDescTopIndex)
    {
        if(currentOpenUrlAction != SmartMetOpenUrlAction::None)
        {
            auto* ctrlViewInterface = CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation();
            auto pointedLatlonPoint = ctrlViewInterface->ToolTipLatLonPoint();
            auto mapTime = ctrlViewInterface->CurrentTime(mapViewDescTopIndex);
            float currentRadiusInKm = float(ctrlViewInterface->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapViewRangeMeter().CalculateStartEndDistanceInMeters() / 1000.);
            if(currentRadiusInKm < 0)
            {
                // Jos distance measure tyˆkalua ei ole viel‰ k‰ytetty, palauttaa se mitatun matkan pituudeksi -1:n,
                // t‰llˆin laitetaan vain joku default arvo eli vaikka 50 km
                currentRadiusInKm = 50.f;
            }
            auto baseUrl = ctrlViewInterface->MouseClickUrlActionData().GetMouseActionUrl(currentOpenUrlAction, pointedLatlonPoint, mapTime, currentRadiusInKm);
            if(!baseUrl.empty())
            {
                CString urlToOpen = CA2T(baseUrl.c_str());
                // Open the URL in the default web browser
                ShellExecute(NULL, _T("open"), urlToOpen, NULL, NULL, SW_SHOWNORMAL);
            }
        }
    }

    SmartMetOpenUrlAction GetOpenUrlKeyPressedState()
    {
        // 1. Jos ALT nappi pohjassa
        if(CtrlView::IsKeyboardKeyDown(VK_MENU))
        {
            // 2. Etsi ensimm‰inen n‰pp‰in joka on pohjassa ja joka on mapattu url-actionille
            for(const auto& mappedActionKeyPair : CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->MouseClickUrlActionData().OpenUrlActionKeyMappings())
            {
                if(CtrlView::IsKeyboardKeyDown(mappedActionKeyPair.first))
                {
                    return mappedActionKeyPair.second;
                }
            }
        }
        // 3. Muuten palautetaan ei-url-action moodiin
        return SmartMetOpenUrlAction::None;
    }

    bool HandleUrlMouseActions(SmartMetOpenUrlAction currentOpenUrlAction)
    {
        return (currentOpenUrlAction != SmartMetOpenUrlAction::None);
    }

    // Sijoita annettu theObjectBoxAbsolute (param/time laatikko) haluttuun kohtaan ikkunaa (theViewFrameRelative).
    void PlaceBoxIntoFrame(NFmiRect& theObjectBoxAbsolute, const NFmiRect& theViewFrameRelative, NFmiToolBox* theToolbox, FmiDirection theWantedLocation)
    {
        NFmiPoint relativePlace = ::GetWantedRelativePlace(theViewFrameRelative, theWantedLocation);

        CPoint absolutePlace;
        theToolbox->ConvertPoint(relativePlace, absolutePlace);
        theObjectBoxAbsolute.Place(NFmiPoint(absolutePlace.x, absolutePlace.y));

        // Riippuen halutusta paikasta, pit‰‰ absoluuttista laatikkoa viel‰ s‰‰t‰‰ tietyiss‰ tilanteissa

        // 1. Jos ollaan alareunassa, pit‰‰ laatikkoa nostaa itsens‰ korkeuden verran
        if(theWantedLocation == kBottomLeft || theWantedLocation == kBottomRight || theWantedLocation == kBottomCenter)
            theObjectBoxAbsolute.Place(NFmiPoint(theObjectBoxAbsolute.Left(), theObjectBoxAbsolute.Top() - theObjectBoxAbsolute.Height()));

        // 2. Jos ollaan oikeassa reunassa, pit‰‰ laatikkoa siirt‰‰ vasemmalle itsens‰ leveyden verran
        if(theWantedLocation == kBottomRight || theWantedLocation == kTopRight)
            theObjectBoxAbsolute.Place(NFmiPoint(theObjectBoxAbsolute.Left() - theObjectBoxAbsolute.Width(), theObjectBoxAbsolute.Top()));

        // 3. Jos ollaan vaaka suunnassa keskell‰, pit‰‰ laatikon keskipiste siirt‰‰ x-suunnassa puoli laatikon leveytt‰ vasemmalle
        if(theWantedLocation == kBottomCenter || theWantedLocation == kTopCenter)
            theObjectBoxAbsolute.Place(NFmiPoint(theObjectBoxAbsolute.Left() - (theObjectBoxAbsolute.Width() / 2.), theObjectBoxAbsolute.Top()));
    }

} // namespace CtrlView
