#pragma once

#include "SmartMetDialogs_resource.h"
#include "NFmiViewPosRegistryInfo.h"
#include "WndResizer.h"
#include "GUSIconEdit.h"
#include "GridCtrl.h"
#include "NFmiCachedRegistryValue.h"
#include "afxwin.h"
#include <functional>
#include "boost/shared_ptr.hpp"

class SmartMetDocumentInterface;
class NFmiLightWeightViewSettingMacro;

struct ViewMacroHeaderParInfo
{
    enum ColumnFunction
    {
        kRowNumber = 0,
        kMacroName,
        kModificationDate,
        kDescription
    };

    ViewMacroHeaderParInfo(void)
        :itsHeader()
        , itsColumnFunction(kMacroName)
        , itsColumnWidth(0)
    {}

    ViewMacroHeaderParInfo(const std::string &theHeader, ColumnFunction theColumnFunction, int theColumnWidth)
        :itsHeader(theHeader)
        , itsColumnFunction(theColumnFunction)
        , itsColumnWidth(theColumnWidth)
    {}

    std::string itsHeader;
    ColumnFunction itsColumnFunction;
    int itsColumnWidth;
};

// Pit‰‰ peri‰ oma gridCtrl-luokka, ett‰ se osaa hanskata parit kikat
class NFmiViewMacroGridCtrl : public CGridCtrl
{
    DECLARE_DYNCREATE(NFmiViewMacroGridCtrl)

public:
    NFmiViewMacroGridCtrl(int nRows = 0, int nCols = 0, int nFixedRows = 0, int nFixedCols = 0)
        :CGridCtrl(nRows, nCols, nFixedRows, nFixedCols)
        , itsLeftClickUpCallback()
        , itsLeftDoubleClickUpCallback()
    {}

    void SetLeftClickUpCallback(std::function<void(const CCellID&)> theCallback) { itsLeftClickUpCallback = theCallback; }
    void SetLeftDoubleClickUpCallback(std::function<void(const CCellID&)> theCallback) { itsLeftDoubleClickUpCallback = theCallback; }

public:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

private:
    std::function<void(const CCellID&)> itsLeftClickUpCallback;
    std::function<void(const CCellID&)> itsLeftDoubleClickUpCallback;
};

// CFmiViewMacroDlg dialog

class CFmiViewMacroDlg : public CDialog
{
private: // n‰ill‰ talletetaan sijainti ja koko rekisteriin
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void){return s_ViewPosRegistryInfo;}

	DECLARE_DYNAMIC(CFmiViewMacroDlg)

public:
	CFmiViewMacroDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiViewMacroDlg();
	void SetDefaultValues(void);
    static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) {return ViewPosRegistryInfo().WinRegistryKeyStr();}
    void Update(void);
    void ResetSearchResource();
    void LeftClickedGridCell(const CCellID &theSelectedCell); // k‰ytet‰‰n gridControllista callbackina
    void DoubleLeftClickedGridCell(const CCellID &theSelectedCell); // k‰ytet‰‰n gridControllista callbackina

// Dialog Data
	enum { IDD = IDD_DIALOG_VIEW_MACRO_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonLoad();
	afx_msg void OnBnClickedButtonStore();
	afx_msg void OnBnClickedButtonRemove();
	afx_msg void OnSize(UINT nType, int cx, int cy);
private:
	void CreateNewDirectory(const std::string &thePath);
    void ChangeDirectory(const std::string &theDirectoryName, bool fDoSpeedSearchLoad);
	bool IsNewMacro(const std::string &theName, int &theIndex);
	void LoadMacro(int theIndex);
	void DoWhenClosing(void);
    void DoResizerHooking(void);
	void InitDialogTexts(void);
	void UpdateDialogButtons(void);
    void InitSpeedSearchControl();
    void DoSpeedSearchMacroSelection(const std::string &theViewMacroName);
    void DoTimedResetSearchResource();
    void InitHeaders(void);
    int GetSelectedViewMacroRow();
    void SelectMacro(const std::string &theFilePath);
    std::string GetSelectedMacroName();
    std::string GetMacroName(int index);
    void SetGridRow(int row, const NFmiLightWeightViewSettingMacro &theListItem);
    bool IsClickedCellOk(const CCellID &theSelectedCell);
    void StoreWinRegistrySettings();
    void EnsureWantedRowVisibilityAfterDirectoryChange(const std::string &theDirectoryName, const std::string &theOldPath);
    int FindMacroNameRow(const std::string &theMacroName);
    void EnableColorCodedControls();

    NFmiViewMacroGridCtrl itsGridCtrl;
    std::vector<ViewMacroHeaderParInfo> itsHeaders;
    bool fGridControlInitialized;

    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
    CString itsMacroDescriptionU_;
    std::string itsSelectedMacroName;
    BOOL fDisableWindowManipulations;
    CWndResizer m_resizer;
    CGUSIconEdit itsSpeedSearchViewMacroControl;
    CMenu itsSpeedSearchViewMacroControlMenu;
    BOOL fSearchOptionCaseSensitive;
    BOOL fSearchOptionMatchAnywhere;

    // Windows rekistereihin talletettavia asetuksia
    boost::shared_ptr<CachedRegInt> mRowColumnWidth; // Row sarakkeen leveys pikseleiss‰
    boost::shared_ptr<CachedRegInt> mNameColumnWidth; // leveys pikseleiss‰
    boost::shared_ptr<CachedRegInt> mModifiedColumnWidth; // leveys pikseleiss‰
    boost::shared_ptr<CachedRegInt> mDescriptionColumnWidth; // leveys pikseleiss‰

public:
	virtual BOOL OnInitDialog();
	void InitMacroListFromDoc(void);
	afx_msg void OnBnClickedButtonRefreshList();
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonViewMacroUndo();
	afx_msg void OnBnClickedButtonViewMacroRedo();
	afx_msg void OnBnClickedCheckDisableWindowManipulation();
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    afx_msg LRESULT OnGUSIconEditLeftIconClicked(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGUSIconEditRightIconClicked(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGUSIconEditSearchListClicked(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSearchOptionCaseSesensitive();
    afx_msg void OnSearchOptionMatchAnywhere();
    afx_msg void OnEnChangeEditSpeedSearchViewMacro();
protected:
    virtual void OnOK();
    virtual void OnCancel();
public:
    CString itsCurrentPathStringU_;
    afx_msg void OnDestroy();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
