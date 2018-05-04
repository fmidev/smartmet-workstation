#pragma once
//#include "afxcmn.h"
//#include "afxwin.h"
#include "SmartMetDialogs_resource.h"
#include "boost/shared_ptr.hpp"
#include <string>

class SmartMetDocumentInterface;
class NFmiFastQueryInfo;
class NFmiParamBag;

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
	void UpdateFontSizeString(void);
	void UpdatePlotSpacingString(void);
	void EnableCheckBox(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, int theParId, int theCheckBoxIdNumber);
	void EnableCheckBox(NFmiParamBag &theParams, int theParId, int theCheckBoxIdNumber);
	void EnableCheckBox(BOOL theState, int theCheckBoxIdNumber);
	void InitDialogTexts(void);

    SmartMetDocumentInterface *itsSmartMetDocumentInterface;

	BOOL fShowT; // n‰ytet‰‰nkˆ T synop-plotissa vai ei
	BOOL fShowTd;
	BOOL fShowV;
	BOOL fShowWw;
	BOOL fShowCh;
	BOOL fShowCm;
	BOOL fShowCl;
	BOOL fShowN;
	BOOL fShowNh;
	BOOL fShowDdff;
	BOOL fShowPPPP;	  // PPPP eli pressure
	BOOL fShowPpp; // ppp eli pressure change
	BOOL fShowA; // a eli paineen muutoksen luonne
	BOOL fShowW1;
	BOOL fShowW2;
	BOOL fShowH; // h eli pilven korkeus
	BOOL fShowRr;
	BOOL fShowTw; // eli veden l‰mpˆtila
	BOOL fSingleColor; // eli k‰ytet‰‰nkˆ yksiv‰rist‰ synop plottausta vai defaultti v‰ri koodattua
	// fontin koon s‰‰tˆ slideri
	CSliderCtrl itsFontSizeSlider;
	// s‰‰det‰‰n kuinka tihe‰sti synop plottaus tehd‰‰n, eli jos luku alle 1, sallitaan p‰‰llekk‰isyyksi‰ ja jos yli 1, tulee v‰ljemp‰‰
	CSliderCtrl itsPlotSpacingSlider;
	// t‰st‰ s‰‰det‰‰n yhdell‰ v‰rill‰ piirrett‰v‰n plttauksen v‰ritys
	CButton itsSingleColorChangeButtom;

	COLORREF itsSingleColorRef;
	CBitmap* itsSingleColorBitmap;
	CRect itsSingleColorRect;

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonRefresh();
	afx_msg void OnBnClickedButtonStationPriorities();
	afx_msg void OnBnClickedCheckShowV();
	afx_msg void OnBnClickedCheckShowT();
	afx_msg void OnBnClickedCheckShowWw();
	afx_msg void OnBnClickedCheckShowTd();
	afx_msg void OnBnClickedCheckShowCh();
	afx_msg void OnBnClickedCheckShowCm();
	afx_msg void OnBnClickedCheckShowN();
	afx_msg void OnBnClickedCheckShowDdff();
	afx_msg void OnBnClickedCheckShowCl();
	afx_msg void OnBnClickedCheckShowH();
	afx_msg void OnBnClickedCheckShowRr();
	afx_msg void OnBnClickedCheckShowPppp();
	afx_msg void OnBnClickedCheckShowPpp();
	afx_msg void OnBnClickedCheckShowNh();
	afx_msg void OnBnClickedCheckShowTw();
	afx_msg void OnBnClickedCheckShowA();
	afx_msg void OnBnClickedCheckShowW1();
	afx_msg void OnBnClickedCheckShowW2();
	afx_msg void OnBnClickedCheckShowSingleColor();
	afx_msg void OnBnClickedButtonSingleColor();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedCheckToggleAllParams();
private:
	void SetAllParamStates(BOOL newState);
	BOOL fSetAllParamState;
};
