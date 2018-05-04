#pragma once

#include "SmartMetToolboxDep_resource.h"
#include "NFmiQueryDataUtil.h"
#include "TextProgressCtrl.h"

// CFmiOperationProgressDlg dialog

class CFmiOperationProgressDlg : public CDialog, public NFmiOperationProgress
{
	DECLARE_DYNAMIC(CFmiOperationProgressDlg)

public:
	CFmiOperationProgressDlg(NFmiStopFunctor &theStopper, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiOperationProgressDlg();

	// overrides from NFmiOperationProgress
	void StepIt(void);
	void SetRange(int low, int high, int stepCount);
	void AddRange(int value);
	bool DoPostMessage(unsigned int message, unsigned int wParam = 0, long lParam = 0);
	bool WaitUntilInitialized(void);

// Dialog Data
	enum { IDD = IDD_DIALOG_CANCEL_OPERATION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
private:
//	CProgressCtrl itsProgressCtrl;
	CTextProgressCtrl itsProgressCtrl;
    CString itsOperationStrU_;
	NFmiStopFunctor &itsStopper;
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};
