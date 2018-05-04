#pragma once

// FmiMaskOperationDlg.h : header file
//
// 1) Dialogin käytöstä:
//
// Tämä dialogi on suunniteltu meteorologin editorin MetEdit tarpeita varten.
// Dialogista käyttäjä voi valita annetun parametrin arvolle rajoituksia.
// Tehdyt valinnat voidaan kysyä dialogilta, samoin maskausoperaation
// nimi FmiMaskOperation. Koska dialogi luodaan tyhjällä konstruktorilla,
// täytyy maskausparametri antaa ulkopuolelta metodilla 
// ParamMask(NFmiParamMask* theParamMask).
//
// 2) Dialogin rakenteesta:
//
// Dialogissa on useita Static-tekstikenttiä (CString), joihin kirjoitetaan
// käyttäjän tekemiä valintoja. Ristiriitaisista raja-arvoista huomautetaan
// punaisessa tekstillä CStatic-ikkunassa itsRemarkCtrl - tämä ikkuna toimii
// värillisen tekstin piirtopintana. Teksti olisi tietysti voitu kirjoittaa
// metodilla CPaintDC::DrawText() suoraan dialoigin piirtopinnalle dc, mutta
// nyt tekstin paikkaa on helppo siirrellä. 
//
// Metodit on kommentoitu cpp-tiedostoon.
//
//
/////////////////////////////////////////////////////////////////////////////
// CFmiMaskOperationDlg dialog


#include "SmartMetDialogs_resource.h"
#include "NFmiGlobals.h"

class NFmiAreaMask;
class NFmiValueString;


class CFmiMaskOperationDlg : public CDialog
{
// Construction and destruction
public:
	CFmiMaskOperationDlg(CWnd* pParent = NULL);   // standard constructor
	
	virtual  ~CFmiMaskOperationDlg(void);

// Methods
	double FirstLimit(void);
	double SecondLimit(void);
	CString FirstLimit(int dummy);
	CString SecondLimit(int dummy);

	FmiMaskOperation MaskOperation(void);        // Get-metodi
	void ParamMask(NFmiAreaMask* theMask); // Set-metodi

// Dialog Data
	//{{AFX_DATA(CFmiMaskOperationDlg)
	enum { IDD = IDD_DLG_MASK_OPER };
	CStatic	itsRemarkCtrl;
	CEdit	itsEditCtrlOfSecondLimit;
	CEdit	itsEditCtrlOfFirstLimit;
	int		itsRadioButtons;
    CString	itsFirstLimitU_;
    CString	itsSecondLimitU_;
    CString	itsParameterNameU_;
    CString	itsLimitsHeaderU_;
	//}}AFX_DATA

protected:

// Attributes

	NFmiAreaMask* itsMask;
    CString	itsRemarkTextU_;
    CString itsAbbreviationU_;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiMaskOperationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void InitDialogTexts(void);

	void UpdateRampBitmaps(void);
	void UpdateDialogAfterMaskChanged(void);
	virtual void OnOK(); // Overrides the virtual method CDialog::OnOK()
	void UpdateStateOfEditCtrl(BOOL updateFromDlg = TRUE);
	void SetRemarkText(BOOL theFirstNumber= TRUE);
	void UpdateRemarkText(BOOL theFirstNumber = TRUE);
	void SetHeaderOfLimits();
	void UpdateHeaderOfLimits();
	void Limit(double firstLimit, double secondLimit);

	// Generated message map functions
	//{{AFX_MSG(CFmiMaskOperationDlg)
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio3();
	afx_msg void OnRadio4();
	afx_msg void OnRadio5();
	afx_msg void OnRadio6();
	afx_msg void OnRadio7();
	afx_msg void OnRadio8();
	afx_msg void OnRadio9();
	afx_msg void OnRadio10();
	afx_msg void OnRadio11();
	afx_msg void OnRadio12();
	afx_msg void OnRadio13();
	afx_msg void OnRadio14();
	afx_msg void OnRadio15();
	afx_msg void OnRadio17();
	afx_msg void OnRadio18();
	afx_msg void OnRadio19();

	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditRaja1();
	afx_msg void OnChangeEditRaja2();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

