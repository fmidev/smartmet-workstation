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
						,CBitmap *theBitmap // piirrettävä bitmap
						,CPrintInfo* pInfo // printer info
						,const NFmiRect *theRelativeOutputArea
						,bool fitToPage
						);

	// Tätä kutsutaan eri apuikkunoista OnPreparePrinting-metodin yhteydessä, Mutta ei pääikkunan kanssa.
	// Tämä on tehty, koska muuten printtauksen yhteydessä pääikkuna on saattanut crashata ja sen
	// crashin esto jutut pitää taas poistaa jos print-dialogista painetaan cancel:ia jne.
	BOOL GeneralHelpWnd_OnPreparePrinting(CView *view, SmartMetDocumentInterface *SmartMetDocumentInterface, CPrintInfo* pInfo);

    void SetDialogItemText(CWnd *theDlg, int theDlgItemId, const char* theMagicWord);
    void SetDialogItemVisibility(CWnd *theDlg, int theDlgItemId, bool visible);

	// Näiden avulla asetetaan näyttömakro ikkunoiden sijaintia ja fiksataan ikkunoiden sijaintia
	// jos näyttö ei osu mihinkään monitoriin (jos SmartMetia on käytetty multi monitori koneessa
	// ja samoja makroja/asetuksia käytetään yksi monitorisessa ympäristössä voi aiheutta tälläisen tilanteen.)
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
        // Luokan avulla yritetään yksinkertaistaa välkkymättömän MFC piirron tekoa.
        // Eli käytössä on memory-bitmap, mihin kuva piirretään ja se 'blitataan' kerralla ruudulle.
        // Lisäksi bimappia käytetään, kun kuva ei ole 'likainen', jolloin se voidaan blitata sellaisenaan
        // ruudulle, esim. kun joku toinen ikkuna/ohjelma peittää osittain näyttöä ja/tai sitä liikutetaan näytön editse.
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


