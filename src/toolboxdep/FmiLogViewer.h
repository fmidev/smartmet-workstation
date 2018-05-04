#pragma once

#include "SmartMetToolboxDep_resource.h"
#include "NFmiViewPosRegistryInfo.h"
#include "GridCtrl.h"
#include "WndResizer.h"

#include <memory>

class NFmiApplicationWinRegistry;
namespace CatLog
{
    struct LogData;
}

struct LogMessageHeaderParInfo
{
    enum ColumnFunction
    {
        kRowNumber = 0,
        kTime,
        kCategory,
        kSeverity,
        kMessage
    };

    LogMessageHeaderParInfo(void)
        :itsHeader()
        , itsColumnFunction(kRowNumber)
        , itsColumnWidth(0)
    {}

    LogMessageHeaderParInfo(const std::string &theHeader, ColumnFunction theColumnFunction, int theColumnWidth)
        :itsHeader(theHeader)
        , itsColumnFunction(theColumnFunction)
        , itsColumnWidth(theColumnWidth)
    {}

    std::string itsHeader;
    ColumnFunction itsColumnFunction;
    int itsColumnWidth;
};

// Must inherit own GridCtrl class, so that right mouse click will deselect selection
class NFmiLogViewerGridCtrl : public CGridCtrl
{
    DECLARE_DYNCREATE(NFmiLogViewerGridCtrl)

public:
    NFmiLogViewerGridCtrl(int nRows = 0, int nCols = 0, int nFixedRows = 0, int nFixedCols = 0)
        :CGridCtrl(nRows, nCols, nFixedRows, nFixedCols)
    {}

public:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
};

const int kFmiRefreshTimerId = 1; // Tämä on tämän luokan käyttämän timerin id, jos tulee muitakin timer virityksiä, tällä voidaan erotella eventit toisistaan (OnTimer metodissa).

// CFmiLogViewer dialog
class CFmiLogViewer : public CDialogEx
{
    // Koodia, mitä tarvitaan näytön paikan muistamiseen rekisterissä
private:
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void) { return s_ViewPosRegistryInfo; }
    static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) { return ViewPosRegistryInfo().WinRegistryKeyStr(); }
    void SetDefaultValues(void);
    // Koodia, mitä tarvitaan näytön paikan muistamiseen rekisterissä

    DECLARE_DYNAMIC(CFmiLogViewer)

public:
	CFmiLogViewer(NFmiApplicationWinRegistry &theApplicationWinRegistry, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiLogViewer();
    void Update();
    void StartRefreshTimer();

	enum { IDD = IDD_DIALOG_LOG_VIEWER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

private:
    void DoWhenClosing();
    void InitDialogTexts();
    void InitHeaders();
    void DoResizerHooking();
    void GetLogMessages();
    void UpdateGridRows();
    void ShowLatestRowOnView();
    void InitLogLevelComboBox();
    void InitCategoryComboBox();
    void FitLastColumnOnVisibleArea();

    NFmiApplicationWinRegistry &itsApplicationWinRegistry; // Tätä tarvitaan että näytön paikka saadaan talteen Windows rekisteriin

    NFmiLogViewerGridCtrl itsGridCtrl;
    std::vector<LogMessageHeaderParInfo> itsHeaders;
    CString itsSearchTextU_; // Jos ei tyhjä, näytetään vain ne lokiviestit, joista löytyy tälläinen teksti (jostain kohtaa, myös kategoriasta tai severitystä)
    CComboBox itsCategoryComboBox; // Jos ei All -valittuna (= 0:s valinta), näytetään vain valitun kategorian viestit
    CComboBox itsSeverityComboBox; // Jos ei All -valittuna (= 0:s valinta), näytetään vain valittua severityn ja sitä tärkeämmät viestit

    CWndResizer m_resizer;
    std::vector<std::shared_ptr<CatLog::LogData>> itsLogMessages;
    BOOL fFollowLatestMessage;

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnEnChangeEditLogViewerSearchText();
    afx_msg void OnCbnSelchangeComboLogViewerCategory();
    afx_msg void OnCbnSelchangeComboLogViewerSeverity();
    virtual BOOL OnInitDialog();

    virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnClose();

    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    afx_msg void OnBnClickedCheckFollowLatest();
    afx_msg void OnSize(UINT nType, int cx, int cy);
};
