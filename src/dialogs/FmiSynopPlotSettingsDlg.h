#pragma once
//#include "afxcmn.h"
//#include "afxwin.h"
#include "SmartMetDialogs_resource.h"
#include "boost/shared_ptr.hpp"
#include <string>

class SmartMetDocumentInterface;
class NFmiFastQueryInfo;
class NFmiParamBag;
class NFmiColor;

class NFmiPlotRelatedControls
{
public:
	NFmiPlotRelatedControls();
	~NFmiPlotRelatedControls();

	BOOL fSetAllParamState = TRUE;
	// Eli k‰ytet‰‰nkˆ yksiv‰rist‰ (synop/metar) plottausta vai defaultti v‰ri koodattua
	BOOL fSingleColor = FALSE;
	// Fontin koon s‰‰tˆ slideri
	CSliderCtrl itsFontSizeSlider;
	// S‰‰det‰‰n kuinka tihe‰sti synop plottaus tehd‰‰n, eli jos luku alle 1, sallitaan p‰‰llekk‰isyyksi‰ ja jos yli 1, tulee v‰ljemp‰‰
	CSliderCtrl itsPlotSpacingSlider;
	// T‰st‰ s‰‰det‰‰n yhdell‰ v‰rill‰ piirrett‰v‰n plottauksen v‰ritys
	CButton itsSingleColorChangeButtom;
	// Edell‰ olevaan painikkeeseen liittyv‰‰ dataa (oletusarvo musta)
	COLORREF itsSingleColorRef = 0;
	CBitmap* itsSingleColorBitmap = nullptr;
	CRect itsSingleColorRect;

};

// CFmiSynopPlotSettingsDlg dialog

class CFmiSynopPlotSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(CFmiSynopPlotSettingsDlg)

public:
	CFmiSynopPlotSettingsDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiSynopPlotSettingsDlg();

	void Update(void);
	void InitFromDoc(void);

// Dialog Data
	enum { IDD = IDD_DIALOG_SYNOP_PLOT_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
private:
	void StoreToDoc(void);
	void RefreshApplicationViews(const std::string &reasonForUpdate);
	void UpdateFontSizeString(bool synopPlotCase);
	void UpdatePlotSpacingString(bool synopPlotCase);
	void InitDialogTexts(void);
	void SetAllSynopParamStates(BOOL newState);
	void SetAllMetarParamStates(BOOL newState);
	void InitPlotControls(bool synopPlotCase, bool useSingleColor, double fontSize, double plotSpacing, const NFmiColor& singleColor, NFmiPlotRelatedControls& plotControls);

    SmartMetDocumentInterface *itsSmartMetDocumentInterface;

	BOOL fShowT = TRUE; // n‰ytet‰‰nkˆ T synop-plotissa vai ei
	BOOL fShowTd = TRUE;
	BOOL fShowV = TRUE;
	BOOL fShowWw = TRUE;
	BOOL fShowCh = TRUE;
	BOOL fShowCm = TRUE;
	BOOL fShowCl = TRUE;
	BOOL fShowN = TRUE;
	BOOL fShowNh = TRUE;
	BOOL fShowDdff = TRUE;
	BOOL fShowPPPP = TRUE;	  // PPPP eli pressure
	BOOL fShowPpp = TRUE; // ppp eli pressure change
	BOOL fShowA = TRUE; // a eli paineen muutoksen luonne
	BOOL fShowW1 = TRUE;
	BOOL fShowW2 = TRUE;
	BOOL fShowH = TRUE; // h eli pilven korkeus
	BOOL fShowRr = TRUE;
	BOOL fShowTw = TRUE; // eli veden l‰mpˆtila
	NFmiPlotRelatedControls itsSynopControls;

	BOOL fMetarShow_SkyInfo = TRUE;
	BOOL fMetarShow_TT = TRUE;
	BOOL fMetarShow_TdTd = TRUE;
	BOOL fMetarShow_PhPhPhPh = TRUE;
	BOOL fMetarShow_dddff = TRUE;
	BOOL fMetarShow_Gff = TRUE;
	BOOL fMetarShow_VVVV = TRUE;
	BOOL fMetarShow_ww = TRUE;
	BOOL fMetarShow_Status = TRUE;
	NFmiPlotRelatedControls itsMetarControls;

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonRefresh();
	afx_msg void OnBnClickedButtonStationPriorities();
	afx_msg void OnBnClickedButtonSynopSingleColor();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedCheckToggleAllSynopParams();
	afx_msg void OnBnClickedButtonMetarSingleColor();
	afx_msg void OnBnClickedCheckToggleAllMetarParams();
};
