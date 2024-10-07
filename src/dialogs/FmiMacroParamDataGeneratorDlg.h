#pragma once
#include "afxdialogex.h"
#include "NFmiViewPosRegistryInfo.h"
#include "NFmiQueryDataUtil.h"
#include "TextProgressCtrl.h"
#include "WndResizer.h"
#include "GridCtrl.h"

class SmartMetDocumentInterface;
class NFmiMacroParamDataGenerator;
class NFmiMacroParamDataInfo;
class NFmiMacroParamDataAutomationListItem;

struct MacroParDataAutomationHeaderParInfo
{
	enum ColumnFunction
	{
		kRowNumber = 0,
		kAutomationName,
		kEnable,
		kNextRuntime,
		kLastRuntime,
		kAutomationStatus,
		kAutomationPath,
	};

	MacroParDataAutomationHeaderParInfo()
		:itsHeader()
		, itsColumnFunction(kAutomationName)
		, itsColumnWidth(0)
	{}

	MacroParDataAutomationHeaderParInfo(const std::string& theHeader, ColumnFunction theColumnFunction, int theColumnWidth)
		:itsHeader(theHeader)
		, itsColumnFunction(theColumnFunction)
		, itsColumnWidth(theColumnWidth)
	{}

	std::string itsHeader;
	ColumnFunction itsColumnFunction;
	int itsColumnWidth;
};

// Pit‰‰ peri‰ oma gridCtrl-luokka, ett‰ se osaa hanskata parit kikat
class NFmiMacroParDataAutomationGridCtrl : public CGridCtrl
{
	DECLARE_DYNCREATE(NFmiMacroParDataAutomationGridCtrl)

public:
	NFmiMacroParDataAutomationGridCtrl(int nRows = 0, int nCols = 0, int nFixedRows = 0, int nFixedCols = 0)
		:CGridCtrl(nRows, nCols, nFixedRows, nFixedCols)
		, itsLeftClickUpCallback()
	{}

	void SetLeftClickUpCallback(std::function<void(const CCellID&)> theCallback) { itsLeftClickUpCallback = theCallback; }
	void SetRightClickUpCallback(std::function<void(const CCellID&)> theCallback) { itsRightClickUpCallback = theCallback; }

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

private:
	std::function<void(const CCellID&)> itsLeftClickUpCallback;
	std::function<void(const CCellID&)> itsRightClickUpCallback;
};

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
	void LaunchMacroParamDataGeneration();
	void InitControlsFromLoadedMacroParamDataInfo();
	void UpdateMacroParamDataInfoName(const std::string& fullPath);
	void UpdateMacroParamDataAutomationListName(const std::string& fullPath);
	void EnableButtons();
	void InitMaxGeneratedFilesKept(int maxGeneratedFilesKept);
	int GetMaxGeneratedFilesKept();
	void DoResizerHooking();
	void SelectedGridCell(const CCellID& theSelectedCell);
	void DeselectGridCell(const CCellID& theSelectedCell);
	void InitHeaders();
	void AddAutomationToList(const std::string& theFullFilePath);
	void UpdateAutomationList();
	void SetGridRow(int row, const NFmiMacroParamDataAutomationListItem& theListItem);
	void HandleEnableAutomationCheckBoxClick(int col, int row);
	void StartDataGenerationControlEnablations();
	void LaunchOnDemandMacroParamDataAutomation(int selectedAutomationIndex, bool doOnlyEnabled);
	void InitEditedMacroParamDataInfo(const NFmiMacroParamDataInfo& macroParamInfoFromDocument);
	void StoreControlValuesToEditedMacroParamDataInfo();
	void UpdateGeneratedDataInfoStr(const std::string& status, bool workFinished, bool operationStatus);
	void ShowCancelButton(bool show);
	void DoUserStartedDataGenerationPreparations(const std::string& infoStr);
	void DoOnStopDataGeneration(const std::string& stopMethodName, bool workFinished, bool operationStatus);
	void ResetProgressControl();
	void DoControlColoringUpdates(int controlId);
	void AddSmarttoolToList(const std::string& newFilePath);
	void DoControlColoring(CDC* pDC, bool status);
	void InitCpuUsagePercentage(double cpuUsagePercentage);
	std::pair<bool,double> GetCpuUsagePercentage();

	NFmiMacroParDataAutomationGridCtrl itsGridCtrl;
	std::vector<MacroParDataAutomationHeaderParInfo> itsHeaders;
	bool fGridControlInitialized = false;
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
	// Polut niihin smarttool skriptein, jonka avulla MacroParam dataan lasketaan arvot.
	// Voi siis olla yksi tai pilkuilla erotellut polut listassa.
	CString itsUsedDataGenerationSmarttoolPathList;
	// T‰m‰n avulla v‰ritet‰‰n static_text kontrolli punaiseksi, jos inputissa vikaa
	bool fUsedDataGenerationSmarttoolPathListHasInvalidValues = false;
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
	std::string mLoadedMacroParamDataInfoFullPath;
	// T‰h‰n tulee datan nimi ja generoinnin tulos lyhyesti
	CString mGeneratedDataInfoStr;
	// Jos t‰m‰ on true, laitetaan mGeneratedDataInfoStr teksti punaiseksi, muuten mustaksi
	bool fDataGenerationStoppedOrFailed = false;
	CWndResizer m_resizer;
	// Sellaisten nappuloiden jotka halutaan enable/disable tiloihin muuttujat
	CButton itsGenerateMacroParamDataButton;
	CButton itsSaveMacroParamDataInfoButton;
	CButton itsSaveMacroParamDataAutomationListButton;
	CButton itsRunSelectedMacroParamDataAutomationButton;
	CButton itsRunEnabledMacroParamDataAutomationButton;
	CButton itsRunAllMacroParamDataAutomationButton;

	CButton itsAddEditedMacroParamDataAutomationToListButton;
	CButton itsAddFromFileMacroParamDataAutomationToListButton;
	CButton itsRemoveMacroParamDataAutomationFromListButton;
	CButton itsLoadMacroParamDataAutomationListButton;
	CButton itsLoadMacroParamDataInfoButton;
	CButton itsCancelDataGenerationButton;
	CBrush itsCancelButtonBackgroundBrush;

	BOOL fAutomationModeOn;
	// Kun MP-data-automaatiolista ladataan tai talletetaan, t‰h‰n laitetaan sen tiedosto nimi tai polku
	CString mLoadedMacroParamDataAutomationListName;
	std::string mLoadedMacroParamDataAutomationListFullPath;
	// T‰h‰n talletetaan kaikki MacroParamDataInfo inputit ja t‰m‰ tekee tarkastelut niiden oikeellisuudesta
	std::shared_ptr<NFmiMacroParamDataInfo> itsEditedMacroParamDataInfo;
	// Base data grid scale string esim. 2.0 tai 1.5,2.0
	CString mBaseDataGridScaleString;
	bool fBaseDataGridScaleHasInvalidValues = false;
	CString itsCpuUsagePercentageString;
	bool fCpuUsagePercentageHasInvalidValues = false;

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
	afx_msg void OnChangeEditUsedDataGenerationSmarttoolPathList();
	afx_msg void OnChangeEditGeneratedDataStorageFileFilter();
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnBnClickedButtonSaveMacroParamData();
	afx_msg void OnBnClickedButtonLoadMacroParamData();
	afx_msg void OnEnChangeEditUsedDataTriggerList();
	afx_msg void OnEnChangeEditMaxGeneratedFilesKept();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnBnClickedButtonAddEditedMacroParamDataAutomationToList();
	afx_msg void OnBnClickedButtonAddMacroParamDataAutomationToList();
	afx_msg void OnBnClickedButtonRemoveMacroParamDataAutomationFromList();
	afx_msg void OnBnClickedCheckMacroParamDataAutomationModeOn();
	afx_msg void OnBnClickedButtonSaveMacroParamDataAutomationList();
	afx_msg void OnBnClickedButtonLoadMacroParamDataAutomationList();
	afx_msg void OnBnClickedButtonRunSelectedMacroParamDataAutomation();
	afx_msg void OnBnClickedButtonRunEnabledMacroParamDataAutomations();
	afx_msg void OnBnClickedButtonRunAllMacroParamDataAutomations();
	afx_msg void OnBnClickedButtonCancelDataGeneration();
	afx_msg void OnBnClickedButtonAddUsedSmarttoolPath();
	afx_msg void OnEnChangeEditBaseDataGridScale();
	afx_msg void OnEnChangeEditCpuUsagePercentage();
};
