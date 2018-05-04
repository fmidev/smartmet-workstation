#pragma once

#include "SmartMetMfcUtils_resource.h"

// CfmiGetDirectoryNameDlg dialog

class CfmiGetDirectoryNameDlg : public CDialog
{
	DECLARE_DYNAMIC(CfmiGetDirectoryNameDlg)

public:
	CfmiGetDirectoryNameDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CfmiGetDirectoryNameDlg();
    CString& DirectoryName(void) { return itsDirectoryNameU_; }

// Dialog Data
	enum { IDD = IDD_DIALOG_GET_DIRECTORY_NAME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
private:
    CString itsDirectoryNameU_;
};
