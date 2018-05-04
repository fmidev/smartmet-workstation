#pragma once

#include "stdafx.h"

class NFmiToolBox;
class NFmiColor;

// Apufunktioita jotka ovat riippuvaisia win32/mfc jutuista
// ja toolboxista ja muusta kivasta.
namespace CtrlView
{
    void DestroyBitmap(CBitmap **bitmap);
    void MakeCombatibleBitmap(CWnd *theView, CBitmap **theMemoryBitmap, int cx = 0, int cy = 0);
    CBitmap* CreateColorBitmap(CWnd *view, COLORREF color, int x, int y);
    void ColorButtonPressed(CWnd *view, COLORREF& color, CBitmap** bitmap, CRect& rect, CButton& button);
    void InitialButtonColorUpdate(CWnd *view, COLORREF& color, CBitmap** bitmap, CRect& rect, CButton& button);
    void InitialButtonColorUpdate(CWnd *view, const NFmiColor& theColor, COLORREF& color, CBitmap** bitmap, CRect& rect, CButton& button);

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
}
