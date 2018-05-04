#pragma once

#include "SmartMetToolboxDep_resource.h"
#include "PPToolTip.h"
#include "NFmiViewPosRegistryInfo.h"

class SmartMetDocumentInterface;
namespace Gdiplus
{
	class Bitmap;
	class Graphics;
}

// CFmiDataQualityCheckerDialog dialog

class CFmiDataQualityCheckerDialog : public CDialog
{
private: // n‰ill‰ talletetaan sijainti ja koko rekisteriin
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void){return s_ViewPosRegistryInfo;}

	DECLARE_DYNAMIC(CFmiDataQualityCheckerDialog)

	CFmiDataQualityCheckerDialog(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiDataQualityCheckerDialog();
	void SetDefaultValues(void);
    static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) {return ViewPosRegistryInfo().WinRegistryKeyStr();}
    // Luokalla pit‰‰ olla Update -metodi, jotta sen luonti smartmetdocissa menee template funktion l‰pi.
    void Update() {}

// Dialog Data
	enum { IDD = IDD_DIALOG_DATA_QUALITY_CHECKER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonDoQualityCheck();
private:
	std::string ComposeToolTipText(CPoint point);
	void DrawBackGround(void);
	void DoWhenClosing(void);
	CRect GetPaintedRect(int theUsedMargin);
	void DoDrawStatistics(void);
	void RefreshWorkingArea(void);

    SmartMetDocumentInterface *itsSmartMetDocumentInterface; // ei omista, ei tuhoa
	Gdiplus::Graphics *itsDoubleBuffer2ScreenGraphics; // t‰ll‰ piiirret‰‰n valmis kuva ruudulle
	Gdiplus::Graphics *itsDrawingGraphics; // t‰h‰n piirret‰‰n kuva muistiin, joka laitetaan kerralla 
													   // ruudulle yll‰ olevalla graphics-oliolla
	Gdiplus::Bitmap *itsDoubleBufferBitmap; // t‰m‰ on k‰ytˆss‰ itsGdiPlusDoubleBufferGraphics:iss‰
	CRect itsClienAreaRect; // t‰m‰ on piirtoalue (alkaa siis hieman nappien alta) ikkunan koordinaatistossa
	CRect itsDrawRect; // t‰m‰ on piirtoalue 0,0 - width-1,size-1 -koordinaatistossa. t‰t‰ k‰ytet‰‰n kun piirret‰‰n double puskurin alueeseen
	CPPToolTip m_tooltip;

public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
	virtual void OnCancel();
public:
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnPaint();
	afx_msg void NotifyDisplayTooltip(NMHDR * pNMHDR, LRESULT * result);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
