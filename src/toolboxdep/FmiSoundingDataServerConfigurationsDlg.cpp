#include "stdafx.h"
#include "FmiSoundingDataServerConfigurationsDlg.h"
#include "FmiWin32TemplateHelpers.h"
#include "SmartMetDocumentInterface.h"
#include "GridCellCheck.h"
#include "persist2.h"
#include "CloneBitmap.h"
#include "SoundingDataServerConfigurations.h"
#include "catlog/catlog.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"
#include "NFmiMTATempSystem.h"
#include "ModelDataServerConfiguration.h"

#include <boost/math/special_functions/round.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

static const COLORREF gFixedBkColor = RGB(239, 235, 222);

IMPLEMENT_DYNCREATE(NFmiSoundingConfGridCtrl, CGridCtrl)

BEGIN_MESSAGE_MAP(NFmiSoundingConfGridCtrl, CGridCtrl)
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

void NFmiSoundingConfGridCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	CGridCtrl::OnRButtonUp(nFlags, point);

	// Tarkoitus on deselectoida kaikki
	SetSelectedRange(-1,-1,-1,-1, TRUE, TRUE);
}


static void SetHeaders(CGridCtrl &theGridCtrl, const std::vector<SoundingConfHeaderParInfo> &theHeaders, int rowCount, bool &fFirstTime, int theFixedRowCount, int theFixedColumnCount)
{
	int columnCount = static_cast<int>(theHeaders.size());
	theGridCtrl.SetRowCount(rowCount);
	theGridCtrl.SetColumnCount(columnCount);
	theGridCtrl.SetGridLines(GVL_BOTH);
	theGridCtrl.SetFixedRowCount(theFixedRowCount);
	theGridCtrl.SetFixedColumnCount(theFixedColumnCount);
	theGridCtrl.SetListMode(TRUE);
	theGridCtrl.SetHeaderSort(FALSE);
	theGridCtrl.SetFixedBkColor(gFixedBkColor);

	int currentRow = 0;
	// 1. on otsikko rivi on parametrien nimi‰ varten
	for(int i=0; i<columnCount; i++)
	{
		theGridCtrl.SetItemText(currentRow, i, CA2T(theHeaders[i].itsHeader.c_str()));
		theGridCtrl.SetItemState(currentRow, i, theGridCtrl.GetItemState(currentRow, i) | GVIS_READONLY);
		if(fFirstTime) // s‰‰det‰‰n sarakkeiden leveydet vain 1. kerran
			theGridCtrl.SetColumnWidth(i, theHeaders[i].itsColumnWidth);
	}
	fFirstTime = false;
}

void CFmiSoundingDataServerConfigurationsDlg::InitHeaders(void)
{
	int basicColumnWidthUnit = 18;
	itsHeaders.clear();
	itsHeaders.push_back(SoundingConfHeaderParInfo("Row", SoundingConfHeaderParInfo::kRowNumber, boost::math::iround(basicColumnWidthUnit*2.5)));
    itsHeaders.push_back(SoundingConfHeaderParInfo("Name", SoundingConfHeaderParInfo::kModelName, basicColumnWidthUnit * 6));
    itsHeaders.push_back(SoundingConfHeaderParInfo("ProdId", SoundingConfHeaderParInfo::kModerProducerId, basicColumnWidthUnit * 4));
    itsHeaders.push_back(SoundingConfHeaderParInfo("Data name on server", SoundingConfHeaderParInfo::kDataNameOnServer, basicColumnWidthUnit * 15));
}

const NFmiViewPosRegistryInfo CFmiSoundingDataServerConfigurationsDlg::s_ViewPosRegistryInfo(CRect(320, 270, 970, 680), "\\SoundingDataServerConfigurationsDlg");

// CFmiSoundingDataServerConfigurationsDlg dialog

IMPLEMENT_DYNAMIC(CFmiSoundingDataServerConfigurationsDlg, CDialogEx)

CFmiSoundingDataServerConfigurationsDlg::CFmiSoundingDataServerConfigurationsDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent)
	: CDialogEx(CFmiSoundingDataServerConfigurationsDlg::IDD, pParent)
	,itsSmartMetDocumentInterface(smartMetDocumentInterface)
	,itsGridCtrl()
	,itsHeaders()
{

}

CFmiSoundingDataServerConfigurationsDlg::~CFmiSoundingDataServerConfigurationsDlg()
{
}

void CFmiSoundingDataServerConfigurationsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_GridControl(pDX, IDC_CUSTOM_GRID_SOUNDING_CONF, itsGridCtrl);
    DDX_Control(pDX, IDC_COMBO_SELECTED_SOUNDING_DATA_SERVER, itsServerUrlSelector);
}

BEGIN_MESSAGE_MAP(CFmiSoundingDataServerConfigurationsDlg, CDialogEx)
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_TIMER()
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_BUTTON_APPLY, &CFmiSoundingDataServerConfigurationsDlg::OnBnClickedButtonApply)
    ON_CBN_SELCHANGE(IDC_COMBO_SELECTED_SOUNDING_DATA_SERVER, &CFmiSoundingDataServerConfigurationsDlg::OnCbnSelchangeComboSelectedSoundingDataServer)
END_MESSAGE_MAP()


// CFmiSoundingDataServerConfigurationsDlg message handlers

void CFmiSoundingDataServerConfigurationsDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 550;
	lpMMI->ptMinTrackSize.y = 350;

    CDialogEx::OnGetMinMaxInfo(lpMMI);
}

void CFmiSoundingDataServerConfigurationsDlg::SetDefaultValues(void)
{
    MoveWindow(CFmiSoundingDataServerConfigurationsDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

BOOL CFmiSoundingDataServerConfigurationsDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    fDialogInitialized = true;
    DoResizerHooking(); // T‰t‰ pit‰‰ kutsua ennen kuin dialogin talletettu koko otetaan Windows rekisterist‰

	HICON hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP_2, ColorPOD(160, 160, 164));
	this->SetIcon(hIcon, FALSE);

    // Call InitHeaders before CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry !!
    InitHeaders();
    // Tee paikan asetus vasta tooltipin alustuksen j‰lkeen, niin se toimii ilman OnSize-kutsua.
	std::string errorBaseStr("Error in CFmiSoundingDataServerConfigurationsDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);
    InitSelectedServerUrlSelector();
    InitDialogTexts();
    InitGridControlValues();
    FitLastColumnOnVisibleArea();

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiSoundingDataServerConfigurationsDlg::InitSelectedServerUrlSelector()
{
    itsServerUrlSelector.ResetContent();
    for(const auto &serverUrl : itsSmartMetDocumentInterface->GetMTATempSystem().GetSoundingDataServerConfigurations().serverBaseUrls())
    {
        itsServerUrlSelector.AddString(CA2T(serverUrl.c_str()));
    }
    itsServerUrlSelector.SetCurSel(itsSmartMetDocumentInterface->GetMTATempSystem().GetSoundingDataServerConfigurations().selectedBaseUrlIndex());
}

void CFmiSoundingDataServerConfigurationsDlg::InitDialogTexts()
{
    SetWindowText(CA2T(::GetDictionaryString("Sounding data from server settings").c_str()));
    CFmiWin32Helpers::SetDialogItemText(this, IDOK, "OK");
    CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "Cancel");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_APPLY, "Apply");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_USED_SERVER_URL_STRING, "Used server Url");
}

void CFmiSoundingDataServerConfigurationsDlg::DoResizerHooking()
{
    BOOL bOk = m_resizer.Hook(this);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDOK, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDCANCEL, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_BUTTON_APPLY, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_COMBO_SELECTED_SOUNDING_DATA_SERVER, ANCHOR_HORIZONTALLY | ANCHOR_TOP);
    ASSERT(bOk == TRUE);
    // GridControl takes rest of the view
    bOk = m_resizer.SetAnchor(IDC_CUSTOM_GRID_SOUNDING_CONF, ANCHOR_HORIZONTALLY | ANCHOR_VERTICALLY);
    ASSERT(bOk == TRUE);
}

void CFmiSoundingDataServerConfigurationsDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    FitLastColumnOnVisibleArea();
}

void CFmiSoundingDataServerConfigurationsDlg::FitLastColumnOnVisibleArea()
{
    static bool firstTime = true;

    CFmiWin32Helpers::FitLastColumnOnVisibleArea(this, itsGridCtrl, firstTime, 120);
}

void CFmiSoundingDataServerConfigurationsDlg::DoOnOk(void)
{
	GetSettingsFromDialog();
	AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan p‰‰karttan‰yttˆ eli mainframe
}

void CFmiSoundingDataServerConfigurationsDlg::OnCancel()
{
	// Ei tehd‰ mit‰‰n extraa ainakaan viel‰

    CDialogEx::OnCancel();
}

void CFmiSoundingDataServerConfigurationsDlg::OnOK()
{
	DoOnOk();

    CDialogEx::OnOK();
}

void CFmiSoundingDataServerConfigurationsDlg::OnClose()
{
    // Ei tehd‰ mit‰‰n extraa ainakaan viel‰

    CDialogEx::OnClose();
}

static std::string GetColumnText(int theRow, int theColumn, const ModelDataServerConfiguration &theSoundingConf)
{
	switch(theColumn)
	{
	case SoundingConfHeaderParInfo::kRowNumber:
		return NFmiStringTools::Convert(theRow);
	case SoundingConfHeaderParInfo::kModelName:
		return theSoundingConf.producerName();
    case SoundingConfHeaderParInfo::kModerProducerId:
        return std::to_string(theSoundingConf.producerId());
    case SoundingConfHeaderParInfo::kDataNameOnServer:
        return theSoundingConf.dataNameOnServer();
    default:
		return "";
	}
}

static const COLORREF gNormalBkColor = RGB(255, 255, 255);
static const COLORREF gReadOnlyBkColor = RGB(245, 245, 245);
static const COLORREF gErrorBkColor = RGB(255, 200, 200);

void CFmiSoundingDataServerConfigurationsDlg::SetGridRow(int row, const ModelDataServerConfiguration &theSoundingConf, int theFixedColumnCount, bool updateOnly)
{
	for(int column = 0; column < static_cast<int>(itsHeaders.size()); column++)
	{
        itsGridCtrl.SetItemText(row, column, CA2T(::GetColumnText(row, column, theSoundingConf).c_str()));
		if(column >= theFixedColumnCount)
		{
            // Vain mallin nime‰ ei voi editoida
            if(column == SoundingConfHeaderParInfo::kModelName)
            {
				itsGridCtrl.SetItemState(row, column, itsGridCtrl.GetItemState(row, column) | GVIS_READONLY); // Laita read-only -bitti p‰‰lle
                itsGridCtrl.SetItemBkColour(row, column, gReadOnlyBkColor);
            }
            else
            {
				itsGridCtrl.SetItemState(row, column, itsGridCtrl.GetItemState(row, column) & ~GVIS_READONLY); // Laita read-only -bitti pois p‰‰lt‰
                itsGridCtrl.SetItemBkColour(row, column, gNormalBkColor);
            }
		}
	}
}

void CFmiSoundingDataServerConfigurationsDlg::InitGridControlValues(void)
{
	static bool fFirstTime = true;
	int fixedRowCount = 1;
	int fixedColumnCount = 1;

    auto &soundingDataConfigurations = itsSmartMetDocumentInterface->GetMTATempSystem().GetSoundingDataServerConfigurations().modelConfigurations();
    int dataRowCount = static_cast<int>(soundingDataConfigurations.size());
	int maxRowCount = fixedRowCount + dataRowCount;
	SetHeaders(itsGridCtrl, itsHeaders, maxRowCount, fFirstTime, fixedRowCount, fixedColumnCount);
    UpdateRows(fixedRowCount, fixedColumnCount, false);

    fFirstTime = false;
}

void CFmiSoundingDataServerConfigurationsDlg::UpdateRows(int fixedRowCount, int fixedColumnCount, bool updateOnly)
{
    auto &soundingDataConfigurations = itsSmartMetDocumentInterface->GetMTATempSystem().GetSoundingDataServerConfigurations().modelConfigurations();
    int currentRowCount = fixedRowCount;
	for(const auto soundingConf : soundingDataConfigurations)
	{
        SetGridRow(currentRowCount++, soundingConf, fixedColumnCount, updateOnly);
	}
}

void CFmiSoundingDataServerConfigurationsDlg::UpdateGridControlValues()
{
    UpdateRows(itsGridCtrl.GetFixedRowCount(), itsGridCtrl.GetFixedColumnCount(), true);
	UpdateData(FALSE);
	Invalidate(FALSE);
}

void CFmiSoundingDataServerConfigurationsDlg::Update()
{
    // Jos ollaan tultu cancelilla ulos, nollataan mahdolliset muutokset dialogissa alustamalla
    // m‰‰r‰tyt kontrollit uudestaan
    InitSelectedServerUrlSelector();
    InitGridControlValues();
}

BOOL CFmiSoundingDataServerConfigurationsDlg::OnEraseBkgnd(CDC* pDC)
{
//	return FALSE;

	return CDialogEx::OnEraseBkgnd(pDC);
}
/*
void CFmiSoundingDataServerConfigurationsDlg::OnPaint()
{
	if(fGridControlInitialized)
	{
		CPaintDC dc(this); // device context for painting

	 // t‰m‰ on pika viritys, kun muuten Print (ja muiden ) -nappulan kohdalta j‰‰ kaista maalaamatta kun laitoin ikkunaan v‰lkkym‰ttˆm‰n p‰ivityksen
		CBrush brush(RGB(240, 240, 240));
		CRect gridCtrlArea;
        itsGridCtrl.GetClientRect(gridCtrlArea);
		CRect clientRect;
		GetClientRect(clientRect);
		clientRect.bottom = gridCtrlArea.top;
		dc.FillRect(&clientRect, &brush);
	}
}
*/

static std::string MakeBaseGridCellErrorSting(int row, int column)
{
    std::string errorMessage = " in SoundingDataServerConfigurationsDlg on row ";
    errorMessage += std::to_string(row);
    errorMessage += ", column ";
    errorMessage += std::to_string(column);
    errorMessage += ": ";

    return errorMessage;
}

void CFmiSoundingDataServerConfigurationsDlg::GetProducerIdFromGridCtrlCell(ModelDataServerConfiguration &modelConfiguration, int row, int column)
{
    std::string prodIdString;
    try
    {
        prodIdString = CT2A(itsGridCtrl.GetItemText(row, column));
        modelConfiguration.setProducerId(std::stoi(prodIdString));
    }
    catch(std::exception &e)
    {
        itsGridCtrl.SetItemBkColour(row, column, gErrorBkColor);

        std::string errorMessage = "Illegal producerId value '";
        errorMessage += prodIdString;
        errorMessage += "'";
        errorMessage += ::MakeBaseGridCellErrorSting(row, column);
        errorMessage += e.what();
        CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Configuration, true);
    }
}

void CFmiSoundingDataServerConfigurationsDlg::GetDataNameOnServerFromGridCtrlCell(ModelDataServerConfiguration &modelConfiguration, int row, int column)
{
    std::string dataNameOnServer = CT2A(itsGridCtrl.GetItemText(row, column));
    if(!dataNameOnServer.empty())
        modelConfiguration.setDataNameOnServer(dataNameOnServer);
    else
    {
        itsGridCtrl.SetItemBkColour(row, column, gErrorBkColor);

        std::string errorMessage = "Illegal Data name on server -option";
        errorMessage += ::MakeBaseGridCellErrorSting(row, column);
        errorMessage += "cannot use empty value";
        CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Configuration, true);
    }
}

void CFmiSoundingDataServerConfigurationsDlg::GetModelConfigurationFromGridCtrlCell(ModelDataServerConfiguration &modelConfiguration, int row, int column)
{
    auto columnFunction = itsHeaders.at(column).itsColumnFunction;
    switch(columnFunction)
    {
    case SoundingConfHeaderParInfo::kModerProducerId:
    {
        GetProducerIdFromGridCtrlCell(modelConfiguration, row, column);
        break;
    }
    case SoundingConfHeaderParInfo::kDataNameOnServer:
    {
        GetDataNameOnServerFromGridCtrlCell(modelConfiguration, row, column);
        break;
    }
    default:
        break;
    }
}

void CFmiSoundingDataServerConfigurationsDlg::GetModelConfigurationsFromGridCtrlRow(ModelDataServerConfiguration &modelConfiguration, int row)
{
    for(int column = itsGridCtrl.GetFixedColumnCount(); column < itsGridCtrl.GetColumnCount(); column++)
    {
        GetModelConfigurationFromGridCtrlCell(modelConfiguration, row, column);
    }
}

void CFmiSoundingDataServerConfigurationsDlg::GetSettingsFromDialog()
{
	UpdateData(TRUE);
    auto &soundingDataConfigurations = itsSmartMetDocumentInterface->GetMTATempSystem().GetSoundingDataServerConfigurations().modelConfigurations();
    int gridCtrlRowIndex = itsGridCtrl.GetFixedRowCount();
    for(auto &modelConfiguration : soundingDataConfigurations)
    {
        GetModelConfigurationsFromGridCtrlRow(modelConfiguration, gridCtrlRowIndex++);
    }
    itsSmartMetDocumentInterface->GetMTATempSystem().GetSoundingDataServerConfigurations().setSelectedBaseUrlIndex(itsServerUrlSelector.GetCurSel());
    // P‰ivitetaan dialogi silt‰ varalta, jos painettu Apply nappia ja on ollut virheit‰
    UpdateData(FALSE);
}


void CFmiSoundingDataServerConfigurationsDlg::OnBnClickedButtonApply()
{
    GetSettingsFromDialog();
}

void CFmiSoundingDataServerConfigurationsDlg::OnCbnSelchangeComboSelectedSoundingDataServer()
{
}
