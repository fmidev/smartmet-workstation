#ifdef _MSC_VER
#pragma warning(disable : 4786 4996) // poistaa n kpl VC++ k‰‰nt‰j‰n "liian pitk‰ tyyppi nimi" varoitusta
#endif

#include "NFmiEditMapGeneralDataDoc.h"
#include "NFmiGrid.h"
#include "NFmiDrawParamList.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiMenuItemList.h"
#include "NFmiMenuItem.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiSmartInfo.h"
#include "NFmiDrawParam.h"
#include "NFmiAreaMaskList.h"
#include "NFmiAreaMask.h"
#include "NFmiLocationSelectionTool.h"
#include "NFmiEditMapDataListHandler.h" //laura lis‰si 30081999
#include "NFmiFileString.h"
#include "NFmiCombinedParam.h"
#include "NFmiEditorControlPointManager.h"
#include "NFmiProducerName.h"
#include "NFmiDataLoadingInfo.h"
#include "NFmiMilliSecondTimer.h"
#include "NFmiDataModifierClasses.h"
#include "NFmiCalculatedAreaMask.h"
#include "NFmiValueString.h"
#include "NFmiProjectionCurvatureInfo.h"
#include "NFmiQueryDataUtil.h"

#include "stdafx.h"
#include "NFmiFileSystem.h"
#include "NFmiSmartToolModifier.h"
#include "NFmiSmartToolInfo.h"
#include "NFmiStreamQueryData.h"
#include "NFmiGdiPlusImageMapHandler.h"

#include "NFmiCommentStripper.h"
#include "NFmiProducerIdLister.h"
#include "NFmiMetEditorModeDataWCTR.h"
#include "NFmiSettings.h"
#include "NFmiGeoShape.h"
#include "NFmiViewSettingMacro.h"
#include "NFmiAreaFactory.h"
#include "NFmiTimeList.h"
#include "NFmiMapViewCache.h"
#include "NFmiTotalWind.h"

#include "FmiMaskOperationDlg.h"
#include "DataLoadingProblemsDlg.h"
#include "SaveDataDlg.h"
#include "FmiCPModifyingDlg.h"
#include "FmiModifyDrawParamDlg.h"
#include "NFmiMacroParamSystem.h"
#include "NFmiMacroParamItem.h"
#include "NFmiMacroParamFolder.h"
#include "NFmiMacroParam.h"
#include "NFmiCrossSectionSystem.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiSynopStationPrioritySystem.h"
#include "FmiMacroPathSettings.h"
#include "NFmiObsComparisonInfo.h"
#include "NFmiMetEditorOptionsData.h"
#include "NFmiMTATempSystem.h"
#include "FmiLanguageSelectionDlg.h"
#include "NFmiTrajectorySystem.h"
#include "NFmiTEMPCode.h"
#include "NFmiFileCleanerSystem.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiProducerSystem.h"
#include "NFmiSmartToolIntepreter.h"
#include "NFmiHelpEditorSystem.h"
#include "NFmiMapViewDescTop.h"
#include "NFmiGridPointCache.h"
#include "NFmiMapConfigurationSystem.h"
#include "NFmiQ2Client.h"
#include "NFmiConceptualModelData.h"
#include "NFmiSatelView.h"
#include "NFmiWindTableSystem.h"
#include "NFmiDataNotificationSettingsWinRegistry.h"
#include "NFmiAutoComplete.h"
#include "NFmiModelDataBlender.h"
#include "NFmiDataStoringHelpers.h"
#include "NFmiAnalyzeToolData.h"
#include "NFmiDataQualityChecker.h"
#include "FmiQueryDataCacheLoaderThread.h"
#include "FmiCombineDataThread.h"
#include "NFmiIgnoreStationsData.h"
#include "NFmiApplicationDataBase.h"

#include "NFmiMacroPathSettings.h"
#include "FmiWin32DirectoryUtils.h"
#include "NFmiCaseStudySystem.h"
#include "NFmiAviationStationInfoSystem.h"
#include "NFmiGenDocDataAdapter.h"
#include "TimeSerialModification.h"
#include "FmiOperationProgressDlg.h"
#include "SmartMetToolboxDep_resource.h"
#include "NFmiBasicSmartMetConfigurations.h"
#include "NFmiLatLonArea.h"
#include "NFmiCPManagerSet.h"
#include "NFmiApplicationWinRegistry.h"
#include "NFmiConceptualDataView.h"
#include "NFmiCapView.h"
#include "MultiProcessClientData.h"
#include "NFmiMultiProcessPoolOptions.h"
#include "NFmiFixedDrawParamSystem.h"
#include "NFmiSatelliteImageCacheSystem.h"
#include "NFmiBetaProductSystem.h"
#include "NFmiBetaProductHelperFunctions.h"
#include "NFmiEditDataUserList.h"
#include "NFmiStationIndexTextView.h"
#include "NFmiInfoAreaMaskOccurrance.h"
#include "HakeMessage/Main.h"
#include "NFmiSeaIcingWarningSystem.h"
#include "NFmiExtraMacroParamData.h"
#include "NFmiStation2GridMask.h"
#include "ToolboxViewsInterfaceForGeneralDataDoc.h"
#include "SpecialDesctopIndex.h"
#include "NFmiFastInfoUtils.h"
#include "CtrlViewFunctions.h"
#include "NFmiTrajectory.h"
#include "CtrlViewDocumentInterfaceForGeneralDataDoc.h"
#include "SmartMetDocumentInterfaceForGeneralDataDoc.h"
#include "NFmiMapViewTimeLabelInfo.h"
#include "CtrlViewFastInfoFunctions.h"
#include "ParameterSelectionSystem.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "QueryDataReading.h"
#include <direct.h> // working directory juttuja varten
#include "psapi.h" // ei lˆytynyt toshibasta, pit‰‰ korjata tilanne!!!
#include "NFmiPathUtils.h"
#include "Q2ServerInfo.h"
#include "SettingsFunctions.h"
#include "ApplicationInterface.h"
#include "MacroParamDataChecker.h"
#include "CapDataSystem.h"
#include "NFmiMacroParamDataCache.h"

#ifdef OLDGCC
 #include <strstream>
#else
 #include <sstream>
#endif

#include <ctime> // srand:ia varten!!!
#include <algorithm> // srand:ia varten!!!
#include <fstream>
#include "boost/unordered_map.hpp"
#include "boost/math/special_functions/round.hpp"
#include "boost/filesystem.hpp"

#ifdef _MSC_VER
#pragma warning (disable : 4244 4267) // boost:in thread kirjastosta tulee ik‰v‰sti 4244 varoituksia
#endif
#include <boost/thread.hpp>
#include "execute-command-in-separate-process.h"

#ifndef DISABLE_CPPRESTSDK
#include "WmsSupport.h"
#include "CapabilitiesHandler.h"
#include "CapabilityTree.h"
#endif // DISABLE_CPPRESTSDK

#include <functional>
#ifdef _MSC_VER
#pragma warning (default : 4244 4267) // laitetaan 4244 takaisin p‰‰lle, koska se on t‰rke‰ (esim. double -> int auto castaus varoitus)
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// #define SETTINGS_DUMP 1

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace
{
	// T‰m‰ palauttaa aina kopion, mik‰ on threadi turvallisempaa.
	// Threadi-lukko takaa ett‰ kopioita pyydet‰‰n vain yksi kerrallaan
	typedef boost::shared_mutex MutexType;
	typedef boost::shared_lock<MutexType> ReadLock; // Read-lockia ei oikeasti tarvita, mutta laitan sen t‰h‰n, jos joskus tarvitaankin
	typedef boost::unique_lock<MutexType> WriteLock;
	MutexType gParamMaskListMutex;

    const std::string tmMasterTcpProcessName = "MasterProcessTcpMFC.exe";
    const std::string tmWorkerTcpProcessName = "WorkerProcessTcpMFC.exe";
}

static void SetFastInfoToZero(boost::shared_ptr<NFmiFastQueryInfo> &theOwnerInfo)
{
	theOwnerInfo = boost::shared_ptr<NFmiFastQueryInfo>();
}

class ObsDataReport
{
public:
	NFmiProducer itsProd;
	NFmiMetTime itsLastDataTime;
	int itsTimeStepInMinutes;
	std::string itsReportLabelStr;
	std::string itsReportTimeFormat;
};

// Tieto havainto tuottajista, mik‰ on viimeisin ajanhetki, milt‰ lˆytyy dataa.
// Tein oman luokan, joka yhdit‰‰ tuottajien tidon, jos samalta tuottajalta on useita data l‰hteit‰.
class ObsDataLoadedReporter
{
public:
typedef std::map<unsigned long, ObsDataReport> Container;

	ObsDataReport* Find(unsigned long theProdId)
	{
		Container::iterator it = itsReporters.find(theProdId);
		if(it != itsReporters.end())
			return &(it->second);
		else
			return 0;
	}

	void Add(unsigned long theProdId, const ObsDataReport &theObsDataReport)
	{
		itsReporters.insert(std::make_pair(theProdId, theObsDataReport));
	}
private:
	Container itsReporters;
};

// T‰m‰ luokka on tehty korvaamaan NFmiDrawParamList-luokka kun sit‰ k‰ytet‰‰n
// dokumnetti luokassa itsModifiedPropertiesDrawParamList toiminnassa.
// T‰st‰ on tullut liian hidas, koska listassa on tuhansia drawParameja. Etsint‰ on ollut
// aina lineaarinen ja listasta etsit‰‰ kokoajan tavaraa, kun tulee uutta dataa ja
// sit‰ tulee jatkuvasti.
// Toteutin luokan hash table:na eli hakuaika on periaatteessa aina vakio.
// K‰ytet‰‰n containerin toteutuksena boost:in unordered_map -luokkaa.
class FastDrawParamList
{
public:
typedef boost::unordered_map<std::string, boost::shared_ptr<NFmiDrawParam> > FastContainer;

	FastDrawParamList(void)
	:itsContainer()
	,itsCurrentKey()
	{
	}

	~FastDrawParamList(void)
	{}

	bool Find(boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool fGroundData)
	{
		itsCurrentKey = "";
        FastContainer::key_type tmpKey = CalcKey(theDrawParam, fGroundData);
		FastContainer::iterator it = itsContainer.find(tmpKey);
		if(it != itsContainer.end())
		{
			itsCurrentKey = tmpKey;
			return true;
		}
		else
			return false;
	}
	bool Find(const NFmiDataIdent& theParam, const NFmiLevel* theLevel, NFmiInfoData::Type theDataType, const std::string &theMacroParamInitFile, bool fUseOnlyParamId)
	{
		itsCurrentKey = "";
		FastContainer::key_type tmpKey = CalcKey(theParam, theLevel, theDataType, theMacroParamInitFile, fUseOnlyParamId);
		FastContainer::iterator it = itsContainer.find(tmpKey);
		if(it != itsContainer.end())
		{
			itsCurrentKey = tmpKey;
			return true;
		}
		else
			return false;
	}
	void Add(boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool fGroundData)
	{
		itsContainer.insert(FastContainer::value_type(CalcKey(theDrawParam, fGroundData), boost::shared_ptr<NFmiDrawParam>(theDrawParam)));
	}
	boost::shared_ptr<NFmiDrawParam> Current(void)
	{
		boost::shared_ptr<NFmiDrawParam> tmpDrawParam;
		try
		{
			tmpDrawParam = itsContainer.at(itsCurrentKey);
		}
		catch(...)
		{
		}
		return tmpDrawParam;
	}
	size_t Size(void)
	{
		return itsContainer.size();
	}
	FastContainer::iterator Begin(void)
	{
		return itsContainer.begin();
	}
	FastContainer::iterator End(void)
	{
		return itsContainer.end();
	}

private:
	// Laskee string avaimen, miss‰ stringi laskettu seuraavasti:
	// 'p' + parid + '_' + 't' + prodid + '_' + 'l' + leveltype + '_' + levelvalue + '_' + 'd' + datatype
	// Jos fUseOnlyParamId on true, j‰‰ pois kohta ['t' + prodid + '_']
	// Jos theLevel on 0-pointteri, j‰‰ pois kohta ['l' + leveltype + '_']
	// Jos kyse on macroParamista, tulee avaimen per‰‰n viel‰ '_'+theMacroParamInitFile
	std::string CalcKey(const NFmiDataIdent& theParam, const NFmiLevel* theLevel, NFmiInfoData::Type theDataType, const std::string &theMacroParamInitFile, bool fUseOnlyParamId)
	{
		std::string aKey;
		aKey += "p";
		aKey += NFmiStringTools::Convert<unsigned long>(theParam.GetParamIdent());
		aKey += "_";
		if(fUseOnlyParamId == false)
		{
			aKey += "t";
			aKey += NFmiStringTools::Convert<unsigned long>(theParam.GetProducer()->GetIdent());
			aKey += "_";
		}
		if(theLevel)
		{
			aKey += "l";
			aKey += NFmiStringTools::Convert<FmiLevelType>(theLevel->LevelType());
			aKey += "_";
			aKey += NFmiStringTools::Convert<float>(theLevel->LevelValue());
			aKey += "_";
		}
		aKey += "d";
		aKey += NFmiStringTools::Convert<NFmiInfoData::Type>(theDataType);
        if(NFmiDrawParam::IsMacroParamCase(theDataType))
		{
			aKey += "_";
			aKey += theMacroParamInitFile;
		}
		return aKey;
	}

    // Tekee syhteellisen macroParamin suhteellisen polun alkuosion ilman tiedoston p‰‰tett‰ (.dpa) esim. 
    // "macroParm1" tai "MinunMakrot\macroParm1"
	std::string MakeMacroParamRelativeFilePathStart(boost::shared_ptr<NFmiDrawParam> &theDrawParam)
    {
        std::string relativeFilePath;
        if(!theDrawParam->MacroParamRelativePath().empty())
        {
            relativeFilePath += theDrawParam->MacroParamRelativePath();
            relativeFilePath += kFmiDirectorySeparator;
        }
        relativeFilePath += theDrawParam->ParameterAbbreviation();
        return relativeFilePath;
    }

	std::string CalcKey(boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool fGroundData)
	{
		bool useParIdOnly = (theDrawParam->DataType() == NFmiInfoData::kEditable) || (theDrawParam->DataType() == NFmiInfoData::kCopyOfEdited);
		return CalcKey(theDrawParam->Param(), fGroundData ? 0 : &theDrawParam->Level(), theDrawParam->DataType(), MakeMacroParamRelativeFilePathStart(theDrawParam), useParIdOnly);
	}

	FastContainer itsContainer;
	FastContainer::key_type itsCurrentKey; // findilla etsityn drawParamin avain (iteraattoria ei saa ottaa talteen koska se voi invalidoitua!!!!)

};

static const std::string g_ObservationMenuName = "Observation";

static const std::string gDummyParamName = "dummyName";

static const char *CONFIG_MAPSYSTEMS = "MetEditor::MapSystems";
static const char *CONFIG_MAPSYSTEM_PROJECTION_FILENAME = "MetEditor::MapSystem::%s::Projection::Filename";
static const char *CONFIG_MAPSYSTEM_PROJECTION_DEFINITION = "MetEditor::MapSystem::%s::Projection::Definition";
static const char *CONFIG_MAPSYSTEM_MAP = "MetEditor::MapSystem::%s::Map";
static const char *CONFIG_MAPSYSTEM_MAP_FILENAME = "MetEditor::MapSystem::%s::Map::%s::Filename";
static const char *CONFIG_MAPSYSTEM_MAP_DRAWINGSTYLE = "MetEditor::MapSystem::%s::Map::%s::DrawingStyle";
static const char *CONFIG_MAPSYSTEM_LAYER = "MetEditor::MapSystem::%s::Layer";
static const char *CONFIG_MAPSYSTEM_LAYER_FILENAME = "MetEditor::MapSystem::%s::Layer::%s::Filename";
static const char *CONFIG_MAPSYSTEM_LAYER_DRAWINGSTYLE = "MetEditor::MapSystem::%s::Layer::%s::DrawingStyle";

static const NFmiPoint kMissingLatlon(kFloatMissing, kFloatMissing);

void Tokenize(const std::string& str, // TODO Move
			  std::vector<std::string>& tokens,
			  const std::string& delimiters = " ")
{
    // Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}


const int gActiveViewRowIndexForTimeSerialView = 99; // h‰t‰inen korjaus, korjaamaan ongelmaa, mik‰ tuli kun parametrien ominaisuuksia
													// muutettiin aikasarjadialogista k‰sin (piti keksi‰ oma rivi-indeksi ett‰ voidaan
													// hakea aikasarjaikkunan drawparamlistista oikea drawparami)

static const double gUsedEpsilon = std::numeric_limits<double>::epsilon() * 3; // t‰m‰ pit‰‰ olla pieni arvo (~epsilon) koska muuten pienet rajat eiv‰t toimi, mutta pelkk‰ epsilon on liian pieni

using namespace std;

//template<typename T>
struct PointerDestroyer // t‰ll‰ on helppo tuhota dynaamista dataa containereista
{
	template<typename T>
	void operator()(T* thePtr)
	{delete thePtr;}
};

// **********************************************************
// ************ UUSI PIMPL idiooma **************************
// **********************************************************

class GeneralDocImpl
{
public:

GeneralDocImpl(unsigned long thePopupMenuStartId)
:itsListHandler(0)
,itsTimeEditingMode(kFmiTimeEditManual)
,itsSmartInfoOrganizer(0)
,fOpenPopup(false)
,itsPopupMenu(0)
,itsPopupMenuStartId(thePopupMenuStartId)
,itsLocationSelectionTool2(0)
,itsFileDialogDirectoryMemory()
,itsTimeSerialViewDrawParamList(0)
,itsModifiedPropertiesDrawParamList()
,itsMapConfigurationSystem()
,itsHelpDataInfoSystem()
,itsCaseStudyHelpDataInfoSystem()
,fCaseStudyModeOn(false)
,itsDataLoadingInfoNormal()
,itsDataLoadingInfoCaseStudy()
,fUseMasksInTimeSerialViews(true)
,itsToolTipRowIndex(-1)
,itsToolTipColumnIndex(-1)
,itsToolTipTime()
,itsDefaultEditedDrawParam()
,itsParamMaskListMT()
,itsFilterFunction(1)
,itsTestFilterUsedMask(NFmiMetEditorTypes::kFmiNoMask)
,itsActiveViewTime()
,fUseMasksWithFilterTool(false)
,itsAreaFilterRangeLimits(-4, -4 , 4, 4)
,itsAreaFilterRangeStart(0, 0, 0, 0)
,itsAreaFilterRangeEnd(0, 0, 0, 0)
,itsTimeFilterLimits(-6, 6)
,itsTimeFilterRangeStart(0, 0)
,itsTimeFilterRangeEnd(0, 0)
,itsFilteringParameterUsageState(0)
,itsFilteringParamBag()
,itsClipBoardData(0)
,fMouseCapturedInTimeWindow(false)
,fMouseCaptured(false)
,itsModifyToolMode(CtrlViewUtils::kFmiEditorModifyToolModeNormal)
,itsUpdateRect()
,itsBrushSize(20, 20)
,itsBrushValue(15)
,itsBrushSpecialParamValue(0)
,fUseMaskWithBrush(0)
,fViewBrushed(false)
,itsActiveViewRect()
,fLeftMouseButtonDown(false)
,fRightMouseButtonDown(false)
,fMiddleMouseButtonDown(false)
,itsLastBrushedViewTime()
,itsLastBrushedViewRow(0)
,fUseTimeInterpolation(false)
,itsMetEditorOptionsData()
,itsCPManagerSet()
,fPrinting(false)
,itsCurrentViewRowIndex(1)
,itsCurrentCrossSectionRowIndex(1)
,fCPDataBackupUsed(false)
,itsBrushToolLimitSetting(0)
,itsBrushToolLimitSettingValue(0)
,itsTimeEditSmootherValue(0)
,itsTimeEditSmootherMaxValue(6)
,itsShowHelperDataInTimeSerialView(1) // 1= n‰yt‰ apu dataa aikasarjassa
,fShowHelperData3InTimeSerialView(false)
,fShowHelperData4InTimeSerialView(false)
,fActivateParamSelectionDlgAfterLeftDoubleClick(false)
,itsDrawDifferenceDrawParam()
,itsSelectedGridPointDrawParam()
,itsSelectedGridPointLimit(100)
,itsSmartToolInfo()
,itsLandBorderShapeFile()
,itsLandBorderGeoShape()
,itsLandBorderPath()
,itsPacificLandBorderPath()
,itsCutLandBorderPaths()
,itsHelperViewMacro()
,itsViewMacroPath()
,itsRootViewMacroPath()
,itsCurrentViewMacro(new NFmiViewSettingMacro())
,itsDoAtSendCommandString()
,fUseDoAtSendCommand(false)
,itsSmartMetEditingMode(CtrlViewUtils::kFmiEditingModeNormal)
,itsSavedDirectory()
,itsCurrentMacroText()
,itsMacroParamSystem()
,itsCrossSectionSystem(CtrlViewUtils::MaxViewGridYSize)
,itsPossibleUsedDataLoadingGrid(0)
,itsSynopStationPrioritySystem()
,itsSynopPlotSettings()
,fMapMouseDragPanMode(false)
,itsObsComparisonInfo()
,itsCrossSectionDrawParamListVector(0)
,itsMTATempSystem()
,fTimeSerialDataViewOn(false)
,fSynopDataGridViewOn(false)
,itsSynopHighlightStationWmoId(-1)
,fShowSynopHighlightStationMarker(false)
,itsSynopHighlightLatlon()
,fMouseOnMapView(false)
,itsLastSelectedSynopWmoId(-1)
,fShowMouseHelpCursorsOnMap(false)
,itsTrajectorySystem(0)
,fUseEditedDataParamDescriptor(false)
,itsEditedDataParamDescriptor()
,itsLastTEMPDataStr()
,fIsTEMPCodeSoundingDataAlsoCopiedToEditedData(false)
,itsFileCleanerSystem()
,itsWmoStationInfoSystem(true, false)
,fWmoStationInfoSystemInitialized(false)
#ifndef DISABLE_CPPRESTSDK
,itsWarningCenterSystem()
#endif DISABLE_CPPRESTSDK
,itsProducerSystem()
,itsObsProducerSystem()
,itsSatelImageProducerSystem()
,itsHelpEditorSystem()
,itsMapViewDescTopList()
,itsCrossSectionViewSizeInPixels()
,fWarnIfCantSaveWorkingFile(false)
,fUseOnePressureLevelDrawParam(false)
,fRawTempRoundSynopTimes(false)
,itsRawTempUnknownStartLonLat()
,fMustDrawTimeSerialView(false)
,fMustDrawTempView(false)
,fMustDrawCrossSectionView(false)
,itsQ2Client()
,itsStationDataGridSize(70, 70)
,itsExtraSoundingProducerList()
,itsDataToDBCheckMethod(2)
,itsOutOfEditedAreaTimeSerialPoint(kFloatMissing, kFloatMissing)
,itsConceptualModelData()
,itsSeaIcingWarningSystem()
,itsCopyPasteDrawParam()
,fCopyPasteDrawParamAvailableYet(false)
,itsTransparencyContourDrawView(0)
,itsSynopDataFilePatternSortOrderVector()
,itsShowToolTipTimeView(true)
,itsShowToolTipTempView(true)
,itsShowToolTipTrajectoryView(true)
,itsMapViewTimeLabelInfo()
,itsTimeSerialViewTimeBag()
,itsCopyPasteDrawParamList()
,itsCopyPasteCrossSectionDrawParamList()
,fCopyPasteCrossSectionDrawParamListUsedYet(false)
,fCopyPasteDrawParamListUsedYet(false)
,itsCopyPasteDrawParamListVector()
,fCopyPasteDrawParamListVectorUsedYet(false)
,itsTimeSerialViewSizeInPixels()
,itsWindTableSystem()
,fStartUpDataLoadCheckDone(false)
,itsUndoRedoViewMacroList()
,itsUndoRedoViewMacroListPointer(-1)
,itsAutoComplete()
,itsObsDataLoadedReporter()
,itsGeneralColors()
,itsModelDataBlender()
,itsAnalyzeToolData()
,itsDataQualityChecker()
,itsSatelDataRefreshTimerInMinutes(0)
,itsIgnoreStationsData()
,fEditedDataNeedsToBeLoaded(false)
,itsEditedDataNeedsToBeLoadedTimer()
,fEditedDataNotInPreferredState(false)
,itsMacroPathSettings()
,itsMacroDirectoriesSyncronizationCounter(1)
,itsCaseStudySystem()
,itsCaseStudySystemOrig()
,itsLoadedCaseStudySystem()
,fUseMultiThreaddingWithEditingtools(true)
,fDataModificationInProgress(false)
,itsBasicConfigurations()
,fEditedPointsSelectionChanged(false)
,fDrawSelectionOnThisView(false)
,itsCPGridCropRect()
,fUseCPGridCrop(false)
,itsCPGridCropLatlonArea()
,itsCPGridCropInnerLatlonArea()
,itsCPGridCropMargin()
,itsApplicationWinRegistry()
,itsActiveMapDescTopIndex(0)
,fStoreLastLoadedFileNameToFile(false)
,itsMultiProcessClientData()
,itsMultiProcessLogFilePath()
,itsMultiProcessLogFilePattern()
,itsMultiProcessPoolOptions()
,itsFixedDrawParamSystem()
,itsSatelliteImageCacheSystemPtr(new NFmiSatelliteImageCacheSystem)
,itsImageCacheUpdateData()
,itsImageCacheUpdateDataMutex()
,itsBetaProductionSystem()
,itsCurrentGeneratedBetaProduct(nullptr)
,itsLastEditedDataSendTime(NFmiMetTime::gMissingTime)
,fLastEditedDataSendHasComeBack(false)
,itsEditDataUserList()
#ifndef DISABLE_CPPRESTSDK
,wmsSupport()
#endif // DISABLE_CPPRESTSDK
,parameterSelectionSystem()
{
	NFmiRect bsRect1(0.,0.,1.,0.905);
	NFmiRect bsRect2(0.,0.,1.,0.91);
	NFmiRect bsRect3(0.,0.,1.,0.915);
	NFmiRect bsRect4(0.,0.,1.,0.902);
	NFmiRect bsRect5(0.,0.,1.,0.904);
	NFmiRect bsRect6(0.,0.,1.,0.907);

	// Seed the random-number generator with current time so that
	// the numbers will be different every time we run. 
	// Random numbers are used in smarttools (RAND-unction) and in trajectories.
	srand( static_cast<unsigned int>(time( NULL ))); // mahd. satunnais funktion k‰ytˆn takia, pit‰‰ 'sekoittaa' random generaattori
}

~GeneralDocImpl(void)
{
	delete itsListHandler;
	delete itsSmartInfoOrganizer;
	delete itsTimeSerialViewDrawParamList;
	delete itsPopupMenu;
	delete itsLocationSelectionTool2;
	if(itsCrossSectionDrawParamListVector)
	{
		itsCrossSectionDrawParamListVector->Clear(true);
		delete itsCrossSectionDrawParamListVector;
	}
	delete itsClipBoardData;
	delete itsProjectionCurvatureInfo;

	delete itsPossibleUsedDataLoadingGrid;
	delete itsTrajectorySystem;

	std::for_each(itsMapViewDescTopList.begin(), itsMapViewDescTopList.end(), PointerDestroyer());

	itsCopyPasteDrawParamListVector.Clear(true);
}

// tein pikaviritys funktion ett‰ saisin t‰m‰n timebagin kaivettua dokumentin syˆvereist‰
const NFmiTimeBag& MaximalCoverageTimeBag(void)
{
	return GetUsedDataLoadingInfo().MaximalCoverageTimeBag();
}

// tein pikaviritys funktion ett‰ saisin t‰m‰n editormodedatan kaivettua dokumentin syˆvereist‰
NFmiMetEditorModeDataWCTR* EditorModeDataWCTR(void)
{
	return GetUsedDataLoadingInfo().MetEditorModeDataWCTR();
}

// Lokitetaan annettu viesti vain jos SmartMet on verbose tilassa (-v optio k‰ynnistyksess‰)
void DoVerboseLogReporting(const std::string &theLogMessage)
{
    if(itsBasicConfigurations.Verbose())
        LogMessage(theLogMessage, CatLog::Severity::Debug, CatLog::Category::Configuration);
}

void DoVerboseFunctionStartingLogReporting(const std::string &theFunctionName)
{
    DoVerboseLogReporting(std::string("Starting ") + theFunctionName + " -function");
}

bool Init(const NFmiBasicSmartMetConfigurations &theBasicConfigurations, std::map<std::string, std::string> &mapViewsPositionMap, std::map<std::string, std::string> &otherViewsPositionPosMap)
{
/* // t‰m‰ on remote debug sessiota varten MSVC71 feature
	for(int remoteDebug = 1; remoteDebug == 1; )
	{
		int x = 1;
	}
*/
	itsBasicConfigurations = theBasicConfigurations; // kopsataan CSmartMetApp:issa alustettu perusasetus GenDocin dataosaan

	LogMessage("SmartMet document initialization starts...", CatLog::Severity::Info, CatLog::Category::Configuration);
	// Laitetaan peruskonffihakemisto lokiin n‰kyviin
	string infoStr("SmartMet uses configurations from base configuration file: ");
	infoStr += itsBasicConfigurations.BaseConfigurationFilePath();
	LogMessage(infoStr, CatLog::Severity::Info, CatLog::Category::Configuration);

    InitSpecialFileStoragePath();
    CheckRunningStatusAtStartup(SpecialFileStoragePath()); // tarkistaa, onko edellisen kerran ohjelma suljettu hallitusti

	InitMachineThreadCount();
	InitMacroPathSettings(); // pit‰‰ olla ennen InfoOrganizer + SmartToolInfo + ViewMacroSystem alustuksia!!!
	InitInfoOrganizer(); // pit‰‰ alustaa ennen InitSettingsFromConfFile-metodia
	InitProjectionCurvatureInfo();
	InitHelpEditorSystem(); // t‰t‰ pit‰‰ kutsua ennen InitHelpDataInfoSystem-metodia
	InitHelpDataInfoSystem(); // T‰t‰ pit‰‰ kutsua ennen InitApplicationWinRegistry -kutsua!
    InitApplicationWinRegistry(mapViewsPositionMap, otherViewsPositionPosMap);

	InitSettingsFromConfFile();
    MakeMultiProcessLogPathValues();
    InitSynopStationPrioritySystem();
    InitSynopPlotSettings();
    InitObsComparisonInfo();
    InitMapConfigurationSystemMain();
    InitDataToDBCheckMethodMain();
	InitOptionsData();

    DoVerboseFunctionStartingLogReporting("NFmiEditMapDataListHandler::Init");
	itsListHandler = new NFmiEditMapDataListHandler();
	itsListHandler->Init(HelpDataPath());

    InitFixedDrawParamSystem();
	InitMapViewDescTopList(); // t‰m‰ pit‰‰ tehd‰ mm. mapconfiguraatioiden, proj-piirto, landborder-piirto, optioiden luvun jne. alustusten j‰lkeen
	InitLandBorderDrawingSystem();
	InitCrossSectionDrawParamListVector();
	InitParamMaskList();
	LoadFileDialogDirectoryMemory();
	InitModelProducerSystem(); // producer system pit‰‰ initialisoida ennen trajektori systeemi‰
	InitObsProducerSystem();
	InitSatelImageProducerSystem();
	InitTrajectorySystem();
	InitFileCleanerSystem();
	InitMacroParamData();
	InitSoundingPlotLevels();
	itsWindTableSystem.ControlDirectory(itsBasicConfigurations.ControlPath());
	InitializeGeneralColors();
    InitSatelliteImageCacheTotal(); // Pit‰‰ alustaa InitHelpDataInfoSystem -kutsun j‰lkeen
    InitDataLoadingInfo();
    InitMetEditorModeDataWCTR();

	itsLocationSelectionTool2 = new NFmiLocationSelectionTool;
	SelectNewParamForSelectionTool(kFmiTopoGraf);

	InitEditedDataParamDescriptor(); // pit‰‰ olla itsDataLoadingInfoManager -otuksen luomisen j‰lkeen
	InitWarningCenterSystem(); // t‰m‰n initialisointi pit‰‰ olla itsDataLoadingInfoManager-olion initialisoinnin per‰ss‰
	InitSeaIcingWarningSystem();
	InitWindTableSystem();
	InitExtraSoundingProducerListFromSettings();
    InitMTATempSystem(); // pit‰‰ kutsua vasta InitProducerSystem- ja InitExtraSoundingProducerListFromSettings -kutsujen j‰lkeen

	InitCPManagerSet();
	InitDrawDifferenceDrawParam();
	InitSmartToolInfo();
	InitMacroParamSystem(true); // macroParamsystem pit‰‰ initialisoida ennen viewMacroSystemin initialisointia!!!!!!!
	InitViewMacroSystem(true);
	InitConceptualModelData();
	InitAutoComplete();
	InitAnalyzeToolData(); // t‰t‰ pit‰‰ kutsua InitHelpDataInfoSystem-metodin j‰lkeen
	InitDataQualityChecker();

	InitUsedDataLoadingGrid();
	InitSynopDataFilePatternSortOrderVector();
	InitStationIgnoreList();
	InitCrossSectionSystem(); // t‰t‰ on kutsuttava InitApplicationWinRegistry-kutsun j‰lkeen
	UpdateCrossSectionMacroParamDataSize();
	InitSmartToolSystem();
	InitCaseStudySystem();
    InitMirwaSymbolMap();
    InitBetterWeatherSymbolMap();
    InitCapSymbolMap();
    InitSmartSymbolMap();
    InitCustomSymbolMap();
    InitMultiProcessPoolOptions(); // HUOM! t‰m‰ pit‰‰ kutsua vasta 
    LoadCrashBackUpViewMacro();
    InitBetaProductionSystem(); // T‰t‰ on kutsuttava InitMacroPathSettings- ja InitApplicationWinRegistry -metodien kutsujen j‰lkeen!!
    InitEditDataUserList();
    InitGriddingCallback();

	// Asetetaan alustuksessa jonkinlainen timeDescriptor, ennen kuin ladataan mit‰‰n datoja
	NFmiMetTime origTime;
	NFmiMetTime endTime(origTime);
	endTime.ChangeByDays(2);
	NFmiTimeBag validTimebag(origTime, endTime, 60);
	NFmiTimeDescriptor timeDesc(origTime, validTimebag);
	SetCrossSectionSystemTimes(timeDesc);
    InitWmsSupport();
    InitCapData();

    // This must be initialized AFTER all other data systems have been initialized
    // Meaning after modelProducers, obsProducers, satelProducers, wsmSupport, smarttools, etc. have initialized
    InitParameterSelectionSystem();
    InitLogFileCleaning();
    InitMacroParamDataCache();

#ifdef SETTINGS_DUMP // TODO enable this with a command line parameter
	std::string str = NFmiSettings::ToString();
	ofstream settingsfile;
	settingsfile.open("allconfigs.conf");
	settingsfile << str;
	settingsfile.close();
#endif

	LogMessage("SmartMet document initialization ends...", CatLog::Severity::Info, CatLog::Category::Configuration);
	return true;
}

void InitMacroParamDataCache()
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
    try
    {
        // Alustetaan macroParam cache kaikkien karttan‰yttˆjen indekseill‰ (0-2) ja viel‰ poikkileikkausn‰ytˆn indeksill‰
        itsMacroParamDataCache.init({ 0ul, 1ul, 2ul, CtrlViewUtils::kFmiCrossSectionView });
    }
    catch(exception &e)
    {
        LogAndWarnUser(e.what(), "Problems with MacroParamDataCache initialization", CatLog::Severity::Error, CatLog::Category::Configuration, true, false, true);
    }
}

void InitWmsSupport()
{
#ifndef DISABLE_CPPRESTSDK
    wmsSupport.initialSetUp();
    if(!wmsSupport.isConfigured())
    {
        UseWmsMaps(false);
    }
#endif // DISABLE_CPPRESTSDK
}

void InitCapData()
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
    try
    {
        capDataSystem.init();
    }
    catch(exception &e)
    {
        LogAndWarnUser(e.what(), "Problems with CapData initialization", CatLog::Severity::Error, CatLog::Category::Configuration, true, false, true);
    }
}

void InitGriddingCallback()
{
    NFmiStation2GridMask::GriddingFunctionCallBackType callbackFunction = &NFmiStationView::GridStationData;
    NFmiStation2GridMask::SetGriddingStationDataCallback(callbackFunction);
}

void InitLogFileCleaning()
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
    auto logFilePattern = CatLog::baseLogFilePath();
    logFilePattern += "*";
    NFmiFilePatternCleanerInfo info(logFilePattern, 25);
    FileCleanerSystem().Add(info);
}

void InitMetEditorModeDataWCTR()
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
    NFmiMetEditorModeDataWCTR* metEdModeData = EditorModeDataWCTR();
    if(metEdModeData && metEdModeData->EditorMode() != NFmiMetEditorModeDataWCTR::kNormalAutoLoad)
        metEdModeData->UseNormalModeForAWhile(true);
}

void InitDataLoadingInfo()
{
    try
    {
        itsDataLoadingInfoNormal.Configure();
        AddWorkingDataPathToCleaner(); // HUOM! DataLoadingInfo ja FileCleanerSystem molemmat pit‰‰ olla initilaisoituina!!!!
        itsDataLoadingInfoCaseStudy = itsDataLoadingInfoNormal; // otetaan CaseStudy dataLoadingInfon pohjatiedot normaalista dataLoadingInfosta
    }
    catch(exception &e)
    {
        LogAndWarnUser(e.what(), "Problems with data-loading-info settings", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
    }
}

void InitObsComparisonInfo()
{
    try
    {
        DoVerboseFunctionStartingLogReporting(__FUNCTION__);
        itsObsComparisonInfo.Init();
    }
    catch(exception &e)
    {
        LogAndWarnUser(e.what(), "Problems with obs_comparison settings", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
    }
}

void InitSynopPlotSettings()
{
    try
    {
        DoVerboseFunctionStartingLogReporting(__FUNCTION__);
        itsSynopPlotSettings.Init();
    }
    catch(exception &e)
    {
        LogAndWarnUser(e.what(), "Problems with SynopPlot settings", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
    }
}

void InitSynopStationPrioritySystem()
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
    string synopPriorityFile(HelpDataPath());
    synopPriorityFile += "\\synop_plot_priority.dat";
    synopPriorityFile = PathUtils::getAbsoluteFilePath(synopPriorityFile, itsBasicConfigurations.ControlPath());
    if(itsSynopStationPrioritySystem.Init(synopPriorityFile) == false)
    {
        std::string errMsgStr("Cannot open synop_plot_priority.dat file from:\n");
        errMsgStr += synopPriorityFile;
        LogAndWarnUser(errMsgStr, "Problems with synop_plot_priority.dat", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
    }
}

void InitEditDataUserList(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
    try
    {
        itsEditDataUserList.InitFromSettings(itsBasicConfigurations.ControlPath());
    }
    catch(std::exception &e)
    {
        string errStr("InitEditDataUserList - Initialization error in configurations: \n");
        errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitEditDataUserList", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
    }
}

void MakeMultiProcessLogPathValues(void)
{
    const std::string dateStr("%Y%m%d"); // tiedostoon tulee YYYYMMDD.log -p‰‰te
    itsMultiProcessLogFilePath = itsBasicConfigurations.LogFileDirectory() + "\\"; // t‰m‰ on normaali loki-hakemisto, laitetaan multi-process lokit samaan hakemistoon
    itsMultiProcessLogFilePath += "tm_process_pool_";
    itsMultiProcessLogFilePath += dateStr + ".log"; 

    itsMultiProcessLogFilePattern = itsMultiProcessLogFilePath;
    boost::replace_first(itsMultiProcessLogFilePattern, dateStr, "*");
    try
    {
        NFmiFileSystem::CleanFilePattern(itsMultiProcessLogFilePattern, 15); // siivotaan alustuksen/k‰ynnistyksen yhteydess‰ vanhat lokit pois
    }
    catch(...)
    {
        // NFmiFileSystem::CleanFilePattern -funktio heitt‰‰ poikkeuksen, jos patternin hakemistoa ei ole olemassa, mutta sill‰ ei ole v‰li‰, joten otetaan vain poikkeus kiinni
    }

    itsMultiProcessClientData.MultiProcessLogPath(itsMultiProcessLogFilePath); // alustetaan samalla Multi-process -olion lokitiedosto polku
}

void InitMTATempSystem()
{
    try
    {
        GetMTATempSystem().Init(ProducerSystem(), ExtraSoundingProducerList(), ApplicationWinRegistry().SoundingTextUpward(), ApplicationWinRegistry().SoundingTimeLockWithMapView());
    }
    catch(std::exception &e)
    {
        LogAndWarnUser(string("Sounding settings problem: ") + e.what(), "Problems with sounding view settings", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
    }
}

void InitFixedDrawParamSystem(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
    try
    {
        auto fixedDrawParamInitRootPath = MakeUsedFixedDrawParamsRootPath();
        CatLog::logMessage(std::string("Starting to initilize Fixed DrawParams from directory: ") + fixedDrawParamInitRootPath, CatLog::Severity::Debug, CatLog::Category::Configuration);
        itsFixedDrawParamSystem.Initialize(fixedDrawParamInitRootPath);
        CatLog::logMessage(std::string("Fixed DrawParams initialized with ") + std::to_string(itsFixedDrawParamSystem.FlatDrawParamList().size()) + " drawParams", CatLog::Severity::Debug, CatLog::Category::Configuration);
    }
    catch(std::exception &e)
    {
        string errStr("InitFixedDrawParamSystem - Initialization error in configurations: \n");
        errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitFixedDrawParamSystem", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
    }
}

void InitBetaProductionSystem()
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
    try
    {
        BetaProduct::SetLoggerFunction(GetLogAndWarnFunction());
        itsBetaProductionSystem.Init(ApplicationWinRegistry().BaseConfigurationRegistryPath(), MacroPathSettings().LocalCacheBasePath());

        if(itsBetaProductionSystem.DoCacheSyncronization())
            FirstTimeMacroDirectoryCheck(itsBetaProductionSystem.GetBetaProductionBaseDirectory(false), itsBetaProductionSystem.GetBetaProductionBaseDirectory(true));
    }
    catch(std::exception &e)
    {
        string errStr("InitBetaProductionSystem - Initialization error in configurations: \n");
        errStr += e.what();
        LogAndWarnUser(errStr, "Problems with Beta production system", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
    }
}

void InitMultiProcessPoolOptions(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
        itsMultiProcessPoolOptions.InitFromSettings("SmartMet::MultiProcessPoolOptions");

        // alustetaan t‰ss‰ myˆs itsMultiProcessClientData:n MultiProcessPoolOptions -osio
        itsMultiProcessClientData.PresetMultiProcessPoolOptions(itsMultiProcessPoolOptions.MultiProcessPoolOptions(), itsMultiProcessPoolOptions.LogLevel());
	}
	catch(std::exception &e)
	{
		string errStr("InitMultiProcessPoolOptions - Initialization error in configurations: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitMultiProcessPoolOptions", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

std::string WomlDirectoryPath()
{
    std::string womlDirectory = itsBasicConfigurations.ControlPath() + "\\woml";
    womlDirectory = NFmiSettings::Optional<std::string>("SmartMet::WomlDirectoryPath", womlDirectory);
    womlDirectory = PathUtils::getAbsoluteFilePath(womlDirectory, itsBasicConfigurations.ControlPath());
    return womlDirectory;
}

void InitMirwaSymbolMap(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
        NFmiConceptualDataView::InitMirwaSymbolMap(WomlDirectoryPath());
	}
	catch(std::exception &e)
	{
		string errStr("InitMirwaSymbolMap - Initialization error in configurations: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitMirwaSymbolMap", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

void InitBetterWeatherSymbolMap(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
    try
    {
        NFmiBetterWeatherSymbolView::InitBetterWeatherSymbolMap(WomlDirectoryPath());
    }
    catch(std::exception &e)
    {
        string errStr("InitBetterWeatherSymbolMap - Initialization error in configurations: \n");
        errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitBetterWeatherSymbolMap", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
    }
}

void InitCapSymbolMap(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
    try
    {
        NFmiCapView::InitCapSymbolMap(WomlDirectoryPath());
    }
    catch(std::exception &e)
    {
        string errStr("InitCapSymbolMap - Initialization error in configurations: \n");
        errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitCapSymbolMap", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
    }
}

void InitSmartSymbolMap(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
    try
    {
        NFmiSmartSymbolView::InitSmartSymbolMap(WomlDirectoryPath());
    }
    catch(std::exception &e)
    {
        string errStr("InitSmartSymbolMap - Initialization error in configurations: \n");
        errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitSmartSymbolMap", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
    }
}

void InitCustomSymbolMap(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
    try
    {
        NFmiCustomSymbolView::InitCustomSymbolMap(WomlDirectoryPath());
    }
    catch(std::exception &e)
    {
        string errStr("InitCustomSymbolMap - Initialization error in configurations: \n");
        errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitCustomSymbolMap", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
    }
}

void InitCrossSectionSystem(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		// alustetaan crosssection-systeemi alku- ja loppupisteill‰
        auto &crossSectionView = ApplicationWinRegistry().ConfigurationRelatedWinRegistry().CrossSectionViewWinRegistry();
        NFmiCrossSectionSystem::CrossSectionInitValuesWinReg initValues;
        initValues.itsStartPointStr = crossSectionView.StartPointStr();
        initValues.itsMiddlePointStr = crossSectionView.MiddlePointStr();
        initValues.itsEndPointStr = crossSectionView.EndPointStr();
        initValues.itsAxisValuesDefault.itsLowerEndOfPressureAxis = crossSectionView.AxisValuesDefaultLowerEndValue();
        initValues.itsAxisValuesDefault.itsUpperEndOfPressureAxis = crossSectionView.AxisValuesDefaultUpperEndValue();
        initValues.itsAxisValuesSpecial.itsLowerEndOfPressureAxis = crossSectionView.AxisValuesSpecialLowerEndValue();
        initValues.itsAxisValuesSpecial.itsUpperEndOfPressureAxis = crossSectionView.AxisValuesSpecialUpperEndValue();
        initValues.itsVerticalPointCount = crossSectionView.VerticalPointCount();
        initValues.itsWantedMinorPointCount = crossSectionView.HorizontalPointCount();
        itsCrossSectionSystem.InitializeFromSettings(initValues);
	}
	catch(std::exception &e)
	{
		string errStr("InitCrossSectionSystem - Initialization error in configurations: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitCrossSectionSystem", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

void InitStationIgnoreList(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsIgnoreStationsData.InitFromSettings("SmartMet::IgnoreStations");
	}
	catch(std::exception &e)
	{
		string errStr("InitStationIgnoreList - Initialization error in configurations: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitStationIgnoreList", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

void FixDirectoryName(std::string &theDir, bool fNoSlashAtEnd)
{
    const char backSlash = '\\';
	NFmiStringTools::ReplaceChars(theDir, '/', backSlash); // kenoviivojen suunta pit‰‰ laittaa winkkari tyyliin '/' -> '\'
    NFmiStringTools::ReplaceAll(theDir, "\\\\", "\\"); // mahdolliset kaksois kenoviivat pit‰‰ korvata yksˆis kenoviivoilla "\\" -> "\"
    if(fNoSlashAtEnd)
		NFmiStringTools::TrimR(theDir, backSlash);
	else
	{
		if(theDir[theDir.size()-1] != backSlash)
			theDir.push_back(backSlash);
	}
}

void RecursiveDirectoryCopy(const std::string &theSrcDirX, const std::string &theDestDirX, CatLog::Category logCategory)
{
	std::string srcDir = theSrcDirX;
	std::string destDir = theDestDirX;
	FixDirectoryName(srcDir, true); // HUOM! lopusta pit‰‰ ottaa mahdollinen kenoviiva pois!!!
	FixDirectoryName(destDir, true); // HUOM! lopusta pit‰‰ ottaa mahdollinen kenoviiva pois!!!

	NFmiFileSystem::CreateDirectory(destDir); // HUOM! XP:ss‰ pit‰‰ luoda kohde hakemisto, muuten ei toimi!!!!
	std::string logStr("Copying dir: \"");
	logStr += srcDir;
	logStr += "\"\nto: \""; 
	logStr += destDir;
	logStr += "\""; 
	LogMessage(logStr, CatLog::Severity::Debug, logCategory);
    CString strFolderSrcU_ = CA2T(srcDir.c_str());
    CString strFolderDestU_ = CA2T(destDir.c_str());
    if(CFmiWin32DirectoryUtils::BruteDirCopy(strFolderSrcU_, strFolderDestU_)) // 0 paluu arvo on ok, muut virheit‰
		LogMessage("Directory copy failed", CatLog::Severity::Error, logCategory);
}

void FirstTimeMacroDirectoryCheck(const std::string &theServerDir, const std::string &theLocalDir)
{
	if(NFmiFileSystem::DirectoryExists(theLocalDir) == false)
	{
		RecursiveDirectoryCopy(theServerDir, theLocalDir, CatLog::Category::Macro);
		if(itsMacroDirectoriesSyncronizationCounter <= 1)
			itsMacroDirectoriesSyncronizationCounter = 2; // jos on kopsattu makrohakemistot suoraan serverilta, voidaan jatkossa synkronoinnissa menn‰ lokaali-prefer -tilaan
	}
}

void DoMacroFileInitializations(void)
{
    // HUOM! T‰‰lll‰ pit‰‰ hanskata samat kansiot kuin DoMacroDirectoriesSyncronization -metodissa.

    // 1. Jos lokaali makro cache ei ole k‰ytˆss‰ ei tehd‰ mit‰‰n.
	if(itsMacroPathSettings.UseLocalCache() == false)
		return ;
	// 2. Tarkistetaan vuorotellen jokainen makro-hakemisto, onko se jo olemassa vai ei.
	// Jos on ei tehd‰ mit‰‰n, jos ei , kopioidaan se t‰ss‰ sitten.
	FirstTimeMacroDirectoryCheck(itsMacroPathSettings.SmartToolPath(false), itsMacroPathSettings.SmartToolPath(true));
	FirstTimeMacroDirectoryCheck(itsMacroPathSettings.DrawParamPath(false), itsMacroPathSettings.DrawParamPath(true));
	FirstTimeMacroDirectoryCheck(itsMacroPathSettings.ViewMacroPath(false), itsMacroPathSettings.ViewMacroPath(true));
	FirstTimeMacroDirectoryCheck(itsMacroPathSettings.MacroParamPath(false), itsMacroPathSettings.MacroParamPath(true));
}

void InitMacroPathSettings(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsMacroPathSettings.InitFromSettings("SmartMet::MacroPathSettings", itsBasicConfigurations.WorkingDirectory());
        DoMacroFileInitializations();
    }
	catch(std::exception &e)
	{
		string errStr("InitMacroPathSettings - Initialization error in configurations: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitMacroPathSettings", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

void InitAnalyzeToolData(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsAnalyzeToolData.InitFromSettings("SmartMet::AnalyseToolData");
		itsAnalyzeToolData.SeekPotentialProducersFileFilters(*HelpDataInfoSystem());
	}
	catch(std::exception &e)
	{
		string errStr("InitAnalyzeToolData - Initialization error in configurations: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitAnalyzeToolData", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

void InitDataQualityChecker(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsDataQualityChecker.InitFromSettings("SmartMet::DataQualityChecker");
	}
	catch(std::exception &e)
	{
		string errStr("InitDataQualityChecker - Initialization error in configurations: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitDataQualityChecker", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

void InitCaseStudySystem(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
//		itsCaseStudySystem.InitFromSettings();
		LoadCaseStudyMemory();
		if(HelpDataInfoSystem()->UseQueryDataCache())
		{
			itsCaseStudySystem.SmartMetLocalCachePath(HelpDataInfoSystem()->CacheDirectory());
		}
        itsCaseStudySystemOrig = itsCaseStudySystem;
	}
	catch(std::exception &e)
	{
		string errStr("InitCaseStudySystem - Initialization error: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitCaseStudySystem", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

// HUOM! DataLoadingInfo ja FileCleanerSystem molemmat pit‰‰ olla initilaisoituina!!!!
void AddWorkingDataPathToCleaner(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	// Siivoa 1 vrk vanhemmat datat 
	NFmiDirectorCleanerInfo dCleanInfo(static_cast<char*>(GetUsedDataLoadingInfo().WorkingPath()), 1);
	FileCleanerSystem().Add(dCleanInfo);

	// ja j‰t‰ vain max 25 tiedostoa
	NFmiFilePatternCleanerInfo pattCleanInfo(static_cast<char*>(GetUsedDataLoadingInfo().CreateWorkingFileNameFilter()), 20);
	FileCleanerSystem().Add(pattCleanInfo);
}

void InitSynopDataFilePatternSortOrderVector(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsSynopDataFilePatternSortOrderVector.clear();
		std::string patternList = NFmiSettings::Require<std::string>("MetEditor::HelpData::SynopDataFilePatternSortOrder");
		itsSynopDataFilePatternSortOrderVector = NFmiStringTools::Split<std::vector<std::string> >(patternList, ",");
	}
	catch(std::exception &e)
	{
		string errStr("InitSynopDataFilePatternSortOrderVector - Initialization error: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitSynopDataFilePatternSortOrderVector", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

void InitSmartToolSystem(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		// t‰m‰ pit‰‰ tehd‰ dokumentin initialisoinnin yhteydess‰, ett‰ smarttoolintepreterin tuottaja listat alustetaan varmasti oikein!!!!
        NFmiSmartToolIntepreter intepreter(&itsProducerSystem, &itsObsProducerSystem);
	}
	catch(std::exception &e)
	{
		string errStr("InitSmartToolSystem - Initialization error: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitSmartToolSystemm", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}

}

void InitHelpEditorSystem(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsHelpEditorSystem.InitFromSettings("HelpEditorSystem");
	}
	catch(std::exception &e)
	{
		string errStr("InitHelpEditorSystem - Initialization error in configurations: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitHelpEditorSystem", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

void InitProjectionCurvatureInfo(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsProjectionCurvatureInfo = new NFmiProjectionCurvatureInfo;
		if(itsProjectionCurvatureInfo)
		{
			itsProjectionCurvatureInfo->InitFromSettings("SmartMet::ProjectionLineSettings");
		}
		else
			throw std::runtime_error("Cannot create ProjectionCurvatureInfo (out of memory?).");
	}
	catch(std::exception &e)
	{
		string errStr("InitProjectionCurvatureInfo - Initialization error in configurations: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitProjectionCurvatureInfo", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

// Muutetaan appversion stringi esim. 5.9.1.2 -> 5.9
std::string GetShortAppVersionString(void)
{
    std::string appVersionStr = ApplicationDataBase().appversion;
    if(appVersionStr.size() >= 5)
    {
        std::string::size_type pos = appVersionStr.find('.');
        if(pos != std::string::npos)
        {
            std::string::size_type pos2 = appVersionStr.find('.', pos+1);
            if(pos2 != std::string::npos)
            {
                appVersionStr = std::string(appVersionStr.begin(), appVersionStr.begin()+pos2);
            }
        }
    }

    return appVersionStr;
}

void InitApplicationWinRegistry(std::map<std::string, std::string> &mapViewsPositionMap, std::map<std::string, std::string> &otherViewsPositionPosMap)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
        std::string shortAppVerStr = GetShortAppVersionString();
        if(shortAppVerStr.size() < 3)
            throw std::runtime_error(std::string("Invalid application's short version number: '") + shortAppVerStr + "', should be in format X.Y");
        // 3 = 3 eri karttan‰yttˆ‰, ei viel‰k‰‰n miss‰‰n asetusta t‰lle, koska p‰‰karttan‰yttˆ poikkeaa kahdesta apukarttan‰ytˆst‰
        itsApplicationWinRegistry.Init(ApplicationDataBase().appversion, shortAppVerStr, itsBasicConfigurations.GetShortConfigurationName(), 3, mapViewsPositionMap, otherViewsPositionPosMap, *HelpDataInfoSystem());

        // We have to set log level here, now that used log level is read from registry
        CatLog::logLevel(static_cast<CatLog::Severity>(itsApplicationWinRegistry.ConfigurationRelatedWinRegistry().LogLevel()));
	}
	catch(std::exception &e)
	{
		string errStr("InitApplicationWinRegistry - Initialization error from Windows Registry: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitApplicationWinRegistry", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

void UpdateEnableDataChangesToWinReg(void)
{
    itsApplicationWinRegistry.HelpDataEnableWinRegistry().Update(*HelpDataInfoSystem());
}

void InitConceptualModelData(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsConceptualModelData.InitFromSettings("ConceptualModelData");

	}
	catch(std::exception &e)
	{
		string errStr("InitConceptualModelData - Initialization error in configurations: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitConceptualModelData", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

void InitAutoComplete(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsAutoComplete.InitFromSettings("SmartMet::AutoComplete");

	}
	catch(std::exception &e)
	{
		string errStr("InitAutoComplete - Initialization error in configurations: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitAutoComplete", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

// Metodi saa 0-pohjaisen mapViewIndeksin (index), mutta se pit‰‰ muuttaa 1-pohjaiseksi, kun tehd‰‰n settings stringi‰.
string GetMapViewDescTopSettingString(const string &theBaseStr, int index)
{
	string str(theBaseStr);
	str += NFmiStringTools::Convert<int>(index+1);
	return str;
}

NFmiMapViewDescTop* CreateMapViewDescTop(const string &baseSettingStr, int mapViewIndex)
{
	string currentSettingStr = GetMapViewDescTopSettingString(baseSettingStr, mapViewIndex);
    NFmiMapViewDescTop *descTop = new NFmiMapViewDescTop(currentSettingStr, &itsMapConfigurationSystem, itsProjectionCurvatureInfo, itsBasicConfigurations.ControlPath());
	descTop->MapViewCache().MaxSizeMB(MetEditorOptionsData().ViewCacheMaxSizeInMB());
    descTop->Init(*ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(mapViewIndex));
    return descTop;
}

// HUOM! t‰m‰ pit‰‰ tehd‰ vasta kun InitApplicationWinRegistry-metodi on ajettu!
void InitMapViewDescTopList(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
         // MapView-indeksit alkavat 0:sta
        int mapViewCount = ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapViewCount();
		string baseSettingStr("MetEditor::MapView::");
        for(int mapViewIndex = 0; mapViewIndex < mapViewCount; mapViewIndex++)
            itsMapViewDescTopList.push_back(CreateMapViewDescTop(baseSettingStr, mapViewIndex));
	}
	catch(std::exception &e)
	{
		string errStr("InitMapViewDescTopList - Initialization error in configurations: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitMapViewDescTopList", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

void InitProducerSystem(NFmiProducerSystem &producerSystem, const std::string &baseConfigurationKey, const std::string &callingFunctionName)
{
    DoVerboseFunctionStartingLogReporting(callingFunctionName);
    try
    {
        producerSystem.InitFromSettings(baseConfigurationKey);
    }
    catch(std::exception &e)
    {
        string errStr = callingFunctionName + " - Initialization error in configurations: \n";
        errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitProducerSystem", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
    }
}

void InitModelProducerSystem(void)
{
    InitProducerSystem(itsProducerSystem, "ProducerSystem", __FUNCTION__);
}

void InitObsProducerSystem(void)
{
    InitProducerSystem(itsObsProducerSystem, "ObsProducerSystem", __FUNCTION__);
}

void InitSatelImageProducerSystem(void)
{
    InitProducerSystem(itsSatelImageProducerSystem, "SatelImageProducerSystem", __FUNCTION__);
}

void InitWarningCenterSystem(void)
{
#ifndef DISABLE_CPPRESTSDK
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
        itsWarningCenterSystem.initialize();
	}
	catch(std::exception &e)
	{
		string errStr("InitWarningCenterSystem - Initialization error in configurations: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitWarningCenterSystem", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
#endif // DISABLE_CPPRESTSDK
}

void InitSeaIcingWarningSystem(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsSeaIcingWarningSystem.InitializeFromSettings("SmartMet::SeaIcingWarningSystem::");
	}
	catch(std::exception &e)
	{
		string errStr("InitSeaIcingWarningSystem - Initialization error in configurations: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitSeaIcingWarningSystem", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

void InitWindTableSystem(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsWindTableSystem.InitializeFromSettings("MetEditor::WindTableSystem::");
	}
	catch(std::exception &e)
	{
		string errStr("InitWindTableSystem - Initialization error in configurations: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitWindTableSystem", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

void InitMacroParamData(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		string minGridStr = NFmiSettings::Require<string>("MetEditor::MacroParamDataMinGridSize");
		string maxGridStr = NFmiSettings::Require<string>("MetEditor::MacroParamDataMaxGridSize");

		checkedVector<int> minGridValues = NFmiStringTools::Split<checkedVector<int> >(minGridStr, ",");
		checkedVector<int> maxGridValues = NFmiStringTools::Split<checkedVector<int> >(maxGridStr, ",");
		if(minGridValues.size() != 2)
			throw runtime_error("MetEditor::MacroParamDataMinGridSize had invalid setting, has to be to numbers (e.g. x,y).");
		if(maxGridValues.size() != 2)
			throw runtime_error("MetEditor::MacroParamDataMaxGridSize had invalid setting, has to be to numbers (e.g. x,y).");

		// Annetaan ensin rajat, ett‰ kun annetaan itse koko, infoorganizer voi tarkistaa annetun koon
		InfoOrganizer()->SetMacroParamDataMinGridSize(minGridValues[0], minGridValues[1]);
		InfoOrganizer()->SetMacroParamDataMaxGridSize(maxGridValues[0], maxGridValues[1]);
		InfoOrganizer()->SetMacroParamDataGridSize(ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MacroParamGridSizeX(), ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MacroParamGridSizeY());
	}
	catch(exception &e)
	{
		string errStr("InitMacroParamData - defined macroParam-datan grid had error: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitMacroParamData", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

NFmiPoint GetMacroParamGridSize(const std::string &theSettingsStr)
{
	int sizeX = 50;
	int sizeY = 50;
	string gridStr = NFmiSettings::Require<string>(theSettingsStr.c_str()); // annetaan tyhj‰ defaultiksi
	if(!gridStr.empty())
	{
		checkedVector<int> values = NFmiStringTools::Split<checkedVector<int> >(gridStr, ",");
		if(values.size() != 2)
			throw runtime_error(theSettingsStr + " had invalid settings, has to be two numbers (e.g. x,y).");
		sizeX = values[0];
		sizeY = values[1];
	}
	else
		throw runtime_error(theSettingsStr + " was empty, has to be two numbers (e.g. x,y).");
	return NFmiPoint(sizeX, sizeY);
}

void InitWmoStationInfoSystem(void)
{
	if(fWmoStationInfoSystemInitialized)
		return ;
	try
	{
		fWmoStationInfoSystemInitialized = true; // laitetaan initialisointi lippu trueksi, vaikka ei ehk‰ onnistukaan
		string fileName;
		std::string tmpPathAndFile = NFmiSettings::Require<std::string>("MetEditor::RAWSoundingStationInfoFile");
    	fileName = PathUtils::getAbsoluteFilePath(tmpPathAndFile, itsBasicConfigurations.ControlPath());
		itsWmoStationInfoSystem.InitFromMasterTableCsv(fileName);
		LogMessage(itsWmoStationInfoSystem.InitLogMessage(), CatLog::Severity::Info, CatLog::Category::Configuration);
	}
	catch(std::exception &e)
	{
		std::string logMsg;
		logMsg += "Problem in InitWmoStationInfoSystem - \n";
		logMsg += e.what();
        LogAndWarnUser(logMsg, "Problems in InitWmoStationInfoSystem", CatLog::Severity::Error, CatLog::Category::Configuration, false);
	}
}

void InitFileCleanerSystem(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsFileCleanerSystem.InitFromSettings("MetEditor::FileCleanerSystem");
		LogMessage(itsFileCleanerSystem.GetInitializeLogStr(), CatLog::Severity::Info, CatLog::Category::Configuration);
	}
	catch(std::exception &e)
	{
        LogAndWarnUser(e.what(), "Problems in InitFileCleanerSystem", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

void InitEditedDataParamDescriptor(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	fUseEditedDataParamDescriptor = false;
	itsEditedDataParamDescriptor = NFmiParamDescriptor();

	try
	{
		NFmiProducer defaultProducer = GetUsedDataLoadingInfo().Producer();

		fUseEditedDataParamDescriptor = NFmiSettings::Optional("MetEditor::UseConfigurationDataLoadingParams", false); // annetaan false defaultiksi
		string paramsStr;
		paramsStr = NFmiSettings::Optional("MetEditor::ConfigurationDataLoadingParams", paramsStr); // annetaan tyhj‰ defaultiksi
		if(!paramsStr.empty())
		{
			try
			{
				NFmiParamBag params;
				// oletus: paramStr:‰ss‰ on parametreja muodossa parID1,nimi1,parID2,nimi2,....
				checkedVector<std::string> valuesStr = NFmiStringTools::Split<checkedVector<std::string> >(paramsStr, ",");
				checkedVector<std::string>::iterator it = valuesStr.begin();
				checkedVector<std::string>::iterator endIt = valuesStr.end();
				for( ; it != endIt; ++it)
				{
					std::string parIdStr(*it);
					++it;
					if(it != endIt)
					{
						std::string parNameStr(*it);
						FmiParameterName parId = static_cast<FmiParameterName>(NFmiStringTools::Convert<int>(parIdStr));
						params.Add(MakeEditedParam(parId, parNameStr, defaultProducer), true);
					}
					else
						break;
				}
				itsEditedDataParamDescriptor = NFmiParamDescriptor(params);

			}
			catch(std::exception &e)
			{
                std::string errStr = std::string(e.what()) + "\nlook in file editor.conf";
                LogAndWarnUser(errStr, "Problems with edited parameter list", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
			}
		}

	}
	catch(exception &e)
	{
        LogAndWarnUser(e.what(), "Problems with edited parameter list in editor.conf", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

// Funktio tutkii onko nimee laitettu interpolaatio tietoa vai ei.
// Esim. 1 Normaali tilanne, nimess‰ ei ole interpolaatiotietoa, "L‰mpˆtila" -> "L‰mpˆtila",kLinearly  (pari)
// Esim. 2 Nimess‰ on interpolaatiotietoa (lopussa kaarisuluissa luku, joka myˆs j‰tet‰‰n nimest‰ pois), "L‰mpˆtila{2}" -> "L‰mpˆtila",kNearestPoint  (pari)
std::pair<std::string, FmiInterpolationMethod> GetEditedParamFinalNameAndInterpolationMethod(const std::string &theParamName)
{
    checkedVector<std::string> nameParts = NFmiStringTools::Split<checkedVector<std::string> >(theParamName, "{");
    if(nameParts.size() < 2)
        return std::make_pair(theParamName, kLinearly);
    else
    {
        if(theParamName[theParamName.size() - 1] != '}') // nimen pit‰‰ myˆs loppua kaarisulkuun, ennen kuin siin‰ olevat osiot hyv‰ksyt‰‰n interpolaatio menetelm‰ksi
            return std::make_pair(theParamName, kLinearly);
        else
        {
            std::string interpStr = nameParts[1];
            interpStr.resize(interpStr.size() - 1); // poistetaan lopun kaarisulku
            try
            {
                FmiInterpolationMethod interpMethod = static_cast<FmiInterpolationMethod>(boost::lexical_cast<int>(interpStr));
                if(interpMethod >= kNoneInterpolation && interpMethod < kMaxInterpolation)
                    return std::make_pair(nameParts[0], interpMethod);
                else
                    throw std::runtime_error("Invalid interpolation method for given parameter");
            }
            catch(std::exception & /* e */ )
            {
                throw std::runtime_error(std::string("Invalid interpolation method for given parameter: ") + theParamName);
            }
        }
    }
}

NFmiDataIdent MakeEditedParam(FmiParameterName theParId, const std::string &theParamName, const NFmiProducer &theDefaultProducer)
{
	if(theParId == kFmiTotalWindMS)
	{
		// t‰m‰ temppuilu CreateParam-jutun kanssa johtuu huonosta koodi suunnittelusta. Pit‰isi tehd‰ NFmiTotalWind ja W&C CreateParam systeemi uusiksi kokonaan
		NFmiTotalWind tmp;
        std::unique_ptr<NFmiDataIdent> dataIdentPtr(tmp.CreateParam(theDefaultProducer));
        dataIdentPtr->GetParam()->SetName(theParamName);
		return *dataIdentPtr;
	}
	else if(theParId == kFmiWeatherAndCloudiness)
	{
		// t‰m‰ temppuilu CreateParam-jutun kanssa johtuu huonosta koodi suunnittelusta. Pit‰isi tehd‰ NFmiTotalWind ja W&C CreateParam systeemi uusiksi kokonaan
		NFmiWeatherAndCloudiness tmp;
        std::unique_ptr<NFmiDataIdent> dataIdentPtr(tmp.CreateParam(theDefaultProducer));
        dataIdentPtr->GetParam()->SetName(theParamName);
        return *dataIdentPtr;
	}
	else
	{
        std::pair<std::string, FmiInterpolationMethod> nameInterpolationPair = GetEditedParamFinalNameAndInterpolationMethod(theParamName);
        NFmiParam tmpParam(theParId, nameInterpolationPair.first, kFloatMissing, kFloatMissing, kFloatMissing, kFloatMissing, "%.1f", nameInterpolationPair.second);
		NFmiDataIdent tmpDataIdent(tmpParam, theDefaultProducer, nameInterpolationPair.second == kLinearly ? kContinuousParam : kNumberParam);
		boost::shared_ptr<NFmiDrawParam> drawParam = itsSmartInfoOrganizer->CreateDrawParam(tmpDataIdent, 0, NFmiInfoData::kEditable);
		if(drawParam)
		{
			tmpDataIdent.GetParam()->MinValue(drawParam->AbsoluteMinValue());
			tmpDataIdent.GetParam()->MaxValue(drawParam->AbsoluteMaxValue());
		}
		return tmpDataIdent;
	}
}

void InitTrajectorySystem(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		if(itsSmartInfoOrganizer == 0)
			throw std::runtime_error("InitTrajectorySystem:: SmartInfoOrganizer was not initialized.");
		delete itsTrajectorySystem;
		itsTrajectorySystem = new NFmiTrajectorySystem(itsSmartInfoOrganizer, &itsProducerSystem);
		if(itsTrajectorySystem)
            itsTrajectorySystem->InitializeFromSettings(ControlDirectory());
	}
	catch(exception &e)
	{
        LogAndWarnUser(e.what(), "Problems initializing the trajectory system", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

void MakeClosingLogMessage(void)
{
    std::string asteriskMarkerLine(itsBasicConfigurations.EditorVersionStr().size() + 1, '*');
    LogMessage(asteriskMarkerLine, CatLog::Severity::Info, CatLog::Category::Configuration);
    LogMessage(itsBasicConfigurations.ApplicationDataBase().appname + " is closing.", CatLog::Severity::Info, CatLog::Category::Configuration);
	LogMessage(itsBasicConfigurations.EditorVersionStr(), CatLog::Severity::Info, CatLog::Category::Configuration);
	LogMessage(string("Program was running: ") + itsBasicConfigurations.MakeRunningTimeString(), CatLog::Severity::Info, CatLog::Category::Configuration);
    LogMessage(asteriskMarkerLine, CatLog::Severity::Info, CatLog::Category::Configuration);
}

// lukee editor.conf-tiedostosta seuraavat m‰‰rittelyt:
// MetEditor::DataLoadingArea ja MetEditor::DataLoadingGridSize,
// joiden avulla rakennetaan haluttu area+hila m‰‰ritys, jota k‰ytet‰‰n pohjana kun
// rakennetaan ladattaessa dataa. Jos molempia ei lˆydy, ei kyseist‰ hila m‰‰rityst‰ oteta
// k‰yttˆˆn ja t‰llˆin hilam‰‰ritykset otetaan prim‰‰ri datasta (mik‰ on prioriteetti 1
// datan lataus dialogissa).
void InitUsedDataLoadingGrid(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	// lue t‰m‰ oikeasti conffi tiedostosta, ‰l‰ vaadi, vaan testaa vain
	if(itsPossibleUsedDataLoadingGrid)
		delete itsPossibleUsedDataLoadingGrid;
	itsPossibleUsedDataLoadingGrid = 0;

	string areaStr;
	areaStr = NFmiSettings::Optional("MetEditor::DataLoadingArea", areaStr); // annetaan tyhj‰ defaultiksi
	string gridStr;
	gridStr = NFmiSettings::Optional("MetEditor::DataLoadingGridSize", gridStr); // annetaan tyhj‰ defaultiksi
	if((!areaStr.empty()) && (!gridStr.empty()))
	{
		try
		{
			boost::shared_ptr<NFmiArea> area = NFmiAreaFactory::Create(areaStr);
			checkedVector<double> values = NFmiStringTools::Split<checkedVector<double> >(gridStr, ",");
			if(values.size() != 2)
				throw runtime_error("MetEditor::DataLoadingGridSize was invlid, has to be two numbers (e.g. x,y).");
			NFmiPoint gridSize(values[0], values[1]);
			NFmiGrid grid(area.get(), static_cast<unsigned int>(gridSize.X()), static_cast<unsigned int>(gridSize.Y()));
			itsPossibleUsedDataLoadingGrid = new NFmiHPlaceDescriptor(grid);
		}
		catch(exception &e)
		{
			string errStr("InitUsedDataLoadingGrid:: defined data grid (MetEditor::DataLoadingArea) was invalid: \n");
			errStr += e.what();
			errStr += "\n";
			errStr += "Its's not going to be used.";
            LogAndWarnUser(errStr, "Problems in InitUsedDataLoadingGrid", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
		}
	}
}

void InitSettingsFromConfFile(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsDoAtSendCommandString = NFmiSettings::Require<string>("MetEditor::AtSending::ShellCommand");
		fUseDoAtSendCommand = NFmiSettings::Require<bool>("MetEditor::AtSending::DoCommand");
		itsSmartMetEditingMode = static_cast<CtrlViewUtils::FmiSmartMetEditingMode>(NFmiSettings::Require<int>("MetEditor::EditXMode"));
		fWarnIfCantSaveWorkingFile = NFmiSettings::Optional("MetEditor::WarnIfCantSaveWorkingFile", true);

		fUseOnePressureLevelDrawParam = NFmiSettings::Optional("MetEditor::UseOnePressureLevelDrawParam", false);
		fRawTempRoundSynopTimes = NFmiSettings::Optional("MetEditor::RawTempRoundSynopTimes", false);
		string rawTempUnknownStartLonLatStr = NFmiSettings::Optional<string>("MetEditor::RawTempUnknownStartLonLat", "0,0");
		checkedVector<double> rawTempUnknownStartLonLatStrVec = NFmiStringTools::Split<checkedVector<double> >(rawTempUnknownStartLonLatStr, ",");
		if(rawTempUnknownStartLonLatStrVec.size() != 2)
			throw runtime_error("InitSettingsFromConfFile MetEditor::RawTempUnknownStartLonLat was invlid, has to be two numbers (like x,y).");
		itsRawTempUnknownStartLonLat = NFmiPoint(rawTempUnknownStartLonLatStrVec[0], rawTempUnknownStartLonLatStrVec[1]);
		itsStationDataGridSize = SettingsFunctions::GetCommaSeaparatedPointFromSettings("MetEditor::StationDataGridSize");

		itsMapViewTimeLabelInfo.InitFromSettings("SmartMet::TimeLabel");
		itsSatelDataRefreshTimerInMinutes = NFmiSettings::Require<int>("SmartMet::SatelDataRefreshTimerInMinutes");
        fStoreLastLoadedFileNameToFile = NFmiSettings::Optional<bool>("SmartMet::StoreLastLoadedFileNameToFile", false);
        itsHardDriveFreeLimitForConfSavesInMB = NFmiSettings::Optional<float>("SmartMet::HardDriveFreeLimitForConfSavesInMB", 10);
        itsHardDriveFreeLimitForEditedDataSavesInMB = NFmiSettings::Optional<float>("SmartMet::HardDriveFreeLimitForEditedDataSavesInMB", 500);
        itsQ2ServerInfo.InitFromSettings();
	}
	catch(exception &e)
	{
        LogAndWarnUser(e.what(), "Problems with InitSettingsFromConfFile", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
}

void SaveSettingsToConfFile(void)
{
	try
	{
		SettingsFunctions::SetCommaSeaparatedPointToSettings("MetEditor::StationDataGridSize", itsStationDataGridSize);
		NFmiSettings::Set("SmartMet::SatelDataRefreshTimerInMinutes", NFmiStringTools::Convert<int>(itsSatelDataRefreshTimerInMinutes), true);
        itsQ2ServerInfo.StoreToSettings();
	}
	catch(exception &e)
	{
        ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(e.what()), _TEXT("Problems with InitSettingsFromConfFile!"), MB_OK);
	}
}

void InitMacroParamSystem(bool haveAbortOption)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsMacroParamSystem.RootPath(itsMacroPathSettings.MacroParamPath(true));
		NFmiFileSystem::CreateDirectory(itsMacroParamSystem.RootPath()); // luodaan varmuuden vuoksi hakemisto, jos ei ole jo

	}
	catch(exception &e)
	{
        LogAndWarnUser(e.what(), "Error with MacroParam system", CatLog::Severity::Error, CatLog::Category::Configuration, false, haveAbortOption);
	}
}

void InitViewMacroSystem(bool haveAbortOption)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsViewMacroPath = itsMacroPathSettings.ViewMacroPath(true);
		if(itsViewMacroPath[itsViewMacroPath.size()-1] == '/')
			itsViewMacroPath[itsViewMacroPath.size()-1] = kFmiDirectorySeparator;
		else if(itsViewMacroPath[itsViewMacroPath.size()-1] != '\\')
			itsViewMacroPath += kFmiDirectorySeparator;
		itsRootViewMacroPath = itsViewMacroPath;
		NFmiFileSystem::CreateDirectory(itsViewMacroPath); // luodaan varmuuden vuoksi hakemisto, jos ei ole jo
		RefreshViewMacroList();
	}
	catch(exception &e)
	{
        LogAndWarnUser(e.what(), "Error with ViewMacro system", CatLog::Severity::Error, CatLog::Category::Configuration, false, haveAbortOption);
	}
}

void InitLandBorderDrawingSystem(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
    try
    {
        // release versiolle on m‰‰r‰tty eri shape-file kuin debug versiolle, koska 
    // debug versio on tolkuttoman hidas laskiessaan koordinaatteja ja siksi
    // on parempi k‰ytt‰‰ harvempaa dataa debug versiolle.
#ifdef NDEBUG 
        itsLandBorderShapeFile = NFmiSettings::Require<string>("MetEditor::LandBorderShapeFile");
#else // debug versio
        itsLandBorderShapeFile = NFmiSettings::Require<string>("MetEditor::LandBorderShapeFileDebug");
#endif
        boost::shared_ptr<Imagine::NFmiGeoShape> landBorderGeoShape(new Imagine::NFmiGeoShape());
        DoVerboseLogReporting(std::string("Reading land border shape file: ") + itsLandBorderShapeFile);
        itsLandBorderShapeFile = PathUtils::getAbsoluteFilePath(itsLandBorderShapeFile, itsBasicConfigurations.ControlPath());
        landBorderGeoShape->Read(itsLandBorderShapeFile, Imagine::kFmiGeoShapeEsri, "");

        boost::shared_ptr<Imagine::NFmiPath> landBorderPath(new Imagine::NFmiPath(landBorderGeoShape->Path()));
        boost::shared_ptr<Imagine::NFmiPath> pacificLandBorderPath(new Imagine::NFmiPath(landBorderPath->PacificView(true)));

        itsLandBorderGeoShape = landBorderGeoShape;
        itsLandBorderPath = landBorderPath;
        itsPacificLandBorderPath = pacificLandBorderPath;
        DoCutBorderDrawInitialization();
    }
    catch(std::exception &e)
    {
        std::string errStr = "Error while reading land border shape file: ";
        errStr += itsLandBorderShapeFile + "\n";
        errStr += e.what();
        LogAndWarnUser(errStr, "Error while reading land border shape file", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
    }
    catch(...)
    {
        std::string errStr = "Unknown error while reading land border shape file: ";
        errStr += itsLandBorderShapeFile;
        LogAndWarnUser(errStr, "Error while reading land border shape file", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
    }
}

bool IsTotalWorld(const boost::shared_ptr<NFmiArea> &theArea)
{
    if(theArea)
    {
        if(theArea->ClassId() == kNFmiLatLonArea)
        {
            if(theArea->PacificView())
            {
                if(theArea->BottomLeftLatLon() == NFmiPoint(0, -90) && theArea->TopRightLatLon() == NFmiPoint(360, 90))
                    return true;
            }
            else
            {
                if(theArea->BottomLeftLatLon() == NFmiPoint(-180, -90) && theArea->TopRightLatLon() == NFmiPoint(180, 90))
                    return true;
            }
        }
        return false;
    }
    else
        throw std::runtime_error("Error in IsTotalWorld: zero-pointer given as parameter.");
}

void DoCutBorderDrawInitialization(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
    { // tehd‰‰n omassa blokissa!!
        itsCutLandBorderPaths.clear();
        std::vector<NFmiGdiPlusImageMapHandler*> &mapHandlerList = itsMapViewDescTopList[0]->GdiPlusImageMapHandlerList(); // otetaan p‰‰karttan‰ytˆn mapHandlerList

        // Ensin lasketaan eri karttapohjille leikatut rajaviivat
        for(size_t i = 0; i < mapHandlerList.size(); i++)
        {
            boost::shared_ptr<Imagine::NFmiPath> cutPath;
            boost::shared_ptr<NFmiArea> totalMapArea = mapHandlerList[i]->TotalArea();
            bool totalWorldArea = IsTotalWorld(totalMapArea);
            if(totalMapArea->PacificView())
            {
                if(totalWorldArea)
                    cutPath = itsPacificLandBorderPath;
                else
                    cutPath = boost::shared_ptr<Imagine::NFmiPath>(new Imagine::NFmiPath(itsPacificLandBorderPath->Clip(totalMapArea.get())));
            }
            else
            {
                if(totalWorldArea)
                    cutPath = itsLandBorderPath;
                else
                    cutPath = boost::shared_ptr<Imagine::NFmiPath>(new Imagine::NFmiPath(itsLandBorderPath->Clip(totalMapArea.get())));
            }
            itsCutLandBorderPaths.push_back(cutPath);
        }
    }

    // Sitten asetetaan kaikkien karttan‰yttˆjen kaikille karttapohjille leikatut rajaviivat
	for(size_t i=0; i<itsMapViewDescTopList.size(); i++)
    {
        std::vector<NFmiGdiPlusImageMapHandler*> &mapHandlerList = itsMapViewDescTopList[i]->GdiPlusImageMapHandlerList();
        for(size_t j = 0; j < mapHandlerList.size(); j++)
        {
            mapHandlerList[j]->LandBorderPath(itsCutLandBorderPaths[j]);
        }
    }
}

void InitDrawDifferenceDrawParam(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	if(itsSmartInfoOrganizer)
	{
		// korjaa t‰m‰ kohta niin ett‰ tulee kolmiv‰rinen isoviiva esitys!!!
		itsDrawDifferenceDrawParam = itsSmartInfoOrganizer->CreateDrawParam(NFmiDataIdent(NFmiParam(NFmiInfoData::kFmiSpDrawDifferenceParam)), 0, NFmiInfoData::kAnyData);
		itsDrawDifferenceDrawParam->SimpleIsoLineColorShadeClassCount(3);
		itsDrawDifferenceDrawParam->SimpleIsoLineLabelHeight(3.1f);
		itsDrawDifferenceDrawParam->UseSingleColorsWithSimpleIsoLines(false);
		itsDrawDifferenceDrawParam->SimpleIsoLineColorShadeLowValue(-0.1f);
		itsDrawDifferenceDrawParam->SimpleIsoLineColorShadeMidValue(0);
		itsDrawDifferenceDrawParam->SimpleIsoLineColorShadeHighValue(0.1f);
		itsDrawDifferenceDrawParam->SimpleIsoLineColorShadeLowValueColor(NFmiColor(0.f,0.f,1.f));
		itsDrawDifferenceDrawParam->SimpleIsoLineColorShadeMidValueColor(NFmiColor(1.f,0.f,1.f));
		itsDrawDifferenceDrawParam->SimpleIsoLineColorShadeHighValueColor(NFmiColor(1.f,0.f,0.f));
		itsDrawDifferenceDrawParam->StationDataViewType(NFmiMetEditorTypes::kFmiIsoLineView);

		// Laitoin alustamaan myˆs valittujen hilapisteiden uuden visualisointi piirron
		itsSelectedGridPointDrawParam = itsSmartInfoOrganizer->CreateDrawParam(NFmiDataIdent(NFmiParam(NFmiInfoData::kFmiSpSelectedGridPoints)), 0, NFmiInfoData::kAnyData);
		if(itsSelectedGridPointDrawParam)
		{
			// HUOM!!!! ei saa laittaa l‰pin‰kyvyytt‰ p‰‰lle!!!!!!
//			itsSelectedGridPointDrawParam->UseCustomIsoLineing(true);
//			checkedVector<float> specialIsolineValues;
//			const float isolineValue = 0.7f;
//			specialIsolineValues.push_back(isolineValue);
//			itsSelectedGridPointDrawParam->SetSpecialIsoLineValues(specialIsolineValues);
//			checkedVector<int> specialIsolineColorIndexies;
//			specialIsolineColorIndexies.push_back(10);
//			itsSelectedGridPointDrawParam->SetSpecialIsoLineColorIndexies(specialIsolineColorIndexies);
//			checkedVector<float> specialIsolineLabelHeights;
//			specialIsolineLabelHeights.push_back(0.f);
//			itsSelectedGridPointDrawParam->SetSpecialIsoLineLabelHeight(specialIsolineLabelHeights);
//			checkedVector<int> specialIsolineLineStyles;
//			specialIsolineColorIndexies.push_back(1);
//			itsSelectedGridPointDrawParam->SetSpecialIsoLineStyle(specialIsolineColorIndexies);

			itsSelectedGridPointLimit = NFmiSettings::Optional<int>("SmartMet::SelectedGridPointsDrawLimit", 100);
			bool doBorders = NFmiSettings::Optional<bool>("SmartMet::SelectedGridPointsDrawBorders", false);
			itsSelectedGridPointDrawParam->IsoLineGab(doBorders ? 1 : 5);
//			NFmiColor isolineColor(0.7f, 0.f, 0.f);
//			itsSelectedGridPointDrawParam->IsolineColor(isolineColor);  // isoviivan piirto ei onnistu t‰ss‰ (valittujen editointi pisteiden piirto) jostain syyst‰ johdonmukaisesti, sen takia piirret‰‰n vain hatch
			itsSelectedGridPointDrawParam->SimpleIsoLineLabelHeight(0);
			itsSelectedGridPointDrawParam->UseWithIsoLineHatch1(true);
			itsSelectedGridPointDrawParam->IsoLineHatchLowValue1(0.8f);
			itsSelectedGridPointDrawParam->IsoLineHatchHighValue1(10.f);
			int usedHatchType = NFmiSettings::Optional<int>("SmartMet::SelectedGridPointsDrawHatchType", 5);
			itsSelectedGridPointDrawParam->IsoLineHatchType1(usedHatchType);
//			itsSelectedGridPointDrawParam->DrawIsoLineHatchWithBorders1(true); // hatch with borders ei myˆsk‰‰n onnistu jostain mystisest‰ syyst‰
			NFmiColor defaultHatchColor(0.2f, 0.2f, 0.5f);
			NFmiColor usedHatchColor = SettingsFunctions::GetColorFromSettings("SmartMet::SelectedGridPointsDrawHatchColor", &defaultHatchColor);
			itsSelectedGridPointDrawParam->IsoLineHatchColor1(usedHatchColor);
		}
	}
}

// Laskee halutun tiedoston nimen sekunneissa. Jos paluu arvo on -99, ei tiedostoa lˆytynyt.
// Huom! t‰m‰ voi olla ongelma, koska tiedosto voi olla 1 sekunnin p‰‰st‰ tulevaisuudesta.
// Eli negatiiviset arvot tarkoittaa, ett‰ tiedosto on 'tulevaisuudesta'.
int GetFileAgeInSeconds(const string& theFileName)
{
	time_t currentTime;
	::time(&currentTime);
	int fileAge = -1;
	string foundFileName; // t‰m‰ on turha, mutta FindFile vaatii sit‰
	time_t fileTime = NFmiFileSystem::FindFile(theFileName, true, &foundFileName);
	if(fileTime)
	{
		fileAge = static_cast<int>(currentTime - fileTime);
	}
	return fileAge;
}

const std::string cantCreateRunningStatusFileError = "Cannot create 'running_status.dat' file (which may help to comfirm that previous SmartMet run has been crashed).";

// Metkuneditori pit‰‰ tietoa tilastaan tyˆhakemistossa olevalla tiedostolla, johon
// kirjoitetaan 'running' kun editori k‰ynnistet‰‰n. Tiedostoon kirjoitetaan stopped,
// kun editori suljetaan hallitusti.
// K‰ynnistett‰ess‰ tarkistaa, onko edellisen kerran ohjelma suljettu hallitusti. Jos ei ole, kirjoittaa
// lokiin varoituksen ja mahd. l‰hett‰‰ varoitus viestin (email, gsm-viesti?) asetuksista riippuen.
// Tarkistaa samalla kuitenkin myˆs, onko samasta editorista useita versioita (prosesseja) k‰ynniss‰.
// 'Sama' tarkoittaa, ett‰ onko olemassa muuta samannimist‰ prosessia, joka on k‰ynnistetty
// samasta paikasta (polusta). Jos lˆytyy muita samanaikaisia 'samoja' prosesseja, ei tee h‰lytyst‰,
// jos k‰ynnistyess‰ lˆytyy tarkistus tiedostosta "running" sana.
void CheckRunningStatusAtStartup(const NFmiString& theCheckedDirectory)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	if(IsOtherSimilarProcessesRunning()) // jos toinen samanlainen prosessi k‰ynniss‰, ei tehd‰ mit‰‰n
	{
		LogMessage("When SmartMet was started there were other SmartMets running. So can't check if SmartMet was crashed before it's was started up again ('running_status.dat' file check).", CatLog::Severity::Debug, CatLog::Category::Operational);
		return ;
	}

	string fileName(theCheckedDirectory);
	fileName += kFmiDirectorySeparator;
	fileName += "running_status.dat";
	if(!NFmiFileSystem::FileExists(fileName)) // globaali NFmiFileSystem-funktio
	{
		LogMessage("The 'running_status.dat' file didn't exist, is SmartMet started first time ion this machine?", CatLog::Severity::Warning, CatLog::Category::Operational);
		ofstream ofile(fileName.c_str()); // tehd‰‰n tarkistus tiedosto sitten jos ei ollut
		if(ofile)
		{
			ofile << "running" << endl; // laitetaan sinne 'k‰ynniss‰' tila tieto
			ofile.close();
		}
		else
			LogMessage(cantCreateRunningStatusFileError, CatLog::Severity::Error, CatLog::Category::Operational);
		return ; // paluu kun on saatu status tiedoto tehty‰
	}

	ifstream ifile(fileName.c_str());
	if(ifile)
	{
		string testStr;
		ifile >> testStr;
		ifile.close();
		if(testStr == string("running")) // status v‰‰r‰ (onko ohjelma kaatunut edellisen kerran)
		{
			LogMessage("The 'running_status.dat' file was in 'running'-mode, did SmartMet crash before that?", CatLog::Severity::Warning, CatLog::Category::Operational);
			// TƒHƒN TULEE SITTEN HƒLYTYS VIESTIN LƒHETYS!!!!!!
			return ; // paluu jo nyt, koska teksti on jo oikea
		}
	}

	// jos oli muuta kuin 'running' teksti, talletetaan tarkistus tiedostoon 'running' tila
	ofstream ofile(fileName.c_str()); // tehd‰‰n tarkistus tiedosto sitten jos ei ollut
	if(ofile)
	{
		ofile << "running" << endl; // laitetaan sinne kaikki ok tila tieto
		ofile.close();
	}
	else
	{
		LogMessage(cantCreateRunningStatusFileError, CatLog::Severity::Error, CatLog::Category::Operational);
		return ;
	}
}

void CheckRunningStatusAtClosing(const NFmiString& theCheckedDirectory)
{
	if(IsOtherSimilarProcessesRunning()) // jos toinen samanlainen prosessi k‰ynniss‰, ei tehd‰ mit‰‰n
	{
		LogMessage("When closing SmartMet, there was another SmartMet running so won't do anything to 'running_status.dat' file.", CatLog::Severity::Debug, CatLog::Category::Operational);
		return ;
	}

	string fileName(theCheckedDirectory);
	fileName += kFmiDirectorySeparator;
	fileName += "running_status.dat";
	ofstream ofile(fileName.c_str()); // tehd‰‰n tarkistus tiedosto sitten jos ei ollut
	if(ofile)
	{
		ofile << "closed" << endl; // laitetaan sinne kaikki ok tila tieto
		ofile.close();
	}
	else
	{
		LogMessage(cantCreateRunningStatusFileError, CatLog::Severity::Error, CatLog::Category::Operational);
		return ;
	}
}

bool GetHardDriveInfo(char driveLetter, double &freeGigaBytesAvailable, double &totalNumberOfGigaBytes, double &freeDriveSpaceInProcents)
{
    const double gigaByte = 1024 * 1024 * 1024;
    __int64 freeBytesAvailable = 0;
    __int64 totalNumberOfBytes = 0;
    __int64 totalNumberOfFreeBytes = 0;
    freeGigaBytesAvailable = 0;
    totalNumberOfGigaBytes = 0;
    freeDriveSpaceInProcents = 0;
    std::string drivePath;
    drivePath .push_back(driveLetter);
    drivePath += ":\\";
    BOOL status = GetDiskFreeSpaceEx(CA2T(drivePath.c_str()), (PULARGE_INTEGER)&freeBytesAvailable, (PULARGE_INTEGER)&totalNumberOfBytes, (PULARGE_INTEGER)&totalNumberOfFreeBytes);
    if(freeBytesAvailable)
        freeGigaBytesAvailable = freeBytesAvailable / gigaByte;
    if(totalNumberOfBytes)
        totalNumberOfGigaBytes = totalNumberOfBytes / gigaByte;
    if(totalNumberOfGigaBytes)
        freeDriveSpaceInProcents = (100. * freeGigaBytesAvailable) / totalNumberOfGigaBytes;
    return status == TRUE;
}

char GetSmartMetDriveLetter()
{
    auto controlPathAbsolute = BasicSmartMetConfigurations().ControlPath();
    NFmiFileString fileStr(controlPathAbsolute);
    std::string driveLetter = fileStr.Device();
    if(driveLetter.size())
        return static_cast<char>(std::toupper(driveLetter[0]));
    else
    {
        LogMessage("GetSmartMetDriveLetter: Unable to get SmartMet drive letter info", CatLog::Severity::Error, CatLog::Category::Operational, true);
        return 'C'; // En tied‰ mit‰ pit‰isi tehd‰ jos aseman kirjainta ei saada, mutta en halua heitt‰‰ poikkeustakaan
    }
}

bool IsSystemDriveSameAsApplicationDrive()
{
    auto driveLetter = GetSmartMetDriveLetter();
    if(driveLetter == 'C')
        return true;
    else
        return false;
}

CatLog::Severity GetHardDriveFreeSpaceLogSeverity(double freeDriveSpaceInProcents)
{
    if(freeDriveSpaceInProcents < 2.5 )
        return CatLog::Severity::Critical;
    else if(freeDriveSpaceInProcents < 10)
        return CatLog::Severity::Error;
    else if(freeDriveSpaceInProcents < 25)
        return CatLog::Severity::Warning;
    else
        return CatLog::Severity::Debug;
}

std::string GetHardDriveReportString(const std::string driverDescription, double freeGigaBytesAvailable, double freeDriveSpaceInProcents)
{
    std::string reportString = driverDescription;
    reportString += std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(freeGigaBytesAvailable, 2));
    reportString += " GB (";
    reportString += std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(freeDriveSpaceInProcents, 2));
    reportString += " %)";
    return reportString;
}

void ReportHardDriveUsage(const std::string &driveDescription, char driveLetter)
{
    double freeGigaBytesAvailable = 0;
    double totalNumberOfGigaBytes = 0;
    double freeDriveSpaceInProcents = 0;
    if(GetHardDriveInfo(driveLetter, freeGigaBytesAvailable, totalNumberOfGigaBytes, freeDriveSpaceInProcents))
    {
        auto logSeverity = GetHardDriveFreeSpaceLogSeverity(freeDriveSpaceInProcents);
        LogMessage(GetHardDriveReportString(driveDescription, freeGigaBytesAvailable, freeDriveSpaceInProcents), logSeverity, CatLog::Category::Operational, true);
    }
    else
        LogMessage(driveDescription + "Unable to get info from " + driveLetter + " drive", CatLog::Severity::Error, CatLog::Category::Operational, true);
}

void ReportSystemHardDriveUsage()
{
    std::string driveDescription = "\t\tSystem drive (C:) free space: \t\t";
    if(IsSystemDriveSameAsApplicationDrive())
        driveDescription = "\t\tSystem and SmartMet drive (C:) free space: ";
    ReportHardDriveUsage(driveDescription, 'C');
}

void ReportSmartMetHardDriveUsage()
{
    if(!IsSystemDriveSameAsApplicationDrive())
    {
        char smartMetDriveLetter = GetSmartMetDriveLetter();
        std::string driveDescription = "\t\tSmartMet drive (";
        driveDescription.push_back(smartMetDriveLetter);
        driveDescription += ":) free space : \t";
        ReportHardDriveUsage(driveDescription, smartMetDriveLetter);
    }
}

void ReportHardDriveUsage()
{

    LogMessage("SmartMet harddrive usage report:", CatLog::Severity::Debug, CatLog::Category::Operational, true);
    LogMessage("------------------------------------------------------", CatLog::Severity::Debug, CatLog::Category::Operational, true);
    ReportSystemHardDriveUsage();
    ReportSmartMetHardDriveUsage();
    LogMessage("------------------------------------------------------", CatLog::Severity::Debug, CatLog::Category::Operational, true);
}

void ReportProcessMemoryUsage(void)
{
	double megabyte = 1024*1024;
	DWORD currentProcessId = GetCurrentProcessId();
	HANDLE hProcess;
	PROCESS_MEMORY_COUNTERS pmc;
	// Print information about the memory usage of the process.
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, currentProcessId);
	if (NULL == hProcess)
		return;

    ReportHardDriveUsage();

	if (GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)))
	{
        LogMessage("SmartMet Memory usage report:", CatLog::Severity::Debug, CatLog::Category::Operational);
        LogMessage("------------------------------------------------------", CatLog::Severity::Debug, CatLog::Category::Operational);
        LogMessage(std::string("\t\tPageFaultCount: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(pmc.PageFaultCount/megabyte, 2)) + " MB", CatLog::Severity::Debug, CatLog::Category::Operational);
        LogMessage(std::string("\t\tPeakWorkingSetSize: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(pmc.PeakWorkingSetSize/megabyte, 1)) + " MB", CatLog::Severity::Debug, CatLog::Category::Operational);
        LogMessage(std::string("\t\tWorkingSetSize: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(pmc.WorkingSetSize/megabyte, 1)) + " MB", CatLog::Severity::Debug, CatLog::Category::Operational);
        LogMessage(std::string("\t\tQuotaPeakPagedPoolUsage: \t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(pmc.QuotaPeakPagedPoolUsage/megabyte, 2)) + " MB", CatLog::Severity::Debug, CatLog::Category::Operational);
        LogMessage(std::string("\t\tQuotaPagedPoolUsage: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(pmc.QuotaPagedPoolUsage/megabyte, 2)) + " MB", CatLog::Severity::Debug, CatLog::Category::Operational);
        LogMessage(std::string("\t\tQuotaPeakNonPagedPoolUsage: \t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(pmc.QuotaPeakNonPagedPoolUsage/megabyte, 2)) + " MB", CatLog::Severity::Debug, CatLog::Category::Operational);
        LogMessage(std::string("\t\tQuotaNonPagedPoolUsage: \t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(pmc.QuotaNonPagedPoolUsage/megabyte, 2)) + " MB", CatLog::Severity::Debug, CatLog::Category::Operational);
        LogMessage(std::string("\t\tPagefileUsage: \t\t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(pmc.PagefileUsage/megabyte, 1)) + " MB", CatLog::Severity::Debug, CatLog::Category::Operational);
        LogMessage(std::string("\t\tPeakPagefileUsage: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(pmc.PeakPagefileUsage/megabyte, 1)) + " MB", CatLog::Severity::Debug, CatLog::Category::Operational);
        LogMessage("------------------------------------------------------", CatLog::Severity::Debug, CatLog::Category::Operational);
	}
	CloseHandle( hProcess );
	ReportSystemMemoryUsage();
}

void ReportSystemMemoryUsage(void)
{
	double megabyte = 1024*1024;
	double gigabyte = megabyte*1024;
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx (&statex);

    LogMessage("Computers memory usage report:", CatLog::Severity::Debug, CatLog::Category::Operational);
    LogMessage("------------------------------------------------------", CatLog::Severity::Debug, CatLog::Category::Operational);
    LogMessage(std::string("\t\tMemory in use: \t\t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(statex.dwMemoryLoad, 0)) + " %", CatLog::Severity::Debug, CatLog::Category::Operational);
    LogMessage(std::string("\t\tTotal physical memory: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(statex.ullTotalPhys/megabyte, 0)) + " MB", CatLog::Severity::Debug, CatLog::Category::Operational);
    LogMessage(std::string("\t\tFree physical memory: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(statex.ullAvailPhys/megabyte, 1)) + " MB", CatLog::Severity::Debug, CatLog::Category::Operational);
    LogMessage(std::string("\t\tTotal paging file: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(statex.ullTotalPageFile/megabyte, 0)) + " MB", CatLog::Severity::Debug, CatLog::Category::Operational);
    LogMessage(std::string("\t\tFree paging file: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(statex.ullAvailPageFile/megabyte, 1)) + " MB", CatLog::Severity::Debug, CatLog::Category::Operational);
    LogMessage(std::string("\t\tTotal virtual memory: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(statex.ullTotalVirtual/gigabyte, 0)) + " GB", CatLog::Severity::Debug, CatLog::Category::Operational);
    LogMessage(std::string("\t\tFree virtual memory: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(statex.ullAvailVirtual/gigabyte, 1)) + " GB", CatLog::Severity::Debug, CatLog::Category::Operational);
    LogMessage("------------------------------------------------------", CatLog::Severity::Debug, CatLog::Category::Operational);
}

// onko samanlaisia prosesseja muita k‰ynniss‰ (= sama nimi ja exe-polku)
bool IsOtherSimilarProcessesRunning(void)
{

	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;
    char currentProcessPathName[MAX_PATH] = "";

	DWORD currentProcessId = GetCurrentProcessId();
    std::string currentProcessPathNameStr(NFmiApplicationDataBase::GetProcessPathAndName(currentProcessId));
	if( EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
	{
	    cProcesses = cbNeeded / sizeof(DWORD);
		int foundCounter = 0;
	    for ( i = 0; i < cProcesses; i++ )
		{
            std::string testedProcessNameStr(NFmiApplicationDataBase::GetProcessPathAndName(aProcesses[i]));
			if(currentProcessPathNameStr == testedProcessNameStr)
			{
				foundCounter++;
				if(foundCounter >= 2)
					return true;
			}
		}
	}

	return false; // pieleen meni jostain syyst‰!!!
}

bool InitOptionsData(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsMetEditorOptionsData.InitFromSettings("SmartMet::GeneralOptions");
		return true;
	}
	catch(std::exception &e)
	{
		std::string errStr("Error while initializing general SmartMet options in InitOptionsData.\n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Error while initializing options", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
	return false;
}

bool StoreOptionsData(void)
{
	try
	{
        DoConfigurationsCanBeSavedCheck(true);

        // T‰nne tullaan mm. CMainFrame:n OnClose -metodista
		StoreCaseStudyMemory();
    	itsMetEditorOptionsData.StoreToSettings();
    	itsHelpDataInfoSystem.StoreToSettings(); // n‰it‰ asetuksia muutetaan myˆs options-dialogissa (HUOM! pit‰‰ tallettaa normaalit datat, ei CaseStudy:yn liittyvi‰ juttuja)
		SaveSettingsToConfFile(); // n‰it‰ asetuksia muutetaan myˆs options-dialogissa
    	itsBasicConfigurations.ApplicationDataBase().StoreToSettings();
    	itsCPManagerSet.StoreToSettings();
		NFmiSettings::Save();
		return true;
	}
	catch(std::exception &e)
	{
		std::string errStr("Error while storing general SmartMet options in StoreOptionsData.\n");
		errStr += e.what();
		this->LogAndWarnUser(errStr, "Error while storing options", CatLog::Severity::Error, CatLog::Category::Configuration, false);
	}
	return false;
}

// Tarkastaa onko kovalevylla tarpeeksi tilaa, ja jos ei, heitt‰‰ poikkeuksen.
void DoConfigurationsCanBeSavedCheck(bool useConfigurationLimit, const std::string &optionalExplanationStr = "")
{
    float freeLimitInMB = useConfigurationLimit ? itsHardDriveFreeLimitForConfSavesInMB : itsHardDriveFreeLimitForEditedDataSavesInMB;
    if(!ConfigurationsCanBeSavedToFiles(freeLimitInMB))
    {
        std::string errorStr("Hard-drive free space is very low, less than ");
        errorStr += NFmiStringTools::Convert(freeLimitInMB);
        errorStr += " MB is free.\n";
        if(!optionalExplanationStr.empty())
            errorStr += optionalExplanationStr;
        else
        {
            if(useConfigurationLimit)
                errorStr += "SmartMet won't store any changed settings to configuration files.";
            else
                errorStr += "SmartMet won't store any edited data modifications to backup files.";
        }
        throw std::runtime_error(errorStr);
    }
}

bool ConfigurationsCanBeSavedToFiles(float freeLimitInMB)
{
    if(itsBasicConfigurations.ApplicationDataBase().GetApplicationHardDriveFreeSpaceInMB() >= freeLimitInMB)
        return true;
    else
        return false;
}

bool StoreAnalyzeToolData(void)
{
	try
	{
    	itsAnalyzeToolData.StoreToSettings();
		return true;
	}
	catch(std::exception &e)
	{
		std::string errStr("Error while storing general SmartMet options in StoreAnalyzeToolData.\n");
		errStr += e.what();
		this->LogAndWarnUser(errStr, "Error while storing options", CatLog::Severity::Error, CatLog::Category::Configuration, false);
	}
	return false;
}

bool StoreDataQualityChecker(void)
{
	try
	{
   		itsDataQualityChecker.StoreToSettings();
		return true;
	}
	catch(std::exception &e)
	{
		std::string errStr("Error while storing general SmartMet options in StoreDataQualityChecker.\n");
		errStr += e.what();
		this->LogAndWarnUser(errStr, "Error while storing options", CatLog::Severity::Error, CatLog::Category::Configuration, false);
	}
	return false;
}

bool LoadFileDialogDirectoryMemory(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsFileDialogDirectoryMemory = NFmiSettings::Require<std::string>("MetEditor::FileDialogDirectoryMemory");
	}
	catch (exception &e)
	{
        LogAndWarnUser(e.what(), "MetEditor::FileDialogDirectoryMemory init fail", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
		return false;
	}

	return true;
}

void InitCrossSectionDrawParamListVector(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	InitDrawParamListVector(&itsCrossSectionDrawParamListVector, CrossSectionSystem()->MaxViewRowSize());
}


void InitDrawParamListVector(NFmiPtrList<NFmiDrawParamList> **theList, int theSize)
{
	if((*theList))
	{
		(*theList)->Clear(true);
		delete (*theList);
	}
	(*theList) = new NFmiPtrList<NFmiDrawParamList>;
	for(int i = 0; i < theSize; i++)
		(*theList)->AddEnd(new NFmiDrawParamList());
}

void InitParamMaskList(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	boost::shared_ptr<NFmiAreaMaskList> paramMaskList(new NFmiAreaMaskList());
	ParamMaskListMT(paramMaskList);
}

bool InitInfoOrganizer(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	if(itsSmartInfoOrganizer)
		delete itsSmartInfoOrganizer;
	itsSmartInfoOrganizer = new NFmiInfoOrganizer;
	itsSmartInfoOrganizer->WorkingDirectory(WorkingDirectory());
	int undoredoDepth = (SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal) ? itsMetEditorOptionsData.UndoRedoDepth() : 0; // ns. viewmodessa undo/redo syvyydeksi 0!
	bool makeCopyOfEditedData = undoredoDepth > 0;
	itsSmartInfoOrganizer->Init(itsMacroPathSettings.DrawParamPath(true), false, makeCopyOfEditedData, fUseOnePressureLevelDrawParam); // 2. parametri (false) tarkoittaa ett‰ jos drawparam-tiedostoja ei ole, ei niit‰ luoda automaattisesti
	NFmiFileSystem::CreateDirectory(itsSmartInfoOrganizer->GetDrawParamPath());

	if(itsTimeSerialViewDrawParamList)
		delete itsTimeSerialViewDrawParamList;
	itsTimeSerialViewDrawParamList = new NFmiDrawParamList;

	return true;
}
const std::string& WorkingDirectory(void) const
{
	return itsBasicConfigurations.WorkingDirectory();
}

const std::string& ControlDirectory(void) const
{
    return itsBasicConfigurations.ControlPath();
}

const std::string& HelpDataPath(void) const
{
    return itsBasicConfigurations.HelpDataPath();
}

void InitMapConfigurationSystemMain()
{
    try
    {
        InitMapConfigurationSystem();
    }
    catch(exception &e)
    {
        LogAndWarnUser(e.what(), "Problems with map configurations", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
    }
}

void InitMapConfigurationSystem(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	// Create config list
	vector<std::string> mapsystems;
	Tokenize(NFmiSettings::Require<std::string>(CONFIG_MAPSYSTEMS), mapsystems, ",");

	std::vector<string>::iterator msiter = mapsystems.begin();
	char key1[1024], key2[1024];
	while (msiter != mapsystems.end())
	{
		std::string mapsystem(*msiter);

		sprintf(key1, CONFIG_MAPSYSTEM_PROJECTION_FILENAME, mapsystem.c_str());
		sprintf(key2, CONFIG_MAPSYSTEM_PROJECTION_DEFINITION, mapsystem.c_str());
		if (!NFmiSettings::IsSet(key1) && !NFmiSettings::IsSet(key2))
		{
			msiter++;
			continue;
		}

		// First read the projection information
		NFmiMapConfiguration mc;
		if (NFmiSettings::IsSet(key1))
		{
			mc.ProjectionFileName(NFmiSettings::Require<std::string>(key1));
		}
		else
		{
			mc.Projection(NFmiSettings::Require<std::string>(key2));
		}

		// Then add the map ..
		sprintf(key1, CONFIG_MAPSYSTEM_MAP, mapsystem.c_str());
		std::vector<std::string> maps = NFmiSettings::ListChildren(key1);
		std::vector<std::string>::iterator mapiter = maps.begin();
		while (mapiter != maps.end())
		{
			std::string map(*mapiter);
			sprintf(key1, CONFIG_MAPSYSTEM_MAP_FILENAME, mapsystem.c_str(), map.c_str());
			sprintf(key2, CONFIG_MAPSYSTEM_MAP_DRAWINGSTYLE, mapsystem.c_str(), map.c_str());
			if (NFmiSettings::IsSet(key1) && NFmiSettings::IsSet(key2))
			{
				mc.AddMap(NFmiSettings::Require<std::string>(key1), NFmiSettings::Optional<int>(key2, 0));
			}

			mapiter++;
		}

		// .. and layer configurations
		sprintf(key1, CONFIG_MAPSYSTEM_LAYER, mapsystem.c_str());
		std::vector<std::string> layers = NFmiSettings::ListChildren(key1);
		std::vector<std::string>::iterator layeriter = layers.begin();
		while (layeriter != layers.end())
		{
			std::string layer(*layeriter);
			sprintf(key1, CONFIG_MAPSYSTEM_LAYER_FILENAME, mapsystem.c_str(), layer.c_str());
			sprintf(key2, CONFIG_MAPSYSTEM_LAYER_DRAWINGSTYLE, mapsystem.c_str(), layer.c_str());
			if (NFmiSettings::IsSet(key1) && NFmiSettings::IsSet(key2))
			{
				mc.AddOverMapDib(NFmiSettings::Require<std::string>(key1), NFmiSettings::Optional<int>(key2, 0));
			}

			layeriter++;
		}

		// The map configuration is ready, add it to the mc system
		itsMapConfigurationSystem.AddMapConfiguration(mc);

		msiter++;
	}

	if(itsMapConfigurationSystem.Size() <= 0)
		throw runtime_error("InitMapConfigurationSystem: No map configurations were found");
	if(itsMapConfigurationSystem.Size() < 4)
		throw runtime_error("InitMapConfigurationSystem: There were less than 4 map configuration found from settings.");
}

std::string CreateHelpEditorFileNameFilter(void)
{
	std::string fileName(itsHelpEditorSystem.DataPath());
	fileName += "\\"; // varmuudeksi laitetaan polku deliminator per‰‰n
	fileName += "*_";
	fileName += itsHelpEditorSystem.FileNameBase();
	return fileName;
}

std::string StripFilePathAndExtension(const std::string &theFileName)
{
	NFmiFileString fileStr(theFileName);
	std::string fileName = fileStr.Header();
	return fileName;
}

bool InitHelpDataInfoSystem(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsHelpDataInfoSystem.InitFromSettings("MetEditor::HelpData", itsBasicConfigurations.ControlPath(), CreateHelpEditorFileNameFilter(), StripFilePathAndExtension(itsHelpEditorSystem.FileNameBase()));
		return true;
	}
	catch(std::exception &e)
	{
		std::string titleStr("Error when initializing HelpData");
		std::string msgStr = titleStr;
		msgStr += "while reading configurations:\n";
		msgStr += e.what();
		LogAndWarnUser(msgStr, titleStr, CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
	return false;
}

void StoreMapViewDescTopToSettings(void)
{
    for(size_t i=0; i<itsMapViewDescTopList.size(); i++)
	    itsMapViewDescTopList[i]->StoreMapViewDescTopToSettings();
}

// t‰t‰ on tarkoitus kutsua AddData-metodista ja vain 1. kerran kun tulee editoitavaa dataa, tai
// kun SmartMeti‰ k‰ytet‰‰n tiputus moodissa eli tiputetaan 'editoitavaa' dataa.
// HUOM! t‰t‰ k‰ytet‰‰n myˆs s‰‰t‰m‰‰n  aikasarjaikkunan aikabagia, koska t‰m‰ on siis tarkoitus kutsua vain kerran AddData-metodista!!!!
void SetCrossSectionSystemTimes(const NFmiTimeDescriptor &theTimeDesc)
{
	if(theTimeDesc.ValidTimeBag())
		CrossSectionSystem()->CrossSectionTimeControlTimeBag(*theTimeDesc.ValidTimeBag());
	else if(theTimeDesc.ValidTimeList())
		CrossSectionSystem()->CrossSectionTimeControlTimeBag(NFmiTimeBag(theTimeDesc.FirstTime(), theTimeDesc.LastTime(), theTimeDesc.Resolution()));
	const NFmiTimeBag &times = CrossSectionSystem()->CrossSectionTimeControlTimeBag();
	TimeSerialViewTimeBag(times);
}

void NormalizeGridDataArea(NFmiQueryData* theData)
{
	// HUOM! t‰m‰ arean size juttu pit‰‰ s‰‰t‰‰ ennen kuin tarkastellaan siivouksia, koska muuten areat eiv‰t ole samanlaisia, jos tapahtuu
	// t‰m‰ s‰‰tˆ ja t‰llˆin infoOrganizeriin j‰‰ talteen vanhat datat
	if(theData && theData->Info() && theData->Info()->Grid() && theData->Info()->Grid()->Area())
	{ // t‰ss‰ varmistetaan ett‰ hiladatassa on arean XYWorld-recti 0,0 - 1,1 maailmassa, ett‰ editori osaa piirt‰‰ datan oikein
		if(theData->Info()->Grid()->Area()->Width() != 1 || theData->Info()->Grid()->Area()->Height() != 1)
		{ // tehd‰‰n kuitenkin s‰‰dˆt vain jos leveys/korkeus poikkeaa 1:st‰ (varmuuden vuoksi)
			theData->Info()->Grid()->Area()->Size(NFmiPoint(1,1));
			theData->Info()->Grid()->Area()->Init();
		}
	}
}

int GetMaxLatestDataCount(NFmiInfoData::Type theType, const std::string &theFileNameFilter)
{
	int maxLatestDataCount = 0;
	if(NFmiDrawParam::IsModelRunDataType(theType))
	{
		maxLatestDataCount = HelpDataInfoSystem()->CacheMaxFilesPerPattern();
		NFmiHelpDataInfo *hInfo = HelpDataInfoSystem()->FindHelpDataInfo(theFileNameFilter);
		if(hInfo)
			maxLatestDataCount += hInfo->AdditionalArchiveFileCount();
	}

	return maxLatestDataCount;
}

int GetModelRunTimeGap(boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
	boost::shared_ptr<NFmiDrawParam> tmpDrawParam(new NFmiDrawParam(*theDrawParam));
	tmpDrawParam->ModelRunIndex(0);
	boost::shared_ptr<NFmiFastQueryInfo> aInfo = InfoOrganizer()->Info(tmpDrawParam, false, false); // kokeillaa ensin ei poikkileikmkaus dataa
	if(aInfo == 0)
		aInfo = InfoOrganizer()->Info(tmpDrawParam, true, false); // jos ei lˆytynyt, kokeillaan poikkileikkausdataa
	if(aInfo)
		return NFmiCaseStudyDataFile::GetModelRunTimeGapInMinutes(aInfo.get(), aInfo->DataType(), HelpDataInfoSystem()->FindHelpDataInfo(aInfo->DataFilePattern()));
	else
		return 0;
}

// P‰ivitt‰‰ maskilistaa juuri lis‰tyn datan mukaan. Eli jos AddData-metodissa lis‰tty data on jonkin
// maski-otuksen k‰ytˆss‰, pit‰‰ vain sellainen maski p‰ivitt‰‰.
void UpdateParamMaskList(NFmiInfoData::Type theType, const std::string& theDataFilePattern, NFmiQueryData* theData)
{
	boost::shared_ptr<NFmiFastQueryInfo> fastInfo = GetMatchingFastInfo(theType, theDataFilePattern, theData);
	if(fastInfo)
	{
		boost::shared_ptr<NFmiAreaMaskList> paramMaskList = ParamMaskListMT();
		if(paramMaskList)
		{
			for(paramMaskList->Reset(); paramMaskList->Next(); )
			{
				boost::shared_ptr<NFmiFastQueryInfo> maskInfo = paramMaskList->Current()->Info();
				if(maskInfo) // kaikilla maskeilla ei ole infoa
				{
					if(fastInfo->DataFilePattern() == maskInfo->DataFilePattern())
					{
						boost::shared_ptr<NFmiFastQueryInfo> fastInfoCopy = NFmiInfoOrganizer::DoDynamicShallowCopy(fastInfo); // pit‰‰ tehd‰ dynaaminen kopio, ett‰ saadaan NFmiOwnerInfo-tasoinen matala kopio maskin UpdateInfo-metodille.
						paramMaskList->Current()->UpdateInfo(fastInfoCopy);
					}
				}
			}
		}
	}
}

// T‰ll‰ funktiolla saadaan juuri se FastInfo, mink‰ data on lis‰tty AddData-funktiossa. 
// Sen hakuun InfoOrganizerista vaaditaan minimiss‰‰n parametrina olevat tiedot.
boost::shared_ptr<NFmiFastQueryInfo> GetMatchingFastInfo(NFmiInfoData::Type theType, const std::string& theDataFilePattern, NFmiQueryData* theData)
{
	boost::shared_ptr<NFmiFastQueryInfo> foundFastInfo;
	if(theType == NFmiInfoData::kEditable)
		foundFastInfo = EditedInfo();
	else if(theType == NFmiInfoData::kCopyOfEdited)
		foundFastInfo = itsSmartInfoOrganizer->FindInfo(NFmiInfoData::kCopyOfEdited);
	else
	{
		checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > fastInfoVector = itsSmartInfoOrganizer->GetInfos(theDataFilePattern);
		if(fastInfoVector.empty() == false)
			foundFastInfo = fastInfoVector[0]; // otetaan vain ensimm‰inen vektorista, koska oikeasti siin‰ ei voi olla kuin yksi lˆytynyt fastInfo
	}

	if(foundFastInfo)
	{
		if(foundFastInfo->RefQueryData() == theData)
			return foundFastInfo;
	}

	return boost::shared_ptr<NFmiFastQueryInfo>();
}

// Yritet‰‰n est‰‰ liiallisen muistin k‰yttˆ‰ kun varataan tilaa undo-redo datalle editoinnissa.
// Jos undo-redo vaatii liikaa muistia, ei oteta sit‰ k‰yttˆˆn ollenkaan.
// Muistin loppuminen kaataa tietyiss‰ systeemeiss‰ olevan SmartMetin, vaikka ei pit‰isi 
// esim. MacOs:ss‰ olevan virtuaali Winkkarin.
int DoMemoryCheckForUndoRedoDepth(NFmiQueryData* theData, int currentUndoRedoDepth)
{
    int usedUndoRedoDepth = currentUndoRedoDepth;
    if(theData && currentUndoRedoDepth)
    {
        float dataSizeInMB = theData->Info()->Size() * 4 / (1024.f * 1024.f);
        float memAvailableInMB = ApplicationDataBase().memavailable;
        float plannedUndoRedoMemInMB = currentUndoRedoDepth * dataSizeInMB;
        if(plannedUndoRedoMemInMB > memAvailableInMB)
            usedUndoRedoDepth = 0;
    }
    return usedUndoRedoDepth;
}

void AddQueryData(NFmiQueryData* theData, const std::string& theDataFileName, const std::string& theDataFilePattern,
			NFmiInfoData::Type theType, const std::string& theNotificationStr, bool loadFromFileState = false)
{
	StoreLastLoadedFileNameToLog(theDataFileName);
	if(theData && theData->Info() == 0)
	{
		// HUOM! joskus tulee ehk‰ jonkin luku virheen takia queryData, jonka rawData on roskaa ja info on 0-pointteri. Sellainen data ignoorataan 
		// t‰ss‰, huom vuotaa muistia, koska en voi deletoida kun rawData-pointteri osoittaa ties minne.
		std::string errMessage = "Data loading failed, following data was read, but it was invalid and not used: ";
		errMessage += theDataFileName;
		LogMessage(errMessage, CatLog::Severity::Error, CatLog::Category::Data);
		return ;
	}

	NormalizeGridDataArea(theData);

	NFmiTimeDescriptor removedDatasTimesOut; // t‰t‰ k‰ytet‰‰n mm. tutka-datan ruudun likaus optimointiin

	if(theData)
		itsSmartInfoOrganizer->ClearThisKindOfData(theData->Info(), theType, theDataFilePattern, removedDatasTimesOut);

	{
		int undoredoDepth = (SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal || SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeStartUpLoading) ? itsMetEditorOptionsData.UndoRedoDepth() : 0; // ns. viewmodessa undo/redo syvyydeksi 0!
        undoredoDepth = DoMemoryCheckForUndoRedoDepth(theData, undoredoDepth);
		try
		{
			int theMaxLatestDataCount = GetMaxLatestDataCount(theType, theDataFilePattern);
			NFmiQueryInfo *aInfo = theData ? theData->Info() : 0;
			int theModelRunTimeGap = NFmiCaseStudyDataFile::GetModelRunTimeGapInMinutes(aInfo, theType, HelpDataInfoSystem()->FindHelpDataInfo(theDataFilePattern));
			bool dataWasDeleted = false;
			itsSmartInfoOrganizer->AddData(theData, theDataFileName, theDataFilePattern, theType, undoredoDepth, theMaxLatestDataCount, theModelRunTimeGap, dataWasDeleted);
			if(dataWasDeleted)
			{ // data on deletoitu, n‰in voi k‰yd‰ jos esim. annetun datan origin aika on pieless‰ esim. 1900.0.0 jne (kuten on ollut mtl_ecmwf_aalto-datan kanssa joskus)
				// tehd‰‰n raportti lokiin ja ei jatketa funktiota eteenp‰in...
				string errorStr("Following data was not accepted to SmartMet: ");
				errorStr += theDataFileName;
				LogAndWarnUser(errorStr, "Data adding failed", CatLog::Severity::Error, CatLog::Category::Data, true);
				return ;
			}
		}
		catch(std::exception &e)
		{
			string errorStr("Error in NFmiSmartInfoOrganizer->AddData method call, \npropably undo-level setting failed:\n");
			errorStr += e.what();
			LogAndWarnUser(errorStr, "Undo-redo setting failed, probably memory was too low", CatLog::Severity::Error, CatLog::Category::Data, false);
		}
		catch(...)
		{
			string errorStr("Error in NFmiSmartInfoOrganizer->AddData method call, \npropably undo-level setting failed for too low memory.");
			LogAndWarnUser(errorStr, "Undo-redo setting failed, probably memory was too low", CatLog::Severity::Error, CatLog::Category::Data, false);
		}

		MakeNeededDirtyOperationsWhenDataAdded(theData, theType, removedDatasTimesOut, theDataFileName);
		UpdateParamMaskList(theType, theDataFilePattern, theData);

		if(theType == NFmiInfoData::kEditable) // 1999.08.30/Marko
		{
			FilterDialogUpdateStatus(1); // 1 = filterdialogin aikakontrolli-ikkuna pit‰‰ p‰ivitt‰‰
			itsLastBrushedViewRow = -1; // sivellint‰ varten pit‰‰ 'nollata' t‰m‰
			boost::shared_ptr<NFmiFastQueryInfo> editedInfo = EditedInfo();
			if(editedInfo)
			{
				if((SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal || SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeStartUpLoading) && undoredoDepth > 0)
					undoredoDepth++; // +1 tulee virheest‰, mit‰ en jaksa nyt etsi‰ maskiotuksesta (Marko: eli leveleit‰ tulee yksi v‰hemm‰n kuin pyydet‰‰n)
				dynamic_cast<NFmiSmartInfo*>(editedInfo.get())->LocationSelectionUndoLevel(undoredoDepth);
				editedInfo->First(); // t‰m‰ asetetaan siksi, ett‰ jos edellisess‰ datassa oli leveleit‰ ja nykyisess‰ ei ole ja oli levelparameja n‰ytˆll‰, levelit menee pieleen jos leveli‰ ei aseteta t‰ss‰
			}

			NFmiParamBag tempbag(itsFilteringParamBag);
			itsFilteringParamBag = theData->Info()->ParamBag();
			itsFilteringParamBag.SetActivities(tempbag, false);

			static bool firstTime = true;

			if(editedInfo)
			{
				if(firstTime || loadFromFileState)
				{ // s‰‰det‰‰n t‰‰ll‰ aikakontrolli ikkunan aikoja vain 1. kun editoitu data laitetaan SmartMetiin tai jos editoitu data on ladattu suoraan tiedostosta (esim. tiputtamalla)
					firstTime = false;
					TimeControlViewTimes(CtrlViewUtils::kDoAllMapViewDescTopIndex, editedInfo->TimeDescriptor());
					SetCrossSectionSystemTimes(editedInfo->TimeDescriptor());
				}

                ResetTimeFilterTimes();
				editedInfo->FirstParam();
				itsDefaultEditedDrawParam = itsSmartInfoOrganizer->CreateDrawParam(editedInfo->Param(), 0, theType);

				std::string fileName(SpecialFileStoragePath());
				fileName += "controlpoint.dat";
				CPManager()->Area(editedInfo->Area());
				CPManager()->Init(editedInfo->TimeDescriptor(), editedInfo->ParamBag(), fileName, false, true);
				if(CPManagerSet().UseOldSchoolStyle() == false) // jos ollaan uudessa multi CPManager setiss‰, pit‰‰ oldSchool CPManager viel‰ p‰ivitt‰‰, ett‰ viewMakroista ladatut CPManagerit ovat kunnossa
				{
					CPManager(true)->Area(editedInfo->Area());
					CPManager(true)->Init(editedInfo->TimeDescriptor(), editedInfo->ParamBag(), fileName, false, true);
				}
			}
			fIsTEMPCodeSoundingDataAlsoCopiedToEditedData = false;

			if(editedInfo)
			{
				dynamic_cast<NFmiSmartInfo*>(editedInfo.get())->LoadedFromFile(loadFromFileState);
				// asetetaan viel‰ editorin currenttime sopivaksi
				editedInfo->TimeToNearestStep(CurrentTime(0), kCenter); // asetetaan current time p‰‰karttaikunan mukaan desctop-indeksi 0
				CurrentTime(CtrlViewUtils::kDoAllMapViewDescTopIndex, editedInfo->Time());
			}
		}

        if(DataNotificationSettings().Use() && DataNotificationSettings().ShowIcon())
            DoNewDataNotifications(theData, theNotificationStr, theDataFilePattern);

		// T‰m‰ on kTEMPCodeSoundingData-speciaali. Jos editori k‰ytt‰‰ vain luotaus datan katseluun,
		// eik‰ ole olemassa editoitua dataa, laitetaan t‰m‰n tyyppinen data myˆs editoitavaksi dataksi,
		// jotta dataa voisi k‰tev‰sti katsella editorilla, ilman ett‰ tarvitsee erikseen tiputella editoitavia datoja
		if(theType == NFmiInfoData::kTEMPCodeSoundingData && (EditedInfo() == nullptr || fIsTEMPCodeSoundingDataAlsoCopiedToEditedData))
		{
			AddQueryData(theData->Clone(), theDataFileName, "", NFmiInfoData::kEditable, "");
			fIsTEMPCodeSoundingDataAlsoCopiedToEditedData = true;
		}

		if(EditedInfo() == nullptr && CaseStudyModeOn()) // jos on ladattu caseStudy, eik‰ ollut pohjilla mit‰‰n dataan, laitetaan 1. ladattu data 'editoitavaksi dataksi'
		{
			if(theData->Info()->SizeLevels() == 1) // mutta vain pintadatalle, koska mahdollinen mallipintadata on niin jumalattoman iso
				AddQueryData(theData->Clone(), theDataFileName, "", NFmiInfoData::kEditable, "");
		}
        PrepareForParamAddSystemUpdate();
	}
}

void PrepareForParamAddSystemUpdate()
{
    if(!ParameterSelectionSystem().updatePending())
    {
        ApplicationInterface::GetApplicationInterfaceImplementation()->ParameterSelectionSystemUpdateTimerStart(ParameterSelectionSystem().updateWaitTimeoutInSeconds());
        ParameterSelectionSystem().updatePending(true);
    }
}

bool IsAnyDataAtThisTime(NFmiFastQueryInfo &theInfo)
{
	for(theInfo.ResetLevel(); theInfo.NextLevel(); )
	{
		for(theInfo.ResetParam(); theInfo.NextParam(false); )
		{
			for(theInfo.ResetLocation(); theInfo.NextLocation(); )
			{
				if(theInfo.FloatValue() != kFloatMissing)
					return true;
			}
		}
	}
	return false;
}

// Etsi viimeinen aika, milt‰ qDatasta lˆytyy mit‰‰n dataa (mik‰ tahansa par, lev, loc).
// Ajan pit‰‰ olla kuitenkin 'uudenpi' kuin annetun latestTimen ja viel‰p‰ niin ett‰ se menee
// synkanssa annetun resoluution kanssa. Eli jos resoluutio 60 (= 1 tunti) ja latestTime on esim. klo 6:00.
// T‰llˆin sallittuja aikoja ovat mm. klo 7:00, klo 8:00 jne. (p‰iv‰ys huomioiden tietenkin).
// Jos uudempaa sallittua aikaa ei lˆydy, palautetaan missing-aika, muuten palautetaan lˆydetty oikea aika.
NFmiMetTime FindLatestTimeWithAnyData(NFmiQueryData *thedata, const NFmiMetTime &theLatestTime, int theReolutionInMinutes)
{
	NFmiMetTime foundTime = NFmiMetTime::gMissingTime;
	if(thedata && theReolutionInMinutes > 0)
	{
		NFmiFastQueryInfo fInfo(thedata);
		NFmiMetTime dataTime = fInfo.TimeDescriptor().LastTime();
		dataTime.SetTimeStep(theReolutionInMinutes, true);
		for( ; ; dataTime.PreviousMetTime())
		{
			if(theLatestTime >= dataTime)
				break; // lopetetaan homma t‰h‰n
			if(fInfo.TimeDescriptor().FirstTime() > dataTime)
				break; // lopetetaan viimeist‰‰n, kun tutkittava aika on mennyt datan alkuajan ohi
			if(fInfo.Time(dataTime))
			{
				// etsi lˆytyykˆ t‰lt‰ ajan hetkelt‰ mit‰‰n dataa
				if(IsAnyDataAtThisTime(fInfo))
				{
					foundTime = dataTime;
					break;
				}
			}
		}

	}
	return foundTime;
}

void DoNewDataNotifications(NFmiQueryData *theData, const std::string &theNotificationStr, const std::string &theDataFilePattern)
{
	if(theNotificationStr.empty() == false)
	{ // jos notifikaatio stringi ei ollut tyhj‰, laitetaan se menem‰‰n SmartMetin system tray iconille
        ApplicationInterface::GetApplicationInterfaceImplementation()->SetNotificationMessage(theNotificationStr, DataNotificationSettings().BalloonHeader(), 2, DataNotificationSettings().TimeOutInSeconds(), !DataNotificationSettings().UseSound());
	}
	else // Jos kyseiselle datalle ei tehd‰ tavallista uusi data notifikaatiota, tarkastetaan pit‰‰ tehd‰ ns. uutta dataa uudelle tunnille ilmoitus
	{
		// Tarkastetaan onko kyseess‰ sellainen data josta pit‰‰ raportoida, 
		// josko on tullut uutta dataa esim. uudelle tasatunnille.
		NFmiHelpDataInfo *helpDataInfo = HelpDataInfoSystem()->FindHelpDataInfo(theDataFilePattern);
		if(helpDataInfo && helpDataInfo->ReportNewDataTimeStepInMinutes() != 0)
		{
			// 1. Onko kyseisest‰ datasta viel‰ merkint‰‰ 'arkistossa'?
			// 1.a) Mik‰ on viimeisin aika milt‰ on ollut todistettavasti dataa
			unsigned long prodId = theData->Info()->Producer()->GetIdent();
			ObsDataReport *obsDataReport = itsObsDataLoadedReporter.Find(prodId);
			NFmiMetTime lastDataTime = NFmiMetTime::gMissingTime;
			if(obsDataReport)
				lastDataTime = obsDataReport->itsLastDataTime;

			// 2.a) pit‰‰ kuitenkin etsi‰ se viimeisin aika, milloin dataa on
			// 4. K‰y dataa l‰pi viimeisest‰ ajasta taaksep‰in ja etsi ensimm‰inen data milt‰ tahansa parametrilta, paikalta tai levelilt‰
			// 5. Looppi - Kunnes ollaan tultu sellaiseen aikaa mill‰ ei ole merkityst‰
			NFmiMetTime foundTime = FindLatestTimeWithAnyData(theData, lastDataTime, obsDataReport ? obsDataReport->itsTimeStepInMinutes : helpDataInfo->ReportNewDataTimeStepInMinutes());
			// 2. Jos ei ole, lis‰t‰‰n arkistoon, mutta ei raportoida mit‰‰n (oletus: SmartMetin k‰ynnistyess‰ ei raportilla ole v‰li‰).
			if(obsDataReport == 0)
			{
				ObsDataReport odr;
				odr.itsLastDataTime = foundTime;
				odr.itsProd = *(theData->Info()->Producer());
				odr.itsReportLabelStr = helpDataInfo->ReportNewDataLabel();
				odr.itsReportTimeFormat = "YYYY.MM.DD HH:mm";
				odr.itsTimeStepInMinutes = helpDataInfo->ReportNewDataTimeStepInMinutes();
				itsObsDataLoadedReporter.Add(prodId, odr);
			}
			else if(foundTime != NFmiMetTime::gMissingTime)
			{
			// 6. Jos lˆytyi dataa ajalta, mill‰ on merkityst‰, 
			// 6.b) laita aika talteen t‰lle tuotttajalle
				obsDataReport->itsLastDataTime = foundTime;
			// 6.a) tee raportti 
				std::string obsNotifStr = GetObsDataReportStr(*obsDataReport);
                ApplicationInterface::GetApplicationInterfaceImplementation()->SetNotificationMessage(obsNotifStr, DataNotificationSettings().BalloonHeader(), 2, DataNotificationSettings().TimeOutInSeconds(), !DataNotificationSettings().UseSound());
			}
		}
	}
}

std::string GetObsDataReportStr(ObsDataReport &theObsDataReport)
{
	std::string str;
	if(theObsDataReport.itsReportLabelStr.empty() == false)
		str += theObsDataReport.itsReportLabelStr + "\n";
	else
	{
		str += "New ";
		str += theObsDataReport.itsProd.GetName();
		str += "at:\n";
	}
	str += theObsDataReport.itsLastDataTime.ToStr(theObsDataReport.itsReportTimeFormat, Language());

	return str;
}

// T‰ss‰ lasketaan tietynlaisille datoille 'minimi' n‰yttˆjen likaus aikav‰li.
// N‰it‰ datoja ovat mm. tutka, mesan analyysi(, laps datat?). Niiden pit‰‰ olla hila dataa.
// Dataa, mill‰ on yksiselitteiset valitimet, eli yksi havainto per aika.
// Jos palautettavan timebagin resoluutio on 0, ei timebagia k‰ytet‰ jatkossa.
NFmiTimeBag GetDirtyViewTimes(NFmiQueryData *theData, NFmiInfoData::Type theType, const NFmiTimeDescriptor &theRemovedDatasTimes)
{
	NFmiTimeBag times; // t‰ss‰ menee resoluutio 0:ksi, toivottavasti kukaan ei muuta default konstruktorin k‰ytt‰ytymist‰.
	if(theType == NFmiInfoData::kObservations || theType == NFmiInfoData::kAnalyzeData)
	{
		if(theData->Info()->Grid())
		{
			if(theRemovedDatasTimes.LastTime() < theData->Info()->TimeDescriptor().LastTime())
			{
				NFmiMetTime aTime = theRemovedDatasTimes.LastTime();
				short step = static_cast<short>(theData->Info()->TimeResolution());
				if(step == 0)
					step = static_cast<short>(theRemovedDatasTimes.Resolution());
				if(step > 0)
				{
					aTime.SetTimeStep(step, true);
					aTime.NextMetTime();
				}
				else if(step == 0)
					step = 1; // asetetaan 0::sta poikkeava, muulla ei ole v‰li‰
				times = NFmiTimeBag(aTime, theData->Info()->TimeDescriptor().LastTime(), step);
			}
		}
	}
	return times;
}

void MakeNeededDirtyOperationsWhenDataAdded(NFmiQueryData *theData, NFmiInfoData::Type theType, const NFmiTimeDescriptor &theRemovedDatasTimes, const std::string &theFileName)
{
    NFmiTimeBag dirtyViewTimes = GetDirtyViewTimes(theData, theType, theRemovedDatasTimes);

    NFmiFastQueryInfo fastInfo(theData);
	unsigned int ssize = static_cast<unsigned int>(itsMapViewDescTopList.size());
	for(unsigned int i = 0; i<ssize; i++)
		MakeNeededDirtyOperationsWhenDataAdded(i, fastInfo, theType, dirtyViewTimes, theFileName);

    if(CtrlViewFastInfoFunctions::IsObservationLockModeDataType(theType))
		CheckAnimationLockedModeTimeBags(CtrlViewUtils::kDoAllMapViewDescTopIndex, false);

    itsCrossSectionSystem.CheckIfCrossSectionViewNeedsUpdate(theData, theType);

    if(theType == NFmiInfoData::kKepaData)
    {
        bool flagChanged = !fLastEditedDataSendHasComeBack;
        fLastEditedDataSendHasComeBack = true;
        if(flagChanged && ApplicationWinRegistry().ConfigurationRelatedWinRegistry().ShowLastSendTimeOnMapView())
            MapViewDirty(0, false, false, true, false, true, false); // 0 = vain p‰‰karttan‰yttˆ liataan, cacheja ei tarvitse tyhjent‰‰, koska t‰h‰n liittyv‰t jutut piirret‰‰n vain karttan‰ytˆn p‰‰lle, ei karttaruudukkoihin
        LogMessage("Operational data has been loaded.", CatLog::Severity::Info, CatLog::Category::Editing);
    }
}

void StoreLastLoadedFileNameToLog(const std::string &theFileName)
{
    CatLog::logMessage(std::string("Adding loaded data into SmartMet's inner database: ") + theFileName, CatLog::Severity::Debug, CatLog::Category::Data, true);
}

void MakeNeededDirtyOperationsWhenDataAdded(unsigned int theDescTopIndex, NFmiFastQueryInfo &theInfo, NFmiInfoData::Type theType, const NFmiTimeBag &theDirtyViewTimes, const std::string &theFileName)
{
    auto dataProducer = theInfo.Producer();
	NFmiMapViewDescTop *descTop = MapViewDescTop(theDescTopIndex);
	MapViewDirty(theDescTopIndex, false, false, true, false, false, false); // ei laiteta cachea likaiseksi

	int cacheRowNumber = 0; // cache row indeksi alkaa 0:sta!!
	NFmiPtrList<NFmiDrawParamList> *drawParamListVector = descTop->DrawParamListVector();
	NFmiPtrList<NFmiDrawParamList>::Iterator iter = drawParamListVector->Start();
	for(; iter.Next();)
	{
		NFmiDrawParamList *aList = iter.CurrentPtr();
		if(aList)
		{
			for(aList->Reset(); aList->Next(); )
			{
				boost::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
                // Ei tarvitse tarkastella mitenk‰‰n, jos drawParam layer on piilossa
                if(drawParam->IsParamHidden())
                    continue;

                if(MakeNormalDataDrawingLayerCahceChecks(theDescTopIndex, theInfo, theType, theDirtyViewTimes, theFileName, drawParam, *dataProducer, *descTop, cacheRowNumber))
					break; // voidaan menn‰ seuraavalle riville saman tien

				if(theType == NFmiInfoData::kEditable && (drawParam->DataType() == NFmiInfoData::kEditable || drawParam->DataType() == NFmiInfoData::kCopyOfEdited))
				{ // jos kyseess‰ oli editoitavan datan p‰ivitys, laitetaan uusiksi ne rivit miss‰ on editoitavan datan ja sen kopion parametreja n‰kyviss‰ (tuottajalla ei ole v‰li‰)
					descTop->MapViewCache().MakeRowDirty(cacheRowNumber);// clean cache row
					break; // voidaan menn‰ seuraavalle riville saman tien
				}

                if(MakeMacroParamDrawingLayerCacheChecks(drawParam, theInfo, theType, *descTop, theDescTopIndex, cacheRowNumber, theFileName))
                    break; // voidaan menn‰ seuraavalle riville saman tien

                if(CheckAllSynopPlotTypeUpdates(theDescTopIndex, drawParam, *dataProducer, *descTop, cacheRowNumber, theFileName))
                    break; // voidaan menn‰ seuraavalle riville saman tien
			}
		}
		cacheRowNumber++;
	}

}

bool IsMacroParamDependentOfEditedData(boost::shared_ptr<NFmiDrawParam> &drawParam)
{
    if(drawParam->DataType() == NFmiInfoData::kMacroParam)
    {
        std::string macroParamStr = DoGetMacroParamFormula(drawParam, CatLog::Category::Visualization);
        MacroParamDataChecker macroParamDataChecker;
        auto macroParamDataInfoVector = macroParamDataChecker.getCalculationParametersFromMacroPram(macroParamStr);
        for(const auto &macroParamDataInfo : macroParamDataInfoVector)
        {
            if(macroParamDataInfo.type_ == NFmiInfoData::kEditable)
                return true;
        }
    }
    return false;
}

std::vector<std::string> MakeListOfUsedMacroParamsDependedOnEditedData()
{
    std::vector<std::string> macroParamPathList;
    for(auto *deskTop : itsMapViewDescTopList)
    {
        NFmiPtrList<NFmiDrawParamList> *drawParamListVector = deskTop->DrawParamListVector();
        NFmiPtrList<NFmiDrawParamList>::Iterator iter = drawParamListVector->Start();

        int cacheRowNumber = 0;
        for(; iter.Next();)
        {
            NFmiDrawParamList *aList = iter.CurrentPtr();
            if(aList)
            {
                for(aList->Reset(); aList->Next(); )
                {
                    boost::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
                    if(IsMacroParamDependentOfEditedData(drawParam))
                    {
                        macroParamPathList.push_back(drawParam->InitFileName());
                        // Tyhjennet‰‰n myˆs kyseisten n‰yttˆrivien bitmap cache (ik‰v‰ kaksois vastuu metodilla)
                        deskTop->MapViewCache().MakeRowDirty(cacheRowNumber);
                    }
                }
            }
            cacheRowNumber++;
        }
    }
    return macroParamPathList;
}

void ClearAllMacroParamDataCacheDependentOfEditedDataAfterEditedDataChanges()
{
    auto macroParamPaths = MakeListOfUsedMacroParamsDependedOnEditedData();
    MacroParamDataCache().clearMacroParamCache(macroParamPaths);
}

void NormalDataDirtiesCacheRowTraceLog(boost::shared_ptr<NFmiDrawParam> &drawParam, unsigned int theDescTopIndex, int cacheRowNumber, const std::string &theFileName)
{
    if(CatLog::doTraceLevelLogging())
    {
        std::string traceLogMessage = "Param '";
        traceLogMessage += CtrlViewUtils::GetParamNameString(drawParam, CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation(), ::GetDictionaryString("MapViewToolTipOrigTimeNormal"), ::GetDictionaryString("MapViewToolTipOrigTimeMinute"), false, false, false);
        traceLogMessage += "' required map-view ";
        traceLogMessage += std::to_string(theDescTopIndex + 1);
        traceLogMessage += " row ";
        traceLogMessage += std::to_string(cacheRowNumber + 1);
        traceLogMessage += " to be updated when data '";
        traceLogMessage += theFileName;
        traceLogMessage += "' was read";
        CatLog::logMessage(traceLogMessage, CatLog::Severity::Trace, CatLog::Category::Visualization);
    }
}

bool MakeNormalDataDrawingLayerCahceChecks(unsigned int theDescTopIndex, NFmiFastQueryInfo &theInfo, NFmiInfoData::Type theType, const NFmiTimeBag &theDirtyViewTimes, const std::string &theFileName, boost::shared_ptr<NFmiDrawParam> &drawParam, NFmiProducer &dataProducer, NFmiMapViewDescTop &descTop, int cacheRowNumber)
{
    const NFmiLevel *level = theInfo.SizeLevels() <= 1 ? 0 : theInfo.Level(); // ns. pinta datan kanssa ei v‰litet‰ leveleist‰
    if(drawParam->DataType() == theType && *drawParam->Param().GetProducer() == dataProducer && theInfo.Param(drawParam->Param()) && (level == 0 || drawParam->Level().LevelType() == level->LevelType()))
    { // jos p‰ivitetty data oli samaa tyyppi‰ ja sill‰ oli sama tuottaja kuin n‰ytˆll‰ olevalla drawParamilla, laitetaan rivin piirto uusiksi
        if(theDirtyViewTimes.Resolution() == 0) // jos dirty-timebagia ei ole alustettu arvoilla, liataan koko rivi
            descTop.MapViewCache().MakeRowDirty(cacheRowNumber);// clean cache row
        else
        { // jos kyse tutka-havainto / mesan analyysi optimoinnista, liataan vain ne ajat joihin on tullut uutta dataa
            descTop.MapViewCache().MakeTimesDirty(theDirtyViewTimes.FirstTime(), theDirtyViewTimes.LastTime(), cacheRowNumber);
        }
        NormalDataDirtiesCacheRowTraceLog(drawParam, theDescTopIndex, cacheRowNumber, theFileName);
        return true;
    }
    return false;
}

// Synop-plot ja muut vastaavat ovat erikoistapauksia, koska sill‰ erikois param-id (kFmiSpSynoPlot, etc.), ja ne pit‰‰ tarkistaa erikseen.
// Palauttaa true, jos pit‰‰ tehd‰ p‰ivityksi‰ n‰yttˆriville.
bool CheckAllSynopPlotTypeUpdates(unsigned int theDescTopIndex, boost::shared_ptr<NFmiDrawParam> &drawParam, NFmiProducer &newDataProducer, NFmiMapViewDescTop &descTop, int cacheRowNumber, const std::string &theFileName)
{
    unsigned long parId = drawParam->Param().GetParamIdent();
    bool updateStatus = false;
    if(SynopPlotNeedsUpdate(parId, newDataProducer))
    { 
        descTop.MapViewCache().MakeRowDirty(cacheRowNumber);// clean cache row
        updateStatus = true;
    }
    if(MetarPlotNeedsUpdate(parId, newDataProducer))
    {
        descTop.MapViewCache().MakeRowDirty(cacheRowNumber);// clean cache row
        updateStatus = true;
    }
    if(SoundingPlotNeedsUpdate(parId, newDataProducer))
    {
        descTop.MapViewCache().MakeRowDirty(cacheRowNumber);// clean cache row
        updateStatus = true;
    }

    if(updateStatus)
        NormalDataDirtiesCacheRowTraceLog(drawParam, theDescTopIndex, cacheRowNumber, theFileName);
    return updateStatus;
}

bool SynopPlotNeedsUpdate(unsigned long drawParamParameterId, NFmiProducer &newDataProducer)
{
    if((drawParamParameterId == NFmiInfoData::kFmiSpSynoPlot || drawParamParameterId == NFmiInfoData::kFmiSpMinMaxPlot) && newDataProducer.GetIdent() == kFmiSYNOP)
        return true;
    else
        return false;
}

bool MetarPlotNeedsUpdate(unsigned long drawParamParameterId, NFmiProducer &newDataProducer)
{
    if(drawParamParameterId == NFmiInfoData::kFmiSpMetarPlot && newDataProducer.GetIdent() == kFmiMETAR)
        return true;
    else
        return false;
}

bool SoundingPlotNeedsUpdate(unsigned long drawParamParameterId, NFmiProducer &newDataProducer)
{
    if(drawParamParameterId == NFmiInfoData::kFmiSpSoundingPlot && newDataProducer.GetIdent() == kFmiTEMP)
        return true;
    else
        return false;
}

void MacroParamDirtiesCacheRowTraceLog(boost::shared_ptr<NFmiDrawParam> &drawParam, const MacroParamDataInfo &macroParamDataInfo, unsigned int theDescTopIndex, int cacheRowNumber, const std::string &theFileName)
{
    if(CatLog::doTraceLevelLogging())
    {
        std::string traceLogMessage = "MacroParam '";
        traceLogMessage += drawParam->ParameterAbbreviation();
        traceLogMessage += "' with variable '";
        traceLogMessage += macroParamDataInfo.variableName_;
        traceLogMessage += "' ";
        if(macroParamDataInfo.usedWithVerticalFunction_)
            traceLogMessage += "in function '" + macroParamDataInfo.possibleVerticalFunctionName_ + "' ";
        traceLogMessage += "required map-view ";
        traceLogMessage += std::to_string(theDescTopIndex+1);
        traceLogMessage += " row ";
        traceLogMessage += std::to_string(cacheRowNumber + 1);
        traceLogMessage += " to be updated when data '";
        traceLogMessage += theFileName;
        traceLogMessage += "' was read";
        CatLog::logMessage(traceLogMessage, CatLog::Severity::Trace, CatLog::Category::Visualization);
    }
}

// Tutkii vertikaali macroParam funktioiden kanssa ett‰ jos kyse on pressure datasta, ett‰ lˆytyykˆ 
// samalta tuottajalta myˆs hybrid dataa, miss‰ on haluttu parametri.
// Kaikissa muissa tapauksissa palauttaa true (eli tehd‰‰n rivin cache likaus), paitsi jos data 
// pressure dataa ja lˆytyy vastaava hybrid data infoOrganizerista.
bool DoMacroParamVerticalDataChecks(NFmiFastQueryInfo &theInfo, NFmiInfoData::Type theType, const MacroParamDataInfo &macroParamDataInfo)
{
    if(macroParamDataInfo.usedWithVerticalFunction_)
    {
        // Jos dataa k‰ytetty vertikaali funktioiden kanssa, pit‰‰ siin‰ olla yli 2 leveli‰, muuten sit‰ ei k‰ytet‰ macroParam laskuissa
        if(theInfo.SizeLevels() > 2)
        {
            if(theType == NFmiInfoData::kViewable)
            {
                if(theInfo.LevelType() == kFmiPressureLevel)
                {
                    auto hybridData = InfoOrganizer()->Info(macroParamDataInfo.dataIdent_, nullptr, NFmiInfoData::kHybridData, false, true);
                    if(hybridData)
                        return false; // lˆytyi vastaava hybrid data, eli ei tehd‰ rivin p‰ivityst‰ t‰lle datalle
                }
            }
        }
        else
            return false;
    }

    return true;
}

std::string DoGetMacroParamFormula(boost::shared_ptr<NFmiDrawParam> &theDrawParam, CatLog::Category category)
{
    static std::set<std::string> reportedMissingMacroParams;

    const auto &initFileName = theDrawParam->InitFileName();
    std::string logErrorMessage;
    try
    {
        return FmiModifyEditdData::GetMacroParamFormula(MacroParamSystem(), theDrawParam);
    }
    catch(std::exception &e)
    {
        logErrorMessage = e.what();
    }
    catch(...)
    {
        logErrorMessage = std::string("Couldn't find macroParam '") + initFileName + "'";
    }
    // Raportoidaan puuttuvasta macroParamista vain kerran per ajo
    if(reportedMissingMacroParams.find(initFileName) == reportedMissingMacroParams.end())
    {
        reportedMissingMacroParams.insert(initFileName);
        LogMessage(logErrorMessage, CatLog::Severity::Error, category);
    }

    return "";
}

bool MakeMacroParamDrawingLayerCacheChecks(boost::shared_ptr<NFmiDrawParam> &drawParam, NFmiFastQueryInfo &theInfo, NFmiInfoData::Type theType, NFmiMapViewDescTop &descTop, unsigned int theDescTopIndex, int cacheRowNumber, const std::string &theFileName)
{
    if(drawParam->DataType() == NFmiInfoData::kMacroParam)
    {
        std::string macroParamStr = DoGetMacroParamFormula(drawParam, CatLog::Category::Visualization);
        MacroParamDataChecker macroParamDataChecker;
        auto macroParamDataInfoVector = macroParamDataChecker.getCalculationParametersFromMacroPram(macroParamStr);
        for(const auto &macroParamDataInfo : macroParamDataInfoVector)
        {
            if(theInfo.Param(macroParamDataInfo.dataIdent_))
            {
                // Jos macroParamDataInfo:n level on 'tyhj‰' (ident = 0, tarkoittaa pinta parametria) tai jos annettu level lˆytyy infosta
                if(macroParamDataInfo.level_.GetIdent() == 0 || theInfo.Level(macroParamDataInfo.level_))
                {
                    if(DoMacroParamVerticalDataChecks(theInfo, theType, macroParamDataInfo))
                    {
                        // clean image cache row
                        descTop.MapViewCache().MakeRowDirty(cacheRowNumber);
                        // MacroParam data cachen rivit alkavat 1:st‰, joten image-cachen riviin on lis‰tt‰v‰ +1
                        MacroParamDataCache().clearMacroParamCache(theDescTopIndex, cacheRowNumber + 1, drawParam->InitFileName());
                        MacroParamDirtiesCacheRowTraceLog(drawParam, macroParamDataInfo, theDescTopIndex, cacheRowNumber, theFileName);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

int FilterDialogUpdateStatus(void){return itsFilterDialogUpdateStatus;};
void FilterDialogUpdateStatus(int newState){itsFilterDialogUpdateStatus = newState;};

NFmiMetTime AdjustTimeToDescTopTimeStep(unsigned int theDescTopIndex, const NFmiMetTime& theTime)
{
	NFmiMetTime aTime(theTime);
	short timeStepInMinutes = static_cast<short>(::round(MapViewDescTop(theDescTopIndex)->TimeControlTimeStep() * 60.f));
	if(timeStepInMinutes == 0) // ei voi olla 0 timesteppi, muuten kaatuu (negatiivisesta en tied‰)
		timeStepInMinutes = 60; // h‰t‰ korjaus defaultti arvoksi jos oli 0
	if(aTime.GetTimeStep() > timeStepInMinutes)
		aTime.SetTimeStep(timeStepInMinutes);
	return aTime;
}

bool CurrentTimeForAllDescTops(const NFmiMetTime& newCurrentTime)
{
	for(size_t i = 0; i < itsMapViewDescTopList.size(); i++)
	{
		NFmiMetTime aTime(AdjustTimeToDescTopTimeStep(static_cast<unsigned int>(i), newCurrentTime));
		MapViewDescTop(static_cast<unsigned int>(i))->CurrentTime(aTime);
	}
	return true;
}

bool CurrentTime(unsigned int theDescTopIndex, const NFmiMetTime& newCurrentTime, bool fStayInsideAnimationTimes = false)
{
	if(theDescTopIndex == CtrlViewUtils::kDoAllMapViewDescTopIndex)
		return CurrentTimeForAllDescTops(newCurrentTime);

	NFmiMetTime aTime(AdjustTimeToDescTopTimeStep(theDescTopIndex, newCurrentTime));
	aTime = CalcAnimationRestrictedTime(theDescTopIndex, aTime, fStayInsideAnimationTimes);
	MapViewDescTop(theDescTopIndex)->CurrentTime(aTime);

	UpdateTimeInLockedDescTops(aTime, theDescTopIndex);
	return true;
}

bool UndoData(void)
{
	return FmiModifyEditdData::UndoData(GenDocDataAdapter());
}

bool RedoData(void)
{
	return FmiModifyEditdData::RedoData(GenDocDataAdapter());
}

boost::shared_ptr<NFmiFastQueryInfo> EditedInfo(void)
{
	if(itsSmartInfoOrganizer)
	{
		boost::shared_ptr<NFmiFastQueryInfo> info = itsSmartInfoOrganizer->FindInfo(NFmiInfoData::kEditable);
		// 27.12.2001/Marko Jouduin lis‰‰m‰‰n t‰h‰n levelin s‰‰tˆ virityksen, koska joskus
		// editoitava data ei ole leveleiden suhteen kohdallaan. T‰m‰ korjaa asian ainakin tilap‰isesti.
		// Quickfix joka pit‰isi tehd‰ kai muillekin datoille kuin editoitavalle datalle.
		if(info && info->SizeLevels() == 1)
		{
			unsigned long levelIndex = info->LevelIndex();
			if(levelIndex == 1 || levelIndex == static_cast<unsigned long>(-1)) // level indeksit 1 ja -1 tarkoittavat taulukon ulkopuolisia indeksej‰ t‰ss‰ tapauksessa (0 on oikea).
			{
				info->FirstLevel(); // korjataan levelindeksi 0:aan
				LogMessage("Editoitava-data oli mennyt leveleiden osalta 'v‰‰r‰‰n' tilaan, korjataan asia t‰ss‰.", CatLog::Severity::Warning, CatLog::Category::Editing);
			}
		}
		return info;
	}
	return boost::shared_ptr<NFmiFastQueryInfo>(); // jos muu ei auta, palautetaan tyhj‰ shared_ptr
}

bool CheckEditedDataForStartUpLoadErrors(int theMessageBoxButtunOptions)
{
	if(fStartUpDataLoadCheckDone == false)
	{
		fStartUpDataLoadCheckDone = true;
		boost::shared_ptr<NFmiFastQueryInfo> editedInfo = EditedInfo();
		if(editedInfo)
		{
			std::string errorStr;
			try
			{
				if(itsPossibleUsedDataLoadingGrid && ((editedInfo->HPlaceDescriptor() == *itsPossibleUsedDataLoadingGrid) == false))
					throw std::runtime_error("Edited data grid or projection is not the same as configured.");

				if(fUseEditedDataParamDescriptor && ((itsEditedDataParamDescriptor == editedInfo->ParamDescriptor()) == false))
					throw std::runtime_error("Edited data params were not the same as configured.");

				return true; // kaikki hyvin, voidaan lopettaa
			}
			catch(std::exception &e)
			{
				errorStr = e.what();
			}
			catch(...)
			{ // t‰h‰n ei pit‰isi menn‰
				errorStr = "Unknown error with edited datan.";
			}
			std::string dlgTitleStr("Problem with edited data");
			std::string msgStr;
			msgStr += "Edited data is not as it is configured:\n";
			msgStr += errorStr;
			msgStr += "\n\nThis is due the data was speed-loaded at the startup without checkings.\n";
			msgStr += "YOU SHOULD load proper data by pressing the Load Data -button.\n\n";
			msgStr += "Press Ok to ignore this warning and continue. Press Cancel otherwise.\n";

			LogMessage(msgStr, CatLog::Severity::Warning, CatLog::Category::Editing);
			if(theMessageBoxButtunOptions == MB_OKCANCEL)
			{
                if(::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(msgStr.c_str()), CA2T(dlgTitleStr.c_str()), theMessageBoxButtunOptions) == IDOK)
					return true;
				else
				{
					fStartUpDataLoadCheckDone = false; // laitetaan takaisin falseksi (t‰m‰ tietokantaan vienti on poikkeustapaus)
					return false;
				}
			}
			else
                ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(msgStr.c_str()), CA2T(dlgTitleStr.c_str()), theMessageBoxButtunOptions);

			return false;
		}
	}
	return true;
}

// Heitt‰‰ poikkeuksen, jos on tehty varoitus
void WarnUserIfProblemWithEditedData(bool fCancelPossible)
{
	if(EditedDataNotInPreferredState())
	{
		std::string msgStr("Current edited data is not suitable to be sent to the database.\n\n");
		if(fCancelPossible)
			msgStr += "SmartMet won't do what you were going to do here now, but next time there is no warning and\nSmartMet wil do this even if it's not preferred action.\n\n";
		else
			msgStr += "SmartMet will do the modifications you were going to do here.\n\n";
		msgStr += "YOU SHOULD do the data loading properly:\n\n => Press Load Data -button 'after' pressing OK button here.";
		std::string dlgTitleStr("Problems with current edited data!");
        ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(msgStr.c_str()), CA2T(dlgTitleStr.c_str()), MB_OK | MB_ICONERROR);
		throw std::runtime_error("Cancel editing action");
	}
}

bool DoTimeSeriesValuesModifying(boost::shared_ptr<NFmiDrawParam> &theModifiedDrawParam, NFmiMetEditorTypes::Mask fUsedMask, NFmiTimeDescriptor& theTimeDescriptor, checkedVector<double> &theModificationFactorCurvePoints, NFmiMetEditorTypes::FmiUsedSmartMetTool theEditorTool, bool fUseSetForDiscreteData, int theUnchangedValue = -1)
{
	// Tehd‰‰n aikasarjamuokkauksille progress ja peruutus dialogi ja toiminnot.
	// Aluksi vain control-point moodille!!!
	if(MetEditorOptionsData().ControlPointMode())
	{
        std::string operationText = ::GetDictionaryString("Doing Control-Point modifications...");
        bool operationTextIsWarning = false;
        if(!UseMultiProcessCpCalc())
        {
            operationText = ::GetDictionaryString("Not using MP-CP option while doing Control-\r\nPoint modifications, this way it will be slower");
            operationTextIsWarning = true;
        }
		NFmiStopFunctor stopper;
		CFmiOperationProgressDlg dlg(operationText, operationTextIsWarning, stopper, ApplicationInterface::GetSmartMetViewAsCView());
		NFmiThreadCallBacks threadCallBacks(&stopper, &dlg);

		FmiModifyEditdData::ModifyFunctionParamHolder modifyFunctionParamHolder(GenDocDataAdapter());
		modifyFunctionParamHolder.itsModifiedDrawParam = theModifiedDrawParam;
		modifyFunctionParamHolder.fUsedMask = fUsedMask;
		modifyFunctionParamHolder.itsEditorTool = theEditorTool;
		modifyFunctionParamHolder.fDoMultiThread = UseMultithreaddingWithModifyingFunctions();
		modifyFunctionParamHolder.itsThreadCallBacks = &threadCallBacks;

		// HUOM! Opin t‰ss‰ kaksi asiaa boost:in threadeista:
		// 1. K‰ytett‰v‰ll‰ funktiolla ei voi olla kuin max 9 parametria.
		// 2. boost::thread-templaatti konstruktori ei osaa hanskata saman nimisi‰ funktioita (eri parametreilla)
//		return FmiModifyEditdData::DoTimeSerialModifications(GenDocDataAdapter(), theModifiedDrawParam, fUsedMask, theTimeDescriptor, theModificationFactorCurvePoints, theEditorTool, fUseSetForDiscreteData, theUnchangedValue, UseMultithreaddingWithModifyingFunctions());
		boost::thread wrk_thread(FmiModifyEditdData::DoTimeSerialModifications2, boost::ref(modifyFunctionParamHolder), boost::ref(theTimeDescriptor), boost::ref(theModificationFactorCurvePoints), fUseSetForDiscreteData, theUnchangedValue);
		dlg.DoModal(); // t‰m‰ dialogi lopettaa kun threadissa kutsutaan threadCallBacks:ille DoPostMessage joko ID_MESSAGE_WORKING_THREAD_COMPLETED tai ID_MESSAGE_WORKING_THREAD_CANCELED parametrilla
        ApplicationInterface::GetApplicationInterfaceImplementation()->PostMessageToDialog(SmartMetViewId::TimeSerialView, ID_MESSAGE_TIME_SERIAL_MODIFICATION_ENDED);
		return modifyFunctionParamHolder.fReturnValue;
	}
	else
		return FmiModifyEditdData::DoTimeSerialModifications(GenDocDataAdapter(), theModifiedDrawParam, fUsedMask, theTimeDescriptor, theModificationFactorCurvePoints, theEditorTool, fUseSetForDiscreteData, theUnchangedValue, UseMultithreaddingWithModifyingFunctions(), 0);
}

// T‰t‰ kutusutaan ulkopuolelta
bool DoSmartToolEditing(const std::string &theSmartToolText, const std::string &theRelativePathMacroName, bool fSelectedLocationsOnly)
{
    std::string operationText = ::GetDictionaryString("Doing smarttool modifications...");
    bool operationTextIsWarning = false;
    NFmiStopFunctor stopper;
	CFmiOperationProgressDlg dlg(operationText, operationTextIsWarning, stopper, ApplicationInterface::GetSmartMetViewAsCView());
	NFmiThreadCallBacks threadCallBacks(&stopper, &dlg);

	FmiModifyEditdData::ModifyFunctionParamHolder modifyFunctionParamHolder(GenDocDataAdapter());
	modifyFunctionParamHolder.fDoMultiThread = UseMultithreaddingWithModifyingFunctions();
	modifyFunctionParamHolder.itsThreadCallBacks = &threadCallBacks;

	boost::thread wrk_thread(FmiModifyEditdData::DoSmartToolEditing2, boost::ref(modifyFunctionParamHolder), boost::ref(theSmartToolText), boost::ref(theRelativePathMacroName), fSelectedLocationsOnly);
	dlg.DoModal(); // t‰m‰ dialogi lopettaa kun threadissa kutsutaan threadCallBacks:ille DoPostMessage joko ID_MESSAGE_WORKING_THREAD_COMPLETED tai ID_MESSAGE_WORKING_THREAD_CANCELED parametrilla
    ApplicationInterface::GetApplicationInterfaceImplementation()->PostMessageToDialog(SmartMetViewId::SmartToolDlg, ID_MESSAGE_SMARTTOOL_MODIFICATION_ENDED);
	return modifyFunctionParamHolder.fReturnValue;
}

boost::shared_ptr<NFmiAreaMaskList> ParamMaskListMT(void)
{
	WriteLock lock(gParamMaskListMutex);
	if(itsParamMaskListMT)
		return boost::shared_ptr<NFmiAreaMaskList>(new NFmiAreaMaskList(*itsParamMaskListMT));
	else
		return itsParamMaskListMT;
}

// T‰m‰ tekee kopion annetusta paramMaskListasta, ja kopiointi on suojassa threadi lukolla
void ParamMaskListMT(const boost::shared_ptr<NFmiAreaMaskList> &theParamMaskList)
{
	WriteLock lock(gParamMaskListMutex);
	if(theParamMaskList)
		itsParamMaskListMT = boost::shared_ptr<NFmiAreaMaskList>(new NFmiAreaMaskList(*theParamMaskList));
	else
		itsParamMaskListMT = theParamMaskList;
    MaskChangedDirtyActions();
}

// s‰‰t‰‰ annetun timebagin niin, ett‰ se on editoitavien aikojen sis‰ll‰
NFmiTimeBag AdjustToEditedDataTimeBag(const NFmiTimeBag& wantedTimebag)
{
	return AdjustTimeBagToGivenTimeBag(EditedDataTimeBag(), wantedTimebag);
}

// t‰t‰ metodia voidaan k‰ytt‰‰ aina kaikkialla kun tehd‰‰n muokkauksia dataan
// theModifyingTool 1 = muokkausdialogi, 2 = pensseli ja 3 = aikasarjaeditori
// kun tyˆkalu on 2 tai 3, annetaan mukana myˆs editoitava parametri
bool CheckAndValidateAfterModifications(NFmiMetEditorTypes::FmiUsedSmartMetTool theModifyingTool, bool fMakeDataSnapshotAction, unsigned int theLocationMask, FmiParameterName theParam, bool fPasteAction = false)
{
	return FmiModifyEditdData::CheckAndValidateAfterModifications(GenDocDataAdapter(), theModifyingTool, fMakeDataSnapshotAction, theLocationMask, theParam, UseMultithreaddingWithModifyingFunctions(), fPasteAction);
}

bool MakeDataValiditation(void)
{
	return FmiModifyEditdData::MakeDataValiditation(GenDocDataAdapter(), UseMultithreaddingWithModifyingFunctions());
}

void SnapShotData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiDataIdent &theDataIdent, const string &theModificationText
				 , const NFmiMetTime &theStartTime, const NFmiMetTime &theEndTime)
{
	FmiModifyEditdData::SnapShotData(GenDocDataAdapter(), theInfo, theDataIdent, theModificationText, theStartTime, theEndTime);
}


// merkitsee annettuun parambagiin aktiivisiksi ne parametrit, jotka ovat voineet muutttua
// kun annettua parametria on muutettu
bool MarkAffectedParams(const NFmiDataIdent& theParam, NFmiParamBag& thePossibleAffectedParams)
{ // korjaa paremmaksi!!!!!
	thePossibleAffectedParams.SetActive(*theParam.GetParam(), true);
	return true;
}

boost::shared_ptr<NFmiDrawParam> ActiveDrawParam(unsigned int theDescTopIndex, int theRowIndex)
{
	NFmiDrawParamList* list = DrawParamList(theDescTopIndex, theRowIndex);
	if(list)
	{
		for(list->Reset(); list->Next(); )
			if(list->Current()->IsActive())
				return list->Current();
	}
	return boost::shared_ptr<NFmiDrawParam>();
}

// T‰m‰ on otettu k‰yttˆˆn ,ett‰ voisi unohtaa tuon kamalan indeksi jupinan, mik‰ johtuu
// 'virtuaali' karttan‰yttˆriveist‰.
// Karttarivi indeksit alkavat 1:st‰. 1. rivi on 1 ja 2. rivi on kaksi jne.
boost::shared_ptr<NFmiDrawParam> ActiveDrawParamWithRealRowNumber(unsigned int theDescTopIndex, int theRowIndex)
{
	NFmiDrawParamList* list = DrawParamListWithRealRowNumber(theDescTopIndex, theRowIndex);
	if(list)
	{
		for(list->Reset(); list->Next(); )
			if(list->Current()->IsActive())
				return list->Current();
	}
	return boost::shared_ptr<NFmiDrawParam>();
}

boost::shared_ptr<NFmiDrawParam> DefaultEditedDrawParam(void)
{
	return itsDefaultEditedDrawParam;
}

const NFmiTimeDescriptor& EditedDataTimeDescriptor(void)
{	// HUOM!!! tarkoituksella ei 0-pointer tarkastusta!!!!
	return EditedInfo()->TimeDescriptor();
}

const NFmiTimeBag& EditedDataTimeBag(void)
{
	static NFmiTimeBag timeBag;
	boost::shared_ptr<NFmiFastQueryInfo> editedInfo = EditedInfo();
	if(editedInfo)
	{
		if(editedInfo->TimeDescriptor().ValidTimeBag())
			return *editedInfo->TimeDescriptor().ValidTimeBag();
		else
		{
			timeBag = NFmiTimeBag(editedInfo->TimeDescriptor().FirstTime(), editedInfo->TimeDescriptor().LastTime(), 60);
			return timeBag;
		}
	}
	else
		return MaximalCoverageTimeBag();
}

void TimeControlViewTimes(unsigned int theDescTopIndex, const NFmiTimeDescriptor &newTimeDescriptor)
{
	if(theDescTopIndex == CtrlViewUtils::kDoAllMapViewDescTopIndex)
	{
		unsigned int ssize = static_cast<unsigned int>(itsMapViewDescTopList.size());
		for(unsigned int i = 0; i<ssize; i++)
			MapViewDescTop(i)->TimeControlViewTimes(newTimeDescriptor);
	}
	else
		MapViewDescTop(theDescTopIndex)->TimeControlViewTimes(newTimeDescriptor);
}

const NFmiTimeDescriptor& TimeControlViewTimes(unsigned int theDescTopIndex)
{
	return 	MapViewDescTop(theDescTopIndex)->TimeControlViewTimes();
}

// funktio palauttaa oikean rivi numero. Karttan‰yttˆ systeemiss‰ on ik‰v‰ virtuaali rivitys
// jolloin karttan‰yttˆ-luokka voi luulla olevansa rivill‰ 1 (= yli karttan‰ytˆss‰ oleva rivi)
// vaikka onkin oikeasti vaikka rivill‰ 4. T‰llˆin pit‰‰ k‰ytt‰‰ apuna desctopin tietoja ett‰ 
// voidaan laskea oikea karttarivi.
// Oikeat karttarivit alkavat siis 1:st‰.
unsigned int GetRealRowNumber(unsigned int theDescTopIndex, int theRowIndex)
{
	if(theRowIndex >= gActiveViewRowIndexForTimeSerialView) // aikasarja rivi numerot ovat erikseen
		return theRowIndex;
	else
		return theRowIndex + MapViewDescTop(theDescTopIndex)->MapRowStartingIndex() - 1;
}

unsigned int GetRelativeRowNumber(unsigned int theDescTopIndex, int theRealRowIndex)
{
    if(theRealRowIndex >= gActiveViewRowIndexForTimeSerialView) // aikasarja rivi numerot ovat erikseen
        return theRealRowIndex;
    else
        return theRealRowIndex - MapViewDescTop(theDescTopIndex)->MapRowStartingIndex() + 1;
}

// muuta k‰ytt‰m‰‰n DrawParamListWithRealRowNumber-funktiota
NFmiDrawParamList* DrawParamList(unsigned int theDescTopIndex, int theRowIndex)
{
	if(theDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
		return CrossSectionViewDrawParamList(theRowIndex);
	if(theDescTopIndex == CtrlViewUtils::kFmiTimeSerialView || theRowIndex >= gActiveViewRowIndexForTimeSerialView) // haetaan aikasarjaikkunan drawparamlistia gActiveViewRowIndexForTimeSerialView on suuri luku (99)
		return itsTimeSerialViewDrawParamList;
	else
		return DrawParamListWithRealRowNumber(theDescTopIndex, theRowIndex + MapViewDescTop(theDescTopIndex)->MapRowStartingIndex() - 1); // ei haittaa, vaikka lis‰t‰‰n rowindeksiin joka kuvaa aikasarjaa jotain. gActiveViewRowIndexForTimeSerialView:iin
}

// T‰m‰ on otettu k‰yttˆˆn ,ett‰ voisi unohtaa tuon kamalan indeksi jupinan, mik‰ johtuu
// 'virtuaali' karttan‰yttˆriveist‰.
// Karttarivi indeksit alkavat 1:st‰. 1. rivi on 1 ja 2. rivi on kaksi jne.
NFmiDrawParamList* DrawParamListWithRealRowNumber(unsigned int theDescTopIndex, int theRealRowIndex)
{
	if(theDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
        return CrossSectionViewDrawParamList(theRealRowIndex);
    if(theRealRowIndex >= gActiveViewRowIndexForTimeSerialView || theDescTopIndex == CtrlViewUtils::kFmiTimeSerialView) // haetaan aikasarjaikkunan drawparamlistia gActiveViewRowIndexForTimeSerialView on suuri luku (99)
		return itsTimeSerialViewDrawParamList;
	NFmiMapViewDescTop *descTop = MapViewDescTop(theDescTopIndex);
    if(descTop)
    {
        int origListSize = static_cast<int>(descTop->DrawParamListVector()->NumberOfItems());
        if(theRealRowIndex <= origListSize)
            return descTop->DrawParamListVector()->Index(theRealRowIndex).CurrentPtr();
        else
            return 0;
    }
	else
		return 0;
}

bool MakeAndStoreFileDialogDirectoryMemory(const std::string& thePathAndFileName)
{
	unsigned char keno[3] = "\\";
    NFmiString tmpPath = thePathAndFileName;
	unsigned long index = tmpPath.SearchLast(keno);
	std::string path = tmpPath.GetChars(1,index);
	itsFileDialogDirectoryMemory = path;
	NFmiSettings::Set(string("MetEditor::FileDialogDirectoryMemory"), itsFileDialogDirectoryMemory, true);
	return true;
}

// oletus theErrorPath p‰‰ttyy / tai \ merkkiin.
void MoveFileToDirectory(const NFmiFileString &theFileNameWithPath, const std::string &theErrorPath) const
{
	NFmiFileSystem::CreateDirectory(theErrorPath);
	std::string errFileName(theErrorPath);
	errFileName += static_cast<char*>(theFileNameWithPath.FileName());
	NFmiFileSystem::RenameFile(static_cast<char*>(theFileNameWithPath), errFileName);
}

// haetaan EC datan luku polku pohjaksi. otetaan 'in' lopusta pois ja laitetaan "error" tilalle.
// t‰t‰ ei konffata tiedostoon, koska joutuisin muuttamaan konffitiedostoja ja niiden luku systeemi‰.
std::string GetErrorFilePath(void)
{
	NFmiString inPath(GetUsedDataLoadingInfo().ErrorFilePath());
	if(inPath.GetLen())
	{
		char ch = inPath[inPath.GetLen()];
		if(ch == '\\' || ch == '/')
		{ // jos viimeinen merkki oli kenoviiva, se poistetaan
			inPath = inPath.GetChars(1, inPath.GetLen()-1);
		}
		const unsigned char delim1[2] = "\\";
		const unsigned char delim2[2] = "/";
		unsigned long index1 = inPath.SearchLast(delim1);
		unsigned long index2 = inPath.SearchLast(delim2);
		unsigned long index = FmiMax(index1, index2);
		std::string errPath;
		if(index > 0)
			errPath = inPath.GetChars(1, index);
		else
			errPath = "";
		errPath += "error/";
		return errPath;
	}
	throw std::runtime_error("GetErrorFilePath : virhe hakemistoa ei voitu rakentaa.");
}

bool DataLoadingOK(bool noError)
{
	if(noError)
		return true;
	else
	{
		CDataLoadingProblemsDlg dlg(_TEXT("Error occured in GeneralDocImpl::DataLoadingOK."));
		if(dlg.DoModal() == IDOK)
			return false;
	}
	return true;
}

const NFmiMetTime& CurrentTime(unsigned int theDescTopIndex)
{
	if(theDescTopIndex > CtrlViewUtils::kFmiMaxMapDescTopIndex)
		return MapViewDescTop(0)->CurrentTime(); // erikois n‰ytˆille palautetaan vain p‰‰karttaikkunan valittu aika
	else
		return MapViewDescTop(theDescTopIndex)->CurrentTime();
}

void UpdateTimeInLockedDescTops(const NFmiMetTime &theTime, unsigned int theOrigDescTopIndex)
{
    // P‰ivitet‰‰n ajan muutoksessa myˆs aina luotausn‰yttˆ‰, jos s‰‰dˆt ovat kohdallaan
    if(GetMTATempSystem().SoundingTimeLockWithMapView() && GetMTATempSystem().TempViewOn())
        UpdateTempView();

	// Jos originaali ikkunassa on animaatio p‰‰ll‰ tai on edes animointi alue n‰kyviss‰, ei tehd‰ mit‰‰n
	if(MapViewDescTop(theOrigDescTopIndex)->AnimationDataRef().AnimationOn() || MapViewDescTop(theOrigDescTopIndex)->AnimationDataRef().ShowTimesOnTimeControl())
		return ;

	// eli jos origIndex oli p‰‰ikkuna (index = 0) tai apuikkuna oli lukittu p‰‰ikkunaan, silloin tehd‰‰n aika p‰ivityksi‰
	if(theOrigDescTopIndex == 0 || MapViewDescTop(theOrigDescTopIndex)->LockToMainMapViewTime())
	{
		unsigned int ssize = static_cast<unsigned int>(itsMapViewDescTopList.size());
		for(unsigned int i = 0; i<ssize; i++)
		{
			if(i == 0 || MapViewDescTop(i)->LockToMainMapViewTime())
			{
				// jos l‰pik‰yt‰v‰ss‰ ikkunassa on animointia tai edes anim.-ikkuna n‰kyviss‰, ei tehd‰ mit‰‰n
				if(MapViewDescTop(i)->AnimationDataRef().AnimationOn() || MapViewDescTop(i)->AnimationDataRef().ShowTimesOnTimeControl())
					continue;
				MapViewDescTop(i)->CurrentTime(theTime);
			}
		}
	}
}

void UpdateRowInLockedDescTops(unsigned int theOrigDescTopIndex)
{
    // eli jos origIndex oli p‰‰ikkuna (index = 0) tai apuikkuna oli lukittu p‰‰ikkunaan, silloin tehd‰‰n rivi p‰ivityksi‰
    if(theOrigDescTopIndex == 0 || MapViewDescTop(theOrigDescTopIndex)->LockToMainMapViewRow())
    {
        auto mapRowStartingIndex = MapViewDescTop(theOrigDescTopIndex)->MapRowStartingIndex();
        unsigned int desctopCount = static_cast<unsigned int>(itsMapViewDescTopList.size());
        for(unsigned int descTopIndex = 0; descTopIndex < desctopCount; descTopIndex++)
        {
            if(descTopIndex == 0 || MapViewDescTop(descTopIndex)->LockToMainMapViewRow())
            {
                MapViewDescTop(descTopIndex)->MapRowStartingIndex(mapRowStartingIndex);
            }
        }
    }
}

bool IsAnimationTimeBoxVisibleOverTimeControlView(unsigned int theDescTopIndex)
{
    const auto &animationTimes = MapViewDescTop(theDescTopIndex)->AnimationDataRef().Times();
    auto timeControlTimes = MapViewDescTop(theDescTopIndex)->TimeControlViewTimes().ValidTimeBag();
    if(timeControlTimes)
    {
        // Peitt‰‰kˆ animaatio ajat koko aikakontrolli-ikkunan
        if(animationTimes.IsInside(timeControlTimes->FirstTime()) && animationTimes.IsInside(timeControlTimes->LastTime()))
            return true;
        // Onko animaatio ajat kokonaan aikakontrolli-ikkunassa
        if(timeControlTimes->IsInside(animationTimes.FirstTime()) && timeControlTimes->IsInside(animationTimes.LastTime()))
            return true;
        // Onko animaation 1. aika on selke‰sti aikakontrolli-ikkunan sis‰ll‰
        if(timeControlTimes->IsInside(animationTimes.FirstTime()) && timeControlTimes->FirstTime() < animationTimes.FirstTime() && timeControlTimes->LastTime() > animationTimes.FirstTime())
            return true;
        // Onko animaation viimeinen aika on selke‰sti aikakontrolli-ikkunan sis‰ll‰
        if(timeControlTimes->IsInside(animationTimes.LastTime()) && timeControlTimes->FirstTime() < animationTimes.LastTime() && timeControlTimes->LastTime() > animationTimes.LastTime())
            return true;

        return false;
    }
    else
        return true; // time-control-view timebag missing => return true as inconclusive
}

NFmiMetTime CalcCenterAnimationTimeBoxTime(unsigned int theDescTopIndex)
{
    const auto &animationTimes = MapViewDescTop(theDescTopIndex)->AnimationDataRef().Times();
    auto totalDiffInMinutes = animationTimes.LastTime().DifferenceInMinutes(animationTimes.FirstTime());
    auto centerTime = animationTimes.FirstTime();
    centerTime.ChangeByMinutes(totalDiffInMinutes / 2);
    return centerTime;
}

// Move animation (light blue box) in the middle of time control view if following criterias are met:
// 1. View is in animation mode and time is changed by mouse wheel (fStayInsideAnimationTimes is on)
// 2. Animation time box is not visible.
// This action shows to the user that moving in time is restricted by animation time frame when using mouse wheel.
void SetAnimationBoxToVisibleIfNecessary(unsigned int theDescTopIndex, bool fStayInsideAnimationTimes)
{
    if(DoAnimationRestriction(theDescTopIndex, fStayInsideAnimationTimes))
    {
        if(!IsAnimationTimeBoxVisibleOverTimeControlView(theDescTopIndex))
        {
            auto newCenterTime = CalcCenterAnimationTimeBoxTime(theDescTopIndex);
            CenterTimeControlView(theDescTopIndex, newCenterTime, false);
        }
    }
}

// asettaa kaikki datat seuraavaan aikaan (jos mahdollista), riippuen aika-askeleesta
// k‰y l‰pi kaikki kartalla n‰kyv‰t datat ja asettaa ne oikeaan aikaan
bool SetDataToNextTime(unsigned int theDescTopIndex, bool fStayInsideAnimationTimes = false)
{
	NFmiMetTime newTime;
	if(theDescTopIndex == CtrlViewUtils::kDoAllMapViewDescTopIndex)
		return SetDataToNextTimeForAllDescTops();
	else
	{
        ActiveMapDescTopIndex(theDescTopIndex);
		newTime = CurrentTime(theDescTopIndex);
		short usedTimeStep = static_cast<short>(::round(MapViewDescTop(theDescTopIndex)->TimeControlTimeStep() * 60));
		newTime.SetTimeStep(1);//usedTimeStep);
		newTime.ChangeByMinutes(usedTimeStep);
		newTime = CalcAnimationRestrictedTime(theDescTopIndex, newTime, fStayInsideAnimationTimes);
		CurrentTime(theDescTopIndex, newTime);
        SetAnimationBoxToVisibleIfNecessary(theDescTopIndex, fStayInsideAnimationTimes);
	}
	UpdateTimeInLockedDescTops(newTime, theDescTopIndex);
	return true;
}

bool SetDataToNextTimeForAllDescTops(void)
{
	unsigned int ssize = static_cast<unsigned int>(itsMapViewDescTopList.size());
	for(unsigned int i = 0; i<ssize; i++)
		SetDataToNextTime(i);
	return true;
}

bool DoAnimationRestriction(unsigned int theDescTopIndex, bool fStayInsideAnimationTimes)
{
    if(fStayInsideAnimationTimes)
        return MapViewDescTop(theDescTopIndex)->AnimationDataRef().ShowTimesOnTimeControl();
    else
        return false;
}

NFmiMetTime CalcAnimationRestrictedTime(unsigned int theDescTopIndex, const NFmiMetTime &theTime, bool fStayInsideAnimationTimes)
{
	NFmiMetTime fixedTime = theTime;
	if(DoAnimationRestriction(theDescTopIndex, fStayInsideAnimationTimes))
	{
        NFmiAnimationData &animData = MapViewDescTop(theDescTopIndex)->AnimationDataRef();
        if(animData.Times().IsInside(fixedTime) == false)
		{
			NFmiTimeBag tmpTimes = animData.Times();
			if(tmpTimes.FindNearestTime(fixedTime))
				fixedTime = tmpTimes.CurrentTime();
		}
	}
	return fixedTime;
}

// asettaa kaikki datat edelliseen aikaan (jos mahdollista), riippuen aikaaskeleesta
// k‰y l‰pi kaikki kartalla n‰kyv‰t datat ja asettaa ne oikeaan aikaan
bool SetDataToPreviousTime(unsigned int theDescTopIndex, bool fStayInsideAnimationTimes = false)
{
	NFmiMetTime newTime;
	if(theDescTopIndex == CtrlViewUtils::kDoAllMapViewDescTopIndex)
		return SetDataToPreviousTimeForAllDescTops();
	else
	{
        ActiveMapDescTopIndex(theDescTopIndex);
		newTime = CurrentTime(theDescTopIndex);
		short usedTimeStep = static_cast<short>(::round(MapViewDescTop(theDescTopIndex)->TimeControlTimeStep() * 60));
		newTime.SetTimeStep(1);//usedTimeStep);
		newTime.ChangeByMinutes(-usedTimeStep);
		newTime = CalcAnimationRestrictedTime(theDescTopIndex, newTime, fStayInsideAnimationTimes);
		CurrentTime(theDescTopIndex, newTime);
        SetAnimationBoxToVisibleIfNecessary(theDescTopIndex, fStayInsideAnimationTimes);
    }
	UpdateTimeInLockedDescTops(newTime, theDescTopIndex);
	return true;
}

bool SetDataToPreviousTimeForAllDescTops(void)
{
	unsigned int ssize = static_cast<unsigned int>(itsMapViewDescTopList.size());
	for(unsigned int i = 0; i<ssize; i++)
		SetDataToPreviousTime(i);
	return true;
}

		// HUOM!!!! theParamIndex on oikeasti FmiParameter tyyppi‰, muuta!
bool SelectNewParamForSelectionTool(int theParamId)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	if(itsLocationSelectionTool2)
	{
		boost::shared_ptr<NFmiFastQueryInfo> info = itsSmartInfoOrganizer->FindInfo(NFmiInfoData::kStationary);
		if(info)
		{
			boost::shared_ptr<NFmiFastQueryInfo> infoCopy = boost::shared_ptr<NFmiFastQueryInfo>(dynamic_cast<NFmiFastQueryInfo*>(info->Clone())); // pit‰‰ tehd‰ kopio
			if(infoCopy)
			{
				infoCopy->Param(static_cast<FmiParameterName>(theParamId));
				itsLocationSelectionTool2->Info(infoCopy);
				return true;
			}
		}
	}
	return false;
}

// asettaa zoomausalueen riippuvaiseksi yhden karttaikkunan x/y-suhteesta
void DoAutoZoom(unsigned int theDescTopIndex)
{
    if(ApplicationWinRegistry().KeepMapAspectRatio())
	{
		boost::shared_ptr<NFmiArea> oldArea = MapViewDescTop(theDescTopIndex)->MapHandler()->Area();
		if(oldArea)
		{
			NFmiMapViewDescTop *mapViewDescTop = MapViewDescTop(theDescTopIndex);
			double currentAreaAspectRatio = oldArea->WorldXYAspectRatio();
			double clientAspectRatio = mapViewDescTop->ClientViewXperYRatio();
			if(CtrlViewUtils::IsEqualEnough(clientAspectRatio, currentAreaAspectRatio, 0.000001) == false) // T‰ss‰ pit‰isi tutkia mik‰ on sellainen pieni arvo, jonka verran ratiot saavat heitt‰‰, ettei t‰t‰ kuitenkaan tarvitsisi laskea
			{ // pit‰‰ muuttaa zoomattua areaa niin, ett‰ sen aspectratio vastaa ikkunan aspectratiota
				boost::shared_ptr<NFmiArea> newArea(oldArea->CreateNewArea(mapViewDescTop->ClientViewXperYRatio(), kCenter, true));
				if(newArea)
				{
					SetMapArea(theDescTopIndex, newArea);
				}
			}
		}
	}
}

//----------------------------------------------------------------
// UpdateModifiedDrawParam		M.K.  26.4.99
//----------------------------------------------------------------
// P‰ivitt‰‰ uuden drawParamin ne ominaisuudet, joita voi muuttaa
// Ominaisuudet -dialogissa.
void UpdateModifiedDrawParam(boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool fGroundData)
{
	if(theDrawParam)
	{
		if(!theDrawParam->ViewMacroDrawParam()) // ei p‰ivitet‰ ominaisuuksia modified listasta, jos oli viewmacro-drawparam
		{
			if(itsModifiedPropertiesDrawParamList.Find(theDrawParam, fGroundData)) // 1999.08.30/Marko
				theDrawParam->Init(itsModifiedPropertiesDrawParamList.Current());
			else
			{ // ei lˆytynyt 'lis‰tt‰v‰‰' drawParamia listasta, joten lis‰t‰‰n t‰ss‰ sen kopio modified-listaan (nyky‰‰n ei lis‰t‰ j‰rjettˆm‰sti kaikkien datojen kaikki drawParam yhdistelmi‰)
				boost::shared_ptr<NFmiDrawParam> tmpDrawParam(new NFmiDrawParam(*theDrawParam.get()));
				itsModifiedPropertiesDrawParamList.Add(tmpDrawParam, fGroundData);
			}
		}
	}
}

// yll‰ oleva versio p‰ivitt‰‰ modified listasta otetulla theDrawParamia
// tein version joka muuttaa modifiedlistan otusta annetulla theDrawParamilla
// lis‰sin myˆs annetun drawlist-rivin likaamisen
void UpdateModifiedDrawParamMarko(unsigned int theDescTopIndex, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex)
{
	if(theDrawParam)
    {
		if(!theDrawParam->ViewMacroDrawParam()) // jos kyseess‰ oli viewmacro-drawparam, ei p‰ivitet‰ modified-listalla olevaa drawparamia!
        {
        	boost::shared_ptr<NFmiFastQueryInfo> info = InfoOrganizer()->Info(theDrawParam, false, true);
            bool groundData = true;
	        if(info)
		        groundData = info->SizeLevels() <= 1;

			if(itsModifiedPropertiesDrawParamList.Find(theDrawParam, groundData))
				itsModifiedPropertiesDrawParamList.Current()->Init(theDrawParam);
        }
    }
	NFmiDrawParamList *drawParamList = DrawParamList(theDescTopIndex, theRowIndex);
    if(drawParamList)
    {
		drawParamList->Dirty(true);
        DrawParamSettingsChangedDirtyActions(theDescTopIndex, GetRealRowNumber(theDescTopIndex, theRowIndex), theDrawParam);
    }
	if(theRowIndex == gActiveViewRowIndexForTimeSerialView)
		TimeSerialViewDirty(true);
	return;
}

struct CaseInsensitiveStringComparison
{
	bool operator()(const std::string &theFirst, const std::string &theSecond)
	{
		return stricmp(theFirst.c_str(), theSecond.c_str()) < 0;
	}
};

struct MenuCreationSettings
{
	MenuCreationSettings(void)
	:itsDescTopIndex(static_cast<unsigned int>(-1))
	,itsMenuCommand(kFmiNoCommand)
	,fLevelDataOnly(false)
	,fGridDataOnly(false)
	,fDoMapMenu(false)
	,fAcceptMacroParams(false)
	,fAcceptCalculateParams(false)
	,fMakeCustomMenu(false)
	{}

	void SetMapViewSettings(unsigned int theDescTopIndex, FmiMenuCommandType theMenuCommand)
	{
		itsDescTopIndex = theDescTopIndex;
		itsMenuCommand = theMenuCommand;

		fLevelDataOnly = false;
		fGridDataOnly = false;
		fDoMapMenu = true;
		fAcceptMacroParams = true;
		fAcceptCalculateParams = false;
		fMakeCustomMenu = false;
	}
	void SetCrossSectionSettings(FmiMenuCommandType theMenuCommand)
	{
		itsDescTopIndex = static_cast<unsigned int>(-1);
		itsMenuCommand = theMenuCommand;

		fLevelDataOnly = true;
		fGridDataOnly = true;
		fDoMapMenu = false;
		fAcceptMacroParams = true;
		fAcceptCalculateParams = false;
		fMakeCustomMenu = false;
	}
	void SetMaskSettings(FmiMenuCommandType theMenuCommand)
	{
		itsDescTopIndex = static_cast<unsigned int>(-1);
		itsMenuCommand = theMenuCommand;

		fLevelDataOnly = false;
		fGridDataOnly = true;
		fDoMapMenu = false;
		fAcceptMacroParams = false;
		fAcceptCalculateParams = true;
		fMakeCustomMenu = false;
	}
	void SetTimeSerialSettings(FmiMenuCommandType theMenuCommand)
	{
		itsDescTopIndex = static_cast<unsigned int>(-1);
		itsMenuCommand = theMenuCommand;

		fLevelDataOnly = false;
		fGridDataOnly = false;
		fDoMapMenu = false;
		fAcceptMacroParams = false;
		fAcceptCalculateParams = false;
		fMakeCustomMenu = false;
	}

	unsigned int itsDescTopIndex;
	FmiMenuCommandType itsMenuCommand; // mik‰ komento on nyt kyseess‰
	bool fLevelDataOnly; // vain vertikaali data kelpaa
	bool fGridDataOnly; // vain hiladata kelpaa
	bool fDoMapMenu; // tietyt parametri saa lis‰t‰ vain karttan‰yttˆˆn
	bool fAcceptMacroParams; // n‰it‰ ei sallita toistaiseksi kuin karttan‰ytˆlle ja poikkileikkaukseen
	bool fAcceptCalculateParams; // n‰m‰ (lat, lon ja elevation angle) hyv‰ksyt‰‰n vain maskeiksi
	bool fMakeCustomMenu; // t‰m‰ pit‰‰ asettaa erikseen p‰‰lle kun rakennetaan custom-menuja
};

void CreateParamSelectionBasePopup(const MenuCreationSettings &theMenuSettings, NFmiMenuItemList *thePopupMenu, const std::string &theMenuItemDictionaryStr)
{
	std::string menuString = ::GetDictionaryString(theMenuItemDictionaryStr.c_str());
	auto menuItem = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable);
	NFmiMenuItemList *menuList = new NFmiMenuItemList;

// ********** lis‰t‰‰n "editoitava" data osa *************************
	AddFirstOfDataTypeToParamSelectionPopup(theMenuSettings, menuList, NFmiInfoData::kEditable, "MapViewParamPopUpEdited");

// ********** lis‰t‰‰n "vertailu-data" data osa *************************
	AddFirstOfDataTypeToParamSelectionPopup(theMenuSettings, menuList, NFmiInfoData::kCopyOfEdited, "MapViewParamPopUpComparisonData");

// ********** lis‰t‰‰n "operatiivinen data" data osa *************************
	AddFirstOfDataTypeToParamSelectionPopup(theMenuSettings, menuList, NFmiInfoData::kKepaData, "MapViewParamPopUpOperativeData");

// ********** lis‰t‰‰n "helpEditor mode data" data osa *************************
	AddFirstOfDataTypeToParamSelectionPopup(theMenuSettings, menuList, NFmiInfoData::kEditingHelpData, "MapViewParamPopUpHelpEditorData");

// ********** lis‰t‰‰n eri mallien omat aliosiot (esim. Hirlam Ecmwf, GFS, jne.)  *************************
	int modelCount = static_cast<int>(this->ProducerSystem().Producers().size());
	int i=0;
	for(i=0; i<modelCount; i++)
		AddConfiguredModelProducerDataToParamSelectionPopup(theMenuSettings, menuList, i+1, NFmiInfoData::kViewable, NFmiInfoData::kHybridData, NFmiInfoData::kModelHelpData, NFmiInfoData::kClimatologyData, NFmiInfoData::kTrajectoryHistoryData, NFmiInfoData::kAnalyzeData);

// ********** lis‰t‰‰n havaintoparametri osa *************************
	menuString = ::GetDictionaryString("MapViewParamPopUpObservations");
    auto menuItem6 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kObservations);
	AddObservationDataToParamSelectionPopup(theMenuSettings, menuItem6.get());
	if(menuItem6->SubMenu()->NumberOfMenuItems() > 0)
		menuList->Add(std::move(menuItem6));
// ********** lis‰t‰‰n havaintoparametri osa *************************

// ********** WMS *************************
    AddWmsDataToParamSelectionPopup(theMenuSettings, menuList, NFmiInfoData::kWmsData);

// ********** lis‰t‰‰n apudata-parametri osa *************************
	menuString = ::GetDictionaryString("MapViewParamPopUpHelpData");
    auto menuItem7 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kViewable);
	AddHelpDataToParamSelectionPopup(theMenuSettings, menuItem7.get());
	if(menuItem7->SubMenu()->NumberOfMenuItems() > 0)
		menuList->Add(std::move(menuItem7));
// ********** lis‰t‰‰n apudata-parametri osa *************************

// ********** lis‰t‰‰n talletetut macroParamit osa *************************
	if(theMenuSettings.fAcceptMacroParams)
	{
		if(theMenuSettings.fDoMapMenu)
			AddMacroParamPartToPopUpMenu(theMenuSettings, menuList, NFmiInfoData::kMacroParam);
		else // viel‰ ei ole muita fAcceptMacroParams -tapauksia kuin poikkileikkaus-makroParamit
			AddMacroParamPartToPopUpMenu(theMenuSettings, menuList, NFmiInfoData::kCrossSectionMacroParam);
	}

// ********** lis‰t‰‰n laskettavat parametrit *************************
	if(theMenuSettings.fAcceptCalculateParams)
		AddCalculatedParamsToMenu(theMenuSettings, menuList, "MapViewMaskSelectionPopUpCalculatedParams");

	AddCustomFoldersToMenu(theMenuSettings, menuList);

	menuItem->AddSubMenu(menuList);
	thePopupMenu->Add(std::move(menuItem));
}

#ifndef DISABLE_CPPRESTSDK
void AddAllWmsProducersToParamSelectionPopup(
    const MenuCreationSettings &theMenuSettings,
    NFmiInfoData::Type theDataType,
    NFmiMenuItemList *producerMenuList,
    const Wms::CapabilityTree& layerTree
    )
{
    auto subMenuItem = std::make_unique<NFmiMenuItem>(
        theMenuSettings.itsDescTopIndex,
        layerTree.value.name,
        NFmiDataIdent(NFmiParam(layerTree.value.paramId, layerTree.value.name), layerTree.value.producer),
        theMenuSettings.itsMenuCommand,
        NFmiMetEditorTypes::kFmiParamsDefaultView,
        nullptr,
        theDataType
    );

    try
    {
        const auto& layerTreeCasted = dynamic_cast<const Wms::CapabilityNode&>(layerTree);
        auto* subMenuList = subMenuItem->SubMenu();
        if(!subMenuList)
        {
            subMenuList = new NFmiMenuItemList;
        }

        for(const auto& child : layerTreeCasted.children)
        {
            AddAllWmsProducersToParamSelectionPopup(theMenuSettings, theDataType, subMenuList, *child);
        }
        subMenuItem->AddSubMenu(subMenuList);
    }
    catch(const std::exception&)
    {

    }
    producerMenuList->Add(std::move(subMenuItem));
}
#endif // DISABLE_CPPRESTSDK

//Wms datan lis‰ys popupiin
void AddWmsDataToParamSelectionPopup(const MenuCreationSettings &theMenuSettings, NFmiMenuItemList *theMenuItemList, NFmiInfoData::Type theDataType)
{
#ifndef DISABLE_CPPRESTSDK
    if(theMenuSettings.fDoMapMenu)
    {
        try
        {
            if(!WmsSupport().isConfigured())
                return;
            const auto& layerTree = WmsSupport().peekCapabilityTree();
            auto menuItem = std::make_unique<NFmiMenuItem>(
                theMenuSettings.itsDescTopIndex,
                "WMS",
                NFmiDataIdent(NFmiParam(layerTree.value.paramId, layerTree.value.name), layerTree.value.producer),
                theMenuSettings.itsMenuCommand,
                NFmiMetEditorTypes::kFmiParamsDefaultView,
                nullptr,
                theDataType
            );
            try
            {
                const auto& layerTreeCasted = dynamic_cast<const Wms::CapabilityNode&>(layerTree);
                auto* subMenuList = menuItem->SubMenu();
                if(!subMenuList)
                {
                    subMenuList = new NFmiMenuItemList;
                }

                for(const auto& child : layerTreeCasted.children)
                {
                    AddAllWmsProducersToParamSelectionPopup(theMenuSettings, theDataType, subMenuList, *child);
                }
                menuItem->AddSubMenu(subMenuList);
            }
            catch(const std::exception&)
            {
            }
            theMenuItemList->Add(std::move(menuItem));
        }
        catch(...)
        {
        }
    }
#endif // DISABLE_CPPRESTSDK
}

void AddCustomFolderToMenuItemList(const MenuCreationSettings &theMenuSettings, NFmiMenuItemList *theCustomMenuList, const std::string &theCustomMenuName)
{
	MenuCreationSettings usedMenuSettings(theMenuSettings);
	usedMenuSettings.fMakeCustomMenu = true;
	// pyydet‰‰ kutakin custom-menua varten siihen kuuluvien helpdatainfojen lista
	std::vector<NFmiHelpDataInfo> helpDataList = HelpDataInfoSystem()->GetCustomMenuHelpDataList(theCustomMenuName);
	// jokaisen info-datan avulla etsit‰‰ info-organizerilta siihen sopiva data fileNameFilterin avulla
	for(size_t j=0; j < helpDataList.size(); j++)
	{
		checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > infoList = itsSmartInfoOrganizer->GetInfos(helpDataList[j].UsedFileNameFilter(*HelpDataInfoSystem()));
		for(size_t k=0; k < infoList.size(); k++)
		{
			AddSmartInfoToMenuList(usedMenuSettings, infoList[k], theCustomMenuList, infoList[k]->DataType());
		}
	}
}

void AddCustomFoldersToMenu(const MenuCreationSettings &theMenuSettings, NFmiMenuItemList *theMenuList)
{
	MenuCreationSettings usedMenuSettings(theMenuSettings);
	usedMenuSettings.fMakeCustomMenu = true;
	std::vector<std::string> customMenuList = HelpDataInfoSystem()->GetUniqueCustomMenuList();
	for(size_t i = 0; i<customMenuList.size(); i++)
	{
		std::string customMenuName = customMenuList[i];
        if(customMenuName == g_ObservationMenuName)
            continue; // Observation-menu pit‰‰ skipata t‰‰ll‰, koska se lis‰t‰‰n havaintojen alivalikkoon
        auto customMenuItem = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, customMenuName, NFmiDataIdent(), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kAnyData);
		NFmiMenuItemList *customMenuItemList = new NFmiMenuItemList;

        AddCustomFolderToMenuItemList(usedMenuSettings, customMenuItemList, customMenuName);
		if(customMenuItem && customMenuItemList)
		{
			customMenuItem->AddSubMenu(customMenuItemList);
			if(customMenuItem->SubMenu()->NumberOfMenuItems())
				theMenuList->Add(std::move(customMenuItem));
		}
	}
}

bool CreateParamSelectionPopup(unsigned int theDescTopIndex)
{
	if(theDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
		return CreateCrossSectionViewPopup(itsCurrentViewRowIndex);

	delete itsPopupMenu;
	itsPopupMenu = 0;
	fOpenPopup = false;
	if(itsSmartInfoOrganizer)
	{
		itsPopupMenu = new NFmiMenuItemList;

		MenuCreationSettings menuSettings;
		menuSettings.SetMapViewSettings(theDescTopIndex, kFmiAddView);
		CreateParamSelectionBasePopup(menuSettings, itsPopupMenu, "MapViewParamPopUpAdd");

		menuSettings.SetMapViewSettings(theDescTopIndex, kFmiAddAsOnlyView);
		CreateParamSelectionBasePopup(menuSettings, itsPopupMenu, "MapViewParamPopUpAddAsOnly");

		std::string menuString = ::GetDictionaryString("MapViewParamPopUpremoveAll");
        auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, menuString, NFmiDataIdent(), kFmiRemoveAllViews, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable);
		itsPopupMenu->Add(std::move(menuItem));

        if(theDescTopIndex != CtrlViewUtils::kFmiTimeSerialView)
        {
            menuString = ::GetDictionaryString("Remove all params from all rows");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, NFmiDataIdent(), kFmiRemoveAllParamsFromAllRows, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable));
            itsPopupMenu->Add(std::move(menuItem));
        }

		menuString = "Copy all map row params";
		menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, NFmiDataIdent(), kFmiCopyDrawParamsFromMapViewRow, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable));
		itsPopupMenu->Add(std::move(menuItem));
		if(fCopyPasteDrawParamListUsedYet)
		{
			menuString = "Paste all map row params";
			menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, NFmiDataIdent(), kFmiPasteDrawParamsToMapViewRow, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable));
			itsPopupMenu->Add(std::move(menuItem));
		}

		menuString = "Copy all map rows and params";
		menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, NFmiDataIdent(), kFmiCopyMapViewDescTopParams, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable));
		itsPopupMenu->Add(std::move(menuItem));
		if(fCopyPasteDrawParamListVectorUsedYet)
		{
			menuString = "Paste all map rows and params";
			menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, NFmiDataIdent(), kFmiPasteMapViewDescTopParams, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable));
			itsPopupMenu->Add(std::move(menuItem));
		}

// ********* muuta n‰yttˆrivin kaikkien datojen tuottajat halutuiksi *********
		AddChangeAllProducersToParamSelectionPopup(theDescTopIndex, itsPopupMenu, kFmiChangeAllProducersInMapRow, false);
// ********* muuta n‰yttˆrivin kaikkien datojen tuottajat halutuiksi *********

		// ************ t‰ss‰ muutetaan kaikki rivin datatyypit (fiksataan ongelma mik‰ syntyi kun LAPS muuttui kViewable:ista kAnalyze -tyyppiseksi)
		AddChangeAllDataTypesToParamSelectionPopup(theDescTopIndex, itsPopupMenu, kFmiChangeAllDataTypesInMapRow);

// ********* piilota/n‰yt‰ kaikki  -  havainnot/ennusteet *********
		menuString = ::GetDictionaryString("MapViewParamPopUpToggleShowAll");
		menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, NFmiDataIdent(), kFmiHideAllMapViewObservations, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable));
		NFmiMenuItemList *showHideMenuList = new NFmiMenuItemList;
		menuString = ::GetDictionaryString("MapViewParamPopUpHideAllObs");
		showHideMenuList->Add(std::make_unique<NFmiMenuItem>(theDescTopIndex, menuString, NFmiDataIdent(), kFmiHideAllMapViewObservations, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable));
		menuString = ::GetDictionaryString("MapViewParamPopUpShowAllObs");
		showHideMenuList->Add(std::make_unique<NFmiMenuItem>(theDescTopIndex, menuString, NFmiDataIdent(), kFmiShowAllMapViewObservations, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable));
		menuString = ::GetDictionaryString("MapViewParamPopUpHideAllFor");
		showHideMenuList->Add(std::make_unique<NFmiMenuItem>(theDescTopIndex, menuString, NFmiDataIdent(), kFmiHideAllMapViewForecasts, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable));
		menuString = ::GetDictionaryString("MapViewParamPopUpShowAllFor");
		showHideMenuList->Add(std::make_unique<NFmiMenuItem>(theDescTopIndex, menuString, NFmiDataIdent(), kFmiShowAllMapViewForecasts, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable));
		menuItem->AddSubMenu(showHideMenuList);

		itsPopupMenu->Add(std::move(menuItem));
// ********* piilota/n‰yt‰ kaikki  -  havainnot/ennusteet *********

		if(!itsPopupMenu->InitializeCommandIDs(itsPopupMenuStartId))
			return false;

		fOpenPopup = true;
		return true;
	}
	return false;
}

void AddMacroParamPartToPopUpMenu(const MenuCreationSettings &theMenuSettings, NFmiMenuItemList *theMenuList, NFmiInfoData::Type theDataType)
{
	string menuString = ::GetDictionaryString("MapViewParamPopUpMacroParams");
	auto menuItem = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kMacroParam);
	NFmiMenuItemList *macroParamsMenuList = new NFmiMenuItemList;

	std::vector<NFmiMacroParamItem> &macroParamItemList = itsMacroParamSystem.MacroParamItemTree();
	AddMacroParamPartToPopUpMenu(theMenuSettings, macroParamsMenuList, macroParamItemList, theDataType);
	menuItem->AddSubMenu(macroParamsMenuList);
	theMenuList->Add(std::move(menuItem));
}

void AddChangeAllProducersToParamSelectionPopup(unsigned int theDescTopIndex, NFmiMenuItemList *theMenuList, FmiMenuCommandType theMenuCommandType, bool crossSectionPopup)
{
		std::vector<NFmiProducerInfo> &prodVec = ProducerSystem().Producers();
		if(prodVec.size() > 0)
		{
            auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, "Change all producers", NFmiDataIdent(), theMenuCommandType, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable);
			NFmiMenuItemList *changeProducersMenuList = new NFmiMenuItemList;
			for(size_t i = 0; i < prodVec.size(); i++)
			{
				NFmiDataIdent dataIdent(NFmiParam(), prodVec[i].GetProducer()); // laitetaan feikki parametri ja vain 1. tuottaja listasta
				changeProducersMenuList->Add(std::make_unique<NFmiMenuItem>(theDescTopIndex, prodVec[i].Name(), dataIdent, theMenuCommandType, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable));
			}
			// lis‰t‰‰n viel‰ editoitu data ja virallinen data tuottajat listaan, jos ei kyse poikkileikkausn‰ytˆst‰
			if(crossSectionPopup == false)
			{
				boost::shared_ptr<NFmiFastQueryInfo> editedInfo = EditedInfo();
				if(editedInfo)
				{
					editedInfo->FirstParam();
					changeProducersMenuList->Add(std::make_unique<NFmiMenuItem>(theDescTopIndex, "Edited data", editedInfo->Param(), theMenuCommandType, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable));
				}
				boost::shared_ptr<NFmiFastQueryInfo> operativeInfo = itsSmartInfoOrganizer->FindInfo(NFmiInfoData::kKepaData);
				if(operativeInfo) // operatiivinen data
				{
					operativeInfo->FirstParam();
					changeProducersMenuList->Add(std::make_unique<NFmiMenuItem>(theDescTopIndex, "Operative data", operativeInfo->Param(), theMenuCommandType, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable));
				}
				boost::shared_ptr<NFmiFastQueryInfo> helpDataInfo = itsSmartInfoOrganizer->FindInfo(NFmiInfoData::kEditingHelpData);
				if(helpDataInfo) // editointi apu data
				{
					helpDataInfo->FirstParam();
					changeProducersMenuList->Add(std::make_unique<NFmiMenuItem>(theDescTopIndex, "Help edit data", helpDataInfo->Param(), theMenuCommandType, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable));
				}
			}
			menuItem->AddSubMenu(changeProducersMenuList);
			theMenuList->Add(std::move(menuItem));
		}
}

void AddChangeAllDataTypesToParamSelectionPopup(unsigned int theDescTopIndex, NFmiMenuItemList *theMenuList, FmiMenuCommandType theMenuCommandType)
{
	std::vector<std::pair<std::string, NFmiInfoData::Type> > dataTypeVec;
	dataTypeVec.push_back(std::make_pair("Edited-Data", NFmiInfoData::kEditable));
	dataTypeVec.push_back(std::make_pair("Model-Data", NFmiInfoData::kViewable));
	dataTypeVec.push_back(std::make_pair("Observations", NFmiInfoData::kObservations));
	dataTypeVec.push_back(std::make_pair("Analyze-Data", NFmiInfoData::kAnalyzeData));
	dataTypeVec.push_back(std::make_pair("Satel-Data", NFmiInfoData::kSatelData));
	dataTypeVec.push_back(std::make_pair("Hybrid-Data", NFmiInfoData::kHybridData));
	dataTypeVec.push_back(std::make_pair("Model-Help-Data", NFmiInfoData::kModelHelpData));
	dataTypeVec.push_back(std::make_pair("Editing-Help-Data", NFmiInfoData::kEditingHelpData));
	dataTypeVec.push_back(std::make_pair("Station-Radar-Data", NFmiInfoData::kSingleStationRadarData));

	if(dataTypeVec.size() > 0)
	{
        auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, "Change data-types (fix view-macros)", NFmiDataIdent(), theMenuCommandType, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable);
		NFmiMenuItemList *menuList = new NFmiMenuItemList;
		for(size_t i = 0; i < dataTypeVec.size(); i++)
		{
			menuList->Add(std::make_unique<NFmiMenuItem>(theDescTopIndex, dataTypeVec[i].first, static_cast<FmiParameterName>(dataTypeVec[i].second), theMenuCommandType, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable));
		}

		menuItem->AddSubMenu(menuList);
		theMenuList->Add(std::move(menuItem));
	}
}

void AddMacroParamPartToPopUpMenu(const MenuCreationSettings &theMenuSettings, NFmiMenuItemList *theMenuList, const std::vector<NFmiMacroParamItem> &theMacroParamItemList, NFmiInfoData::Type theDataType)
{
	int mpSize = static_cast<int>(theMacroParamItemList.size());
	for(int i=0 ; i<mpSize; i++)
	{
		const NFmiMacroParamItem &macroParamItem = theMacroParamItemList[i];
		if(macroParamItem.itsMacroParam->IsMacroParamDirectory())
		{
            auto macroParamsItem = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, macroParamItem.itsMacroParam->Name().c_str(), NFmiDataIdent(NFmiParam(998, "makro")), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, theDataType);
			NFmiMenuItemList *macroParamsMenuList = new NFmiMenuItemList;
			AddMacroParamPartToPopUpMenu(theMenuSettings, macroParamsMenuList, macroParamItem.itsDirectoryItems, theDataType);
			macroParamsItem->AddSubMenu(macroParamsMenuList);
			theMenuList->Add(std::move(macroParamsItem));
		}
		else
		{
			string macroParamName = macroParamItem.itsMacroParam->Name();
			if(macroParamItem.itsMacroParam->ErrorInMacro())
				macroParamName += " (ERROR)";
            auto macroParamsItem = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, macroParamName.c_str(), NFmiDataIdent(NFmiParam(998, "makro")), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, theDataType);
			// HUOM! TƒSSƒ pit‰‰ k‰ytt‰‰ drawParamin initfile-namea, koska muuten ei lˆydy macroparamia kuin juuri tasolta
			macroParamsItem->MacroParamInitName(macroParamItem.itsMacroParam->DrawParam()->InitFileName());
			theMenuList->Add(std::move(macroParamsItem));
		}
	}
}

// t‰ss‰ lis‰t‰‰n popup apudata valikko
// mm. mesan, tuliset,verifikaatiot, fuzzy, geo-data, macroparamit jne.
void AddHelpDataToParamSelectionPopup(const MenuCreationSettings &theMenuSettings, NFmiMenuItem *theMenuItem)
{
	NFmiMenuItemList *menuList = new NFmiMenuItemList;
	AddProducerDataToParamSelectionPopup(theMenuSettings, menuList, 160); // 160 = mesan 2
    AddProducerDataToParamSelectionPopup(theMenuSettings, menuList, 189); // 189 = MetNo Analysis
	AddProducerDataToParamSelectionPopup(theMenuSettings, menuList, kFmiRADARNRD, NFmiInfoData::kObservations); // tuliset dataa, mutta ei 'tutka'-dataa eli ignooraa kObservations-data tyyppi joka normaali tutkalla on
	AddProducerDataToParamSelectionPopup(theMenuSettings, menuList, 108); // 108 = lumikuorma-datan tuottaja numero
    AddProducerDataToParamSelectionPopup(theMenuSettings, menuList, 101); // 101 = kriging-datan tuottaja numero
	AddFirstOfDataTypeToParamSelectionPopup(theMenuSettings, menuList, NFmiInfoData::kStationary, ""); // kun annetaan tyhj‰ dictionary -stringi, k‰ytet‰‰n tuottaja nimea menu otsikossa

	if(theMenuSettings.fDoMapMenu && ConceptualModelData().Use()) // jos k‰siteanalyysi systeemi k‰ytˆss‰, lis‰t‰‰n sen lis‰‰mismahdollisuus popup-valikkoon
	{
		NFmiProducer prod(1028); // t‰ll‰ ei ole viel‰ virallista tuottaja id:t‰
		std::string menuString = "Conceptual analysis";
        auto menuItem1 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(NFmiParam(kFmiLastParameter, ConceptualModelData().DefaultUserName()), prod), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kConceptualModelData);
		menuList->Add(std::move(menuItem1));
	}

    if(capDataSystem.useCapData())
    {
        NFmiProducer prod(NFmiSettings::Optional<int>("SmartMet::Warnings::ProducerId", 12345)); // No official producerId, reads this from Cap.conf. If multiple ids, read them all here.
        std::string menuString = "Warnings (CAP)";
        auto menuItem1 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(NFmiParam(kFmiLastParameter, "cap-data"), prod), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kCapData);
        menuList->Add(std::move(menuItem1));
    }

	// T‰ss‰ tehd‰‰n sekaisin kaikkia mahdollisia datoja yhteen popup-osioon.
	// Toivottavasti auttaa erilaisten datojen selaamisessa mm. drag'n drop tilanteissa.
	// Mix popup-osiota ei tehd‰ operatiivisessa moodissa, koska se hidastaa k‰yttˆ‰
	if(EditorModeDataWCTR()->EditorMode() == NFmiMetEditorModeDataWCTR::kNormal)
	{
        auto mixMenuItem = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, "Mix", NFmiDataIdent(), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kViewable);
		bool mixSuccess = AddMixDataToParamSelectionPopup(theMenuSettings, mixMenuItem.get());
		if(mixSuccess)
			menuList->Add(std::move(mixMenuItem));
	}
	theMenuItem->AddSubMenu(menuList);
}

bool IsProducerWanted(int theCurrentProdId, int theProducerId1, int theProducerId2 = -1, int theProducerId3 = -1, int theProducerId4 = -1)
{
	if(theCurrentProdId == theProducerId1)
		return true;
	else if(theProducerId2 != -1 && theCurrentProdId == theProducerId2)
		return true;
	else if(theProducerId3 != -1 && theCurrentProdId == theProducerId3)
		return true;
	else if(theProducerId4 != -1 && theCurrentProdId == theProducerId4)
		return true;
	return false;
}

bool WantedSynopPlotObsFound(void)
{
	bool status = false;

	status |= (itsSmartInfoOrganizer->FindInfo(NFmiInfoData::kObservations, NFmiProducer(kFmiSYNOP), true) != 0);

	return status;
}

bool WantedMetarPlotObsFound(void)
{
	bool status = false;

	status |= (itsSmartInfoOrganizer->FindInfo(NFmiInfoData::kObservations, NFmiProducer(kFmiMETAR), true) != 0);

	return status;
}

// Lis‰t‰‰ ali-listaan kaikki t‰m‰n tyyppiset datat
void AddToListAllThisDataTypes(const MenuCreationSettings &theMenuSettings, NFmiMenuItemList *theMenuList, NFmiInfoData::Type theDataType)
{
	checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > infos(itsSmartInfoOrganizer->GetInfos(theDataType));
	int size = static_cast<int>(infos.size());
	if(size > 0)
	{
		NFmiMenuItemList *subMenuList = new NFmiMenuItemList; // luodaan ali menu
		for(int i=0; i<size; i++)
			AddSmartInfoToMenuList(theMenuSettings, infos[i], subMenuList, theDataType);

        auto subMenuItem = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, "Radar data",
            NFmiDataIdent(), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, 
            nullptr, NFmiInfoData::kSingleStationRadarData);

		subMenuItem->AddSubMenu(subMenuList);
		theMenuList->Add(std::move(subMenuItem));
	}
}

void AddSatelliteImagesToMenu(const MenuCreationSettings &theMenuSettings, NFmiMenuItemList *theMenuList)
{
	bool anySatelProducersFound = false;
	std::string menuString = ::GetDictionaryString("MapViewParamPopUpSatelliteData");
    auto menuItem2 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(),
        theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kSatelData);

	NFmiMenuItemList *satelProducerMenuList = new NFmiMenuItemList;
	std::vector<NFmiProducerInfo> &satelProducers = itsSatelImageProducerSystem.Producers();
	for(size_t j=0; j<satelProducers.size(); j++)
	{
		bool anyChannelsFound = false;
        auto menuItemProducer = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, satelProducers[j].Name(), NFmiDataIdent(),
            theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kSatelData);
		NFmiMenuItemList *satelChannelMenuList = new NFmiMenuItemList;
		int helpDataSize = HelpDataInfoSystem()->DynamicCount();
		for(int i=0; i<helpDataSize; i++)
		{ // etsit‰‰n currentin tuottajan kuva kanavat/parametrit
			NFmiHelpDataInfo &helpDataInfo = HelpDataInfoSystem()->DynamicHelpDataInfo(i);
			if(helpDataInfo.IsEnabled() && helpDataInfo.DataType() == NFmiInfoData::kSatelData)
			{
				int prodId = static_cast<int>(satelProducers[j].ProducerId());
				int helpDataProdId = helpDataInfo.FakeProducerId();
				if(prodId > 0 && prodId == helpDataProdId)
				{
                    auto satelItem = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, std::string(helpDataInfo.ImageDataIdent().GetParamName()),
                        helpDataInfo.ImageDataIdent(), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kSatelData);
					satelChannelMenuList->Add(std::move(satelItem));
					anyChannelsFound = true;
				}
			}
		}
		if(anyChannelsFound)
		{
			menuItemProducer->AddSubMenu(satelChannelMenuList);
			satelProducerMenuList->Add(std::move(menuItemProducer));
			anySatelProducersFound = true;
		}
		else
		{ 
            // jos ei lˆytynyt, t‰m‰ pit‰‰ tuhota 'k‰sin'
			delete satelChannelMenuList;
		}
	}
	if(anySatelProducersFound)
	{
		menuItem2->AddSubMenu(satelProducerMenuList);
		theMenuList->Add(std::move(menuItem2));
	}
	else
	{ 
        // jos ei lˆytynyt, t‰m‰ pit‰‰ tuhota 'k‰sin'
		delete satelProducerMenuList;
	}
}

// t‰ss‰ lis‰t‰‰n popup havainto valikko
// mm. synop, tutka, salamat, satel jne.
void AddObservationDataToParamSelectionPopup(const MenuCreationSettings &theMenuSettings, NFmiMenuItem *theMenuItem)
{
	NFmiMenuItemList *obsMenuList = new NFmiMenuItemList;
	AddProducerDataToParamSelectionPopup(theMenuSettings, obsMenuList, kFmiSYNOP, NFmiInfoData::kNoDataType, NFmiInfoData::kNoDataType, NFmiInfoData::kNoDataType, true); // 1001 = synop tuottaja
	AddProducerDataToParamSelectionPopup(theMenuSettings, obsMenuList, kFmiRADARNRD, NFmiInfoData::kViewable); // tutka dataa, mutta ei tuliset-dataa eli ignooraa kViewable-data tyyppi joka tulisetilla on
	AddProducerDataToParamSelectionPopup(theMenuSettings, obsMenuList, kFmiMETAR);
	AddProducerDataToParamSelectionPopup(theMenuSettings, obsMenuList, 10002); // sadeasema dataa
	AddProducerDataToParamSelectionPopup(theMenuSettings, obsMenuList, kFmiRoadObs); // kFmiRoadObs = ties‰‰asema-datan tuottaja numero
	AddProducerDataToParamSelectionPopup(theMenuSettings, obsMenuList, kFmiTestBed);
	AddToListAllThisDataTypes(theMenuSettings, obsMenuList, NFmiInfoData::kSingleStationRadarData);
    AddCustomFolderToMenuItemList(theMenuSettings, obsMenuList, g_ObservationMenuName);

// ********** lis‰t‰‰n salama osa *************************
	if(theMenuSettings.fDoMapMenu) // n‰m‰ osiot menev‰t vain karttan‰ytt‰ menuun
	{
		NFmiInfoData::Type flashType = NFmiInfoData::kFlashData;
		boost::shared_ptr<NFmiFastQueryInfo> editedInfo = EditedInfo();
		if(editedInfo && editedInfo->Producer()->GetIdent() == kFmiFlashObs)
			flashType = NFmiInfoData::kEditable; // jos salama data editoitavana, pit‰‰ tyypiksi laittaa editable
		if(flashType == NFmiInfoData::kEditable || itsSmartInfoOrganizer->FindInfo(NFmiInfoData::kFlashData, 0))
		{
			NFmiProducer flashProducer(flashType == NFmiInfoData::kEditable ? *(editedInfo->Producer()) : *(itsSmartInfoOrganizer->FindInfo(NFmiInfoData::kFlashData, 0)->Producer()));
			std::string menuString = ::GetDictionaryString("MapViewParamPopUpFlashData");
            auto menuItem1 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(NFmiParam(kFmiFlashStrength, "salama"), flashProducer), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, flashType);
			obsMenuList->Add(std::move(menuItem1));
		}
// ********** lis‰t‰‰n satelliitti osa dynaamisesti *************************
		AddSatelliteImagesToMenu(theMenuSettings, obsMenuList);

// ********** lis‰t‰‰n synop-plot data osa *************************
		// kFmiSpSynoPlot on parid synopplotille
		NFmiInfoData::Type synopType = NFmiInfoData::kObservations;
		if(editedInfo)
		{
			if(IsProducerWanted(editedInfo->Producer()->GetIdent(), kFmiSYNOP, kFmiTestBed, kFmiSHIP, kFmiBUOY))
				synopType = NFmiInfoData::kEditable; // jos tietynlaista asema data editoitavana, pit‰‰ tyypiksi laittaa editable
		}
		bool wantedPlotObsFound = WantedSynopPlotObsFound();
		if(synopType == NFmiInfoData::kEditable || wantedPlotObsFound)
		{ // lis‰t‰‰n t‰m‰ siis vain jos editoitavassa datassa on synop-dataa, tai lˆytyy yleens‰ synop dataa
			std::string menuString = ::GetDictionaryString("MapViewParamPopUpSynopPlot");
            auto menuItem3 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(NFmiParam(NFmiInfoData::kFmiSpSynoPlot, "synop")), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, synopType);
			obsMenuList->Add(std::move(menuItem3));

			// lis‰t‰‰n myˆs min/max plot
			std::string menuStringMinMax = "Min/Max";
            auto menuItem4 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuStringMinMax, NFmiDataIdent(NFmiParam(NFmiInfoData::kFmiSpMinMaxPlot, "min/max")), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, synopType);
			obsMenuList->Add(std::move(menuItem4));
		}
// ********** lis‰t‰‰n synop-plot data osa *************************

// ********** lis‰t‰‰n metar-plot data osa *************************
		// kFmiSpMetarPlot on parid synopplotille
		NFmiInfoData::Type metarType = NFmiInfoData::kObservations;
		if(editedInfo)
		{
			if(IsProducerWanted(editedInfo->Producer()->GetIdent(), kFmiMETAR))
				metarType = NFmiInfoData::kEditable; // jos tietynlaista asema data editoitavana, pit‰‰ tyypiksi laittaa editable
		}
		bool wantedMetarPlotObsFound = WantedMetarPlotObsFound();
		if(metarType == NFmiInfoData::kEditable || wantedMetarPlotObsFound)
		{ // lis‰t‰‰n t‰m‰ siis vain jos editoitavassa datassa on metar-dataa, tai lˆytyy yleens‰ metar dataa
			std::string menuString = ::GetDictionaryString("Metar-plot");
            auto menuItem3 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(NFmiParam(NFmiInfoData::kFmiSpMetarPlot, "metar")), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, metarType);
			obsMenuList->Add(std::move(menuItem3));
		}
// ********** lis‰t‰‰n metar-plot data osa *************************

// ********** lis‰t‰‰n luotaus synop-plot data osa *************************
        boost::shared_ptr<NFmiFastQueryInfo> soundingInfo = InfoOrganizer()->GetPrioritizedSoundingInfo(NFmiInfoOrganizer::ParamCheckFlags(true));
		if(soundingInfo)
		{
			NFmiInfoData::Type soundingType = soundingInfo->DataType();
			NFmiLevel defaultLevel(50, "850xxx", 850);
			std::string menuString = "Sounding-Plot";
            auto menuItem4 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(NFmiParam(NFmiInfoData::kFmiSpSoundingPlot, "temp")), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, &defaultLevel, soundingType);
			obsMenuList->Add(std::move(menuItem4));

			menuString = "Sounding";
            auto menuItem5 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, &defaultLevel, soundingType);
			NFmiMenuItemList *soundingMenuList = new NFmiMenuItemList(theMenuSettings.itsDescTopIndex, const_cast<NFmiParamBag*>(&(soundingInfo->ParamBag())), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, const_cast<NFmiLevelBag*>(itsSoundingPlotLevels.Levels()), soundingType);
			menuItem5->AddSubMenu(soundingMenuList);
			obsMenuList->Add(std::move(menuItem5));
		}
// ********** lis‰t‰‰n luotaus synop-plot data osa *************************
	} // addOnlyToMapView

	theMenuItem->AddSubMenu(obsMenuList);
}

void AddProducerDataToParamSelectionPopup(const MenuCreationSettings &theMenuSettings, NFmiMenuItemList *theMenuList, 
										  int theProducerId,
										  NFmiInfoData::Type theIgnoreDataType1 = NFmiInfoData::kNoDataType,
										  NFmiInfoData::Type theIgnoreDataType2 = NFmiInfoData::kNoDataType,
										  NFmiInfoData::Type theIgnoreDataType3 = NFmiInfoData::kNoDataType,
										  bool fTakeOnlyFirst = false)
{
	checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > infos(itsSmartInfoOrganizer->GetInfos(theProducerId));
	int size = static_cast<int>(infos.size());
	if(size > 0)
	{
		for(int i=0; i<size; i++)
		{
			if(infos[i]->DataType() == theIgnoreDataType1 || infos[i]->DataType() == theIgnoreDataType2 || infos[i]->DataType() == theIgnoreDataType3)
				continue; // tiettyj‰ data tyyppeja ei haluttukkaan listaan
			NFmiParamBag paramBag(infos[i]->ParamBag());
			if(fTakeOnlyFirst)
				paramBag = itsSmartInfoOrganizer->GetParams(theProducerId);

			AddSmartInfoToMenuList(theMenuSettings, infos[i], theMenuList, infos[i]->DataType(), "", &paramBag);
			if(fTakeOnlyFirst)
				break;
		}
	}
}

void AddConfiguredModelProducerDataToParamSelectionPopup(const MenuCreationSettings &theMenuSettings
									,NFmiMenuItemList *theMenuList, int theModelIndex
									,NFmiInfoData::Type usedDataType1 = NFmiInfoData::kViewable
									,NFmiInfoData::Type usedDataType2 = NFmiInfoData::kNoDataType
									,NFmiInfoData::Type usedDataType3 = NFmiInfoData::kNoDataType
									,NFmiInfoData::Type usedDataType4 = NFmiInfoData::kNoDataType
									,NFmiInfoData::Type usedDataType5 = NFmiInfoData::kNoDataType
									,NFmiInfoData::Type usedDataType6 = NFmiInfoData::kNoDataType)
{
	if(ProducerSystem().ExistProducer(theModelIndex))
	{
		bool doCrossSectionMenu = theMenuSettings.fLevelDataOnly;
		if(doCrossSectionMenu == false || (doCrossSectionMenu == true && ProducerSystem().Producer(theModelIndex).HasRealVerticalData()))
		{
			NFmiProducerInfo &prodInfo = ProducerSystem().Producer(theModelIndex);
			string menuString(prodInfo.Name());

			checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > infos(itsSmartInfoOrganizer->GetInfos(prodInfo.ProducerId()));
			int size = static_cast<int>(infos.size());
			int foundSize = 0;
			if(size > 0)
			{
				NFmiMenuItemList *producerMenuItemList = new NFmiMenuItemList;
				for(int i=0; i<size; i++)
				{
					if(theMenuSettings.fLevelDataOnly && infos[i]->SizeLevels() <= 1)
						continue;
					NFmiInfoData::Type dataType = infos[i]->DataType();
					if(dataType == usedDataType1 || dataType == usedDataType2 || dataType == usedDataType3 || dataType == usedDataType4 || dataType == usedDataType5 || dataType == usedDataType6)
					{
						AddSmartInfoToMenuList(theMenuSettings, infos[i], producerMenuItemList, dataType, "", 0);
						foundSize++;
					}
				}

				if(producerMenuItemList && producerMenuItemList->NumberOfMenuItems() > 0)
				{
                    auto producerMenuItem = std::make_unique<NFmiMenuItem>(prodInfo.Name().c_str(), kFmiBadParameter);
					producerMenuItem->AddSubMenu(producerMenuItemList);
					theMenuList->Add(std::move(producerMenuItem));
				}
				else
					delete producerMenuItemList;
			}
		}
	}
}

// TODO
// Ett‰ t‰m‰n mixdata jutun saa toimimaan kunnolla, pit‰‰ laittaa MenuItem:iin mukaan
// myˆs tiedoston nimi ett‰ kyseisen datan datan (tiedoston) parametrien valinta onnistuisi
// tarvittaessa. Nyt homma menee sekaisin kun jos tiputetaan esim. pari saman tuottajan saman
// tyyppist‰ dataa editoriin ja t‰llˆin vain 1. datoista oikeasti voidaan valita n‰ytˆlle, koska
// tuottaja/datatyyppi/leveltype on kaikilla sama.
bool AddMixDataToParamSelectionPopup(const MenuCreationSettings &theMenuSettings, NFmiMenuItem *theMenuItem)
{
	vector<NFmiInfoData::Type> dataTypes;
	dataTypes.push_back(NFmiInfoData::kViewable);
	dataTypes.push_back(NFmiInfoData::kHybridData);
	dataTypes.push_back(NFmiInfoData::kObservations);
	NFmiMenuItemList *producerMenuList = 0;

	bool foundAnyData = false;
	for(size_t typeCounter = 0; typeCounter<dataTypes.size(); typeCounter++)
	{
		checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > infos(itsSmartInfoOrganizer->GetInfos(dataTypes[typeCounter]));
		int ssize = static_cast<int>(infos.size());
		if(ssize > 0)
		{
			if(producerMenuList == 0)
				producerMenuList = new NFmiMenuItemList;
			for(int i=0; i<ssize; i++)
			{
				foundAnyData = true;
				AddSmartInfoToMenuList(theMenuSettings, infos[i], producerMenuList, dataTypes[typeCounter], ""); // "" -> k‰ytet‰‰n menu itemiss‰ tuottajan nime‰
			}
		}
	}
	if(producerMenuList)
		theMenuItem->AddSubMenu(producerMenuList);

	return foundAnyData;
}

std::unique_ptr<NFmiDataIdent> MakePossibleStreamLineDataIdent(const MenuCreationSettings &theMenuSettings, boost::shared_ptr<NFmiFastQueryInfo> &theSmartInfo)
{
    static const NFmiParam streamlineBaseParam(NFmiInfoData::kFmiSpStreamline, "Streamline");
    bool addStreamlineParam = theMenuSettings.fDoMapMenu && theSmartInfo->IsGrid();
    bool hasWindParams = theSmartInfo->Param(kFmiTotalWindMS) || theSmartInfo->Param(kFmiWindSpeedMS) || theSmartInfo->Param(kFmiWindUMS);
    if(addStreamlineParam && hasWindParams)
        return std::make_unique<NFmiDataIdent>(streamlineBaseParam, *theSmartInfo->Producer());
    else
        return nullptr;
}

std::unique_ptr<NFmiDataIdent> MakePossibleWindVectorDataIdent(const MenuCreationSettings &theMenuSettings, boost::shared_ptr<NFmiFastQueryInfo> &theSmartInfo)
{
    static const NFmiParam windVectorBaseParam(kFmiWindVectorMS, "Wind vector (meta)");
    auto metaWindVectorParamUsage = NFmiFastInfoUtils::CheckMetaWindVectorParamUsage(theSmartInfo);
    bool addWindVectorParam = metaWindVectorParamUsage.fUseMetaWindVectorParam && (theMenuSettings.fDoMapMenu || theMenuSettings.fLevelDataOnly);
    if(addWindVectorParam)
        return std::make_unique<NFmiDataIdent>(windVectorBaseParam, *theSmartInfo->Producer());
    else
        return nullptr;
}

void AddSmartInfoToMenuItem(const MenuCreationSettings &theMenuSettings, boost::shared_ptr<NFmiFastQueryInfo> &theSmartInfo, NFmiMenuItem *theMenuItem, NFmiInfoData::Type theDataType, NFmiParamBag *theWantedParamBag = 0)
{
	if(theSmartInfo == 0 && theMenuItem == 0)
		return ;
	else if(theSmartInfo && theMenuItem)
	{
		// tarkistetaan, lˆytyykˆ kyseiselle datalle custom-valikkoa
		NFmiHelpDataInfo *helpDataInfo = HelpDataInfoSystem()->FindHelpDataInfo(dynamic_cast<NFmiOwnerInfo*>(theSmartInfo.get())->DataFilePattern());
		if(helpDataInfo && helpDataInfo->CustomMenuFolder().empty() == false && theMenuSettings.fMakeCustomMenu == false)
			return ; // t‰t‰ ei laiteta menuun, koska se menee erikseen rakennettavaan custom-menuun

		NFmiParamBag paramBag(theSmartInfo->ParamBag());
		if(theWantedParamBag)
			paramBag = *theWantedParamBag;
		NFmiLevelBag* levels = 0;
		if(theSmartInfo->SizeLevels() > 1)
			levels = const_cast<NFmiLevelBag*>(theSmartInfo->VPlaceDescriptor().Levels());
        auto possibleStreamLineDataIdentPtr = MakePossibleStreamLineDataIdent(theMenuSettings, theSmartInfo);
        auto possibleWindVectorDataIdentPtr = MakePossibleWindVectorDataIdent(theMenuSettings, theSmartInfo);
		NFmiMenuItemList *menuList = 0;
		bool doCrossSectionMenu = theMenuSettings.fLevelDataOnly && theMenuSettings.fGridDataOnly;
		if(doCrossSectionMenu == false && levels) // poikkileikkaus menun yhteydess‰ ei halutakaan laittaa level tietoja menu-popupiin, vain parametrit
            menuList = new NFmiMenuItemList(theMenuSettings.itsDescTopIndex, &paramBag, theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, levels, theDataType, kFmiLastParameter, possibleStreamLineDataIdentPtr.get(), possibleWindVectorDataIdentPtr.get());
		else
			menuList = new NFmiMenuItemList(theMenuSettings.itsDescTopIndex, &paramBag, theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, theDataType, possibleStreamLineDataIdentPtr.get(), possibleWindVectorDataIdentPtr.get());

		if(menuList)
			theMenuItem->AddSubMenu(menuList);
	}
	else
		throw std::runtime_error("Error when making param selection popup menu in function AddSmartInfoToMenuItem. One was zero pointer: menuIten or smartInfo.");
}

// Lis‰t‰‰n annetun menuListan per‰‰n menuItem, joka on muodostettu annetun smartInfon avulla.
// Jos theDictionaryStr on tyhj‰, annetaan menuItemi otsikoksi smartInfon tuottaja nimi, muuten 
// pyydet‰‰n sanakirjasta k‰ytetty otsikko.
void AddSmartInfoToMenuList(const MenuCreationSettings &theMenuSettings, boost::shared_ptr<NFmiFastQueryInfo> &theSmartInfo, NFmiMenuItemList *theMenuItemList, NFmiInfoData::Type theDataType, const std::string &theDictionaryStr = "", NFmiParamBag *theWantedParamBag = 0)
{
	if(theSmartInfo == 0 && theMenuItemList == 0)
		return ;
	else if(theSmartInfo && theMenuItemList)
	{
		if(theMenuSettings.fGridDataOnly && theSmartInfo->IsGrid() == false)
			return ;

		// jos kyseess‰ olisi poikkileikkaus menu rakentelua, ei ole syyt‰ p‰‰st‰‰ l‰pi dataa, miss‰ on vain yksi leveli ja se ei ole hila dataa
		if(theMenuSettings.fLevelDataOnly == false || (theMenuSettings.fLevelDataOnly == true && theSmartInfo->SizeLevels() > 1))
		{
			std::string menuTitleStr = theDictionaryStr.empty() ? theSmartInfo->FirstParamProducer().GetName() : ::GetDictionaryString(theDictionaryStr.c_str());
            auto menuItem = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuTitleStr, NFmiDataIdent(), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, theDataType);
			AddSmartInfoToMenuItem(theMenuSettings, theSmartInfo, menuItem.get(), theDataType, theWantedParamBag);
			if(menuItem)
			{
				if(menuItem->SubMenu() && menuItem->SubMenu()->NumberOfMenuItems() > 0)
					theMenuItemList->Add(std::move(menuItem));
			}
		}
	}
	else
		throw std::runtime_error("Error when making param selection popup menu in function AddSmartInfoToMenuList. One was zero pointer: menuItenList or smartInfo.");
}

void AddFirstOfDataTypeToParamSelectionPopup(const MenuCreationSettings &theMenuSettings, NFmiMenuItemList *theMenuItemList, NFmiInfoData::Type theDataType, const std::string &theDictionaryStr)
{
	boost::shared_ptr<NFmiFastQueryInfo> info = itsSmartInfoOrganizer->FindInfo(theDataType);
	if(info)
	{
		AddSmartInfoToMenuList(theMenuSettings, info, theMenuItemList, theDataType, theDictionaryStr);
	}
}

void AddCalculatedParamsToMenu(const MenuCreationSettings &theMenuSettings, NFmiMenuItemList *theMenuItemList, const std::string &theMenuItemDictionaryStr)
{
	std::string menuString = ::GetDictionaryString(theMenuItemDictionaryStr.c_str());
    auto menuItem4 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(), theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kCalculatedValue);
	NFmiMenuItemList *menuList4 = new NFmiMenuItemList;
	menuString = ::GetDictionaryString("MapViewMaskSelectionPopUpLatitude");
	menuList4->Add(std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, kFmiLatitude, theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kCalculatedValue));
	menuString = ::GetDictionaryString("MapViewMaskSelectionPopUpLongitude");
	menuList4->Add(std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, kFmiLongitude, theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kCalculatedValue));
	menuString = ::GetDictionaryString("MapViewMaskSelectionPopUpElevationAngle");
	menuList4->Add(std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, kFmiElevationAngle, theMenuSettings.itsMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kCalculatedValue));
	menuItem4->AddSubMenu(menuList4);
	theMenuItemList->Add(std::move(menuItem4));
}

bool CreateMaskSelectionPopup(void)
{
	delete itsPopupMenu;
	itsPopupMenu = 0;
	fOpenPopup = false;
	if(itsSmartInfoOrganizer)
	{
		itsPopupMenu = new NFmiMenuItemList;

		MenuCreationSettings menuSettings;
		menuSettings.SetMaskSettings(kFmiAddMask);
		CreateParamSelectionBasePopup(menuSettings, itsPopupMenu, "MapViewMaskSelectionPopUpAdd");

		menuSettings.SetMaskSettings(kFmiAddAsOnlyMask);
		CreateParamSelectionBasePopup(menuSettings, itsPopupMenu, "MapViewMaskSelectionPopUpAddAsOnly");

		std::string menuString = ::GetDictionaryString("MapViewMaskSelectionPopUpRemoveAll");
        auto menuItem = std::make_unique<NFmiMenuItem>(-1, menuString, NFmiDataIdent(), kFmiRemoveAllMasks, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable);
		itsPopupMenu->Add(std::move(menuItem));

		if(!itsPopupMenu->InitializeCommandIDs(itsPopupMenuStartId))
			return false;
		fOpenPopup = true;
		return true;
	}
	return false;
}

void AddMultiModelRunToMenu(boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiMenuItemList &thePopupMenu, unsigned int theDescTopIndex, int index)
{
	if(theDrawParam)
	{
		NFmiDataIdent param = theDrawParam->Param();
		std::string menuString = "SetMultiRunMode"; // ::GetDictionaryString("TimeSerialViewParamPopUpRemove");
        auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, menuString, param, kFmiTimeSerialModelRunCountSet, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, theDrawParam->DataType(), index);

		NFmiMenuItemList *menuList = new NFmiMenuItemList;
		for(int i = 0; i <= 10; i++)
		{
			std::string multiModeStr;
			if(i == 0)
				multiModeStr += "Set normal (0)";
			else
				multiModeStr += std::string("Count to ") + NFmiStringTools::Convert(i);

            auto item1 = std::make_unique<NFmiMenuItem>(theDescTopIndex, multiModeStr, param, kFmiTimeSerialModelRunCountSet, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, theDrawParam->DataType(), index);
			item1->ExtraParam(i);
			menuList->Add(std::move(item1));
		}
		menuItem->AddSubMenu(menuList);

		thePopupMenu.Add(std::move(menuItem));
	}
}


// T‰t‰ popupia k‰ytet‰‰n kun on klikattu olemassa olevaa n‰yttˆ‰.
bool CreateTimeSerialDialogOnViewPopup(int index)
{
	itsCurrentViewRowIndex = gActiveViewRowIndexForTimeSerialView;
	itsTimeSerialViewIndex = index;
	delete itsPopupMenu;
	itsPopupMenu = 0;
	fOpenPopup = false;
	if(itsTimeSerialViewDrawParamList)
	{
		itsPopupMenu = new NFmiMenuItemList;
		if(itsTimeSerialViewDrawParamList->Index(index))
			AddMultiModelRunToMenu(itsTimeSerialViewDrawParamList->Current(), *itsPopupMenu, CtrlViewUtils::kFmiTimeSerialView, index);

		if(!itsPopupMenu->InitializeCommandIDs(itsPopupMenuStartId))
			return false;
		fOpenPopup = true;
		return true;
	}
	return false;
}

void AddShowHelperDataOntimeSerialViewPopup(const NFmiDataIdent &param, NFmiInfoData::Type infoDataType, const std::string &dictionaryStr, FmiMenuCommandType commandId, const std::string &acceleratorHelpStr)
{
    std::string menuString = ::GetDictionaryString(dictionaryStr.c_str());
    menuString += acceleratorHelpStr;
    auto menuItem = std::make_unique<NFmiMenuItem>(-1, menuString, param, commandId, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, infoDataType);
    itsPopupMenu->Add(std::move(menuItem));
}

void AddShowHelperData1OntimeSerialViewPopup(const NFmiDataIdent &param, NFmiInfoData::Type infoDataType)
{
    std::string acceleratorHelpStr(" (SHIFT + F)");
    if(!ShowHelperData1InTimeSerialView())
        AddShowHelperDataOntimeSerialViewPopup(param, infoDataType, "TimeSerialViewParamPopUpShowHelpData1", kFmiShowHelperDataOnTimeSerialView, acceleratorHelpStr);
    else
        AddShowHelperDataOntimeSerialViewPopup(param, infoDataType, "TimeSerialViewParamPopUpHideHelpData1", kFmiDontShowHelperDataOnTimeSerialView, acceleratorHelpStr);
}

void AddShowHelperData2OntimeSerialViewPopup(const NFmiDataIdent &param, NFmiInfoData::Type infoDataType)
{
    std::string acceleratorHelpStr(" (CTRL + F)");
    if(GetFavoriteSurfaceModelFractileData())
    {
        if(!ShowHelperData2InTimeSerialView())
            AddShowHelperDataOntimeSerialViewPopup(param, infoDataType, "TimeSerialViewParamPopUpShowHelpData2", kFmiShowHelperData2OnTimeSerialView, acceleratorHelpStr);
        else
            AddShowHelperDataOntimeSerialViewPopup(param, infoDataType, "TimeSerialViewParamPopUpHideHelpData2", kFmiDontShowHelperData2OnTimeSerialView, acceleratorHelpStr);
    }
}

void AddShowHelperData3OntimeSerialViewPopup(const NFmiDataIdent &param, NFmiInfoData::Type infoDataType)
{
    std::string acceleratorHelpStr(" (CTRL + SHIFT + F)");
    if(GetModelClimatologyData())
    {
        if(!ShowHelperData3InTimeSerialView())
            AddShowHelperDataOntimeSerialViewPopup(param, infoDataType, "Show help data 3", kFmiShowHelperData3OnTimeSerialView, acceleratorHelpStr);
        else
            AddShowHelperDataOntimeSerialViewPopup(param, infoDataType, "Hide help data 3", kFmiDontShowHelperData3OnTimeSerialView, acceleratorHelpStr);
    }
}

void AddShowHelperData4OntimeSerialViewPopup(const NFmiDataIdent &param, NFmiInfoData::Type infoDataType)
{
    std::string acceleratorHelpStr(" (ALT + F)");
    if(GetMosTemperatureMinAndMaxData())
    {
        if(!ShowHelperData4InTimeSerialView())
            AddShowHelperDataOntimeSerialViewPopup(param, infoDataType, "Show help data 4", kFmiShowHelperData4OnTimeSerialView, acceleratorHelpStr);
        else
            AddShowHelperDataOntimeSerialViewPopup(param, infoDataType, "Hide help data 4", kFmiDontShowHelperData4OnTimeSerialView, acceleratorHelpStr);
    }
}

// T‰t‰ popupia k‰ytet‰‰n kun lis‰t‰‰n aikasarjaikkunaan uusia n‰yttˆj‰.
bool CreateTimeSerialDialogPopup(int index)
{
	itsCurrentViewRowIndex = gActiveViewRowIndexForTimeSerialView;
	itsTimeSerialViewIndex = index;
	delete itsPopupMenu;
	itsPopupMenu = 0;
	fOpenPopup = false;
	NFmiInfoData::Type infoDataType = NFmiInfoData::kEditable;
	if(itsTimeSerialViewDrawParamList)
	{
		itsPopupMenu = new NFmiMenuItemList;

		MenuCreationSettings menuSettings;
		menuSettings.SetTimeSerialSettings(kFmiAddTimeSerialView);
		CreateParamSelectionBasePopup(menuSettings, itsPopupMenu, "MapViewMaskSelectionPopUpAdd");

		if(itsTimeSerialViewDrawParamList->Index(index))
		{
			NFmiDataIdent param = itsTimeSerialViewDrawParamList->Current()->Param();
			std::string menuString = ::GetDictionaryString("TimeSerialViewParamPopUpRemove");
            auto menuItem = std::make_unique<NFmiMenuItem>(-1, menuString, param, kFmiRemoveTimeSerialView, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, infoDataType, index);
			itsPopupMenu->Add(std::move(menuItem));
			menuString = ::GetDictionaryString("TimeSerialViewParamPopUpRemoveAll");
			menuItem.reset(new NFmiMenuItem(-1, menuString, param, kFmiRemoveAllTimeSerialViews, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, infoDataType));
			itsPopupMenu->Add(std::move(menuItem));

            AddShowHelperData1OntimeSerialViewPopup(param, infoDataType);
            AddShowHelperData2OntimeSerialViewPopup(param, infoDataType);
            AddShowHelperData3OntimeSerialViewPopup(param, infoDataType);
            AddShowHelperData4OntimeSerialViewPopup(param, infoDataType);

			if(itsTimeSerialViewDrawParamList->NumberOfItems())
			{
				menuString = ::GetDictionaryString("TimeSerialViewParamPopUpProperties");
                menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiAddTimeSerialView, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, infoDataType));
				NFmiMenuItemList *menuList = new NFmiMenuItemList;
				int aIndex = 1;
				for(itsTimeSerialViewDrawParamList->Reset(); itsTimeSerialViewDrawParamList->Next();)
				{
                    auto item5 = std::make_unique<NFmiMenuItem>(-1, itsTimeSerialViewDrawParamList->Current()->ParameterAbbreviation()
														,itsTimeSerialViewDrawParamList->Current()->Param()
														,kFmiModifyDrawParam
														,NFmiMetEditorTypes::kFmiParamsDefaultView
														,&itsTimeSerialViewDrawParamList->Current()->Level()
														,infoDataType
														,aIndex
														,itsTimeSerialViewDrawParamList->Current()->ViewMacroDrawParam());
					menuList->Add(std::move(item5));
					aIndex++;
				}
				menuItem->AddSubMenu(menuList);
				itsPopupMenu->Add(std::move(menuItem));
			}
		}

		if(!itsPopupMenu->InitializeCommandIDs(itsPopupMenuStartId))
			return false;
		fOpenPopup = true;
		return true;
	}
	return false;
}

std::string GetFileName(const std::string &theFilePath)
{
    NFmiFileString fileString(theFilePath);
    return std::string(fileString.Header());
}

void AddFixedDrawParamsToMenu(const NFmiFixedDrawParamFolder &theFixedDrawParamFolder, const std::string &theSubFolderName, NFmiMenuItemList &thePopupMenu, unsigned int theDescTopIndex, int theRowIndex, int index, const NFmiDataIdent &theParam, const NFmiLevel* theLevel, NFmiInfoData::Type theDataType, boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
    if(!theFixedDrawParamFolder.Empty())
    {
        auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, theSubFolderName, theParam, kFmiFixedDrawParam, NFmiMetEditorTypes::kFmiIsoLineView, theLevel, theDataType, index, theDrawParam->ViewMacroDrawParam());
        NFmiMenuItemList *subFolderMenuList = new NFmiMenuItemList;

        const std::vector<NFmiFixedDrawParamFolder> &subFolders = theFixedDrawParamFolder.SubFolders();
        for(auto subFolder : subFolders)
            AddFixedDrawParamsToMenu(subFolder, subFolder.SubFolderName(), *subFolderMenuList, theDescTopIndex, theRowIndex, index, theParam, theLevel, theDataType, theDrawParam);

        const std::vector<std::shared_ptr<NFmiDrawParam>> &drawParams = theFixedDrawParamFolder.DrawParams();
        for(auto drawParam : drawParams)
        {
            std::string menuText = GetFileName(drawParam->InitFileName());
            auto drawParamMenuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, menuText, theParam, kFmiFixedDrawParam, NFmiMetEditorTypes::kFmiIsoLineView, theLevel, theDataType, index, theDrawParam->ViewMacroDrawParam());
            drawParamMenuItem->MacroParamInitName(drawParam->InitFileName()); // t‰m‰n avulla haluttu drawParam etsit‰‰n
            subFolderMenuList->Add(std::move(drawParamMenuItem));
        }

        menuItem->AddSubMenu(subFolderMenuList);
        thePopupMenu.Add(std::move(menuItem));
    }
    else
    { // theFixedDrawParamFolder oli tyhj‰, varoitetaan k‰ytt‰j‰‰ ett‰ ei ole lˆytynyt yht‰‰n tehdasasetuksia
        auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, ::GetDictionaryString("No FixedDrawParams were found!"), theParam, kFmiFixedDrawParam, NFmiMetEditorTypes::kFmiIsoLineView, theLevel, theDataType, index, theDrawParam->ViewMacroDrawParam());
        thePopupMenu.Add(std::move(menuItem));
    }
}

bool CreateViewParamsPopup(unsigned int theDescTopIndex, int theRowIndex, int index)
{
	itsCurrentViewRowIndex = theRowIndex;
	delete itsPopupMenu;
	itsPopupMenu = 0;
	fOpenPopup = false;
	NFmiDrawParamList* rowDrawParamList = DrawParamList(theDescTopIndex, theRowIndex);
	if(itsSmartInfoOrganizer && rowDrawParamList)
	{
		if(rowDrawParamList->Index(index))
		{
			bool crossSectionPopup = theDescTopIndex == CtrlViewUtils::kFmiCrossSectionView;
			bool mapViewSectionPopup = theDescTopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex;
			boost::shared_ptr<NFmiDrawParam> drawParam = rowDrawParamList->Current();
			NFmiDataIdent param = drawParam->Param();
			NFmiInfoData::Type dataType2 = drawParam->DataType();
			bool macroParamInCase = NFmiDrawParam::IsMacroParamCase(dataType2);
			if(macroParamInCase) // macroParamin yhteydess‰ parametrin nimeksi pit‰‰ laittaa sen makroparamin alustus tiedosto, koska se on macroParamin yksi tunniste!!
				param.GetParam()->SetName(drawParam->InitFileName());
			const NFmiLevel* level = &rowDrawParamList->Current()->Level();
			NFmiInfoData::Type dataType = rowDrawParamList->Current()->DataType();
			itsPopupMenu = new NFmiMenuItemList;
			std::string menuString;
			std::unique_ptr<NFmiMenuItem> menuItem;
			if(crossSectionPopup == false)
			{ // poikkileikkaus-n‰yttˆ ei tue tekstimuotoista piirtoa
				menuString = ::GetDictionaryString("MapViewParamOptionPopUpText");
				menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiModifyView, NFmiMetEditorTypes::kFmiTextView, level, dataType, index, drawParam->ViewMacroDrawParam()));
				itsPopupMenu->Add(std::move(menuItem));
			}
			menuString = ::GetDictionaryString("MapViewParamOptionPopUpIsoline");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiModifyView, NFmiMetEditorTypes::kFmiIsoLineView, level, dataType, index, drawParam->ViewMacroDrawParam()));
			itsPopupMenu->Add(std::move(menuItem));

			menuString = ::GetDictionaryString("MapViewParamOptionPopUpContour");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiModifyView, NFmiMetEditorTypes::kFmiColorContourView, level, dataType, index, drawParam->ViewMacroDrawParam()));
			itsPopupMenu->Add(std::move(menuItem));
			menuString = ::GetDictionaryString("MapViewParamOptionPopUpContourIsoline");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiModifyView, NFmiMetEditorTypes::kFmiColorContourIsoLineView, level, dataType, index, drawParam->ViewMacroDrawParam()));
			itsPopupMenu->Add(std::move(menuItem));
			menuString = ::GetDictionaryString("MapViewParamOptionPopUpQuickContour");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiModifyView, NFmiMetEditorTypes::kFmiQuickColorContourView, level, dataType, index, drawParam->ViewMacroDrawParam()));
			itsPopupMenu->Add(std::move(menuItem));

			menuString = ::GetDictionaryString("MapViewParamOptionPopUpRemove");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiRemoveView, NFmiMetEditorTypes::kFmiIsoLineView, level, dataType, index, drawParam->ViewMacroDrawParam()));
			itsPopupMenu->Add(std::move(menuItem));
			menuString = ::GetDictionaryString("MapViewParamOptionPopUpHide");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiHideView, NFmiMetEditorTypes::kFmiIsoLineView, level, dataType, index, drawParam->ViewMacroDrawParam()));
			itsPopupMenu->Add(std::move(menuItem));
			menuString = ::GetDictionaryString("MapViewParamOptionPopUpShow");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiShowView, NFmiMetEditorTypes::kFmiIsoLineView, level, dataType, index, drawParam->ViewMacroDrawParam()));
			itsPopupMenu->Add(std::move(menuItem));
			menuString = ::GetDictionaryString("MapViewParamOptionPopUpActivate");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiActivateView, NFmiMetEditorTypes::kFmiIsoLineView, level, dataType, index, drawParam->ViewMacroDrawParam()));
			menuItem->ExtraParam(0); // kun parametria aktivoidaan (tai tulevaisuudessa tehd‰‰n mit‰ vain), k‰ytet‰‰n extraParamia kertomaan miss‰ parametri on, 0=karttan‰yttˆ, 1=poikkileikkaus ja 2=aikasarja
			itsPopupMenu->Add(std::move(menuItem));
            menuString = ::GetDictionaryString("Save DrawParam");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiStoreDrawParam, NFmiMetEditorTypes::kFmiIsoLineView, level, dataType, index, drawParam->ViewMacroDrawParam()));
            itsPopupMenu->Add(std::move(menuItem));
            
            menuString = ::GetDictionaryString("Reload DrawParam");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiReloadDrawParam, NFmiMetEditorTypes::kFmiIsoLineView, level, dataType, index, drawParam->ViewMacroDrawParam()));
            itsPopupMenu->Add(std::move(menuItem));

            // FixedDrawParam valikko t‰h‰n v‰liin
            AddFixedDrawParamsToMenu(itsFixedDrawParamSystem.RootFolder(), ::GetDictionaryString("FixedDrawParams"), *itsPopupMenu, theDescTopIndex, theRowIndex, index, param, level, dataType, drawParam);

			// copy/paste komennot t‰h‰n
			menuString = "Copy draw options";
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiCopyDrawParamOptions, NFmiMetEditorTypes::kFmiIsoLineView, level, dataType, index, drawParam->ViewMacroDrawParam()));
			itsPopupMenu->Add(std::move(menuItem));
			if(fCopyPasteDrawParamAvailableYet)
			{
				menuString = "Paste draw options";
                menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiPasteDrawParamOptions, NFmiMetEditorTypes::kFmiIsoLineView, level, dataType, index, drawParam->ViewMacroDrawParam()));
				itsPopupMenu->Add(std::move(menuItem));
			}

			if(!macroParamInCase)
			{
				if(mapViewSectionPopup)
				{ // t‰ss‰ lis‰t‰‰n vertailu analyysi/havainto -dataan optiot (vain karttan‰yttˆ parametreille)
					if(drawParam->IsModelRunDataType() && drawParam->Level().GetIdent() == 0)
					{ // tehd‰‰n t‰m‰ vain pintadatoille
						checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > analyzeInfos = InfoOrganizer()->GetInfos(NFmiInfoData::kAnalyzeData);
						if(analyzeInfos.size())
						{
							menuString = "DiffToAnalyzeData"; // ::GetDictionaryString("DiffToAnalyzeData");
							auto diffToAnalyzeMenuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, menuString, param, kFmiDiffToAnalyzeData, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kNoDataType, index);
							NFmiMenuItemList *diffToAnalyzeMenuList = new NFmiMenuItemList;

                            auto item1 = std::make_unique<NFmiMenuItem>(theDescTopIndex, "Diff To None", param, kFmiDiffToAnalyzeData, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kNoDataType, index);
							item1->ExtraParam(0);
							diffToAnalyzeMenuList->Add(std::move(item1));

							for(size_t i = 0; i < analyzeInfos.size(); i++)
							{
								if(analyzeInfos[i]->SizeLevels() <= 1)
								{ // vain pintadatat otetaan k‰sittelyyn
                                    auto item2 = std::make_unique<NFmiMenuItem>(theDescTopIndex, std::string(analyzeInfos[i]->Param().GetProducer()->GetName()), param, kFmiDiffToAnalyzeData, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, analyzeInfos[i]->DataType(), index);
									item2->ExtraParam(analyzeInfos[i]->Param().GetProducer()->GetIdent());
									diffToAnalyzeMenuList->Add(std::move(item2));
								}
							}

							diffToAnalyzeMenuItem->AddSubMenu(diffToAnalyzeMenuList);
							itsPopupMenu->Add(std::move(diffToAnalyzeMenuItem));
						}
					}
				}

				menuString = "Previous model run"; //::GetDictionaryString("MapViewParamOptionPopUpActivate");
				menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiModelRunOffsetPrevious, NFmiMetEditorTypes::kFmiIsoLineView, level, dataType, index, drawParam->ViewMacroDrawParam()));
				itsPopupMenu->Add(std::move(menuItem));

				if(drawParam->IsModelRunDataType())
				{ // t‰ss‰ lis‰t‰‰n malliajojen ero-piirto -osio
					std::string menuString2 = "DiffBetweenModelRuns"; // ::GetDictionaryString("DiffBetweenModelRuns");
					auto modelRunDiffMenuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, menuString2, param, kFmiDiffBetweenModelRuns, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, dataType, index);

					NFmiMenuItemList *modelRunDiffMenuList = new NFmiMenuItemList;
					for(int i = 0; i >= -10; i--)
					{
						std::string multiModeStr;
						if(i == 0)
							multiModeStr += "No diff (0)";
						else
							multiModeStr += std::string("Diff of ") + NFmiStringTools::Convert(i);

                        auto item1 = std::make_unique<NFmiMenuItem>(theDescTopIndex, multiModeStr, param, kFmiDiffBetweenModelRuns, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, dataType, index);
						item1->ExtraParam(i);
						modelRunDiffMenuList->Add(std::move(item1));
					}
					modelRunDiffMenuItem->AddSubMenu(modelRunDiffMenuList);

					itsPopupMenu->Add(std::move(modelRunDiffMenuItem));

					// lis‰t‰‰n multi-model piirto valikko
					AddMultiModelRunToMenu(drawParam, *itsPopupMenu, theDescTopIndex, index);
				}

				if(drawParam && drawParam->ShowDifference())
				{
					menuString = ::GetDictionaryString("MapViewParamOptionPopUpTimeDiffOff");
					menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiToggleShowDifference, NFmiMetEditorTypes::kFmiIsoLineView, level, dataType, index, drawParam->ViewMacroDrawParam()));
				}
				else
				{
					menuString = ::GetDictionaryString("MapViewParamOptionPopUpTimeDiffOn");
					menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiToggleShowDifference, NFmiMetEditorTypes::kFmiIsoLineView, level, dataType, index, drawParam->ViewMacroDrawParam()));
				}
				itsPopupMenu->Add(std::move(menuItem));

				if(crossSectionPopup == false)
				{
					if(drawParam && drawParam->ShowDifferenceToOriginalData())
					{
						menuString = ::GetDictionaryString("MapViewParamOptionPopUpOrigDiffOff");
						menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiToggleShowDifferenceToOriginalData, NFmiMetEditorTypes::kFmiIsoLineView, level, dataType, index, drawParam->ViewMacroDrawParam()));
					}
					else
					{
						menuString = ::GetDictionaryString("MapViewParamOptionPopUpOrigDiffOn");
						menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiToggleShowDifferenceToOriginalData, NFmiMetEditorTypes::kFmiIsoLineView, level, dataType, index, drawParam->ViewMacroDrawParam()));
					}
					itsPopupMenu->Add(std::move(menuItem));
				}
			}
			menuString = ::GetDictionaryString("MapViewParamOptionPopUpProperties");
			menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiModifyDrawParam, NFmiMetEditorTypes::kFmiIsoLineView, level, dataType, index, drawParam->ViewMacroDrawParam()));
			itsPopupMenu->Add(std::move(menuItem));

			if(!itsPopupMenu->InitializeCommandIDs(itsPopupMenuStartId))
				return false;
			fOpenPopup = true;
			return true;
		}
	}
	return false;
}

bool CreateMaskParamsPopup(int theRowIndex, int index)
{
	itsCurrentViewRowIndex = theRowIndex;
	delete itsPopupMenu;
	itsPopupMenu = 0;
	fOpenPopup = false;
	boost::shared_ptr<NFmiAreaMaskList> tempParamMaskList = ParamMaskListMT();
	if(tempParamMaskList)
	{
		if(tempParamMaskList->Index(index))
		{
			const NFmiDataIdent *paramPtr = tempParamMaskList->Current()->DataIdent();
			NFmiDataIdent param = paramPtr ? *paramPtr : NFmiDataIdent();
			const NFmiLevel* level = 0;
			if(tempParamMaskList->Current()->UseLevelInfo())
				level = tempParamMaskList->Current()->Level();
			NFmiInfoData::Type dataType = tempParamMaskList->Current()->GetDataType();

			itsPopupMenu = new NFmiMenuItemList;
			std::string menuString = ::GetDictionaryString("NormalWordCapitalRemove");
            auto menuItem = std::make_unique<NFmiMenuItem>(-1, menuString, param, kFmiRemoveMask, NFmiMetEditorTypes::kFmiParamsDefaultView, level, dataType, index);
			itsPopupMenu->Add(std::move(menuItem));
			menuString = ::GetDictionaryString("NormalWordCapitalUse");
			menuItem.reset(new NFmiMenuItem(-1, menuString, param, kFmiEnAbleMask, NFmiMetEditorTypes::kFmiParamsDefaultView, level, dataType, index));
			itsPopupMenu->Add(std::move(menuItem));
			menuString = ::GetDictionaryString("NormalWordCapitalDisable");
			menuItem.reset(new NFmiMenuItem(-1, menuString, param, kFmiDisAbleMask, NFmiMetEditorTypes::kFmiParamsDefaultView, level, dataType, index));
			itsPopupMenu->Add(std::move(menuItem));
			menuString = ::GetDictionaryString("NormalWordCapitalModify");
			menuItem.reset(new NFmiMenuItem(-1, menuString, param, kFmiModifyMask, NFmiMetEditorTypes::kFmiParamsDefaultView, level, dataType, index));
			itsPopupMenu->Add(std::move(menuItem));

			if(!itsPopupMenu->InitializeCommandIDs(itsPopupMenuStartId))
				return false;
			fOpenPopup = true;
			return true;
		}
	}
	return false;
}

// n‰yttˆkomentoja, joita voi antaa dokumentille.
// K‰ytet‰‰n aluksi ainakin erilaisten tuplaklikkausten toteutuksia varten.
// Tulevaisuudessa n‰yttˆ makrot voisi toteuttaa t‰t‰ kautta?
// Menuitemiin on sis‰llytetty mm. komento, parametri ja level ja tuottaja tiedot.
// viewIndex on esim. joko karttarivi tai aikasarjaikkunan indeksi.
// viewType voi kertoa esim. onko kyseeess‰ kartta tai aikasarja n‰yttˆ (tulevaisuudessa lis‰‰ esim. luotaus jne.).
bool ExecuteCommand(const NFmiMenuItem &theMenuItem, int theRowIndex, int /* theViewTypeId */ )
{
	FmiMenuCommandType command = theMenuItem.CommandType();
	switch(command)
	{
	case kFmiModifyView:
		ModifyView(theMenuItem, theRowIndex);
		break;
	case kFmiModifyDrawParam:
		ModifyDrawParam(theMenuItem, theRowIndex);
		break;
	case kFmiActivateView:
		ActivateView(theMenuItem, theRowIndex);
        break;
    case kAddViewWithRealRowNumber:
        AddViewWithRealRowNumber(true, theMenuItem, theRowIndex);
        break;
    default:
        return false;
    }

    MakeMacroParamCacheUpdatesForWantedRow(theMenuItem.MapViewDescTopIndex(), theRowIndex);
    return true;
}

void SetModelRunOffset(const NFmiMenuItem &theMenuItem, int theViewRowIndex)
{
	boost::shared_ptr<NFmiDrawParam> modifiedDrawParam = GetDrawParamFromViewLists(theMenuItem, theViewRowIndex);
	if(modifiedDrawParam)
	{
		if(theMenuItem.CommandType() == kFmiModelRunOffsetPrevious)
		{
			modifiedDrawParam->ModelOriginTime(NFmiMetTime::gMissingTime); // nollataan mahd. fiksattu origin aika
			modifiedDrawParam->ModelRunIndex(modifiedDrawParam->ModelRunIndex()-1); // siirret‰‰n offset edelliseen aikaan
		}
		else if(theMenuItem.CommandType() == kFmiModelRunOffsetNext)
		{
			modifiedDrawParam->ModelOriginTime(NFmiMetTime::gMissingTime); // nollataan mahd. fiksattu origin aika
			modifiedDrawParam->ModelRunIndex(modifiedDrawParam->ModelRunIndex()+1); // siirret‰‰n offset seuraavaan aikaan
			if(modifiedDrawParam->ModelRunIndex() > 0)
				modifiedDrawParam->ModelRunIndex(0);
		}

        MakeMapViewRowDirty(theMenuItem.MapViewDescTopIndex(), theViewRowIndex);
	}
}

void MakeMapViewRowDirty(int theMapViewDescTopIndex, int theViewRowIndex)
{
    // mapview cached alkaa 0:sta ja theViewRowIndex alkaa 1:st‰
    MapViewDescTop(theMapViewDescTopIndex)->MapViewCache().MakeRowDirty(theViewRowIndex - 1);
    MapViewDirty(theMapViewDescTopIndex, false, false, true, false, false, true);
}

void SetModelRunDifference(const NFmiMenuItem &theMenuItem, int theViewRowIndex)
{
	boost::shared_ptr<NFmiDrawParam> modifiedDrawParam = GetDrawParamFromViewLists(theMenuItem, theViewRowIndex);
	if(modifiedDrawParam)
	{
		modifiedDrawParam->ModelRunDifferenceIndex(static_cast<int>(theMenuItem.ExtraParam()));

		MapViewDescTop(theMenuItem.MapViewDescTopIndex())->MapViewCache().MakeRowDirty(theViewRowIndex);
		MapViewDirty(theMenuItem.MapViewDescTopIndex(), false, false, true, false, false, false);
	}
}

void SetDiffToAnalyzeData(const NFmiMenuItem &theMenuItem, int theViewRowIndex)
{
	boost::shared_ptr<NFmiDrawParam> &modifiedDrawParam = GetDrawParamFromViewLists(theMenuItem, theViewRowIndex);
	if(modifiedDrawParam)
	{
		modifiedDrawParam->DataComparisonProdId(static_cast<unsigned long>(theMenuItem.ExtraParam()));
		modifiedDrawParam->DataComparisonType(theMenuItem.DataType());

		MapViewDescTop(theMenuItem.MapViewDescTopIndex())->MapViewCache().MakeRowDirty(theViewRowIndex);
		MapViewDirty(theMenuItem.MapViewDescTopIndex(), false, false, true, false, false, false);
	}
}

// uusi versio MakePopUpCommand:ista, joka k‰ytt‰‰ hyv‰kseen itsCurrentViewRowIndex-attribuuttia
bool MakePopUpCommandUsingRowIndex(unsigned short theCommandID)
{
	NFmiMetTime time;
	if(itsPopupMenu->Find(theCommandID))
	{
		NFmiMenuItem * menuItem = itsPopupMenu->RecursivelyFoundMenuItem();
		if(!menuItem)
			return false;
		FmiMenuCommandType command = menuItem->CommandType();
		switch(command)
		{
		case kFmiAddView:
			AddView(*menuItem, itsCurrentViewRowIndex);
			break;
		case kFmiAddParamCrossSectionView:
			AddCrossSectionView(*menuItem, itsCurrentCrossSectionRowIndex, false);
			break;
		case kFmiAddAsOnlyView:
			AddAsOnlyView(*menuItem, itsCurrentViewRowIndex);
			break;
		case kFmiAddAsOnlyParamCrossSectionView:
			AddAsOnlyCrossSectionView(*menuItem, itsCurrentCrossSectionRowIndex);
			break;
		case kFmiRemoveView:
			RemoveView(*menuItem, itsCurrentViewRowIndex);
			break;
		case kFmiRemoveParamCrossSectionView:
			RemoveCrosssectionDrawParam(*menuItem, itsCurrentCrossSectionRowIndex);
			break;
		case kFmiRemoveAllViews:
			RemoveAllViews(menuItem->MapViewDescTopIndex(), itsCurrentViewRowIndex);
			break;
        case kFmiRemoveAllParamsFromAllRows:
            ClearDesctopsAllParams(menuItem->MapViewDescTopIndex());
            break;
        case kFmiRemoveAllParamsCrossSectionView:
			RemoveAllCrossSectionViews(itsCurrentCrossSectionRowIndex);
			break;
		case kFmiModifyView:
			ModifyView(*menuItem, itsCurrentViewRowIndex);
			break;
        case kFmiFixedDrawParam:
            ApplyFixeDrawParam(*menuItem, itsCurrentViewRowIndex);
            break;
        case kFmiStoreDrawParam:
            SaveDrawParamSettings(*menuItem, itsCurrentViewRowIndex);
            break;
        case kFmiReloadDrawParam:
            ReloadDrawParamSettings(*menuItem, itsCurrentViewRowIndex);
            break;
        case kFmiChangeAllProducersInMapRow:
			ChangeAllProducersInMapRow(*menuItem, itsCurrentViewRowIndex, false);
			break;
		case kFmiChangeAllProducersInCrossSectionRow:
			ChangeAllProducersInMapRow(*menuItem, itsCurrentCrossSectionRowIndex, true);
			break;
		case kFmiChangeAllDataTypesInMapRow:
			ChangeAllDataTypesInMapRow(*menuItem, itsCurrentViewRowIndex, false);
			break;
		case kFmiChangeAllDataTypesInCrossSectionRow:
			ChangeAllDataTypesInMapRow(*menuItem, itsCurrentCrossSectionRowIndex, true);
			break;
		case kFmiCopyDrawParamOptions:
			CopyDrawParamOptions(*menuItem, itsCurrentViewRowIndex, false);
			break;
		case kFmiCopyMapViewDescTopParams:
			CopyMapViewDescTopParams(menuItem->MapViewDescTopIndex());
			break;
		case kFmiPasteMapViewDescTopParams:
			PasteMapViewDescTopParams(menuItem->MapViewDescTopIndex());
			break;
		case kFmiCopyDrawParamsFromMapViewRow:
			CopyDrawParamsFromViewRow(*menuItem, itsCurrentViewRowIndex, false);
			break;
		case kFmiPasteDrawParamsToMapViewRow:
			PasteDrawParamsToViewRow(*menuItem, itsCurrentViewRowIndex, false);
			break;
		case kFmiPasteDrawParamsToCrossSectionViewRow:
			PasteDrawParamsToViewRow(*menuItem, itsCurrentCrossSectionRowIndex, true);
			break;
		case kFmiCopyDrawParamsFromCrossSectionViewRow:
			CopyDrawParamsFromViewRow(*menuItem, itsCurrentCrossSectionRowIndex, true);
			break;
		case kFmiPasteDrawParamOptions:
			PasteDrawParamOptions(*menuItem, itsCurrentViewRowIndex, false);
			break;
		case kFmiCopyDrawParamOptionsCrossSectionView:
			CopyDrawParamOptions(*menuItem, itsCurrentCrossSectionRowIndex, true);
			break;
		case kFmiPasteDrawParamOptionsCrossSectionView:
			PasteDrawParamOptions(*menuItem, itsCurrentCrossSectionRowIndex, true);
			break;
		case kFmiAddMask:
			AddMask(*menuItem, itsCurrentViewRowIndex, false);
			break;
		case kFmiAddAsOnlyMask:
			AddAsOnlyMask(*menuItem, itsCurrentViewRowIndex);
			break;
		case kFmiRemoveAllMasks:
			RemoveAllMasks(itsCurrentViewRowIndex);
			break;
		case kFmiHideView:
			HideView(*menuItem, itsCurrentViewRowIndex);
			break;
		case kFmiShowView:
			ShowView(*menuItem, itsCurrentViewRowIndex);
			break;
		case kFmiRemoveMask:
			RemoveMask(*menuItem);
			break;
		case kFmiDisAbleMask:
			DisableMask(*menuItem);
			break;
		case kFmiEnAbleMask:
			EnableMask(*menuItem);
			break;
		case kFmiModifyMask:
			ModifyMask(*menuItem);
			break;
		case kFmiModifyDrawParam:
			ModifyDrawParam(*menuItem, itsCurrentViewRowIndex);
			break;
		case kFmiModifyCrossSectionDrawParam:
			ModifyCrossSectionDrawParam(*menuItem, itsCurrentCrossSectionRowIndex);
			break;
		case kFmiShowParamCrossSectionView:
		case kFmiHideParamCrossSectionView:
			ShowCrossSectionDrawParam(*menuItem, itsCurrentCrossSectionRowIndex, command == kFmiShowParamCrossSectionView ? true : false);
			break;
		case kFmiCrossSectionSetTrajectoryTimes:
			SetCrossSectionTrajectoryTimes(itsCurrentCrossSectionRowIndex);
			break;
		case kFmiCrossSectionSetTrajectoryParams:
			SetCrossSectionTrajectoryParams();
			break;
		case kFmiAddTimeSerialView:
			AddTimeSerialView(*menuItem);
			break;
		case kFmiRemoveTimeSerialView:
			RemoveTimeSerialView(*menuItem);
			break;
		case kFmiTimeSerialModelRunCountSet:
			TimeSerialViewModelRunCountSet(*menuItem, itsCurrentViewRowIndex);
			break;
		case kFmiActivateView:
			ActivateView(*menuItem, itsCurrentViewRowIndex);
			break;
		case kFmiActivateCrossSectionDrawParam:
			ActivateCrossSectionParam(*menuItem, itsCurrentCrossSectionRowIndex);
			break;
		case kFmiToggleShowDifference:
			ToggleShowDifferenceOnMapView(*menuItem, itsCurrentViewRowIndex);
			break;
		case kFmiToggleShowDifferenceToOriginalData:
			ToggleShowDifferenceToOriginalData(*menuItem, itsCurrentViewRowIndex);
			break;
		case kFmiRemoveAllTimeSerialViews:
			RemoveAllTimeSerialViews();
			break;
		case kFmiShowHelperDataOnTimeSerialView:
		case kFmiDontShowHelperDataOnTimeSerialView:
		case kFmiShowHelperData2OnTimeSerialView:
		case kFmiDontShowHelperData2OnTimeSerialView:
        case kFmiShowHelperData3OnTimeSerialView:
        case kFmiDontShowHelperData3OnTimeSerialView:
        case kFmiShowHelperData4OnTimeSerialView:
        case kFmiDontShowHelperData4OnTimeSerialView:
        {
            ToggleShowHelperDatasInTimeView(command);
            break;
        }
		case kFmiActivateCP:
		case kFmiDeactivateCP:
		case kFmiEnableCP:
		case kFmiDisableCP:
		case kFmiShowCPAllwaysOnTimeView:
		case kFmiDontShowCPAllwaysOnTimeView:
		case kFmiShowAllCPsAllwaysOnTimeView:
		case kFmiShowOnlyActiveCPOnTimeView:
		case kFmiModifyCPAttributes:
            DoControlPointCommand(command);
            break;

		case kFmiSelectCPManagerFromSet:
			itsCPManagerSet.SetCPManager(menuItem->IndexInViewRow());
			TimeSerialViewDirty(true);
			break;
        case kFmiObservationStationsToCpPoints:
            MakeObservationStationsToCpPoints(*menuItem);
            break;

		case kFmiHideAllMapViewObservations:
			HideShowAllMapViewParams(menuItem->MapViewDescTopIndex(), true, false, false, false);
			break;
		case kFmiShowAllMapViewObservations:
			HideShowAllMapViewParams(menuItem->MapViewDescTopIndex(), false, true, false, false);
			break;
		case kFmiHideAllMapViewForecasts:
			HideShowAllMapViewParams(menuItem->MapViewDescTopIndex(), false, false, true, false);
			break;
		case kFmiShowAllMapViewForecasts:
			HideShowAllMapViewParams(menuItem->MapViewDescTopIndex(), false, false, false, true);
			break;

		case kFmiModelRunOffsetPrevious:
		case kFmiModelRunOffsetNext:
			SetModelRunOffset(*menuItem, itsCurrentViewRowIndex);
			break;

		case kFmiDiffBetweenModelRuns:
			SetModelRunDifference(*menuItem, itsCurrentViewRowIndex);
			break;

		case kFmiDiffToAnalyzeData:
			SetDiffToAnalyzeData(*menuItem, itsCurrentViewRowIndex);
			break;

		default:
			return false;
		}
	}
	return true;
}

//bool IsCommandForCrossSectionChanges(FmiMenuCommandType command)
//{
//    switch(command)
//    {
//    case kFmiAddParamCrossSectionView:
//    case kFmiAddAsOnlyParamCrossSectionView:
//    case kFmiDontShowPressureLevelsCrossSectionView:
//    case kFmiShowPressureLevelsUnderCrossSectionView:
//    case kFmiShowPressureLevelsOverCrossSectionView:
//    case kFmiRemoveParamCrossSectionView:
//    case kFmiRemoveAllParamsCrossSectionView:
//    case kFmiHideParamCrossSectionView:
//    case kFmiShowParamCrossSectionView:
//    case kFmiModifyCrossSectionDrawParam:
//    case kFmiCrossSectionSetTrajectoryTimes:
//    case kFmiCrossSectionSetTrajectoryParams:
//    case kFmiCopyDrawParamOptionsCrossSectionView:
//    case kFmiPasteDrawParamOptionsCrossSectionView:
//    case kFmiChangeAllProducersInCrossSectionRow:
//    case kFmiCopyDrawParamsFromCrossSectionViewRow:
//    case kFmiPasteDrawParamsToCrossSectionViewRow:
//    case kFmiActivateCrossSectionDrawParam:
//    case kFmiChangeAllDataTypesInCrossSectionRow:
//        return true;
//    default:
//        return false;
//    }
//}

// Nyt on voitu lis‰t‰/poistaa/muuttaa eri n‰ytˆill‰ olevien parametrien ja siten myˆs riveill‰ 
// olevien macroParamien paikkaa riviss‰. T‰m‰n vuoksi pit‰‰ p‰ivitt‰‰ rivin macroParam cachen tila.
void MakeMacroParamCacheUpdatesForCurrentRow(int mapViewDescTopIndex)
{
    int usedRowIndex = 0;
    if(mapViewDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
    {
        usedRowIndex = itsCurrentCrossSectionRowIndex;
    }
    else if(mapViewDescTopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex)
    {
        usedRowIndex = itsCurrentViewRowIndex;
    }

    MakeMacroParamCacheUpdatesForWantedRow(mapViewDescTopIndex, usedRowIndex);
}

void MakeMacroParamCacheUpdatesForWantedRow(int mapViewDescTopIndex, int usedRowIndex)
{
    NFmiDrawParamList *drawParamList = DrawParamList(mapViewDescTopIndex, usedRowIndex);
    if(drawParamList)
        MacroParamDataCache().update(mapViewDescTopIndex, usedRowIndex, *drawParamList);
}


void DoControlPointCommand(FmiMenuCommandType command)
{
    switch(command)
    {
    case kFmiActivateCP:
    {
        if(CPManager()->FindNearestCP(itsToolTipLatLonPoint, true))
            CPManager()->ActivateCP(CPManager()->CPIndex(), true);
    }
    break;
    case kFmiDeactivateCP:
    {
        if(CPManager()->FindNearestCP(itsToolTipLatLonPoint, true))
            CPManager()->ActivateCP(CPManager()->CPIndex(), false);
    }
    break;
    case kFmiEnableCP:
    {
        if(CPManager()->FindNearestCP(itsToolTipLatLonPoint, true))
            CPManager()->EnableCP(CPManager()->CPIndex(), true);
    }
    break;
    case kFmiDisableCP:
    {
        if(CPManager()->FindNearestCP(itsToolTipLatLonPoint, true))
            CPManager()->EnableCP(CPManager()->CPIndex(), false);
    }
    break;

    case kFmiShowCPAllwaysOnTimeView:
    {
        if(CPManager()->FindNearestCP(itsToolTipLatLonPoint, true))
            CPManager()->ShowCPAllwaysOnTimeView(true);
    }
    break;
    case kFmiDontShowCPAllwaysOnTimeView:
    {
        if(CPManager()->FindNearestCP(itsToolTipLatLonPoint, true))
            CPManager()->ShowCPAllwaysOnTimeView(false);
    }
    break;
    case kFmiShowAllCPsAllwaysOnTimeView:
    {
        CPManager()->ShowAllCPsAllwaysOnTimeView(true);
    }
    break;
    case kFmiShowOnlyActiveCPOnTimeView:
    {
        CPManager()->ShowAllCPsAllwaysOnTimeView(false);
    }
    break;
    case kFmiModifyCPAttributes:
    {
        if(CPManager()->FindNearestCP(itsToolTipLatLonPoint, true))
        {
            boost::shared_ptr<NFmiEditorControlPointManager> tempCPMan(new NFmiEditorControlPointManager(*CPManager()));
            const NFmiGrid* dataGrid = 0;
            boost::shared_ptr<NFmiFastQueryInfo> editedInfo = EditedInfo();
            if(editedInfo)
                dataGrid = editedInfo->Grid();
            CFmiCPModifyingDlg dlg(&(*tempCPMan), itsToolTipLatLonPoint, dataGrid);
            if(dlg.DoModal() == IDOK)
            {
                itsCPManagerSet.SetCPManager(tempCPMan);
            }
        }
    }
    break;
    }
}


bool IsDrawParamForecast(boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
	if(theDrawParam)
	{
		NFmiInfoData::Type aType = theDrawParam->DataType();
		if(aType == NFmiInfoData::kViewable)
			return true;
		if(aType == NFmiInfoData::kHybridData)
			return true;
		if(aType == NFmiInfoData::kKepaData)
			return true;
		if(aType == NFmiInfoData::kEditable)
			return true;
		if(aType == NFmiInfoData::kCopyOfEdited)
			return true;
		if(aType == NFmiInfoData::kMacroParam)
			return true;
		if(aType == NFmiInfoData::kCrossSectionMacroParam)
			return true;
		if(aType == NFmiInfoData::kQ3MacroParam)
			return true;
		if(aType == NFmiInfoData::kModelHelpData)
			return true;
		if(aType == NFmiInfoData::kConceptualModelData)
			return true;
		if(aType == NFmiInfoData::kEditingHelpData)
			return true;
		if(aType == NFmiInfoData::kTrajectoryHistoryData)
			return true;
	}
	return false;
}

bool IsDrawParamObservation(boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
	if(theDrawParam)
	{
		NFmiInfoData::Type aType = theDrawParam->DataType();
		if(aType == NFmiInfoData::kObservations)
			return true;
		if(aType == NFmiInfoData::kClimatologyData)
			return true;
		if(aType == NFmiInfoData::kAnalyzeData)
			return true;
		if(aType == NFmiInfoData::kFlashData)
			return true;
		if(aType == NFmiInfoData::kSatelData)
			return true;
	}
	return false;
}

void HideShowAllMapViewParams(unsigned int theDescTopIndex, bool fHideAllObservations, bool fShowAllObservations, bool fHideAllForecasts, bool fShowAllForecasts)
{
	NFmiPtrList<NFmiDrawParamList>::Iterator iter = MapViewDescTop(theDescTopIndex)->DrawParamListVector()->Start();
	for(; iter.Next();)
	{
		NFmiDrawParamList *aList = iter.CurrentPtr();
		if(aList)
		{
			for(aList->Reset(); aList->Next(); )
			{
				boost::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
				if(drawParam)
				{
					if(fHideAllObservations)
					{
						if(IsDrawParamObservation(drawParam))
							drawParam->HideParam(true);
					}
					else if(fShowAllObservations)
					{
						if(IsDrawParamObservation(drawParam))
							drawParam->HideParam(false);
					}
					else if(fHideAllForecasts)
					{
						if(IsDrawParamForecast(drawParam))
							drawParam->HideParam(true);
					}
					else if(fShowAllForecasts)
					{
						if(IsDrawParamForecast(drawParam))
							drawParam->HideParam(false);
					}

				}
			}
		}
	}
	CheckAnimationLockedModeTimeBags(theDescTopIndex, false); // kun parametrin n‰kyvyytt‰ vaihdetaan, pit‰‰ tehd‰ mahdollisesti animaatio moodin datan tarkistus
    MakeWholeDesctopDirtyActions(theDescTopIndex, nullptr);
}

void ToggleShowDifferenceOnMapView(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
	boost::shared_ptr<NFmiDrawParam> &modifiedDrawParam = GetDrawParamFromViewLists(theMenuItem, theRowIndex);
	if(modifiedDrawParam)
	{
		modifiedDrawParam->ShowDifference(!(modifiedDrawParam->ShowDifference()));
		UpdateModifiedDrawParamMarko(theMenuItem.MapViewDescTopIndex(), modifiedDrawParam, theRowIndex);
	}
}

void ToggleShowDifferenceToOriginalData(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
	boost::shared_ptr<NFmiDrawParam> &modifiedDrawParam = GetDrawParamFromViewLists(theMenuItem, theRowIndex);
	if(modifiedDrawParam)
	{
		modifiedDrawParam->ShowDifferenceToOriginalData(!(modifiedDrawParam->ShowDifferenceToOriginalData()));
		UpdateModifiedDrawParamMarko(theMenuItem.MapViewDescTopIndex(), modifiedDrawParam, theRowIndex);
	}
}

void ToggleShowDifferenceOnMapViewCompareModels(const NFmiMenuItem& /* theMenuItem */ , int /* theRowIndex */ )
{
}
void RemoveAllTimeSerialViews(void)
{
	TimeSerialViewDirty(true);
	itsTimeSerialViewDrawParamList->Clear();
}

void AddView(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
	AddViewWithRealRowNumber(true, theMenuItem, theRowIndex + MapViewDescTop(theMenuItem.MapViewDescTopIndex())->MapRowStartingIndex() - 1); // lasketaan todellinen rivinumero (johtuu karttan‰ytˆn virtuaali riveist‰)

	// lis‰‰n t‰m‰n CheckAnimationLockedModeTimeBags -kutsun vain perus AddView-metodin yhteyteen, mutta en esim.
	// AddViewWithRealRowNumber -metodin yhteyteen, ett‰ homma ei mene pelk‰ksi tarkasteluksi.
	// AddViewWithRealRowNumber -metodia k‰ytet‰‰n varsin laajasti ja tarkasteluja tulisi tehty‰ liikaa.
    if(CtrlViewFastInfoFunctions::IsObservationLockModeDataType(theMenuItem.DataType()) || theMenuItem.DataType() == NFmiInfoData::kSatelData)
		CheckAnimationLockedModeTimeBags(theMenuItem.MapViewDescTopIndex(), false); // kun parametrin n‰kyvyytt‰ vaihdetaan, pit‰‰ tehd‰ mahdollisesti animaatio moodin datan tarkistus
}

void SetDrawMacroSettings(const NFmiMenuItem& theMenuItem, boost::shared_ptr<NFmiDrawParam> &theDrawParam, const std::string *theMacroParamInitFileName)
{
	NFmiInfoData::Type dataType = theMenuItem.DataType();
	if(NFmiDrawParam::IsMacroParamCase(dataType))
	{
		theDrawParam->ParameterAbbreviation(theMenuItem.MenuText()); // macroParamin tapauksessa pit‰‰ nimi asettaa t‰ss‰ (t‰t‰ nimilyhennett‰ k‰ytet‰‰n tunnisteen‰ myˆhemmin!!)
		boost::shared_ptr<NFmiMacroParam> usedMacroParam;
	if(theMacroParamInitFileName == 0)
	{
		boost::shared_ptr<NFmiMacroParamFolder> currentFolder = MacroParamSystem().GetCurrent();
			if(currentFolder && currentFolder->Find(theDrawParam->ParameterAbbreviation()))
				usedMacroParam = currentFolder->Current();
			// kokeillaan viel‰ onko macroparam laitettu popup-menun kautta, jolloin pit‰‰ tehd‰ findtotal -juttu
			else if(MacroParamSystem().FindTotal(theMenuItem.MacroParamInitName()))
				usedMacroParam = MacroParamSystem().CurrentMacroParam();
	}
	else if(MacroParamSystem().FindTotal(*theMacroParamInitFileName))
			usedMacroParam = MacroParamSystem().CurrentMacroParam();
		if(usedMacroParam != 0 && usedMacroParam->ErrorInMacro() == false) // ei alusteta, jos oli virheellinen macroParami
		{
			theDrawParam->Init(usedMacroParam->DrawParam());
			theDrawParam->DataType(usedMacroParam->DrawParam()->DataType());; // q3macroparam tyyppi pit‰‰ asettaa t‰ss‰
																		// PITƒISIK÷ se asettaa jo DrawParam:in Init-metodissa?!?!?
		}
	}
}

// T‰m‰ on otettu k‰yttˆˆn ,ett‰ voisi unohtaa tuon kamalan indeksi jupinan, mik‰ johtuu
// 'virtuaali' karttan‰yttˆriveist‰.
// Karttarivi indeksit alkavat 1:st‰. 1. rivi on 1 ja 2. rivi on kaksi jne.
// theMacroParamInitFileName on sit‰ varten ett‰ jos viewmacrosta ladataan macroparam, t‰h‰n pit‰‰ antaa init tiedoston nimi
// muuten macroparamin yhteydess‰ etsit‰‰n menuitemista annettua nime‰
// normalParameterAdd -parametrilla kerrotaan tuleeko normaali lis‰ys vai erilaisista viewmakroista lis‰ys. T‰m‰
// haluttiin erottaa viel‰ isViewMacroDrawParam:ista, jolla merkit‰‰n vain drawParamin ViewMacroDrawParam -asetus.
void AddViewWithRealRowNumber(bool normalParameterAdd, const NFmiMenuItem& theMenuItem, int theRealRowIndex, bool isViewMacroDrawParam = false, const std::string *theMacroParamInitFileName = 0)
{
	boost::shared_ptr<NFmiDrawParam> drawParam = itsSmartInfoOrganizer->CreateDrawParam(theMenuItem.DataIdent(), theMenuItem.Level(), theMenuItem.DataType());
	if(!drawParam)
		return; // HUOM!! Ei saisi menn‰ t‰h‰n!!!!!!!

    if(theMenuItem.DataType() == NFmiInfoData::kSatelData && normalParameterAdd) // n‰yttˆ makroissa ei satelImagen alphaa en‰‰ laiteta default 80%:iin
		drawParam->Alpha(80.f); // laitetaan satelliitti/kuva tyyppiselle datalle defaulttina 80% opaque eli pikkuisen l‰pin‰kyv‰

	SetDrawMacroSettings(theMenuItem, drawParam, theMacroParamInitFileName);

	if(!ActiveDrawParamWithRealRowNumber(theMenuItem.MapViewDescTopIndex(), theRealRowIndex))
		drawParam->Activate(true);
    drawParam->ViewMacroDrawParam(isViewMacroDrawParam); // asetetaan viewmacrodrawparam-flagin tila
	boost::shared_ptr<NFmiFastQueryInfo> info = InfoOrganizer()->Info(drawParam, false, true);
    NFmiDrawParamList *drawParamList = DrawParamListWithRealRowNumber(theMenuItem.MapViewDescTopIndex(), theRealRowIndex);
    if(drawParamList)
	{
		string logStr("Added to map view ");
        if(NFmiDrawParam::IsMacroParamCase(drawParam->DataType()))
		{
			logStr += "macro parameter '";
			logStr += drawParam->ParameterAbbreviation();
			logStr += "'";
		}
		else if(info)
			logStr += GetSelectedParamInfoString(info, false);
		else // satelliitti jutuissa ei ole infoa, joten
			logStr += theMenuItem.DataIdent().GetParamName();
		LogMessage(logStr, CatLog::Severity::Debug, CatLog::Category::Visualization);

        if(!normalParameterAdd) // jos n‰yttˆ macrosta kyse, pit‰‰ parametri laittaa tarkalleen siihen mik‰ rivi oli
								// kyseess‰ (eli listan per‰‰n j‰rjestyksess‰). T‰m‰ sen takia ett‰ satel-kanavat heitet‰‰n aina pohjalle ja
								// n‰yttˆmakroissa kaksi satelliitti kuvaa samalla rivill‰ aiheutti ongelmia.
            drawParamList->Add(drawParam, drawParamList->NumberOfItems() + 1);
		else
            drawParamList->Add(drawParam); // laittaa parametrit listan per‰‰n, paitsi satel-kuvat laitetaan keulille (n‰in satelkuva ei peit‰ mahdollisia muita parametreja alleen)
	}

	bool groundData = true;
	if(info)
		groundData = info->SizeLevels() <= 1;
	UpdateModifiedDrawParam(drawParam, groundData);
    DrawParamSettingsChangedDirtyActions(theMenuItem.MapViewDescTopIndex(), theRealRowIndex, drawParam);
}

void AddAsOnlyCrossSectionView(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
	if(CrossSectionViewDrawParamList(theRowIndex))
		CrossSectionViewDrawParamList(theRowIndex)->Clear();
	AddCrossSectionView(theMenuItem, theRowIndex, false);
}

void SetCrossSectionDrawMacroSettings(const NFmiMenuItem& theMenuItem, boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
	if(theMenuItem.DataType() == NFmiInfoData::kCrossSectionMacroParam)
	{
		boost::shared_ptr<NFmiMacroParam> usedMacroParam;
		theDrawParam->ParameterAbbreviation(theMenuItem.MenuText()); // macroParamin tapauksessa pit‰‰ nimi asettaa t‰ss‰ (t‰t‰ nimilyhennett‰ k‰ytet‰‰n tunnisteen‰ myˆhemmin!!)
		boost::shared_ptr<NFmiMacroParamFolder> currentFolder = MacroParamSystem().GetCurrent();
		if(currentFolder && currentFolder->Find(theDrawParam->ParameterAbbreviation()))
			usedMacroParam = currentFolder->Current();
		// kokeillaan viel‰ onko macroparam laitettu popup-menun kautta, jolloin pit‰‰ tehd‰ findtotal -juttu
		else if(MacroParamSystem().FindTotal(theMenuItem.MacroParamInitName()))
			usedMacroParam = MacroParamSystem().CurrentMacroParam();
		if(usedMacroParam != 0 && usedMacroParam->ErrorInMacro() == false) // ei alusteta, jos oli virheellinen macroParami
		{
			theDrawParam->Init(usedMacroParam->DrawParam());
			theDrawParam->DataType(theMenuItem.DataType()); // MARKO t‰m‰ on mielest‰ni toiminut ennen ilman t‰t‰ viilausta, mik‰ on muuttunut
		}
	}
}

void AddCrossSectionView(const NFmiMenuItem& theMenuItem, int theRowIndex, bool fTreatAsViewMacro)
{
	boost::shared_ptr<NFmiDrawParam> drawParam;
	if(theMenuItem.DataType() == NFmiInfoData::kCrossSectionMacroParam)
		drawParam = itsSmartInfoOrganizer->CreateDrawParam(theMenuItem.DataIdent(), 0, theMenuItem.DataType());
	else
		drawParam = itsSmartInfoOrganizer->CreateCrossSectionDrawParam(theMenuItem.DataIdent(), theMenuItem.DataType());
	if(!drawParam)
		return; // HUOM!! Ei saisi menn‰ t‰h‰n!!!!!!!
	SetCrossSectionDrawMacroSettings(theMenuItem, drawParam);
    drawParam->ViewMacroDrawParam(fTreatAsViewMacro);

	boost::shared_ptr<NFmiFastQueryInfo> info = InfoOrganizer()->Info(drawParam, true, true);
	if(CrossSectionViewDrawParamList(theRowIndex))
	{
		string logStr("Adding to crosssection-view ");
		if(info)
			logStr += GetSelectedParamInfoString(info, true);
		else
			logStr += "??????"; // t‰m‰ on virhe tilanne
		LogMessage(logStr, CatLog::Severity::Debug, CatLog::Category::Visualization);
		CrossSectionViewDrawParamList(theRowIndex)->Add(drawParam);
		CrossSectionViewDrawParamList(theRowIndex)->ActivateOnlyOne(); // varmistaa, ett‰ yksi ja vain yksi paramtri listassa on aktiivinen
	}
    DrawParamSettingsChangedDirtyActions(theMenuItem.MapViewDescTopIndex(), GetRealRowNumber(theMenuItem.MapViewDescTopIndex(), theRowIndex), drawParam);
}

// Funktio, joka palauttaa annetusta drawparamista stringin ,joka kertoo:
// Param: id (name) Prod: id (name) Level: id (name, value)
string GetSelectedParamInfoString(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, bool ignoreLevel)
{
	const NFmiDataIdent &dataIdent = theInfo->Param();
	const NFmiLevel *level = 0;
	if(ignoreLevel == false)
	{
		if(theInfo->SizeLevels() > 1)
			if(theInfo->LevelIndex() < theInfo->SizeLevels()) // pit‰‰ pit‰‰ huoli ett‰ level index osoittaa johonkin, t‰ss‰ ei ruveta kuitenkaan s‰‰t‰m‰‰n leveli‰ esim. 1. leveliin
				level = theInfo->Level();
	}
	return GetSelectedParamInfoString(&dataIdent, level);
}

string GetSelectedParamInfoString(const NFmiDataIdent *theDataIdent, const NFmiLevel *theLevel)
{
	string str("Param:");
	str += NFmiStringTools::Convert<unsigned long>(theDataIdent->GetParamIdent());
	str += " ('";
	str += theDataIdent->GetParamName();
	str += "')";

	str += ", Producer: ";
	str += NFmiStringTools::Convert<unsigned long>(theDataIdent->GetProducer()->GetIdent());
	str += " ('";
	str += theDataIdent->GetProducer()->GetName();
	str += "')";

	if(theLevel)
	{
		str += ", Level: ";
		str += NFmiStringTools::Convert<unsigned long>(theLevel->LevelTypeId());
		str += " ('";
		str += theLevel->GetName();
		str += ", ";
		str += NFmiStringTools::Convert<float>(theLevel->LevelValue());
		str += "')";
	}
	str += ".";
	return str;
}

void AddAsOnlyView(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
	if(DrawParamList(theMenuItem.MapViewDescTopIndex(), theRowIndex))
		DrawParamList(theMenuItem.MapViewDescTopIndex(), theRowIndex)->Clear();
	AddView(theMenuItem, theRowIndex);
}

void RemoveView(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
	NFmiDrawParamList* drawParamList = DrawParamList(theMenuItem.MapViewDescTopIndex(), theRowIndex);
	if(!drawParamList)
		return;
	if(drawParamList->Index(theMenuItem.IndexInViewRow()))
	{
		NFmiInfoData::Type dataType = drawParamList->Current()->DataType();
		drawParamList->Remove();
        if(CtrlViewFastInfoFunctions::IsObservationLockModeDataType(dataType))
			CheckAnimationLockedModeTimeBags(theMenuItem.MapViewDescTopIndex(), false); // kun parametrin n‰kyvyytt‰ vaihdetaan, pit‰‰ tehd‰ mahdollisesti animaatio moodin datan tarkistus

        DrawParamSettingsChangedDirtyActions(theMenuItem.MapViewDescTopIndex(), GetRealRowNumber(theMenuItem.MapViewDescTopIndex(), theRowIndex), boost::shared_ptr<NFmiDrawParam>());
    }
	if(drawParamList->NumberOfItems() && (!ActiveDrawParam(theMenuItem.MapViewDescTopIndex(), theRowIndex)))
	{// editoitu parametri poistettiin, asetetaan 1. listasta aktiiviseksi
		for(drawParamList->Reset(); drawParamList->Next(); )
		{
			drawParamList->Current()->Activate(true);
			break;
		}
	}
}

void RemoveAllViews(unsigned int theDescTopIndex, int theRowIndex)
{
    NFmiDrawParamList* drawParamList = DrawParamList(theDescTopIndex, theRowIndex);
    if(drawParamList)
    {
        drawParamList->Clear();
        MakeViewRowDirtyActions(theDescTopIndex, GetRealRowNumber(theDescTopIndex, theRowIndex), drawParamList);
    }
}

void ReloadAllDynamicHelpData()
{
    LogMessage("Reloading all the dynamic data.", CatLog::Severity::Info, CatLog::Category::Data);
    InfoOrganizer()->ClearDynamicHelpData(); // tuhoa kaikki olemassa olevat dynaamiset help-datat (ei edit-data tai sen kopiota ,eik‰ staattisia helpdatoja kuten topografia ja fraktiilit)
	HelpDataInfoSystem()->ResetAllDynamicDataTimeStamps(); // merkit‰‰n kaikkien dynaamisten help datojen aikaleimaksi -1, eli ei ole luettu ollenkaan
    SatelliteImageCacheSystem().ResetImages();
    MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, true, true, false, false, true); // laitetaan kaikki kartta n‰ytˆt likaiseksi
    MacroParamDataCache().clearAllLayers();
	// T‰m‰n j‰lkeen pit‰‰ laittaa datan luku threadi heti p‰‰lle ylemm‰ll‰ tasolla eli CSmartMetDoc-luokassa, mist‰ t‰t‰ metodia on kutsuttukkin.
}

void RemoveAllCrossSectionViews(int theRowIndex)
{
	NFmiDrawParamList* drawParamList = this->CrossSectionViewDrawParamList(theRowIndex);
	if(!drawParamList)
		return;
	drawParamList->Clear();
    MakeViewRowDirtyActions(CtrlViewUtils::kFmiCrossSectionView, theRowIndex, drawParamList);
}

// T‰m‰ on otettu k‰yttˆˆn ,ett‰ voisi unohtaa tuon kamalan indeksi jupinan, mik‰ johtuu
// 'virtuaali' karttan‰yttˆriveist‰.
// Karttarivi indeksit alkavat 1:st‰. 1. rivi on 1 ja 2. rivi on kaksi jne.
void RemoveAllViewsWithRealRowNumber(unsigned int theDescTopIndex, int theRealRowIndex)
{
    NFmiDrawParamList* drawParamList = DrawParamListWithRealRowNumber(theDescTopIndex, theRealRowIndex);
    if(drawParamList)
    {
        drawParamList->Clear();
        MakeViewRowDirtyActions(theDescTopIndex, theRealRowIndex, drawParamList);
    }
}

void ActivateView(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
	NFmiDrawParamList* drawParamList = DrawParamList(theMenuItem.MapViewDescTopIndex(), theRowIndex);

	if(drawParamList == 0)
		return;
	else
	{
		drawParamList->DeactivateAll();
		if(drawParamList->Index(theMenuItem.IndexInViewRow()))
		{
			drawParamList->Current()->Activate(true);
			drawParamList->Dirty(true);
            if(ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(theMenuItem.MapViewDescTopIndex())->ShowStationPlot())
            {
                // Jos karttan‰ytˆll‰ n‰ytet‰‰n aktiivisen datan pisteet, pit‰‰ t‰ss‰ liata kaikki kuva cachet
                auto cacheRowIndex = GetRealRowNumber(theMenuItem.MapViewDescTopIndex(), theRowIndex) - 1;
                MapViewDescTop(theMenuItem.MapViewDescTopIndex())->MapViewCache().MakeRowDirty(cacheRowIndex);
            }
            MapViewDirty(theMenuItem.MapViewDescTopIndex(), false, false, true, false, false, false);
		}
	}
}

void ActivateCrossSectionParam(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
	NFmiDrawParamList* drawParamList = CrossSectionViewDrawParamList(theRowIndex);

	if(drawParamList == 0)
		return;
	else
	{
		drawParamList->DeactivateAll();
		if(drawParamList->Index(theMenuItem.IndexInViewRow()))
		{
			drawParamList->Current()->Activate(true);
			drawParamList->Dirty(true);
		}
	}
}


NFmiInfoData::Type GetFinalDataType(NFmiInfoData::Type theDataType, boost::shared_ptr<NFmiFastQueryInfo> &theData, const NFmiProducer &theGivenProducer, const NFmiLevel &theLevel)
{
	NFmiInfoData::Type finalDataType = theDataType;
	if(theData)
	{
		if(theGivenProducer != *(theData->Producer()))
		{
			if(theLevel.LevelType() == kFmiHybridLevel)
				finalDataType = NFmiInfoData::kHybridData;
			else
				finalDataType = NFmiInfoData::kViewable;
		}
	}
	return finalDataType;
}

NFmiInfoData::Type GetUsableCrossSectionDataType(NFmiInfoData::Type theDataType, NFmiInfoData::Type theAlternateType, const NFmiProducer &theGivenProducer)
{
	NFmiInfoData::Type finalDataType = theDataType;
	boost::shared_ptr<NFmiFastQueryInfo> info = InfoOrganizer()->FindInfo(theDataType, theGivenProducer, false);
	if(info == 0)
	{
		info = InfoOrganizer()->FindInfo(theAlternateType, theGivenProducer, false);
		if(info)
			finalDataType = theAlternateType;
	}
	return finalDataType;
}

// jos on katsottu hirlma mallipinta dataa poikkileikkaus ikkunassa ja tuottaja vaihdetaan GFS:ksi,
// t‰llˆin ei tule mit‰‰n n‰kyviin, koska GFS:ll‰ ei ole hybridi dataa. Ja jos on katsottu painepintadataa
// ja vaihdetaan Arome tuottajaan, t‰llˆin ei tule mit‰‰n n‰kyviin, koska ei ole kuin hybridi dataa.
// T‰t‰ varten t‰m‰ funktio tekee viel‰ viimeiset tarkastelut, lˆytyykˆ dataa ja s‰‰t‰‰ datatyyppi‰ tarvittaessa.
NFmiInfoData::Type CheckCrossSectionLevelData(NFmiInfoData::Type theDataType, const NFmiProducer &theGivenProducer)
{
	NFmiInfoData::Type finalDataType = theDataType;
	if(theDataType == NFmiInfoData::kViewable)
		finalDataType = GetUsableCrossSectionDataType(NFmiInfoData::kViewable, NFmiInfoData::kHybridData, theGivenProducer);
	else if(theDataType == NFmiInfoData::kHybridData)
		finalDataType = GetUsableCrossSectionDataType(NFmiInfoData::kHybridData, NFmiInfoData::kViewable, theGivenProducer);
	return finalDataType;
}

NFmiInfoData::Type GetFinalDataType(boost::shared_ptr<NFmiDrawParam> &theDrawParam, const NFmiProducer &theGivenProducer, bool fUseCrossSectionParams, bool fGroundData)
{
	boost::shared_ptr<NFmiFastQueryInfo> editedData = EditedInfo();
	boost::shared_ptr<NFmiFastQueryInfo> operationalData = InfoOrganizer()->FindInfo(NFmiInfoData::kKepaData);
	boost::shared_ptr<NFmiFastQueryInfo> helpData = InfoOrganizer()->FindInfo(NFmiInfoData::kEditingHelpData, 0);
	NFmiInfoData::Type finalDataType = theDrawParam->DataType();
	if(finalDataType == NFmiInfoData::kEditable)
		finalDataType = GetFinalDataType(finalDataType, editedData, theGivenProducer, theDrawParam->Level());
	else if(finalDataType == NFmiInfoData::kKepaData)
		finalDataType = GetFinalDataType(finalDataType, operationalData, theGivenProducer, theDrawParam->Level());
	else if(finalDataType == NFmiInfoData::kEditingHelpData)
		finalDataType = GetFinalDataType(finalDataType, helpData, theGivenProducer, theDrawParam->Level());

    boost::shared_ptr<NFmiFastQueryInfo> foundData = InfoOrganizer()->FindInfo(finalDataType, theGivenProducer, fGroundData);
    if(!foundData)
    { // Vain jos ei lˆydy suoraan haluttua dataa, etsit‰‰n n‰ist‰ erikois datoista. T‰m‰ siksi ett‰ muuten editoitavaksi dataksi luettu pintadata sotkee kuviot. K‰ytt‰j‰ haluaa muuttaa EC:n pinta l‰mpˆtilaksi, eik‰ esim. editoidun datan pintal‰mpˆtilaksi, joka sattui olemaan Ec:t‰.
	    if(editedData && theGivenProducer == *(editedData->Producer()))
		    finalDataType = NFmiInfoData::kEditable;
	    else if(operationalData && theGivenProducer == *(operationalData->Producer()))
		    finalDataType = NFmiInfoData::kKepaData;
	    else if(helpData && theGivenProducer == *(helpData->Producer()))
		    finalDataType = NFmiInfoData::kEditingHelpData;
    }

	if(fUseCrossSectionParams)
		finalDataType = CheckCrossSectionLevelData(finalDataType, theGivenProducer);

	return finalDataType;
}

void ChangeAllProducersInMapRow(const NFmiMenuItem& theMenuItem, int theRowIndex, bool fUseCrossSectionParams)
{
	NFmiDrawParamList* drawParamList = 0;
	if(fUseCrossSectionParams)
		drawParamList = CrossSectionViewDrawParamList(theRowIndex);
	else
		drawParamList = DrawParamList(theMenuItem.MapViewDescTopIndex(), theRowIndex);
	if(drawParamList)
	{
		const NFmiProducer &givenProducer = *(theMenuItem.DataIdent().GetProducer());
		for(drawParamList->Reset(); drawParamList->Next(); )
		{
            auto drawParam = drawParamList->Current();
            auto dataType = drawParam->DataType();
			if(dataType != NFmiInfoData::kSatelData && dataType != NFmiInfoData::kMacroParam && dataType != NFmiInfoData::kQ3MacroParam)
			{
				bool groundData = (drawParam->Level().GetIdent() == 0);
				NFmiInfoData::Type finalDataType = GetFinalDataType(drawParam, givenProducer, fUseCrossSectionParams, groundData); // pit‰‰ p‰‰tt‰‰ viel‰ muutentun tuottajan datatyyppi
				// pit‰‰ hakea FindInfo:lla tuottajan mukaan dataa, josta saadaan oikea tuottaja (nimineen kaikkineen)
				boost::shared_ptr<NFmiFastQueryInfo> info = InfoOrganizer()->FindInfo(finalDataType, givenProducer, groundData);
				if(info)
					drawParam->Param().SetProducers(*info->Producer()); // pit‰‰ laittaa tuottaja datasta, koska tuottajan nimikin ratkaisee, kun haetaan dataa
				else // jos ei lˆytynyt dataa, t‰m‰ luultavasti menee pieleen, mutta laitetaan kuitenkin tuottaja kohdalleen
					drawParam->Param().SetProducers(*(theMenuItem.DataIdent().GetProducer()));
				drawParam->DataType(finalDataType);
			}
		}
		drawParamList->Dirty(true);
        auto cacheRowIndex = GetRealRowNumber(theMenuItem.MapViewDescTopIndex(), theRowIndex);
        MakeViewRowDirtyActions(theMenuItem.MapViewDescTopIndex(), theRowIndex, drawParamList);
	}
}

void ChangeAllDataTypesInMapRow(const NFmiMenuItem& theMenuItem, int theRowIndex, bool fUseCrossSectionParams)
{
	NFmiDrawParamList* drawParamList = 0;
	if(fUseCrossSectionParams)
		drawParamList = CrossSectionViewDrawParamList(theRowIndex);
	else
		drawParamList = DrawParamList(theMenuItem.MapViewDescTopIndex(), theRowIndex);
	if(drawParamList)
	{
		NFmiInfoData::Type wantedType = static_cast<NFmiInfoData::Type>(theMenuItem.Parameter());
		for(drawParamList->Reset(); drawParamList->Next(); )
		{
			drawParamList->Current()->DataType(wantedType);
		}
		drawParamList->Dirty(true);
        MakeViewRowDirtyActions(theMenuItem.MapViewDescTopIndex(), theRowIndex, drawParamList);
	}
}

void CopyDrawParamOptions(const NFmiMenuItem& theMenuItem, int theRowIndex, bool fUseCrossSectionParams)
{
	NFmiDrawParamList* drawParamList = 0;
	if(fUseCrossSectionParams)
		drawParamList = CrossSectionViewDrawParamList(theRowIndex);
	else
		drawParamList = DrawParamList(theMenuItem.MapViewDescTopIndex(), theRowIndex);

	if(!drawParamList)
		return;
	if(drawParamList->Index(theMenuItem.IndexInViewRow()))
	{
		itsCopyPasteDrawParam.Init(drawParamList->Current(), true);
		fCopyPasteDrawParamAvailableYet = true;
	}
}

void PasteDrawParamOptions(const NFmiMenuItem& theMenuItem, int theRowIndex, bool fUseCrossSectionParams)
{
	NFmiDrawParamList* drawParamList = 0;
	if(fUseCrossSectionParams)
		drawParamList = CrossSectionViewDrawParamList(theRowIndex);
	else
		drawParamList = DrawParamList(theMenuItem.MapViewDescTopIndex(), theRowIndex);

	if(!drawParamList)
		return;
	if(drawParamList->Index(theMenuItem.IndexInViewRow()))
	{
		boost::shared_ptr<NFmiDrawParam> drawParam = drawParamList->Current();
		drawParam->Init(&itsCopyPasteDrawParam, true);
		drawParamList->Dirty(true);
		if(fUseCrossSectionParams == false)
			UpdateModifiedDrawParamMarko(theMenuItem.MapViewDescTopIndex(), drawParam, theRowIndex);
		if(NFmiDrawParam::IsMacroParamCase(theMenuItem.DataType()))
		{ // macroParam pit‰‰ viel‰ p‰ivitt‰‰ macroParamSystemiin!!
			string macroParamName = theMenuItem.DataIdent().GetParamName();
			if(itsMacroParamSystem.FindTotal(macroParamName)) // t‰ss‰ tod. init fileName
				itsMacroParamSystem.CurrentMacroParam()->DrawParam()->Init(&itsCopyPasteDrawParam, true);
		}
	}
}

boost::shared_ptr<NFmiDrawParam> GetUsedMapViewDrawParam(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
	NFmiDrawParamList* drawParamList = DrawParamList(theMenuItem.MapViewDescTopIndex(), theRowIndex);
	if(drawParamList && drawParamList->Index(theMenuItem.IndexInViewRow()))
		return drawParamList->Current();
	else
		return boost::shared_ptr<NFmiDrawParam>();
}

void ModifyView(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
	bool macroParamCase = NFmiDrawParam::IsMacroParamCase(theMenuItem.DataType());
	boost::shared_ptr<NFmiDrawParam> drawParam = macroParamCase ? GetUsedMacroDrawParam(theMenuItem) : GetUsedMapViewDrawParam(theMenuItem, theRowIndex);
	if(drawParam)
	{
		NFmiMetEditorTypes::View viewType = theMenuItem.ViewType();
		drawParam->ViewType(viewType);
		boost::shared_ptr<NFmiFastQueryInfo> info = itsSmartInfoOrganizer->Info(drawParam, false, true);
		if(info && info->IsGrid() == false)
			drawParam->StationDataViewType(viewType);
		else
			drawParam->GridDataPresentationStyle(viewType);
		NFmiDrawParamList* drawParamList = DrawParamList(theMenuItem.MapViewDescTopIndex(), theRowIndex);
        if(drawParamList)
        {
            drawParamList->Dirty(true);
            if(macroParamCase)
                UpdateMacroDrawParam(theMenuItem, theRowIndex, false, drawParam);
            else
                UpdateModifiedDrawParamMarko(theMenuItem.MapViewDescTopIndex(), drawParam, theRowIndex);

            DrawParamSettingsChangedDirtyActions(theMenuItem.MapViewDescTopIndex(), GetRealRowNumber(theMenuItem.MapViewDescTopIndex(), theRowIndex), drawParam);
        }
	}
}

void SaveDrawParamSettings(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
    bool macroParamCase = NFmiDrawParam::IsMacroParamCase(theMenuItem.DataType());
    boost::shared_ptr<NFmiDrawParam> drawParam = macroParamCase ? GetUsedMacroDrawParam(theMenuItem) : GetUsedMapViewDrawParam(theMenuItem, theRowIndex);
    if(drawParam)
    {
        if(drawParam->ViewMacroDrawParam())
        {
            std::string msgStr = ::GetDictionaryString("Cannot store drawParam");
            std::string dialogTitleStr = ::GetDictionaryString("DrawParam was in viewmacro, you must save the changes made to viewMacro");
            LogAndWarnUser(msgStr, dialogTitleStr, CatLog::Severity::Error, CatLog::Category::Macro, false);
        }
        else if(!drawParam->StoreData(drawParam->InitFileName()))
        {
            std::string msgStr = ::GetDictionaryString("Error storing drawParam");
            std::string dialogTitleStr = ::GetDictionaryString("Unknown error while trying to store drawParam settings");
            LogAndWarnUser(msgStr, dialogTitleStr, CatLog::Severity::Error, CatLog::Category::Macro, false);
        }
    }
}

void ForceStationViewRowUpdate(unsigned int theDescTopIndex, unsigned int theRealRowIndex)
{
    NFmiDrawParamList *drawParamList = DrawParamListWithRealRowNumber(theDescTopIndex, theRealRowIndex);
    if(drawParamList)
    {
        // Liataan haluttu drawParamList, jotta k‰ytˆss‰ karttan‰ytˆss‰ ollut stationView p‰ivittyy oikein tarvittaessa jos esim. isoline piirto vaihtuu teksti esitykseen.
        drawParamList->Dirty(true);
    }
}

void ReloadDrawParamSettings(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
    bool macroParamCase = NFmiDrawParam::IsMacroParamCase(theMenuItem.DataType());
    boost::shared_ptr<NFmiDrawParam> drawParam = macroParamCase ? GetUsedMacroDrawParam(theMenuItem) : GetUsedMapViewDrawParam(theMenuItem, theRowIndex);
    if(drawParam)
    {
        // Pit‰‰ ladata erikseen originaali drawParam asetukset omaan olioon ja sen avulla initialisoida k‰ytˆss‰ olevan asetukset
        NFmiDrawParam origDrawParam;
        origDrawParam.Init(drawParam->InitFileName());
        drawParam->Init(&origDrawParam, true);
        DrawParamSettingsChangedDirtyActions(theMenuItem.MapViewDescTopIndex(), GetRealRowNumber(theMenuItem.MapViewDescTopIndex(), theRowIndex), drawParam);
    }
}

void MakeViewRowDirtyActions(unsigned int theDescTopIndex, int theRealRowIndex, NFmiDrawParamList *drawParamList)
{
    auto bitmapCacheRowIndex = theRealRowIndex - 1;
    try
    {
        MapViewDescTop(theDescTopIndex)->MapViewCache().MakeRowDirty(bitmapCacheRowIndex);
    }
    catch(...)
    {
    } // Jos jokin muu kuin karttan‰yttˆ, MapViewDescTop(theDescTopIndex) -kutsu heitt‰‰ poikkeuksen ja se on ok t‰ss‰

    if(drawParamList)
        MacroParamDataCache().update(theDescTopIndex, theRealRowIndex, *drawParamList);
    MapViewDirty(theDescTopIndex, false, false, true, false, false, true);
}

void DrawParamSettingsChangedDirtyActions(unsigned int theDescTopIndex, int theRealMapRow, boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
    auto bitmapCacheRowIndex = theRealMapRow - 1;
    try
    {
        MapViewDescTop(theDescTopIndex)->MapViewCache().MakeRowDirty(bitmapCacheRowIndex);
    }
    catch(...)
    {    } // Jos jokin muu kuin karttan‰yttˆ, MapViewDescTop(theDescTopIndex) -kutsu heitt‰‰ poikkeuksen ja se on ok t‰ss‰

    if(theDrawParam)
    {
        if(theDrawParam->IsMacroParamCase(true))
            MacroParamDataCache().clearMacroParamCache(theDescTopIndex, theRealMapRow, theDrawParam->InitFileName());
    }
    MapViewDirty(theDescTopIndex, false, false, true, false, false, true);
}

// Liataan vain 1. n‰kyv‰t karttarivit niist‰ karttan‰ytˆist‰, miss‰ n‰yt‰-maski on p‰‰ll‰
void MaskChangedDirtyActions()
{
    for(unsigned int desctopIndex = 0; desctopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex; desctopIndex++)
    {
        auto desctop = MapViewDescTop(desctopIndex);
        if(desctop)
        {
            if(ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(desctopIndex)->ShowMasksOnMap())
            {
                // Maskit ovat siis n‰kyviss‰ 1. relatiivisella rivill‰
                unsigned int firstVisibleRowIndex = 1;
                auto cleanedCacheRowIndex = GetRealRowNumber(desctopIndex, firstVisibleRowIndex) - 1;
                desctop->MapViewCache().MakeRowDirty(cleanedCacheRowIndex);
                MapViewDirty(desctopIndex, false, false, true, false, false, false);
            }
        }
    }
}

bool ChangeParamSettingsToNextFixedDrawParam(unsigned int theDescTopIndex, int theRealMapRow, int theParamIndex, bool fNext, bool /* fUseCrossSectionParams */ )
{
    NFmiDrawParamList *list = DrawParamListWithRealRowNumber(theDescTopIndex, theRealMapRow);
    if(list)
    {
        if(list->Index(theParamIndex))
        {
            boost::shared_ptr<NFmiDrawParam> drawParam = list->Current();
            if(drawParam)
            {
                NFmiMenuItem menuItem(theDescTopIndex, "", drawParam->Param(), kFmiModifyView, NFmiMetEditorTypes::kFmiTextView, &drawParam->Level(), drawParam->DataType(), theParamIndex, drawParam->ViewMacroDrawParam());
                if(fNext)
                    itsFixedDrawParamSystem.Next();
                else
                    itsFixedDrawParamSystem.Previous();
                const std::shared_ptr<NFmiDrawParam> &fixedDrawParam = itsFixedDrawParamSystem.GetCurrentDrawParam();
                ApplyFixeDrawParam(menuItem, GetRelativeRowNumber(theDescTopIndex, theRealMapRow), fixedDrawParam);
                DrawParamSettingsChangedDirtyActions(theDescTopIndex, theRealMapRow, drawParam);
                return true;
            }
        }
    }
    return false;
}

void ApplyFixeDrawParam(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
    const std::shared_ptr<NFmiDrawParam> &fixedDrawParam = itsFixedDrawParamSystem.GetDrawParam(theMenuItem.MacroParamInitName());
    ApplyFixeDrawParam(theMenuItem, theRowIndex, fixedDrawParam);
}

void ApplyFixeDrawParam(const NFmiMenuItem& theMenuItem, int theRowIndex, const std::shared_ptr<NFmiDrawParam> &theFixedDrawParam)
{
    bool macroParamCase = NFmiDrawParam::IsMacroParamCase(theMenuItem.DataType());
    boost::shared_ptr<NFmiDrawParam> drawParam = macroParamCase ? GetUsedMacroDrawParam(theMenuItem) : GetUsedMapViewDrawParam(theMenuItem, theRowIndex);
    if(drawParam && theFixedDrawParam)
    {
        drawParam->Init(theFixedDrawParam.get(), true);

        NFmiDrawParamList* drawParamList = DrawParamList(theMenuItem.MapViewDescTopIndex(), theRowIndex);
        if(drawParamList)
            drawParamList->Dirty(true);

        if(macroParamCase)
            UpdateMacroDrawParam(theMenuItem, theRowIndex, false, drawParam);
        else
            UpdateModifiedDrawParamMarko(theMenuItem.MapViewDescTopIndex(), drawParam, theRowIndex);
    }
}

boost::shared_ptr<NFmiDrawParam> GetDrawParamFromViewLists(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
	NFmiDrawParamList* drawParamList = DrawParamList(theMenuItem.MapViewDescTopIndex(), theRowIndex);
	if(!drawParamList)
		return boost::shared_ptr<NFmiDrawParam>();
	if(drawParamList->Index(theMenuItem.IndexInViewRow()))
		return drawParamList->Current();
	return boost::shared_ptr<NFmiDrawParam>();
}

boost::shared_ptr<NFmiDrawParam> GetCrosssectionDrawParamFromViewLists(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
	NFmiDrawParamList* drawParamList = CrossSectionViewDrawParamList(theRowIndex);
	if(!drawParamList)
		return boost::shared_ptr<NFmiDrawParam>();
	if(drawParamList->Index(theMenuItem.IndexInViewRow()))
		return drawParamList->Current();
	return boost::shared_ptr<NFmiDrawParam>();
}

void RemoveCrosssectionDrawParam(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
	NFmiDrawParamList* drawParamList = this->CrossSectionViewDrawParamList(theRowIndex);
    if(drawParamList && drawParamList->Index(theMenuItem.IndexInViewRow()))
    {
        drawParamList->Remove();
        DrawParamSettingsChangedDirtyActions(theMenuItem.MapViewDescTopIndex(), GetRealRowNumber(theMenuItem.MapViewDescTopIndex(), theRowIndex), boost::shared_ptr<NFmiDrawParam>());
    }
}

NFmiAreaMask* CreateMask(const NFmiMenuItem& theMenuItem)
{
	NFmiAreaMask* mask = 0;
	if(theMenuItem.DataType() == NFmiInfoData::kCalculatedValue)
	{
		switch(theMenuItem.Parameter())
		{
		case kFmiLatitude:
			{
			NFmiCalculationCondition calCond(kFmiMaskGreaterThan, 0.,1.);
			mask = new NFmiLatLonAreaMask(NFmiDataIdent(NFmiParam(kFmiLatitude, "Lat")), calCond);
			break;
			}
		case kFmiLongitude:
			{
			NFmiCalculationCondition calCond(kFmiMaskGreaterThan, 0.,1.);
			mask = new NFmiLatLonAreaMask(NFmiDataIdent(NFmiParam(kFmiLongitude, "Lon")), calCond);
			break;
			}
		case kFmiElevationAngle:
			{
			NFmiCalculationCondition calCond(kFmiMaskGreaterThan, 0.,1.);
			mask = new NFmiElevationAngleAreaMask(NFmiDataIdent(NFmiParam(kFmiElevationAngle, "ElevAngle")), calCond);
			break;
			}
		}
	}
	else
	{
		boost::shared_ptr<NFmiFastQueryInfo> info = itsSmartInfoOrganizer->Info(theMenuItem.DataIdent(), theMenuItem.Level(), theMenuItem.DataType());
		if(info)
		{
			// T‰ss‰ tehd‰‰ dynaaminen pinta kopio infosta, jotta ainakin NFmiOwnerInfo-osio tulee mukaan.
			boost::shared_ptr<NFmiFastQueryInfo> infoCopy = NFmiInfoOrganizer::DoDynamicShallowCopy(info);
			if(infoCopy)
			{
				infoCopy->Param(static_cast<FmiParameterName>(theMenuItem.DataIdent().GetParamIdent()));
				if(theMenuItem.Level())
					infoCopy->Level(*(theMenuItem.Level()));
				if(infoCopy->Param().Type() != kContinuousParam)
				{
					NFmiCalculationCondition calCond(kFmiMaskEqual, 0.,0.);
					mask = new NFmiInfoAreaMask(calCond, NFmiAreaMask::kInfo, infoCopy->DataType(), infoCopy, NFmiAreaMask::kNoValue);
				}
				else
				{
					NFmiCalculationCondition calCond(kFmiMaskGreaterThan, 0.,1.);
					mask = new NFmiInfoAreaMask(calCond, NFmiAreaMask::kInfo, infoCopy->DataType(), infoCopy, NFmiAreaMask::kNoValue);
				}
			}
		}
	}
	return mask;
}

void AddMask(const NFmiMenuItem& theMenuItem, int /* theRowIndex */ , bool fClearListFirst)
{
	TimeSerialViewDirty(true);
	boost::shared_ptr<NFmiAreaMask> mask(CreateMask(theMenuItem));
	if(mask)
	{
		CFmiMaskOperationDlg dlg;
		dlg.ParamMask(mask.get());
		if(dlg.DoModal() == IDOK)
		{
			boost::shared_ptr<NFmiAreaMaskList> paramMaskList = ParamMaskListMT();
			if(paramMaskList)
			{
				string logStr("Adding mask ");
				logStr += GetSelectedParamInfoString(mask->DataIdent(), mask->Level());
				LogMessage(logStr, CatLog::Severity::Debug, CatLog::Category::Editing);

				if(fClearListFirst)
					paramMaskList->Clear();
				paramMaskList->Add(mask);
				ParamMaskListMT(paramMaskList); // maskilistaan tuli lis‰ys/poisto, t‰llˆin lista pit‰‰ asettaa dokumenttiin k‰yttˆˆn, muutokset yksitt‰isiin maskeihin menev‰t automaattisesti k‰yttˆˆn
			}
		}
	}
}

void AddAsOnlyMask(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
	AddMask(theMenuItem, theRowIndex, true);
}

void RemoveAllMasks(int /* theRowIndex */ )
{
	TimeSerialViewDirty(true);
	boost::shared_ptr<NFmiAreaMaskList> paramMaskList = ParamMaskListMT();
	if(paramMaskList)
	{
		paramMaskList->Clear();
		ParamMaskListMT(paramMaskList); // maskilistaan tuli lis‰ys/poisto, t‰llˆin lista pit‰‰ asettaa dokumenttiin k‰yttˆˆn, muutokset yksitt‰isiin maskeihin menev‰t automaattisesti k‰yttˆˆn
	}
}

void ActivateFirstNonHiddenViewParam(NFmiDrawParamList* theDrawParamList)
{
	for(theDrawParamList->Reset(); theDrawParamList->Next(); )
	{
		if(!theDrawParamList->Current()->IsParamHidden())
		{
			theDrawParamList->Current()->Activate(true);
			break;
		}
	}
}

void HideView(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
    auto mapViewDesctopIndex = theMenuItem.MapViewDescTopIndex();
	NFmiDrawParamList* drawParamList = DrawParamList(mapViewDesctopIndex, theRowIndex);
	if(!drawParamList)
		return;
	if(drawParamList->Index(theMenuItem.IndexInViewRow()))
	{
		drawParamList->Current()->HideParam(true);
		if(drawParamList->Current()->IsActive())
		{// deaktivoidaan piilotettu n‰yttˆparametri (jos oli aktiivinen), ettei pensselill‰ yritet‰ sutia sit‰ vahingossa
			drawParamList->Current()->Activate(false);
			ActivateFirstNonHiddenViewParam(drawParamList);
		}
		drawParamList->Dirty(true);
		CheckAnimationLockedModeTimeBags(mapViewDesctopIndex, false); // kun parametrin n‰kyvyytt‰ vaihdetaan, pit‰‰ tehd‰ mahdollisesti animaatio moodin datan tarkistus
        MapViewDescTop(mapViewDesctopIndex)->MapViewCache().MakeRowDirty(GetRealRowNumber(mapViewDesctopIndex, theRowIndex));
		MapViewDirty(mapViewDesctopIndex, false, false, true, false, false, false);
	}
	UpdateModifiedDrawParamMarko(mapViewDesctopIndex, drawParamList->Current(), theRowIndex);
}

void ShowView(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
    auto mapViewDesctopIndex = theMenuItem.MapViewDescTopIndex();
    NFmiDrawParamList* drawParamList = DrawParamList(mapViewDesctopIndex, theRowIndex);
	if(!drawParamList)
		return;
	if(drawParamList->Index(theMenuItem.IndexInViewRow()))
	{
		drawParamList->Current()->HideParam(false);
		drawParamList->Dirty(true);
		CheckAnimationLockedModeTimeBags(mapViewDesctopIndex, false); // kun parametrin n‰kyvyytt‰ vaihdetaan, pit‰‰ tehd‰ mahdollisesti animaatio moodin datan tarkistus
        MapViewDescTop(mapViewDesctopIndex)->MapViewCache().MakeRowDirty(GetRealRowNumber(mapViewDesctopIndex, theRowIndex));
        MapViewDirty(mapViewDesctopIndex, false, false, true, false, false, false);
	}
	UpdateModifiedDrawParamMarko(mapViewDesctopIndex, drawParamList->Current(), theRowIndex);
}

void RemoveMask(const NFmiMenuItem& theMenuItem)
{
	TimeSerialViewDirty(true);
	boost::shared_ptr<NFmiAreaMaskList> paramMaskList = ParamMaskListMT();
	if(paramMaskList)
	{
		if(paramMaskList->Index(theMenuItem.IndexInViewRow()))
		{
			paramMaskList->Remove();
			ParamMaskListMT(paramMaskList); // maskilistaan tuli lis‰ys/poisto, t‰llˆin lista pit‰‰ asettaa dokumenttiin k‰yttˆˆn, muutokset yksitt‰isiin maskeihin menev‰t automaattisesti k‰yttˆˆn
		}
	}
}

void DisableMask(const NFmiMenuItem& theMenuItem)
{
	TimeSerialViewDirty(true);
	boost::shared_ptr<NFmiAreaMaskList> paramMaskList = ParamMaskListMT();
	if(paramMaskList)
	{
		if(paramMaskList->Index(theMenuItem.IndexInViewRow()))
		{
			paramMaskList->Current()->Enable(false);
            ParamMaskListMT(paramMaskList); // maskilistaan tuli muutos, t‰llˆin lista pit‰‰ asettaa dokumenttiin k‰yttˆˆn, jolloin myˆs n‰yttˆjen likaukset tehd‰‰n oikein
        }
	}
}

void EnableMask(const NFmiMenuItem& theMenuItem)
{
	TimeSerialViewDirty(true);
	boost::shared_ptr<NFmiAreaMaskList> paramMaskList = ParamMaskListMT();
	if(paramMaskList)
	{
		if(paramMaskList->Index(theMenuItem.IndexInViewRow()))
		{
			paramMaskList->Current()->Enable(true);
            ParamMaskListMT(paramMaskList); // maskilistaan tuli muutos, t‰llˆin lista pit‰‰ asettaa dokumenttiin k‰yttˆˆn, jolloin myˆs n‰yttˆjen likaukset tehd‰‰n oikein
        }
	}
}

void ModifyMask(const NFmiMenuItem& theMenuItem)
{
	TimeSerialViewDirty(true);
	boost::shared_ptr<NFmiAreaMaskList> paramMaskList = ParamMaskListMT();
	if(paramMaskList && paramMaskList->Index(theMenuItem.IndexInViewRow()))
	{
		boost::shared_ptr<NFmiAreaMask> mask = paramMaskList->Current();
		if(mask)
		{
			CFmiMaskOperationDlg dlg;
			dlg.ParamMask(mask.get());
			if(dlg.DoModal() == IDOK)
			{
                ParamMaskListMT(paramMaskList); // maskilistaan tuli muutos, t‰llˆin lista pit‰‰ asettaa dokumenttiin k‰yttˆˆn, jolloin myˆs n‰yttˆjen likaukset tehd‰‰n oikein
            }
		}
	}
}

void ModifyCrossSectionDrawParam(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
	// TƒHƒN aluksi pika viritys macroParam asetukselle
	if(theMenuItem.DataType() == NFmiInfoData::kCrossSectionMacroParam)
		ModifyMacroDrawParam(theMenuItem, theRowIndex, true);
	else
	{
		boost::shared_ptr<NFmiDrawParam> modifiedDrawParam = GetCrosssectionDrawParamFromViewLists(theMenuItem, theRowIndex);
		if(modifiedDrawParam)
		{
            CWnd *parentView = ApplicationInterface::GetApplicationInterfaceImplementation()->GetView(theMenuItem.MapViewDescTopIndex());
            CFmiModifyDrawParamDlg dlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), modifiedDrawParam, itsMacroPathSettings.DrawParamPath(true), false, true, theMenuItem.MapViewDescTopIndex(), parentView);
			if(dlg.DoModal() == IDOK)
			{
                DrawParamSettingsChangedDirtyActions(theMenuItem.MapViewDescTopIndex(), GetRealRowNumber(theMenuItem.MapViewDescTopIndex(), theRowIndex), modifiedDrawParam);
            }
		}
	}
}

void SetCrossSectionTrajectoryParams()
{
	// ei aseteta vain halutun rivin parametreja, vaan kaikkien rivien kaikki parametrit
	// vastaamaan kulloisenkin rivin osoittamaa trajektoria

	int ssize = itsCrossSectionDrawParamListVector->NumberOfItems();
	for(int i = 1; i <= ssize; i++)
	{
		NFmiDrawParamList *aList = CrossSectionViewDrawParamList(i); // HUOM! t‰‰ll‰ indeksointi alkaa 1:st‰
		if(aList)
		{
			const NFmiTrajectory &trajectory = this->TrajectorySystem()->Trajectory(i-1); // HUOM! t‰ss‰ -1, koska indeksointi alkaa 0:sta
			for(aList->Reset(); aList->Next(); )
			{
				boost::shared_ptr<NFmiDrawParam> aDParam = aList->Current();
				aDParam->Param().SetProducer(trajectory.Producer());
				if(trajectory.DataType() == 2)
				{
					aDParam->DataType(NFmiInfoData::kHybridData);
					aDParam->Level().SetIdent(kFmiHybridLevel);
				}
				else if(trajectory.DataType() == 1)
				{
					aDParam->DataType(NFmiInfoData::kViewable);
					aDParam->Level().SetIdent(kFmiPressureLevel);
				}
				else
				{ // pinta dataa, joka ei n‰y kuitenkaan poikkileikkaus n‰ytˆss‰
					aDParam->DataType(NFmiInfoData::kHybridData);
					aDParam->Level().SetIdent(kFmiHeight);
				}
			}
		}
	}
}

void SetCrossSectionTrajectoryTimes(int theRowIndex)
{
	if(TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
	{
		const NFmiTrajectory &trajectory = TrajectorySystem()->Trajectory(theRowIndex-1);
		const checkedVector<NFmiMetTime> &times = trajectory.CrossSectionTrajectoryTimes();
		if(times.size())
		{
			NFmiTimeBag timeBag(times[0], times[times.size()-1], CrossSectionSystem()->CrossSectionTimeControlTimeBag().Resolution());
			CrossSectionSystem()->CrossSectionTimeControlTimeBag(timeBag);
		}
	}
	else if(CrossSectionSystem()->GetCrossMode() == NFmiCrossSectionSystem::kTime)
	{ // muutin systeemin asettamaan ajat myˆs aika-moodissa. Ottaa ensimm‰isen drawParamia
	  // vastaavan infon ja s‰‰t‰‰ alku ja loppu ajat sen mukaan
		NFmiDrawParamList *aList = CrossSectionViewDrawParamList(theRowIndex);
		if(aList)
		{
			if(aList->Index(1)) // haetaan 1. drawparami
			{
				boost::shared_ptr<NFmiDrawParam> dParam = aList->Current();
				boost::shared_ptr<NFmiFastQueryInfo> info = InfoOrganizer()->Info(dParam, true, false);
				if(info)
				{
					NFmiTimeBag times(info->TimeDescriptor().FirstTime(), info->TimeDescriptor().LastTime(), CrossSectionSystem()->CrossSectionTimeControlTimeBag().Resolution());
					CrossSectionSystem()->CrossSectionTimeControlTimeBag(times);
				}
			}
		}
	}
}

void ShowCrossSectionDrawParam(const NFmiMenuItem& theMenuItem, int theRowIndex, bool fShowParam)
{
	boost::shared_ptr<NFmiDrawParam> modifiedDrawParam = GetCrosssectionDrawParamFromViewLists(theMenuItem, theRowIndex);
    if(modifiedDrawParam)
    {
		modifiedDrawParam->HideParam(!fShowParam);
        DrawParamSettingsChangedDirtyActions(theMenuItem.MapViewDescTopIndex(), GetRealRowNumber(theMenuItem.MapViewDescTopIndex(), theRowIndex), modifiedDrawParam);
    }
}

bool ModifyDrawParam(const NFmiMenuItem& theMenuItem, int theRowIndex)
{
	// TƒHƒN aluksi pika viritys macroParam asetukselle
	if(NFmiDrawParam::IsMacroParamCase(theMenuItem.DataType()) && theMenuItem.ViewMacroDrawParam() == false)
		return ModifyMacroDrawParam(theMenuItem, theRowIndex, theMenuItem.MapViewDescTopIndex() == CtrlViewUtils::kFmiCrossSectionView);
	else
	{
        bool updateStatus = false;
		boost::shared_ptr<NFmiDrawParam> modifiedDrawParam = GetDrawParamFromViewLists(theMenuItem, theRowIndex);
		if(modifiedDrawParam)
		{
            CWnd *parentView = ApplicationInterface::GetApplicationInterfaceImplementation()->GetView(theMenuItem.MapViewDescTopIndex());
			CFmiModifyDrawParamDlg dlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), modifiedDrawParam, itsMacroPathSettings.DrawParamPath(true), true, false, theMenuItem.MapViewDescTopIndex(), parentView);
			if(dlg.DoModal() == IDOK)
			{
				UpdateModifiedDrawParamMarko(theMenuItem.MapViewDescTopIndex(), modifiedDrawParam, theRowIndex);
                updateStatus = true;
			}
			else
                updateStatus = dlg.RefreshPressed(); // myˆs false:lla halutaan ruudun p‰ivitys, koska jos painettu p‰ivit‰-nappia ja sitten cancelia, pit‰‰ ruutu p‰ivitt‰‰
		}

        if(updateStatus)
            DrawParamSettingsChangedDirtyActions(theMenuItem.MapViewDescTopIndex(), GetRealRowNumber(theMenuItem.MapViewDescTopIndex(), theRowIndex), modifiedDrawParam);
        return updateStatus;
	}
}

void UpdateMacroDrawParam(const NFmiMenuItem& theMenuItem, int theRowIndex, bool fCrossSectionCase, boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
	NFmiDrawParamList* drawParamList = fCrossSectionCase ? CrossSectionViewDrawParamList(theRowIndex) : DrawParamList(theMenuItem.MapViewDescTopIndex(), theRowIndex);
	if(drawParamList)
	{
		if(drawParamList->Index(theMenuItem.IndexInViewRow()))
		{
			drawParamList->Current()->Init(theDrawParam);
			drawParamList->Dirty(true);
            DrawParamSettingsChangedDirtyActions(theMenuItem.MapViewDescTopIndex(), GetRealRowNumber(theMenuItem.MapViewDescTopIndex(), theRowIndex), theDrawParam);
		}
	}
}

boost::shared_ptr<NFmiDrawParam> GetUsedMacroDrawParam(const NFmiMenuItem& theMenuItem)
{
	std::string macroParamName = theMenuItem.DataIdent().GetParamName();
    if(itsMacroParamSystem.FindTotal(macroParamName)) // t‰ss‰ tod. init fileName
    {
        auto usedDrawParam = itsMacroParamSystem.CurrentMacroParam()->DrawParam();
        if(usedDrawParam)
        {
            usedDrawParam->ViewMacroDrawParam(theMenuItem.ViewMacroDrawParam()); // t‰m‰ pit‰‰ viel‰ asettaa
            return usedDrawParam;
        }
    }
    throw std::runtime_error(std::string("Error in ") + __FUNCTION__ + ": couldn't find searched macroParam '" + macroParamName + "'");
}

// muokataan macroParametrin asetuksia
bool ModifyMacroDrawParam(const NFmiMenuItem& theMenuItem, int theRowIndex, bool fCrossSectionCase)
{
	boost::shared_ptr<NFmiDrawParam> usedDrawParam = GetUsedMacroDrawParam(theMenuItem);
	if(usedDrawParam)
	{
        CWnd *parentView = ApplicationInterface::GetApplicationInterfaceImplementation()->GetView(theMenuItem.MapViewDescTopIndex());
        CFmiModifyDrawParamDlg dlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), usedDrawParam, itsMacroPathSettings.DrawParamPath(true), true, false, theMenuItem.MapViewDescTopIndex(), parentView);
		if(dlg.DoModal() == IDOK)
		{
			UpdateMacroDrawParam(theMenuItem, theRowIndex, fCrossSectionCase, usedDrawParam);
			return true;
		}
	}
	return false;
}

// laitetaan drawparam aikasarjan omaan listaa ja jos vertailutila k‰ytˆss‰, lis‰t‰‰
// viel‰ eri tuottajien drawparamit erilliseen listaan
void AddTimeSerialView(const NFmiMenuItem& theMenuItem, bool isViewMacroDrawParam = false)
{
	TimeSerialViewDirty(true);
	boost::shared_ptr<NFmiDrawParam> drawParam = itsSmartInfoOrganizer->CreateDrawParam(theMenuItem.DataIdent(), theMenuItem.Level(), theMenuItem.DataType());
	if(drawParam)
	{
		string logStr("Laitetaan aikasarja-n‰ytˆlle ");
		boost::shared_ptr<NFmiFastQueryInfo> info = InfoOrganizer()->Info(drawParam, false, false);
		if(info)
			logStr += GetSelectedParamInfoString(info, true);
		LogMessage(logStr, CatLog::Severity::Debug, CatLog::Category::Visualization);
		if(isViewMacroDrawParam)
			drawParam->ViewMacroDrawParam(true); // asetetaan tarvittaessa viewmacrodrawparam-flagi p‰‰lle

		itsTimeSerialViewDrawParamList->Add(drawParam, itsTimeSerialViewIndex);

		bool groundData = true;
		if(info)
			groundData = info->SizeLevels() <= 1;
		UpdateModifiedDrawParam(drawParam, groundData);
	}
}

void RemoveTimeSerialView(const NFmiMenuItem& theMenuItem)
{
	TimeSerialViewDirty(true);
	if(itsTimeSerialViewDrawParamList->Index(theMenuItem.IndexInViewRow()))
		itsTimeSerialViewDrawParamList->Remove();
}

void TimeSerialViewModelRunCountSet(const NFmiMenuItem &theMenuItem, int theRowIndex)
{
	NFmiDrawParamList *drawParamList = DrawParamList(theMenuItem.MapViewDescTopIndex(), theRowIndex);
	if(drawParamList && drawParamList->Index(theMenuItem.IndexInViewRow()))
	{
		drawParamList->Current()->TimeSerialModelRunCount(static_cast<int>(theMenuItem.ExtraParam()));

		drawParamList->Dirty(true);
		CheckAnimationLockedModeTimeBags(theMenuItem.MapViewDescTopIndex(), false); // kun parametrin n‰kyvyytt‰ vaihdetaan, pit‰‰ tehd‰ mahdollisesti animaatio moodin datan tarkistus
		MapViewDirty(theMenuItem.MapViewDescTopIndex(), false, false, true, false, false, false);
		UpdateModifiedDrawParamMarko(theMenuItem.MapViewDescTopIndex(), drawParamList->Current(), theRowIndex);
		if(theMenuItem.MapViewDescTopIndex() == CtrlViewUtils::kFmiTimeSerialView)
			TimeSerialViewDirty(true);
	}
}

// laskee n‰yttˆruudukon yhden ruudun koon pikseleiss‰
NFmiPoint ActualMapBitmapSizeInPixels(unsigned int theDescTopIndex)
{
	return MapViewDescTop(theDescTopIndex)->ActualMapBitmapSizeInPixels();
}
NFmiEditMapDataListHandler* DataLists(void)
{
	return itsListHandler;
}
// tallettaa aktiivisen n‰yttˆrivin aktiivisen parametrin currentin ajan gridin tiedostoon,
// jonka nimi annetaan parametrina, mutta se talletetaan tyˆhakemistoon
bool MakeGridFile(const NFmiString& theFileName)
{
	boost::shared_ptr<NFmiDrawParam> drawParam = ActiveDrawParam(0, ActiveViewRow(0)); // tehd‰‰n vain p‰‰karttaikkunasta n‰it‰ talletuksia
	bool status = false;
	if(drawParam)
	{
		boost::shared_ptr<NFmiFastQueryInfo> info = InfoOrganizer()->Info(drawParam, false, false);
		if(info == 0)
			return false;
		NFmiMetTime oldTime = info->Time();
		if(info->DataType() == NFmiInfoData::kStationary || info->TimeDescriptor().IsInside(itsActiveViewTime))
		{
			NFmiDataMatrix<float> dMatrix;
			if(info->DataType() == NFmiInfoData::kStationary)
				info->Values(dMatrix);
			else
				info->Values(dMatrix, itsActiveViewTime);
			NFmiString fileName(itsBasicConfigurations.WorkingDirectory());
			fileName += "\\";
			fileName += theFileName;
			std::ofstream out(fileName, std::ios::binary);
			if(out)
			{
				out << dMatrix;
				out.close();
				status = true;
			}
		}
		info->Time(oldTime);
	}
	return status;
}
void UpdateEditedDataCopy(void)
{
	if(itsSmartInfoOrganizer)
	{
		itsSmartInfoOrganizer->UpdateEditedDataCopy();
	}
}
const NFmiRect& AreaFilterRange(int index)
{
	if(index == 1)
		return itsAreaFilterRangeStart;
	else
		return itsAreaFilterRangeEnd;
}
void AreaFilterRange(int index, const NFmiRect& theRect)
{
	if(index == 1)
	{
		itsAreaFilterRangeStart = theRect;
		AdjustAreaFilterRanges(itsAreaFilterRangeStart);
	}
	else
	{
		itsAreaFilterRangeEnd = theRect;
		AdjustAreaFilterRanges(itsAreaFilterRangeEnd);
	}
}
void AdjustAreaFilterRanges(NFmiRect& theRanges)
{
	int left = (int)theRanges.Left();
	int right = (int)theRanges.Right();
	int maxLeft = (int)itsAreaFilterRangeLimits.Left();
	int maxRight = (int)itsAreaFilterRangeLimits.Right();

	if(left < maxLeft)
		left = maxLeft;
	if(left > right)
		left = right;
	if(left > maxRight)
		left = maxRight;

	if(right < maxLeft)
		right = maxLeft;
	if(right < left) // turha tarkastelu, tehty jo edell‰(??)
		right = left;
	if(right > maxRight)
		right = maxRight;

	int top = (int)theRanges.Top();
	int bottom = (int)theRanges.Bottom();
	int maxTop = (int)itsAreaFilterRangeLimits.Top();
	int maxBottom = (int)itsAreaFilterRangeLimits.Bottom();

	if(bottom < maxTop)
		bottom = maxTop;
	if(bottom < top)
		bottom = top;
	if(bottom > maxBottom)
		bottom = maxBottom;

	if(top < maxTop)
		top = maxTop;
	if(top > bottom) // turha tarkastelu, tehty jo edell‰(??)
		top = bottom;
	if(top > maxBottom)
		top = maxBottom;

	theRanges = NFmiRect(left, top, right, bottom);
}
const NFmiPoint& TimeFilterRange(int index)
{
	if(index == 1)
		return itsTimeFilterRangeStart;
	else
		return itsTimeFilterRangeEnd;
}

void TimeFilterRange(int index, const NFmiPoint& thePoint, bool fRoundToNearestHour)
{
	if(thePoint.X() == thePoint.Y())
	{
		if(fRoundToNearestHour)
		{
			int hoursInDataTimeResolution = 1;
			boost::shared_ptr<NFmiFastQueryInfo> editedInfo = EditedInfo();
			if(editedInfo)
			{
				if(editedInfo->TimeResolution() > 60) // piirret‰‰n apu-tunti-viivat jos resoluutio on yli tunnin
					hoursInDataTimeResolution = int(editedInfo->TimeResolution()/60);
			}
			double xValue = round(thePoint.X() * hoursInDataTimeResolution)/double(hoursInDataTimeResolution);
			double yValue = round(thePoint.Y() * hoursInDataTimeResolution)/double(hoursInDataTimeResolution);
			if(index == 1)
				itsTimeFilterRangeStart = NFmiPoint(xValue, yValue);
			else // index == 2
				itsTimeFilterRangeEnd = NFmiPoint(xValue, yValue);
		}
		else
		{
			if(index == 1)
				itsTimeFilterRangeStart = thePoint;
			else // index == 2
				itsTimeFilterRangeEnd = thePoint;
		}
	}
	else
	{
		int first = boost::math::iround(thePoint.X());
        int second = boost::math::iround(thePoint.Y());
		if(first < itsTimeFilterLimits.X())
			first = int(itsTimeFilterLimits.X());
		if(first > itsTimeFilterLimits.Y())
			first = int(itsTimeFilterLimits.Y());
		if(first > second)
			first = second;
		if(second < itsTimeFilterLimits.X())
			second = int(itsTimeFilterLimits.X());
		if(second > itsTimeFilterLimits.Y())
			second = int(itsTimeFilterLimits.Y());
		if(second < first) // turha tarkastelu, tehty jo yll‰
			second = first;

		if(index == 1)
			itsTimeFilterRangeStart = NFmiPoint(first, second);
		else // index == 2
			itsTimeFilterRangeEnd = NFmiPoint(first, second);
	}
}

void ResetFilters(void)
{
	itsAreaFilterRangeStart = NFmiRect();
	itsAreaFilterRangeEnd = NFmiRect();
	itsTimeFilterRangeStart = NFmiPoint();
	itsTimeFilterRangeEnd = NFmiPoint();
    ResetTimeFilterTimes();
}
bool IsSmoothTimeShiftPossible(void)
{
	if(itsTimeFilterRangeStart.X() == itsTimeFilterRangeStart.Y() &&
		itsTimeFilterRangeEnd.X() == itsTimeFilterRangeEnd.Y())
		return true;
	return false;
}

bool DoAreaFiltering(bool fPasteClipBoardData = false)
{
	return FmiModifyEditdData::DoAreaFiltering(GenDocDataAdapter(), fPasteClipBoardData, UseMultithreaddingWithModifyingFunctions());
}

bool DoTimeFiltering(void)
{
	return FmiModifyEditdData::DoTimeFiltering(GenDocDataAdapter(), UseMultithreaddingWithModifyingFunctions());
}

bool CreatePasteData(void)
{
	if(itsClipBoardData)
		delete itsClipBoardData, itsClipBoardData = 0;

	boost::shared_ptr<NFmiDrawParam> drawParam = ActiveDrawParam(0, ActiveViewRow(0)); // t‰m‰ copy/paste tyˆkalulla tehdyt muokkaukset tehd‰‰n aina p‰‰karttaikkunan (0-desctop-indeksi) aktiiviselle datalle
	if(drawParam)
	{
		boost::shared_ptr<NFmiFastQueryInfo> info = InfoOrganizer()->Info(drawParam, false, false);
		if(info && info->IsGrid())
		{
			NFmiMetTime oldTime = info->Time();
			if(info->Time(itsActiveViewTime))
			{
				itsClipBoardData = new NFmiGrid(info->GridValue());
			}
			info->Time(oldTime);
			return true;
		}
	}
	return false;
}
bool HasActiveViewChanged(void)
{
	if(itsLastBrushedViewRow != ActiveViewRow(0))
		return true;
	if(itsLastBrushedViewTime != itsActiveViewTime)
		return true;
	return false;
}
void SetTimeFilterStartTime(const NFmiMetTime& theTime)
{
	// etsit‰‰n l‰hin aika, koska nyt on mahdollista ett‰ aikaresoluutio muuttuu datassa
	boost::shared_ptr<NFmiFastQueryInfo> editedInfo = EditedInfo(); // HUOM! EditedInfo pit‰‰ ottaa omaan muuttujaan, ja k‰ytt‰‰ siit‰. Jos pyyt‰‰ EditedInfo-funktion kautta kokoajan uutta, voi palautua eri iteraattori ja s‰‰dˆt menev‰t pieleen.
	if(editedInfo && editedInfo->TimeToNearestStep(theTime, kCenter))
	{
		NFmiMetTime nearestTime(editedInfo->Time());
		if(nearestTime > itsTimeFilterEndTime)
			itsTimeFilterEndTime = nearestTime;
		else
			itsTimeFilterStartTime = nearestTime;
	}
}

void SetTimeFilterEndTime(const NFmiMetTime& theTime)
{
	// etsit‰‰n l‰hin aika, koska nyt on mahdollista ett‰ aikaresoluutio muuttuu datassa
	boost::shared_ptr<NFmiFastQueryInfo> editedInfo = EditedInfo();
	if(editedInfo && editedInfo->TimeToNearestStep(theTime, kCenter))
	{
		NFmiMetTime nearestTime(editedInfo->Time());
		if(nearestTime < itsTimeFilterStartTime)
			itsTimeFilterStartTime = nearestTime;
		else
			itsTimeFilterEndTime = nearestTime;
	}
}

void ResetTimeFilterTimes()
{
    const NFmiTimeBag &editedTimeBag = EditedDataTimeBag();
    itsTimeFilterStartTime = editedTimeBag.FirstTime();
    itsTimeFilterEndTime = editedTimeBag.LastTime();
}

void SetActiveParamMissingValues(double theValue)
{
	FmiModifyEditdData::SetActiveParamMissingValues(GenDocDataAdapter(), theValue, UseMultithreaddingWithModifyingFunctions());
}

void ChangeWmsMapType(unsigned int theDescTopIndex, bool fForward)
{
#ifndef DISABLE_CPPRESTSDK
    if(fForward)
        WmsSupport().nextBackground();
    else
        WmsSupport().previousBackground();

    MapViewDirty(theDescTopIndex, true, true, true, false, false, false);
#endif // DISABLE_CPPRESTSDK
}

void ChangeFileBitmapMapType(unsigned int theDescTopIndex, bool fForward)
{
    if(fForward)
        MapViewDescTop(theDescTopIndex)->MapHandler()->NextMap();
    else
        MapViewDescTop(theDescTopIndex)->MapHandler()->PreviousMap();
    MapViewDirty(theDescTopIndex, true, true, true, false, false, false);
}

void ChangeMapType(unsigned int theDescTopIndex, bool fForward)
{
    if(UseWmsMaps())
        ChangeWmsMapType(theDescTopIndex, fForward);
    else
        ChangeFileBitmapMapType(theDescTopIndex, fForward);
}

bool LoadStaticHelpData(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	bool status = false;
	int dCount = HelpDataInfoSystem()->StaticCount();
	for(int i=0; i<dCount; i++)
	{
		NFmiHelpDataInfo& hDataInfo = HelpDataInfoSystem()->StaticHelpDataInfo(i);
		if(LoadHelpData(hDataInfo, true, true))
			status = true;
	}
	return status;
}

bool LoadHelpData(NFmiHelpDataInfo& theHelpDataInfo, bool fMustFindData, bool useControlPathIfFileFilterNotAbsolute = false)
{
	if(theHelpDataInfo.DataType() == NFmiInfoData::kSatelData) // satelliitti data on kuva, eik‰ sit‰ ladata t‰‰ll‰
		return false;
	NFmiMilliSecondTimer timer;
	timer.StartTimer();
	string fileFilter(theHelpDataInfo.UsedFileNameFilter(*HelpDataInfoSystem()));
	time_t timeStamp = 0;
	time_t latestTimeStamp = theHelpDataInfo.LatestFileTimeStamp();
	string latestFileName;
	try
	{
        if(useControlPathIfFileFilterNotAbsolute)
            fileFilter = PathUtils::getAbsoluteFilePath(fileFilter, ControlDirectory());
		std::unique_ptr<NFmiQueryData> data = QueryDataReading::ReadLatestDataWithFileFilterAfterTimeStamp(fileFilter, latestTimeStamp, latestFileName, timeStamp);
        if(data)
        {
            if(theHelpDataInfo.FakeProducerId() != 0)
            {
                data->Info()->First();
                NFmiProducer kepaProducer(*data->Info()->Producer());
                kepaProducer.SetIdent(theHelpDataInfo.FakeProducerId());
                data->Info()->SetProducer(kepaProducer);
            }

            AddQueryData(data.release(), latestFileName, fileFilter, theHelpDataInfo.DataType(), "", false);
            theHelpDataInfo.LatestFileTimeStamp(timeStamp);
            theHelpDataInfo.LatestFileName(latestFileName);
            timer.StopTimer();
            string logStr("Apudatan luku: ");
            logStr += latestFileName;
            logStr += " kesto: ";
            logStr += NFmiValueString::GetStringWithMaxDecimalsSmartWay(timer.TimeDiffInMSeconds() / 1000, 0);
            logStr += ".";
            logStr += NFmiValueString::GetStringWithMaxDecimalsSmartWay(timer.TimeDiffInMSeconds() % 1000, 0);
            logStr += " s.";

            this->LogMessage(logStr, CatLog::Severity::Info, CatLog::Category::Data);
            return true;
        }
        else if(fMustFindData)
        {
            string debugStr("Tiedostoa '");
            debugStr += fileFilter;
            debugStr += ("' ei saatu luettua.");
            LogMessage(debugStr, CatLog::Severity::Error, CatLog::Category::Data);
        }
    }
	catch(char *msg)
	{
		string debugStr("Tiedoston '");
		debugStr += fileFilter;
		debugStr += ("' luvussa oli ongelma: ");
		debugStr += msg;
		LogMessage(debugStr, CatLog::Severity::Error, CatLog::Category::Data);
	}

	return false;
}

// scrollaa n‰yttˆriveja halutun m‰‰r‰n (negatiivinen skrollaa ylˆs ja positiivinen count alas)
bool ScrollViewRow(unsigned int theDescTopIndex, int theCount)
{
	int activeViewRow = ActiveViewRow(theDescTopIndex);
	if(MapViewDescTop(theDescTopIndex)->ScrollViewRow(theCount, activeViewRow))
	{
		ActiveViewRow(theDescTopIndex, activeViewRow); // asetetaan uusi suhteellinen aktiivinen rivi takaisin desctopiin
        UpdateRowInLockedDescTops(theDescTopIndex);
		MapViewDirty(theDescTopIndex, false, false, true, false, false, true);
		return true;
	}
	else
		return false;
}

bool IsMacroParamAndDrawWithSymbols(boost::shared_ptr<NFmiDrawParam> &drawParam)
{
    if(drawParam && drawParam->IsMacroParamCase(true))
    {
        // MacroParam on aina hila muotoista, joten tarkastetaan hilamuotoinen visualisointi tyyppi
        auto visualizationType = drawParam->GridDataPresentationStyle();
        if(!(visualizationType >= NFmiMetEditorTypes::kFmiIsoLineView && visualizationType <= NFmiMetEditorTypes::kFmiQuickColorContourView))
            return true;
    }
    return false;
}

// Jos karttan‰ytˆn koko muutetaan, muuttuu t‰llˆin piirrett‰v‰n karttaalueen koko pikseleiss‰.
// T‰llˆin jos n‰ytˆll‰ on macroParametreja, jotka piirret‰‰n symboleilla, pit‰‰ niiden macroParamCachet 
// laittaa uusiksi, koska piirto harvennus saattaa muuttua. Ei ole v‰li‰, onko parametri piilotettu tai
// ei, koska kun alue muuuttuu, pit‰‰ varmuuden vuoksi kaikki kyseiset macroParamien datacachet tyhjent‰‰.
void MapViewSizeChangedDoSymbolMacroParamCacheChecks(int mapViewDescTopIndex)
{
    auto mapViewDesctop = MapViewDescTop(mapViewDescTopIndex);
    if(mapViewDesctop)
    {
        int rowIndex = 1;
        NFmiPtrList<NFmiDrawParamList> *drawParamListVector = mapViewDesctop->DrawParamListVector();
        NFmiPtrList<NFmiDrawParamList>::Iterator iter = drawParamListVector->Start();
        for(; iter.Next();)
        {
            NFmiDrawParamList *aList = iter.CurrentPtr();
            if(aList)
            {
                for(aList->Reset(); aList->Next(); )
                {
                    boost::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
                    if(IsMacroParamAndDrawWithSymbols(drawParam))
                    {
                        MacroParamDataCache().clearMacroParamCache(mapViewDescTopIndex, rowIndex, drawParam->InitFileName());
                    }
                }
            }
            rowIndex++;
        }
    }
}

void DoMapViewOnSize(int mapViewDescTopIndex, const NFmiPoint &totalPixelSize, const NFmiPoint &clientPixelSize)
{
    auto keepMapAspectRatio = ApplicationWinRegistry().KeepMapAspectRatio();
    // Jos karttan‰yttˆ‰ venytet‰‰n ja keepMapAspectRatio on true, t‰llˆin tapahtuu automaattinen 
    // alueen zoomaus ja silloin macroParamDataCache pit‰‰ tyhjent‰‰ t‰lle n‰ytˆlle.
    MapViewDirty(mapViewDescTopIndex, true, true, true, keepMapAspectRatio, false, false);
    MapViewSizeChangedDoSymbolMacroParamCacheChecks(mapViewDescTopIndex);
    auto mapViewDesctop = MapViewDescTop(mapViewDescTopIndex);
    if(mapViewDesctop)
    {
        mapViewDesctop->CalcClientViewXperYRatio(totalPixelSize);
        mapViewDesctop->MapViewSizeInPixels(clientPixelSize);
        mapViewDesctop->BorderDrawDirty(true);
    }
}

void TimeControlTimeStep(unsigned int theDescTopIndex, float newValue)
{
	if(MapViewDescTop(theDescTopIndex)->TimeControlTimeStep() != newValue)
	{
		MapViewDescTop(theDescTopIndex)->TimeControlTimeStep(newValue);
        // laitetaan viela kaikki ajat likaisiksi cachesta kun aika-askel muuttuu, 
        // pit‰‰ mahdollisesti piirt‰‰ uusiksi salama dataa (ja ehk‰ jotain muuta?), 
        // joten varmuuden vuoksi laitan aina v‰limuistin likaiseksi
		MapViewDirty(theDescTopIndex, false, true, true, false, false, false);
	}
}
// palauttaa k‰ytetyn aikastepin tunteina. Jos asetuksissa m‰‰r‰tty aikasteppi
// on pienempi, kuin datan aikaresoluutio, palautetaan datan aikaresoluutio tunteina.
float TimeControlTimeStep(unsigned int theDescTopIndex)
{
	return MapViewDescTop(theDescTopIndex)->TimeControlTimeStep();
}

NFmiTimeBag AdjustTimeBagToGivenTimeBag(const NFmiTimeBag& theRestrictingTimebag, const NFmiTimeBag& wantedTimebag)
{
	if(theRestrictingTimebag.IsInside(wantedTimebag.FirstTime()) && theRestrictingTimebag.IsInside(wantedTimebag.LastTime()))
		return wantedTimebag;
	NFmiMetTime firstTime(wantedTimebag.FirstTime());
	if(firstTime < theRestrictingTimebag.FirstTime())
		firstTime = theRestrictingTimebag.FirstTime();
	NFmiMetTime lastTime(wantedTimebag.LastTime());
	if(lastTime > theRestrictingTimebag.LastTime())
		lastTime = theRestrictingTimebag.LastTime();
	NFmiTimeBag returnbag(firstTime, lastTime, wantedTimebag.Resolution());
	return returnbag;
}

// T‰m‰ on pika toiminto editorille.
// Kun 'Klapse'-nappia painaa, otetaan aikarajat aikas‰‰timist‰ ja yhdist‰miskertoimet
// ja k‰ytetty maski (valittu,kaikki) muokkausdialogista. Sitten yhdistet‰‰n
// Klapse ja mallin sateet mallinsateisiin halutulle aikav‰lille.
bool DoCombineModelAndKlapse(void)
{
	return FmiModifyEditdData::DoCombineModelAndKlapse(GenDocDataAdapter(), UseMultithreaddingWithModifyingFunctions());
}

NFmiMetEditorOptionsData& MetEditorOptionsData(void)
{	return itsMetEditorOptionsData; }


void DoAutoSaveData(void)
{
	if(DataModificationInProgress())
		return ; // jos ollaan modifioimassa dataa, ei sallita auto-save -toimintoa, koska modifiointi on erillisess‰ threadissa 
				 // ja voi aiheuttaa ongelmia t‰m‰n talletuksen kanssa. Esim. kontrollipiste editointi ja CP-pisteiden backup ongelma.

    static bool hasMadeWarningWithDialog = false; // Varoitetaan k‰ytt‰j‰‰ vain yhden kerran, jos levytila loppuu ja ei tehd‰ en‰‰ auto talletuksia.
    try
    {
	    if(MetEditorOptionsData().UseAutoSave())
	    {
		    if(MetEditorOptionsData().ControlPointMode()) // talletetaan CP-dataa vain kun k‰ytet‰‰n kyseist‰ tyˆkalua, eik‰ joka ikinen minuutti
            {
                DoConfigurationsCanBeSavedCheck(true, "SmartMet won't store any modified control-point information to backup files.");
			    StoreAllCPDataToFiles(); // t‰m‰ tallentaa CP-datan (mm. muutos k‰yr‰t joka pisteeseen jokaiselle parametrille) backuppiin kaatumisten varalta (ladataan sitten k‰ynnistett‰ess‰)
            }

		    boost::shared_ptr<NFmiFastQueryInfo> smart = EditedInfo();
		    if(smart)
		    {
			    if(dynamic_cast<NFmiSmartInfo*>(smart.get())->IsDirty())
			    {
                    DoConfigurationsCanBeSavedCheck(false);
				    if(dynamic_cast<NFmiSmartInfo*>(smart.get())->LoadedFromFile())
				    {
					    LogMessage("Automatically saved edited data that was loaded from a file.", CatLog::Severity::Info, CatLog::Category::Editing);
					    StoreData(false, false); // false = ei uusi tiedosto, false ei kysy talletukseen varmistusta
				    }
				    else
				    {
					    bool status = StoreWorkingData(smart, false); // false ei kysy talletukseen varmistusta
					    if(status)
						    LogMessage("Automatically saved edited data.", CatLog::Severity::Info, CatLog::Category::Editing);
					    else
						    LogMessage("Unable to automatically save edited data!", CatLog::Severity::Error, CatLog::Category::Editing);
				    }
			    }
		    }
	    }
    }
    catch(std::exception &e)
    {
        if(!hasMadeWarningWithDialog)
        {
            hasMadeWarningWithDialog = true;
		    std::string problemStr = e.what();
		    LogAndWarnUser(problemStr, "Problems when doing editing related auto savings", CatLog::Severity::Error, CatLog::Category::Editing, false);
        }
    }
}

// Ns. working data talletetaan vain jos on k‰ytˆss‰ undo/redo toiminto (editointia ei voi tehd‰ ilman sit‰: memorymapping read-only jutun takia)
// JA jos ollaan ns. normaali editointi moodissa.
// Muissa tapauksissa ei talletetan t‰t‰ editointiin liittyv‰‰ backup tiedostoa.
bool IsWorkingDataSaved()
{
    if(MetEditorOptionsData().UndoRedoDepth() > 0)
    {
        if(SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal)
        {
            return true;
        }
    }
    return false;
}

bool StoreWorkingData(boost::shared_ptr<NFmiFastQueryInfo> &smart, bool askForSave)
{
    if(IsWorkingDataSaved())
    {
        GetUsedDataLoadingInfo().InitFileNameLists();
        NFmiString fileNameWithPath(GetUsedDataLoadingInfo().CreateWorkingFileName(GetUsedDataLoadingInfo().LatestWorkingVersion() + 1));
        return StoreData(fileNameWithPath, smart, askForSave);
    }
    else
        return false;
}

bool StoreData(bool newFile, bool askForSave)
{
	boost::shared_ptr<NFmiFastQueryInfo> info = EditedInfo();
	if(info)
	{
		if(newFile)
		{
			if(dynamic_cast<NFmiSmartInfo*>(info.get())->IsDirty())
				return StoreWorkingData(info, askForSave);
		}
		else
		{
			NFmiString fileName(dynamic_cast<NFmiSmartInfo*>(info.get())->DataFileName());
			bool status = StoreData(fileName, info, askForSave);
			return status;
		}
	}
	return true;
}

std::string GetHelperForecasterId()
{
    if(ApplicationWinRegistry().ConfigurationRelatedWinRegistry().AddHelpDataIdAtSendindDataToDatabase())
    {
        boost::shared_ptr<NFmiFastQueryInfo> helperData = InfoOrganizer()->FindInfo(NFmiInfoData::kEditingHelpData);
        if(helperData && helperData->FindFirstKey(FmiModifyEditdData::ForecasterIDKey)) // Lˆytyikˆ helperData ja lˆytyykˆ siit‰ ForecasterID:t‰?
            return std::string(helperData->GetCurrentKeyValue());
    }

    return "";
}

// Saves data to server (or for press-test).
bool StoreDataToDataBase(const std::string &theForecasterId)
{
    std::string helperForecasterId = GetHelperForecasterId();
    bool status = StoreDataToDataBase(theForecasterId, helperForecasterId);
    if(status && fUseDoAtSendCommand)
    {
        status = status && CFmiProcessHelpers::ExecuteCommandInSeparateProcess(itsDoAtSendCommandString);
    }
    return status;
}

void DoMacroDirectoriesSyncronization(void)
{
    // HUOM! T‰‰lll‰ pit‰‰ hanskata samat kansiot kuin DoMacroFileInitializations -metodissa.
    try
	{
		if(itsMacroPathSettings.UseLocalCache() == false)
			return ; // ei tehd‰ mit‰‰n jos lokaali makro cachetys ei ole k‰ytˆss‰

		// 1. Tarkista ettei ole yht‰‰n unison-ohjelmaa k‰ytˆss‰, jos on, oletetaan ett‰ joku toinen SmartMet on juuri tekem‰ss‰ synkronointia ja t‰m‰n
		// instanssin ei tarvitse tehd‰ sit‰.
		NFmiApplicationDataBase::AppSpyData appData(std::make_pair(GetUnisonApplicationName(), false)); // false tarkoittaa ett‰ aplikaatiolta ei pyydet‰ versionumeroa
		std::string appVersionsStrOutDummy; // t‰m‰ pit‰‰ antaa NFmiApplicationDataBase::CountProcessCount-funktiolle, mutta sit‰ ei k‰ytet‰
		int unisonProcessCount = NFmiApplicationDataBase::CountProcessCount(appData, appVersionsStrOutDummy);
		if(unisonProcessCount > 0)
			return ;

		// 2. Tarkista monesko synkronointi on menossa, jos on 1., prefer-suunta on server->local (konflikteissa)
		// jos on 2-n:s synkronointi, prefer-suunta on local->server
		bool preferRoot1 = (itsMacroDirectoriesSyncronizationCounter <= 1);

		// 3. K‰ynnist‰ tarvittava m‰‰r‰ synkronointeja, ne ajetaan omissa processeissaan omalla ajallaan loppuun, SmartMet ei odota niiden valmistumista.
		DWORD priorityClass = BELOW_NORMAL_PRIORITY_CLASS;
        if(itsMachineThreadCount >= 8) 
            priorityClass = NORMAL_PRIORITY_CLASS;
		DoUnisonDirectorySync(itsMacroPathSettings.DrawParamPath(false), itsMacroPathSettings.DrawParamPath(true), preferRoot1, SW_HIDE, false, priorityClass);
		DoUnisonDirectorySync(itsMacroPathSettings.MacroParamPath(false), itsMacroPathSettings.MacroParamPath(true), preferRoot1, SW_HIDE, false, priorityClass);
		DoUnisonDirectorySync(itsMacroPathSettings.SmartToolPath(false), itsMacroPathSettings.SmartToolPath(true), preferRoot1, SW_HIDE, false, priorityClass);
		DoUnisonDirectorySync(itsMacroPathSettings.ViewMacroPath(false), itsMacroPathSettings.ViewMacroPath(true), preferRoot1, SW_HIDE, false, priorityClass);
        if(itsBetaProductionSystem.DoCacheSyncronization())
            DoUnisonDirectorySync(itsBetaProductionSystem.GetBetaProductionBaseDirectory(false), itsBetaProductionSystem.GetBetaProductionBaseDirectory(true), preferRoot1, SW_HIDE, false, priorityClass);

		// 4. kasvata synkronointi counteria
		itsMacroDirectoriesSyncronizationCounter++;
	}
	catch(...)
	{
	}
}


#undef CreateDirectory // pit‰‰ poistaa winkkarin tekemi‰ definej‰

bool CheckThatParentDirectoryExists(const std::string &theDir)
{
	if(theDir.empty())
		return false;
	std::string dirStr = theDir;
	NFmiStringTools::ReplaceAll(dirStr, "\\", "/"); // korvataan kaikki mahdolliset windows-kenot linux-kenoilla (toimii sitten molemmisssa systeemeiss‰)
	NFmiStringTools::TrimR(dirStr, '/'); // poistetaan mahdollinen viimeinen keno
	std::string::size_type pos = dirStr.rfind('/'); // etsit‰‰n viimeisen kenon paikka
	if(pos != std::string::npos)
	{
		std::string parentDir(dirStr.begin(), dirStr.begin()+pos);
		if(NFmiFileSystem::DirectoryExists(parentDir) == false)
		{
			if(NFmiFileSystem::CreateDirectory(parentDir) == false)
				return false;
		}
		return true;
	}
	else
		return false;
}

std::string GetUnisonApplicationName(void)
{
	return std::string("unison_smartmet.exe");
}

// theRoot1 on serverill‰ oleva hakemisto (t‰rke‰mpi)
// theRoot2 on lokaalilla kovalevyll‰ oleva hakemisto
void DoUnisonDirectorySync(const std::string &theRoot1, const std::string &theRoot2, bool preferRoot1, WORD theShowWindow, bool waitExecutionToStop, DWORD dwCreationFlags)
{
	if(CheckThatParentDirectoryExists(theRoot2) == false) // lokaali hakemistosta pit‰‰ olla emo-hakemisto olemassa ennen ei synkronointi onnistu
	{
		std::string errStr = "Local macro folder:\n'";
		errStr += theRoot2;
		errStr += ",\ncannot be created.";
		LogAndWarnUser(errStr, "Error when creating local macro folder", CatLog::Severity::Error, CatLog::Category::Operational, false);
		return ;
	}

	std::string commandStr = WorkingDirectory();
	commandStr += "\\utils\\";
	commandStr += GetUnisonApplicationName();
	commandStr += " ";
	commandStr += theRoot1;
	commandStr += " ";
	commandStr += theRoot2;

	// lis‰t‰‰n unison optioita loppuun
	commandStr += " -batch";
	commandStr += " -prefer " + (preferRoot1 ? theRoot1 : theRoot2);
	commandStr += " -retry 3";
	bool ignoreArchives = (NFmiFileSystem::DirectoryExists(theRoot1) == false) || (NFmiFileSystem::DirectoryExists(theRoot2) == false);
	if(ignoreArchives)
		commandStr += " -ignorearchives"; // jos jompi kumpi hakemistoista oli poistettu, ignoorataan arkistot (n‰in ei p‰‰se tapahtumaan isoja tuhoja aikaan)

    CFmiProcessHelpers::ExecuteCommandInSeparateProcess(commandStr, nullptr, false, theShowWindow, waitExecutionToStop, dwCreationFlags);
}

std::string CreateHelpEditorFileNameWithPath(void)
{
	std::string fileName(itsHelpEditorSystem.DataPath());
	fileName += "\\"; // varmuudeksi laitetaan polku deliminator per‰‰n
	NFmiMetTime aTime(1); // currentti aika aikaleimaa varten
	fileName += aTime.ToStr(kYYYYMMDDHHMM);
	fileName += "_";
	fileName += itsHelpEditorSystem.FileNameBase();
	return fileName;
}

bool StoreDataToDataBase(const std::string &theForecasterId, const std::string &theHelperForecasterId)
{
	bool status = FmiModifyEditdData::StoreDataToDataBase(GenDocDataAdapter(), theForecasterId, theHelperForecasterId, UseMultithreaddingWithModifyingFunctions());
    if(status) // If sending was succesfull
    { 
        // Change last send time to current time and reset return flag for edited data.
        itsLastEditedDataSendTime = NFmiMetTime(1);
        fLastEditedDataSendHasComeBack = false;
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Sending edited data to database", TRUE, FALSE, 0);
    }
    return status;
}

// writeLocalDiskThenCopyToDestination talletus on erikoistapaus. Jos kopiointi serveriin (esim. wiuhti:in) on normaalia c++ stream bin‰‰ri write 
// operaatiota k‰ytt‰en jostain syyst‰ hidasta, talletetaan data ensin lokaaliin hakemistoon. Sitten se kopioidaan serverille k‰ytt‰en CopyFileEx 
// win32-funktiota tmp-nimell‰, sitten rename oikeaksi nimeksi, sitten lokaali tmp-tiedosto pit‰‰ siivota pois.
bool StoreData(bool fDoSaveTmpRename, const NFmiString& theFileName, NFmiQueryData *theData, bool writeLocalDiskThenCopyToDestination)
{
	if(theData)
	{
		NFmiStreamQueryData sQData;
		if(fDoSaveTmpRename)
		{ // tehd‰‰n kirjoitus tmp-tiedostoon ja lopuksi tehd‰‰n rename tiedostolle lopullisen nimiseksi
			NFmiFileString tmpFileNameStr(theFileName);
			NFmiString tmpFileName = "TMP_";
			tmpFileName += tmpFileNameStr.FileName();
			tmpFileName += "_TMP";
			tmpFileNameStr.FileName(tmpFileName);

            bool status = false;
            if(writeLocalDiskThenCopyToDestination)
            {
                std::string localTmpDirectory("C:\\tmp");
                if(NFmiFileSystem::CreateDirectory(localTmpDirectory))
                {
                    NFmiFileString localTmpFileNameStr(localTmpDirectory + "\\tmpData.sqd");
                    localTmpFileNameStr.FileName(tmpFileNameStr.FileName());
                    status = sQData.WriteData(localTmpFileNameStr, theData, static_cast<long>(theData->InfoVersion()));
                    if(status)
                    {
                        DWORD dwCopyFlags = COPY_FILE_FAIL_IF_EXISTS; // & COPY_FILE_NO_BUFFERING;
                        status = CopyFileEx(CA2T(localTmpFileNameStr), CA2T(tmpFileNameStr), 0, 0, 0, dwCopyFlags) == TRUE;
                    }
                    NFmiFileSystem::RemoveFile(std::string(localTmpFileNameStr));
                }
            }
            else
                status = sQData.WriteData(tmpFileNameStr, theData, static_cast<long>(theData->InfoVersion()));

			if(status)
			{
				if(NFmiFileSystem::RenameFile(tmpFileNameStr.CharPtr(), theFileName.CharPtr()))
					return true;
				else
				{
					std::string errLogStr("Error: StoreData failed, cannot rename tmp-file to final file: \n");
					errLogStr += tmpFileNameStr.CharPtr();
					errLogStr += " -> ";
					errLogStr += theFileName.CharPtr();
					LogMessage(errLogStr, CatLog::Severity::Error, CatLog::Category::Data);
					return false;
				}
			}
			else
			{
				std::string errLogStr("Error: StoreData failed, cannot store file in tmp-file: ");
				errLogStr += tmpFileNameStr.CharPtr();
				LogMessage(errLogStr, CatLog::Severity::Error, CatLog::Category::Data);
				return false;
			}
		}
		else
		{
			if(sQData.WriteData(theFileName, theData, static_cast<long>(theData->InfoVersion())))
				return true;
			else
			{
				std::string errLogStr("Error: StoreData failed, cannot store file in file: ");
				errLogStr += theFileName.CharPtr();
				LogMessage(errLogStr, CatLog::Severity::Error, CatLog::Category::Data);
			}
		}
	}
	std::string errLogStr("Error: StoreData failed, there were no data (null pointer) to store to file: ");
	errLogStr += theFileName.CharPtr();
	LogMessage(errLogStr, CatLog::Severity::Error, CatLog::Category::Data);
	return false;
}

bool StoreData(const NFmiString& theFileName, boost::shared_ptr<NFmiFastQueryInfo> &theSmartInfo, bool askForSave)
{
	if(askForSave) // lis‰sin askForSaven, ett‰ ei aina kysytt‰isi talletetaanko
	{
        CSaveDataDlg dlg;
        if(dlg.DoModal() != IDOK)
			return false;
	}
	if(theSmartInfo && (theFileName != NFmiString())) // MITƒ JOS EI OLE NIMEƒ!!!!!!!
	{
		NFmiQueryData *data = dynamic_cast<NFmiOwnerInfo*>(theSmartInfo.get())->DataReference().get();
		bool status = false;
		if(itsHelpEditorSystem.UsedHelpEditorStatus())
		{ // pit‰‰ muuttaa help-datan tuottaja id, mutta kopioon!
			NFmiQueryData helpData(*data);
			helpData.Info()->SetProducer(NFmiProducer(NFmiProducerSystem::gHelpEditorDataProdId, "HelpData"));
			status = StoreData(true, theFileName, &helpData, true); // t‰m‰n talletukseen on laitettava tmp-file v‰livaihe, koska t‰h‰n help-dataan muut SmartMetit voi tarrata heti kiinni
		}
        else
        {
            // Laitetaan normaali talletus myˆs ensin lokaali tmp-tiedostoon josta kopiointi win32 funktiolla serverille, miss‰ rename lopulliseksi.
            // T‰m‰ siksi ett‰ nyt myˆs normaali talletus on alkanut tˆkkim‰‰n ja se saattaa kest‰‰ kymmeni‰ minuutteja.
            status = StoreData(true, theFileName, data, true);
        }
		if(status)
			dynamic_cast<NFmiSmartInfo*>(theSmartInfo.get())->Dirty(false);
		return status;
	}
	return false;
}

bool TryAutoStartUpLoad(void)
{
	return FmiModifyEditdData::TryAutoStartUpLoad(GenDocDataAdapter(), UseMultithreaddingWithModifyingFunctions());
}

bool GetProducerIdsLister(NFmiQueryInfo *theInfo, NFmiProducerIdLister &theProducerIdsLister)
{
	if(theInfo)
		if(theInfo->FindFirstKey(NFmiQueryDataUtil::GetOfficialQueryDataProdIdsKey()))
			return theProducerIdsLister.IntepretProducerIdString(string(theInfo->GetCurrentKeyValue()));
	return false;
}

// Tekee producerIdListan ladatun datan mukaisesti.
// Tallettaa sen datan infon headeriin ProdIds-avaimen alle
void PutProducerIdListInDataHeader(boost::shared_ptr<NFmiFastQueryInfo> &theSmartInfo, checkedVector<NFmiFastQueryInfo*> &theSourceInfos, checkedVector<int> &theModelIndexVector)
{
	NFmiProducerIdLister workingDataIds;
	bool workingDataIdsExist = GetProducerIdsLister(theSourceInfos[2], workingDataIds); // 2=working data indeksi
	NFmiProducerIdLister officialDataIds;
	bool officialDataIdsExist = GetProducerIdsLister(theSourceInfos[3], officialDataIds); // 3=virallinen data indeksi
	NFmiProducerIdLister prodIdLister(theSmartInfo->TimeDescriptor(), -1);
	int counterSize = static_cast<int>(theModelIndexVector.size());
	for(int i=0; i<counterSize; i++)
	{
		int prodId = -1;
		NFmiMetTime origTime(1900, 1, 1, 0, 0);
		int currentModelIndex = theModelIndexVector[i];
		if(currentModelIndex >= 0 && currentModelIndex < static_cast<int>(theSourceInfos.size()))
			if(theSourceInfos[currentModelIndex])
			{
				prodId = theSourceInfos[currentModelIndex]->Producer()->GetIdent();
				origTime = theSourceInfos[currentModelIndex]->OriginTime();
			}
		if(prodId != kFmiMETEOR)
		{
			prodIdLister.ProducerId(i, prodId);
			prodIdLister.ModelOriginTime(i, origTime);
		}
		else // tuottaja on meteor, nyt tutkitaan loytyyko datasta tuotttaja id tietoa
		{
			prodIdLister.ProducerId(i, -1); // turhaa?? // asetetaan valmiiksi puuttuva arvo, joka overridataan jos loytyy oikea id
			if(theSmartInfo->TimeIndex(i))
			{
				if(theModelIndexVector[i] == 2)
				{
					if(workingDataIdsExist)
					{
						prodIdLister.ProducerId(i, workingDataIds.ProducerId(theSmartInfo->Time(), true));
						prodIdLister.ModelOriginTime(i, workingDataIds.ModelOriginTime(theSmartInfo->Time(), true));
					}
				}
				else if(theModelIndexVector[i] == 3)
				{
					if(officialDataIdsExist)
					{
						prodIdLister.ProducerId(i, officialDataIds.ProducerId(theSmartInfo->Time(), true));
						prodIdLister.ModelOriginTime(i, officialDataIds.ModelOriginTime(theSmartInfo->Time(), true));
					}
				}
			}
		}
	}

	theSmartInfo->AddKey(NFmiQueryDataUtil::GetOfficialQueryDataProdIdsKey(), prodIdLister.MakeProducerIdString(), true);
	itsProducerIdLister = prodIdLister;

}

boost::shared_ptr<NFmiEditorControlPointManager> CPManager(bool getOldSchoolCPManager = false)
{
	return itsCPManagerSet.CPManager(getOldSchoolCPManager);
}

bool CreateCPPopup()
{
	delete itsPopupMenu;
	itsPopupMenu = 0;
	fOpenPopup = false;
	if(itsMetEditorOptionsData.ControlPointMode())
	{
		NFmiInfoData::Type infoDataType = NFmiInfoData::kEditable;
		itsPopupMenu = new NFmiMenuItemList;

		std::string menuString = ::GetDictionaryString("ControlPointPopUpActivate");
        auto menuItem = std::make_unique<NFmiMenuItem>(-1, menuString, NFmiDataIdent(), kFmiActivateCP, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, infoDataType);
		itsPopupMenu->Add(std::move(menuItem));
		menuString = ::GetDictionaryString("ControlPointPopUpDeactivate");
		menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiDeactivateCP, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, infoDataType)); 
		itsPopupMenu->Add(std::move(menuItem));
		menuString = ::GetDictionaryString("ControlPointPopUpUse");
		menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiEnableCP, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, infoDataType));
		itsPopupMenu->Add(std::move(menuItem));
		menuString = ::GetDictionaryString("ControlPointPopUpDontUse");
		menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiDisableCP, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, infoDataType));
		itsPopupMenu->Add(std::move(menuItem));

		menuString = ::GetDictionaryString("ControlPointPopUpAllwaysInTimeSerialView");
		menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiShowCPAllwaysOnTimeView, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, infoDataType));
		itsPopupMenu->Add(std::move(menuItem));
		menuString = ::GetDictionaryString("ControlPointPopUpNotAllwaysInTimeSerialView");
		menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiDontShowCPAllwaysOnTimeView, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, infoDataType));
		itsPopupMenu->Add(std::move(menuItem));
		menuString = ::GetDictionaryString("ControlPointPopUpAllPointsInTimeView");
		menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiShowAllCPsAllwaysOnTimeView, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, infoDataType));
		itsPopupMenu->Add(std::move(menuItem));
		menuString = ::GetDictionaryString("ControlPointPopUpOnlyActiveInTimeView");
		menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiShowOnlyActiveCPOnTimeView, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, infoDataType));
		itsPopupMenu->Add(std::move(menuItem));

		menuString = ::GetDictionaryString("ControlPointPopUpModify");
		menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiModifyCPAttributes, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, infoDataType));
		itsPopupMenu->Add(std::move(menuItem));

        AddCpManagerSetsToCpPopupMenu(itsPopupMenu, infoDataType);
        AddObservationStationsToCpPointsCommands(itsPopupMenu);

		if(!itsPopupMenu->InitializeCommandIDs(itsPopupMenuStartId))
			return false;
		fOpenPopup = true;
		return true;
	}
	return false;
}

// Havaintodatan asemista voidaan tehd‰ ControlPoint pisteet
void AddObservationStationsToCpPointsCommands(NFmiMenuItemList *mainPopupMenu)
{
    auto &cpObsBlendingData = AnalyzeToolData().ControlPointObservationBlendingData();
    cpObsBlendingData.SeekProducers(*InfoOrganizer());
    AddObservationStationsToCpPointsCommands(mainPopupMenu, cpObsBlendingData.Producers(), NFmiInfoData::kObservations);
}

void AddObservationStationsToCpPointsCommands(NFmiMenuItemList *mainPopupMenu, const checkedVector<NFmiProducer> &producerList, NFmiInfoData::Type usedInfoData)
{
    if(!producerList.empty())
    {
        NFmiParam dummyParam;
        NFmiMenuItemList *producerMenuList = new NFmiMenuItemList;
        for(const auto &producer : producerList)
        {
            auto menuItem = std::make_unique<NFmiMenuItem>(-1, std::string(producer.GetName()), NFmiDataIdent(dummyParam, producer), kFmiObservationStationsToCpPoints, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, usedInfoData);
            producerMenuList->Add(std::move(menuItem));
        }

        if(producerMenuList->NumberOfMenuItems())
        {
            std::string menuString = ::GetDictionaryString("Observations To CP points");
            auto menuItem = std::make_unique<NFmiMenuItem>(-1, menuString, NFmiDataIdent(), kFmiObservationStationsToCpPoints, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, usedInfoData);
            menuItem->AddSubMenu(producerMenuList);
            mainPopupMenu->Add(std::move(menuItem));
        }
        else
            delete producerMenuList;
    }
}

void MakeObservationStationsToCpPoints(NFmiMenuItem &menuItem)
{
    // Haetaan k‰ytˆss‰ oleva CP-manager
    auto controlPointManager = itsCPManagerSet.CPManager(itsCPManagerSet.UseOldSchoolStyle());
    if(controlPointManager)
    {
        // Haetaan k‰ytetyn datatyypin, halutun tuottajan kaikki tiedostot, jotka ovat pinta datoja (= leveleit‰ on vain 1)
        auto producerId = menuItem.DataIdent().GetProducer()->GetIdent();
        auto observationInfos = InfoOrganizer()->GetInfos(menuItem.DataType(), true, producerId);
        controlPointManager->SetZoomedAreaStationsAsControlPoints(observationInfos, MapViewDescTop(0)->MapHandler()->Area());
        AnalyzeToolData().ControlPointObservationBlendingData().SelectProducer(producerId);

        // Vain aikasarja pit‰‰ laittaa t‰ss‰ likaiseksi, sielt‰ mist‰ t‰t‰ kutsutaan (MakePopUpCommandUsingRowIndex) laitetaan karttan‰ytˆt likaisiksi.
        TimeSerialViewDirty(true);
    }
}

void AddCpManagerSetsToCpPopupMenu(NFmiMenuItemList *mainPopupMenu, NFmiInfoData::Type infoDataType)
{
    // Lis‰t‰‰n mahdollisen CPManagerSetin valinnat
    size_t cpSetSize = itsCPManagerSet.CPSetSize();
    if(cpSetSize > 1)
    {
        NFmiMenuItemList *cpManagerMenuList = new NFmiMenuItemList;
        for(size_t i = 0; i < cpSetSize; i++)
        {
            boost::shared_ptr<NFmiEditorControlPointManager> cpManager = itsCPManagerSet.CPManagerFromSet(i);
            if(cpManager)
            {
                auto cpMenuItem = std::make_unique<NFmiMenuItem>(-1, cpManager->Name(), NFmiDataIdent(), kFmiSelectCPManagerFromSet, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, infoDataType, static_cast<int>(i));
                cpManagerMenuList->Add(std::move(cpMenuItem));
            }
        }

        if(cpManagerMenuList->NumberOfMenuItems())
        {
            std::string menuString = ::GetDictionaryString("CPManagers");
            auto menuItem = std::make_unique<NFmiMenuItem>(-1, menuString, NFmiDataIdent(), kFmiSelectCPManagerFromSet, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, infoDataType);
            menuItem->AddSubMenu(cpManagerMenuList);
            mainPopupMenu->Add(std::move(menuItem));
        }
        else
            delete cpManagerMenuList;
    }
}

// nelj‰ tilaa:
// 0 = n‰yt‰ aikakontrolliikkuna+teksti
// 1=vain aik.kont.ikkuna
// 2=‰l‰ n‰yt‰ kumpaakaan
// 3= n‰yt‰ vain teksti
// palauttaa currentin tilan
int ToggleShowTimeOnMapMode(unsigned int theDescTopIndex)
{
	return MapViewDescTop(theDescTopIndex)->ToggleShowTimeOnMapMode();
}

// aikasarja ikkunat asetetaan samalla likaiseksi kuin karttan‰yttˆkin (ainakin toistaiseksi)
bool TimeSerialViewDirty(void)
{return fTimeSerialViewDirty;}

void TimeSerialViewDirty(bool newValue)
{fTimeSerialViewDirty = newValue;}

bool IsToolMasterAvailable(void) const
{return itsBasicConfigurations.IsToolMasterAvailable();}

void ToolMasterAvailable(bool newValue)
{
    itsBasicConfigurations.ToolMasterAvailable(newValue);
}

// Tallettaa mm. CP pisteet, muutosk‰yr‰t jne.
// T‰t‰ kutsutaan kun editori suljetaan.
void StoreSupplementaryData(void)
{
	// armeija / military / CD rom editor versiossa on try - catch blokki k‰ytˆss‰!!
	// t‰m‰ on vain pikaviritys sellaiselle editorille, joka ei voi tallettaa asetuksia.
	// Koko t‰m‰ try - catch koodi on normaalisti kommenteissa!!!!
	try
	{
        DoConfigurationsCanBeSavedCheck(true);

		::_chdir(itsBasicConfigurations.WorkingDirectory().c_str()); // palautetaan alkuper‰inen tyˆhakemisto voimaan talletuksia varten
		itsMTATempSystem.StoreSettings();
        // N‰m‰ sounding dialogin asetukset pit‰‰ ottaa MTATempSystem:ist‰ takaisin Win-registeriin, koska niit‰ on mahdollisesti latailtu n‰yttˆmakroista
        ApplicationWinRegistry().SoundingTextUpward(itsMTATempSystem.SoundingTextUpward());
        ApplicationWinRegistry().SoundingTimeLockWithMapView(itsMTATempSystem.SoundingTimeLockWithMapView());

        itsTrajectorySystem->StoreSettings();
		StoreMapViewDescTopToSettings();
		StoreAnalyzeToolData();
		StoreDataQualityChecker();
		IgnoreStationsData().StoreToSettings();
		StoreOptionsData(); // t‰m‰ tekee myˆs asetuksien talletuksen konfiguraatio tiedostoihin! T‰m‰ pit‰‰ siis kutsua viimeisen‰.
		DoMacroDirectoriesSyncronization(); // laitetaan makrojen synkronointi viel‰ lopuksi k‰yntiin, ett‰ mahdolliset muutokset menev‰t serverille

		CheckRunningStatusAtClosing(SpecialFileStoragePath());
		StoreAllCPDataToFiles();
		ReportInfoOrganizerDataConsumption();
        StoreBackUpViewMacro(true);
#ifndef DISABLE_CPPRESTSDK
        itsWarningCenterSystem.getLegacyData().StoreSettings();
#endif // DISABLE_CPPRESTSDK
	}
	catch(std::exception &e)
	{
		std::string problemStr("Problems when trying to save the settings to the settings files:\n");
		problemStr += e.what();
		LogAndWarnUser(problemStr, "Problems when saving settings.", CatLog::Severity::Error, CatLog::Category::Configuration, false);
	}
	catch(...)
	{
		std::string problemStr("Unknown problems when trying to save the settings to the settings files.");
		LogAndWarnUser(problemStr, "Problems when saving settings.", CatLog::Severity::Error, CatLog::Category::Configuration, false);
	}
}

void LogMessage(const std::string& theMessage, CatLog::Severity severity, CatLog::Category category, bool flushLogger = false)
{
    itsBasicConfigurations.LogMessage(theMessage, severity, category, flushLogger);
}

void LogAndWarnUser(const std::string &theMessageStr, const std::string &theDialogTitleStr, CatLog::Severity severity, CatLog::Category category, bool justLog, bool addAbortOption = false, bool flushLogger = false)
{
    itsBasicConfigurations.LogAndWarnUser(theMessageStr, theDialogTitleStr, severity, category, justLog, addAbortOption, flushLogger);
}

bool StoreAllCPDataToFiles(void)
{
	return itsCPManagerSet.StoreAllCurrentCPDataToFiles();
}

// t‰m‰ tiet‰‰ miten lat-lon-viivoja piirret‰‰n karttojen ylle
NFmiProjectionCurvatureInfo* ProjectionCurvatureInfo(void)
{return itsProjectionCurvatureInfo;}
void ProjectionCurvatureInfo(const NFmiProjectionCurvatureInfo& newValue)
{
	std::string oldBaseNameBaseStr = itsProjectionCurvatureInfo->BaseNameSpaceStr();
	*itsProjectionCurvatureInfo = newValue;
	if(itsProjectionCurvatureInfo->BaseNameSpaceStr().empty())
		itsProjectionCurvatureInfo->BaseNameSpaceStr(oldBaseNameBaseStr); // Pakko asettaa conffi-nimiavuruus t‰ss‰, jos newValue:ssa se oli puuttuva, koska muuten 
																			// ei voi muuttaa projektio asetuksia sen j‰lkeen kun on ladattu n‰yttˆmakro
}

void SetDataLoadingProducerIndexVector(const checkedVector<int>& theIndexVector)
{
	itsDataLoadingProducerIndexVector = theIndexVector;
}

void RemoveAllParamsFromMapViewRow(unsigned int theDescTopIndex, int theRowIndex)
{
	RemoveAllViewsWithRealRowNumber(theDescTopIndex, theRowIndex);
}

int TimeEditSmootherValue(void)
{
	return itsTimeEditSmootherValue;
}
void TimeEditSmootherValue(int newValue)
{
	itsTimeEditSmootherValue = newValue;
}
int TimeEditSmootherMaxValue(void)
{
	return itsTimeEditSmootherMaxValue;
}
void TimeEditSmootherMaxValue(int newValue)
{
	itsTimeEditSmootherMaxValue = newValue;
}
int BrushToolLimitSetting(void)
{
	return itsBrushToolLimitSetting;
}
void BrushToolLimitSetting(int newValue)
{
	itsBrushToolLimitSetting = newValue;
}
float BrushToolLimitSettingValue(void)
{
	return itsBrushToolLimitSettingValue;
}
void BrushToolLimitSettingValue(float newValue)
{
	itsBrushToolLimitSettingValue = newValue;
}

int ShowHelperDataInTimeSerialView(void)
{
	return itsShowHelperDataInTimeSerialView;
}
void ShowHelperDataInTimeSerialView(int newValue)
{
	itsShowHelperDataInTimeSerialView = newValue;
}

void SetBitOn(int &value, int nthBit)
{
    value |= 1 << nthBit;
}

void SetBitOff(int &value, int nthBit)
{
    value &= ~(1 << nthBit);
}

bool IsBitSet(int value, int nthBit)
{
    return (value >> nthBit) & 1;
}

bool ShowHelperData1InTimeSerialView()
{
    return IsBitSet(itsShowHelperDataInTimeSerialView, 0);
}

void SetOnShowHelperData1InTimeSerialView()
{
    SetBitOn(itsShowHelperDataInTimeSerialView, 0);
}

void SetOffShowHelperData1InTimeSerialView()
{
    SetBitOff(itsShowHelperDataInTimeSerialView, 0);
}

bool ShowHelperData2InTimeSerialView()
{
    return IsBitSet(itsShowHelperDataInTimeSerialView, 1);
}

void SetOnShowHelperData2InTimeSerialView()
{
    SetBitOn(itsShowHelperDataInTimeSerialView, 1);
}

void SetOffShowHelperData2InTimeSerialView()
{
    SetBitOff(itsShowHelperDataInTimeSerialView, 1);
}

bool ShowHelperData3InTimeSerialView()
{
    return fShowHelperData3InTimeSerialView;
}

void ShowHelperData3InTimeSerialView(bool newValue)
{
    fShowHelperData3InTimeSerialView = newValue;
}

bool ShowHelperData4InTimeSerialView()
{
    return fShowHelperData4InTimeSerialView;
}

void ShowHelperData4InTimeSerialView(bool newValue)
{
    fShowHelperData4InTimeSerialView = newValue;
}

NFmiInfoOrganizer* InfoOrganizer(void)
{
	return itsSmartInfoOrganizer;
}

bool IsOperationalModeOn(void)
{
	NFmiMetEditorModeDataWCTR *editorMode = EditorModeDataWCTR();
	if(editorMode && editorMode->EditorMode() != NFmiMetEditorModeDataWCTR::kNormal)
		return true;
	else
		return false;
}

const string& EditorVersionStr(void)
{
	return itsBasicConfigurations.EditorVersionStr();
}

bool InitCPManagerSet(void)
{
    DoVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
        itsCPManagerSet.InitFromSettings("SmartMet::CPManagerSet", itsBasicConfigurations.ControlPath(), SpecialFileStoragePath());
		return true;
	}
	catch(std::exception &e)
	{
		LogAndWarnUser(e.what(), "Error while initializing CPManagerSet", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
	return false;
}


	std::string& SmartToolEditingErrorText(void)
	{
		return itsSmartToolEditingErrorText;
	}

	bool InitSmartToolInfo(void)
	{
        DoVerboseFunctionStartingLogReporting(__FUNCTION__);
		bool status = itsSmartToolInfo.Init(itsMacroPathSettings.SmartToolPath(true));
		NFmiFileSystem::CreateDirectory(itsSmartToolInfo.LoadDirectory());
		if(!status)
		{
			string errStr("SmartTool-ohjausta ei saatu luettua asetuksista.");
			LogMessage(errStr, CatLog::Severity::Error, CatLog::Category::Configuration);
		}
		string fileName2(itsBasicConfigurations.ControlPath());
		fileName2 += "DBChecker.st";
		itsSmartToolInfo.DBCheckerFileName(fileName2);
		bool status2 = itsSmartToolInfo.LoadDBChecker();
		SetCurrentSmartToolMacro(itsSmartToolInfo.CurrentScript()); // laitetaan currentti skripti myˆs dociin

        // Alustetaan myˆs yksi smartTool kieleen liittyv‰t callback funktiot
        NFmiInfoAreaMaskOccurrance::SetMultiSourceDataGetterCallback([this](checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector, boost::shared_ptr<NFmiDrawParam> &theDrawParam, const boost::shared_ptr<NFmiArea> &theArea) {this->MakeDrawedInfoVectorForMapView(theInfoVector, theDrawParam, theArea); });

		return status && status2;
	}

	NFmiSmartToolInfo* SmartToolInfo(void){return &itsSmartToolInfo;}

	std::string GetViewMacroFileName(const std::string &theName)
	{
		string fileName(itsViewMacroPath);
        PathUtils::addDirectorySeparatorAtEnd(fileName);
		fileName += theName;
		fileName += ".vmr"; // vmr = ViewMacRo
		return fileName;
	}
	std::string GetViewMacroTotalFileName(const std::string &theFileName)
	{
		string fileName(itsViewMacroPath);
        PathUtils::addDirectorySeparatorAtEnd(fileName);
        fileName += theFileName;
		return fileName;
	}
	std::string GetViewMacroFilePattern(void)
	{
		string filePattern(itsViewMacroPath);
        PathUtils::addDirectorySeparatorAtEnd(filePattern);
        filePattern += "*.vmr"; // vmr = ViewMacRo
		return filePattern;
	}

	void FillGeneralDocMacro(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::GeneralDoc &gDoc = theMacro.GetGeneralDoc();
		gDoc.ProjectionCurvatureInfo(*ProjectionCurvatureInfo());
		gDoc.CPLocationVector(CPManager()->CPLocationVector());
	}

	void FillTimeViewMacro(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::TimeView &timeView = theMacro.GetTimeView();
		timeView.SetAllParams(itsTimeSerialViewDrawParamList);
        timeView.ShowHelpData(ShowHelperDataInTimeSerialView() != 0);
        timeView.ShowHelpData2(ShowHelperData2InTimeSerialView());
        timeView.ShowHelpData3(ShowHelperData3InTimeSerialView());
        timeView.ShowHelpData4(ShowHelperData4InTimeSerialView());

		boost::shared_ptr<NFmiFastQueryInfo> editedInfo = EditedInfo();
		if(editedInfo)
		{
			// n‰m‰ offsetit ovat oikeasti obsoliittej‰
			int startOffset = TimeControlViewTimes(0).FirstTime().DifferenceInMinutes(editedInfo->TimeDescriptor().FirstTime());
			int endOffset = TimeControlViewTimes(0).LastTime().DifferenceInMinutes(editedInfo->TimeDescriptor().LastTime());
			timeView.StartTimeOffset(startOffset);
			timeView.EndTimeOffset(endOffset);
		}
		timeView.TimeBag(TimeSerialViewTimeBag());
	}

	void FillTempViewMacro(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::TempView &tempView = theMacro.GetTempView();

		tempView.ShowEcmwf(false);
		tempView.ShowHirlam(false);

		tempView.MTATempSystem(itsMTATempSystem);
	}

	void FillTrajectoryViewMacro(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::TrajectoryView &trajectoryView = theMacro.GetTrajectoryView();

		trajectoryView.TrajectorySystem(*itsTrajectorySystem);
	}

	void FillCrossSectionViewMacro(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::CrossSectionView &view = theMacro.GetCrossSectionView();

		view.CrossSectionSystem(itsCrossSectionSystem);
		view.SetAllRowParams(itsCrossSectionDrawParamListVector, itsMacroParamSystem);
	}

	void FillExtraMapViewSettingMacro2(NFmiViewSettingMacro::MapViewDescTop &theViewMacro, NFmiMapViewDescTop &theDescTop, int theDescTopIndex)
	{
        boost::shared_ptr<NFmiMapViewWinRegistry> mapViewWinRegistry = ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(theDescTopIndex);
        theViewMacro.SetMapViewDescTop(theDescTop, *mapViewWinRegistry.get());
		theViewMacro.SetAllRowParams(theDescTop.DrawParamListVector(), itsMacroParamSystem);
		theViewMacro.DipMapHelperList(theDescTop.GetViewMacroDipMapHelperList());
	}

	void FillExtraMapViewSettingsMacro(NFmiViewSettingMacro &theMacro)
	{
		std::vector<NFmiViewSettingMacro::MapViewDescTop> &extraMapViewDescTops = theMacro.ExtraMapViewDescTops();
		extraMapViewDescTops.resize(MapViewDescTopList().size()); // varmuuden vuoksi koon asetus

		for(size_t i = 0; i < MapViewDescTopList().size(); i++)
			FillExtraMapViewSettingMacro2(extraMapViewDescTops[i], *MapViewDescTop(static_cast<unsigned int>(i)), static_cast<int>(i));
	}

	void FillWarningCenterViewMacro(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::WarningCenterView &view = theMacro.GetWarningCenterView();

#ifndef DISABLE_CPPRESTSDK
		view.WarningCenterSystem(itsWarningCenterSystem.getLegacyData());
#endif // DISABLE_CPPRESTSDK
        view.ShowHakeMessages(ApplicationWinRegistry().ShowHakeMessages());
        view.ShowKaHaMessages(ApplicationWinRegistry().ShowKaHaMessages());
        view.MinimumTimeRangeForWarningsOnMapViewsInMinutes(ApplicationWinRegistry().MinimumTimeRangeForWarningsOnMapViewsInMinutes());
	}

#ifdef max
#undef max
#endif

    int GetTimeRangeForWarningMessagesOnMapViewInMinutes()
    {
        int timeStepInMapView = static_cast<int>(::round(MapViewDescTop(0)->TimeControlTimeStep() * 60));
        int minimumTimeRange = ApplicationWinRegistry().MinimumTimeRangeForWarningsOnMapViewsInMinutes();
        if(minimumTimeRange <= 0)
            return timeStepInMapView;
        else
            return std::max(timeStepInMapView, minimumTimeRange);
    }

	void FillSynopPlotSettingsMacro(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::SynopPlotSettings &settings = theMacro.GetSynopPlotSettings();

		settings.SetSynopPlotSettings(itsSynopPlotSettings);
	}

	void FillObsComparisonMacro(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::ObsComparisonInfo &settings = theMacro.GetObsComparisonInfo();

		settings.SetObsComparisonInfo(itsObsComparisonInfo);
	}

	void FillMaskSettingsMacro(NFmiViewSettingMacro &theMacro)
	{
        boost::shared_ptr<NFmiMapViewWinRegistry> mapViewWinRegistry0 = ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(0); // t‰m‰ on p‰‰karttaikkunan asetuksia
		NFmiViewSettingMacro::MaskSettings &masks = theMacro.GetMaskSettings();
		boost::shared_ptr<NFmiAreaMaskList> paramMaskList(ParamMaskListMT());
		masks.SetAllMasks(*paramMaskList);
		masks.ShowMasksOnMapView(mapViewWinRegistry0->ShowMasksOnMap());
		masks.UseMasksInTimeSerialViews(fUseMasksInTimeSerialViews);
		masks.UseMasksWithFilterTool(fUseMasksWithFilterTool);
		masks.UseMaskWithBrush(fUseMaskWithBrush);
	}

	void FillViewMacro(NFmiViewSettingMacro &theMacro, const std::string &theName, const std::string &theDescription)
	{ // t‰yt‰ makro editorin n‰yttˆ asetuksilla
		theMacro.Name(theName);
		theMacro.Description(theDescription);
		FillGeneralDocMacro(theMacro);
		FillTimeViewMacro(theMacro);
		FillTempViewMacro(theMacro);
		FillTrajectoryViewMacro(theMacro);
		FillCrossSectionViewMacro(theMacro);
		FillWarningCenterViewMacro(theMacro);
		FillSynopPlotSettingsMacro(theMacro);
		FillExtraMapViewSettingsMacro(theMacro);
		// SynopDataGridView hoidetaan CSmartMetDoc-luokassa
		FillObsComparisonMacro(theMacro);
		FillMaskSettingsMacro(theMacro);
		theMacro.UseAnimationTool(false); //DoAnimation());
		theMacro.AnimationDelayInMS(200); //MetEditorOptionsData().AnimationFrameDelayInMSec());
		int animationStartOffset = 0; //itsAnimationStartTime.DifferenceInMinutes(TimeControlViewTimes(0).FirstTime());
		theMacro.AnimationStartPosition(animationStartOffset);
		int animationEndOffset = 0; //itsAnimationEndTime.DifferenceInMinutes(TimeControlViewTimes(0).LastTime());
		theMacro.AnimationEndPosition(animationEndOffset);
        theMacro.KeepMapAspectRatio(ApplicationWinRegistry().KeepMapAspectRatio());
    }

	void SetGeneralDoc(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::GeneralDoc &gDoc = theMacro.GetGeneralDoc();
		ProjectionCurvatureInfo(gDoc.ProjectionCurvatureInfo());
		CPManager(true)->Init(gDoc.CPLocationVector()); // viewMacron CP-pisteet asetetaan ensin 'old-school' CP-manageriin (CPManager(true))
		itsCPManagerSet.UpdateViewMacroCPManager(CPManager(true)); // sitten laitetaan se viel‰ ns. viewmacro-CPManageriksi
	}

	void AdjustDrawParam(boost::shared_ptr<NFmiDrawParam> &theDrawParam, const NFmiViewSettingMacro::Param &theParam)
	{
		if(theDrawParam)
		{
			if(theDrawParam->ParameterAbbreviation() == gDummyParamName)
			{ 
				// HUOM! T‰m‰ on oikea purkka viritys. Ongelma on nyt siin‰ ett‰ viewmakrojen makroParamit voivat ep‰onnistua, jos macroParam-threadi ei ole viel‰ ehtinyt lukea macroParameja ollenkaan.
				// T‰st‰ seuraa ett‰ viemakrossa ollut makroParami ei tule oikein alustettua. gDummyParamName-vakion avulla varmistan ett‰ theDrawParamia ei ole alustettu oikein.
				// En uskalla tehd‰ koodia niin ett‰ aina teht‰isiin t‰m‰ asetus, koska se luultavasti rikkoisi jotain, siksi else-haara on tuossa per‰ss‰.
				// TODO: koko t‰m‰ drawParam + macroParam + viewMacro alustelut ja muut s‰‰dˆt pit‰isi koodata uudestaan (se on ik‰v‰ kyll‰ iso ja vaivaloinen tyˆ).
				theDrawParam->Init(theParam.DrawParam(), false);
				theDrawParam->DataType(theParam.DataType());
				theDrawParam->HideParam(theParam.Hidden());
				theDrawParam->MacroParamRelativePath(theParam.DrawParam()->MacroParamRelativePath());
			}
			else
			{
				theDrawParam->Init(theParam.DrawParam(), true); // HUOM! pit‰isikˆ t‰ss‰ k‰ytt‰‰ sittenkin false-arvoa, ett‰ myˆs ei piirto-ominaisuudet initialisoituvat?!?!?!
				theDrawParam->InitFileName(theParam.DrawParam()->InitFileName());
				theDrawParam->Activate(theParam.Active());
				theDrawParam->HideParam(theParam.Hidden());
				theDrawParam->ShowDifference(theParam.ShowTimeDifference());
				theDrawParam->ShowDifferenceToOriginalData(theParam.ShowDifferenceToOriginalData());

				// Joudun lis‰‰m‰‰n n‰m‰ alustukset t‰h‰n, koska niit‰ ei initialisoida theDrawParam->Init(&theParam.DrawParam(), true) -rivill‰ true-parametri arvon takia!!!!
				theDrawParam->ModelRunIndex(theParam.DrawParam()->ModelRunIndex());
				theDrawParam->ModelOriginTime(theParam.DrawParam()->ModelOriginTime());
				theDrawParam->TimeSerialModelRunCount(theParam.DrawParam()->TimeSerialModelRunCount());
                theDrawParam->ParameterAbbreviation(theParam.DrawParam()->ParameterAbbreviation());

                // T‰m‰ on ik‰v‰ viritelm‰, mutta n‰in est‰n macroParamia lataamasta piirtoasetuksia tiedostosta (kuten normaalisti macroOParamien kanssa tehd‰‰n)
                // ja lis‰ksi tietyt toiminnot eiv‰t saa viewMcro asetuksia p‰‰lle (esim. SmartMetin F12 ja CTRL + F12 lataukset). ViewMacro lippu p‰‰ll‰ piiirto-ominaisuus
                // dialogi toimii erilailla kuin normaalisti...
                theDrawParam->UseViewMacrosSettingsForMacroParam(true);
			}
		}
	}

	void AdjustDrawParam(unsigned int theDescTopIndex, const NFmiViewSettingMacro::Param &theParam, int theRowIndex, int paramIndexInRow)
	{
		NFmiDrawParamList* dList = DrawParamListWithRealRowNumber(theDescTopIndex, theRowIndex);
		if(dList)
		{
			boost::shared_ptr<NFmiDrawParam> dPar;
			if(theRowIndex == gActiveViewRowIndexForTimeSerialView)
			{
				// jos data on editoitavaa, silloin ei ole v‰li‰ makroon s‰ilytetyist‰ tuottajista jne. jolloin Find:in viimeinen parametri on true
				if(dList->Find(theParam.DataIdent(), &theParam.Level(), theParam.DataType(), (theParam.DataType() == NFmiInfoData::kEditable)))
					dPar = dList->Current();
			}
			else
			{
				if(dList->Index(paramIndexInRow))
				{
					dPar = dList->Current();
				}
			}
			AdjustDrawParam(dPar, theParam);
		}
	}

	void AdjustCrossSectionDrawParam(const NFmiViewSettingMacro::Param &theParam, int theRowIndex, int paramIndexInRow)
	{
		NFmiDrawParamList* dList = this->CrossSectionViewDrawParamList(theRowIndex);
		if(dList)
		{
			boost::shared_ptr<NFmiDrawParam> dPar;
			if(dList->Index(paramIndexInRow))
				dPar = dList->Current();
			AdjustDrawParam(dPar, theParam);
		}
	}

    void SetTimeViewParams(NFmiViewSettingMacro &theMacro, bool fTreatAsViewMacro)
	{
		RemoveAllTimeSerialViews();

		const checkedVector<NFmiViewSettingMacro::TimeViewRow>& rows = theMacro.GetTimeView().Rows();
		checkedVector<NFmiViewSettingMacro::TimeViewRow>::size_type ssize = rows.size();
		checkedVector<NFmiViewSettingMacro::TimeViewRow>::size_type counter = 0;
		itsTimeSerialViewIndex = 0;
		for( ;counter < ssize ; counter++)
		{
			itsTimeSerialViewIndex++; // t‰t‰ juoksuttamalla saan parametrit menem‰‰n aikasarja ikkunaan oikeaan j‰rjestykseen
			const NFmiViewSettingMacro::Param &par = rows[counter].Param();
			NFmiMenuItem menuItem(-1, "x", par.DataIdent(), kFmiAddTimeSerialView, NFmiMetEditorTypes::kFmiParamsDefaultView, &par.Level(), par.DataType());
            AddTimeSerialView(menuItem, fTreatAsViewMacro);
			AdjustDrawParam(0, par, gActiveViewRowIndexForTimeSerialView, 0); // kaikki makroon talletetut drawparam asetukset pit‰‰ viel‰ ladata luotuun ja listoihin laitettuun drawparamiin
																			// viimeinen 0 on feikki indeksi jota tarvitaan karttan‰yttˆ tapauksessa
		}

        if(theMacro.GetTimeView().ShowHelpData())
            SetOnShowHelperData1InTimeSerialView();
        else
            SetOffShowHelperData1InTimeSerialView();
        if(theMacro.GetTimeView().ShowHelpData2())
            SetOnShowHelperData2InTimeSerialView();
        else
            SetOffShowHelperData2InTimeSerialView();
        ShowHelperData3InTimeSerialView(theMacro.GetTimeView().ShowHelpData3());
        ShowHelperData4InTimeSerialView(theMacro.GetTimeView().ShowHelpData4());

		boost::shared_ptr<NFmiFastQueryInfo> editedInfo = EditedInfo();
		if(editedInfo)
		{
			// n‰m‰ offset jutu ovat oikeasti obsoliitteja ja niit‰ ei en‰‰ k‰ytet‰
			int startOffset = theMacro.GetTimeView().StartTimeOffset();
			int endOffset = theMacro.GetTimeView().EndTimeOffset();
			NFmiMetTime startTime(editedInfo->TimeDescriptor().FirstTime());
			startTime.ChangeByMinutes(startOffset);
			NFmiMetTime endTime(editedInfo->TimeDescriptor().LastTime());
			endTime.ChangeByMinutes(endOffset);
			NFmiTimeBag validTimes(startTime, endTime, TimeControlViewTimes(0).Resolution());
			TimeSerialViewTimeBag(validTimes);
		}
		if(theMacro.GetTimeView().TimeBagUpdated())
			TimeSerialViewTimeBag(theMacro.GetTimeView().TimeBag());
	}

	void SetTempView(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::TempView &tempView = theMacro.GetTempView();

		itsMTATempSystem.InitFromViewMacro(tempView.MTATempSystem());
		itsMTATempSystem.UpdateFromViewMacro(true);
	}

	void SetTrajectoryView(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::TrajectoryView &view = theMacro.GetTrajectoryView();

		itsTrajectorySystem->Init(view.TrajectorySystem());
	}

	void SetWarningCenterView(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::WarningCenterView &view = theMacro.GetWarningCenterView();

#ifndef DISABLE_CPPRESTSDK
		itsWarningCenterSystem.getLegacyData().Init(view.WarningCenterSystem());
#endif // DISABLE_CPPRESTSDK
        ApplicationWinRegistry().ShowHakeMessages(view.ShowHakeMessages());
        ApplicationWinRegistry().ShowKaHaMessages(view.ShowKaHaMessages());
        ApplicationWinRegistry().MinimumTimeRangeForWarningsOnMapViewsInMinutes(view.MinimumTimeRangeForWarningsOnMapViewsInMinutes());
    }

	void SetSynopPlotSettings(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::SynopPlotSettings &settings = theMacro.GetSynopPlotSettings();

		itsSynopPlotSettings = settings.GetSynopPlotSettings();
	}

	void SetObsComparisonSettings(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::ObsComparisonInfo &settings = theMacro.GetObsComparisonInfo();

		itsObsComparisonInfo = settings.GetObsComparisonInfo();
	}

    void SetCrossSectionView(NFmiViewSettingMacro &theMacro, bool fTreatAsViewMacro)
	{
		NFmiViewSettingMacro::CrossSectionView &view = theMacro.GetCrossSectionView();

		itsCrossSectionSystem.Init(view.CrossSectionSystem());

        // Pit‰‰ tyhjent‰‰ aluksi poikkileikkausn‰ytˆn kaikkien rivien parametrit, koska sen t‰yttˆ looppia (joka tyhjent‰‰ kulloisenkin rivin ensin) ei v‰ltt‰m‰tt‰ edes ajeta
        ClearDesctopsAllParams(CtrlViewUtils::kFmiCrossSectionView);

		// asetetaan poikkileikkaus parametrit riveilleen
		const checkedVector<NFmiViewSettingMacro::MapRow>& mapRows = theMacro.GetCrossSectionView().MapRowSettings();
		size_t ssize = mapRows.size();
		for(size_t i = 0; i < ssize ; i++)
		{
			NFmiDrawParamList *dList = CrossSectionViewDrawParamList(static_cast<int>(i+1));
			if(dList)
			{
				dList->Clear();
				const checkedVector<NFmiViewSettingMacro::Param>& params = mapRows[i].RowParams();
				size_t psize = params.size();
				for(size_t j=0 ;j < psize ; j++)
				{
					const NFmiViewSettingMacro::Param &par = params[j];
					NFmiMenuItem menuItem(-1, gDummyParamName, par.DataIdent(), kFmiAddParamCrossSectionView, NFmiMetEditorTypes::kFmiParamsDefaultView, &par.Level(), par.DataType());
                    AddCrossSectionView(menuItem, static_cast<int>(i + 1), fTreatAsViewMacro);
					AdjustCrossSectionDrawParam(par, static_cast<int>(i+1), static_cast<int>(j+1)); // kaikki makroon talletetut drawparam asetukset pit‰‰ viel‰ ladata luotuun ja listoihin laitettuun drawparamiin
				}
			}
			else
				throw runtime_error("Error in application. NFmiEditMapGeneralDataDoc::SetCrossSectionView had null pointer drawParamList.");
		}
	}

    NFmiPtrList<NFmiDrawParamList>* DrawParamListVector(unsigned int theDescTopIndex)
    {
        if(theDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
            return itsCrossSectionDrawParamListVector;
        if(theDescTopIndex == CtrlViewUtils::kFmiTimeSerialView)
            return nullptr;
        else
        {
            NFmiMapViewDescTop *descTop = MapViewDescTop(theDescTopIndex);
            if(descTop)
            {
                return descTop->DrawParamListVector();
            }
            else
                return nullptr;
        }
    }

    void ClearDesctopsAllParams(unsigned int theDescTopIndex)
    {
        auto drawParamListVector = DrawParamListVector(theDescTopIndex);
        if(drawParamListVector)
        {
            // Huom! NFmiPtrList:iss‰ indeksit alkavat 1:st‰...
            for(unsigned long rowIndex = 1; rowIndex <= drawParamListVector->NumberOfItems(); rowIndex++)
            {
                auto *drawParamList = drawParamListVector->Index(rowIndex).CurrentPtr();
                if(drawParamList)
                {
                    drawParamList->Clear();
                    MakeViewRowDirtyActions(theDescTopIndex, rowIndex, drawParamList);
                }
            }
        }
    }

    // fTreatAsViewMacro -parametria tarvitaan, koska ns. backupViewMacro:jen latauksien yhteydess‰ halutaan ladata parametreja n‰ytˆille
    // niin ett‰ niit‰ ei pidet‰ viewMacro -paramereina. Jos parametri on ladattu normaalista viewMacrosta, ja halutaan tehd‰ sen piirto-ominaisuuksiin
    // muutoksia, se ei onnistu Piirto-ominaisuus dialogista (dialogissa varoitus ja nappuloiden estot), vaan talletus pit‰‰ tehd‰ itse viewMacroon.
	void SetMapViewRowsParams(const checkedVector<NFmiViewSettingMacro::MapRow>& theMapRows, unsigned int theDescTopIndex, size_t theStartRowIndex, bool fTreatAsViewMacro)
	{
        // Pit‰‰ tyhjent‰‰ aluksi karttan‰ytˆn kaikkien rivien parametrit, koska sen t‰yttˆ looppia (joka tyhjent‰‰ kulloisenkin rivin ensin) ei v‰ltt‰m‰tt‰ edes ajeta
        ClearDesctopsAllParams(theDescTopIndex);

		checkedVector<NFmiViewSettingMacro::MapRow>::size_type ssize = theMapRows.size();
		checkedVector<NFmiViewSettingMacro::MapRow>::size_type counter = theStartRowIndex;
		for( ;counter < ssize + theStartRowIndex ; counter++)
		{
			const checkedVector<NFmiViewSettingMacro::Param>& params = theMapRows[counter - theStartRowIndex].RowParams();
			checkedVector<NFmiViewSettingMacro::Param>::size_type psize = params.size();
			checkedVector<NFmiViewSettingMacro::Param>::size_type counter2 = 0;
			for( ;counter2 < psize ; counter2++)
			{
				const NFmiViewSettingMacro::Param &par = params[counter2];
				NFmiInfoData::Type usedDataType = par.DataType();
				if(usedDataType >= NFmiInfoData::kSoundingParameterData)
					usedDataType = NFmiInfoData::kModelHelpData; // vanhoissa makroissa on k‰ytetty t‰t‰ NFmiInfoData::kSoundingParameterData
																// virityst‰. SmartMet versiosta 5.6 l‰htien sounding-index datat lasketaan valmiiksi datoiksi
																// ja sen tyyppi on NFmiInfoData::kModelHelpData. T‰m‰ siis vain pit‰‰
																// yll‰ taaksep‰in yhteen sopivuutta n‰yttˆmakroissa.
				NFmiMenuItem menuItem(theDescTopIndex, gDummyParamName, par.DataIdent(), kFmiAddView, NFmiMetEditorTypes::kFmiParamsDefaultView, &par.Level(), usedDataType);
                AddViewWithRealRowNumber(false, menuItem, static_cast<int>(counter + 1), fTreatAsViewMacro, &(par.DrawParam()->InitFileName())); // true tarkoittaa ett‰ kyseess‰ on viewmacrodrawparam, jota k‰sitellaan drawparam-modifioinnissa hieman erilailla
				AdjustDrawParam(theDescTopIndex, par, static_cast<int>(counter+1), static_cast<int>(counter2+1)); // kaikki makroon talletetut drawparam asetukset pit‰‰ viel‰ ladata luotuun ja listoihin laitettuun drawparamiin
			}
		}
	}

    void SetSingleExtraMapViewSettingMacro(NFmiViewSettingMacro::MapViewDescTop &theViewMacro, unsigned int theDescTopIndex, bool fTreatAsViewMacro)
	{
		NFmiMapViewDescTop &descTop = *MapViewDescTop(theDescTopIndex);
        boost::shared_ptr<NFmiMapViewWinRegistry> mapViewWinRegistry = ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(theDescTopIndex);

		descTop.InitForViewMacro(theViewMacro.GetMapViewDescTop(), *mapViewWinRegistry.get(), false);
		// Mit‰ tehd‰‰n extra rows:ien kanssa
//		theViewMacro.SetAllExtraRowParams(descTop.ExtraDrawParamListVector(), itsMacroParamSystem);
		descTop.SetViewMacroDipMapHelperList(theViewMacro.DipMapHelperList());


		// asetetaan parametrit 'normaali' riveilleen
        SetMapViewRowsParams(theViewMacro.MapRowSettings(), theDescTopIndex, 0, fTreatAsViewMacro);
	}

    void SetExtraMapViewSettingsMacro(NFmiViewSettingMacro &theMacro, bool fTreatAsViewMacro)
	{
		std::vector<NFmiViewSettingMacro::MapViewDescTop> &extraMapViewDescTops = theMacro.ExtraMapViewDescTops();
		for(size_t i = 0; i < extraMapViewDescTops.size(); i++)
            SetSingleExtraMapViewSettingMacro(extraMapViewDescTops[i], static_cast<unsigned int>(i), fTreatAsViewMacro);
	}

	void SetMasksParams(NFmiViewSettingMacro &theMacro)
	{
		boost::shared_ptr<NFmiAreaMaskList> paramMaskList(new NFmiAreaMaskList());
		const checkedVector<NFmiViewSettingMacro::Mask>& masks = theMacro.GetMaskSettings().Masks();
		const checkedVector<NFmiViewSettingMacro::Mask>::size_type ssize = masks.size();
		checkedVector<NFmiViewSettingMacro::Mask>::size_type counter = 0;
		for( ;counter < ssize ; counter++)
		{
			const NFmiViewSettingMacro::Mask &mask = masks[counter];
			NFmiMenuItem menuItem(-1, "x", mask.ParamSettings().DataIdent(), kFmiAddMask, NFmiMetEditorTypes::kFmiParamsDefaultView, &mask.ParamSettings().Level(), mask.ParamSettings().DataType());
			boost::shared_ptr<NFmiAreaMask> areaMask(CreateMask(menuItem));
			if(areaMask)
			{
				areaMask->Enable(mask.MaskEnabled());
				areaMask->Condition(mask.MaskSettings());
				paramMaskList->Add(areaMask);
			}
		}
		ParamMaskListMT(paramMaskList); // asetetaan lopuksi uudet maskit k‰yttˆˆn
	}

	void SetMaskSettings(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::MaskSettings &masks = theMacro.GetMaskSettings();
		SetMasksParams(theMacro);
        SetShowMaskOnMap(0, masks.ShowMasksOnMapView());
		fUseMasksInTimeSerialViews = masks.UseMasksInTimeSerialViews();
		fUseMasksWithFilterTool = masks.UseMasksWithFilterTool();
		fUseMaskWithBrush = masks.UseMaskWithBrush();
	}

    void MakeApplyViewMacroDirtyActions()
    {
        // l‰j‰ dirty funktio kutsuja, ota nyt t‰st‰ selv‰‰. Pit‰isi laittaa uuteen uskoon koko p‰ivitys asetus juttu.
        MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, true, true, false, false, true);
        unsigned int ssize = static_cast<unsigned int>(itsMapViewDescTopList.size());
        for(unsigned int i = 0; i<ssize; i++)
        {
            MapViewDescTop(i)->MapViewBitmapDirty(true);
            MapViewDescTop(i)->BorderDrawDirty(true);
        }
        MacroParamDataCache().clearAllLayers();
    }

    void ApplyViewMacro(NFmiViewSettingMacro &theMacro, bool fTreatAsViewMacro, bool undoRedoAction)
	{ // ota k‰yttˆˆn kaikki makron asetukset ja tee n‰ytˆist‰ 'likaisia'
		SetGeneralDoc(theMacro);
        SetTimeViewParams(theMacro, fTreatAsViewMacro);
		SetTempView(theMacro);
		SetTrajectoryView(theMacro);
        SetCrossSectionView(theMacro, fTreatAsViewMacro);
		SetWarningCenterView(theMacro);
		SetSynopPlotSettings(theMacro);
        SetExtraMapViewSettingsMacro(theMacro, fTreatAsViewMacro);
		// SynopDataGridView hoidetaan CSmartMetDoc-luokassa
		SetObsComparisonSettings(theMacro);
		SetMaskSettings(theMacro);

        ApplicationWinRegistry().KeepMapAspectRatio(theMacro.KeepMapAspectRatio());

        MakeApplyViewMacroDirtyActions();

        // Lopuksi (jos poikkeuksia ei ole lent‰nyt) laitetaan ladatun macron nimi talteen p‰‰karttan‰ytˆn title teksti‰ varten
        SetLastLoadedViewMacroName(theMacro, fTreatAsViewMacro, undoRedoAction);
        ApplicationInterface::GetApplicationInterfaceImplementation()->UpdateMainFrameTitle(); // t‰m‰ sitten p‰ivitt‰‰ p‰‰karttan‰ytˆn otsikko tekstiin viimeisimm‰n viewMacron nimen
    }

    // Jos normaali view macro lataus, haetaan nimeen mukaan suhteellinen polku.
    // Jos erikois viewMacro kuten F12, CTRL+F12 ja SHIFT+F12 toiminnot, k‰ytet‰‰n nime‰ suoraan.
    void SetLastLoadedViewMacroName(NFmiViewSettingMacro &theMacro, bool fTreatAsViewMacro, bool undoRedoAction)
    {
        if(fTreatAsViewMacro && !undoRedoAction)
        {
            std::string relativeMacroPath = PathUtils::getRelativePathIfPossible(itsViewMacroPath, itsRootViewMacroPath);
            itsLastLoadedViewMacroName = relativeMacroPath + theMacro.Name();
        }
        else
            itsLastLoadedViewMacroName = theMacro.Name();
    }

	bool HandleViewMacroreadingException(const std::string &theFileName, const std::string &theErrorStr)
	{
        auto it = std::find(itsCorruptedViewMacroFileList.begin(), itsCorruptedViewMacroFileList.end(), theFileName);

        if(it == itsCorruptedViewMacroFileList.end())
		{
            itsCorruptedViewMacroFileList.push_back(theFileName);
			string errorMessage(::GetDictionaryString("ViewMacroErrorCorrupted1"));
			errorMessage  += "\n";
			errorMessage  += theFileName;
			errorMessage  += "\n";
			errorMessage  += theErrorStr;
			errorMessage  += "\n\n";
			errorMessage  += ::GetDictionaryString("ViewMacroErrorCorrupted2");
			errorMessage  += "\n";
			errorMessage  += ::GetDictionaryString("ViewMacroErrorCorrupted3");
			errorMessage  += "\n";
			errorMessage  += ::GetDictionaryString("ViewMacroErrorCorrupted4");
			errorMessage  += "\n";
			errorMessage  += ::GetDictionaryString("ViewMacroErrorCorrupted5");
			errorMessage  += "\n\n";
			errorMessage  += ::GetDictionaryString("ViewMacroErrorCorrupted6");
			errorMessage  += "\n";
			errorMessage  += ::GetDictionaryString("ViewMacroErrorCorrupted7");
			throw runtime_error(errorMessage);
		}
		else
			return false; // ei tehd‰ virheilmoitus vialliselle makrolle kuin 1. kerran
	}

	// palauttaa nyky‰‰n tiedon onnistuiko viewmakron luku vai ei
	bool ReadViewMacro(NFmiViewSettingMacro &theViewMacro, const std::string &theFileName, bool fShowErrorDialog = true)
	{
		bool status = true;
		try
		{
			if(theFileName.empty())
				throw runtime_error(::GetDictionaryString("ViewMacroErrorEmpty"));

            NFmiCommentStripper stripComments(true, true, true, false, false);
			if(stripComments.ReadAndStripFile(theFileName))
			{
			#ifdef OLDGCC
				istrstream strippedMacroFile(stripComments.GetString().c_str());
			#else
				stringstream strippedMacroFile(stripComments.GetString());
			#endif
				try
				{
                    strippedMacroFile >> theViewMacro;
				}
				catch(exception &e)
				{
					return HandleViewMacroreadingException(theFileName, std::string(e.what()));
				}
				catch(...)
				{
					return HandleViewMacroreadingException(theFileName, std::string("Unknown error while reading viewMacro"));
				}
			}
			else
				throw runtime_error(::GetDictionaryString("ViewMacroErrorFile") + "'" + theFileName + "'");
		}
		catch (exception &e)
		{
            if(fShowErrorDialog)
    			::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(e.what()), CA2T(::GetDictionaryString("ViewMacroErrorFileTitle").c_str()), MB_OK);
			status = false;
		}
		// alunperin makron nimi talletettiin tiedostoon. T‰st‰ on seurannut ongelmia. Nyt
		// makron nimi otetaan tiedoston nimest‰.
        theViewMacro.Name(GetFileNameHeader(theFileName));
        theViewMacro.SetMacroParamInitFileNames(MacroParamSystem().RootPath());

		return status;
	}

	void WriteViewMacro(const NFmiViewSettingMacro &theMacro, const std::string &theFileName)
	{
		try
		{
			if(theFileName.empty())
				throw runtime_error("Viewmacro-filename was empty.");
			ofstream out(theFileName.c_str(), std::ios::binary);
			if(out)
				out << theMacro;
			else
				throw runtime_error(string("Viewmacro couldn't be stored in file:\n") + theFileName + "\nCheck that the folder exists and you have write permissions there.");
            RemoveFromCorruptedViewmacroList(theFileName);
		}
		catch (exception &e)
		{
			::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(e.what()), _TEXT("View Macro write error"), MB_OK);
		}
	}

    void LoadViewMacroInfo(NFmiViewSettingMacro &theViewMacro, bool fTreatAsViewMacro, bool undoRedoAction)
	{
        ApplyViewMacro(theViewMacro, fTreatAsViewMacro, undoRedoAction);
        ApplicationInterface::GetApplicationInterfaceImplementation()->LoadViewMacroWindowsSettings(theViewMacro); // t‰m‰ hoitaa CView ikkunoiden asetukset makroista
	}

    // Jos n‰yttˆmakro oli korruptoitunut, palautetaan false, muuten true
	bool LoadViewMacro(const std::string &theName)
	{
		try
		{
			SnapshotViewMacro(true); // true = tyhjennet‰‰n redo-lista

			// HUOM! t‰ss‰ vaiheessa pit‰‰ n‰yttˆmakro lukea uudestaan, ett‰ ajat p‰ivittyv‰t nykyhetkeen.
			// Muuten ajat voivat olla lukittuna suunnilleen siihen aikaa kun editori on k‰ynnistetty
            std::string initFileName = itsViewMacroPath + theName + ".vmr";
            if(ReadViewMacro(*CurrentViewMacro(), initFileName))
            {
                itsHelperViewMacro = *CurrentViewMacro();

                LoadViewMacroInfo(*CurrentViewMacro(), true, false);

                ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Loading view-macro in use"); // t‰m‰ sitten p‰ivitt‰‰ kaikki ruudut

                // lokitetaan, mik‰ makro on ladattu k‰yttˆˆn...
                string logStr("Applying the view-macro: ");
                logStr += CurrentViewMacro()->InitFileName();
                LogMessage(logStr, CatLog::Severity::Info, CatLog::Category::Macro);
                return true;
            }
		}
		catch(exception &e)
		{
			string errStr("Unable to load view-macro: ");
			errStr += theName;
			errStr += "\nReason: ";
			errStr += e.what();
			LogAndWarnUser(errStr, "ViewMacro loading problem", CatLog::Severity::Error, CatLog::Category::Macro, false);
		}
		catch(...)
		{
			string errStr("Unable to load view-macro (reason unknown): ");
			errStr += theName;
			LogAndWarnUser(errStr, "ViewMacro loading problem", CatLog::Severity::Error, CatLog::Category::Macro, false);
		}
        return false;
	}

    bool LoadViewMacroFromBetaProduct(const std::string &theAbsoluteVieMacroPath, std::string &theErrorStringOut, bool justLogMessages)
    {
        try
        {
            SnapshotViewMacro(true); // true = tyhjennet‰‰n redo-lista

            NFmiViewSettingMacro betaProductMacro;
            if(ReadViewMacro(betaProductMacro, theAbsoluteVieMacroPath, false))
            {
                LoadViewMacroInfo(betaProductMacro, true, false);

                ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Loading view-macro from beta product"); // t‰m‰ sitten p‰ivitt‰‰ kaikki ruudut

                // lokitetaan, mik‰ makro on ladattu k‰yttˆˆn...
                string logStr("Applying the Beta product view-macro: ");
                logStr += theAbsoluteVieMacroPath;
                LogMessage(logStr, CatLog::Severity::Info, CatLog::Category::Macro);
                return true;
            }
            else
                throw std::runtime_error("ReadViewMacro -function failed for unknown reason");
        }
        catch(exception &e)
        {
            string errStr("Unable to load Beta product view-macro: ");
            errStr += theAbsoluteVieMacroPath;
            errStr += "\nReason: ";
            errStr += e.what();
            theErrorStringOut = errStr;
            LogAndWarnUser(errStr, "ViewMacro loading problem", CatLog::Severity::Error, CatLog::Category::Macro, justLogMessages);
        }
        catch(...)
        {
            string errStr("Unable to load Beta product view-macro (reason unknown): ");
            errStr += theAbsoluteVieMacroPath;
            theErrorStringOut = errStr;
            LogAndWarnUser(errStr, "ViewMacro loading problem", CatLog::Severity::Error, CatLog::Category::Macro, justLogMessages);
        }
        return false;
    }

void AdjustViewMacroUndoListSize(size_t maxSize)
{
	for( ; itsUndoRedoViewMacroList.size() > maxSize; )
	{
		itsUndoRedoViewMacroList.pop_front();
		itsUndoRedoViewMacroListPointer--;
	}
}

// kun ladataan normaalisti viewMacro, pit‰‰ redo-lista tyhjent‰‰
void SnapshotViewMacro(bool fClearRedoList)
{
	// 1. tyhjenn‰ redolista-osio jos haluttu
	if(fClearRedoList)
	{
		size_t newSize = itsUndoRedoViewMacroList.size() - (itsUndoRedoViewMacroList.size() - itsUndoRedoViewMacroListPointer) + 1;
		itsUndoRedoViewMacroList.resize(newSize);
		itsUndoRedoViewMacroListPointer++;
	}
	// 2. Ota kuva nykyhetkest‰
	NFmiViewSettingMacro currentSnapShot;
	FillViewMacroInfo(currentSnapShot, itsLastLoadedViewMacroName, "snapshot description");
	// 3. talleta se undoredoListaan per‰‰n
	itsUndoRedoViewMacroList.push_back(currentSnapShot);

	AdjustViewMacroUndoListSize(50); // pidet‰‰n undoredo listan koko max 50:ss‰
}

void UndoViewMacro(void)
{
	// 0. onko undo mahdollista
	if(IsUndoableViewMacro())
	{
	    // 0.1 ollaanko viimeisess‰ kohdassa undolistaa, jos ollaan, voidaan tehd‰ SnapshotViewMacro
		if(itsUndoRedoViewMacroListPointer == itsUndoRedoViewMacroList.size()-1)
		{
	    // 1. ota kuva nyky hetkest‰ talteen
			SnapshotViewMacro(false); // false = ei tyhjennet‰ redo-listaa, ei ole tarvetta
		}
	    // 2. Lataa k‰yttˆˆn undoredoListasta edellinen makro k‰yttˆˆn
		LoadViewMacroInfo(itsUndoRedoViewMacroList[itsUndoRedoViewMacroListPointer], true, true);
	    // 3. aseta pointteri edelliseen kohtaan
		itsUndoRedoViewMacroListPointer--;

	    // 4. Pit‰‰ viel‰ varmuuden vuoksi p‰ivitt‰‰ kaikki ruudut
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Undo current view-macro"); // t‰m‰ sitten p‰ivitt‰‰ kaikki ruudut
    }
}
void RedoViewMacro(void)
{
	// 1. onko redo mahdollista
	if(IsRedoableViewMacro())
	{
    	// 2. Lataa k‰yttˆˆn undoredoListasta seuraava makro k‰yttˆˆn
		itsUndoRedoViewMacroListPointer++;
		LoadViewMacroInfo(itsUndoRedoViewMacroList[itsUndoRedoViewMacroListPointer+1], true, true); // redo:ssa pit‰‰ ladata yksi yli currentin paikan

        // 3. Pit‰‰ viel‰ varmuuden vuoksi p‰ivitt‰‰ kaikki ruudut
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Redo last view-macro"); // t‰m‰ sitten p‰ivitt‰‰ kaikki ruudut
    }
}
bool IsUndoableViewMacro(void)
{
	if(itsUndoRedoViewMacroList.size() > 0 && itsUndoRedoViewMacroListPointer >= 0 && itsUndoRedoViewMacroListPointer < static_cast<int>(itsUndoRedoViewMacroList.size()))
		return true;
	return false;
}
bool IsRedoableViewMacro(void)
{
	if(itsUndoRedoViewMacroList.size() > 0 && itsUndoRedoViewMacroListPointer < static_cast<int>(itsUndoRedoViewMacroList.size())-2)
		return true;
	return false;
}

	void FillViewMacroInfo(NFmiViewSettingMacro &theViewMacro, const std::string &theName, const std::string &theDescription)
	{
        ApplicationInterface::GetApplicationInterfaceImplementation()->StoreViewMacroWindowsSettings(theViewMacro); // t‰m‰ t‰ytt‰‰ itsHelperViewMacro:on CView osat viewmacrosta
		FillViewMacro(theViewMacro, theName, theDescription); // t‰h‰n on jo t‰ytetty windows ikkunoiden asetukset ulkopuolella
	}

	void StoreViewMacro(const std::string &theName, const std::string &theDescription)
	{
		FillViewMacroInfo(itsHelperViewMacro, theName, theDescription);

		string fileName(GetViewMacroFileName(theName));
		WriteViewMacro(itsHelperViewMacro, fileName);
		RefreshViewMacroList();
	}
	std::shared_ptr<NFmiViewSettingMacro> CurrentViewMacro(void)
	{
        return itsCurrentViewMacro;
	}
	NFmiViewSettingMacro& HelperViewMacro(void)
	{
		return itsHelperViewMacro;
	}

	// funktio palauttaa tiedoston nimest‰ header-osion eli esim.
	// "d:\directory\filename.ext" -tiedostonimest‰ palautetaan
	// "filename"
	std::string GetFileNameHeader(const std::string &theFileName)
	{
		NFmiFileString fileStr(theFileName);
		string realNameStr = fileStr.Header();
		return realNameStr;
	}

    std::time_t GetLastWriteTime(const std::string &theFullPath)
    {
        boost::system::error_code er;
        return boost::filesystem::last_write_time(theFullPath, er);
    }

    NFmiLightWeightViewSettingMacro MakeViewMacroDescriptionFromFile(const std::string &theFileName)
	{
        NFmiLightWeightViewSettingMacro viewMacroDescription;
		viewMacroDescription.itsInitFilePath = theFileName;
        viewMacroDescription.itsName = GetFileNameHeader(theFileName);
        viewMacroDescription.itsFileModificationTime = NFmiTime(GetLastWriteTime(theFileName));
        std::ifstream in(theFileName, std::ios::binary);
        if(in)
        {
            in >> viewMacroDescription; // lukee vain description -osion viewmacro tiedoston alusta
            if(!in)
                viewMacroDescription.fViewMacroOk = false;
        }
        else
            viewMacroDescription.fViewMacroOk = false;

        return viewMacroDescription;
	}

	NFmiLightWeightViewSettingMacro MakeViewMacroDescriptionFromDirectory(const std::string &theDirectoryName)
	{
		// "this"-hakemistoa ei laiteta
		if(theDirectoryName == ".")
            return NFmiLightWeightViewSettingMacro();
		// jos ollaan ns. root hakemistossa, ei up-hakemistoa laiteta
		if(itsViewMacroPath == itsRootViewMacroPath && theDirectoryName == "..")
            return NFmiLightWeightViewSettingMacro();
        NFmiLightWeightViewSettingMacro viewMacroDescription;
		viewMacroDescription.fIsViewMacroDirectory = true;
		std::string name("<");
		name += theDirectoryName;
		name += ">";
        viewMacroDescription.itsName = name;
        viewMacroDescription.itsDescription = "<Directory>";
        std::string fullPath = itsViewMacroPath + theDirectoryName;
        viewMacroDescription.itsFileModificationTime = NFmiTime(GetLastWriteTime(fullPath));
        return viewMacroDescription;
	}

	void RefreshViewMacroList(void)
	{
		// tyhjennet‰‰n ensin olemassa oleva lista
        itsViewMacroDescriptionList.clear();

		// listataan alkuun hakemistot ja jos ei olla rootissa, laitetaan viel‰ ..-hakemsito mukaan
		std::list<std::string> directories = NFmiFileSystem::Directories(itsViewMacroPath);
		std::list<std::string>::iterator itDir = directories.begin();
		std::list<std::string>::iterator itEndDir = directories.end();
		for( ; itDir != itEndDir; ++itDir)
		{
            NFmiLightWeightViewSettingMacro viewMacroDescription = MakeViewMacroDescriptionFromDirectory(*itDir);
			if(!viewMacroDescription.IsEmpty())
                itsViewMacroDescriptionList.push_back(viewMacroDescription);
		}

		// luodaan sitten uusi lista
		string filePattern(GetViewMacroFilePattern());
		std::list<std::string> macroFileList = NFmiFileSystem::PatternFiles(filePattern);
		std::list<std::string>::iterator it = macroFileList.begin();
		std::list<std::string>::iterator itEnd = macroFileList.end();
		for( ; it != itEnd; ++it)
		{
			std::string initFileName = GetViewMacroTotalFileName(*it);
            NFmiLightWeightViewSettingMacro viewMacroDescription = MakeViewMacroDescriptionFromFile(initFileName);
			itsViewMacroDescriptionList.push_back(viewMacroDescription); // laitetaan myˆs virheellinen listaan, ett‰ se voidaan poistaa ViewMacro-dialogin avulla
		}
	}

	bool RemoveViewMacro(const string& theMacroName)
	{
        bool removalSuccessful = false;
		if(theMacroName.empty())
            removalSuccessful = false;
		else if(theMacroName[0] == '<')
		{
			string tmp(theMacroName);
			NFmiStringTools::TrimL(tmp, '<');
			NFmiStringTools::TrimR(tmp, '>');
			string totalPath = itsViewMacroPath;
			totalPath += tmp;
			list<string> fileList = NFmiFileSystem::DirectoryFiles(totalPath);
			if(fileList.empty())
			{
				if(::rmdir(totalPath.c_str()) != 0)
                    ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(::GetDictionaryString("ViewMacroErrorFolder1").c_str()), CA2T(::GetDictionaryString("ViewMacroErrorFolder2").c_str()), MB_OK | MB_ICONERROR);
                else
                    removalSuccessful = true;
			}
			else
                ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(::GetDictionaryString("ViewMacroErrorFolder3").c_str()), CA2T(::GetDictionaryString("ViewMacroErrorFolder2").c_str()), MB_OK | MB_ICONERROR);

		}
		else
		{
			string fileName(GetViewMacroFileName(theMacroName));
			if(NFmiFileSystem::RemoveFile(fileName))
                removalSuccessful = true;
        }

        RefreshViewMacroList();
        return removalSuccessful;
	}

    CtrlViewUtils::FmiSmartMetEditingMode SmartMetEditingMode(void)
	{
		return itsSmartMetEditingMode;
	}

	void SmartMetEditingMode(CtrlViewUtils::FmiSmartMetEditingMode newValue, bool modifySettings)
    {
		itsSmartMetEditingMode = newValue;
        if(modifySettings)
    		NFmiSettings::Set(string("MetEditor::EditXMode"), NFmiStringTools::Convert<int>(itsSmartMetEditingMode), true);
	}

	bool UseDoAtSendCommandString(void)
	{
		return fUseDoAtSendCommand;
	}

	void UseDoAtSendCommandString(bool newValue)
	{
		fUseDoAtSendCommand = newValue;
	}

	std::string DoAtSendCommandString(void)
	{
		return itsDoAtSendCommandString;
	}

	void DoAtSendCommandString(const std::string &newValue)
	{
		itsDoAtSendCommandString = newValue;
	}

	void UseMap(unsigned int theDescTopIndex, unsigned int theMapIndex)
	{
		SetSelectedMap(theDescTopIndex, theMapIndex);
		MapViewDirty(theDescTopIndex, true, true, true, false, false, false);
	}

void ClearMacroParamDataCache(unsigned int theDescTopIndex, bool clearMacroParamDataCache, bool clearEditedDataDependentCaches)
{
    if(clearMacroParamDataCache)
        MacroParamDataCache().clearView(theDescTopIndex);
    if(clearEditedDataDependentCaches)
    {
        ClearAllMacroParamDataCacheDependentOfEditedDataAfterEditedDataChanges();
        ClearAllViewRowsWithEditedData();
    }
}

bool IsEditedRelatedDataType(NFmiInfoData::Type dataType)
{
    return (dataType == NFmiInfoData::kEditable) || (dataType == NFmiInfoData::kCopyOfEdited);
}

bool DrawParamListContainsEditedData(NFmiDrawParamList &drawParamList)
{
    for(drawParamList.Reset(); drawParamList.Next(); )
    {
        boost::shared_ptr<NFmiDrawParam> drawParam = drawParamList.Current();
        if(!drawParam->IsParamHidden())
        {
            if(IsEditedRelatedDataType(drawParam->DataType()))
            {
                return true;
            }
        }
    }
    return false;
}

// Tyhjennet‰‰n bitmap cached eri karttan‰ytˆist‰ niilt‰ riveilt‰, miss‰ on editoitua dataa katseltavana.
// Lis‰ksi liataan aikasarja ikkuna, jos siell‰ on editoitua parametria valittuna.
void ClearAllViewRowsWithEditedData()
{
    unsigned int descTopIndex = 0;
    for(auto *deskTop : itsMapViewDescTopList)
    {
        auto mapViewDesctop = MapViewDescTop(descTopIndex);
        int cacheRowNumber = 0; // cache row indeksi alkaa 0:sta!!
        NFmiPtrList<NFmiDrawParamList> *drawParamListVector = deskTop->DrawParamListVector();
        NFmiPtrList<NFmiDrawParamList>::Iterator iter = drawParamListVector->Start();
        for(; iter.Next();)
        {
            NFmiDrawParamList *drawParamList = iter.CurrentPtr();
            if(DrawParamListContainsEditedData(*drawParamList))
            {
                mapViewDesctop->MapViewCache().MakeRowDirty(cacheRowNumber);
            }
            cacheRowNumber++;
        }
        descTopIndex++;
    }

    // Lopuksi viel‰ likaus aikasarjaan, jos tarvis
    if(DrawParamListContainsEditedData(*itsTimeSerialViewDrawParamList))
        fTimeSerialViewDirty = true;
}

void MapViewDirty(unsigned int theDescTopIndex, bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool clearMacroParamDataCache, bool clearEditedDataDependentCaches, bool updateMapViewDrawingLayers)
{
    if(redrawMapView)
    {
        // Ainakin toistaiseksi laitan aikasarjan likauksen t‰nne
        fTimeSerialViewDirty = true;
    }

    if(theDescTopIndex == CtrlViewUtils::kDoAllMapViewDescTopIndex)
        MapViewDirtyForAllDescTops(makeNewBackgroundBitmap, clearMapViewBitmapCacheRows, redrawMapView, clearMacroParamDataCache, clearEditedDataDependentCaches, updateMapViewDrawingLayers);
    else
    {
        try
        {
            MapViewDescTop(theDescTopIndex)->MapViewDirty(makeNewBackgroundBitmap, clearMapViewBitmapCacheRows, redrawMapView, updateMapViewDrawingLayers);
        }
        catch(...)
        { } // Jos t‰t‰ kutsutaan esim. poikkileikkaus n‰ytˆlle, lent‰‰ poikkeus, mik‰ on t‰ysin ok

        ClearMacroParamDataCache(theDescTopIndex, clearMacroParamDataCache, clearEditedDataDependentCaches);
    }
}

void MapViewDirtyForAllDescTops(bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool clearMacroParamDataCache, bool clearEditedDataDependentCaches, bool updateMapViewDrawingLayers)
{
    unsigned int ssize = static_cast<unsigned int>(itsMapViewDescTopList.size());
    for(unsigned int viewIndex = 0; viewIndex < ssize; viewIndex++)
    {
        try
        {
            MapViewDescTop(viewIndex)->MapViewDirty(makeNewBackgroundBitmap, clearMapViewBitmapCacheRows, redrawMapView, updateMapViewDrawingLayers);
        }
        catch(...)
        { } // Jos t‰t‰ kutsutaan esim. poikkileikkaus n‰ytˆlle, lent‰‰ poikkeus, mik‰ on t‰ysin ok

        ClearMacroParamDataCache(viewIndex, clearMacroParamDataCache, clearEditedDataDependentCaches);
    }
}

void MakeEditorDescTopClearCache(unsigned int theDescTopIndex, bool clearCache)
{
	if(clearCache)
	{
		if(theDescTopIndex == CtrlViewUtils::kDoAllMapViewDescTopIndex)
		{
			unsigned int ssize = static_cast<unsigned int>(itsMapViewDescTopList.size());
			for(unsigned int i = 0; i<ssize; i++)
				MapViewDescTop(i)->MapViewCache().MakeDirty(); // laitetaan cache halutessa likaiseksi
		}
		else
			MapViewDescTop(theDescTopIndex)->MapViewCache().MakeDirty(); // laitetaan cache halutessa likaiseksi
	}
}

void SetSelectedMap(unsigned int theDescTopIndex, int newMapIndex)
{
    // HUOM! newMapIndex pit‰‰ asettaa sek‰ Windows rekisteriin ett‰ mapViewDescTopiin!
    ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(theDescTopIndex)->SelectedMapIndex(newMapIndex);
	MapViewDescTop(theDescTopIndex)->SelectedMapIndex(newMapIndex);

	SetMapArea(theDescTopIndex, MapViewDescTop(theDescTopIndex)->MapHandler()->Area());
}

void SetSelectedMapHandler(unsigned int theDescTopIndex, unsigned int newMapIndex)
{
    SetSelectedMap(theDescTopIndex, newMapIndex);
	MapViewDirty(theDescTopIndex, true, true, true, false, false, false);
}

boost::shared_ptr<NFmiArea> MakeCPCropArea(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, float theShrinkFactor, unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long shrinkX, unsigned long shrinkY)
{
	unsigned long gridSizeX = theInfo->GridXNumber();
	unsigned long bottomLeftLocationIndex = FmiRound((y1 + (shrinkY * theShrinkFactor)) * gridSizeX + (x1 + (shrinkX * theShrinkFactor)));
	theInfo->LocationIndex(bottomLeftLocationIndex);
	NFmiPoint bottomLeftLatlon = theInfo->LatLon();
	unsigned long topRightLocationIndex = FmiRound((y2 - (shrinkY * theShrinkFactor)) * gridSizeX + (x2 - (shrinkX * theShrinkFactor)));
	theInfo->LocationIndex(topRightLocationIndex);
	NFmiPoint topRightLatlon = theInfo->LatLon();
	return boost::shared_ptr<NFmiArea>(new NFmiLatLonArea(bottomLeftLatlon, topRightLatlon));
}

void SetCPCropGridSettings(const boost::shared_ptr<NFmiArea> &theArea)
{
	itsCPGridCropMargin = NFmiPoint();
	itsCPGridCropRect = NFmiRect(); // asetetaan tyhj‰ crop-recti aluksi
	itsCPGridCropLatlonArea = boost::shared_ptr<NFmiArea>();
	itsCPGridCropInnerLatlonArea = boost::shared_ptr<NFmiArea>();
	boost::shared_ptr<NFmiFastQueryInfo> editedInfo = EditedInfo();
	if(theArea && editedInfo && editedInfo->Grid())
	{
        boost::shared_ptr<NFmiArea> editedArea(editedInfo->Area()->Clone());
		if(NFmiQueryDataUtil::AreAreasSameKind(theArea.get(), editedArea.get()))
		{ // crop rect voidaan laskea vain kun editoidun datan area ja zoomattu area ovat samaa tyyppi‰, t‰llˆin zoomi alue on neliskanttinen editoidulla datalla
			editedInfo->NearestPoint(theArea->BottomLeftLatLon());
			unsigned long bottomLeftCornerLocationIndex = editedInfo->LocationIndex();
			editedInfo->NearestPoint(theArea->TopRightLatLon());
			unsigned long topRightCornerLocationIndex = editedInfo->LocationIndex();
			if((bottomLeftCornerLocationIndex != gMissingIndex) && (topRightCornerLocationIndex != gMissingIndex) && (bottomLeftCornerLocationIndex != topRightCornerLocationIndex))
			{
				unsigned long gridSizeX = editedInfo->GridXNumber();
				unsigned long x1 = bottomLeftCornerLocationIndex % gridSizeX;
				unsigned long y1 = bottomLeftCornerLocationIndex / gridSizeX;
				unsigned long x2 = topRightCornerLocationIndex % gridSizeX;
				unsigned long y2 = topRightCornerLocationIndex / gridSizeX;
				const double divider = 18.;
				unsigned long shrinkX = FmiRound((x2 - x1) / divider);
				unsigned long shrinkY = FmiRound((y2 - y1) / divider);
				if(shrinkX >= 2 && shrinkY >= 2)
				{
					itsCPGridCropMargin = NFmiPoint(shrinkX, shrinkY);
					itsCPGridCropRect = NFmiRect(x1 + shrinkX, y2 - shrinkY, x2 - shrinkX, y1 + shrinkY);
					itsCPGridCropLatlonArea = MakeCPCropArea(editedInfo, 1, x1, y1, x2, y2, shrinkX, shrinkY);
					// lasketaan viel‰ inner crop rect
					itsCPGridCropInnerLatlonArea = MakeCPCropArea(editedInfo, 2, x1, y1, x2, y2, shrinkX, shrinkY);
				}
				else
				{ // CP crop ei ole j‰rkev‰‰, on zoomattu datassa liian l‰helle ja k‰ytˆss‰ on liian v‰h‰n hilapisteit‰.
					// Nyt lasketaan vain ulompi laatiiko, jonka sis‰‰n voidaan karttan‰ytˆss‰ piirt‰‰ punainen rasti.
					itsCPGridCropLatlonArea = MakeCPCropArea(editedInfo, 1, x1, y1, x2, y2, shrinkX, shrinkY);
				}
			}
		}
	}
}

void SetMapArea(unsigned int theDescTopIndex, const boost::shared_ptr<NFmiArea> &newArea)
{
	if(newArea)
	{
		NFmiMapViewDescTop *mapDescTop = MapViewDescTop(theDescTopIndex);
		// t‰h‰n laitetaan tarkistus, ett‰ zoomi area ja karttapohjan areat ovat saman tyyppiset
		// Jos eiv‰t ole, tehd‰‰n mahd. samanlainen area, mik‰ newArea on, mutta mik‰ sopii
		// k‰ytˆss‰ olevan dipmaphandlerin kartta pohjaan
		if(NFmiQueryDataUtil::AreAreasSameKind(newArea.get(), mapDescTop->MapHandler()->TotalArea().get()))
		{
			mapDescTop->MapHandler()->Area(newArea);
			SetCPCropGridSettings(newArea);
		}
		else
		{ // tehd‰‰n sitten karttapohjalle sopiva area
			boost::shared_ptr<NFmiArea> correctTypeArea(mapDescTop->MapHandler()->TotalArea()->NewArea(newArea->BottomLeftLatLon(), newArea->TopRightLatLon()));
			if(correctTypeArea)
			{
				if(!mapDescTop->MapHandler()->TotalArea()->IsInside(*correctTypeArea))
				{ // pit‰‰ v‰h‰n viilata areaa, koska se ei mene kartta-alueen sis‰lle
					NFmiRect xyRect(mapDescTop->MapHandler()->TotalArea()->XYArea(correctTypeArea.get()));
					if(xyRect.Left() < 0)
						xyRect.Left(0);
					if(xyRect.Right() > 1)
						xyRect.Right(1);
					if(xyRect.Top() < 0)
						xyRect.Top(0);
					if(xyRect.Bottom() > 1)
						xyRect.Bottom(1);
					NFmiPoint blLatlon(mapDescTop->MapHandler()->TotalArea()->ToLatLon(xyRect.BottomLeft()));
					NFmiPoint trLatlon(mapDescTop->MapHandler()->TotalArea()->ToLatLon(xyRect.TopRight()));
					correctTypeArea = boost::shared_ptr<NFmiArea>(mapDescTop->MapHandler()->TotalArea()->NewArea(blLatlon, trLatlon));
				}
			}
			mapDescTop->MapHandler()->Area(correctTypeArea);
			SetCPCropGridSettings(correctTypeArea);
		}
		mapDescTop->BorderDrawDirty(true);
        // laitetaan viela kaikki ajat likaisiksi cachesta
		MapViewDirty(theDescTopIndex, true, true, true, true, false, false);
		mapDescTop->GridPointCache().Clear();
	}
}

void MakeSwapBaseArea(unsigned int theDescTopIndex)
{
	MapViewDescTop(theDescTopIndex)->MapHandler()->MakeSwapBaseArea();
	// t‰‰ll‰ ei tarvitse liata mit‰‰ eik‰ p‰ivitt‰‰ mit‰‰n
}

void SwapArea(unsigned int theDescTopIndex)
{
	NFmiMapViewDescTop *mapDescTop = MapViewDescTop(theDescTopIndex);
	mapDescTop->MapHandler()->SwapArea();

	// sitten viel‰ tarvitt‰v‰t likaukset ja p‰ivitykset
	mapDescTop->BorderDrawDirty(true);
	MapViewDirty(theDescTopIndex, true, true, true, true, false, false); // laitetaan viela kaikki ajat likaisiksi cachesta
	mapDescTop->GridPointCache().Clear();
}

	bool ChangeToEditorsWorkingDirectory(void)
	{
		bool status = ::_chdir(itsBasicConfigurations.WorkingDirectory().c_str()) == 0; // _chdir palauttaa 0, jos onnistuu
		return status;
	}
	bool SaveCurrentPath(void)
	{
		static char path[_MAX_PATH];
		int savedDrive = ::_getdrive(); // _getdrive:lla ei virhe paluu arvoa
		bool status = ::_getdcwd(savedDrive , path, _MAX_PATH ) != 0; // _getdcwd palauttaa 0-pointterin, jos ep‰onnistuu
		itsSavedDirectory = NFmiString(path);
		return status;
	}
	bool LoadSavedPath(void)
	{
		bool status = ::_chdir(itsSavedDirectory) == 0; // _chdir palauttaa 0, jos onnistuu
		return status;
	}

	void SetCurrentSmartToolMacro(const std::string& theMacroText)
	{
		itsSmartToolInfo.CurrentScript(theMacroText);
	}
	const std::string& GetCurrentSmartToolMacro(void)
	{
		return itsSmartToolInfo.CurrentScript();
	}

	NFmiMacroParamSystem& MacroParamSystem(void)
	{
		return itsMacroParamSystem;
	}

	// lis‰‰ halutun nimisen macroParamin halutun karttan‰ytˆn riville (1-5)
	void AddMacroParamToView(unsigned int theDescTopIndex, int theViewRow, const std::string &theName)
	{
		NFmiMenuItem menuItem(theDescTopIndex, theName, static_cast<FmiParameterName>(998), kFmiAddView, NFmiMetEditorTypes::kFmiParamsDefaultView, 0, NFmiInfoData::kMacroParam, theViewRow);
		AddViewWithRealRowNumber(true, menuItem, theViewRow);
	}

	// poistaa halutun macroparamin dokumentista, tiedostoista ja n‰ytˆilt‰
	void RemoveMacroParam(const std::string &theName)
	{
		if(theName.empty())
			return ;
		else if(theName[0] == '<')
		{
			string tmp(theName);
			NFmiStringTools::TrimL(tmp, '<');
			NFmiStringTools::TrimR(tmp, '>');
			string totalPath(itsMacroParamSystem.CurrentPath());
			totalPath += tmp;
			list<string> fileList = NFmiFileSystem::DirectoryFiles(totalPath);
			if(fileList.empty())
			{
				if(::rmdir(totalPath.c_str()) != 0)
					::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), _TEXT("Directory removal failed!"), _TEXT("Directory removal failed!"), MB_OK | MB_ICONERROR);
			}
			else
				::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), _TEXT("Directory contains macros, that you have to remove first!"), _TEXT("Directory removal failed!"), MB_OK | MB_ICONERROR);
		}
		else
		{
			boost::shared_ptr<NFmiMacroParamFolder> currentFolder = MacroParamSystem().GetCurrent();
			if(currentFolder)
				currentFolder->Remove(theName); // poista macroparam mpsysteemist‰ (joka tuhoaa myˆs tiedostot)
			RemoveMacroParamFromDrawParamLists(theName); // poista macroparam drawparamlistoista
			MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, true, true, false, false, true); // laitetaan viela kaikki ajat likaisiksi cachesta
		}
	}

	void RemoveMacroParamFromDrawParamLists(const std::string &theName)
	{
		auto ssize = static_cast<unsigned long>(itsMapViewDescTopList.size());
		for(auto viewIndex = 0ul; viewIndex < ssize; viewIndex++)
		{
			NFmiPtrList<NFmiDrawParamList>::Iterator iter = MapViewDescTop(viewIndex)->DrawParamListVector()->Start();
            for(unsigned long rowIndex = 0; iter.Next(); rowIndex++)
            {
                if(iter.CurrentPtr()->RemoveMacroParam(theName))
                {
                    MacroParamDataCache().update(viewIndex, rowIndex, iter.Current());
                }
            }
		}
	}

	void SetLatestMacroParamErrorText(const std::string& theErrorText)
	{
		itsLatestMacroParamErrorText = theErrorText;
	}
	const std::string& GetLatestMacroParamErrorText(void)
	{
		return itsLatestMacroParamErrorText;
	}

	NFmiCrossSectionSystem* CrossSectionSystem(void)
	{
		return &itsCrossSectionSystem;
	}

	// Kun ollaan hiiren kanssa N‰yt‰-ikkunan p‰‰ll‰ (karttan‰ytˆss‰) ja rullataan
	// hiiren rullaa, yritet‰‰n siirt‰‰ sill‰ rivill‰ olevaa aktiivista parametria
	// ylˆs/alas riippuen rullauksen suunnasta. Jos fRaise on true, tuodaan aktiivista
	// parametria piirto j‰rjestyksess‰ pintaan p‰in.
	// Palauttaa true jos siirto onnistui ja pit‰‰ p‰ivitt‰‰ n‰ytˆt, muuten false.
	bool MoveActiveMapViewParamInDrawingOrderList(unsigned int theDescTopIndex, int theMapRow, bool fRaise, bool fUseCrossSectionParams)
	{
		NFmiDrawParamList *list = DrawParamListWithRealRowNumber(theDescTopIndex, theMapRow);
		if(fUseCrossSectionParams)
			list = CrossSectionViewDrawParamList(theMapRow);
		if(list)
		{
			if(list->MoveActiveParam(fRaise ? -1 : 1))
			{
				MapViewDescTop(theDescTopIndex)->MapViewCache().MakeRowDirty(theMapRow - 1); // t‰‰ll‰ rivit alkavat 1:st‰, mutta cachessa 0:sta!!!
                MakeMacroParamCacheUpdatesForWantedRow(theDescTopIndex, theMapRow);
				return true;
			}
		}
		return false;
	}

	// Muuta theChangedDrawParam:ia niin ett‰ muuten asetukset tulevat theNewDrawParamSettings:ista, paitsi muutamat erikseen asetettavat
	// on otettava vanhasta.
	void SetUpChangedDrawParam(boost::shared_ptr<NFmiDrawParam> &theChangedDrawParam, boost::shared_ptr<NFmiDrawParam> &theNewDrawParamSettings)
	{
		// 1. Ota ensin tietyt asetukset vanhasta uuteen
		theNewDrawParamSettings->ModelRunIndex(theChangedDrawParam->ModelRunIndex());
		theNewDrawParamSettings->TimeSerialModelRunCount(theChangedDrawParam->TimeSerialModelRunCount());
		theNewDrawParamSettings->ModelRunDifferenceIndex(theChangedDrawParam->ModelRunDifferenceIndex());
		theNewDrawParamSettings->DataComparisonProdId(theChangedDrawParam->DataComparisonProdId());
		theNewDrawParamSettings->DataComparisonType(theChangedDrawParam->DataComparisonType());

		// 2. Aseta sitten muutettavan kaikki asetukset uusista asetuksista
		theChangedDrawParam->Init(theNewDrawParamSettings);
	}

	// Kun ollaan hiiren kanssa N‰yt‰-ikkunan p‰‰ll‰ (karttan‰ytˆss‰) ja rullataan
	// hiiren rullaa CTRL-nappi pohjassa, yritet‰‰n muuttaa sill‰ rivill‰ olevaa
	// aktiivista parametria seuraavaan/edelliseen mit‰ datasta lˆytyy riippuen rullauksen
	// suunnasta. Jos fNext on true, haetaan seuraava parametri (querydatan) parametrilistasta,
	// muuten edellinen. Menee p‰‰dyist‰ yli, eli viimeisest‰ menee 1. parametriin.
	// K‰y l‰pi myˆs aliparametri (TotalWind ja W&C).
	// Palauttaa true jos parametrin vaihto onnistui ja pit‰‰ p‰ivitt‰‰ n‰ytˆt, muuten false.
	bool ChangeActiveMapViewParam(unsigned int theDescTopIndex, int theRealMapRow, int theParamIndex, bool fNext, bool fUseCrossSectionParams)
	{
//	TRACE("ChangeActiveMapViewParam 1\n");
		NFmiDrawParamList *drawParamList = DrawParamListWithRealRowNumber(theDescTopIndex, theRealMapRow);
		if(fUseCrossSectionParams)
            drawParamList = CrossSectionViewDrawParamList(theRealMapRow);
		if(drawParamList)
		{
			if(drawParamList->Index(theParamIndex))
			{
				boost::shared_ptr<NFmiDrawParam> drawParam = drawParamList->Current();
				boost::shared_ptr<NFmiFastQueryInfo> info = itsSmartInfoOrganizer->Info(drawParam, fUseCrossSectionParams, true);
				if(info && info->SizeParams() > 1)
				{
					bool couldChangeParam = fNext ? info->NextParam(false) : info->PreviousParam(false);
					if(couldChangeParam == false)
						fNext ? info->FirstParam(false) : info->LastParam(false);

					boost::shared_ptr<NFmiDrawParam> drawParamTmp = itsSmartInfoOrganizer->CreateDrawParam(info->Param(), &drawParam->Level(), drawParam->DataType());
                    if(fUseCrossSectionParams)
                        drawParamTmp = itsSmartInfoOrganizer->CreateCrossSectionDrawParam(info->Param(), drawParam->DataType());
					if(drawParamTmp)
					{
						SetUpChangedDrawParam(drawParam, drawParamTmp);
                        drawParamList->Dirty(true);
                        auto cacheMapRow = theRealMapRow - 1; // real-map-row alkaa 1:st‰ ja cache-map-row 0:sta
                        MapViewDescTop(theDescTopIndex)->MapViewCache().MakeRowDirty(cacheMapRow);
						MapViewDirty(theDescTopIndex, false, false, true, false, false, true);
						return true;
					}
				}
			}
		}
		return false;
	}

	NFmiSynopStationPrioritySystem* SynopStationPrioritySystem(void)
	{
		return &itsSynopStationPrioritySystem;
	}

	NFmiSynopPlotSettings* SynopPlotSettings(void)
	{
		return &itsSynopPlotSettings;
	}

	bool LeftMouseButtonDown(void)
	{
		return fLeftMouseButtonDown;
	}
	void LeftMouseButtonDown(bool newState)
	{
		fLeftMouseButtonDown = newState;
		if(newState)
		{
			fRightMouseButtonDown = false;
			fMiddleMouseButtonDown = false;
			fMouseCaptured = true;
		}
		else
			fMouseCaptured = false;
	}
	bool RightMouseButtonDown(void)
	{
		return fRightMouseButtonDown;
	}
	void RightMouseButtonDown(bool newState)
	{
		fRightMouseButtonDown = newState;
		if(newState)
		{
			fLeftMouseButtonDown = false;
			fMiddleMouseButtonDown = false;
			fMouseCaptured = true;
		}
		else
			fMouseCaptured = false;
	}
	bool MiddleMouseButtonDown(void)
	{
		return fMiddleMouseButtonDown;
	}
	void MiddleMouseButtonDown(bool newState)
	{
		fMiddleMouseButtonDown = newState;
		if(newState)
		{
			fLeftMouseButtonDown = false;
			fRightMouseButtonDown = false;
			fMouseCaptured = true;
		}
		else
			fMouseCaptured = false;
	}

	bool MapMouseDragPanMode(void)
	{
		return fMapMouseDragPanMode;
	}
	void MapMouseDragPanMode(bool newState)
	{
		fMapMouseDragPanMode = newState;
	}

	bool ChangeLoggerPath(const string &theNewLoggerPath)
	{
		if(theNewLoggerPath != itsBasicConfigurations.LogFileDirectory())
		{
			if(WarnUserAboutNotExistingPath(theNewLoggerPath))
			{
				itsBasicConfigurations.LogFileDirectory(theNewLoggerPath);
				try
				{
					itsBasicConfigurations.StoreLoggerConfigurations();
				}
				catch(...)
				{
					// varoita k‰ytt‰j‰‰ ?!?!?!? 
				}
				return true;
			}
		}
		return false;
	}

	bool ChangeSmartToolPath(const std::string &theNewDirectory)
	{ 
		// SmartTool -polku muutetaan jos on tarpeen. Jos polkua ei ole varoitus ja mahd. sen luonti.
		// Palauttaa true jos muutos otettiin k‰yttˆˆn ja false jos ei tehd‰ mit‰‰n.
		if(theNewDirectory != itsMacroPathSettings.SmartToolPath(false))
		{
			if(WarnUserAboutNotExistingPath(theNewDirectory))
			{
				NFmiSettings::Set(string("MetEditor::SmartTools::LoadDirectory"), theNewDirectory, true);
				itsMacroPathSettings.SetOrigSmartToolPath(theNewDirectory);
				itsSmartToolInfo.LoadDirectory(itsMacroPathSettings.SmartToolPath(true), true);
				return true;
			}
		}
		return false;
	}

	bool ChangeViewMacroPath(const std::string &theNewDirectory)
	{ 
		// ViewMacro -polku muutetaan jos on tarpeen. Jos polkua ei ole varoitus ja mahd. sen luonti.
		// Palauttaa true jos muutos otettiin k‰yttˆˆn ja false jos ei tehd‰ mit‰‰n.
		if(theNewDirectory != itsMacroPathSettings.ViewMacroPath(false))
		{
			if(WarnUserAboutNotExistingPath(theNewDirectory))
			{
				NFmiSettings::Set(string("MetEditor::ViewMacro::LoadDirectory"), theNewDirectory, true);
				itsMacroPathSettings.SetOrigViewMacroPath(theNewDirectory);
				InitViewMacroSystem(false);
				return true;
			}
		}
		return false;
	}

	bool ChangeMacroParamPath(const std::string &theNewDirectory)
	{ 
		// ViewMacro -polku muutetaan jos on tarpeen. Jos polkua ei ole varoitus ja mahd. sen luonti.
		// Palauttaa true jos muutos otettiin k‰yttˆˆn ja false jos ei tehd‰ mit‰‰n.
		if(theNewDirectory != itsMacroPathSettings.MacroParamPath(false))
		{
			if(WarnUserAboutNotExistingPath(theNewDirectory))
			{
				NFmiSettings::Set(string("MetEditor::MacroParams::LoadDirectory"), theNewDirectory, true);
				itsMacroPathSettings.SetOrigMacroParamPath(theNewDirectory);
				InitMacroParamSystem(false);
				return true;
			}
		}
		return false;
	}

	bool ChangeDrawParamPath(const std::string &theNewDirectory)
	{ 
		// ViewMacro -polku muutetaan jos on tarpeen. Jos polkua ei ole varoitus ja mahd. sen luonti.
		// Palauttaa true jos muutos otettiin k‰yttˆˆn ja false jos ei tehd‰ mit‰‰n.
		if(theNewDirectory != itsMacroPathSettings.DrawParamPath(false))
		{
			if(WarnUserAboutNotExistingPath(theNewDirectory))
			{
				NFmiSettings::Set(string("MetEditor::DrawParams::LoadDirectory"), theNewDirectory, true);
				itsMacroPathSettings.SetOrigDrawParamPath(theNewDirectory);
				itsSmartInfoOrganizer->SetDrawParamPath(itsMacroPathSettings.DrawParamPath(true));
				return true;
			}
		}
		return false;
	}

	bool MakeMacroPathConfigurations(void)
	{
		string oldSmartToolPath(itsMacroPathSettings.SmartToolPath(false));
		string oldViewMacroPath(itsMacroPathSettings.ViewMacroPath(false));
		string oldMacroParamPath(itsMacroPathSettings.MacroParamPath(false));
		string oldDrawParamPath(itsMacroPathSettings.DrawParamPath(false));
		string oldLoggerPath(itsBasicConfigurations.LogFileDirectory());
		CFmiMacroPathSettings dlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(),
								oldSmartToolPath,
								oldViewMacroPath,
								oldMacroParamPath,
								oldDrawParamPath,
								oldLoggerPath
								);
		if(dlg.DoModal() == IDOK)
		{
			string newSmartToolPath = CT2A(dlg.SmartToolPathStr());
            string newViewMacroPath = CT2A(dlg.ViewMacroPathStr());
            string newMacroParamPath = CT2A(dlg.MacroParamPathStr());
            string newDrawParamPath = CT2A(dlg.DrawParamPathStr());
            string newLoggerPath = CT2A(dlg.LoggerPathStr());
			bool updateStatus = false;

			if(ChangeSmartToolPath(newSmartToolPath))
				updateStatus = true;

			if(ChangeViewMacroPath(newViewMacroPath))
				updateStatus = true;

			if(ChangeMacroParamPath(newMacroParamPath))
				updateStatus = true;

			if(ChangeDrawParamPath(newDrawParamPath))
				updateStatus = true;

			updateStatus = ChangeLoggerPath(newLoggerPath);

			if(updateStatus)
				LogMessage("Macro path settings changed.", CatLog::Severity::Info, CatLog::Category::Configuration);

			return updateStatus;
		}
		return false;
	}

	bool WarnUserAboutNotExistingPath(const string &thePath)
	{
		string absolutePath = PathUtils::getAbsoluteFilePath(thePath, itsBasicConfigurations.ControlPath());
		if(!NFmiFileSystem::DirectoryExists(absolutePath))
		{
			string errStr("Path '");
			errStr += absolutePath;
			errStr += "'\ndoes not exist.\nDo you want to create it?";
			if(::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(errStr.c_str()), _TEXT("Path doesn't exist."), MB_YESNO) == IDYES)
			{
				if(!NFmiFileSystem::CreateDirectory(absolutePath))
				{
					string errStr2("Path '");
					errStr2 += absolutePath;
					errStr2 += "'\ncouldn't be created, path change is ignored.";
					::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(errStr2.c_str()), _TEXT("Cannot create path"), IDOK);
					return false;
				}
				return true;
			}
			else
				return false;
		}
		return true;
	}
	void SetApplicationTitle(const std::string &theTitle)
	{
		itsBasicConfigurations.SetApplicationTitle(theTitle);
	}

	std::string GetApplicationTitle(void)
	{
        auto finalTitleString = itsBasicConfigurations.GetApplicationTitle();
        if(!itsLastLoadedViewMacroName.empty())
        {
            finalTitleString += " (ViewMacro: " + itsLastLoadedViewMacroName + ")";
        }
		return finalTitleString;
	}

	NFmiObsComparisonInfo& ObsComparisonInfo(void)
	{
		return itsObsComparisonInfo;
	}

	NFmiDrawParamList* CrossSectionViewDrawParamList(int theRowIndex)
	{
		if(itsCrossSectionDrawParamListVector)
		{
			NFmiPtrList<NFmiDrawParamList>::Iterator it = itsCrossSectionDrawParamListVector->Index(theRowIndex);
			return it.CurrentPtr();
		}
		return 0;
	}

	bool CreateCrossSectionViewPopup(int theRowIndex)
	{
		itsCurrentCrossSectionRowIndex = theRowIndex;
		delete itsPopupMenu;
		itsPopupMenu = 0;
		fOpenPopup = false;
		itsPopupMenu = new NFmiMenuItemList;

		MenuCreationSettings menuSettings;
		menuSettings.SetCrossSectionSettings(kFmiAddParamCrossSectionView);
		CreateParamSelectionBasePopup(menuSettings, itsPopupMenu, "NormalWordCapitalAdd");

		menuSettings.SetCrossSectionSettings(kFmiAddAsOnlyParamCrossSectionView);
		CreateParamSelectionBasePopup(menuSettings, itsPopupMenu, "CrossSectionViewSelectionPopUpAddAsOnly");

		NFmiDrawParamList * dpList = this->CrossSectionViewDrawParamList(theRowIndex);
		std::string menuString = ::GetDictionaryString("NormalWordCapitalShow");
		AddToCrossSectionPopupMenu(itsPopupMenu, dpList, menuString, kFmiShowParamCrossSectionView);
		menuString = ::GetDictionaryString("NormalWordCapitalHide");
		AddToCrossSectionPopupMenu(itsPopupMenu, dpList, menuString, kFmiHideParamCrossSectionView);
		menuString = ::GetDictionaryString("NormalWordCapitalRemove");
		AddToCrossSectionPopupMenu(itsPopupMenu, dpList, menuString, kFmiRemoveParamCrossSectionView);
		menuString = ::GetDictionaryString("MapViewParamOptionPopUpActivate");
		AddToCrossSectionPopupMenu(itsPopupMenu, dpList, menuString, kFmiActivateCrossSectionDrawParam);
		menuString = ::GetDictionaryString("NormalWordCapitalProperties");
		AddToCrossSectionPopupMenu(itsPopupMenu, dpList, menuString, kFmiModifyCrossSectionDrawParam);

		// copy/paste komennot t‰h‰n
		menuString = "Copy draw options";
		AddToCrossSectionPopupMenu(itsPopupMenu, dpList, menuString, kFmiCopyDrawParamOptionsCrossSectionView);
		if(fCopyPasteDrawParamAvailableYet)
		{
			menuString = "Paste draw options";
			AddToCrossSectionPopupMenu(itsPopupMenu, dpList, menuString, kFmiPasteDrawParamOptionsCrossSectionView);
		}


		menuString = ::GetDictionaryString("CrossSectionViewSelectionPopUpRemoveAll");
        auto menuItem = std::make_unique<NFmiMenuItem>(-1, menuString, NFmiDataIdent(), kFmiRemoveAllParamsCrossSectionView, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable);
		itsPopupMenu->Add(std::move(menuItem));

        menuString = ::GetDictionaryString("Remove all params from all rows");
        menuItem.reset(new NFmiMenuItem(CtrlViewUtils::kFmiCrossSectionView, menuString, NFmiDataIdent(), kFmiRemoveAllParamsFromAllRows, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable));
        itsPopupMenu->Add(std::move(menuItem));

		menuString = "Copy all row params";
		menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiCopyDrawParamsFromCrossSectionViewRow, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable));
		itsPopupMenu->Add(std::move(menuItem));
		if(fCopyPasteCrossSectionDrawParamListUsedYet)
		{
			menuString = "Paste all row params";
			menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiPasteDrawParamsToCrossSectionViewRow, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kEditable));
			itsPopupMenu->Add(std::move(menuItem));
		}

// ********* muuta n‰yttˆrivin kaikkien datojen tuottajat halutuiksi *********
		AddChangeAllProducersToParamSelectionPopup(static_cast<unsigned int>(-1), itsPopupMenu, kFmiChangeAllProducersInCrossSectionRow, true);
// ********* muuta n‰yttˆrivin kaikkien datojen tuottajat halutuiksi *********

		// ************ t‰ss‰ muutetaan kaikki rivin datatyypit (fiksataan ongelma mik‰ syntyi kun LAPS muuttui kViewable:ista kAnalyze -tyyppiseksi)
		AddChangeAllDataTypesToParamSelectionPopup(static_cast<unsigned int>(-1), itsPopupMenu, kFmiChangeAllDataTypesInCrossSectionRow);

		// ********** lis‰t‰‰n mahdollinen aika-moodin aikas‰‰tˆ komento osa *************************
		AddTimeSettingToCrossSectionPopupMenu(itsPopupMenu);

		// ********** lis‰t‰‰n mahdollinen trajektori komento osa *************************
		AddTrajectoryOptionsToCrossSectionPopupMenu(itsPopupMenu);

		if(!itsPopupMenu->InitializeCommandIDs(itsPopupMenuStartId))
			return false;

		fOpenPopup = true;
		return true;
	}

void AddTimeSettingToCrossSectionPopupMenu(NFmiMenuItemList *thePopupMenu)
{
	if(CrossSectionSystem()->GetCrossMode() == NFmiCrossSectionSystem::kTime && !TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
	{
        auto item = std::make_unique<NFmiMenuItem>(-1, ::GetDictionaryString("CrossSectionViewSelectionPopUpSetTimes")
											,NFmiDataIdent()
											,kFmiCrossSectionSetTrajectoryTimes
											,NFmiMetEditorTypes::kFmiParamsDefaultView
											,nullptr
											,NFmiInfoData::kViewable);
		thePopupMenu->Add(std::move(item));
	}
}

void AddTrajectoryOptionsToCrossSectionPopupMenu(NFmiMenuItemList *thePopupMenu)
{
	if(TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
	{
        auto menuItem = std::make_unique<NFmiMenuItem>(-1, "Trajectories", NFmiDataIdent(), kFmiCrossSectionSetTrajectoryTimes, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kViewable); // NFmiInfoData::kViewable on vain merkityksetˆn default arvo
		NFmiMenuItemList *menuList = new NFmiMenuItemList;
        auto item = std::make_unique<NFmiMenuItem>(-1, ::GetDictionaryString("CrossSectionViewSelectionPopUpSetTimes")
											,NFmiDataIdent()
											,kFmiCrossSectionSetTrajectoryTimes
											,NFmiMetEditorTypes::kFmiParamsDefaultView
											,nullptr
											,NFmiInfoData::kViewable);
		menuList->Add(std::move(item));

		item.reset(new NFmiMenuItem(-1, ::GetDictionaryString("CrossSectionViewSelectionPopUpSetParams")
											,NFmiDataIdent()
											,kFmiCrossSectionSetTrajectoryParams
											,NFmiMetEditorTypes::kFmiParamsDefaultView
											,nullptr
											,NFmiInfoData::kViewable));
		menuList->Add(std::move(item));

		menuItem->AddSubMenu(menuList);
		thePopupMenu->Add(std::move(menuItem));
	}
}

void AddToCrossSectionPopupMenu(NFmiMenuItemList *thePopupMenu, NFmiDrawParamList *theDrawParamList, const std::string &theMenuTitle, FmiMenuCommandType theMenuCommand)
{
	if(theDrawParamList && theDrawParamList->NumberOfItems() > 0)
	{
        auto menuItem = std::make_unique<NFmiMenuItem>(-1, theMenuTitle, NFmiDataIdent(), theMenuCommand, NFmiMetEditorTypes::kFmiParamsDefaultView, nullptr, NFmiInfoData::kViewable); // NFmiInfoData::kViewable on vain merkityksetˆn default arvo
		NFmiMenuItemList *menuList = new NFmiMenuItemList;
		int aIndex = 1;
		for(theDrawParamList->Reset(); theDrawParamList->Next();)
		{
			boost::shared_ptr<NFmiDrawParam> drawParam = theDrawParamList->Current();
			NFmiDataIdent param(drawParam->Param());
			std::string usedParamName = drawParam->ParameterAbbreviation();
			bool macroParam = (drawParam->DataType() == NFmiInfoData::kCrossSectionMacroParam);
			if(macroParam)
				param.GetParam()->SetName(drawParam->InitFileName());
			else
			{ // muuten laitetaan parametrin nimeen myˆs mallitietoa, jos data lˆytyy
				boost::shared_ptr<NFmiFastQueryInfo> info = InfoOrganizer()->Info(drawParam, true, true);
				if(info)
				{
					info->FirstLevel(); // varmistetaan ett‰ 1. leveli kohdalla
					usedParamName += " ";
					usedParamName += ProducerSystem().GetProducerAndLevelTypeString(*info->Producer(), *info->Level(), info->OriginTime(), true);
				}
			}

            auto item = std::make_unique<NFmiMenuItem>(-1, usedParamName
												,param
												,theMenuCommand
												,NFmiMetEditorTypes::kFmiParamsDefaultView
												,&drawParam->Level()
												,drawParam->DataType()
												,aIndex
												,drawParam->ViewMacroDrawParam());
			item->ExtraParam(1); // kun parametria aktivoidaan (tai tulevaisuudessa tehd‰‰n mit‰ vain), k‰ytet‰‰n extraParamia kertomaan miss‰ parametri on, 0=karttan‰yttˆ, 1=poikkileikkaus ja 2=aikasarja
			menuList->Add(std::move(item));
			aIndex++;
		}
		menuItem->AddSubMenu(menuList);
		thePopupMenu->Add(std::move(menuItem));
	}
}

	// editorin pit‰‰ siivota aina v‰lill‰ (latauksen yhteydess‰, koska niit‰ tehd‰‰n siin‰ yhteydess‰)
	// omat-hakemistoa, ettei sinne j‰‰ ‰lyttˆmi‰ m‰‰ri‰ tiedostoja roikkumaan
	bool CleanFilesFromDirectory(const NFmiString& theWorkingPath, int theTimeLimitInHours)
	{
		NFmiTime currentTime;
		string wantedPath(static_cast<const char*>(theWorkingPath));
		if(NFmiFileSystem::DirectoryExists(wantedPath))
		{
			const std::list<std::string> fileList = NFmiFileSystem::DirectoryFiles(wantedPath);
			std::list<std::string>::const_iterator it = fileList.begin();
			for( ; it != fileList.end(); ++it)
			{
				string fullFileName(wantedPath);
				fullFileName += *it;
				time_t tim = NFmiFileSystem::FileModificationTime(fullFileName);
				if(tim < 0)
					continue; // NFmiTime:n konstruktori kaatuu, jos tim on negatiivinen, ja luku on negatiivinen jos tiedopstoa ei kaikesta huolimatta lˆydyk‰‰n.
				NFmiTime aTime(tim);
				if(currentTime.DifferenceInHours(aTime) > theTimeLimitInHours)
					NFmiFileSystem::RemoveFile(fullFileName);
			}
			return true;
		}
		return false;
	}

	NFmiMTATempSystem& GetMTATempSystem(void)
	{
		return itsMTATempSystem;
	}

	bool TimeSerialDataViewOn(void)
	{
		return fTimeSerialDataViewOn;
	}

	void TimeSerialDataViewOn(bool newValue)
	{
		fTimeSerialDataViewOn = newValue;
	}

	bool SynopDataGridViewOn(void)
	{
		return fSynopDataGridViewOn;
	}

	void SynopDataGridViewOn(bool newState)
	{
		fSynopDataGridViewOn = newState;
	}

	const NFmiPoint& GetSynopHighlightLatlon(void)
	{
		return itsSynopHighlightLatlon;
	}
	bool ShowSynopHighlight(void)
	{
		return fShowSynopHighlightStationMarker;
	}
	void SetHighlightedSynopStation(const NFmiPoint &theLatlon, int theWmoId, bool fShowHighlight)
	{
		itsSynopHighlightStationWmoId = theWmoId;
		fShowSynopHighlightStationMarker = fShowHighlight;
		itsSynopHighlightLatlon = theLatlon;
	}

	void MouseOnMapView(bool newState)
	{
		fMouseOnMapView = newState;
	}
	bool MouseOnMapView(void)
	{
		return fMouseOnMapView;
	}

	void LastSelectedSynopWmoId(int theWmoId)
	{
		itsLastSelectedSynopWmoId = theWmoId;
	}
	int LastSelectedSynopWmoId(void)
	{
		return itsLastSelectedSynopWmoId;
	}
	bool ShowMouseHelpCursorsOnMap(void)
	{
		return fShowMouseHelpCursorsOnMap;
	}
	void ShowMouseHelpCursorsOnMap(bool newState)
	{
		fShowMouseHelpCursorsOnMap = newState;
	}

	// poistaa viimeisen osan polusta
	// c:\data\src\inc\ -> c:\data\src\
	// eli inc pois esimerkist‰
	void RemoveLastPartOfDirectory(string &thePath)
	{
		NFmiStringTools::TrimR(thePath, '\\');
		NFmiStringTools::TrimR(thePath, '/');
		string::size_type pos1 = thePath.find_last_of('/');
		string::size_type pos2 = thePath.find_last_of('\\');
		string::size_type usedPos = string::npos;
		if(pos1 != string::npos && pos2 != string::npos)
		{
			if(pos1 < pos2)
				usedPos = pos2;
			else
				usedPos = pos1;
		}
		else if(pos1 != string::npos)
			usedPos = pos1;
		else if(pos2 != string::npos)
			usedPos = pos2;

		if(usedPos != string::npos)
			thePath = string(thePath.begin(), thePath.begin()+usedPos+1);
	}

	void ChangeCurrentViewMacroDirectory(const string & theDirectoryName, bool fUseRootPathAsBase)
	{
        if(fUseRootPathAsBase)
        {
            itsViewMacroPath = itsRootViewMacroPath;
            itsViewMacroPath += theDirectoryName;
        }
        else
        {
            // nimi tulee <> sulkujen sis‰ll‰ joten ne on poistettava ensin
            std::string usedDirectoryName(theDirectoryName);
            NFmiStringTools::TrimL(usedDirectoryName, '<');
            NFmiStringTools::TrimR(usedDirectoryName, '>');

            if(usedDirectoryName == "..")
            {
                RemoveLastPartOfDirectory(itsViewMacroPath);
            }
            else
            {
                itsViewMacroPath += usedDirectoryName;
            }
        }
        PathUtils::addDirectorySeparatorAtEnd(itsViewMacroPath);
        // HUOM! t‰ss‰ ei ole tarkoitus p‰ivitt‰‰ rootti-hakemistoa, koska nyt menn‰‰n
		// vain johonkin rootin alihakemistoihin
		RefreshViewMacroList();
	}

	bool CreateNewViewMacroDirectory(const std::string &thePath)
	{
		std::string usedDirectoryName(thePath);
		NFmiStringTools::TrimL(usedDirectoryName, '<');
		NFmiStringTools::TrimR(usedDirectoryName, '>');
		if(NFmiFileSystem::CreateDirectory(itsViewMacroPath + usedDirectoryName))
		{
			RefreshViewMacroList();
			return true;
		}
		return false;
	}

	bool SelectLanguage(void)
	{
		try
		{
			int usedLanguage = -1;
            if(Language() == kFinnish)
				usedLanguage = 0;
            else if(Language() == kEnglish)
				usedLanguage = 1;
            else
				throw std::runtime_error("Cannot find currently used language,\nerror in program or in configuration files.");
			CFmiLanguageSelectionDlg dlg(usedLanguage, AfxGetMainWnd());
			if(dlg.DoModal() == IDOK)
			{
				int selectedLanguage = dlg.LanguageSelection();
				switch(selectedLanguage)
				{
				case 0:
					Language(kFinnish);
					break;
				case 1:
					Language(kEnglish);
					break;
				default:
					throw std::runtime_error("Error in program language selection incorrect,\ncannot change language.");
					break;
				}
				return true;
			}
		}
		catch(std::exception &e)
		{
			::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(e.what()), _TEXT("Problem with language setting!"), MB_OK);
		}
		return false;
	}
	FmiLanguage Language(void)
	{
		return itsBasicConfigurations.Language();
	}
	void Language(FmiLanguage newValue)
	{
		itsBasicConfigurations.Language(newValue);
	}

	void RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, bool fMakeAreaViewDirty, bool fClearCache, int theWantedMapViewDescTop = -1)
	{
		if(ApplicationInterface::GetApplicationInterfaceImplementation)
            ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs(reasonForUpdate, fMakeAreaViewDirty, fClearCache, theWantedMapViewDescTop);
	}

	// Sama koodi lˆytyy NFmiStationViewHandler-luokasta
	NFmiSilamStationList::Station GetClosestSilamStation(NFmiSilamStationList &theLocations, const NFmiLocation &theSearchPlace)
	{
		NFmiSilamStationList::Station closestLoc;
		closestLoc.itsLatlon = kMissingLatlon;
		checkedVector<NFmiSilamStationList::Station> &locList = theLocations.Locations();
		double minDist = 9999999999;
		for(size_t i=0; i < locList.size(); i++)
		{
			double currentDist = theSearchPlace.Distance(locList[i].itsLatlon);
			if(currentDist < minDist)
			{
				minDist = currentDist;
				closestLoc = locList[i];
			}
		}

		return closestLoc;
	}

	// Melkein vastaava koodi lˆytyy NFmiStationViewHandler::ComposeSilamLocationsToolTipText -metodista
	// kannattaa varmaan yhdist‰‰ koodistoa v‰h‰n.
	NFmiPoint GetClosestSilamPlace()
	{
		NFmiPoint latlon = kMissingLatlon;
		if(TrajectorySystem()->TrajectoryViewOn())
		{
			double maxAllowedDistanceInMeters = 200*1000;
			NFmiLocation wantedLoc(itsToolTipLatLonPoint);
			NFmiSilamStationList::Station stat1 = GetClosestSilamStation(TrajectorySystem()->NuclearPlants(), wantedLoc);
			NFmiSilamStationList::Station stat2 = GetClosestSilamStation(TrajectorySystem()->OtherPlaces(), wantedLoc);
			double minDist1 = wantedLoc.Distance(stat1.itsLatlon);
			double minDist2 = wantedLoc.Distance(stat2.itsLatlon);
			if(minDist1 < minDist2)
			{
				if(minDist1 < maxAllowedDistanceInMeters)
					return stat1.itsLatlon;
			}
			else
			{
				if(minDist2 < maxAllowedDistanceInMeters)
					return stat2.itsLatlon;
			}
		}
		return latlon;
	}


	// T‰m‰ funktio oli alun peri kahdessa paikassa NFmiStationView ja NFmiStationViewHandler:issa
	// Laitoin funktion t‰nne ett‰ ei tarvitse duplikoida hankalaa koodia.
	void SelectLocations(unsigned int theDescTopIndex, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const boost::shared_ptr<NFmiArea> &theMapArea, const NFmiPoint& theLatLon
						,const NFmiMetTime &theTime, int theSelectionCombineFunction, unsigned long theMask
						,bool fMakeMTAModeAdd // vain tietyist‰ paikoista kun t‰t‰ metodia kutsutaan, saa luotauksen lis‰t‰ (left buttom up karttan‰ytˆll‰ l‰hinn‰)
						,bool fDoOnlyMTAModeAdd)
	{
		bool doMTAModeAdd = fDoOnlyMTAModeAdd || (fMakeMTAModeAdd && GetMTATempSystem().TempViewOn());
		bool doTrajectoryModeAdd = (fMakeMTAModeAdd || (theSelectionCombineFunction == 1)) && TrajectorySystem()->TrajectoryViewOn();
		if(fDoOnlyMTAModeAdd == false)
		{ // ei tehd‰ oikean hiiren klikkauksen displayedMask valintaa, jos tehd‰‰n MTA-luotaus nollaus
            if(AllowRightClickDisplaySelection() || theMask != NFmiMetEditorTypes::kFmiDisplayedMask)
            {
                PreciseTimeSerialLatlonPoint(theLatLon); // T‰m‰ tarkka arvo asetetaan nyt aina talteen
                ResetOutOfEditedAreaTimeSerialPoint();
                fEditedPointsSelectionChanged = true;
                itsLocationSelectionTool2->SelectLocations(theInfo
                    , theLatLon
                    , theMapArea
                    , FmiSelectionCombineFunction(theSelectionCombineFunction)
                    , theMask
                    , MapViewDescTop(theDescTopIndex)->ViewGridSize());
                if(theInfo && theInfo->Grid() && theInfo->Area()->IsInside(theLatLon) == false)
                    OutOfEditedAreaTimeSerialPoint(theLatLon);
            }
		}

		// jos ollaan luotaus n‰ytˆn MTA-moodissa, lis‰t‰‰n myˆs valittu luotaus info dokumenttiin
		if(doMTAModeAdd)
		{
			if(theMask == NFmiMetEditorTypes::kFmiDisplayedMask) // HUOM! Hiiren oikealla saa valita useita luotauspaikkoja hiiren oikealla, vaikka se olisi estetty aikasarja ikkunassa
			{ // jos oikealla klikkaus, lis‰t‰‰n yksi luotaus listaan
				NFmiMTATempSystem::TempInfo tempInfo(theLatLon, theTime, GetMTATempSystem().CurrentProducer());
				GetMTATempSystem().AddTemp(tempInfo);
//				MakeEditorDirty(theDescTopIndex, true, true, true);
				MapViewDirty(theDescTopIndex, false, false, true, false, false, false);
			}
			else
			{
				// Jos vasen klikkaus karttan‰ytˆll‰ ja ollaan MTA-moodissa, nollataan luotaukset ja lis‰t‰‰n yksi
				GetMTATempSystem().ClearTemps();
				NFmiMTATempSystem::TempInfo tempInfo(theLatLon, theTime, GetMTATempSystem().CurrentProducer());
				GetMTATempSystem().AddTemp(tempInfo);
				MapViewDirty(theDescTopIndex, false, false, true, false, false, false);
			}
		}
		if(doTrajectoryModeAdd)
		{
			if(theSelectionCombineFunction == 0) // eli pelkk‰ hiiren klikkaus
				TrajectorySystem()->SelectedLatLon(theLatLon);
			else // ctrl tai shift on ollut pohjassa
			{
				NFmiPoint nearestNuclearPlantLatlon = GetClosestSilamPlace();
				if(nearestNuclearPlantLatlon != kMissingLatlon)
					TrajectorySystem()->SelectedLatLon(nearestNuclearPlantLatlon);
				else
					TrajectorySystem()->SelectedLatLon(theLatLon);
			}
            // Trajektorien muutos vaikuttaa kaikkiin karttan‰ytˆn ruutuihin joka rivill‰ (siksi 3. parametri on true)
			MapViewDirty(theDescTopIndex, false, true, true, false, false, false);
		}
	}


	NFmiTrajectorySystem* TrajectorySystem(void)
	{
		return itsTrajectorySystem;
	}

	void DoTEMPDataUpdate(const std::string &theTEMPCodeTextStr, std::string &theTEMPCodeCheckReportStr, bool fJustCheckTEMPCode)
	{
		itsLastTEMPDataStr = theTEMPCodeTextStr;
		NFmiQueryData *newData = DecodeTEMP::MakeNewDataFromTEMPStr(theTEMPCodeTextStr, theTEMPCodeCheckReportStr, WmoStationInfoSystem(), itsRawTempUnknownStartLonLat, NFmiProducer(kFmiRAWTEMP, "TEMP"), fRawTempRoundSynopTimes);
		if(newData && fJustCheckTEMPCode == false)
		{
			// otetaan TEMP koodi data k‰yttˆˆn jos lˆytyi ja  ei ollut pelkk‰ tarkistus operaatio
			AddQueryData(newData, "TEMPData.sqd", "TEMPDataFilePattern", NFmiInfoData::kTEMPCodeSoundingData, "");
		}
		else
			delete newData;
	}

	void ClearTEMPData(void)
	{
        // tyhjennet‰‰n TEMP-data
		InfoOrganizer()->ClearData(NFmiInfoData::kTEMPCodeSoundingData); 
		// Karttan‰yttˆ on piirrett‰v‰ uudestaan, koska siell‰ saattaa n‰ky‰  uusia luotaus merkkej‰.
		MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, false, true, false, false, false);
	}

	const std::string& LastTEMPDataStr(void)
	{
		return itsLastTEMPDataStr;
	}

	void CleanDataDirectories(void)
	{
		try
		{
			if(IsOperationalModeOn())
			{ // putsataan tiedostoja vain operatiivisessa moodissa
				LogMessage("Starting to clean wanted directories and file patterns...", CatLog::Severity::Info, CatLog::Category::Operational);
				FileCleanerSystem().DoCleaning();
			}
		}
		catch(std::exception &e)
		{
			LogMessage(e.what(), CatLog::Severity::Warning, CatLog::Category::Operational);
		}
	}

	NFmiFileCleanerSystem& FileCleanerSystem (void)
	{
		return itsFileCleanerSystem;
	}

	void SetMacroParamDataGridSize(int xSize, int ySize)
	{
		InfoOrganizer()->SetMacroParamDataGridSize(xSize, ySize);
		SaveMacroParamDataGridSizeSettings();
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Set macro-param general data grid size", TRUE, TRUE);
	}

	void SaveMacroParamDataGridSizeSettings(void)
	{
		NFmiPoint gridSize(InfoOrganizer()->GetMacroParamDataGridSize());
        ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MacroParamGridSizeX(static_cast<int>(gridSize.X()));
        ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MacroParamGridSizeY(static_cast<int>(gridSize.Y()));
	}

#ifndef DISABLE_CPPRESTSDK
    HakeMessage::Main& WarningCenterSystem(void)
	{
		return itsWarningCenterSystem;
	}
#endif // DISABLE_CPPRESTSDK

	void UpdateCrossSectionMacroParamDataSize(void)
	{
        DoVerboseFunctionStartingLogReporting(__FUNCTION__);
		InfoOrganizer()->UpdateCrossSectionMacroParamDataSize(CrossSectionSystem()->WantedMinorPointCount(), CrossSectionSystem()->VerticalPointCount());
	}

	void RemoveAllParamsFromCrossSectionViewRow(int theRowIndex)
	{
		RemoveAllCrossSectionViews(theRowIndex);
	}

	NFmiProducerSystem& ProducerSystem(void)
	{
		return itsProducerSystem;
	}
	NFmiProducerSystem& ObsProducerSystem(void)
	{
		return itsObsProducerSystem;
	}
	NFmiProducerSystem& SatelImageProducerSystem(void)
	{
		return itsSatelImageProducerSystem;
	}

	void AddMacroParamToCrossSectionView(int theViewRow, const std::string &theName)
	{
		NFmiMenuItem menuItem(-1, theName, static_cast<FmiParameterName>(998), kFmiAddParamCrossSectionView, NFmiMetEditorTypes::kFmiParamsDefaultView, 0, NFmiInfoData::kCrossSectionMacroParam, theViewRow);
		AddCrossSectionView(menuItem, theViewRow, false);
	}

	void ReportInfoOrganizerDataConsumption(void)
	{
		if(itsSmartInfoOrganizer)
		{
			// report here
			int dataCount = itsSmartInfoOrganizer->CountData();
			double dataSize = itsSmartInfoOrganizer->CountDataSize();
			int qDataConstructors = NFmiQueryData::itsConstructorCalls;
			int qDatadestructors = NFmiQueryData::itsDestructorCalls;
			std::string logStr("SmartMet has ");
			logStr += NFmiStringTools::Convert(dataCount);
			logStr += " queryData objects, size ";
			logStr += NFmiValueString::GetStringWithMaxDecimalsSmartWay(dataSize/(1024.*1024.), 1);
			logStr += " MB. (constr. ";
			logStr += NFmiStringTools::Convert(qDataConstructors);
			logStr += " - destr. ";
			logStr += NFmiStringTools::Convert(qDatadestructors);
			logStr += " = ";
			logStr += NFmiStringTools::Convert(qDataConstructors - qDatadestructors);
			logStr += ") + mod-drawp count ";
			logStr += NFmiStringTools::Convert(itsModifiedPropertiesDrawParamList.Size());
			logStr += ")";
			LogMessage(logStr, CatLog::Severity::Info, CatLog::Category::Operational);
		}
	}

	void InitializeWantedDrawParams(FastDrawParamList &theDrawParamList, boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool useWithViewMacros)
	{
		FastDrawParamList::FastContainer::iterator it = theDrawParamList.Begin();
		FastDrawParamList::FastContainer::iterator endIt = theDrawParamList.End();
		for(; it != endIt; ++it)
		{
			boost::shared_ptr<NFmiDrawParam> aDrawParam = it->second;
			if(useWithViewMacros || aDrawParam->ViewMacroDrawParam() == false)
			{
				if(theDrawParam->Param().GetParamIdent() == aDrawParam->Param().GetParamIdent() && theDrawParam->Level() == aDrawParam->Level())
					aDrawParam->Init(theDrawParam, true);
			}
		}
	}

	void InitializeWantedDrawParams(NFmiDrawParamList &theDrawParamList, boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool useWithViewMacros)
	{
		for(theDrawParamList.Reset(); theDrawParamList.Next(); )
		{
			boost::shared_ptr<NFmiDrawParam> aDrawParam = theDrawParamList.Current();
			if(useWithViewMacros || aDrawParam->ViewMacroDrawParam() == false)
			{
				if(theDrawParam->Param().GetParamIdent() == aDrawParam->Param().GetParamIdent() && theDrawParam->Level() == aDrawParam->Level())
					aDrawParam->Init(theDrawParam, true);
			}
		}
	}

	void InitializeWantedDrawParams(NFmiPtrList<NFmiDrawParamList> &theDrawParamListVector, boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool useWithViewMacros)
	{
		NFmiPtrList<NFmiDrawParamList>::Iterator iter = theDrawParamListVector.Start();
		for( ; iter.Next(); )
			InitializeWantedDrawParams((*iter.CurrentPtr()), theDrawParam, useWithViewMacros);
	}
	// Kun jonkun parametrin piirto-ominaisuuksia muutetaan, eiv‰t ne tulevoimaan kuin sille yhdelle
	// l‰mpˆtilalle tai paine parametrille mit‰ s‰‰det‰‰n. Tai jos on erilaisia malli datoja, niill‰ on jo
	// ladattu valmiiksi omat drawparamit vaikka niit‰ ei katsottaisikaan ja niihink‰‰n ei tule muutokset
	// voimaan, ennen kuin editori k‰ynnistet‰‰n uudestaan. T‰m‰ mahdollistaa sen ett‰
	// piirto-ominaisuudet saadaan heti k‰yttˆˆn esim. kaikille l‰mpˆtila (T eli par id 4) parametreille.
	// Tekee siis vain piirto-ominaisuuksien kopioinnin.
	void TakeDrawParamInUseEveryWhere(boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool useInMap, bool useInTimeSerial, bool useInCrossSection, bool useWithViewMacros)
	{
		// 1. k‰y l‰pi kartta drawparam listat (ota huomioon view-macrot)
		if(useInMap)
		{
			unsigned int ssize = static_cast<unsigned int>(itsMapViewDescTopList.size());
			for(unsigned int i = 0; i<ssize; i++)
				InitializeWantedDrawParams(*(MapViewDescTop(i)->DrawParamListVector()), theDrawParam, useWithViewMacros);
		}
		// 2. k‰y l‰pi aikasarja drawparam listat
		if(useInTimeSerial)
			InitializeWantedDrawParams(*itsTimeSerialViewDrawParamList, theDrawParam, useWithViewMacros);
		// 3. k‰y l‰pi poikkileikkaus drawparamit
		if(useInCrossSection)
			InitializeWantedDrawParams(*itsCrossSectionDrawParamListVector, theDrawParam, useWithViewMacros);
		// 4. k‰y l‰pi alussa (kaikelle datalle) tehty drawparamlista
		InitializeWantedDrawParams(itsModifiedPropertiesDrawParamList, theDrawParam, useWithViewMacros);
	}

	NFmiHelpEditorSystem& HelpEditorSystem(void)
	{
		return itsHelpEditorSystem;
	}

	bool ActivateParamSelectionDlgAfterLeftDoubleClick(void)
	{
		return fActivateParamSelectionDlgAfterLeftDoubleClick;
	}
	void ActivateParamSelectionDlgAfterLeftDoubleClick(bool newValue)
	{
		fActivateParamSelectionDlgAfterLeftDoubleClick = newValue;
	}

	const NFmiPoint& CrossSectionViewSizeInPixels(void)
	{
		return itsCrossSectionViewSizeInPixels;
	}

	void CrossSectionViewSizeInPixels(const NFmiPoint& newSize)
	{
		itsCrossSectionViewSizeInPixels = newSize;
	}

	// laskee poikkileikkausn‰yttˆruudukon yhden ruudun koon pikseleiss‰
	NFmiPoint ActualCrossSectionBitmapSizeInPixels(void)
	{
		return NFmiPoint(itsCrossSectionViewSizeInPixels.X() * itsCrossSectionDataViewFrame.Width()
							,itsCrossSectionViewSizeInPixels.Y() * itsCrossSectionDataViewFrame.Height());
	}

	void CrossSectionDataViewFrame(const NFmiRect &theRect)
	{
		itsCrossSectionDataViewFrame = theRect;
	}

	const NFmiRect& CrossSectionDataViewFrame(void)
	{
		return itsCrossSectionDataViewFrame;
	}

	void MustDrawTimeSerialView(bool newValue)
	{
		fMustDrawTimeSerialView = newValue;
	}
	bool MustDrawTimeSerialView(void)
	{
		return fMustDrawTimeSerialView;
	}
	void MustDrawTempView(bool newValue)
	{
		fMustDrawTempView = newValue;
	}
	bool MustDrawTempView(void)
	{
		return fMustDrawTempView;
	}
	void MustDrawCrossSectionView(bool newValue)
	{
		fMustDrawCrossSectionView = newValue;
	}
	bool MustDrawCrossSectionView(void)
	{
		return fMustDrawCrossSectionView;
	}

	void BorrowParams(unsigned int theDescTopIndex, int row)
	{
		NFmiDrawParamList* drawParamListFrom = DrawParamListWithRealRowNumber(theDescTopIndex, row); // row on absoluuttinen rivi
		NFmiDrawParamList* drawParamListTo = DrawParamList(theDescTopIndex, ActiveViewRow(theDescTopIndex)); // itsActiveViewRow on suhteutettuna n‰kyv‰‰n rivistˆˆn
		if(drawParamListFrom && drawParamListTo && (drawParamListFrom != drawParamListTo))
		{
			if(drawParamListTo->HasBorrowedParams())
				drawParamListTo->ClearBorrowedParams();
			else
				drawParamListTo->BorrowParams(*drawParamListFrom);

			MapViewDirty(theDescTopIndex, false, true, true, false, false, true); // laitetaan viela kaikki ajat likaisiksi cachesta
            MacroParamDataCache().update(theDescTopIndex, row, *drawParamListTo);
		}
	}

	void GetDataFromQ2Server(const std::string &theURLStr,
							const std::string &theParamsStr,
							bool fUseBinaryData,
							int theUsedCompression,
							NFmiDataMatrix<float> &theDataMatrixOut,
							std::string &theExtraInfoStrOut)
	{
		itsQ2Client.GetDataFromQ2Server(theURLStr, theParamsStr, fUseBinaryData, static_cast<CompressionType>(theUsedCompression),
										theDataMatrixOut, theExtraInfoStrOut, itsQ2ServerInfo.LogQ2Requests());
	}

	std::vector<NFmiProducer>& ExtraSoundingProducerList(void)
	{
		return itsExtraSoundingProducerList;
	}

	void InitExtraSoundingProducerListFromSettings(void)
	{
        DoVerboseFunctionStartingLogReporting(__FUNCTION__);
		itsExtraSoundingProducerList.clear();

		try
		{
			string producersStr = NFmiSettings::Optional<string>("MetEditor::ExtraSoundingProducers", "");
			if(producersStr.empty() == false)
			{
				checkedVector<string> prodStrVec = NFmiStringTools::Split<checkedVector<string> >(producersStr, ",");
				if(prodStrVec.size() % 2 != 0)
					throw runtime_error(string("Odd number parameters in producer list, give 'id1,name1,id2,name2,...' type pairs.\n") + producersStr);

				for(size_t i=0; i<prodStrVec.size(); i+=2)
				{
					unsigned long id = NFmiStringTools::Convert<unsigned long>(prodStrVec[i]);
					NFmiProducer prod(id, prodStrVec[i+1]);
					itsExtraSoundingProducerList.push_back(prod);
				}
			}
		}
		catch(std::exception &e)
		{
            LogAndWarnUser(e.what(), "Problems with extra sounding producers settings", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
		}
	}

	int DataToDBCheckMethod(void)
	{
		return itsDataToDBCheckMethod;
	}

	void DataToDBCheckMethod(int newValue)
	{
		itsDataToDBCheckMethod = newValue;
		NFmiSettings::Set(string("MetEditor::DataToDBCheckMethod"), NFmiStringTools::Convert<int>(itsDataToDBCheckMethod), true);
	}

    void InitDataToDBCheckMethodMain()
    {
        try
        {
            InitDataToDBCheckMethod();
        }
        catch(exception &e)
        {
            LogAndWarnUser(e.what(), "Problems with editor.conf settings", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
        }
    }

	void InitDataToDBCheckMethod(void)
	{
        DoVerboseFunctionStartingLogReporting(__FUNCTION__);
		itsDataToDBCheckMethod = NFmiSettings::Require<int>("MetEditor::DataToDBCheckMethod");
        // Option 0 was removed, when harmonization system was removed from SmartMet
        if(itsDataToDBCheckMethod == 0)
            itsDataToDBCheckMethod = 2;
	}

	// t‰m‰ asettaa uuden karttan‰ytˆn hilaruudukon koon.
	// tekee tarvittavat 'likaukset' ja palauttaa true, jos
	// n‰yttˆj‰ tarvitsee p‰ivitt‰‰, muuten false (eli ruudukko ei muuttunut).
	bool SetMapViewGrid(unsigned int theDescTopIndex, const NFmiPoint &newValue)
	{
		LogMessage("Map view grid changed.", CatLog::Severity::Info, CatLog::Category::Visualization);
        boost::shared_ptr<NFmiMapViewWinRegistry> mapViewWinRegistry = ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(theDescTopIndex);
        return MapViewDescTop(theDescTopIndex)->SetMapViewGrid(newValue, mapViewWinRegistry.get());
	}

	CtrlViewUtils::GraphicalInfo& GetGraphicalInfo(unsigned int theDescTopIndex)
	{
		if(theDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
			return CrossSectionSystem()->GetGraphicalInfo();
		else
			return MapViewDescTop(theDescTopIndex)->GetGraphicalInfo();
	}

    // T‰m‰ on ainoa Q2Serveriin liittyv‰ kysely, jonka pit‰‰ menn‰ genDataDocin kautta, johtuen IsOperationalModeOn kyselyst‰!!!
	bool UseQ2Server(void)
	{
		return itsQ2ServerInfo.UseQ2Server() && IsOperationalModeOn();
	}

	const NFmiPoint& StationDataGridSize(void) {return itsStationDataGridSize;}
	void StationDataGridSize(const NFmiPoint &newValue) {itsStationDataGridSize = newValue;}

	const NFmiVPlaceDescriptor& SoundingPlotLevels(void)
	{return itsSoundingPlotLevels;}

	void InitSoundingPlotLevels(void)
	{
        DoVerboseFunctionStartingLogReporting(__FUNCTION__);
		NFmiLevelBag levBag;
		levBag.AddLevel(NFmiLevel(kFmiSoundingLevel, "1000", 1000));
		levBag.AddLevel(NFmiLevel(kFmiSoundingLevel, "925", 925));
		levBag.AddLevel(NFmiLevel(kFmiSoundingLevel, "850", 850));
		levBag.AddLevel(NFmiLevel(kFmiSoundingLevel, "800", 800));
		levBag.AddLevel(NFmiLevel(kFmiSoundingLevel, "700", 700));
		levBag.AddLevel(NFmiLevel(kFmiSoundingLevel, "600", 600));
		levBag.AddLevel(NFmiLevel(kFmiSoundingLevel, "500", 500));
		levBag.AddLevel(NFmiLevel(kFmiSoundingLevel, "400", 400));
		levBag.AddLevel(NFmiLevel(kFmiSoundingLevel, "300", 300));
		levBag.AddLevel(NFmiLevel(kFmiSoundingLevel, "250", 250));
		levBag.AddLevel(NFmiLevel(kFmiSoundingLevel, "200", 200));
		levBag.AddLevel(NFmiLevel(kFmiSoundingLevel, "150", 150));
		levBag.AddLevel(NFmiLevel(kFmiSoundingLevel, "100", 100));
		levBag.AddLevel(NFmiLevel(kFmiSoundingLevel, "70", 70));
		levBag.AddLevel(NFmiLevel(kFmiSoundingLevel, "50", 50));
		levBag.AddLevel(NFmiLevel(kFmiSoundingLevel, "30", 30));
		levBag.AddLevel(NFmiLevel(kFmiSoundingLevel, "20", 20));
		itsSoundingPlotLevels = NFmiVPlaceDescriptor(levBag);
	}

	const NFmiPoint& OutOfEditedAreaTimeSerialPoint(void) const
	{
		return itsOutOfEditedAreaTimeSerialPoint;
	}
	void OutOfEditedAreaTimeSerialPoint(const NFmiPoint &newValue)
	{
		itsOutOfEditedAreaTimeSerialPoint = newValue;
	}
	void ResetOutOfEditedAreaTimeSerialPoint(void)
	{
		itsOutOfEditedAreaTimeSerialPoint = NFmiPoint(kFloatMissing, kFloatMissing);
	}

	bool NearestShipLocation(NFmiFastQueryInfo &theInfo, const NFmiLocation &theLocation)
	{
		double minDist = 999999999.;
		unsigned long minLocInd = static_cast<unsigned long>(-1);
		for(theInfo.ResetLocation(); theInfo.NextLocation(); )
		{
			NFmiPoint p(theInfo.GetLatlonFromData());
			if(p.X() == kFloatMissing || p.Y() == kFloatMissing)
				continue;
			double currentDist = theLocation.Distance(p);
			if(currentDist < minDist)
			{
				minDist = currentDist;
				minLocInd = theInfo.LocationIndex();
			}
		}
		return theInfo.LocationIndex(minLocInd);
	}

	boost::shared_ptr<NFmiFastQueryInfo> GetNearestSynopStationInfo(const NFmiLocation &theLocation, const NFmiMetTime &theTime, bool ignoreTime, checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > *thePossibleInfoVector, double maxDistanceInMeters = 1000. * kFloatMissing)
	{
		boost::shared_ptr<NFmiFastQueryInfo> info;
		checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector = (thePossibleInfoVector == 0) ? GetSortedSynopInfoVector(kFmiSYNOP, kFmiTestBed, kFmiSHIP, kFmiBUOY) : *thePossibleInfoVector;

		if(infoVector.size() > 0)
		{ // etsit‰‰n useasta infosta l‰hint‰ asemaa
			const double defaultDist = 999999999.;
			double minDist = defaultDist;
			int wantedInfoIndex = -1;
			int index = 0;
			unsigned long minLocationIndex = static_cast<unsigned long>(-1);
			for(auto &info : infoVector)
			{
				if(ignoreTime || info->Time(theTime))
				{
					FmiProducerName prod = static_cast<FmiProducerName>(info->Producer()->GetIdent());
                    auto doShipDataLocations = (info->IsGrid() == false && (info->HasLatlonInfoInData()));
					if(doShipDataLocations ? NearestShipLocation(*info, theLocation) : info->NearestLocation(theLocation))
					{
						double currentDistance = theLocation.Distance(doShipDataLocations ? info->GetLatlonFromData() : info->LatLonFast());
						if(currentDistance < minDist && currentDistance < maxDistanceInMeters)
						{
							minDist = currentDistance;
							wantedInfoIndex = index;
							minLocationIndex = info->LocationIndex();
						}
					}
				}
				index++;
			}
			if(wantedInfoIndex >=0)
			{
				info = infoVector[wantedInfoIndex];
			}
		}

		return info;
	}

	NFmiMapViewDescTop* MapViewDescTop(unsigned int theIndex)
	{
		if(theIndex >= itsMapViewDescTopList.size())
		{
			if(theIndex > CtrlViewUtils::kFmiMaxMapDescTopIndex)
			{
				return itsMapViewDescTopList[0]; // palautetaan erikois n‰yttˆj‰ varten p‰‰karttanaytˆn desctop
			}
			else
				throw std::runtime_error("GeneralDataDoc::MapViewDescTop - Given mapViewDescTop index was out of bounds, error in program.");
		}
		return itsMapViewDescTopList[theIndex];
	}

	std::vector<NFmiMapViewDescTop*>& MapViewDescTopList(void)
	{
		return itsMapViewDescTopList;
	}

	void OnShowGridPoints(unsigned int theDescTopIndex)
	{
		// t‰m‰ on oikeasti toggle funktio, eli n‰yt‰/piilota hila/asemapisteet
        bool newState = !ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(theDescTopIndex)->ShowStationPlot();
        ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(theDescTopIndex)->ShowStationPlot(newState);
        MapViewDescTop(theDescTopIndex)->ShowStationPlotVM(newState); // t‰m‰ pit‰‰ p‰ivitt‰‰ molempiin paikkoihin, koska jotkin operaatiot riippuvat ett‰ MapViewDescTop:issa on p‰ivitetty arvo
		MapViewDirty(theDescTopIndex, false, true, true, false, false, false); // t‰m‰ laittaa cachen likaiseksi
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Show/hide active parameters data's grid/station points");
	}

	void OnToggleGridPointColor(unsigned int theDescTopIndex)
	{
		MapViewDescTop(theDescTopIndex)->ToggleStationPointColor();
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Change active parameters data's grid/station points color");
	}

	void OnToggleGridPointSize(unsigned int theDescTopIndex)
	{
		MapViewDescTop(theDescTopIndex)->ToggleStationPointSize();
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Change active parameters data's grid/station points size");
	}

	void OnEditSpaceOut(unsigned int theDescTopIndex)
	{
        ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(theDescTopIndex)->ToggleSpacingOutFactor();
        // MacroParamDataCache ongelma (MAPADACA): Kun harvennusta muutetaan, pit‰‰ liata sellaiset macroParamit, jotka piirret‰‰n symboleilla
		MapViewDirty(theDescTopIndex, false, true, true, false, false, false);
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Toggle spacing out factor");
	}

	void OnHideParamWindow(unsigned int theDescTopIndex)
	{
		MapViewDescTop(theDescTopIndex)->ShowParamWindowView(!MapViewDescTop(theDescTopIndex)->ShowParamWindowView());
		MapViewDirty(theDescTopIndex, false, false, true, false, false, false); // laitetaan kartta likaiseksi
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Show/Hide param-view");
    }

	void OnShowTimeString(unsigned int theDescTopIndex)
	{
		ToggleShowTimeOnMapMode(theDescTopIndex);
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Toggle show time on map view mode");
	}

	void OnButtonRefresh(void)
	{
		LogMessage("Refreshing all views.", CatLog::Severity::Info, CatLog::Category::Visualization);
		MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, true, true, false, false, true); // laitetaan kartta likaiseksi
        MacroParamDataCache().clearAllLayers();
        WindTableSystem().MustaUpdateTable(true);
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Refreshing all views (F5)");
	}

	void OnChangeMapType(unsigned int theDescTopIndex, bool fForward)
	{
		ChangeMapType(theDescTopIndex, fForward);
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view's background map style changed");
	}

    void ChangeWmsOverlayMapType(unsigned int theDescTopIndex, bool fForward)
    {
#ifndef DISABLE_CPPRESTSDK
        if(fForward)
            WmsSupport().nextOverlay();
        else
            WmsSupport().previousOverlay();

        MapViewDirty(theDescTopIndex, true, true, true, false, false, false);
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view's Wms overlay map type changed");
#endif // DISABLE_CPPRESTSDK
    }

    void ChangeFileBitmapOverlayMapType(unsigned int theDescTopIndex, bool fForward)
    {
        if(MapViewDescTop(theDescTopIndex)->MapHandler())
        {
            if(fForward)
                MapViewDescTop(theDescTopIndex)->MapHandler()->NextOverMap();
            else
                MapViewDescTop(theDescTopIndex)->MapHandler()->PreviousOverMap();
            MapViewDirty(theDescTopIndex, true, true, true, false, false, false);
            ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view's overlay map style changed");
        }
    }

	void OnToggleShowNamesOnMap(unsigned int theDescTopIndex, bool fForward)
	{
        if(UseWmsMaps())
            ChangeWmsOverlayMapType(theDescTopIndex, fForward);
        else
            ChangeFileBitmapOverlayMapType(theDescTopIndex, fForward);
	}

    // T‰m‰ asettaa maskOnMap -asetuksen kahteen paikkaan, windows rekistereihin ja 
    // SmartMetin sis‰iseen k‰yttˆˆn.
    void SetShowMaskOnMap(unsigned int theDescTopIndex, bool newValue)
    {
        ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(static_cast<int>(theDescTopIndex))->ShowMasksOnMap(newValue);
    }

	// vaihtaaa "n‰yt‰ maskit kartalla" -tilaa ja p‰ivitt‰‰ ruudut
	void OnShowMasksOnMap(unsigned int theDescTopIndex)
	{
        bool newValue = !ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(static_cast<int>(theDescTopIndex))->ShowMasksOnMap();
        SetShowMaskOnMap(theDescTopIndex, newValue);
		MapViewDirty(theDescTopIndex, false, true, true, false, false, false);
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Show/hide masks on map views");
	}

	void OnShowProjectionLines(void)
	{
		NFmiProjectionCurvatureInfo* projInfo = ProjectionCurvatureInfo();
		if(projInfo)
		{
			NFmiProjectionCurvatureInfo::DrawingMode mode = projInfo->GetDrawingMode();
			mode = NFmiProjectionCurvatureInfo::DrawingMode(mode + NFmiProjectionCurvatureInfo::DrawingMode(1));
			if(mode > NFmiProjectionCurvatureInfo::kOverEverything)
				mode = NFmiProjectionCurvatureInfo::kNoDraw;
			projInfo->SetDrawingMode(mode);
			MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, true, true, false, false, false);
			LogMessage("Projektion line draw style changed.", CatLog::Severity::Debug, CatLog::Category::Visualization);
            ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view projektion line draw style changed");
		}
	}

	void OnToggleLandBorderDrawColor(unsigned int theDescTopIndex)
	{
		MapViewDescTop(theDescTopIndex)->ToggleLandBorderColor();
		MapViewDirty(theDescTopIndex, false, true, true, false, false, false);
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: Toggle land border draw color");
	}

	void OnToggleLandBorderPenSize(unsigned int theDescTopIndex)
	{
		MapViewDescTop(theDescTopIndex)->ToggleLandBorderPenSize();
		MapViewDirty(theDescTopIndex, false, true, true, false, false, false);
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: Toggle land border draw line width");
	}

	void UpdateMapView(unsigned int theDescTopIndex)
	{
        ApplicationInterface::GetApplicationInterfaceImplementation()->UpdateMapView(theDescTopIndex);
	}

	void OnAcceleratorBorrowParams(unsigned int theDescTopIndex, int row)
	{
		BorrowParams(theDescTopIndex, row);
		UpdateMapView(theDescTopIndex);
	}

	void OnAcceleratorMapRow(unsigned int theDescTopIndex, int theStartingRow)
	{
		MapViewDescTop(theDescTopIndex)->MapRowStartingIndex(theStartingRow);
        UpdateRowInLockedDescTops(theDescTopIndex);
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: changed starting absolute map row");
	}

	void OnToggleOverMapBackForeGround(unsigned int theDescTopIndex)
	{
		MapViewDescTop(theDescTopIndex)->DrawOverMapMode(MapViewDescTop(theDescTopIndex)->DrawOverMapMode() == 0 ? 1 : 0);
		MapViewDirty(theDescTopIndex, false, true, true, false, false, false);
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: changed map overlay's draw order");
	}

	void OnAcceleratorToggleKeepMapRatio(void)
	{
        auto newKeepAspectRatioState = !ApplicationWinRegistry().KeepMapAspectRatio();
        ApplicationWinRegistry().KeepMapAspectRatio(newKeepAspectRatioState);

		// keep ratio laskut pit‰‰ tehd‰ kaikille karttan‰ytˆille!!!
		for(size_t i = 0; i<itsMapViewDescTopList.size(); i++)
		{
			CRect rect;
			MapViewDescTop(static_cast<unsigned int>(i))->MapView()->GetClientRect(rect);
			MapViewDescTop(static_cast<unsigned int>(i))->CalcClientViewXperYRatio(NFmiPoint(rect.Width(), rect.Height()));
            // t‰m‰ 'aiheuttaa' datan harvennuksen. Jos newKeepAspectRatioState on true, tapahtuu silloin 
            // automaattinen kartan zoomaus ja macroParamCacheData pit‰‰ silloin tyhjent‰‰ kaikille karttan‰ytˆille
			MapViewDirty(static_cast<unsigned int>(i), true, true, true, newKeepAspectRatioState, false, false);
            ApplicationInterface::GetApplicationInterfaceImplementation()->UpdateMapView(static_cast<unsigned int>(i));
		}
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: toggle keep map's aspect ratio setting");
	}

	// Jos datan area ja kartta ovat "samanlaisia", laitetaan zoomiksi editoiavan datan alue
	// muuten laitetaan kurrentti kartta kokonaisuudessaan n‰kyviin.
	void OnButtonDataArea(unsigned int theDescTopIndex)
	{
		boost::shared_ptr<NFmiFastQueryInfo> info = EditedInfo();
		bool editedDataExist = info ? true : false;
		bool dataAreaExist = false;
		if(editedDataExist)
			dataAreaExist = info->Area() != 0;
		bool areasAreSameKind = false;
		if(dataAreaExist)
        {
            boost::shared_ptr<NFmiArea> infoArea(info->Area()->Clone());
			areasAreSameKind = NFmiQueryDataUtil::AreAreasSameKind(infoArea.get(), MapViewDescTop(theDescTopIndex)->MapHandler()->TotalArea().get());
        }
		if(areasAreSameKind)
		{
			static int counter = 0;
			counter++; // t‰m‰n avulla jos kartan alue ja datan alue samat, joka toisella kerralla zoomataan dataa, ja joka toisella kartan alueeseen
			NFmiRect intersectionRect(MapViewDescTop(theDescTopIndex)->MapHandler()->TotalArea()->XYArea().Intersection(MapViewDescTop(theDescTopIndex)->MapHandler()->TotalArea()->XYArea(info->Area())));
			boost::shared_ptr<NFmiArea> usedArea(MapViewDescTop(theDescTopIndex)->MapHandler()->TotalArea()->CreateNewArea(intersectionRect));
			LogMessage("Setting zoomed area the same as edited data.", CatLog::Severity::Info, CatLog::Category::Visualization);
			if(counter%2 == 0)
				SetMapArea(theDescTopIndex, MapViewDescTop(theDescTopIndex)->MapHandler()->TotalArea());
			else
				SetMapArea(theDescTopIndex, usedArea);
		}
		else
		{
			LogMessage("Setting zoomed are the same as map area.", CatLog::Severity::Info, CatLog::Category::Visualization);
			SetMapArea(theDescTopIndex, MapViewDescTop(theDescTopIndex)->MapHandler()->TotalArea());
		}
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: data area button pressed");
	}

	double DrawObjectScaleFactor(void)
	{
		return ApplicationWinRegistry().DrawObjectScaleFactor();
	}
	void DrawObjectScaleFactor(double newValue)
	{
        ApplicationWinRegistry().DrawObjectScaleFactor(newValue);

		// laitetaan s‰‰dˆn yhteydess‰ kaikki desctop graphicalinfot likaisiksi
		for(size_t i = 0; i < itsMapViewDescTopList.size(); i++)
			GetGraphicalInfo(static_cast<unsigned int>(i)).fInitialized = false;
	}

	NFmiConceptualModelData& ConceptualModelData(void)
	{
		return itsConceptualModelData;
	}

	NFmiQ2Client& Q2Client(void)
	{
		return itsQ2Client;
	}

	NFmiSeaIcingWarningSystem& SeaIcingWarningSystem(void)
	{
		return itsSeaIcingWarningSystem;
	}

	int SoundingViewWindBarbSpaceOutFactor(void)
	{
		return itsMTATempSystem.WindBarbSpaceOutFactor();
	}

	void SoundingViewWindBarbSpaceOutFactor(int newValue)
	{
		itsMTATempSystem.WindBarbSpaceOutFactor(newValue);
	}

	void TransparencyContourDrawView(CWnd *theView)
	{
		itsTransparencyContourDrawView = theView;
	}

	CWnd* TransparencyContourDrawView(void)
	{
		return itsTransparencyContourDrawView;
	}

	// T‰m‰ funktio takaa ett‰ haluttaessa synop-data tulevat k‰yttˆˆn aina samassa j‰rjestyksess‰.
	// Eli kun nyt synop-dataa on euro+maailma+10min+testbed+ship+poiju jne., ja niiden j‰rjestys annetussa
	// info-vektorissa riippuu v‰h‰n siit‰, milloin mikin datan on luettu viimeksi.
	// T‰st‰ seuraa ett‰ jos synop-datoja ei j‰rjestet‰, voi ei niin t‰rke‰ automaatti asema
	// peitt‰‰ synop-plotissa tarke‰n havaintoaseman.
	checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > GetSortedSynopInfoVector(int theProducerId, int theProducerId2 = -1, int theProducerId3 = -1, int theProducerId4 = -1)
	{
		checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector = InfoOrganizer()->GetInfos(theProducerId, theProducerId2, theProducerId3, theProducerId4);

		if(itsSynopDataFilePatternSortOrderVector.size())
		{ // jos on m‰‰r‰tty sorttaus j‰rjestys, tehd‰‰n sorttaus
			checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > copyOfInfoVector(infoVector); // t‰st‰ listasta otetaan sorttauksessa lˆydetyt infot pois,
																	// ett‰ lopuksi voidaan laittaa j‰ljelle j‰‰neet t‰st‰ listasta lopulliseen
																	// sortattuun listaan. Alkuper‰isest‰ listasta ei voi poistaa infoja kesken loopin.
			checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > sortedInfoVector;
			for(size_t i = 0; i < itsSynopDataFilePatternSortOrderVector.size(); i++)
			{
				for(size_t j = 0; j < infoVector.size(); j++)
				{
					NFmiFileString fileString(infoVector[j]->DataFilePattern());
					fileString.NormalizeDelimiter(); // hakemisto erottimet pit‰‰ saada oikein p‰in, ett‰ FileName-metodi toimisi
					std::string noPathFilePattern(fileString.FileName());
					if(itsSynopDataFilePatternSortOrderVector[i] == noPathFilePattern)
					{
						sortedInfoVector.push_back(infoVector[j]);
						copyOfInfoVector[j] = boost::shared_ptr<NFmiFastQueryInfo>(); // laitetaan 0-pointteri t‰h‰n merkiksi, ett‰ on otettu jo k‰yttˆˆn
						break;
					}
				}
			}
			for(size_t k = 0; k < copyOfInfoVector.size(); k++)
			{
				if(copyOfInfoVector[k] != 0)
					sortedInfoVector.push_back(copyOfInfoVector[k]); // t‰t‰ ei lˆytynyt sorttaus listasta, eli laitetaan se per‰‰n
			}

			return sortedInfoVector;
		}

		return infoVector;
	}

	void ToggleShowHelperDatasInTimeView(int theCommand)
	{
        TimeSerialViewDirty(true);
        switch(theCommand)
		{
		case kFmiShowHelperDataOnTimeSerialView:
            SetOnShowHelperData1InTimeSerialView();
			break;
		case kFmiDontShowHelperDataOnTimeSerialView:
            SetOffShowHelperData1InTimeSerialView();
			break;
		case kFmiShowHelperData2OnTimeSerialView:
            SetOnShowHelperData2InTimeSerialView();
			break;
		case kFmiDontShowHelperData2OnTimeSerialView:
            SetOffShowHelperData2InTimeSerialView();
			break;
        case kFmiShowHelperData3OnTimeSerialView:
            ShowHelperData3InTimeSerialView(true);
            break;
        case kFmiDontShowHelperData3OnTimeSerialView:
            ShowHelperData3InTimeSerialView(false);
            break;
        case kFmiShowHelperData4OnTimeSerialView:
            ShowHelperData4InTimeSerialView(true);
            break;
        case kFmiDontShowHelperData4OnTimeSerialView:
            ShowHelperData4InTimeSerialView(false);
            break;
        }
	}

	bool ShowToolTipTimeView(void)
	{
		return itsShowToolTipTimeView;
	}
	void ShowToolTipTimeView(bool newValue)
	{
		itsShowToolTipTimeView = newValue;
	}

	bool ShowToolTipTempView(void)
	{
		return itsShowToolTipTempView;
	}
	void ShowToolTipTempView(bool newValue)
	{
		itsShowToolTipTempView = newValue;
	}

	bool ShowToolTipTrajectoryView(void)
	{
		return itsShowToolTipTrajectoryView;
	}
	void ShowToolTipTrajectoryView(bool newValue)
	{
		itsShowToolTipTrajectoryView = newValue;
	}

	NFmiMapViewTimeLabelInfo& MapViewTimeLabelInfo()
	{
		return itsMapViewTimeLabelInfo;
	}

	void TimeSerialViewTimeBag(const NFmiTimeBag &theTimeBag)
	{
		itsTimeSerialViewTimeBag = theTimeBag;
	}

	const NFmiTimeBag& TimeSerialViewTimeBag(void) const
	{
		return itsTimeSerialViewTimeBag;
	}

	void CopyDrawParamsFromViewRow(const NFmiMenuItem& theMenuItem, int theRowIndex, bool fUseCrossSectionParams)
	{
		NFmiDrawParamList* drawParamList = 0;
		if(fUseCrossSectionParams)
			drawParamList = CrossSectionViewDrawParamList(theRowIndex);
		else
			drawParamList = DrawParamList(theMenuItem.MapViewDescTopIndex(), theRowIndex);
		if(drawParamList)
		{
			if(fUseCrossSectionParams)
			{
				fCopyPasteCrossSectionDrawParamListUsedYet = true;
				itsCopyPasteCrossSectionDrawParamList.CopyList(*drawParamList, true);
			}
			else
			{
				fCopyPasteDrawParamListUsedYet = true;
				itsCopyPasteDrawParamList.CopyList(*drawParamList, true);
			}
		}
	}

	void PasteDrawParamsToViewRow(const NFmiMenuItem& theMenuItem, int theRowIndex, bool fUseCrossSectionParams)
	{
		NFmiDrawParamList* drawParamList = 0;
		if(fUseCrossSectionParams)
			drawParamList = CrossSectionViewDrawParamList(theRowIndex);
		else
			drawParamList = DrawParamList(theMenuItem.MapViewDescTopIndex(), theRowIndex);
		if(drawParamList)
		{
			if(fUseCrossSectionParams)
				drawParamList->CopyList(itsCopyPasteCrossSectionDrawParamList, false);
			else
				drawParamList->CopyList(itsCopyPasteDrawParamList, false);
            MakeViewRowDirtyActions(theMenuItem.MapViewDescTopIndex(), GetRealRowNumber(theMenuItem.MapViewDescTopIndex(), theRowIndex), drawParamList);
		}
	}

	void CopyDrawParamsFromMapViewRow(unsigned int theDescTopIndex)
	{
		NFmiDrawParamList * activeDrawParamList = DrawParamList(theDescTopIndex, ActiveViewRow(theDescTopIndex));
		if(activeDrawParamList)
		{
			fCopyPasteDrawParamListUsedYet = true;
			itsCopyPasteDrawParamList.CopyList(*activeDrawParamList, true);
		}
	}

	void PasteDrawParamsToMapViewRow(unsigned int theDescTopIndex)
	{
		NFmiDrawParamList * activeDrawParamList = DrawParamList(theDescTopIndex, ActiveViewRow(theDescTopIndex));
		if(activeDrawParamList)
		{
			activeDrawParamList->CopyList(itsCopyPasteDrawParamList, false);
            MakeViewRowDirtyActions(theDescTopIndex, ActiveViewRow(theDescTopIndex), activeDrawParamList);
            ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: Paste drawParams to map view row");
		}
	}

	int ActiveViewRow(unsigned int theDescTopIndex)
	{
        if(theDescTopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex)
            return MapViewDescTop(theDescTopIndex)->ActiveViewRow();
        else
            return itsCurrentCrossSectionRowIndex;
	}

	void ActiveViewRow(unsigned int theDescTopIndex, int theActiveRowIndex)
	{
		MapViewDescTop(theDescTopIndex)->ActiveViewRow(theActiveRowIndex);
	}

    void SetLastActiveDescTopAndViewRow(unsigned int theDescTopIndex, int theActiveRowIndex)
    {
        ParameterSelectionSystem().LastAcivatedDescTopIndex(theDescTopIndex);
        ParameterSelectionSystem().LastActivatedRowIndex(GetRealRowNumber(theDescTopIndex, theActiveRowIndex));
    }

	void CopyDrawParamsList(NFmiPtrList<NFmiDrawParamList> &copyFromList, NFmiPtrList<NFmiDrawParamList> &copyToList)
	{
		copyToList.Clear(true); // tuhotaan ensin vanhan listan sis‰ltˆ
		NFmiPtrList<NFmiDrawParamList>::Iterator iter = copyFromList.Start();
		for( ; iter.Next(); )
		{
			NFmiDrawParamList *tmpList = new NFmiDrawParamList;
			tmpList->CopyList(iter.Current(), false);
			copyToList.AddEnd(tmpList);
		}
	}

	void CopyMapViewDescTopParams(unsigned int theDescTopIndex)
	{
		if(MapViewDescTop(theDescTopIndex))
		{
			NFmiPtrList<NFmiDrawParamList> *copiedDrawParamsList = MapViewDescTop(theDescTopIndex)->DrawParamListVector();
			if(copiedDrawParamsList)
			{
				fCopyPasteDrawParamListVectorUsedYet = true;
				CopyDrawParamsList(*copiedDrawParamsList, itsCopyPasteDrawParamListVector);
			}
		}
	}

	void PasteMapViewDescTopParams(unsigned int theDescTopIndex)
	{
		if(MapViewDescTop(theDescTopIndex))
		{
			NFmiPtrList<NFmiDrawParamList> *copiedDrawParamsList = MapViewDescTop(theDescTopIndex)->DrawParamListVector();
			if(copiedDrawParamsList)
			{
				CopyDrawParamsList(itsCopyPasteDrawParamListVector, *copiedDrawParamsList);
                MakeWholeDesctopDirtyActions(theDescTopIndex, copiedDrawParamsList);
                ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: Paste all copyed parameters on this map view");
			}
		}
	}

    void MakeWholeDesctopDirtyActions(unsigned int theDescTopIndex, NFmiPtrList<NFmiDrawParamList> *drawParamListVector)
    {
        MapViewDirty(theDescTopIndex, false, true, true, false, false, true);
        if(drawParamListVector)
        {
            MacroParamDataCache().update(theDescTopIndex, drawParamListVector);
        }
    }

    // Kun animaatio on p‰‰ll‰, aina ei haluta menn‰ animaation loppuun asti kokonaan.
    // Tapaus 1: Kun animaatiossa on havaintoja seuraava moodi p‰‰ll‰ ja on isompi ruudukko k‰ytˆss‰ (esim. 3x2 eli 3 aikaa ja 2 rivi‰), 
    // t‰llˆin on tarkoitus j‰tt‰‰ animaatio looppi vajaaksi, niin ett‰ animaatio loppuu silloin kun viimeisten havaintojen aika on juuri
    // ruudukon oikeassa reunassa. Muuten k‰visi niin ett‰ kun on 3 aikasaraketta, animaatio menisi niin pitk‰lle ett‰ havaintojen viimeinen aika
    // olisi vasemmassa sarakkeessa ja sen oikealla puolella olevissa sarakkeissa ei n‰kyisi havainto datoja.
    int CalcReducedAnimationSteps(NFmiAnimationData::AnimationLockMode theLockMode, CtrlViewUtils::MapViewMode theMapViewDisplayMode, int theViewGridSizeX)
    {
        if(theLockMode != NFmiAnimationData::kNoLock && theMapViewDisplayMode == CtrlViewUtils::MapViewMode::kNormal && theViewGridSizeX > 1)
            return theViewGridSizeX - 1;
        else
            return 0;
    }

	// Do all animations funktiota kutsutaan SmartMetin Application luokasta (CSmartMetApp) OnIdle-metodista.
	// Jos jossain on menossa animaatiota,  tarkista onko aika tehd‰ askelliuksia (animaatio viive riippuvuus).
	// Jos ei ole aniomaatioita k‰ynniss‰, palauta 0.
	// Jos joku animaatio on jossain k‰ynniss‰, palauta 1.
	int DoAllAnimations(void)
	{
		int status = 0; // status voi saada arvon 0 (ei animaatiota), 1 (animaatio p‰‰ll‰, mutta odota) ja 2 (animaatio p‰‰ll‰, p‰ivit‰ ruudut)
		int maxStatus = 0;
		bool mainViewMustBeUpdated = false; // t‰ss‰ yritet‰‰n optimoida p‰ivitys rutiinia. Jos p‰‰ ikkunaa pit‰‰ p‰ivitt‰‰,
										// pit‰‰ p‰ivitt‰‰ smartmetin kaikkia n‰yttˆj‰. Jos animaatiot pyˆriv‰t vain apukarttan‰ytˆiss‰
										// voidaan p‰ivitt‰‰ vain niit‰ ja se on kevyemp‰‰!!!
		bool helpView1Updated = false;
		bool helpView2Updated = false;
		double minWaitTimeInMS = NFmiAnimationData::kNoAnimationWaitTime;
		for(size_t i = 0; i<itsMapViewDescTopList.size(); i++)
		{
			NFmiMapViewDescTop &descTop = *itsMapViewDescTopList[i];
            NFmiAnimationData &animationData = descTop.AnimationDataRef();
			animationData.CurrentTime(descTop.CurrentTime()); // currentti aika pit‰‰ ottaa desctopista ja antaa animaattorille
            int reducedAnimationTimeSteps = CalcReducedAnimationSteps(animationData.LockMode(), descTop.MapViewDisplayMode(), static_cast<int>(descTop.ViewGridSize().X()));
            status = animationData.Animate(reducedAnimationTimeSteps);
			double waitTimeInMS = animationData.CalcWaitTimeInMSForNextFrame();
			if(waitTimeInMS < minWaitTimeInMS)
				minWaitTimeInMS = waitTimeInMS;
			if(status > maxStatus)
					maxStatus = status;
			if(i == 0 && status == 2)
				mainViewMustBeUpdated = true;
			if(i == 1 && status == 2)
				helpView1Updated = true;
			if(i == 2 && status == 2)
				helpView2Updated = true;
			if(status == 2)
			{
				descTop.CurrentTime(animationData.CurrentTime());
			}
		}

		if(maxStatus)
		{
			if(maxStatus == 2)
			{
				if(mainViewMustBeUpdated)
                    ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Animation related main map view update");
                else
                {
                    CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging("Animation related map view 2/3 update", nullptr);
                    ApplicationInterface::GetApplicationInterfaceImplementation()->UpdateOnlyExtraMapViews(helpView1Updated, helpView2Updated);
                }
			}
			else
			{
				// jos ei ollut v‰litˆnt‰ animaatio tarvetta, lasketaan t‰ss‰ pieni nukkumis aika, 
				// ett‰ SmartMet ei jauha OnIdle-looppia t‰ydell‰ hˆyryll‰ kokoaikaa. Aika on joko 
				// 30 ms tai minimi odostus aika jos se on alle 30 ms.

#ifdef min
#undef min // MSVC:n m‰‰rittelem‰ min-makro pit‰‰ disabloida
#endif
				int sleepTimeInMS = boost::math::iround(std::min(30., minWaitTimeInMS));
				if(sleepTimeInMS > 0)
					boost::this_thread::sleep(boost::posix_time::milliseconds(sleepTimeInMS));
			}
			return 1; // jos status on ollut 1 tai 2, palautetaan 1
		}
		else
			return 0;
	}

	// tarkasta CView-n‰yttˆluokissa, onko mahdollisesti animaatiota k‰ynniss‰. Jos on, ‰l‰ laita odota-cursoria n‰kyviin, koska se vilkuttaa ik‰v‰sti
	bool ShowWaitCursorWhileDrawingView(void)
	{
		for(size_t i = 0; i<itsMapViewDescTopList.size(); i++)
		{
			if(itsMapViewDescTopList[i]->AnimationDataRef().IsAnimationRunning() || itsMapViewDescTopList[i]->AnimationDataRef().ShowTimesOnTimeControl())
				return false;
		}
		return true;
	}

	// etsi timedescriptorista viimeisin aika joka sopii annettuun aika-steppiin ja joka on lis‰ksi annettujen aikarajojen alueella.
	// Jos ei lˆydy, palauttaa false.
	// fDemandExactCheck muuttuja vaatii ett‰ aikojen pit‰‰ olla juuri tarkalleen aika stepiss‰. Mutta jos kyse on
	// esim. salama datasta, haetaan se aika, joka sopii steppiin ja joka on myˆh‰isin aika.
	bool GetLatestValidTimeWithCorrectTimeStep(NFmiTimeDescriptor &theTimes, int theTimeStepInMinutes, bool fDemandExactCheck, const NFmiMetTime &theLimit1, const NFmiMetTime &theLimit2, NFmiMetTime &theFoundTime)
	{
		theTimes.Time(theTimes.LastTime()); // asetetaan viimeiseen aikaan
		do
		{
			NFmiMetTime aTime(theTimes.Time());
			aTime.SetTimeStep(theTimeStepInMinutes, true);
			if(fDemandExactCheck)
			{
				if(aTime == theTimes.Time()) // kun vaaditaan tarkaa checkki‰, eih‰n aika muuttunut, kun sit‰ rukattiin halutulla timestepill‰
				{
					if(aTime >= theLimit1 && aTime <= theLimit2) // onko aika annettujen rajojen sis‰ll‰
					{
						theFoundTime = aTime;
						return true;
					}
				}
			}
			else
			{
				if(theTimes.Time() < aTime)
					aTime.NextMetTime(); // jos aika oli pyˆristynyt taaksep‰in, laitetaan se askeleen verran eteenp‰in
				if(aTime >= theLimit1 && aTime <= theLimit2)
				{
					theFoundTime = aTime;
					return true;
				}
			}
			if(theTimes.Time() <= theLimit1)
				break; // ei tarvetta jatkaa en‰‰, koska loput ajoista ovat kaikki pienempi‰ kuin ala rajan aika
		} while(theTimes.Previous());
		return false;
	}


	bool GetLatestSatelImageTime(const NFmiDataIdent &theDataIdent, NFmiMetTime &theFoundTime)
	{
        NFmiMetTime latestTime = itsSatelliteImageCacheSystemPtr->GetLatestImageTime(theDataIdent);
        if(latestTime != NFmiMetTime::gMissingTime)
        {
            theFoundTime = latestTime;
            return true;
        }
        else
            return false;
	}

	static const int statObservationSeekTimeLimitInMinutes = 240;

	void MakeObsSeekTimeLimits(const NFmiMetTime &theCurrentTime, int theTimeStepInMinutes, NFmiMetTime &theLimit1Out, NFmiMetTime &theLimit2Out)
	{
		theLimit1Out = theCurrentTime;
		if(theTimeStepInMinutes <= statObservationSeekTimeLimitInMinutes)
			theLimit1Out.ChangeByMinutes(-statObservationSeekTimeLimitInMinutes);
		else
			theLimit1Out.ChangeByMinutes(-theTimeStepInMinutes);
		theLimit2Out = theCurrentTime;
		if(theCurrentTime > theLimit2Out)
			theLimit2Out.NextMetTime(); // jos limit2 oli pyˆristynyt taaksep‰in, laitetaan se askel eteenp‰in
	}

	bool FindLastObservation(unsigned long theDescTopIndex, int theTimeStepInMinutes, NFmiMetTime &theNewLastTime, bool ignoreSatelImages)
	{
		NFmiMetTime currentTime(theTimeStepInMinutes); // t‰m‰ on jonkinlainen rajapyykki, eli t‰m‰n yli kun menn‰‰n (ei pit‰isi menn‰), lopetetaan havaintojen etsiminene siihen
		// haetaan min ja maksimi aika limitit, jotka on n. nykyhetki ja 2h - nykyhetki
		NFmiMetTime timeLimit1;
		NFmiMetTime timeLimit2;
		MakeObsSeekTimeLimits(currentTime, theTimeStepInMinutes, timeLimit1, timeLimit2);

		NFmiMapViewDescTop &descTop = *(MapViewDescTop(theDescTopIndex));
		NFmiPtrList<NFmiDrawParamList> *drawParamListVector = descTop.DrawParamListVector();
		NFmiPtrList<NFmiDrawParamList>::Iterator iter = drawParamListVector->Start();
		bool newerTimeFound = false;
		int viewRowIndex = 1;
		for( ; iter.Next(); viewRowIndex++)
		{
			if(descTop.IsVisibleRow(viewRowIndex) == false)
				continue; // ei k‰yd‰ l‰pi piilossa olevia rivej‰
			NFmiDrawParamList *aList = iter.CurrentPtr();
			if(aList)
			{
				for(aList->Reset(); aList->Next(); )
				{
					boost::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
					if(drawParam->IsParamHidden() == false)
					{
                        if(CtrlViewFastInfoFunctions::IsObservationLockModeDataType(drawParam->DataType()))
						{
							// Ignooraa toistaiseksi tuottaja kFmiTEMP, koska niiden par haku tuottaa ajallisesti liian pitk‰lle menevi‰ datoja (viimeinen aika on tyhj‰‰).
                            if(!NFmiInfoOrganizer::IsTempData(drawParam->Param().GetProducer()->GetIdent(), true))
							{
								checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector;
								MakeDrawedInfoVectorForMapView(infoVector, drawParam, descTop.MapHandler()->Area());
								for(size_t i = 0; i < infoVector.size(); i++)
								{
									bool demandExactTimeChecking = drawParam->DataType() != NFmiInfoData::kFlashData; // t‰ss‰ vaiheessa salama data on sellainen jossa ei vaadita tarkkoja aika tarkasteluja
									boost::shared_ptr<NFmiFastQueryInfo> &info = infoVector[i];
									NFmiMetTime dataLastTime;
									NFmiTimeDescriptor timeDesc = info->TimeDescriptor();
									if(GetLatestValidTimeWithCorrectTimeStep(timeDesc, theTimeStepInMinutes, demandExactTimeChecking, timeLimit1, timeLimit2, dataLastTime))
									{
										if(newerTimeFound == false || dataLastTime > theNewLastTime)
										{
											newerTimeFound = true;
											theNewLastTime = dataLastTime;
											if(theNewLastTime >= currentTime)
												return true; // ei tarvitse en‰‰ jatkaa, koska aika joka lˆytyi on viimeisin mahdollinen
										}
									}
								}
							}
						}
						else if(drawParam->DataType() == NFmiInfoData::kSatelData && ignoreSatelImages == false)
						{ // tutki lˆytyykˆ satel-data hakemistosta uudempia datoja, kuin annettu theLastTime
							NFmiMetTime satelLastTime;
							if(GetLatestSatelImageTime(drawParam->Param(), satelLastTime))
							{
								if(newerTimeFound == false || satelLastTime > theNewLastTime)
								{
									newerTimeFound = true;
									theNewLastTime = satelLastTime;
									if(theNewLastTime >= currentTime)
										return true; // ei tarvitse en‰‰ jatkaa, koska aika joka lˆytyi on viimeisin mahdollinen
								}
							}
						}
					}
				}
			}
		}
		return newerTimeFound;
	}

	// Tutkii k‰ikki n‰kyv‰t havainto datat ja etsii sen ajan, mik‰ on aikaisin eri datojen viimeisist‰ ajoista.
	// T‰ll‰ on pyrkimys siihen ett‰ animaatio ei v‰lky, kun kaikilta datoilta periaattessa lˆytyy dataa myˆs
	// t‰ll‰ tavalla etsittyyn viimeiseen aikaan.
	bool FindEarliestLastObservation(unsigned long theDescTopIndex, int theTimeStepInMinutes, NFmiMetTime &theNewLastTime, bool ignoreSatelImages)
	{
		NFmiMetTime currentTime(theTimeStepInMinutes); // t‰m‰ on jonkinlainen rajapyykki, eli t‰m‰n yli kun menn‰‰n (ei pit‰isi menn‰), lopetetaan havaintojen etsiminene siihen
		// haetaan min ja maksimi aika limitit, jotka on n. nykyhetki ja 2h - nykyhetki
		NFmiMetTime timeLimit1;
		NFmiMetTime timeLimit2;
		MakeObsSeekTimeLimits(currentTime, theTimeStepInMinutes, timeLimit1, timeLimit2);

		NFmiMapViewDescTop &descTop = *(MapViewDescTop(theDescTopIndex));
		NFmiPtrList<NFmiDrawParamList> *drawParamListVector = descTop.DrawParamListVector();
		NFmiPtrList<NFmiDrawParamList>::Iterator iter = drawParamListVector->Start();
		bool anyTimeFound = false;
		NFmiMetTime earliestLastTime(2200, 1, 1); // t‰h‰n vain iso tulevaisuuden luku
		int viewRowIndex = 1;
		for( ; iter.Next(); viewRowIndex++)
		{
			if(descTop.IsVisibleRow(viewRowIndex) == false)
				continue; // ei k‰yd‰ l‰pi piilossa olevia rivej‰
			NFmiDrawParamList *aList = iter.CurrentPtr();
			if(aList)
			{
				for(aList->Reset(); aList->Next(); )
				{
					boost::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
					if(drawParam->IsParamHidden() == false)
					{
                        if(CtrlViewFastInfoFunctions::IsObservationLockModeDataType(drawParam->DataType()))
						{
							// Ignooraa toistaiseksi tuottaja kFmiTEMP, koska niiden par haku tuottaa ajallisesti liian pitk‰lle menevi‰ datoja (viimeinen aika on tyhj‰‰).
                            if(!NFmiInfoOrganizer::IsTempData(drawParam->Param().GetProducer()->GetIdent(), true))
							{
								checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector;
								MakeDrawedInfoVectorForMapView(infoVector, drawParam, descTop.MapHandler()->Area());
								NFmiMetTime lastTimeOfThisDataType; // mm. synop datan tapauksessa haetaan ehk‰ jopa 6:sta datat tiedostosta viimeisint‰ aikaa
								bool lastTimeOfThisDataTypeFoundYet = false;
								for(size_t i = 0; i < infoVector.size(); i++)
								{
                                    if(drawParam->DataType() != NFmiInfoData::kFlashData) // Salamadatoja ei oteta "Latest mutual time" tarkasteluihin
                                    {
                                        bool demandExactTimeChecking = true;
                                        boost::shared_ptr<NFmiFastQueryInfo> &info = infoVector[i];
                                        NFmiMetTime dataLastTime;
                                        NFmiTimeDescriptor timeDesc = info->TimeDescriptor();
                                        if(GetLatestValidTimeWithCorrectTimeStep(timeDesc, theTimeStepInMinutes, demandExactTimeChecking, timeLimit1, timeLimit2, dataLastTime))
                                        {
                                            if(lastTimeOfThisDataTypeFoundYet == false || dataLastTime > lastTimeOfThisDataType)
                                            {
                                                lastTimeOfThisDataTypeFoundYet = true;
                                                lastTimeOfThisDataType = dataLastTime;
                                            }
                                        }
                                    }
								}
								// t‰ss‰ drawParam kohtaisten datojen aika tarkastelut
								if(lastTimeOfThisDataTypeFoundYet && (anyTimeFound == false || lastTimeOfThisDataType < earliestLastTime))
								{
									anyTimeFound = true;
									earliestLastTime = lastTimeOfThisDataType;
								}
							}
						}
						else if(drawParam->DataType() == NFmiInfoData::kSatelData && ignoreSatelImages == false)
						{ // tutki lˆytyykˆ satel-data hakemistosta uudempia datoja, kuin annettu theLastTime
							NFmiMetTime satelLastTime;
							if(GetLatestSatelImageTime(drawParam->Param(), satelLastTime))
							{
								if(anyTimeFound == false || satelLastTime < earliestLastTime)
								{
									anyTimeFound = true;
									earliestLastTime = satelLastTime;
								}
							}
						}
					}
				}
			}
		}
		if(anyTimeFound)
		{
			theNewLastTime = earliestLastTime;
			return true;
		}
		else
			return false;
	}

	// Liataan descTopin view-cachesta ne ajat uudesta timebagist‰, mit‰ ei ole vanhassa timebagissa
	void MakeNewTimesDirtyFromViewCache(NFmiMapViewDescTop &theMapViewDescTop, NFmiTimeBag &theOldAnimTimes, NFmiTimeBag &theNewAnimTimes)
	{
		for(theNewAnimTimes.Reset(); theNewAnimTimes.Next(); )
		{
			const NFmiMetTime &aTime = theNewAnimTimes.CurrentTime();
			if(theOldAnimTimes.SetCurrent(aTime) == false)
				theMapViewDescTop.MapViewCache().MakeTimeDirty(aTime);
		}
	}

	bool IsAnimationTimebagCheckNeeded(void)
	{
		for(size_t i = 0; i<itsMapViewDescTopList.size(); i++)
		{
			if(itsMapViewDescTopList[i]->AnimationDataRef().ShowTimesOnTimeControl())
				return true;
		}
		return false;
	}

	// T‰t‰ kutsutaan mm. CFmiMainFrame:n OnTimer:ista kerran minuutissa.
	// Tarkistaa eri n‰yttˆjen animaation tilan ja moodit.
	// P‰ivitt‰‰ tarvittaessa lukittujen moodien animaatio timebagit.
	// 'Likaa' tarvittaessa uudet ajat cache:sta ja tekee ruudun p‰ivitykset.
	// theDescTopIndex:ill‰ voidaan antaa jos halutaan tarkistaa vain tietyn n‰ytˆn p‰ivitys tarve.
	// Jos theDescTopIndex:in arvo on kDoAllMapViewDescTopIndex, silloin tarkistus tehd‰‰n kaikille n‰ytˆille.
	void CheckAnimationLockedModeTimeBags(unsigned int theDescTopIndex, bool ignoreSatelImages)
	{
		if(IsAnimationTimebagCheckNeeded())
		{ // edellinen metodi tarkisti, onko jossain animaatio boksi n‰kyviss‰.
            bool needToUpdateViews = false;
			// tutkitaan eri n‰yttˆjen animaattoreita ja niiden tiloja
			for(size_t i = 0; i<itsMapViewDescTopList.size(); i++)
			{
				if(theDescTopIndex == CtrlViewUtils::kDoAllMapViewDescTopIndex || i == theDescTopIndex)
				{
                    NFmiAnimationData &animData = itsMapViewDescTopList[i]->AnimationDataRef();
                    NFmiAnimationData::AnimationLockMode lockMode = animData.LockMode();
					if(lockMode == NFmiAnimationData::kFollowLastObservation || lockMode == NFmiAnimationData::kFollowEarliestLastObservation)
					{
						NFmiMetTime lastTime = animData.Times().LastTime();
						NFmiMetTime currentTime(1); // otetaan viel‰ sein‰kelloaika
						if(currentTime < lastTime || currentTime.DifferenceInMinutes(lastTime) > statObservationSeekTimeLimitInMinutes)
						{ // pit‰‰ fiksata viimeist‰ aikaa, koska se ei oikeastaan voi olla tulevaisuudessa, tai joku havainto on tulevaisuudesta
							// Lis‰ksi pit‰‰ fiksata jos alkuaika oli liian kaukana menneisyydess‰, turha k‰yd‰ l‰pi esim. viikon verran dataa ja etsi viimeist‰
							lastTime = currentTime;
							lastTime.SetTimeStep(static_cast<short>(animData.TimeStepInMinutes()));
							lastTime.ChangeByHours(-2); // l‰hdet‰‰n etsim‰‰ viimeist‰ havaintoa 3 tuntia menneisyydest‰
						}
						NFmiMetTime newLastTime(animData.TimeStepInMinutes()); // t‰h‰n sijoitetaan se aika, josta lˆytyi viimeinen n‰ytˆss‰ olevan parametrin havainto data
						bool foundObservations = false;
						if(lockMode == NFmiAnimationData::kFollowLastObservation)
							foundObservations = FindLastObservation(static_cast<unsigned long>(i), animData.TimeStepInMinutes(), newLastTime, ignoreSatelImages);
						else
							foundObservations = FindEarliestLastObservation(static_cast<unsigned long>(i), animData.TimeStepInMinutes(), newLastTime, ignoreSatelImages);

						if(foundObservations)
						{ // lˆytyi jotain ennusteita, jossa uusi lastTime (=newLastTime), joten voidaan p‰ivitt‰‰ animaation timebagi.
							int timeDiffInMinutes = animData.Times().LastTime().DifferenceInMinutes(animData.Times().FirstTime());
							NFmiMetTime newFirstTime(newLastTime);
							newFirstTime.ChangeByMinutes(-timeDiffInMinutes);
							NFmiTimeBag newAnimTimes(newFirstTime, newLastTime, animData.TimeStepInMinutes());
							NFmiTimeBag oldAnimTimes = animData.Times();
							animData.Times(newAnimTimes);
							// Laitetaan myˆs aikakontrolli-ikkuna seuraavaan animaatio ikkunaa, eli keskitet‰‰n aikaikkuna animaatioaikojen keskikohtaan
							int animDiffInMinutes = newAnimTimes.LastTime().DifferenceInMinutes(newAnimTimes.FirstTime());
							NFmiMetTime animMiddleTime(newAnimTimes.FirstTime());
							animMiddleTime.ChangeByMinutes(boost::math::lround(animDiffInMinutes/2.));
							animMiddleTime.SetTimeStep(animData.TimeStepInMinutes());
							CenterTimeControlView(static_cast<unsigned long>(i), animMiddleTime, false);
							// Lis‰ksi likaa n‰yttˆ cachesta uudet lˆytyneet ajat
							MakeNewTimesDirtyFromViewCache(*(itsMapViewDescTopList[i]), oldAnimTimes, newAnimTimes);
							MapViewDirty(static_cast<unsigned long>(i), false, false, true, false, false, false); // t‰m‰n pit‰isi asettaa n‰yttˆ p‰ivitys tilaan, mutta cachea ei tarvitse en‰‰ erikseen tyhjent‰‰
						}
					}
				}
			}

            if(needToUpdateViews)
                ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Animation related update because locked time mode event occured"); // viel‰ p‰ivitet‰‰n n‰ytˆt
        }
	}

	bool DoAutoLoadDataAtStartUp(void) const
	{
        return  itsApplicationWinRegistry.ConfigurationRelatedWinRegistry().LoadDataAtStartUp();
	}
	void DoAutoLoadDataAtStartUp(bool newValue)
	{
		itsApplicationWinRegistry.ConfigurationRelatedWinRegistry().LoadDataAtStartUp(newValue);
	}

	void ToggleTimeControlAnimationView(unsigned int theDescTopIndex)
	{
		NFmiMapViewDescTop *descTop = MapViewDescTop(theDescTopIndex);
		if(descTop)
		{
			descTop->AnimationDataRef().ShowTimesOnTimeControl(!(descTop->AnimationDataRef().ShowTimesOnTimeControl()));
			MapViewDirty(theDescTopIndex, false, false, true, false, false, false); // t‰m‰n pit‰isi asettaa n‰yttˆ p‰ivitys tilaan, mutta cachea ei tarvitse en‰‰ erikseen tyhjent‰‰
            ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: Show/hide animation times on map view's time control view");
		}
	}

	void SetInfosMask(checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector, unsigned long theMask)
	{
		for(size_t i = 0; i < theInfoVector.size(); i++)
		{
			if(theInfoVector[i])
				theInfoVector[i]->MaskType(theMask);
		}
	}

	void MakeDrawedInfoVectorForMapView(checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector, boost::shared_ptr<NFmiDrawParam> &theDrawParam, const boost::shared_ptr<NFmiArea> &theArea)
	{
		theInfoVector.clear();
		NFmiInfoData::Type dataType = theDrawParam->DataType();
		bool notEditedData = ((dataType != NFmiInfoData::kEditable) && (dataType != NFmiInfoData::kCopyOfEdited));
        if(notEditedData && theDrawParam->DataType() == NFmiInfoData::kFlashData)
        {
            theInfoVector = InfoOrganizer()->GetInfos(NFmiInfoData::kFlashData);
        }
        else if(notEditedData && (theDrawParam->Param().GetProducer()->GetIdent() == kFmiSYNOP || theDrawParam->Param().GetProducer()->GetIdent() == NFmiInfoData::kFmiSpSynoXProducer || theDrawParam->Param().GetParamIdent() == NFmiInfoData::kFmiSpSynoPlot || theDrawParam->Param().GetParamIdent() == NFmiInfoData::kFmiSpMinMaxPlot))
		{ // synop-data on aluksi poikkeus, mille tehd‰‰n vektori, miss‰ useita infoja, jos niit‰ lˆytyy
            if(theDrawParam->Param().GetProducer()->GetIdent() == NFmiInfoData::kFmiSpSynoXProducer)
    			theInfoVector = GetSortedSynopInfoVector(kFmiSYNOP, kFmiTestBed);
            else
                theInfoVector = GetSortedSynopInfoVector(kFmiSYNOP, kFmiTestBed, kFmiSHIP, kFmiBUOY);
            for(auto fastInfo : theInfoVector)
                fastInfo->Param(static_cast<FmiParameterName>(theDrawParam->Param().GetParamIdent())); // Laitetaan kaikki synop-datat osoittamaan haluttua parametria
		}
		else
		{
			boost::shared_ptr<NFmiFastQueryInfo> info = InfoOrganizer()->Info(theDrawParam, false, false);
			if(info)
			{
				if(dataType == NFmiInfoData::kMacroParam || dataType == NFmiInfoData::kQ3MacroParam)
				{ // makroparamille pit‰‰ s‰‰t‰‰ laskettavan hilan alue vastaamaan karttan‰ytˆn aluetta
                    NFmiExtraMacroParamData::SetUsedAreaForData(info, theArea.get());
				}

				theInfoVector.push_back(info);
			}
		}
		// Lopuksi pit‰‰ viel‰ s‰‰t‰‰ piirrett‰vien infojen maskit knomask-tilaan, koska 
		// olen poistanut ns. aktivationMaskin k‰ytˆn ja info on voinut j‰‰d‰ esim. selected-mask tilaan
		SetInfosMask(theInfoVector, NFmiMetEditorTypes::kFmiNoMask);
	}

    void CheckForNewConceptualModelData(void)
	{
        bool needsToUpdateViews = false;
        unsigned int ssize = static_cast<unsigned int>(itsMapViewDescTopList.size());
		for(unsigned int i = 0; i<ssize; i++)
            needsToUpdateViews |= CheckForNewConceptualModelDataBruteForce(i);

        if(needsToUpdateViews)
            ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: Brute force conceptual model data update required");
	}

    bool CheckForNewConceptualModelDataBruteForce(unsigned int theDescTopIndex)
	{
		// T‰m‰ on ns. brute force ratkaisu ei-queryData pohjaisten datojen p‰ivitys tarpeelle.
		// Jos n‰ytˆn rivill‰ on jotain t‰ll‰ist‰ dataa, 'liataan' aina koko rivi varmuuden vuoksi.
		// HUOM! jos jossain karttan‰ytˆss‰ on animaattori p‰‰ll‰, ei tehd‰ likausta, koska animaatioidulle datalle oli omia tarkastuksia.

		// 1. Tarkita onko karttan‰ytˆn n‰yttˆriveill‰ k‰siteanalyysi datoja
		// 2. Jos on, liataan kyseinen rivi
		NFmiMapViewDescTop *descTop = MapViewDescTop(theDescTopIndex);
		if(descTop->AnimationDataRef().AnimationOn())
			return false;

        bool needsToUpdateViews = false;

		int cacheRowNumber = 0; // cache row indeksi alkaa 0:sta!!
		NFmiPtrList<NFmiDrawParamList> *drawParamListVector = descTop->DrawParamListVector();
		NFmiPtrList<NFmiDrawParamList>::Iterator iter = drawParamListVector->Start();
		for(; iter.Next();)
		{
			NFmiDrawParamList *aList = iter.CurrentPtr();
			if(aList)
			{
				for(aList->Reset(); aList->Next(); )
				{
					boost::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
                    if(!drawParam->IsParamHidden())
                    {
                        NFmiInfoData::Type dataType = drawParam->DataType();
                        if(dataType == NFmiInfoData::kConceptualModelData || dataType == NFmiInfoData::kCapData)
                        {
                            MapViewDirty(theDescTopIndex, false, false, true, false, false, false); // liataan mapView, mutta ei viel‰ t‰ll‰ rivill‰ cachea (menisi kaikki rivit kerralla)
                            descTop->MapViewCache().MakeRowDirty(cacheRowNumber);// clean cache row
                            needsToUpdateViews = true;
                            if(dataType == NFmiInfoData::kCapData)
                            {
                                capDataSystem.refreshCapData();
                            }
                        }
                    }
				}
			}
			cacheRowNumber++;
		}
        return needsToUpdateViews;
	}

	const NFmiPoint& TimeSerialViewSizeInPixels(void) const
	{
		return itsTimeSerialViewSizeInPixels;
	}
	void TimeSerialViewSizeInPixels(const NFmiPoint &newValue)
	{
		itsTimeSerialViewSizeInPixels = newValue;
	}

	// Funktio s‰‰t‰‰ halutun aikakontrolli ikkunan siten ett‰ se keskitt‰‰ annetun ajan n‰kyviin.
	// Eli annettu aika menee aikakontrolli ikkunan keskelle. Halutessa myˆs p‰ivitet‰‰n
	// valittu aika siihen.
	void CenterTimeControlView(unsigned int theDescTopIndex, const NFmiMetTime &theTime, bool fUpdateSelectedTime)
	{
		NFmiTimeBag times = *(MapViewDescTop(theDescTopIndex)->TimeControlViewTimes().ValidTimeBag());
		int timeStepInMinutes = boost::math::iround(MapViewDescTop(theDescTopIndex)->TimeControlTimeStep() * 60);
		long timeLengthInMinutes = times.LastTime().DifferenceInMinutes(times.FirstTime());
		long neededChangeInMinutes = times.FirstTime().DifferenceInMinutes(theTime) + timeLengthInMinutes/2;
		times.MoveByMinutes(-neededChangeInMinutes);
		NFmiTimeBag fixedTimeBag = CtrlViewUtils::GetAdjustedTimeBag(times.FirstTime(), times.LastTime(), timeStepInMinutes);
		NFmiTimeDescriptor newTimeDesc(MapViewDescTop(theDescTopIndex)->TimeControlViewTimes().OriginTime(), fixedTimeBag);
		MapViewDescTop(theDescTopIndex)->TimeControlViewTimes(newTimeDesc);

		if(fUpdateSelectedTime)
		{
			NFmiMetTime newTime(theTime);
			newTime.SetTimeStep(static_cast<short>(timeStepInMinutes), true);
			CurrentTime(theDescTopIndex, newTime);
		}
		MapViewDirty(theDescTopIndex, false, false, true, false, false, false); // t‰m‰n pit‰isi asettaa n‰yttˆ p‰ivitys tilaan, mutta cachea ei tarvitse en‰‰ erikseen tyhjent‰‰
	}

	NFmiWindTableSystem& WindTableSystem(void)
	{
		return itsWindTableSystem;
	}

    NFmiPoint GetPrintedMapAreaOnScreenSizeInPixels(unsigned int theDescTopIndex)
	{
		NFmiMapViewDescTop &descTop = *(MapViewDescTop(theDescTopIndex));
		NFmiPoint oldSize = descTop.MapViewSizeInPixels();
		int usedDestWidth = static_cast<int>(oldSize.X());
		int usedDestHeight = static_cast<int>(oldSize.Y());
		NFmiRect oldRelativeMapRect = descTop.RelativeMapRect();
		if(oldRelativeMapRect.Height() < 1.0)
		{ // pit‰‰ laskea uusi printti alue ilman k‰ytˆss‰ olevaa aikakortrolli-ikkuna osiota 
			usedDestHeight = boost::math::iround(usedDestHeight * oldRelativeMapRect.Height());
		}
		return NFmiPoint(usedDestWidth, usedDestHeight);
	}

	bool DeveloperModePath(void) 
	{
		return itsBasicConfigurations.DeveloperModePath();
	}
	void DeveloperModePath(bool newValue) 
	{
		itsBasicConfigurations.DeveloperModePath(newValue);
	}

	NFmiDataNotificationSettingsWinRegistry& DataNotificationSettings(void)
	{
        return ApplicationWinRegistry().DataNotificationSettingsWinRegistry();
	}

	bool UseTimeSerialAxisAutoAdjust(void)
	{
        return itsApplicationWinRegistry.UseTimeSerialAxisAutoAdjust();
	}
	void UseTimeSerialAxisAutoAdjust(bool newValue)
	{
		itsApplicationWinRegistry.UseTimeSerialAxisAutoAdjust(newValue);
	}

	// On olemassa seuraavat asetukset (loki hakemisto, editoinnin tyˆhakemisto ja tietyt cleaner asetukset):
	// Lis‰ksi uutena on combineData cache-polku (dataPath4), joka on m‰‰ritelty data_conf\datapaths_*.conf -tiedostoissa
	// ---------------------------------
	// MetEditor::FileCleanerSystem::DirectoryInfoPath_2 = d:\smartmet\wrk\data\omat
	// MetEditor::FileCleanerSystem::PatternInfo_1 = c:\weto\wrk\data\in\*_havy_suomi_synop_10min.sqd
	// MetEditor::DataLoadingInfo::WorkingPath = d:/smartmet/wrk/data/omat/
	// SmartMet::Logger::LogFilePath = d:\smartmet\log
	// dataPath4 = d:\smartmet\wrk\data\cache
	// ---------------------------------
	// Joita pit‰‰ voida s‰‰t‰‰ helposti, ettei tarvitse tehd‰ omia konfiguraatioita, kun
	// SmartMet asennetaankin C-asemalle D-aseman sijasta. N‰m‰ ovat ainoat joita tarvitsee t‰llˆin s‰‰t‰‰.
	void ChangeDriveLetterInSettings(const std::string &theNewLetter)
	{
		std::string warningStr = "Are you sure you want to change the drive letter?\n\nThis will change drive letter of couple of SmartMet settings paths.\nThis includes logger's log-file path and working files path.\nChanges will be made when you press the button and confirm the change.\nDon't press Yes-button if you don't know what you are doing!";
		if(::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(warningStr.c_str()), _TEXT("Confirm the drive letter change"), MB_YESNO|MB_ICONWARNING) == IDYES)
		{
			std::string newLetter(theNewLetter);
			NFmiStringTools::UpperCase(newLetter);
			// 1. katso mik‰ oli vanha drive-letter loki hakemistossa
			NFmiFileString logPathStr(itsBasicConfigurations.LogFileDirectory());
			std::string oldLetter1 = logPathStr.Device();
			NFmiStringTools::UpperCase(oldLetter1);
			// 1.1 mik‰ oli tyˆhakemiston vanha drive-letter
			NFmiFileString workingFilePathStr = itsDataLoadingInfoNormal.WorkingPath();
			std::string oldLetter2 = workingFilePathStr.Device();
			NFmiStringTools::UpperCase(oldLetter2);
			// 1.2 mik‰ oli combined-data cachen vanha drive-letter
			NFmiFileString cacheFilePathStr = NFmiSettings::Optional<std::string>("dataPath4", "");
			std::string oldLetter3 = cacheFilePathStr.Device();
			NFmiStringTools::UpperCase(oldLetter3);

			// 1.3 mik‰ oli qdata cache-hakemiston vanha drive-letter
			NFmiFileString qdataCacheFilePathStr = HelpDataInfoSystem()->CacheDirectory();
			std::string oldLetter4 = qdataCacheFilePathStr.Device();
			NFmiStringTools::UpperCase(oldLetter4);

			// 1.4 mik‰ oli local makro cache-hakemiston vanha drive-letter
			NFmiFileString localMacroCacheFilePathStr = MacroPathSettings().LocalCacheBasePath();
			std::string oldLetter5 = localMacroCacheFilePathStr.Device();
			NFmiStringTools::UpperCase(oldLetter5);

			// 2. tarvitseeko vaihtaa kirjainta?
			if(newLetter != oldLetter1 || newLetter != oldLetter2 || newLetter != oldLetter3 || newLetter != oldLetter4 || newLetter != oldLetter5)
			{
				// 3. vaihda loggerin asema kirjain
				logPathStr.NormalizeDelimiter();
				logPathStr.Device(newLetter);
				ChangeLoggerPath(static_cast<char*>(logPathStr));

				// 4. vaihda tyˆhakemiston asema kirjain
				workingFilePathStr.NormalizeDelimiter();
				workingFilePathStr.Device(newLetter);
				WarnUserAboutNotExistingPath(static_cast<char*>(workingFilePathStr));
				NFmiSettings::Set("MetEditor::DataLoadingInfo::WorkingPath", static_cast<char*>(workingFilePathStr), true); // pit‰‰ myˆs tallettaa asetuksiin pysyv‰sti

				// 5. Lis‰‰ uusi working path fileCleanereiden listaan
				AddWorkingDataPathToCleaner();

				// 6. vaihda combine data cache polun asema kirjain
				cacheFilePathStr.NormalizeDelimiter();
				cacheFilePathStr.Device(newLetter);
				NFmiSettings::Set("dataPath4", static_cast<char*>(cacheFilePathStr), true);

				// 7. vaihda querydata cache polun asema kirjain
				qdataCacheFilePathStr.NormalizeDelimiter();
				qdataCacheFilePathStr.Device(newLetter);
				ChangeQuyeryDataCachePaths(newLetter);

				// 7.1 vaihda local makro cache polun asema kirjain
				localMacroCacheFilePathStr.NormalizeDelimiter();
				localMacroCacheFilePathStr.Device(newLetter);
				MacroPathSettings().LocalCacheBasePath(static_cast<char*>(localMacroCacheFilePathStr));
				MacroPathSettings().StoreToSettings();

				// 8. vaihda myˆs asetus-tiedostojen polut, jos tarvis
				ChangeSettingsPathDriveLetters(newLetter, oldLetter1);
			}
		}
	}

	// HUOM! t‰t‰ saa vain kutsua, jos perus-drive-letteri‰ on muutettu (theNewLetter != theOldLetter1)
	void ChangeSettingsPathDriveLetters(const std::string &theNewLetter, const std::string &theOldLetter)
	{
		if(itsMacroPathSettings.UseLocalCache())
		{ // s‰‰det‰‰n lokaali polkuja ja niiden johdannaisia
			NFmiFileString pathStr = itsMacroPathSettings.LocalCacheBasePath();
			std::string pathDrive = pathStr.Device();
			NFmiStringTools::UpperCase(pathDrive);
			if(pathDrive == theOldLetter)
			{ // jos vanha drive-letter oli sama kuin mit‰ on lokaali makro cachella k‰ytˆss‰, pit‰‰ se muuttaa uuteen asemaan
				pathStr.Device(theNewLetter);
				itsMacroPathSettings.LocalCacheBasePath(pathStr.CharPtr());
				// Asetetaan sitten eri makro-systeemit k‰ytt‰m‰‰n uusia lokaali polkuja
				InfoOrganizer()->SetDrawParamPath(itsMacroPathSettings.DrawParamPath(true));
				itsViewMacroPath = itsMacroPathSettings.ViewMacroPath(true);
				MacroParamSystem().RootPath(itsMacroPathSettings.MacroParamPath(true));
				SmartToolInfo()->LoadDirectory(itsMacroPathSettings.SmartToolPath(true), true);
			}
		}
		else
		{ // Ei ole lokaali makro-hakemistoja k‰ytˆss‰, tehd‰‰n s‰‰dˆt vanhalla tavalla
			// 1. tarkista ja vaihda (jos tarpeen) drawParam-polku
			NFmiFileString drawParamPath = InfoOrganizer()->GetDrawParamPath();
			std::string drawParamPathDrive = drawParamPath.Device();
			NFmiStringTools::UpperCase(drawParamPathDrive);
			if(drawParamPathDrive == theOldLetter)
			{ // jos vanha drive-letter oli sama kuin mit‰ on drawParameilla k‰ytˆss‰, pit‰‰ se muuttaa uuteen asemaan
				drawParamPath.Device(theNewLetter);
				ChangeDrawParamPath(drawParamPath.CharPtr());
			}

			// 2. tarkista ja vaihda (jos tarpeen) viewMacro-polku
			NFmiFileString viewMacroPath = itsViewMacroPath;
			std::string viewMacroPathDrive = viewMacroPath.Device();
			NFmiStringTools::UpperCase(viewMacroPathDrive);
			if(viewMacroPathDrive == theOldLetter)
			{ // jos vanha drive-letter oli sama kuin mit‰ on viewMacroille k‰ytˆss‰, pit‰‰ se muuttaa uuteen asemaan
				viewMacroPath.Device(theNewLetter);
				ChangeViewMacroPath(viewMacroPath.CharPtr());
			}

			// 3. tarkista ja vaihda (jos tarpeen) macroParam-polku
			NFmiFileString macroParamPath = MacroParamSystem().RootPath();
			std::string macroParamPathDrive = macroParamPath.Device();
			NFmiStringTools::UpperCase(macroParamPathDrive);
			if(macroParamPathDrive == theOldLetter)
			{ // jos vanha drive-letter oli sama kuin mit‰ on macroParameilla k‰ytˆss‰, pit‰‰ se muuttaa uuteen asemaan
				macroParamPath.Device(theNewLetter);
				ChangeMacroParamPath(macroParamPath.CharPtr());
			}

			// 4. tarkista ja vaihda (jos tarpeen) smartTool-polku
			NFmiFileString smartToolPath = SmartToolInfo()->LoadDirectory();
			std::string smartToolPathDrive = smartToolPath.Device();
			NFmiStringTools::UpperCase(smartToolPathDrive);
			if(smartToolPathDrive == theOldLetter)
			{ // jos vanha drive-letter oli sama kuin mit‰ on smartTooleilla k‰ytˆss‰, pit‰‰ se muuttaa uuteen asemaan
				smartToolPath.Device(theNewLetter);
				ChangeSmartToolPath(smartToolPath.CharPtr());
			}
		}
	}

	void ChangeQuyeryDataCachePaths(std::string newDriveLetter)
	{
		// 1. muutetaan cache-hakemiston asema
		NFmiFileString cacheFilePathStr = HelpDataInfoSystem()->CacheDirectory();
		cacheFilePathStr.NormalizeDelimiter();
		cacheFilePathStr.Device(newDriveLetter);
		HelpDataInfoSystem()->CacheDirectory(static_cast<char*>(cacheFilePathStr));
		// 2. muutetaan cache-tmp -hakemiston asema
		NFmiFileString cacheTmpFilePathStr = HelpDataInfoSystem()->CacheTmpDirectory();
		cacheTmpFilePathStr.NormalizeDelimiter();
		cacheTmpFilePathStr.Device(newDriveLetter);
		HelpDataInfoSystem()->CacheTmpDirectory(static_cast<char*>(cacheTmpFilePathStr));
		// 3. muutetaan partial-data cache-hakemiston asema
		NFmiFileString cachePartialDataFilePathStr = HelpDataInfoSystem()->CachePartialDataDirectory();
		cachePartialDataFilePathStr.NormalizeDelimiter();
		cachePartialDataFilePathStr.Device(newDriveLetter);
		HelpDataInfoSystem()->CachePartialDataDirectory(static_cast<char*>(cachePartialDataFilePathStr));

		HelpDataInfoSystem()->StoreToSettings();
		CFmiQueryDataCacheLoaderThread::UpdateSettings(*HelpDataInfoSystem());
        ApplicationInterface::GetApplicationInterfaceImplementation()->RestartHistoryDataCacheThread();
		 // kun cache asetukset (cache-drive -letter) muuttuvat, pit‰‰ yhdistelm‰ data asetuksia p‰vitt‰‰
		CFmiCombineDataThread::InitCombineDataInfos(*HelpDataInfoSystem());
	}

	NFmiMetTime GetNewerOriginTimeFromInfos(checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfos, const NFmiMetTime &theCurrentLatestTime)
	{
		NFmiMetTime latestTime = theCurrentLatestTime;
		if(theInfos.size() > 0)
		{
			for(size_t i=0; i < theInfos.size(); i++)
			{
				if(latestTime == NFmiMetTime::gMissingTime)
					latestTime = theInfos[i]->OriginTime();
				else if(latestTime < theInfos[i]->OriginTime())
					latestTime = theInfos[i]->OriginTime();
			}
		}
		return latestTime;
	}

	NFmiMetTime GetLatestOriginTimeFromModel(boost::shared_ptr<NFmiDrawParam> &theDrawParam)
	{
		NFmiMetTime latestTime = NFmiMetTime::gMissingTime;
		NFmiInfoData::Type dataType = theDrawParam->DataType();
		if(dataType == NFmiInfoData::kViewable || dataType == NFmiInfoData::kHybridData || dataType == NFmiInfoData::kModelHelpData) // jos kyseess‰ tavallinen mallidata, tarkastellaan kolmea eri dataa ja etsit‰‰n niist‰ uusin origin aika
		{
			unsigned long prodId = theDrawParam->Param().GetProducer()->GetIdent();
			// haetaan halutun mallin eri datoista (pinta, painepinta ja mallipinta) viimeisin origin aika
			checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > groundInfos = InfoOrganizer()->GetInfos(NFmiInfoData::kViewable, true, prodId);
			latestTime = GetNewerOriginTimeFromInfos(groundInfos, latestTime);
			checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > pressureInfos = InfoOrganizer()->GetInfos(NFmiInfoData::kViewable, false, prodId);
			latestTime = GetNewerOriginTimeFromInfos(pressureInfos, latestTime);
			checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > hybridInfos = InfoOrganizer()->GetInfos(NFmiInfoData::kHybridData, false, prodId);
			latestTime = GetNewerOriginTimeFromInfos(hybridInfos, latestTime);
		}
		else
		{
			boost::shared_ptr<NFmiDrawParam> tmpDrawParam(new NFmiDrawParam(*theDrawParam));
			tmpDrawParam->ModelRunIndex(0); // haetaan viimeisin data
			boost::shared_ptr<NFmiFastQueryInfo> aInfo = InfoOrganizer()->Info(tmpDrawParam, false, false);
			if(aInfo)
				return aInfo->OriginTime();
		}
		return latestTime;
	}

	// palauttaa annettuun aikaan l‰himm‰n sopivan edellisen malliajan. Eli jos annettu aika on 2009.12.07 klo 05 UTC, olisi l‰hin edellinen HIR RCR aika 00 UTC samalta p‰iv‰lt‰
	bool SetNearestBeforeModelOrigTimeRunoff(boost::shared_ptr<NFmiDrawParam> &theDrawParam ,const NFmiMetTime &theTime, unsigned int theDescTopIndex, int theViewRowIndex)
	{
		// laske se origin-aika, mik‰ tulee suhteellisesta aikasiirrosta
		int modelRunTimeGapInMinutes = GetModelRunTimeGap(theDrawParam);
		// 3.1 Katso mik‰ on halutun mallin viimeisin aika (pinta, painepinta ja mallipinta datoista viimeinen)
		NFmiMetTime latestTime = GetLatestOriginTimeFromModel(theDrawParam);
		// 3.2 jos ei aikaa, tee nyky ajan hetkest‰ aika pyˆristettyn‰ l‰himp‰‰n edelliseen mahdolliseen malliaikaan
		if(latestTime != NFmiMetTime::gMissingTime)
		{
			if(modelRunTimeGapInMinutes < 0)
			{
				theDrawParam->ModelRunIndex(InfoOrganizer()->GetNearestUnRegularTimeIndex(theDrawParam, theTime));
			}
			else if(modelRunTimeGapInMinutes > 0)
			{
				long diffInMinutes = theTime.DifferenceInMinutes(latestTime);
				// pit‰‰ hakea l‰hin edellinen indeksi eli floorilla menn‰‰n
				int runoffIndex = static_cast<int>(::floor(diffInMinutes / static_cast<float>(modelRunTimeGapInMinutes)));
				theDrawParam->ModelOriginTime(NFmiMetTime::gMissingTime);
				if(runoffIndex > 0)
					runoffIndex = 0;
				theDrawParam->ModelRunIndex(runoffIndex);
			}
			else if(modelRunTimeGapInMinutes == 0)
				return false;
			MapViewDescTop(theDescTopIndex)->MapViewCache().MakeRowDirty(GetRealRowNumber(theDescTopIndex, theViewRowIndex)-1);
			MapViewDirty(theDescTopIndex, false, false, true, false, false, false);
			return true;
		}
		return false;
	}

	void SetModelRunOffset(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theMoveByValue, unsigned int theDescTopIndex, int theViewRowIndex)
	{
		int oldModelRunIndex = theDrawParam->ModelRunIndex();
		theDrawParam->ModelOriginTime(NFmiMetTime::gMissingTime); // nollataan mahd. fiksattu origin aika
		theDrawParam->ModelRunIndex(theDrawParam->ModelRunIndex() + theMoveByValue); // siirret‰‰n offset edelliseen aikaan
		if(theDrawParam->ModelRunIndex() > 0)
			theDrawParam->ModelRunIndex(0);
		if(oldModelRunIndex != theDrawParam->ModelRunIndex())
		{
			MapViewDescTop(theDescTopIndex)->MapViewCache().MakeRowDirty(GetRealRowNumber(theDescTopIndex, theViewRowIndex)-1);
			MapViewDirty(theDescTopIndex, false, false, true, false, false, false);
		}
	}


	NFmiMetTime GetModelOrigTime(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theIndex = 0)
	{
		if(theDrawParam->UseArchiveModelData() || theIndex < 0)
		{
		// 2. Jos on suora absoluutti origin aika, palauta se
			if(theDrawParam->ModelOriginTime() != NFmiMetTime::gMissingTime)
				return theDrawParam->ModelOriginTime();
		// 3. Jos suhteellinen origin aika
			else
			{ // laske se origin-aika, mik‰ tulee suhteellisesta aikasiirrosta
				long timeBetweenRunsInMinutes = GetModelRunTimeGap(theDrawParam);
		// 3.1 Katso mik‰ on halutun mallin viimeisin aika (pinta, painepinta ja mallipinta datoista viimeinen)
				NFmiMetTime latestTime = GetLatestOriginTimeFromModel(theDrawParam);
		// 3.2 jos ei aikaa, tee nyky ajan hetkest‰ aika pyˆristettyn‰ l‰himp‰‰n edelliseen mahdolliseen malliaikaan
				if(latestTime == NFmiMetTime::gMissingTime && timeBetweenRunsInMinutes > 0)
				{
					NFmiMetTime aTime;
					NFmiMetTime aTime2(aTime);
					aTime2.SetTimeStep(static_cast<short>(timeBetweenRunsInMinutes));
					if(aTime < aTime2)
						aTime2.PreviousMetTime();
					latestTime = aTime2;
				}
		// 3.3 siirr‰ saatu aika kyseisen mallin ajon v‰lin (hirlamissa 6 h, EC 12 h jne.) verran taaksep‰in kerrottuna suhteelisella indeksill‰ drawParamista
				if(theIndex < 0)
					latestTime.ChangeByMinutes(timeBetweenRunsInMinutes * theIndex);
				else
					latestTime.ChangeByMinutes(timeBetweenRunsInMinutes * theDrawParam->ModelRunIndex());
				theDrawParam->ModelOriginTimeCalculated(latestTime);
				return latestTime;
			}
		}
		return NFmiMetTime::gMissingTime;
	}

	// Palauttaa annetussa drawParamissa viitatun mallidatan originTime stringin (muotoa YYYYMMDDHHmmSS).
	std::string GetModelOrigTimeString(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theIndex = 0)
	{
		std::string str;
		NFmiMetTime origTime = GetModelOrigTime(theDrawParam, theIndex);
		if(origTime != NFmiMetTime::gMissingTime)
		{
			str += "origintime=";
			str += origTime.ToStr(kYYYYMMDDHHMMSS);
		}
		return str;
	}

	NFmiAutoComplete& AutoComplete(void)
	{
		return itsAutoComplete;
	}

	void SetMapViewCacheSize(double theNewSizeInMB)
	{
		MetEditorOptionsData().ViewCacheMaxSizeInMB(theNewSizeInMB);

		for(size_t i=0; i<itsMapViewDescTopList.size(); i++)
			itsMapViewDescTopList[i]->MapViewCache().MaxSizeMB(theNewSizeInMB);
	}

	const NFmiColor& GeneralColor(int theIndex)
	{
		static NFmiColor dummy(0.f,0.f,0.f);  // musta on dummy v‰ri
		if(theIndex >= 0 && theIndex < static_cast<int>(itsGeneralColors.size()))
			return itsGeneralColors[theIndex];
		else
			return dummy;
	}

	void InitializeGeneralColors(void)
	{
        DoVerboseFunctionStartingLogReporting(__FUNCTION__);
		try
		{
			int colorCount = NFmiSettings::Require<int>("SmartMet::General::ColorCount");
			std::string colorText("SmartMet::General::Color");
			for(int i=0; i<colorCount; i++)
			{
				// v‰rin luku vaatii hieman kikkailua stringstreamin kanssa
				std::string settingStr(colorText + NFmiStringTools::Convert<int>(i+1)); // colorText-stringiin pit‰‰ saada j‰rjestys numero per‰‰n
				itsGeneralColors.push_back(SettingsFunctions::GetColorFromSettings(settingStr));
			}
		}
		catch(std::exception &e)
		{
			LogAndWarnUser(e.what(), "Problems in InitializeGeneralColors.", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
		}
	}

	// Jos jonkin mallidatan leveldatasta lasketaan automaattisesti soundingIndex data,
    // t‰‰ll‰ luetaan datan talletushakemiston peruspolku.
    // Legacy syist‰ polkua yritet‰‰n lukea myˆs vanhasta dataPath4 -asetuksesta.
    std::string AutoGeneratedSoundingIndexBasePath(void)
    {
        std::string cacheDir = NFmiSettings::Optional<std::string>("soundingIndexBasePath", "");
        if(cacheDir.empty())
            cacheDir = NFmiSettings::Optional<std::string>("dataPath4", "");

        return cacheDir;
    }

	// theTypeOfChange 0 = minutes, 1 = 6hrs, 2 = day, 3 = week, 4 = month, 5 = year
	// theDirection vaihtoehdot ovat kBackward ja kForward eli ajan siirto suunta
	// theViewType 1 = mapview, 2 = timeserialview, 3 = crosssectionview, 4 = soundingview, 5 = trajectoryview, 6 = synoptableview
	// theMapViewIndex el ijos saman tyyppisi‰ n‰yttˆj‰ monia, t‰m‰ on halutun n‰ytˆn indeksi (p‰tee nyt vain karttan‰yttˆihin, joita on 3 kpl)
	// theAmountOfChange eli kuinka monta p‰iv‰‰/viikkoa jne. liikutaan ajassa eteen/taakse (yleens‰ 1)
	bool ChangeTime(int theTypeOfChange, FmiDirection theDirection, int theViewType, unsigned long theMapViewIndex, double theAmountOfChange)
	{
		// TODO ajan s‰‰tˆ koodia t‰‰ll‰, pit‰‰ korjata kutsuvat metodit myˆs
		if(theViewType == 1)
		{
			NFmiMetTime selectedTime = CurrentTime(theMapViewIndex);
            if(theTypeOfChange == 0) // minutes
            {
                selectedTime.ChangeByMinutes(boost::math::lround(theDirection == kForward ? theAmountOfChange : -theAmountOfChange));
            }
            else if(theTypeOfChange == 1) // 6 hrs
			{
				theAmountOfChange = 6;
                selectedTime.ChangeByHours(boost::math::lround(theDirection == kForward ? theAmountOfChange : -theAmountOfChange));
			}
			else if(theTypeOfChange == 2) // day
                selectedTime.ChangeByDays(boost::math::lround(theDirection == kForward ? theAmountOfChange : -theAmountOfChange));
			else if(theTypeOfChange == 3) // week
                selectedTime.ChangeByDays(boost::math::lround(theDirection == kForward ? 7 * theAmountOfChange : -7 * theAmountOfChange));
			else if(theTypeOfChange == 4) // month
			{
				NFmiTimePerioid period(0,1,0,0,0,0);
				if(theDirection == kForward)
					selectedTime.NextMetTime(period);  // HUOM! n‰iss‰ bugi, ei osaa ottaa huomioon eri pituisia kuukausia!!!!!
				else
					selectedTime.PreviousMetTime(period);  // HUOM! n‰iss‰ bugi, ei osaa ottaa huomioon eri pituisia kuukausia!!!!!
			}
/*
			else if(theTypeOfChange == 4) // year
			{
				NFmiTimePerioid period(1,0,0,0,0,0);
				if(theDirection == kForward)
					selectedTime.NextMetTime(period);  // HUOM! n‰iss‰ bugi, ei osaa ottaa huomioon eri pituisia kuukausia!!!!!
				else
					selectedTime.PreviousMetTime(period);  // HUOM! n‰iss‰ bugi, ei osaa ottaa huomioon eri pituisia kuukausia!!!!!
			}
*/
			CenterTimeControlView(theMapViewIndex, selectedTime, true); // t‰m‰ asettaa ajan, myˆs lukittuihin n‰yttˆihin ja likaa n‰ytˆn
            ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: Times changed by PageUp/Down keys");
			return true;
		}

		return false;
	}

	NFmiModelDataBlender& ModelDataBlender(void)
	{
		return itsModelDataBlender;
	}

	NFmiParamBag& AllStaticParams(void)
	{
		static NFmiParamBag staticParams;

		staticParams = NFmiParamBag(); // tyhjennet‰‰n static-parBagi ensin
		boost::shared_ptr<NFmiFastQueryInfo> statInfo = itsSmartInfoOrganizer->FindInfo(NFmiInfoData::kStationary);
		if(statInfo)
			staticParams = statInfo->ParamBag();
		return staticParams;
	}

	NFmiAnalyzeToolData& AnalyzeToolData(void)
	{
		return itsAnalyzeToolData;
	}

	NFmiDataQualityChecker& DataQualityChecker(void)
	{
		return itsDataQualityChecker;
	}

	void DrawOverBitmapThings(NFmiToolBox * theGTB)
	{
        ApplicationInterface::GetApplicationInterfaceImplementation()->DrawOverBitmapThings(theGTB);
	}

	void UpdateTempView(void)
	{
        ApplicationInterface::GetApplicationInterfaceImplementation()->UpdateTempView();
	}

	void UpdateCrossSectionView(void)
	{
        ApplicationInterface::GetApplicationInterfaceImplementation()->UpdateCrossSectionView();
	}

	void ActivateViewParamSelectorDlg(int /* theMapViewDescTopIndex */)
	{
        ApplicationInterface::GetApplicationInterfaceImplementation()->ActivateParameterSelectionDlg();
	}

	std::string GetToolTipString(unsigned int commandID, std::string &theMagickWord)
	{
        return ApplicationInterface::GetApplicationInterfaceImplementation()->GetToolTipString(commandID, theMagickWord);
	}

	void ActivateZoomDialog(int theWantedDescTopIndex)
	{
        ApplicationInterface::GetApplicationInterfaceImplementation()->ActivateZoomDialog(theWantedDescTopIndex);
	}

	void ForceDrawOverBitmapThings(unsigned int originalCallerDescTopIndex, bool doOriginalView, bool doAllOtherMapViews)
	{
        ApplicationInterface::GetApplicationInterfaceImplementation()->ForceDrawOverBitmapThings(originalCallerDescTopIndex, doOriginalView, doAllOtherMapViews);
	}

	void InvalidateMapView(bool bErase)
	{
        ApplicationInterface::GetApplicationInterfaceImplementation()->InvalidateMainMapView(bErase);
	}

	void SetMacroErrorText(const std::string &theErrorStr)
	{
        ApplicationInterface::GetApplicationInterfaceImplementation()->SetMacroErrorText(theErrorStr);
	}

	int MachineThreadCount(void)
	{
		return itsMachineThreadCount;
	}

	void InitMachineThreadCount(void)
	{
        DoVerboseFunctionStartingLogReporting(__FUNCTION__);
		itsMachineThreadCount = static_cast<int>(boost::thread::hardware_concurrency());

		string infoStr("This workstation has support for ");
		infoStr += NFmiStringTools::Convert(itsMachineThreadCount);
		infoStr += " thread(s) from hardware (CPU x core (x 2 if hyper-threading))";
		LogMessage(infoStr, CatLog::Severity::Info, CatLog::Category::Operational);
	}

	int SatelDataRefreshTimerInMinutes(void)
	{
		return itsSatelDataRefreshTimerInMinutes;
	}
	void SatelDataRefreshTimerInMinutes(int newValue)
	{
		itsSatelDataRefreshTimerInMinutes = newValue;
	}

	NFmiIgnoreStationsData& IgnoreStationsData(void)
	{
			return itsIgnoreStationsData;
	}

	bool EditedDataNeedsToBeLoaded(void)
	{
		return fEditedDataNeedsToBeLoaded;
	}
	void EditedDataNeedsToBeLoaded(bool newValue)
	{
		fEditedDataNeedsToBeLoaded = newValue;
		if(fEditedDataNeedsToBeLoaded)
			EditedDataNeedsToBeLoadedTimer().StartTimer();
		else
			EditedDataNeedsToBeLoadedTimer().StopTimer();
	}
	NFmiMilliSecondTimer& EditedDataNeedsToBeLoadedTimer(void) 
	{
		return itsEditedDataNeedsToBeLoadedTimer;
	}

	bool EditedDataNotInPreferredState(void)
	{
		return fEditedDataNotInPreferredState;
	}

	void EditedDataNotInPreferredState(bool newValue)
	{
		fEditedDataNotInPreferredState = newValue;
	}

	bool CheckEditedDataAfterAutoLoad(void)
	{
		EditedDataNotInPreferredState(true); // laitetaan lippu p‰‰lle ett‰ data ei ole halutunlaista
		// 1. onko editoitua dataa
		boost::shared_ptr<NFmiFastQueryInfo> editedInfo = EditedInfo();
		if(editedInfo)
		{
			NFmiMetTime aTime(1);
			NFmiMetTime firstTime = editedInfo->TimeDescriptor().FirstTime();
			// 2. poikkeaako alkuaika liikaa sein‰kello ajasta
			if(aTime.DifferenceInMinutes(firstTime) <= 4 * 60) // esim. 4 tuntia voisi olla hyv‰ksytt‰v‰ raja
			{
				// 3. Onko hilarakenne oikein
				if(itsPossibleUsedDataLoadingGrid == 0 || editedInfo->HPlaceDescriptor() == *itsPossibleUsedDataLoadingGrid)
				{
					// 4. Onko aikarakenne halutun mukainen
					// TODO
					// 5. Onko parametrit halutun mukaiset
					if(!fUseEditedDataParamDescriptor || itsEditedDataParamDescriptor == editedInfo->ParamDescriptor())
					{
						// 6. eih‰n tietyilt‰ parametreilta puutu liikaa dataa...
						// TODO
						EditedDataNotInPreferredState(false); // laitetaan lippu pois p‰‰lt‰ eli data on halutunlaista
						return true;
					}
				}
			}
		}
		return false;
	}

	NFmiApplicationDataBase& ApplicationDataBase(void)
	{
		return itsBasicConfigurations.ApplicationDataBase();
	}

	void OnButtonRedo(void) 
	{
		boost::shared_ptr<NFmiFastQueryInfo> info = EditedInfo();
		NFmiSmartInfo *smartInfo = dynamic_cast<NFmiSmartInfo*>(info.get());
		if(smartInfo && smartInfo->Redo())
		{
			if(RedoData())
			{
				MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, true, true, false, true, false);
				RefreshApplicationViewsAndDialogs("Edited data modifications Redo", false, false, -1);
			}
		}
	}

	void OnButtonUndo(void)
	{
		boost::shared_ptr<NFmiFastQueryInfo> info = EditedInfo();
		NFmiSmartInfo *smartInfo = dynamic_cast<NFmiSmartInfo*>(info.get());
		if(smartInfo && smartInfo->Undo())
		{
			if(UndoData())
			{
                MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, true, true, false, true, false);
				RefreshApplicationViewsAndDialogs("Edited data modifications Undo", false, false, -1);
			}
		}
	}

	int CleanUnusedDataFromMemory(void)
	{
		int datasRemovedFrommemory = InfoOrganizer()->CleanUnusedDataFromMemory();
		if(datasRemovedFrommemory)
		{
			std::string msgStr("Memory cleaning rutine: Archive datas removed from memory: ");
			msgStr += NFmiStringTools::Convert(datasRemovedFrommemory);
			LogMessage(msgStr, CatLog::Severity::Debug, CatLog::Category::Operational);
		}
		return datasRemovedFrommemory;
	}

	NFmiMacroPathSettings& MacroPathSettings(void)
	{
		return itsMacroPathSettings;
	}

	NFmiCaseStudySystem& CaseStudySystem(void)
	{
		return itsCaseStudySystem;
	}

	NFmiCaseStudySystem& LoadedCaseStudySystem(void)
	{
		return itsLoadedCaseStudySystem;
	}

	NFmiAviationStationInfoSystem& WmoStationInfoSystem(void)
	{
		if(fWmoStationInfoSystemInitialized == false)
			InitWmoStationInfoSystem(); // lazy-init, koska t‰t‰ dataa k‰ytet‰‰n ainakin toistaiseksi vain yhdess‰ paikassa ja initialisointi kest‰‰ useita sekunteja.
		return itsWmoStationInfoSystem;
	}

	NFmiHelpDataInfoSystem* HelpDataInfoSystem(void)
	{
		if(fCaseStudyModeOn)
			return itsCaseStudyHelpDataInfoSystem.get();
		else
			return &itsHelpDataInfoSystem;
	}

	NFmiDataLoadingInfo& GetUsedDataLoadingInfo(void)
	{
		if(fCaseStudyModeOn)
			return itsDataLoadingInfoCaseStudy;
		else
			return itsDataLoadingInfoNormal;
	}

    void SetAllSystemsToCaseStudyModeChangeTime(const NFmiMetTime &theFromWallClockTime, const NFmiMetTime &theToWallClockTime, bool changeToNormalMode)
	{
        if(changeToNormalMode)
            NFmiDataStoringHelpers::SetUsedViewMacroTime(NFmiMetTime::gMissingTime);
        else
            NFmiDataStoringHelpers::SetUsedViewMacroTime(theToWallClockTime);

        itsTimeSerialViewTimeBag.SetNewStartTime(theToWallClockTime);

        for(size_t i = 0; i < itsMapViewDescTopList.size(); i++)
        {
            itsMapViewDescTopList[i]->AnimationDataRef().AdjustTimesRelatedtoCaseStudyModeChange(theFromWallClockTime, theToWallClockTime);
            itsMapViewDescTopList[i]->SetCaseStudyTimes(theToWallClockTime);
        }
        CrossSectionSystem()->SetCaseStudyTimes(theToWallClockTime);
        GetMTATempSystem().SetCaseStudyTimes(theToWallClockTime);
        TrajectorySystem()->SetCaseStudyTimes(theToWallClockTime);

        itsDataLoadingInfoNormal.CaseStudyTime(theToWallClockTime);
        itsDataLoadingInfoNormal.UpdatedTimeDescriptor();
    }

    void InitializeSatelImageCacheForCaseStudy()
    {
        // P‰ivitys threadit pit‰‰ laittaa tauolle
        if(NFmiSatelliteImageCacheSystem::WaitUpdateThreadsToTakeABreak(5000))
            SatelliteImageCacheSystem().Init(*HelpDataInfoSystem()); // Jos tauko onnistui annetussa ajassa, SatelliteImageCacheTotal pit‰‰ alustaa k‰ytt‰m‰‰n CaseStudyn datoja
        // Lopuksi sanotaan working threadeille ett‰ tauko ohi
        NFmiSatelliteImageCacheSystem::TellUpdateThreadsToStopTheBreak();
    }

	// Funktio lataa annetun metadata-tiedoston ja laittaa SmartMetin CaseStudy-moodiin (ladaten datat jne.)
	bool LoadCaseStudyData(const std::string &theCaseStudyMetaFile)
	{
		std::string specificError;
		// 0. ota talteen erilaisia muuttujia, jos CaseStudyn lataus ep‰onnistuu ja pit‰‰ palauttaa olemassa oleva tila takaisin
		bool oldCaseStudyModeOn = fCaseStudyModeOn;
		boost::shared_ptr<NFmiHelpDataInfoSystem> oldCaseStudyHelpDataInfoSystem = itsCaseStudyHelpDataInfoSystem;
		NFmiCaseStudySystem oldLoadedCaseStudySystem = itsLoadedCaseStudySystem;
		try
		{
			// 1. Ota metadatasta polku talteen
			std::string caseStudyBasePath = NFmiFileSystem::PathFromPattern(theCaseStudyMetaFile);
			// 2. Lataa metadata
			if(itsLoadedCaseStudySystem.ReadMetaData(theCaseStudyMetaFile, ApplicationInterface::GetSmartMetViewAsCView()))
			{
			// 3. Luo metadatan avulla HelpDataInfoSystem (HUOM! t‰ss‰ pit‰‰ k‰ytt‰‰ suoraan itsHelpDataInfoSystem-dataosiota, kun silt‰ pyydet‰‰n static datoja)
				itsCaseStudyHelpDataInfoSystem = itsLoadedCaseStudySystem.MakeHelpDataInfoSystem(itsHelpDataInfoSystem, caseStudyBasePath);
				if(itsCaseStudyHelpDataInfoSystem)
				{
			// 3.1. Laita CaseStudy-moodi p‰‰lle
					fCaseStudyModeOn = true;
			// 4. Laita CaseStudyn kellonaika sein‰kelloajaksi
			// 5. Laita kaikkien n‰yttˆjen kello CaseStudy-aikaan
                    NFmiMetTime oldWallClockTime = oldCaseStudyModeOn ? oldLoadedCaseStudySystem.Time() : NFmiMetTime();
                    SetAllSystemsToCaseStudyModeChangeTime(oldWallClockTime, itsLoadedCaseStudySystem.Time(), false);
					itsLoadedCaseStudySystem.SetUpDataLoadinInfoForCaseStudy(itsDataLoadingInfoCaseStudy, caseStudyBasePath);

			// 6. Heit‰ kaikki dynaaminen data roskiin
					InfoOrganizer()->ClearDynamicHelpData(); // tuhoa kaikki olemassa olevat dynaamiset help-datat (ei edit-data tai sen kopiota ,eik‰ staattisia helpdatoja kuten topografia ja fraktiilit)
                    InitializeSatelImageCacheForCaseStudy();

                    // Lopetetaan cache datojen lataus ja siivous
                    CFmiQueryDataCacheLoaderThread::AutoLoadNewCacheDataMode(false);

                    // 7. Laita HelpDataInfoSystem-tarvittaviin paikkoihin k‰yttˆˆn (HelpDataInfoSystem-funktio, datanlataus-threadeihin jne.) JA aloita dynaamisen datan uudelleen lataus
			// CSmartMetDoc:in CaseStudyLoadingActions myˆs likaa cachet ja p‰ivitt‰‰ lopuksi kaikki n‰ytˆt!!, joten se pit‰‰ tehd‰ viimeisen‰...
                    ApplicationInterface::GetApplicationInterfaceImplementation()->CaseStudyLoadingActions(itsLoadedCaseStudySystem.Time(), "Going into case study mode"); // itsCaseStudyHelpDataInfoSystem -pit‰‰ olla ladattuna ja fCaseStudyModeOn pit‰‰ olla asetettuna true:ksi ennen t‰m‰n kutsua

					return true;
				}
			}
		}
		catch(std::exception &e)
		{
			specificError = e.what();
		}
		catch(...)
		{
			specificError = ::GetDictionaryString("Unknown error occured");
		}
		// 10. Tee MessageBox ilmenneest‰ virheest‰
		// 11. Palauta virhetilanteessa vanhat muuttuja arvot takaisin
		fCaseStudyModeOn = oldCaseStudyModeOn;
		itsCaseStudyHelpDataInfoSystem = oldCaseStudyHelpDataInfoSystem;
		itsLoadedCaseStudySystem = oldLoadedCaseStudySystem;
		std::string errStr(::GetDictionaryString("Error occured while loading Case Study data from file"));
		errStr += ":\n";
		errStr += theCaseStudyMetaFile;
		errStr += "\n";
		errStr += "Error was";
		errStr += ": '";
		errStr += specificError;
		errStr += "'\n";
		errStr += "SmartMet can't be using that data";
		std::string titleStr(::GetDictionaryString("Error while loading Case Study data"));
		::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(errStr.c_str()), CA2T(titleStr.c_str()), MB_OK | MB_ICONERROR);
		return false;
	}

	bool CaseStudyModeOn(void) 
	{
		return fCaseStudyModeOn;
	}

	void CaseStudyToNormalMode(void)
	{
		// 3.1. Laita CaseStudy-moodi p‰‰lle
		fCaseStudyModeOn = false;
        SetAllSystemsToCaseStudyModeChangeTime(itsLoadedCaseStudySystem.Time(), NFmiMetTime(), true);
		InfoOrganizer()->ClearDynamicHelpData(); // tuhoa kaikki olemassa olevat dynaamiset help-datat (ei edit-data tai sen kopiota ,eik‰ staattisia helpdatoja kuten topografia ja fraktiilit)
        InitializeSatelImageCacheForCaseStudy();
        // Palataan taas normaaliin cache datojen lataukseen ja siivoukseen
        CFmiQueryDataCacheLoaderThread::AutoLoadNewCacheDataMode(ApplicationWinRegistry().ConfigurationRelatedWinRegistry().AutoLoadNewCacheData());
        ApplicationInterface::GetApplicationInterfaceImplementation()->CaseStudyToNormalModeActions();
	}

	std::string MakeCaseStudyMemoryFilename(void)
	{
		std::string pathAndFileName = SpecialFileStoragePath();
		pathAndFileName += "CaseStudyMemory.csmeta";

		return pathAndFileName;
	}

	bool LoadCaseStudyMemory(void)
	{
		std::string caseStudyFileName = MakeCaseStudyMemoryFilename();
		if(NFmiFileSystem::FileExists(caseStudyFileName))
		{
			if(itsCaseStudySystem.ReadMetaData(caseStudyFileName, ApplicationInterface::GetSmartMetViewAsCView()))
				return true;
		}
		// jos ei ollut tiedostoa, tai siell‰ oli puppua, yritet‰‰n talllettaa t‰ss‰ perus-setti tiedostoon
		StoreCaseStudyMemory();

		return false;
	}

	bool StoreCaseStudyMemory(void)
	{
		if(itsCaseStudySystem.CategoriesData().size() > 0) // turha tallettaa mit‰‰n, jos CaseStudy-systeemi‰ ei ole edes alustettu
		{
            if(itsCaseStudySystem.AreStoredMetaDataChanged(itsCaseStudySystemOrig))
            {
			    std::string caseStudyFileName = MakeCaseStudyMemoryFilename();
			    return itsCaseStudySystem.StoreMetaData(ApplicationInterface::GetSmartMetViewAsCView(), caseStudyFileName, true);
            }
		}
		return false;
	}

	int DoMessageBox(const std::string & theMessage, const std::string &theTitle, unsigned int theMessageBoxType)
	{
        return ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(theMessage.c_str()), CA2T(theTitle.c_str()), theMessageBoxType);
	}

	void AllMapViewDescTopsTimeDirty(const NFmiMetTime &theTime)
	{
		unsigned int ssize = static_cast<unsigned int>(itsMapViewDescTopList.size());
		for(unsigned int i = 0; i<ssize; i++)
			MapViewDescTop(i)->MapViewCache().MakeTimeDirty(theTime);
	}

	boost::shared_ptr<NFmiDrawParam> GetUsedDrawParam(const NFmiDataIdent &theDataIdent, NFmiInfoData::Type /* theDataType */ )
	{
		if(itsModifiedPropertiesDrawParamList.Find(theDataIdent, 0, NFmiInfoData::kEditable, "", true))
			return itsModifiedPropertiesDrawParamList.Current(); // katsotaan lˆytyykˆ ensin jo k‰ytˆss‰ olevista DrawParameista haluttu (jos siin‰ on muutoksia arvoissa)
		else
			return InfoOrganizer()->CreateDrawParam(theDataIdent, 0, NFmiInfoData::kEditable);
	}

	const NFmiPoint& TimeFilterRangeStart(void)
	{
		return itsTimeFilterRangeStart;
	}

	const NFmiPoint& TimeFilterRangeEnd(void)
	{
		return itsTimeFilterRangeEnd;
	}

	bool WarnIfCantSaveWorkingFile(void)
	{
		return fWarnIfCantSaveWorkingFile;
	}

	NFmiHPlaceDescriptor* PossibleUsedDataLoadingGrid(void)
	{
		return itsPossibleUsedDataLoadingGrid;
	}

	bool UseEditedDataParamDescriptor(void)
	{
		return fUseEditedDataParamDescriptor;
	}

	NFmiParamDescriptor& EditedDataParamDescriptor(void)
	{
		return itsEditedDataParamDescriptor;
	}

	checkedVector<int>& DataLoadingProducerIndexVector(void)
	{
		return itsDataLoadingProducerIndexVector;
	}

	void PutWarningFlagTimerOn(void)
	{
        ApplicationInterface::GetApplicationInterfaceImplementation()->PutWarningFlagTimerOn();
	}

	NFmiProducerIdLister& ProducerIdLister(void)
	{
		return itsProducerIdLister;
	}

	void DoDataLoadingProblemsDlg(const std::string &theMessage)
	{
        CDataLoadingProblemsDlg dlg(CString(CA2T(theMessage.c_str())));
		dlg.DoModal();
	}

    TimeSerialModificationDataInterface& GenDocDataAdapter(void)
	{
		return *itsGenDocDataAdapter;
	}

	void InitGenDocDataAdapter(NFmiEditMapGeneralDataDoc *theDoc)
	{
		itsGenDocDataAdapter = boost::shared_ptr<NFmiGenDocDataAdapter>(new NFmiGenDocDataAdapter(theDoc));
	}

	bool UseMultithreaddingWithModifyingFunctions(void)
	{
		if(fUseMultiThreaddingWithEditingtools == false)
			return false; // jos k‰ytt‰j‰ on s‰‰t‰nyt ett‰ ei k‰ytet‰ multi-threaddausta, niin ei sitten
		if(itsMachineThreadCount <= 1)
			return false; // jos oli k‰ytˆss‰ vain yksi core, turha multi-threadata, koska se vain hidastaa
		return true; // muuten voidaan k‰ytt‰‰ multi-threaddausta
	}

	bool DataModificationInProgress(void)
	{
		return fDataModificationInProgress;
	}

	void DataModificationInProgress(bool newValue)
	{
		fDataModificationInProgress = newValue;
	}

	bool EditedPointsSelectionChanged(void)
	{
		return fEditedPointsSelectionChanged;
	}
	bool EditedPointsSelectionChanged(bool newValue)
	{
		bool oldValue = fEditedPointsSelectionChanged;
		fEditedPointsSelectionChanged = newValue;
		return oldValue;
	}

	size_t SelectedGridPointLimit(void) 
	{
		return itsSelectedGridPointLimit;
	}
	void SelectedGridPointLimit(size_t newValue) 
	{
		itsSelectedGridPointLimit = newValue;
	}

	bool DrawSelectionOnThisView(void)
	{
		return fDrawSelectionOnThisView;
	}

	void DrawSelectionOnThisView(bool newValue)
	{
		fDrawSelectionOnThisView = newValue;
	}

	const NFmiRect& CPGridCropRect(void)
	{
		if(IsCPGridCropInAction())
			return itsCPGridCropRect;
		else
		{
			static NFmiRect emptyRect(0, 0, 1, 1);
			return emptyRect;
		}
	}

	bool UseCPGridCrop(void)
	{
		return fUseCPGridCrop;
	}

	void UseCPGridCrop(bool newValue)
	{
		fUseCPGridCrop = newValue;
	}

	bool IsCPGridCropInAction(void)
	{
		if(fUseCPGridCrop && MetEditorOptionsData().ControlPointMode() && TimeSerialDataViewOn())
			return true;
		return false;
	}

	// Halutaan kysy‰ NFmiStationViewHAndler-luokasta, ett‰ onko  CP-crop mahdollisuus p‰‰ll‰, mutta sit‰ ei voi k‰ytt‰‰, jolloin piirret‰‰n crop-laatikkoon punainen rasti sen merkiksi.
	bool IsCPGridCropNotPlausible(void)
	{
		if(IsCPGridCropInAction() && itsCPGridCropLatlonArea && itsCPGridCropInnerLatlonArea == 0)
			return true;
		return false;
	}

	const boost::shared_ptr<NFmiArea> CPGridCropLatlonArea(void) 
	{
		return itsCPGridCropLatlonArea;
	}

	boost::shared_ptr<NFmiArea> CPGridCropInnerLatlonArea(void)
	{
		return itsCPGridCropInnerLatlonArea;
	}

	const NFmiPoint& CPGridCropMargin(void)
	{
		return itsCPGridCropMargin;
	}

	NFmiCPManagerSet& CPManagerSet(void)
	{
		return itsCPManagerSet;
	}

    NFmiApplicationWinRegistry& ApplicationWinRegistry()
    {
	    return itsApplicationWinRegistry;
    }

    void InitSpecialFileStoragePath(void)
    {
        DoVerboseFunctionStartingLogReporting(__FUNCTION__);
        itsSpecialFileStoragePath = NFmiSettings::Optional<std::string>("SmartMet::SpecialFileStoragePath", ""); // oletus arvo on tyhj‰
        if(!itsSpecialFileStoragePath.empty())
        {
            // pit‰‰ tehd‰ viel‰ absoluutti vs suhteellinen polku tarkasteluja
            NFmiFileString fileString(itsSpecialFileStoragePath);
            if(!fileString.IsAbsolutePath())
            { // jos oli suhteellinen polku, lis‰t‰‰n se kontrollihakemistoon
                itsSpecialFileStoragePath = itsBasicConfigurations.ControlPath() + "\\" + itsSpecialFileStoragePath;
            }

            // jos polussa ei ole lopussa kenoviivaa '\', lis‰t‰‰n se
            char ch = itsSpecialFileStoragePath[itsSpecialFileStoragePath.size()-1];
            if(ch != '\\' && ch != '/')
                itsSpecialFileStoragePath += kFmiDirectorySeparator;

            // varmistetaan viel‰ ett‰ hakemisto on olemassa
            if(!NFmiFileSystem::CreateDirectory(itsSpecialFileStoragePath))
            {
                std::string dialogTitle("Error in creating directory");
                std::string errorMessage("Unable to create SpecialFileStoragePath:\n");
                errorMessage += itsSpecialFileStoragePath;
                LogAndWarnUser(errorMessage, dialogTitle, CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
            }
        }
    }

    const std::string&  SpecialFileStoragePath(void) const 
    {
        if(itsSpecialFileStoragePath.empty())
            return itsBasicConfigurations.ControlPath();
        else
            return itsSpecialFileStoragePath;
    }

    void ZoomMapWithRelativeRect(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theMapArea, const NFmiRect &theZoomedRect)
    {
 	    boost::shared_ptr<NFmiArea> newZoomedArea(theMapArea->CreateNewArea(theZoomedRect));
	    if(newZoomedArea)
	    {
		    newZoomedArea->SetXYArea(NFmiRect(0,0,1,1)); // T‰m‰ on h‰m‰r‰ juttu, muttu jos xy-areaa ei laiteta 0,0 - 1,1:ksi, macroParam-systeemi ei toimi jos ollaan usean kartan ruudukossa
		    MapViewDirty(theMapViewDescTopIndex, true, true, true, true, false, false);
		    MapViewDescTop(theMapViewDescTopIndex)->BorderDrawDirty(true);
		    SetMapArea(theMapViewDescTopIndex, newZoomedArea);
	    }
	    MapViewDirty(theMapViewDescTopIndex, true, false, true, false, false, false); // t‰m‰ laittaa karttan‰ytˆn cachen sile‰ksi
   }

    void PanMap(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theMapArea, const NFmiPoint &theMousePoint, const NFmiPoint &theZoomDragUpPoint)
    {
        NFmiRect maxXYRect(theMapArea->XYArea(MapViewDescTop(theMapViewDescTopIndex)->MapHandler()->TotalArea().get()));
		NFmiRect pannedRect(theMapArea->XYArea());
		NFmiPoint panMove(theZoomDragUpPoint - theMousePoint);
		if(pannedRect.Right() + panMove.X() > maxXYRect.Right())
			panMove.X(maxXYRect.Right() - pannedRect.Right());
		if(pannedRect.Left() + panMove.X() < maxXYRect.Left())
			panMove.X(maxXYRect.Left() - pannedRect.Left());
		if(pannedRect.Top() + panMove.Y() < maxXYRect.Top())
			panMove.Y(maxXYRect.Top() - pannedRect.Top());
		if(pannedRect.Bottom() + panMove.Y() > maxXYRect.Bottom())
			panMove.Y(maxXYRect.Bottom() - pannedRect.Bottom());
		pannedRect += panMove;
        ZoomMapWithRelativeRect(theMapViewDescTopIndex, theMapArea, pannedRect);
    }

    NFmiRect ScaleZoomedAreaRect(boost::shared_ptr<NFmiArea> &theMapArea, const NFmiRect &theMaxRect, const NFmiPoint &theMousePoint, double theScaleFactor)
    {
        const NFmiRect &currentXYRect = theMapArea->XYArea();
	    NFmiRect newRect(currentXYRect);
	    double width = currentXYRect.Width() * theScaleFactor;
	    if(width > theMaxRect.Width())
		    width = theMaxRect.Width();
	    double height = currentXYRect.Height() * theScaleFactor;
	    if(height > theMaxRect.Height())
		    height = theMaxRect.Height();
	    newRect.Size(NFmiPoint(width, height));
        newRect.Center(currentXYRect.Center()); // pit‰‰ asettaa myˆs keskipiste vanhan rect:in keskipisteen kohdalle, ett‰ keskitys laskut onnistuvat

        boost::shared_ptr<NFmiArea> tmpArea(theMapArea->CreateNewArea(newRect));
        if(tmpArea)
        { // yritet‰‰n laskea uusi keskipiste siten, ett‰ hiiren paikka (laskettu latlon) pysyy zoomatessa paikallaan
            NFmiPoint origOffset = theMousePoint - currentXYRect.Center();
            NFmiPoint newOffset = origOffset * theScaleFactor;
            NFmiPoint movePoint = origOffset - newOffset;
            movePoint *= NFmiPoint(-1, -1);
            newRect.Center(newRect.Center() - movePoint);
        }

	    // rajoitetaan uutta recti‰ siten ett‰ se ei mene yli valitun kartta-alueen
	    if(newRect.Left() < theMaxRect.Left())
	    {
		    NFmiPoint newCenter(newRect.Center());
		    newCenter.X(newCenter.X() + (theMaxRect.Left() - newRect.Left()));
		    newRect.Center(newCenter);
	    }
	    if(newRect.Right() > theMaxRect.Right())
	    {
		    NFmiPoint newCenter(newRect.Center());
		    newCenter.X(newCenter.X() + (theMaxRect.Right() - newRect.Right()));
		    newRect.Center(newCenter);
	    }
	    if(newRect.Top() < theMaxRect.Top())
	    {
		    NFmiPoint newCenter(newRect.Center());
		    newCenter.Y(newCenter.Y() + (theMaxRect.Top() - newRect.Top()));
		    newRect.Center(newCenter);
	    }
	    if(newRect.Bottom() > theMaxRect.Bottom())
	    {
		    NFmiPoint newCenter(newRect.Center());
		    newCenter.Y(newCenter.Y() + (theMaxRect.Bottom() - newRect.Bottom()));
		    newRect.Center(newCenter);
	    }

	    // Pieni hienos‰‰tˆ tarvitaan jossain tapauksissa viel‰, ett‰ ei varmasti menn‰ max alueen yli (t‰m‰ on t‰rke‰‰ ainakin koko maailman karttojen kanssa)
	    if(newRect.Left() < theMaxRect.Left())
		    newRect.Left(theMaxRect.Left());

	    return newRect;
    }

    void ZoomMapInOrOut(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theMapArea, const NFmiPoint &theMousePoint, double theZoomFactor)
    {
        boost::shared_ptr<NFmiArea> totalMapArea = MapViewDescTop(theMapViewDescTopIndex)->MapHandler()->TotalArea();
        NFmiRect maxXYRect(theMapArea->XYArea(totalMapArea.get()));
        NFmiRect zoomedXYRect(ScaleZoomedAreaRect(theMapArea, maxXYRect, theMousePoint, theZoomFactor));
        ZoomMapWithRelativeRect(theMapViewDescTopIndex, theMapArea, zoomedXYRect);
    }

    // t‰t‰ kutsutaan aika-ajoin tallettamaan erilaisia asetuksia Windows rekistereihin 
    // ja kun SmartMetin CSmartMetDoc suljetaan eli ohjelmaa suljettaessa.
    void StoreSettingsToWinRegistry(void)
    {
        // Joitain arvoja s‰‰det‰‰n suoraan NFmiCrossSectionSystem-luokan l‰pi, joten t‰t‰ pit‰‰ p‰ivitt‰‰ takaisin rekistereihin
        NFmiCrossSectionSystem::CrossSectionInitValuesWinReg initValues;
        CrossSectionSystem()->StoreSettings(initValues);
        auto &crossSectionView = ApplicationWinRegistry().ConfigurationRelatedWinRegistry().CrossSectionViewWinRegistry();
        crossSectionView.StartPointStr(initValues.itsStartPointStr);
        crossSectionView.MiddlePointStr(initValues.itsMiddlePointStr);
        crossSectionView.EndPointStr(initValues.itsEndPointStr);
        crossSectionView.AxisValuesDefaultLowerEndValue(initValues.itsAxisValuesDefault.itsLowerEndOfPressureAxis);
        crossSectionView.AxisValuesDefaultUpperEndValue(initValues.itsAxisValuesDefault.itsUpperEndOfPressureAxis);
        crossSectionView.AxisValuesSpecialLowerEndValue(initValues.itsAxisValuesSpecial.itsLowerEndOfPressureAxis);
        crossSectionView.AxisValuesSpecialUpperEndValue(initValues.itsAxisValuesSpecial.itsUpperEndOfPressureAxis);
        crossSectionView.VerticalPointCount(initValues.itsVerticalPointCount);
        crossSectionView.HorizontalPointCount(initValues.itsWantedMinorPointCount);

        // Joitain arvoja s‰‰det‰‰n suoraan NFmiMapViewDescTop-luokan l‰pi, joten n‰it‰ pit‰‰ p‰ivitt‰‰ takaisin rekistereihin
        for(unsigned int i = 0; i < itsMapViewDescTopList.size(); i++)
            MapViewDescTop(i)->StoreToMapViewWinRegistry(*ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(i));
    }

    const NFmiMetTime& ActiveMapTime(void)
    {
        return CurrentTime(itsActiveMapDescTopIndex);
    }

    unsigned int ActiveMapDescTopIndex(void)
    {
        return itsActiveMapDescTopIndex;
    }

    // T‰t‰ ei saa asettaa GenDocin ulkoa suoraan, t‰m‰n asetukset tapahtuvat t‰m‰n 
    // luokan sis‰ll‰, joten t‰m‰ ei ole julkinen metodi
    void ActiveMapDescTopIndex(unsigned newValue)
    {
        if(newValue < itsMapViewDescTopList.size())
            itsActiveMapDescTopIndex = newValue;
        else
            itsActiveMapDescTopIndex = 0;
    }

    MultiProcessClientData& GetMultiProcessClientData(void)
    {
        return itsMultiProcessClientData;
    }

    const std::string& GetSmartMetGuid(void)
    {
        return ApplicationDataBase().GuidStr();
    }

    bool UseMultiProcessCpCalc(void)
    {
        return itsApplicationWinRegistry.UseMultiProcessCpCalc();
    }
    void UseMultiProcessCpCalc(bool newValue)
    {
        itsApplicationWinRegistry.UseMultiProcessCpCalc(newValue);
    }

    const std::string& GetMasterProcessExeName()
    {
        return tmMasterTcpProcessName;
    }

    const std::string& GetWorkerProcessExeName()
    {
        return tmWorkerTcpProcessName;
    }

    int MasterProcessRunningCount()
    {
        // Tarkista kuinka monta Master-prosessia on k‰ynniss‰
        NFmiApplicationDataBase::AppSpyData appData(std::make_pair(GetMasterProcessExeName(), false)); // false tarkoittaa ett‰ aplikaatiolta ei pyydet‰ versionumeroa
		std::string appVersionsStrOutDummy; // t‰m‰ pit‰‰ antaa NFmiApplicationDataBase::CountProcessCount-funktiolle, mutta sit‰ ei k‰ytet‰
		return NFmiApplicationDataBase::CountProcessCount(appData, appVersionsStrOutDummy);
    }

    // MP-CP systeemiss‰ on marter ja worker prosessi executablet.
    // T‰m‰ funktio rakentaa tilanteeseen sopivan (master/worker) prosessin bin‰‰rin absoluuttisen polun.
    // Polku rakennetaan 3 eri tavalla:
    // 1. bin‰‰rien polkuja ei ole annettu konffeissa, rakenna polku niin ett‰ bin‰‰reill‰ on oletus nimet ja ne sijaitsevat SmartMet bin‰‰ri hakemistossa.
    // 2. Polut on annettu konffeissa absoluuttisina, palauta se suoraan
    // 3. Polut on annettu suhteellisina, jolloin lopullinen polku rakennetaan SmartMet bin‰‰rihakemiston suhteen
    std::string MakeMpcpProcessPath(const std::string &absoluteSmartMetAppPath, const std::string &processSettingsKey, const std::string &defaultProcessName)
    {
        std::string configuredMasterProcessPath = NFmiSettings::Optional<std::string>(processSettingsKey, "");
        if(configuredMasterProcessPath.empty())
        {

            std::string usedAppPath = absoluteSmartMetAppPath;
            usedAppPath += "\\";
            usedAppPath += defaultProcessName;
            return usedAppPath;
        }
        else
        {
            
            if(NFmiFileSystem::IsAbsolutePath(configuredMasterProcessPath))
            {
                return configuredMasterProcessPath;
            }
            else
            {
                return PathUtils::getAbsoluteFilePath(configuredMasterProcessPath, absoluteSmartMetAppPath);
            }
        }
    }

    bool MakeSureToolMasterPoolIsRunning2()
    {
        // Tarkista onko Master-prosessi jo k‰ynniss‰, jos oli, lopetetaan
        if(MasterProcessRunningCount() <= 0)
        {
            std::string usedAppPath = ApplicationDataBase().apppath;
            usedAppPath = NFmiStringTools::UrlDecode(usedAppPath); // valitettavasti t‰m‰ stringi on url-encodattu ja se pit‰‰ purkaa...

            // Jos ei ollut, k‰ynnist‰ Master-prosessi
            std::string commandStr;
            commandStr += "\""; // laitetaan lainausmerkit Master-prosessi komento polun ymp‰rille, jos siin‰ sattuisi olemaan spaceja
            commandStr += MakeMpcpProcessPath(usedAppPath, "SmartMet::MP-CP::MasterProcessPath", GetMasterProcessExeName());
            commandStr += "\""; // laitetaan lainausmerkit Master-prosessi komento polun ymp‰rille, jos siin‰ sattuisi olemaan spaceja

            commandStr += " -H"; // laitetaan TM-master prosessi piiloon -h optiolla
            commandStr += " -l \""; // laitetaan lokitiedosto -l optiolla
            commandStr += itsMultiProcessLogFilePath;
            commandStr += "\""; // laitetaan lainausmerkit lokitiedoston polun ymp‰rille, jos siin‰ sattuisi olemaan spaceja
            commandStr += " -L "; // laitetaan lokitus taso -L optiolla
            commandStr += boost::lexical_cast<std::string>((int)itsMultiProcessPoolOptions.LogLevel());
            if(itsMultiProcessPoolOptions.MultiProcessPoolOptions().verbose_logging)
                commandStr += " -v"; // laitetaan verbose lokitus p‰‰lle

            commandStr += " -W "; // laitetaan Worker-prosessin polku -W optiolla
            commandStr += "\""; // laitetaan lainausmerkit Worker-prosessi komento polun ymp‰rille, jos siin‰ sattuisi olemaan spaceja
            commandStr += MakeMpcpProcessPath(usedAppPath, "SmartMet::MP-CP::WorkerProcessPath", GetWorkerProcessExeName());
            commandStr += "\""; // laitetaan lainausmerkit Worker-prosessi komento polun ymp‰rille, jos siin‰ sattuisi olemaan spaceja

//            WORD showWindow = SW_HIDE;
            return CFmiProcessHelpers::ExecuteCommandInSeparateProcess(commandStr, true);
        }
        else
        {
            LogMessage("MP-CP calculations's Master process was already up and running, continuing to calculations...", CatLog::Severity::Debug, CatLog::Category::Editing);
            return true;
        }
    }

    bool MakeSureToolMasterPoolIsRunning(void)
    {
        return MakeSureToolMasterPoolIsRunning2();
//        boost::thread tt(boost::bind(&GeneralDocImpl::MakeSureToolMasterPoolIsRunning2, this));
        // Eli ei j‰‰d‰ odottamaan lopetusta tt.join():illa
    }

    NFmiMultiProcessPoolOptions& MultiProcessPoolOptions(void)
    {
        return itsMultiProcessPoolOptions;
    }

    bool AllowRightClickDisplaySelection(void)
    {
        return itsApplicationWinRegistry.AllowRightClickDisplaySelection();
    }
    void AllowRightClickDisplaySelection(bool newValue)
    {
        itsApplicationWinRegistry.AllowRightClickDisplaySelection(newValue);
    }

    NFmiFixedDrawParamSystem& FixedDrawParamSystem()
    {
        return itsFixedDrawParamSystem;
    }

    std::string MakeUsedFixedDrawParamsRootPath()
    {
        bool useLocals = ApplicationWinRegistry().UseLocalFixedDrawParams();
        std::string rootPath = useLocals ? "FixedDrawParams" : ApplicationWinRegistry().FixedDrawParamsPath();
        rootPath = PathUtils::getAbsoluteFilePath(rootPath, useLocals ? WorkingDirectory() : ControlDirectory());
        return rootPath;
    }

    void ReloadFixedDrawParams()
    {
        itsFixedDrawParamSystem.Initialize(MakeUsedFixedDrawParamsRootPath());
    }

    std::string MakeBackUpViewMacroFileName(bool fUseNormalBackup)
    {
        std::string fileName = SpecialFileStoragePath();
        if(fUseNormalBackup)
            fileName += "backupViewMacro.wmr";
        else
            fileName += "crashBackupViewMacro.wmr";
        return fileName;
    }

    void StoreBackUpViewMacro(bool fUseNormalBackup)
    {
        try
        {
            // 1. Ota 'kuva' nykyhetkest‰
            NFmiViewSettingMacro currentStateViewMacro;
            FillViewMacroInfo(currentStateViewMacro, "Current state", "Current state view macro description");
            // 2. Talleta viewmacro stringiin
            std::stringstream out;
            out << currentStateViewMacro;
            // 3. Talleta stringi viewmacro 'turvallisesti' backup tiedostoon
            NFmiFileSystem::SafeFileSave(MakeBackUpViewMacroFileName(fUseNormalBackup), out.str());
            if(fUseNormalBackup)
                NFmiFileSystem::SafeFileSave(MakeBackUpViewMacroFileName(false), out.str()); // Jos normaali lopetus, talletetaan t‰ss‰ samalla myˆs crashBackupViewMacro
        }
        catch(std::exception &e)
        {
            std::string errorMessage = "StoreBackUpViewmacro failed: ";
            errorMessage += e.what();
            LogAndWarnUser(errorMessage, "StoreBackUpViewmacro failed", CatLog::Severity::Error, CatLog::Category::Macro, true);
        }
    }

    void ApplyBackUpViewMacro()
    {
        try
        {
            SnapshotViewMacro(true); // true = tyhjennet‰‰n redo-lista
            NFmiViewSettingMacro backupViewMacro;
            bool status = ReadViewMacro(backupViewMacro, MakeBackUpViewMacroFileName(true), false); // false = Ei n‰ytet‰ error dialogia, jos esim. tiedostoa ei ole
            if(!status)
                throw std::runtime_error("BackupViewMacro was not correctly loaded, maybe there were no file...");
            LoadViewMacroInfo(backupViewMacro, false, false);
            ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Loading backup view-macro (F12)"); // t‰m‰ sitten p‰ivitt‰‰ kaikki ruudut
        }
        catch(std::exception &e)
        {
            std::string errorMessage = "ApplyBackUpViewMacro failed (F12): ";
            errorMessage += e.what();
            LogAndWarnUser(errorMessage, "ApplyBackUpViewMacro failed", CatLog::Severity::Error, CatLog::Category::Macro, false);
        }
    }

    void ApplyViewMacro(NFmiViewSettingMacro &theViewMacro, const std::string &theViewMacroName, const std::string &theErrorExtraText)
    {
        try
        {
            if(theViewMacro.ViewMacroWasCorrupted())
                throw std::runtime_error(theViewMacroName + " was not correctly loaded, maybe there were no file...");
            SnapshotViewMacro(true); // true = tyhjennet‰‰n redo-lista
            LoadViewMacroInfo(theViewMacro, false, false);
            ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs(std::string("Loading view-macro: ") + theViewMacroName); // t‰m‰ sitten p‰ivitt‰‰ kaikki ruudut
        }
        catch(std::exception &e)
        {
            std::string errorMessage = std::string("Apply ") + theViewMacroName + " failed " + theErrorExtraText + ": ";
            errorMessage += e.what();
            LogAndWarnUser(errorMessage, std::string("Apply ") + theViewMacroName + " failed", CatLog::Severity::Error, CatLog::Category::Macro, false);
        }
    }

    void ApplyCrashBackUpViewMacro()
    {
        // Kun lis‰sin toisen ApplyViewMacro metodiin toisen bool parametrin, meni k‰‰nt‰j‰ sekaisin ja 
        // muuttaa "xxx" c-string:in osoitteen booleaniksi, eik‰ edes varoita 4. tasolla vaikka yleens‰ bool muunnoksia ei saa mill‰‰n vaikenemaan.
        ApplyViewMacro(itsCrashBackupViewMacro, std::string("CrashBackupViewMacro"), "(CTRL + F12)");
    }

    void LoadCrashBackUpViewMacro()
    {
        try
        {
            bool status = ReadViewMacro(itsCrashBackupViewMacro, MakeBackUpViewMacroFileName(false), false); // false = Ei n‰ytet‰ error dialogia, jos esim. tiedostoa ei ole
            if(!status)
                itsCrashBackupViewMacro.ViewMacroWasCorrupted(true);
        }
        catch(std::exception &e)
        {
            itsCrashBackupViewMacro.ViewMacroWasCorrupted(true);
            std::string errorMessage = "LoadCrashBackUpViewMacro failed: ";
            errorMessage += e.what();
            LogAndWarnUser(errorMessage, "LoadCrashBackUpViewMacro failed", CatLog::Severity::Error, CatLog::Category::Macro, true);
        }
    }

    void ApplyBackupViewMacro(bool fUseNormalBackup)
    {
        if(fUseNormalBackup)
            ApplyBackUpViewMacro();
        else
            ApplyCrashBackUpViewMacro();
    }

    const NFmiPoint& PreciseTimeSerialLatlonPoint() const
    {
        return itsPreciseTimeSerialLatlonPoint;
    }

    void PreciseTimeSerialLatlonPoint(const NFmiPoint &newValue)
    {
        itsPreciseTimeSerialLatlonPoint = newValue;
    }

    bool IsPreciseTimeSerialLatlonPointUsed()
    {
        if(!AllowRightClickDisplaySelection())
        { // Sallittu k‰ytt‰‰ vain kun ei olla vanhassa Right-click -moodissa
            boost::shared_ptr<NFmiFastQueryInfo> editedInfo = EditedInfo();
            if(editedInfo && CtrlViewFastInfoFunctions::GetMaskedCount(editedInfo, NFmiMetEditorTypes::kFmiSelectionMask, AllowRightClickDisplaySelection()) <= 1)
            {
                return true;
            }
        }
        return false;
    }

    void InitSatelliteImageCacheTotal()
    {
        DoVerboseFunctionStartingLogReporting(__FUNCTION__);
        try
        {
            itsSatelliteImageCacheSystemPtr->Init(*HelpDataInfoSystem());
            NFmiSatelliteImageCacheSystem::StartUpdateThreads(itsSatelliteImageCacheSystemPtr);
            std::function<void(const ImageCacheUpdateData&)> updateCallback = std::bind(&GeneralDocImpl::ImageCacheUpdatedCallback, this, std::placeholders::_1);
            std::function<void(const ImageCacheUpdateData&)> loadCallback = std::bind(&GeneralDocImpl::ImageCacheLoadedCallback, this, std::placeholders::_1);
            itsSatelliteImageCacheSystemPtr->SetCallbacks(updateCallback, loadCallback);
        }
        catch(std::exception &e)
        {
            string errStr = __FUNCTION__;
            errStr += " - Initialization error with SatelImageCache-system: \n";
            errStr += e.what();
            LogAndWarnUser(errStr, "Problems in InitSatelliteImageCacheTotal", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
        }
    }

    void ImageCacheUpdatedCallback(const ImageCacheUpdateData &theUpdatedData)
    {
        std::lock_guard<std::mutex> lock(itsImageCacheUpdateDataMutex);
        itsImageCacheUpdateData.insert(itsImageCacheUpdateData.end(), theUpdatedData.begin(), theUpdatedData.end());
    }

    void ImageCacheLoadedCallback(const ImageCacheUpdateData &theLoadedData)
    {
        std::lock_guard<std::mutex> lock(itsImageCacheUpdateDataMutex);
        itsImageCacheUpdateData.insert(itsImageCacheUpdateData.end(), theLoadedData.begin(), theLoadedData.end());
    }

    int DoImageCacheUpdates(const ImageCacheUpdateData &theImageCacheUpdateData)
    {
        if(theImageCacheUpdateData.size())
        {
            bool foundUpdates = false;
            for(unsigned int i = 0; i < static_cast<unsigned int>(itsMapViewDescTopList.size()); i++)
                foundUpdates |= DoImageCacheUpdates(i, theImageCacheUpdateData);

            if(foundUpdates)
            {
                ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: satellite images from files loaded update"); // t‰m‰ sitten p‰ivitt‰‰ kaikki ruudut
                return 1;
            }
        }

        return 0;
    }

    bool DoImageCacheUpdates(unsigned int theDescTopIndex, const ImageCacheUpdateData &theImageCacheUpdateData)
    {
        // Tutkitaan lˆytyykˆ karttan‰ytˆst‰ mit‰‰n theImageCacheUpdateData -listan jutuista.
        // Jos lˆytyy, liataan sen karttan‰yttˆcache ja palautetaan true.
        // Jos ei, palautetaan false.

        bool updateStatus = false;
        NFmiMapViewDescTop *descTop = MapViewDescTop(theDescTopIndex);

        int cacheRowNumber = 0; // cache row indeksi alkaa 0:sta!!
        NFmiPtrList<NFmiDrawParamList> *drawParamListVector = descTop->DrawParamListVector();
        NFmiPtrList<NFmiDrawParamList>::Iterator iter = drawParamListVector->Start();
        for(; iter.Next();)
        {
            NFmiDrawParamList *aList = iter.CurrentPtr();
            if(aList)
            {
                for(aList->Reset(); aList->Next();)
                {
                    boost::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
                    for(const auto &updateData : theImageCacheUpdateData)
                    {
                        if(drawParam->Param() == updateData.first)
                        {
                            long offsetInMinutes = NFmiSatelView::ImagesOffsetInMinutes(drawParam->Param());
                            NFmiMetTime minTime = updateData.second;
                            minTime.ChangeByMinutes(-offsetInMinutes);
                            NFmiMetTime maxTime = updateData.second;
                            maxTime.ChangeByMinutes(offsetInMinutes);
                            descTop->MapViewCache().MakeTimesDirty(minTime, maxTime, cacheRowNumber);// clean cache row
                            updateStatus = true;
                        }
                    }
                }
            }
            cacheRowNumber++;
        }
        return updateStatus;
    }

    // Katsoo onko imageCachesta tulleita ruudunp‰ivitys tarpeita. 
    // Jos on, tee tarvittavat n‰yttˆjen likaukset, p‰ivit‰ ne jos oli tarvetta.
    // Jos n‰yttˆj‰ p‰ivitettiin palauta 1 jos ei, palauta 0.
    // HUOM! Vaikka olisi joku p‰ivitys tarve listalla, ei se tarkoita sit‰ ett‰ 
    // kyseinen kuva olisi mill‰‰n n‰ytˆll‰ valittuna. Eli voi tulla ilmoitus ett‰
    // on tullut uusi meteosat9 HRV kanavan kuva, mutta sit‰ ei v‰ltt‰m‰tt‰ ole mill‰‰n
    // karttan‰ytˆn rivill‰ valittuna.
    int DoImageCacheUpdates(void)
    {
        ImageCacheUpdateData tmpImageCacheUpdateData;

        { 
            // T‰ss‰ tehd‰‰n s‰ie turvallinen siirto updatedatoista v‰liaikaiseen muuttujaan, n‰in lukko on p‰‰ll‰ minimaalisen ajan
            std::lock_guard<std::mutex> lock(itsImageCacheUpdateDataMutex);
            tmpImageCacheUpdateData.swap(itsImageCacheUpdateData);
        }

        return DoImageCacheUpdates(tmpImageCacheUpdateData);
    }

    NFmiSatelliteImageCacheSystem& SatelliteImageCacheSystem()
    {
        return *itsSatelliteImageCacheSystemPtr;
    }

    void ReloadFailedSatelliteImages()
    {
        ImageCacheUpdateData imageCacheUpdateData;
        SatelliteImageCacheSystem().ResetFailedImages(imageCacheUpdateData);
        if(DoImageCacheUpdates(imageCacheUpdateData))
        {
            RefreshApplicationViewsAndDialogs("Map view: Reload failed satellite images update", true, false);
        }
    }

    void OnAcceleratorSetHomeTime(int theMapViewDescTopIndex)
    {
        NFmiMetTime currentTime(1);
        if(CaseStudyModeOn())
            currentTime = itsLoadedCaseStudySystem.Time(); // CaseStudy moodissa sein‰kelloksi otetaan CasStudyn oma aika
        CenterTimeControlView(theMapViewDescTopIndex, currentTime, true);
        RefreshApplicationViewsAndDialogs("Map view: Selcted map time is set to wall clock time", true, false, theMapViewDescTopIndex);
    }

    const std::string& RootViewMacroPath()
    {
        return itsRootViewMacroPath;
    }

    bool BetaProductGenerationRunning()
    {
        return itsBetaProductionSystem.BetaProductGenerationRunning();
    }

    void BetaProductGenerationRunning(bool newValue)
    {
        itsBetaProductionSystem.BetaProductGenerationRunning(newValue);
    }

    NFmiBetaProductionSystem& BetaProductionSystem()
    {
        return itsBetaProductionSystem;
    }

    LogAndWarnFunctionType GetLogAndWarnFunction()
    {
        return [this](const std::string &errorStr, const std::string &titleStr, CatLog::Severity severity, CatLog::Category category, bool justLog) {this->LogAndWarnUser(errorStr, titleStr, severity, category, justLog); };
    }

    void DoGenerateBetaProductsChecks()
    {
        static bool firstTime = true;
        if(firstTime)
        {
            // 1. kerralla pit‰‰ mahdollisesti avata piilossa Beta-product dialogi, muuten tuotteiden generointia ei voi tehd‰.
            // Jos generointi ei ole p‰‰ll‰ tai ladatussa Beta-automaatio listassa ei ole tuotteita, ei dialogia tarvitse initialisoida.
            firstTime = false;
            if(BetaProductionSystem().AutomationModeOn() && !BetaProductionSystem().UsedAutomationList().IsEmpty())
            {
                ApplicationInterface::GetApplicationInterfaceImplementation()->CreateBetaProductDialog(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation());
            }
        }

        if(BetaProductionSystem().DoNeededBetaAutomation())
        {
        }
    }

    void SetCurrentGeneratedBetaProduct(const NFmiBetaProduct *theBetaProduct)
    {
        itsCurrentGeneratedBetaProduct = theBetaProduct;
    }

    const NFmiBetaProduct* GetCurrentGeneratedBetaProduct()
    {
        return itsCurrentGeneratedBetaProduct;
    }

    void SetAllViewIconsDynamically(void)
    {
        ApplicationInterface::GetApplicationInterfaceImplementation()->SetAllViewIconsDynamically();
    }

    const NFmiMetTime& LastEditedDataSendTime()
    {
        return itsLastEditedDataSendTime;
    }

    bool IsLastEditedDataSendHasComeBack()
    {
        return fLastEditedDataSendHasComeBack;
    }

    void ListDirRecursive(const std::string &startDirNane, std::vector<std::string>& dirFileList, const std::string &ext, bool justRelativePath)
    {
        try
        {
            std::string usedStartDirName = startDirNane;
            PathUtils::addDirectorySeparatorAtEnd(usedStartDirName);
            boost::filesystem::recursive_directory_iterator rdi(usedStartDirName);
            boost::filesystem::recursive_directory_iterator end_rdi;

            dirFileList.clear();

            for(; rdi != end_rdi; rdi++)
            {
                boost::filesystem::path currentPath = rdi->path();
                if(ext == currentPath.extension().string())
                {
                    if(justRelativePath)
                    {
                        std::string fullFilePath = currentPath.string();
                        std::string relativeFilePath(fullFilePath.begin() + usedStartDirName.size(), fullFilePath.end());
                        dirFileList.push_back(relativeFilePath);
                    }
                    else
                        dirFileList.push_back(currentPath.string());
                }
            }
        }
        catch(...)
        { // jos esim. hakemistoa ei ole olemassa, recursive_directory_iterator konstruktori heitt‰‰ poikkeuksen
        }
    }

    const std::vector<std::string>& ViewMacroFileNames(bool updateList)
    {
        static bool firstTime = true;
        if(firstTime || updateList)
        {
            std::string viewMacroFileExtension = ".vmr";
            ListDirRecursive(itsRootViewMacroPath, itsViewMacroFileNames, viewMacroFileExtension, true);
        }
        return itsViewMacroFileNames;
    }

    std::vector<NFmiLightWeightViewSettingMacro>& ViewMacroDescriptionList()
    {
        return itsViewMacroDescriptionList;
    }

    // Jos itsViewMacroPath     on C:\xxx\yyy\zzz
    // ja itsRootViewMacroPath  on C:\xxx\
    // t‰llˆin t‰m‰ funktio palauttaa arvon: yyy\zzz
    std::string GetRelativeViewMacroPath()
    {
        std::string relativePath = itsViewMacroPath;
        NFmiStringTools::ReplaceAll(relativePath, itsRootViewMacroPath, "");
        return relativePath;
    }

    const std::vector<std::string>& SmartToolFileNames(bool updateList)
    {
        static bool firstTime = true;
        if(firstTime || updateList)
        {
            std::string fileExtension = "." + SmartToolInfo()->ScriptFileExtension();
            ListDirRecursive(SmartToolInfo()->RootLoadDirectory(), itsSmartToolFileNames, fileExtension, true);
        }
        return itsSmartToolFileNames;
    }

    const std::vector<std::string>& CorruptedViewMacroFileList() const
    {
        return itsCorruptedViewMacroFileList;
    }

    void RemoveFromCorruptedViewmacroList(const std::string &theFileName)
    {
        std::vector<std::string> &v = itsCorruptedViewMacroFileList;
        v.erase(std::remove(v.begin(), v.end(), theFileName), v.end());
    }

    NFmiEditDataUserList& EditDataUserList()
    {
        return itsEditDataUserList;
    }

    void AddMessageBasedData(std::unique_ptr<NFmiQueryData> data, const std::string &filePath, const std::string &filePattern)
    {
        if(data)
        {
            AddQueryData(data.release(), filePath, filePattern, NFmiInfoData::kObservations, "");
        }
    }

    // Tarkistetaan onko tehty uutta Hake/Kaha sanoma pohjaista queryDataa ja laitetaan se infoOrganizeriin
    void CheckForNewWarningMessageData()
    {
#ifndef DISABLE_CPPRESTSDK
        AddMessageBasedData(itsWarningCenterSystem.getHakeQueryData(), "fakeHakeFileName", "fakeHakeFilePattern"); // "New warning center message data");
        AddMessageBasedData(itsWarningCenterSystem.getKahaQueryData(), "fakeKaHaFileName", "fakeKaHaFilePattern"); // "New Kansalais Havainto message data");
#endif // DISABLE_CPPRESTSDK
    }

    void ApplyStartupViewMacro()
    {
        // Kun lis‰sin toisen ApplyViewMacro metodiin toisen bool parametrin, meni k‰‰nt‰j‰ sekaisin ja 
        // muuttaa "xxx" c-string:in osoitteen booleaniksi, eik‰ edes varoita 4. tasolla vaikka yleens‰ bool muunnoksia ei saa mill‰‰n vaikenemaan.
        ApplyViewMacro(itsStartupViewMacro, std::string("StartupViewMacro"), "(SHIFT + F12)");
    }

    void FillStartupViewMacro()
    {
        FillViewMacroInfo(itsStartupViewMacro, "Startup state", "Taking startup state view macro");
    }

    void MakeHTTPRequest(const std::string &theUrlStr, std::string &theResponseStrOut, bool fDoGet)
    {
        Q2Client().MakeHTTPRequest(theUrlStr, theResponseStrOut, fDoGet);
    }

    // Return data that is in grid format (model data) and has one year of data (one-year climatology data)
    boost::shared_ptr<NFmiFastQueryInfo> GetModelClimatologyData()
    {
        auto infoVector = itsSmartInfoOrganizer->GetInfos(NFmiInfoData::kClimatologyData);
        for(auto &info : infoVector)
        {
            if(NFmiFastInfoUtils::IsModelClimatologyData(info))
            {
                return info; // returns the first suitable data
            }
        }
        return boost::shared_ptr<NFmiFastQueryInfo>();
    }

    // Return data that is wanted producer and has surface T-min and T-max params
    boost::shared_ptr<NFmiFastQueryInfo> GetMosTemperatureMinAndMaxData()
    {
        std::vector<FmiProducerName> acceptedProducers{ static_cast<FmiProducerName>(122), static_cast<FmiProducerName>(123) };
        std::vector<FmiParameterName> requiredParameters{kFmiMinimumTemperature, kFmiMaximumTemperature};
        for(auto producerId : acceptedProducers)
        {
            auto infoVector = itsSmartInfoOrganizer->GetInfos(producerId);
            for(auto &info : infoVector)
            {
                size_t foundParamCount = 0;
                for(auto paramId : requiredParameters)
                {
                    if(info->Param(paramId))
                        foundParamCount++;
                    else
                        break;
                }
                if(requiredParameters.size() == foundParamCount)
                    return info; // returns the first data that contained all required parameters
            }
        }
        return boost::shared_ptr<NFmiFastQueryInfo>();
    }
	
    std::vector<boost::shared_ptr<NFmiFastQueryInfo>> FilterOnlyGridSurfaceNonYearLongData(std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &origDataVector)
    {
        std::vector<boost::shared_ptr<NFmiFastQueryInfo>> filteredData;
        for(auto &info : origDataVector)
        {
            if(info->IsGrid())
            {
                if(info->SizeLevels() == 1)
                {
                    if(!NFmiFastInfoUtils::IsYearLongData(info))
                    {
                        filteredData.push_back(info);
                    }
                }
            }
        }
        return filteredData;
    }

    boost::shared_ptr<NFmiFastQueryInfo> SeekMostFavoriteProducerData(std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &dataVector, const std::vector<unsigned long> &favoriteProducers)
    {
        if(dataVector.size())
        {
            for(auto producer : favoriteProducers)
            {
                for(auto &info : dataVector)
                {
                    if(info->Producer()->GetIdent() == static_cast<long>(producer))
                        return info;
                }
            }
            // If producers not in favorite list, just return first on the vector
            return dataVector[0];
        }
        // Return empty if there is no data on vector
        return boost::shared_ptr<NFmiFastQueryInfo>();
    }

    boost::shared_ptr<NFmiFastQueryInfo> GetFavoriteSurfaceModelFractileData()
    {
        auto infoVector = itsSmartInfoOrganizer->GetInfos(NFmiInfoData::kClimatologyData);
        auto filteredInfoVector = FilterOnlyGridSurfaceNonYearLongData(infoVector);
        // Seek first if there is data from these producers, in this order. 
        // If none found, then return first data in filtered list.
        // 199 = harmonie, 54 = gfs, 260 = MEPS, 
        std::vector<unsigned long> favoriteProducers{kFmiMTAECMWF, 199, 54, kFmiMTAHIRLAM, 260};
        return SeekMostFavoriteProducerData(filteredInfoVector, favoriteProducers);
    }

    bool UseWmsMaps()
    {
        return ApplicationWinRegistry().UseWmsMaps();
    }

    void UseWmsMaps(bool newValue)
    {
        ApplicationWinRegistry().UseWmsMaps(newValue);
        MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, true, true, false, false, false);
    }

#ifndef DISABLE_CPPRESTSDK
    Wms::WmsSupport& WmsSupport()
    {
        return wmsSupport;
    }
#endif // DISABLE_CPPRESTSDK

    std::vector<int> HelpDataIdsForParameterSelectionSystem()
    {
        std::string idStr;
        std::vector<int> idVector;
        idStr = NFmiSettings::Optional("SmartMet::AddParams::helpDataIDs", idStr); 
        if(!idStr.empty())
        {
            try
            {
                for (std::string id : NFmiStringTools::Split(idStr, ","))
                {
                    idVector.push_back(std::stoi(id));
                }
            }
            catch(std::exception &e)
            {
                LogAndWarnUser(e.what(), "Unable to read help data ids", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
            }
        }
        return idVector;
    }

    std::vector<std::string> CustomMenuFolders()
    {
        std::vector<std::string> customMenus;
        std::vector<std::string> customMenuList = HelpDataInfoSystem()->GetUniqueCustomMenuList();
        for(auto menuItem : customMenuList)
        {
            if(menuItem == g_ObservationMenuName)
                continue; // Observation-menu is skipped, because those needs to be added among existing observations.
            customMenus.push_back(menuItem);
        }
        return customMenus;
    }

    void InitParameterSelectionSystem()
    {
        DoVerboseFunctionStartingLogReporting(__FUNCTION__);
        try
        {
            auto customCategories = CustomMenuFolders();
            parameterSelectionSystem.initialize(ProducerSystem(), ObsProducerSystem(), SatelImageProducerSystem(),
                *InfoOrganizer(), *HelpDataInfoSystem(), HelpDataIdsForParameterSelectionSystem(), customCategories);

            auto macroParamSystemCallBackFunction = [this]() {return std::ref(this->MacroParamSystem()); };
            parameterSelectionSystem.setMacroParamSystemCallback(macroParamSystemCallBackFunction);

            // Add other data to help data. 
            if(capDataSystem.useCapData())
            {
                NFmiProducer prod(NFmiSettings::Optional<int>("SmartMet::Warnings::ProducerId", 12345), "CAP"); // No official producerId, reads this from Cap.conf. If multiple ids, read them all here.
                std::string menuString = "Warnings";
                parameterSelectionSystem.addHelpData(prod, menuString, NFmiInfoData::kCapData);
            }

            if(ConceptualModelData().Use())
            {
                NFmiProducer prod(1028, "Analysis"); // No official producerId
                std::string menuString = "Conceptual analysis";
                std::string displayName = ConceptualModelData().DefaultUserName(); // Conceptual analysis uses displayName to fetch correct data!
                parameterSelectionSystem.addHelpData(prod, menuString, NFmiInfoData::kConceptualModelData, displayName);
            }
            
            // TODO
            // WmsSupport(), Streamlines, Edited data
        }
        catch(std::exception &e)
        {
            LogAndWarnUser(e.what(), "Problems in InitParameterSelectionSystem", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
        }
    }

    void UpdateParameterSelectionSystem()
    {
        ParameterSelectionSystem().updateData();
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Update Parameter Selection dialog system", SmartMetViewId::ParameterSelectionDlg);
    }

    AddParams::ParameterSelectionSystem& ParameterSelectionSystem()
    {
        return parameterSelectionSystem;
    }

    NFmiBasicSmartMetConfigurations& BasicSmartMetConfigurations()
    {
        return itsBasicConfigurations;
    }

    int RunningTimeInSeconds()
    {
        return itsBasicConfigurations.RunningTimeInSeconds();
    }

    void UpdateViewForOffScreenDraw(unsigned int theMapViewDescTopIndex)
    {
        ApplicationInterface::GetApplicationInterfaceImplementation()->UpdateViewForOffScreenDraw(theMapViewDescTopIndex); // T‰m‰ p‰ivitt‰‰ oikeat n‰yttˆrivit kohdalleen ja niihin oikeat parametrit
    }

    void DoOffScreenDraw(unsigned int theMapViewDescTopIndex, CBitmap &theDrawedScreenBitmapOut)
    {
        ApplicationInterface::GetApplicationInterfaceImplementation()->DoOffScreenDraw(theMapViewDescTopIndex, theDrawedScreenBitmapOut);
    }

    Q2ServerInfo& GetQ2ServerInfo()
    {
        return itsQ2ServerInfo;
    }
    
    Warnings::CapDataSystem& GetCapDataSystem()
    {
        return capDataSystem;
    }

    bool MakeControlPointAcceleratorAction(ControlPointAcceleratorActions action, const std::string &updateMessage)
    {
        if(MetEditorOptionsData().ControlPointMode())
        {
            if(CPManager()->MakeControlPointAcceleratorAction(action))
            {
                TimeSerialViewDirty(true);
                RefreshApplicationViewsAndDialogs(updateMessage, true, true, 0);
                return true;
            }
        }
        return false;
    }

    void InitGriddingProperties()
    {
        ApplicationWinRegistry().InitGriddingProperties(IsToolMasterAvailable());
    }

    NFmiMacroParamDataCache& MacroParamDataCache()
    {
        return itsMacroParamDataCache;
    }

    NFmiMacroParamDataCache itsMacroParamDataCache;
    std::string itsLastLoadedViewMacroName; // t‰t‰ nime‰ k‰ytet‰‰n smartmet:in p‰‰ikkunan title tekstiss‰ (jotta k‰ytt‰j‰ n‰kee mik‰ viewMacro on ladattuna)
    Warnings::CapDataSystem capDataSystem;
    Q2ServerInfo itsQ2ServerInfo;
    AddParams::ParameterSelectionSystem parameterSelectionSystem;
#ifndef DISABLE_CPPRESTSDK
    Wms::WmsSupport wmsSupport;
#endif // DISABLE_CPPRESTSDK
    NFmiViewSettingMacro itsStartupViewMacro; // T‰h‰n ladataan SmartMetin tyhj‰ alkutilanne, jotta SHIFT + F12 pikan‰pp‰imell‰ p‰‰see takaisin alkutilaan milloin tahansa ajon aikana.
    NFmiEditDataUserList itsEditDataUserList; // Jos konffeissa on niin m‰‰r‰tty, t‰h‰n luetaan editoijien tunnukset, ja t‰t‰ listaa k‰ytet‰‰n L‰het‰-data-tietokantaan dialogissa
    std::vector<std::string> itsSmartToolFileNames; // T‰h‰n on listattu rekursiivisesti kaikki perus smarttool hakemistosta haetut tiedostot, joissa on oikea vmr -p‰‰te. Tiedoston nimill‰ on relatiivienn polku (suhteessa perus hakemistoon)
    std::vector<std::string> itsCorruptedViewMacroFileList; // T‰h‰n talletetaan havaitut korruptoituneet n‰yttˆmakro nimet, ett‰ niiden avulla voidaan varoittaa k‰ytt‰ji‰ niist‰ viewMacro-dialogissa 
    std::vector<NFmiLightWeightViewSettingMacro> itsViewMacroDescriptionList;
    std::vector<std::string> itsViewMacroFileNames; // T‰h‰n on listattu rekursiivisesti kaikki perus viewMacro hakemistosta haetut tiedostot, joissa on oikea vmr -p‰‰te. Tiedoston nimill‰ on relatiivienn polku (suhteessa perus hakemistoon)
    NFmiMetTime itsLastEditedDataSendTime; // Milloin on viimeksi l‰hetetty editoitu data tietokantaan
    bool fLastEditedDataSendHasComeBack; // Onko viimeisen editoidun datan tietokantaan l‰hetyksen j‰lkeen tullut operatiivista dataa k‰yttˆˆn?
    const NFmiBetaProduct *itsCurrentGeneratedBetaProduct; // T‰h‰n asetetaan beta-tuotannossa se juuri nyt generoitava beta-tuote, muuten t‰ss‰ pit‰‰ olla nullptr (ei omista, ei tuhoa)
    NFmiBetaProductionSystem itsBetaProductionSystem;
    ImageCacheUpdateData itsImageCacheUpdateData; // T‰h‰n tulee tietoa p‰ivitett‰vist‰ satel/muut parametrien kuvista ja niiden ajoista
    std::mutex itsImageCacheUpdateDataMutex; // t‰ll‰ lukitaan itsImageCacheUpdateData, koska siihen tulee lis‰yksi‰ worker-threadista ja siit‰ siit‰ luetaan p‰‰-threadista
    std::shared_ptr<NFmiSatelliteImageCacheSystem> itsSatelliteImageCacheSystemPtr;
    NFmiPoint itsPreciseTimeSerialLatlonPoint; // Aina kun tehd‰‰n hiirell‰ kartalta vasen click valintaa, laitetaan t‰h‰n tarkat koordinaatit
    NFmiViewSettingMacro itsCrashBackupViewMacro; // T‰m‰ viewMacro saadaan ladattua k‰yttˆˆn SmartMetissa pikan‰pp‰imell‰ CTRL + F12. 
                                                // Kyseess‰ on edellisen SmartMet istunnon aikana talletettu viewMacro, t‰t‰ talletetaan m‰‰r‰ajoin (joka 1.5 minuutti) 
                                                // crashBackupViewMacro.wmr tiedostoon. T‰m‰ siis ladataan k‰ynnistyksen yhteydess‰ ja sitten SmartMet alkaa tekem‰‰n kyseiseen tiedostoon uusia talletuksia.
    NFmiFixedDrawParamSystem itsFixedDrawParamSystem; // T‰h‰n on tallletettu kaikki ns. tehdasasetus drawParamit. K‰ytt‰j‰t voivat k‰ytt‰‰ n‰it‰, mutta eiv‰t voi muuttaa.
    float itsHardDriveFreeLimitForConfSavesInMB; // Kuinka paljon levytilaa pit‰‰ olla minimiss‰‰n, jotta SmartMet suostuu tekem‰‰n konffi/muita asetus tiedosto talletuksia
    float itsHardDriveFreeLimitForEditedDataSavesInMB; // Kuinka paljon levytilaa pit‰‰ olla minimiss‰‰n, jotta SmartMet suostuu tekem‰‰n editoidun datan backup tiedosto talletuksia
    NFmiMultiProcessPoolOptions itsMultiProcessPoolOptions;
    std::string itsMultiProcessLogFilePath; // t‰m‰n avulla initialisoidaan boost-log
    std::string itsMultiProcessLogFilePattern; // t‰m‰n avulla siivotaan loki hakemistosta vanhat lokit pois
    MultiProcessClientData itsMultiProcessClientData;

    bool fStoreLastLoadedFileNameToFile; // joskus on tarpeen ett‰ SmartMet tallettaa viimeisen ladatun tiedoston nimen talteen, mutta oletuksena se ei ole tarpeen.
    unsigned int itsActiveMapDescTopIndex; // t‰h‰n talletetaan sen karttan‰ytˆn indeksi, mink‰ oletetaan olevan aktiivinen, oletus on 0
    std::string itsSpecialFileStoragePath; // SmartMetin voi s‰‰t‰‰ halutessa tallettamaan tietyt tiedostot pois SmartMetin kontrollihakemistosta. 
                                            // T‰t‰ tarvitaan erityisesti ulkomaisissa Dropbox-asennuksissa (ks. SMARTMET-393).
                                            // Jos asetusta ei ole s‰‰detty, k‰ytet‰‰n polkuna normaalia kontrollihakemistoa.
    NFmiApplicationWinRegistry itsApplicationWinRegistry;
	NFmiRect itsCPGridCropRect; // Jos kontrollipiste muokkaukset halutaan rajoittaa tietyn ali-hilan alueelle, 
								// k‰ytet‰‰n t‰t‰ hilapiste-rect:i‰. T‰‰ll‰ on siis bottom-left ja top-right editoidun datan hila-indeksit.
								// T‰m‰n arvoa p‰ivitet‰‰n kun zoomataan p‰‰ikkunaa.
	bool fUseCPGridCrop; // flagi ett‰ k‰ytet‰‰nkˆ croppia vai ei. T‰m‰ menee p‰‰lle aikasarjaikkunasta.
	boost::shared_ptr<NFmiArea> itsCPGridCropLatlonArea; // T‰ss‰ on tallessa latlon kulmapisteet croppi laatikosta (kartalle piirtoa varten)
	boost::shared_ptr<NFmiArea> itsCPGridCropInnerLatlonArea; // T‰ss‰ on tallessa latlon kulmapisteet sisemm‰st‰ croppi laatikosta (kartalle piirtoa varten)
	NFmiPoint itsCPGridCropMargin; // t‰ss‰ kerrotaan kuinka monta hilapistett‰ x- ja y-suunnassa on ulko- ja sis‰-laatikon v‰liss‰. 
									// T‰ll‰ alueella muutokset menev‰t t‰ydest‰ l‰hes nollaan, jotta reunoista tulisi mahdollisimman pehme‰t.

	bool fDrawSelectionOnThisView; // haluan piirt‰‰ maalattaessa valitut pisteet piirto ikkunaan, t‰m‰ on sit‰ varten tehty kikka vitonen
	bool fEditedPointsSelectionChanged; // onko muutettu editoinnissa valittuja pisteit‰, jos on, niin ruutua pit‰‰ p‰ivitt‰‰ tietyll‰ tavalla.
	NFmiBasicSmartMetConfigurations itsBasicConfigurations; // T‰‰ll‰ on tietyt perus asetukset, mitk‰ alustetaan jo ennen GenDocin alustusta
															// CSmartMetApp:in InitInstance-metodissa. Uusi CrashRpt-systeemi vaatii tiettyjen tietojen asetuksia
															// ennen kuin GenDoc saadaan alustetuksia, ja siksi t‰m‰ piti tehd‰ n‰in osissa ja ehk‰ hieman hankalasti.

	bool fDataModificationInProgress;	// uudet datan modifiointi rutiinit (FmiModifyEditdData -namespacessa) saattavat h‰iriinty‰ multi-thread koodin tai 
										// erillisess‰ threadissa ajosta johtuen (progress/Cancel vaatii erillist‰ s‰iett‰). T‰st‰ johtuen kokeilen, 
										// josko muokkausten aikan SmartMetin p‰ivityksen esto auttaisi asiaa. T‰m‰ esto toimii ainakin printtauksien 
										// yhteydess‰ koettujen ongelmien kanssa.
	bool fUseMultiThreaddingWithEditingtools; // k‰ytt‰j‰ voi asettaa erikseen sallitaanko multi-threaddaavan koodin k‰yttˆ editoitaessa
	boost::shared_ptr<TimeSerialModificationDataInterface> itsGenDocDataAdapter;
	NFmiHelpDataInfoSystem itsHelpDataInfoSystem; // eri apudatojen tiedot ovat t‰ss‰ ja ne luetaan tiedostosta
	boost::shared_ptr<NFmiHelpDataInfoSystem> itsCaseStudyHelpDataInfoSystem; // K‰ytˆss‰ olevan CaseStudyn eri apudatojen tiedot ovat t‰ss‰
	bool fCaseStudyModeOn;

	NFmiAviationStationInfoSystem itsWmoStationInfoSystem;
	bool fWmoStationInfoSystemInitialized;
	NFmiCaseStudySystem itsCaseStudySystem; // t‰ll‰ tehd‰‰n uusia CaseStudy-datoja
	NFmiCaseStudySystem itsCaseStudySystemOrig; // Lopussa kun SmartMetia suljetaan, tarkistetaan onko tehty muutoksia suhteessa originaali olioon ja vasta sitten tehd‰‰n talletukset asetuksiin
	NFmiCaseStudySystem itsLoadedCaseStudySystem; // t‰h‰n ladataan olemassa oleva CaseStudy-data k‰yttˆˆn

	int itsMacroDirectoriesSyncronizationCounter; // kuinka monta kertaa makro-hakemistojen synkronointi on tehty, alku arvo on 1, jos luku on 1, on serverill‰ prefer-status, muuten lokaali on prefer-tilassa

	bool fEditedDataNotInPreferredState; // Jos pika lataus on tehty, editoitu data tarkastetaan sen rakenteen ja alkuajan ja sein‰kellon suhteen. Jos data ei ole 'ihanteellinen', yritet‰‰n ladata jo ladatun tilalle vimeisint‰ virallista dataa, kunnes t‰rpp‰‰ tai tulee joku aika raja vastaan.
	NFmiMilliSecondTimer itsEditedDataNeedsToBeLoadedTimer; // kun todetaan ett‰ editoitua dataa pit‰‰ alkaa kytt‰‰m‰‰n, k‰ynnistet‰‰n t‰m‰ timeri, ett‰ tiedet‰‰n kuinka kauan ollaan odotettu. 
															// Jotta k‰ytt‰j‰‰ voidaan informoida asiasta ja lis‰ksi ehk‰ tietyn ajan yritt‰misen j‰lkeen voidaan lopettaa homma toivottomana.
	bool fEditedDataNeedsToBeLoaded; // jos auto startup ep‰onnistuu lataamasta mit‰‰ editoitua dataa, laitetaan t‰m‰ lippu p‰‰lle.
									// Sen j‰lkeen pit‰‰ MainFramessa timerill‰ k‰yd‰ v‰liajoin katsomassa onko tullut sopivaa dataa ladattavaksi, kunnes sit‰ lˆytyy.

	NFmiMacroPathSettings itsMacroPathSettings;
	NFmiIgnoreStationsData itsIgnoreStationsData;
	int itsSatelDataRefreshTimerInMinutes; // T‰m‰ on brute force ratkaisu ei queryData pohjaisten datojen (mm. satel kuvat, k‰site analyysit)
											// ruudunp‰ivityksen varmistamiseen. T‰m‰ kertoo kuinka usein tarkasteluja tehd‰‰n. Jos arvo on <= 0, ei tarkastella ollenkaan.
	int itsMachineThreadCount; // kertoo kuinka paljon koneessa on theaddaus mahdollisuutta (CPU x core x hyper-threading)
	NFmiDataQualityChecker itsDataQualityChecker;
	NFmiAnalyzeToolData itsAnalyzeToolData;
	NFmiModelDataBlender itsModelDataBlender;
	checkedVector<NFmiColor> itsGeneralColors; // Erilaisissa paikoissa on aiemmin k‰ytetty luotausn‰ytˆn v‰risetti‰ hyv‰kseen kuvaamaan eri k‰yri‰ (aikasarja, trajektori, jne)
												// Nyt teen oman v‰ri setin t‰ll‰iseen k‰yttˆˆn, koska esim. LENiss‰ on v‰ritetty kaikki luotaukset mustaksi

	ObsDataLoadedReporter itsObsDataLoadedReporter;
	NFmiAutoComplete itsAutoComplete;
	std::deque<NFmiViewSettingMacro> itsUndoRedoViewMacroList;
	int itsUndoRedoViewMacroListPointer; // T‰m‰ osoittaa nykyiseen kohtaan yll‰ olevaan undo-listaan.
										// T‰m‰ indeksi osoittaa siis siihen viewMacroon, johon seuraava undo kohdistuu.
										// Jos arvo on -1, ollaan listan alun ohi ja undo:ta ei voi tehd‰

	bool fStartUpDataLoadCheckDone; // Kun SmartMet lataa datan alussa automaattisesti, se ei tarkista luettua dataa mitenk‰‰n.
									// Jos editoidaan dataa, pit‰‰ datan olla oikeanlaista, konfiguraatioiden mukaista. Tein vivun,josta
									// n‰kee, ett‰ pit‰‰kˆ data tarkistaa viel‰ kun editoidaan tai dataa l‰hetet‰‰n tietokantaan.
	NFmiWindTableSystem itsWindTableSystem;
	NFmiPoint itsTimeSerialViewSizeInPixels;

	NFmiPtrList<NFmiDrawParamList> itsCopyPasteDrawParamListVector; // t‰t‰ k‰ytet‰‰n kopioimaan ja pasteamaan koko mapViewdescTopin kaikkien rivien kaikki parametrit
	bool fCopyPasteDrawParamListVectorUsedYet;
	NFmiDrawParamList itsCopyPasteCrossSectionDrawParamList; // t‰t‰ k‰ytet‰‰n kopioimaan ja pasteamaan poikkileikkausn‰yttˆrivi drawParameita
	bool fCopyPasteCrossSectionDrawParamListUsedYet; // t‰m‰ vaikuttaa siihen ilmestyykˆ pop-up valikkoon paste
	NFmiDrawParamList itsCopyPasteDrawParamList; // t‰t‰ k‰ytet‰‰n kopioimaan ja pasteamaan n‰yttˆrivi drawParameita
	bool fCopyPasteDrawParamListUsedYet; // t‰m‰ vaikuttaa siihen ilmestyykˆ pop-up valikkoon paste

	NFmiTimeBag itsTimeSerialViewTimeBag; // t‰m‰ on aikasarja ikkunassa n‰kyv‰ aikav‰li
	NFmiMapViewTimeLabelInfo itsMapViewTimeLabelInfo;
	bool itsShowToolTipTimeView;
	bool itsShowToolTipTempView;
	bool itsShowToolTipTrajectoryView;

	std::vector<std::string> itsSynopDataFilePatternSortOrderVector; // t‰m‰ on m‰‰r‰tty asetuksissa helpdatainfo.conf-tiedostossa.

	CWnd *itsTransparencyContourDrawView; // t‰m‰ pit‰‰ asettaa aina kun joku n‰yttˆ piirt‰‰ isoviiva/contoureja. T‰m‰n avulla piirret‰‰n
											// l‰pin‰kyv‰t parametrit pinell‰ kikalla ja Gdiplus-piirtoa hyv‰ksi k‰ytt‰en.
	NFmiDrawParam itsCopyPasteDrawParam; // t‰t‰ k‰ytet‰‰n piirto-ominaisuuksien copy/paste komennon talletuspaikkana
	bool fCopyPasteDrawParamAvailableYet; // onko yht‰‰n piirto-ominaisuus oliota kopioitu viel‰ t‰h‰n?
	NFmiSeaIcingWarningSystem itsSeaIcingWarningSystem;
	NFmiConceptualModelData itsConceptualModelData; // rintama piirto ominaisuudet on t‰‰ll‰
	std::vector<NFmiMapViewDescTop*> itsMapViewDescTopList; // t‰h‰n tulee kaikkien eri mapview desctoppien tiedot
	NFmiPoint itsOutOfEditedAreaTimeSerialPoint;
	NFmiVPlaceDescriptor itsSoundingPlotLevels; // luotauksia piirret‰‰n karttan‰ytˆlle vain tiettyihin vakio painepinnoille
	int itsDataToDBCheckMethod; // kun dataa l‰hetet‰‰n tietokantaan, kysyt‰‰n l‰hetys dialogissa, mit‰ tarkastuus metodia
								// halutaan k‰ytt‰‰. 1=datavalidation, 2= niin kuin asetuksissa sanotaan
								// DBChecker ohitetaan t‰ss‰.
	std::vector<NFmiProducer> itsExtraSoundingProducerList; // settingseissa on lista mahdollisista ylim‰‰r‰isist‰ tuottajista,
															// joita on tarkoitus katsella luotaus-n‰ytˆll‰
	NFmiQ2Client itsQ2Client;
	NFmiPoint itsStationDataGridSize; // miss‰ hilassa asema/piste datan hilaus lasketaan

	bool fMustDrawTimeSerialView; // tietyss‰ tilanteessa CDokumentti voidaan pakottaa piirt‰m‰‰n aikasarjan‰yttˆ
	bool fMustDrawCrossSectionView; // tietyss‰ tilanteessa CDokumentti voidaan pakottaa piirt‰m‰‰n poikkileikkausn‰yttˆn‰yttˆ
	bool fMustDrawTempView; // tietyss‰ tilanteessa CDokumentti voidaan pakottaa piirt‰m‰‰n luotausn‰yttˆ
	bool fUseOnePressureLevelDrawParam;
	bool fRawTempRoundSynopTimes;
	NFmiPoint itsRawTempUnknownStartLonLat;
	bool fWarnIfCantSaveWorkingFile; // kaikilla konfiguraatioilla ei voi tallettaa tyˆtiedostoa, eik‰ siit‰ tarvitse varoittaa, t‰m‰ luetaan settingseist‰
	NFmiRect itsCrossSectionDataViewFrame; // t‰m‰n avulla voidaan laskea yhden poikkileikkaus dataikkunan koko pikseleiss‰
	NFmiPoint itsCrossSectionViewSizeInPixels; // poikkileikkausn‰yttˆ ikkunan clientti osan koko pikseleiss‰
	NFmiHelpEditorSystem itsHelpEditorSystem;

	NFmiProducerSystem itsProducerSystem;
	NFmiProducerSystem itsObsProducerSystem;
	NFmiProducerSystem itsSatelImageProducerSystem; // satelliitti/tutka kuvien tuottajat laitetaan t‰h‰n (k‰ytet‰‰n mm. parametri pop-up valikoiden tekoon)
#ifndef DISABLE_CPPRESTSDK
    HakeMessage::Main itsWarningCenterSystem;
#endif // DISABLE_CPPRESTSDK
	NFmiFileCleanerSystem itsFileCleanerSystem;

	bool fIsTEMPCodeSoundingDataAlsoCopiedToEditedData;
	std::string itsLastTEMPDataStr; // t‰h‰n on talletettu aina viimeksi k‰ytetty TEMP koodin purussa ollut stringi
	bool fUseEditedDataParamDescriptor; // jos true, CreateLoadedData-metodissa k‰ytet‰‰n alla olevaa parambagi‰, muuten se otetaan 1. source datsta
	NFmiParamDescriptor itsEditedDataParamDescriptor; // editor.conf-tiedostossa voidaan halutessa m‰‰ritt‰ rakennettava
													  // ladatun datan parambagi. Jos ei m‰‰ritelty, ottaa parambagin 1. source-datsta

	NFmiTrajectorySystem* itsTrajectorySystem;
	bool fShowMouseHelpCursorsOnMap; // kartta ruudukolla n‰ytett‰vien apukursorien tila, halutaan s‰‰dett‰v‰ksi, koska mm. kontrollipiste tyˆkalu vilkkuu viel‰ liikaa
	int itsLastSelectedSynopWmoId; // mik‰ on synop-taulukkon‰yttˆ ikkunassa viimeksi valitun aseman wmoid
	bool fMouseOnMapView; // onko hiiren kursori kartan p‰‰ll‰ vai ei
	int itsSynopHighlightStationWmoId;
	bool fShowSynopHighlightStationMarker; // n‰ytet‰‰nkˆ merkki kartta n‰ytˆss‰, kun jokin asema on mahdollisesti valittu
	NFmiPoint itsSynopHighlightLatlon; // t‰ss‰ latlon pisteess‰ oli viimeksi merkattu synop asema synop-teksti-taulukko n‰ytˆss‰
	bool fSynopDataGridViewOn; // onko synop teksti muodossa katselu ikkuna n‰kyviss‰ vai ei
	bool fTimeSerialDataViewOn; // onko aikasarja n‰yttˆ p‰‰ll‰ vai ei
	NFmiMTATempSystem itsMTATempSystem; // tieto luotaus moodista ja jos MTA-moodi p‰‰ll‰, tietoa luotausn‰ytˆss‰ n‰ytett‰vist‰ luotauksista
	NFmiPtrList<NFmiDrawParamList> *itsCrossSectionDrawParamListVector; // poikkileikkaus ikkuna-ruudukon drawparamit
	NFmiObsComparisonInfo itsObsComparisonInfo;
	bool fMapMouseDragPanMode; // ollaanko karttan‰ytˆll‰ tekem‰ss‰ pan:i‰ hiiren keski nappia vet‰m‰ll‰
	NFmiSynopPlotSettings itsSynopPlotSettings;
	NFmiSynopStationPrioritySystem itsSynopStationPrioritySystem;
	NFmiHPlaceDescriptor *itsPossibleUsedDataLoadingGrid; // jos editorissa halutaan k‰ytt‰‰ muuta kuin apudatojen omia hilam‰‰reit‰, k‰ytet‰‰n t‰t‰ conffi-tiedostosta luettua speksi‰
	NFmiCrossSectionSystem itsCrossSectionSystem; // poikkileikkaus piirron pisteiden hanskaus systeemi
	string itsLatestMacroParamErrorText; // kun macroParam piirret‰‰n karttan‰ytˆlle, ja tulee virhe, talletetaan t‰h‰n aina viimeisin virhe ilmoitus
	NFmiMacroParamSystem itsMacroParamSystem; // t‰m‰ hanskaa macroParamit, joilla voi piirt‰‰ smarttoolmacroja karttan‰ytˆlle
	string itsCurrentMacroText; // t‰ss‰ on tallessa viimeisin smarttool-dialogin teksti
								// k‰ytet‰‰n macroparam-n‰ytˆn testivaiheessa

	NFmiString itsSavedDirectory; // talletettu tyˆhakemisto, mik‰ voidaan ladata k‰yttˆˆn myˆhemmin

	string itsDoAtSendCommandString; // t‰m‰ stringi annetaan system-komennolla (jos niin asetettu) kun dataa l‰hetet‰‰n tietokantaan
	bool fUseDoAtSendCommand; // tehd‰‰nkˆ ed. mainittu komento dataa l‰hetett‰ess‰.
    CtrlViewUtils::FmiSmartMetEditingMode itsSmartMetEditingMode;

	string itsViewMacroPath;
	string itsRootViewMacroPath; // jos ollaan 'rootti' hakemistossa, ei laiteta ..-hakemistoa viewparamlistaan
    std::shared_ptr<NFmiViewSettingMacro> itsCurrentViewMacro;
    NFmiViewSettingMacro itsHelperViewMacro; // t‰m‰ on apuna n‰yttˆ-makrojen latauksessa ja talletuksessa. Todelliset makrot ovat makrolistassa
											// mutta esim. windows asetukset talletetaan t‰m‰n avulla ennen kuin todellinen makro on edes olemassa ja kopioidaan
											// t‰st‰ lopulliseen makroon makro listaan.

	string itsLandBorderShapeFile; // mist‰ filest‰ shape luetaan
	boost::shared_ptr<Imagine::NFmiGeoShape> itsLandBorderGeoShape; // t‰h‰n luetaan alkuper‰inen shape-file, josta sitten tehd‰‰n sopivia path-olioita
    boost::shared_ptr<Imagine::NFmiPath> itsLandBorderPath; // t‰h‰n lasketaan normaali maailman path kerran
    boost::shared_ptr<Imagine::NFmiPath> itsPacificLandBorderPath; // t‰h‰n lasketaan pacific maailman path kerran
    std::vector<boost::shared_ptr<Imagine::NFmiPath> > itsCutLandBorderPaths; // t‰h‰n lasketaan jokaiseen eri karttapohjaan (kartat 1-4) leikatut rajaviivat. N‰ill‰ voidaan optimoida zoomaukseen k‰ytettyj‰ rajaviiva laskuja.
	NFmiProducerIdLister itsProducerIdLister; // jos ajon aikana tarvitaan ladatun datan alkuper‰isi‰ tuottajia, ovat ne t‰ss‰ tallessa

	NFmiSmartToolInfo itsSmartToolInfo;

	boost::shared_ptr<NFmiDrawParam> GetDrawDifferenceDrawParam(void){return itsDrawDifferenceDrawParam;}
	boost::shared_ptr<NFmiDrawParam> itsDrawDifferenceDrawParam; // t‰m‰n avulla piirret‰‰ toolmasterilla erotus kent‰t eri parametreille
	boost::shared_ptr<NFmiDrawParam> GetSelectedGridPointDrawParam(void){return itsSelectedGridPointDrawParam;}
	boost::shared_ptr<NFmiDrawParam> itsSelectedGridPointDrawParam; // t‰m‰n avulla piirret‰‰n toolmasterilla editoinnissa valittujen pisteiden joukko
	size_t itsSelectedGridPointLimit; // kuinka monta hilapistett‰ pit‰‰ v‰hint‰‰n olla valittuna ennen kuin menn‰‰n uuteen hatchill‰ visualisointi tapaan
	string itsSmartToolEditingErrorText;

	bool fActivateParamSelectionDlgAfterLeftDoubleClick; // kikka 6, jotta zeditmap2view tiet‰‰, pit‰‰kˆ dialogi aktivoida

    // N‰ytet‰‰nkˆ erilaisia apudatoja aikasarjaikkunoissa?
    // 0 = ei apudatoja k‰ytˆss‰,
    // 1. bitti p‰‰ll‰, n‰ytet‰‰n asema fraktiili dataa ja havaintoja, analyysi dataa, viim. editoitu data ja l‰j‰ eri malli datoja
    // 2. bitti p‰‰ll‰ n‰ytet‰‰n lyhytt‰ fraktiili dataa
    int itsShowHelperDataInTimeSerialView;
    // N‰ytet‰‰n yhden vuoden mallipohjaista klimatologia dataa
    bool fShowHelperData3InTimeSerialView;
    // N‰ytet‰‰n T-min ja max dataa halutusta datal‰hteest‰ apuna
    bool fShowHelperData4InTimeSerialView;

	int itsBrushToolLimitSetting; // 0=ei rajoitusta, 1=ei alle, 2= ei yli ja 3= arvon abs. asetus (rajoittaa miten siveltimell‰ voi rajoittaa arvoja)
	float itsBrushToolLimitSettingValue;
	int itsTimeEditSmootherValue;
	int itsTimeEditSmootherMaxValue;

	bool fCPDataBackupUsed; // Tarkistus bitti sille onko jostain syyst‰ pit‰nyt lukea kˇnnistyksen yhteydess‰ CP-datan backup. Tiedon avulla ei tarvitse lukea uudestaan Init vaiheessa CP-pisteit‰ (ja tuhota muutos k‰yri‰).

	checkedVector<int> itsDataLoadingProducerIndexVector; // t‰ss‰ on tiedot siit‰, mit‰ tuottajaa k‰ytet‰‰n mill‰kin ajan hetkell‰ latauksen yhteydess‰
	NFmiProjectionCurvatureInfo* itsProjectionCurvatureInfo; // t‰m‰ tiet‰‰ miten piirret‰‰n lat-lon viivat eri projektioihin
	bool fTimeSerialViewDirty; // k‰ytet‰‰n aikasarja ikkunan kaksoispuskuroinnisssa

	bool Printing(void){return fPrinting;};

	void Printing(bool newStatus){fPrinting = newStatus;}
	bool fPrinting;

	NFmiEditMapDataListHandler* itsListHandler; //laura lis‰si 30081999
	NFmiDataLoadingInfo itsDataLoadingInfoNormal;
	NFmiDataLoadingInfo itsDataLoadingInfoCaseStudy;

	NFmiInfoOrganizer *itsSmartInfoOrganizer; // holds different kind of smartinfo's
	NFmiDrawParamList *itsTimeSerialViewDrawParamList; // knows all the parameters that has to be drawn on timeserial-view
	FastDrawParamList itsModifiedPropertiesDrawParamList;

	NFmiMapConfigurationSystem itsMapConfigurationSystem;

	FmiTimeEditMode itsTimeEditingMode;
	bool fOpenPopup;
	NFmiMenuItemList* itsPopupMenu;
	unsigned long itsPopupMenuStartId;

	NFmiLocationSelectionTool *itsLocationSelectionTool2;

	std::string itsFileDialogDirectoryMemory;
	unsigned long itsTimeSerialViewIndex; // hiiren oikean klikkauksen paikka talletetaan t‰h‰n (RUMAA!!)

	bool fUseMasksInTimeSerialViews;

	int itsToolTipRowIndex; // hiiren kursorin lepopaikan n‰yttˆrivi tooltippi‰ varten
	int itsToolTipColumnIndex; // hiiren kursorin lepopaikan n‰yttˆrivin sarake numero tooltippi‰ varten
	NFmiPoint itsToolTipLatLonPoint; // hiiren kursorin lepopaikka tooltippi‰ varten (kartan koordinaatistossa latlon-piste)
	NFmiMetTime itsToolTipTime; // tooltipill‰ voi olla mik‰ tahansa ruudukon aika, ja se talletetaan t‰h‰n
	boost::shared_ptr<NFmiDrawParam> itsDefaultEditedDrawParam; // t‰m‰n jouduin ottamaan k‰yttˆˆn kun upudatat
											  // alkoivat tuottaa ongelmia t‰ll‰inen luodaan aina
											  // uudesta datasta (esim. 1. parametri). Jos k‰ytt‰j‰
											  // on laittanut esim apuparametrin n‰yttˆˆn, ei ole
											  // olemassa mit‰‰n EditedDrawParam:ia jonka voisi antaa
											  // mm. NFmiTimeControlView:lle ja ajan juoksutus saattaa
											  // menn‰ pieleen apuparametrien kanssa.
	boost::shared_ptr<NFmiAreaMaskList> itsParamMaskListMT; // universaali maskilista, olen tehnyt siit‰ nyt multi-threaddausta tukevan version. T‰st‰ pit‰‰ aina pyyt‰‰ kopio ParamMaskList -metodilla
	int itsCurrentViewRowIndex; // jokaisella n‰yttˆrivill‰ (ruudukossa) on indeksi ja
								// kun tehd‰‰n valintoja popup-menun kautta, rivin indeksi
								// talletetaan t‰h‰n muuttujaan, ett‰ n‰yttˆparametrien
								// muutokset saadaan tehty‰ oikeaan kohtaan drawparamlist-listiss‰
	int itsCurrentCrossSectionRowIndex;
	int itsFilterFunction; // testi k‰ytˆss‰ toistaiseksi 0 = avg, 1 = max ja 2 = min filtterit
	int itsTestFilterUsedMask; // joko 1 = NFmiMetEditorTypes::kFmiNoMask tai 4 = kFmiSelectionMask
	bool fUseMasksWithFilterTool;
	NFmiRect itsAreaFilterRangeLimits;	// filtterimuokkauksen aluerajojen min ja max t‰ss‰
	NFmiRect itsAreaFilterRangeStart;	// filtterimuokkauksen aluerajat on nyt talletettu rect:iin (muokkausdialogin 1. kohta)
// Aluemuokkausfiltterin laskenta-alue m‰‰r‰t‰‰n n‰ill‰
// nelj‰ll‰ 's‰‰timell‰'. rect:in Top ja Left tarkoittavat alueen
// vasenta- ja alareunaa. Bottom ja Right oikeaa- ja yl‰reunaa.
// Luvut annetaan positiivisina, esim. seuraavilla luvuilla
//        1(Bottom)         alueesta tulee   .....
// 2(Left)       2(Right)       ==>          .....  eli t‰lt‰ hila-alueelta lasketaan
//        1(Top)                             .....  joku luku (esim avg), joka
//                                                  talletetaan keskimm‰iseen hilaan
	NFmiRect itsAreaFilterRangeEnd;		// lis‰ksi nyt on alkuajankohdalle ja lopulle omat rectit (muokkausdialogin 2. kohta)
	NFmiPoint itsTimeFilterLimits; // aikafiltterˆinniss‰ olevien rajojen minimi ja maksimi arvot minuuteissa
	NFmiPoint itsTimeFilterRangeStart;	// aikasarjafiltterˆinnin aloitus rajat (x=alkuaikasiirtym‰ ja y=loppuaikasiirtym‰) (muokkausdialogin 1. kohta)
	NFmiPoint itsTimeFilterRangeEnd; // (muokkausdialogin 2. kohta)
	NFmiMetTime itsActiveViewTime; // kun n‰yttˆruudukkoa klikataan hiirell‰, yksi ruuduista muuttuu aktiiviseksi, sen ruudun aika talletetaan t‰h‰n
	int itsFilteringParameterUsageState; // onko valittuna 0=aktiivinen parametri, 1=kaikkiparametrit vai 2=valitutParametrit
	NFmiParamBag itsFilteringParamBag; // t‰t‰ bagia aktivoidaan kun halutaan valita tietyt parametrit, jotka filtterˆid‰‰n

	NFmiGrid* itsClipBoardData; // k‰ytet‰‰n copy, paste toimintoihin (kopioidaan aktiivinen ja pastetaan aktiiviseen)

	bool fMouseCapturedInTimeWindow; // onko aikasarja ikkunassa hiiri seurannassa?
	bool fMouseCaptured;		// onko jollain ikkunalla hiiri kaapattu
    CtrlViewUtils::FmiEditorModifyToolMode itsModifyToolMode;
	NFmiRect itsUpdateRect; // piirr‰ t‰m‰ alue n‰ytˆst‰ uudelleen (ei ehk‰ tarvita)
	NFmiPoint itsBrushSize; // prosenteissa n‰yttˆruudun koosta
	double itsBrushValue; // jos tehd‰‰n simppeli muutos, esim pilvisyytt‰ lis‰‰ 10%, tulee t‰h‰n arvo 10
	double itsBrushSpecialParamValue; // t‰h‰n ehk‰ laitetaan esim. sateen olomuodon tapauksessa 2=r‰nt‰‰ jne.
	bool fUseMaskWithBrush;
	bool fViewBrushed; // kun t‰m‰ on true, p‰ivitet‰‰n vain aktiivinen ikkuna
	NFmiRect itsActiveViewRect; // pensselipiiron j‰lkeen p‰ivitet‰‰n vain aktiivinen ikkuna
	bool fLeftMouseButtonDown;
	bool fRightMouseButtonDown;
	bool fMiddleMouseButtonDown;
	NFmiMetTime itsLastBrushedViewTime;
	int itsLastBrushedViewRow;

	NFmiMetTime itsTimeFilterStartTime;
	NFmiMetTime itsTimeFilterEndTime;

	int itsFilterDialogUpdateStatus; // 0 = ei tarvii tehd‰ mit‰‰n, 1 = p‰ivit‰ aikakontrolli-ikkunaa
									 // 2 = p‰ivit‰ koko ruutu
	bool fUseTimeInterpolation;

	NFmiMetEditorOptionsData itsMetEditorOptionsData;
	NFmiCPManagerSet itsCPManagerSet;

};
// **********************************************************
// ************ UUSI PIMPL idiooma **************************
// **********************************************************




NFmiEditMapGeneralDataDoc::NFmiEditMapGeneralDataDoc(unsigned long thePopupMenuStartId)
:pimpl(new GeneralDocImpl(thePopupMenuStartId))
{
}

NFmiEditMapGeneralDataDoc::~NFmiEditMapGeneralDataDoc()
{
	delete pimpl;
}

void NFmiEditMapGeneralDataDoc::SetGeneralDataDocInterfaceCallbacks()
{
    ToolboxViewsInterface::GetToolboxViewsInterfaceCallBack = [&]() {return &GetToolboxViewsInterface(); };
    CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation = [&]() {return &GetCtrlViewDocumentInterface(); };
    SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation = [&]() {return &GetSmartMetDocumentInterface(); };
}

ToolboxViewsInterface& NFmiEditMapGeneralDataDoc::GetToolboxViewsInterface()
{
    static ToolboxViewsInterfaceForGeneralDataDoc toolboxViewsInterfaceForGeneralDataDoc(this);
    return toolboxViewsInterfaceForGeneralDataDoc;
}

CtrlViewDocumentInterface& NFmiEditMapGeneralDataDoc::GetCtrlViewDocumentInterface()
{
    static CtrlViewDocumentInterfaceForGeneralDataDoc ctrlViewDocumentInterfaceForGeneralDataDoc(this);
    return ctrlViewDocumentInterfaceForGeneralDataDoc;
}

SmartMetDocumentInterface& NFmiEditMapGeneralDataDoc::GetSmartMetDocumentInterface()
{
    static SmartMetDocumentInterfaceForGeneralDataDoc smartMetDocumentInterfaceForGeneralDataDoc(this);
    return smartMetDocumentInterfaceForGeneralDataDoc;
}

//--------------------------------------------------------
// Init
//--------------------------------------------------------
bool NFmiEditMapGeneralDataDoc::Init(const NFmiBasicSmartMetConfigurations &theBasicConfigurations, std::map<std::string, std::string> &mapViewsPositionMap, std::map<std::string, std::string> &otherViewsPositionPosMap)
{
    SetGeneralDataDocInterfaceCallbacks();
    bool status = pimpl->Init(theBasicConfigurations, mapViewsPositionMap, otherViewsPositionPosMap);
    CrossSectionSystem()->SetDocumentInterface(&GetCtrlViewDocumentInterface());

	return status;
}



void NFmiEditMapGeneralDataDoc::AddQueryData(NFmiQueryData* theData, const std::string& theDataFileName, const std::string& theDataFilePattern,
										int theType, const std::string& theNotificationStr, bool loadFromFileState)
{
	pimpl->AddQueryData(theData, theDataFileName, theDataFilePattern, NFmiInfoData::Type(theType), theNotificationStr, loadFromFileState);
}

boost::shared_ptr<NFmiDrawParam> NFmiEditMapGeneralDataDoc::ActiveDrawParam(unsigned int theDescTopIndex, int theRowIndex)
{
	return pimpl->ActiveDrawParam(theDescTopIndex, theRowIndex);
}

boost::shared_ptr<NFmiDrawParam> NFmiEditMapGeneralDataDoc::DefaultEditedDrawParam(void)
{
	return pimpl->DefaultEditedDrawParam();
}
bool NFmiEditMapGeneralDataDoc::CreateMaskSelectionPopup(void)
{
	return pimpl->CreateMaskSelectionPopup();
}

bool NFmiEditMapGeneralDataDoc::CreateTimeSerialDialogPopup(int index)
{
	return pimpl->CreateTimeSerialDialogPopup(index);
}

bool NFmiEditMapGeneralDataDoc::CreateTimeSerialDialogOnViewPopup(int index)
{
	return pimpl->CreateTimeSerialDialogOnViewPopup(index);
}

bool NFmiEditMapGeneralDataDoc::SelectAllLocations(bool newState)
{
// valitsee kaikki editoitavan parametrin paikat (PITƒISI VALITA VAIN NE, JOTKA OVAT AKTIIVISIA!!)
	boost::shared_ptr<NFmiFastQueryInfo> editedInfo = EditedSmartInfo();
	if(editedInfo)
	{
		dynamic_cast<NFmiSmartInfo*>(editedInfo.get())->MaskAllLocations(newState, NFmiMetEditorTypes::kFmiSelectionMask);
		return true;
	}
	return false;
}

bool NFmiEditMapGeneralDataDoc::MakeAndStoreFileDialogDirectoryMemory(const std::string& thePathAndFileName)
{
	return pimpl->MakeAndStoreFileDialogDirectoryMemory(thePathAndFileName);
}

bool NFmiEditMapGeneralDataDoc::LoadData(bool fRemoveThundersOnLoad)
{
	return FmiModifyEditdData::LoadData(GenDocDataAdapter(), fRemoveThundersOnLoad, pimpl->UseMultithreaddingWithModifyingFunctions());
}

const NFmiMetTime& NFmiEditMapGeneralDataDoc::CurrentTime(unsigned int theDescTopIndex)
{
	return pimpl->CurrentTime(theDescTopIndex);
}

bool NFmiEditMapGeneralDataDoc::CurrentTime(unsigned int theDescTopIndex, const NFmiMetTime& newCurrentTime, bool fStayInsideAnimationTimes)
{
	return pimpl->CurrentTime(theDescTopIndex, newCurrentTime, fStayInsideAnimationTimes);
}

bool NFmiEditMapGeneralDataDoc::SetDataToNextTime(unsigned int theDescTopIndex, bool fStayInsideAnimationTimes)
{
	return pimpl->SetDataToNextTime(theDescTopIndex, fStayInsideAnimationTimes);
}
bool NFmiEditMapGeneralDataDoc::SetDataToPreviousTime(unsigned int theDescTopIndex, bool fStayInsideAnimationTimes)
{
	return pimpl->SetDataToPreviousTime(theDescTopIndex, fStayInsideAnimationTimes);
}

bool NFmiEditMapGeneralDataDoc::StoreOptionsData(void)
{
	return pimpl->StoreOptionsData();
}

bool NFmiEditMapGeneralDataDoc::SelectNewParamForSelectionTool(int theParamId)
{
	return pimpl->SelectNewParamForSelectionTool(theParamId);
}

NFmiParamBag& NFmiEditMapGeneralDataDoc::AllStaticParams(void)
{
	return pimpl->AllStaticParams();
}

void NFmiEditMapGeneralDataDoc::DoAutoZoom(unsigned int theDescTopIndex)
{
	pimpl->DoAutoZoom(theDescTopIndex);
}

NFmiDrawParamList* NFmiEditMapGeneralDataDoc::DrawParamList(int theDescTopIndex, int theIndex)
{
	return pimpl->DrawParamList(theDescTopIndex, theIndex);
}

NFmiDrawParamList* NFmiEditMapGeneralDataDoc::DrawParamListWithRealRowNumber(unsigned int theDescTopIndex, int theRealRowIndex)
{
    return pimpl->DrawParamListWithRealRowNumber(theDescTopIndex, theRealRowIndex);
}

bool NFmiEditMapGeneralDataDoc::CreateParamSelectionPopup(unsigned int theDescTopIndex, int theRowIndex)
{
	pimpl->itsCurrentViewRowIndex = theRowIndex;
	return pimpl->CreateParamSelectionPopup(theDescTopIndex);
}

bool NFmiEditMapGeneralDataDoc::CreateViewParamsPopup(unsigned int theDescTopIndex, int theRowIndex, int index)
{
	return pimpl->CreateViewParamsPopup(theDescTopIndex, theRowIndex, index);
}

bool NFmiEditMapGeneralDataDoc::CreateMaskSelectionPopup(int theRowIndex)
{
	pimpl->itsCurrentViewRowIndex = theRowIndex;
	return CreateMaskSelectionPopup();
}

bool NFmiEditMapGeneralDataDoc::CreateMaskParamsPopup(int theRowIndex, int index)
{
	return pimpl->CreateMaskParamsPopup(theRowIndex, index);
}

bool NFmiEditMapGeneralDataDoc::MakePopUpCommandUsingRowIndex(unsigned short theCommandID)
{
	return pimpl->MakePopUpCommandUsingRowIndex(theCommandID);
}

NFmiPoint NFmiEditMapGeneralDataDoc::ActualMapBitmapSizeInPixels(unsigned int theDescTopIndex)
{
	return pimpl->ActualMapBitmapSizeInPixels(theDescTopIndex);
}


boost::shared_ptr<NFmiFastQueryInfo> NFmiEditMapGeneralDataDoc::EditedSmartInfo(void)
{return pimpl->EditedInfo();}

bool NFmiEditMapGeneralDataDoc::MakeGridFile(const NFmiString& theFileName)
{
	return pimpl->MakeGridFile(theFileName);
}

void NFmiEditMapGeneralDataDoc::UpdateEditedDataCopy(void)
{
	pimpl->UpdateEditedDataCopy();
}

const NFmiRect& NFmiEditMapGeneralDataDoc::AreaFilterRange(int index)
{
	return pimpl->AreaFilterRange(index);
}

void NFmiEditMapGeneralDataDoc::AreaFilterRange(int index, const NFmiRect& theRect)
{
	pimpl->AreaFilterRange(index, theRect);
}

const NFmiPoint& NFmiEditMapGeneralDataDoc::TimeFilterRange(int index)
{
	return pimpl->TimeFilterRange(index);
}
void NFmiEditMapGeneralDataDoc::TimeFilterRange(int index, const NFmiPoint& thePoint, bool fRoundToNearestHour)
{
	pimpl->TimeFilterRange(index, thePoint, fRoundToNearestHour);
}

void NFmiEditMapGeneralDataDoc::ResetFilters(void)
{
	pimpl->ResetFilters();
}

bool NFmiEditMapGeneralDataDoc::DoAreaFiltering(bool fPasteClipBoardData)
{
	return pimpl->DoAreaFiltering(fPasteClipBoardData);
}


bool NFmiEditMapGeneralDataDoc::DoTimeFiltering(void)
{
	return pimpl->DoTimeFiltering();
}

bool NFmiEditMapGeneralDataDoc::CopyActiveDataToClipBoard(void)
{
	return pimpl->CreatePasteData();
}

bool NFmiEditMapGeneralDataDoc::PasteClipBoardToActiveData(void)
{
	DoAreaFiltering(true);
	return true;
}

bool NFmiEditMapGeneralDataDoc::HasActiveViewChanged(void)
{
	return pimpl->HasActiveViewChanged();
}

void NFmiEditMapGeneralDataDoc::SetTimeFilterStartTime(const NFmiMetTime& theTime)
{
	pimpl->SetTimeFilterStartTime(theTime);
}
void NFmiEditMapGeneralDataDoc::SetTimeFilterEndTime(const NFmiMetTime& theTime)
{
	pimpl->SetTimeFilterEndTime(theTime);
}

void NFmiEditMapGeneralDataDoc::SetActiveParamMissingValues(double theValue)
{
	pimpl->SetActiveParamMissingValues(theValue);
}

void NFmiEditMapGeneralDataDoc::ChangeMapType(unsigned int theDescTopIndex, bool fForward)
{
	pimpl->ChangeMapType(theDescTopIndex, fForward);
}

bool NFmiEditMapGeneralDataDoc::MakeDataValiditation(void)
{
	return pimpl->MakeDataValiditation();
}


bool NFmiEditMapGeneralDataDoc::CheckAndValidateAfterModifications(NFmiMetEditorTypes::FmiUsedSmartMetTool theModifyingTool, bool fMakeDataSnapshotAction, unsigned int theLocationMask, FmiParameterName theParam, bool fPasteAction)
{
	return pimpl->CheckAndValidateAfterModifications(theModifyingTool, fMakeDataSnapshotAction, theLocationMask, theParam, fPasteAction);
}

bool NFmiEditMapGeneralDataDoc::ScrollViewRow(unsigned int theDescTopIndex, int theCount)
{
	return pimpl->ScrollViewRow(theDescTopIndex, theCount);
}

bool NFmiEditMapGeneralDataDoc::DoCombineModelAndKlapse(void)
{
	return pimpl->DoCombineModelAndKlapse();
}

CtrlViewUtils::FmiEditorModifyToolMode NFmiEditMapGeneralDataDoc::ModifyToolMode(void)
{
	return pimpl->itsModifyToolMode;
}

void NFmiEditMapGeneralDataDoc::ModifyToolMode(CtrlViewUtils::FmiEditorModifyToolMode newState)
{
	pimpl->itsModifyToolMode = newState;
}

NFmiMetEditorModeDataWCTR* NFmiEditMapGeneralDataDoc::EditorModeDataWCTR(void)
{
	return pimpl->EditorModeDataWCTR();
}

NFmiEditMapDataListHandler* NFmiEditMapGeneralDataDoc::DataLists(void)
{
	return pimpl->DataLists();
}

NFmiDrawParamList* NFmiEditMapGeneralDataDoc::TimeSerialViewDrawParamList(void)
{
	return pimpl->itsTimeSerialViewDrawParamList;
}

FmiTimeEditMode NFmiEditMapGeneralDataDoc::TimeEditMode(void)const
{
	return pimpl->itsTimeEditingMode;
}

void NFmiEditMapGeneralDataDoc::TimeEditMode(const int& newMode)
{
	pimpl->itsTimeEditingMode = (FmiTimeEditMode) newMode;
}

bool NFmiEditMapGeneralDataDoc::OpenPopupMenu(void)
{
	return pimpl->fOpenPopup;
}

void NFmiEditMapGeneralDataDoc::OpenPopupMenu(bool newState)
{
	pimpl->fOpenPopup = newState;
}

NFmiMenuItemList* NFmiEditMapGeneralDataDoc::PopupMenu(void)
{
	return pimpl->itsPopupMenu;
}


NFmiLocationSelectionTool* NFmiEditMapGeneralDataDoc::LocationSelectionTool2(void)
{
	return pimpl->itsLocationSelectionTool2;
}

boost::shared_ptr<NFmiAreaMaskList> NFmiEditMapGeneralDataDoc::ParamMaskListMT(void)
{
	return pimpl->ParamMaskListMT();
}

const std::string& NFmiEditMapGeneralDataDoc::FileDialogDirectoryMemory(void)
{
	return pimpl->itsFileDialogDirectoryMemory;
}
bool NFmiEditMapGeneralDataDoc::IsMasksUsedInTimeSerialViews(void)
{
	return pimpl->fUseMasksInTimeSerialViews;
}

void NFmiEditMapGeneralDataDoc::UseMasksInTimeSerialViews(bool newValue)
{
	pimpl->fUseMasksInTimeSerialViews = newValue;
}
int NFmiEditMapGeneralDataDoc::ToolTipRowIndex(void)
{
	return pimpl->itsToolTipRowIndex;
}

void NFmiEditMapGeneralDataDoc::ToolTipRowIndex(int newIndex)
{
	pimpl->itsToolTipRowIndex = newIndex;
}

int NFmiEditMapGeneralDataDoc::ToolTipColumnIndex(void)
{
	return pimpl->itsToolTipColumnIndex;
}
void NFmiEditMapGeneralDataDoc::ToolTipColumnIndex(int newIndex)
{
	pimpl->itsToolTipColumnIndex = newIndex;
}

const NFmiPoint& NFmiEditMapGeneralDataDoc::ToolTipLatLonPoint(void) const {return pimpl->itsToolTipLatLonPoint;};
void NFmiEditMapGeneralDataDoc::ToolTipLatLonPoint(const NFmiPoint& theLatLon){pimpl->itsToolTipLatLonPoint = theLatLon;};
void NFmiEditMapGeneralDataDoc::ToolTipTime(const NFmiMetTime& theTime){pimpl->itsToolTipTime = theTime;};
const NFmiMetTime& NFmiEditMapGeneralDataDoc::ToolTipTime(void){return pimpl->itsToolTipTime;};
int NFmiEditMapGeneralDataDoc::FilterFunction(void){return pimpl->itsFilterFunction;};
void NFmiEditMapGeneralDataDoc::FilterFunction(int newFilter){pimpl->itsFilterFunction = newFilter;};
const NFmiMetTime& NFmiEditMapGeneralDataDoc::ActiveViewTime(void){return pimpl->itsActiveViewTime;};
void NFmiEditMapGeneralDataDoc::ActiveViewTime(const NFmiMetTime& theTime){pimpl->itsActiveViewTime = theTime;};
int NFmiEditMapGeneralDataDoc::ActiveViewRow(unsigned int theDescTopIndex)
{
	return pimpl->ActiveViewRow(theDescTopIndex);
}
void NFmiEditMapGeneralDataDoc::ActiveViewRow(unsigned int theDescTopIndex, int theActiveRowIndex)
{
	pimpl->ActiveViewRow(theDescTopIndex, theActiveRowIndex);
}
bool NFmiEditMapGeneralDataDoc::UseMasksWithFilterTool(void){return pimpl->fUseMasksWithFilterTool;};
void NFmiEditMapGeneralDataDoc::UseMasksWithFilterTool(bool newStatus){pimpl->fUseMasksWithFilterTool = newStatus;};
const NFmiRect& NFmiEditMapGeneralDataDoc::AreaFilterRangeLimits(void){return pimpl->itsAreaFilterRangeLimits;};
void NFmiEditMapGeneralDataDoc::AreaFilterRangeLimits(const NFmiRect& theRect){pimpl->itsAreaFilterRangeLimits = theRect;};
const NFmiPoint& NFmiEditMapGeneralDataDoc::TimeFilterLimits(void){return pimpl->itsTimeFilterLimits;};
void NFmiEditMapGeneralDataDoc::TimeFilterLimits(const NFmiPoint& thePoint){pimpl->itsTimeFilterLimits = thePoint;};
int NFmiEditMapGeneralDataDoc::FilteringParameterUsageState(void){return pimpl->itsFilteringParameterUsageState;};
void NFmiEditMapGeneralDataDoc::FilteringParameterUsageState(int newState){pimpl->itsFilteringParameterUsageState = newState;};
NFmiParamBag& NFmiEditMapGeneralDataDoc::FilteringParamBag(void){return pimpl->itsFilteringParamBag;};
void NFmiEditMapGeneralDataDoc::FilteringParamBag(const NFmiParamBag& newBag){pimpl->itsFilteringParamBag = newBag;};
void NFmiEditMapGeneralDataDoc::SetTestFilterMask(int newValue){pimpl->itsTestFilterUsedMask = newValue;};
int NFmiEditMapGeneralDataDoc::GetTestFilterMask(void){return pimpl->itsTestFilterUsedMask;};
NFmiGrid* NFmiEditMapGeneralDataDoc::ClipBoardData(void){return pimpl->itsClipBoardData;};
bool NFmiEditMapGeneralDataDoc::MouseCapturedInTimeWindow(void){return pimpl->fMouseCapturedInTimeWindow;};
void NFmiEditMapGeneralDataDoc::MouseCapturedInTimeWindow(bool newValue){pimpl->fMouseCapturedInTimeWindow = newValue;};
bool NFmiEditMapGeneralDataDoc::MouseCaptured(void){return pimpl->fMouseCaptured;};
void NFmiEditMapGeneralDataDoc::MouseCaptured(bool newState){pimpl->fMouseCaptured = newState;};
const NFmiRect& NFmiEditMapGeneralDataDoc::UpdateRect(void){return pimpl->itsUpdateRect;};
void NFmiEditMapGeneralDataDoc::UpdateRect(const NFmiRect& theRect){pimpl->itsUpdateRect = theRect;};
const NFmiPoint& NFmiEditMapGeneralDataDoc::BrushSize(void){return pimpl->itsBrushSize;};
void NFmiEditMapGeneralDataDoc::BrushSize(const NFmiPoint& theSize){pimpl->itsBrushSize = theSize;};
double NFmiEditMapGeneralDataDoc::BrushValue(void){return pimpl->itsBrushValue;};
void NFmiEditMapGeneralDataDoc::BrushValue(double newValue){pimpl->itsBrushValue = newValue;};
double NFmiEditMapGeneralDataDoc::BrushSpecialParamValue(void){return pimpl->itsBrushSpecialParamValue;};
void NFmiEditMapGeneralDataDoc::BrushSpecialParamValue(double newValue){pimpl->itsBrushSpecialParamValue = newValue;};
bool NFmiEditMapGeneralDataDoc::UseMaskWithBrush(void){return pimpl->fUseMaskWithBrush;};
void NFmiEditMapGeneralDataDoc::UseMaskWithBrush(bool newState){pimpl->fUseMaskWithBrush = newState;};
bool NFmiEditMapGeneralDataDoc::ViewBrushed(void){return pimpl->fViewBrushed;};
void NFmiEditMapGeneralDataDoc::ViewBrushed(bool newState){pimpl->fViewBrushed = newState;};
const NFmiRect& NFmiEditMapGeneralDataDoc::ActiveViewRect(void){return pimpl->itsActiveViewRect;};
void NFmiEditMapGeneralDataDoc::ActiveViewRect(const NFmiRect& theRect){pimpl->itsActiveViewRect = theRect;};
bool NFmiEditMapGeneralDataDoc::LeftMouseButtonDown(void)
{return pimpl->LeftMouseButtonDown();};
void NFmiEditMapGeneralDataDoc::LeftMouseButtonDown(bool newState)
{pimpl->LeftMouseButtonDown(newState);};
bool NFmiEditMapGeneralDataDoc::RightMouseButtonDown(void)
{return pimpl->RightMouseButtonDown();};
void NFmiEditMapGeneralDataDoc::RightMouseButtonDown(bool newState)
{pimpl->RightMouseButtonDown(newState);};
bool NFmiEditMapGeneralDataDoc::MiddleMouseButtonDown(void)
{return pimpl->MiddleMouseButtonDown();}
void NFmiEditMapGeneralDataDoc::MiddleMouseButtonDown(bool newState)
{pimpl->MiddleMouseButtonDown(newState);}
const NFmiMetTime& NFmiEditMapGeneralDataDoc::LastBrushedViewTime(void){return pimpl->itsLastBrushedViewTime;};
void NFmiEditMapGeneralDataDoc::LastBrushedViewTime(const NFmiMetTime& newTime){pimpl->itsLastBrushedViewTime = newTime;};
int NFmiEditMapGeneralDataDoc::LastBrushedViewRow(void){return pimpl->itsLastBrushedViewRow;};
void NFmiEditMapGeneralDataDoc::LastBrushedViewRow(int newRow){pimpl->itsLastBrushedViewRow = newRow;};
const NFmiMetTime& NFmiEditMapGeneralDataDoc::TimeFilterStartTime(void){return pimpl->itsTimeFilterStartTime;};
const NFmiMetTime& NFmiEditMapGeneralDataDoc::TimeFilterEndTime(void){return pimpl->itsTimeFilterEndTime;};
int NFmiEditMapGeneralDataDoc::FilterDialogUpdateStatus(void){return pimpl->FilterDialogUpdateStatus();};
void NFmiEditMapGeneralDataDoc::FilterDialogUpdateStatus(int newState){pimpl->FilterDialogUpdateStatus(newState);};
bool NFmiEditMapGeneralDataDoc::UseTimeInterpolation(void){return pimpl->fUseTimeInterpolation;};
void NFmiEditMapGeneralDataDoc::UseTimeInterpolation(bool newState){pimpl->fUseTimeInterpolation = newState;};
bool NFmiEditMapGeneralDataDoc::DoTimeSeriesValuesModifying(boost::shared_ptr<NFmiDrawParam> &theModifiedDrawParam, int theUsedMask, NFmiTimeDescriptor& theTimeDescriptor, checkedVector<double> &theModificationFactorCurvePoints, NFmiMetEditorTypes::FmiUsedSmartMetTool theEditorTool, bool fUseSetForDiscreteData, int theUnchangedValue)
{
	return pimpl->DoTimeSeriesValuesModifying(theModifiedDrawParam, NFmiMetEditorTypes::Mask(theUsedMask), theTimeDescriptor, theModificationFactorCurvePoints, theEditorTool, fUseSetForDiscreteData, theUnchangedValue);
}


NFmiMetEditorOptionsData& NFmiEditMapGeneralDataDoc::MetEditorOptionsData(void)
{
	return pimpl->MetEditorOptionsData();
}

void NFmiEditMapGeneralDataDoc::DoAutoSaveData(void)
{
	pimpl->DoAutoSaveData();
}

boost::shared_ptr<NFmiEditorControlPointManager> NFmiEditMapGeneralDataDoc::CPManager(bool getOldSchoolCPManager)
{
	return pimpl->CPManager(getOldSchoolCPManager);
}

bool NFmiEditMapGeneralDataDoc::CreateCPPopup()
{
	return pimpl->CreateCPPopup();
}

bool NFmiEditMapGeneralDataDoc::Printing(void)
{return pimpl->Printing();}

void NFmiEditMapGeneralDataDoc::Printing(bool newStatus)
{pimpl->Printing(newStatus);}

// nelj‰ tilaa: 0 = n‰yt‰ aikakontrolliikkuna+teksti 1=vain aik.kont.ikkuna, 2=‰l‰ n‰yt‰ kumpaakaan ja 3= n‰yt‰ vain teksti
int NFmiEditMapGeneralDataDoc::ToggleShowTimeOnMapMode(unsigned int theDescTopIndex)
{return pimpl->ToggleShowTimeOnMapMode(theDescTopIndex);}

// aikasarja ikkunat asetetaan samalla likaiseksi kuin karttan‰yttˆkin (ainakin toistaiseksi)
bool NFmiEditMapGeneralDataDoc::TimeSerialViewDirty(void)
{return pimpl->TimeSerialViewDirty();}
void NFmiEditMapGeneralDataDoc::TimeSerialViewDirty(bool newValue)
{pimpl->TimeSerialViewDirty(newValue);}

bool NFmiEditMapGeneralDataDoc::IsToolMasterAvailable() const
{return pimpl->IsToolMasterAvailable();}
void NFmiEditMapGeneralDataDoc::ToolMasterAvailable(bool newValue)
{pimpl->ToolMasterAvailable(newValue);}

void NFmiEditMapGeneralDataDoc::UpdateModifiedDrawParamMarko(boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{ // HUOM!! t‰t‰ kutsutaan vain aikasarja ikkunasta ulkopuolelta, joten voin laittaa aikasarjaa kuvaavan
	// rivi numeron (99) t‰h‰n, jos joku tulee k‰ytt‰m‰‰n t‰t‰ jostain muualta ja tarkoittaa
	// esim. k‰rtta n‰ytˆn drawparameja t‰ll‰, t‰ytyy metodit tehd‰ uusiksi.
	// Lis‰ksi jouduin laittamaan 0 desctop indeksin, vaikka aikasarjaikkunalla ei ole desctopindeksi‰.
	pimpl->UpdateModifiedDrawParamMarko(0, theDrawParam, gActiveViewRowIndexForTimeSerialView);
}

// tallettaa mm. CP pisteet, muutosk‰yr‰t jne.
void NFmiEditMapGeneralDataDoc::StoreSupplementaryData(void)
{pimpl->StoreSupplementaryData();}

// t‰m‰ tiet‰‰ miten lat-lon-viivoja piirret‰‰n karttojen ylle
NFmiProjectionCurvatureInfo* NFmiEditMapGeneralDataDoc::ProjectionCurvatureInfo(void)
{return pimpl->ProjectionCurvatureInfo();}

void NFmiEditMapGeneralDataDoc::LogMessage(const std::string& theMessage, CatLog::Severity severity, CatLog::Category category, bool flushLogger)
{ pimpl->LogMessage(theMessage, severity, category, flushLogger); }

void NFmiEditMapGeneralDataDoc::LogAndWarnUser(const std::string &theMessageStr, const std::string &theDialogTitleStr, CatLog::Severity severity, CatLog::Category category, bool justLog, bool addAbortOption, bool flushLogger)
{ pimpl->LogAndWarnUser(theMessageStr, theDialogTitleStr, severity, category, justLog, addAbortOption, flushLogger); }

void NFmiEditMapGeneralDataDoc::SetDataLoadingProducerIndexVector(const checkedVector<int>& theIndexVector)
{
	pimpl->SetDataLoadingProducerIndexVector(theIndexVector);
}

void NFmiEditMapGeneralDataDoc::RemoveAllParamsFromMapViewRow(unsigned int theDescTopIndex, int theRowIndex)
{
	pimpl->RemoveAllParamsFromMapViewRow(theDescTopIndex, theRowIndex);
}

bool NFmiEditMapGeneralDataDoc::StoreDataToDataBase(const std::string &theForecasterId)
{
	return pimpl->StoreDataToDataBase(theForecasterId);
}

bool NFmiEditMapGeneralDataDoc::StoreAllCPDataToFiles(void)
{
	return pimpl->StoreAllCPDataToFiles();
}

int NFmiEditMapGeneralDataDoc::TimeEditSmootherValue(void)
{
	return pimpl->TimeEditSmootherValue();
}
void NFmiEditMapGeneralDataDoc::TimeEditSmootherValue(int newValue)
{
	pimpl->TimeEditSmootherValue(newValue);
}
int NFmiEditMapGeneralDataDoc::TimeEditSmootherMaxValue(void)
{
	return pimpl->TimeEditSmootherMaxValue();
}
void NFmiEditMapGeneralDataDoc::TimeEditSmootherMaxValue(int newValue)
{
	pimpl->TimeEditSmootherMaxValue(newValue);
}
int NFmiEditMapGeneralDataDoc::BrushToolLimitSetting(void)
{
	return pimpl->BrushToolLimitSetting();
}
void NFmiEditMapGeneralDataDoc::BrushToolLimitSetting(int newValue)
{
	pimpl->BrushToolLimitSetting(newValue);
}
float NFmiEditMapGeneralDataDoc::BrushToolLimitSettingValue(void)
{
	return pimpl->BrushToolLimitSettingValue();
}
void NFmiEditMapGeneralDataDoc::BrushToolLimitSettingValue(float newValue)
{
	pimpl->BrushToolLimitSettingValue(newValue);
}
bool NFmiEditMapGeneralDataDoc::ShowHelperData1InTimeSerialView()
{
    return pimpl->ShowHelperData1InTimeSerialView();
}
bool NFmiEditMapGeneralDataDoc::ShowHelperData2InTimeSerialView()
{
    return pimpl->ShowHelperData2InTimeSerialView();
}
bool NFmiEditMapGeneralDataDoc::ShowHelperData3InTimeSerialView()
{
    return pimpl->ShowHelperData3InTimeSerialView();
}
bool NFmiEditMapGeneralDataDoc::ShowHelperData4InTimeSerialView()
{
    return pimpl->ShowHelperData4InTimeSerialView();
}

NFmiInfoOrganizer* NFmiEditMapGeneralDataDoc::InfoOrganizer(void)
{
	return pimpl->InfoOrganizer();
}

bool NFmiEditMapGeneralDataDoc::IsOperationalModeOn(void)
{
	return pimpl->IsOperationalModeOn();
}

const string& NFmiEditMapGeneralDataDoc::EditorVersionStr(void)
{
	return pimpl->EditorVersionStr();
}

bool NFmiEditMapGeneralDataDoc::ExecuteCommand(const NFmiMenuItem &theMenuItem, int theViewIndex, int theViewTypeId)
{
	return pimpl->ExecuteCommand(theMenuItem, theViewIndex, theViewTypeId);
}

bool NFmiEditMapGeneralDataDoc::DoSmartToolEditing(const std::string &theSmartToolText, const std::string &theRelativePathMacroName, bool fSelectedLocationsOnly)
{
	return pimpl->DoSmartToolEditing(theSmartToolText, theRelativePathMacroName, fSelectedLocationsOnly);
}

std::string& NFmiEditMapGeneralDataDoc::SmartToolEditingErrorText(void)
{
	return pimpl->SmartToolEditingErrorText();
}

boost::shared_ptr<NFmiDrawParam> NFmiEditMapGeneralDataDoc::GetDrawDifferenceDrawParam(void)
{
	return pimpl->GetDrawDifferenceDrawParam();
}

boost::shared_ptr<NFmiDrawParam> NFmiEditMapGeneralDataDoc::GetSelectedGridPointDrawParam(void)
{
	return pimpl->GetSelectedGridPointDrawParam();
}

NFmiSmartToolInfo* NFmiEditMapGeneralDataDoc::SmartToolInfo(void)
{
	return pimpl->SmartToolInfo();
}

const NFmiTimeDescriptor& NFmiEditMapGeneralDataDoc::EditedDataTimeDescriptor(void)
{
	return pimpl->EditedDataTimeDescriptor();
}

const NFmiTimeBag& NFmiEditMapGeneralDataDoc::EditedDataTimeBag(void)
{
	return pimpl->EditedDataTimeBag();
}

bool NFmiEditMapGeneralDataDoc::LoadViewMacro(const std::string &theName)
{
	return pimpl->LoadViewMacro(theName);
}
void NFmiEditMapGeneralDataDoc::StoreViewMacro(const std::string &theName, const std::string &theDescription)
{
	pimpl->StoreViewMacro(theName, theDescription);
}

void NFmiEditMapGeneralDataDoc::RefreshViewMacroList(void)
{
	pimpl->RefreshViewMacroList();
}

bool NFmiEditMapGeneralDataDoc::RemoveViewMacro(const string& theMacroName)
{
	return pimpl->RemoveViewMacro(theMacroName);
}

CtrlViewUtils::FmiSmartMetEditingMode NFmiEditMapGeneralDataDoc::SmartMetEditingMode(void)
{
	return pimpl->SmartMetEditingMode();
}

void NFmiEditMapGeneralDataDoc::SmartMetEditingMode(CtrlViewUtils::FmiSmartMetEditingMode newValue, bool modifySettings)
{
	pimpl->SmartMetEditingMode(newValue, modifySettings);
}

bool NFmiEditMapGeneralDataDoc::UseDoAtSendCommandString(void)
{
	return pimpl->UseDoAtSendCommandString();
}

void NFmiEditMapGeneralDataDoc::UseDoAtSendCommandString(bool newValue)
{
	pimpl->UseDoAtSendCommandString(newValue);
}

std::string NFmiEditMapGeneralDataDoc::DoAtSendCommandString(void)
{
	return pimpl->DoAtSendCommandString();
}

void NFmiEditMapGeneralDataDoc::DoAtSendCommandString(const std::string &newValue)
{
	pimpl->DoAtSendCommandString(newValue);
}

void NFmiEditMapGeneralDataDoc::MapViewDirty(unsigned int theDescTopIndex, bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool clearMacroParamDataCache, bool clearEditedDataDependentCaches, bool updateMapViewDrawingLayers)
{
    pimpl->MapViewDirty(theDescTopIndex, makeNewBackgroundBitmap, clearMapViewBitmapCacheRows, redrawMapView, clearMacroParamDataCache, clearEditedDataDependentCaches, updateMapViewDrawingLayers);
}

void NFmiEditMapGeneralDataDoc::ForceStationViewRowUpdate(unsigned int theDescTopIndex, unsigned int theRealRowIndex)
{
    pimpl->ForceStationViewRowUpdate(theDescTopIndex, theRealRowIndex);
}

void NFmiEditMapGeneralDataDoc::SetMapArea(unsigned int theDescTopIndex, const boost::shared_ptr<NFmiArea> &newArea)
{
	pimpl->SetMapArea(theDescTopIndex, newArea);
}

void NFmiEditMapGeneralDataDoc::SnapShotData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiDataIdent &theDataIdent, const string &theModificationText
				 , const NFmiMetTime &theStartTime, const NFmiMetTime &theEndTime)
{
	pimpl->SnapShotData(theInfo, theDataIdent, theModificationText, theStartTime, theEndTime);
}

bool NFmiEditMapGeneralDataDoc::ChangeToEditorsWorkingDirectory(void)
{
	return pimpl->ChangeToEditorsWorkingDirectory();
}
bool NFmiEditMapGeneralDataDoc::SaveCurrentPath(void)
{
	return pimpl->SaveCurrentPath();
}
bool NFmiEditMapGeneralDataDoc::LoadSavedPath(void)
{
	return pimpl->LoadSavedPath();
}

void NFmiEditMapGeneralDataDoc::SetCurrentSmartToolMacro(const std::string& theMacroText)
{
	return pimpl->SetCurrentSmartToolMacro(theMacroText);
}
const std::string& NFmiEditMapGeneralDataDoc::GetCurrentSmartToolMacro(void)
{
	return pimpl->GetCurrentSmartToolMacro();
}

NFmiMacroParamSystem& NFmiEditMapGeneralDataDoc::MacroParamSystem(void)
{
	return pimpl->MacroParamSystem();
}

void NFmiEditMapGeneralDataDoc::AddMacroParamToView(unsigned int theDescTopIndex, int theViewRow, const std::string &theName) // lis‰‰ halutun nimisen macroParamin halutun karttan‰ytˆn riville (1-5)
{
	pimpl->AddMacroParamToView(theDescTopIndex, theViewRow, theName);
}

void NFmiEditMapGeneralDataDoc::RemoveMacroParam(const std::string &theName)
{
	pimpl->RemoveMacroParam(theName);
}

void NFmiEditMapGeneralDataDoc::SetLatestMacroParamErrorText(const std::string& theErrorText)
{
	pimpl->SetLatestMacroParamErrorText(theErrorText);
}
const std::string& NFmiEditMapGeneralDataDoc::GetLatestMacroParamErrorText(void)
{
	return pimpl->GetLatestMacroParamErrorText();
}

NFmiCrossSectionSystem* NFmiEditMapGeneralDataDoc::CrossSectionSystem(void)
{
	return pimpl->CrossSectionSystem();
}

NFmiHelpDataInfoSystem* NFmiEditMapGeneralDataDoc::HelpDataInfoSystem(void)
{
	return pimpl->HelpDataInfoSystem();
}

bool NFmiEditMapGeneralDataDoc::MoveActiveMapViewParamInDrawingOrderList(unsigned int theDescTopIndex, int theMapRow, bool fRaise, bool fUseCrossSectionParams)
{
	return pimpl->MoveActiveMapViewParamInDrawingOrderList(theDescTopIndex, theMapRow, fRaise, fUseCrossSectionParams);
}

bool NFmiEditMapGeneralDataDoc::ChangeActiveMapViewParam(unsigned int theDescTopIndex, int theMapRow, int theParamIndex, bool fNext, bool fUseCrossSectionParams)
{
	return pimpl->ChangeActiveMapViewParam(theDescTopIndex, theMapRow, theParamIndex, fNext, fUseCrossSectionParams);
}

NFmiSynopStationPrioritySystem* NFmiEditMapGeneralDataDoc::SynopStationPrioritySystem(void)
{
	return pimpl->SynopStationPrioritySystem();
}

NFmiSynopPlotSettings* NFmiEditMapGeneralDataDoc::SynopPlotSettings(void)
{
	return pimpl->SynopPlotSettings();
}

bool NFmiEditMapGeneralDataDoc::MapMouseDragPanMode(void)
{
	return pimpl->MapMouseDragPanMode();
}
void NFmiEditMapGeneralDataDoc::MapMouseDragPanMode(bool newState)
{
	pimpl->MapMouseDragPanMode(newState);
}

bool NFmiEditMapGeneralDataDoc::MakeMacroPathConfigurations(void)
{
	return pimpl->MakeMacroPathConfigurations();
}

void NFmiEditMapGeneralDataDoc::SetApplicationTitle(const std::string &theTitle)
{
	pimpl->SetApplicationTitle(theTitle);
}

std::string NFmiEditMapGeneralDataDoc::GetApplicationTitle(void)
{
	return pimpl->GetApplicationTitle();
}

NFmiObsComparisonInfo& NFmiEditMapGeneralDataDoc::ObsComparisonInfo(void)
{
	return pimpl->ObsComparisonInfo();
}

NFmiDrawParamList* NFmiEditMapGeneralDataDoc::CrossSectionViewDrawParamList(int theRowIndex)
{
	return pimpl->CrossSectionViewDrawParamList(theRowIndex);
}

bool NFmiEditMapGeneralDataDoc::CreateCrossSectionViewPopup(int theRowIndex)
{
	return pimpl->CreateCrossSectionViewPopup(theRowIndex);
}

NFmiMTATempSystem& NFmiEditMapGeneralDataDoc::GetMTATempSystem(void)
{
	return pimpl->GetMTATempSystem();
}

bool NFmiEditMapGeneralDataDoc::TimeSerialDataViewOn(void)
{
	return pimpl->TimeSerialDataViewOn();
}

void NFmiEditMapGeneralDataDoc::TimeSerialDataViewOn(bool newValue)
{
	pimpl->TimeSerialDataViewOn(newValue);
}

bool NFmiEditMapGeneralDataDoc::SynopDataGridViewOn(void)
{
	return pimpl->SynopDataGridViewOn();
}

void NFmiEditMapGeneralDataDoc::SynopDataGridViewOn(bool newState)
{
	pimpl->SynopDataGridViewOn(newState);
}

const NFmiPoint& NFmiEditMapGeneralDataDoc::GetSynopHighlightLatlon(void)
{
	return pimpl->GetSynopHighlightLatlon();
}
bool NFmiEditMapGeneralDataDoc::ShowSynopHighlight(void)
{
	return pimpl->ShowSynopHighlight();
}
void NFmiEditMapGeneralDataDoc::SetHighlightedSynopStation(const NFmiPoint &theLatlon, int theWmoId, bool fShowHighlight)
{
	pimpl->SetHighlightedSynopStation(theLatlon, theWmoId, fShowHighlight);
}
void NFmiEditMapGeneralDataDoc::MouseOnMapView(bool newState)
{
	pimpl->MouseOnMapView(newState);
}
bool NFmiEditMapGeneralDataDoc::MouseOnMapView(void)
{
	return pimpl->MouseOnMapView();
}
void NFmiEditMapGeneralDataDoc::LastSelectedSynopWmoId(int theWmoId)
{
	pimpl->LastSelectedSynopWmoId(theWmoId);
}
int NFmiEditMapGeneralDataDoc::LastSelectedSynopWmoId(void)
{
	return pimpl->LastSelectedSynopWmoId();
}
bool NFmiEditMapGeneralDataDoc::ShowMouseHelpCursorsOnMap(void)
{
	return pimpl->ShowMouseHelpCursorsOnMap();
}
void NFmiEditMapGeneralDataDoc::ShowMouseHelpCursorsOnMap(bool newState)
{
	pimpl->ShowMouseHelpCursorsOnMap(newState);
}

void NFmiEditMapGeneralDataDoc::MakeClosingLogMessage(void)
{
	pimpl->MakeClosingLogMessage();
}

void NFmiEditMapGeneralDataDoc::ChangeCurrentViewMacroDirectory(const std::string & theDirectoryName, bool fUseRootPathAsBase)
{
    pimpl->ChangeCurrentViewMacroDirectory(theDirectoryName, fUseRootPathAsBase);
}

bool NFmiEditMapGeneralDataDoc::CreateNewViewMacroDirectory(const std::string &thePath)
{
	return pimpl->CreateNewViewMacroDirectory(thePath);
}

bool NFmiEditMapGeneralDataDoc::SelectLanguage(void)
{
	return pimpl->SelectLanguage();
}

FmiLanguage NFmiEditMapGeneralDataDoc::Language(void)
{
	return pimpl->Language();
}

void NFmiEditMapGeneralDataDoc::RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, bool fMakeAreaViewDirty, bool fClearCache, int theWantedMapViewDescTop)
{
	pimpl->RefreshApplicationViewsAndDialogs(reasonForUpdate, fMakeAreaViewDirty, fClearCache, theWantedMapViewDescTop);
}

NFmiTrajectorySystem* NFmiEditMapGeneralDataDoc::TrajectorySystem(void)
{
	return pimpl->TrajectorySystem();
}

void NFmiEditMapGeneralDataDoc::SelectLocations(unsigned int theDescTopIndex, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const boost::shared_ptr<NFmiArea> &theMapArea, const NFmiPoint& theLatLon
						,const NFmiMetTime &theTime, int theSelectionCombineFunction, unsigned long theMask
						,bool fMakeMTAModeAdd
						,bool fDoOnlyMTAModeAdd)
{
	pimpl->SelectLocations(theDescTopIndex, theInfo, theMapArea, theLatLon
						,theTime, theSelectionCombineFunction, theMask
						,fMakeMTAModeAdd
						,fDoOnlyMTAModeAdd);
}

void NFmiEditMapGeneralDataDoc::DoTEMPDataUpdate(const std::string &theTEMPCodeTextStr, std::string &theTEMPCodeCheckReportStr, bool fJustCheckTEMPCode)
{
	pimpl->DoTEMPDataUpdate(theTEMPCodeTextStr, theTEMPCodeCheckReportStr, fJustCheckTEMPCode);
}

void NFmiEditMapGeneralDataDoc::ClearTEMPData(void)
{
	pimpl->ClearTEMPData();
}

const std::string& NFmiEditMapGeneralDataDoc::LastTEMPDataStr(void)
{
	return pimpl->LastTEMPDataStr();
}

void NFmiEditMapGeneralDataDoc::CleanDataDirectories(void)
{
	pimpl->CleanDataDirectories();
}

NFmiFileCleanerSystem& NFmiEditMapGeneralDataDoc::FileCleanerSystem(void)
{
	return pimpl->FileCleanerSystem();
}

void NFmiEditMapGeneralDataDoc::SetMacroParamDataGridSize(int xSize, int ySize)
{
	pimpl->SetMacroParamDataGridSize(xSize, ySize);
}

#ifndef DISABLE_CPPRESTSDK
HakeMessage::Main& NFmiEditMapGeneralDataDoc::WarningCenterSystem(void)
{
	return pimpl->WarningCenterSystem();
}
#endif // DISABLE_CPPRESTSDK
/*
void NFmiEditMapGeneralDataDoc::UpdateWarningMessages(void)
{
	pimpl->UpdateWarningMessages();
}
*/
void NFmiEditMapGeneralDataDoc::UpdateCrossSectionMacroParamDataSize(void)
{
	pimpl->UpdateCrossSectionMacroParamDataSize();
}

void NFmiEditMapGeneralDataDoc::RemoveAllParamsFromCrossSectionViewRow(int theRowIndex)
{
	pimpl->RemoveAllParamsFromCrossSectionViewRow(theRowIndex);
}

void NFmiEditMapGeneralDataDoc::AddMacroParamToCrossSectionView(int theViewRow, const std::string &theName)
{
	pimpl->AddMacroParamToCrossSectionView(theViewRow, theName);
}

NFmiProducerSystem& NFmiEditMapGeneralDataDoc::ProducerSystem(void)
{
	return pimpl->ProducerSystem();
}
NFmiProducerSystem& NFmiEditMapGeneralDataDoc::ObsProducerSystem(void)
{
	return pimpl->ObsProducerSystem();
}

NFmiProducerSystem& NFmiEditMapGeneralDataDoc::SatelImageProducerSystem(void)
{
	return pimpl->SatelImageProducerSystem();
}

void NFmiEditMapGeneralDataDoc::ReportInfoOrganizerDataConsumption(void)
{
	pimpl->ReportInfoOrganizerDataConsumption();
}

void NFmiEditMapGeneralDataDoc::TakeDrawParamInUseEveryWhere(boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool useInMap, bool useInTimeSerial, bool useInCrossSection, bool useWithViewMacros)
{
	pimpl->TakeDrawParamInUseEveryWhere(theDrawParam, useInMap, useInTimeSerial, useInCrossSection, useWithViewMacros);
}

NFmiHelpEditorSystem& NFmiEditMapGeneralDataDoc::HelpEditorSystem(void)
{
	return pimpl->HelpEditorSystem();
}

NFmiMapViewDescTop* NFmiEditMapGeneralDataDoc::MapViewDescTop(unsigned int theIndex)
{
	return pimpl->MapViewDescTop(theIndex);
}

bool NFmiEditMapGeneralDataDoc::ActivateParamSelectionDlgAfterLeftDoubleClick(void)
{
	return pimpl->ActivateParamSelectionDlgAfterLeftDoubleClick();
}
void NFmiEditMapGeneralDataDoc::ActivateParamSelectionDlgAfterLeftDoubleClick(bool newValue)
{
	pimpl->ActivateParamSelectionDlgAfterLeftDoubleClick(newValue);
}
const NFmiPoint& NFmiEditMapGeneralDataDoc::CrossSectionViewSizeInPixels(void)
{
	return pimpl->CrossSectionViewSizeInPixels();
}

void NFmiEditMapGeneralDataDoc::CrossSectionViewSizeInPixels(const NFmiPoint& newSize)
{
	pimpl->CrossSectionViewSizeInPixels(newSize);
}

NFmiPoint NFmiEditMapGeneralDataDoc::ActualCrossSectionBitmapSizeInPixels(void)
{
	return pimpl->ActualCrossSectionBitmapSizeInPixels();
}

void NFmiEditMapGeneralDataDoc::CrossSectionDataViewFrame(const NFmiRect &theRect)
{
	pimpl->CrossSectionDataViewFrame(theRect);
}

const NFmiRect& NFmiEditMapGeneralDataDoc::CrossSectionDataViewFrame(void)
{
	return pimpl->CrossSectionDataViewFrame();
}

void NFmiEditMapGeneralDataDoc::MustDrawTimeSerialView(bool newValue)
{
	pimpl->MustDrawTimeSerialView(newValue);
}
bool NFmiEditMapGeneralDataDoc::MustDrawTimeSerialView(void)
{
	return pimpl->MustDrawTimeSerialView();
}

void NFmiEditMapGeneralDataDoc::MustDrawTempView(bool newValue)
{
	pimpl->MustDrawTempView(newValue);
}
bool NFmiEditMapGeneralDataDoc::MustDrawTempView(void)
{
	return pimpl->MustDrawTempView();
}

void NFmiEditMapGeneralDataDoc::MustDrawCrossSectionView(bool newValue)
{
	pimpl->MustDrawCrossSectionView(newValue);
}
bool NFmiEditMapGeneralDataDoc::MustDrawCrossSectionView(void)
{
	return pimpl->MustDrawCrossSectionView();
}

void NFmiEditMapGeneralDataDoc::BorrowParams(unsigned int theDescTopIndex, int row)
{
	pimpl->BorrowParams(theDescTopIndex, row);
}

std::vector<NFmiProducer>& NFmiEditMapGeneralDataDoc::ExtraSoundingProducerList(void)
{
	return pimpl->ExtraSoundingProducerList();
}
int NFmiEditMapGeneralDataDoc::DataToDBCheckMethod(void)
{
	return pimpl->DataToDBCheckMethod();
}

void NFmiEditMapGeneralDataDoc::DataToDBCheckMethod(int newValue)
{
	pimpl->DataToDBCheckMethod(newValue);
}

bool NFmiEditMapGeneralDataDoc::SetMapViewGrid(unsigned int theDescTopIndex, const NFmiPoint &newValue)
{
	return pimpl->SetMapViewGrid(theDescTopIndex, newValue);
}

CtrlViewUtils::GraphicalInfo& NFmiEditMapGeneralDataDoc::GetGraphicalInfo(unsigned int theDescTopIndex)
{
	return pimpl->GetGraphicalInfo(theDescTopIndex);
}

const NFmiPoint& NFmiEditMapGeneralDataDoc::StationDataGridSize(void)
{return pimpl->StationDataGridSize();}
void NFmiEditMapGeneralDataDoc::StationDataGridSize(const NFmiPoint &newValue)
{pimpl->StationDataGridSize(newValue);}

const NFmiVPlaceDescriptor& NFmiEditMapGeneralDataDoc::SoundingPlotLevels(void)
{return pimpl->SoundingPlotLevels();}

const NFmiPoint& NFmiEditMapGeneralDataDoc::OutOfEditedAreaTimeSerialPoint(void) const
{
	return pimpl->OutOfEditedAreaTimeSerialPoint();
}
void NFmiEditMapGeneralDataDoc::OutOfEditedAreaTimeSerialPoint(const NFmiPoint &newValue)
{
	pimpl->OutOfEditedAreaTimeSerialPoint(newValue);
}
void NFmiEditMapGeneralDataDoc::ResetOutOfEditedAreaTimeSerialPoint(void)
{
	pimpl->ResetOutOfEditedAreaTimeSerialPoint();
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiEditMapGeneralDataDoc::GetNearestSynopStationInfo(const NFmiLocation &theLocation, const NFmiMetTime &theTime, bool ignoreTime, checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > *thePossibleInfoVector, double maxDistanceInMeters)
{
	return pimpl->GetNearestSynopStationInfo(theLocation, theTime, ignoreTime, thePossibleInfoVector, maxDistanceInMeters);
}

void NFmiEditMapGeneralDataDoc::TimeControlTimeStep(unsigned int theDescTopIndex, float newValue)
{
	pimpl->TimeControlTimeStep(theDescTopIndex, newValue);
}

float NFmiEditMapGeneralDataDoc::TimeControlTimeStep(unsigned int theDescTopIndex)
{
	return pimpl->TimeControlTimeStep(theDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::SetSelectedMapHandler(unsigned int theDescTopIndex, unsigned int newMapIndex)
{
	pimpl->SetSelectedMapHandler(theDescTopIndex, newMapIndex);
}

std::vector<NFmiMapViewDescTop*>& NFmiEditMapGeneralDataDoc::MapViewDescTopList(void)
{
	return pimpl->MapViewDescTopList();
}

void NFmiEditMapGeneralDataDoc::OnShowGridPoints(unsigned int theMapViewIndex)
{
	pimpl->OnShowGridPoints(theMapViewIndex);
}

void NFmiEditMapGeneralDataDoc::OnToggleGridPointColor(unsigned int theDescTopIndex)
{
	pimpl->OnToggleGridPointColor(theDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::OnToggleGridPointSize(unsigned int theDescTopIndex)
{
	pimpl->OnToggleGridPointSize(theDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::OnEditSpaceOut(unsigned int theDescTopIndex)
{
	pimpl->OnEditSpaceOut(theDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::OnHideParamWindow(unsigned int theDescTopIndex)
{
	pimpl->OnHideParamWindow(theDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::OnShowTimeString(unsigned int theDescTopIndex)
{
	pimpl->OnShowTimeString(theDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::OnButtonRefresh(void)
{
	pimpl->OnButtonRefresh();
}

void NFmiEditMapGeneralDataDoc::OnChangeMapType(unsigned int theDescTopIndex, bool fForward)
{
	pimpl->OnChangeMapType(theDescTopIndex, fForward);
}

void NFmiEditMapGeneralDataDoc::OnToggleShowNamesOnMap(unsigned int theDescTopIndex, bool fForward)
{
	pimpl->OnToggleShowNamesOnMap(theDescTopIndex, fForward);
}

void NFmiEditMapGeneralDataDoc::OnShowMasksOnMap(unsigned int theDescTopIndex)
{
	pimpl->OnShowMasksOnMap(theDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::OnShowProjectionLines(void)
{
	pimpl->OnShowProjectionLines();
}

void NFmiEditMapGeneralDataDoc::OnToggleLandBorderDrawColor(unsigned int theDescTopIndex)
{
	pimpl->OnToggleLandBorderDrawColor(theDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::OnToggleLandBorderPenSize(unsigned int theDescTopIndex)
{
	pimpl->OnToggleLandBorderPenSize(theDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::OnAcceleratorBorrowParams(unsigned int theDescTopIndex, int row)
{
	pimpl->OnAcceleratorBorrowParams(theDescTopIndex, row);
}

void NFmiEditMapGeneralDataDoc::OnAcceleratorMapRow(unsigned int theDescTopIndex, int theStartingRow)
{
	pimpl->OnAcceleratorMapRow(theDescTopIndex, theStartingRow);
}

void NFmiEditMapGeneralDataDoc::OnToggleOverMapBackForeGround(unsigned int theDescTopIndex)
{
	pimpl->OnToggleOverMapBackForeGround(theDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::OnAcceleratorToggleKeepMapRatio(void)
{
	pimpl->OnAcceleratorToggleKeepMapRatio();
}

void NFmiEditMapGeneralDataDoc::OnButtonDataArea(unsigned int theDescTopIndex)
{
	pimpl->OnButtonDataArea(theDescTopIndex);
}

double NFmiEditMapGeneralDataDoc::DrawObjectScaleFactor(void)
{
	return pimpl->DrawObjectScaleFactor();
}
void NFmiEditMapGeneralDataDoc::DrawObjectScaleFactor(double newValue)
{
	pimpl->DrawObjectScaleFactor(newValue);
}

NFmiConceptualModelData& NFmiEditMapGeneralDataDoc::ConceptualModelData(void)
{
	return pimpl->ConceptualModelData();
}

void NFmiEditMapGeneralDataDoc::GetDataFromQ2Server(const std::string &theURLStr,
													const std::string &theParamsStr,
													bool fUseBinaryData,
													int theUsedCompression,
													NFmiDataMatrix<float> &theDataMatrixOut,
													std::string &theExtraInfoStrOut)
{
	pimpl->GetDataFromQ2Server(theURLStr, theParamsStr, fUseBinaryData, theUsedCompression,
								theDataMatrixOut, theExtraInfoStrOut);
}

NFmiQ2Client& NFmiEditMapGeneralDataDoc::Q2Client(void)
{
	return pimpl->Q2Client();
}
bool NFmiEditMapGeneralDataDoc::UseQ2Server(void)
{return pimpl->UseQ2Server();}

NFmiSeaIcingWarningSystem& NFmiEditMapGeneralDataDoc::SeaIcingWarningSystem(void)
{
	return pimpl->SeaIcingWarningSystem();
}

NFmiWindTableSystem& NFmiEditMapGeneralDataDoc::WindTableSystem(void)
{
	return pimpl->WindTableSystem();
}

int NFmiEditMapGeneralDataDoc::SoundingViewWindBarbSpaceOutFactor(void)
{
	return pimpl->SoundingViewWindBarbSpaceOutFactor();
}

void NFmiEditMapGeneralDataDoc::SoundingViewWindBarbSpaceOutFactor(int newValue)
{
	pimpl->SoundingViewWindBarbSpaceOutFactor(newValue);
}

void NFmiEditMapGeneralDataDoc::TransparencyContourDrawView(CWnd *theView)
{
	pimpl->TransparencyContourDrawView(theView);
}

CWnd* NFmiEditMapGeneralDataDoc::TransparencyContourDrawView(void)
{
	return pimpl->TransparencyContourDrawView();
}

checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > NFmiEditMapGeneralDataDoc::GetSortedSynopInfoVector(int theProducerId, int theProducerId2, int theProducerId3, int theProducerId4)
{
	return pimpl->GetSortedSynopInfoVector(theProducerId, theProducerId2, theProducerId3, theProducerId4);
}

void NFmiEditMapGeneralDataDoc::MakeSwapBaseArea(unsigned int theDescTopIndex)
{
	pimpl->MakeSwapBaseArea(theDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::SwapArea(unsigned int theDescTopIndex)
{
	pimpl->SwapArea(theDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::ToggleShowHelperDatasInTimeView(int theCommand)
{
	pimpl->ToggleShowHelperDatasInTimeView(theCommand);
}

bool NFmiEditMapGeneralDataDoc::ShowToolTipTimeView(void)
{
	return pimpl->ShowToolTipTimeView();
}
void NFmiEditMapGeneralDataDoc::ShowToolTipTimeView(bool newValue)
{
	pimpl->ShowToolTipTimeView(newValue);
}

bool NFmiEditMapGeneralDataDoc::ShowToolTipTempView(void)
{
	return pimpl->ShowToolTipTempView();
}
void NFmiEditMapGeneralDataDoc::ShowToolTipTempView(bool newValue)
{
	pimpl->ShowToolTipTempView(newValue);
}

bool NFmiEditMapGeneralDataDoc::ShowToolTipTrajectoryView(void)
{
	return pimpl->ShowToolTipTrajectoryView();
}
void NFmiEditMapGeneralDataDoc::ShowToolTipTrajectoryView(bool newValue)
{
	pimpl->ShowToolTipTrajectoryView(newValue);
}

void NFmiEditMapGeneralDataDoc::ReloadAllDynamicHelpData()
{
	pimpl->ReloadAllDynamicHelpData();
}

NFmiMapViewTimeLabelInfo& NFmiEditMapGeneralDataDoc::MapViewTimeLabelInfo()
{
	return pimpl->MapViewTimeLabelInfo();
}

void NFmiEditMapGeneralDataDoc::TimeControlViewTimes(unsigned int theDescTopIndex, const NFmiTimeDescriptor &newTimeDescriptor)
{
	pimpl->TimeControlViewTimes(theDescTopIndex, newTimeDescriptor);
}

const NFmiTimeDescriptor& NFmiEditMapGeneralDataDoc::TimeControlViewTimes(unsigned int theDescTopIndex)
{
	return 	pimpl->TimeControlViewTimes(theDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::TimeSerialViewTimeBag(const NFmiTimeBag &theTimeBag)
{
	pimpl->TimeSerialViewTimeBag(theTimeBag);
}

const NFmiTimeBag& NFmiEditMapGeneralDataDoc::TimeSerialViewTimeBag(void) const
{
	return pimpl->TimeSerialViewTimeBag();
}

void NFmiEditMapGeneralDataDoc::CopyDrawParamsFromMapViewRow(unsigned int theDescTopIndex)
{
	pimpl->CopyDrawParamsFromMapViewRow(theDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::PasteDrawParamsToMapViewRow(unsigned int theDescTopIndex)
{
	pimpl->PasteDrawParamsToMapViewRow(theDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::CopyMapViewDescTopParams(unsigned int theDescTopIndex)
{
	pimpl->CopyMapViewDescTopParams(theDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::PasteMapViewDescTopParams(unsigned int theDescTopIndex)
{
	pimpl->PasteMapViewDescTopParams(theDescTopIndex);
}

int NFmiEditMapGeneralDataDoc::DoAllAnimations(void)
{
	return pimpl->DoAllAnimations();
}

bool NFmiEditMapGeneralDataDoc::ShowWaitCursorWhileDrawingView(void)
{
	return pimpl->ShowWaitCursorWhileDrawingView();
}

void NFmiEditMapGeneralDataDoc::CheckAnimationLockedModeTimeBags(unsigned int theDescTopIndex, bool ignoreSatelImages)
{
	pimpl->CheckAnimationLockedModeTimeBags(theDescTopIndex, ignoreSatelImages);
}

bool NFmiEditMapGeneralDataDoc::DoAutoLoadDataAtStartUp(void) const
{
	return pimpl->DoAutoLoadDataAtStartUp();
}
void NFmiEditMapGeneralDataDoc::DoAutoLoadDataAtStartUp(bool newValue)
{
	pimpl->DoAutoLoadDataAtStartUp(newValue);
}

void NFmiEditMapGeneralDataDoc::CheckForNewConceptualModelData(void)
{
    pimpl->CheckForNewConceptualModelData();
}

void NFmiEditMapGeneralDataDoc::ToggleTimeControlAnimationView(unsigned int theDescTopIndex)
{
	pimpl->ToggleTimeControlAnimationView(theDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::MakeDrawedInfoVectorForMapView(checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector, boost::shared_ptr<NFmiDrawParam> &theDrawParam, const boost::shared_ptr<NFmiArea> &theArea)
{
	pimpl->MakeDrawedInfoVectorForMapView(theInfoVector, theDrawParam, theArea);
}

const std::string& NFmiEditMapGeneralDataDoc::WorkingDirectory(void) const
{
	return pimpl->WorkingDirectory();
}

const std::string& NFmiEditMapGeneralDataDoc::ControlDirectory(void) const
{
    return pimpl->ControlDirectory();
}

const std::string& NFmiEditMapGeneralDataDoc::HelpDataPath(void) const
{
    return pimpl->HelpDataPath();
}

const NFmiPoint& NFmiEditMapGeneralDataDoc::TimeSerialViewSizeInPixels(void) const
{
	return pimpl->TimeSerialViewSizeInPixels();
}
void NFmiEditMapGeneralDataDoc::TimeSerialViewSizeInPixels(const NFmiPoint &newValue)
{
	pimpl->TimeSerialViewSizeInPixels(newValue);
}

void NFmiEditMapGeneralDataDoc::CenterTimeControlView(unsigned int theDescTopIndex, const NFmiMetTime &theTime, bool fUpdateSelectedTime)
{
	pimpl->CenterTimeControlView(theDescTopIndex, theTime, fUpdateSelectedTime);
}

bool NFmiEditMapGeneralDataDoc::CheckEditedDataForStartUpLoadErrors(int theMessageBoxButtunOptions)
{
	return pimpl->CheckEditedDataForStartUpLoadErrors(theMessageBoxButtunOptions);
}

NFmiPoint NFmiEditMapGeneralDataDoc::GetPrintedMapAreaOnScreenSizeInPixels(unsigned int theDescTopIndex)
{
	return pimpl->GetPrintedMapAreaOnScreenSizeInPixels(theDescTopIndex);
}

bool NFmiEditMapGeneralDataDoc::DeveloperModePath(void) 
{
	return pimpl->DeveloperModePath();
}
void NFmiEditMapGeneralDataDoc::DeveloperModePath(bool newValue) 
{
	pimpl->DeveloperModePath(newValue);
}

NFmiDataNotificationSettingsWinRegistry& NFmiEditMapGeneralDataDoc::DataNotificationSettings(void)
{
	return pimpl->DataNotificationSettings();
}

bool NFmiEditMapGeneralDataDoc::UseTimeSerialAxisAutoAdjust(void)
{
	return pimpl->UseTimeSerialAxisAutoAdjust();
}
void NFmiEditMapGeneralDataDoc::UseTimeSerialAxisAutoAdjust(bool newValue)
{
	pimpl->UseTimeSerialAxisAutoAdjust(newValue);
}

void NFmiEditMapGeneralDataDoc::ChangeDriveLetterInSettings(const std::string &newLetter)
{
	pimpl->ChangeDriveLetterInSettings(newLetter);
}

std::string NFmiEditMapGeneralDataDoc::GetModelOrigTimeString(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theIndex)
{
	return pimpl->GetModelOrigTimeString(theDrawParam, theIndex);
}

NFmiMetTime NFmiEditMapGeneralDataDoc::GetModelOrigTime(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theIndex)
{
	return pimpl->GetModelOrigTime(theDrawParam, theIndex);
}

void NFmiEditMapGeneralDataDoc::UndoViewMacro(void)
{
	pimpl->UndoViewMacro();
}
void NFmiEditMapGeneralDataDoc::RedoViewMacro(void)
{
	pimpl->RedoViewMacro();
}
bool NFmiEditMapGeneralDataDoc::IsUndoableViewMacro(void)
{
	return pimpl->IsUndoableViewMacro();
}
bool NFmiEditMapGeneralDataDoc::IsRedoableViewMacro(void)
{
	return pimpl->IsRedoableViewMacro();
}

NFmiAutoComplete& NFmiEditMapGeneralDataDoc::AutoComplete(void)
{
	return pimpl->AutoComplete();
}

void NFmiEditMapGeneralDataDoc::SetMapViewCacheSize(double theNewSizeInMB)
{
	pimpl->SetMapViewCacheSize(theNewSizeInMB);
}

const NFmiColor& NFmiEditMapGeneralDataDoc::GeneralColor(int theIndex)
{
	return pimpl->GeneralColor(theIndex);
}

bool NFmiEditMapGeneralDataDoc::SetNearestBeforeModelOrigTimeRunoff(boost::shared_ptr<NFmiDrawParam> &theDrawParam ,const NFmiMetTime &theTime, unsigned int theDescTopIndex, int theViewRowIndex)
{
	return pimpl->SetNearestBeforeModelOrigTimeRunoff(theDrawParam ,theTime, theDescTopIndex, theViewRowIndex);
}

void NFmiEditMapGeneralDataDoc::SetModelRunOffset(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theMoveByValue, unsigned int theDescTopIndex, int theViewRowIndex)
{
	pimpl->SetModelRunOffset(theDrawParam, theMoveByValue, theDescTopIndex, theViewRowIndex);
}

std::string NFmiEditMapGeneralDataDoc::AutoGeneratedSoundingIndexBasePath(void)
{
	return pimpl->AutoGeneratedSoundingIndexBasePath();
}

bool NFmiEditMapGeneralDataDoc::ChangeTime(int theTypeOfChange, FmiDirection theDirection, int theViewType, unsigned long theMapViewIndex, double theAmountOfChange)
{
	return pimpl->ChangeTime(theTypeOfChange, theDirection, theViewType, theMapViewIndex, theAmountOfChange);
}

NFmiModelDataBlender& NFmiEditMapGeneralDataDoc::ModelDataBlender(void)
{
	return pimpl->ModelDataBlender();
}

NFmiAnalyzeToolData& NFmiEditMapGeneralDataDoc::AnalyzeToolData(void)
{
	return pimpl->AnalyzeToolData();
}

NFmiDataQualityChecker& NFmiEditMapGeneralDataDoc::DataQualityChecker(void)
{
	return pimpl->DataQualityChecker();
}

void NFmiEditMapGeneralDataDoc::DrawOverBitmapThings(NFmiToolBox * theGTB)
{
	pimpl->DrawOverBitmapThings(theGTB);
}

void NFmiEditMapGeneralDataDoc::UpdateTempView(void)
{
	pimpl->UpdateTempView();
}

void NFmiEditMapGeneralDataDoc::UpdateCrossSectionView(void)
{
	pimpl->UpdateCrossSectionView();
}

void NFmiEditMapGeneralDataDoc::ActivateViewParamSelectorDlg(int theMapViewDescTopIndex)
{
	pimpl->ActivateViewParamSelectorDlg(theMapViewDescTopIndex);
}

std::string NFmiEditMapGeneralDataDoc::GetToolTipString(unsigned int commandID, std::string &theMagickWord)
{
	return pimpl->GetToolTipString(commandID, theMagickWord);
}

void NFmiEditMapGeneralDataDoc::ActivateZoomDialog(int theWantedDescTopIndex)
{
	pimpl->ActivateZoomDialog(theWantedDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::ForceDrawOverBitmapThings(unsigned int originalCallerDescTopIndex, bool doOriginalView, bool doAllOtherMapViews)
{
	pimpl->ForceDrawOverBitmapThings(originalCallerDescTopIndex, doOriginalView, doAllOtherMapViews);
}

void NFmiEditMapGeneralDataDoc::InvalidateMapView(bool bErase)
{
	pimpl->InvalidateMapView(bErase);
}

void NFmiEditMapGeneralDataDoc::SetMacroErrorText(const std::string &theErrorStr)
{
	pimpl->SetMacroErrorText(theErrorStr);
}

void NFmiEditMapGeneralDataDoc::ReportProcessMemoryUsage(void)
{
	pimpl->ReportProcessMemoryUsage();
}

int NFmiEditMapGeneralDataDoc::MachineThreadCount(void)
{
	return pimpl->MachineThreadCount();
}

int NFmiEditMapGeneralDataDoc::SatelDataRefreshTimerInMinutes(void)
{
	return pimpl->SatelDataRefreshTimerInMinutes();
}
void NFmiEditMapGeneralDataDoc::SatelDataRefreshTimerInMinutes(int newValue)
{
	pimpl->SatelDataRefreshTimerInMinutes(newValue);
}

NFmiIgnoreStationsData& NFmiEditMapGeneralDataDoc::IgnoreStationsData(void)
{
	return pimpl->IgnoreStationsData();
}

bool NFmiEditMapGeneralDataDoc::EditedDataNeedsToBeLoaded(void)
{
	return pimpl->EditedDataNeedsToBeLoaded();
}

void NFmiEditMapGeneralDataDoc::EditedDataNeedsToBeLoaded(bool newValue)
{
	pimpl->EditedDataNeedsToBeLoaded(newValue);
}

bool NFmiEditMapGeneralDataDoc::TryAutoStartUpLoad(void)
{
	return pimpl->TryAutoStartUpLoad();
}

NFmiMilliSecondTimer& NFmiEditMapGeneralDataDoc::EditedDataNeedsToBeLoadedTimer(void) 
{
	return pimpl->EditedDataNeedsToBeLoadedTimer();
}

bool NFmiEditMapGeneralDataDoc::EditedDataNotInPreferredState(void)
{
	return pimpl->EditedDataNotInPreferredState();
}

void NFmiEditMapGeneralDataDoc::EditedDataNotInPreferredState(bool newValue)
{
	pimpl->EditedDataNotInPreferredState(newValue);
}

bool NFmiEditMapGeneralDataDoc::CheckEditedDataAfterAutoLoad(void)
{
	return pimpl->CheckEditedDataAfterAutoLoad();
}

int NFmiEditMapGeneralDataDoc::RunningTimeInSeconds()
{
	return pimpl->RunningTimeInSeconds();
}

NFmiApplicationDataBase& NFmiEditMapGeneralDataDoc::ApplicationDataBase(void)
{
	return pimpl->ApplicationDataBase();
}

void NFmiEditMapGeneralDataDoc::OnButtonRedo(void) 
{
	pimpl->OnButtonRedo();
}

void NFmiEditMapGeneralDataDoc::OnButtonUndo(void)
{
	return pimpl->OnButtonUndo();
}

int NFmiEditMapGeneralDataDoc::CleanUnusedDataFromMemory(void)
{
	return pimpl->CleanUnusedDataFromMemory();
}

NFmiMacroPathSettings& NFmiEditMapGeneralDataDoc::MacroPathSettings(void)
{
	return pimpl->MacroPathSettings();
}

void NFmiEditMapGeneralDataDoc::DoUnisonDirectorySync(const std::string &theRoot1, const std::string &theRoot2, bool preferRoot1, WORD theShowWindow, bool waitExecutionToStop, DWORD dwCreationFlags)
{
	pimpl->DoUnisonDirectorySync(theRoot1, theRoot2, preferRoot1, theShowWindow, waitExecutionToStop, dwCreationFlags);
}

void NFmiEditMapGeneralDataDoc::DoMacroDirectoriesSyncronization(void)
{
	pimpl->DoMacroDirectoriesSyncronization();
}

NFmiCaseStudySystem& NFmiEditMapGeneralDataDoc::CaseStudySystem(void)
{
	return pimpl->CaseStudySystem();
}

bool NFmiEditMapGeneralDataDoc::LoadCaseStudyData(const std::string &theCaseStudyMetaFile)
{
	return pimpl->LoadCaseStudyData(theCaseStudyMetaFile);
}

bool NFmiEditMapGeneralDataDoc::CaseStudyModeOn(void) 
{
	return pimpl->CaseStudyModeOn();
}

NFmiCaseStudySystem& NFmiEditMapGeneralDataDoc::LoadedCaseStudySystem(void)
{
	return pimpl->LoadedCaseStudySystem();
}

void NFmiEditMapGeneralDataDoc::CaseStudyToNormalMode(void)
{
	pimpl->CaseStudyToNormalMode();
}

bool NFmiEditMapGeneralDataDoc::StoreCaseStudyMemory(void)
{
	return pimpl->StoreCaseStudyMemory();
}

NFmiDataLoadingInfo& NFmiEditMapGeneralDataDoc::GetUsedDataLoadingInfo(void)
{
	return pimpl->GetUsedDataLoadingInfo();
}

int NFmiEditMapGeneralDataDoc::DoMessageBox(const std::string & theMessage, const std::string &theTitle, unsigned int theMessageBoxType)
{
	return pimpl->DoMessageBox(theMessage, theTitle, theMessageBoxType);
}

void NFmiEditMapGeneralDataDoc::AllMapViewDescTopsTimeDirty(const NFmiMetTime &theTime)
{
	pimpl->AllMapViewDescTopsTimeDirty(theTime);
}

boost::shared_ptr<NFmiDrawParam> NFmiEditMapGeneralDataDoc::GetUsedDrawParam(const NFmiDataIdent &theDataIdent, NFmiInfoData::Type theDataType)
{
	return pimpl->GetUsedDrawParam(theDataIdent, theDataType);
}

const NFmiPoint& NFmiEditMapGeneralDataDoc::TimeFilterRangeStart(void)
{
	return pimpl->TimeFilterRangeStart();
}

const NFmiPoint& NFmiEditMapGeneralDataDoc::TimeFilterRangeEnd(void)
{
	return pimpl->TimeFilterRangeEnd();
}

bool NFmiEditMapGeneralDataDoc::IsSmoothTimeShiftPossible(void)
{
	return pimpl->IsSmoothTimeShiftPossible();
}

bool NFmiEditMapGeneralDataDoc::StoreData(const std::string& theFileName, boost::shared_ptr<NFmiFastQueryInfo> &theSmartInfo, bool askForSave)
{
	return pimpl->StoreData(theFileName, theSmartInfo, askForSave);
}

bool NFmiEditMapGeneralDataDoc::StoreData(bool fDoSaveTmpRename, const NFmiString& theFileName, NFmiQueryData *theData, bool writeLocalDiskThenCopyToDestination)
{
    return pimpl->StoreData(fDoSaveTmpRename, theFileName, theData, writeLocalDiskThenCopyToDestination);
}

bool NFmiEditMapGeneralDataDoc::DataLoadingOK(bool noError)
{
	return pimpl->DataLoadingOK(noError);
}

bool NFmiEditMapGeneralDataDoc::WarnIfCantSaveWorkingFile(void)
{
	return pimpl->WarnIfCantSaveWorkingFile();
}

NFmiHPlaceDescriptor* NFmiEditMapGeneralDataDoc::PossibleUsedDataLoadingGrid(void)
{
	return pimpl->PossibleUsedDataLoadingGrid();
}

bool NFmiEditMapGeneralDataDoc::UseEditedDataParamDescriptor(void)
{
	return pimpl->UseEditedDataParamDescriptor();
}

NFmiParamDescriptor& NFmiEditMapGeneralDataDoc::EditedDataParamDescriptor(void)
{
	return pimpl->EditedDataParamDescriptor();
}

checkedVector<int>& NFmiEditMapGeneralDataDoc::DataLoadingProducerIndexVector(void)
{
	return pimpl->DataLoadingProducerIndexVector();
}

void NFmiEditMapGeneralDataDoc::PutWarningFlagTimerOn(void)
{
	pimpl->PutWarningFlagTimerOn();
}

NFmiProducerIdLister& NFmiEditMapGeneralDataDoc::ProducerIdLister(void)
{
	return pimpl->ProducerIdLister();
}

void NFmiEditMapGeneralDataDoc::DoDataLoadingProblemsDlg(const std::string &theMessage)
{
	pimpl->DoDataLoadingProblemsDlg(theMessage);
}

TimeSerialModificationDataInterface& NFmiEditMapGeneralDataDoc::GenDocDataAdapter(void)
{
	return pimpl->GenDocDataAdapter();
}

void NFmiEditMapGeneralDataDoc::InitGenDocDataAdapter(NFmiEditMapGeneralDataDoc *theDoc)
{
	pimpl->InitGenDocDataAdapter(theDoc);
}

bool NFmiEditMapGeneralDataDoc::DataModificationInProgress(void)
{
	return pimpl->DataModificationInProgress();
}

void NFmiEditMapGeneralDataDoc::DataModificationInProgress(bool newValue)
{
	pimpl->DataModificationInProgress(newValue);
}

bool NFmiEditMapGeneralDataDoc::UseMultithreaddingWithModifyingFunctions(void)
{
	return pimpl->UseMultithreaddingWithModifyingFunctions();
}

bool NFmiEditMapGeneralDataDoc::EditedPointsSelectionChanged(void)
{
	return pimpl->EditedPointsSelectionChanged();
}

bool NFmiEditMapGeneralDataDoc::EditedPointsSelectionChanged(bool newValue)
{
	return pimpl->EditedPointsSelectionChanged(newValue);
}

size_t NFmiEditMapGeneralDataDoc::SelectedGridPointLimit(void) 
{
	return pimpl->SelectedGridPointLimit();
}
void NFmiEditMapGeneralDataDoc::SelectedGridPointLimit(size_t newValue) 
{
	pimpl->SelectedGridPointLimit(newValue);
}

bool NFmiEditMapGeneralDataDoc::DrawSelectionOnThisView(void)
{
	return pimpl->DrawSelectionOnThisView();
}

void NFmiEditMapGeneralDataDoc::DrawSelectionOnThisView(bool newValue)
{
	pimpl->DrawSelectionOnThisView(newValue);
}

const NFmiRect& NFmiEditMapGeneralDataDoc::CPGridCropRect(void)
{
	return pimpl->CPGridCropRect();
}

bool NFmiEditMapGeneralDataDoc::UseCPGridCrop(void)
{
	return pimpl->UseCPGridCrop();
}

void NFmiEditMapGeneralDataDoc::UseCPGridCrop(bool newValue)
{
	pimpl->UseCPGridCrop(newValue);
}

bool NFmiEditMapGeneralDataDoc::IsCPGridCropInAction(void)
{
	return pimpl->IsCPGridCropInAction();
}

const boost::shared_ptr<NFmiArea> NFmiEditMapGeneralDataDoc::CPGridCropLatlonArea(void) 
{
	return pimpl->CPGridCropLatlonArea();
}

boost::shared_ptr<NFmiArea> NFmiEditMapGeneralDataDoc::CPGridCropInnerLatlonArea(void)
{
	return pimpl->CPGridCropInnerLatlonArea();
}

const NFmiPoint& NFmiEditMapGeneralDataDoc::CPGridCropMargin(void)
{
	return pimpl->CPGridCropMargin();
}

bool NFmiEditMapGeneralDataDoc::IsCPGridCropNotPlausible(void)
{
	return pimpl->IsCPGridCropNotPlausible();
}

NFmiCPManagerSet& NFmiEditMapGeneralDataDoc::CPManagerSet(void)
{
	return pimpl->CPManagerSet();
}

NFmiApplicationWinRegistry& NFmiEditMapGeneralDataDoc::ApplicationWinRegistry()
{
	return pimpl->ApplicationWinRegistry();
}

void NFmiEditMapGeneralDataDoc::ZoomMapWithRelativeRect(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theMapArea, const NFmiRect &theZoomedRect)
{
    pimpl->ZoomMapWithRelativeRect(theMapViewDescTopIndex, theMapArea, theZoomedRect);
}

void NFmiEditMapGeneralDataDoc::PanMap(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theMapArea, const NFmiPoint &theMousePoint, const NFmiPoint &theZoomDragUpPoint)
{
    pimpl->PanMap(theMapViewDescTopIndex, theMapArea, theMousePoint, theZoomDragUpPoint);
}

void NFmiEditMapGeneralDataDoc::ZoomMapInOrOut(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theMapArea, const NFmiPoint &theMousePoint, double theZoomFactor)
{
    pimpl->ZoomMapInOrOut(theMapViewDescTopIndex, theMapArea, theMousePoint, theZoomFactor);
}

void NFmiEditMapGeneralDataDoc::StoreSettingsToWinRegistry(void)
{
    pimpl->StoreSettingsToWinRegistry();
}

const NFmiMetTime& NFmiEditMapGeneralDataDoc::ActiveMapTime(void)
{
    return pimpl->ActiveMapTime();
}

unsigned int NFmiEditMapGeneralDataDoc::ActiveMapDescTopIndex(void)
{
    return pimpl->ActiveMapDescTopIndex();
}

void NFmiEditMapGeneralDataDoc::ActiveMapDescTopIndex(unsigned int newValue)
{
    pimpl->ActiveMapDescTopIndex(newValue);
}

void NFmiEditMapGeneralDataDoc::UpdateEnableDataChangesToWinReg(void)
{
        pimpl->UpdateEnableDataChangesToWinReg();
}

MultiProcessClientData& NFmiEditMapGeneralDataDoc::GetMultiProcessClientData(void)
{
    return pimpl->GetMultiProcessClientData();
}

const std::string& NFmiEditMapGeneralDataDoc::GetSmartMetGuid(void)
{
    return pimpl->GetSmartMetGuid();
}

bool NFmiEditMapGeneralDataDoc::UseMultiProcessCpCalc(void)
{
    return pimpl->UseMultiProcessCpCalc();
}

void NFmiEditMapGeneralDataDoc::UseMultiProcessCpCalc(bool newValue)
{
    pimpl->UseMultiProcessCpCalc(newValue);
}

bool NFmiEditMapGeneralDataDoc::MakeSureToolMasterPoolIsRunning(void)
{
    return pimpl->MakeSureToolMasterPoolIsRunning();
}

NFmiMultiProcessPoolOptions& NFmiEditMapGeneralDataDoc::MultiProcessPoolOptions(void)
{
    return pimpl->MultiProcessPoolOptions();
}

bool NFmiEditMapGeneralDataDoc::AllowRightClickDisplaySelection(void)
{
    return pimpl->AllowRightClickDisplaySelection();
}

void NFmiEditMapGeneralDataDoc::AllowRightClickDisplaySelection(bool newValue)
{
    pimpl->AllowRightClickDisplaySelection(newValue);
}

bool NFmiEditMapGeneralDataDoc::ChangeParamSettingsToNextFixedDrawParam(unsigned int theDescTopIndex, int theMapRow, int theParamIndex, bool fNext, bool fUseCrossSectionParams)
{
    return pimpl->ChangeParamSettingsToNextFixedDrawParam(theDescTopIndex, theMapRow, theParamIndex, fNext, fUseCrossSectionParams);
}

NFmiFixedDrawParamSystem& NFmiEditMapGeneralDataDoc::FixedDrawParamSystem()
{
    return pimpl->FixedDrawParamSystem();
}

void NFmiEditMapGeneralDataDoc::ReloadFixedDrawParams()
{
    pimpl->ReloadFixedDrawParams();
}

void NFmiEditMapGeneralDataDoc::ApplyBackupViewMacro(bool fUseNormalBackup)
{
    pimpl->ApplyBackupViewMacro(fUseNormalBackup);
}

void NFmiEditMapGeneralDataDoc::StoreBackUpViewMacro(bool fUseNormalBackup)
{
    pimpl->StoreBackUpViewMacro(fUseNormalBackup);
}

const NFmiPoint& NFmiEditMapGeneralDataDoc::PreciseTimeSerialLatlonPoint() const
{
    return pimpl->PreciseTimeSerialLatlonPoint();
}

void NFmiEditMapGeneralDataDoc::PreciseTimeSerialLatlonPoint(const NFmiPoint &newValue)
{
    pimpl->PreciseTimeSerialLatlonPoint(newValue);
}

bool NFmiEditMapGeneralDataDoc::IsPreciseTimeSerialLatlonPointUsed()
{
    return pimpl->IsPreciseTimeSerialLatlonPointUsed();
}

int NFmiEditMapGeneralDataDoc::DoImageCacheUpdates(void)
{
    return pimpl->DoImageCacheUpdates();
}

NFmiSatelliteImageCacheSystem& NFmiEditMapGeneralDataDoc::SatelliteImageCacheSystem()
{
    return pimpl->SatelliteImageCacheSystem();
}

void NFmiEditMapGeneralDataDoc::ReloadFailedSatelliteImages()
{
    return pimpl->ReloadFailedSatelliteImages();
}

void NFmiEditMapGeneralDataDoc::OnAcceleratorSetHomeTime(int theMapViewDescTopIndex)
{
    pimpl->OnAcceleratorSetHomeTime(theMapViewDescTopIndex);
}

bool NFmiEditMapGeneralDataDoc::BetaProductGenerationRunning()
{
    return pimpl->BetaProductGenerationRunning();
}

void NFmiEditMapGeneralDataDoc::BetaProductGenerationRunning(bool newValue)
{
    pimpl->BetaProductGenerationRunning(newValue);
}

const std::string& NFmiEditMapGeneralDataDoc::RootViewMacroPath()
{
    return pimpl->RootViewMacroPath();
}

bool NFmiEditMapGeneralDataDoc::LoadViewMacroFromBetaProduct(const std::string &theAbsoluteVieMacroPath, std::string &theErrorStringOut, bool justLogMessages)
{
    return pimpl->LoadViewMacroFromBetaProduct(theAbsoluteVieMacroPath, theErrorStringOut, justLogMessages);
}

NFmiBetaProductionSystem& NFmiEditMapGeneralDataDoc::BetaProductionSystem()
{
    return pimpl->BetaProductionSystem();
}

LogAndWarnFunctionType NFmiEditMapGeneralDataDoc::GetLogAndWarnFunction()
{
    return pimpl->GetLogAndWarnFunction();
}

void NFmiEditMapGeneralDataDoc::DoGenerateBetaProductsChecks()
{
    pimpl->DoGenerateBetaProductsChecks();
}

void NFmiEditMapGeneralDataDoc::FillViewMacroInfo(NFmiViewSettingMacro &theViewMacro, const std::string &theName, const std::string &theDescription)
{
    pimpl->FillViewMacroInfo(theViewMacro, theName, theDescription);
}

void NFmiEditMapGeneralDataDoc::LoadViewMacroInfo(NFmiViewSettingMacro &theViewMacro, bool fTreatAsViewMacro)
{
    // T‰t‰ kutsutaan GenDocin ulkoa vain BetaProduct dialogista, siksi kyse ei ole undoRedo-actionista ja siksi viimeinen parametri on false
    pimpl->LoadViewMacroInfo(theViewMacro, fTreatAsViewMacro, false); 
}

void NFmiEditMapGeneralDataDoc::SetCurrentGeneratedBetaProduct(const NFmiBetaProduct *theBetaProduct)
{
    pimpl->SetCurrentGeneratedBetaProduct(theBetaProduct);
}

const NFmiBetaProduct* NFmiEditMapGeneralDataDoc::GetCurrentGeneratedBetaProduct()
{
    return pimpl->GetCurrentGeneratedBetaProduct();
}

void NFmiEditMapGeneralDataDoc::SetAllViewIconsDynamically(void)
{
    pimpl->SetAllViewIconsDynamically();
}

void NFmiEditMapGeneralDataDoc::ResetTimeFilterTimes()
{
    pimpl->ResetTimeFilterTimes();
}

const NFmiMetTime& NFmiEditMapGeneralDataDoc::LastEditedDataSendTime()
{
    return pimpl->LastEditedDataSendTime();
}

bool NFmiEditMapGeneralDataDoc::IsLastEditedDataSendHasComeBack()
{
    return pimpl->IsLastEditedDataSendHasComeBack();
}

const std::vector<std::string>& NFmiEditMapGeneralDataDoc::ViewMacroFileNames(bool updateList)
{
    return pimpl->ViewMacroFileNames(updateList);
}

std::vector<NFmiLightWeightViewSettingMacro>& NFmiEditMapGeneralDataDoc::ViewMacroDescriptionList()
{
    return pimpl->ViewMacroDescriptionList();
}

std::string NFmiEditMapGeneralDataDoc::GetRelativeViewMacroPath()
{
    return pimpl->GetRelativeViewMacroPath();
}

const std::vector<std::string>& NFmiEditMapGeneralDataDoc::SmartToolFileNames(bool updateList)
{
    return pimpl->SmartToolFileNames(updateList);
}

const std::vector<std::string>& NFmiEditMapGeneralDataDoc::CorruptedViewMacroFileList() const
{
    return pimpl->CorruptedViewMacroFileList();
}

NFmiEditDataUserList& NFmiEditMapGeneralDataDoc::EditDataUserList()
{
    return pimpl->EditDataUserList();
}

void NFmiEditMapGeneralDataDoc::CheckForNewWarningMessageData()
{
    pimpl->CheckForNewWarningMessageData();
}

void NFmiEditMapGeneralDataDoc::ApplyStartupViewMacro()
{
    pimpl->ApplyStartupViewMacro();
}

void NFmiEditMapGeneralDataDoc::FillStartupViewMacro()
{
    pimpl->FillStartupViewMacro();
}

void NFmiEditMapGeneralDataDoc::MakeHTTPRequest(const std::string &theUrlStr, std::string &theResponseStrOut, bool fDoGet)
{
    pimpl->MakeHTTPRequest(theUrlStr, theResponseStrOut, fDoGet);
}

bool NFmiEditMapGeneralDataDoc::IsWorkingDataSaved()
{
    return pimpl->IsWorkingDataSaved();
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiEditMapGeneralDataDoc::GetModelClimatologyData()
{
    return pimpl->GetModelClimatologyData();
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiEditMapGeneralDataDoc::GetFavoriteSurfaceModelFractileData()
{
    return pimpl->GetFavoriteSurfaceModelFractileData();
}

bool NFmiEditMapGeneralDataDoc::UseWmsMaps()
{
    return pimpl->UseWmsMaps();
}

void NFmiEditMapGeneralDataDoc::UseWmsMaps(bool newValue)
{
    pimpl->UseWmsMaps(newValue);
}

#ifndef DISABLE_CPPRESTSDK
Wms::WmsSupport& NFmiEditMapGeneralDataDoc::WmsSupport()
{
    return pimpl->WmsSupport();
}
#endif // DISABLE_CPPRESTSDK

AddParams::ParameterSelectionSystem& NFmiEditMapGeneralDataDoc::ParameterSelectionSystem()
{
    return pimpl->ParameterSelectionSystem();
}

void NFmiEditMapGeneralDataDoc::UpdateParameterSelectionSystem()
{
    pimpl->UpdateParameterSelectionSystem();
}

bool NFmiEditMapGeneralDataDoc::LoadStaticHelpData(void)
{
    return pimpl->LoadStaticHelpData();
}

void NFmiEditMapGeneralDataDoc::SetLastActiveDescTopAndViewRow(unsigned int theDescTopIndex, int theActiveRowIndex)
{
    pimpl->SetLastActiveDescTopAndViewRow(theDescTopIndex, theActiveRowIndex);
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiEditMapGeneralDataDoc::GetMosTemperatureMinAndMaxData()
{
    return pimpl->GetMosTemperatureMinAndMaxData();
}

NFmiBasicSmartMetConfigurations& NFmiEditMapGeneralDataDoc::BasicSmartMetConfigurations()
{
    return pimpl->BasicSmartMetConfigurations();
}

void NFmiEditMapGeneralDataDoc::UpdateViewForOffScreenDraw(unsigned int theMapViewDescTopIndex)
{
    pimpl->UpdateViewForOffScreenDraw(theMapViewDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::DoOffScreenDraw(unsigned int theMapViewDescTopIndex, CBitmap &theDrawedScreenBitmapOut)
{
    pimpl->DoOffScreenDraw(theMapViewDescTopIndex, theDrawedScreenBitmapOut);
}

Q2ServerInfo& NFmiEditMapGeneralDataDoc::GetQ2ServerInfo()
{
    return pimpl->GetQ2ServerInfo();
}

Warnings::CapDataSystem& NFmiEditMapGeneralDataDoc::GetCapDataSystem()
{
    return pimpl->GetCapDataSystem();
}

void NFmiEditMapGeneralDataDoc::UpdateRowInLockedDescTops(unsigned int theOrigDescTopIndex)
{
    pimpl->UpdateRowInLockedDescTops(theOrigDescTopIndex);
}

int NFmiEditMapGeneralDataDoc::GetTimeRangeForWarningMessagesOnMapViewInMinutes()
{
    return pimpl->GetTimeRangeForWarningMessagesOnMapViewInMinutes();
}

bool NFmiEditMapGeneralDataDoc::MakeControlPointAcceleratorAction(ControlPointAcceleratorActions action, const std::string &updateMessage)
{
    return pimpl->MakeControlPointAcceleratorAction(action, updateMessage);
}

void NFmiEditMapGeneralDataDoc::InitGriddingProperties()
{
    pimpl->InitGriddingProperties();
}

NFmiMacroParamDataCache& NFmiEditMapGeneralDataDoc::MacroParamDataCache()
{
    return pimpl->MacroParamDataCache();
}

void NFmiEditMapGeneralDataDoc::DoMapViewOnSize(int mapViewDescTopIndex, const NFmiPoint &totalPixelSize, const NFmiPoint &clientPixelSize)
{
    pimpl->DoMapViewOnSize(mapViewDescTopIndex, totalPixelSize, clientPixelSize);
}
