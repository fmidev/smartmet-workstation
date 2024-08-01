#include "stdafx.h"
#include "FmiCaseStudyDlg.h"
#include "FmiWin32TemplateHelpers.h"
#include "SmartMetDocumentInterface.h"
#include "CloneBitmap.h"
#include "SmartMetToolboxDep_resource.h"
#include "GridCellCheck.h"
#include "NFmiCaseStudySystem.h"
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "NFmiFileString.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiApplicationDataBase.h"
#include "NFmiFileSystem.h"
#include "FmiWin32Helpers.h"
#include "persist2.h"
#include "HakeMessage/Main.h"
#include "NFmiValueString.h"
#include "CtrlViewFunctions.h"
#include "NFmiGdiPlusImageMapHandler.h"
#include "ColorStringFunctions.h"
#include "NFmiBetaProductHelperFunctions.h"

#include <boost/math/special_functions/round.hpp>
#include <boost/algorithm/string.hpp>
#include "execute-command-in-separate-process.h"
#include "QueryDataToLocalCacheLoaderThread.h"

namespace
{
	const int CASE_STUDY_GRID_CONTROL_TOOLTIP_ID = 1234383;
	const int CASE_STUDY_DIALOG_TOOLTIP_ID = 1234384;

	const COLORREF gFixedBkColor = RGB(239, 235, 222);
	const double gMegaByteSize = 1024 * 1024;

	const std::string gEditEnableDataCheckControlOffStr = "Edit Enable Data";
	const std::string gEditEnableDataCheckControlOnStr = "Edit Enable Data (MUST restart SmartMet to take changes in use)";

	const NFmiApplicationDataBase::AppSpyData gCaseStudyMakerData("CaseStudyExe.exe", false); // false = ei olla kiinnostuneita ohjelman versiosta

	const std::string g_GridCtrlHeaderName1 = "Row";
	const std::string g_GridCtrlHeaderName2 = "Name";
	const std::string g_GridCtrlHeaderName3 = "ProdId";
	const std::string g_GridCtrlHeaderName4 = "Store";
	const std::string g_GridCtrlHeaderName5 = "CS [h] Offset-Range";
	const std::string g_GridCtrlHeaderName6 = "LC CNT";
	const std::string g_GridCtrlHeaderName7 = "Enable Data";
	const std::string g_GridCtrlHeaderName8 = "File(s) size [MB]";

	std::string g_GridCtrlHeaderNameExplanation3 = "(= Producer Id)";
	std::string g_GridCtrlHeaderNameExplanation4 = "(= store data to Case-study package)";
	std::string g_GridCtrlHeaderNameExplanation5 = "(= Case Study model data stored origin-time offset range in hours)";
	std::string g_GridCtrlHeaderNameExplanation6 = "(= Local Cache data file Count)";
	std::string g_GridCtrlHeaderNameExplanation7 = "(= is this data used by SmartMet at all)";
	std::string g_GridCtrlHeaderNameExplanation7_2 = "(= is this data used by SmartMet at all, now hidden)";
	std::string g_GridCtrlHeaderNameExplanation8 = "(= estimate of total data size in stored Case-study)";

	std::string GetProducerIdString(const NFmiCaseStudyDataFile& rowData)
	{
		if(rowData.CategoryHeader())
			return "-";
		else
			return std::to_string(rowData.Producer().GetIdent());
	}

	std::string GetRowExplanationString(const NFmiCaseStudyDataFile& rowData)
	{
		if(rowData.CategoryHeader())
			return "(= Category line)";
		else if(rowData.ProducerHeader())
			return "(= Producer line)";
		else
			return "(= Data file line)";
	}

	double ConvertToMB(double theFileSizeInBytes)
	{
		return theFileSizeInBytes / gMegaByteSize;
	}

	std::string GetMegaByteSizeString(double theSizeInMB)
	{
		double rounder = 1;
		if(theSizeInMB < 1)
			rounder = 0.01;
		else if(theSizeInMB < 10)
			rounder = 0.1;

		theSizeInMB = ::round(theSizeInMB / rounder) * rounder;
		if(theSizeInMB > 20)
			return std::to_string(boost::math::iround(theSizeInMB));
		else if(theSizeInMB > 2)
			return std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(theSizeInMB, 1));
		else if(theSizeInMB > 0.2)
			return std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(theSizeInMB, 2));
		else
			return std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(theSizeInMB, 3));
	}

	bool IsHeaderRow(const NFmiCaseStudyDataFile& theCaseData)
	{
		if(theCaseData.CategoryHeader() == true || theCaseData.ProducerHeader() == true)
			return true;
		else
			return false;
	}

	// Sizi column string on seuraavanlainen:
	// 1. Category/producer -headerit:
	// Tot=xx.x Max=yy.y
	// 2. normaali data:
	// Tot=xx.x 1=yy.y
	// 3. normaali data jos store=false:
	// (Tot=xx.x 1=yy.y)
	// 4. normaali data, jossa vain viim. data talletetaan:
	// 1=yy.y
	// 5. normaali data, jossa vain viim. data talletetaan, mutta store=false:
	// (1=yy.y)
	std::string GetSizeColumnString(const NFmiCaseStudyDataFile& theCaseData)
	{
		size_t tabLineCount = 8; // yritin laittaa tabulaattorin stringiin joka annetaan hila ruutuun, mutta se ignoorataan jotenkin
							  // joten yritän itse laskea mistä kohtaa alkaa sulku lause, niin että ne alkaisivat tasaisesti taulukossa
		std::string str;
		if(::IsHeaderRow(theCaseData))
		{
			str += "Tot=";
			str += ::GetMegaByteSizeString(::ConvertToMB(theCaseData.TotalFileSize()));
			str += "   Max=";
			str += ::GetMegaByteSizeString(::ConvertToMB(theCaseData.MaxFileSize()));
		}
		else
		{
			bool dontStoreData = (theCaseData.DataFileWinRegValues().Store() == false); // huom! sulut tulee vain jos ollaan tavallisen datan kohdassa, ei headerien
			if(dontStoreData)
				str += "(";
			if(theCaseData.StoreLastDataOnly())
			{
				str += "1=";
				str += ::GetMegaByteSizeString(::ConvertToMB(theCaseData.SingleFileSize()));
			}
			else
			{
				str += "Tot=";
				str += ::GetMegaByteSizeString(::ConvertToMB(theCaseData.TotalFileSize()));
				str += "   1=";
				str += ::GetMegaByteSizeString(::ConvertToMB(theCaseData.SingleFileSize()));
			}

			if(dontStoreData)
				str += ")";
		}
		return str;
	}

	std::string GetColoredBoldTextForTooltip(const std::string& text, bool useBold, bool useColor, const NFmiColor& color)
	{
		std::string str;
		if(useBold)
			str += "<b>";
		if(useColor)
		{
			str += "<font color=";
			str += ColorString::Color2HtmlColorStr(color);
			str += ">";
		}
		str += text;
		if(useColor)
			str += "</font>";
		if(useBold)
			str += "</b>";
		return str;
	}

} // nameless namespace

IMPLEMENT_DYNCREATE(NFmiCaseStudyGridCtrl, CGridCtrl)

BEGIN_MESSAGE_MAP(NFmiCaseStudyGridCtrl, CGridCtrl)
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_NOTIFY(UDM_TOOLTIP_DISPLAY, NULL, NotifyDisplayTooltip)
END_MESSAGE_MAP()

BOOL NFmiCaseStudyGridCtrl::OnInitDialog()
{
	CFmiWin32Helpers::InitializeCPPTooltip(this, m_grid_control_tooltip, CASE_STUDY_GRID_CONTROL_TOOLTIP_ID, 800);
	// Kuinka nopeasti tooltip ilmestyy näkyviin, jos kursoria ei liikuteta [ms],
	// Laitetaan tähän defaulttia 0.5 s isompi viive eli 1 s.
	m_grid_control_tooltip.SetDelayTime(PPTOOLTIP_TIME_INITIAL, 1000);

	return TRUE;
}

void NFmiCaseStudyGridCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	CGridCtrl::OnRButtonUp(nFlags, point);

	// Tarkoitus on deselectoida kaikki
	SetSelectedRange(-1,-1,-1,-1, TRUE, TRUE);
}

BOOL NFmiCaseStudyGridCtrl::PreTranslateMessage(MSG* pMsg)
{
	m_grid_control_tooltip.RelayEvent(pMsg);

	return CGridCtrl::PreTranslateMessage(pMsg);
}

void NFmiCaseStudyGridCtrl::OnSize(UINT nType, int cx, int cy)
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
	m_grid_control_tooltip.SetToolRect(this, CASE_STUDY_GRID_CONTROL_TOOLTIP_ID, rect);
}

void NFmiCaseStudyGridCtrl::NotifyDisplayTooltip(NMHDR* pNMHDR, LRESULT* result)
{
	*result = 0;
	NM_PPTOOLTIP_DISPLAY* pNotify = (NM_PPTOOLTIP_DISPLAY*)pNMHDR;

	if(pNotify->ti->nIDTool == CASE_STUDY_GRID_CONTROL_TOOLTIP_ID)
	{
		CPoint pt = *pNotify->pt;
		ScreenToClient(&pt);

		CString strU_;

		try
		{
			strU_ = CA2T(ComposeToolTipText(pt).c_str());
		}
		catch(std::exception& e)
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

bool NFmiCaseStudyGridCtrl::IsEnableDataColumnVisible() const
{
	return GetColumnWidth(CaseStudyHeaderParInfo::kEnableData) > 0;
}

static NFmiColor GetStoredDataStatusColor(bool storeData)
{
	const NFmiColor storedFileColor(0, 0.7f, 0);
	const NFmiColor nonStoredFileColor(0.95f, 0, 0);

	return storeData ? storedFileColor : nonStoredFileColor;
}

static std::string MakeFinalFilePathTooltipText(const std::string& finalFilePath, int index, bool storeData)
{
	auto filePathColor = ::GetStoredDataStatusColor(storeData);
	std::string str = "<br>";
	str += std::to_string(index);
	str += ".  ";
	str += finalFilePath;
	return ::GetColoredBoldTextForTooltip(str, true, true, filePathColor);
}

static std::string MakeModelRunText(const std::string& dataFilePath, bool isModelRunStored)
{
	try
	{
		NFmiQueryData data(dataFilePath);
		std::string modelRunTimeString = data.Info()->OriginTime().ToStr("YYYY.MM.DD HH:mm Utc", kEnglish);
		std::string str = "<br>    ";
		// Checked symboli 'V' tai crossed-box 'X', utf-8 symbolit eivät näytä toimivan tooltip html piirto systeemin kanssa
		str += isModelRunStored ? "On " : "Off"; // "&#x2611;" : "&#x2612;";
		str += ": model-run-time  ";
		str += modelRunTimeString;
		auto textColor = ::GetStoredDataStatusColor(isModelRunStored);
		return ::GetColoredBoldTextForTooltip(str, true, true, textColor);
	}
	catch(...)
	{ }
	return ::GetColoredBoldTextForTooltip("<br>Unknown error when opening data file", true, true, NFmiColor(1,0,0));
}

std::string MakeOffsetTimeString(const NFmiMetTime& offsetTime)
{
	return std::string(offsetTime.ToStr("MM.DD Www HH:mm", kEnglish));
}

std::string MakeRangeOffsetString(const NFmiCaseStudyDataFile& dataFile, const NFmiMetTime& usedWallClockTime)
{
	const auto& offsetRange = dataFile.DataFileWinRegValues().CaseStudyModelDataOffsetRangeInHours();
	std::string rangeStr = "(";
	auto offsetTime1(usedWallClockTime);
	offsetTime1.ChangeByMinutes(-(offsetRange.first * 60));
	rangeStr += ::MakeOffsetTimeString(offsetTime1);
	rangeStr += " - ";
	auto offsetTime2(usedWallClockTime);
	offsetTime2.ChangeByMinutes(-(offsetRange.second * 60));
	rangeStr += ::MakeOffsetTimeString(offsetTime2);
	rangeStr += " utc)";
	return rangeStr;
}


std::string MakeObsRangeOffsetString(const NFmiCaseStudyDataFile& dataFile, const NFmiMetTime& usedWallClockTime)
{
	const auto& offsetRange = dataFile.DataFileWinRegValues().CaseStudyModelDataOffsetRangeInHours();
	if(offsetRange == gLatestDataOnlyRangeInHours)
	{
		return "(latest data only)";
	}

	return MakeRangeOffsetString(dataFile, usedWallClockTime);
}

// Oletuksia:
// 1. timeSortedFiles ei ole tyhjä
// 2. datafile ei ole producer/category header
// 3. Kyse on model-datasta, jolla voidaan tallettaa useita eri malliajoja
static std::string MakeModelDataStoredFileListText(const NFmiCaseStudyDataFile& dataFile, const CaseStudyMatchingFiles &matchingFilePaths, const NFmiMetTime& usedWallClockTime)
{
	std::string str = "<br><br>";
	if(!dataFile.DataFileWinRegValues().Store())
	{
		str += ::GetColoredBoldTextForTooltip("None of the following data files will be stored (Store option off) ", true, false, NFmiColor());
		// Asetetaan vielä kaikki tiedostot false tilaan, eli mitään niistä ei ole tarkoitus tallettaa
		for(auto matchingFilePath : matchingFilePaths)
		{
			matchingFilePath.second = false;
		}
	}
	else if(dataFile.StoreLastDataOnly())
	{
		str += ::GetColoredBoldTextForTooltip("Following data files are available but only one to be stored ", true, false, NFmiColor());
		std::string rangeStr = ::MakeObsRangeOffsetString(dataFile, usedWallClockTime);
		str += ::GetColoredBoldTextForTooltip(rangeStr, true, false, NFmiColor());
	}
	else
	{
		str += ::GetColoredBoldTextForTooltip("Following model-run data files are available to be stored ", true, false, NFmiColor());
		std::string rangeStr = ::MakeRangeOffsetString(dataFile, usedWallClockTime);
		str += ::GetColoredBoldTextForTooltip(rangeStr, true, false, NFmiColor());
	}
	int index = 1;
	for(const auto& filePath : matchingFilePaths)
	{
		str += ::MakeFinalFilePathTooltipText(filePath.first, index, filePath.second);
		if(!dataFile.StoreLastDataOnly())
		{
			str += ::MakeModelRunText(filePath.first, filePath.second);
		}
		index++;
	}
	return str;
}

static std::string AddPossibleSelectedModelRunTexts(const NFmiCaseStudyDataFile& dataFile)
{
	std::string str;
	if(!dataFile.CategoryHeader() && !dataFile.ProducerHeader())
	{
		if(dataFile.Category() == NFmiCaseStudyDataCategory::SatelImage)
		{
			str += "<br><br>";
			str += ::GetColoredBoldTextForTooltip("Satellite image files are ignored in tooltip", true, false, NFmiColor());
		}
		else if(!dataFile.DataEnabled())
		{
			str += "<br><br>";
			str += ::GetColoredBoldTextForTooltip("This data is disabled and files are ignored in tooltip", true, false, NFmiColor());
		}
		else
		{
			NFmiMetTime usedWallClockTime(1);
			auto storedFilePaths = NFmiCaseStudyDataFile::GetTimeOffsetMatchingFileList(dataFile, usedWallClockTime, true);
			if(storedFilePaths.empty())
			{
				str += "<br><br>";
				str += ::GetColoredBoldTextForTooltip("No data files were found with file filter:<br>", true, false, NFmiColor());
				str += ::GetColoredBoldTextForTooltip(dataFile.FileFilter(), true, false, NFmiColor());
			}
			else
			{
				str += ::MakeModelDataStoredFileListText(dataFile, storedFilePaths, usedWallClockTime);
			}
		}
	}
	return str;
}

std::string NFmiCaseStudyGridCtrl::ComposeToolTipText(const CPoint& point)
{
	std::string str;
	CCellID idCurrentCell = GetCellFromPt(point);
	if(idCurrentCell.row >= this->GetFixedRowCount() && idCurrentCell.row < this->GetRowCount()
		&& idCurrentCell.col >= this->GetFixedColumnCount() && idCurrentCell.col < this->GetColumnCount())
	{
		const auto& caseStudyDialogData = SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation()->CaseStudySystem().CaseStudyDialogData();
		auto actualRowIndex = idCurrentCell.row - GetFixedRowCount();
		if(actualRowIndex < caseStudyDialogData.size())
		{
			NFmiColor headerNameColor(0, 0.67f, 0);
			NFmiColor explanationTextColor(1, 0, 0);
			NFmiColor valueTextColor(0, 0, 1);
			const auto& rowData = caseStudyDialogData[actualRowIndex];

			str += ::GetColoredBoldTextForTooltip(g_GridCtrlHeaderName1, true, true, headerNameColor) + ": ";
			str += ::GetColoredBoldTextForTooltip(std::to_string(idCurrentCell.row), true, true, valueTextColor);
			str += "<br>";
			str += ::GetColoredBoldTextForTooltip(g_GridCtrlHeaderName2, true, true, headerNameColor) + " ";
			str += ::GetColoredBoldTextForTooltip(::GetRowExplanationString(*rowData), true, true, explanationTextColor) + ": ";
			str += ::GetColoredBoldTextForTooltip(rowData->Name(), true, true, valueTextColor);
			str += "<br><hr color=darkblue><br>";
			str += ::GetColoredBoldTextForTooltip(g_GridCtrlHeaderName3, true, true, headerNameColor) + " ";
			str += ::GetColoredBoldTextForTooltip(g_GridCtrlHeaderNameExplanation3, true, true, explanationTextColor) + ": ";
			str += ::GetColoredBoldTextForTooltip(GetProducerIdString(*rowData), true, true, valueTextColor);
			str += "<br>";
			str += ::GetColoredBoldTextForTooltip(g_GridCtrlHeaderName4, true, true, headerNameColor) + " ";
			str += ::GetColoredBoldTextForTooltip(g_GridCtrlHeaderNameExplanation4, true, true, explanationTextColor) + ": ";
			str += ::GetColoredBoldTextForTooltip(rowData->DataFileWinRegValues().Store() ? "On" : "Off", true, true, valueTextColor);
			str += "<br>";
			str += ::GetColoredBoldTextForTooltip(g_GridCtrlHeaderName5, true, true, headerNameColor) + " ";
			str += ::GetColoredBoldTextForTooltip(g_GridCtrlHeaderNameExplanation5, true, true, explanationTextColor) + ": ";
			str += ::GetColoredBoldTextForTooltip(NFmiCaseStudySystem::MakeModelDataOffsetRangeInHoursString(rowData->DataFileWinRegValues().CaseStudyModelDataOffsetRangeInHours()), true, true, valueTextColor);
			str += "<br>";
			str += ::GetColoredBoldTextForTooltip(g_GridCtrlHeaderName6, true, true, headerNameColor) + " ";
			str += ::GetColoredBoldTextForTooltip(g_GridCtrlHeaderNameExplanation6, true, true, explanationTextColor) + ": ";
			str += ::GetColoredBoldTextForTooltip(std::to_string(rowData->DataFileWinRegValues().LocalCacheDataCount()), true, true, valueTextColor);
			str += "<br>";
			auto enableDataColumnVisible = IsEnableDataColumnVisible();
			auto useBoldFontForEnableData = enableDataColumnVisible;
			if(!enableDataColumnVisible)
				str += "(";
			str += ::GetColoredBoldTextForTooltip(g_GridCtrlHeaderName7, useBoldFontForEnableData, true, headerNameColor) + " ";
			str += ::GetColoredBoldTextForTooltip(enableDataColumnVisible ? g_GridCtrlHeaderNameExplanation7 : g_GridCtrlHeaderNameExplanation7_2, useBoldFontForEnableData, true, explanationTextColor) + ": ";
			str += ::GetColoredBoldTextForTooltip(rowData->DataEnabled() ? "On" : "Off", useBoldFontForEnableData, true, valueTextColor);
			if(!enableDataColumnVisible)
				str += ")";
			str += "<br>";
			str += ::GetColoredBoldTextForTooltip(g_GridCtrlHeaderName8, true, true, headerNameColor) + " ";
			str += ::GetColoredBoldTextForTooltip(g_GridCtrlHeaderNameExplanation8, true, true, explanationTextColor) + ": ";
			str += "<br>";
			str += ::GetColoredBoldTextForTooltip(::GetSizeColumnString(*rowData), true, true, valueTextColor);
			str += ::AddPossibleSelectedModelRunTexts(*rowData);
		}
	}

	return str;
}

static void SetHeaders(CGridCtrl &theGridCtrl, const std::vector<CaseStudyHeaderParInfo> &theHeaders, int rowCount, bool fFirstTime, int theFixedRowCount, int theFixedColumnCount)
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
	// 1. on otsikko rivi on parametrien nimiä varten
	for(int i=0; i<columnCount; i++)
	{
		theGridCtrl.SetItemText(currentRow, i, CA2T(theHeaders[i].itsHeader.c_str()));
		theGridCtrl.SetItemState(currentRow, i, theGridCtrl.GetItemState(currentRow, i) | GVIS_READONLY);
		if(fFirstTime) // säädetään sarakkeiden leveydet vain 1. kerran
			theGridCtrl.SetColumnWidth(i, theHeaders[i].itsColumnWidth);
	}
}

void CFmiCaseStudyDlg::InitHeaders()
{
	int basicColumnWidthUnit = 18;
	itsHeaders.clear();
	itsHeaders.push_back(CaseStudyHeaderParInfo(g_GridCtrlHeaderName1, CaseStudyHeaderParInfo::kRowNumber, boost::math::iround(basicColumnWidthUnit*2.5)));
    itsHeaders.push_back(CaseStudyHeaderParInfo(g_GridCtrlHeaderName2, CaseStudyHeaderParInfo::kModelName, basicColumnWidthUnit * 18));
    itsHeaders.push_back(CaseStudyHeaderParInfo(g_GridCtrlHeaderName3, CaseStudyHeaderParInfo::kProducerId, basicColumnWidthUnit * 4));
	itsHeaders.push_back(CaseStudyHeaderParInfo(g_GridCtrlHeaderName4, CaseStudyHeaderParInfo::kStoreData, basicColumnWidthUnit*3));
    itsHeaders.push_back(CaseStudyHeaderParInfo(g_GridCtrlHeaderName5, CaseStudyHeaderParInfo::kCaseStudyDataOffsetRange, boost::math::iround(basicColumnWidthUnit * 3.5)));
    itsHeaders.push_back(CaseStudyHeaderParInfo(g_GridCtrlHeaderName6, CaseStudyHeaderParInfo::kLocalCacheDataCount, boost::math::iround(basicColumnWidthUnit * 3.5)));
    itsHeaders.push_back(CaseStudyHeaderParInfo(g_GridCtrlHeaderName7, CaseStudyHeaderParInfo::kEnableData, basicColumnWidthUnit*5)); // sijoita tämä indeksille CaseStudyHeaderParInfo::kEnableData
    itsHeaders.push_back(CaseStudyHeaderParInfo(g_GridCtrlHeaderName8, CaseStudyHeaderParInfo::kDataSize, boost::math::iround(basicColumnWidthUnit * 11.)));
}

const NFmiViewPosRegistryInfo CFmiCaseStudyDlg::s_ViewPosRegistryInfo(CRect(150, 250, 670, 800), "\\CaseStudyDialog");

// CFmiCaseStudyDlg dialog

IMPLEMENT_DYNAMIC(CFmiCaseStudyDlg, CDialog)

CFmiCaseStudyDlg::CFmiCaseStudyDlg(SmartMetDocumentInterface *smartMetDocumentInterface, const std::string theTitleStr, CWnd* pParent)
	: CDialog(CFmiCaseStudyDlg::IDD, pParent)
	,itsSmartMetDocumentInterface(smartMetDocumentInterface)
	,itsTitleStr(theTitleStr)
	,fGridControlInitialized(false)
	,fDialogInitialized(false)
	,itsGridCtrl()
	,itsHeaders()
	,itsOffsetEditTimer(0)
	,itsDisableStoreButtonTimer(0)
	,itsOffsetEditedCell()
	,itsBoldFont()
	, itsCaseStudySettingsWinRegistry(smartMetDocumentInterface->ApplicationWinRegistry().CaseStudySettingsWinRegistry())
    , fEditEnableData(FALSE)
    , fZipData(FALSE)
	, fStoreWarningMessages(FALSE)
	, fCropDataToZoomedMapArea(FALSE)
	, itsCaseStudyNameStrU_(_T(""))
	, itsCaseStudyInfoStrU_(_T(""))
	, itsCaseStudyPathStrU_(_T(""))
{

}

CFmiCaseStudyDlg::~CFmiCaseStudyDlg()
{
}

BOOL CFmiCaseStudyDlg::Create(CWnd* pParentWnd)
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialog::Create(CFmiCaseStudyDlg::IDD, pParentWnd);
}

void CFmiCaseStudyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_GridControl(pDX, IDC_CUSTOM_GRID_CASE_STUDY, itsGridCtrl);
	DDX_Check(pDX, IDC_CHECK_EDIT_ENABLE_DATA, fEditEnableData);
	DDX_Check(pDX, IDC_CHECK_ZIP_DATA, fZipData);
	DDX_Check(pDX, IDC_CHECK_STORE_WARNING_MESSAGES, fStoreWarningMessages);
	DDX_Check(pDX, IDC_CHECK_CROP_DATA_TO_ZOOMED_MAP_AREA, fCropDataToZoomedMapArea);
	DDX_Text(pDX, IDC_STATIC_CASE_STUDY_NAME, itsCaseStudyNameStrU_);
	DDX_Text(pDX, IDC_EDIT_INFO_STR, itsCaseStudyInfoStrU_);
	DDX_Text(pDX, IDC_STATIC_CASE_STUDY_PATH, itsCaseStudyPathStrU_);
}

#define WM_CASE_STUDY_OFFSET_EDITED = WM_USER + 222

BEGIN_MESSAGE_MAP(CFmiCaseStudyDlg, CDialog)
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_WM_CLOSE()
//	ON_WM_PAINT()
	ON_NOTIFY(GVN_ENDLABELEDIT, IDC_CUSTOM_GRID_CASE_STUDY, OnGridEndEdit)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_STORE_DATA, &CFmiCaseStudyDlg::OnBnClickedButtonStoreData)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_DATA, &CFmiCaseStudyDlg::OnBnClickedButtonLoadData)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE_MODE, &CFmiCaseStudyDlg::OnBnClickedButtonCloseMode)
    ON_BN_CLICKED(IDC_CHECK_EDIT_ENABLE_DATA, &CFmiCaseStudyDlg::OnBnClickedCheckEditEnableData)
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_BUTTON_REFRESH_GRID, &CFmiCaseStudyDlg::OnBnClickedButtonRefreshGrid)
	ON_BN_CLICKED(IDC_BUTTON_PRIORITIZE_DATA, &CFmiCaseStudyDlg::OnBnClickedButtonPrioritizeData)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_OLD_DATA, &CFmiCaseStudyDlg::OnBnClickedButtonLoadOldData)
END_MESSAGE_MAP()


// CFmiCaseStudyDlg message handlers

void CFmiCaseStudyDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	CRect windowRect;
	GetWindowRect(&windowRect);
	CRect clientRect;
	GetClientRect(&clientRect);
	int widthOverHead = windowRect.Width() - clientRect.Width();
	int heightOverHead = windowRect.Height() - clientRect.Height();
	lpMMI->ptMinTrackSize.x = 400 + widthOverHead;
	lpMMI->ptMinTrackSize.y = 400 + heightOverHead;

	CDialog::OnGetMinMaxInfo(lpMMI);
}

void CFmiCaseStudyDlg::SetDefaultValues()
{
    MoveWindow(CFmiCaseStudyDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

BOOL CFmiCaseStudyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	DoResizerHooking(); // Tätä pitää kutsua ennen kuin dialogin talletettu koko otetaan Windows rekisteristä
	fDialogInitialized = true;

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	InitDialogTexts();
	// Call InitHeaders before CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry !!
    InitHeaders();
    // Tee paikan asetus vasta tooltipin alustuksen jälkeen, niin se toimii ilman OnSize-kutsua.
	std::string errorBaseStr("Error in CFmiCaseStudyDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);

    auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
	itsCaseStudyNameStrU_ = CA2T(caseStudySystem.CaseStudyName().c_str());
    itsCaseStudyInfoStrU_ = CA2T(caseStudySystem.CaseStudyInfo().c_str());
    itsCaseStudyPathStrU_ = CA2T(caseStudySystem.CaseStudyPath().c_str());
	fZipData = caseStudySystem.ZipFiles();
	fStoreWarningMessages = caseStudySystem.StoreWarningMessages();
	fCropDataToZoomedMapArea = caseStudySystem.CropDataToZoomedMapArea();

	UpdateButtonStates();
    UpdateEditEnableDataText();
    EnableColorCodedControls();
	InitializeGridControlRelatedData();
	InitTooltipControl();

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

static std::string Make_StoreData_ButtonTooltipText()
{
	std::string text = "To store selected Case-Study data:\n";
	text += "1. Give stored Case-Study a description (-> Info:)\n";
	text += " - This is optional information.\n";
	text += "2. Press 'Store data' button to open file browser.\n";
	text += "3. Browse to the directory you want to store Case-Study.\n";
	text += "4. Give Case-Study a name as filename in browser.\n";
	text += " - You won't need to give file extension.\n";
	text += " - File extension will be given automatically.\n";
	text += "5. Press 'Save' button to start data storage operation.";
	return text;
}

static std::string Make_LoadData_ButtonTooltipText()
{
	std::string text = "To load wanted Case-Study data:\n";
	text += "1. Press 'Load data' button to open file browser.\n";
	text += "2. Browse to wanted directory and select Case-Study meta-file.\n";
	text += " - Case-Study meta-files have .csmeta file extension.\n";
	text += "3. Press 'Open' button to start data loading operation.";
	return text;
}

static std::string Make_CloseMode_ButtonTooltipText()
{
	std::string text = "To close Case-Study mode:\n";
	text += "1. Press 'Close mode' button which sets back to normal mode.\n";
	text += " - Used Case-Study data is not used anymore.\n";
	text += " - Normal latest data will be read into use.";
	return text;
}

static std::string Make_Refresh_ButtonTooltipText()
{
	std::string text = "To refresh data grid control:\n";
	text += "1. Press 'Refresh' button which will update the data grid.\n";
	text += " - This can be usefull if more data is read into system.";
	return text;
}

static std::string Make_Name_StaticTooltipText()
{
	std::string text = "Here is displayed loaded Case-Study's name.";
	return text;
}

static std::string Make_Path_StaticTooltipText()
{
	std::string text = "Here is displayed loaded/stored Case-Study's meta-file path.";
	return text;
}

static std::string Make_CropData_CheckTooltipText()
{
	std::string text = "With this checked Smartmet will try to save in data sizes.\n";
	text += " - Depending on data and main-map-view's area data is cropped.\n";
	text += " - This is usefull if having global data but wants to cover only smaller area on map.";

	return text;
}

static std::string Make_StoreWarnings_CheckTooltipText()
{
	std::string text = "Check this if you want to store HAKE warnings too.\n";
	text += " - This is useful only in FMI, these HAKE warnings are available only there.";

	return text;
}

static std::string Make_ZipData_CheckTooltipText()
{
	std::string text = "Check this if you want to zip stored Case-Study data.\n";
	text += " - This will generate a zip file containing all the relevant files.\n";
	text += " - Leaves original meta-data file and data directories intact.\n";
	text += " - So user needs to delete those if wanting to free space on hard-drive.";

	return text;
}

static std::string Make_EditEnableData_CheckTooltipText()
{
	std::string text = "Check this to change enabled/disabled state of each query-data.\n";
	text += " - When checked, 7th column appears with check-box controls.\n";
	text += " - If data is enabled, Smartmet will try to use it and load it's newest data from server.\n";
	text += " - If disabled, smartmet will ignore the data in every way.\n";
	text += " - If disabled, data row will turn into grey colored (grey indicates disabled state).\n";
	text += " - After changes has been made, you'll have to restart Smartmet to apply changes.";

	return text;
}

static std::string Make_MultiDataSelection_ButtonTooltipText()
{
	std::string text = "1. Select a data from data-grid control with left mouse click.\n";
	text += " - You can also select a Producer or Category to load more with less clicking.\n";
	text += "2. If you want to select multiple data, keep CTRL key down while left clicking.\n";

	return text;
}

static std::string Make_PrioritizeData_ButtonTooltipText()
{
	std::string text = "Makes 'prioritized data loading' background works.\n";
	text += " - Usable if Smartmet has been shutdown for a while and startup loading is slow.\n";
	text += " - Especially if connection to server is slow and query-data files are big.\n";
	text += " - And there are some data you need soon but know that they are loaded much later after all other data.\n";
	text += Make_MultiDataSelection_ButtonTooltipText();
	text += "3. Press 'Prioritize data' button and Smartmet will add these to fast load list.\n";
	text += " - They are removed from this list as soon as the latest wanted data is loaded.";

	return text;
}

static std::string Make_LoadOldData_ButtonTooltipText()
{
	std::string text = "Makes 'load older model run data' background works.\n";
	text += " - Usable if Smartmet has been shutdown for a while.\n";
	text += " - You need older model run data that are missing in local cache directory.\n";
	text += Make_MultiDataSelection_ButtonTooltipText();
	text += "3. Press 'Load old data' button and Smartmet will add these to load list.\n";
	text += " - SmartMet will load all the older data that are missing locally that are found in server.";

	return text;
}

void CFmiCaseStudyDlg::InitTooltipControl()
{
	CFmiWin32Helpers::InitializeCPPTooltip(this, m_dialog_tooltip, CASE_STUDY_DIALOG_TOOLTIP_ID, 800);
	m_dialog_tooltip.SetDelayTime(PPTOOLTIP_TIME_INITIAL, 750); // kuinka nopeasti tooltip ilmestyy näkyviin, jos kursoria ei liikuteta [ms]

	// Tässä erikseen jokainen kontrolli, jolle halutaan joku tooltip teksti
	SetDialogControlTooltip(IDC_BUTTON_STORE_DATA, ::Make_StoreData_ButtonTooltipText());
	SetDialogControlTooltip(IDC_BUTTON_LOAD_DATA, ::Make_LoadData_ButtonTooltipText());
	SetDialogControlTooltip(IDC_BUTTON_CLOSE_MODE, ::Make_CloseMode_ButtonTooltipText());
	SetDialogControlTooltip(IDC_BUTTON_REFRESH_GRID, ::Make_Refresh_ButtonTooltipText());
	SetDialogControlTooltip(IDC_STATIC_CASE_STUDY_NAME, ::Make_Name_StaticTooltipText());
	SetDialogControlTooltip(IDC_STATIC_CASE_STUDY_PATH, ::Make_Path_StaticTooltipText());
	SetDialogControlTooltip(IDC_CHECK_CROP_DATA_TO_ZOOMED_MAP_AREA, ::Make_CropData_CheckTooltipText());
	SetDialogControlTooltip(IDC_CHECK_STORE_WARNING_MESSAGES, ::Make_StoreWarnings_CheckTooltipText());
	SetDialogControlTooltip(IDC_CHECK_ZIP_DATA, ::Make_ZipData_CheckTooltipText());
	SetDialogControlTooltip(IDC_CHECK_EDIT_ENABLE_DATA, ::Make_EditEnableData_CheckTooltipText());
	SetDialogControlTooltip(IDC_BUTTON_PRIORITIZE_DATA, ::Make_PrioritizeData_ButtonTooltipText());
	SetDialogControlTooltip(IDC_BUTTON_LOAD_OLD_DATA, ::Make_LoadOldData_ButtonTooltipText());
}

void CFmiCaseStudyDlg::SetDialogControlTooltip(int controlId, const std::string& tooltipRawText)
{
	CWnd* controlWnd = GetDlgItem(controlId);
	if(controlWnd)
	{
		auto finalText = ::GetDictionaryString(tooltipRawText.c_str());
		m_dialog_tooltip.AddTool(controlWnd, CA2T(finalText.c_str()));
	}
}

BOOL CFmiCaseStudyDlg::PreTranslateMessage(MSG* pMsg)
{
	m_dialog_tooltip.RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}

void CFmiCaseStudyDlg::InitDialogTexts()
{
	SetWindowText(CA2T(itsTitleStr.c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_STORE_DATA, "Store data");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_LOAD_DATA, "Load data");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_CLOSE_MODE, "Close mode");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_REFRESH_GRID, "Refresh");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_NAME2_STR, "Name:");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_INFO_STR, "Info:");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_PATH_STR, "Absolute path:\n(e.g.c:\\data)");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_STORE_WARNING_MESSAGES, "Store warnings");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_ZIP_DATA, "Zip data");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_CROP_DATA_TO_ZOOMED_MAP_AREA, "Crop data to zoomed main-map area");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_PRIORITIZE_DATA, "Prioritize data");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_LOAD_OLD_DATA, "Load old data");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_EDIT_ENABLE_DATA, gEditEnableDataCheckControlOffStr.c_str());
}

void CFmiCaseStudyDlg::DoResizerHooking(void)
{
	BOOL bOk = m_resizer.Hook(this);
	ASSERT(bOk == TRUE);

	bOk = m_resizer.SetAnchor(IDC_BUTTON_STORE_DATA, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_LOAD_DATA, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_CLOSE_MODE, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_REFRESH_GRID, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_NAME2_STR, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_INFO_STR, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_PATH_STR, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_CHECK_STORE_WARNING_MESSAGES, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_CHECK_ZIP_DATA, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);

	bOk = m_resizer.SetAnchor(IDC_CHECK_CROP_DATA_TO_ZOOMED_MAP_AREA, ANCHOR_TOP | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_PRIORITIZE_DATA, ANCHOR_TOP | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_LOAD_OLD_DATA, ANCHOR_TOP | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);

	bOk = m_resizer.SetAnchor(IDC_STATIC_CASE_STUDY_NAME, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_EDIT_INFO_STR, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_CASE_STUDY_PATH, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_CHECK_EDIT_ENABLE_DATA, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
	ASSERT(bOk == TRUE);

	bOk = m_resizer.SetAnchor(IDC_CUSTOM_GRID_CASE_STUDY, ANCHOR_VERTICALLY | ANCHOR_HORIZONTALLY);
	ASSERT(bOk == TRUE);
}

void CFmiCaseStudyDlg::EnableColorCodedControls()
{
    CFmiWin32Helpers::EnableColorCodedControl(this, IDC_CHECK_EDIT_ENABLE_DATA);
}

void CFmiCaseStudyDlg::UpdateEditEnableDataText()
{
    if(fEditEnableData)
        CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_EDIT_ENABLE_DATA, gEditEnableDataCheckControlOnStr.c_str());
    else
        CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_EDIT_ENABLE_DATA, gEditEnableDataCheckControlOffStr.c_str());
}

void CFmiCaseStudyDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	AdjustGridControl();
}

void CFmiCaseStudyDlg::AdjustGridControl()
{
	if(fGridControlInitialized)
	{
		CRect gridControlRect = CalcGridArea();
		FitNameColumnOnVisibleArea(gridControlRect.Width());
	}
}

CRect CFmiCaseStudyDlg::CalcGridArea()
{
	CRect clientRect;
	GetClientRect(clientRect);
	CWnd* win = GetDlgItem(IDC_CUSTOM_GRID_CASE_STUDY);
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

void CFmiCaseStudyDlg::FitNameColumnOnVisibleArea(int gridCtrlWidth)
{
	if(itsGridCtrl.GetColumnCount())
	{
		int otherColumnsCombinedWidth = 0;
		for(auto columnIndex = 0; columnIndex <= itsGridCtrl.GetColumnCount(); columnIndex++)
		{
			// Skipataan ModelName column
			if(columnIndex != CaseStudyHeaderParInfo::kModelName)
			{
				CRect cellRect;
				itsGridCtrl.GetCellRect(0, columnIndex, cellRect);
				otherColumnsCombinedWidth += cellRect.Width();
			}
		}

		// Calculate new width for name column so that it will fill the client area
		// Total width (gridCtrlWidth) - otherColumnsCombinedWidth - some value (60) that represents the width of the vertical scroll control
		int newNameColumnWidth = gridCtrlWidth - otherColumnsCombinedWidth - 60;
		// Let's make sure that last column isn't shrinken too much
		newNameColumnWidth = std::max(newNameColumnWidth, 120);
		itsGridCtrl.SetColumnWidth(CaseStudyHeaderParInfo::kModelName, newNameColumnWidth);
	}
}

void CFmiCaseStudyDlg::InitializeGridControlRelatedData()
{
	if(fGridControlInitialized == false)
	{
		fGridControlInitialized = true;
		itsProducerSystemsHolder = std::make_unique<NFmiProducerSystemsHolder>();
		itsProducerSystemsHolder->itsModelProducerSystem = &(itsSmartMetDocumentInterface->ProducerSystem());
		itsProducerSystemsHolder->itsObsProducerSystem = &(itsSmartMetDocumentInterface->ObsProducerSystem());
		itsProducerSystemsHolder->itsSatelImageProducerSystem = &(itsSmartMetDocumentInterface->SatelImageProducerSystem());
		itsGridCtrl.GetFont()->GetLogFont(&itsBoldFont); // otetaan defaultti fontti tiedot talteen
		itsBoldFont.lfWeight = FW_BOLD; // asetetaan 'paino' bold-arvoon 
		itsGridCtrl.AllowNonChangingEdit_Marko(TRUE);
		// CaseStudy datoihin pitää tehdä tuottaja kohtaisia päivityksiä ennen kuin ne näytetään 1. kerran
		OnBnClickedButtonRefreshGrid();
		// Kun dialogia alustetaan, pitää tätä kutsua kerran, koska tämä laittaa piiloon "Enable data" sarakkeen, 
		// joka aukeaa näkyville vasta kun sen toglaava checkbox kontrollia on klikattu.
		ShowEnableColumn();
		DoFirstTimeSilamCategoryCollapse();
	}
}

void CFmiCaseStudyDlg::DoFirstTimeSilamCategoryCollapse()
{
	// Pitää testata itsTreeColumn:n koko että ei ole tyhjä, muuten voi kaatua
	if(itsTreeColumn.GetRowCount() > 0)
	{
		if(itsPossibleSilamCategoryRowNumber > 0)
		{
			itsTreeColumn.TreeDataCollapseAllSubLevels(itsPossibleSilamCategoryRowNumber);
			itsTreeColumn.TreeRefreshRows();
		}
	}
}

void CFmiCaseStudyDlg::DoWhenClosing()
{
//	itsDoc->DataQualityChecker().ViewOn(false); // jos jossain olisi tieto että onko caseStudy-dialogi auki/ei, tässä olisi muokattava sen statusta
	GetBasicInfoFromDialog();
	itsSmartMetDocumentInterface->StoreCaseStudyMemory(); // Kun dialogi suljetaan, talletetaan aina casestudy-metatieto talteen
	AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttanäyttö eli mainframe
}

void CFmiCaseStudyDlg::OnCancel()
{
	DoWhenClosing();

	CDialog::OnCancel();
}

void CFmiCaseStudyDlg::OnOK()
{
	DoWhenClosing();

	CDialog::OnOK();
}

void CFmiCaseStudyDlg::OnClose()
{
	DoWhenClosing();

	CDialog::OnClose();
}

static std::string GetDataCountStr(int theDataCount)
{
	return std::to_string(theDataCount);
}

static std::string GetColumnText(int theRow, int theColumn, const NFmiCaseStudyDataFile &theCaseData)
{
	switch(theColumn)
	{
	case CaseStudyHeaderParInfo::kRowNumber:
		return NFmiStringTools::Convert(theRow);
	case CaseStudyHeaderParInfo::kModelName:
		return theCaseData.Name();
    case CaseStudyHeaderParInfo::kProducerId:
        return ::GetProducerIdString(theCaseData);
    case CaseStudyHeaderParInfo::kCaseStudyDataOffsetRange:
		return NFmiCaseStudySystem::MakeModelDataOffsetRangeInHoursString(theCaseData.DataFileWinRegValues().CaseStudyModelDataOffsetRangeInHours());
	case CaseStudyHeaderParInfo::kLocalCacheDataCount:
		return ::GetDataCountStr(theCaseData.DataFileWinRegValues().LocalCacheDataCount());
	case CaseStudyHeaderParInfo::kDataSize:
		return ::GetSizeColumnString(theCaseData);
	default:
		return "";
	}
}

static bool IsReadOnlyCell(int column, const NFmiCaseStudyDataFile &theCaseData)
{
    if(column == CaseStudyHeaderParInfo::kStoreData || column == CaseStudyHeaderParInfo::kEnableData)
		return false;
	if(column == CaseStudyHeaderParInfo::kCaseStudyDataOffsetRange)
	{
		if(theCaseData.StoreLastDataOnly())
			return true;
		else
			return false;
	}
	if(column == CaseStudyHeaderParInfo::kLocalCacheDataCount)
	{
		if(theCaseData.StoreLastDataOnly())
			return true;
		if(theCaseData.DataType() == NFmiInfoData::kSatelData)
			return true;
		else
			return false;
	}

	return true;
}

static const COLORREF gNormalBkColor = RGB(255, 255, 255);
static const COLORREF gCategoryBkColor = RGB(250, 220, 220);
static const COLORREF gProducerBkColor = RGB(220, 250, 220);
static const COLORREF gOneDataBkColor = RGB(240, 240, 255);
static const COLORREF gDisabledDataBkColor = RGB(170, 170, 170); // disabloitujen datojen rivi väriksi harmaa
static const COLORREF gLocalCacheColumnColor = RGB(255, 255, 150);

static void SetCellFont(CGridCtrl &theGridCtrl, int row, int col, const NFmiCaseStudyDataFile &theCaseData, LOGFONT *theBoldFont)
{
	bool useBoldFont = theCaseData.DataFileWinRegValues().Store();
	if(::IsHeaderRow(theCaseData))
	{
		if(theCaseData.TotalFileSize() <= 0)
			useBoldFont = false; // jos kyse on header-rivistä ja sen total koko on 0, ei laiteta riviä boldilla riippumatta mikä on Store-asetus
		else
			useBoldFont = true; // jos kyse on header-rivistä ja total size suupempi kuin 0, laitetaan boldiksi
	}

	if(useBoldFont)
		theGridCtrl.SetItemFont(row, col, theBoldFont);
	else
		theGridCtrl.SetItemFont(row, col, NULL);
}

void CFmiCaseStudyDlg::SetGridRow(int row, const NFmiCaseStudyDataFile& theCaseData, int theFixedColumnCount, bool updateOnly)
{
	for(int column = 0; column < static_cast<int>(itsHeaders.size()); column++)
	{
		if(!updateOnly)
			::SetCellFont(itsGridCtrl, row, column, theCaseData, &itsBoldFont);
		bool dataDisabled = theCaseData.DataEnabled() == false;
		bool readOnlyCell = ::IsReadOnlyCell(column, theCaseData);
		itsGridCtrl.SetItemText(row, column, CA2T(::GetColumnText(row, column, theCaseData).c_str()));
		if(column >= theFixedColumnCount)
		{
			if(column != CaseStudyHeaderParInfo::kEnableData)
			{
				if(readOnlyCell || dataDisabled)
					itsGridCtrl.SetItemState(row, column, itsGridCtrl.GetItemState(row, column) | GVIS_READONLY); // Laita read-only -bitti päälle
				else
					itsGridCtrl.SetItemState(row, column, itsGridCtrl.GetItemState(row, column) & ~GVIS_READONLY); // Laita read-only -bitti pois päältä
			}

			if(column == CaseStudyHeaderParInfo::kLocalCacheDataCount)
				itsGridCtrl.SetItemBkColour(row, column, gLocalCacheColumnColor);
			else if(dataDisabled)
				itsGridCtrl.SetItemBkColour(row, column, gDisabledDataBkColor);
			else if(theCaseData.CategoryHeader())
				itsGridCtrl.SetItemBkColour(row, column, gCategoryBkColor);
			else if(theCaseData.ProducerHeader())
				itsGridCtrl.SetItemBkColour(row, column, gProducerBkColor);
			else if(theCaseData.OnlyOneData())
				itsGridCtrl.SetItemBkColour(row, column, gOneDataBkColor);
			else
				itsGridCtrl.SetItemBkColour(row, column, gNormalBkColor);

			if(column == CaseStudyHeaderParInfo::kStoreData)
			{
				if(!updateOnly)
					itsGridCtrl.SetCellType(row, column, RUNTIME_CLASS(CGridCellCheck));
				CGridCellCheck* pTempCell = (CGridCellCheck*)itsGridCtrl.GetCell(row, column);
				pTempCell->SetCheck(theCaseData.DataFileWinRegValues().Store());
				if(!updateOnly)
					pTempCell->SetCheckBoxClickedCallback(boost::bind(&CFmiCaseStudyDlg::HandleCheckBoxClick, this, _1, _2));
			}
			else if(column == CaseStudyHeaderParInfo::kEnableData)
			{
				if(!updateOnly)
					itsGridCtrl.SetCellType(row, column, RUNTIME_CLASS(CGridCellCheck));
				CGridCellCheck* pTempCell = (CGridCellCheck*)itsGridCtrl.GetCell(row, column);
				pTempCell->SetCheck(theCaseData.DataEnabled());
				if(!updateOnly)
					pTempCell->SetCheckBoxClickedCallback(boost::bind(&CFmiCaseStudyDlg::HandleEnableDataCheckBoxClick, this, _1, _2));
			}
		}
	}
}

void CFmiCaseStudyDlg::InitGridControlValues()
{
	static bool fFirstTime = true;
	int fixedRowCount = 1;
	int fixedColumnCount = 1;

    auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
    int dataRowCount = static_cast<int>(caseStudySystem.CaseStudyDialogData().size());
	int maxRowCount = fixedRowCount + dataRowCount;
	SetHeaders(itsGridCtrl, itsHeaders, maxRowCount, fFirstTime, fixedRowCount, fixedColumnCount);
    UpdateRows(fixedRowCount, fixedColumnCount, false);

	std::vector<unsigned char> &treePatternArray = caseStudySystem.TreePatternArray();
	if(treePatternArray.size()) // pitää testata 0 koko vastaan, muuten voi kaatua
	{
		itsTreeColumn.TreeSetup(&itsGridCtrl, 1, static_cast<int>(treePatternArray.size()), 1, &treePatternArray[0], TRUE, FALSE);
	}
    fFirstTime = false;
}

void CFmiCaseStudyDlg::UpdateRows(int fixedRowCount, int fixedColumnCount, bool updateOnly)
{
	auto &dataVector = itsSmartMetDocumentInterface->CaseStudySystem().CaseStudyDialogData();
	int currentRowCount = fixedRowCount;
	for(auto data : dataVector)
	{
		const auto& caseStudyData = *data;
		// Suljetaan mahdollinen Silam sektio 1. kerralla
		if(caseStudyData.CategoryHeader() && caseStudyData.Name() == NFmiCaseStudySystem::GetSilamCustomFolderName())
		{
			itsPossibleSilamCategoryRowNumber = currentRowCount;
		}

        SetGridRow(currentRowCount++, caseStudyData, fixedColumnCount, updateOnly);
	}
}

void CFmiCaseStudyDlg::UpdateGridControlValues(bool updateOnly)
{
    UpdateRows(1, 1, updateOnly);
	UpdateData(FALSE);
	Invalidate(FALSE);
}

void CFmiCaseStudyDlg::HandleCheckBoxClick(int col, int row)
{
	UpdateData(TRUE);
	CGridCellCheck *pCell = dynamic_cast<CGridCellCheck *>(itsGridCtrl.GetCell(row, col));
	if(pCell)
	{
		bool newState = pCell->GetCheck() == TRUE;
		int dataIndex = row - 1; // rivit alkavat 1:stä, mutta datat on vektorissa 0:sta alkaen
        auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
        std::vector<NFmiCaseStudyDataFile*> &dataVector = caseStudySystem.CaseStudyDialogData();
		if(dataIndex >= 0 && dataIndex < static_cast<int>(dataVector.size()))
		{
			NFmiCaseStudyDataFile &dataFile = *dataVector[dataIndex];
			if(dataFile.CategoryHeader())
			{
                caseStudySystem.CategoryStore(dataFile, newState); // tämä tekee myös tarvittavat updatet data rakenteille
			}
			else if(dataFile.ProducerHeader())
			{
                caseStudySystem.ProducerStore(dataFile, newState); // tämä tekee myös tarvittavat updatet data rakenteille
			}
			else
			{
				dataFile.DataFileWinRegValues().Store(newState);
                caseStudySystem.Update(dataFile);
			}
			UpdateGridControlValues(true);
		}
	}
}

void CFmiCaseStudyDlg::HandleEnableDataCheckBoxClick(int col, int row)
{
	UpdateData(TRUE);
	CGridCellCheck *pCell = dynamic_cast<CGridCellCheck *>(itsGridCtrl.GetCell(row, col));
	if(pCell)
	{
		bool newState = pCell->GetCheck() == TRUE;
		int dataIndex = row - 1; // rivit alkavat 1:stä, mutta datat on vektorissa 0:sta alkaen
        auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
		std::vector<NFmiCaseStudyDataFile*> &dataVector = caseStudySystem.CaseStudyDialogData();
		if(dataIndex >= 0 && dataIndex < static_cast<int>(dataVector.size()))
		{
			NFmiCaseStudyDataFile &dataFile = *dataVector[dataIndex];
			if(dataFile.CategoryHeader())
			{
                caseStudySystem.CategoryEnable(*itsSmartMetDocumentInterface->HelpDataInfoSystem(), dataFile, newState); // tämä tekee myös tarvittavat updatet data rakenteille
			}
			else if(dataFile.ProducerHeader())
			{
                caseStudySystem.ProducerEnable(*itsSmartMetDocumentInterface->HelpDataInfoSystem(), dataFile, newState); // tämä tekee myös tarvittavat updatet data rakenteille
			}
			else
			{
                dataFile.DataEnabled(*itsSmartMetDocumentInterface->HelpDataInfoSystem(), newState);
                caseStudySystem.Update(dataFile);
			}
			UpdateGridControlValues(true);
            itsSmartMetDocumentInterface->UpdateEnableDataChangesToWinReg();
		}
	}
}

void CFmiCaseStudyDlg::OnGridEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	UpdateData(TRUE);
    NM_GRIDVIEW* pItem = (NM_GRIDVIEW*) pNotifyStruct;
    BOOL bAllowEdit = AcceptChange(pItem->iColumn, pItem->iRow);
    *pResult = (bAllowEdit)? 0 : -1;
}

// AcceptChange is a routine that returns TRUE
// if you want to accept the new value for the cell.
BOOL CFmiCaseStudyDlg::AcceptChange(int col, int row)
{
	if(itsGridCtrl.IsValid(row, col))
	{
		std::string str = CT2A(itsGridCtrl.GetCell(row, col)->GetText());
		try
		{
			if(col == CaseStudyHeaderParInfo::kCaseStudyDataOffsetRange)
				auto offsetRange = NFmiCaseStudySystem::MakeTimeOffsetRange(str);
			else
				double offsetInHours = NFmiStringTools::Convert<double>(str);
			itsOffsetEditedCell = CCellID(row, col);
			itsOffsetEditTimer = static_cast<UINT>(SetTimer(kFmiDataCountEditTimer, 50, NULL)); // käynnistetään timer, että saadaan pikkuisen viiveen jälkeen käynnistettyä offsetin muutoksesta aiheutuva päivitys
			return TRUE;
		}
		catch(...)
		{
			return FALSE;
		}
	}
	return FALSE;
}

void CFmiCaseStudyDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
	case kFmiDataCountEditTimer:
		{
			KillTimer(itsOffsetEditTimer);
			OnDataCountEdited();
			return ;
		}
	case kFmiDisableStoreButtonTimer:
		{
			if(IsThereCaseStudyMakerRunning() == false)
			{ // voidaan lopettaa timer ja laittaa Store-nappi taas enabloiduksi
				KillTimer(itsDisableStoreButtonTimer);
				itsDisableStoreButtonTimer = 0;
				UpdateButtonStates();
			}
			return ;
		}
	}
	CDialog::OnTimer(nIDEvent);
}

void CFmiCaseStudyDlg::OnDataCountEdited()
{
	if(itsGridCtrl.IsValid(itsOffsetEditedCell))
	{

		int dataIndex = itsOffsetEditedCell.row - 1; // rivit alkavat 1:stä, mutta datat on vektorissa 0:sta alkaen
        auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
        std::vector<NFmiCaseStudyDataFile*> &dataVector = caseStudySystem.CaseStudyDialogData();
		if(dataIndex >= 0 && dataIndex < static_cast<int>(dataVector.size()))
		{

			std::string cellText = CT2A(itsGridCtrl.GetCell(itsOffsetEditedCell.row, itsOffsetEditedCell.col)->GetText());
			auto& dataFile = *dataVector[dataIndex];
			if(itsOffsetEditedCell.col == CaseStudyHeaderParInfo::kLocalCacheDataCount)
				DoLocalCacheCountEditing(dataFile, cellText);
			else if(itsOffsetEditedCell.col == CaseStudyHeaderParInfo::kCaseStudyDataOffsetRange)
				DoCaseStudyOffsetRangeEditing(dataFile, cellText);
		}
	}
	itsOffsetEditedCell = CCellID();
}

void CFmiCaseStudyDlg::DoLocalCacheCountEditing(NFmiCaseStudyDataFile& dataFile, const std::string& cellText)
{
	try
	{
		auto dataCount = boost::math::iround(NFmiStringTools::Convert<double>(cellText));
		auto& caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();

		if(dataFile.CategoryHeader())
		{
			caseStudySystem.CategoryLocalCacheDataCount(dataFile, dataCount);
		}
		else if(dataFile.ProducerHeader())
		{
			caseStudySystem.ProducerLocalCacheDataCount(dataFile, dataCount);
		}
		else
		{
			dataFile.DataFileWinRegValues().LocalCacheDataCount(dataCount);
		}
		UpdateGridControlValues(true);
	}
	catch(std::exception &e)
	{
		std::string errorMessage = __FUNCTION__;
		errorMessage += " cell editing error: ";
		errorMessage += e.what();
		itsSmartMetDocumentInterface->LogAndWarnUser(errorMessage, "", CatLog::Severity::Error, CatLog::Category::Operational, true);
	}
}

void CFmiCaseStudyDlg::DoCaseStudyOffsetRangeEditing(NFmiCaseStudyDataFile& dataFile, const std::string& cellText)
{
	try
	{
		auto timeOffsetRange = NFmiCaseStudySystem::MakeTimeOffsetRange(cellText);
		auto& caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();

		if(dataFile.CategoryHeader())
		{
			// Tämä tekee myös tarvittavat updatet data rakenteiden case-studyyn talletettavien tiedostojen datamäärille
			caseStudySystem.CategoryCaseStudyOffsetRange(dataFile, timeOffsetRange);
		}
		else if(dataFile.ProducerHeader())
		{
			// Tämä tekee myös tarvittavat updatet data rakenteiden case-studyyn talletettavien tiedostojen datamäärille
			caseStudySystem.ProducerCaseStudyOffsetRange(dataFile, timeOffsetRange);
		}
		else
		{
			dataFile.DataFileWinRegValues().CaseStudyModelDataOffsetRangeInHours(timeOffsetRange);
			caseStudySystem.Update(dataFile);
		}
		UpdateGridControlValues(true);
	}
	catch(std::exception &e)
	{
		const std::string rangeIndexHelperText = " \n(give range-index input in form 'm-n', e.g. '3-1' or just '3' which means '3-1')";
		std::string errorMessage = __FUNCTION__;
		errorMessage += " cell editing error: ";
		errorMessage += e.what();
		errorMessage += rangeIndexHelperText;
		itsSmartMetDocumentInterface->LogAndWarnUser(errorMessage, "", CatLog::Severity::Error, CatLog::Category::Operational, true);
	}
}

bool CFmiCaseStudyDlg::IsThereCaseStudyMakerRunning()
{
	std::string dummyStr;
	return (NFmiApplicationDataBase::CountProcessCount(gCaseStudyMakerData, dummyStr) > 0);
}

void CFmiCaseStudyDlg::GetBasicInfoFromDialog()
{
	UpdateData(TRUE);
    auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
    caseStudySystem.CaseStudyName(std::string(CT2A(itsCaseStudyNameStrU_)));
    caseStudySystem.CaseStudyInfo(std::string(CT2A(itsCaseStudyInfoStrU_)));

	// polun perässä ei saa olla minkäänsuuntaisia kenoviivoja. Datan kopioinnissa käytetty 
	// CShellFileOp -luokka on traumaattisen herkkä minkäänlaisille tai asteisille epämääräisyyksille.
	// nyt jos polussa on kenoviiva perässä, tulee siihen kaksi kenoviivaa ja tiedostojen kopioinnit epäonnistuvat.
	std::string strippedPathString = CT2A(itsCaseStudyPathStrU_);
	NFmiStringTools::TrimR(strippedPathString, '\\');
	NFmiStringTools::TrimR(strippedPathString, '/');
	NFmiStringTools::ReplaceChars(strippedPathString, '/', '\\'); // varmistetaan vielä että kaikki kenot ovat winkkarin omaan suuntaan.
    caseStudySystem.CaseStudyPath(strippedPathString);

	caseStudySystem.ZipFiles(fZipData == TRUE);
	caseStudySystem.StoreWarningMessages(fStoreWarningMessages == TRUE);
	caseStudySystem.CropDataToZoomedMapArea(fCropDataToZoomedMapArea == TRUE);
}

static std::string AddFileExtensionIfMissing(std::string filePath, const std::string &fileExtension)
{
	NFmiFileString fileString(filePath);
	if(fileString.Extension() != NFmiString(fileExtension))
	{
		filePath += "." + fileExtension;
	}
	return filePath;
}

void CFmiCaseStudyDlg::OnBnClickedButtonStoreData()
{
    //	return; // Store-napin toiminta on väliaikaisesti disabloitu, että saan tehtyä 5.8.3.1-jakeluversion, joka korjaa kaatumistaudin 

    EnableButton(IDC_BUTTON_STORE_DATA, FALSE);
    UpdateData(TRUE);
    GetBasicInfoFromDialog();

    itsSmartMetDocumentInterface->StoreCaseStudyMemory(); // laitetaan CaseStudy-datan tilanne varmuudeksi talteen erilliseen 'muisti'-tiedostoon
    std::string dummyStr;
    if(IsThereCaseStudyMakerRunning())
    { // vain yksi CaseStudyMaker voi olla käynnisssä kerrallaan...
        std::string noticeStr(::GetDictionaryString("Only one Case Study data collector can be run at once."));
        noticeStr += "\n";
        noticeStr += (::GetDictionaryString("Stopping this data storing action."));
        noticeStr += "\n";
        noticeStr += (::GetDictionaryString("Try again later..."));
        std::string titleStr(::GetDictionaryString("Case Study data won't be stored"));
        ::MessageBox(this->GetSafeHwnd(), CA2T(noticeStr.c_str()), CA2T(titleStr.c_str()), MB_OK | MB_ICONSTOP);
        UpdateButtonStates();
        return;
    }

	std::string fileExtension = "csmeta";
	std::string fileFilter = "CaseStudy Files (*.csmeta)|*.csmeta|All Files (*.*)|*.*||";
	std::string initialDirectory = itsSmartMetDocumentInterface->LoadedCaseStudySystem().CaseStudyPath();
	initialDirectory = PathUtils::getPathSectionFromTotalFilePath(initialDirectory);
	std::string usedMetaDataFilePath;
	std::string initialfileName = "CaseStudy1";
	if(BetaProduct::GetFilePathFromUser(fileFilter, initialDirectory, usedMetaDataFilePath, false, initialfileName, this))
	{
		usedMetaDataFilePath = ::AddFileExtensionIfMissing(usedMetaDataFilePath, fileExtension);
		if(itsSmartMetDocumentInterface->CaseStudySystem().StoreMetaData(this, usedMetaDataFilePath, true))
		{
			std::string commandStr;
			commandStr += "\""; // laitetaan lainausmerkit komento polun ympärille, jos siinä sattuisi olemaan spaceja
			commandStr += itsSmartMetDocumentInterface->ApplicationDataBase().GetDecodedApplicationDirectory();
			commandStr += "\\";
			commandStr += gCaseStudyMakerData.first;
			commandStr += "\" \""; // laitetaan lainausmerkit metadatatiedoston polun ympärille, jos siinä sattuisi olemaan spaceja
			commandStr += usedMetaDataFilePath;
			commandStr += "\""; // laitetaan lainausmerkit metadatatiedoston polun ympärille, jos siinä sattuisi olemaan spaceja
			commandStr += AddPossibleZippingOptions();
			commandStr += AddPossibleHakeMessageOptions();
			commandStr += AddPossibleCropDataToZoomedMapAreaOptions();

			CFmiProcessHelpers::ExecuteCommandInSeparateProcess(commandStr, true, true, SW_MINIMIZE);
		}
	}
    UpdateButtonStates();
}

std::string CFmiCaseStudyDlg::AddPossibleZippingOptions() const
{
	if(fZipData)
	{
		// Tämä kysymysmerkki on erotin, jolla erotellaan metatiedosto ja zip-ohjelman polku+nimi
		// En keksinyt tähän hätään muuta keinoa antaa winExelle erilaisia argumentteja yhdellä kertaa, joutuisin tekemään erillesen komentorivi 
		// parserin, joka osaisi käsitellä erilaisia optioita (kuten SmartMetissa tehdään).
		// '?' on erotin, koska sellaista kirjainta ei voi olla polussa kuin wild-card -merkkinä, ei varsinaisessa polussa tai tiedoston nimessä.
		std::string zipOptions = "?";
		zipOptions += CFmiProcessHelpers::Make7zipExePath(itsSmartMetDocumentInterface->WorkingDirectory());
		return zipOptions;
	}
	else
		return "";
}

std::string CFmiCaseStudyDlg::AddPossibleHakeMessageOptions() const
{
	if(fStoreWarningMessages && itsSmartMetDocumentInterface->WarningCenterSystem().isThereAnyWorkToDo())
	{
		// Tälläisiä store-messages optioita voi olla monia, jos tulevaisuudessa tulee uusia talletettavia sanomia.
		// ?store-messages="absolute-path-to-messages-file-filter"
		// And here is actual sample from FMI HAKE messages file-filter:
		// ?store-messages="D:\SmartMet\Dropbox (FMI)\data_FMI\HAKE\*.json"
		std::string storeMessagesOptions = "?";
		storeMessagesOptions += CFmiProcessHelpers::GetStoreMessagesString();
		storeMessagesOptions += "\""; // laitetaan lainausmerkit komento polun ympärille, jos siinä sattuisi olemaan spaceja
		storeMessagesOptions += itsSmartMetDocumentInterface->WarningCenterSystem().getHakeMessageAbsoluteFileFilter();
		storeMessagesOptions += "\""; // laitetaan lainausmerkit komento polun ympärille, jos siinä sattuisi olemaan spaceja
		return storeMessagesOptions;
	}
	else
		return "";
}

std::string CFmiCaseStudyDlg::AddPossibleCropDataToZoomedMapAreaOptions() const
{
	if(fCropDataToZoomedMapArea)
	{
		// Tämä kysymysmerkki on erotin, jolla erotellaan metatiedosto ja zip-ohjelman polku+nimi
		// En keksinyt tähän hätään muuta keinoa antaa winExelle erilaisia argumentteja yhdellä kertaa, joutuisin tekemään erillesen komentorivi 
		// parserin, joka osaisi käsitellä erilaisia optioita (kuten SmartMetissa tehdään).
		// Jos croppaus option on päällä, lisätään seuraavat optiot: ?CropDataArea=newbasearea
		int mainMapViewIndex = 0;
		auto area = itsSmartMetDocumentInterface->GetMapHandlerInterface(mainMapViewIndex)->Area();
		std::string optionStr = "?";
		optionStr += NFmiCaseStudySystem::GetCropDataOptionStartPart();
		optionStr += area->AreaStr();
		return optionStr;
	}
	else
		return "";
}

void CFmiCaseStudyDlg::OnBnClickedButtonLoadData()
{
	UpdateData(TRUE);
	std::string fileFilter = "CaseStudy Files (*.csmeta)|*.csmeta|All Files (*.*)|*.*||";
	std::string initialDirectory = itsSmartMetDocumentInterface->LoadedCaseStudySystem().CaseStudyPath();
	std::string usedFilePath;
	std::string initialfileName = "CaseStudy1";
	if(BetaProduct::GetFilePathFromUser(fileFilter, initialDirectory, usedFilePath, true, initialfileName, this))
	{
		if(itsSmartMetDocumentInterface->LoadCaseStudyData(usedFilePath))
		{
			itsCaseStudyNameStrU_ = CA2T(itsSmartMetDocumentInterface->LoadedCaseStudySystem().CaseStudyName().c_str());
			itsCaseStudyInfoStrU_ = CA2T(itsSmartMetDocumentInterface->LoadedCaseStudySystem().CaseStudyInfo().c_str());
			itsCaseStudyPathStrU_ = CA2T(usedFilePath.c_str());
			UpdateData(FALSE);
		}
		UpdateButtonStates();
		itsSmartMetDocumentInterface->SetAllViewIconsDynamically();
	}
}

std::pair<std::string, std::string> CFmiCaseStudyDlg::GetNameAndInfoStringsFromSelectedMetaFilePath(std::string fullPathToMetaFile)
{
	NFmiCaseStudySystem tmpCaseStudySystem;
	tmpCaseStudySystem.ReadMetaData(fullPathToMetaFile, nullptr, false);
	return std::make_pair(tmpCaseStudySystem.CaseStudyName(), tmpCaseStudySystem.CaseStudyInfo());
}

void CFmiCaseStudyDlg::OnBnClickedButtonCloseMode()
{
	CWaitCursor waitCursor;
	itsSmartMetDocumentInterface->CaseStudyToNormalMode();
	UpdateButtonStates();
	itsSmartMetDocumentInterface->SetAllViewIconsDynamically();
}

// Tätä kutsutaan aina tietyissä tilanteissa (initialisointi, Store- ja Close Mode- nappien handlauksen lopuksi).
// Tarkoitus on enabloida ja disabloida Store- ja Close Mode- nappeja.
// Store-nappi on disabloitu, jos SmartMet on CaseStudy-moodissa.
// Kun Store-nappia on painettu, mennään erikois tilaan, eli disabloidaan Store-nappi ja käynnistyy timer, 
// jonka avulla katsotaan onko CaseStudyExe-tallentamassa dataa, kun ei enää ole tätä ohjelmaa käynnissä, voidaan Store-nappi enabloida (jos ei olla CaseStudy-moodissa tietenkin).
// Close Mode-nappi on disabloitu, jos ei olla CaseStudy-moodissa.
void CFmiCaseStudyDlg::UpdateButtonStates()
{
	if(itsSmartMetDocumentInterface->CaseStudyModeOn())
	{
		EnableButton(IDC_BUTTON_STORE_DATA, FALSE);
		EnableButton(IDC_BUTTON_CLOSE_MODE, TRUE);
	}
	else
	{
		if(IsThereCaseStudyMakerRunning() && itsDisableStoreButtonTimer == 0)
		{
			EnableButton(IDC_BUTTON_STORE_DATA, FALSE);
			itsDisableStoreButtonTimer = static_cast<UINT>(SetTimer(kFmiDisableStoreButtonTimer, 2000, NULL)); // käynnistetään timer, joka kahden sekunnin välein tarkistaa, voidaanko Store-nappi taas enabloida
		}
		else if(itsDisableStoreButtonTimer == 0)
			EnableButton(IDC_BUTTON_STORE_DATA, TRUE);
		EnableButton(IDC_BUTTON_CLOSE_MODE, FALSE);
	}
}

void CFmiCaseStudyDlg::EnableButton(UINT theButtonId, BOOL state)
{
	CWnd *button = GetDlgItem(theButtonId);
	if(button)
		button->EnableWindow(state);
}


void CFmiCaseStudyDlg::OnBnClickedCheckEditEnableData()
{
    ShowEnableColumn();
}

void CFmiCaseStudyDlg::ShowEnableColumn()
{
    static int lastTrueEnableColumnWidth = 0;

	UpdateData(TRUE);

    for(int i = 0; i < itsGridCtrl.GetRowCount(); i++)
    {
        if(fEditEnableData)
            itsGridCtrl.SetColumnWidth(CaseStudyHeaderParInfo::kEnableData, lastTrueEnableColumnWidth); // otetaan talessa ollut viimeinen leveys käyttöön kun sarake näytetään taas
        else
        {
            if(i == 0)
                lastTrueEnableColumnWidth = itsGridCtrl.GetColumnWidth(CaseStudyHeaderParInfo::kEnableData); // otetaan sarakkeen leveys talteen ennen piilotusta
            itsGridCtrl.SetColumnWidth(CaseStudyHeaderParInfo::kEnableData, 0);
        }
    }

    UpdateRows(1, 1, true);
    itsGridCtrl.EnableHiddenColUnhide(FALSE);
    UpdateEditEnableDataText();
	AdjustGridControl();
	UpdateData(FALSE);
	Invalidate(FALSE);
}


HBRUSH CFmiCaseStudyDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if(pWnd && pWnd->GetDlgCtrlID() == IDC_CHECK_EDIT_ENABLE_DATA)
    {
        CFmiWin32Helpers::SetErrorColorForTextControl(pDC, !fEditEnableData);
    }

    return hbr;
}


void CFmiCaseStudyDlg::OnBnClickedButtonRefreshGrid()
{
    auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
    caseStudySystem.UpdateNoProducerData(*itsSmartMetDocumentInterface->HelpDataInfoSystem(), *itsSmartMetDocumentInterface->InfoOrganizer());
	caseStudySystem.PutNoneProducerDataToEndFix();
	caseStudySystem.FillCaseStudyDialogData(*itsProducerSystemsHolder);
    InitGridControlValues();
	UpdateData(FALSE);
}

std::list<std::string> CFmiCaseStudyDlg::GetSelectedDataFileFilters()
{
	std::vector<int> selectedRowIndexies;
	// Haetaan valittujen rivien numerot listaan
	auto fixedRowCount = itsGridCtrl.GetFixedRowCount();
	auto rowCount = itsGridCtrl.GetRowCount();
	for(int rowIndex = fixedRowCount; rowIndex < rowCount; rowIndex++)
	{
		if(itsGridCtrl.IsCellSelected(rowIndex, 1))
		{
			selectedRowIndexies.push_back(rowIndex);
		}
	}

	std::list<std::string> selectedDataFileFilters;
	auto& caseStudyDialogData = itsSmartMetDocumentInterface->CaseStudySystem().CaseStudyDialogData();
	for(auto selectedRowIndex : selectedRowIndexies)
	{
		auto usedRowIndex = selectedRowIndex - 1;
		if(usedRowIndex >= 0 && usedRowIndex < caseStudyDialogData.size())
		{
			const auto* rowData = caseStudyDialogData[usedRowIndex];
			if(!rowData->FileFilter().empty())
			{
				selectedDataFileFilters.push_back(rowData->FileFilter());
			}
		}
	}
	return selectedDataFileFilters;
}

void CFmiCaseStudyDlg::OnBnClickedButtonPrioritizeData()
{
	QueryDataToLocalCacheLoaderThread::AddPrioritizedDataLoadWork(GetSelectedDataFileFilters());
}


void CFmiCaseStudyDlg::OnBnClickedButtonLoadOldData()
{
	QueryDataToLocalCacheLoaderThread::AddLoadOldDataWork(GetSelectedDataFileFilters());
}
