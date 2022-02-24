#pragma once

#include "SmartMetDialogs_resource.h"
#include "NFmiViewPosRegistryInfo.h"

class NFmiApplicationWinRegistry;

// CFmiVisualizationSettings dialog

class CFmiVisualizationSettings : public CDialogEx
{
private: // näillä talletetaan sijainti ja koko rekisteriin
	static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
	static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void) { return s_ViewPosRegistryInfo; }

	DECLARE_DYNAMIC(CFmiVisualizationSettings)

public:
	CFmiVisualizationSettings(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CFmiVisualizationSettings();

	void SetDefaultValues();
	static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) { return ViewPosRegistryInfo().WinRegistryKeyStr(); }
	void Update();

// Dialog Data
	enum { IDD = IDD_DIALOG_VISUALIZATION_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	void InitDialogTexts();
	void UpdateValuesBackToDocument();
	void CheckInputAndDoWarningTexts();
	void InitPixelToGridPointRatioSlider();
	void InitGlobalVisualizationSpaceoutFactorSlider();
	int CalcPixelToGridPointRatioSliderPosition(double value);
	int CalcGlobalVisualizationSpaceoutFactorSliderPosition(double value);
	double GetPixelToGridPointRatioFromSlider();
	double GetGlobalVisualizationSpaceoutFactorFromSlider();
	void UpdatePixelToGridPointRatioFromSliderToStringCtrl();
	void UpdateGlobalVisualizationSpaceoutFactorFromSliderToStringCtrl();

	NFmiApplicationWinRegistry* itsApplicationWinRegistry = nullptr;
	double itsPixelToGridPointRatio_valueFromSlider;
	double itsGlobalVisualizationSpaceoutFactor_valueFromSlider;
	BOOL itsUsePixelToGridPointRatioSafetyFeature;
	BOOL itsUseGlobalVisualizationSpaceoutFactorOptimization;
	CComboBox itsSpaceoutDataGatheringMethodComboBox;
	CString itsPixelToGridPointRatioWarningStr;
	CSliderCtrl itsPixelToGridPointRatioSlider;
	CString itsPixelToGridPointRatioValueStr;
	CSliderCtrl itsGlobalVisualizationSpaceoutFactorSlider;
	CString itsGlobalVisualizationSpaceoutFactorResultsStr;
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBnClickedButtonUpdate();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedCheckVisualizationsUsePixelToGridPointRatioSafetyFeature();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
