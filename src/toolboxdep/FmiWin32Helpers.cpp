// FmiWin32Helpers.cpp : implementation file
//

#ifdef _MSC_VER
#pragma warning(disable : 4996) // poistaa varoituksen "warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead"
#endif

#include "FmiWin32Helpers.h"
#include "NFmiDictionaryFunction.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiMapViewDescTop.h"
#include "NFmiViewSettingMacro.h"

#include <fstream>

#include "PPToolTip.h"
#include "NFmiToolBox.h"


/////////////////////////////////////////////////////////////////////////////
// Printing Dialog
/*
BOOL CALLBACK _AfxAbortProc(HDC, int)
{
	_AFX_WIN_STATE* pWinState = _afxWinState;
	MSG msg;
	while (!pWinState->m_bUserAbort &&
		::PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE))
	{
		if (!AfxGetThread()->PumpMessage())
			return FALSE;   // terminate if WM_QUIT received
	}
	return !pWinState->m_bUserAbort;
}
*/
BOOL CFmiWin32Helpers::CPrintingDialog::OnInitDialog()
{
	SetWindowText(AfxGetAppName());
	CenterWindow();
	return CDialog::OnInitDialog();
}

void CFmiWin32Helpers::CPrintingDialog::OnCancel()
{
//	_afxWinState->m_bUserAbort = TRUE;  // flag that user aborted print
	CDialog::OnCancel();
}

void CFmiWin32Helpers::SetDescTopGraphicalInfo(CtrlViewUtils::GraphicalInfo &theGraphicalInfo, CDC* pDC, const NFmiPoint &theViewGridSizeInPixels, double theScaleRatio, bool forceInitialization)
{
	if(forceInitialization || theGraphicalInfo.fInitialized == false)
	{
		int logpixX = GetDeviceCaps(pDC->GetSafeHdc(), LOGPIXELSX);
		int logpixY = GetDeviceCaps(pDC->GetSafeHdc(), LOGPIXELSY);

		theGraphicalInfo.itsScreenWidthInMM = GetDeviceCaps(pDC->GetSafeHdc(), HORZSIZE);
		theGraphicalInfo.itsScreenHeightInMM = GetDeviceCaps(pDC->GetSafeHdc(), VERTSIZE);
		theGraphicalInfo.itsScreenWidthInPixels = GetDeviceCaps(pDC->GetSafeHdc(), HORZRES);
		theGraphicalInfo.itsScreenHeightInPixels = GetDeviceCaps(pDC->GetSafeHdc(), VERTRES);
		if(theScaleRatio == 0)
		{ // jos oletus arvossa 0, käytetään koneen antamia arvoja laskettaessa pixel/mm
			theGraphicalInfo.itsPixelsPerMM_x = static_cast<double>(theGraphicalInfo.itsScreenWidthInPixels) / static_cast<double>(theGraphicalInfo.itsScreenWidthInMM);
			theGraphicalInfo.itsPixelsPerMM_y = static_cast<double>(theGraphicalInfo.itsScreenHeightInPixels) / static_cast<double>(theGraphicalInfo.itsScreenHeightInMM);
		}
		else
		{ // jos skaalan arvo on asetettu, käytetään koneen antamia dpi arvoja ja käytetään vielä annettua skaalaa muutettaessa dpi (dots per inch) -> dpmm (dots per millimeter)
			theGraphicalInfo.itsPixelsPerMM_x = logpixX/25.2 * theScaleRatio; // muutos dpi-maailmasta (dots-per-inch) dpmm (dots-per-mm) + konekohtainen skaalauskerroin
			theGraphicalInfo.itsPixelsPerMM_y = logpixY/25.2 * theScaleRatio;
		}
		theGraphicalInfo.itsViewWidthInMM = theViewGridSizeInPixels.X() / theGraphicalInfo.itsPixelsPerMM_x;
		theGraphicalInfo.itsViewHeightInMM = theViewGridSizeInPixels.Y() / theGraphicalInfo.itsPixelsPerMM_y;
		theGraphicalInfo.fInitialized = true;
	}
}

// ***************************************************************
// Dib conversio ja printtaus viritykset pöllitty code projectista
// http://www.codeproject.com/printing/printingtricksandtips.asp
// ***************************************************************

// DDBToDIB        - Creates a DIB from a DDB
// bitmap        - Device dependent bitmap
// dwCompression    - Type of compression - see BITMAPINFOHEADER
// pPal            - Logical palette
HANDLE CFmiWin32Helpers::DDBToDIB(CBitmap& bitmap, DWORD dwCompression, CPalette* pPal)
{
    BITMAP            bm;
    BITMAPINFOHEADER    bi;
    LPBITMAPINFOHEADER     lpbi;
    DWORD            dwLen;
    HANDLE            hDIB;
    HANDLE            handle;
    HDC             hDC;
    HPALETTE        hPal;


    ASSERT( bitmap.GetSafeHandle() );

    // The function has no arg for bitfields
    if( dwCompression == BI_BITFIELDS )
        return NULL;

    // If a palette has not been supplied use defaul palette
    hPal = (HPALETTE) pPal->GetSafeHandle();
    if (hPal==NULL)
        hPal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);

    // Get bitmap information
    bitmap.GetObject(sizeof(bm),(LPSTR)&bm);

    // Initialize the bitmapinfoheader
    bi.biSize        = sizeof(BITMAPINFOHEADER);
    bi.biWidth        = bm.bmWidth;
    bi.biHeight         = bm.bmHeight;
    bi.biPlanes         = 1;
    bi.biBitCount        = (unsigned short)(bm.bmPlanes * bm.bmBitsPixel) ;
    bi.biCompression    = dwCompression;
    bi.biSizeImage        = 0;
    bi.biXPelsPerMeter    = 0;
    bi.biYPelsPerMeter    = 0;
    bi.biClrUsed        = 0;
    bi.biClrImportant    = 0;

    // Compute the size of the  infoheader and the color table
    int nColors = 0;
    if(bi.biBitCount <= 8)
        {
        nColors = (1 << bi.biBitCount);
        }
    dwLen  = bi.biSize + nColors * sizeof(RGBQUAD);

    // We need a device context to get the DIB from
    hDC = ::GetDC(NULL);
    hPal = SelectPalette(hDC,hPal,FALSE);
    RealizePalette(hDC);

    // Allocate enough memory to hold bitmapinfoheader and color table
    hDIB = GlobalAlloc(GMEM_FIXED,dwLen);

    if (!hDIB){
        SelectPalette(hDC,hPal,FALSE);
        ::ReleaseDC(NULL,hDC);
        return NULL;
    }

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

    *lpbi = bi;

    // Call GetDIBits with a NULL lpBits param, so the device driver
    // will calculate the biSizeImage field
    GetDIBits(hDC, (HBITMAP)bitmap.GetSafeHandle(), 0L, (DWORD)bi.biHeight,
            (LPBYTE)NULL, (LPBITMAPINFO)lpbi, (DWORD)DIB_RGB_COLORS);

    bi = *lpbi;

    // If the driver did not fill in the biSizeImage field, then compute it
    // Each scan line of the image is aligned on a DWORD (32bit) boundary
    if (bi.biSizeImage == 0){
        bi.biSizeImage = ((((bi.biWidth * bi.biBitCount) + 31) & ~31) / 8)
                        * bi.biHeight;

        // If a compression scheme is used the result may infact be larger
        // Increase the size to account for this.
        if (dwCompression != BI_RGB)
            bi.biSizeImage = (bi.biSizeImage * 3) / 2;
    }

    // Realloc the buffer so that it can hold all the bits
    dwLen += bi.biSizeImage;
    handle = GlobalReAlloc(hDIB, dwLen, GMEM_MOVEABLE) ;
    if (handle != NULL)
        hDIB = handle;
    else
        {
        GlobalFree(hDIB);

        // Reselect the original palette
        SelectPalette(hDC,hPal,FALSE);
        ::ReleaseDC(NULL,hDC);
        return NULL;
        }

    // Get the bitmap bits
    lpbi = (LPBITMAPINFOHEADER)hDIB;

    // FINALLY get the DIB
    BOOL bGotBits = GetDIBits( hDC, (HBITMAP)bitmap.GetSafeHandle(),
                0L,                      // Start scan line
                (DWORD)bi.biHeight,      // # of scan lines
                (LPBYTE)lpbi             // address for bitmap bits
                + (bi.biSize + nColors * sizeof(RGBQUAD)),
                (LPBITMAPINFO)lpbi,      // address of bitmapinfo
                (DWORD)DIB_RGB_COLORS);  // Use RGB for color table

    if( !bGotBits )
    {
        GlobalFree(hDIB);

        SelectPalette(hDC,hPal,FALSE);
        ::ReleaseDC(NULL,hDC);
        return NULL;
    }

    SelectPalette(hDC,hPal,FALSE);
    ::ReleaseDC(NULL,hDC);
    return hDIB;
}

// Funktio laskee esim. printtauksen yhteydessä minkä kokoiselle alueelle tulostus tehdään. Eli haluttaessa printin alue on laskettava että
// suhteet säilyvät ja mahdollisimman paljon paperin alueesta tulee täytetyksi. Lisäksi alkuperäisestä alueesta halutaan ehkä vain osa
// käyttöön, joka annetaan parametrilla theRelativeOutputArea.
// Param theTotalOutputSize on koko mikä on käytettävissä tulostuksessa esim. printauksen parepin pikseli koko.
// Param theTotalClientScreenSize on koko, mikä printattavan ikkunan client-alueen pikseli koko on nyt ruudulla (tästä saadaan kuvan lopputuloksen suhteet)
// Param theRelativeOutputArea on alue suhteellisessa 0, 0 - 1, 1 maailmassa. Jos tulostuksesta halutaan rajata joku alue pois se onnistuu antamalla tässä
//		parametrissa alue mikä halutaan tulostukseen. Jos annettu 0-pointteri, tämä jää huomiotta ja koko lähde alue otetaan käyttöön.
// Param fitToPage määrää venytetäänkö alue sopimaan koko tulostus alueeseen vai säilytetäänkö kuva suhteet alkuperäisinä.
CSize CFmiWin32Helpers::CalcPrintOutputAreaSize(const CSize &theTotalOutputSize, const CSize &theTotalClientScreenSize, const NFmiRect *theRelativeOutputArea, bool fitToPage)
{
	if(fitToPage)
		return theTotalOutputSize;

	CSize destSize = theTotalOutputSize;
	CSize screenArea = theTotalClientScreenSize;
	if(theRelativeOutputArea)
	{
		screenArea.cy = static_cast<long>(::round(screenArea.cy * theRelativeOutputArea->Height()));
        screenArea.cx = static_cast<long>(::round(screenArea.cx * theRelativeOutputArea->Width()));
 	}

	int usedDestWidth = screenArea.cx;
	int usedDestHeight = screenArea.cy;
	double widthFactor = destSize.cx / static_cast<double>(usedDestWidth);
	double heightFactor = destSize.cy / static_cast<double>(usedDestHeight);
	if(widthFactor > heightFactor) // tämä koodi sovittaa kuvan niin että siitä tuleee mahd. iso ja mahtuu annetulle paperille muotoa muuttamatta
	{
        usedDestWidth = static_cast<long>(::round(usedDestWidth * heightFactor));
        usedDestHeight = static_cast<long>(::round(usedDestHeight * heightFactor));
	}
	else
	{
        usedDestWidth = static_cast<long>(::round(usedDestWidth * widthFactor));
        usedDestHeight = static_cast<long>(::round(usedDestHeight * widthFactor));
	}

	CSize newDestSize = CSize(usedDestWidth, usedDestHeight);
	return newDestSize;
}

static CRect CalcWantedSourcePixelArea(const CSize &theTotalClientScreenSize, const NFmiRect *theRelativeOutputArea)
{
	CSize wantedSize = theTotalClientScreenSize;
	POINT startPoint;
	startPoint.x = 0;
	startPoint.y = 0;
	if(theRelativeOutputArea)
	{
        wantedSize.cx = static_cast<long>(::round(wantedSize.cx * theRelativeOutputArea->Width()));
        wantedSize.cy = static_cast<long>(::round(wantedSize.cy * theRelativeOutputArea->Height()));
        startPoint.x = static_cast<long>(::round(theTotalClientScreenSize.cx * theRelativeOutputArea->Left()));
        startPoint.y = static_cast<long>(::round(theTotalClientScreenSize.cy * (1. - theRelativeOutputArea->Bottom()))); // HUOM! tässä pitää olla Bottom, koska MFC-bitmap maailma menee toisin päin
	}
	return CRect(startPoint, wantedSize);
}

bool CFmiWin32Helpers::DipToPrinter(HANDLE hDib, CDC *thePrinterDC, CPrintInfo* pInfo, const NFmiRect *theRelativeOutputArea, bool fitToPage)
{
	if(hDib)
	{
		BITMAPINFOHEADER    *pBMI ;
		pBMI = (BITMAPINFOHEADER*)GlobalLock(hDib) ;
		int nColors = 0;
		if (pBMI->biBitCount <= 8)
		{
			nColors = (1 << pBMI->biBitCount);
		}

		// lasketaan sovitus printtaus alueeseen, niin että kuva ei vääristy ja se sopii paperille
		CSize destTotalSize = pInfo->m_rectDraw.Size();
		CSize clientSize(pBMI->biWidth, pBMI->biHeight);
		CSize usedDestSize = CFmiWin32Helpers::CalcPrintOutputAreaSize(destTotalSize, clientSize, theRelativeOutputArea, fitToPage);
		CRect usedSourceRect = ::CalcWantedSourcePixelArea(clientSize, theRelativeOutputArea);

		// print the correct image
		int status =::StretchDIBits(thePrinterDC->m_hDC,
									pInfo->m_rectDraw.left,
									pInfo->m_rectDraw.top, // + cs.cy * j,
									usedDestSize.cx,
									usedDestSize.cy,
									usedSourceRect.left,
									usedSourceRect.top,
									usedSourceRect.Width(),
									usedSourceRect.Height(),
									(LPBYTE)pBMI + (pBMI->biSize + nColors * sizeof(RGBQUAD)),
									(BITMAPINFO*)pBMI,
									DIB_RGB_COLORS,
									SRCCOPY);
		// free resources
		GlobalUnlock(hDib) ;

		return status != GDI_ERROR;
	}
	return false;
}


bool CFmiWin32Helpers::BitmapToPrinter(CDC *thePrinterDC // printterin device context
										,CBitmap *theBitmap // piirrettävä bitmap
										,CPrintInfo* pInfo // printer info
										,const NFmiRect *theRelativeOutputArea, bool fitToPage)
{
	// Tee CBitmap:ista ensin DIB koska printterit osaavat hanskata niitä paremmin.
	DWORD dwCompression = BI_RGB; // en tiedä mikä olisi hyvä kompressio
	CPalette pPal; // en tiedä miten luodaan tämä paletti oikein, toivottavasti tästä syntyy tyhjä paletti, jolloin käytetään defaultti palettia
	HANDLE hDib = CFmiWin32Helpers::DDBToDIB(*theBitmap, dwCompression, &pPal);
	bool status = CFmiWin32Helpers::DipToPrinter(hDib, thePrinterDC, pInfo, theRelativeOutputArea, fitToPage);

	GlobalFree(hDib);
	return status;
}

void CFmiWin32Helpers::InitializeCPPTooltip(CWnd *theParentView, CPPToolTip &theTooltip, int theTooltipID)
{
	theTooltip.Create(theParentView);
	theTooltip.SetNotify();
	theTooltip.SetDelayTime(PPTOOLTIP_TIME_AUTOPOP, 30000); // kuinka kauan tooltippi viipyy, jos kursoria ei liikuteta [ms]
	theTooltip.SetDelayTime(PPTOOLTIP_TIME_INITIAL, 500); // kuinka nopeasti tooltip ilmestyy näkyviin, jos kursoria ei liikuteta [ms]

	CRect rc;
    theParentView->GetClientRect(&rc);
    theParentView->ClientToScreen(&rc);

	//Add tooltip to the Map
	PPTOOLTIP_INFO ti;
	ti.nBehaviour = PPTOOLTIP_MULTIPLE_SHOW;
	ti.nIDTool = theTooltipID;
	ti.rectBounds = rc;
	ti.sTooltip = "";
	ti.nMask = PPTOOLTIP_MASK_BEHAVIOUR;
	theTooltip.AddTool(theParentView, ti);
	theTooltip.SetTransparency(20); // läpinäkyvyys 0-100 asteikolla (100 täysin läpinäkyvä)
	theTooltip.SetMaxTipWidth(400); // max leveys esim. 400 pikseliä, jonka jälkeen word-wrap päälle
}

HBITMAP CFmiWin32Helpers::GetBitmapFromResources(DWORD dwID)
{
	if (0 == dwID)
		return NULL;

	// Find correct resource handle
#ifdef _MFC_VER
	HINSTANCE hInstResource = AfxFindResourceHandle(MAKEINTRESOURCE(dwID), RT_BITMAP);
#else
	HINSTANCE hInstResource = ::GetModuleHandle(NULL);
#endif
	// Load bitmap
	HBITMAP hBitmap = (HBITMAP)::LoadImage(hInstResource, MAKEINTRESOURCE(dwID), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);

	return hBitmap;
}

BOOL CFmiWin32Helpers::GeneralHelpWnd_OnPreparePrinting(CView *view, SmartMetDocumentInterface *smartMetDocumentInterface, CPrintInfo* pInfo)
{
	if(smartMetDocumentInterface)
        smartMetDocumentInterface->Printing(true);
	pInfo->SetMaxPage(1);

	// TODO: call DoPreparePrinting to invoke the Print dialog box
	BOOL status = FALSE;
	try
	{
		status = view->DoPreparePrinting(pInfo);
	}
	catch(...)
	{
	}
	if(status == FALSE)
	{
        smartMetDocumentInterface->Printing(false); // pitää laittaa false:ksi, jos on painettu cancel:ia, muuten pääikkuna jää jumiin
		// lisäksi ainakin pääikkuna pitää päivittää
        smartMetDocumentInterface->MapDirty(0, true, false);
        smartMetDocumentInterface->RefreshApplicationViewsAndDialogs("After printing any view, have to update all views");
		// this->SetActiveWindow(); // HUOM! aktivointi pitää tehdä ylempänä, täälä ei toimi // aktivoidaan vielä tämä ikkuna, koska jostain syystä print-dialogi aktivoi pääikkunan
	}
	return status;
}

void CFmiWin32Helpers::SetDialogItemText(CWnd *theDlg, int theDlgItemId, const char* theMagicWord)
{
	if(theDlg)
	{
		CWnd *win = theDlg->GetDlgItem(theDlgItemId);
		if(win)
			win->SetWindowText(CA2T(::GetDictionaryString(theMagicWord).c_str()));
	}
}

void CFmiWin32Helpers::SetDialogItemVisibility(CWnd *theDlg, int theDlgItemId, bool visible)
{
    if(theDlg)
    {
        CWnd *win = theDlg->GetDlgItem(theDlgItemId);
        if(win)
            win->ShowWindow(visible ? SW_SHOW : SW_HIDE);
    }
}

// Tarkastetaan josko annettu alue näkyy jossain monitorissa, niin annetaan sen olla.
// Jos ikkunaa ei näy missään näytössä, laitetaan se näkyviin lähimpään näyttöön.
// theStartCornerCounter:in avulla voidaan laskea erilaisia aloitus kulmia, kun pitää pakottaa ikkunoita
// sopimaan näkyvien näyttöjen alueelle.
CRect CFmiWin32Helpers::FixWindowPosition(const CRect &theRect, int &theStartCornerCounter)
{
	RECT rc = theRect;
/*
	rc.left = static_cast<LONG>(theRect.Left());
	rc.right = static_cast<LONG>(theRect.Right());
	rc.top = static_cast<LONG>(theRect.Top());
	rc.bottom = static_cast<LONG>(theRect.Bottom());
*/
	HMONITOR hMonitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONULL);
	if(hMonitor == NULL)
	{ // annettu alue ei osu minkään näytön alueelle, sitä pitää fiksata
		int roundingValue = 8; // jos startinCounter menee yli tämän luvun, siirretään aloitus kulma takasin vasempaan, mutta pykälän alas
		theStartCornerCounter++;
		int startLeft = 10*(theStartCornerCounter%roundingValue);
		int startTop = 10*(theStartCornerCounter%roundingValue) + (theStartCornerCounter/roundingValue)*10;
		SIZE ssize = {theRect.Size().cx, theRect.Size().cy};
		HMONITOR hMonitor2 = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
		if(hMonitor2 == NULL)
		{ // jotain vikaa MonitorFromRect-funktio kutsussa, sijoitetaan alue vain pää näytön yläkulmaan
			POINT usedStartPoint = {startLeft, startTop};
			return CRect(usedStartPoint, ssize);
		}
		else
		{
			MONITORINFO mi;
			mi.cbSize = sizeof(mi);
			GetMonitorInfo(hMonitor2, &mi);
			RECT aMonitorRect = mi.rcMonitor;
			POINT usedStartPoint = {aMonitorRect.left + startLeft, aMonitorRect.top + startTop};
			return CRect(usedStartPoint, ssize);
		}
	}
	else
		return theRect;
}

static UINT GetUsedShowCommand(CWnd *win, const MfcViewStatus &viewStatus)
{
    // 1. Jos haluttu status on ShowWindow == false, palauta SW_HIDE
    if(!viewStatus.ShowWindow())
        return SW_HIDE;

    // 2. Jos haluttu ikkuna status on minimized/maximized eli != 0, palauta SHOW_MIN/MAX
    if(viewStatus.ShowCommand())
    {
        if(viewStatus.ShowCommand() == SW_MINIMIZE || viewStatus.ShowCommand() == SW_SHOWMINIMIZED)
            return SW_SHOWMINIMIZED;
        if(viewStatus.ShowCommand() == SW_MAXIMIZE || viewStatus.ShowCommand() == SW_SHOWMAXIMIZED)
            return SW_SHOWMAXIMIZED;
    }

    // 3. Jos ikkuna oli min/max moodissa, mutta nyt se halutaan normaali näyttömoodiin, pitää palauttaa 
    //if(CFmiWin32Helpers::IsWindowMinimizedOrMaximized(win))
    //    return SW_RESTORE;
    //else
        return SW_SHOWNORMAL;
}

//static const CRect DebugHelper_WeirdViewRectSize(0, 0, 136, 39);
//static void DebugHelper_CheckFor_WeirdViewRectSize(const CRect &theRect)
//{
//    if(theRect.Size() == DebugHelper_WeirdViewRectSize.Size())
//    {
//        int x = 0;
//    }
//}

// Jos showCommand parametri on 0:sta poikkeava, annetaan se win oliolle ShowWindow 
// komennossa (tässä halutaan vain minimized/maximized tilojen palautus näyttömakrosta)
void CFmiWin32Helpers::SetWindowSettings(CWnd *win, const CRect &theRect, const MfcViewStatus &viewStatus, CWnd *mainFrame, int &theStartCornerCounter)
{
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    BOOL bRet = win->GetWindowPlacement(&wndpl); // gets current window position and iconized/maximized status

//    DebugHelper_CheckFor_WeirdViewRectSize(theRect);
    // tähän väliin tehdään tarkastelut, että jos ikkuna menee näyttöjen ulkopuolelle, laitetaan se lähimpään ikkunaan
    CRect fixedRect(CFmiWin32Helpers::FixWindowPosition(theRect, theStartCornerCounter));
    //    DebugHelper_CheckFor_WeirdViewRectSize(fixedRect);

    wndpl.rcNormalPosition = fixedRect;
    /*
        wndpl.rcNormalPosition.left = static_cast<LONG>(fixedRect.Left());
        wndpl.rcNormalPosition.top = static_cast<LONG>(fixedRect.Top());
        wndpl.rcNormalPosition.right = static_cast<LONG>(fixedRect.Right());
        wndpl.rcNormalPosition.bottom = static_cast<LONG>(fixedRect.Bottom());
    */
    bRet = win->SetWindowPlacement(&wndpl);
    win->ShowWindow(::GetUsedShowCommand(win, viewStatus));
    if(mainFrame)
        mainFrame->ShowWindow(SW_SHOW); // aktivoin pääikkunan, koska edellinen käsky jätti aikasarjaikkunan aktiiviseksi
}

bool CFmiWin32Helpers::IsWindowMinimized(CWnd *win)
{
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    BOOL bRet = win->GetWindowPlacement(&wndpl); // gets current window position and iconized/maximized status
    if(bRet)
    {
        return wndpl.showCmd == SW_MINIMIZE || wndpl.showCmd == SW_SHOWMINIMIZED;
    }
    
    return false;
}

bool CFmiWin32Helpers::IsWindowMinimizedOrMaximized(CWnd *win)
{
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    BOOL bRet = win->GetWindowPlacement(&wndpl); // gets current window position and iconized/maximized status
    if(bRet)
    {
        return wndpl.showCmd == SW_MINIMIZE || wndpl.showCmd == SW_SHOWMINIMIZED || wndpl.showCmd == SW_MAXIMIZE || wndpl.showCmd == SW_SHOWMAXIMIZED;
    }

    return false;
}

std::auto_ptr<CWaitCursor> CFmiWin32Helpers::GetWaitCursorIfNeeded(bool showWaitCursorWhileDrawingView)
{
	std::auto_ptr<CWaitCursor> cursorPtr;
	if(showWaitCursorWhileDrawingView)
	{ // tiimalasi laitetaan vain jos ei ole animaatiota karttanäytössä käynnissä
		cursorPtr.reset(new CWaitCursor());
	}
	return cursorPtr;
}

////////////////////////////////////////////////////////////////////////////
//	STATIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////
//	Desc: static member function for changing print orientation
//	params: CDC* to be used and the mode to switch to
//	Returns: nada
///////////////////////////////////////////////////////////////////////////
void CFmiWin32Helpers::SetPrinterMode(CDC* pDC, int Mode)
{
	if(Mode !=DMORIENT_LANDSCAPE && Mode != DMORIENT_PORTRAIT)
		return;
	PRINTDLG* pPrintDlg = new PRINTDLG;
	AfxGetApp()->GetPrinterDeviceDefaults(pPrintDlg);
	DEVMODE* lpDevMode = (DEVMODE*)::GlobalLock(pPrintDlg->hDevMode);
	lpDevMode->dmOrientation = (short)Mode;
	pDC->ResetDC(lpDevMode);
	::GlobalUnlock(pPrintDlg->hDevMode);
	delete pPrintDlg;
}

// Jos tekee high quality printtauksia ja kesken printtauksen menee hiiren kanssa printattavan ruudun päälle,
// voi ohjelma kaatua, koska toolboxista vaihdetaan CDC kesken printtauksen. Ilmeisesti Printtaus tapahtuu
// eri threadissa tms. Laitan kaikkiin HQ printtaus ikkunoiden hiiri käsittelyihin tämän testi, että
// CDC:tä ei vahingossa vaihdeta kesken kaiken.
bool CFmiWin32Helpers::StopMouseEventWhenPrinting(NFmiToolBox *theUsedToolBox)
{
	if(theUsedToolBox->GetDC()->IsPrinting())
		return true;
	else
		return false;
}

void CFmiWin32Helpers::SetWindowIconDynamically(CWnd *theWindow, HICON theSmallIcon, HICON theBigIcon)
{
	if(theWindow)
	{
		theWindow->SetIcon(theBigIcon, TRUE);
		theWindow->SetIcon(theSmallIcon, FALSE);
	}
}

// Värjätään annetun kontrollin tekstin väritys seuraavasti:
// 1. Jos statusOk == false => virhe eli punainen väri
// 2. Jos isControlUsed == false => kontrollilla ei ole merkitystä eli harmaa
// 3. Muuten normaali musta väri
void CFmiWin32Helpers::SetErrorColorForTextControl(CDC* pDC, bool statusOk, bool isControlUsed)
{
    if(!statusOk)
        pDC->SetTextColor(RGB(255, 0, 0)); // Värjätään info-teksti punaisella jos virhe
    else if(!isControlUsed)
        pDC->SetTextColor(RGB(160, 160, 160)); // Jos kontrolli 'merkityksetön', käytetään harmaata väriä
    else
        pDC->SetTextColor(RGB(0, 0, 0)); // Jos ok, käytetään mustaa väriä
}

CFmiWin32Helpers::MemoryBitmapHelper::MemoryBitmapHelper(CWnd *usedWindow, CBitmap *usedMemoryBitmap)
    :clientDc(usedWindow)
    , clientRect()
    , dcMem()
    , memoryBitmap(usedMemoryBitmap)
{
    usedWindow->GetClientRect(&clientRect);
    dcMem.CreateCompatibleDC(&clientDc);
}

CFmiWin32Helpers::MemoryBitmapHelper::~MemoryBitmapHelper()
{
    dcMem.DeleteDC();
}

void CFmiWin32Helpers::MemoryBitmapHelper::SetupMemoryBitmap()
{
    if(memoryBitmap)
        memoryBitmap->DeleteObject();
    else
        memoryBitmap = new CBitmap;

    memoryBitmap->CreateCompatibleBitmap(&clientDc, clientRect.Width(), clientRect.Height());
    ASSERT(memoryBitmap->m_hObject != 0);
}

void CFmiWin32Helpers::MemoryBitmapHelper::SetMemoryBitmapInUse()
{
    oldBitmap = dcMem.SelectObject(memoryBitmap);
}

CBitmap* CFmiWin32Helpers::MemoryBitmapHelper::GetMemoryBitmapFromUse()
{
    return memoryBitmap = dcMem.SelectObject(oldBitmap);
}

void CFmiWin32Helpers::InitLogLevelComboBox(CComboBox &comboBox)
{
    comboBox.ResetContent();
    comboBox.AddString(CA2T(::GetDictionaryString("Trace").c_str()));
    comboBox.AddString(CA2T(::GetDictionaryString("Debug").c_str()));
    comboBox.AddString(CA2T(::GetDictionaryString("Info").c_str()));
    comboBox.AddString(CA2T(::GetDictionaryString("Warning").c_str()));
    comboBox.AddString(CA2T(::GetDictionaryString("Error").c_str()));
    comboBox.AddString(CA2T(::GetDictionaryString("Critical").c_str()));
}

void CFmiWin32Helpers::InitCategoryComboBox(CComboBox &comboBox)
{
    comboBox.ResetContent();
    comboBox.AddString(CA2T(::GetDictionaryString("AllCategories").c_str()));
    comboBox.AddString(CA2T(::GetDictionaryString("Configuration").c_str()));
    comboBox.AddString(CA2T(::GetDictionaryString("Data").c_str()));
    comboBox.AddString(CA2T(::GetDictionaryString("Editing").c_str()));
    comboBox.AddString(CA2T(::GetDictionaryString("Visualization").c_str()));
    comboBox.AddString(CA2T(::GetDictionaryString("Macro").c_str()));
    comboBox.AddString(CA2T(::GetDictionaryString("Operational").c_str()));
    comboBox.AddString(CA2T(::GetDictionaryString("NetRequest").c_str()));
}
