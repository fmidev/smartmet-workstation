#pragma once

#include "SmartMetToolboxDep_resource.h"
#include "NFmiQueryDataUtil.h"
#include "TextProgressCtrl.h"
#include "afxdialogex.h"

// CFmiOperationProgressAndCancellationDlg dialog

class CFmiOperationProgressAndCancellationDlg : public CDialogEx, public NFmiOperationProgress
{
	DECLARE_DYNAMIC(CFmiOperationProgressAndCancellationDlg)

public:
	CFmiOperationProgressAndCancellationDlg(const std::string& operationText, bool operationTextIsWarning, NFmiStopFunctor& theStopper, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CFmiOperationProgressAndCancellationDlg();

	// overrides from NFmiOperationProgress
	void StepIt(void) override;
	void SetRange(int low, int high, int stepCount) override;
	void AddRange(int value) override;
	bool DoPostMessage(unsigned int message, unsigned int wParam = 0, long lParam = 0) override;
	bool WaitUntilInitialized(void) override;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_OPERATION_PROGRESS_AND_CANCEL };
#endif

private:
	CString itsOperationRelatedStrU_;
	CTextProgressCtrl itsOperationProgressBar;
	NFmiStopFunctor& itsStopper;
	// Piirretäänkö operaatioon liittyvä teksti punaisella vai ei.
	bool fOperationTextIsWarning;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
	virtual void OnCancel();
public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
