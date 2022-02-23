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

	NFmiApplicationWinRegistry* itsApplicationWinRegistry = nullptr;
	double itsPixelToGridPointRatio;
	double itsGlobalVisualizationSpaceoutFactor;
	BOOL itsUsePixelToGridPointRatioSafetyFeature;
	BOOL itsUseGlobalVisualizationSpaceoutFactorOptimization;
	CComboBox itsSpaceoutDataGatheringMethodComboBox;
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBnClickedButtonUpdate();
};
