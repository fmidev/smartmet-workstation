#pragma once

#include "SmartMetToolboxDep_resource.h"

class NFmiTempBalloonTrajectorSettings;

// CFmiTempBalloonSettingsDlg dialog

class CFmiTempBalloonSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(CFmiTempBalloonSettingsDlg)

public:
	CFmiTempBalloonSettingsDlg(NFmiTempBalloonTrajectorSettings &theSettings, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiTempBalloonSettingsDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_TEMP_BALLOON_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
private:
	void InitDialogTexts(void);

	NFmiTempBalloonTrajectorSettings &itsSettings;
	double itsRisingSpeed;
	double itsFallSpeed;
	double itsTopHeightInKM;
	double itsFloatingTimeInMinutes;
};
