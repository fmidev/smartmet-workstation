#pragma once

#include "SmartMetDialogs_resource.h"
#include "GridCtrl.h"
#include "TreeColumn.h"
#include <vector>
#include <memory>
#include "NFmiViewPosRegistryInfo.h"
#include "WndResizer.h"
#include "PPTooltip.h"
#include <list>

class SmartMetDocumentInterface;
class NFmiCaseStudyDataFile;
class NFmiProducerSystemsHolder;
class NFmiCaseStudySettingsWinRegistry;

struct CaseStudyHeaderParInfo
{
	enum ColumnFunction
	{
		kRowNumber = 0,
		kModelName,
        kProducerId,
		kStoreData,
		kCaseStudyDataOffsetRange,
		kLocalCacheDataCount,
		kEnableData, // t‰t‰ ei saa laittaa viimeiseen sarakkeeseen, koska sit‰ on tarkoitus piilottaa ja CGridCtrl-luokassa on bugi, joka estaa viimeisen sarakkeen Unhide-toiminnon eston, eli viimeisen sarakkeen saa aina auki hiirell‰ vet‰m‰ll‰.
		kDataSize
	};

	CaseStudyHeaderParInfo()
	:itsHeader()
	,itsColumnFunction(kModelName)
	,itsColumnWidth(0)
	{}

	CaseStudyHeaderParInfo(const std::string &theHeader, ColumnFunction theColumnFunction, int theColumnWidth)
	:itsHeader(theHeader)
	,itsColumnFunction(theColumnFunction)
	,itsColumnWidth(theColumnWidth)
	{}

	std::string itsHeader;
	ColumnFunction itsColumnFunction;
	int itsColumnWidth;
};

// pit‰‰ peri‰ oma gridCtrl-luokka, ett‰ se osaa hanskata parit kikat
class NFmiCaseStudyGridCtrl : public CGridCtrl
{
    DECLARE_DYNCREATE(NFmiCaseStudyGridCtrl)

public:
    NFmiCaseStudyGridCtrl(int nRows = 0, int nCols = 0, int nFixedRows = 0, int nFixedCols = 0)
	:CGridCtrl(nRows, nCols, nFixedRows, nFixedCols)
	{}

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void NotifyDisplayTooltip(NMHDR* pNMHDR, LRESULT* result);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
private:
	std::string ComposeToolTipText(const CPoint& point);
	bool IsEnableDataColumnVisible() const;

	CPPToolTip m_tooltip;
};

const int kFmiDataCountEditTimer = 1;
const int kFmiDisableStoreButtonTimer = 2;

// CFmiCaseStudyDlg dialog

class CFmiCaseStudyDlg : public CDialog
{
private: // n‰ill‰ talletetaan sijainti ja koko rekisteriin
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo() {return s_ViewPosRegistryInfo;}

    DECLARE_DYNAMIC(CFmiCaseStudyDlg)

public:
	CFmiCaseStudyDlg(SmartMetDocumentInterface *smartMetDocumentInterface, const std::string theTitleStr, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiCaseStudyDlg();
	void SetDefaultValues();
	virtual BOOL Create(CWnd* pParentWnd = NULL);
	void GetBasicInfoFromDialog();
    static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) {return ViewPosRegistryInfo().WinRegistryKeyStr();}
	// Update metodia tarvitaan (piti luoda tyhj‰ sellainen) kun kyseinen toiminto toteutetaan CSmartmetDoc luokassa SetViewPlaceToDefault -template yleisfunktiolla
	void Update(){}

// Dialog Data
	enum { IDD = IDD_DIALOG_CASE_STUDY_STORE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	void UpdateButtonStates();
	void EnableButton(UINT theButtonId, BOOL state);
	void InitializeGridControlRelatedData();
	void DoWhenClosing();
	void InitGridControlValues();
    void UpdateRows(int fixedRowCount, int fixedColumnCount, bool updateOnly);
	void UpdateGridControlValues(bool updateOnly);
	void InitHeaders();
	void HandleCheckBoxClick(int col, int row);
	void HandleEnableDataCheckBoxClick(int col, int row);
	void SetGridRow(int row, const NFmiCaseStudyDataFile &theCaseData, int theFixedColumnCount, bool updateOnly);
	BOOL AcceptChange(int col, int row);
	void OnDataCountEdited();
	bool IsThereCaseStudyMakerRunning();
    void ShowEnableColumn();
    void UpdateEditEnableDataText();
    void EnableColorCodedControls();
	void InitDialogTexts();
	void AdjustGridControl();
	CRect CalcGridArea();
	void FitNameColumnOnVisibleArea(int gridCtrlWidth);

	NFmiCaseStudyGridCtrl itsGridCtrl;
    CTreeColumn itsTreeColumn;   // provides tree column support
	std::vector<CaseStudyHeaderParInfo> itsHeaders;
	bool fGridControlInitialized;
	bool fDialogInitialized; // t‰m‰ on OnSize -s‰‰tˆj‰ varten, 1. kerralla ei saa s‰‰t‰‰ ikkunoita, koska niit‰ ei ole viel‰ olemassa.

    SmartMetDocumentInterface *itsSmartMetDocumentInterface; // ei omista, ei tuhoa
	std::string itsTitleStr;
	UINT itsOffsetEditTimer; // en saa jostain syyst‰ l‰hetetty‰ sanomaa t‰ss‰ CDialog-perityss‰ luokassa (ON_MESSAGE -makro ei suostu k‰‰ntym‰‰n), lis‰ksi 
							// aiemmin totesin ett‰ sanomien l‰hetys ei vain syyst‰ tai toisesta onnistu (Esim. OnWndMsg -virtuaali funktio ei saa haluttuja sanomia mill‰‰n)
							// T‰st‰ syyst‰ kun tehd‰‰n takastelu offset-editoinnista ja syˆte hyv‰ksyt‰‰n, k‰ynnistet‰‰n lyhyt itsOffsetEditTimer -timer,
							// ett‰ voidaan tehd‰ hyv‰ksynn‰n j‰lkeen tietorakenteiden p‰ivitys ja hilan‰ytˆn arvojen p‰ivitys.
	UINT itsDisableStoreButtonTimer; // halutaan sallia vain yksi CaseStudy-talletus kerrallaan, joten disabloin Store-napin tietyiss‰ tilanteissa.
									// Nappi pit‰‰ tietenkin joskus enabloida ja se tehd‰‰n timerin avulla eli esim. kahden sekunnin v‰lein k‰yd‰‰n tarkistamassa
									// ett‰ onko se Case study ohjelma jo lopettanut...
	CCellID itsOffsetEditedCell; // t‰h‰n talletetaan sen hila-alkion sijainti, mit‰ ollaan editoitu, ett‰ timerist‰ kutsuttavasta p‰ivitys funktiosta tiedet‰‰n 
								// mit‰ solua on editoitu.
	LOGFONT itsBoldFont; // haluan tietyissa tapauksissa k‰ytt‰‰ bold-fonttia ja t‰h‰n talletetaan sen rakenne
    std::unique_ptr<NFmiProducerSystemsHolder> itsProducerSystemsHolder; // T‰h‰n laitetaan selkokieliset tuottaja nimet, joiden avulla rakennetaan CaseStudy datojen rakenteita CaseStudySystemissa.
	NFmiCaseStudySettingsWinRegistry& itsCaseStudySettingsWinRegistry;
public:
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
	virtual void OnCancel();
	virtual void OnOK();
    afx_msg void OnGridEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult);
public:
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
private:
	std::string AddPossibleZippingOptions() const;
	std::string AddPossibleHakeMessageOptions() const;
	std::string AddPossibleCropDataToZoomedMapAreaOptions() const;
	void DoResizerHooking();
	void DoLocalCacheCountEditing(NFmiCaseStudyDataFile& dataFile, const std::string& cellText);
	void DoCaseStudyOffsetRangeEditing(NFmiCaseStudyDataFile& dataFile, const std::string& cellText);
	std::pair<std::string, std::string> GetNameAndInfoStringsFromSelectedMetaFilePath(std::string fullPathToMetaFile);
	void DoFirstTimeSilamCategoryCollapse();
	std::list<std::string> GetSelectedDataFileFilters();

    CString itsNameStrU_;
    CString itsInfoStrU_;
    CString itsPathStrU_;
    BOOL fEditEnableData;
    BOOL fZipData;
	BOOL fStoreWarningMessages;
	BOOL fCropDataToZoomedMapArea;
	CWndResizer m_resizer;
	int itsPossibleSilamCategoryRowNumber = -1;
public:
	afx_msg void OnBnClickedButtonStoreData();
	afx_msg void OnBnClickedButtonLoadData();
	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnBnClickedButtonCloseMode();
    afx_msg void OnBnClickedCheckEditEnableData();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnBnClickedButtonRefreshGrid();
	afx_msg void OnBnClickedButtonBrowseFolder();
	afx_msg void OnBnClickedButtonPrioritizeData();
	afx_msg void OnBnClickedButtonLoadOldData();
};
