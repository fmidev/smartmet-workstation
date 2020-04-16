// FmiMapViewSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiMapViewSettingsDlg.h"
#include "NFmiMapViewDescTop.h"
#include "NFmiMetEditorOptionsData.h"
#include "NFmiApplicationWinRegistry.h"
#include "CloneBitmap.h"
#include "CtrlViewFunctions.h"
#include "FmiWin32Helpers.h"


// CFmiMapViewSettingsDlg dialog

IMPLEMENT_DYNAMIC(CFmiMapViewSettingsDlg, CDialog)
CFmiMapViewSettingsDlg::CFmiMapViewSettingsDlg(NFmiMapViewDescTop &theMapViewdescTop, NFmiMapViewWinRegistry &theMapViewWinRegistry, CWnd* pParent)
	: CDialog(CFmiMapViewSettingsDlg::IDD, pParent)
	, itsMapViewdescTop(theMapViewdescTop)
    , itsMapViewWinRegistry(theMapViewWinRegistry)
	, fDrawCountryBorders(FALSE)
	, fSpaceOutSymbolicGridData(FALSE)
	, fLockToMainMapViewTime(FALSE)
    , fLockToMainMapViewRow(FALSE)
	, fShowTrajectorsOnMap(FALSE)
	, fShowSoundingMarkersOnMap(FALSE)
	, fShowCrossSectionMarkersOnMap(FALSE)
	, fShowSelectedPointsOnMap(FALSE)
	, fShowControlPointsOnMap(FALSE)
	, fShowObsComparisonOnMap(FALSE)
	, itsTimeControlTimeStepInHours(0)
	, fShowWarningMessageMarkersOnMap(FALSE)
	, itsMapViewDisplayMode(0)
	, itsMapViewGridColumnCount(0)
	, itsMapViewGridRowCount(0)
{
}

CFmiMapViewSettingsDlg::~CFmiMapViewSettingsDlg()
{
}

void CFmiMapViewSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_MAP_VIEW_DRAW_LAND_BORDERS, fDrawCountryBorders);
	DDX_Check(pDX, IDC_CHECK_MAP_VIEW_SPACE_OUT_SYMBOLIC_DATA, fSpaceOutSymbolicGridData);
    DDX_Check(pDX, IDC_CHECK_MAP_VIEW_LOCK_TIME, fLockToMainMapViewTime);
    DDX_Check(pDX, IDC_CHECK_MAP_VIEW_LOCK_ROW, fLockToMainMapViewRow);
	DDX_Check(pDX, IDC_CHECK_MAP_VIEW_SHOW_TRAJECTORS_ON_MAP, fShowTrajectorsOnMap);
	DDX_Check(pDX, IDC_CHECK_MAP_VIEW_SHOW_SOUNDING_MARKERS_ON_MAP, fShowSoundingMarkersOnMap);
	DDX_Check(pDX, IDC_CHECK_MAP_VIEW_SHOW_CROSSSECTION_MARKERS_ON_MAP, fShowCrossSectionMarkersOnMap);
	DDX_Check(pDX, IDC_CHECK_MAP_VIEW_SHOW_SELECTED_POINTS_ON_MAP, fShowSelectedPointsOnMap);
	DDX_Check(pDX, IDC_CHECK_MAP_VIEW_SHOW_CONTROL_POINTS_ON_MAP, fShowControlPointsOnMap);
	DDX_Check(pDX, IDC_CHECK_MAP_VIEW_SHOW_OBS_COMPARISON_ON_MAP, fShowObsComparisonOnMap);
	DDX_Text(pDX, IDC_EDIT_MAP_VIEW_TIME_STEP, itsTimeControlTimeStepInHours);
	DDX_Check(pDX, IDC_CHECK_MAP_VIEW_SHOW_WARNING_MESSAGE_MARKERS_ON_MAP, fShowWarningMessageMarkersOnMap);
	DDX_Radio(pDX, IDC_RADIO_MAP_VIEW_DISPLAY_MODE1, itsMapViewDisplayMode);
	DDX_Text(pDX, IDC_EDIT_MAP_VIEW_GRID_ROW_COUNT, itsMapViewGridColumnCount);
	DDV_MinMaxInt(pDX, itsMapViewGridColumnCount, 1, CtrlViewUtils::MaxViewGridXSize);
	DDX_Text(pDX, IDC_EDIT_MAP_VIEW_GRID_COLUMN_COUNT, itsMapViewGridRowCount);
	DDV_MinMaxInt(pDX, itsMapViewGridRowCount, 1, CtrlViewUtils::MaxViewGridYSize);
}


BEGIN_MESSAGE_MAP(CFmiMapViewSettingsDlg, CDialog)
END_MESSAGE_MAP()


// CFmiMapViewSettingsDlg message handlers

BOOL CFmiMapViewSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	fDrawCountryBorders = itsMapViewdescTop.LandBorderColorIndex() >= 0;
    fSpaceOutSymbolicGridData = itsMapViewWinRegistry.SpacingOutFactor() > 0;
    fLockToMainMapViewTime = itsMapViewdescTop.LockToMainMapViewTime();
    fLockToMainMapViewRow = itsMapViewdescTop.LockToMainMapViewRow();
	fShowTrajectorsOnMap = itsMapViewdescTop.ShowTrajectorsOnMap();
	fShowSoundingMarkersOnMap = itsMapViewdescTop.ShowSoundingMarkersOnMap();
	fShowCrossSectionMarkersOnMap = itsMapViewdescTop.ShowCrossSectionMarkersOnMap();
	fShowSelectedPointsOnMap = itsMapViewdescTop.ShowSelectedPointsOnMap();
	fShowControlPointsOnMap = itsMapViewdescTop.ShowControlPointsOnMap();
	fShowObsComparisonOnMap = itsMapViewdescTop.ShowObsComparisonOnMap();
	itsTimeControlTimeStepInHours = itsMapViewdescTop.TimeControlTimeStep();
	fShowWarningMessageMarkersOnMap = itsMapViewdescTop.ShowWarningMarkersOnMap();
	itsMapViewDisplayMode = static_cast<int>(itsMapViewdescTop.MapViewDisplayMode());
	itsMapViewGridColumnCount = static_cast<int>(itsMapViewdescTop.ViewGridSize().X());
	itsMapViewGridRowCount = static_cast<int>(itsMapViewdescTop.ViewGridSize().Y());

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiMapViewSettingsDlg::OnOK()
{
	UpdateData(TRUE);

	// Landborder piirron kanssa katsotaan onko valinta muuttunut siten ett‰ 
	// asetuksia pit‰‰ muuttaa. Eli valinnat on on/off mutta asetuksissa on oikeasti
	// v‰riindeksi, joista -1 tarkoittaa ett‰ ei piirtoa.
	if(fDrawCountryBorders)
	{
		if(itsMapViewdescTop.LandBorderColorIndex() < 0)
			itsMapViewdescTop.LandBorderColorIndex(0);
	}
	else
	{
		if(itsMapViewdescTop.LandBorderColorIndex() >= 0)
			itsMapViewdescTop.LandBorderColorIndex(-1);
	}

	// Spaceout factorin kanssa vain katsotaan onko tilanne muuttunut niin ett‰ asetuksia pit‰‰ vaihtaa.
	// Nyt valinta on on/off, mutta oikeasti asetukset ovat 0=ei harvennusta, 1=harvennus ja 2=vahva harvennus
	if(fSpaceOutSymbolicGridData)
	{
		if(itsMapViewWinRegistry.SpacingOutFactor() == 0)
			itsMapViewWinRegistry.SpacingOutFactor(1);
	}
	else
	{
		if(itsMapViewWinRegistry.SpacingOutFactor() > 0)
			itsMapViewWinRegistry.SpacingOutFactor(0);
	}

    itsMapViewdescTop.LockToMainMapViewTime(fLockToMainMapViewTime == TRUE);
    itsMapViewdescTop.LockToMainMapViewRow(fLockToMainMapViewRow == TRUE);
	itsMapViewdescTop.ShowTrajectorsOnMap(fShowTrajectorsOnMap == TRUE);
	itsMapViewdescTop.ShowSoundingMarkersOnMap(fShowSoundingMarkersOnMap == TRUE);
	itsMapViewdescTop.ShowCrossSectionMarkersOnMap(fShowCrossSectionMarkersOnMap == TRUE);
	itsMapViewdescTop.ShowSelectedPointsOnMap(fShowSelectedPointsOnMap == TRUE);
	itsMapViewdescTop.ShowControlPointsOnMap(fShowControlPointsOnMap == TRUE);
	itsMapViewdescTop.ShowObsComparisonOnMap(fShowObsComparisonOnMap == TRUE);
	itsMapViewdescTop.TimeControlTimeStep(itsTimeControlTimeStepInHours);
	itsMapViewdescTop.ShowWarningMarkersOnMap(fShowWarningMessageMarkersOnMap == TRUE);
	itsMapViewdescTop.MapViewDisplayMode(static_cast<CtrlViewUtils::MapViewMode>(itsMapViewDisplayMode));
	itsMapViewdescTop.ViewGridSize(NFmiPoint(itsMapViewGridColumnCount, itsMapViewGridRowCount), &itsMapViewWinRegistry);

	CDialog::OnOK();
}

