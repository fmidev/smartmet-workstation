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
#include "GridCtrl.h"


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
		{ // jos oletus arvossa 0, k�ytet��n koneen antamia arvoja laskettaessa pixel/mm
			theGraphicalInfo.itsPixelsPerMM_x = static_cast<double>(theGraphicalInfo.itsScreenWidthInPixels) / static_cast<double>(theGraphicalInfo.itsScreenWidthInMM);
			theGraphicalInfo.itsPixelsPerMM_y = static_cast<double>(theGraphicalInfo.itsScreenHeightInPixels) / static_cast<double>(theGraphicalInfo.itsScreenHeightInMM);
		}
		else
		{ // jos skaalan arvo on asetettu, k�ytet��n koneen antamia dpi arvoja ja k�ytet��n viel� annettua skaalaa muutettaessa dpi (dots per inch) -> dpmm (dots per millimeter)
			theGraphicalInfo.itsPixelsPerMM_x = logpixX/25.2 * theScaleRatio; // muutos dpi-maailmasta (dots-per-inch) dpmm (dots-per-mm) + konekohtainen skaalauskerroin
			theGraphicalInfo.itsPixelsPerMM_y = logpixY/25.2 * theScaleRatio;
		}
		theGraphicalInfo.itsViewWidthInMM = theViewGridSizeInPixels.X() / theGraphicalInfo.itsPixelsPerMM_x;
		theGraphicalInfo.itsViewHeightInMM = theViewGridSizeInPixels.Y() / theGraphicalInfo.itsPixelsPerMM_y;
		theGraphicalInfo.fInitialized = true;
	}
}

// ***************************************************************
// Dib conversio ja printtaus viritykset p�llitty code projectista
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

// Funktio laskee esim. printtauksen yhteydess� mink� kokoiselle alueelle tulostus tehd��n. Eli haluttaessa printin alue on laskettava ett�
// suhteet s�ilyv�t ja mahdollisimman paljon paperin alueesta tulee t�ytetyksi. Lis�ksi alkuper�isest� alueesta halutaan ehk� vain osa
// k�ytt��n, joka annetaan parametrilla theRelativeOutputArea.
// Param theTotalOutputSize on koko mik� on k�ytett�viss� tulostuksessa esim. printauksen parepin pikseli koko.
// Param theTotalClientScreenSize on koko, mik� printattavan ikkunan client-alueen pikseli koko on nyt ruudulla (t�st� saadaan kuvan lopputuloksen suhteet)
// Param theRelativeOutputArea on alue suhteellisessa 0, 0 - 1, 1 maailmassa. Jos tulostuksesta halutaan rajata joku alue pois se onnistuu antamalla t�ss�
//		parametrissa alue mik� halutaan tulostukseen. Jos annettu 0-pointteri, t�m� j�� huomiotta ja koko l�hde alue otetaan k�ytt��n.
// Param fitToPage m��r�� venytet��nk� alue sopimaan koko tulostus alueeseen vai s�ilytet��nk� kuva suhteet alkuper�isin�.
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
	if(widthFactor > heightFactor) // t�m� koodi sovittaa kuvan niin ett� siit� tuleee mahd. iso ja mahtuu annetulle paperille muotoa muuttamatta
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
        startPoint.y = static_cast<long>(::round(theTotalClientScreenSize.cy * (1. - theRelativeOutputArea->Bottom()))); // HUOM! t�ss� pit�� olla Bottom, koska MFC-bitmap maailma menee toisin p�in
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

		// lasketaan sovitus printtaus alueeseen, niin ett� kuva ei v��risty ja se sopii paperille
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
										,CBitmap *theBitmap // piirrett�v� bitmap
										,CPrintInfo* pInfo // printer info
										,const NFmiRect *theRelativeOutputArea, bool fitToPage)
{
	// Tee CBitmap:ista ensin DIB koska printterit osaavat hanskata niit� paremmin.
	DWORD dwCompression = BI_RGB; // en tied� mik� olisi hyv� kompressio
	CPalette pPal; // en tied� miten luodaan t�m� paletti oikein, toivottavasti t�st� syntyy tyhj� paletti, jolloin k�ytet��n defaultti palettia
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
	theTooltip.SetDelayTime(PPTOOLTIP_TIME_INITIAL, 500); // kuinka nopeasti tooltip ilmestyy n�kyviin, jos kursoria ei liikuteta [ms]

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
	theTooltip.SetTransparency(20); // l�pin�kyvyys 0-100 asteikolla (100 t�ysin l�pin�kyv�)
	theTooltip.SetMaxTipWidth(400); // max leveys esim. 400 pikseli�, jonka j�lkeen word-wrap p��lle
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
        smartMetDocumentInterface->Printing(false); // pit�� laittaa false:ksi, jos on painettu cancel:ia, muuten p��ikkuna j�� jumiin
		// lis�ksi ainakin p��ikkuna pit�� p�ivitt��
        smartMetDocumentInterface->MapViewDirty(0, true, false, true, false, false, false);
        smartMetDocumentInterface->RefreshApplicationViewsAndDialogs("After printing any view, have to update all views");
		// this->SetActiveWindow(); // HUOM! aktivointi pit�� tehd� ylemp�n�, t��l� ei toimi // aktivoidaan viel� t�m� ikkuna, koska jostain syyst� print-dialogi aktivoi p��ikkunan
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

// Tarkastetaan josko annettu alue n�kyy jossain monitorissa, niin annetaan sen olla.
// Jos ikkunaa ei n�y miss��n n�yt�ss�, laitetaan se n�kyviin l�himp��n n�ytt��n.
// theStartCornerCounter:in avulla voidaan laskea erilaisia aloitus kulmia, kun pit�� pakottaa ikkunoita
// sopimaan n�kyvien n�ytt�jen alueelle.
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
	{ // annettu alue ei osu mink��n n�yt�n alueelle, sit� pit�� fiksata
		int roundingValue = 8; // jos startinCounter menee yli t�m�n luvun, siirret��n aloitus kulma takasin vasempaan, mutta pyk�l�n alas
		theStartCornerCounter++;
		int startLeft = 10*(theStartCornerCounter%roundingValue);
		int startTop = 10*(theStartCornerCounter%roundingValue) + (theStartCornerCounter/roundingValue)*10;
		SIZE ssize = {theRect.Size().cx, theRect.Size().cy};
		HMONITOR hMonitor2 = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
		if(hMonitor2 == NULL)
		{ // jotain vikaa MonitorFromRect-funktio kutsussa, sijoitetaan alue vain p�� n�yt�n yl�kulmaan
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

bool CFmiWin32Helpers::IsShowCommandMinimized(UINT showCommand)
{
    return showCommand == SW_MINIMIZE || showCommand == SW_SHOWMINIMIZED;
}

bool CFmiWin32Helpers::IsShowCommandMaximized(UINT showCommand)
{
    return showCommand == SW_MAXIMIZE || showCommand == SW_SHOWMAXIMIZED;
}

static UINT GetUsedShowCommand(const MfcViewStatus &viewStatus)
{
    // 1. Jos haluttu status on ShowWindow == false, palauta SW_HIDE
    if(!viewStatus.ShowWindow())
        return SW_HIDE;

    // 2. Jos haluttu ikkuna status on minimized/maximized eli != 0, palauta SHOW_MIN/MAX
    if(viewStatus.ShowCommand())
    {
        if(CFmiWin32Helpers::IsShowCommandMinimized(viewStatus.ShowCommand()))
            return SW_SHOWMINIMIZED;
        if(CFmiWin32Helpers::IsShowCommandMaximized(viewStatus.ShowCommand()))
            return SW_SHOWMAXIMIZED;
    }

    // 3. Muuten palautetaan n�ytt� normaalimoodiin
    return SW_SHOWNORMAL;
}

// Jos maximized tilasta hyp�t��n suoraan minimized tilaan, j�� ikkuna piilossa maximized kokoonsa.
// T�ss� on tarkoitus laittaa se ensin normaali tilaan, jotta se voidaan minimoida my�hemmin kunnolla.
// HUOM! T�m� ei korjaa kokonaan tilannetta, eli minimoitu ikkuna on oikean kokoinen oikeassa paikassa
// mutta siin� on virheellinen visualisointi niin kauan kuin se on minimoituna.
static void FixMaximizedToMinimizedProblem(CWnd *win, const MfcViewStatus &viewStatus)
{
    if(CFmiWin32Helpers::IsShowCommandMinimized(viewStatus.ShowCommand()))
    {
        if(CFmiWin32Helpers::IsWindowMaximized(win))
        {
            win->ShowWindow(SW_SHOWNORMAL);
        }
    }
}

// Jos showCommand parametri on 0:sta poikkeava, annetaan se win oliolle ShowWindow 
// komennossa (t�ss� halutaan vain minimized/maximized tilojen palautus n�ytt�makrosta)
void CFmiWin32Helpers::SetWindowSettings(CWnd *win, const CRect &theRect, const MfcViewStatus &viewStatus, int &theStartCornerCounter)
{
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    BOOL bRet = win->GetWindowPlacement(&wndpl); // gets current window position and iconized/maximized status

    // t�h�n v�liin tehd��n tarkastelut, ett� jos ikkuna menee n�ytt�jen ulkopuolelle, laitetaan se l�himp��n ikkunaan
    CRect fixedRect(CFmiWin32Helpers::FixWindowPosition(theRect, theStartCornerCounter));

    wndpl.rcNormalPosition = fixedRect;
    bRet = win->SetWindowPlacement(&wndpl);
    ::FixMaximizedToMinimizedProblem(win, viewStatus);
    win->ShowWindow(::GetUsedShowCommand(viewStatus));
}

bool CFmiWin32Helpers::IsWindowMinimized(CWnd *win)
{
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    BOOL bRet = win->GetWindowPlacement(&wndpl); // gets current window position and iconized/maximized status
    if(bRet)
    {
        return CFmiWin32Helpers::IsShowCommandMinimized(wndpl.showCmd);
    }
    
    return false;
}

bool CFmiWin32Helpers::IsWindowMaximized(CWnd *win)
{
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    BOOL bRet = win->GetWindowPlacement(&wndpl); // gets current window position and iconized/maximized status
    if(bRet)
    {
        return CFmiWin32Helpers::IsShowCommandMaximized(wndpl.showCmd);
    }

    return false;
}

std::auto_ptr<CWaitCursor> CFmiWin32Helpers::GetWaitCursorIfNeeded(bool showWaitCursorWhileDrawingView)
{
	std::auto_ptr<CWaitCursor> cursorPtr;
	if(showWaitCursorWhileDrawingView)
	{ // tiimalasi laitetaan vain jos ei ole animaatiota karttan�yt�ss� k�ynniss�
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

// Jos tekee high quality printtauksia ja kesken printtauksen menee hiiren kanssa printattavan ruudun p��lle,
// voi ohjelma kaatua, koska toolboxista vaihdetaan CDC kesken printtauksen. Ilmeisesti Printtaus tapahtuu
// eri threadissa tms. Laitan kaikkiin HQ printtaus ikkunoiden hiiri k�sittelyihin t�m�n testi, ett�
// CDC:t� ei vahingossa vaihdeta kesken kaiken.
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

// V�rj�t��n annetun kontrollin tekstin v�ritys seuraavasti:
// 1. Jos statusOk == false => virhe eli punainen v�ri
// 2. Jos isControlUsed == false => kontrollilla ei ole merkityst� eli harmaa
// 3. Muuten normaali musta v�ri
void CFmiWin32Helpers::SetErrorColorForTextControl(CDC* pDC, bool statusOk, bool isControlUsed)
{
    if(!statusOk)
        pDC->SetTextColor(RGB(255, 0, 0)); // V�rj�t��n info-teksti punaisella jos virhe
    else if(!isControlUsed)
        pDC->SetTextColor(RGB(160, 160, 160)); // Jos kontrolli 'merkitykset�n', k�ytet��n harmaata v�ri�
    else
        pDC->SetTextColor(RGB(0, 0, 0)); // Jos ok, k�ytet��n mustaa v�ri�
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

#ifdef max
#undef max
#endif

void CFmiWin32Helpers::FitLastColumnOnVisibleArea(CWnd *gridCtrlParentView, CGridCtrl &gridCtrl, bool &firstTime, int minimumLastColumnWidthInPixels)
{
    if(!gridCtrlParentView)
        return;

    if(firstTime)
        firstTime = false;
    else
    {
        if(gridCtrl.GetColumnCount())
        {
            CRect clientRect;
            gridCtrlParentView->GetClientRect(clientRect);

            int lastColumnIndex = gridCtrl.GetColumnCount() - 1;
            CRect lastHeaderCellRect;
            gridCtrl.GetCellRect(0, lastColumnIndex, lastHeaderCellRect);
            // Calculate new width for last column so that it will fill the client area
            // Total width (cx) - lastColumns left edge - some value (40) that represents the width of the vertical scroll control
            int newLastColumnWidth = clientRect.Width() - lastHeaderCellRect.left - 40;
            // Let's make sure that last column isn't shrinken too much
            newLastColumnWidth = std::max(newLastColumnWidth, 220);
            gridCtrl.SetColumnWidth(lastColumnIndex, newLastColumnWidth);
        }
    }
}

void CFmiWin32Helpers::DoGraphReportOnDraw(const CtrlViewUtils::GraphicalInfo &graphicalInfo, double scaleFactor)
{
    static bool graphInfoReported = false;
    if(graphInfoReported == false)
    { // vain 1. kerran tehd��n lokiin raportti
        graphInfoReported = true;
        std::stringstream sstream;
        sstream << "\nViewWidthInMM: " << graphicalInfo.itsViewWidthInMM << std::endl;
        sstream << "ViewHeightInMM: " << graphicalInfo.itsViewHeightInMM << std::endl;
        sstream << "ScreenWidthInMM: " << graphicalInfo.itsScreenWidthInMM << std::endl;
        sstream << "ScreenHeightInMM: " << graphicalInfo.itsScreenHeightInMM << std::endl;
        sstream << "ScreenWidthInPixels: " << graphicalInfo.itsScreenWidthInPixels << std::endl;
        sstream << "ScreenHeightInPixels: " << graphicalInfo.itsScreenHeightInPixels << std::endl;
        sstream << "DPI x: " << graphicalInfo.itsDpiX << std::endl;
        sstream << "DPI y: " << graphicalInfo.itsDpiY << std::endl;
        sstream << "DrawObjectScaleFactor (editor.conf - MetEditor::DrawObjectScaleFactor ): " << scaleFactor << std::endl;
        if(scaleFactor == 0)
            sstream << "Using ScreenWidthInMM and ScreenHeightInMM to calculate pixels-per-mm values" << std::endl;
        else
            sstream << "Using DPI x and y and DrawObjectScaleFactor to calculate pixels-per-mm values" << std::endl;
        sstream << "PixelsPerMM_x: " << graphicalInfo.itsPixelsPerMM_x << std::endl;
        sstream << "PixelsPerMM_y: " << graphicalInfo.itsPixelsPerMM_y << std::endl;
        CatLog::logMessage(sstream.str(), CatLog::Severity::Debug, CatLog::Category::Visualization);
    }
}

CFmiWin32Helpers::DeviceContextHelper::DeviceContextHelper(CDC *originalDc)
:memoryDc()
{
    memoryDc.CreateCompatibleDC(originalDc);
}

CFmiWin32Helpers::DeviceContextHelper::~DeviceContextHelper()
{
    memoryDc.SelectObject(originalBitmap);
    memoryDc.DeleteDC();
}

void CFmiWin32Helpers::DeviceContextHelper::SelectBitmap(CBitmap *usedBitmap)
{
    originalBitmap = memoryDc.SelectObject(usedBitmap);
}
