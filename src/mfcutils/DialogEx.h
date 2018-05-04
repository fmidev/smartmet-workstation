#pragma once

/********************************************************************************************
* MOD-NAME      : DialogEx.h
* LONG-NAME     : 
*
* AUTHOR        : Martin Ziacek (Martin.Ziacek@pobox.sk)
* DEPARTMENT    : 
* TELEPHONE     : 
* CREATION-DATE : 22/02/2000 09:15:33
* SP-NO         : 
* FUNCTION      : 
* 
*********************************************************************************************/

#include "ControlsEx.h"

/////////////////////////////////////////////////////////////////////////////
// CDialogEx2 Marko: piti muuttaa nimi CDialogEx -> CDialogEx2, koska MFC Feature Packissa on saman niminen luokka

class CDialogEx2 : public CDialog
{
public:
	CDialogEx2();
	CDialogEx2(UINT nIDTemplate,CWnd* pParentWnd = NULL);

	CStatusBar* GetStatusBar() { return &m_CSBCtrlStatus; }				
	CToolBar*   GetToolBar()   { return &m_CSToolBar; }

// Implementation
protected:

	CDlgStatusBar	m_CSBCtrlStatus;
	CDlgToolBar		m_CSToolBar;
	UINT			m_nIDTracking;
	UINT			m_nIDLastMessage;
	BOOL			m_bResetPrompt;

	BOOL			m_bToolTips;
	BOOL			m_bStatusBar;
	BOOL			m_bToolBar;

	BOOL			m_bKillItself;

	BOOL			m_bModal;
	UINT			m_Timer;

	// 256 värisen ja vaihtuva kokoisen tolbarin teko vaatii näiden käyttöön oton
	HBITMAP itsToolBarBitmapHandle;
	CBitmap itsToolBarBitmap;
	CImageList itsToolBarImagelist;

	BOOL InitDialogEx(BOOL bToolTips = FALSE, BOOL bStatusBar = FALSE, UINT *pIndicators = NULL, UINT nIndicators = 0, UINT uiToolBar = 0, DWORD dwToolBarStyle = TBSTYLE_FLAT | WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, UINT uiToolBar24_bit = 0);

	// Marko: jouduin lisäämään tämän että saan oikean status sanoman 
	// näkyviin statusbariin lapsi luokassa
	virtual CString GetFinalStatusString(UINT nID, const CString &strPromtpText);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogEx2)
	public:
	virtual void OnCancel();
	virtual void OnOK();
#ifdef _WIN64
	virtual INT_PTR DoModal();
#else
	virtual int DoModal();
#endif
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CDialogEx2)
	afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);
	afx_msg LRESULT OnPopMessageString(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam = 0L);
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnUpdateKeyIndicator(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusBarMenu(CCmdUI* pCmdUI);
	afx_msg void OnStatusBarCheck();
	afx_msg void OnToolBarCheck();
	afx_msg void OnUpdateToolBarMenu(CCmdUI* pCmdUI);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg LRESULT OnKickIdle(WPARAM, LPARAM lCount);
#ifdef _WIN64
	afx_msg void OnTimer(UINT_PTR nIDEvent);
#else
	afx_msg void OnTimer(UINT nIDEvent);
#endif
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};

