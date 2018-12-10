#pragma once

// FmiSmartToolDlg.h : header file
//

#include "SmartMetToolboxDep_resource.h"
#include "afxwin.h"
#include "boost/shared_ptr.hpp"
#include "WndResizer.h"
#include "NFmiViewPosRegistryInfo.h"
#include "GUSIconEdit.h"
#include "NFmiInfoData.h"

/////////////////////////////////////////////////////////////////////////////
// CFmiSmartToolDlg dialog
class SmartMetDocumentInterface;
class NFmiSmartToolInfo;
class NFmiDrawParam;
class NFmiMacroParam;
#ifndef DISABLE_EXTREME_TOOLKITPRO
class CXTPSyntaxEditCtrl;
#endif // DISABLE_EXTREME_TOOLKITPRO
class NFmiMacroParamSystem;

class CFmiSmartToolDlg : public CDialog
{
private: // näillä talletetaan sijainti ja koko rekisteriin
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void){return s_ViewPosRegistryInfo;}

	DECLARE_DYNAMIC(CFmiSmartToolDlg);

    // Construction
public:
	CFmiSmartToolDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	~CFmiSmartToolDlg(void);
	void Update(void);
	void UpdateMacroParamDisplayList(bool fForceThreadUpdate);
	void SetMacroErrorText(const std::string &theErrorStr);
    static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) {return ViewPosRegistryInfo().WinRegistryKeyStr();}
	void SetDefaultValues(void);

// Dialog Data
	//{{AFX_DATA(CFmiSmartToolDlg)
	enum { IDD = IDD_DIALOG_SMART_TOOL };
    CString	itsMacroErrorTextU_;
	BOOL	fModifyOnlySelectedLocations;
	BOOL	fMakeDBCheckAtSend;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiSmartToolDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFmiSmartToolDlg)
	virtual BOOL OnInitDialog();
    afx_msg void OnButtonAction();
	afx_msg void OnButtonSmartToolLoad();
	afx_msg void OnButtonSmartToolLoadDbChecker();
	afx_msg void OnButtonSmartToolSave();
	afx_msg void OnButtonSmartToolSaveDbChecker();
	afx_msg void OnCheckMakeDbCheckAtSend();
    virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    //}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void DisableActionButtomIfInViewMode(void);
	void AddSelectedMacroParamToRow(int theRow);
	void RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, bool editedDataModified, const std::string &possibleEditedDrawParamPath);
	void UpdateMacroName(void);
	//std::string GetWantedMacroString(void);
	NFmiMacroParam* FindSelectedMacroParam(void);
	void ClearMapViewRow(int theRowIndex);
	bool CreateNewMacroParamDirectory(const std::string &thePath);
	void InitDialogTexts(void);
	bool EnableDlgItem(int theDlgId, bool fEnable);
    void DoResizerHooking(void);
    void UpdateLoadDirectoryString();
    void ResetSearchResource();
    void InitSpeedSearchControl();
    void DoTimedResetSearchResource();
    void DoSmartToolLoad(const std::string &theSmartToolName, bool fDoSpeedLoad);
    std::string GetSmarttoolFilePath();
    void WarnUserAboutNoEditingSmarttools();
    std::string GetSmarttoolFormulaText();
    bool LoadSmarttoolFormula(const std::string &theFilePath);
#ifndef DISABLE_EXTREME_TOOLKITPRO
    void InitializeSyntaxEditControl();
    std::string MakeSyntaxEditConfigFilePath();
    void AdjustSyntaxEditControlWindows();
    bool LoadSmarttoolToSyntaxEditControl(const std::string &theFilePath);
    void UpdateSyntaxEditControl();
    bool StoreSmarttoolFromSyntaxEditControl(const std::string &theFilePath);
    std::string GetMacroTextFromSyntaxEditor();
#endif // DISABLE_EXTREME_TOOLKITPRO
    std::string GetMacroParamFilePath(NFmiMacroParamSystem &theMacroParamSystem);
    NFmiInfoData::Type GetUsedMacroParamType();
    void DoFinalMacroParamWrite(NFmiMacroParamSystem& macroParamSystem, boost::shared_ptr<NFmiMacroParam> &macroParamPointer);

    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
	NFmiSmartToolInfo *itsSmartToolInfo;
	int itsSelectedMapViewDescTopIndex;
    CString itsMacroParamNameU_;
    CListBox itsMacroParamList;
    CString itsMacroNameU_;
    int itsMacroParamDataGridSizeX;
    int itsMacroParamDataGridSizeY;
    BOOL fCrossSectionMode;
    BOOL fQ3Macro;
    CWndResizer m_resizer;
    CGUSIconEdit itsSpeedSearchMacroControl;
    CMenu itsSpeedSearchMacroControlMenu;
    BOOL fSearchOptionCaseSensitive;
    BOOL fSearchOptionMatchAnywhere;
    CString itsUsedMacroPathU_;

#ifndef DISABLE_EXTREME_TOOLKITPRO
    std::unique_ptr<CXTPSyntaxEditCtrl>	itsSyntaxEditControl;
    HACCEL itsSyntaxEditControlAcceleratorTable;
#endif // DISABLE_EXTREME_TOOLKITPRO

public:
	afx_msg void OnBnClickedCheckModifyOnlySelectedLocations();
	afx_msg void OnBnClickedButtonMacroParamSave();
    afx_msg void OnBnClickedButtonMacroParamRemove();
	afx_msg void OnBnClickedButtonMacroParamProperties();
	afx_msg void OnBnClickedButtonAddToRow1();
	afx_msg void OnBnClickedButtonAddToRow2();
	afx_msg void OnBnClickedButtonAddToRow3();
	afx_msg void OnBnClickedButtonAddToRow4();
	afx_msg void OnBnClickedButtonAddToRow5();
    afx_msg void OnLbnSelchangeListParamMacros();
	afx_msg void OnLbnDblclkListParamMacros();
	afx_msg void OnBnClickedButtonMacroParamLatestErrorText();
	afx_msg void OnBnClickedButtonSmartToolRemove();
	afx_msg void OnBnClickedButtonRemoveAllFromRow1();
	afx_msg void OnBnClickedButtonRemoveAllFromRow2();
	afx_msg void OnBnClickedButtonRemoveAllFromRow3();
	afx_msg void OnBnClickedButtonRemoveAllFromRow4();
	afx_msg void OnBnClickedButtonRemoveAllFromRow5();
	afx_msg void OnBnClickedButtonMacroParamRefreshList();
	afx_msg void OnBnClickedButtonMacroParamDataGridSizeUse();
	afx_msg void OnBnClickedCheckMacroParamCrosssectionMode();
	afx_msg void OnBnClickedCheckQ3Macro();
	afx_msg void OnBnClickedRadioViewmacroSelectedMap1();
	afx_msg void OnBnClickedRadioViewmacroSelectedMap2();
	afx_msg void OnBnClickedRadioViewmacroSelectedMap3();
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    afx_msg LRESULT OnGUSIconEditLeftIconClicked(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGUSIconEditRightIconClicked(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGUSIconEditSearchListClicked(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSearchOptionCaseSesensitive();
    afx_msg void OnSearchOptionMatchAnywhere();
    afx_msg void OnEnChangeEditSpeedSearchViewMacro();
    afx_msg void OnSize(UINT nType, int cx, int cy);
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
