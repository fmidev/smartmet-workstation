#pragma once

// SaveToDataBaseDlg.h : header file
//
#include "SmartMetDialogs_resource.h"
#include "NFmiString.h"
#include "afxwin.h"

class NFmiDataLoadingInfo;
class SmartMetDocumentInterface;
/////////////////////////////////////////////////////////////////////////////
// CSaveToDataBaseDlg dialog

class CSaveToDataBaseDlg : public CDialog
{
// Construction
public:
	CSaveToDataBaseDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor

//	NFmiString HourStr(void);
    const CString& ForecasterIDStr(void) const { return itsForecasterIDStrU_; }
    int DataCheckMethod(void) const { return itsDataCheckMethod; }
// Dialog Data
	//{{AFX_DATA(CSaveToDataBaseDlg)
	enum { IDD = IDD_DIALOG_SAVE_TO_DATABASE };
    CString	itsTimeStringU_;
    CString	itsNameStringU_;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSaveToDataBaseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSaveToDataBaseDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	NFmiString TimeString(void);
	NFmiString NameString(void);
	CString StartTimeWarningString(void);
	void InitDialogTexts(void);
    void InitUserNameControls();
    bool IsUserNameListUsed() const;
    void CheckUserNameSelection();
    bool IsProperUserNameSelectedFromList() const;
    void StoreSelectedUserName();

    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
	NFmiDataLoadingInfo* itsDataLoadingInfo;

	// L‰hett‰ess‰‰n dataa tietokantaan, ennustaja laitttaa t‰h‰n puumerkkins‰
    CString itsForecasterIDStrU_;
    int itsDataCheckMethod; // 1=datavalidation, 2=as settings say
    CString itsStartTimeWarningStringU_;
    bool fStartTimeWarningOn;
    CComboBox itsUserNameComboBox;
    CString itsUserNameNotSelecterErrorStrU_;
    CButton itsAcceptButton; // Enabloidaan/disabloidaan t‰m‰n avulla Hyv‰ksy nappulaa
    BOOL fAddHelperId;
    static int itsSelectedUserIndex; // SmartMetin ajon aikana dialogi muistaa ett‰ mik‰ k‰ytt‰j‰ on valittuna (sen indeksi comboboxissa)
public:
	afx_msg void OnBnClickedOk();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnCbnSelchangeComboUserNames();
    afx_msg void OnBnClickedCheckAddHelperId();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

