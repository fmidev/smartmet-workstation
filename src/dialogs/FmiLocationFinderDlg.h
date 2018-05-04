#pragma once
#include "SmartMetDialogs_resource.h"
#include <string>
//#include "afxwin.h"

class SmartMetDocumentInterface;

// CFmiLocationFinderDlg dialog

class CFmiLocationFinderDlg : public CDialog
{
	DECLARE_DYNAMIC(CFmiLocationFinderDlg)

public:
	CFmiLocationFinderDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiLocationFinderDlg();
	BOOL Create(void); // modaalittomaa varten

// Dialog Data
	enum { IDD = IDD_DIALOG_LOCATION_FINDER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual void OnCancel();
	virtual void OnOK();
	void DoWhenClosing(void);
public:
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
private:
	void SetErrorStr(std::string &theErrorStr);
	CComboBox itsLocationFinderComboBox;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;

	// ComboBox:in ListBoxin sulku ei toimi kuten haluan (kun klikataan esim. kartalle ja nimi lista sulkeutuu),
	// tällöin tulee jostain syystä valituksi listan 1. nimi edit kenttään. Tämä viritys estää tämän tapahtumisen.
	bool fReplaceStoredWordOnClose;
    CString itsfReplaceStoredWordOnCloseStrU_;
	DWORD itsfReplaceStoredWordOnCloseSelection;

public:
	afx_msg void OnCbnSelchangeComboLocationFinder();
	afx_msg void OnCbnEditchangeComboLocationFinder();
	afx_msg void OnCbnCloseupComboLocationFinder();
	afx_msg void OnCbnSelendcancelComboLocationFinder();
};
