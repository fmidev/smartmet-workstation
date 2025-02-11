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
#include "NFmiValueString.h"
#include "WmsSupport/WmsSupport.h"
#include "WmsSupport/ChangedLayers.h"
#include "UnicodeStringConversions.h"
#include <boost/algorithm/string.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const int PARAM_ADDING_DIALOG_TOOLTIP_ID = 1234371;

// *************************************************
// NFmiParameterSelectionGridCtrl

UINT_PTR g_TitleTextUpdater = 0;
const UINT_PTR g_TitleTextUpdaterTimer = 1;
const std::string g_TitleStr = ::GetDictionaryString("Parameter Selection");

IMPLEMENT_DYNCREATE(NFmiParameterSelectionGridCtrl, CGridCtrl)

// CFmiParameterSelectionDlg message handlers
BEGIN_MESSAGE_MAP(NFmiParameterSelectionGridCtrl, CGridCtrl)
    ON_WM_RBUTTONUP()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_SIZE()
    ON_NOTIFY(UDM_TOOLTIP_DISPLAY, NULL, NotifyDisplayTooltip)
END_MESSAGE_MAP()

void NFmiParameterSelectionGridCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
    CGridCtrl::OnRButtonUp(nFlags, point);

    // Deselect all with right mouse click
    SetSelectedRange(-1, -1, -1, -1, TRUE, TRUE);
    CCellID emptyCellId;
    SetFocusCell(emptyCellId);
}

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
    CFmiWin32Helpers::InitializeCPPTooltip(this, m_tooltip, PARAM_ADDING_DIALOG_TOOLTIP_ID, 700);

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

std::string timeSteps(const boost::shared_ptr<NFmiFastQueryInfo> &info)
{
    int timeSteps = 0;

    if(info->TimeDescriptor().ValidTimeList() != nullptr)
    {
        auto timeList = info->TimeDescriptor().ValidTimeList();
        timeSteps = timeList->NumberOfItems();
    }
    else
    {
        auto timeBag = info->TimeDescriptor().ValidTimeBag();
        timeSteps = timeBag->GetSize();
    }

    return std::to_string(timeSteps);
}

std::vector<std::pair<long, double>> getResolutionInminutesAndForecastHourList(NFmiTimeList& timeList)
{
    const auto* timeFirst = timeList.Time(0);
    std::vector<std::pair<long, double>> resoAndFHourList;
    for(int index = 0; index < timeList.NumberOfItems() - 1; index++)
    {
        const auto* time1 = timeList.Time(index);
        const auto* time2 = timeList.Time(index + 1);
        long resolution = time2->DifferenceInMinutes(*time1);
        double fhour = time2->DifferenceInHours(*timeFirst);
        resoAndFHourList.push_back(std::make_pair(resolution, fhour));
    }
    return resoAndFHourList;
}

bool areResolutionsSame(const std::vector<std::pair<long, double>>& resoAndFHourList)
{
    auto firstReso = resoAndFHourList.front().first;
    for(const auto& resoFHourPair : resoAndFHourList)
    {
        if(firstReso != resoFHourPair.first)
            return false;
    }
    return true;
}

std::pair<bool,long> mostResolutionsSame(const std::vector<std::pair<long, double>>& resoAndFHourList)
{
    if(resoAndFHourList.size() < 20)
        return std::make_pair(false, 0);

    std::map<long, size_t> resolutionCounter;
    for(const auto& resoFHourPair : resoAndFHourList)
    {
        resolutionCounter[resoFHourPair.first]++;
    }
    
    size_t highCount = 0;
    long highCountReso = 0;
    for(const auto& counter : resolutionCounter)
    {
        if(highCount < counter.second)
        {
            highCount = counter.second;
            highCountReso = counter.first;
        }
    }

    double highCountPercentage = 100. * (highCount / (double)resoAndFHourList.size());
    if(highCountPercentage >= 95)
        return std::make_pair(true, highCountReso);

    return std::make_pair(false, highCountReso);
}

bool areResolutionsRisingAndSimple(const std::vector<std::pair<long, double>>& resoAndFHourList)
{
    int differentResoCounter = 1;
    for(size_t index = 0; index < resoAndFHourList.size() - 1; index++)
    {
        if(resoAndFHourList[index].first < resoAndFHourList[index + 1].first)
            differentResoCounter++;
        if(resoAndFHourList[index].first > resoAndFHourList[index + 1].first)
            return false; // resoluutio pieneni
    }
    return differentResoCounter <= 3;
}

std::string makeResolutionString(long resolutionInMinutes)
{
    if(resolutionInMinutes % 60 == 0)
    {
        return std::to_string(resolutionInMinutes/60) + "h";
    }

    return std::to_string(resolutionInMinutes) + "min";
}

std::string timeResolutionFromTimeList(NFmiTimeList& timeList)
{
    if(timeList.NumberOfItems() < 2)
    {
        return "Only 1 time step";
    }

    auto oldIndex = timeList.Index();
    auto resoAndFHourList = ::getResolutionInminutesAndForecastHourList(timeList);
    timeList.Index(oldIndex);

    if(areResolutionsSame(resoAndFHourList))
    {
        return makeResolutionString(resoAndFHourList.front().first);
    }

    auto mostCheckResult = mostResolutionsSame(resoAndFHourList);
    if(mostCheckResult.first)
    {
        return makeResolutionString(mostCheckResult.second) + " (minor variances)";
    }
    
    if(!areResolutionsRisingAndSimple(resoAndFHourList))
    {
        return " varies";
    }

    std::string resolution;
    double lastStartFHour = 0;
    for(size_t index = 0; index < resoAndFHourList.size() - 1; index++)
    {
        const auto& resoFHourPair1 = resoAndFHourList[index];
        const auto& resoFHourPair2 = resoAndFHourList[index + 1];
        if(resoFHourPair1.first != resoFHourPair2.first)
        {
            if(!resolution.empty())
                resolution += ", ";
            resolution += makeResolutionString(resoFHourPair1.first) + " (";
            resolution += NFmiValueString::GetStringWithMaxDecimalsSmartWay(lastStartFHour, 2) + "-" + NFmiValueString::GetStringWithMaxDecimalsSmartWay(resoFHourPair1.second, 2) + "h)";
            lastStartFHour = resoFHourPair1.second;
        }
    }
    resolution += ", " + makeResolutionString(resoAndFHourList.back().first) + " (";
    resolution += NFmiValueString::GetStringWithMaxDecimalsSmartWay(lastStartFHour, 2) + "-" + NFmiValueString::GetStringWithMaxDecimalsSmartWay(resoAndFHourList.back().second, 2) + "h)";
    return resolution;
}

std::string timeResolution(const boost::shared_ptr<NFmiFastQueryInfo> &info)
{
    std::string resolution;

    auto* validTimeList = info->TimeDescriptor().ValidTimeList();
    if(validTimeList)
    {
        resolution = ::timeResolutionFromTimeList(*validTimeList);
    }
    else
    {
        resolution = makeResolutionString(info->TimeDescriptor().ValidTimeBag()->Resolution());
    }

    return resolution;
}

unsigned long long fileSizeInMB(const std::string &totalFilePath)
{
    return  NFmiFileSystem::FileSize(totalFilePath);
}

unsigned long long fileSizeInMB(const AddParams::SingleRowItem &singleRowItem)
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

std::string NFmiParameterSelectionGridCtrl::DataTypeString(NFmiInfoData::Type dataType)
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

std::string NFmiParameterSelectionGridCtrl::TooltipForDataType(const AddParams::SingleRowItem &singleRowItem)
{
    auto infoVector = itsSmartMetDocumentInterface->InfoOrganizer()->GetInfos(singleRowItem.uniqueDataId());
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
    str += "\t\t\trange: " + info->TimeDescriptor().FirstTime().ToStr("YYYY.MM.DD HH:mm") + " - " + info->TimeDescriptor().LastTime().ToStr("YYYY.MM.DD HH:mm") + " UTC ";
    str += "<br><hr color=darkblue><br>";
    str += "<b>Parameters: </b>\ttotal: " + std::to_string(info->ParamBag().GetSize()) + "\n";
    str += "<b>Levels:  </b>\t\t" + levels;
    str += "<br><hr color=darkblue><br>";
    str += "<b>Grid info: </b>\tarea: " + gridArea + "\n";
    str += "\t\t\tgrid points: " + std::to_string(info->GridXNumber()) + " x " + std::to_string(info->GridYNumber()) + "\n";
    str += "\t\t\thorizontal resolution: " + gridSizeInKm(info->Grid());
    str += "<br><hr color=darkblue><br>";
    str += "<b>File size: </b>\t" + ConvertSizeToMBorGB(fileSizeInMB(CombineFilePath(info->DataFileName(), info->DataFilePattern()))) + "\n";
    str += "<b>Local path: </b>\t" + CombineFilePath(info->DataFileName(), info->DataFilePattern()) + "\n";
    str += "<b>Server path: </b>\t" + serverPath;
    str += "<br><hr color=darkblue><br>";

    return str;
}

std::string NFmiParameterSelectionGridCtrl::TooltipForProducerType(const AddParams::SingleRowItem &singleRowItem, const std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &infoVector, const NFmiProducerInfo &producerInfo)
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

std::string NFmiParameterSelectionGridCtrl::TooltipForCategoryType(const AddParams::SingleRowItem &singleRowItem, const std::vector<AddParams::SingleRowItem> &singleRowItemVector, int rowNumber)
{
    int numberOfProducers = 0;
    int numberOfDataFiles = 0;
    unsigned long long combinedSize = 0;
    int depth = singleRowItem.treeDepth();
    std::vector<AddParams::SingleRowItem> subvector((singleRowItemVector.begin() + rowNumber), singleRowItemVector.end());

    for(const auto &item : subvector)
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

std::string NFmiParameterSelectionGridCtrl::TooltipForMacroParamCategoryType(const AddParams::SingleRowItem &singleRowItem, const std::vector<AddParams::SingleRowItem> &singleRowItemVector, int rowNumber)
{
    int numberOfParams = 0;
    int depth = singleRowItem.treeDepth();
    std::vector<AddParams::SingleRowItem> subvector((singleRowItemVector.begin() + rowNumber), singleRowItemVector.end());

    for(const auto &item : subvector)
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

std::string NFmiParameterSelectionGridCtrl::TooltipForWmsDataCategoryType(const AddParams::SingleRowItem& singleRowItem, const std::vector<AddParams::SingleRowItem>& singleRowItemVector, int rowNumber)
{
    std::string str;
    str += "<b><font face=\"Serif\" size=\"5\" color=\"darkblue\">";
    str += "Wms layer info";
    str += "</font></b>";
    str += "<br><hr color=darkblue><br>";
    str += "<b>Name: </b> \t" + singleRowItem.itemName();
    str += "<br><b>Wms server: </b> \t" + singleRowItem.parentItemName();
    AddParams::SingleRowItem possibleChildLeafNode;
    try
    {
        // Katsotaan löytyykö seuraavalta riviltä currentin rivin, joka on vasta data-tasoa 
        // eikä ole leaf-node, vastaava parametri-tason leaf-node olio.
        // Huom! rowNumber alkaa 1:stä ja dialogRowData:n vector alkaa 0:sta, siksi seuraavan rivin indeksinä käytetään
        // suoraan rowNumber:ia.
        possibleChildLeafNode = itsSmartMetDocumentInterface->ParameterSelectionSystem().dialogRowData().at(rowNumber);
    }
    catch(...)
    { }

    bool currentIsLeafNode = singleRowItem.leafNode();
    bool childIsLeafNode = possibleChildLeafNode.dataType() == NFmiInfoData::kWmsData && possibleChildLeafNode.leafNode();
    if(currentIsLeafNode || childIsLeafNode)
    {
        auto useChildNodeRowItem = (!currentIsLeafNode && childIsLeafNode);
        const auto& usedSingleRowItem = useChildNodeRowItem ? possibleChildLeafNode : singleRowItem;
        str += "<br><b>Has time dimension: </b> \t" + std::string((usedSingleRowItem.wmsLayerHasTimeDimension() ? "Yes" : "No"));
        try
        {
            if(usedSingleRowItem.wmsLayerHasTimeDimension())
            {
                auto wmsSupportPtr = itsSmartMetDocumentInterface->GetCombinedMapHandlerInterface().getWmsSupport();
                NFmiDataIdent wmsLayerDataIdent(NFmiParam(usedSingleRowItem.itemId()), NFmiProducer(usedSingleRowItem.parentItemId()));
                auto timeDimensionStr = wmsSupportPtr->makeWmsLayerTimeDimensionTooltipString(wmsLayerDataIdent, false);
                if(!timeDimensionStr.empty())
                {
                    str += timeDimensionStr;
                }
            }
        }
        catch(...)
        { }
    }
    str += "<br><b>Tree depth: </b> \t" + std::to_string(singleRowItem.treeDepth());
    str += "<br><hr color=darkblue><br>";

    return str;
}

std::string NFmiParameterSelectionGridCtrl::TooltipForParameterType(const AddParams::SingleRowItem &rowItem)
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

std::string NFmiParameterSelectionGridCtrl::ComposeToolTipText(const CPoint &point)
{
    CCellID idCurrentCell = GetCellFromPt(point);
    if(idCurrentCell.row >= this->GetFixedRowCount() && idCurrentCell.row < this->GetRowCount() 
        && idCurrentCell.col >= this->GetFixedColumnCount() && idCurrentCell.col < this->GetColumnCount())
    {
        int rowNumber = idCurrentCell.row;
        AddParams::SingleRowItem singleRowItem = itsSmartMetDocumentInterface->ParameterSelectionSystem().dialogRowData().at(rowNumber - 1);
        std::vector<AddParams::SingleRowItem> singleRowItemVector = itsSmartMetDocumentInterface->ParameterSelectionSystem().dialogRowData();
        if(singleRowItem.dataType() == NFmiInfoData::kWmsData)
        {
            return TooltipForWmsDataCategoryType(singleRowItem, singleRowItemVector, rowNumber);
        }
        else
        {
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
            // Pelkälle parametrille ei enä ätehdä tooltippiä, koska siinä ei ole enää mitään uutta tietoa (interpolaatio), 
            // mutta tooltipin esille pomppaaminen häiritsee parametrin tupla-klik valintaa.
            //else if(IsParameterType(singleRowItem.rowType()))
            //{
            //    return TooltipForParameterType(singleRowItem);
            //}
            else
                return "";
        }
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
    , fTimeSerialSideParameterCase(FALSE)
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
    DDX_Check(pDX, IDC_CHECK_TIME_SERIAL_SIDE_PARAM_CASE, fTimeSerialSideParameterCase);
}


BEGIN_MESSAGE_MAP(CFmiParameterSelectionDlg, CDialogEx)
    ON_WM_CLOSE()
    ON_WM_GETMINMAXINFO()
    ON_WM_SIZE()
    ON_WM_TIMER()
END_MESSAGE_MAP()

void CFmiParameterSelectionDlg::SetDefaultValues(void)
{
    MoveWindow(CFmiParameterSelectionDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

BOOL CFmiParameterSelectionDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    DoResizerHooking(); // Tätä pitää kutsua ennen kuin dialogin talletettu koko otetaan Windows rekisteristä

    fDialogInitialized = true;
    CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
    // Call InitHeaders before CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry !!
    InitDialogTexts();
    InitHeaders();

    // Tee paikan asetus vasta tooltipin alustuksen jälkeen, niin se toimii ilman OnSize-kutsua.
    std::string errorBaseStr("Error in CFmiCaseStudyDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);
    itsGridCtrl.SetDocument(itsSmartMetDocumentInterface);

    UpdateGridControlValues(false, "");
    auto LButtonDblClkCallback = [this]() {this->HandleGridCtrlsLButtonDblClk(); };
    itsGridCtrl.itsLButtonDblClkCallback = LButtonDblClkCallback;
    AdjustDialogControls();
    DoTimeSerialSideParametersCheckboxAdjustments();

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
    lpMMI->ptMinTrackSize.x = 350;
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
    }
}

void CFmiParameterSelectionDlg::DoResizerHooking(void)
{
    BOOL bOk = m_resizer.Hook(this);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_TEXT_SEARCH, ANCHOR_TOP | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_CHECK_TIME_SERIAL_SIDE_PARAM_CASE, ANCHOR_TOP | ANCHOR_RIGHT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_EDIT_TEXT, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_CUSTOM_GRID_PARAM_ADDING, ANCHOR_VERTICALLY | ANCHOR_HORIZONTALLY);
    ASSERT(bOk == TRUE);
}

void CFmiParameterSelectionDlg::AdjustGridControl(void)
{
    CRect gridControlRect = CalcGridArea();
    FitNameColumnOnVisibleArea(gridControlRect.Width());
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

#ifdef max
#undef max
#endif

void CFmiParameterSelectionDlg::FitNameColumnOnVisibleArea(int gridCtrlWidth)
{
    if(itsGridCtrl.GetColumnCount())
    {
        int otherColumnsCombinedWidth = 0;
        for(auto columnIndex = 0; columnIndex <= itsGridCtrl.GetColumnCount(); columnIndex++)
        {
            // Skipataan Name column, jonka indeksi on siis 1
            if(columnIndex != 1)
            {
                CRect cellRect;
                itsGridCtrl.GetCellRect(0, columnIndex, cellRect);
                otherColumnsCombinedWidth += cellRect.Width();
            }
        }

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
    itsHeaders.push_back(ParameterSelectionHeaderParInfo("Name", ParameterSelectionHeaderParInfo::kItemName, boost::math::iround(basicColumnWidthUnit * 16.0)));
    itsHeaders.push_back(ParameterSelectionHeaderParInfo("Time", ParameterSelectionHeaderParInfo::kOrigOrLastTime, boost::math::iround(basicColumnWidthUnit * 6.5)));
    itsHeaders.push_back(ParameterSelectionHeaderParInfo("Id", ParameterSelectionHeaderParInfo::kItemId, boost::math::iround(basicColumnWidthUnit * 4.0)));
    itsHeaders.push_back(ParameterSelectionHeaderParInfo("Interp.", ParameterSelectionHeaderParInfo::kInterpolationType, boost::math::iround(basicColumnWidthUnit * 4.0)));
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
        auto isCollapsed = ::IsTreeNodeCollapsed(itsTreeColumn, rowIndex++);
        rowItem.dialogTreeNodeCollapsed(isCollapsed);
    }
}

static const COLORREF gCategoryColor = RGB(250, 220, 220);
static const COLORREF gProducerColor = RGB(220, 250, 220);
static const COLORREF gParamColor = RGB(255, 255, 255);
static const COLORREF gErrorColor = RGB(190, 190, 190);

COLORREF levelColor(int level)
{
	int step = 10, red = 220, green = 220, blue = 255;

	int newRed = red - (level * step);
	int newGreen = green - (level * step);
	int newBlue = blue - level;
	if (newRed < 0) newRed = 0;
	if (newGreen < 0) newGreen = 0;
	if (newBlue < 0) newBlue = 0;

	return RGB(newRed, newGreen, newBlue);
}

static COLORREF getUsedBackgroundColor(const AddParams::SingleRowItem &theRowItem)
{
    // Params always have white background color
    if(theRowItem.leafNode()) 
	{ 
		return gParamColor; 
	}
	else if (theRowItem.treeDepth() == 1) { 
		return gCategoryColor;	
	}
	else if (theRowItem.treeDepth() == 2) 
	{ 
		return gProducerColor; 
	}
	else if (theRowItem.treeDepth())
	{
		return levelColor(theRowItem.treeDepth());
	}
	else
	{
		return gErrorColor;
	}
}

static std::string GetInterpolationText(FmiInterpolationMethod interpolationMethod)
{
    switch(interpolationMethod)
    {
    case kNearestPoint:
        return ::GetDictionaryString("Nearest");
    case kLinearly:
        return ::GetDictionaryString("Linear");
    case kByCombinedParam:
        return ::GetDictionaryString("Combined");
    case kLinearlyFast:
        return ::GetDictionaryString("LinearFast");
    case kLagrange:
        return ::GetDictionaryString("Lagrange");
    case kNoneInterpolation:
        return ""; // None on default sellaisille riveille, joilla ei ole interpolaatiota (esim. data/producer/category)
    default:
    {
        std::string interpolationString = std::to_string(interpolationMethod);
        interpolationString += " (?) ";
        interpolationString += ::GetDictionaryString("interpolation");
        return interpolationString;
    }
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
    case ParameterSelectionHeaderParInfo::kInterpolationType:
    {
        return ::GetInterpolationText(theRowItem.interpolationType());
    }
    default:
        return "";
    }
}

void CFmiParameterSelectionDlg::SetGridRow(int row, const AddParams::SingleRowItem &theRowItem, int theFixedColumnCount)
{
    for(int column = 0; column < static_cast<int>(itsHeaders.size()); column++)
    {
        if(column == ParameterSelectionHeaderParInfo::kItemName)
            itsGridCtrl.SetItemText(row, column, ::convertPossibleUtf8StringToWideString(theRowItem.itemName()).c_str());
        else
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

void CFmiParameterSelectionDlg::UpdateGridControlValues(bool searchRemoved, const std::string &searchStr)
{
    static bool fFirstTime = true;

    if(searchRemoved)
    {
        UpdateGridControlValuesWhenSearchRemoved();
    }
    else if(fFirstTime || itsParameterSelectionSystem->dialogDataNeedsUpdate() && searchStr.empty())
    {
        UpdateGridControlValuesInNormalMode(fFirstTime);
        fFirstTime = false;
    }
    else if(!searchStr.empty())
    {
        UpdateGridControlValuesWhenSearchActive();
    }
    itsParameterSelectionSystem->dialogDataNeedsUpdate(false);
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

static bool IsCategoryMainLevelData(const AddParams::SingleRowItem& currentRowData, const AddParams::SingleRowItem* previousRowData)
{
    if(previousRowData)
    {
        if(currentRowData.rowType() == AddParams::RowType::kDataType)
        {
            if(previousRowData->rowType() == AddParams::RowType::kCategoryType)
            {
                // jos kategorian jälkeen tulee heti data tason rivi, on se kategoria tason data
                return true;
            }
            else if(previousRowData->rowType() == AddParams::RowType::kDataType && currentRowData.itemId() != previousRowData->itemId())
            {
                // jos data-tyypin jälkeen tulee heti data tason rivi, ja niillä on eri id:t (eri tuottajat), on se kategoria tason data
                return true;
            }
            else if(previousRowData->rowType() < AddParams::RowType::kDataType && currentRowData.itemId() != previousRowData->parentItemId())
            {
                // jos param/level/subparam -tyypin jälkeen tulee heti data tason rivi, ja niillä on eri id:t (eri tuottajat), on se kategoria tason data, previous pitää katsoa parentId:sta!
                return true;
            }
        }
    }
    return false;
}

void CFmiParameterSelectionDlg::CollapseAllButCategories()
{
    int currentRowCount = itsGridCtrl.GetFixedRowCount();
    const AddParams::SingleRowItem* previousRowData = nullptr;
    // Collapse producers' sub levels
    for(const auto &rowItem : itsParameterSelectionSystem->dialogRowData())
    {
        if(rowItem.rowType() == AddParams::RowType::kProducerType)
        {
            itsTreeColumn.TreeDataCollapseAllSubLevels(currentRowCount);       
        }
        else if(::IsCategoryMainLevelData(rowItem, previousRowData))
        { 
            // erikoistapaukset pitää hoitaa näin, on olemassa päätason datoja, joilla ei ole producer tasoa
            itsTreeColumn.TreeDataCollapseAllSubLevels(currentRowCount);
        }
        currentRowCount++;
        previousRowData = &rowItem;
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
        auto searchRemovedAndSearchStr = UpdateSearchIfNeeded();
        UpdateGridControlValues(searchRemovedAndSearchStr.first, searchRemovedAndSearchStr.second);
    }
}

void CFmiParameterSelectionDlg::DoTimeSerialSideParametersCheckboxAdjustments()
{
    CWnd* checkboxControl = GetDlgItem(IDC_CHECK_TIME_SERIAL_SIDE_PARAM_CASE);
    if(checkboxControl)
    {
        if(itsLastActivatedDesktopIndex == CtrlViewUtils::kFmiTimeSerialView)
            checkboxControl->EnableWindow(TRUE);
        else
            checkboxControl->EnableWindow(FALSE);
    }
    UpdateData(FALSE);
}

// Returns true if search word is removed and current cleaned search string
std::pair<bool, std::string> CFmiParameterSelectionDlg::UpdateSearchIfNeeded()
{
    //If search word has changed, do update.
    auto searchtext = CFmiWin32Helpers::CT2std(itsSearchText);
    // Let's trim all white space away from start and end of input string
    boost::trim(searchtext);
    if(searchtext != itsPreviousSearchText)
    {   
        itsParameterSelectionSystem->searchItemsThatMatchToSearchWords(searchtext);
        itsPreviousSearchText = searchtext;
        if(searchtext.empty()) //If search text is removed, collapse all but category nodes
        {
            return std::make_pair(true, searchtext);
        }  
    }
    return std::make_pair(false, searchtext);
}

void CFmiParameterSelectionDlg::InitDialogTexts(void)
{
    SetWindowText(CA2T(g_TitleStr.c_str()));
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEXT_SEARCH, "Search text\n(Press Enter!)");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_TIME_SERIAL_SIDE_PARAM_CASE, "Side params");
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

static NFmiInfoData::Type GetFinalMacroParamDataType(unsigned int desktopIndex, NFmiInfoData::Type originalDataType)
{
    if(originalDataType == NFmiInfoData::kMacroParam)
    {
        if(desktopIndex == CtrlViewUtils::kFmiCrossSectionView)
            return NFmiInfoData::kCrossSectionMacroParam;
        if(desktopIndex == CtrlViewUtils::kFmiTimeSerialView)
            return NFmiInfoData::kTimeSerialMacroParam;
    }
    return originalDataType;
}

void CFmiParameterSelectionDlg::HandleRowItemSelection(const AddParams::SingleRowItem &rowItem)
{    
    UpdateData(TRUE);

    auto normalLeafNode = (rowItem.dataType() != NFmiInfoData::kNoDataType && rowItem.leafNode());
    auto crossSectionLeafNode = (itsParameterSelectionSystem->LastActivatedDesktopIndex() == CtrlViewUtils::kFmiCrossSectionView &&
        rowItem.crossSectionLeafNode());
    if((normalLeafNode || crossSectionLeafNode))
    {
        std::unique_ptr<NFmiMenuItem> addParamCommandPtr(new NFmiMenuItem(
            static_cast<int>(itsParameterSelectionSystem->LastActivatedDesktopIndex()),
            rowItem.itemName(),
            NFmiDataIdent(NFmiParam(rowItem.itemId(), rowItem.displayName()), NFmiProducer(rowItem.parentItemId(), rowItem.parentItemName())),
            kAddViewWithRealRowNumber,
            NFmiMetEditorTypes::View::kFmiParamsDefaultView,
            rowItem.level().get(),
            rowItem.dataType()));

        if(NFmiDrawParam::IsMacroParamCase(rowItem.dataType()))
        {
            // Asetetaan oikea datatyyppi
            addParamCommandPtr->DataType(::GetFinalMacroParamDataType(itsParameterSelectionSystem->LastActivatedDesktopIndex(), rowItem.dataType()));
            // Asetetaan macroParamin init tiedoston nimi
            addParamCommandPtr->MacroParamInitName(rowItem.uniqueDataId());
        }

        if(crossSectionLeafNode)
        {
            addParamCommandPtr->CommandType(kFmiAddParamCrossSectionView);
        }
        else if(itsParameterSelectionSystem->LastActivatedDesktopIndex() == CtrlViewUtils::kFmiTimeSerialView)
        {
            auto usedParamAddingCommand = kFmiAddTimeSerialView;
            if(fTimeSerialSideParameterCase)
            {
                usedParamAddingCommand = kFmiAddTimeSerialSideParam;
            }
            addParamCommandPtr->CommandType(usedParamAddingCommand);
        }

        itsSmartMetDocumentInterface->ExecuteCommand(*addParamCommandPtr, itsParameterSelectionSystem->LastActivatedRowIndex(), 0);
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
	if (itsParameterSelectionSystem->dialogDataNeedsUpdate())
	{
        // Huom! HasViewTypeChanged pitää kutsua ennen MakeTitleText kutsua, joka asettaa em. metodissa tarkasteltuja muuttujia
        bool viewDesktopIndexChangeRequiresDialogUpdate = HasViewTypeChanged();
		SetWindowText(CA2T(MakeTitleText().c_str()));
        // DoTimeSerialSideParametersCheckboxAdjustments metodia on kutsuttava vasta MakeTitleText metodi kutsun jälkeen
        DoTimeSerialSideParametersCheckboxAdjustments();
        if(viewDesktopIndexChangeRequiresDialogUpdate)
        {
    		Update();
        }
	}
	else if (itsLastActivatedRowIndex != itsParameterSelectionSystem->LastActivatedRowIndex())
	{
		SetWindowText(CA2T(MakeTitleText().c_str()));
	}
}

// Palautetaan true, jos molemmat annetut indeksit ovat karttanäyttö tyyppisiä
bool BothViewIndexWereMapViewType(unsigned int desktopIndex1, unsigned int desktopIndex2)
{
    return (desktopIndex1 <= CtrlViewUtils::kFmiMaxMapDescTopIndex) && (desktopIndex2 <= CtrlViewUtils::kFmiMaxMapDescTopIndex);
}

bool CFmiParameterSelectionDlg::HasViewTypeChanged()
{
    if(itsLastActivatedDesktopIndex != itsParameterSelectionSystem->LastActivatedDesktopIndex())
    {
        return !::BothViewIndexWereMapViewType(itsLastActivatedDesktopIndex, itsParameterSelectionSystem->LastActivatedDesktopIndex());
    }
    return false;
}

std::string CFmiParameterSelectionDlg::MakeTitleText()
{
    std::string str = g_TitleStr;
    str += " (";
    str += MakeActiveViewRowText();
    str += ")";
    return str;
}

void CFmiParameterSelectionDlg::UpdateAfterSearchText()
{
    UpdateData(TRUE);
    Update();
}

// Kun dialogi avataan '+' -napista, kutsutaan tätä tehdään kyseisen näytön asetukset kuntoon,
// jotta parametrit lisätään sen näytön aktiiviseen riviin.
void CFmiParameterSelectionDlg::SetIndexes(unsigned int theDesktopIndex)
{
    int absoluteActiveRow = 1;
    if(theDesktopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex)
    {
        // Karttanäytöillä on omat aktiivisten rivien muistit, joten käytetään niitä
        absoluteActiveRow = itsSmartMetDocumentInterface->MapViewDescTop(theDesktopIndex)->AbsoluteActiveViewRow();
    }
    else
    {
        //  Muille näytöille ParameterSelectionSystem saa toimia aktiivisten rivien muistina
        absoluteActiveRow = itsParameterSelectionSystem->GetLastActivatedRowIndexFromWantedDesktop(theDesktopIndex);
    }
    itsParameterSelectionSystem->SetLastActiveIndexes(theDesktopIndex, absoluteActiveRow);
    UpdateGridControlIfNeeded();
}

BOOL CFmiParameterSelectionDlg::PreTranslateMessage(MSG* pMsg)
{
    // Erikoiskäsittely, jos kyse RETURN napin painalluksesta (alas/ylös)
    if(CtrlView::DoReturnKeyOperation(pMsg, [this](){this->UpdateAfterSearchText(); }))
        return TRUE; // Palautetaan true, jotta tätä messagea ei käsitellä enää muualla

    return CDialogEx::PreTranslateMessage(pMsg);
}
