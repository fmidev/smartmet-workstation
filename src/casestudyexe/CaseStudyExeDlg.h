
#pragma once

#include "NFmiCaseStudySystem.h"
#include "WndResizer.h"

const int kFmiStartOperationTimer = 1;

// CCaseStudyExeDlg dialog
class CCaseStudyExeDlg : public CDialog
{
// Construction
public:
	CCaseStudyExeDlg(const std::string &theSimpleCommandLineStr, CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CASESTUDYEXE_DIALOG };

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

private:
	void DoCaseDataOperation(void);
	CWnd* GetCopyDialogPositionWindow(void);
	void BringDialogUpFront(const std::string &theTitleStr, const std::string &theMessageStr);
    void DoResizerHooking(void);

	std::string itsSimpleCommandLineStr;
	std::string itsMetaFileName;
	std::string itsZipExe;
	NFmiCaseStudySystem itsCaseStudySystem;
	UINT itsStartOperationTimer; // k‰ynnistet‰‰n OnInitDialog:in lopuksi timer, joka laukaisee halutun toiminnan initialisoinnin lopuksi pienell‰ viiveell‰ (ett‰ dialogi on varmasti alustettu)
    CWndResizer m_resizer;
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
