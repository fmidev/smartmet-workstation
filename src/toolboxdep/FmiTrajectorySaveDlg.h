#pragma once

#include "SmartMetToolboxDep_resource.h"

class SmartMetDocumentInterface;

// CFmiTrajectorySaveDlg dialog

class CFmiTrajectorySaveDlg : public CDialog
{
	DECLARE_DYNAMIC(CFmiTrajectorySaveDlg)

public:
	CFmiTrajectorySaveDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiTrajectorySaveDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SAVE_TRAJECTORY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
private:
    CString itsLegendTextU_;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface; // ei omista ei tuhoa
public:
	afx_msg void OnBnClickedTrajectorySaveAs();
};
