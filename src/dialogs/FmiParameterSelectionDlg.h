#pragma once

#include <boost/shared_ptr.hpp>
#include "SmartMetDialogs_resource.h"
#include "GridCtrl.h"
#include "TreeColumn.h"
#include <vector>
#include <functional>
#include "NFmiViewPosRegistryInfo.h"
#include "PPTooltip.h"
#include "NFmiProducerSystem.h"
#include "WndResizer.h"

class SmartMetDocumentInterface;
class NFmiFastQueryInfo;
class NFmiHelpDataInfo;
class SingleRowItem;

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
        kItemId,
        kInterpolationType
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
    std::string ComposeToolTipText(const CPoint &point);
    CPPToolTip m_tooltip;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
    std::string TooltipForCategoryType(const AddParams::SingleRowItem &singleRowItem, const std::vector<AddParams::SingleRowItem> &singleRowItemVector, int rowNumber);
    std::string TooltipForCategoryType();
    std::string TooltipForMacroParamCategoryType(const AddParams::SingleRowItem& singleRowItem, const std::vector<AddParams::SingleRowItem>& singleRowItemVector, int rowNumber);
    std::string TooltipForWmsDataCategoryType(const AddParams::SingleRowItem &singleRowItem, const std::vector<AddParams::SingleRowItem> &singleRowItemVector, int rowNumber);
    std::string TooltipForParameterType(const AddParams::SingleRowItem &rowItem);
    std::string TooltipForDataType(const AddParams::SingleRowItem &singleRowItem);
    std::string TooltipForProducerType(const AddParams::SingleRowItem &singleRowItem, const std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &infoVector, const NFmiProducerInfo &producerInfo);
    std::string DataTypeString(NFmiInfoData::Type dataType);
public:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
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
	int fixedRowCount = 1;
	int fixedColumnCount = 1;

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
    void InitHeaders();
    void UpdateGridControlValues(bool collapseAll = false);
    void UpdateGridControlValuesInNormalMode(bool fFirstTime);
    void UpdateGridControlValuesWhenSearchActive(void);
    void UpdateGridControlValuesWhenSearchRemoved(void);
    void ExpandAllNodes();
    void UpdateRows(int fixedRowCount, int fixedColumnCount, bool updateOnly);
    void SetGridRow(int row, const AddParams::SingleRowItem &theRowItem, int theFixedColumnCount);
    void InitDialogTexts(void);
    void CollapseAllTreeNodes();
    void MakeTreeNodeCollapseSettings();
    void SetTreeNodeInformationBackToDialogRowData();
    void HandleGridCtrlsLButtonDblClk();
    void HandleRowItemSelection(const AddParams::SingleRowItem &rowItem);
    void FitNameColumnOnVisibleArea(int gridCtrlWidth);
    std::string MakeActiveViewRowText();
	void UpdateGridControlIfNeeded();
	std::string MakeTitleText();
    bool UpdateSearchIfNeeded();
    void CollapseAllButCategories();
    void UpdateAfterSearchText();
    void DoResizerHooking();
    void DoTimeSerialSideParametersCheckboxAdjustments();

    NFmiParameterSelectionGridCtrl itsGridCtrl;
    CTreeColumn itsTreeColumn;   // provides tree column support
    std::vector<ParameterSelectionHeaderParInfo> itsHeaders;
    bool fDialogInitialized; // t‰m‰ on OnSize -s‰‰tˆj‰ varten, 1. kerralla ei saa s‰‰t‰‰ ikkunoita, koska niit‰ ei ole viel‰ olemassa.

    SmartMetDocumentInterface *itsSmartMetDocumentInterface; // ei omista, ei tuhoa
    AddParams::ParameterSelectionSystem *itsParameterSelectionSystem;
    unsigned int itsLastActivatedDesktopIndex; // Mik‰ oli viimeksi DesktopIndex, kun otsikon teksti‰ tehtiin
    int itsLastActivatedRowIndex; // Mik‰ oli viimeksi RowIndex, kun otsikon teksti‰ tehtiin
    CString itsSearchText;
    std::string itsPreviousSearchText;
    BOOL fTimeSerialSideParameterCase;
    CWndResizer m_resizer;

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
	void SetIndexes(unsigned int theDescTopIndex);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
};
