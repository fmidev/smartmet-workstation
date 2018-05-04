#pragma once

#include "SmartMetToolboxDep_resource.h"
#include "WndResizer.h"
#include "TabCtrlSSL.h"
#include "DynamicSplitterHelper.h"
#include "GridCtrl.h"

class SmartMetDocumentInterface;

// CFmiMacroParamTabDlg dialog

class CFmiMacroParamTabDlg : public CTabPageSSL
{
	DECLARE_DYNAMIC(CFmiMacroParamTabDlg)

public:
    CFmiMacroParamTabDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiMacroParamTabDlg();
    virtual BOOL Create(CWnd* pParentWnd = NULL); // modaalitonta dialogi luontia varten
    virtual BOOL Create(UINT nIDTemplate, CWnd* pParentWnd = NULL); // Tab dialogia varten
    void Update();
    void DoWhenClosing(void);

// Dialog Data
	enum { IDD = IDD_DIALOG_MACROPARAM_TAB };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
    void DoResizerHooking(void);

    CDynamicSplitterHelper itsSplitterHelper;
    CWndResizer m_resizer;
    CGridCtrl itsGridCtrl;
    SmartMetDocumentInterface* itsSmartMetDocumentInterface;
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg LRESULT OnSplitterControlMove(WPARAM wParam, LPARAM lParam);
    afx_msg void OnBnClickedCheckHorizontalSplitScreen();
};
