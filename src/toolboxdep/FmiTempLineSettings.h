#pragma once

#include "SmartMetToolboxDep_resource.h"
#include "afxcmn.h"
#include "NFmiMTATempSystem.h"
#include "afxwin.h"


// CFmiTempLineSettings dialog

class CFmiTempLineSettings : public CDialog
{
	DECLARE_DYNAMIC(CFmiTempLineSettings)

public:
	CFmiTempLineSettings(const CString &theTitleStr, bool isHelpLine, bool isHelpText,
						NFmiTempLineInfo *theLineInfo,
						NFmiTempLabelInfo *theLabelInfo,
						CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiTempLineSettings();

// Dialog Data
	enum { IDD = IDD_DIALOG_HELP_LINE_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnBnClickedButtonColor();
private:
	void EnableControls(void);
	void UpdateLineThicknessStr(void);
	void InitDialogTexts(void);

    CString itsTitleStrU_;
	bool fIsHelpLine; // esim. T ja TD ovat toisenlaisia (falseja), n‰ist‰ s‰‰det‰‰n vain viivan tyyppi ja paksuus
					  // mutta kaikille todellisille apuviivoille (mix,dry,moist jne.) voidaan s‰‰t‰‰ kaikkia arvoja
	bool fIsHelpText; // esim. z-arvojen kanssa alustetaan ja muokataan vain osaa jutuista, kyse on tekstist‰, ei apu viivasta
    NFmiTempLineInfo *itsLineInfoOrig; // ei omista, ei tuhoa
    NFmiTempLabelInfo *itsLabelInfoOrig; // ei omista, ei tuhoa
    NFmiTempLineInfo itsLineInfo; // t‰h‰n muutokset, jotka kopioidaan orig:eihin jos OnOK
    NFmiTempLabelInfo itsLabelInfo; // t‰h‰n muutokset, jotka kopioidaan orig:eihin jos OnOK

	CSliderCtrl itsLineWidthSlider;
	BOOL fDrawLine;
	int itsLineType;
	int itsLabelOffsetX;
	int itsLabelOffsetY;
	int itsLabelFontSize;
	BOOL fDrawLabel;
	BOOL fDrawLabelInDataBox;
	CButton itsColorButtom;
	COLORREF itsColorRef;
	CBitmap* itsColorBitmap;
	CRect itsColorRect;
    CString itsLabelFontAlignmentStrU_;
};
