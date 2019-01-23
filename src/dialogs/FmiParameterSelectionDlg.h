#pragma once

#include "SmartMetDialogs_resource.h"
#include "GridCtrl.h"
#include "TreeColumn.h"
#include <vector>
#include <functional>
#include "NFmiViewPosRegistryInfo.h"
#include "PPTooltip.h"

class SmartMetDocumentInterface;

namespace AddParams
{
    class ParameterSelectionSystem;
    class SingleRowItem;
}

struct ParameterSelectionHeaderParInfo
{
    enum ColumnFunction
    {
        kRowNumber = 0,
        kItemName,
        kOrigOrLastTime,
        kItemId
    };

    ParameterSelectionHeaderParInfo(void)
        :itsHeader()
        , itsColumnFunction(kItemName)
        , itsColumnWidth(0)
    {}

    ParameterSelectionHeaderParInfo(const std::string &theHeader, ColumnFunction theColumnFunction, int theColumnWidth)
        :itsHeader(theHeader)
        , itsColumnFunction(theColumnFunction)
        , itsColumnWidth(theColumnWidth)
    {}

    std::string itsHeader;
    ColumnFunction itsColumnFunction;
    int itsColumnWidth;
};

// pit‰‰ peri‰ oma gridCtrl-luokka, ett‰ se osaa hanskata parit kikat
class NFmiParameterSelectionGridCtrl : public CGridCtrl
{
    DECLARE_DYNCREATE(NFmiParameterSelectionGridCtrl)

public:
    NFmiParameterSelectionGridCtrl(int nRows = 0, int nCols = 0, int nFixedRows = 0, int nFixedCols = 0)
        :CGridCtrl(nRows, nCols, nFixedRows, nFixedCols)
        , itsSmartMetDocumentInterface(nullptr)
    {}

    std::function<void(void)> itsLButtonDblClkCallback;
    void SetDocument(SmartMetDocumentInterface *smartMetDocumentInterface) { itsSmartMetDocumentInterface = smartMetDocumentInterface; };

private:
    std::string ComposeToolTipText(CPoint point);
    CPPToolTip m_tooltip;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
    std::string TooltipForCategoryType(AddParams::SingleRowItem singleRowItem, std::vector<AddParams::SingleRowItem> singleRowItemVector, int rowNumber);
    std::string TooltipForCategoryType();
public:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void NotifyDisplayTooltip(NMHDR * pNMHDR, LRESULT * result);
    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
};

// CFmiParameterSelectionDlg dialog

class CFmiParameterSelectionDlg : public CDialogEx
{
private: // n‰ill‰ talletetaan sijainti ja koko rekisteriin
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void) { return s_ViewPosRegistryInfo; }
    
    DECLARE_DYNAMIC(CFmiParameterSelectionDlg)

public:
	CFmiParameterSelectionDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiParameterSelectionDlg();
    void SetDefaultValues(void);
    static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) { return ViewPosRegistryInfo().WinRegistryKeyStr(); }
    void Update();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PARAM_ADDING };
#endif

private:
    void DoWhenClosing(void);
    void AdjustDialogControls(void);
    void AdjustGridControl(void);
    CRect CalcGridArea(void);
    void AdjustControlWidth(int theControlId, int rightOffset, int maxWidth);
    void InitHeaders();
    void UpdateGridControlValues(bool collapseAll = false);
    void UpdateGridControlValuesInNormalMode(bool fFirstTime);
    void UpdateGridControlValuesWhenSearchActive(void);
    void UpdateGridControlValuesWhenSearchRemoved(void);
    void ExpandAllNodes();
    void UpdateRows(int fixedRowCount, int fixedColumnCount, bool updateOnly);
    void SetGridRow(int row, const AddParams::SingleRowItem &theRowItem, int theFixedColumnCount);
    void InitDialogTexts(void);
    void MakeTreeNodeCollapseSettings();
    void SetTreeNodeInformationBackToDialogRowData();
    void HandleGridCtrlsLButtonDblClk();
    void HandleRowItemSelection(const AddParams::SingleRowItem &rowItem);
    void FitNameColumnOnVisibleArea(int gridCtrlWidth);
    std::string MakeActiveViewRowText();
    bool NeedToUpdateTitleText();
    std::string MakeTitleText();
    bool UpdateSearchIfNeeded();
    void CollapseAllButCategories();

    NFmiParameterSelectionGridCtrl itsGridCtrl;
    CTreeColumn itsTreeColumn;   // provides tree column support
    std::vector<ParameterSelectionHeaderParInfo> itsHeaders;
    bool fDialogInitialized; // t‰m‰ on OnSize -s‰‰tˆj‰ varten, 1. kerralla ei saa s‰‰t‰‰ ikkunoita, koska niit‰ ei ole viel‰ olemassa.

    SmartMetDocumentInterface *itsSmartMetDocumentInterface; // ei omista, ei tuhoa
    AddParams::ParameterSelectionSystem *itsParameterSelectionSystem;
    unsigned int itsLastAcivatedDescTopIndex; // Mik‰ oli viimeksi DescTopIndex, kun otsikon teksti‰ tehtiin
    int itsLastActivatedRowIndex; // Mik‰ oli viimeksi RowIndex, kun otsikon teksti‰ tehtiin
    CString itsSearchText;
    std::string itsPreviousSearchText;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
    virtual void OnOK();
    afx_msg void OnClose();
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnEnChangeEditParameterSelectionSearchText();
    afx_msg void OnPaint();
};
