#pragma once

#include "SmartMetDialogs_resource.h"
#include "PPToolTip.h"
#include <string>
#include "afxwin.h"

class SmartMetDocumentInterface;

// CFmiMacroPathSettings dialog

class CFmiMacroPathSettings : public CDialog
{
	DECLARE_DYNAMIC(CFmiMacroPathSettings)

public:
	CFmiMacroPathSettings(SmartMetDocumentInterface *smartMetDocumentInterface,
						const std::string &theSmartToolPathStr,
						const std::string &theViewMacroPathStr,
						const std::string &theMacroParamPathStr,
						const std::string &theDrawParamPathStr,
						const std::string &theLoggerPathStr,
						CWnd* pParent = NULL);
	virtual ~CFmiMacroPathSettings();

    CString& SmartToolPathStr(void){ return itsSmartToolPathStrU_; }
    CString& ViewMacroPathStr(void){ return itsViewMacroPathStrU_; }
    CString& MacroParamPathStr(void){ return itsMacroParamPathStrU_; }
    CString& DrawParamPathStr(void){ return itsDrawParamPathStrU_; }
    CString& LoggerPathStr(void){ return itsLoggerPathStrU_; }
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_MACRO_PATH_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonSmartToolsBrowse();
	afx_msg void OnBnClickedButtonViewMacroBrowse();
	afx_msg void OnBnClickedButtonMacroParamBrowse();
	afx_msg void OnBnClickedButtonDrawParamBrowse();
	afx_msg void OnBnClickedButtonLoggerBrowse();
	afx_msg void OnBnClickedButtonChangeDriveLetterNow();
private:
    void Browse(CString & thePathU_);
	void InitDialogTexts(void);
	void InitDriveLetterCombo(void);

    CString itsSmartToolPathStrU_;
    CString itsViewMacroPathStrU_;
    CString itsMacroParamPathStrU_;
    CString itsDrawParamPathStrU_;
    CString itsLoggerPathStrU_;
	CComboBox itsDriveLetterCombo;

    SmartMetDocumentInterface *itsSmartMetDocumentInterface; // t‰t‰ k‰ytet‰‰n vain change drive letter toiminnossa
	CPPToolTip m_tooltip;
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
