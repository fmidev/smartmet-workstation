
// MainFrm.h : interface of the CMainFrame class
//

#pragma once

#include <string>

const int kFmiCheckClosingTimer = 1;

namespace tcp_tools
{
    struct multi_process_tcp_pool_options;
}


class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    void StartWorkerLoopThread(void);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CToolBar          m_wndToolBar;
	CStatusBar        m_wndStatusBar;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()

private:
    void worker_loop_function(const tcp_tools::multi_process_tcp_pool_options &mpp_options);

	UINT itsCheckClosingTimer;
    bool mToolmasterInUse;
    int mToolmasterContext;
public:
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnClose();
};


