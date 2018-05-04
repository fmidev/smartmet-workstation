#pragma once

#include "SmartMetToolboxDep_resource.h"
#include "WndResizer.h"
#include "afxwin.h"
#include "TabCtrlSSL.h"
#include "DynamicSplitterHelper.h"
#include "NFmiMetTime.h"
#include "GUSIconEdit.h"

#include "GridCtrl.h"
#include "TreeColumn.h"
#include "NFmiApplicationWinRegistry.h"
#include "FmiSmarttoolsTabDlgHelpers.h"
#include "NFmiMilliSecondTimer.h"

#include <functional>
#include "afxcmn.h"

class SmartMetDocumentInterface;
class NFmiSmartToolInfo;

struct SmarttoolsMacroHeaderParInfo
{
    enum ColumnFunction
    {
        kRowNumber = 0,
        kMacroName,
        kModificationDate
    };

    SmarttoolsMacroHeaderParInfo(void)
        :itsHeader()
        , itsColumnFunction(kMacroName)
        , itsColumnWidth(0)
    {}

    SmarttoolsMacroHeaderParInfo(const std::string &theHeader, ColumnFunction theColumnFunction, int theColumnWidth)
        :itsHeader(theHeader)
        , itsColumnFunction(theColumnFunction)
        , itsColumnWidth(theColumnWidth)
    {}

    std::string itsHeader;
    ColumnFunction itsColumnFunction;
    int itsColumnWidth;
};

// Pit‰‰ peri‰ oma gridCtrl-luokka, ett‰ se osaa hanskata parit kikat
class NFmiSmarttoolsMacroGridCtrl : public CGridCtrl
{
    DECLARE_DYNCREATE(NFmiSmarttoolsMacroGridCtrl)

public:
    NFmiSmarttoolsMacroGridCtrl(int nRows = 0, int nCols = 0, int nFixedRows = 0, int nFixedCols = 0)
        :CGridCtrl(nRows, nCols, nFixedRows, nFixedCols)
        , itsLeftClickUpCallback()
        , itsLeftDoubleClickUpCallback()
        , itsRightClickUpCallback()
    {}

    void SetLeftClickUpCallback(std::function<void(const CCellID&)> theCallback) { itsLeftClickUpCallback = theCallback; }
    void SetLeftDoubleClickUpCallback(std::function<void(const CCellID&)> theCallback) { itsLeftDoubleClickUpCallback = theCallback; }
    void SetRightClickUpCallback(std::function<void(const CCellID&)> theCallback) { itsRightClickUpCallback = theCallback; }
    void SetHeaders(const std::vector<SmarttoolsMacroHeaderParInfo> &theHeaders);

public:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);

private:
    std::function<void(const CCellID&)> itsLeftClickUpCallback;
    std::function<void(const CCellID&)> itsLeftDoubleClickUpCallback;
    std::function<void(const CCellID&)> itsRightClickUpCallback;
};

class MacroTreeItemData
{
public:
    std::string itsOriginalFullName;
    std::string itsDisplayedName;
    NFmiTime itsModifiedTime;
    bool fIsFolder;
    size_t itsIndexInOriginalContainer;
    int itsTreeLevel;
    std::vector<MacroTreeItemData> itsDirectoryItems; // Jos kyse oli hakemistosta, t‰ss‰ on kaikki itemit siit‰
};

// CFmiSmarttoolTabDlg dialog

class CFmiSmarttoolTabDlg : public CTabPageSSL
{
    DECLARE_DYNAMIC(CFmiSmarttoolTabDlg)

public:
    CFmiSmarttoolTabDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
    virtual ~CFmiSmarttoolTabDlg();
    virtual BOOL Create(CWnd* pParentWnd = NULL); // modaalitonta dialogi luontia varten
    virtual BOOL Create(UINT nIDTemplate, CWnd* pParentWnd = NULL); // Tab dialogia varten
    void Update();
    void DoWhenClosing(void);
    void LeftClickedGridCell(const CCellID &theSelectedCell); // k‰ytet‰‰n gridControllista callbackina
    void DoubleLeftClickedGridCell(const CCellID &theSelectedCell); // k‰ytet‰‰n gridControllista callbackina
    void RightClickedGridCell(const CCellID &theSelectedCell); // k‰ytet‰‰n gridControllista callbackina

    // Dialog Data
    enum { IDD = IDD_DIALOG_SMARTTOOL_TAB };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

private:
    void DoResizerHooking(void);
    void InitHeaders(void);
    void StoreWinRegistrySettings();
    void InitMacroListFromDoc(void);
    void SetGridRow(int row, const SmartToolMacroData &theMacroData, FmiLanguage theLanguage);
    bool IsClickedCellOk(const CCellID &theSelectedCell);
    void ResetSearchResource();
    void InitSpeedSearchControl();
    void MakeSmartToolMacroDataContent(const std::vector<std::string> &theSmarttoolMacroNames, const std::string &theAbsoluteRootPath);
    void FillGridRowsWithDirectoryMacroData(SmartToolMacroData::ContainerType &theMacroDirectory, int &theCurrentRowNumberInOut, FmiLanguage theLanguage);
    bool IgnoreLeftClickAfterDoubleClick();
    void MarkLeftClickIgnoreActions();
    void PutSelectedMacroToEditingControl(const CCellID &theSelectedCell, bool doDoubleClickActions);
    void UpdatePathInfo(CString &thePathVariable, const std::string &theRelativeMacroPath, bool editedMacro);
    void MakeLeftClickBackups();
    void RestoreLeftClickBackups();

    CDynamicSplitterHelper itsSplitterHelper;
    CWndResizer m_resizer;
    CGUSIconEdit itsSpeedSearchMacroControl;
    CMenu itsSpeedSearchMacroControlMenu;
    BOOL fSearchOptionCaseSensitive;
    BOOL fSearchOptionMatchAnywhere;
    SmartToolMacroData itsSmartToolMacroDataTree;
    CRichEditCtrl itsMacroTextControl;
    CRichEditCtrl itsViewOnlyMacroTextControl;
    // Ei ole tiedosssa hyv‰‰ keinoa est‰‰ left-click toiminto, jos on tulossa viel‰ double-click.
    // Siksi kun grid-controllia klikataan hiiren vasemmalla, tehd‰‰n normaalit toiminnot, mutta laitetaan viewOnlyMacroTeksti talteen.
    // Jos sitten p‰‰tyd‰‰n tupla-klikkaus toimintoon, laitetaan editoivan macron tekstit kuntoon ja undo:ataan left-click
    // laittamalla takaisin talteen otettu stringi takaisin viewOnlyMacroTekstiksi.
    CString itsViewOnlyMacroText_DoubleClickBackupU_;
    // Lis‰ksi polku teksti pit‰‰ olla myˆs palautettavissa.
    CString itsViewOnlyMacroPathText_DoubleClickBackupU_;

    // Kun grid-controlissa tekee left double clickin, tapahtuu seuraavat callbackit: 1. left-click, 2. double-left-click ja 3. left-click 
    // Eli double clikin j‰lkeinen left-click pit‰‰kin ohittaa, jotta double-clikissa palautettu originaali teksti itsViewOnlyMacroTextControl:iin ei pyyhkiytyisi pois editointiin valitulla tekstill‰.
    // Lis‰ksi kyseinen tapahtuma pit‰‰ tapahtua tarpeeksi nopeasti, aina ei v‰ltt‰m‰tt‰ tule double-clickin j‰lkeen left-clicki‰ (esim. debugatessa).
    bool fIgnoreLeftClickAfterDoubleClick; 
    NFmiMilliSecondTimer itsIgnoreLeftClickTimer;

    CString itsEditedMacroPathU_;
    CString itsViewedMacroPathU_;

    NFmiSmarttoolsMacroGridCtrl itsGridCtrl;
    std::vector<SmarttoolsMacroHeaderParInfo> itsHeaders;
    bool fGridControlInitialized;
    CTreeColumn itsTreeColumn;   // provides tree column support for gridctrl
    // Windows rekistereihin talletettavia asetuksia
    boost::shared_ptr<CachedRegInt> mRowColumnWidth; // Row sarakkeen leveys pikseleiss‰
    boost::shared_ptr<CachedRegInt> mNameColumnWidth; // leveys pikseleiss‰
    boost::shared_ptr<CachedRegInt> mModifiedColumnWidth; // leveys pikseleiss‰


    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
    NFmiSmartToolInfo *itsSmartToolInfo;
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg LRESULT OnSplitterControlMove(WPARAM wParam, LPARAM lParam);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    afx_msg void OnBnClickedCheckHorizontalSplitScreen();
    afx_msg LRESULT OnGUSIconEditLeftIconClicked(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGUSIconEditRightIconClicked(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGUSIconEditSearchListClicked(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSearchOptionCaseSesensitive();
    afx_msg void OnSearchOptionMatchAnywhere();
    afx_msg void OnEnChangeEditSpeedSearchViewMacro();
    afx_msg void OnBnClickedButtonSmarttoolEdit();
};
