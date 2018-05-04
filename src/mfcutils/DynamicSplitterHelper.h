#pragma once

#include "ControlSplitter.h"

class CDynamicSplitterHelper
{
public:
    CDynamicSplitterHelper(CWnd *parentWnd, int horizontalCtrlId, int verticalCtrlId, double splitRatio, int useHorizontalSplitterCtrlId, int topOrLeftCtrlId, int bottomOrRightCtrlId, int totalSplitterAreaCtrlId);
    void SetUpDataExchange(CDataExchange* pDX);
    void ShowHideSplitters();
    void PreResizerHookingSetup(); // Splitter systeemille pit‰‰ antaa alkuarvot ennen kuin tehd‰‰n muita s‰‰tˆj‰.
    void PostResizerHookingSetup();
    void AdjustSplitterControl();
    int GetUsedSplitterControlId();
    CRect GetSplitterDividerRect();
    CRect GetTotalSplitterRect();
    void ReCalculateNewSplitFactor();
    void OnSplitterOrientationCheckboxClicked();
    bool IsParentWndFullyOperational();

    CWnd *itsParentWnd; // T‰ll‰ luokalla on k‰ytˆss‰‰n t‰m‰ (this) CDynamicSplitterHelper -luokan olio. T‰lt‰ pyydet‰‰n mm. dialogin kontrolli ikkunoita kontrolli Id:t‰ vastaan.
    int itsSplitterHorizontalCtrlId; // Horisontaali splitter divider kontrollin id (joka on siis button tyyppinen kontrolli)
    CControlSplitter itsSplitterHorizontal; // T‰ll‰ on hallussaan tiedot splitterin yl‰- ja alapuolisista kontrolleista.
    int itsSplitterVerticalCtrlId; // Vertikaali splitter divider kontrollin id (joka on siis button tyyppinen kontrolli)
    CControlSplitter itsSplitterVertical; // T‰ll‰ on hallussaan tiedot splitterin vasemman ja oikean puolisista kontrolleista.
    double itsSplitRatio; // T‰ss‰ on editoitavan tekstin osuus koko splitter kontrollin alasta eli luku v‰lill‰ 0..1. Jos luku on 1, on koko tila editoinnilla.
    int itsUseHorizontalSplitterCtrlId; // T‰m‰ on sen checkboxin kontolli id, josta s‰‰det‰‰n splitterin orientaatiota
    BOOL fUseHorizontalSplitter; // T‰ss‰ tiedossa k‰ytt‰j‰n valitsema splitterin orientaatio moodi, t‰t‰ p‰ivitet‰‰n parent dialogin DataExchange -systeemin kautta automaattisesti.
    int itsTopOrLeftCtrlId; // Splitterin yl‰/vasemman puoleisen kontrollin id.
    int itsBottomOrRightCtrlId; // Splitterin ala/oikean puoleisen kontrollin id.
    int itsTotalSplitterAreaCtrlId; // T‰m‰ kontrolli tiet‰‰ koko splitter kontrolli systeemin totaali alueen.
    bool fParentWndInitialized; // t‰m‰ asetetaan true tilaan vasta PostResizerHookingSetup metodin lopuksi. Jostain syyst‰ parentWnd:n GetSafeHwnd tarkastelu ei riit‰, kun dialodi menee OnSize -metodiin jo Create -metodissa ja safe hwnd on jo olemassa.
};
