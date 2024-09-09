#pragma once

// FmiModifyDrawParamDlg.h : header file
//
#include "stdafx.h"
#include "SmartMetToolboxDep_resource.h"
#include "afxwin.h"
#include "NFmiDrawParam.h"
#include "WzComboBox.h"
#include "CtrlViewWin32Functions.h"
#include "PPToolTip.h"

// MARKO Huomio! Outo MFC juttu: GROUPBOX:ien pit‰‰ olla dialogissa viimeisin‰, muuten static -tyyppiset kontrollit eiv‰t saa
// CPPToolTip luokan tooltippi‰ toimimaan. Tuo j‰rjestys t‰ss‰ viittaa tab-orderiin ja sit‰ ei siis saa j‰rjest‰‰ 'oikein' groupboxien kanssa.

class SmartMetDocumentInterface;
class NFmiFixedDrawParamFolder;

enum class ColorType
{
	SimpleContour = 1,
	SimpleIsoline
};

/////////////////////////////////////////////////////////////////////////////
// CFmiModifyDrawParamDlg dialog

class CFmiModifyDrawParamDlg : public CDialog
{
// Construction
public:
	CFmiModifyDrawParamDlg(SmartMetDocumentInterface *smartMetDocumentInterface, boost::shared_ptr<NFmiDrawParam> &theDrawParam, const std::string &theDrawParamPath, bool modifyMapViewParam, bool modifyCrossSectionViewParam, unsigned int theDescTopIndex, unsigned int theRealRowNumber, CWnd* pParent = NULL);
	~CFmiModifyDrawParamDlg(void);

	bool RefreshPressed(void) const {return fRefreshPressed;}
	bool SkipReadingSpecialClassColorIndices(void) const {return fSkipReadingSpecialClassColorIndices;}
	void SkipReadingSpecialClassColorIndices(bool newValue) {fSkipReadingSpecialClassColorIndices = newValue;}

	// Dialog Data
	//{{AFX_DATA(CFmiModifyDrawParamDlg)
	enum { IDD = IDD_DIALOG_DRAWPARAM_EDITOR_DLG2 };
	CButton	itsSimpleColorContourMidColor;
	CButton	itsSimpleColorContourLowColor;
	CButton	itsSimpleColorContourHighColor;
	CButton itsSimpleColorContourHigh2Color;
	CButton itsSimpleColorContourHigh3Color;
	CButton	itsHatch2Color;
	CButton	itsSymbolFillColor;
	CButton	itsSymbolColor;
	CButton	itsIsoLineLabelColor;
	CButton	itsIsoLineColor;
	CButton itsIsolineLabelBoxFillColor;
	CButton	itsSymbolMidColor;
	CButton	itsSymbolLowColor;
	CButton	itsSymbolHighColor;
	CButton	itsSimpleIsoLineLowColor;
	CButton	itsSimpleIsoLineMidColor;
	CButton	itsSimpleIsoLineHighColor;
	CButton	itsSimpleIsoLineHigh2Color;
	CButton itsSimpleIsoLineHigh3Color;
	CButton	itsHatch1Color;
	BOOL	fUSeChangingColorsWithSymbols;
	BOOL	fUSeMultiColorWithSimpleIsoLines;
	BOOL	fUSeColorBlendWithSimpleIsoLines;
	BOOL	fUseIsoLineFeathering;
	BOOL	fUseHatch1;
	BOOL	fDrawLabelBox;
	BOOL	fDrawOnlyOverMask; // oikeasti t‰m‰ on piirr‰ vain maskin p‰‰lle optio
	BOOL	fUseSpecialClasses;
	int		itsHatch1Type;
	double	itsHatch1EndValue;
	double	itsHatch1StartValue;
	double	itsIsoLineGap;
	int		itsIsoLineDecimals;
	double	itsIsoLineLabelHeight;
	float	itsIsoLineSmoothFactor;
	int		itsIsoLineStyle;
	float	itsIsoLineWidth;
	double	itsModifyingStep;
	double	itsOneSymbolHeight;
	double	itsOneSymbolWidth;
    double itsOneSymbolHorizontalOffset_NEW;
    double itsOneSymbolVerticalOffset_NEW;
    double	itsParamAbsolutValueMax;
	double	itsParamAbsolutValueMin;
    CString	itsParamAbbreviationStrU_;
    CString	itsParamNameStrU_;
    CString	itsProducerStrU_;
	int		itsGridDataDrawStyle;
	float	itsSimpleIsoLineColorLowValue;
	CString itsSimpleIsoLineColorLowValueStringU_;
	float   itsSimpleIsoLineColorMidValue;
	CString itsSimpleIsoLineColorMidValueStringU_;
	float	itsSimpleIsoLineColorHighValue;
	CString itsSimpleIsoLineColorHighValueStringU_;
	float	itsSimpleIsoLineColorHigh2Value;
	CString itsSimpleIsoLineColorHigh2ValueStringU_;
	int		itsSymbolsWithColorsClassCount;
	float	itsSymbolsWithColorsEndValue;
	float	itsSymbolsWithColorsMiddleValue;
	float	itsSymbolsWithColorsStartValue;
	int		itsSpecialIsolineClassCount;
	int		itsSpecialContourClassCount;
    CString	itsSpecialIsolineColorIndicesStrU_;
    CString	itsSpecialClassLabelHeightsStrU_; // ei lˆydy drawparamista!!!!! // k‰ytet‰‰n v‰liaikaisesti label korkeuden kanssa
    CString	itsSpecialClassLineStylesStrU_;
    CString	itsSpecialClassLineWidthsStrU_;
    CString	itsSpecialIsolineClassValuesStrU_;
	CString itsSpecialContourClassValuesStrU_;
	CString itsSpecialContourColorIndicesStrU_;
	double	itsTimeSeriesModifyLimit;
	double	itsTimeSeriesScaleMax;
	double	itsTimeSeriesScaleMin;
    CString	itsParamUnitStrU_;
	BOOL	fIsHidden;
	BOOL	fUseHatch2;
	int 	itsHatch2Type;
	double	itsHatch2EndValue;
	double	itsHatch2StartValue;
	BOOL	fUSeSeparatingLinesBetweenColorContourClasses;
    double itsIsoLineZeroValue_NEW;
	CString itsSimpleColorContourLimit1StringU_;
	float itsSimpleColorContourLimit1Value = kFloatMissing; // Saadaan vastaavasta string valuesta
	CString itsSimpleColorContourLimit2StringU_;
	float itsSimpleColorContourLimit2Value = kFloatMissing; // Saadaan vastaavasta string valuesta
	CString itsSimpleColorContourLimit3StringU_;
	float itsSimpleColorContourLimit3Value = kFloatMissing; // Saadaan vastaavasta string valuesta
	CString itsSimpleColorContourLimit4StringU_;
	float itsSimpleColorContourLimit4Value = kFloatMissing; // Saadaan vastaavasta string valuesta
	//}}AFX_DATA

	// T‰m‰n muuttujan avulla v‰ritet‰‰n static "Isoline classes" kontrolli teksti 
	// tarvittaessa punaiseksi ett‰ k‰ytt‰j‰ n‰kee ett‰ annetut luokka rajat ovat 
	// jotenkin virheellisi‰. Suurin ongelma tulee kun kaikki arvot eiv‰t ole 
	// nousevassa j‰rjestyksess‰. T‰llˆin ohjelma toimii oudosti.
	bool fSpecialIsolineClassesHaveInvalidValues;
	bool fSpecialContourClassesHaveInvalidValues;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiModifyDrawParamDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFmiModifyDrawParamDlg)
	afx_msg void OnButtonHatch1Color();
	afx_msg void OnButtonColorShowSimpleIsolineLow();
	afx_msg void OnButtonColorShowSimpleIsolineMid();
	afx_msg void OnButtonColorShowSimpleIsolineHigh();
	afx_msg void OnButtonColorShowSimpleIsolineHigh2();
	afx_msg void OnButtonColorShowSimpleIsolineHigh3();
	afx_msg void OnButtonColorShowSymbHigh();
	afx_msg void OnButtonColorShowSymbLow();
	afx_msg void OnButtonColorShowSymbMid();
	afx_msg void OnButtonIsolineColor();
	afx_msg void OnBnClickedButtonIsolineLabelBoxFillColor();
	afx_msg void OnButtonIsolineLabelColor();
	afx_msg void OnButtonSymbolColor();
	afx_msg void OnButtonSymbolFillColor();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSaveAsButton();
	afx_msg void OnSaveButton();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedShowIsolineColorIndexDlg();
	afx_msg void OnButtonHatch2Color();
	afx_msg void OnButtonColorShowSimpleColorcontourLow();
	afx_msg void OnButtonColorShowSimpleColorcontourMid();
	afx_msg void OnButtonColorShowSimpleColorcontourHigh();
	afx_msg void OnButtonColorShowSimpleColorcontourHigh2();
	afx_msg void OnButtonColorShowSimpleColorcontourHigh3();
	afx_msg void OnButtonResetDrawParam();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	void InitDialogFromDrawParam(void);
	void IsHidden(void);
	void ParamAbbreviation(void);
	void OneSymbolRelativeSize(void);
	void OneSymbolRelativePositionOffset(void);
	void IsoLineGab(void);
	void ModifyingStep(void);
	void AbsoluteMinAndMaxValue(void);
	void TimeSeriesScaleMinAndMax(void);
	void TimeSerialModifyingLimit(void);
	void ReadParameterAbbreviation(void);
	void ReadHidden(void);
	void ReadOneSymbolRelativeSize();
	void ReadOneSymbolRelativePositionOffset(void);
	void ReadIsoLineGab(void);
	void ReadModifyingStep(void);
	void ReadAbsoluteMinAndMaxValue(void);
	void ReadTimeSeriesScaleMinAndMax(void);
	void ReadTimeSerialModifyingLimit(void);
	void GetSelectedDrawParamSetUp(void);
	void ReadAllButtonColors(void);
	void InitRestOfVersion2Data(void);
	void InitSpecialClassesData(void);
	void ReadRestOfVersion2Data(void);
	void ReadSpecialClassesData(void);
	void InitColors(void);
	void MakeViewMacroAdjustments(void);
    void FillStationDataViewSelector(void);

	void InitDialogTexts(void);
	void TakeDrawParamModificationInUse(void);
    void DoOnCancel();
    void InitFixedDrawParamSelector();
    void AddFixedDrawParamFolderToSelector(int theTreeIndex, const NFmiFixedDrawParamFolder &theFolder, CWzComboBox &theFixedDrawParamSelector);
    std::shared_ptr<NFmiDrawParam> GetSelectedFixedDrawParam();
    std::string GetSelectedFixedDrawParamPath();
    void ForceStationViewUpdate();
    void AdjustStartingPosition();
	std::pair<bool, bool> GetSimpleContourTransparencyAndDisabledOptions(int colorIndex) const;
	std::pair<bool, bool> GetSimpleIsolineTransparencyAndDisabledOptions(int colorIndex) const;
	NFmiColorButtonDrawingData GetSimpleContourColorButtonData(int colorIndex, bool initColor);
	NFmiColorButtonDrawingData GetSimpleIsolineColorButtonData(int colorIndex, bool initColor);
	void SetSimpleColorContourLimit(float limitValue, float* limitValueDlg, CString* limitStringDlgU_);
	void OnEnChangeShowSimpleColorcontourLimitValue(ColorType colorType, int colorIndex, CString& limitStringU_, float& limitValue);
	void InitSymbolDrawDensitySliders();
	std::pair<double, double> GetSymbolDrawDensityValuesFromSlider();
	void UpdateSymbolDrawDensityStr();
	bool IsMacroParamCase();
	bool IsMacroParamSymbolDrawCase();
	bool IsPossibleColorParameterStrOk(const CString& colorParameterStr);
	void DoPostInitializationChecks();
	void InitTooltipControl();
	void SetDialogControlTooltip(int controlId, const std::string& tooltipRawText);
	void OnShowColorIndexDlg(bool doIsolineCase);

	std::string itsDrawParamPath;
	boost::shared_ptr<NFmiDrawParam> itsDrawParam;
	boost::shared_ptr<NFmiDrawParam> itsBackupDrawParam; // viel‰ yksi vara drawParam, jos joku haluaa katsoa muutoksia p‰ivitys-napista, muttasitten painaa
									  // lopuksi cancelia, t‰llˆin t‰‰ll‰ on originaali tallessa.
	boost::shared_ptr<NFmiDrawParam> itsOrigDrawParam; // t‰m‰ on originaali data ja se on tallessa tallessa sit‰ varten ett‰ muutokset eiv‰t
									 // voimaan muutoin kun OK-nappia painamalla, tai kun parametri talletetaan ja/tai otetaan k‰yttˆˆn kaikkialla.
	bool fRefreshPressed; // optimointia, t‰h‰n merkit‰‰n vain onko refreshi‰ painettu, jonka j‰lkeen jos cancelia painettu, pit‰‰ ruutu p‰ivitt‰‰
	bool fModifyMapViewParam;
	bool fModifyCrossSectionViewParam;

	COLORREF itsSymbolFillColorRef;
	COLORREF itsSymbolColorRef;
	COLORREF itsIsoLineLabelColorRef;
	COLORREF itsIsoLineColorRef;
	COLORREF itsIsoLineLabelBoxFillColorRef;
	COLORREF itsSymbolMidColorRef;
	COLORREF itsSymbolLowColorRef;
	COLORREF itsSymbolHighColorRef;
	COLORREF itsSimpleIsoLineLowColorRef;
	COLORREF itsSimpleIsoLineMidColorRef;
	COLORREF itsSimpleIsoLineHighColorRef;
	COLORREF itsSimpleIsoLineHigh2ColorRef;
	COLORREF itsSimpleIsoLineHigh3ColorRef;
	COLORREF itsSimpleColorContourLowColorRef;
	COLORREF itsSimpleColorContourMidColorRef;
	COLORREF itsSimpleColorContourHighColorRef;
	COLORREF itsSimpleColorContourHigh2ColorRef;
	COLORREF itsSimpleColorContourHigh3ColorRef;
	COLORREF itsHatch1ColorRef;
	COLORREF itsHatch2ColorRef;
	CBitmap* itsSymbolFillBitmap;
	CBitmap* itsSymbolBitmap;
	CBitmap* itsIsoLineLabelBitmap;
	CBitmap* itsIsoLineBitmap;
	CBitmap* itsIsoLineLabelBoxFillBitmap;
	CBitmap* itsSymbolMidBitmap;
	CBitmap* itsSymbolLowBitmap;
	CBitmap* itsSymbolHighBitmap;
	CBitmap* itsSimpleIsoLineLowBitmap;
	CBitmap* itsSimpleIsoLineMidBitmap;
	CBitmap* itsSimpleIsoLineHighBitmap;
	CBitmap* itsSimpleIsoLineHigh2Bitmap;
	CBitmap* itsSimpleIsoLineHigh3Bitmap;
	CBitmap* itsSimpleColorContourLowBitmap;
	CBitmap* itsSimpleColorContourMidBitmap;
	CBitmap* itsSimpleColorContourHighBitmap;
	CBitmap* itsSimpleColorContourHigh2Bitmap;
	CBitmap* itsSimpleColorContourHigh3Bitmap;
	CBitmap* itsHatch1Bitmap;
	CBitmap* itsHatch2Bitmap;
	CRect itsSymbolFillColorRect;
	CRect itsSymbolColorRect;
	CRect itsIsoLineLabelColorRect;
	CRect itsIsoLineColorRect;
	CRect itsIsoLineLabelBoxFillColorRect;
	CRect itsSymbolMidColorRect;
	CRect itsSymbolLowColorRect;
	CRect itsSymbolHighColorRect;
	CRect itsSimpleIsoLineLowColorRect;
	CRect itsSimpleIsoLineMidColorRect;
	CRect itsSimpleIsoLineHighColorRect;
	CRect itsSimpleIsoLineHigh2ColorRect;
	CRect itsSimpleIsoLineHigh3ColorRect;
	CRect itsSimpleColorContourLowColorRect;
	CRect itsSimpleColorContourMidColorRect;
	CRect itsSimpleColorContourHighColorRect;
	CRect itsSimpleColorContourHigh2ColorRect;
	CRect itsSimpleColorContourHigh3ColorRect;
	CRect itsHatch1ColorRect;
	CRect itsHatch2ColorRect;

	bool fSkipReadingSpecialClassColorIndices; // t‰m‰ on viritys, mit‰ tarvitaan ett‰ v‰riindeksi pikap‰ivitys toimisi

    SmartMetDocumentInterface *itsSmartMetDocumentInterface; // p‰ivit‰ napista p‰ivitet‰‰n ruudut t‰m‰n avulla (ja muita tarpeita)
	unsigned int itsDescTopIndex; // jos piirto-ominaisuudet liittyv‰t jonkun karttan‰ytˆn (desctop) parametriin, pit‰‰ siit‰ olla tieto, ett‰ oikeaa karttan‰yttˆ‰ osataan p‰ivitt‰‰
	unsigned int itsRealRowNumber; // Tietyiss‰ tilanteissa pit‰‰ tiet‰‰ mill‰ rivill‰ joku drawParam oli
    CString itsDrawParamFileNameStrU_;
    CComboBox itsStationDataViewSelector;
    BOOL fUseColorBlendingWithCustomContours;
    double itsContourGap;
    float itsAlpha;
    CWzComboBox itsFixedDrawParamSelector; // puurakenteinen dropdown lista, miss‰ kansioita ja drawParameita
    bool fFixedDrawParamSelectorInitialized;
    int itsIconTypeFolderId; // kun itsFixedDrawParamSelector:ille annetaan kansio ja drawParam ikonit, n‰ihin talletetaan niiden vastaavat id:t
    int itsIconTypeFileId;
    BOOL fUseTransparentLabelBoxFillColor;
    BOOL fDoSparseDataSymbolVisualization;
    BOOL fUseLegend;
	BOOL fSimpleContourTransparency1;
	BOOL fSimpleContourTransparency2;
	BOOL fSimpleContourTransparency3;
	BOOL fSimpleContourTransparency4;
	BOOL fSimpleContourTransparency5;
	BOOL fTreatWmsLayerAsObservation;
	CSliderCtrl itsSymbolDrawDensityXSlider;
	CSliderCtrl itsSymbolDrawDensityYSlider;
	int itsFixedTextSymbolDrawLength;
	CString itsSymbolDrawDensityStr;
	CString itsPossibleColorParameterStr;
	bool fPossibleColorParameterOk = false;
	CPPToolTip m_tooltip;
	BOOL fFlipArrowSymbol;

public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedModifyDrwParamRefresh();
	afx_msg void OnBnClickedDrawParamLoadFrom();
	afx_msg void OnBnClickedModifyDrwParamUseWithAll();
	afx_msg void OnClose();
	afx_msg void OnEnChangeSpecialIsolineClassesValues();
    afx_msg void OnCbnSelchangeComboFixedDrawParamSelector();
    afx_msg void OnBnClickedButtonReloadOriginal();
	afx_msg void OnBnClickedCheckSimpleContourTransparency1();
	afx_msg void OnBnClickedCheckSimpleContourTransparency2();
	afx_msg void OnBnClickedCheckSimpleContourTransparency3();
	afx_msg void OnBnClickedCheckSimpleContourTransparency4();
	afx_msg void OnBnClickedCheckSimpleContourTransparency5();
	afx_msg void OnEnChangeShowSimpleColorcontourWithColorsStartValue();
	afx_msg void OnEnChangeShowSimpleColorcontourWithColorsMiddleValue();
	afx_msg void OnEnChangeShowSimpleColorcontourWithColorsEndValue();
	afx_msg void OnEnChangeShowSimpleColorcontourWithColorsEnd2Value();
	afx_msg void OnEnChangeShowSimpleIsolineWithColorsStartValue();
	afx_msg void OnEnChangeShowSimpleIsolineWithColorsMiddleValue();
	afx_msg void OnEnChangeShowSimpleIsolineWithColorsEndValue();
	afx_msg void OnEnChangeShowSimpleIsolineWithColorsEnd2Value();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEnChangeEditDrawParamColorParamStr();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnEnChangeSpecialContourClassesValues();
	afx_msg void OnBnClickedShowContourColorIndexDlg();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

