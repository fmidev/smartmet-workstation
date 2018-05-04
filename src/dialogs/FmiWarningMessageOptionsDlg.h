#pragma once

#include "SmartMetDialogs_resource.h"
#include "afxwin.h"
#include "WndResizer.h"
#include "NFmiViewPosRegistryInfo.h"

namespace HakeMessage
{
    class Main;
}
class NFmiApplicationWinRegistry;

// CFmiWarningMessageOptionsDlg dialog

class CFmiWarningMessageOptionsDlg : public CDialog
{
    // Koodia, mitä tarvitaan näytön paikan muistamiseen rekisterissä
private:
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void) { return s_ViewPosRegistryInfo; }
    static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) { return ViewPosRegistryInfo().WinRegistryKeyStr(); }
    void SetDefaultValues(void);
    // Koodia, mitä tarvitaan näytön paikan muistamiseen rekisterissä
    
    DECLARE_DYNAMIC(CFmiWarningMessageOptionsDlg)

public:
	CFmiWarningMessageOptionsDlg(HakeMessage::Main *theWarningCenterSystem, NFmiApplicationWinRegistry &theApplicationWinRegistry, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiWarningMessageOptionsDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_WARNING_CENTER_SYSTEM_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnOK();

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
private:
    void InitDialogTexts();
    void DoResizerHooking();
    void DoWhenClosing();
    void StoreMessageTypeInfoToWarningCenterSystem();
    void LoadMessageTypeInfoFromWarningCenterSystem();

    NFmiApplicationWinRegistry &itsApplicationWinRegistry; // Tätä tarvitaan että näytön paikka saadaan talteen Windows rekisteriin
    HakeMessage::Main *itsWarningCenterSystem;
    CCheckListBox itsMessageTypeList;
    CWndResizer m_resizer;
public:
    afx_msg void OnBnClickedButtonAdd();
    afx_msg void OnBnClickedButtonRemove();
    virtual void OnCancel();
    afx_msg void OnClose();
    afx_msg void OnBnClickedButtonEdit();
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    afx_msg void OnBnClickedCheckWarningOptionsCheckAll();
private:
    CButton itsCheckAllControl;
};
