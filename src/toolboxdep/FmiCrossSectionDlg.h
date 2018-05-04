#pragma once

#include "SmartMetToolboxDep_resource.h"
#include "afxcmn.h"
#include "NFmiViewPosRegistryInfo.h"

class SmartMetDocumentInterface;
class CFmiCrossSectionView;
// CFmiCrossSectionDlg dialog

class CFmiCrossSectionDlg : public CDialog
{
private:
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void){return s_ViewPosRegistryInfo;}

	DECLARE_DYNAMIC(CFmiCrossSectionDlg)

public:
	CFmiCrossSectionDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiCrossSectionDlg();
	void Update();
	void UpdateMapMode(void);
	CBitmap* MemoryBitmap(void);
	void SetDefaultValues(void);
	void InitFromCrossSectionSystem(void);
    static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) {return ViewPosRegistryInfo().WinRegistryKeyStr();}
    CFmiCrossSectionView* GetCrossSectionView() { return itsView; }


// Dialog Data
	enum { IDD = IDD_DIALOG_CROSS_SECTION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnCancel();
    virtual void OnOK();

	DECLARE_MESSAGE_MAP()

private:
	int GetDialogButtonHeight(void);
	void EnableControls(void);
	CRect CalcClientArea(void);
	CRect CalcOtherArea(void);
	void DoWhenClosing(void);
	void InitDialogTexts(void);
    void MakeCrossSectionModeUpdates(const std::string &reasonForUpdate);

	CFmiCrossSectionView* itsView;
    SmartMetDocumentInterface* itsSmartMetDocumentInterface;
	HACCEL m_hAccel;
    BOOL fUseTimeCrossSection;
    // kun t�m� on p��ll�, kartalla n�kyy poikkileikkaus pallukat
    BOOL fUseCrossSectionMapMode;
    // onko k�yt�ss� 3- vai 2-piste moodi eli tehd��nk� poikkileikkaus k�ytt�en 2 vai 3 pistett�
    BOOL fUse3PointMode;
    // T�ll� s��det��n kuinka monta ikkunaa on n�kyviss� poikkileikkaus-n�yt�ss� kerrallaan (max 5)
    CSliderCtrl itsViewCountSlider;
    BOOL fUseRouteCrossSection;
    BOOL fShowHybridLevels;
    BOOL fShowTrajectories;
    BOOL fUseObsAndForCrossSection;

public:
	afx_msg void OnBnClickedButtonRefresh();
	afx_msg void OnBnClickedButtonPrint();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnBnClickedCheckUseTimeCrossSection();
	afx_msg void OnBnClickedCheckUseCrosssectionMapMode();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnNMReleasedcaptureSliderCrossSectionViewCount(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCheckUseCrosssection3PointMode2();
	afx_msg void OnClose();
	afx_msg void OnBnClickedCheckUseRouteCrossSection();
	afx_msg void OnBnClickedCheckCrosssectionShowHybridLevels();
	afx_msg void OnBnClickedCheckCrosssectionShowTrajectories();
	afx_msg void OnAcceleratorToggleTooltip();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedCheckUseObsandforCrossSection();
	afx_msg void OnAcceleratorHideParamWindowExtraMap();
	afx_msg void OnAcceleratorSetCrossSectionSpecialAxis();
	afx_msg void OnAcceleratorSetCrossSectionDefaultAxis();
	afx_msg void OnAcceleratorSetCrossSectionDefaultAxisAll();
	afx_msg void OnAcceleratorSetCrossSectionSpecialAxisAll();
	afx_msg void OnAcceleratorSaveCrossSectionDefaultAxis();
	afx_msg void OnAcceleratorSaveCrossSectionSpecialAxis();
};
