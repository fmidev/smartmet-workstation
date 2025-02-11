#pragma once

#include "FmiBetaProductDialog.h"
#include "FmiBetaProductAutomationDialog.h"
#include "NFmiViewPosRegistryInfo.h"

class SmartMetDocumentInterface;

// CFmiBetaProductTabControlDialog dialog

class CFmiBetaProductTabControlDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CFmiBetaProductTabControlDialog)

private: // näillä talletetaan sijainti ja koko rekisteriin
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void){ return s_ViewPosRegistryInfo; }

public:
    CFmiBetaProductTabControlDialog(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiBetaProductTabControlDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_BETA_PRODUCT_TAB_CONTROL };

    // näillä talletetaan sijainti ja koko rekisteriin
    void SetDefaultValues(void);
    void DoWhenClosing(bool calledFromDestructor);
    void StoreControlValuesToDocument();
    static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) { return ViewPosRegistryInfo().WinRegistryKeyStr(); }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:

    CFmiBetaProductDialog itsBetaProductDialogTab;
    CFmiBetaProductAutomationDialog itsBetaProductAutomationDialogTab;
    CTabCtrlSSL itsBetaProductControl;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
public:
    virtual BOOL OnInitDialog();
    virtual BOOL Create(CWnd* pParentWnd = NULL); // modaalitonta dialogi luontia varten
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    void Update();
    virtual void OnCancel();
    virtual void OnOK();
    afx_msg void OnClose();
};
