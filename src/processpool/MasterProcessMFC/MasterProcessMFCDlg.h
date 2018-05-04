
// MasterProcessMFCDlg.h : header file
//

#pragma once

#include "ProgramOptions.h"

const int kFmiLoggingTimer = 1;
const int kFmiCheckClosingTimer = 2;

// CMasterProcessMFCDlg dialog
class CMasterProcessMFCDlg : public CDialogEx
{
// Construction
public:
	CMasterProcessMFCDlg(const MasterProcessMFCOptions &options, CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MASTERPROCESSMFC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
private:
    void FillClientAreaWithLogView(void);
    void DoWhenClosing(void);
    void StartMasterLoopThread(void);
    void init_listview_logging();
    void log_on_list_view();

    CListCtrl mLogListView;
    bool mDoOnClose;
	UINT itsLoggingTimer;
	UINT itsCheckClosingTimer;
    MasterProcessMFCOptions mOptions;
    bool m_visible;
public:
    afx_msg void OnClose();
    virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
};
