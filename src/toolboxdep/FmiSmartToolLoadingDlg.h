#pragma once
#include "SmartMetToolboxDep_resource.h"
#include "afxwin.h"
#include <string>

class NFmiSmartToolInfo;
// CFmiSmartToolLoadingDlg dialog

const int kFmiCloseAfterDblClick = 1;

class CFmiSmartToolLoadingDlg : public CDialog
{
	DECLARE_DYNAMIC(CFmiSmartToolLoadingDlg)

public:
	CFmiSmartToolLoadingDlg(NFmiSmartToolInfo *theSmartToolInfo, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiSmartToolLoadingDlg();
	const std::string& SelectedScriptName(void) const {return itsSelectedScriptName;}

// Dialog Data
	enum { IDD = IDD_DIALOG_SMART_TOOL_LOADING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	void UpdateScriptNameList(void);
	bool CreateNewSmartToolDirectory(const std::string &thePath);

	NFmiSmartToolInfo *itsSmartToolInfo;
	std::string itsSelectedScriptName;
	UINT itsCloseAfterDblClick;
    static int itsLastLoadedMacroIndex; // Tämä on 'pysyvä' muisti, joka osoittaa viimeksi ladattun makron indeksiin. Kun dialogi avataan uudestaan, otetaan tämä käyttöön OnInitDialog -metodissa
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonRemove();
private:
	CListBox itsScriptNameList;
    static CRect itsStartingPlacement; // pidetään tässä tallessa koko ohjelman suorituksen ajan smartool:ien modaalisen lataus dialogin paikka ja koko
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnLbnDblclkListParamMacros();
	afx_msg void OnBnClickedButtonAddDirectory();
#ifdef _WIN64
	afx_msg void OnTimer(UINT_PTR nIDEvent);
#else
	afx_msg void OnTimer(UINT nIDEvent);
#endif
    afx_msg void OnSize(UINT nType, int cx, int cy);
    CRect CalcClientArea(void);
	void DoWhenClosing(void);
    virtual void OnCancel();
    virtual void OnOK();
    afx_msg void OnClose();
};
