#pragma once

#ifndef UNIX

#include "stdafx.h"
#include <utility>
#include "NFmiColor.h"
#include "SmartMetOpenUrlAction.h"
#include "WaitCursorHelper.h"

class NFmiToolBox;
class NFmiRect;

class NFmiColorButtonDrawingData
{
public:
    CWnd* view_ = nullptr;
    NFmiColor nfmiColor_;
    bool isNfmiColorUsed_ = false;
    COLORREF &color_;
    CBitmap** bitmap_ = nullptr;
    CRect &rect_;
    CButton& button_;
    // first = colorIsTransparent
    // second = colorIsDisabled
    std::pair<bool, bool> colorOptions_ = std::pair<bool, bool>(false, false);

    NFmiColorButtonDrawingData(CWnd* view, COLORREF& color, CBitmap** bitmap, CRect& rect, CButton& button, const std::pair<bool, bool>& colorOptions);
    NFmiColorButtonDrawingData(CWnd* view, const NFmiColor& nfmiColor, COLORREF& color, CBitmap** bitmap, CRect& rect, CButton& button, const std::pair<bool, bool>& colorOptions);
    NFmiColorButtonDrawingData(CWnd* view, const NFmiColor& nfmiColor, COLORREF& color, CBitmap** bitmap, CRect& rect, CButton& button);
    NFmiColorButtonDrawingData(CWnd* view, COLORREF& color, CBitmap** bitmap, CRect& rect, CButton& button);
    NFmiColorButtonDrawingData(CWnd* view, COLORREF& color, CRect& rect);

    void SetNfmiColor(const NFmiColor& nfmiColor);
};

// Apufunktioita jotka ovat riippuvaisia win32/mfc jutuista
// ja toolboxista ja muusta kivasta.
namespace CtrlView
{
    void DestroyBitmap(CBitmap **bitmap, bool deleteOldBitmap = true);
    void MakeCombatibleBitmap(CWnd *theView, CBitmap **theMemoryBitmap, int cx = 0, int cy = 0);
    CBitmap* CreateColorBitmap(int x, int y, const NFmiColorButtonDrawingData& drawingData);
    void ColorButtonDraw(NFmiColorButtonDrawingData& drawingData);
    void ColorButtonPressed(NFmiColorButtonDrawingData& drawingData);
    void InitialButtonColorUpdate(NFmiColorButtonDrawingData& drawingData);

    // asettaa toolmasterin ja toolboxin DC:t
    // esim. ennen piirto tai ennen kuin tarkastellaan hiiren klikkausta ruudulta
    // HUOM!! printtauksen yhteydess� kutsu ensin DC:n asetus ja sitten printinfon!!!
    void SetToolsDCs(CDC* theDC, CWnd *theView, NFmiToolBox *theToolBox, bool fToolMasterAvailable); // t�m� asettaa kaikki kerralla
    void SetToolBoxsDC(CDC* theDC, NFmiToolBox *theToolBox);
    void SetToolMastersDC(CDC* theDC, CWnd *theView, bool fToolMasterAvailable);

    // uudet versiot SetToolsDC:st�, jossa haluttu piirtoalue annetaan parametrina, t�ll�in
    // printti alueen erilainen k�sittely ei j�� huomiotta
    void SetToolsDCs(CDC* theDC, NFmiToolBox *theToolBox, const CRect &theClientRect, bool fToolMasterAvailable); // t�m� asettaa kaikki kerralla
    void SetToolBoxsDC(CDC* theDC, NFmiToolBox *theToolBox, const CRect &theClientRect);
    void SetToolMastersDC(CDC* theDC, const CRect &theClientRect);

    void OpenWantedUrlInBrowser(SmartMetOpenUrlAction currentOpenUrlAction, int mapViewDescTopIndex);
    SmartMetOpenUrlAction GetOpenUrlKeyPressedState();
    bool HandleUrlMouseActions(SmartMetOpenUrlAction currentOpenUrlAction);
    void PlaceBoxIntoFrame(NFmiRect& theObjectBoxAbsolute, const NFmiRect& theViewFrameRelative, NFmiToolBox* theToolbox, FmiDirection theWantedLocation);

    template<typename SearchFunction>
    BOOL DoReturnKeyOperation(MSG* pMsg, SearchFunction& searchFunction)
    {
        // Erikoisk�sittely, jos kyse RETURN napin painalluksesta (alas/yl�s)
        auto messageType = pMsg->message;
        if((WM_KEYDOWN == messageType || WM_KEYUP == messageType) && VK_RETURN == pMsg->wParam)
        {
            // Toimitaan lopulta vain silloin kun RETURN nappi p��tet��n yl�s
            if(WM_KEYUP == messageType)
            {
                WaitCursorHelper waitCursorHelper(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation()->ShowWaitCursorWhileDrawingView());
                searchFunction();
            }
            return TRUE; // Palautetaan true, jotta t�t� messagea ei k�sitell� en�� muualla
        }
        return FALSE; // Palautetaan false, ett� message k�sitell��n virallisia teit� pitkin
    }

    // SimpleDeviceContextHandler luokka tekee ik�v�st� normi CDC handlauksesta siedett�v�mp��...
    class SimpleDeviceContextHandler
    {
        CWnd* window_ = nullptr;
        CDC* dcPtr_ = nullptr;
    public:
        SimpleDeviceContextHandler(CWnd* window)
            : window_(window)
        {
            if(window_)
            {
                dcPtr_ = window_->GetDC();
            }
        }

        ~SimpleDeviceContextHandler()
        {
            if(window_ && dcPtr_)
                window_->ReleaseDC(dcPtr_);
        }

        CDC* GetDcFromHandler() { return dcPtr_; }
    };


    // DeviceContextHandler luokka tekee ik�v�st� Fmi CDC handlauksesta siedett�v�mp��...
    template<typename SetToolsDCsView>
    class DeviceContextHandler
    {
        SetToolsDCsView* window_ = nullptr;
        CDC* dcPtr_ = nullptr;
    public:
        DeviceContextHandler(SetToolsDCsView* window)
        : window_(window)
        {
            if(window_)
            {
                dcPtr_ = window_->GetDC();
                window->SetToolsDCs(dcPtr_);
            }
        }

        ~DeviceContextHandler()
        {
            if(window_ && dcPtr_)
                window_->ReleaseDC(dcPtr_);
        }

        CDC* GetDcFromHandler() { return dcPtr_; }
    };
}

#else  // UNIX - Windows-only header; Linux implementation will use Qt
#endif // UNIX
