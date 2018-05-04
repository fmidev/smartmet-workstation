#pragma once

#include "SmartMetDialogs_resource.h"
#include "afxwin.h"

// CFmiInputValueDlg dialog

class CFmiInputValueDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFmiInputValueDlg)

public:
    CFmiInputValueDlg(const CString &theHelpStrU_, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiInputValueDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_VALUE_IMPUT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
    CString itsValueU_;
private:
	CEdit itsValueEditor;
    CString itsHelpTextU_;
};
