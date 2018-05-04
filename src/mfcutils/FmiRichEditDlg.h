#pragma once

#include <string>
#include "SmartMetMfcUtils_resource.h"

// CFmiRichEditDlg dialog

class CFmiRichEditDlg : public CDialog
{
	DECLARE_DYNAMIC(CFmiRichEditDlg)

public:
	CFmiRichEditDlg(const std::string theCaption, const std::string &theRichEditStr, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiRichEditDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_WITH_RICH_EDIT };
	CRichEditCtrl itsRichEditCtrl;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	void SetRichEditToClientArea(void);
	void SetRichEditFont(void);

	std::string itsCaption;
	std::string itsRichEditStr;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
