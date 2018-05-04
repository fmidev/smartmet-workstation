#pragma once

#include "SmartMetDialogs_resource.h"
#include "gridctrl.h"

class WmoIdFilterManager;

// pit‰‰ peri‰ oma gridCtrl-luokka, ett‰ se osaa hanskata parit kikat
class NFmiGridCtrl2 : public CGridCtrl
{
    DECLARE_DYNCREATE(NFmiGridCtrl2)

public:
    NFmiGridCtrl2(int nRows = 0, int nCols = 0, int nFixedRows = 0, int nFixedCols = 0)
	:CGridCtrl(nRows, nCols, nFixedRows, nFixedCols)
	,itsWmoIdFilterManager(0)
	{}

	void SetWmoIdFilterManager(WmoIdFilterManager *theWmoIdFilterManager){itsWmoIdFilterManager = theWmoIdFilterManager;};

private:
	WmoIdFilterManager *itsWmoIdFilterManager; // ei omista/tuhoa
public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};


// CFmiCountryFilterDlg dialog

class CFmiCountryFilterDlg : public CDialog
{
	DECLARE_DYNAMIC(CFmiCountryFilterDlg)

public:
	CFmiCountryFilterDlg(WmoIdFilterManager *theWmoIdFilterManager, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiCountryFilterDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_COUNTRY_FILTER };

	virtual BOOL OnInitDialog();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	void InitDialogTexts(void);

	DECLARE_MESSAGE_MAP()
private:
	BOOL fShowAll;
	WmoIdFilterManager *itsWmoIdFilterManager; // ei omista, ei tuhoa, p‰ivitet‰‰n vain OnOk-metodissa
	NFmiGridCtrl2 itsGridCtrl;
};
