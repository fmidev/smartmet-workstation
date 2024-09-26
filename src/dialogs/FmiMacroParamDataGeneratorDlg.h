#pragma once
#include "afxdialogex.h"
#include "NFmiViewPosRegistryInfo.h"
#include "NFmiQueryDataUtil.h"
#include "TextProgressCtrl.h"
#include "WndResizer.h"

class SmartMetDocumentInterface;
class NFmiMacroParamDataGenerator;
class NFmiMacroParamDataInfo;

// CFmiMacroParamDataGeneratorDlg dialog

class CFmiMacroParamDataGeneratorDlg : public CDialogEx, public NFmiOperationProgress
{
	// n‰ill‰ talletetaan sijainti ja koko rekisteriin
	static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;

	DECLARE_DYNAMIC(CFmiMacroParamDataGeneratorDlg)

public:
	static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void) { return s_ViewPosRegistryInfo; }
	static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) { return ViewPosRegistryInfo().WinRegistryKeyStr(); }

	CFmiMacroParamDataGeneratorDlg(SmartMetDocumentInterface* smartMetDocumentInterface, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CFmiMacroParamDataGeneratorDlg();
	void SetDefaultValues(void);
	void Update();
	void StoreControlValuesToDocument();

	// overrides from NFmiOperationProgress
	void StepIt(void) override;
	void SetRange(int low, int high, int stepCount) override;
	void AddRange(int value) override;
	bool DoPostMessage(unsigned int message, unsigned int wParam = 0, long lParam = 0) override;
	bool WaitUntilInitialized(void) override;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_MACRO_PARAM_DATA_GENERATOR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	void InitDialogTexts();
	void InitControlsFromDocument();
	void DoFullInputChecks();
	void DoWhenClosing();
	void EnableDialogueControl(int controlId, bool enable);
	void LaunchMacroParamDataGeneration();
	void InitControlsFromLoadedMacroParamDataInfo(const NFmiMacroParamDataInfo &macroParamsDataInfo);
	void UpdateMacroParamDataInfoName(const std::string& fullPath);
	void EnableButtons();
	void InitMaxGeneratedFilesKept(int maxGeneratedFilesKept);
	int GetMaxGeneratedFilesKept();
	void DoResizerHooking();

	// itsSmartMetDocumentInterface ei omista, ei tuhoa
	SmartMetDocumentInterface* itsSmartMetDocumentInterface;
	// itsMacroParamDataGenerator ei omista, ei tuhoa
	NFmiMacroParamDataGenerator* itsMacroParamDataGenerator;
	// Annetaan esim. arvo T_Ec, joka tarkoittaa se Ecmwf pintadata, jossa on l‰mpˆtila parametri
	CString itsBaseDataParamProducerString;
	// T‰m‰n avulla v‰ritet‰‰n static_text kontrolli punaiseksi, jos inputissa vikaa
	bool fBaseDataParamProducerStringHasInvalidValues = false;
	// Annetaan tuotettavan MacroParam datan tuottaja tiedot, esim. 5601,My Producer Name
	CString itsProducerIdNamePairString;
	// T‰m‰n avulla v‰ritet‰‰n static_text kontrolli punaiseksi, jos inputissa vikaa
	bool fProducerIdNamePairStringHasInvalidValues = false;
	// Polku siihen smarttool skriptiin, jonka avulla MacroParam dataan lasketaan arvot
	CString itsUsedDataGenerationSmarttoolPath;
	// T‰m‰n avulla v‰ritet‰‰n static_text kontrolli punaiseksi, jos inputissa vikaa
	bool fUsedDataGenerationSmarttoolPathHasInvalidValues = false;
	// Annetaan pilkuilla erotettu lista parametreja (id,name pareina), joista muodostuu MacroParam datan parametrit
	CString itsUsedParameterListString;
	// T‰m‰n avulla v‰ritet‰‰n static_text kontrolli punaiseksi, jos inputissa vikaa
	bool fUsedParameterListStringHasInvalidValues = false;
	// Tuotettavan datan polku ja file filter, esim. c:\data\*_mydata.sqd, miss‰ * korvataan YYYYMMDDHHmmSS timestamp tekohetkest‰
	CString itsGeneratedDataStorageFileFilter;
	// T‰m‰n avulla v‰ritet‰‰n static_text kontrolli punaiseksi, jos inputissa vikaa
	bool fGeneratedDataStorageFileFilterHasInvalidValues = false;
	// Annetaan pilkulla eroteltuja triggeri datoja (T_ec[,...]), voi olla myˆs tyhj‰.
	CString itsDataTriggerList;
	// T‰m‰n avulla v‰ritet‰‰n static_text kontrolli punaiseksi, jos inputissa vikaa
	bool fDataTriggerListHasInvalidValues = false;
	CString itsMaxGeneratedFilesKept;

	// Jos macroparam dataa ollaan generoimassa, t‰ss‰ on tarkoitus kertoa sen edistymisest‰
	CTextProgressCtrl mProgressControl;
	NFmiStopFunctor mStopper;
	std::unique_ptr<NFmiThreadCallBacks> mThreadCallBacksPtr;
	// Kun MP-data-info ladataan tai talletetaan, t‰h‰n laitetaan sen tiedosto nimi tai polku
	CString mLoadedMacroParamDataInfoName;
	CWndResizer m_resizer;

public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonGenerateMacroParamData();
	afx_msg void OnChangeEditBaseDataParamProducer();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnChangeEditProducerIdNamePair();
	afx_msg void OnChangeEditUsedParameterList();
	afx_msg void OnChangeEditUsedDataGenerationSmarttoolPath();
	afx_msg void OnChangeEditGeneratedDataStorageFileFilter();
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnBnClickedButtonSaveMacroParamData();
	afx_msg void OnBnClickedButtonLoadMacroParamData();
	afx_msg void OnEnChangeEditUsedDataTriggerList();
	afx_msg void OnEnChangeEditMaxGeneratedFilesKept();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
};
