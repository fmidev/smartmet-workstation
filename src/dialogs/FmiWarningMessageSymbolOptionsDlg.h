#pragma once
//#include "afxwin.h"
#include "SmartMetDialogs_resource.h"
#include "afxwin.h"

namespace HakeLegacySupport
{
    class MessageTypeInfo;
}
class NFmiColor;

// CFmiWarningMessageSymbolOptionsDlg dialog

class CFmiWarningMessageSymbolOptionsDlg : public CDialog
{
	DECLARE_DYNAMIC(CFmiWarningMessageSymbolOptionsDlg)

public:
	CFmiWarningMessageSymbolOptionsDlg(HakeLegacySupport::MessageTypeInfo &theMessageTypeInfo, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiWarningMessageSymbolOptionsDlg() = default;

// Dialog Data
	enum { IDD = IDD_DIALOG_WARNING_MESSAGE_SYMBOL_OPTION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
private:
    // Tähän otetaan originaali otus referenssinä. Jos painetaan lopuksi Ok -nappia, kopsataan
    // tähän tehdyt muutokset.
    HakeLegacySupport::MessageTypeInfo &itsOriginalMessageTypeInfoReference;
    BOOL fMessageTypeInfoShow;
    int itsMessageTypeInfoCategoryId;
    CString itsMessageTypeInfoCategoryDescription;
    CComboBox itsMessageTypeSelector;

    void InitializeFromMessageTypeInfo(HakeLegacySupport::MessageTypeInfo &theMessageTypeInfo);
    void StoreMessageTypeInfoFromControls(HakeLegacySupport::MessageTypeInfo &theMessageTypeInfo);
};
