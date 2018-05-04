#pragma once

#include "SmartMetToolboxDep_resource.h"

class NFmiMapViewDescTop;
class NFmiMapViewWinRegistry;

// CFmiMapViewSettingsDlg dialog

class CFmiMapViewSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(CFmiMapViewSettingsDlg)

public:
	CFmiMapViewSettingsDlg(NFmiMapViewDescTop &theMapViewdescTop, NFmiMapViewWinRegistry &theMapViewWinRegistry, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiMapViewSettingsDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_MAP_VIEW_DESC_TOP_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
private:

	NFmiMapViewDescTop &itsMapViewdescTop;
	NFmiMapViewWinRegistry &itsMapViewWinRegistry;

	BOOL fDrawCountryBorders;
	BOOL fSpaceOutSymbolicGridData;
	BOOL fLockToMainMapViewTime;
	BOOL fShowTrajectorsOnMap;
	BOOL fShowSoundingMarkersOnMap;
	BOOL fShowCrossSectionMarkersOnMap;
	BOOL fShowSelectedPointsOnMap;
	BOOL fShowControlPointsOnMap;
	BOOL fShowObsComparisonOnMap;
	float itsTimeControlTimeStepInHours;
	BOOL fShowWarningMessageMarkersOnMap;
	int itsMapViewDisplayMode;
	int itsMapViewGridColumnCount;
	int itsMapViewGridRowCount;
};
