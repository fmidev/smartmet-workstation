#pragma once

#include "SmartMetDialogs_resource.h"

// CFmiLanguageSelectionDlg dialog

class CFmiLanguageSelectionDlg : public CDialog
{
	DECLARE_DYNAMIC(CFmiLanguageSelectionDlg)

public:
	CFmiLanguageSelectionDlg(int theLanguageSelection, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiLanguageSelectionDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_LANGUAGE_SELECTION };

	int LanguageSelection(void) const {return itsLanguageSelection;}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void InitDialogTexts(void);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
private:
	int itsLanguageSelection; // radio buttonilla valittu kieli (0:sta alkaen)
};
