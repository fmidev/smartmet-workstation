#pragma once

#include "SmartMetDialogs_resource.h"

class NFmiVisualizationSpaceoutSettings;

// CFmiVisualizationSettings dialog

class CFmiVisualizationSettings : public CDialog
{
	DECLARE_DYNAMIC(CFmiVisualizationSettings)

public:
	CFmiVisualizationSettings(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CFmiVisualizationSettings();

// Dialog Data
	enum { IDD = IDD_DIALOG_VISUALIZATION_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	void InitDialogTexts();

	NFmiVisualizationSpaceoutSettings* itsVisualizationSpaceoutSettings = nullptr;
	double itsPixelToGridPointRatio;
	double itsGlobalVisualizationSpaceoutFactor;
	BOOL itsUsePixelToGridPointRatioSafetyFeature;
	BOOL itsUseGlobalVisualizationSpaceoutFactorOptimization;
	CComboBox itsSpaceoutDataGatheringMethodComboBox;
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};
