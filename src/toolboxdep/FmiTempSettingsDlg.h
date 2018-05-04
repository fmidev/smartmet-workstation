#pragma once

#include "SmartMetToolboxDep_resource.h"
#include "afxcmn.h"
#include "NFmiMTATempSystem.h"
#include "afxwin.h"

class CFmiTempDlg;
// CFmiTempSettingsDlg dialog

class CFmiTempSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(CFmiTempSettingsDlg)

public:
    CFmiTempSettingsDlg(NFmiMTATempSystem *theTempSystem, CFmiTempDlg* theTempDialog);
	virtual ~CFmiTempSettingsDlg();
    BOOL Create(void); // modaalittomaa varten

// Dialog Data
	enum { IDD = IDD_DIALOG_TEMP_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
	virtual void OnCancel();
private:
	CRect GetDldResourceRectInClientSpace(int helpImageResourceId);
	void UpdateDegreeStr(void);
	void DoLineSettingDialog(int helpImageResourceId, const CString &theTitle, bool isHelpLine, bool isHelpText,
							NFmiTempLineInfo *theLineInfo,
							NFmiTempLabelInfo *theLabelInfo);
	void DrawHelperLineSampleImages(CDC *dc, int imageHolderResourceId,
        NFmiTempLineInfo *theLineInfo,
        NFmiTempLabelInfo *theLabelInfo);
	void InitDialogTexts(void);

	NFmiMTATempSystem *itsTempSystem; // ei omista, ei tuhoa

	CSliderCtrl itsTDegreeSlider;
	double itsTStart1;
	double itsTEnd1;
	double itsTStart2;
	double itsTEnd2;
	double itsTStart3;
	double itsTEnd3;
	double itsPStart;
	double itsPEnd;
	int itsWindVectorSizeX;
	int itsWindVectorSizeY;
	int itsLegendTextSize;
	double itsHelpLineTStartValue;
	double itsHelpLineTEndValue;
	double itsHelpLineTStepValue;
    CString itsHelpLineValuesPU_;
    CString itsHelpLineValuesDryU_;
    CString itsHelpLineValuesMoistU_;
    CString itsHelpLineValuesMixU_;
public:
	afx_msg void OnBnClickedButtonSetHelpLineSettingsT();
	afx_msg void OnBnClickedButtonSetHelpLineSettingsP();
	afx_msg void OnBnClickedButtonSetHelpLineSettingsDry();
	afx_msg void OnBnClickedButtonSetHelpLineSettingsMoist();
	afx_msg void OnBnClickedButtonSetHelpLineSettingsMix();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedButtonSetLineSettingsT();
	afx_msg void OnBnClickedButtonSetLineSettingsTd();
	afx_msg void OnBnClickedButtonSetLineSettingsZ();
private:
	BOOL fDrawWindVector;
	BOOL fDrawLegend;
	BOOL fDrawHeightValuesOnlyForFirstSounding;
	CButton itsColorButtomTemp1;
	CButton itsColorButtomTemp2;
	CButton itsColorButtomTemp3;
	CButton itsColorButtomTemp4;
	CButton itsColorButtomTemp5;
	CButton itsColorButtomTemp6;
	CButton itsColorButtomTemp7;
	CButton itsColorButtomTemp8;
	CButton itsColorButtomTemp9;
	CButton itsColorButtomTemp10;

	COLORREF itsColorRef1;
	COLORREF itsColorRef2;
	COLORREF itsColorRef3;
	COLORREF itsColorRef4;
	COLORREF itsColorRef5;
	COLORREF itsColorRef6;
	COLORREF itsColorRef7;
	COLORREF itsColorRef8;
	COLORREF itsColorRef9;
	COLORREF itsColorRef10;
	CBitmap* itsColorBitmap1;
	CBitmap* itsColorBitmap2;
	CBitmap* itsColorBitmap3;
	CBitmap* itsColorBitmap4;
	CBitmap* itsColorBitmap5;
	CBitmap* itsColorBitmap6;
	CBitmap* itsColorBitmap7;
	CBitmap* itsColorBitmap8;
	CBitmap* itsColorBitmap9;
	CBitmap* itsColorBitmap10;
	CRect itsColorRect1;
	CRect itsColorRect2;
	CRect itsColorRect3;
	CRect itsColorRect4;
	CRect itsColorRect5;
	CRect itsColorRect6;
	CRect itsColorRect7;
	CRect itsColorRect8;
	CRect itsColorRect9;
	CRect itsColorRect10;
public:
	afx_msg void OnBnClickedButtonColorTemp1();
	afx_msg void OnBnClickedButtonColorTemp2();
	afx_msg void OnBnClickedButtonColorTemp3();
	afx_msg void OnBnClickedButtonColorTemp4();
	afx_msg void OnBnClickedButtonColorTemp5();
	afx_msg void OnBnClickedButtonColorTemp6();
	afx_msg void OnBnClickedButtonColorTemp7();
	afx_msg void OnBnClickedButtonColorTemp8();
	afx_msg void OnBnClickedButtonColorTemp9();
	afx_msg void OnBnClickedButtonColorTemp10();

private:
	// kuinka monta luotausta kerrallaan voidaan maksimissaan näyttää
	int itsMaxTempsOnView;
public:
	afx_msg void OnPaint();
private:
	// indeksi/luotaus tekstinä näytössä käytetty fontti koko pikseleinä
	int itsIndexiesFontSize;
	BOOL fShowCondensationTrailProbabilityLines;
	int itsSoundingTextFontSize;
	BOOL fShowKilometerScale;
	BOOL fShowFlightLevelScale;
	BOOL fShowOnlyFirstSoundingInHodograf;
	double itsResetScalesStartP;
	double itsResetScalesEndP;
	double itsResetScalesStartT;
	double itsResetScalesEndT;
	double itsResetScalesSkewTStartT;
	double itsResetScalesSkewTEndT;
public:
	afx_msg void OnBnClickedButtonSetHelpLineSettingsAirParcel1();
	afx_msg void OnBnClickedButtonSetHelpLineSettingsAirParcel2();
	afx_msg void OnBnClickedButtonSetHelpLineSettingsAirParcel3();
	afx_msg void OnBnClickedButtonSetHelpLineSettingsWindModificationArea();
    afx_msg void OnBnClickedButtonSetHelpLineSettingsWs();
    afx_msg void OnBnClickedButtonSetHelpLineSettingsN();
    afx_msg void OnBnClickedButtonSetHelpLineSettingsRh();
    afx_msg void OnBnClickedButtonRefrashSoundingView();
private:
    void UpdateTempView(void);
    void GetSettingsFromDialog();
    void DoOnCancel();

    BOOL fShowSecondaryDataView;
    // Kuinka ison osan prosenteissa apudatanäyttö view luotausnäytön leveydestä (10 - 40 %)
    int itsSecondaryDataViewWidth;
    CFmiTempDlg* itsTempDialog;  // Luotaus dialogi jotta sitä voidaan päivittää refresh nappulalla
    NFmiMTATempSystem itsOrigMTATempSystem; // Tämän avulla voidaan palauttaa ooriginaali asetukset, jos on vaihdettu asetuksia ja sitten painettu Refresh -nappulaa ja lopuksi halutaan canceloida kaikki muutokset.
    bool fRefreshPressed; // Tieto siitä jos on painettu Refresh-nappulaa
public:
    afx_msg void OnClose();
};
