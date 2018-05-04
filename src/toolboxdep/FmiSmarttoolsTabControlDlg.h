#pragma once

#include "SmartMetToolboxDep_resource.h"
#include "WndResizer.h"
#include "FmiMacroParamTabDlg.h"
#include "FmiSmarttoolTabDlg.h"

class SmartMetDocumentInterface;

// CFmiSmarttoolsTabControlDlg dialog

class CFmiSmarttoolsTabControlDlg : public CDialog
{
	DECLARE_DYNAMIC(CFmiSmarttoolsTabControlDlg)

public:
    CFmiSmarttoolsTabControlDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiSmarttoolsTabControlDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SMARTTOOLS_TAB_CONTROL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
    void DoResizerHooking(void);
    void DoWhenClosing(bool calledFromDestructor);

    CFmiSmarttoolTabDlg itsSmarttoolsTab;
    CFmiMacroParamTabDlg itsMacroParamTab;
    HICON itsIcon;
    CTabCtrlSSL itsTabControl;
    CWndResizer m_resizer;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
public:
    virtual BOOL OnInitDialog();
    virtual BOOL Create(CWnd* pParentWnd = NULL); // modaalitonta dialogi luontia varten
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    void Update();
    virtual void OnCancel();
    virtual void OnOK();
    afx_msg void OnClose();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
};
