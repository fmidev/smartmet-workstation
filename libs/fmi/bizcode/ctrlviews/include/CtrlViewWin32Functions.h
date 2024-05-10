#pragma once

#include "stdafx.h"
#include <utility>
#include "NFmiColor.h"
#include "SmartMetOpenUrlAction.h"
#include "WaitCursorHelper.h"

class NFmiToolBox;

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
    // HUOM!! printtauksen yhteydessä kutsu ensin DC:n asetus ja sitten printinfon!!!
    void SetToolsDCs(CDC* theDC, CWnd *theView, NFmiToolBox *theToolBox, bool fToolMasterAvailable); // tämä asettaa kaikki kerralla
    void SetToolBoxsDC(CDC* theDC, NFmiToolBox *theToolBox);
    void SetToolMastersDC(CDC* theDC, CWnd *theView, bool fToolMasterAvailable);

    // uudet versiot SetToolsDC:stä, jossa haluttu piirtoalue annetaan parametrina, tällöin
    // printti alueen erilainen käsittely ei jää huomiotta
    void SetToolsDCs(CDC* theDC, NFmiToolBox *theToolBox, const CRect &theClientRect, bool fToolMasterAvailable); // tämä asettaa kaikki kerralla
    void SetToolBoxsDC(CDC* theDC, NFmiToolBox *theToolBox, const CRect &theClientRect);
    void SetToolMastersDC(CDC* theDC, const CRect &theClientRect);

    void OpenWantedUrlInBrowser(SmartMetOpenUrlAction currentOpenUrlAction, int mapViewDescTopIndex);
    SmartMetOpenUrlAction GetOpenUrlKeyPressedState();
    bool HandleUrlMouseActions(SmartMetOpenUrlAction currentOpenUrlAction);

    template<typename SearchFunction>
    BOOL DoReturnKeyOperation(MSG* pMsg, SearchFunction& searchFunction)
    {
        // Erikoiskäsittely, jos kyse RETURN napin painalluksesta (alas/ylös)
        auto messageType = pMsg->message;
        if((WM_KEYDOWN == messageType || WM_KEYUP == messageType) && VK_RETURN == pMsg->wParam)
        {
            // Toimitaan lopulta vain silloin kun RETURN nappi päätetään ylös
            if(WM_KEYUP == messageType)
            {
                WaitCursorHelper waitCursorHelper(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation()->ShowWaitCursorWhileDrawingView());
                searchFunction();
            }
            return TRUE; // Palautetaan true, jotta tätä messagea ei käsitellä enää muualla
        }
        return FALSE; // Palautetaan false, että message käsitellään virallisia teitä pitkin
    }

    // SimpleDeviceContextHandler luokka tekee ikävästä normi CDC handlauksesta siedettävämpää...
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


    // DeviceContextHandler luokka tekee ikävästä Fmi CDC handlauksesta siedettävämpää...
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
