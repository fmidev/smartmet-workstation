#pragma once
#include "afxwin.h"
#include "SmartMetDialogs_resource.h"

#include <string>

class SmartMetDocumentInterface;
// CFmiIgnoreStationsDlg dialog

class CFmiIgnoreStationsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFmiIgnoreStationsDlg)

public:
	CFmiIgnoreStationsDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiIgnoreStationsDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_IGNORE_STATIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	void DoWhenClosing(void);
	void UpdateStationList(void);
	void StoreSettingsToDoc(void);

	CCheckListBox itsIgnoreStationList;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonRemove();
	afx_msg void OnBnClickedButtonUpdate();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnClose();
private:
	BOOL fUseWithSymbolDraw;
	BOOL fUseWithContourDraw;
public:
	afx_msg void OnLbnSelchangeListStations();
    // Luokalla pit‰‰ olla Update -metodi, jotta sen luonti smartmetdocissa menee template funktion l‰pi.
    void Update() {}
};
