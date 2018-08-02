#pragma once

#include "stdafx.h"
#include "NFmiColor.h"
#include "NFmiTimeBag.h"
#include "matrix3d.h"

class SmartMetDocumentInterface;
class CPPToolTip;
class CDC;
class CWnd;
class NFmiToolBox;
class NFmiPoint;
class NFmiRect;
class MfcViewStatus;
namespace CtrlViewUtils
{
    struct GraphicalInfo;
}

// CFmiWin32Helpers

namespace CFmiWin32Helpers
{
	/////////////////////////////////////////////////////////////////////////////
	// Printing Dialog
	class CPrintingDialog : public CDialog
	{
	public:
		//{{AFX_DATA(CPrintingDialog)
		enum { IDD = AFX_IDD_PRINTDLG };
		//}}AFX_DATA
		CPrintingDialog::CPrintingDialog(CWnd* pParent)
			{
				Create(CPrintingDialog::IDD, pParent);      // modeless !
	//			_afxWinState->m_bUserAbort = FALSE;
			}
		virtual ~CPrintingDialog() { }

		virtual BOOL OnInitDialog();
		virtual void OnCancel();
	};

	// tooltip/statusbar string helper
	void SetDescTopGraphicalInfo(CtrlViewUtils::GraphicalInfo &theGraphicalInfo, CDC* pDC, const NFmiPoint &theViewGridSizeInPixels, double theScaleRatio, bool forceInitialization = false);
	void InitializeCPPTooltip(CWnd *theParentView, CPPToolTip &theTooltip, int theTooltipID);
	HBITMAP GetBitmapFromResources(DWORD dwID);

	// printtaus apuja
	CSize CalcPrintOutputAreaSize(const CSize &theTotalOutputSize, const CSize &theTotalClientScreenSize, const NFmiRect *theRelativeOutputArea, bool fitToPage);
	HANDLE DDBToDIB(CBitmap& bitmap, DWORD dwCompression, CPalette* pPal);
	bool DipToPrinter(HANDLE hDib, CDC *thePrinterDC, CPrintInfo* pInfo, const NFmiRect *theRelativeOutputArea, bool fitToPage);
	bool BitmapToPrinter(CDC *thePrinterDC // printterin device context
						,CBitmap *theBitmap // piirrett�v� bitmap
						,CPrintInfo* pInfo // printer info
						,const NFmiRect *theRelativeOutputArea
						,bool fitToPage
						);

	// T�t� kutsutaan eri apuikkunoista OnPreparePrinting-metodin yhteydess�, Mutta ei p��ikkunan kanssa.
	// T�m� on tehty, koska muuten printtauksen yhteydess� p��ikkuna on saattanut crashata ja sen
	// crashin esto jutut pit�� taas poistaa jos print-dialogista painetaan cancel:ia jne.
	BOOL GeneralHelpWnd_OnPreparePrinting(CView *view, SmartMetDocumentInterface *SmartMetDocumentInterface, CPrintInfo* pInfo);

    void SetDialogItemText(CWnd *theDlg, int theDlgItemId, const char* theMagicWord);
    void SetDialogItemVisibility(CWnd *theDlg, int theDlgItemId, bool visible);

	// N�iden avulla asetetaan n�ytt�makro ikkunoiden sijaintia ja fiksataan ikkunoiden sijaintia
	// jos n�ytt� ei osu mihink��n monitoriin (jos SmartMetia on k�ytetty multi monitori koneessa
	// ja samoja makroja/asetuksia k�ytet��n yksi monitorisessa ymp�rist�ss� voi aiheutta t�ll�isen tilanteen.)
	CRect FixWindowPosition(const CRect &theRect, int &theStartCornerCounter);
	void SetWindowSettings(CWnd *win, const CRect &theRect, const MfcViewStatus &viewStatus, int &theStartCornerCounter);
    bool IsWindowMinimized(CWnd *win);
    bool IsWindowMaximized(CWnd *win);

	std::auto_ptr<CWaitCursor> GetWaitCursorIfNeeded(bool showWaitCursorWhileDrawingView);

	void SetPrinterMode(CDC* pDC, int Mode);

	bool StopMouseEventWhenPrinting(NFmiToolBox *theUsedToolBox);

	void SetWindowIconDynamically(CWnd *theWindow, HICON theSmallIcon, HICON theBigIcon);

    inline std::string CT2std(const CString &theStr)    { return std::string(CT2A(theStr)); }
    void SetErrorColorForTextControl(CDC* pDC, bool statusOk, bool isControlUsed = true);

    class MemoryBitmapHelper
    {
        // Luokan avulla yritet��n yksinkertaistaa v�lkkym�tt�m�n MFC piirron tekoa.
        // Eli k�yt�ss� on memory-bitmap, mihin kuva piirret��n ja se 'blitataan' kerralla ruudulle.
        // Lis�ksi bimappia k�ytet��n, kun kuva ei ole 'likainen', jolloin se voidaan blitata sellaisenaan
        // ruudulle, esim. kun joku toinen ikkuna/ohjelma peitt�� osittain n�ytt�� ja/tai sit� liikutetaan n�yt�n editse.
    public:
        MemoryBitmapHelper(CWnd *usedWindow, CBitmap *usedMemoryBitmap);
        ~MemoryBitmapHelper();

        void SetupMemoryBitmap();
        void SetMemoryBitmapInUse();
        CBitmap* GetMemoryBitmapFromUse();

        CClientDC clientDc;
        CRect clientRect;
        CDC dcMem;
        CBitmap *memoryBitmap;
        CBitmap *oldBitmap;
    };

    void InitLogLevelComboBox(CComboBox &comboBox);
    void InitCategoryComboBox(CComboBox &comboBox);
};


