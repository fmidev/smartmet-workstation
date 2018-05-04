#pragma once

#include "SmartMetToolboxDep_resource.h"

// CFmiTempCodeInsertDlg dialog
// Dialogi, johon sy�tet��n raaka TEMP koodeja, jotka
// tulkitaan ja muutetaan querydataksi. N�it�
// tulkittuja luotauksia voidaan katsoa sitten editorilla.

class SmartMetDocumentInterface;

class CFmiTempCodeInsertDlg : public CDialog
{
	DECLARE_DYNAMIC(CFmiTempCodeInsertDlg)

public:
	CFmiTempCodeInsertDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiTempCodeInsertDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_TEMP_STRING_INPUT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CRect CalcRichEditArea(void);
	CRect CalcCheckReportArea(void);
	void InitDialogTexts(void);
	
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonCheckClearTemps();
	afx_msg void OnBnClickedButtonCheckTempTexts();

    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
    CString itsTempCodeInputStrU_;
    CString itsTempCheckRaportStrU_;
};
