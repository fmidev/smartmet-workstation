#pragma once
// TableWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTableWnd window

#define TW_SETPARAMS		(WM_APP+100)


class CTableWnd : public CWnd
{
// Construction
public:
	CTableWnd(int maxx,int maxy,int nX,int nY,UINT nId=TW_SETPARAMS);
	BOOL Create(CWnd *pParent,int x,int y);
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTableWnd)
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTableWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTableWnd)
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL RegisterClass();
	void CloseWindow();

	CSize CalcSize(int nx,int ny); 
	void DrawText(int x,int y);
	void MoveStatic();
	CStatic m_WndStatic;
	CWnd *m_pParent;

	BOOL m_bInitOver;
	CFont m_Font;
	int m_nMaxX,m_nMaxY;
	int m_nNumX,m_nNumY;
	int m_nSelX,m_nSelY;
	BOOL m_bCaptured;
	CRect m_Rect;
	UINT n_nMessageId;
};

/////////////////////////////////////////////////////////////////////////////

