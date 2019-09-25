#include "stdafx.h"
#include "FmiParameterSelectionDlg.h"
#include "afxdialogex.h"
#include "PERSIST2.H"
#include "CloneBitmap.h"
#include "SmartMetToolboxDep_resource.h"
#include "FmiWin32TemplateHelpers.h"
#include "SmartMetDocumentInterface.h"
#include "ParameterSelectionSystem.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiMenuItem.h"
#include "SpecialDesctopIndex.h"
#include "boost\math\special_functions\round.hpp"
#include "FmiWin32Helpers.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiTimeList.h"
#include "NFmiFileSystem.h"
#include "CtrlViewFunctions.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiCrossSectionSystem.h"


static const int PARAM_ADDING_DIALOG_TOOLTIP_ID = 1234371;

// *************************************************
// NFmiParameterSelectionGridCtrl

UINT_PTR g_TitleTextUpdater = 0;
const UINT_PTR g_TitleTextUpdaterTimer = 1;
const std::string g_TitleStr = ::GetDictionaryString("Parameter Selection");

IMPLEMENT_DYNCREATE(NFmiParameterSelectionGridCtrl, CGridCtrl)

// CFmiParameterSelectionDlg message handlers
BEGIN_MESSAGE_MAP(NFmiParameterSelectionGridCtrl, CGridCtrl)
    ON_WM_LBUTTONDBLCLK()
    ON_WM_SIZE()
    ON_NOTIFY(UDM_TOOLTIP_DISPLAY, NULL, NotifyDisplayTooltip)
END_MESSAGE_MAP()

void NFmiParameterSelectionGridCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    // TODO: Add your message handler code here and/or call default

    CGridCtrl::OnLButtonDblClk(nFlags, point);

    if(itsLButtonDblClkCallback)
        itsLButtonDblClkCallback();
}

void NFmiParameterSelectionGridCtrl::OnSize(UINT nType, int cx, int cy)
{
    CGridCtrl::OnSize(nType, cx, cy);

    static bool firstTime = true;
    if(firstTime)
    {
        OnInitDialog();
        firstTime = false;
    }

    CRect rect;
    GetClientRect(rect);
    m_tooltip.SetToolRect(this, PARAM_ADDING_DIALOG_TOOLTIP_ID, rect);
}

void NFmiParameterSelectionGridCtrl::NotifyDisplayTooltip(NMHDR * pNMHDR, LRESULT * result)
{
    *result = 0;
    NM_PPTOOLTIP_DISPLAY * pNotify = (NM_PPTOOLTIP_DISPLAY*)pNMHDR;

    if(pNotify->ti->nIDTool == PARAM_ADDING_DIALOG_TOOLTIP_ID)
    {
        CPoint pt = *pNotify->pt;
        ScreenToClient(&pt);

        CString strU_;

        try
        {
            strU_ = CA2T(NFmiParameterSelectionGridCtrl::ComposeToolTipText(pt).c_str());
        }
        catch(std::exception &e)
        {
            strU_ = _TEXT("Error while making the tooltip string:\n");
            strU_ += CA2T(e.what());
        }
        catch(...)
        {
            strU_ = _TEXT("Error (unknown) while making the tooltip string");
        }

        pNotify->ti->sTooltip = strU_;

    }
}

BOOL NFmiParameterSelectionGridCtrl::PreTranslateMessage(MSG* pMsg)
{
    m_tooltip.RelayEvent(pMsg);

    return CGridCtrl::PreTranslateMessage(pMsg);
}

BOOL NFmiParameterSelectionGridCtrl::OnInitDialog()
{
    CFmiWin32Helpers::InitializeCPPTooltip(this, m_tooltip, PARAM_ADDING_DIALOG_TOOLTIP_ID);
    m_tooltip.SetMaxTipWidth(600);

    return TRUE;
}

const std::string CombineFilePath(const std::string &fileName, const std::string &fileNameFilter)
{
    try
    {
        std::size_t found = fileNameFilter.find_last_of("/\\");
        std::string filePath = found ? fileNameFilter.substr(0, found + 1) + fileName : "";
        return filePath;
    }
    catch(...)
    {
        return "";
    }
}

std::string GetFileFilter(const std::string &fileNameFilter)
{
    try
    {
        std::size_t found = fileNameFilter.find_last_of("/\\");
        std::string fileFilter = found ? fileNameFilter.substr(found + 1, fileNameFilter.length()) : "";
        return fileFilter;
    }
    catch(...)
    {
        return "";
    }
}

std::string ConvertSizeToMBorGB(unsigned long long size)
{
    double sizeInMB = static_cast<double>(size) / (1024.0 * 1000.0);
    std::stringstream fileSizeInMB;
    if(sizeInMB < 1000)
    {
        fileSizeInMB << std::fixed << std::setprecision(0) << sizeInMB;
        return fileSizeInMB.str() + " MB";
    }
    else
    {
        sizeInMB = sizeInMB / 1000;
        fileSizeInMB << std::fixed << std::showpoint << std::setprecision(2) << sizeInMB;
        return fileSizeInMB.str() + " GB";
    }
}

std::string gridSizeInKm(const NFmiGrid *grid)
{
    if(grid)
    {
        auto dataAreaWidthInKm = grid->Area()->WorldRect().Width() / 1000.;
        auto dataAreaHeightInKm = grid->Area()->WorldRect().Height() / 1000.;
        auto gridSizeXinKm = dataAreaWidthInKm / (grid->XNumber() - 1);
        auto gridSizeYinKm = dataAreaHeightInKm / (grid->YNumber() - 1);
        std::stringstream gridSizeX;
        std::stringstream gridSizeY;

        gridSizeX << std::fixed << std::setprecision(2) << gridSizeXinKm;
        gridSizeY << std::fixed << std::setprecision(2) << gridSizeYinKm;
        return gridSizeX.str() + " km x " + gridSizeY.str() + " km";
    }
    else
        return "-";
}

std::string timeSteps(boost::shared_ptr<NFmiFastQueryInfo> info)
{
    NFmiTimeList* timeList;
    NFmiTimeBag* timeBag;
    int timeSteps;
    std::string resolution;

    if(info->TimeDescriptor().ValidTimeList() != nullptr)
    {
        timeList = info->TimeDescriptor().ValidTimeList();
        timeSteps = timeList->NumberOfItems();
        resolution = "varies";
    }
    else
    {
        timeBag = info->TimeDescriptor().ValidTimeBag();
        timeSteps = timeBag->GetSize();
        resolution = std::to_string(timeBag->Resolution()) + " min";
    }

    return std::to_string(timeSteps);
}

std::string timeResolution(boost::shared_ptr<NFmiFastQueryInfo> info)
{
    NFmiTimeBag* timeBag;
    std::string resolution;

    if(info->TimeDescriptor().ValidTimeList() != nullptr)
    {
        resolution = "varies";
    }
    else
    {
        timeBag = info->TimeDescriptor().ValidTimeBag();
        resolution = std::to_string(timeBag->Resolution()) + " min";
    }

    return resolution;
}

unsigned long long fileSizeInMB(const std::string &totalFilePath)
{
    return  NFmiFileSystem::FileSize(totalFilePath);
}

unsigned long long fileSizeInMB(AddParams::SingleRowItem &singleRowItem)
{   
    return fileSizeInMB(singleRowItem.totalFilePath());
}

std::string GetParameterInterpolationMethodString(FmiInterpolationMethod method)
{
    switch(method)
    {
    case FmiInterpolationMethod::kNoneInterpolation:
        return "None";
    case FmiInterpolationMethod::kLinearly:
        return "Linear";
    case FmiInterpolationMethod::kNearestPoint:
        return "Nearest Point";
    case FmiInterpolationMethod::kByCombinedParam:
        return "Combined Parameter";
    case FmiInterpolationMethod::kLinearlyFast:
        return "Linear (Fast)";
    case FmiInterpolationMethod::kLagrange:
        return "Lagrange";
    case FmiInterpolationMethod::kNearestNonMissing:
        return "Nearest Non Missing";
    default:
        return "undefined";
    }
}

const std::string NFmiParameterSelectionGridCtrl::DataTypeString(NFmiInfoData::Type dataType)
{
    switch(dataType)
    {
    case NFmiInfoData::kNoDataType:
        return "No Data Type";
    case NFmiInfoData::kEditable:
        return "Editable";
    case NFmiInfoData::kViewable:
        return "Viewable";
    case NFmiInfoData::kStationary:
        return "Stationary";
    case NFmiInfoData::kCopyOfEdited:
        return "Copy Of Edited";
    case NFmiInfoData::kObservations:
        return "Observations";
    case NFmiInfoData::kCalculatedValue:
        return "Calculated Value";
    case NFmiInfoData::kKepaData:
        return "Official Data";
    case NFmiInfoData::kClimatologyData:
        return "Climatology Data";
    case NFmiInfoData::kAnalyzeData:
        return "Analyze Data";
    case NFmiInfoData::kScriptVariableData:
        return "Script Variable Data";
    case NFmiInfoData::kAnyData:
        return "Any Data";
    case NFmiInfoData::kSatelData:
        return "Satellite Data";
    case NFmiInfoData::kFlashData:
        return "Flash Data";
    case NFmiInfoData::kMacroParam:
        return "MacroParam";
    case NFmiInfoData::kHybridData:
        return "Hybrid Data";
    case NFmiInfoData::kFuzzyData:
        return "Fuzzy Data";
    case NFmiInfoData::kVerificationData:
        return "Verification Data";
    case NFmiInfoData::kModelHelpData:
        return "Model HelpData";
    case NFmiInfoData::kTrajectoryHistoryData:
        return "TrajectoryHistory Data";
    case NFmiInfoData::kTEMPCodeSoundingData:
        return "TEMP Code SoundingData";
    case NFmiInfoData::kCrossSectionMacroParam:
        return "CrossSection MacroParam";
    case NFmiInfoData::kEditingHelpData:
        return "Editing HelpData";
    case NFmiInfoData::kConceptualModelData:
        return "ConceptualModel Data";
    case NFmiInfoData::kSingleStationRadarData:
        return "SingleStation RadarData";
    case NFmiInfoData::kQ3MacroParam:
        return "Q3 MacroParam";
    case NFmiInfoData::kCapData:
        return "Cap Data";
    case NFmiInfoData::kWmsData:
        return "Wms Data";
    case NFmiInfoData::kSoundingParameterData:
        return "SoundingParameter Data";
    default:
        return "Special or Undefined";
    }
}

std::string NFmiParameterSelectionGridCtrl::TooltipForDataType(AddParams::SingleRowItem singleRowItem)
{
    checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> infoVector = itsSmartMetDocumentInterface->InfoOrganizer()->GetInfos(singleRowItem.uniqueDataId());
    NFmiHelpDataInfo *helpDataInfo;
    boost::shared_ptr<NFmiFastQueryInfo> info;
    std::string serverPath = "";

    if(singleRowItem.itemName() == "Editable data")
    {
        info = itsSmartMetDocumentInterface->InfoOrganizer()->FindInfo(NFmiInfoData::kEditable);
    }
    else if(infoVector.empty())
    {
        return "";
    }
    else
    {
        info = infoVector.at(0);
    }
    helpDataInfo = itsSmartMetDocumentInterface->HelpDataInfoSystem()->FindHelpDataInfo(singleRowItem.uniqueDataId());
    if(helpDataInfo != nullptr)
    {
        serverPath = CombineFilePath(info->DataFileName(), helpDataInfo->FileNameFilter());
    }

    std::string gridArea;
    std::string levels;

    gridArea = info->IsGrid() ? info->Area()->AreaStr() : "-";
    levels = (info->SizeLevels() == 1) ? "surface data" : std::to_string(info->SizeLevels());

    std::string str;
    str += "<b><font face=\"Serif\" size=\"5\" color=\"darkblue\">";
    str += "Data information";
    str += "</font></b>";
    str += "<br><hr color=darkblue><br>";
    str += "<b>Item name: </b>\t" + singleRowItem.itemName() + "\n";
    str += "<b>File filter: </b>\t" + GetFileFilter(info->DataFilePattern()) + "\n";
    str += "<b>Data type: </b>\t" + DataTypeString(info->DataType()) + "\n";
    str += "<b>Origin Time: </b>\t" + singleRowItem.origTime() + " UTC";
    str += "<br><hr color=darkblue><br>";
    str += "<b>Time info: </b>";
    str += "\tsteps: " + timeSteps(info) + "\n";
    str += "\t\t\tresolution: " + timeResolution(info) + "\n";
    str += "\t\t\trange: " + singleRowItem.origTime() + " - " + info->TimeDescriptor().LastTime().ToStr("YYYY.MM.DD HH:mm") + " UTC ";
    str += "<br><hr color=darkblue><br>";
    str += "<b>Parameters:  </b>\ttotal: " + std::to_string(info->ParamBag().GetSize()) + "\n";
    str += "<b>Levels:  </b>\t\t" + levels;
    str += "<br><hr color=darkblue><br>";
    str += "<b>Grid info: </b>\tarea: " + gridArea + "\n";
    str += "\t\t\tgrid points: " + std::to_string(info->GridXNumber()) + " x " + std::to_string(info->GridYNumber()) + "\n";
    str += "\t\t\thorizontal resolution: " + gridSizeInKm(info->Grid());
    str += "<br><hr color=darkblue><br>";
    str += "<b>File size: </b>\t\t" + ConvertSizeToMBorGB(fileSizeInMB(CombineFilePath(info->DataFileName(), info->DataFilePattern()))) + "\n";
    str += "<b>Local path: </b>\t" + CombineFilePath(info->DataFileName(), info->DataFilePattern()) + "\n";
    str += "<b>Server path: </b>\t" + serverPath;
    str += "<br><hr color=darkblue><br>";

    return str;
}

std::string NFmiParameterSelectionGridCtrl::TooltipForProducerType(AddParams::SingleRowItem singleRowItem, checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> infoVector, NFmiProducerInfo producerInfo)
{
    std::string shortName = (producerInfo.ShortNameCount() == 0) ? "" : producerInfo.ShortName();

    std::string str;
    str += "<b><font face=\"Serif\" size=\"5\" color=\"darkblue\">";
    str += "Producer information";
    str += "</font></b>";
    str += "<br><hr color=darkblue><br>";
    str += "<b>Name: </b>\t\t" + singleRowItem.itemName() + "\n";
    str += "<b>Short name: </b>\t" + shortName + "\n";
    str += "<b>Ultra short name: </b>\t" + producerInfo.UltraShortName() + "\n";
    str += "<b>Description: </b>\t" + producerInfo.Description() + "\n";
    str += "<b>Id: </b>\t\t\t" + std::to_string(singleRowItem.itemId());
    str += "<br><hr color=darkblue><br>";
    str += "<b>Data files:</b>\n";
    std::string dataFiles;
    int n = 1;
    unsigned long long combinedSize = 0;
    std::vector<std::string> colors = {"darkred", "darkblue" };

    for(auto &info : infoVector)
    {
        dataFiles += "<font color=";
        dataFiles += colors.at(n % 2);
        dataFiles += ">";
        dataFiles += "<b>" + std::to_string(n) + ".</b>";
        n++;
        auto size = fileSizeInMB(CombineFilePath(info->DataFileName(), info->DataFilePattern()));
        combinedSize += size;
        dataFiles += " Name: " + info->DataFileName() + "\n";
        dataFiles += "    File size: " + ConvertSizeToMBorGB(size) + "\n";
        dataFiles += "</font>";
    }
    str += dataFiles;
    str += "<br><hr color=darkblue><br>";
    str += "<b>Total size: </b>\t" + ConvertSizeToMBorGB(combinedSize);
    str += "<br><hr color=darkblue><br>";

    return str;
}

std::string NFmiParameterSelectionGridCtrl::TooltipForCategoryType(AddParams::SingleRowItem singleRowItem, std::vector<AddParams::SingleRowItem> singleRowItemVector, int rowNumber)
{
    int numberOfProducers = 0;
    int numberOfDataFiles = 0;
    unsigned long long combinedSize = 0;
    int depth = singleRowItem.treeDepth();
    std::vector<AddParams::SingleRowItem> subvector((singleRowItemVector.begin() + rowNumber), singleRowItemVector.end());

    for(auto item : subvector)
    {
        if(item.rowType() == AddParams::RowType::kProducerType)
            numberOfProducers++;
        if(item.rowType() == AddParams::RowType::kDataType)
        {
            numberOfDataFiles++;
            auto infoVector = itsSmartMetDocumentInterface->InfoOrganizer()->GetInfos(item.uniqueDataId());
            if(infoVector.empty())
                continue;
            auto info = infoVector.at(0);
            auto size = fileSizeInMB(CombineFilePath(info->DataFileName(), info->DataFilePattern()));
            combinedSize += size;
        }
        if(item.treeDepth() == depth) // End when one set of category data has been dealt with
            break;
    }

    std::string str;
    str += "<b><font face=\"Serif\" size=\"5\" color=\"darkblue\">";
    str += "Category information";
    str += "</font></b>";
    str += "<br><hr color=darkblue><br>";
    str += "<b>Producers: </b>\t\t" + std::to_string(numberOfProducers) + "\n";
    str += "<b>Data files: </b>\t\t" + std::to_string(numberOfDataFiles) + "\n";;
    str += "<b>Combined size: </b>\t" + ConvertSizeToMBorGB(combinedSize);
    str += "<br><hr color=darkblue><br>";

    return str;
}

std::string NFmiParameterSelectionGridCtrl::TooltipForCategoryType()
{
    std::string str;
    str += "<b><font face=\"Serif\" size=\"5\" color=\"darkblue\">";
    str += "Category information";
    str += "</font></b>";
    str += "<br><hr color=darkblue><br>";
    str += "<b>Operational data </b>";
    str += "<br><hr color=darkblue><br>";
    return str;
}

std::string NFmiParameterSelectionGridCtrl::TooltipForMacroParamCategoryType(AddParams::SingleRowItem singleRowItem, std::vector<AddParams::SingleRowItem> singleRowItemVector, int rowNumber)
{
    int numberOfParams = 0;
    int depth = singleRowItem.treeDepth();
    std::vector<AddParams::SingleRowItem> subvector((singleRowItemVector.begin() + rowNumber), singleRowItemVector.end());

    for(auto item : subvector)
    {
        if(item.leafNode())
            numberOfParams++;
        if(item.treeDepth() == depth) // End when one set of category data has been dealt with
            break;
    }

    std::string str;
    str += "<b><font face=\"Serif\" size=\"5\" color=\"darkblue\">";
    str += "Category information";
    str += "</font></b>";
    str += "<br><hr color=darkblue><br>";
    str += "<b>Number of macro parameters: </b> \t" + std::to_string(numberOfParams);
    str += "<br><hr color=darkblue><br>";

    return str;
}

std::string NFmiParameterSelectionGridCtrl::TooltipForParameterType(AddParams::SingleRowItem &rowItem)
{
    FmiInterpolationMethod interpolation = kNoneInterpolation;   
    NFmiParamBag params = itsSmartMetDocumentInterface->InfoOrganizer()->GetParams(rowItem.parentItemId()); 
    NFmiParam *param = nullptr;

    for(auto &dataIdent : params.ParamsVector())
    {
        auto id = dataIdent.GetParamIdent();
        if(id == rowItem.itemId())
        {
            param = dataIdent.GetParam();
            break;
        }
    }
    if(param == nullptr)
    {
        return "";
    }
        
    std::string paramName = std::string(param->GetName());
    std::string paramId = std::to_string(param->GetIdent());
    interpolation = param->InterpolationMethod();

    std::string str;
    str += "<b><font face=\"Serif\" size=\"5\" color=\"darkblue\">";
    str += "Parameter information";
    str += "</font></b>";
    str += "<br><hr color=darkblue><br>";
    str += "<b>Name:</b>\t\t" + paramName + "\n";
    str += "<b>Id:</b>\t\t\t" + paramId + "\n";
    str += "<b>Interpolation:</b>\t" + GetParameterInterpolationMethodString(interpolation);
    str += "<br><hr color=darkblue><br>";
    return str;
}

bool IsParameterType(AddParams::RowType rowType)
{
    if(rowType == AddParams::RowType::kParamType || rowType == AddParams::RowType::kSubParamType
        || rowType == AddParams::RowType::kLevelType || rowType == AddParams::RowType::kSubParamLevelType)
    {
        return true;
    }
    return false;
}

std::string NFmiParameterSelectionGridCtrl::ComposeToolTipText(CPoint point)
{
    CCellID idCurrentCell = GetCellFromPt(point);
    if(idCurrentCell.row >= this->GetFixedRowCount() && idCurrentCell.row < this->GetRowCount() 
        && idCurrentCell.col >= this->GetFixedColumnCount() && idCurrentCell.col < this->GetColumnCount())
    {
        int rowNumber = idCurrentCell.row;
        AddParams::SingleRowItem singleRowItem = itsSmartMetDocumentInterface->ParameterSelectionSystem().dialogRowData().at(rowNumber - 1);
        std::vector<AddParams::SingleRowItem> singleRowItemVector = itsSmartMetDocumentInterface->ParameterSelectionSystem().dialogRowData();
        auto fastQueryInfo = itsSmartMetDocumentInterface->InfoOrganizer()->GetInfos(singleRowItem.uniqueDataId());
        auto fastQueryInfoVector = itsSmartMetDocumentInterface->InfoOrganizer()->GetInfos(singleRowItem.itemId());
        auto helpDataInfo = itsSmartMetDocumentInterface->HelpDataInfoSystem()->FindHelpDataInfo(singleRowItem.uniqueDataId());
        auto producerInfo = itsSmartMetDocumentInterface->ProducerSystem().Producer(itsSmartMetDocumentInterface->ProducerSystem().FindProducerInfo(NFmiProducer(singleRowItem.itemId())));

        if(singleRowItem.rowType() == AddParams::RowType::kCategoryType && singleRowItemVector.at(rowNumber).itemId() == 998)
        {
            return TooltipForMacroParamCategoryType(singleRowItem, singleRowItemVector, rowNumber);
        }
        else if(singleRowItem.rowType() == AddParams::RowType::kCategoryType && singleRowItem.itemName() == "Operational data")
        {
            return TooltipForCategoryType();
        }
        else if(singleRowItem.rowType() == AddParams::RowType::kDataType)
        {
            return TooltipForDataType(singleRowItem);
        }
        else if(!fastQueryInfoVector.empty() && singleRowItem.rowType() == AddParams::RowType::kProducerType)
        {
            return TooltipForProducerType(singleRowItem, fastQueryInfoVector, producerInfo);
        }
        else if(!fastQueryInfoVector.empty() && singleRowItem.rowType() == AddParams::RowType::kCategoryType)
        {
            return TooltipForCategoryType(singleRowItem, singleRowItemVector, rowNumber);
        }
        else if(IsParameterType(singleRowItem.rowType()))
        {
            return TooltipForParameterType(singleRowItem);
        }
        else
            return "";
    }
    
    return std::string("Parameter Selection");
}


// *************************************************
// CFmiParameterSelectionDlg dialog

const NFmiViewPosRegistryInfo CFmiParameterSelectionDlg::s_ViewPosRegistryInfo(CRect(360, 240, 830, 760), "\\ParameterSelectionDlg");

IMPLEMENT_DYNAMIC(CFmiParameterSelectionDlg, CDialogEx)

CFmiParameterSelectionDlg::CFmiParameterSelectionDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent)
    : CDialogEx(IDD_DIALOG_PARAM_ADDING, pParent)
    , itsGridCtrl()
    , itsTreeColumn()
    , itsHeaders()
    , fDialogInitialized(false)
    , itsSmartMetDocumentInterface(smartMetDocumentInterface)
    , itsParameterSelectionSystem(&(smartMetDocumentInterface->ParameterSelectionSystem()))
    , itsSearchText(_T(""))
{

}

CFmiParameterSelectionDlg::~CFmiParameterSelectionDlg()
{
}

void CFmiParameterSelectionDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_GridControl(pDX, IDC_CUSTOM_GRID_PARAM_ADDING, itsGridCtrl);
    DDX_Text(pDX, IDC_EDIT_TEXT, itsSearchText);
}


BEGIN_MESSAGE_MAP(CFmiParameterSelectionDlg, CDialogEx)
    ON_WM_CLOSE()
    ON_WM_GETMINMAXINFO()
    ON_WM_SIZE()
    ON_WM_TIMER()
    ON_WM_ERASEBKGND()
    ON_EN_CHANGE(IDC_EDIT_TEXT, &CFmiParameterSelectionDlg::OnEnChangeEditParameterSelectionSearchText)
    ON_WM_PAINT()
END_MESSAGE_MAP()

void CFmiParameterSelectionDlg::SetDefaultValues(void)
{
    MoveWindow(CFmiParameterSelectionDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

BOOL CFmiParameterSelectionDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    fDialogInitialized = true;

    HICON hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP_2, ColorPOD(160, 160, 164));
    this->SetIcon(hIcon, FALSE);

    // Call InitHeaders before CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry !!
    InitDialogTexts();
    InitHeaders();

    // Tee paikan asetus vasta tooltipin alustuksen jälkeen, niin se toimii ilman OnSize-kutsua.
    std::string errorBaseStr("Error in CFmiCaseStudyDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);
    itsGridCtrl.SetDocument(itsSmartMetDocumentInterface);

    // Do not allow selection, it would look bad for one row being all blue
    itsGridCtrl.EnableSelection(FALSE);
    UpdateGridControlValues();
    auto LButtonDblClkCallback = [this]() {this->HandleGridCtrlsLButtonDblClk(); };
    itsGridCtrl.itsLButtonDblClkCallback = LButtonDblClkCallback;
    AdjustDialogControls();

    // Aletaan tarkastelemaan kerran sekunnissa että mikä on aktiivinen näyttö ja aktiivinen rivi ja päivitetään tarvittaessa otsikon tekstiä vastaavasti
    g_TitleTextUpdater = SetTimer(g_TitleTextUpdaterTimer, 1000, NULL);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiParameterSelectionDlg::OnCancel()
{
    DoWhenClosing();

    CDialogEx::OnCancel();
}

void CFmiParameterSelectionDlg::OnOK()
{
    DoWhenClosing();

    CDialogEx::OnOK();
}

void CFmiParameterSelectionDlg::OnClose()
{
    DoWhenClosing();

    CDialogEx::OnClose();
}

void CFmiParameterSelectionDlg::DoWhenClosing(void)
{
    AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttanäyttö eli mainframe
}

void CFmiParameterSelectionDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    lpMMI->ptMinTrackSize.x = 250;
    lpMMI->ptMinTrackSize.y = 250;

    CDialogEx::OnGetMinMaxInfo(lpMMI);
}

void CFmiParameterSelectionDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    AdjustDialogControls();
}

void CFmiParameterSelectionDlg::AdjustDialogControls(void)
{
    if(fDialogInitialized)
    {
        AdjustGridControl();
        AdjustControlWidth(IDC_EDIT_TEXT, 15, 600);
    }
}

void CFmiParameterSelectionDlg::AdjustGridControl(void)
{
    CWnd *win = GetDlgItem(IDC_CUSTOM_GRID_PARAM_ADDING);
    if(win)
    {
        CRect gridControlRect = CalcGridArea();
        win->MoveWindow(gridControlRect);
        FitNameColumnOnVisibleArea(gridControlRect.Width());
    }
}

CRect CFmiParameterSelectionDlg::CalcGridArea(void)
{
    CRect clientRect;
    GetClientRect(clientRect);
    CWnd *win = GetDlgItem(IDC_EDIT_TEXT); //This is used to get the bottom boundary
    if(win)
    {
        CRect rect2;
        win->GetWindowRect(rect2);
        CPoint pt(rect2.BottomRight());
        ScreenToClient(&pt);
        clientRect.top = clientRect.top + pt.y + 2;
    }
    return clientRect;
}

void CFmiParameterSelectionDlg::AdjustControlWidth(int theControlId, int rightOffset, int maxWidth)
{
    CWnd *win = GetDlgItem(theControlId);
    if(win)
    {
        CRect clientRect;
        GetClientRect(clientRect);

        CRect rect2;
        win->GetWindowRect(rect2);
        CPoint tl(rect2.TopLeft());
        ScreenToClient(&tl);
        CPoint br(rect2.BottomRight());
        ScreenToClient(&br);
        br.x = clientRect.right - rightOffset;
        if(br.x > maxWidth)
            br.x = maxWidth;
        CRect nameRect(tl, br);
        win->MoveWindow(nameRect);
    }
}

#ifdef max
#undef max
#endif

void CFmiParameterSelectionDlg::FitNameColumnOnVisibleArea(int gridCtrlWidth)
{
    if(itsGridCtrl.GetColumnCount())
    {
        CRect cellRect;
        itsGridCtrl.GetCellRect(0, 0, cellRect);
        int otherColumnsCombinedWidth = cellRect.Width();
        itsGridCtrl.GetCellRect(0, 2, cellRect);
        otherColumnsCombinedWidth += cellRect.Width();
        itsGridCtrl.GetCellRect(0, 3, cellRect);
        otherColumnsCombinedWidth += cellRect.Width();

        // Calculate new width for name column so that it will fill the client area
        // Total width (gridCtrlWidth) - otherColumnsCombinedWidth - some value (32) that represents the width of the vertical scroll control
        int newNameColumnWidth = gridCtrlWidth - otherColumnsCombinedWidth - 32;
        // Let's make sure that last column isn't shrinken too much
        newNameColumnWidth = std::max(newNameColumnWidth, 120);
        itsGridCtrl.SetColumnWidth(1, newNameColumnWidth);
    }
}


void CFmiParameterSelectionDlg::InitHeaders(void)
{
    int basicColumnWidthUnit = 18;
    itsHeaders.clear();
    itsHeaders.push_back(ParameterSelectionHeaderParInfo("Row", ParameterSelectionHeaderParInfo::kRowNumber, boost::math::iround(basicColumnWidthUnit * 3.5)));
    itsHeaders.push_back(ParameterSelectionHeaderParInfo("Name", ParameterSelectionHeaderParInfo::kItemName, boost::math::iround(basicColumnWidthUnit * 20.0)));
    itsHeaders.push_back(ParameterSelectionHeaderParInfo("Time", ParameterSelectionHeaderParInfo::kOrigOrLastTime, boost::math::iround(basicColumnWidthUnit * 6.5)));
    itsHeaders.push_back(ParameterSelectionHeaderParInfo("Id", ParameterSelectionHeaderParInfo::kItemId, boost::math::iround(basicColumnWidthUnit * 4.0)));
}

static const COLORREF gFixedBkColor = RGB(239, 235, 222);

static void SetHeaders(CGridCtrl &theGridCtrl, const std::vector<ParameterSelectionHeaderParInfo> &theHeaders, int rowCount, int theFixedRowCount, int theFixedColumnCount, bool &fFirstTime)
{
	int columnCount = static_cast<int>(theHeaders.size());

	if (fFirstTime) // These can be set directly only once!
	{
		theGridCtrl.SetFixedRowCount(theFixedRowCount);
		theGridCtrl.SetFixedColumnCount(theFixedColumnCount);
		theGridCtrl.SetListMode(TRUE);
		theGridCtrl.SetHeaderSort(FALSE);
	}
    theGridCtrl.SetRowCount(rowCount);
    theGridCtrl.SetColumnCount(columnCount);
    theGridCtrl.SetGridLines(GVL_BOTH);
    theGridCtrl.SetFixedBkColor(gFixedBkColor);

    int currentRow = 0;
    // 1st row is for parameter names
    for(int i = 0; i<columnCount; i++)
    {
        theGridCtrl.SetItemText(currentRow, i, CA2T(theHeaders[i].itsHeader.c_str()));
        theGridCtrl.SetItemState(currentRow, i, theGridCtrl.GetItemState(currentRow, i) | GVIS_READONLY);
        if(fFirstTime) // Adjust column width only once!
            theGridCtrl.SetColumnWidth(i, theHeaders[i].itsColumnWidth);
    }
    fFirstTime = false;
}

static bool IsTreeNodeCollapsed(CTreeColumn &treeColumnControl, int rowIndex)
{
    BOOL isPlus = FALSE;
    BOOL isMinus = FALSE;
    BOOL isLastLeaf = FALSE;

    if(treeColumnControl.TreeCellHasPlusMinus(rowIndex, &isPlus, &isMinus, &isLastLeaf))
    {
        return isPlus == TRUE;
    }
    else
        return true;
}

void CFmiParameterSelectionDlg::SetTreeNodeInformationBackToDialogRowData()
{
    auto &rowData = itsParameterSelectionSystem->dialogRowData();
    int rowIndex = itsGridCtrl.GetFixedRowCount();
    for(auto &rowItem : rowData)
    {
        rowItem.dialogTreeNodeCollapsed(::IsTreeNodeCollapsed(itsTreeColumn, rowIndex++));
    }
}

static const COLORREF gCategoryColor = RGB(250, 220, 220);
static const COLORREF gProducerColor = RGB(220, 250, 220);
//static const COLORREF gDataColor = RGB(220, 220, 250);
static const COLORREF gDataColor = RGB(200, 200, 255);
static const COLORREF gParamColor = RGB(255, 255, 255);
static const COLORREF gSubParamColor = RGB(255, 255, 245);
static const COLORREF gLevelColor1 = RGB(200, 200, 255);
static const COLORREF gLevelColor2 = RGB(170, 170, 255);
static const COLORREF gLevelColor3 = RGB(140, 140, 255);
static const COLORREF gLevelColor4 = RGB(110, 110, 255);
static const COLORREF gLevelColor5 = RGB(90, 90, 255);
static const COLORREF gLevelColor6 = RGB(60, 60, 255);
static const COLORREF gLevelColor7 = RGB(40, 40, 255);
static const COLORREF gLevelColor8 = RGB(20, 20, 255);
static const COLORREF gLevelColor9 = RGB(10, 10, 255);
static const COLORREF gLevelColor10 = RGB(0, 0, 255);
static const COLORREF gErrorColor = RGB(190, 190, 190);

static COLORREF getUsedBackgroundColor(const AddParams::SingleRowItem &theRowItem)
{
    // Params always have white background color
    if(theRowItem.leafNode()) { return gParamColor; }

//     if(!NFmiDrawParam::IsMacroParamCase(theRowItem.dataType()))
//     {
//         switch(theRowItem.rowType())
//         {
//         case AddParams::kCategoryType:
//             return gCategoryColor;
//         case AddParams::kProducerType:
//             return gProducerColor;
//         case AddParams::kDataType:
//             return gDataColor;
//         case AddParams::kParamType:
//             return gParamColor;
//         case AddParams::kSubParamType:
//         case AddParams::kSubParamLevelType:
//             return gSubParamColor;
//         case AddParams::kLevelType:
//             return gLevelColor1;
//         default:
//             return gErrorColor;
//         }
//     }

    switch(theRowItem.treeDepth())
    {
    case 1:
        return gCategoryColor;
    case 2:
        return gProducerColor;
    case 3:
        return gLevelColor1;
    case 4:
        return gLevelColor2;
    case 5:
        return gLevelColor3;
    case 6:
        return gLevelColor4;
    case 7:
        return gLevelColor5;
    case 8:
        return gLevelColor6;
    case 9:
        return gLevelColor7;
    case 10:
        return gLevelColor8;
    case 11:
        return gLevelColor9;
    case 12:
        return gLevelColor10;
    default:
        return gErrorColor;
    }
}

static std::string GetColumnText(int theRow, int theColumn, const AddParams::SingleRowItem &theRowItem)
{
    switch(theColumn)
    {
    case ParameterSelectionHeaderParInfo::kRowNumber:
        return std::to_string(theRow);
    case ParameterSelectionHeaderParInfo::kItemName:
        return theRowItem.itemName();
    case ParameterSelectionHeaderParInfo::kOrigOrLastTime:
        return theRowItem.origTime();
    case ParameterSelectionHeaderParInfo::kItemId:
    {
        if(theRowItem.itemId())
            return std::to_string(theRowItem.itemId());
        else
            return ""; // 0 is value for non interesting id (like for category id)
    }
    default:
        return "";
    }
}

void CFmiParameterSelectionDlg::SetGridRow(int row, const AddParams::SingleRowItem &theRowItem, int theFixedColumnCount)
{
    for(int column = 0; column < static_cast<int>(itsHeaders.size()); column++)
    {
        itsGridCtrl.SetItemText(row, column, CA2T(::GetColumnText(row, column, theRowItem).c_str()));
        if(column >= theFixedColumnCount)
        {
            // Laita read-only -bitti päälle
            auto state = 128 | GVIS_READONLY;
            itsGridCtrl.SetItemState(row, column, state);
            //itsGridCtrl.SetItemState(row, column, itsGridCtrl.GetItemState(row, column) | GVIS_READONLY);
            COLORREF usedBkColor = ::getUsedBackgroundColor(theRowItem);
            itsGridCtrl.SetItemBkColour(row, column, usedBkColor);
        }
    }
}

void CFmiParameterSelectionDlg::UpdateGridControlValues(bool searchRemoved)
{
    static bool fFirstTime = true;
    std::string searchText = CFmiWin32Helpers::CT2std(itsSearchText);

    if(searchRemoved)
    {
        UpdateGridControlValuesWhenSearchRemoved();
    }
    else if(fFirstTime || itsParameterSelectionSystem->dialogDataNeedsUpdate() && searchText.empty())
    {
        UpdateGridControlValuesInNormalMode(fFirstTime);
        fFirstTime = false;
    }
    else if(!searchText.empty())
        UpdateGridControlValuesWhenSearchActive();
}

void CFmiParameterSelectionDlg::UpdateGridControlValuesInNormalMode(bool fFirstTime)
{
    SetTreeNodeInformationBackToDialogRowData();
    itsParameterSelectionSystem->updateDialogData();
    int dataRowCount = static_cast<int>(itsParameterSelectionSystem->dialogRowData().size());
    int maxRowCount = fixedRowCount + dataRowCount;
    SetHeaders(itsGridCtrl, itsHeaders, maxRowCount, fixedRowCount, fixedColumnCount, fFirstTime);
    UpdateRows(fixedRowCount, fixedColumnCount, false);

    const auto &treePatternArray = itsParameterSelectionSystem->dialogTreePatternArray();
    if(treePatternArray.size()) // Test for 0 size, otherwise can crash
    {
        itsTreeColumn.TreeSetup(&itsGridCtrl, 1, static_cast<int>(treePatternArray.size()), 1, &treePatternArray[0], TRUE, FALSE);
        MakeTreeNodeCollapseSettings();
    }
}

void CFmiParameterSelectionDlg::UpdateGridControlValuesWhenSearchActive(void)
{
    const auto &treePatternArray = itsParameterSelectionSystem->dialogTreePatternArray();
    if(treePatternArray.size()) // Test for 0 size, otherwise can crash
    {
        itsTreeColumn.TreeSetup(&itsGridCtrl, 1, static_cast<int>(treePatternArray.size()), 1, &treePatternArray[0], TRUE, FALSE);
        ExpandAllNodes();
    }
    UpdateRows(fixedRowCount, fixedColumnCount, false);
}

void CFmiParameterSelectionDlg::UpdateGridControlValuesWhenSearchRemoved(void)
{
    const auto &treePatternArray = itsParameterSelectionSystem->dialogTreePatternArray();
    if(treePatternArray.size()) // Test for 0 size, otherwise can crash
    {
        itsTreeColumn.TreeSetup(&itsGridCtrl, 1, static_cast<int>(treePatternArray.size()), 1, &treePatternArray[0], TRUE, FALSE);
        CollapseAllButCategories();
    }
    UpdateRows(fixedRowCount, fixedColumnCount, false);
}

void CFmiParameterSelectionDlg::ExpandAllNodes()
{
    const auto &rowItemData = itsParameterSelectionSystem->dialogRowData();
    int currentRowCount = itsGridCtrl.GetFixedRowCount();
    for(const auto &rowItem : rowItemData)
    {
        itsTreeColumn.TreeDataExpandOneLevel(currentRowCount);
        currentRowCount++;
    }
    itsTreeColumn.TreeRefreshRows();
}

void CFmiParameterSelectionDlg::CollapseAllTreeNodes()
{
    const auto& rowItemData = itsParameterSelectionSystem->dialogRowData();
    int currentRowCount = itsGridCtrl.GetFixedRowCount();
    // Go through all top level (category) nodes and collapse them
    for(const auto& rowItem : rowItemData)
    {
        if(rowItem.rowType() == AddParams::kCategoryType)
        {
            itsTreeColumn.TreeDataCollapseAllSubLevels(currentRowCount);
        }
        currentRowCount++;
    }
}

void CFmiParameterSelectionDlg::MakeTreeNodeCollapseSettings()
{
    const auto &rowItemData = itsParameterSelectionSystem->dialogRowData();
    int currentRowCount = itsGridCtrl.GetFixedRowCount();
    // First collapse all nodes
    CollapseAllTreeNodes();
    // Then open them one by one according to settings
    for(const auto &rowItem : rowItemData)
    {
        if(!rowItem.dialogTreeNodeCollapsed())
        {
            itsTreeColumn.TreeDataExpandOneLevel(currentRowCount);
        }
        currentRowCount++;
    }
    itsTreeColumn.TreeRefreshRows();
}

void CFmiParameterSelectionDlg::CollapseAllButCategories()
{
    int currentRowCount = itsGridCtrl.GetFixedRowCount();
    // Collapse producers' sub levels
    for(auto &rowItem : itsParameterSelectionSystem->dialogRowData())
    {
        if(rowItem.rowType() == AddParams::RowType::kProducerType)
        {
            itsTreeColumn.TreeDataCollapseAllSubLevels(currentRowCount);       
        }
        currentRowCount++;
    }
    SetTreeNodeInformationBackToDialogRowData();
    itsTreeColumn.TreeRefreshRows();
}

void CFmiParameterSelectionDlg::UpdateRows(int fixedRowCount, int fixedColumnCount, bool updateOnly)
{
	std::vector<AddParams::SingleRowItem>& rowData = itsParameterSelectionSystem->dialogRowData();
    int currentRowCount = fixedRowCount;
    for(size_t i = 0; i < rowData.size(); i++)
    {
        SetGridRow(currentRowCount++, rowData[i], fixedColumnCount);
    }
}

void CFmiParameterSelectionDlg::Update()
{
    if(IsWindowVisible())
    {
        bool searchRemoved = UpdateSearchIfNeeded();
        UpdateGridControlValues(searchRemoved);
    }
}

// Returns true if search word is removed
bool CFmiParameterSelectionDlg::UpdateSearchIfNeeded()
{
    //If search word has changed, do update.
    auto searchtext = CFmiWin32Helpers::CT2std(itsSearchText);
    if(searchtext != itsPreviousSearchText)
    {   
        itsParameterSelectionSystem->searchItemsThatMatchToSearchWords(searchtext);
        itsPreviousSearchText = searchtext;
        if(searchtext.empty()) //If search text is removed, collapse all but category nodes
        {
            return true;
        }  
    }
    return false;
}

void CFmiParameterSelectionDlg::InitDialogTexts(void)
{
    SetWindowText(CA2T(g_TitleStr.c_str()));
}

void CFmiParameterSelectionDlg::HandleGridCtrlsLButtonDblClk()
{
    auto cell = itsGridCtrl.GetFocusCell();
    if(cell.IsValid())
    {
        int rowItemIndex = cell.row - itsGridCtrl.GetFixedRowCount();
        const auto &rowData = itsParameterSelectionSystem->dialogRowData();
        if(rowItemIndex >= 0 && rowItemIndex < rowData.size())
            HandleRowItemSelection(rowData[rowItemIndex]);
    }
}

void CFmiParameterSelectionDlg::HandleRowItemSelection(const AddParams::SingleRowItem &rowItem)
{    
    if((rowItem.dataType() != NFmiInfoData::kNoDataType && rowItem.leafNode()) 
		|| (itsParameterSelectionSystem->LastActivatedDesktopIndex() == CtrlViewUtils::kFmiCrossSectionView && rowItem.crossSectionLeafNode())
		)
    {
        NFmiMenuItem *addParamCommand;
        if(NFmiDrawParam::IsMacroParamCase(rowItem.dataType())) {
            addParamCommand = new NFmiMenuItem(
                static_cast<int>(itsParameterSelectionSystem->LastActivatedDesktopIndex()),
                rowItem.itemName(),
                NFmiDataIdent(NFmiParam(rowItem.itemId(), rowItem.displayName()), NFmiProducer(rowItem.parentItemId(), rowItem.parentItemName())),
                kAddViewWithRealRowNumber,
                NFmiMetEditorTypes::View::kFmiParamsDefaultView,
                rowItem.level().get(),
                rowItem.dataType());
                                                                                                                                                                                                                                                          
            addParamCommand->MacroParamInitName(rowItem.uniqueDataId());
        } 
		else if (itsParameterSelectionSystem->LastActivatedDesktopIndex() == CtrlViewUtils::kFmiCrossSectionView && rowItem.crossSectionLeafNode())
		{
			addParamCommand = new NFmiMenuItem(
				static_cast<int>(itsParameterSelectionSystem->LastActivatedDesktopIndex()),
				rowItem.itemName(),
				NFmiDataIdent(NFmiParam(rowItem.itemId(), rowItem.displayName()), NFmiProducer(rowItem.parentItemId(), rowItem.parentItemName())),
				kFmiAddParamCrossSectionView,
				NFmiMetEditorTypes::View::kFmiParamsDefaultView,
				rowItem.level().get(),
				rowItem.dataType());
		}
        else
        {
            addParamCommand = new NFmiMenuItem(
                static_cast<int>(itsParameterSelectionSystem->LastActivatedDesktopIndex()),
                "Add some param",
                NFmiDataIdent(NFmiParam(rowItem.itemId(), rowItem.displayName()), NFmiProducer(rowItem.parentItemId(), rowItem.parentItemName())),
                kAddViewWithRealRowNumber,
                NFmiMetEditorTypes::View::kFmiParamsDefaultView,
                rowItem.level().get(), 
                rowItem.dataType());
        }

        itsSmartMetDocumentInterface->ExecuteCommand(*addParamCommand, itsParameterSelectionSystem->LastActivatedRowIndex(), 0);
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("ParameterSelectionDlg: Parameter added from Parameter selection dialog", ::GetWantedMapViewIdFlag(itsParameterSelectionSystem->LastActivatedDesktopIndex()));
    }
}

std::string CFmiParameterSelectionDlg::MakeActiveViewRowText()
{
    std::string str;
    itsLastActivatedDesktopIndex = itsParameterSelectionSystem->LastActivatedDesktopIndex();
    if(itsLastActivatedDesktopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex)
    {
        str += "Active Map view ";
        str += std::to_string(itsLastActivatedDesktopIndex + 1);
    }
    else if(itsLastActivatedDesktopIndex == CtrlViewUtils::kFmiTimeSerialView)
    {
        str += "Time Series view active";
		return str;
    }
    else if(itsLastActivatedDesktopIndex == CtrlViewUtils::kFmiCrossSectionView)
    {
        str += "Cross Section view active";
    }

    str += ", active row ";
    itsLastActivatedRowIndex = itsParameterSelectionSystem->LastActivatedRowIndex();
    str += std::to_string(itsLastActivatedRowIndex);

    return str;
}

void CFmiParameterSelectionDlg::OnTimer(UINT_PTR nIDEvent)
{
    switch(nIDEvent)
    {
		case g_TitleTextUpdaterTimer:
		{
			UpdateGridControlIfNeeded();
		}
    }

    CDialogEx::OnTimer(nIDEvent);
}

void CFmiParameterSelectionDlg::UpdateGridControlIfNeeded()
{
	if (itsLastActivatedDesktopIndex != itsParameterSelectionSystem->LastActivatedDesktopIndex())
	{
		SetWindowText(CA2T(MakeTitleText().c_str()));
		itsParameterSelectionSystem->dialogDataNeedsUpdate(true);
		Update();
	}
	else if (itsLastActivatedRowIndex != itsParameterSelectionSystem->LastActivatedRowIndex())
	{
		SetWindowText(CA2T(MakeTitleText().c_str()));
	}
}

std::string CFmiParameterSelectionDlg::MakeTitleText()
{
    std::string str = g_TitleStr;
    str += " (";
    str += MakeActiveViewRowText();
    str += ")";
    return str;
}

BOOL CFmiParameterSelectionDlg::OnEraseBkgnd(CDC* pDC)
{
    return FALSE;

    //return CDialogEx::OnEraseBkgnd(pDC);
}

void CFmiParameterSelectionDlg::OnEnChangeEditParameterSelectionSearchText()
{
    UpdateData(TRUE);
    Update();
}

void CFmiParameterSelectionDlg::OnPaint()
{
    CPaintDC dc(this); 
    CBrush brush(RGB(240, 240, 240));
    CRect gridCtrlArea(CalcGridArea());
    CRect clientRect;
    GetClientRect(clientRect);
    clientRect.bottom = gridCtrlArea.top;
    dc.FillRect(&clientRect, &brush);
}

void CFmiParameterSelectionDlg::SetIndexes(unsigned int theDesktopIndex)
{
// 	auto row = itsSmartMetDocumentInterface->GetFirstRowNumber(theDesktopIndex);
	auto row = itsSmartMetDocumentInterface->CrossSectionSystem()->StartRowIndex();
	itsParameterSelectionSystem->LastActivatedDesktopIndex(theDesktopIndex);
	itsParameterSelectionSystem->LastActivatedRowIndex(row);
	SetWindowText(CA2T(MakeTitleText().c_str()));
	itsParameterSelectionSystem->dialogDataNeedsUpdate(true);
	Update();
}
