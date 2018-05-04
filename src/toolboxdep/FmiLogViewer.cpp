#include "stdafx.h"
#include "FmiLogViewer.h"
#include "CloneBitmap.h"
#include "NFmiDictionaryFunction.h"

#include <mutex> 
#include <atomic>

class SmartMetDocumentInterface; // T‰m‰ pit‰‰ forward declare:ta, jotta FmiWin32TemplateHelpers.h:in template koodit k‰‰ntyv‰t. Pit‰isi yritt‰‰ katkoa riippuvuuksia...
#include "FmiWin32TemplateHelpers.h"
#include "FmiWin32Helpers.h"

namespace 
{
    // Jos t‰m‰ on true, ei aloiteta uutta refresh timeria
    std::atomic_flag g_RefreshFlag = ATOMIC_FLAG_INIT; 
    // Kun timer laitetaan k‰yntiin kFmiRefreshTimerId:ll‰, otetaan palautuva dynaaminen timer-id talteen t‰h‰n. T‰ll‰ k‰ynniss‰ oleva timer taas tapetaan.
    UINT g_RefreshTimerDynamicId; 
}

IMPLEMENT_DYNCREATE(NFmiLogViewerGridCtrl, CGridCtrl)

BEGIN_MESSAGE_MAP(NFmiLogViewerGridCtrl, CGridCtrl)
    ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

void NFmiLogViewerGridCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
    CGridCtrl::OnRButtonUp(nFlags, point);

    // Deselect all with right mouse click
    SetSelectedRange(-1, -1, -1, -1, TRUE, TRUE);
    CCellID emptyCellId;
    SetFocusCell(emptyCellId);
}

// CFmiLogViewer dialog

const NFmiViewPosRegistryInfo CFmiLogViewer::s_ViewPosRegistryInfo(CRect(320, 130, 700, 670), "\\LogViewer");

IMPLEMENT_DYNAMIC(CFmiLogViewer, CDialogEx)

CFmiLogViewer::CFmiLogViewer(NFmiApplicationWinRegistry &theApplicationWinRegistry, CWnd* pParent)
	:CDialogEx(IDD_DIALOG_LOG_VIEWER, pParent)
    ,itsApplicationWinRegistry(theApplicationWinRegistry)
    ,itsGridCtrl()
    ,itsHeaders()
    ,itsSearchTextU_(_T(""))
    ,fFollowLatestMessage(TRUE)
{

}

CFmiLogViewer::~CFmiLogViewer()
{
}

void CFmiLogViewer::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_GridControl(pDX, IDC_CUSTOM_GRID_LOG_MESSAGES, itsGridCtrl);
    DDX_Text(pDX, IDC_EDIT_LOG_VIEWER_SEARCH_TEXT, itsSearchTextU_);
    DDX_Control(pDX, IDC_COMBO_LOG_VIEWER_CATEGORY, itsCategoryComboBox);
    DDX_Control(pDX, IDC_COMBO_LOG_VIEWER_SEVERITY, itsSeverityComboBox);
    DDX_Check(pDX, IDC_CHECK_FOLLOW_LATEST, fFollowLatestMessage);
}


BEGIN_MESSAGE_MAP(CFmiLogViewer, CDialogEx)
    ON_EN_CHANGE(IDC_EDIT_LOG_VIEWER_SEARCH_TEXT, &CFmiLogViewer::OnEnChangeEditLogViewerSearchText)
    ON_CBN_SELCHANGE(IDC_COMBO_LOG_VIEWER_CATEGORY, &CFmiLogViewer::OnCbnSelchangeComboLogViewerCategory)
    ON_CBN_SELCHANGE(IDC_COMBO_LOG_VIEWER_SEVERITY, &CFmiLogViewer::OnCbnSelchangeComboLogViewerSeverity)
    ON_WM_CLOSE()
    ON_WM_TIMER()
    ON_WM_GETMINMAXINFO()
    ON_BN_CLICKED(IDC_CHECK_FOLLOW_LATEST, &CFmiLogViewer::OnBnClickedCheckFollowLatest)
    ON_WM_SIZE()
END_MESSAGE_MAP()


// CFmiLogViewer message handlers

void CFmiLogViewer::SetDefaultValues(void)
{
    MoveWindow(CFmiLogViewer::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsApplicationWinRegistry, MakeUsedWinRegistryKeyStr(0), this);
}

void CFmiLogViewer::OnEnChangeEditLogViewerSearchText()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialog::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    UpdateData(TRUE);
    Update();
}


void CFmiLogViewer::OnCbnSelchangeComboLogViewerCategory()
{
    UpdateData(TRUE);
    itsApplicationWinRegistry.ConfigurationRelatedWinRegistry().LogViewerCategory(itsCategoryComboBox.GetCurSel());
    Update();
}


void CFmiLogViewer::OnCbnSelchangeComboLogViewerSeverity()
{
    UpdateData(TRUE);
    itsApplicationWinRegistry.ConfigurationRelatedWinRegistry().LogViewerLogLevel(itsSeverityComboBox.GetCurSel());
    Update();
}

// Call this only from OnInitDialog (gridCtrl column width are set here)
static void InitializeGridControl(CGridCtrl &theGridCtrl, const std::vector<LogMessageHeaderParInfo> &theHeaders, int theFixedRowCount, int theFixedColumnCount)
{
    static const COLORREF gFixedBkColor = RGB(239, 235, 222);
    int columnCount = static_cast<int>(theHeaders.size());
    theGridCtrl.SetColumnCount(columnCount);
    theGridCtrl.SetGridLines(GVL_BOTH);
    theGridCtrl.SetFixedRowCount(theFixedRowCount);
    theGridCtrl.SetFixedColumnCount(theFixedColumnCount);
    theGridCtrl.SetListMode(TRUE);
    theGridCtrl.SetHeaderSort(FALSE);
    theGridCtrl.SetFixedBkColor(gFixedBkColor);

    int currentRow = 0;
    // 1. on otsikko rivi on parametrien nimi‰ varten
    for(int i = 0; i<columnCount; i++)
    {
        theGridCtrl.SetItemText(currentRow, i, CA2T(theHeaders[i].itsHeader.c_str()));
        theGridCtrl.SetItemState(currentRow, i, theGridCtrl.GetItemState(currentRow, i) | GVIS_READONLY);
        theGridCtrl.SetColumnWidth(i, theHeaders[i].itsColumnWidth);
    }
}

BOOL CFmiLogViewer::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    DoResizerHooking(); // T‰t‰ pit‰‰ kutsua ennen kuin dialogin talletettu koko otetaan Windows rekisterist‰

    HICON hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP_2, ColorPOD(160, 160, 164));
    SetIcon(hIcon, FALSE);

    std::string errorBaseStr("Error in CFmiLogViewer::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsApplicationWinRegistry, this, false, errorBaseStr, 0);
    InitDialogTexts();
    InitLogLevelComboBox();
    InitCategoryComboBox();
    InitHeaders();
    InitializeGridControl(itsGridCtrl, itsHeaders, 1, 1);
    FitLastColumnOnVisibleArea();
    Update();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiLogViewer::OnOK()
{
    DoWhenClosing();

    CDialogEx::OnOK();
}


void CFmiLogViewer::OnCancel()
{
    DoWhenClosing();

    CDialogEx::OnCancel();
}


void CFmiLogViewer::OnClose()
{
    DoWhenClosing();

    CDialogEx::OnClose();
}


void CFmiLogViewer::OnTimer(UINT_PTR nIDEvent)
{
    switch(nIDEvent)
    {
    case kFmiRefreshTimerId:
    {
        KillTimer(g_RefreshTimerDynamicId);
        g_RefreshFlag.clear();
        Update();
        return;
    }
    }

    CDialogEx::OnTimer(nIDEvent);
}

void CFmiLogViewer::Update()
{
    if(IsWindowVisible() && !IsIconic()) // N‰yttˆ‰ p‰ivitet‰‰n vain jos se on n‰kyviss‰ ja se ei ole minimized tilassa
    {
        GetLogMessages();
        UpdateGridRows();
        if(fFollowLatestMessage)
            ShowLatestRowOnView();
    }
}

void CFmiLogViewer::ShowLatestRowOnView()
{
    itsGridCtrl.EnsureVisible(itsGridCtrl.GetRowCount() - 1, 2);
}

void CFmiLogViewer::StartRefreshTimer()
{
    // If atomic flag was not in set state, start update timer, otherwise do nothing
    if(!g_RefreshFlag.test_and_set(std::memory_order_acquire))
    {
        // When there is new log message coming, we want to wait about one second before we update view, so that we are not in update cyle all the time
        // because log messages may come in bunches
        g_RefreshTimerDynamicId = static_cast<UINT>(SetTimer(kFmiRefreshTimerId, static_cast<UINT>(1000), NULL));
    }
}

void CFmiLogViewer::DoWhenClosing()
{
    AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttan‰yttˆ eli mainframe
}

void CFmiLogViewer::InitDialogTexts()
{
    SetWindowText(CA2T(::GetDictionaryString("Log viewer").c_str()));
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_LOG_VIEWER_SEARCH_TEXT, "Search text");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_LOG_VIEWER_CATEGORY_TEXT, "Log category");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_LOG_VIEWER_SEVERITY_TEXT, "Min severity");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_FOLLOW_LATEST, "Follow latest");
}

void CFmiLogViewer::InitHeaders()
{
    int basicColumnWidthUnit = 16;
    itsHeaders.clear();
    itsHeaders.push_back(LogMessageHeaderParInfo("Row", LogMessageHeaderParInfo::kRowNumber, basicColumnWidthUnit * 3));
    itsHeaders.push_back(LogMessageHeaderParInfo("Time", LogMessageHeaderParInfo::kTime, basicColumnWidthUnit * 9));
    itsHeaders.push_back(LogMessageHeaderParInfo("Category", LogMessageHeaderParInfo::kCategory, basicColumnWidthUnit * 5));
    itsHeaders.push_back(LogMessageHeaderParInfo("Severity", LogMessageHeaderParInfo::kSeverity, basicColumnWidthUnit * 3));
    itsHeaders.push_back(LogMessageHeaderParInfo("Message", LogMessageHeaderParInfo::kMessage, basicColumnWidthUnit * 35));
}

void CFmiLogViewer::DoResizerHooking()
{
    BOOL bOk = m_resizer.Hook(this);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_LOG_VIEWER_SEARCH_TEXT, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_LOG_VIEWER_CATEGORY_TEXT, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_LOG_VIEWER_SEVERITY_TEXT, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_EDIT_LOG_VIEWER_SEARCH_TEXT, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_COMBO_LOG_VIEWER_CATEGORY, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_COMBO_LOG_VIEWER_SEVERITY, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_CHECK_FOLLOW_LATEST, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    // GridControl takes rest of the view
    bOk = m_resizer.SetAnchor(IDC_CUSTOM_GRID_LOG_MESSAGES, ANCHOR_HORIZONTALLY | ANCHOR_VERTICALLY);
    ASSERT(bOk == TRUE);
}

void CFmiLogViewer::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    // set the minimum tracking width and height of the window
    lpMMI->ptMinTrackSize.x = 460;
    lpMMI->ptMinTrackSize.y = 360;
}

void CFmiLogViewer::GetLogMessages()
{
    CatLog::Severity logLevel = static_cast<CatLog::Severity>(itsSeverityComboBox.GetCurSel());
    CatLog::Category category = static_cast<CatLog::Category>(itsCategoryComboBox.GetCurSel());
    itsLogMessages = CatLog::searchMessages(CFmiWin32Helpers::CT2std(itsSearchTextU_), 0, category, logLevel);
}

static const COLORREF gTraceBkColor = RGB(240, 240, 240);
static const COLORREF gDebugBkColor = RGB(230, 230, 230);
static const COLORREF gInfoBkColor = RGB(255, 255, 255);
static const COLORREF gWarningBkColor = RGB(252, 232, 135);
static const COLORREF gErrorBkColor = RGB(255, 140, 140);
static const COLORREF gCriticalBkColor = RGB(220, 185, 255);

static COLORREF getSeverityBackgroundColor(CatLog::Severity severity)
{
    switch(severity)
    {
    case CatLog::Severity::Trace:
        return gTraceBkColor;
    case CatLog::Severity::Debug:
        return gDebugBkColor;
    case CatLog::Severity::Info:
        return gInfoBkColor;
    case CatLog::Severity::Warning:
        return gWarningBkColor;
    case CatLog::Severity::Error:
        return gErrorBkColor;
    case CatLog::Severity::Critical:
        return gCriticalBkColor;
    default:
        return gInfoBkColor;
    }
}

const std::string& categoryToString(CatLog::Category category)
{
    switch(category)
    {
    case CatLog::Category::Configuration:
    {
        static const std::string str("Configuration");
        return str;
    }
    case CatLog::Category::Data:
    {
        static const std::string str("Data");
        return str;
    }
    case CatLog::Category::Editing:
    {
        static const std::string str("Editing");
        return str;
    }
    case CatLog::Category::Visualization:
    {
        static const std::string str("Visualization");
        return str;
    }
    case CatLog::Category::Macro:
    {
        static const std::string str("Macro");
        return str;
    }
    case CatLog::Category::Operational:
    {
        static const std::string str("Operational");
        return str;
    }
    case CatLog::Category::NetRequest:
    {
        static const std::string str("NetRequest");
        return str;
    }
    default:
    {
        static const std::string str("NoCategory");
        return str;
    }
    }
}

const std::string& severityToString(CatLog::Severity severity)
{
    switch(severity)
    {
    case CatLog::Severity::Trace:
    {
        static const std::string str("Trace");
        return str;
    }
    case CatLog::Severity::Debug:
    {
        static const std::string str("Debug");
        return str;
    }
    case CatLog::Severity::Info:
    {
        static const std::string str("Info");
        return str;
    }
    case CatLog::Severity::Warning:
    {
        static const std::string str("Warning");
        return str;
    }
    case CatLog::Severity::Error:
    {
        static const std::string str("Error");
        return str;
    }
    case CatLog::Severity::Critical:
    {
        static const std::string str("Critical");
        return str;
    }
    default:
    {
        static const std::string str("NoSeverity");
        return str;
    }
    }
}

static void SetGridCell(CGridCtrl &theGridCtrl, int row, int column, const std::string &str)
{ 
    theGridCtrl.SetItemText(row, column, CA2T(str.c_str()));
    theGridCtrl.SetItemState(row, column, theGridCtrl.GetItemState(row, column) | GVIS_READONLY);
}

#pragma warning (disable : 4996) // "std::localtime is unsafe" warning disabled

static std::string timePointToString(const std::chrono::system_clock::time_point &timePoint)
{
    auto timeT = std::chrono::system_clock::to_time_t(timePoint);
    std::stringstream out;
    // Outputs local time in format: YYYY-MM-DD HH:mm:SS
    out << std::put_time(std::localtime(&timeT), "%F %T");
    // Let's put milliseconds after seconds
    auto durationSinceEpochInMicroSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch());
    // Must make fill width to 3 and fill character to '0' to produce 001 ms, 011 ms, 111 ms comes without manipulations
    out << "." << std::setw(3) << std::setfill('0') << durationSinceEpochInMicroSeconds.count() % 1000; // Must us only the remainder part of milliseconds
    auto timePointString = out.str();
    return timePointString;
}

static void SetGridRow(CGridCtrl &gridCtrl, const CatLog::LogData &logMessage, const LogMessageHeaderParInfo &headerParInfo, int row, int column)
{
    switch(headerParInfo.itsColumnFunction)
    {
    case LogMessageHeaderParInfo::kRowNumber:
        ::SetGridCell(gridCtrl, row, column, std::to_string(row));
        break;
    case LogMessageHeaderParInfo::kTime:
        ::SetGridCell(gridCtrl, row, column, ::timePointToString(logMessage.logTime_));
        break;
    case LogMessageHeaderParInfo::kCategory:
        ::SetGridCell(gridCtrl, row, column, ::categoryToString(logMessage.category_));
        break;
    case LogMessageHeaderParInfo::kSeverity:
        ::SetGridCell(gridCtrl, row, column, ::severityToString(logMessage.severity_));
        break;
    case LogMessageHeaderParInfo::kMessage:
        ::SetGridCell(gridCtrl, row, column, logMessage.message_);
        break;
    }

    // Set backgroung colors depending on log message severity.
    // Fixed column BK color is fixed to dark gray.
    if(column != 0)
    {
            gridCtrl.SetItemBkColour(row, column, ::getSeverityBackgroundColor(logMessage.severity_));
    }
}

void CFmiLogViewer::UpdateGridRows()
{
    itsGridCtrl.SetRowCount(static_cast<int>(itsGridCtrl.GetFixedRowCount() + itsLogMessages.size()));

    int currentRowCounter = itsGridCtrl.GetFixedRowCount();
    for(const auto &logMessage : itsLogMessages)
    {
        for(int column = 0; column < static_cast<int>(itsHeaders.size()); column++)
        {
            SetGridRow(itsGridCtrl, *logMessage, itsHeaders[column], currentRowCounter, column);
        }
        currentRowCounter++;
    }
}

void CFmiLogViewer::OnBnClickedCheckFollowLatest()
{
    UpdateData(TRUE);
    if(fFollowLatestMessage)
        ShowLatestRowOnView();
}

void CFmiLogViewer::InitLogLevelComboBox()
{
    CFmiWin32Helpers::InitLogLevelComboBox(itsSeverityComboBox);
    itsSeverityComboBox.SetCurSel(itsApplicationWinRegistry.ConfigurationRelatedWinRegistry().LogViewerLogLevel());
}

void CFmiLogViewer::InitCategoryComboBox()
{
    CFmiWin32Helpers::InitCategoryComboBox(itsCategoryComboBox);
    itsCategoryComboBox.SetCurSel(itsApplicationWinRegistry.ConfigurationRelatedWinRegistry().LogViewerCategory());
}


void CFmiLogViewer::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    FitLastColumnOnVisibleArea();
}

#ifdef max
#undef max
#endif

void CFmiLogViewer::FitLastColumnOnVisibleArea()
{
    static bool firstTime = true;

    if(firstTime)
        firstTime = false;
    else
    {
        if(itsGridCtrl.GetColumnCount())
        {
            CRect clientRect;
            GetClientRect(clientRect);

            int lastColumnIndex = itsGridCtrl.GetColumnCount() - 1;
            CRect lastHeaderCellRect;
            itsGridCtrl.GetCellRect(0, lastColumnIndex, lastHeaderCellRect);
            // Calculate new width for last column so that it will fill the client area
            // Total width (cx) - lastColumns left edge - some value (40) that represents the width of the vertical scroll control
            int newLastColumnWidth = clientRect.Width() - lastHeaderCellRect.left - 40;
            // Let's make sure that last column isn't shrinken too much
            newLastColumnWidth = std::max(newLastColumnWidth, 220);
            itsGridCtrl.SetColumnWidth(lastColumnIndex, newLastColumnWidth);
        }
    }
}

