#pragma once
#include "afxcmn.h"
#include "NFmiProducerSystem.h"
#include "NFmiDataMatrix.h"
#include "SmartMetToolboxDep_resource.h"
#include "NFmiViewPosRegistryInfo.h"

class CFmiTrajectoryView;
class SmartMetDocumentInterface;

// CFmiTrajectoryDlg dialog

class CFmiTrajectoryDlg : public CDialog
{
private: // näillä talletetaan sijainti ja koko rekisteriin
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void){return s_ViewPosRegistryInfo;}

	DECLARE_DYNAMIC(CFmiTrajectoryDlg)

public:
	CFmiTrajectoryDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiTrajectoryDlg();

	void Update();
	void UpdateTrajectorySystem(void);
	void InitFromTrajectorySystem(void);
	void SetDefaultValues(void);
	CBitmap* MemoryBitmap(void);
    static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) {return ViewPosRegistryInfo().WinRegistryKeyStr();}
// Dialog Data
	enum { IDD = IDD_DIALOG_TRAJECTORY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CRect CalcClientArea(void);
	void DoWhenClosing(void);
	void SetDlgItemText2(int theDlgItemId, const std::string &theStr);
	void InitDialogTexts(void);
	void RefreshViewsAndDialogs(const std::string &reasonForUpdate);
	BOOL InitTooltipControl(void);
	void InitControlsTooltip(int theControlId, const char *theDictionaryStr);
	void EnableModelAndTypeControls(void);
	void EnableDataTypeRadioControls(const std::string &theSelectedProducerNameStr, int selectedDataType, int checkedDataTypeNumber, int controlID, bool &selectedDataTypeWasEnabled, int &firstEnabledDataType);

	CFmiTrajectoryView* itsView;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
	CToolTipCtrl itsTooltipCtrl;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonTrajectoryAdd();
	afx_msg void OnBnClickedButtonTrajectoryClear();
private:
	int itsSelectedDataType;
	CSpinButtonCtrl itsTimeStepSpinCtrl;
	CSpinButtonCtrl itsTimeLengthSpinCtrl;
public:
	afx_msg void OnBnClickedButtonTrajectorySetAll();
private:
	BOOL fUsePlumes;
	CSpinButtonCtrl itsProbFactorSpinCtrl;
	CSpinButtonCtrl itsParticleCountSpinCtrl;
	CSpinButtonCtrl itsStartLocationRangeSpinCtrl;
	CSpinButtonCtrl itsStartTimeRangeSpinCtrl;
	CSpinButtonCtrl itsStartPressureLevelRangeSpinCtrl;
	int itsTrajectoryDirection; // 0 = eteen ja 1 =  taakse
public:
	afx_msg void OnBnClickedButtonTrajectoryRecalculate();
	afx_msg void OnBnClickedButtonTrajectorySetLast();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
private:
	BOOL fShowTrajectoryArrows;
	BOOL fShowTrajectoryAnimationMarkers;
public:
	afx_msg void OnBnClickedCheckTrajectoryShowArrows();
	afx_msg void OnBnClickedCheckTrajectoryShowAnimationMarkers();
private:
	// [m/s]
    CString itsTrajectoryConstantVerticalVelocityStrU_;
	CSpinButtonCtrl itsTrajectoryConstantVerticalVelocitySpinCtrl;
	BOOL fTrajectoryIsentropic;
	BOOL fShowTrajectoriesInCrossSectionView;
public:
	afx_msg void OnBnClickedCheckTrajectoryShowInCrossSectionView();
	afx_msg void OnBnClickedButtonTrajectoryConstantSpeedSettings();
private:
	void UpdateProducerList(void);
	BOOL fCalcTempBalloonTrajectories;
	std::vector<NFmiProducerHelperInfo> itsProducerList;
	CComboBox itsProducerSelector;
	bool fProducerSelectorUsedYet;
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonSaveTrajectory();
	afx_msg void OnCbnSelchangeComboTrajectoryProducerSelector();
    afx_msg void OnBnClickedRadioTrajectoryDataType1();
    afx_msg void OnBnClickedRadioTrajectoryDataType2();
    afx_msg void OnBnClickedRadioTrajectoryDataType3();
    afx_msg void OnBnClickedRadioTrajectoryDataType4();
};
