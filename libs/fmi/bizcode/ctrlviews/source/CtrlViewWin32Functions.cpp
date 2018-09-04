#include "CtrlViewWin32Functions.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "NFmiToolBox.h"
#include <agx\agx.h>

#include <stdexcept>

namespace CtrlView
{
    void DestroyBitmap(CBitmap **bitmap)
    {
        CBitmap *singlePointerBitmap = *bitmap;
        if(singlePointerBitmap)
        {
            singlePointerBitmap->DeleteObject();
            delete singlePointerBitmap;
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
        CClientDC dc(theView);
        CRect clientArea(0, 0, cx, cy);
        if(cx == 0 && cy == 0)
            theView->GetClientRect(&clientArea);

        (*theMemoryBitmap)->CreateCompatibleBitmap(&dc, clientArea.Width()
            , clientArea.Height());
        ASSERT((*theMemoryBitmap)->m_hObject != 0);
    }

    CBitmap* CreateColorBitmap(CWnd *view, COLORREF color, int x, int y)
    {
        CClientDC clientDC(view);
        CDC dc;
        dc.CreateCompatibleDC(&clientDC);

        CBitmap *bitmap = new CBitmap;
        bitmap->CreateCompatibleBitmap(&clientDC, x, y);
        CBitmap *tmpBitmap = dc.SelectObject(bitmap);

        dc.FillSolidRect(0, 0, x, y, color);
        bitmap = dc.SelectObject(tmpBitmap);
        dc.DeleteDC();
        return bitmap;
    }

    void ColorButtonPressed(CWnd *view, COLORREF& color, CBitmap** bitmap, CRect& rect, CButton& button)
    {
        CColorDialog dlg(color, 0, view);
        if(dlg.DoModal() == IDOK)
        {
            color = dlg.GetColor();

            if((*bitmap))
                (*bitmap)->DeleteObject();
            delete *bitmap;
            *bitmap = CreateColorBitmap(view, color, rect.Width() - 6, rect.Height() - 6);
            button.SetBitmap(**bitmap);
        }
    }

    void InitialButtonColorUpdate(CWnd *view, COLORREF& color, CBitmap** bitmap, CRect& rect, CButton& button)
    {
        button.GetWindowRect(rect);
        if((*bitmap))
            (*bitmap)->DeleteObject();
        delete *bitmap;
        *bitmap = CreateColorBitmap(view, color, rect.Width() - 6, rect.Height() - 6);
        button.SetBitmap(**bitmap);
    }

    void InitialButtonColorUpdate(CWnd *view, const NFmiColor& theColor, COLORREF& color, CBitmap** bitmap, CRect& rect, CButton& button)
    {
        color = CtrlView::Color2ColorRef(theColor);
        InitialButtonColorUpdate(view, color, bitmap, rect, button);
    }

    // asettaa toolmasterin ja toolboxin DC:t
    // esim. ennen piirto tai ennen kuin tarkastellaan hiiren klikkausta ruudulta
    // HUOM!! printtauksen yhteydessä kutsu ensin DC:n asetus ja sitten printinfon!!!
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

    // uudet versiot SetToolsDC:stä, jossa haluttu piirtoalue annetaan parametrina, tällöin
    // printti alueen erilainen käsittely ei jää huomiotta
    void SetToolsDCs(CDC* theDC, NFmiToolBox *theToolBox, const CRect &theClientRect, bool fToolMasterAvailable) // tämä asettaa kaikki kerralla
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
        XuWindowSize(theClientRect.Width(), theClientRect.Height());
        XuWindowSelect(theDC->GetSafeHdc());
    }
}
