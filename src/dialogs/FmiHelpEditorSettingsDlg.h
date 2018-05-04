#pragma once

#include "SmartMetDialogs_resource.h"

class NFmiHelpEditorSystem;

// CFmiHelpEditorSettingsDlg dialog

class CFmiHelpEditorSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(CFmiHelpEditorSettingsDlg)

public:
	CFmiHelpEditorSettingsDlg(NFmiHelpEditorSystem &theHelpEditorSystem, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiHelpEditorSettingsDlg();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonHelpEditorModeBrowse();

// Dialog Data
	enum { IDD = IDD_DIALOG_HELP_EDITOR_MODE_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();

	
	DECLARE_MESSAGE_MAP()

private:
    void Browse(CString & thePathU_);

	NFmiHelpEditorSystem &itsHelpEditorSystem; // ei omista
	BOOL fUseSystem;
	BOOL fHelpModeOn;
    CString itsHelpDataPathStrU_;
    CString itsHelpDataBaseFileNameStrU_;
};
