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
#include "CFmiOperationProgressAndCancellationDlg.h"
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
#include "TimeSerialParameters.h"
#include "NFmiColorContourLegendSettings.h"
#include "NFmiCombinedMapHandler.h"
#include "NFmiFastDrawParamList.h"
#include "NFmiParameterInterpolationFixer.h"
#include "NFmiQueryDataKeeper.h"
#include "NFmiSeaLevelPlumeData.h"
#include "NFmiDataModifierModMinMax.h"
#include "NFmiDataModifierModAvg.h"
#include "NFmiLedLightStatus.h"
#include "NFmiTempDataGenerator.h"
#include "FmiDataLoadingThread2.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "ProducerData.h"
#include "NFmiMouseClickUrlActionData.h"
#include "NFmiVirtualTimeData.h"
#include "AnimationProfiler.h"

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
#include "boost/algorithm/string.hpp"

#ifdef _MSC_VER
#pragma warning (disable : 4244 4267) // boost:in thread kirjastosta tulee ik‰v‰sti 4244 varoituksia
#endif
#include <boost/thread.hpp>
#include "execute-command-in-separate-process.h"

#ifndef DISABLE_CPPRESTSDK
#include "wmssupport/WmsSupport.h"
#include "wmssupport/CapabilitiesHandler.h"
#include "wmssupport/CapabilityTree.h"
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
    const NFmiMetEditorTypes::View g_DefaultParamView = g_DefaultParamView;

	// Kartta-layereiden suoravalinta k‰ytt‰‰ n‰yt‰ arvoja kertoakseen menuIten oliolle onko
	// valittu karttapohja tiedosto vai Wms pohjainen.
	const double g_mapLayerSelectionIsFileBased = 987654321.01;
	const double g_mapLayerSelectionIsWmsBased = 987654321.02;
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



static const std::string g_ObservationMenuName = "Observation";
static const std::string gDummyParamName = "dummyName";
static const NFmiPoint kMissingLatlon(kFloatMissing, kFloatMissing);
// T‰m‰n pit‰‰ olla pieni arvo (~epsilon) koska muuten pienet rajat eiv‰t toimi, mutta pelkk‰ epsilon on liian pieni
static const double gUsedEpsilon = std::numeric_limits<double>::epsilon() * 3; 

using namespace std;


// **********************************************************
// ************ UUSI PIMPL idiooma **************************
// **********************************************************

class GeneralDocImpl
{
public:

GeneralDocImpl(unsigned long thePopupMenuStartId)
:itsListHandler(0)
,itsSmartInfoOrganizer(0)
,fOpenPopup(false)
,itsPopupMenu()
,itsPopupMenuStartId(thePopupMenuStartId)
,itsLocationSelectionTool2(0)
,itsFileDialogDirectoryMemory()
,itsHelpDataInfoSystem()
,itsCaseStudyHelpDataInfoSystem()
,fCaseStudyModeOn(false)
,itsDataLoadingInfoNormal()
,itsDataLoadingInfoCaseStudy()
,fUseMasksInTimeSerialViews(true)
,itsToolTipRealRowIndex(-1)
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
,itsLastBrushedViewRealRowIndex(0)
,fUseTimeInterpolation(false)
,itsMetEditorOptionsData()
,itsCPManagerSet()
,fPrinting(false)
,itsCurrentViewRowIndex(1)
,itsCurrentCrossSectionRowIndex(1)
,fCPDataBackupUsed(false)
,itsBrushToolLimitSetting(0)
,itsBrushToolLimitSettingValue(0)
,itsShowHelperDataInTimeSerialView(1) // 1= n‰yt‰ apu dataa aikasarjassa
,fShowHelperData3InTimeSerialView(false)
,fShowHelperData4InTimeSerialView(false)
,fActivateParamSelectionDlgAfterLeftDoubleClick(false)
,itsDrawDifferenceDrawParam()
,itsSelectedGridPointDrawParam()
,itsSelectedGridPointLimit(100)
,itsSmartToolInfo()
,itsHelperViewMacro()
,itsViewMacroPath()
,itsRootViewMacroPath()
,itsCurrentViewMacro(new NFmiViewSettingMacro())
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
,itsTransparencyContourDrawView(0)
,itsSynopDataFilePatternSortOrderVector()
,itsShowToolTipTimeView(true)
,itsShowToolTipTempView(true)
,itsShowToolTipTrajectoryView(true)
,itsMapViewTimeLabelInfo()
,itsTimeSerialViewTimeBag()
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
,itsMacroPathSettings()
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
,parameterSelectionSystem()
,profiler()
,profiling(false)
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
	delete itsLocationSelectionTool2;
	delete itsClipBoardData;

	delete itsPossibleUsedDataLoadingGrid;
	delete itsTrajectorySystem;
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

void TestDataModifiers()
{
	NFmiDataModifierModMinMax::DoSomeTestRoutines();
	NFmiDataModifierModAvg::DoSomeTestRoutines();
}

bool Init(const NFmiBasicSmartMetConfigurations &theBasicConfigurations, std::map<std::string, std::string> &mapViewsPositionMap, std::map<std::string, std::string> &otherViewsPositionPosMap)
{
/* // t‰m‰ on remote debug sessiota varten MSVC71 feature
	for(int remoteDebug = 1; remoteDebug == 1; )
	{
		int x = 1;
	}
*/
//	TestDataModifiers();

	itsBasicConfigurations = theBasicConfigurations; // kopsataan CSmartMetApp:issa alustettu perusasetus GenDocin dataosaan
	CombinedMapHandlerInterface::verboseLogging(itsBasicConfigurations.Verbose());

	SetupQueryDataSetKeeperCallbacks();

	LogMessage("SmartMet document initialization starts...", CatLog::Severity::Info, CatLog::Category::Configuration);
	// Laitetaan peruskonffihakemisto lokiin n‰kyviin
	string infoStr("SmartMet uses configurations from base configuration file: ");
	infoStr += itsBasicConfigurations.BaseConfigurationFilePath();
	LogMessage(infoStr, CatLog::Severity::Info, CatLog::Category::Configuration);

    InitSpecialFileStoragePath();
	// Tarkistaa, onko edellisen kerran ohjelma suljettu hallitusti
    CheckRunningStatusAtStartup(SpecialFileStoragePath()); 

	InitMachineThreadCount();
	InitMacroPathSettings(); // pit‰‰ olla ennen InfoOrganizer + SmartToolInfo + ViewMacroSystem alustuksia!!!
	InitInfoOrganizer(); // pit‰‰ alustaa ennen InitSettingsFromConfFile-metodia
	InitHelpEditorSystem(); // t‰t‰ pit‰‰ kutsua ennen InitHelpDataInfoSystem-metodia
	InitHelpDataInfoSystem(); // T‰t‰ pit‰‰ kutsua ennen InitApplicationWinRegistry ja InitCaseStudySystem -kutsuja!
    InitApplicationWinRegistry(mapViewsPositionMap, otherViewsPositionPosMap);

	InitSettingsFromConfFile();
    MakeMultiProcessLogPathValues();
    InitSynopStationPrioritySystem();
    InitSynopPlotSettings();
    InitObsComparisonInfo();
    InitDataToDBCheckMethodMain();
	InitOptionsData();
	// InitLedLightStatusSystem kutsu pit‰‰ olla InitApplicationWinRegistry:n j‰lkeen, mutta ennen InitCombinedMapHandler:ia.
	InitLedLightStatusSystem();

    CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting("NFmiEditMapDataListHandler::Init");
	itsListHandler = new NFmiEditMapDataListHandler();
	itsListHandler->Init(HelpDataPath());

    InitFixedDrawParamSystem();
	InitCombinedMapHandler();
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

	InitLocationSelectionTool();

	InitEditedDataParamDescriptor(); // pit‰‰ olla itsDataLoadingInfoManager -otuksen luomisen j‰lkeen
	InitWarningCenterSystem(); // t‰m‰n initialisointi pit‰‰ olla itsDataLoadingInfoManager-olion initialisoinnin per‰ss‰
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
	InitMouseClickUrlActionData();

	// Asetetaan alustuksessa jonkinlainen timeDescriptor, ennen kuin ladataan mit‰‰n datoja
	NFmiMetTime origTime;
	NFmiMetTime endTime(origTime);
	endTime.ChangeByDays(2);
	NFmiTimeBag validTimebag(origTime, endTime, 60);
	NFmiTimeDescriptor timeDesc(origTime, validTimebag);
	SetCrossSectionSystemTimes(timeDesc);
    InitCapData();

    // This must be initialized AFTER all other data systems have been initialized
    // Meaning after modelProducers, obsProducers, satelProducers, wsmSupport, smarttools, etc. have initialized
    InitParameterSelectionSystem();
    InitLogFileCleaning();
    InitMacroParamDataCache();
	InitTimeSerialParameters();
    InitColorContourLegendSettings();
	InitParameterInterpolationFixer();
	InitSeaLevelPlumeData();
	UpdateMacroParamDataGridSizeAfterVisualizationOptimizationsChanged();

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

void InitSeaLevelPlumeData()
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsSeaLevelPlumeData.InitFromSettings("SeaLevelPlumeData");
		LogMessage(itsSeaLevelPlumeData.baseConfigurationMessage(), CatLog::Severity::Debug, CatLog::Category::Configuration);
		
		if(!itsSeaLevelPlumeData.configurationErrorMessage().empty())
		{
			LogMessage(itsSeaLevelPlumeData.configurationErrorMessage(), CatLog::Severity::Error, CatLog::Category::Configuration);
		}
	}
	catch(exception& e)
	{
		LogAndWarnUser(e.what(), "Problems with SeaLevelPlumeData initialization", CatLog::Severity::Error, CatLog::Category::Configuration, true, false, false);
	}
}


void SetupQueryDataSetKeeperCallbacks()
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	// Laitetaan mallidata-ajo systeemin loggaus callbackit kuntoon, ett‰ voidaan tutkia, 
	// miksi joskus viittaukset edellisiin malliajoihin ep‰onnistuvat.
	try
	{
		TraceLogMessageCallback traceLogCallback = [](const std::string& message) {CatLog::logMessage(message, CatLog::Severity::Trace, CatLog::Category::Data); };
		NFmiQueryDataSetKeeper::SetTraceLogMessageCallback(traceLogCallback);
		IsTraceLoggingInUseCallback isTraceLoggingInUseCallback = []() {return CatLog::doTraceLevelLogging(); };
		NFmiQueryDataSetKeeper::SetIsTraceLoggingInUseCallback(isTraceLoggingInUseCallback);
	}
	catch(exception& e)
	{
		LogAndWarnUser(e.what(), "Problems with SetupQueryDataSetKeeperCallbacks initialization", CatLog::Severity::Error, CatLog::Category::Configuration, true, false, false);
	}
}

void InitParameterInterpolationFixer()
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsParameterInterpolationFixer.init();
	}
	catch(exception& e)
	{
		LogAndWarnUser(e.what(), "Problems with ParameterInterpolationFixer initialization", CatLog::Severity::Error, CatLog::Category::Configuration, true, false, false);
	}
}

void InitMacroParamDataCache()
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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

void InitCapData()
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	// Let's clean log directory so that there is max 30 days old files
	auto baseLogFilePath = CatLog::baseLogFilePath();
	auto directoryPart = PathUtils::getPathSectionFromTotalFilePath(baseLogFilePath);
    NFmiDirectorCleanerInfo info(directoryPart, 30);
    FileCleanerSystem().Add(info);
}

/*
void LogWallclockVsStartTime_Debug(const NFmiMetTime& usedWallClockTime, const NFmiMetTime& usedCurrentTime, const NFmiMetTime& startTime)
{
	std::string logMessage = usedWallClockTime.ToStr("Wallclock: YYYY.MM.DD HH:mm ==> ", kEnglish);
	logMessage += usedCurrentTime.ToStr("CurrentTime: YYYY.MM.DD HH:mm ==> ", kEnglish);
	logMessage += startTime.ToStr("StartTime: YYYY.MM.DD HH:mm", kEnglish);
	LogMessage(logMessage, CatLog::Severity::Debug, CatLog::Category::Operational, true);
}

NFmiMetTime CalcStartTime_Debug(const NFmiMetEditorModeDataWCTR::TimeSectionData& theStartSection, const NFmiMetTime& theCurrentTime)
{
	NFmiMetTime startTime(theCurrentTime);
	startTime.SetTimeStep(theStartSection.itsStartTimeResolutionInMinutes);
	if(startTime > theCurrentTime)
		startTime.PreviousMetTime();
	if(theStartSection.itsStartTimeResolutionInMinutes <= 60)
	{
		startTime.PreviousMetTime(); // laitetaan koko jutun (1. osion 1h aikaresoluutio-alue) aloitusaika aina niin, ett‰ se on yhden tunnin ennen currenttia aikaa
	}
	return startTime;
}

void LogStartTimesForDifferentMinutes_Debug(const NFmiMetEditorModeDataWCTR::TimeSectionData& theStartSection)
{
	NFmiMetTime usedWallClockTime;
	usedWallClockTime.SetTimeStep(1);
	for(; ; usedWallClockTime.NextMetTime())
	{
		NFmiMetTime usedCurrentTime(usedWallClockTime, 60);
		NFmiMetTime startTime(CalcStartTime_Debug(theStartSection, usedCurrentTime));
		LogWallclockVsStartTime_Debug(usedWallClockTime, usedCurrentTime, startTime);
		if(usedWallClockTime.GetMin() == 59)
			break;
	}
}
*/

void InitMetEditorModeDataWCTR()
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	NFmiMetEditorModeDataWCTR* metEdModeData = EditorModeDataWCTR();
	if(metEdModeData && metEdModeData->EditorMode() != NFmiMetEditorModeDataWCTR::kNormalAutoLoad)
	{
		metEdModeData->UseNormalModeForAWhile(true);
//		LogStartTimesForDifferentMinutes_Debug(metEdModeData->TimeSections().at(1));
	}
}

void InitDataLoadingInfo()
{
    try
    {
        itsDataLoadingInfoNormal.Configure(WorkingDirectory(), itsHelpDataInfoSystem.LocalDataBaseDirectory(), itsHelpDataInfoSystem.LocalDataLocalDirectory(), itsHelpDataInfoSystem.UseQueryDataCache());
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
		CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
		CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
		itsSynopPlotSettings.Init();
    }
    catch(exception &e)
    {
        LogAndWarnUser(e.what(), "Problems with SynopPlot settings", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
    }
}

void InitSynopStationPrioritySystem()
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
        GetMTATempSystem().Init(ProducerSystem(), ExtraSoundingProducerList(), ApplicationWinRegistry().GetSoundingViewSettings());
    }
    catch(std::exception &e)
    {
        LogAndWarnUser(string("Sounding settings problem: ") + e.what(), "Problems with sounding view settings", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
    }
}

void InitFixedDrawParamSystem(void)
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
    {
        auto fixedDrawParamInitRootPath = MakeUsedFixedDrawParamsRootPath();
        // Laitetaan 'siivottu' polku takaisin, sit‰ ei laiteta minnek‰‰n talteen, mutta se n‰ytet‰‰n Settings dialogissa
        ApplicationWinRegistry().FixedDrawParamsPath(fixedDrawParamInitRootPath);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
    {
        BetaProduct::SetLoggerFunction(GetLogAndWarnFunction());
        itsBetaProductionSystem.Init(ApplicationWinRegistry().BaseConfigurationRegistryPath(), WorkingDirectory(), BasicSmartMetConfigurations().BetaAutomationListPath());
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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

void InitMacroPathSettings(void)
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsMacroPathSettings.InitFromSettings("SmartMet::MacroPathSettings", itsBasicConfigurations.WorkingDirectory());
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
//		itsCaseStudySystem.InitFromSettings();
		if(HelpDataInfoSystem()->UseQueryDataCache())
		{
			itsCaseStudySystem.SmartMetLocalCachePath(HelpDataInfoSystem()->LocalDataLocalDirectory());
		}
		itsCaseStudySystem.Init(*HelpDataInfoSystem(), *InfoOrganizer(), ApplicationWinRegistry().CaseStudySettingsWinRegistry());

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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	// Siivoa 1 vrk vanhemmat datat 
	NFmiDirectorCleanerInfo dCleanInfo(static_cast<char*>(GetUsedDataLoadingInfo().WorkingPath()), 1);
	FileCleanerSystem().Add(dCleanInfo);

	// ja j‰t‰ vain max 25 tiedostoa
	NFmiFilePatternCleanerInfo pattCleanInfo(static_cast<char*>(GetUsedDataLoadingInfo().CreateWorkingFileNameFilter()), 20);
	FileCleanerSystem().Add(pattCleanInfo);
}

void InitLedLightStatusSystem()
{
	if(ApplicationWinRegistry().UseLedLightStatusSystem())
	{
		// Katso g_maximumNumberOfLedsInStatusbar vakion selitys, jos haluat lis‰t‰ ledien m‰‰r‰‰
		auto usedLedChannelAndColors = std::vector<NFmiLedChannelInitializer>
		{
			{NFmiLedChannel::QueryData, NFmiLedColor::Green, "QueryData related operations", "No queryData operations at the moment", false},
			{NFmiLedChannel::WmsData, NFmiLedColor::Blue, "Wms server query operations", "No Wms operations at the moment", false},
			{NFmiLedChannel::OperationalInfo, NFmiLedColor::Red, "General operational warnings", "No operational warnings at the moment", true},
			{NFmiLedChannel::DataIsLate, NFmiLedColor::Orange, "Data is late warnings", "No data is late at the moment", false}
		};
		itsLedLightStatusSystem.Initialize(usedLedChannelAndColors, true);
		NFmiLedLightStatusSystem::InitializeStaticInstance(&itsLedLightStatusSystem);
	}
	else
		LogAndWarnUser("LedLightStatusSystem (on statusbar) was disabled, if you want to enabled it, do it from Settings dialog and restart SmartMet", "", CatLog::Severity::Info, CatLog::Category::Configuration, true);
}

void InitMouseClickUrlActionData()
{
	itsMouseClickUrlActionData.InitFromSettings("SmartMet::MouseClickActionData");
}

void InitSynopDataFilePatternSortOrderVector(void)
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		// t‰m‰ pit‰‰ tehd‰ dokumentin initialisoinnin yhteydess‰, ett‰ smarttoolintepreterin tuottaja listat alustetaan varmasti oikein!!!!
        NFmiSmartToolIntepreter intepreter(&itsProducerSystem, &itsObsProducerSystem);
		// T‰m‰ korjaa mm. SymbolTooltipFile asetukset kuntoon, jos skriptiss‰ polku annetaan ilman drive-letter:i‰,
		// mik‰ onkin jatkossa suositeltavaa.
		NFmiSmartToolIntepreter::SetAbsoluteBasePaths(MacroPathSettings().SmartToolPath(), MacroPathSettings().MacroParamPath());
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
        std::string shortAppVerStr = GetShortAppVersionString();
        if(shortAppVerStr.size() < 3)
            throw std::runtime_error(std::string("Invalid application's short version number: '") + shortAppVerStr + "', should be in format X.Y");
        // 3 = 3 eri karttan‰yttˆ‰, ei viel‰k‰‰n miss‰‰n asetusta t‰lle, koska p‰‰karttan‰yttˆ poikkeaa kahdesta apukarttan‰ytˆst‰
        itsApplicationWinRegistry.Init(ApplicationDataBase().appversion, shortAppVerStr, itsBasicConfigurations.GetShortConfigurationName(), 3, mapViewsPositionMap, otherViewsPositionPosMap, *HelpDataInfoSystem(), NFmiCaseStudySystem::GetCategoryHeaders());

		// V‰h‰n nurinkurisesti t‰ss‰ asetetaan rekisterist‰ yksi arvo edelleen pariin paikkaan (mm. takaisin itseens‰, mutta modulaarisuus vaatii t‰m‰n)
		ApplicationInterface::GetApplicationInterfaceImplementation()->SetHatchingToolmasterEpsilonFactor(itsApplicationWinRegistry.HatchingToolmasterEpsilonFactor());

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
    itsApplicationWinRegistry.CaseStudySettingsWinRegistry().HelpDataEnableWinRegistry().Update(*HelpDataInfoSystem());
}

void InitConceptualModelData(void)
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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

void InitCombinedMapHandler()
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	// parameterSelectionUpdateCallback varmistaa ett‰ kun kaikki Wms serverit on k‰yty 1. kerran l‰pi, 
	// t‰m‰n j‰lkeen p‰ivitet‰‰n Parameter selection dialogi, jotta siell‰ n‰kyy Wms datat (jos dialogi avattu
	// ennen t‰t‰, j‰i dialogi ilman Wms datoja ennen kuin jokin muu eventti teki kunnon updaten).
	std::function<void()> parameterSelectionUpdateCallback = []() {ApplicationInterface::GetApplicationInterfaceImplementation()->SetToDoFirstTimeWmsDataBasedUpdate(); };
	Wms::CapabilitiesHandler::setParameterSelectionUpdateCallback(parameterSelectionUpdateCallback);
	// NFmiCombinedMapHandler luokka hanskaa itse kaikki poikkeukset ja mahdolliset k‰ytt‰j‰n tekem‰t ohjelman lopetukset.
	itsCombinedMapHandler.initialize(itsBasicConfigurations.ControlPath());
	if(!itsCombinedMapHandler.wmsSupportAvailable())
	{
		// Jos wms systeemi‰ ei oteta k‰yttˆˆn, pit‰‰ lopettaa timer joka odottaa 1. data p‰ivityst‰ joka 3. sekunti
		parameterSelectionUpdateCallback();
	}
}

void InitProducerSystem(NFmiProducerSystem &producerSystem, const std::string &baseConfigurationKey, const std::string &callingFunctionName)
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
        itsWarningCenterSystem.initialize(itsBasicConfigurations.ControlPath());
	}
	catch(std::exception &e)
	{
		string errStr("InitWarningCenterSystem - Initialization error in configurations: \n");
		errStr += e.what();
        LogAndWarnUser(errStr, "Problems in InitWarningCenterSystem", CatLog::Severity::Error, CatLog::Category::Configuration, false, true);
	}
#endif // DISABLE_CPPRESTSDK
}

void InitWindTableSystem(void)
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		string minGridStr = NFmiSettings::Require<string>("MetEditor::MacroParamDataMinGridSize");
		string maxGridStr = NFmiSettings::Require<string>("MetEditor::MacroParamDataMaxGridSize");

		std::vector<int> minGridValues = NFmiStringTools::Split<std::vector<int> >(minGridStr, ",");
		std::vector<int> maxGridValues = NFmiStringTools::Split<std::vector<int> >(maxGridStr, ",");
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
		std::vector<int> values = NFmiStringTools::Split<std::vector<int> >(gridStr, ",");
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
				std::vector<std::string> valuesStr = NFmiStringTools::Split<std::vector<std::string> >(paramsStr, ",");
				std::vector<std::string>::iterator it = valuesStr.begin();
				std::vector<std::string>::iterator endIt = valuesStr.end();
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
    std::vector<std::string> nameParts = NFmiStringTools::Split<std::vector<std::string> >(theParamName, "{");
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
			std::vector<double> values = NFmiStringTools::Split<std::vector<double> >(gridStr, ",");
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsSmartMetEditingMode = static_cast<CtrlViewUtils::FmiSmartMetEditingMode>(NFmiSettings::Require<int>("MetEditor::EditXMode"));
		fWarnIfCantSaveWorkingFile = NFmiSettings::Optional("MetEditor::WarnIfCantSaveWorkingFile", true);

		fUseOnePressureLevelDrawParam = NFmiSettings::Optional("MetEditor::UseOnePressureLevelDrawParam", false);
		fRawTempRoundSynopTimes = NFmiSettings::Optional("MetEditor::RawTempRoundSynopTimes", false);
		string rawTempUnknownStartLonLatStr = NFmiSettings::Optional<string>("MetEditor::RawTempUnknownStartLonLat", "0,0");
		std::vector<double> rawTempUnknownStartLonLatStrVec = NFmiStringTools::Split<std::vector<double> >(rawTempUnknownStartLonLatStr, ",");
		if(rawTempUnknownStartLonLatStrVec.size() != 2)
			throw runtime_error("InitSettingsFromConfFile MetEditor::RawTempUnknownStartLonLat was invlid, has to be two numbers (like x,y).");
		itsRawTempUnknownStartLonLat = NFmiPoint(rawTempUnknownStartLonLatStrVec[0], rawTempUnknownStartLonLatStrVec[1]);
		itsStationDataGridSize = SettingsFunctions::GetCommaSeparatedPointFromSettings("MetEditor::StationDataGridSize");

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
		SettingsFunctions::SetCommaSeparatedPointToSettings("MetEditor::StationDataGridSize", itsStationDataGridSize);
		NFmiSettings::Set("SmartMet::SatelDataRefreshTimerInMinutes", NFmiStringTools::Convert<int>(itsSatelDataRefreshTimerInMinutes), true);
        itsQ2ServerInfo.StoreToSettings();
	}
	catch(exception &e)
	{
        ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(e.what()), _TEXT("Problems with InitSettingsFromConfFile!"), MB_OK);
	}
}

void InitColorContourLegendSettings()
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
    {
        itsColorContourLegendSettings.initFromSettings("SmartMet::ColorContourLegendSettings");
    }
    catch(exception& e)
    {
        ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(e.what()), _TEXT("Problems with InitColorContourLegendSettings!"), MB_OK);
    }
}

void InitMacroParamSystem(bool haveAbortOption)
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsMacroParamSystem.RootPath(itsMacroPathSettings.MacroParamPath());
		NFmiFileSystem::CreateDirectory(itsMacroParamSystem.RootPath()); // luodaan varmuuden vuoksi hakemisto, jos ei ole jo

	}
	catch(exception &e)
	{
        LogAndWarnUser(e.what(), "Error with MacroParam system", CatLog::Severity::Error, CatLog::Category::Configuration, false, haveAbortOption);
	}
}

void InitViewMacroSystem(bool haveAbortOption)
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsViewMacroPath = itsMacroPathSettings.ViewMacroPath();
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

void InitDrawDifferenceDrawParam(void)
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	if(itsSmartInfoOrganizer)
	{
		// korjaa t‰m‰ kohta niin ett‰ tulee kolmiv‰rinen isoviiva esitys!!!
		itsDrawDifferenceDrawParam = itsSmartInfoOrganizer->CreateDrawParam(NFmiDataIdent(NFmiParam(NFmiInfoData::kFmiSpDrawDifferenceParam, "Diff to orig")), 0, NFmiInfoData::kAnyData);
		itsDrawDifferenceDrawParam->SimpleIsoLineColorShadeClassCount(3);
		itsDrawDifferenceDrawParam->SimpleIsoLineLabelHeight(3.1f);
		itsDrawDifferenceDrawParam->UseSingleColorsWithSimpleIsoLines(false);
		itsDrawDifferenceDrawParam->SimpleIsoLineColorShadeLowValue(-0.1f);
		itsDrawDifferenceDrawParam->SimpleIsoLineColorShadeMidValue(0);
		itsDrawDifferenceDrawParam->SimpleIsoLineColorShadeHighValue(0.1f);
		itsDrawDifferenceDrawParam->SimpleIsoLineColorShadeLowValueColor(NFmiColor(0.f,0.f,1.f));
		itsDrawDifferenceDrawParam->SimpleIsoLineColorShadeMidValueColor(NFmiColor(1.f,0.f,1.f));
		itsDrawDifferenceDrawParam->SimpleIsoLineColorShadeHighValueColor(NFmiColor(1.f,0.f,0.f));
		itsDrawDifferenceDrawParam->StationDataViewType(NFmiMetEditorTypes::View::kFmiIsoLineView);

		// Laitoin alustamaan myˆs valittujen hilapisteiden uuden visualisointi piirron
		itsSelectedGridPointDrawParam = itsSmartInfoOrganizer->CreateDrawParam(NFmiDataIdent(NFmiParam(NFmiInfoData::kFmiSpSelectedGridPoints, "Selected grid points")), 0, NFmiInfoData::kAnyData);
		if(itsSelectedGridPointDrawParam)
		{
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
    if(freeDriveSpaceInProcents < 2.5)
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

void DoLedLightFreeSpaceReport(char driveLetter, std::string logMessage, CatLog::Severity severity)
{
	// C ja D asemilta tehd‰‰n eri raportit
	std::string reporterName = "HardDriveUsage-" + driveLetter;
	if(severity <= CatLog::Severity::Info)
		NFmiLedLightStatusSystem::StopReportToChannelFromThread(NFmiLedChannel::OperationalInfo, reporterName);
	else
	{
		boost::replace_all(logMessage, "\t", "");
		NFmiLedLightStatusSystem::ReportToChannelFromThread(NFmiLedChannel::OperationalInfo, reporterName, logMessage, severity);
	}
}

void ReportHardDriveUsage(const std::string &driveDescription, char driveLetter)
{
	CatLog::Severity logSeverity = CatLog::Severity::Debug;
	std::string hardDriveReportStr;
    double freeGigaBytesAvailable = 0;
    double totalNumberOfGigaBytes = 0;
    double freeDriveSpaceInProcents = 0;
    if(GetHardDriveInfo(driveLetter, freeGigaBytesAvailable, totalNumberOfGigaBytes, freeDriveSpaceInProcents))
    {
		logSeverity = GetHardDriveFreeSpaceLogSeverity(freeDriveSpaceInProcents);
		hardDriveReportStr = GetHardDriveReportString(driveDescription, freeGigaBytesAvailable, freeDriveSpaceInProcents);
    }
	else
	{
		logSeverity = CatLog::Severity::Error;
		hardDriveReportStr = driveDescription + "Unable to get info from " + driveLetter + " drive";
	}
	DoLedLightFreeSpaceReport(driveLetter, hardDriveReportStr, logSeverity);
	LogSystemInfo(hardDriveReportStr, logSeverity);
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
    LogSystemInfo("SmartMet harddrive usage report:");
    LogSystemInfo("------------------------------------------------------");
    ReportSystemHardDriveUsage();
    ReportSmartMetHardDriveUsage();
    LogSystemInfo("------------------------------------------------------");
}

void LogSystemInfo(std::string message, CatLog::Severity logSeverity = CatLog::Severity::Debug, CatLog::Category logCategory = CatLog::Category::Operational, bool flushLog = true)
{
    LogMessage(message, logSeverity, logCategory, flushLog);
}

void ReportProcessMemoryUsage(void)
{
    ReportHardDriveUsage();

	const double megabyte = 1024. * 1024.;
	DWORD currentProcessId = GetCurrentProcessId();
	HANDLE hProcess;
	PROCESS_MEMORY_COUNTERS pmc;
	// Print information about the memory usage of the process.
	hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, currentProcessId);
	if (NULL == hProcess)
		return;

	if (GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)))
	{
        LogSystemInfo("SmartMet Memory usage report:");
        LogSystemInfo("------------------------------------------------------");
        LogSystemInfo(std::string("\t\tPageFaultCount: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(pmc.PageFaultCount/megabyte, 2)) + " MB");
        LogSystemInfo(std::string("\t\tPeakWorkingSetSize: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(pmc.PeakWorkingSetSize/megabyte, 1)) + " MB");
        LogSystemInfo(std::string("\t\tWorkingSetSize: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(pmc.WorkingSetSize/megabyte, 1)) + " MB");
        LogSystemInfo(std::string("\t\tQuotaPeakPagedPoolUsage: \t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(pmc.QuotaPeakPagedPoolUsage/megabyte, 2)) + " MB");
        LogSystemInfo(std::string("\t\tQuotaPagedPoolUsage: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(pmc.QuotaPagedPoolUsage/megabyte, 2)) + " MB");
        LogSystemInfo(std::string("\t\tQuotaPeakNonPagedPoolUsage: \t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(pmc.QuotaPeakNonPagedPoolUsage/megabyte, 2)) + " MB");
        LogSystemInfo(std::string("\t\tQuotaNonPagedPoolUsage: \t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(pmc.QuotaNonPagedPoolUsage/megabyte, 2)) + " MB");
        LogSystemInfo(std::string("\t\tPagefileUsage: \t\t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(pmc.PagefileUsage/megabyte, 1)) + " MB");
        LogSystemInfo(std::string("\t\tPeakPagefileUsage: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(pmc.PeakPagefileUsage/megabyte, 1)) + " MB");
        LogSystemInfo("------------------------------------------------------");
	}
	CloseHandle( hProcess );
	ReportSystemMemoryUsage();
}

void ReportSystemMemoryUsage(void)
{
    const double megabyte = 1024. * 1024.;
    const double gigabyte = megabyte * 1024.;
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx (&statex);

    LogSystemInfo("Computers memory usage report:");
    LogSystemInfo("------------------------------------------------------");
    LogSystemInfo(std::string("\t\tMemory in use: \t\t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(statex.dwMemoryLoad, 0)) + " %");
    LogSystemInfo(std::string("\t\tTotal physical memory: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(statex.ullTotalPhys/megabyte, 0)) + " MB");
    LogSystemInfo(std::string("\t\tFree physical memory: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(statex.ullAvailPhys/megabyte, 1)) + " MB");
    LogSystemInfo(std::string("\t\tTotal paging file: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(statex.ullTotalPageFile/megabyte, 0)) + " MB");
    LogSystemInfo(std::string("\t\tFree paging file: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(statex.ullAvailPageFile/megabyte, 1)) + " MB");
    LogSystemInfo(std::string("\t\tTotal virtual memory: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(statex.ullTotalVirtual/gigabyte, 0)) + " GB");
    LogSystemInfo(std::string("\t\tFree virtual memory: \t\t") + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(statex.ullAvailVirtual/gigabyte, 1)) + " GB");
    LogSystemInfo("------------------------------------------------------");
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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

void InitParamMaskList(void)
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	boost::shared_ptr<NFmiAreaMaskList> paramMaskList(new NFmiAreaMaskList());
	ParamMaskListMT(paramMaskList);
}

bool InitInfoOrganizer(void)
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	if(itsSmartInfoOrganizer)
		delete itsSmartInfoOrganizer;
	itsSmartInfoOrganizer = new NFmiInfoOrganizer;
	itsSmartInfoOrganizer->WorkingDirectory(WorkingDirectory());
	int undoredoDepth = (SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal) ? itsMetEditorOptionsData.UndoRedoDepth() : 0; // ns. viewmodessa undo/redo syvyydeksi 0!
	bool makeCopyOfEditedData = undoredoDepth > 0;
	itsSmartInfoOrganizer->Init(itsMacroPathSettings.DrawParamPath(), false, makeCopyOfEditedData, fUseOnePressureLevelDrawParam); // 2. parametri (false) tarkoittaa ett‰ jos drawparam-tiedostoja ei ole, ei niit‰ luoda automaattisesti
	NFmiFileSystem::CreateDirectory(itsSmartInfoOrganizer->GetDrawParamPath());

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

void LogHelpDataInfoSystemCombineDataConfigurationProblems()
{
	for(const auto& helpDataInfo : itsHelpDataInfoSystem.DynamicHelpDataInfos())
	{
		if(!helpDataInfo.CombineDataErrorMessage().empty())
		{
			LogAndWarnUser(helpDataInfo.CombineDataErrorMessage(), "", CatLog::Severity::Error, CatLog::Category::Configuration, true);
		}
	}
}

bool InitHelpDataInfoSystem(void)
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		itsHelpDataInfoSystem.InitFromSettings("MetEditor::HelpData", itsBasicConfigurations.ControlPath(), CreateHelpEditorFileNameFilter(), StripFilePathAndExtension(itsHelpEditorSystem.FileNameBase()));
		LogHelpDataInfoSystemCombineDataConfigurationProblems();
		// T‰m‰ caseStudy dataan liittyv‰ alustus pit‰‰ tehd‰ heti itsHelpDataInfoSystem:in alustuksen j‰lkeen...
		NFmiCaseStudySystem::SetAllCustomFolderNames(itsHelpDataInfoSystem);
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

// CaseStudy systeemi tiet‰‰ nyky‰‰n kuinka monta tiedostoa halutaan s‰ilytt‰‰ lokaaleissa 
// cache hakemistoissa.
// HUOM! maxLatestDataCount on lukuna yhden pienempi kuin LocalCacheDataCount, joten se pit‰‰ v‰hent‰‰.
int GetMaxLatestDataCount(NFmiInfoData::Type theType, const std::string& theFileNameFilter)
{
	int maxLatestDataCount = 0;
	auto* helpDataInfo = HelpDataInfoSystem()->FindHelpDataInfo(theFileNameFilter);
	if(helpDataInfo)
	{
		auto* caseStudyDataInfo = CaseStudySystem().FindCaseStudyDataFile(helpDataInfo->Name());
		if(caseStudyDataInfo)
		{
			maxLatestDataCount = caseStudyDataInfo->DataFileWinRegValues().LocalCacheDataCount() - 1;
			if(maxLatestDataCount < 0)
				maxLatestDataCount = 0;
		}
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
		std::vector<boost::shared_ptr<NFmiFastQueryInfo> > fastInfoVector = itsSmartInfoOrganizer->GetInfos(theDataFilePattern);
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

bool ModifyParametersInterpolationToLinear(NFmiDataIdent& editedData)
{
    auto interpolationMethod = editedData.GetParam()->InterpolationMethod();
    if(interpolationMethod == kNearestPoint || interpolationMethod == kNoneInterpolation)
    {
        editedData.GetParam()->InterpolationMethod(kLinearly);
        return true;
    }
    return false;
}

// TotalWind yhdistelm‰ parametrilla on jostain syyst‰ valittu wind-vector metaparametrin 
// interpolaatioksi nearest, mik‰ on turhaa ja aiheuttaa ristiriitoja tuulen suunnan ja 
// nopeuksien kanssa, kun niit‰ interpoloidaan lineaarisesti. T‰m‰ on j‰lkik‰teen tehty
// dataan teht‰v‰ fiksaus ja t‰ss‰ asetetaan interpolaatio halutuksia, jos datasta lˆytyy total-wind.
void FixTotalWindsWindVectorInterpolation(NFmiQueryData* data, const std::string& theDataFileName)
{
	itsParameterInterpolationFixer.fixCheckedParametersInterpolation(data, theDataFileName);
}

bool IsDataReloadedInCaseStudyEvent(const std::string& theDataFilePattern)
{
  const auto helpData = HelpDataInfoSystem()->FindHelpDataInfo(theDataFilePattern);
  if(helpData)
	return helpData->ReloadCaseStudyData();
  else
	return true;
}

void AddQueryData(NFmiQueryData* theData, const std::string& theDataFileName, const std::string& theDataFilePattern,
			NFmiInfoData::Type theType, const std::string& theNotificationStr, bool loadFromFileState, bool& fDataWasDeletedOut, bool setCurrentTimeToNearestHour = false)
{
	StoreLastLoadedFileNameToLog(theDataFileName);
	if(theData == nullptr || theData->Info() == nullptr)
	{
		// HUOM! joskus tulee ehk‰ jonkin luku virheen takia queryData, jonka rawData on roskaa ja info on 0-pointteri. Sellainen data ignoorataan 
		// t‰ss‰, huom vuotaa muistia, koska en voi deletoida kun rawData-pointteri osoittaa ties minne.
		std::string errMessage = "Data loading failed, following data was read, but it was invalid and not used: ";
		errMessage += theDataFileName;
		LogMessage(errMessage, CatLog::Severity::Error, CatLog::Category::Data);
		return ;
	}

	NormalizeGridDataArea(theData);
    FixTotalWindsWindVectorInterpolation(theData, theDataFileName);
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
			auto reloadCaseStudyData = IsDataReloadedInCaseStudyEvent(theDataFilePattern);
			itsSmartInfoOrganizer->AddData(theData, theDataFileName, theDataFilePattern, theType, undoredoDepth, theMaxLatestDataCount, theModelRunTimeGap, fDataWasDeletedOut, reloadCaseStudyData);
			if(fDataWasDeletedOut)
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

		GetCombinedMapHandler()->makeNeededDirtyOperationsWhenDataAdded(theData, theType, removedDatasTimesOut, theDataFileName);
		DoLastEditedDataSendHasComeBackChecks(theType);
		UpdateParamMaskList(theType, theDataFilePattern, theData);

		if(theType == NFmiInfoData::kEditable) // 1999.08.30/Marko
		{
			FilterDialogUpdateStatus(1); // 1 = filterdialogin aikakontrolli-ikkuna pit‰‰ p‰ivitt‰‰
			itsLastBrushedViewRealRowIndex = -1; // sivellint‰ varten pit‰‰ 'nollata' t‰m‰
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
					GetCombinedMapHandler()->timeControlViewTimes(CtrlViewUtils::kDoAllMapViewDescTopIndex, editedInfo->TimeDescriptor());
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
				GetCombinedMapHandler()->clearAllMacroParamDataCacheDependentOfEditedDataAfterEditedDataChanges();
			}
			fIsTEMPCodeSoundingDataAlsoCopiedToEditedData = false;

			DoEditedInfoTimeSetup(editedInfo, loadFromFileState, setCurrentTimeToNearestHour);
		}

        if(DataNotificationSettings().Use() && DataNotificationSettings().ShowIcon())
            DoNewDataNotifications(theData, theNotificationStr, theDataFilePattern);

		// T‰m‰ on kTEMPCodeSoundingData-speciaali. Jos editori k‰ytt‰‰ vain luotaus datan katseluun,
		// eik‰ ole olemassa editoitua dataa, laitetaan t‰m‰n tyyppinen data myˆs editoitavaksi dataksi,
		// jotta dataa voisi k‰tev‰sti katsella editorilla, ilman ett‰ tarvitsee erikseen tiputella editoitavia datoja
		bool dataWasDeletedInInnerCall = false;
		if(theType == NFmiInfoData::kTEMPCodeSoundingData && (EditedInfo() == nullptr || fIsTEMPCodeSoundingDataAlsoCopiedToEditedData))
		{
			AddQueryData(theData->Clone(), theDataFileName, "", NFmiInfoData::kEditable, "", false, dataWasDeletedInInnerCall);
			fIsTEMPCodeSoundingDataAlsoCopiedToEditedData = true;
		}

		DoPossibleCaseStudyEditedDataSetup(theData, theDataFileName, theType, fDataWasDeletedOut);
        PrepareForParamAddSystemUpdate();
		RemoveCombinedDataFromLedChannelReport(theDataFilePattern);
		RemoveLateDataFromLedChannelReport(theDataFilePattern);
		AddLoadedDataToTriggerList(theDataFilePattern);
	}
}

// Luettu data lis‰t‰‰n listaan vain jos ei olla 1. kierroksen jo lokaali cachehakemistossa 
// olevia datoja lukemassa eli kun smartmet k‰ynnistyy tai ladataan caseStudy setti‰.
void AddLoadedDataToTriggerList(const std::string& theDataFilePattern)
{
	if(!NFmiInfoOrganizer::IsLoadedDataTreatedAsOld())
	{
		itsLoadedDataTriggerList.push_back(theDataFilePattern);
	}
}

// T‰ss‰ siis palautetaan triggerList ja nollataan se samalla, eli
// kunkin dataTriggerin voi pyyt‰‰ vain kerran.
std::vector<std::string> GetDataTriggerListOwnership()
{
	std::vector<std::string> returnedDataTriggerList;
	returnedDataTriggerList.swap(itsLoadedDataTriggerList);
	return returnedDataTriggerList;
}

void RemoveCombinedDataFromLedChannelReport(const std::string& theDataFileFilter)
{
	auto helpDataInfo = HelpDataInfoSystem()->FindHelpDataInfo(theDataFileFilter);
	if(helpDataInfo && helpDataInfo->IsCombineData())
	{
		LedLightStatusSystem().StopReportToChannelWithFileFilter(NFmiLedChannel::QueryData, theDataFileFilter);
	}
}

void RemoveLateDataFromLedChannelReport(const std::string& theDataFileFilter)
{
	LedLightStatusSystem().StopReportToChannel(NFmiLedChannel::DataIsLate, theDataFileFilter);
}

void DoEditedInfoTimeSetup(boost::shared_ptr<NFmiFastQueryInfo>& editedInfo, bool loadFromFileState, bool setCurrentTimeToNearestHour)
{
	if(editedInfo)
	{
		dynamic_cast<NFmiSmartInfo*>(editedInfo.get())->LoadedFromFile(loadFromFileState);
		// asetetaan viel‰ editorin currenttime sopivaksi
		bool foundTime = editedInfo->TimeToNearestStep(GetCombinedMapHandler()->currentTime(0), kCenter); // asetetaan current time p‰‰karttaikunan mukaan desctop-indeksi 0
		if(foundTime)
		{
			auto dataTime = editedInfo->Time();
			if(setCurrentTimeToNearestHour && (dataTime.GetMin() != 0 || dataTime.GetSec() != 0))
			{
				std::string logMessage = "Loaded edited data: ";
				logMessage += editedInfo->DataFileName();
				logMessage += ", dataTime: ";
				logMessage += dataTime.ToStr(kYYYYMMDDHHMMSS);
				logMessage += " had to be fixed to have 0 minutes and seconds for user convienience";
				CatLog::logMessage(logMessage, CatLog::Severity::Info, CatLog::Category::Operational);
				// Sallitaan vain tasa minuutit ja sekunnit t‰ss‰ karttapohjien aika-asetuksessa
				dataTime.SetMin(0);
				dataTime.SetSec(0);
			}
			GetCombinedMapHandler()->currentTime(CtrlViewUtils::kDoAllMapViewDescTopIndex, dataTime, false);
		}
	}
}

// Jos on ladattu caseStudy, eik‰ ollut pohjilla mit‰‰n editoitavaa dataa, laitetaan 1. sopiva ladattu data 'editoitavaksi dataksi'
void DoPossibleCaseStudyEditedDataSetup(NFmiQueryData* theData, const std::string& theDataFileName, NFmiInfoData::Type theType, bool fDataWasDeletedOut)
{
	// Jos ei ole editoitavaa dataa ja on jonkinn‰kˆinen case-study lataus/sulkeminen -tilanne
	if(EditedInfo() == nullptr && CaseStudyModeOn() || fChangingCaseStudyToNormalMode)
	{
		// Jos data oli jo deletoitu, ei yritet‰ ‰nke‰ sit‰ en‰‰ t‰h‰n
		if(fDataWasDeletedOut)
			return;
		// Vain tietyn tyyppiset datat kannattaa kelpuuttaa 'editoitavaksi' dataksi
		if(theType == NFmiInfoData::kEditable || theType == NFmiInfoData::kCopyOfEdited || theType == NFmiInfoData::kEditingHelpData || 
			theType == NFmiInfoData::kKepaData || theType == NFmiInfoData::kViewable || theType == NFmiInfoData::kModelHelpData || 
			theType == NFmiInfoData::kObservations || theType == NFmiInfoData::kAnalyzeData)
		{
			if(theData->Info()->SizeLevels() == 1) // mutta vain pintadatalle, koska leveldatat voivat olla niin jumalattoman isoja
			{
				auto producerId = theData->Info()->Producer()->GetIdent();
				// Havainnoista vain synop ja metar tuottaja kelp‰‰ t‰ss‰ tilanteessa
				if(theType != NFmiInfoData::kObservations || (producerId == kFmiSYNOP || producerId == kFmiMETAR))
				{
					fChangingCaseStudyToNormalMode = false;
					bool dataWasDeletedInInnerCall = false;
					AddQueryData(theData->Clone(), theDataFileName, "", NFmiInfoData::kEditable, "", false, dataWasDeletedInInnerCall, true);
					if(!dataWasDeletedInInnerCall)
					{
						std::string logMessage = "CaseStudy change situation (load/close), now using following as edited data: ";
						logMessage += theDataFileName;
						LogMessage(logMessage, CatLog::Severity::Info, CatLog::Category::Data);
					}
				}
			}
		}
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

void DoLastEditedDataSendHasComeBackChecks(NFmiInfoData::Type theType)
{
	if(theType == NFmiInfoData::kKepaData)
	{
		bool flagChanged = !fLastEditedDataSendHasComeBack;
		fLastEditedDataSendHasComeBack = true;
		if(flagChanged && ApplicationWinRegistry().ConfigurationRelatedWinRegistry().ShowLastSendTimeOnMapView())
			// 0 = vain p‰‰karttan‰yttˆ liataan, cacheja ei tarvitse tyhjent‰‰, koska t‰h‰n liittyv‰t jutut piirret‰‰n vain karttan‰ytˆn p‰‰lle, ei karttaruudukkoihin
			GetCombinedMapHandler()->mapViewDirty(0, false, false, true, false, true, false); 
		LogMessage("Operational data has been loaded.", CatLog::Severity::Info, CatLog::Category::Editing);
	}
}

void StoreLastLoadedFileNameToLog(const std::string &theFileName)
{
    CatLog::logMessage(std::string("Adding loaded data into SmartMet's inner database: ") + theFileName, CatLog::Severity::Debug, CatLog::Category::Data, true);
}

int FilterDialogUpdateStatus(void){return itsFilterDialogUpdateStatus;};
void FilterDialogUpdateStatus(int newState){itsFilterDialogUpdateStatus = newState;};

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

bool DoTimeSeriesValuesModifying(boost::shared_ptr<NFmiDrawParam> &theModifiedDrawParam, NFmiMetEditorTypes::Mask fUsedMask, NFmiTimeDescriptor& theTimeDescriptor, std::vector<float> &theModificationFactorCurvePoints, NFmiMetEditorTypes::FmiUsedSmartMetTool theEditorTool, bool fUseSetForDiscreteData, int theUnchangedValue = -1)
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
		CFmiOperationProgressAndCancellationDlg dlg(operationText, operationTextIsWarning, stopper, ApplicationInterface::GetSmartMetViewAsCView());
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
	CFmiOperationProgressAndCancellationDlg dlg(operationText, operationTextIsWarning, stopper, ApplicationInterface::GetSmartMetViewAsCView());
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
    GetCombinedMapHandler()->maskChangedDirtyActions();
}

// s‰‰t‰‰ annetun timebagin niin, ett‰ se on editoitavien aikojen sis‰ll‰
NFmiTimeBag AdjustToEditedDataTimeBag(const NFmiTimeBag& wantedTimebag)
{
	return AdjustTimeBagToGivenTimeBag(EditedDataTimeBag(), wantedTimebag);
}

// t‰t‰ metodia voidaan k‰ytt‰‰ aina kaikkialla kun tehd‰‰n muokkauksia dataan
// theModifyingTool 1 = muokkausdialogi, 2 = pensseli ja 3 = aikasarjaeditori
// kun tyˆkalu on 2 tai 3, annetaan mukana myˆs editoitava parametri
bool CheckAndValidateAfterModifications(NFmiMetEditorTypes::FmiUsedSmartMetTool theModifyingTool, bool fMakeDataSnapshotAction, unsigned int theLocationMask, FmiParameterName theParam)
{
	return FmiModifyEditdData::CheckAndValidateAfterModifications(GenDocDataAdapter(), theModifyingTool, fMakeDataSnapshotAction, theLocationMask, theParam, UseMultithreaddingWithModifyingFunctions());
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

bool InitLocationSelectionTool()
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
	itsLocationSelectionTool2 = new NFmiLocationSelectionTool;
	return true;
}

class MenuCreationSettings
{
public:
	unsigned int itsDescTopIndex = static_cast<unsigned int>(-1);
	// mik‰ komento on nyt kyseess‰
	FmiMenuCommandType itsMenuCommand = kFmiNoCommand;
	// vain vertikaali data kelpaa
	bool fLevelDataOnly = false;
	// vain hiladata kelpaa
	bool fGridDataOnly = false;
	// tietyt parametri saa lis‰t‰ vain karttan‰yttˆˆn
	bool fDoMapMenu = false;
	// n‰it‰ ei sallita toistaiseksi kuin karttan‰ytˆlle ja poikkileikkaukseen
	bool fAcceptMacroParams = false;
	// n‰m‰ (lat, lon ja elevation angle) hyv‰ksyt‰‰n vain maskeiksi
	bool fAcceptCalculateParams = false;
	// t‰m‰ pit‰‰ asettaa erikseen p‰‰lle kun rakennetaan custom-menuja
	bool fMakeCustomMenu = false;

	MenuCreationSettings() = default;

	// T‰t‰ k‰ytet‰‰n kartta- poikkileikkausn‰ytˆille
	void SetIsolineViewSettings(unsigned int theDescTopIndex, FmiMenuCommandType theMenuCommand)
	{
		if(theDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
			SetCrossSectionSettings(theDescTopIndex, theMenuCommand);
		else
			SetMapViewSettings(theDescTopIndex, theMenuCommand);
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
		itsDescTopIndex = CtrlViewUtils::kFmiTimeSerialView;
		itsMenuCommand = theMenuCommand;

		fLevelDataOnly = false;
		fGridDataOnly = false;
		fDoMapMenu = false;
		fAcceptMacroParams = true;
		fAcceptCalculateParams = false;
		fMakeCustomMenu = false;
	}

private:
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
	void SetCrossSectionSettings(unsigned int theDescTopIndex, FmiMenuCommandType theMenuCommand)
	{
		itsDescTopIndex = theDescTopIndex;
		itsMenuCommand = theMenuCommand;

		fLevelDataOnly = true;
		fGridDataOnly = true;
		fDoMapMenu = false;
		fAcceptMacroParams = true;
		fAcceptCalculateParams = false;
		fMakeCustomMenu = false;
	}
};

void CreateParamSelectionBasePopup(const MenuCreationSettings &theMenuSettings, NFmiMenuItemList *thePopupMenu, const std::string &theMenuItemDictionaryStr)
{
	std::string menuString = ::GetDictionaryString(theMenuItemDictionaryStr.c_str());
    CtrlViewUtils::CtrlViewTimeConsumptionReporter timeConsumptionReporter(nullptr, std::string(__FUNCTION__) + ": " + menuString);
    auto menuItem = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(), theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
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
    auto menuItem6 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(), theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kObservations);
	AddObservationDataToParamSelectionPopup(theMenuSettings, menuItem6.get());
	if(menuItem6->SubMenu()->NumberOfMenuItems() > 0)
		menuList->Add(std::move(menuItem6));
// ********** lis‰t‰‰n havaintoparametri osa *************************

// ********** WMS *************************
    AddWmsDataToParamSelectionPopup(theMenuSettings, menuList, NFmiInfoData::kWmsData);

// ********** lis‰t‰‰n apudata-parametri osa *************************
	menuString = ::GetDictionaryString("MapViewParamPopUpHelpData");
    auto menuItem7 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(), theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kViewable);
	AddHelpDataToParamSelectionPopup(theMenuSettings, menuItem7.get());
	if(menuItem7->SubMenu()->NumberOfMenuItems() > 0)
		menuList->Add(std::move(menuItem7));
// ********** lis‰t‰‰n apudata-parametri osa *************************

// ********** lis‰t‰‰n talletetut macroParamit osa *************************
	if(theMenuSettings.fAcceptMacroParams)
	{
		if(theMenuSettings.fDoMapMenu)
			AddMacroParamPartToPopUpMenu(theMenuSettings, menuList, NFmiInfoData::kMacroParam);
		else if(theMenuSettings.itsDescTopIndex == CtrlViewUtils::kFmiTimeSerialView)
			AddMacroParamPartToPopUpMenu(theMenuSettings, menuList, NFmiInfoData::kTimeSerialMacroParam);
		else if(theMenuSettings.itsDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
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
        g_DefaultParamView,
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
			auto wmsSupportPtr = GetCombinedMapHandler()->getWmsSupport();
            if(!wmsSupportPtr->isCapabilityTreeAvailable())
                return;
            CtrlViewUtils::CtrlViewTimeConsumptionReporter timeConsumptionReporter(nullptr, __FUNCTION__);
            auto layerTree = wmsSupportPtr->getCapabilityTree();
			if(layerTree)
			{
				auto menuItem = std::make_unique<NFmiMenuItem>(
					theMenuSettings.itsDescTopIndex,
					"WMS",
					NFmiDataIdent(NFmiParam(layerTree->value.paramId, layerTree->value.name), layerTree->value.producer),
					theMenuSettings.itsMenuCommand,
					g_DefaultParamView,
					nullptr,
					theDataType
					);
				try
				{
					const auto& layerTreeCasted = dynamic_cast<const Wms::CapabilityNode&>(*layerTree);
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
					theMenuItemList->Add(std::move(menuItem));
				}
				catch(const std::exception& e)
				{
					std::string errorMessage = "WMS popup menu section failed: ";
					errorMessage += e.what();
					LogMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::NetRequest, true);
				}
			}
        }
        catch(const std::exception & e)
        {
            std::string errorMessage = "WMS popup menu section failed: ";
            errorMessage += e.what();
            LogMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::NetRequest, true);
        }
        catch(...)
        {
            std::string errorMessage = "Unknown errror when building WMS popup menu";
            LogMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::NetRequest, true);
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
		std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infoList = itsSmartInfoOrganizer->GetInfos(helpDataList[j].UsedFileNameFilter(*HelpDataInfoSystem()));
		for(size_t k=0; k < infoList.size(); k++)
		{
			AddSmartInfoToMenuList(usedMenuSettings, infoList[k], theCustomMenuList, infoList[k]->DataType());
		}
	}
}

void AddCustomFoldersToMenu(const MenuCreationSettings &theMenuSettings, NFmiMenuItemList *theMenuList)
{
    CtrlViewUtils::CtrlViewTimeConsumptionReporter timeConsumptionReporter(nullptr, __FUNCTION__);
    MenuCreationSettings usedMenuSettings(theMenuSettings);
	usedMenuSettings.fMakeCustomMenu = true;
	std::vector<std::string> customMenuList = HelpDataInfoSystem()->GetUniqueCustomMenuList();
	for(size_t i = 0; i<customMenuList.size(); i++)
	{
		std::string customMenuName = customMenuList[i];
        if(customMenuName == g_ObservationMenuName)
            continue; // Observation-menu pit‰‰ skipata t‰‰ll‰, koska se lis‰t‰‰n havaintojen alivalikkoon
        auto customMenuItem = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, customMenuName, NFmiDataIdent(), theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kAnyData);
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

void AddMapLayerRelatedInfosToMapLayerSelectionPopup(unsigned int theDescTopIndex, const std::string& finalMenuString, FmiMenuCommandType menuCommand, const MapAreaMapLayerRelatedInfo & mapAreaMapLayerRelatedInfos, NFmiMenuItemList& theMenuItemList)
{
	if(!mapAreaMapLayerRelatedInfos.empty())
	{
		auto mainMenuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, finalMenuString, NFmiDataIdent(), menuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
		auto menuItemList = std::make_unique<NFmiMenuItemList>();

		// Lis‰t‰‰n overlay tapauksessa 'none' layer 1. vaihtoehdoksi
		if(menuCommand == kFmiSelectOverlayMapLayer)
		{
			auto noneSelectionMenuString = CombinedMapHandlerInterface::getNoneOverlayName();
			auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, noneSelectionMenuString, NFmiDataIdent(), menuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
			menuItemList->Add(std::move(menuItem));
		}

		for(const auto& mapLayerRelatedInfo : mapAreaMapLayerRelatedInfos)
		{
			auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, mapLayerRelatedInfo.guiName_, NFmiDataIdent(), menuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
			// Laitetaan n‰in menu-itemiin tieto siit‰ ett‰ oliko kyse tiedosto vai Wms pohjaisesta kartta layerista
			menuItem->ExtraParam(mapLayerRelatedInfo.isWmsLayer_ ? g_mapLayerSelectionIsWmsBased : g_mapLayerSelectionIsFileBased);
			menuItemList->Add(std::move(menuItem));
		}

		// Kuinka monta ei none layeria lis‰tty
		auto actualLayersAdded = (menuCommand == kFmiSelectOverlayMapLayer) ? menuItemList->NumberOfMenuItems() - 1 : menuItemList->NumberOfMenuItems();

		if(actualLayersAdded > 0)
		{
			mainMenuItem->AddSubMenu(menuItemList.release());
			theMenuItemList.Add(std::move(mainMenuItem));
		}
	}
}

void AddToMapLayerSelectionPopup(unsigned int theDescTopIndex, const std::string &finalMainLevelMenuString, FmiMenuCommandType menuCommand, bool backgroundMapCase, NFmiMenuItemList &theMenuList)
{
	auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, finalMainLevelMenuString, NFmiDataIdent(), menuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);

	auto subMenuItemList = std::make_unique<NFmiMenuItemList>();
	std::string finalStaticMenuString = ::GetDictionaryString("Static maps");
	auto fileBasedMapLayerRelatedInfos = itsCombinedMapHandler.getCurrentMapLayerRelatedInfos(theDescTopIndex, backgroundMapCase, false);
	AddMapLayerRelatedInfosToMapLayerSelectionPopup(theDescTopIndex, finalStaticMenuString, menuCommand, fileBasedMapLayerRelatedInfos, *subMenuItemList);
	if(!itsCombinedMapHandler.localOnlyMapModeUsed())
	{
		std::string finalDynamicMenuString = ::GetDictionaryString("Dynamic maps");
		auto wmsMapLayerRelatedInfos = itsCombinedMapHandler.getCurrentMapLayerRelatedInfos(theDescTopIndex, backgroundMapCase, true);
		AddMapLayerRelatedInfosToMapLayerSelectionPopup(theDescTopIndex, finalDynamicMenuString, menuCommand, wmsMapLayerRelatedInfos, *subMenuItemList);
	}

	if(subMenuItemList->NumberOfMenuItems() > 0)
	{
		menuItem->AddSubMenu(subMenuItemList.release());
		theMenuList.Add(std::move(menuItem));
	}
}

// Luodaan popup, jossa kaksi perustoimintoa:
// 1. Background map-layerin valinta
// 2. Overlay map-layerin valinta
// Molemmat valinnat menev‰t omaan haaraan. Lis‰ksi n‰m‰ haarat jakaantuvat sek‰ static bitmap ja wms haaroihin.
bool CreateMapLayerSelectionPopup(unsigned int theDescTopIndex)
{
	if(theDescTopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex)
	{
		itsPopupMenu = std::make_unique<NFmiMenuItemList>();

		std::string finalBackgroundMenuString = ::GetDictionaryString("Select background");
		AddToMapLayerSelectionPopup(theDescTopIndex, finalBackgroundMenuString, kFmiSelectBackgroundMapLayer, true, *itsPopupMenu);
		std::string finalOverlayMenuString = ::GetDictionaryString("Select Overlay");
		AddToMapLayerSelectionPopup(theDescTopIndex, finalOverlayMenuString, kFmiSelectOverlayMapLayer, false, *itsPopupMenu);

		if(!itsPopupMenu->InitializeCommandIDs(itsPopupMenuStartId))
			return false;

		fOpenPopup = true;
		return true;
	}
	return false;
}

void SelectMapLayerDirectly(NFmiMenuItem& menuItem)
{
	bool wmsCase = (menuItem.ExtraParam() == g_mapLayerSelectionIsWmsBased);
	bool backgroundCase = (menuItem.CommandType() == kFmiSelectBackgroundMapLayer);
	itsCombinedMapHandler.selectMapLayer(menuItem.MapViewDescTopIndex(), menuItem.MenuText(), backgroundCase, wmsCase);
}

bool CreateParamSelectionPopup(unsigned int theDescTopIndex)
{
    CtrlViewUtils::CtrlViewTimeConsumptionReporter::increaseCurrentUpdateId();
    CtrlViewUtils::CtrlViewTimeConsumptionReporter timeConsumptionReporter(nullptr, __FUNCTION__);
	if(theDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
		return CreateCrossSectionViewPopup(itsCurrentViewRowIndex);

	fOpenPopup = false;
	if(itsSmartInfoOrganizer)
	{
		itsPopupMenu = std::make_unique<NFmiMenuItemList>();

		MenuCreationSettings menuSettings;
		menuSettings.SetIsolineViewSettings(theDescTopIndex, kFmiAddView);
		CreateParamSelectionBasePopup(menuSettings, itsPopupMenu.get(), "MapViewParamPopUpAdd");

        AddSwapViewRowsToPopup(theDescTopIndex, itsPopupMenu.get());

		std::string menuString = ::GetDictionaryString("MapViewParamPopUpremoveAll");
        auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, menuString, NFmiDataIdent(), kFmiRemoveAllViews, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
		itsPopupMenu->Add(std::move(menuItem));

        if(theDescTopIndex != CtrlViewUtils::kFmiTimeSerialView)
        {
            menuString = ::GetDictionaryString("Remove all params from all rows");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, NFmiDataIdent(), kFmiRemoveAllParamsFromAllRows, g_DefaultParamView, nullptr, NFmiInfoData::kEditable));
            itsPopupMenu->Add(std::move(menuItem));
        }

		menuString = "Copy all map row params";
		menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, NFmiDataIdent(), kFmiCopyDrawParamsFromMapViewRow, g_DefaultParamView, nullptr, NFmiInfoData::kEditable));
		itsPopupMenu->Add(std::move(menuItem));
		if(GetCombinedMapHandler()->copyPasteDrawParamListUsedYet())
		{
			menuString = "Paste all map row params";
			menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, NFmiDataIdent(), kFmiPasteDrawParamsToMapViewRow, g_DefaultParamView, nullptr, NFmiInfoData::kEditable));
			itsPopupMenu->Add(std::move(menuItem));
		}

		menuString = "Copy all map rows and params";
		menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, NFmiDataIdent(), kFmiCopyMapViewDescTopParams, g_DefaultParamView, nullptr, NFmiInfoData::kEditable));
		itsPopupMenu->Add(std::move(menuItem));
		if(GetCombinedMapHandler()->copyPasteDrawParamListVectorUsedYet())
		{
			menuString = "Paste all map rows and params";
			menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, NFmiDataIdent(), kFmiPasteMapViewDescTopParams, g_DefaultParamView, nullptr, NFmiInfoData::kEditable));
			itsPopupMenu->Add(std::move(menuItem));
		}

// ********* muuta n‰yttˆrivin kaikkien datojen tuottajat halutuiksi *********
		AddChangeAllProducersToParamSelectionPopup(theDescTopIndex, itsPopupMenu.get(), kFmiChangeAllProducersInMapRow, false);
// ********* muuta n‰yttˆrivin kaikkien datojen tuottajat halutuiksi *********

		// ************ t‰ss‰ muutetaan kaikki rivin datatyypit (fiksataan ongelma mik‰ syntyi kun LAPS muuttui kViewable:ista kAnalyze -tyyppiseksi)
		AddChangeAllDataTypesToParamSelectionPopup(theDescTopIndex, itsPopupMenu.get(), kFmiChangeAllDataTypesInMapRow);

// ********* piilota/n‰yt‰ kaikki  -  havainnot/ennusteet *********
		menuString = ::GetDictionaryString("MapViewParamPopUpToggleShowAll");
		menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, NFmiDataIdent(), kFmiHideAllMapViewObservations, g_DefaultParamView, nullptr, NFmiInfoData::kEditable));
		NFmiMenuItemList *showHideMenuList = new NFmiMenuItemList;
		menuString = ::GetDictionaryString("MapViewParamPopUpHideAllObs");
		showHideMenuList->Add(std::make_unique<NFmiMenuItem>(theDescTopIndex, menuString, NFmiDataIdent(), kFmiHideAllMapViewObservations, g_DefaultParamView, nullptr, NFmiInfoData::kEditable));
		menuString = ::GetDictionaryString("MapViewParamPopUpShowAllObs");
		showHideMenuList->Add(std::make_unique<NFmiMenuItem>(theDescTopIndex, menuString, NFmiDataIdent(), kFmiShowAllMapViewObservations, g_DefaultParamView, nullptr, NFmiInfoData::kEditable));
		menuString = ::GetDictionaryString("MapViewParamPopUpHideAllFor");
		showHideMenuList->Add(std::make_unique<NFmiMenuItem>(theDescTopIndex, menuString, NFmiDataIdent(), kFmiHideAllMapViewForecasts, g_DefaultParamView, nullptr, NFmiInfoData::kEditable));
		menuString = ::GetDictionaryString("MapViewParamPopUpShowAllFor");
		showHideMenuList->Add(std::make_unique<NFmiMenuItem>(theDescTopIndex, menuString, NFmiDataIdent(), kFmiShowAllMapViewForecasts, g_DefaultParamView, nullptr, NFmiInfoData::kEditable));
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
    CtrlViewUtils::CtrlViewTimeConsumptionReporter timeConsumptionReporter(nullptr, std::string(__FUNCTION__) + ": " + menuString);
    auto menuItem = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(), theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kMacroParam);
	NFmiMenuItemList *macroParamsMenuList = new NFmiMenuItemList;

	std::vector<NFmiMacroParamItem> &macroParamItemList = itsMacroParamSystem.MacroParamItemTree();
	AddMacroParamPartToPopUpMenu(theMenuSettings, macroParamsMenuList, macroParamItemList, theDataType);
	menuItem->AddSubMenu(macroParamsMenuList);
	theMenuList->Add(std::move(menuItem));
}

void AddSwapViewRowsToPopup(unsigned int theDescTopIndex, NFmiMenuItemList* theMenuList)
{
    bool crossSectionCase = (theDescTopIndex == CtrlViewUtils::kFmiCrossSectionView);
    int maxRowIndex = crossSectionCase ? CtrlViewUtils::MaxViewGridYSize : CtrlViewUtils::MaxViewGridXSize * CtrlViewUtils::MaxViewGridYSize;
    int currentAbsoluteRowIndex = GetCombinedMapHandler()->getRealRowNumber(theDescTopIndex, itsCurrentViewRowIndex);
    std::string menuText = "Swap row ";
    menuText += std::to_string(currentAbsoluteRowIndex);
    menuText += " with row";
    auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, menuText, NFmiDataIdent(), kFmiSwapViewRows, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
    AddSwapViewRowsToToMenuItem(theDescTopIndex, menuItem.get(), maxRowIndex, currentAbsoluteRowIndex);
    theMenuList->Add(std::move(menuItem));
}

void AddSwapViewRowsToToMenuItem(unsigned int theDescTopIndex, NFmiMenuItem* theMenuItem, int maxRowIndex, int currentAbsoluteRowIndex)
{
    NFmiMenuItemList* rowNumbersMenuList = new NFmiMenuItemList;
    for(int rowIndex = 1; rowIndex <= maxRowIndex; rowIndex++)
    {
        if(rowIndex != currentAbsoluteRowIndex)
        {
            std::string menuText = std::to_string(rowIndex);
            // NFmiMenuItem:in IndexInViewRow saa arvokseen currentAbsoluteRowIndex:in ja rowIndex menee ExtraParam:iin talteen
            auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, menuText, NFmiDataIdent(), kFmiSwapViewRows, g_DefaultParamView, nullptr, NFmiInfoData::kEditable, currentAbsoluteRowIndex);
            menuItem->ExtraParam(rowIndex);
            rowNumbersMenuList->Add(std::move(menuItem));
        }
    }
    theMenuItem->AddSubMenu(rowNumbersMenuList);
}

void AddChangeAllProducersOperativeModelPart(unsigned int theDescTopIndex, NFmiMenuItemList* changeProducersMenuList, FmiMenuCommandType theMenuCommandType, bool crossSectionPopup)
{
	// Laitetaan alkuun editoitu data ja virallinen data tuottajat listaan, jos ei kyse poikkileikkausn‰ytˆst‰
	if(crossSectionPopup == false)
	{
		boost::shared_ptr<NFmiFastQueryInfo> editedInfo = EditedInfo();
		if(editedInfo)
		{
			editedInfo->FirstParam();
			changeProducersMenuList->Add(std::make_unique<NFmiMenuItem>(theDescTopIndex, "Edited data", editedInfo->Param(), theMenuCommandType, g_DefaultParamView, nullptr, NFmiInfoData::kEditable));
		}
		boost::shared_ptr<NFmiFastQueryInfo> operativeInfo = itsSmartInfoOrganizer->FindInfo(NFmiInfoData::kKepaData);
		if(operativeInfo) // operatiivinen data
		{
			operativeInfo->FirstParam();
			changeProducersMenuList->Add(std::make_unique<NFmiMenuItem>(theDescTopIndex, "Operative data", operativeInfo->Param(), theMenuCommandType, g_DefaultParamView, nullptr, NFmiInfoData::kEditable));
		}
		boost::shared_ptr<NFmiFastQueryInfo> helpDataInfo = itsSmartInfoOrganizer->FindInfo(NFmiInfoData::kEditingHelpData);
		if(helpDataInfo) // editointi apu data
		{
			helpDataInfo->FirstParam();
			changeProducersMenuList->Add(std::make_unique<NFmiMenuItem>(theDescTopIndex, "Help edit data", helpDataInfo->Param(), theMenuCommandType, g_DefaultParamView, nullptr, NFmiInfoData::kEditable));
		}
	}
}

void AddChangeAllProducersFromVectorPart(unsigned int theDescTopIndex, std::vector<NFmiProducerInfo>& producerInfos, NFmiMenuItemList* changeProducersMenuList, FmiMenuCommandType theMenuCommandType)
{
		for(auto &producerInfo : producerInfos)
		{
			auto producer = producerInfo.GetProducer();
			auto infoVector = itsSmartInfoOrganizer->GetInfos(producer.GetIdent());
			if(!infoVector.empty())
			{
				// Jos lˆytyy jotain dataa, laitetaan feikki parametri ja tuottaja listasta
				NFmiDataIdent dataIdent(NFmiParam(), producer);
				changeProducersMenuList->Add(std::make_unique<NFmiMenuItem>(theDescTopIndex, producerInfo.Name(), dataIdent, theMenuCommandType, g_DefaultParamView, nullptr, NFmiInfoData::kEditable));
			}
		}
}

void AddChangeAllProducersDataTypePart(unsigned int theDescTopIndex, NFmiMenuItemList* theMenuList, FmiMenuCommandType theMenuCommandType, bool crossSectionPopup, const std::string &theDataTypeMenuTitleName, std::vector<NFmiProducerInfo>& producerInfos, bool modelTypeCase)
{
	if(producerInfos.size() > 0)
	{
		NFmiMenuItemList* changeProducersMenuList = new NFmiMenuItemList;
		if(modelTypeCase)
		{
			AddChangeAllProducersOperativeModelPart(theDescTopIndex, changeProducersMenuList, theMenuCommandType, crossSectionPopup);
		}
		AddChangeAllProducersFromVectorPart(theDescTopIndex, producerInfos, changeProducersMenuList, theMenuCommandType);
		auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, theDataTypeMenuTitleName, NFmiDataIdent(), theMenuCommandType, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
		menuItem->AddSubMenu(changeProducersMenuList);
		theMenuList->Add(std::move(menuItem));
	}
}

void AddChangeAllProducersToParamSelectionPopup(unsigned int theDescTopIndex, NFmiMenuItemList* theMenuList, FmiMenuCommandType theMenuCommandType, bool crossSectionPopup)
{
	CtrlViewUtils::CtrlViewTimeConsumptionReporter timeConsumptionReporter(nullptr, __FUNCTION__);
	NFmiMenuItemList* dataTypePartSubMenuList = new NFmiMenuItemList;
	AddChangeAllProducersDataTypePart(theDescTopIndex, dataTypePartSubMenuList, theMenuCommandType, crossSectionPopup, "Model data", ProducerSystem().Producers(), true);
	AddChangeAllProducersDataTypePart(theDescTopIndex, dataTypePartSubMenuList, theMenuCommandType, crossSectionPopup, "Observations", ObsProducerSystem().Producers(), false);
	if(dataTypePartSubMenuList->NumberOfMenuItems() > 0)
	{
		auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, "Change all producers", NFmiDataIdent(), theMenuCommandType, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
		menuItem->AddSubMenu(dataTypePartSubMenuList);
		theMenuList->Add(std::move(menuItem));
	}
}

void AddChangeAllDataTypesToParamSelectionPopup(unsigned int theDescTopIndex, NFmiMenuItemList *theMenuList, FmiMenuCommandType theMenuCommandType)
{
    CtrlViewUtils::CtrlViewTimeConsumptionReporter timeConsumptionReporter(nullptr, __FUNCTION__);
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
        auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, "Change data-types (fix view-macros)", NFmiDataIdent(), theMenuCommandType, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
		NFmiMenuItemList *menuList = new NFmiMenuItemList;
		for(size_t i = 0; i < dataTypeVec.size(); i++)
		{
			menuList->Add(std::make_unique<NFmiMenuItem>(theDescTopIndex, dataTypeVec[i].first, static_cast<FmiParameterName>(dataTypeVec[i].second), theMenuCommandType, g_DefaultParamView, nullptr, NFmiInfoData::kEditable));
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
            auto macroParamsItem = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, macroParamItem.itsMacroParam->Name().c_str(), NFmiDataIdent(NFmiParam(998, "makro")), theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, theDataType);
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
            auto macroParamsItem = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, macroParamName.c_str(), NFmiDataIdent(NFmiParam(998, "makro")), theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, theDataType);
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
    CtrlViewUtils::CtrlViewTimeConsumptionReporter timeConsumptionReporter(nullptr, __FUNCTION__);
    NFmiMenuItemList *menuList = new NFmiMenuItemList;
	AddProducerDataToParamSelectionPopup(theMenuSettings, menuList, 160); // 160 = mesan 2
    AddProducerDataToParamSelectionPopup(theMenuSettings, menuList, 189); // 189 = MetNo Analysis
	AddProducerDataToParamSelectionPopup(theMenuSettings, menuList, kFmiRADARNRD, NFmiInfoData::kObservations); // tuliset dataa, mutta ei 'tutka'-dataa eli ignooraa kObservations-data tyyppi joka normaali tutkalla on
	AddProducerDataToParamSelectionPopup(theMenuSettings, menuList, 108); // 108 = lumikuorma-datan tuottaja numero
    AddProducerDataToParamSelectionPopup(theMenuSettings, menuList, 101); // 101 = kriging-datan tuottaja numero
	AddFirstOfDataTypeToParamSelectionPopup(theMenuSettings, menuList, NFmiInfoData::kStationary, ""); // kun annetaan tyhj‰ dictionary -stringi, k‰ytet‰‰n tuottaja nimea menu otsikossa
	AddProducerDataToParamSelectionPopup(theMenuSettings, menuList, kFmiHakeMessages); // hake-sanoma (h‰lytyskeskus) dataa
	AddProducerDataToParamSelectionPopup(theMenuSettings, menuList, kFmiKaHaMessages); // kaha-sanoma (kansalaishavainto) dataa

	if(theMenuSettings.fDoMapMenu && ConceptualModelData().Use()) // jos k‰siteanalyysi systeemi k‰ytˆss‰, lis‰t‰‰n sen lis‰‰mismahdollisuus popup-valikkoon
	{
		NFmiProducer prod(1028); // t‰ll‰ ei ole viel‰ virallista tuottaja id:t‰
		std::string menuString = "Conceptual analysis";
        auto menuItem1 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(NFmiParam(kFmiLastParameter, ConceptualModelData().DefaultUserName()), prod), theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kConceptualModelData);
		menuList->Add(std::move(menuItem1));
	}

    if(capDataSystem.useCapData())
    {
        NFmiProducer prod(NFmiSettings::Optional<int>("SmartMet::Warnings::ProducerId", 12345)); // No official producerId, reads this from Cap.conf. If multiple ids, read them all here.
        std::string menuString = "Warnings (CAP)";
        auto menuItem1 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(NFmiParam(kFmiLastParameter, "cap-data"), prod), theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kCapData);
        menuList->Add(std::move(menuItem1));
    }

	// T‰ss‰ tehd‰‰n sekaisin kaikkia mahdollisia datoja yhteen popup-osioon.
	// Toivottavasti auttaa erilaisten datojen selaamisessa mm. drag'n drop tilanteissa.
	// Mix popup-osiota ei tehd‰ operatiivisessa moodissa, koska se hidastaa k‰yttˆ‰
	if(EditorModeDataWCTR()->EditorMode() == NFmiMetEditorModeDataWCTR::kNormal)
	{
        auto mixMenuItem = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, "Mix", NFmiDataIdent(), theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kViewable);
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
	std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infos(itsSmartInfoOrganizer->GetInfos(theDataType));
	int size = static_cast<int>(infos.size());
	if(size > 0)
	{
		NFmiMenuItemList *subMenuList = new NFmiMenuItemList; // luodaan ali menu
		for(int i=0; i<size; i++)
			AddSmartInfoToMenuList(theMenuSettings, infos[i], subMenuList, theDataType);

        auto subMenuItem = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, "Radar data",
            NFmiDataIdent(), theMenuSettings.itsMenuCommand, g_DefaultParamView,
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
        theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kSatelData);

	auto satelProducerMenuListPtr = std::make_unique<NFmiMenuItemList>();
	std::vector<NFmiProducerInfo> &satelProducers = itsSatelImageProducerSystem.Producers();
	for(size_t j=0; j<satelProducers.size(); j++)
	{
		bool anyChannelsFound = false;
        auto menuItemProducer = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, satelProducers[j].Name(), NFmiDataIdent(),
            theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kSatelData);
		auto satelChannelMenuListPtr = std::make_unique<NFmiMenuItemList>();
		int helpDataSize = HelpDataInfoSystem()->DynamicCount();
		for(int i=0; i<helpDataSize; i++)
		{ 
			// etsit‰‰n currentin tuottajan kuva kanavat/parametrit
			const NFmiHelpDataInfo &helpDataInfo = HelpDataInfoSystem()->DynamicHelpDataInfo(i);
			if(helpDataInfo.IsEnabled() && helpDataInfo.IsDataUsedCaseStudyChecks(CaseStudyModeOn()) && helpDataInfo.DataType() == NFmiInfoData::kSatelData)
			{
				int prodId = static_cast<int>(satelProducers[j].ProducerId());
				int helpDataProdId = helpDataInfo.FakeProducerId();
				if(prodId > 0 && prodId == helpDataProdId)
				{
                    auto satelItem = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, std::string(helpDataInfo.ImageDataIdent().GetParamName()),
                        helpDataInfo.ImageDataIdent(), theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kSatelData);
					satelChannelMenuListPtr->Add(std::move(satelItem));
					anyChannelsFound = true;
				}
			}
		}
		if(anyChannelsFound)
		{
			menuItemProducer->AddSubMenu(satelChannelMenuListPtr.release());
			satelProducerMenuListPtr->Add(std::move(menuItemProducer));
			anySatelProducersFound = true;
		}
	}
	if(anySatelProducersFound)
	{
		menuItem2->AddSubMenu(satelProducerMenuListPtr.release());
		theMenuList->Add(std::move(menuItem2));
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
            auto menuItem1 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(NFmiParam(kFmiFlashStrength, "salama"), flashProducer), theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, flashType);
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
            auto menuItem3 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(NFmiParam(NFmiInfoData::kFmiSpSynoPlot, "synop")), theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, synopType);
			obsMenuList->Add(std::move(menuItem3));

			// lis‰t‰‰n myˆs min/max plot
			std::string menuStringMinMax = "Min/Max";
            auto menuItem4 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuStringMinMax, NFmiDataIdent(NFmiParam(NFmiInfoData::kFmiSpMinMaxPlot, "min/max")), theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, synopType);
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
            auto menuItem3 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(NFmiParam(NFmiInfoData::kFmiSpMetarPlot, "metar")), theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, metarType);
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
            auto menuItem4 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(NFmiParam(NFmiInfoData::kFmiSpSoundingPlot, "temp")), theMenuSettings.itsMenuCommand, g_DefaultParamView, &defaultLevel, soundingType);
			obsMenuList->Add(std::move(menuItem4));

			menuString = "Sounding";
            auto menuItem5 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(), theMenuSettings.itsMenuCommand, g_DefaultParamView, &defaultLevel, soundingType);
			NFmiMenuItemList *soundingMenuList = new NFmiMenuItemList(theMenuSettings.itsDescTopIndex, const_cast<NFmiParamBag*>(&(soundingInfo->ParamBag())), theMenuSettings.itsMenuCommand, g_DefaultParamView, const_cast<NFmiLevelBag*>(itsSoundingPlotLevels.Levels()), soundingType);
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
	std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infos(itsSmartInfoOrganizer->GetInfos(theProducerId));
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
            CtrlViewUtils::CtrlViewTimeConsumptionReporter timeConsumptionReporter(nullptr, std::string(__FUNCTION__) + ": " + prodInfo.Name());

			std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infos(itsSmartInfoOrganizer->GetInfos(prodInfo.ProducerId()));
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
                    auto producerMenuItem = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, prodInfo.Name().c_str(), kFmiBadParameter, kFmiNoCommand, NFmiMetEditorTypes::View::kFmiTextView, nullptr, NFmiInfoData::kNoDataType);
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
		std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infos(itsSmartInfoOrganizer->GetInfos(dataTypes[typeCounter]));
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
        bool allowStreamlineParameter = theMenuSettings.fDoMapMenu;
        auto possibleWindMetaParams = NFmiFastInfoUtils::MakePossibleWindMetaParams(*theSmartInfo, allowStreamlineParameter);
		NFmiMenuItemList *menuList = 0;
		bool doCrossSectionMenu = theMenuSettings.fLevelDataOnly && theMenuSettings.fGridDataOnly;
		if(doCrossSectionMenu == false && levels) // poikkileikkaus menun yhteydess‰ ei halutakaan laittaa level tietoja menu-popupiin, vain parametrit
            menuList = new NFmiMenuItemList(theMenuSettings.itsDescTopIndex, &paramBag, theMenuSettings.itsMenuCommand, g_DefaultParamView, levels, theDataType, kFmiLastParameter, &possibleWindMetaParams);
		else
			menuList = new NFmiMenuItemList(theMenuSettings.itsDescTopIndex, &paramBag, theMenuSettings.itsMenuCommand, g_DefaultParamView, theDataType, &possibleWindMetaParams);

		if(menuList)
			theMenuItem->AddSubMenu(menuList);
	}
	else
		throw std::runtime_error("Error when making param selection popup menu in function AddSmartInfoToMenuItem. One was zero pointer: menuIten or smartInfo.");
}

// Jos lis‰tt‰v‰ data on salama tyyppist‰, sallitaan sen lis‰ys vain karttan‰ytˆille
bool DoLightningDataTypePopupCheck(const MenuCreationSettings& theMenuSettings, boost::shared_ptr<NFmiFastQueryInfo>& info)
{
	if(NFmiFastInfoUtils::IsLightningTypeData(info))
	{
		if(theMenuSettings.fDoMapMenu)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return true;
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

		if(!DoLightningDataTypePopupCheck(theMenuSettings, theSmartInfo))
			return;

		// jos kyseess‰ olisi poikkileikkaus menu rakentelua, ei ole syyt‰ p‰‰st‰‰ l‰pi dataa, miss‰ on vain yksi leveli ja se ei ole hila dataa
		if(theMenuSettings.fLevelDataOnly == false || (theMenuSettings.fLevelDataOnly == true && theSmartInfo->SizeLevels() > 1))
		{
			std::string menuTitleStr = theDictionaryStr.empty() ? theSmartInfo->FirstParamProducer().GetName() : ::GetDictionaryString(theDictionaryStr.c_str());
            auto menuItem = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuTitleStr, NFmiDataIdent(), theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, theDataType);
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
    CtrlViewUtils::CtrlViewTimeConsumptionReporter timeConsumptionReporter(nullptr, std::string(__FUNCTION__) + ": " + menuString);
    auto menuItem4 = std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, NFmiDataIdent(), theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kCalculatedValue);
	NFmiMenuItemList *menuList4 = new NFmiMenuItemList;
	menuString = ::GetDictionaryString("MapViewMaskSelectionPopUpLatitude");
	menuList4->Add(std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, kFmiLatitude, theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kCalculatedValue));
	menuString = ::GetDictionaryString("MapViewMaskSelectionPopUpLongitude");
	menuList4->Add(std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, kFmiLongitude, theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kCalculatedValue));
	menuString = ::GetDictionaryString("MapViewMaskSelectionPopUpElevationAngle");
	menuList4->Add(std::make_unique<NFmiMenuItem>(theMenuSettings.itsDescTopIndex, menuString, kFmiElevationAngle, theMenuSettings.itsMenuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kCalculatedValue));
	menuItem4->AddSubMenu(menuList4);
	theMenuItemList->Add(std::move(menuItem4));
}

bool CreateMaskSelectionPopup(void)
{
	fOpenPopup = false;
	if(itsSmartInfoOrganizer)
	{
		itsPopupMenu = std::make_unique<NFmiMenuItemList>();

		MenuCreationSettings menuSettings;
		menuSettings.SetMaskSettings(kFmiAddMask);
		CreateParamSelectionBasePopup(menuSettings, itsPopupMenu.get(), "MapViewMaskSelectionPopUpAdd");

		std::string menuString = ::GetDictionaryString("MapViewMaskSelectionPopUpRemoveAll");
        auto menuItem = std::make_unique<NFmiMenuItem>(-1, menuString, NFmiDataIdent(), kFmiRemoveAllMasks, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
		itsPopupMenu->Add(std::move(menuItem));

		if(!itsPopupMenu->InitializeCommandIDs(itsPopupMenuStartId))
			return false;
		fOpenPopup = true;
		return true;
	}
	return false;
}

void AddMultiModelRunToMenu(boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiMenuItemList &thePopupMenu, unsigned int theDescTopIndex)
{
	if(theDrawParam)
	{
		NFmiDataIdent param = theDrawParam->Param();
		std::string menuString = "SetMultiRunMode"; // ::GetDictionaryString("TimeSerialViewParamPopUpRemove");
        auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, menuString, param, kFmiTimeSerialModelRunCountSet, g_DefaultParamView, nullptr, theDrawParam->DataType());

		NFmiMenuItemList *menuList = new NFmiMenuItemList;
		for(int i = 0; i <= 10; i++)
		{
			std::string multiModeStr;
			if(i == 0)
				multiModeStr += "Set normal (0)";
			else
				multiModeStr += std::string("Count to ") + NFmiStringTools::Convert(i);

            auto item1 = std::make_unique<NFmiMenuItem>(theDescTopIndex, multiModeStr, param, kFmiTimeSerialModelRunCountSet, g_DefaultParamView, nullptr, theDrawParam->DataType());
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
	itsCurrentViewRowIndex = CtrlViewUtils::kFmiTimeSerialView;
	GetCombinedMapHandler()->getTimeSerialViewIndexReference() = index;
	fOpenPopup = false;
	itsPopupMenu = std::make_unique<NFmiMenuItemList>();

	MenuCreationSettings menuSettings;
	menuSettings.SetTimeSerialSettings(kFmiAddTimeSerialSideParam);
	CreateParamSelectionBasePopup(menuSettings, itsPopupMenu.get(), "Add side parameter");

	auto sideParamList = GetCombinedMapHandler()->getTimeSerialViewSideParameters(index);
	if(sideParamList && sideParamList->NumberOfItems() > 0)
	{
		// Yksitt‰isen side-paramin poistot
		auto removeSideParamMenuItem = std::make_unique<NFmiMenuItem>(CtrlViewUtils::kFmiTimeSerialView, "Remove side parameter", kFmiBadParameter, kFmiRemoveSelectedTimeSerialSideParam, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
		auto removeSideParamMenuItemList = new NFmiMenuItemList();
		for(sideParamList->Reset(); sideParamList->Next(); )
		{
			auto currentSideParam = sideParamList->Current();
			auto paramNameInMenu = CtrlViewUtils::GetParamNameString(currentSideParam, false, false, false, 0, false, false, true, nullptr);
			const auto &dataIdent = currentSideParam->Param();
			const auto &level = currentSideParam->Level();
			auto sideParamMenuItem = std::make_unique<NFmiMenuItem>(CtrlViewUtils::kFmiTimeSerialView, paramNameInMenu, dataIdent, kFmiRemoveSelectedTimeSerialSideParam, g_DefaultParamView, &level, currentSideParam->DataType());
			removeSideParamMenuItemList->Add(std::move(sideParamMenuItem));
		}
		removeSideParamMenuItem->AddSubMenu(removeSideParamMenuItemList);
		itsPopupMenu->Add(std::move(removeSideParamMenuItem));

		// Kaikkien side-paramien poisto kerralla
		auto clearAllSideParamsMenuItem = std::make_unique<NFmiMenuItem>(CtrlViewUtils::kFmiTimeSerialView, "Clear side parameters", kFmiBadParameter, kFmiRemoveAllTimeSerialRowSideParams, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
		itsPopupMenu->Add(std::move(clearAllSideParamsMenuItem));
	}

	auto& timeSerialViewDrawParamList = GetCombinedMapHandler()->getTimeSerialViewDrawParamList();
	if(timeSerialViewDrawParamList.Index(index))
		AddMultiModelRunToMenu(timeSerialViewDrawParamList.Current(), *itsPopupMenu, CtrlViewUtils::kFmiTimeSerialView);

	if(!itsPopupMenu->InitializeCommandIDs(itsPopupMenuStartId))
		return false;
	fOpenPopup = true;
	return true;
}

void AddShowHelperDataOntimeSerialViewPopup(const NFmiDataIdent &param, NFmiInfoData::Type infoDataType, const std::string &dictionaryStr, FmiMenuCommandType commandId, const std::string &acceleratorHelpStr)
{
    std::string menuString = ::GetDictionaryString(dictionaryStr.c_str());
    menuString += acceleratorHelpStr;
    auto menuItem = std::make_unique<NFmiMenuItem>(-1, menuString, param, commandId, g_DefaultParamView, nullptr, infoDataType);
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

void AddShowHelperData2OntimeSerialViewPopup(const NFmiDataIdent& param, NFmiInfoData::Type infoDataType)
{
	std::string acceleratorHelpStr(" (CTRL + F)");
	if(!ShowHelperData2InTimeSerialView())
		AddShowHelperDataOntimeSerialViewPopup(param, infoDataType, "TimeSerialViewParamPopUpShowHelpData2", kFmiShowHelperData2OnTimeSerialView, acceleratorHelpStr);
	else
		AddShowHelperDataOntimeSerialViewPopup(param, infoDataType, "TimeSerialViewParamPopUpHideHelpData2", kFmiDontShowHelperData2OnTimeSerialView, acceleratorHelpStr);
}

void AddShowHelperData3OntimeSerialViewPopup(const NFmiDataIdent &param, NFmiInfoData::Type infoDataType)
{
    std::string acceleratorHelpStr(" (CTRL + SHIFT + F)");
	NFmiLevel defaultSurfaceLevel;
    if(GetModelClimatologyData(defaultSurfaceLevel))
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
	itsCurrentViewRowIndex = CtrlViewUtils::kFmiTimeSerialView;
	GetCombinedMapHandler()->getTimeSerialViewIndexReference() = index;
	fOpenPopup = false;
	NFmiInfoData::Type infoDataType = NFmiInfoData::kEditable;
	auto& timeSerialViewDrawParamList = GetCombinedMapHandler()->getTimeSerialViewDrawParamList();
	itsPopupMenu = std::make_unique<NFmiMenuItemList>();

	MenuCreationSettings menuSettings;
	menuSettings.SetTimeSerialSettings(kFmiAddTimeSerialView);
	CreateParamSelectionBasePopup(menuSettings, itsPopupMenu.get(), "MapViewMaskSelectionPopUpAdd");

	if(timeSerialViewDrawParamList.Index(index))
	{
		NFmiDataIdent param = timeSerialViewDrawParamList.Current()->Param();
		std::string menuString = ::GetDictionaryString("TimeSerialViewParamPopUpRemove");
		auto menuItem = std::make_unique<NFmiMenuItem>(-1, menuString, param, kFmiRemoveTimeSerialView, g_DefaultParamView, nullptr, infoDataType);
		itsPopupMenu->Add(std::move(menuItem));
		menuString = ::GetDictionaryString("TimeSerialViewParamPopUpRemoveAll");
		menuItem.reset(new NFmiMenuItem(-1, menuString, param, kFmiRemoveAllTimeSerialViews, g_DefaultParamView, nullptr, infoDataType));
		itsPopupMenu->Add(std::move(menuItem));

		AddShowHelperData1OntimeSerialViewPopup(param, infoDataType);
		AddShowHelperData2OntimeSerialViewPopup(param, infoDataType);
		AddShowHelperData3OntimeSerialViewPopup(param, infoDataType);
		AddShowHelperData4OntimeSerialViewPopup(param, infoDataType);

		if(timeSerialViewDrawParamList.NumberOfItems())
		{
			menuString = ::GetDictionaryString("TimeSerialViewParamPopUpProperties");
			menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiAddTimeSerialView, g_DefaultParamView, nullptr, infoDataType));
			NFmiMenuItemList* menuList = new NFmiMenuItemList;
			int aIndex = 1;
			for(timeSerialViewDrawParamList.Reset(); timeSerialViewDrawParamList.Next();)
			{
				auto item5 = std::make_unique<NFmiMenuItem>(-1, timeSerialViewDrawParamList.Current()->ParameterAbbreviation()
					, timeSerialViewDrawParamList.Current()->Param()
					, kFmiModifyDrawParam
					, g_DefaultParamView
					, &timeSerialViewDrawParamList.Current()->Level()
					, infoDataType
					, aIndex
					, timeSerialViewDrawParamList.Current()->ViewMacroDrawParam());
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

std::string GetFileName(const std::string &theFilePath)
{
    NFmiFileString fileString(theFilePath);
    return std::string(fileString.Header());
}

void AddFixedDrawParamsToMenu(const NFmiFixedDrawParamFolder &theFixedDrawParamFolder, const std::string &theSubFolderName, NFmiMenuItemList &thePopupMenu, unsigned int theDescTopIndex, int theRowIndex, int index, const NFmiDataIdent &theParam, const NFmiLevel* theLevel, NFmiInfoData::Type theDataType, boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
    if(!theFixedDrawParamFolder.Empty())
    {
        auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, theSubFolderName, theParam, kFmiFixedDrawParam, NFmiMetEditorTypes::View::kFmiIsoLineView, theLevel, theDataType, index, theDrawParam->ViewMacroDrawParam());
        NFmiMenuItemList *subFolderMenuList = new NFmiMenuItemList;

        const std::vector<NFmiFixedDrawParamFolder> &subFolders = theFixedDrawParamFolder.SubFolders();
        for(auto subFolder : subFolders)
            AddFixedDrawParamsToMenu(subFolder, subFolder.SubFolderName(), *subFolderMenuList, theDescTopIndex, theRowIndex, index, theParam, theLevel, theDataType, theDrawParam);

        const std::vector<std::shared_ptr<NFmiDrawParam>> &drawParams = theFixedDrawParamFolder.DrawParams();
        for(auto drawParam : drawParams)
        {
            std::string menuText = GetFileName(drawParam->InitFileName());
            auto drawParamMenuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, menuText, theParam, kFmiFixedDrawParam, NFmiMetEditorTypes::View::kFmiIsoLineView, theLevel, theDataType, index, theDrawParam->ViewMacroDrawParam());
            drawParamMenuItem->MacroParamInitName(drawParam->InitFileName()); // t‰m‰n avulla haluttu drawParam etsit‰‰n
            subFolderMenuList->Add(std::move(drawParamMenuItem));
        }

        menuItem->AddSubMenu(subFolderMenuList);
        thePopupMenu.Add(std::move(menuItem));
    }
    else
    { // theFixedDrawParamFolder oli tyhj‰, varoitetaan k‰ytt‰j‰‰ ett‰ ei ole lˆytynyt yht‰‰n tehdasasetuksia
        auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, ::GetDictionaryString("No FixedDrawParams were found!"), theParam, kFmiFixedDrawParam, NFmiMetEditorTypes::View::kFmiIsoLineView, theLevel, theDataType, index, theDrawParam->ViewMacroDrawParam());
        thePopupMenu.Add(std::move(menuItem));
    }
}

void SetLayerIndexForWantedMenucommandItems(NFmiMenuItemList& thePopupMenu, FmiMenuCommandType wantedMenucommand, int layerIndex)
{
	for(const auto &menuItem : thePopupMenu)
	{
		if(menuItem->CommandType() == wantedMenucommand)
			menuItem->IndexInViewRow(layerIndex);
		auto* subMenuList = menuItem->SubMenu();
		if(subMenuList)
			SetLayerIndexForWantedMenucommandItems(*subMenuList, wantedMenucommand, layerIndex);
	}
}

void AddInsertParamLayerSectionIntoPopupMenu(NFmiMenuItemList *thePopupMenu, unsigned int theDescTopIndex, int theRowIndex, int layerIndex, double layerIndexRealValue)
{
	MenuCreationSettings menuSettings;
	menuSettings.SetIsolineViewSettings(theDescTopIndex, kFmiInsertParamLayer);
	CreateParamSelectionBasePopup(menuSettings, thePopupMenu, "Insert new parameter in here");
	int wantedLayerIndex = boost::math::iround(layerIndexRealValue);
	SetLayerIndexForWantedMenucommandItems(*thePopupMenu, kFmiInsertParamLayer, wantedLayerIndex);
}

void AddChangeParamSectionIntoPopupMenu(NFmiMenuItemList* thePopupMenu, unsigned int theDescTopIndex, int layerIndex, boost::shared_ptr<NFmiDrawParam>& drawParam)
{
	if(drawParam && !NFmiDrawParam::IsMacroParamCase(drawParam->DataType()))
	{
		auto crossSectionCase = theDescTopIndex == CtrlViewUtils::kFmiCrossSectionView;
		auto smartInfo = InfoOrganizer()->Info(drawParam, crossSectionCase, false);
		if(smartInfo)
		{
			auto menuCommand = kFmiChangeParam;
			auto dataType = drawParam->DataType();
			auto menuTitleStr = ::GetDictionaryString("Change parameter/level");
			auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, menuTitleStr, NFmiDataIdent(), menuCommand, g_DefaultParamView, nullptr, dataType);
			NFmiLevelBag levels;
			if(!crossSectionCase && smartInfo->SizeLevels() > 1)
			{
				levels = smartInfo->VPlaceDescriptor().LevelBag();
			}
			auto paramBag = smartInfo->ParamBag();
			bool allowStreamlineParameter = theDescTopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex;
			auto possibleWindMetaParams = NFmiFastInfoUtils::MakePossibleWindMetaParams(*smartInfo, allowStreamlineParameter);
			NFmiMenuItemList* menuList = nullptr;
			// Poikkileikkaus menun yhteydess‰ ei halutakaan laittaa level tietoja menu-popupiin, vain parametrit
			if(crossSectionCase == false && levels.GetSize() > 1)
				menuList = new NFmiMenuItemList(theDescTopIndex, &paramBag, menuCommand, g_DefaultParamView, &levels, dataType, kFmiLastParameter, &possibleWindMetaParams);
			else
				menuList = new NFmiMenuItemList(theDescTopIndex, &paramBag, menuCommand, g_DefaultParamView, dataType, &possibleWindMetaParams);

			if(menuList && menuList->NumberOfMenuItems() > 0)
			{
				SetLayerIndexForWantedMenucommandItems(*menuList, menuCommand, layerIndex);

				menuItem->AddSubMenu(menuList);
				thePopupMenu->Add(std::move(menuItem));
			}
		}
	}
}

bool IsMapLayerSelectionCase(unsigned int theDescTopIndex, int theRowIndex, int layerIndex)
{
	return (theDescTopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex) && (layerIndex == 0);
}

void AddSetLocationForTimeBoxPopup(std::unique_ptr<NFmiMenuItemList>& subMenuList, unsigned int theDescTopIndex, FmiDirection wantedLocation, std::string menuNameString)
{
	auto mapViewDescTop = GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex);
	// Lis‰t‰‰n annettu uusi paikka popupiin, vain jos se ei ole sama kuin nykyinen sijainti
	if(mapViewDescTop && mapViewDescTop->TimeBoxLocation() != wantedLocation)
	{
		// Varsinaiset halutut locationit talletetaan menuItem luokan extraParam kohdassa (FmiDirection enum castataan doubleksi)
		auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, menuNameString, NFmiDataIdent(), kFmiSetTimeBoxLocation, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
		menuItem->ExtraParam(static_cast<double>(wantedLocation));
		subMenuList->Add(std::move(menuItem));
	}
}

void AddSetLocationSubMenuForTimeBoxPopup(std::unique_ptr<NFmiMenuItemList>& mainMenuList, unsigned int theDescTopIndex)
{
	std::string finalSetLocationMenuString = ::GetDictionaryString("Set time legend location");
	auto setLocationMenuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, finalSetLocationMenuString, NFmiDataIdent(), kFmiSetTimeBoxLocation, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
	auto setLocationSubMenuItemList = std::make_unique<NFmiMenuItemList>();
	AddSetLocationForTimeBoxPopup(setLocationSubMenuItemList, theDescTopIndex, kBottomLeft, "Bottom-left");
	AddSetLocationForTimeBoxPopup(setLocationSubMenuItemList, theDescTopIndex, kTopLeft, "Top-left");
	AddSetLocationForTimeBoxPopup(setLocationSubMenuItemList, theDescTopIndex, kTopCenter, "Top-center");
	AddSetLocationForTimeBoxPopup(setLocationSubMenuItemList, theDescTopIndex, kTopRight, "Top-right");
	AddSetLocationForTimeBoxPopup(setLocationSubMenuItemList, theDescTopIndex, kBottomRight, "Bottom-right");
	AddSetLocationForTimeBoxPopup(setLocationSubMenuItemList, theDescTopIndex, kBottomCenter, "Bottom-center");

	setLocationMenuItem->AddSubMenu(setLocationSubMenuItemList.release());
	mainMenuList->Add(std::move(setLocationMenuItem));
}

void AddSetTextSizeFactorToSubMenu(std::unique_ptr<NFmiMenuItemList>& subMenuList, unsigned int theDescTopIndex, float textSizeFactor)
{
	auto mapViewDescTop = GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex);
	// Lis‰t‰‰n annettu uusi paikka popupiin, vain jos se ei ole sama kuin nykyinen sijainti
	if(mapViewDescTop && mapViewDescTop->TimeBoxTextSizeFactor() != textSizeFactor)
	{
		std::string textSizeFactorStr = NFmiValueString::GetStringWithMaxDecimalsSmartWay(textSizeFactor, 1);
		auto setTextSizeValueMenuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, textSizeFactorStr, NFmiDataIdent(), kFmiSetTimeBoxTextSizeFactor, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
		// Haluttu size factor talletetaan menuItem luokan extraParam kohdassa
		setTextSizeValueMenuItem->ExtraParam(textSizeFactor);
		subMenuList->Add(std::move(setTextSizeValueMenuItem));
	}
}

void AddSetTextSizeFactorSubMenuForTimeBoxPopup(std::unique_ptr<NFmiMenuItemList>& mainMenuList, unsigned int theDescTopIndex)
{
	std::string finalSetLocationMenuString = ::GetDictionaryString("Set text size factor");
	auto setTextSizeMenuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, finalSetLocationMenuString, NFmiDataIdent(), kFmiSetTimeBoxTextSizeFactor, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
	auto setTextSizeSubMenuItemList = std::make_unique<NFmiMenuItemList>();

	for(auto textSizeFactor : NFmiMapViewDescTop::TimeBoxTextSizeAllowedFactors())
	{
		AddSetTextSizeFactorToSubMenu(setTextSizeSubMenuItemList, theDescTopIndex, textSizeFactor);
	}
	setTextSizeMenuItem->AddSubMenu(setTextSizeSubMenuItemList.release());
	mainMenuList->Add(std::move(setTextSizeMenuItem));
}

void AddSetAlphaToSubMenu(std::unique_ptr<NFmiMenuItemList>& subMenuList, unsigned int theDescTopIndex, float alphaValue)
{
	auto mapViewDescTop = GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex);
	// Lis‰t‰‰n annettu uusi paikka popupiin, vain jos se ei ole sama kuin nykyinen sijainti
	if(mapViewDescTop && mapViewDescTop->GetTimeBoxFillColorAlpha() != alphaValue)
	{
		std::string alphaStr = std::to_string(boost::math::iround(alphaValue * 100));
		alphaStr += " %";
		auto setAlphaMenuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, alphaStr, NFmiDataIdent(), kFmiSetTimeBoxFillColorAlpha, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
		// Haluttu alpha kerroin talletetaan k‰‰nteisen‰ arvona (johtuen NFmiColor luokan k‰‰nteisest‰ alpha kanavan jutusta) menuItem luokan extraParam kohdassa
		setAlphaMenuItem->ExtraParam(alphaValue);
		subMenuList->Add(std::move(setAlphaMenuItem));
	}
}

const std::vector<float> gTimeBoxFillColorAlphaValues{ 0, 0.05f, 0.1f, 0.15f, 0.2f, 0.25f, 0.3f, 0.35f, 0.4f, 0.45f, 0.5f, 0.55f, 0.6f, 0.65f, 0.7f, 0.75f, 0.8f, 0.85f, 0.9f, 0.95f, 1 };

void AddSetTimeBoxFillColorAlphaSubMenuForTimeBoxPopup(std::unique_ptr<NFmiMenuItemList>& mainMenuList, unsigned int theDescTopIndex)
{
	std::string finalSetLocationMenuString = ::GetDictionaryString("Set fill color alpha");
	auto setTextSizeMenuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, finalSetLocationMenuString, NFmiDataIdent(), kFmiSetTimeBoxFillColorAlpha, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
	auto setTextSizeSubMenuItemList = std::make_unique<NFmiMenuItemList>();

	for(auto alphaValue : gTimeBoxFillColorAlphaValues)
	{
		AddSetAlphaToSubMenu(setTextSizeSubMenuItemList, theDescTopIndex, alphaValue);
	}
	setTextSizeMenuItem->AddSubMenu(setTextSizeSubMenuItemList.release());
	mainMenuList->Add(std::move(setTextSizeMenuItem));
}

const std::string gTimeBoxFillColorCustomName = "custom color";

void AddSetTimeBoxFillColorToSubMenu(std::unique_ptr<NFmiMenuItemList>& subMenuList, unsigned int theDescTopIndex, const std::string& name, const NFmiColor& color)
{
	auto mapViewDescTop = GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex);
	// Lis‰t‰‰n annettu uusi paikka popupiin, vain jos se ei ole sama kuin nykyinen sijainti
	if(mapViewDescTop)
	{
		bool useCustomColor = (name == gTimeBoxFillColorCustomName);
		auto usedMenuCommand = useCustomColor ? kFmiSetTimeBoxCustomFillColor : kFmiSetTimeBoxFillColor;
		auto setAlphaMenuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, name, NFmiDataIdent(), usedMenuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
		// Haluttu alpha kerroin talletetaan menuItem luokan extraParam kohdassa
		setAlphaMenuItem->ExtraTextParam(NFmiDrawParam::Color2String(color));
		subMenuList->Add(std::move(setAlphaMenuItem));
	}
}

void DoTimeBoxCustomFillColorSetup(NFmiMenuItem& theMenuItem)
{
	auto* parentView = ApplicationInterface::GetApplicationInterfaceImplementation()->GetView(theMenuItem.MapViewDescTopIndex());
	auto color = NFmiDrawParam::String2Color(theMenuItem.ExtraTextParam());
	auto colorRef = CtrlView::Color2ColorRef(color);
	CColorDialog dlg(colorRef, 0, parentView);
	if(dlg.DoModal() == IDOK)
	{
		auto newColorRef = dlg.GetColor();
		auto newColor = CtrlView::ColorRef2Color(newColorRef);

		GetCombinedMapHandler()->onSetTimeBoxFillColor(theMenuItem.MapViewDescTopIndex(), newColor);
	}
}

void DoTimeBoxToDefaultValues(NFmiMenuItem& theMenuItem)
{
	auto combinedMapHandler = GetCombinedMapHandler();
	combinedMapHandler->onSetTimeBoxLocation(theMenuItem.MapViewDescTopIndex(), NFmiMapViewDescTop::TimeBoxLocationDefault);
	combinedMapHandler->onSetTimeBoxTextSizeFactor(theMenuItem.MapViewDescTopIndex(), NFmiMapViewDescTop::TimeBoxTextSizeFactorDefault);
	combinedMapHandler->onSetTimeBoxFillColor(theMenuItem.MapViewDescTopIndex(), NFmiMapViewDescTop::TimeBoxFillColorDefault);
	combinedMapHandler->onSetTimeBoxFillColorAlpha(theMenuItem.MapViewDescTopIndex(), NFmiMapViewDescTop::TimeBoxFillColorDefault.Alpha());
}

void SetTimeBoxFillColorFromMenu(NFmiMenuItem& theMenuItem)
{
	if(theMenuItem.ExtraTextParam() == gTimeBoxFillColorCustomName)
	{
		DoTimeBoxCustomFillColorSetup(theMenuItem);
	}
	else
	{
		GetCombinedMapHandler()->onSetTimeBoxFillColor(theMenuItem.MapViewDescTopIndex(), NFmiDrawParam::String2Color(theMenuItem.ExtraTextParam()));
	}
}

const float gBaseChannelValue = 0.85f;

const std::vector<std::pair<std::string, NFmiColor>> gTimeBoxFillColorsWithNames
{ 
	std::make_pair(gTimeBoxFillColorCustomName, NFmiColor(1,1,1,0.4f)),
	std::make_pair("white", NFmiColor(1,1,1,0.4f)),
	std::make_pair("light grey", NFmiColor(gBaseChannelValue,gBaseChannelValue,gBaseChannelValue,0.4f)),
	std::make_pair("light green", NFmiColor(gBaseChannelValue,1,gBaseChannelValue,0.4f)),
	std::make_pair("light red", NFmiColor(1,gBaseChannelValue,gBaseChannelValue,0.4f)),
	std::make_pair("light blue", NFmiColor(gBaseChannelValue,gBaseChannelValue,1,0.4f)),
	std::make_pair("light yellow", NFmiColor(1,1,gBaseChannelValue,0.4f)),
	std::make_pair("light magenta", NFmiColor(1,gBaseChannelValue,1,0.4f)),
	std::make_pair("light cyan", NFmiColor(gBaseChannelValue,1,1,0.4f)),
};

void AddSetTimeBoxFillColorSubMenuForTimeBoxPopup(std::unique_ptr<NFmiMenuItemList>& mainMenuList, unsigned int theDescTopIndex)
{
	std::string finalSetLocationMenuString = ::GetDictionaryString("Set fill color");
	auto setTextSizeMenuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, finalSetLocationMenuString, NFmiDataIdent(), kFmiSetTimeBoxFillColor, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
	auto setTextSizeSubMenuItemList = std::make_unique<NFmiMenuItemList>();

	for(const auto &nameColorPair : gTimeBoxFillColorsWithNames)
	{
		AddSetTimeBoxFillColorToSubMenu(setTextSizeSubMenuItemList, theDescTopIndex, nameColorPair.first, nameColorPair.second);
	}
	setTextSizeMenuItem->AddSubMenu(setTextSizeSubMenuItemList.release());
	mainMenuList->Add(std::move(setTextSizeMenuItem));
}

void AddSetTimeBoxSetToDefaultForTimeBoxPopup(std::unique_ptr<NFmiMenuItemList>& mainMenuList, unsigned int theDescTopIndex)
{
	std::string setToDefaultMenuString = ::GetDictionaryString("Set to default");
	auto setToDefaultMenuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, setToDefaultMenuString, NFmiDataIdent(), kFmiSetTimeBoxToDefaultValues, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
	mainMenuList->Add(std::move(setToDefaultMenuItem));
}

bool CreateMapViewTimeBoxPopup(unsigned int theDescTopIndex)
{
	if(theDescTopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex)
	{
		itsPopupMenu = std::make_unique<NFmiMenuItemList>();

		AddSetTimeBoxSetToDefaultForTimeBoxPopup(itsPopupMenu, theDescTopIndex);
		AddSetLocationSubMenuForTimeBoxPopup(itsPopupMenu, theDescTopIndex);
		AddSetTextSizeFactorSubMenuForTimeBoxPopup(itsPopupMenu, theDescTopIndex);
		AddSetTimeBoxFillColorAlphaSubMenuForTimeBoxPopup(itsPopupMenu, theDescTopIndex);
		AddSetTimeBoxFillColorSubMenuForTimeBoxPopup(itsPopupMenu, theDescTopIndex);

		if(!itsPopupMenu->InitializeCommandIDs(itsPopupMenuStartId))
			return false;

		fOpenPopup = true;
		return true;
	}
	return false;
}

bool CreateViewParamsPopup(unsigned int theDescTopIndex, int theRowIndex, int layerIndex, double layerIndexRealValue)
{
	if(IsMapLayerSelectionCase(theDescTopIndex, theRowIndex, layerIndex))
		return CreateMapLayerSelectionPopup(theDescTopIndex);

	itsCurrentViewRowIndex = theRowIndex;
	fOpenPopup = false;
	NFmiDrawParamList* rowDrawParamList = GetCombinedMapHandler()->getDrawParamList(theDescTopIndex, theRowIndex);
	if(itsSmartInfoOrganizer && rowDrawParamList)
	{
		if(rowDrawParamList->Index(layerIndex))
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
			itsPopupMenu = std::make_unique<NFmiMenuItemList>();
			std::string menuString;
			std::unique_ptr<NFmiMenuItem> menuItem;

			AddInsertParamLayerSectionIntoPopupMenu(itsPopupMenu.get(), theDescTopIndex, theRowIndex, layerIndex, layerIndexRealValue);
			AddChangeParamSectionIntoPopupMenu(itsPopupMenu.get(), theDescTopIndex, layerIndex, drawParam);

			if(crossSectionPopup == false)
			{ // poikkileikkaus-n‰yttˆ ei tue tekstimuotoista piirtoa
				menuString = ::GetDictionaryString("MapViewParamOptionPopUpText");
				menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiModifyView, NFmiMetEditorTypes::View::kFmiTextView, level, dataType, layerIndex, drawParam->ViewMacroDrawParam()));
				itsPopupMenu->Add(std::move(menuItem));
			}
			menuString = ::GetDictionaryString("MapViewParamOptionPopUpIsoline");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiModifyView, NFmiMetEditorTypes::View::kFmiIsoLineView, level, dataType, layerIndex, drawParam->ViewMacroDrawParam()));
			itsPopupMenu->Add(std::move(menuItem));

			menuString = ::GetDictionaryString("MapViewParamOptionPopUpContour");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiModifyView, NFmiMetEditorTypes::View::kFmiColorContourView, level, dataType, layerIndex, drawParam->ViewMacroDrawParam()));
			itsPopupMenu->Add(std::move(menuItem));
			menuString = ::GetDictionaryString("MapViewParamOptionPopUpContourIsoline");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiModifyView, NFmiMetEditorTypes::View::kFmiColorContourIsoLineView, level, dataType, layerIndex, drawParam->ViewMacroDrawParam()));
			itsPopupMenu->Add(std::move(menuItem));
			menuString = ::GetDictionaryString("MapViewParamOptionPopUpQuickContour");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiModifyView, NFmiMetEditorTypes::View::kFmiQuickColorContourView, level, dataType, layerIndex, drawParam->ViewMacroDrawParam()));
			itsPopupMenu->Add(std::move(menuItem));

			menuString = ::GetDictionaryString("MapViewParamOptionPopUpRemove");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiRemoveView, NFmiMetEditorTypes::View::kFmiIsoLineView, level, dataType, layerIndex, drawParam->ViewMacroDrawParam()));
			itsPopupMenu->Add(std::move(menuItem));

            if(drawParam->ShowColorLegend())
                menuString = ::GetDictionaryString("Hide legend");
            else
                menuString = ::GetDictionaryString("Show legend");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiToggleShowLegendState, NFmiMetEditorTypes::View::kFmiIsoLineView, level, dataType, layerIndex, drawParam->ViewMacroDrawParam()));
            itsPopupMenu->Add(std::move(menuItem));

            menuString = ::GetDictionaryString("MapViewParamOptionPopUpHide");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiHideView, NFmiMetEditorTypes::View::kFmiIsoLineView, level, dataType, layerIndex, drawParam->ViewMacroDrawParam()));
			itsPopupMenu->Add(std::move(menuItem));
			menuString = ::GetDictionaryString("MapViewParamOptionPopUpShow");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiShowView, NFmiMetEditorTypes::View::kFmiIsoLineView, level, dataType, layerIndex, drawParam->ViewMacroDrawParam()));
			itsPopupMenu->Add(std::move(menuItem));
			menuString = ::GetDictionaryString("MapViewParamOptionPopUpActivate");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiActivateView, NFmiMetEditorTypes::View::kFmiIsoLineView, level, dataType, layerIndex, drawParam->ViewMacroDrawParam()));
			menuItem->ExtraParam(0); // kun parametria aktivoidaan (tai tulevaisuudessa tehd‰‰n mit‰ vain), k‰ytet‰‰n extraParamia kertomaan miss‰ parametri on, 0=karttan‰yttˆ, 1=poikkileikkaus ja 2=aikasarja
			itsPopupMenu->Add(std::move(menuItem));
            menuString = ::GetDictionaryString("Save DrawParam");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiStoreDrawParam, NFmiMetEditorTypes::View::kFmiIsoLineView, level, dataType, layerIndex, drawParam->ViewMacroDrawParam()));
            itsPopupMenu->Add(std::move(menuItem));
            
            menuString = ::GetDictionaryString("Reload DrawParam");
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiReloadDrawParam, NFmiMetEditorTypes::View::kFmiIsoLineView, level, dataType, layerIndex, drawParam->ViewMacroDrawParam()));
            itsPopupMenu->Add(std::move(menuItem));

            // FixedDrawParam valikko t‰h‰n v‰liin
            AddFixedDrawParamsToMenu(itsFixedDrawParamSystem.RootFolder(), ::GetDictionaryString("FixedDrawParams"), *itsPopupMenu, theDescTopIndex, theRowIndex, layerIndex, param, level, dataType, drawParam);

			// copy/paste komennot t‰h‰n
			menuString = "Copy draw options";
            menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiCopyDrawParamOptions, NFmiMetEditorTypes::View::kFmiIsoLineView, level, dataType, layerIndex, drawParam->ViewMacroDrawParam()));
			itsPopupMenu->Add(std::move(menuItem));
			if(GetCombinedMapHandler()->copyPasteDrawParamAvailableYet())
			{
				menuString = "Paste draw options";
                menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiPasteDrawParamOptions, NFmiMetEditorTypes::View::kFmiIsoLineView, level, dataType, layerIndex, drawParam->ViewMacroDrawParam()));
				itsPopupMenu->Add(std::move(menuItem));
			}

			if(!macroParamInCase)
			{
				if(crossSectionPopup == false)
				{
					if(drawParam && drawParam->ShowDifferenceToOriginalData())
					{
						menuString = ::GetDictionaryString("MapViewParamOptionPopUpOrigDiffOff");
						menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiToggleShowDifferenceToOriginalData, NFmiMetEditorTypes::View::kFmiIsoLineView, level, dataType, layerIndex, drawParam->ViewMacroDrawParam()));
					}
					else
					{
						menuString = ::GetDictionaryString("MapViewParamOptionPopUpOrigDiffOn");
						menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiToggleShowDifferenceToOriginalData, NFmiMetEditorTypes::View::kFmiIsoLineView, level, dataType, layerIndex, drawParam->ViewMacroDrawParam()));
					}
					itsPopupMenu->Add(std::move(menuItem));
				}
			}

			if(mapViewSectionPopup)
				AddBorderLayerActionToPopup(theDescTopIndex, theRowIndex, layerIndex, layerIndexRealValue, itsPopupMenu.get());

			menuString = ::GetDictionaryString("MapViewParamOptionPopUpProperties");
			menuItem.reset(new NFmiMenuItem(theDescTopIndex, menuString, param, kFmiModifyDrawParam, NFmiMetEditorTypes::View::kFmiIsoLineView, level, dataType, layerIndex, drawParam->ViewMacroDrawParam()));
			itsPopupMenu->Add(std::move(menuItem));

			if(!itsPopupMenu->InitializeCommandIDs(itsPopupMenuStartId))
				return false;
			fOpenPopup = true;
			return true;
		}
	}
	return false;
}

void AddBorderLayerActionToPopup(unsigned int theDescTopIndex, int theRowIndex, int layerIndex, double layerIndexRealValue, NFmiMenuItemList* theMenuList)
{
	if(theDescTopIndex > CtrlViewUtils::kFmiMaxMapDescTopIndex)
		return; // Jos kyse muusta kuin karttan‰ytˆst‰, ei tehd‰ mit‰‰n...

	auto *drawParamList = GetCombinedMapHandler()->getDrawParamList(theDescTopIndex, theRowIndex);
	if(drawParamList)
	{
		FmiMenuCommandType commandType = kFmiAddBorderLineLayer;
		std::string commandText = ::GetDictionaryString("Add country border layer here");
		auto existingBorderLayerIndex = CombinedMapHandlerInterface::getBorderLayerIndex(drawParamList);
		if(layerIndex == existingBorderLayerIndex)
			return; // Ei tarvitse tehd‰ mit‰‰n, jos hiirell‰ osoitetaan jo suoraan border-layeria
		auto borderLayerExist = (existingBorderLayerIndex != -1);
		if(borderLayerExist)
		{
			commandType = kFmiMoveBorderLineLayer;
			commandText = ::GetDictionaryString("Move country border layer here");
		}
		// Todellinen k‰ytt‰j‰n haluama layer-indeksi (mihin uusi layer lis‰t‰‰n tai siirret‰‰n) on pyˆristys layerIndexRealValue:sta.
		int wantedLayerIndex = boost::math::iround(layerIndexRealValue);
		auto menuItem = std::make_unique<NFmiMenuItem>(theDescTopIndex, commandText, NFmiDataIdent(), commandType, g_DefaultParamView, nullptr, NFmiInfoData::kMapLayer, wantedLayerIndex);
		theMenuList->Add(std::move(menuItem));
	}
}


bool CreateMaskParamsPopup(int theRowIndex, int index)
{
	itsCurrentViewRowIndex = theRowIndex;
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

			itsPopupMenu = std::make_unique<NFmiMenuItemList>();
			std::string menuString = ::GetDictionaryString("NormalWordCapitalRemove");
            auto menuItem = std::make_unique<NFmiMenuItem>(-1, menuString, param, kFmiRemoveMask, g_DefaultParamView, level, dataType, index);
			itsPopupMenu->Add(std::move(menuItem));
			menuString = ::GetDictionaryString("NormalWordCapitalUse");
			menuItem.reset(new NFmiMenuItem(-1, menuString, param, kFmiEnAbleMask, g_DefaultParamView, level, dataType, index));
			itsPopupMenu->Add(std::move(menuItem));
			menuString = ::GetDictionaryString("NormalWordCapitalDisable");
			menuItem.reset(new NFmiMenuItem(-1, menuString, param, kFmiDisAbleMask, g_DefaultParamView, level, dataType, index));
			itsPopupMenu->Add(std::move(menuItem));
			menuString = ::GetDictionaryString("NormalWordCapitalModify");
			menuItem.reset(new NFmiMenuItem(-1, menuString, param, kFmiModifyMask, g_DefaultParamView, level, dataType, index));
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
		GetCombinedMapHandler()->modifyView(theMenuItem, theRowIndex);
		break;
	case kFmiModifyDrawParam:
		GetCombinedMapHandler()->modifyDrawParam(theMenuItem, theRowIndex);
		break;
	case kFmiActivateView:
		GetCombinedMapHandler()->activateView(theMenuItem, theRowIndex);
        break;
    case kAddViewWithRealRowNumber:
		GetCombinedMapHandler()->addViewWithRealRowNumber(true, theMenuItem, theRowIndex, false);
        break;
	case kFmiAddParamCrossSectionView:
		GetCombinedMapHandler()->addCrossSectionView(theMenuItem, theRowIndex, false);
		break;
	case kFmiAddTimeSerialView:
		GetCombinedMapHandler()->getTimeSerialViewIndexReference() = theRowIndex;
		GetCombinedMapHandler()->addTimeSerialView(theMenuItem, false);
		break;
	case kFmiAddTimeSerialSideParam:
		GetCombinedMapHandler()->getTimeSerialViewIndexReference() = theRowIndex;
		GetCombinedMapHandler()->addTimeSerialViewSideParameter(theMenuItem, false);
		break;
	default:
        return false;
    }

	GetCombinedMapHandler()->makeMacroParamCacheUpdatesForWantedRow(theMenuItem.MapViewDescTopIndex(), theRowIndex);
    return true;
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
			GetCombinedMapHandler()->addView(*menuItem, itsCurrentViewRowIndex);
			break;
		case kFmiAddParamCrossSectionView:
			GetCombinedMapHandler()->addCrossSectionView(*menuItem, itsCurrentCrossSectionRowIndex, false);
			break;
		case kFmiChangeParam:
		{
			auto crossSectionCase = menuItem->MapViewDescTopIndex() == CtrlViewUtils::kFmiCrossSectionView;
			GetCombinedMapHandler()->changeParamLevel(*menuItem, crossSectionCase ? itsCurrentCrossSectionRowIndex : itsCurrentViewRowIndex);
			break;
		}
		case kFmiRemoveView:
			GetCombinedMapHandler()->removeView(*menuItem, itsCurrentViewRowIndex);
			break;
        case kFmiToggleShowLegendState:
			GetCombinedMapHandler()->toggleShowLegendState(*menuItem, itsCurrentViewRowIndex);
            break;
        case kFmiSwapViewRows:
			GetCombinedMapHandler()->swapViewRows(*menuItem);
            break;
        case kFmiRemoveParamCrossSectionView:
			GetCombinedMapHandler()->removeCrosssectionDrawParam(*menuItem, itsCurrentCrossSectionRowIndex);
			break;
		case kFmiRemoveAllViews:
			GetCombinedMapHandler()->removeAllViews(menuItem->MapViewDescTopIndex(), itsCurrentViewRowIndex);
			break;
        case kFmiRemoveAllParamsFromAllRows:
			GetCombinedMapHandler()->clearDesctopsAllParams(menuItem->MapViewDescTopIndex());
            break;
        case kFmiRemoveAllParamsCrossSectionView:
			GetCombinedMapHandler()->removeAllCrossSectionViews(itsCurrentCrossSectionRowIndex);
			break;
		case kFmiModifyView:
			GetCombinedMapHandler()->modifyView(*menuItem, itsCurrentViewRowIndex);
			break;
        case kFmiFixedDrawParam:
			GetCombinedMapHandler()->applyFixeDrawParam(*menuItem, itsCurrentViewRowIndex);
            break;
        case kFmiStoreDrawParam:
			GetCombinedMapHandler()->saveDrawParamSettings(*menuItem, itsCurrentViewRowIndex);
            break;
        case kFmiReloadDrawParam:
			GetCombinedMapHandler()->reloadDrawParamSettings(*menuItem, itsCurrentViewRowIndex);
            break;
        case kFmiChangeAllProducersInMapRow:
			GetCombinedMapHandler()->changeAllProducersInMapRow(*menuItem, itsCurrentViewRowIndex, false);
			break;
		case kFmiChangeAllProducersInCrossSectionRow:
			GetCombinedMapHandler()->changeAllProducersInMapRow(*menuItem, itsCurrentCrossSectionRowIndex, true);
			break;
		case kFmiChangeAllDataTypesInMapRow:
			GetCombinedMapHandler()->changeAllDataTypesInMapRow(*menuItem, itsCurrentViewRowIndex, false);
			break;
		case kFmiChangeAllDataTypesInCrossSectionRow:
			GetCombinedMapHandler()->changeAllDataTypesInMapRow(*menuItem, itsCurrentCrossSectionRowIndex, true);
			break;
		case kFmiCopyDrawParamOptions:
			GetCombinedMapHandler()->copyDrawParamOptions(*menuItem, itsCurrentViewRowIndex, false);
			break;
		case kFmiCopyMapViewDescTopParams:
			GetCombinedMapHandler()->copyMapViewDescTopParams(menuItem->MapViewDescTopIndex());
			break;
		case kFmiPasteMapViewDescTopParams:
			GetCombinedMapHandler()->pasteMapViewDescTopParams(menuItem->MapViewDescTopIndex());
			break;
		case kFmiCopyDrawParamsFromMapViewRow:
			GetCombinedMapHandler()->copyDrawParamsFromViewRow(*menuItem, itsCurrentViewRowIndex, false);
			break;
		case kFmiPasteDrawParamsToMapViewRow:
			GetCombinedMapHandler()->pasteDrawParamsToViewRow(*menuItem, itsCurrentViewRowIndex, false);
			break;
		case kFmiPasteDrawParamsToCrossSectionViewRow:
			GetCombinedMapHandler()->pasteDrawParamsToViewRow(*menuItem, itsCurrentCrossSectionRowIndex, true);
			break;
		case kFmiCopyDrawParamsFromCrossSectionViewRow:
			GetCombinedMapHandler()->copyDrawParamsFromViewRow(*menuItem, itsCurrentCrossSectionRowIndex, true);
			break;
		case kFmiPasteDrawParamOptions:
			GetCombinedMapHandler()->pasteDrawParamOptions(*menuItem, itsCurrentViewRowIndex, false);
			break;
		case kFmiCopyDrawParamOptionsCrossSectionView:
			GetCombinedMapHandler()->copyDrawParamOptions(*menuItem, itsCurrentCrossSectionRowIndex, true);
			break;
		case kFmiPasteDrawParamOptionsCrossSectionView:
			GetCombinedMapHandler()->pasteDrawParamOptions(*menuItem, itsCurrentCrossSectionRowIndex, true);
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
			GetCombinedMapHandler()->hideView(*menuItem, itsCurrentViewRowIndex);
			break;
		case kFmiShowView:
			GetCombinedMapHandler()->showView(*menuItem, itsCurrentViewRowIndex);
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
			GetCombinedMapHandler()->modifyDrawParam(*menuItem, itsCurrentViewRowIndex);
			break;
		case kFmiModifyCrossSectionDrawParam:
			GetCombinedMapHandler()->modifyCrossSectionDrawParam(*menuItem, itsCurrentCrossSectionRowIndex);
			break;
		case kFmiShowParamCrossSectionView:
		case kFmiHideParamCrossSectionView:
			GetCombinedMapHandler()->showCrossSectionDrawParam(*menuItem, itsCurrentCrossSectionRowIndex, command == kFmiShowParamCrossSectionView ? true : false);
			break;
		case kFmiCrossSectionSetTrajectoryTimes:
			SetCrossSectionTrajectoryTimes(itsCurrentCrossSectionRowIndex);
			break;
		case kFmiCrossSectionSetTrajectoryParams:
			SetCrossSectionTrajectoryParams();
			break;
		case kFmiAddTimeSerialView:
			GetCombinedMapHandler()->addTimeSerialView(*menuItem, false);
			break;
		case kFmiRemoveTimeSerialView:
			GetCombinedMapHandler()->removeTimeSerialView(*menuItem);
			break;
		case kFmiAddTimeSerialSideParam:
			GetCombinedMapHandler()->addTimeSerialViewSideParameter(*menuItem, false);
			break;
		case kFmiRemoveSelectedTimeSerialSideParam:
			GetCombinedMapHandler()->removeTimeSerialViewSideParameter(*menuItem);
			break;
		case kFmiRemoveAllTimeSerialRowSideParams:
			GetCombinedMapHandler()->removeAllTimeSerialViewSideParameters(GetCombinedMapHandler()->getTimeSerialViewIndexReference());
			break;
		case kFmiTimeSerialModelRunCountSet:
			GetCombinedMapHandler()->timeSerialViewModelRunCountSet(*menuItem);
			break;
		case kFmiActivateView:
			GetCombinedMapHandler()->activateView(*menuItem, itsCurrentViewRowIndex);
			break;
		case kFmiActivateCrossSectionDrawParam:
			GetCombinedMapHandler()->activateCrossSectionParam(*menuItem, itsCurrentCrossSectionRowIndex);
			break;
		case kFmiToggleShowDifferenceToOriginalData:
			GetCombinedMapHandler()->toggleShowDifferenceToOriginalData(*menuItem, itsCurrentViewRowIndex);
			break;
		case kFmiRemoveAllTimeSerialViews:
			GetCombinedMapHandler()->removeAllTimeSerialViews();
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
			GetCombinedMapHandler()->timeSerialViewDirty(true);
			break;
        case kFmiObservationStationsToCpPoints:
            MakeObservationStationsToCpPoints(*menuItem);
            break;

		case kFmiHideAllMapViewObservations:
			GetCombinedMapHandler()->hideShowAllMapViewParams(menuItem->MapViewDescTopIndex(), true, false, false, false);
			break;
		case kFmiShowAllMapViewObservations:
			GetCombinedMapHandler()->hideShowAllMapViewParams(menuItem->MapViewDescTopIndex(), false, true, false, false);
			break;
		case kFmiHideAllMapViewForecasts:
			GetCombinedMapHandler()->hideShowAllMapViewParams(menuItem->MapViewDescTopIndex(), false, false, true, false);
			break;
		case kFmiShowAllMapViewForecasts:
			GetCombinedMapHandler()->hideShowAllMapViewParams(menuItem->MapViewDescTopIndex(), false, false, false, true);
			break;
		case kFmiAddBorderLineLayer:
			GetCombinedMapHandler()->addBorderLineLayer(*menuItem, itsCurrentViewRowIndex);
			break;
		case kFmiMoveBorderLineLayer:
			GetCombinedMapHandler()->moveBorderLineLayer(*menuItem, itsCurrentViewRowIndex);
			break;
		case kFmiInsertParamLayer:
			InsertWantedParamLayer(*menuItem, itsCurrentViewRowIndex);
			break;
		case kFmiSelectBackgroundMapLayer:
		case kFmiSelectOverlayMapLayer:
			SelectMapLayerDirectly(*menuItem);
			break;
		case kFmiSetTimeBoxLocation:
			GetCombinedMapHandler()->onSetTimeBoxLocation(menuItem->MapViewDescTopIndex(), static_cast<FmiDirection>(menuItem->ExtraParam()));
			break;
		case kFmiSetTimeBoxTextSizeFactor:
			GetCombinedMapHandler()->onSetTimeBoxTextSizeFactor(menuItem->MapViewDescTopIndex(), static_cast<float>(menuItem->ExtraParam()));
			break;
		case kFmiSetTimeBoxFillColorAlpha:
			GetCombinedMapHandler()->onSetTimeBoxFillColorAlpha(menuItem->MapViewDescTopIndex(), static_cast<float>(menuItem->ExtraParam()));
			break;
		case kFmiSetTimeBoxFillColor:
			SetTimeBoxFillColorFromMenu(*menuItem);
			break;
		case kFmiSetTimeBoxCustomFillColor:
			DoTimeBoxCustomFillColorSetup(*menuItem);
			break;
		case kFmiSetTimeBoxToDefaultValues:
			DoTimeBoxToDefaultValues(*menuItem);
			break;

		default:
			return false;
		}
	}
	return true;
}

void InsertWantedParamLayer(NFmiMenuItem& theMenuItem, int theSelectedViewRowIndex)
{
	GetCombinedMapHandler()->insertParamLayer(theMenuItem, theSelectedViewRowIndex);
}

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

	GetCombinedMapHandler()->makeMacroParamCacheUpdatesForWantedRow(mapViewDescTopIndex, usedRowIndex);
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
					mask = new NFmiInfoAreaMask(calCond, NFmiAreaMask::kInfo, infoCopy->DataType(), infoCopy, theMenuItem.DataIdent().GetParamIdent(), NFmiAreaMask::kNoValue);
				}
				else
				{
					NFmiCalculationCondition calCond(kFmiMaskGreaterThan, 0.,1.);
					mask = new NFmiInfoAreaMask(calCond, NFmiAreaMask::kInfo, infoCopy->DataType(), infoCopy, theMenuItem.DataIdent().GetParamIdent(), NFmiAreaMask::kNoValue);
				}
			}
		}
	}
	return mask;
}

void AddMask(const NFmiMenuItem& theMenuItem, int /* theRowIndex */ , bool fClearListFirst)
{
	GetCombinedMapHandler()->timeSerialViewDirty(true);
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
				logStr += GetCombinedMapHandler()->getSelectedParamInfoString(mask->DataIdent(), mask->Level());
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
	GetCombinedMapHandler()->timeSerialViewDirty(true);
	boost::shared_ptr<NFmiAreaMaskList> paramMaskList = ParamMaskListMT();
	if(paramMaskList)
	{
		paramMaskList->Clear();
		ParamMaskListMT(paramMaskList); // maskilistaan tuli lis‰ys/poisto, t‰llˆin lista pit‰‰ asettaa dokumenttiin k‰yttˆˆn, muutokset yksitt‰isiin maskeihin menev‰t automaattisesti k‰yttˆˆn
	}
}

void RemoveMask(const NFmiMenuItem& theMenuItem)
{
	GetCombinedMapHandler()->timeSerialViewDirty(true);
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
	GetCombinedMapHandler()->timeSerialViewDirty(true);
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
	GetCombinedMapHandler()->timeSerialViewDirty(true);
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
	GetCombinedMapHandler()->timeSerialViewDirty(true);
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

void SetCrossSectionTrajectoryParams()
{
	// ei aseteta vain halutun rivin parametreja, vaan kaikkien rivien kaikki parametrit
	// vastaamaan kulloisenkin rivin osoittamaa trajektoria

	int ssize = GetCombinedMapHandler()->getCrossSectionDrawParamListVector().NumberOfItems();
	for(int i = 1; i <= ssize; i++)
	{
		NFmiDrawParamList *aList = GetCombinedMapHandler()->getCrossSectionViewDrawParamList(i); // HUOM! t‰‰ll‰ indeksointi alkaa 1:st‰
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
		const std::vector<NFmiMetTime> &times = trajectory.CrossSectionTrajectoryTimes();
		if(times.size())
		{
			NFmiTimeBag timeBag(times[0], times[times.size()-1], CrossSectionSystem()->CrossSectionTimeControlTimeBag().Resolution());
			CrossSectionSystem()->CrossSectionTimeControlTimeBag(timeBag);
		}
	}
	else if(CrossSectionSystem()->GetCrossMode() == NFmiCrossSectionSystem::kTime)
	{ // muutin systeemin asettamaan ajat myˆs aika-moodissa. Ottaa ensimm‰isen drawParamia
	  // vastaavan infon ja s‰‰t‰‰ alku ja loppu ajat sen mukaan
		NFmiDrawParamList *aList = GetCombinedMapHandler()->getCrossSectionViewDrawParamList(theRowIndex);
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

// laskee n‰yttˆruudukon yhden ruudun koon pikseleiss‰
NFmiPoint ActualMapBitmapSizeInPixels(unsigned int theDescTopIndex)
{
	return GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->ActualMapBitmapSizeInPixels();
}
NFmiEditMapDataListHandler* DataLists(void)
{
	return itsListHandler;
}

bool StoreMatrixToGridFileFinal(const NFmiDataMatrix<float>& dataMatrix, const std::string& fullFilePath)
{
	std::ofstream out(fullFilePath, std::ios::binary);
	if(out)
	{
		out << dataMatrix;
		out.close();
		return true;
	}
	return false;
}

bool StoreMatrixToGridfile(const NFmiDataMatrix<float> &dataMatrix, const NFmiString& theFileName)
{
	// Yritet‰‰n ensin D-aseman juureen talletusta
	std::string fileName1 = "D:\\" + theFileName;
	if(!StoreMatrixToGridFileFinal(dataMatrix, fileName1))
	{
		// Yritet‰‰n sitten C-aseman juureen talletusta
		std::string fileName1 = "C:\\" + theFileName;
		return StoreMatrixToGridFileFinal(dataMatrix, fileName1);
	}
	return true;
}

bool MakeGridFileForMacroParam(unsigned long usedMapViewIndex, boost::shared_ptr<NFmiDrawParam> &drawParam, const NFmiString& theFileName)
{
	auto absoluteActiveViewRow = GetCombinedMapHandler()->absoluteActiveViewRow(usedMapViewIndex);
	auto drawParamList = GetCombinedMapHandler()->getDrawParamListWithRealRowNumber(usedMapViewIndex, absoluteActiveViewRow);
	if(drawParamList)
	{
		auto activeParamLayerIndex = drawParamList->FindActive();
		NFmiMacroParamLayerCacheDataType macroParamLayerCacheDataType;
		if(MacroParamDataCache().getCache(usedMapViewIndex, absoluteActiveViewRow, activeParamLayerIndex, itsActiveViewTime, drawParam->InitFileName(), macroParamLayerCacheDataType))
		{
			const auto& dataMatrix = macroParamLayerCacheDataType.getDataMatrix();
			return StoreMatrixToGridfile(dataMatrix, theFileName);
		}
	}
	return false;
}

// tallettaa aktiivisen n‰yttˆrivin aktiivisen parametrin currentin ajan gridin tiedostoon,
// jonka nimi annetaan parametrina, mutta se talletetaan tyˆhakemistoon
bool MakeGridFile(const NFmiString& theFileName)
{
	// tehd‰‰n vain p‰‰karttaikkunasta n‰it‰ talletuksia
	auto usedMapViewIndex = 0ul;
	boost::shared_ptr<NFmiDrawParam> drawParam = GetCombinedMapHandler()->activeDrawParamFromActiveRow(usedMapViewIndex);
	bool status = false;
	if(drawParam)
	{
		if(drawParam->IsMacroParamCase(true))
			return MakeGridFileForMacroParam(usedMapViewIndex, drawParam, theFileName);
		else
		{
			boost::shared_ptr<NFmiFastQueryInfo> info = InfoOrganizer()->Info(drawParam, false, false);
			if(info == 0)
				return false;
			NFmiMetTime oldTime = info->Time();
			if(info->DataType() == NFmiInfoData::kStationary || info->TimeDescriptor().IsInside(itsActiveViewTime))
			{
				NFmiDataMatrix<float> dataMatrix;
				if(info->DataType() == NFmiInfoData::kStationary)
					info->Values(dataMatrix);
				else
					info->Values(dataMatrix, itsActiveViewTime);

				status = StoreMatrixToGridfile(dataMatrix, theFileName);
			}
			info->Time(oldTime);
		}
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

bool DoAreaFiltering()
{
	return FmiModifyEditdData::DoAreaFiltering(GenDocDataAdapter(), UseMultithreaddingWithModifyingFunctions());
}

bool DoTimeFiltering(void)
{
	return FmiModifyEditdData::DoTimeFiltering(GenDocDataAdapter(), UseMultithreaddingWithModifyingFunctions());
}

bool HasActiveViewChanged(void)
{
	if(itsLastBrushedViewRealRowIndex != GetCombinedMapHandler()->absoluteActiveViewRow(ParameterSelectionSystem().LastActivatedDesktopIndex()))
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

bool LoadStaticHelpData(void)
{
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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

			bool dataWasDeleted = false;
            AddQueryData(data.release(), latestFileName, fileFilter, theHelpDataInfo.DataType(), "", false, dataWasDeleted);
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

bool IsMacroParamAndDrawWithSymbols(boost::shared_ptr<NFmiDrawParam> &drawParam)
{
    if(drawParam && drawParam->IsMacroParamCase(true))
    {
        // MacroParam on aina hila muotoista, joten tarkastetaan hilamuotoinen visualisointi tyyppi
        auto visualizationType = drawParam->GridDataPresentationStyle();
        if(!(visualizationType >= NFmiMetEditorTypes::View::kFmiIsoLineView && visualizationType <= NFmiMetEditorTypes::View::kFmiQuickColorContourView))
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
    auto mapViewDesctop = GetCombinedMapHandler()->getMapViewDescTop(mapViewDescTopIndex);
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

void UpdateCrossSectionViewTrueSizeViewAfterViewMacro()
{
	auto drawObjectScaleFactor = ApplicationWinRegistry().DrawObjectScaleFactor();
	auto& crossSectionSystem = *CrossSectionSystem();
	auto& trueMapViewSizeInfo = crossSectionSystem.GetTrueMapViewSizeInfo();
	auto& clientPixelSize = trueMapViewSizeInfo.clientAreaSizeInPixels();
	NFmiPoint crossSectionViewGridSize(1, crossSectionSystem.RowCount());
	trueMapViewSizeInfo.onSize(clientPixelSize, nullptr, crossSectionViewGridSize, true, drawObjectScaleFactor);
}

bool DoMapViewOnSize(int mapViewDescTopIndex, const NFmiPoint& clientPixelSize, CDC* pDC)
{
	auto isMapView = mapViewDescTopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex;
	auto drawObjectScaleFactor = ApplicationWinRegistry().DrawObjectScaleFactor();
	auto mapViewDesctop = GetCombinedMapHandler()->getMapViewDescTop(mapViewDescTopIndex, true);
	if(mapViewDesctop)
	{
		// MapViewSizeInPixels(clientPixelSize) -kutsu pit‰‰ olla ennen CFmiWin32Helpers::SetDescTopGraphicalInfo funktio kutsua.
		mapViewDesctop->MapViewSizeInPixels(clientPixelSize, pDC, drawObjectScaleFactor, !mapViewDesctop->IsTimeControlViewVisible());
		CFmiWin32Helpers::SetDescTopGraphicalInfo(isMapView, mapViewDesctop->GetGraphicalInfo(), pDC, clientPixelSize, drawObjectScaleFactor, true, &mapViewDesctop->GetTrueMapViewSizeInfo().singleMapSizeInMM());

		// Nyky‰‰n jos kartan koko muuttuu, pit‰‰ macroParam cache tyhjent‰‰, koska sen koko saattaa muuttua.
		// Laskentahilan koko lasketaan aina uudestaan, jolloin tehd‰‰n hila- vs pikseli-koko vertailuja.
		auto cleanMacroParamCache = true;
		GetCombinedMapHandler()->mapViewDirty(mapViewDescTopIndex, true, true, true, cleanMacroParamCache, false, false);
		MapViewSizeChangedDoSymbolMacroParamCacheChecks(mapViewDescTopIndex);
		mapViewDesctop->SetBorderDrawDirtyState(CountryBorderDrawDirtyState::Geometry);
		return true; // Jos kyse oli karttan‰ytˆn asetuksesta, palautetaan true
	}
	else if(mapViewDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
	{
		auto &crossSectionSystem = *CrossSectionSystem();
		auto& trueMapViewSizeInfo = crossSectionSystem.GetTrueMapViewSizeInfo();
		NFmiPoint crossSectionViewGridSize(1, crossSectionSystem.RowCount());
		trueMapViewSizeInfo.onSize(clientPixelSize, pDC, crossSectionViewGridSize,  true, drawObjectScaleFactor);
		CFmiWin32Helpers::SetDescTopGraphicalInfo(isMapView, crossSectionSystem.GetGraphicalInfo(), pDC, clientPixelSize, drawObjectScaleFactor, true);
		return true; // Jos kyse oli poikkileikkausn‰ytˆn asetuksesta, palautetaan true
	}
	return false; // Jos kyse oli jostain muusta kuin karttan‰ytˆst‰, palautetaan false sen merkiksi ett‰ mit‰‰n ei tehty
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
    return status;
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
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Sending edited data to database", SmartMetViewId::MainMapView, true, false, 0);
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
void PutProducerIdListInDataHeader(boost::shared_ptr<NFmiFastQueryInfo> &theSmartInfo, std::vector<NFmiFastQueryInfo*> &theSourceInfos, std::vector<int> &theModelIndexVector)
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
	fOpenPopup = false;
	if(itsMetEditorOptionsData.ControlPointMode())
	{
		NFmiInfoData::Type infoDataType = NFmiInfoData::kEditable;
		itsPopupMenu = std::make_unique<NFmiMenuItemList>();

		std::string menuString = ::GetDictionaryString("ControlPointPopUpActivate");
        auto menuItem = std::make_unique<NFmiMenuItem>(-1, menuString, NFmiDataIdent(), kFmiActivateCP, g_DefaultParamView, nullptr, infoDataType);
		itsPopupMenu->Add(std::move(menuItem));
		menuString = ::GetDictionaryString("ControlPointPopUpDeactivate");
		menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiDeactivateCP, g_DefaultParamView, nullptr, infoDataType));
		itsPopupMenu->Add(std::move(menuItem));
		menuString = ::GetDictionaryString("ControlPointPopUpUse");
		menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiEnableCP, g_DefaultParamView, nullptr, infoDataType));
		itsPopupMenu->Add(std::move(menuItem));
		menuString = ::GetDictionaryString("ControlPointPopUpDontUse");
		menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiDisableCP, g_DefaultParamView, nullptr, infoDataType));
		itsPopupMenu->Add(std::move(menuItem));

		menuString = ::GetDictionaryString("ControlPointPopUpAllwaysInTimeSerialView");
		menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiShowCPAllwaysOnTimeView, g_DefaultParamView, nullptr, infoDataType));
		itsPopupMenu->Add(std::move(menuItem));
		menuString = ::GetDictionaryString("ControlPointPopUpNotAllwaysInTimeSerialView");
		menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiDontShowCPAllwaysOnTimeView, g_DefaultParamView, nullptr, infoDataType));
		itsPopupMenu->Add(std::move(menuItem));
		menuString = ::GetDictionaryString("ControlPointPopUpAllPointsInTimeView");
		menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiShowAllCPsAllwaysOnTimeView, g_DefaultParamView, nullptr, infoDataType));
		itsPopupMenu->Add(std::move(menuItem));
		menuString = ::GetDictionaryString("ControlPointPopUpOnlyActiveInTimeView");
		menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiShowOnlyActiveCPOnTimeView, g_DefaultParamView, nullptr, infoDataType));
		itsPopupMenu->Add(std::move(menuItem));

		menuString = ::GetDictionaryString("ControlPointPopUpModify");
		menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiModifyCPAttributes, g_DefaultParamView, nullptr, infoDataType));
		itsPopupMenu->Add(std::move(menuItem));

        AddCpManagerSetsToCpPopupMenu(itsPopupMenu.get(), infoDataType);
        AddObservationStationsToCpPointsCommands(itsPopupMenu.get());

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

void AddObservationStationsToCpPointsCommands(NFmiMenuItemList *mainPopupMenu, const std::vector<NFmiProducer> &producerList, NFmiInfoData::Type usedInfoData)
{
    if(!producerList.empty())
    {
        NFmiParam dummyParam;
        NFmiMenuItemList *producerMenuList = new NFmiMenuItemList;
        for(const auto &producer : producerList)
        {
            auto menuItem = std::make_unique<NFmiMenuItem>(-1, std::string(producer.GetName()), NFmiDataIdent(dummyParam, producer), kFmiObservationStationsToCpPoints, g_DefaultParamView, nullptr, usedInfoData);
            producerMenuList->Add(std::move(menuItem));
        }

        if(producerMenuList->NumberOfMenuItems())
        {
            std::string menuString = ::GetDictionaryString("Observations To CP points");
            auto menuItem = std::make_unique<NFmiMenuItem>(-1, menuString, NFmiDataIdent(), kFmiObservationStationsToCpPoints, g_DefaultParamView, nullptr, usedInfoData);
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
        controlPointManager->SetZoomedAreaStationsAsControlPoints(observationInfos, GetCombinedMapHandler()->getMapViewDescTop(0)->MapHandler()->Area());
        AnalyzeToolData().ControlPointObservationBlendingData().SelectProducer(producerId);

        // Vain aikasarja pit‰‰ laittaa t‰ss‰ likaiseksi, sielt‰ mist‰ t‰t‰ kutsutaan (MakePopUpCommandUsingRowIndex) laitetaan karttan‰ytˆt likaisiksi.
		GetCombinedMapHandler()->timeSerialViewDirty(true);
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
                auto cpMenuItem = std::make_unique<NFmiMenuItem>(-1, cpManager->Name(), NFmiDataIdent(), kFmiSelectCPManagerFromSet, g_DefaultParamView, nullptr, infoDataType, static_cast<int>(i));
                cpManagerMenuList->Add(std::move(cpMenuItem));
            }
        }

        if(cpManagerMenuList->NumberOfMenuItems())
        {
            std::string menuString = ::GetDictionaryString("CPManagers");
            auto menuItem = std::make_unique<NFmiMenuItem>(-1, menuString, NFmiDataIdent(), kFmiSelectCPManagerFromSet, g_DefaultParamView, nullptr, infoDataType);
            menuItem->AddSubMenu(cpManagerMenuList);
            mainPopupMenu->Add(std::move(menuItem));
        }
        else
            delete cpManagerMenuList;
    }
}

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
		ApplicationWinRegistry().SetSoundingViewSettings(itsMTATempSystem.GetSoundingViewSettingsFromWindowsRegisty());

        itsTrajectorySystem->StoreSettings();
		GetCombinedMapHandler()->storeMapViewDescTopToSettings();
		StoreAnalyzeToolData();
		StoreDataQualityChecker();
		IgnoreStationsData().StoreToSettings();
		StoreOptionsData(); // t‰m‰ tekee myˆs asetuksien talletuksen konfiguraatio tiedostoihin! T‰m‰ pit‰‰ siis kutsua viimeisen‰.

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

void SetDataLoadingProducerIndexVector(const std::vector<int>& theIndexVector)
{
	itsDataLoadingProducerIndexVector = theIndexVector;
}

void RemoveAllParamsFromMapViewRow(unsigned int theDescTopIndex, int theRowIndex)
{
	GetCombinedMapHandler()->removeAllViewsWithRealRowNumber(theDescTopIndex, theRowIndex);
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
	CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
		CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
		bool status = itsSmartToolInfo.Init(itsMacroPathSettings.SmartToolPath());
		NFmiFileSystem::CreateDirectory(itsSmartToolInfo.LoadDirectory());
		if(!status)
		{
			string errStr("SmartTool-ohjausta ei saatu luettua asetuksista.");
			LogMessage(errStr, CatLog::Severity::Error, CatLog::Category::Configuration);
		}
		string dbCheckerSettingsKey = "SmartMet::OptionalDBCheckerPath";
		string fileName2 = NFmiSettings::Optional<string>(dbCheckerSettingsKey, "");
		if(!fileName2.empty())
		{
			string augmentedFileName2 = PathUtils::makeFixedAbsolutePath(fileName2, itsBasicConfigurations.ControlPath());
			if(!NFmiFileSystem::FileExists(augmentedFileName2))
			{
				string errorMessage = dbCheckerSettingsKey;
				errorMessage += " was given with value: '";
				errorMessage += fileName2;
				errorMessage += "' but that path has no existing file to be read";
				LogAndWarnUser(errorMessage, "", CatLog::Severity::Error, CatLog::Category::Configuration, true, false, true);
			}
			else
				fileName2 = augmentedFileName2;
		}
		else
		{
			fileName2 = itsBasicConfigurations.ControlPath();
			fileName2 += "DBChecker.st";
		}

		itsSmartToolInfo.DBCheckerFileName(fileName2);
		bool status2 = itsSmartToolInfo.LoadDBChecker();
		SetCurrentSmartToolMacro(itsSmartToolInfo.CurrentScript()); // laitetaan currentti skripti myˆs dociin

        // Alustetaan myˆs yksi smartTool kieleen liittyv‰t callback funktiot
        NFmiInfoAreaMask::SetMultiSourceDataGetterCallback(
			[this](std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector, 
				const NFmiDataIdent& dataIdent,
				const NFmiLevel &level,
				NFmiInfoData::Type dataType,
				const boost::shared_ptr<NFmiArea> &theArea)
			{
				boost::shared_ptr<NFmiDrawParam> drawParam(new NFmiDrawParam(dataIdent, level, 0, dataType));
				GetCombinedMapHandler()->makeDrawedInfoVectorForMapView(theInfoVector, drawParam, theArea); 
			});

		return status && status2;
	}

	NFmiSmartToolInfo* SmartToolInfo(void){return &itsSmartToolInfo;}

	std::string GetViewMacroFileName(const std::string &theName)
	{
        NFmiFileString fileString(theName);
        if(fileString.IsAbsolutePath())
            return theName;

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
		gDoc.ProjectionCurvatureInfo(*GetCombinedMapHandler()->projectionCurvatureInfo());
		gDoc.CPLocationVector(CPManager()->CPLocationVector());
	}

	void FillTimeViewMacro(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::TimeView &timeView = theMacro.GetTimeView();
		timeView.SetAllParams(&GetCombinedMapHandler()->getTimeSerialViewDrawParamList());
		timeView.SetAllSideParameters(GetCombinedMapHandler()->getTimeSerialViewSideParameterList());
        timeView.ShowHelpData(ShowHelperDataInTimeSerialView() != 0);
        timeView.ShowHelpData2(ShowHelperData2InTimeSerialView());
        timeView.ShowHelpData3(ShowHelperData3InTimeSerialView());
        timeView.ShowHelpData4(ShowHelperData4InTimeSerialView());

		timeView.TimeBag(TimeSerialViewTimeBag());
		timeView.PreciseTimeSerialLatlonPoint(PreciseTimeSerialLatlonPoint());
	}

	void FillTempViewMacro(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::TempView &tempView = theMacro.GetTempView();

		tempView.ShowEcmwf(false);
		tempView.ShowHirlam(false);

		tempView.MTATempSystem(itsMTATempSystem, false);
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
		view.SetAllRowParams(&GetCombinedMapHandler()->getCrossSectionDrawParamListVector(), itsMacroParamSystem);
	}

	void FillExtraMapViewSettingMacro2(NFmiViewSettingMacro::MapViewDescTop &theViewMacro, NFmiMapViewDescTop &theDescTop, int theDescTopIndex)
	{
        boost::shared_ptr<NFmiMapViewWinRegistry> mapViewWinRegistry = ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(theDescTopIndex);
        theViewMacro.SetMapViewDescTop(theDescTop, *mapViewWinRegistry.get(), false);
		theViewMacro.SetAllRowParams(theDescTop.DrawParamListVector(), itsMacroParamSystem);
		theViewMacro.DipMapHelperList(theDescTop.GetViewMacroDipMapHelperList());
	}

	void FillExtraMapViewSettingsMacro(NFmiViewSettingMacro &theMacro)
	{
		std::vector<NFmiViewSettingMacro::MapViewDescTop> &extraMapViewDescTops = theMacro.ExtraMapViewDescTops();
		auto& mapViewDescTops = GetCombinedMapHandler()->getMapViewDescTops();
		extraMapViewDescTops.resize(mapViewDescTops.size()); // varmuuden vuoksi koon asetus

		for(size_t i = 0; i < mapViewDescTops.size(); i++)
			FillExtraMapViewSettingMacro2(extraMapViewDescTops[i], *mapViewDescTops[i], static_cast<int>(i));
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
        int timeStepInMapView = static_cast<int>(::round(GetCombinedMapHandler()->getMapViewDescTop(0)->TimeControlTimeStep() * 60));
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
        theMacro.UseControlPoinTool(MetEditorOptionsData().ControlPointMode());
    }

	void SetGeneralDoc(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::GeneralDoc &gDoc = theMacro.GetGeneralDoc();
		GetCombinedMapHandler()->projectionCurvatureInfo(gDoc.ProjectionCurvatureInfo());
        if(itsCPManagerSet.UseOldSchoolStyle())
    		CPManager(true)->Init(gDoc.CPLocationVector(), true);
        else
    		itsCPManagerSet.UpdateViewMacroCPManager(gDoc.CPLocationVector());
	}

	void AdjustDrawParam(boost::shared_ptr<NFmiDrawParam> &theDrawParam, const NFmiViewSettingMacro::Param &theParam, bool fTreatAsViewMacro)
	{
		if(theDrawParam)
		{
				theDrawParam->Init(theParam.DrawParam(), false);
				theDrawParam->DataType(theParam.DataType());
				theDrawParam->HideParam(theParam.Hidden());
				theDrawParam->Activate(theParam.Active());
				theDrawParam->ShowDifferenceToOriginalData(theParam.ShowDifferenceToOriginalData());
				theDrawParam->MacroParamRelativePath(theParam.DrawParam()->MacroParamRelativePath());
                theDrawParam->UseViewMacrosSettingsForMacroParam(fTreatAsViewMacro);

		}
	}

	void AdjustDrawParam(unsigned int theDescTopIndex, const NFmiViewSettingMacro::Param &theParam, int theRowIndex, int paramIndexInRow, bool fTreatAsViewMacro)
	{
		NFmiDrawParamList* dList = GetCombinedMapHandler()->getDrawParamListWithRealRowNumber(theDescTopIndex, theRowIndex);
		if(dList)
		{
			boost::shared_ptr<NFmiDrawParam> dPar;
			if(theRowIndex == CtrlViewUtils::kFmiTimeSerialView)
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
			AdjustDrawParam(dPar, theParam, fTreatAsViewMacro);
		}
	}

	void AdjustCrossSectionDrawParam(const NFmiViewSettingMacro::Param &theParam, int theRowIndex, int paramIndexInRow, bool fTreatAsViewMacro)
	{
		NFmiDrawParamList* dList = GetCombinedMapHandler()->getCrossSectionViewDrawParamList(theRowIndex);
		if(dList)
		{
			boost::shared_ptr<NFmiDrawParam> dPar;
			if(dList->Index(paramIndexInRow))
				dPar = dList->Current();
			AdjustDrawParam(dPar, theParam, fTreatAsViewMacro);
		}
	}

    void SetTimeViewParams(NFmiViewSettingMacro &theMacro, bool fTreatAsViewMacro)
	{
		auto& combinedMapHandler = *GetCombinedMapHandler();
		combinedMapHandler.removeAllTimeSerialViews();
		auto& timeViewSettings = theMacro.GetTimeView();

		const std::vector<NFmiViewSettingMacro::TimeViewRow>& rows = timeViewSettings.Rows();
		std::vector<NFmiViewSettingMacro::TimeViewRow>::size_type ssize = rows.size();
		std::vector<NFmiViewSettingMacro::TimeViewRow>::size_type counter = 0;
		auto& timeSerialViewIndexReference = combinedMapHandler.getTimeSerialViewIndexReference();
		timeSerialViewIndexReference = 0;
		for( ;counter < ssize ; counter++)
		{
			auto& macroTimeRow = rows[counter];
			timeSerialViewIndexReference++; // t‰t‰ juoksuttamalla saan parametrit menem‰‰n aikasarja ikkunaan oikeaan j‰rjestykseen
			const NFmiViewSettingMacro::Param &par = macroTimeRow.Param();
			NFmiMenuItem menuItem(-1, "x", par.DataIdent(), kFmiAddTimeSerialView, g_DefaultParamView, &par.Level(), par.DataType());
			auto addedDrawParam = combinedMapHandler.addTimeSerialView(menuItem, fTreatAsViewMacro);
            // Kaikki makroon talletetut drawparam asetukset pit‰‰ viel‰ ladata luotuun ja listoihin laitettuun drawparamiin
			AdjustDrawParam(addedDrawParam, par, fTreatAsViewMacro);

			// Tehd‰‰n side-parameter osio t‰ss‰
			const auto& sideParameters = macroTimeRow.SideParameters();
			if(sideParameters.size())
			{
				for(const auto& sideParameter : sideParameters)
				{
					NFmiMenuItem sideParameterMenuItem(-1, "x", sideParameter.DataIdent(), kFmiAddTimeSerialSideParam, g_DefaultParamView, &sideParameter.Level(), sideParameter.DataType());
					auto addedSideDrawParam = combinedMapHandler.addTimeSerialViewSideParameter(sideParameterMenuItem, fTreatAsViewMacro);
					AdjustDrawParam(addedSideDrawParam, sideParameter, fTreatAsViewMacro);
				}
			}
		}

        if(timeViewSettings.ShowHelpData())
            SetOnShowHelperData1InTimeSerialView();
        else
            SetOffShowHelperData1InTimeSerialView();
        if(timeViewSettings.ShowHelpData2())
            SetOnShowHelperData2InTimeSerialView();
        else
            SetOffShowHelperData2InTimeSerialView();
        ShowHelperData3InTimeSerialView(timeViewSettings.ShowHelpData3());
        ShowHelperData4InTimeSerialView(timeViewSettings.ShowHelpData4());

		if(timeViewSettings.TimeBagUpdated())
			TimeSerialViewTimeBag(timeViewSettings.TimeBag());
		if(timeViewSettings.PreciseTimeSerialLatlonPoint() != NFmiPoint::gMissingLatlon)
		{
			PreciseTimeSerialLatlonPoint(timeViewSettings.PreciseTimeSerialLatlonPoint());
		}
	}

	void SetTempView(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::TempView &tempView = theMacro.GetTempView();

		itsMTATempSystem.InitFromViewMacro(tempView.MTATempSystem(), MetEditorOptionsData().DisableWindowManipulations());
		itsMTATempSystem.UpdateFromViewMacro(true);
	}

	void SetTrajectoryView(NFmiViewSettingMacro &theMacro)
	{
		NFmiViewSettingMacro::TrajectoryView &view = theMacro.GetTrajectoryView();

		itsTrajectorySystem->Init(view.TrajectorySystem(), MetEditorOptionsData().DisableWindowManipulations());
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

		itsCrossSectionSystem.Init(view.CrossSectionSystem(), MetEditorOptionsData().DisableWindowManipulations());

        // Pit‰‰ tyhjent‰‰ aluksi poikkileikkausn‰ytˆn kaikkien rivien parametrit, koska sen t‰yttˆ looppia (joka tyhjent‰‰ kulloisenkin rivin ensin) ei v‰ltt‰m‰tt‰ edes ajeta
		GetCombinedMapHandler()->clearDesctopsAllParams(CtrlViewUtils::kFmiCrossSectionView);

		// asetetaan poikkileikkaus parametrit riveilleen
		const std::vector<NFmiViewSettingMacro::MapRow>& mapRows = theMacro.GetCrossSectionView().MapRowSettings();
		size_t ssize = mapRows.size();
		for(size_t i = 0; i < ssize ; i++)
		{
			NFmiDrawParamList *dList = GetCombinedMapHandler()->getCrossSectionViewDrawParamList(static_cast<int>(i+1));
			if(dList)
			{
				dList->Clear();
				const std::vector<NFmiViewSettingMacro::Param>& params = mapRows[i].RowParams();
				size_t psize = params.size();
				for(size_t j=0 ;j < psize ; j++)
				{
					const NFmiViewSettingMacro::Param &par = params[j];
					NFmiMenuItem menuItem(-1, gDummyParamName, par.DataIdent(), kFmiAddParamCrossSectionView, g_DefaultParamView, &par.Level(), par.DataType());
					GetCombinedMapHandler()->addCrossSectionView(menuItem, static_cast<int>(i + 1), fTreatAsViewMacro);
                    // Kaikki makroon talletetut drawparam asetukset pit‰‰ viel‰ ladata luotuun ja listoihin laitettuun drawparamiin
					AdjustCrossSectionDrawParam(par, static_cast<int>(i+1), static_cast<int>(j+1), fTreatAsViewMacro); 
				}
			}
			else
				throw runtime_error("Error in application. NFmiEditMapGeneralDataDoc::SetCrossSectionView had null pointer drawParamList.");
		}
	}

    // fTreatAsViewMacro -parametria tarvitaan, koska ns. backupViewMacro:jen latauksien yhteydess‰ halutaan ladata parametreja n‰ytˆille
    // niin ett‰ niit‰ ei pidet‰ viewMacro -paramereina. Jos parametri on ladattu normaalista viewMacrosta, ja halutaan tehd‰ sen piirto-ominaisuuksiin
    // muutoksia, se ei onnistu Piirto-ominaisuus dialogista (dialogissa varoitus ja nappuloiden estot), vaan talletus pit‰‰ tehd‰ itse viewMacroon.
	void SetMapViewRowsParams(const std::vector<NFmiViewSettingMacro::MapRow>& theMapRows, unsigned int theDescTopIndex, bool fTreatAsViewMacro)
	{
        // Pit‰‰ tyhjent‰‰ aluksi karttan‰ytˆn kaikkien rivien parametrit, koska sen t‰yttˆ looppia (joka tyhjent‰‰ kulloisenkin rivin ensin) ei v‰ltt‰m‰tt‰ edes ajeta
		GetCombinedMapHandler()->clearDesctopsAllParams(theDescTopIndex);

		auto mapRowsSize = theMapRows.size();
		for(size_t rowVectorCounter = 0; rowVectorCounter < mapRowsSize; rowVectorCounter++)
		{
			const auto& viewMacroRowParams = theMapRows[rowVectorCounter].RowParams();
			for(size_t viewMacroLayerIndex = 0 ; viewMacroLayerIndex < viewMacroRowParams.size(); viewMacroLayerIndex++)
			{
				const auto &viewMacroLayerParam = viewMacroRowParams[viewMacroLayerIndex];
				auto usedDataType = viewMacroLayerParam.DataType();
                if(usedDataType >= NFmiInfoData::kSoundingParameterData)
                {
                    // vanhoissa makroissa on k‰ytetty t‰t‰ NFmiInfoData::kSoundingParameterData virityst‰. 
                    // SmartMet versiosta 5.6 l‰htien sounding-index datat lasketaan valmiiksi datoiksi ja sen tyyppi 
                    // on NFmiInfoData::kModelHelpData. T‰m‰ siis vain pit‰‰ yll‰ taaksep‰in yhteen sopivuutta n‰yttˆmakroissa.
                    usedDataType = NFmiInfoData::kModelHelpData; 
                }
				NFmiMenuItem addParamMenuItem(theDescTopIndex, gDummyParamName, viewMacroLayerParam.DataIdent(), kFmiAddView, g_DefaultParamView, &viewMacroLayerParam.Level(), usedDataType);
				addParamMenuItem.MacroParamInitName(viewMacroLayerParam.DrawParam()->InitFileName());
                auto realRowNumber = rowVectorCounter + 1;
				GetCombinedMapHandler()->addViewWithRealRowNumber(false, addParamMenuItem, static_cast<int>(realRowNumber), fTreatAsViewMacro);
                // Kaikki makroon talletetut drawparam asetukset pit‰‰ viel‰ ladata luotuun ja listoihin laitettuun drawparamiin
				AdjustDrawParam(theDescTopIndex, viewMacroLayerParam, static_cast<int>(realRowNumber), static_cast<int>(viewMacroLayerIndex + 1), fTreatAsViewMacro); 
			}
		}
	}

    void SetSingleExtraMapViewSettingMacro(NFmiViewSettingMacro::MapViewDescTop &theViewMacro, unsigned int theDescTopIndex, bool fTreatAsViewMacro)
	{
		NFmiMapViewDescTop &descTop = *GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex);
        boost::shared_ptr<NFmiMapViewWinRegistry> mapViewWinRegistry = ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(theDescTopIndex);

		descTop.InitForViewMacro(theViewMacro.GetMapViewDescTop(), *mapViewWinRegistry.get(), false, MetEditorOptionsData().DisableWindowManipulations());
		// Mit‰ tehd‰‰n extra rows:ien kanssa
//		theViewMacro.SetAllExtraRowParams(descTop.ExtraDrawParamListVector(), itsMacroParamSystem);
		descTop.SetViewMacroDipMapHelperList(theViewMacro.DipMapHelperList());

		// asetetaan parametrit 'normaali' riveilleen
        SetMapViewRowsParams(theViewMacro.MapRowSettings(), theDescTopIndex, fTreatAsViewMacro);
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
		const std::vector<NFmiViewSettingMacro::Mask>& masks = theMacro.GetMaskSettings().Masks();
		const std::vector<NFmiViewSettingMacro::Mask>::size_type ssize = masks.size();
		std::vector<NFmiViewSettingMacro::Mask>::size_type counter = 0;
		for( ;counter < ssize ; counter++)
		{
			const NFmiViewSettingMacro::Mask &mask = masks[counter];
			NFmiMenuItem menuItem(-1, "x", mask.ParamSettings().DataIdent(), kFmiAddMask, g_DefaultParamView, &mask.ParamSettings().Level(), mask.ParamSettings().DataType());
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

    void ApplyViewMacro(NFmiViewSettingMacro &theMacro, bool fTreatAsViewMacro, bool undoRedoAction)
	{ 
		// ota k‰yttˆˆn kaikki makron asetukset ja tee n‰ytˆist‰ 'likaisia'
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
        MetEditorOptionsData().ControlPointMode(theMacro.UseControlPoinTool());

        GetCombinedMapHandler()->makeApplyViewMacroDirtyActions(ApplicationWinRegistry().DrawObjectScaleFactor());
		UpdateCrossSectionViewTrueSizeViewAfterViewMacro();

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
		theViewMacro.InitFileName(theFileName);

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
		std::string initFileName = itsViewMacroPath + theName + ".vmr";
		std::string loggedLongMacroName = std::string("'") + theName + "' (" + initFileName + ")";
		try
		{
			LogMessage(std::string("Starting to load viewMacro: ") + loggedLongMacroName, CatLog::Severity::Debug, CatLog::Category::Macro, true);

			SnapshotViewMacro(true); // true = tyhjennet‰‰n redo-lista

			// HUOM! t‰ss‰ vaiheessa pit‰‰ n‰yttˆmakro lukea uudestaan, ett‰ ajat p‰ivittyv‰t nykyhetkeen.
			// Muuten ajat voivat olla lukittuna suunnilleen siihen aikaa kun editori on k‰ynnistetty
            if(ReadViewMacro(*CurrentViewMacro(), initFileName))
            {
                itsHelperViewMacro = *CurrentViewMacro();

                LoadViewMacroInfo(*CurrentViewMacro(), true, false);

                ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Loading view-macro in use"); // t‰m‰ sitten p‰ivitt‰‰ kaikki ruudut

                // lokitetaan, mik‰ makro on ladattu k‰yttˆˆn...
                string logStr("Finished loading the view-macro: ");
                logStr += loggedLongMacroName;
                LogMessage(logStr, CatLog::Severity::Info, CatLog::Category::Macro);
                return true;
            }
		}
		catch(exception &e)
		{
			string errStr("Unable to load view-macro: ");
			errStr += loggedLongMacroName;
			errStr += ", Reason: ";
			errStr += e.what();
			LogAndWarnUser(errStr, "ViewMacro loading problem", CatLog::Severity::Error, CatLog::Category::Macro, false);
		}
		catch(...)
		{
			string errStr("Unable to load view-macro (reason unknown): ");
			errStr += loggedLongMacroName;
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
	itsUndoRedoViewMacroList.emplace_back(std::move(currentSnapShot));

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

	void StoreViewMacro(const std::string & theAbsoluteMacroFilePath, const std::string &theDescription)
	{
        NFmiFileString fileString(theAbsoluteMacroFilePath);
		FillViewMacroInfo(itsHelperViewMacro, std::string(fileString.Header()), theDescription);

		WriteViewMacro(itsHelperViewMacro, theAbsoluteMacroFilePath);
        ChangeCurrentViewMacroDirectory(theAbsoluteMacroFilePath);
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
		if(boost::iequals(itsViewMacroPath, itsRootViewMacroPath) && theDirectoryName == "..")
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

void SetCPCropGridSettings(const boost::shared_ptr<NFmiArea> &theArea, unsigned int theDescTopIndex)
{
    if(theDescTopIndex != 0)
        return; // Ei tehd‰ CP-crop juttuja kuin p‰‰karttan‰ytˆn kanssa

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
		NFmiMenuItem menuItem(theDescTopIndex, theName, static_cast<FmiParameterName>(998), kFmiAddView, g_DefaultParamView, 0, NFmiInfoData::kMacroParam, theViewRow);
		GetCombinedMapHandler()->addViewWithRealRowNumber(true, menuItem, theViewRow, false);
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
			boost::shared_ptr<NFmiMacroParamFolder> currentFolder = MacroParamSystem().GetCurrentFolder();
			if(currentFolder)
				currentFolder->Remove(theName); // poista macroparam mpsysteemist‰ (joka tuhoaa myˆs tiedostot)
			GetCombinedMapHandler()->removeMacroParamFromDrawParamLists(theName); // poista macroparam drawparamlistoista
			GetCombinedMapHandler()->mapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, true, true, false, false, true); // laitetaan viela kaikki ajat likaisiksi cachesta
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

	bool CreateCrossSectionViewPopup(int theRowIndex)
	{
		itsCurrentCrossSectionRowIndex = theRowIndex;
		fOpenPopup = false;
		itsPopupMenu = std::make_unique<NFmiMenuItemList>();

		MenuCreationSettings menuSettings;
		menuSettings.SetIsolineViewSettings(CtrlViewUtils::kFmiCrossSectionView, kFmiAddParamCrossSectionView);
		CreateParamSelectionBasePopup(menuSettings, itsPopupMenu.get(), "NormalWordCapitalAdd");

        AddSwapViewRowsToPopup(CtrlViewUtils::kFmiCrossSectionView, itsPopupMenu.get());

		NFmiDrawParamList * dpList = GetCombinedMapHandler()->getCrossSectionViewDrawParamList(theRowIndex);
		std::string menuString = ::GetDictionaryString("NormalWordCapitalShow");
		AddToCrossSectionPopupMenu(itsPopupMenu.get(), dpList, menuString, kFmiShowParamCrossSectionView);
		menuString = ::GetDictionaryString("NormalWordCapitalHide");
		AddToCrossSectionPopupMenu(itsPopupMenu.get(), dpList, menuString, kFmiHideParamCrossSectionView);
		menuString = ::GetDictionaryString("NormalWordCapitalRemove");
		AddToCrossSectionPopupMenu(itsPopupMenu.get(), dpList, menuString, kFmiRemoveParamCrossSectionView);
		menuString = ::GetDictionaryString("MapViewParamOptionPopUpActivate");
		AddToCrossSectionPopupMenu(itsPopupMenu.get(), dpList, menuString, kFmiActivateCrossSectionDrawParam);
		menuString = ::GetDictionaryString("NormalWordCapitalProperties");
		AddToCrossSectionPopupMenu(itsPopupMenu.get(), dpList, menuString, kFmiModifyCrossSectionDrawParam);

		// copy/paste komennot t‰h‰n
		menuString = "Copy draw options";
		AddToCrossSectionPopupMenu(itsPopupMenu.get(), dpList, menuString, kFmiCopyDrawParamOptionsCrossSectionView);
		if(GetCombinedMapHandler()->copyPasteDrawParamAvailableYet())
		{
			menuString = "Paste draw options";
			AddToCrossSectionPopupMenu(itsPopupMenu.get(), dpList, menuString, kFmiPasteDrawParamOptionsCrossSectionView);
		}


		menuString = ::GetDictionaryString("CrossSectionViewSelectionPopUpRemoveAll");
        auto menuItem = std::make_unique<NFmiMenuItem>(-1, menuString, NFmiDataIdent(), kFmiRemoveAllParamsCrossSectionView, g_DefaultParamView, nullptr, NFmiInfoData::kEditable);
		itsPopupMenu->Add(std::move(menuItem));

        menuString = ::GetDictionaryString("Remove all params from all rows");
        menuItem.reset(new NFmiMenuItem(CtrlViewUtils::kFmiCrossSectionView, menuString, NFmiDataIdent(), kFmiRemoveAllParamsFromAllRows, g_DefaultParamView, nullptr, NFmiInfoData::kEditable));
        itsPopupMenu->Add(std::move(menuItem));

		menuString = "Copy all row params";
		menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiCopyDrawParamsFromCrossSectionViewRow, g_DefaultParamView, nullptr, NFmiInfoData::kEditable));
		itsPopupMenu->Add(std::move(menuItem));
		if(GetCombinedMapHandler()->copyPasteCrossSectionDrawParamListUsedYet())
		{
			menuString = "Paste all row params";
			menuItem.reset(new NFmiMenuItem(-1, menuString, NFmiDataIdent(), kFmiPasteDrawParamsToCrossSectionViewRow, g_DefaultParamView, nullptr, NFmiInfoData::kEditable));
			itsPopupMenu->Add(std::move(menuItem));
		}

// ********* muuta n‰yttˆrivin kaikkien datojen tuottajat halutuiksi *********
		AddChangeAllProducersToParamSelectionPopup(static_cast<unsigned int>(-1), itsPopupMenu.get(), kFmiChangeAllProducersInCrossSectionRow, true);
// ********* muuta n‰yttˆrivin kaikkien datojen tuottajat halutuiksi *********

		// ************ t‰ss‰ muutetaan kaikki rivin datatyypit (fiksataan ongelma mik‰ syntyi kun LAPS muuttui kViewable:ista kAnalyze -tyyppiseksi)
		AddChangeAllDataTypesToParamSelectionPopup(static_cast<unsigned int>(-1), itsPopupMenu.get(), kFmiChangeAllDataTypesInCrossSectionRow);

		// ********** lis‰t‰‰n mahdollinen aika-moodin aikas‰‰tˆ komento osa *************************
		AddTimeSettingToCrossSectionPopupMenu(itsPopupMenu.get());

		// ********** lis‰t‰‰n mahdollinen trajektori komento osa *************************
		AddTrajectoryOptionsToCrossSectionPopupMenu(itsPopupMenu.get());

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
											, g_DefaultParamView
											,nullptr
											,NFmiInfoData::kViewable);
		thePopupMenu->Add(std::move(item));
	}
}

void AddTrajectoryOptionsToCrossSectionPopupMenu(NFmiMenuItemList *thePopupMenu)
{
	if(TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
	{
        auto menuItem = std::make_unique<NFmiMenuItem>(-1, "Trajectories", NFmiDataIdent(), kFmiCrossSectionSetTrajectoryTimes, g_DefaultParamView, nullptr, NFmiInfoData::kViewable); // NFmiInfoData::kViewable on vain merkityksetˆn default arvo
		NFmiMenuItemList *menuList = new NFmiMenuItemList;
        auto item = std::make_unique<NFmiMenuItem>(-1, ::GetDictionaryString("CrossSectionViewSelectionPopUpSetTimes")
											,NFmiDataIdent()
											,kFmiCrossSectionSetTrajectoryTimes
											, g_DefaultParamView
											,nullptr
											,NFmiInfoData::kViewable);
		menuList->Add(std::move(item));

		item.reset(new NFmiMenuItem(-1, ::GetDictionaryString("CrossSectionViewSelectionPopUpSetParams")
											,NFmiDataIdent()
											,kFmiCrossSectionSetTrajectoryParams
											, g_DefaultParamView
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
        auto menuItem = std::make_unique<NFmiMenuItem>(-1, theMenuTitle, NFmiDataIdent(), theMenuCommand, g_DefaultParamView, nullptr, NFmiInfoData::kViewable); // NFmiInfoData::kViewable on vain merkityksetˆn default arvo
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
												, g_DefaultParamView
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

    void ChangeCurrentViewMacroDirectory(const string& theAbsoluteMacroFilePath)
    {
        NFmiFileString fileString(theAbsoluteMacroFilePath);
        itsViewMacroPath = fileString.Device();
        itsViewMacroPath += fileString.Path();
        PathUtils::addDirectorySeparatorAtEnd(itsViewMacroPath);
        // HUOM! t‰ss‰ ei ole tarkoitus p‰ivitt‰‰ rootti-hakemistoa, koska nyt menn‰‰n
        // vain johonkin rootin alihakemistoihin
        RefreshViewMacroList();
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
		std::vector<NFmiSilamStationList::Station> &locList = theLocations.Locations();
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
                    , GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->ViewGridSize());
                if(theInfo && theInfo->Grid() && theInfo->Area()->IsInside(theLatLon) == false)
                    OutOfEditedAreaTimeSerialPoint(theLatLon);
				ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::TimeSerialView);
			}
		}

		// jos ollaan luotaus n‰ytˆn MTA-moodissa, lis‰t‰‰n myˆs valittu luotaus info dokumenttiin
		if(doMTAModeAdd)
		{
			ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::SoundingView);
			if(theMask == NFmiMetEditorTypes::kFmiDisplayedMask) // HUOM! Hiiren oikealla saa valita useita luotauspaikkoja hiiren oikealla, vaikka se olisi estetty aikasarja ikkunassa
			{ // jos oikealla klikkaus, lis‰t‰‰n yksi luotaus listaan
				NFmiMTATempSystem::TempInfo tempInfo(theLatLon, theTime, GetMTATempSystem().CurrentProducer());
				GetMTATempSystem().AddTemp(tempInfo);
//				MakeEditorDirty(theDescTopIndex, true, true, true);
				GetCombinedMapHandler()->mapViewDirty(theDescTopIndex, false, false, true, false, false, false);
			}
			else
			{
				// Jos vasen klikkaus karttan‰ytˆll‰ ja ollaan MTA-moodissa, nollataan luotaukset ja lis‰t‰‰n yksi
				GetMTATempSystem().ClearTemps();
				NFmiMTATempSystem::TempInfo tempInfo(theLatLon, theTime, GetMTATempSystem().CurrentProducer());
				GetMTATempSystem().AddTemp(tempInfo);
				GetCombinedMapHandler()->mapViewDirty(theDescTopIndex, false, false, true, false, false, false);
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
			ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::TrajectoryView);
			GetCombinedMapHandler()->mapViewDirty(theDescTopIndex, false, true, true, false, false, false);
		}
	}


	NFmiTrajectorySystem* TrajectorySystem(void)
	{
		return itsTrajectorySystem;
	}

	void DoTEMPDataUpdate(const std::string &theTEMPCodeTextStr, std::string &theTEMPCodeCheckReportStr, bool fJustCheckTEMPCode)
	{
		itsLastTEMPDataStr = theTEMPCodeTextStr;
		auto newDataPtr = NFmiTempDataGenerator::GenerateDataFromText(theTEMPCodeTextStr, theTEMPCodeCheckReportStr, WmoStationInfoSystem(), itsRawTempUnknownStartLonLat, NFmiProducer(kFmiRAWTEMP, "TEMP"), fRawTempRoundSynopTimes);
		if(newDataPtr && fJustCheckTEMPCode == false)
		{
			// otetaan TEMP koodi data k‰yttˆˆn jos lˆytyi ja  ei ollut pelkk‰ tarkistus operaatio
			bool dataWasDeleted = false;
			AddQueryData(newDataPtr.release(), "TEMPData.sqd", "TEMPDataFilePattern", NFmiInfoData::kTEMPCodeSoundingData, "", false, dataWasDeleted);
		}
	}

	void ClearTEMPData(void)
	{
        // tyhjennet‰‰n TEMP-data
		InfoOrganizer()->ClearData(NFmiInfoData::kTEMPCodeSoundingData); 
		// Karttan‰yttˆ on piirrett‰v‰ uudestaan, koska siell‰ saattaa n‰ky‰  uusia luotaus merkkej‰.
		GetCombinedMapHandler()->mapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, false, true, false, false, false);
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

	void UpdateMacroParamDataGridSizeAfterVisualizationOptimizationsChanged()
	{
		auto& visSettings = ApplicationWinRegistry().VisualizationSpaceoutSettings();
		auto baseOptimizedGridSize = visSettings.baseSpaceoutGridSize();
		NFmiPoint optimizedVisualizationGridSize(baseOptimizedGridSize, baseOptimizedGridSize);
		NFmiSmartToolModifier::UseVisualizationOptimazation(visSettings.useGlobalVisualizationSpaceoutFactorOptimization());
		InfoOrganizer()->SetOptimizedVisualizationMacroParamDataGridSize(boost::math::iround(optimizedVisualizationGridSize.X()), boost::math::iround(optimizedVisualizationGridSize.Y()));
	}

	void SetMacroParamDataGridSize(int xSize, int ySize)
	{
		InfoOrganizer()->SetMacroParamDataGridSize(xSize, ySize);
		SaveMacroParamDataGridSizeSettings();
        // Pit‰‰ tyhjent‰‰ kaikkien ikkunoiden kaikkien rivien macroParam datat (voisi optimoida, jos tiedett‰isiin, miss‰ on k‰ytˆss‰ "RESOLUTION = xxx", niit‰ ei tarvitsisi tyhjent‰‰)
        MacroParamDataCache().clearAllLayers();
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Set macro-param general data grid size", SmartMetViewId::AllMapViews, true, true);
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
		CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
		InfoOrganizer()->UpdateCrossSectionMacroParamDataSize(CrossSectionSystem()->WantedMinorPointCount(), CrossSectionSystem()->VerticalPointCount());
	}

	void RemoveAllParamsFromCrossSectionViewRow(int theRowIndex)
	{
		GetCombinedMapHandler()->removeAllCrossSectionViews(theRowIndex);
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
		NFmiMenuItem menuItem(-1, theName, static_cast<FmiParameterName>(998), kFmiAddParamCrossSectionView, g_DefaultParamView, 0, NFmiInfoData::kCrossSectionMacroParam, theViewRow);
		GetCombinedMapHandler()->addCrossSectionView(menuItem, theViewRow, false);
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
			logStr += NFmiStringTools::Convert(GetCombinedMapHandler()->getModifiedPropertiesDrawParamList()->Size());
			logStr += ")";
			LogMessage(logStr, CatLog::Severity::Info, CatLog::Category::Operational);
		}
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
		CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
		itsExtraSoundingProducerList.clear();

		try
		{
			string producersStr = NFmiSettings::Optional<string>("MetEditor::ExtraSoundingProducers", "");
			if(producersStr.empty() == false)
			{
				std::vector<string> prodStrVec = NFmiStringTools::Split<std::vector<string> >(producersStr, ",");
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
		CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
		itsDataToDBCheckMethod = NFmiSettings::Require<int>("MetEditor::DataToDBCheckMethod");
        // Option 0 was removed, when harmonization system was removed from SmartMet
        if(itsDataToDBCheckMethod == 0)
            itsDataToDBCheckMethod = 2;
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
		CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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

	boost::shared_ptr<NFmiFastQueryInfo> GetNearestSynopStationInfo(const NFmiLocation &theLocation, const NFmiMetTime &theTime, bool ignoreTime, std::vector<boost::shared_ptr<NFmiFastQueryInfo> > *thePossibleInfoVector, double maxDistanceInMeters = 1000. * kFloatMissing)
	{
		boost::shared_ptr<NFmiFastQueryInfo> info;
		std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector = (thePossibleInfoVector == 0) ? GetSortedSynopInfoVector(kFmiSYNOP, kFmiTestBed, kFmiSHIP, kFmiBUOY) : *thePossibleInfoVector;

		if(infoVector.size() > 0)
		{ // etsit‰‰n useasta infosta l‰hint‰ asemaa
			const double defaultDist = 999999999.;
			double minDist = defaultDist;
			int wantedInfoIndex = -1;
			int index = 0;
			unsigned long minLocationIndex = static_cast<unsigned long>(-1);
			for(auto &info : infoVector)
			{
				auto doShipDataLocations = NFmiFastInfoUtils::IsInfoShipTypeData(*info);
				if((ignoreTime && !doShipDataLocations) || info->Time(theTime))
				{
					FmiProducerName prod = static_cast<FmiProducerName>(info->Producer()->GetIdent());
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

	void DoSmartMetRefreshActions(const std::string& message)
	{
		LogMessage(message, CatLog::Severity::Info, CatLog::Category::Visualization);
		GetCombinedMapHandler()->mapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, true, true, true, true, true); // laitetaan kartta likaiseksi
		WindTableSystem().MustaUpdateTable(true);
		GetCombinedMapHandler()->makeApplyViewMacroDirtyActions(ApplicationWinRegistry().DrawObjectScaleFactor());

		ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs(message);
	}

	void OnButtonRefresh(const std::string& message)
	{
		DoSmartMetRefreshActions(message);
	}

	void ReloadAllDynamicHelpData()
	{
		InfoOrganizer()->ClearDynamicHelpData(false); // tuhoa kaikki olemassa olevat dynaamiset help-datat (ei edit-data tai sen kopiota ,eik‰ staattisia helpdatoja kuten topografia ja fraktiilit)
		HelpDataInfoSystem()->ResetAllDynamicDataTimeStamps(); // merkit‰‰n kaikkien dynaamisten help datojen aikaleimaksi -1, eli ei ole luettu ollenkaan
		SatelliteImageCacheSystem().ResetImages();
		// T‰nne pit‰isi lis‰t‰ muidenkin datojen uudelleen lataus, kuten kaikki Wms jutut, CAP, Hake, KaHa, muita?
		DoSmartMetRefreshActions("Reloading all the dynamic data (CTRL+SHIFT+F5)");
		// T‰m‰n j‰lkeen pit‰‰ laittaa datan luku threadi heti p‰‰lle ylemm‰ll‰ tasolla eli CSmartMetDoc-luokassa, mist‰ t‰t‰ metodia on kutsuttukkin.
	}

	void OnChangeMapType(unsigned int theDescTopIndex, bool fForward)
	{
		GetCombinedMapHandler()->changeMapType(theDescTopIndex, fForward);
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
		GetCombinedMapHandler()->mapViewDirty(theDescTopIndex, false, true, true, false, false, false);
        CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(theDescTopIndex);
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Show/hide masks on map views");
	}

	void OnShowProjectionLines(void)
	{
		NFmiProjectionCurvatureInfo* projInfo = GetCombinedMapHandler()->projectionCurvatureInfo();
		if(projInfo)
		{
			NFmiProjectionCurvatureInfo::DrawingMode mode = projInfo->GetDrawingMode();
			mode = NFmiProjectionCurvatureInfo::DrawingMode(mode + NFmiProjectionCurvatureInfo::DrawingMode(1));
			if(mode > NFmiProjectionCurvatureInfo::kOverEverything)
				mode = NFmiProjectionCurvatureInfo::kNoDraw;
			projInfo->SetDrawingMode(mode);
			GetCombinedMapHandler()->mapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, true, true, false, false, false);
            ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
            ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view projektion line draw style changed");
		}
	}

	NFmiConceptualModelData& ConceptualModelData(void)
	{
		return itsConceptualModelData;
	}

	NFmiQ2Client& Q2Client(void)
	{
		return itsQ2Client;
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
	std::vector<boost::shared_ptr<NFmiFastQueryInfo> > GetSortedSynopInfoVector(int theProducerId, int theProducerId2 = -1, int theProducerId3 = -1, int theProducerId4 = -1)
	{
		std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector = InfoOrganizer()->GetInfos(theProducerId, theProducerId2, theProducerId3, theProducerId4);

		if(itsSynopDataFilePatternSortOrderVector.size())
		{ // jos on m‰‰r‰tty sorttaus j‰rjestys, tehd‰‰n sorttaus
			std::vector<boost::shared_ptr<NFmiFastQueryInfo> > copyOfInfoVector(infoVector); // t‰st‰ listasta otetaan sorttauksessa lˆydetyt infot pois,
																	// ett‰ lopuksi voidaan laittaa j‰ljelle j‰‰neet t‰st‰ listasta lopulliseen
																	// sortattuun listaan. Alkuper‰isest‰ listasta ei voi poistaa infoja kesken loopin.
			std::vector<boost::shared_ptr<NFmiFastQueryInfo> > sortedInfoVector;
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
        GetCombinedMapHandler()->timeSerialViewDirty(true);
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

    void SetLastActiveDescTopAndViewRow(unsigned int theDesktopIndex, int theActiveRowIndex)
    {
		GetCombinedMapHandler()->activeEditedParameterMayHaveChangedViewUpdateFlagSetting(theDesktopIndex);
		auto finalActiveRowIndex = GetCombinedMapHandler()->getRealRowNumber(theDesktopIndex, theActiveRowIndex);
		ParameterSelectionSystem().SetLastActiveIndexes(theDesktopIndex, finalActiveRowIndex);
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

		if (profiling) return DoAllProfilingAnimations();

		auto& mapViewDescTops = GetCombinedMapHandler()->getMapViewDescTops();
		for(size_t mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops.size(); mapViewDescTopIndex++)
		{
			NFmiMapViewDescTop &descTop = *mapViewDescTops[mapViewDescTopIndex];
            NFmiAnimationData &animationData = descTop.AnimationDataRef();
			animationData.CurrentTime(descTop.CurrentTime()); // currentti aika pit‰‰ ottaa desctopista ja antaa animaattorille
 
			int reducedAnimationTimeSteps = CalcReducedAnimationSteps(animationData.LockMode(), descTop.MapViewDisplayMode(), static_cast<int>(descTop.ViewGridSize().X()));
            status = animationData.Animate(reducedAnimationTimeSteps);
			double waitTimeInMS = animationData.CalcWaitTimeInMSForNextFrame();
			if(waitTimeInMS < minWaitTimeInMS)
				minWaitTimeInMS = waitTimeInMS;
			if(status > maxStatus)
					maxStatus = status;
			if(mapViewDescTopIndex == 0 && status == 2)
				mainViewMustBeUpdated = true;
			if(mapViewDescTopIndex == 1 && status == 2)
				helpView1Updated = true;
			if(mapViewDescTopIndex == 2 && status == 2)
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
                    ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Animation related main map view update", GetCombinedMapHandler()->getUpdatedViewIdMaskForChangingTime());
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

	int DoAllProfilingAnimations()
	{
		unsigned int mainMapViewDescTopIndex = 0;
		NFmiMapViewDescTop& mainMapViewDescTop = *GetCombinedMapHandler()->getMapViewDescTop(mainMapViewDescTopIndex);
		NFmiAnimationData& animationData = mainMapViewDescTop.AnimationDataRef();

		int reducedAnimationTimeSteps = CalcReducedAnimationSteps(animationData.LockMode(), mainMapViewDescTop.MapViewDisplayMode(), static_cast<int>(mainMapViewDescTop.ViewGridSize().X()));
		if(profiler.tickCount() == 0)
			animationData.CurrentTime(animationData.Times().FirstTime()); // 1. kierroksella asetetaan aika alkuun
		else
			animationData.Animate(reducedAnimationTimeSteps); // Muilla kierroksilla juoksutetaan aikaa normaalisti

		if(profiler.dataCount() > 0	&& mainMapViewDescTop.CurrentTime() == animationData.Times().FirstTime())
		{
			StopProfiling();
			return 0;
		}
		else 
		{
			profiler.Tick(animationData.CurrentTime());
		}

		mainMapViewDescTop.CurrentTime(animationData.CurrentTime());
		ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Animation related main map view update (profiling)");

		return 1;
	}


	void StartProfiling() 
	{

		if (profiling) 
		{
			StopProfiling();
			return;
		}

		unsigned int mapViewDescTopIndex = 0;
		for (auto& mapViewDescTop : GetCombinedMapHandler()->getMapViewDescTops())
		{
			NFmiAnimationData& animationData = mapViewDescTop->AnimationDataRef();
			profiler.getSettings().push_back(animationData);

			animationData.SetRunMode(NFmiAnimationData::kNormal);
			animationData.LockMode(NFmiAnimationData::kNoLock);
			animationData.FrameDelayInMS(0);
			animationData.LastFrameDelayFactor(0);
			animationData.ShowVerticalControl(false);

 			if(!animationData.ShowTimesOnTimeControl() )
				animationData.Times( *mapViewDescTop->TimeControlViewTimes().ValidTimeBag() );

			mapViewDescTop->CurrentTime(animationData.Times().FirstTime());
			animationData.CurrentTime(animationData.Times().FirstTime());
			animationData.TimeStepInMinutes( animationData.Times().Resolution() );
			animationData.ShowTimesOnTimeControl(true);
			animationData.AnimationOn(true);

			GetCombinedMapHandler()->mapViewDirty(mapViewDescTopIndex, false, true, true, true, false, false);
			mapViewDescTopIndex++;
		}

		profiling = true;
	}

	void StopProfiling() 
	{
		unsigned int mapViewDescTopIndex = 0;
		for(auto& mapViewDescTop : GetCombinedMapHandler()->getMapViewDescTops())
		{
			auto times = mapViewDescTop->AnimationDataRef().Times();
			NFmiAnimationData& animationData = mapViewDescTop->AnimationDataRef();
			animationData = profiler.getSettings()[mapViewDescTopIndex];
			animationData.Times(times);
			// Hide the blue animation time-box on time-control views
			animationData.ShowTimesOnTimeControl(false);
			mapViewDescTop->MapViewDirty(false, false, true, false);
			mapViewDescTopIndex++;
		}

		profiler.Report();
		profiler.Reset();
		profiling = false;
		ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Stopping profiling view updates");
	}

	// tarkasta CView-n‰yttˆluokissa, onko mahdollisesti animaatiota k‰ynniss‰. Jos on, ‰l‰ laita odota-cursoria n‰kyviin, koska se vilkuttaa ik‰v‰sti
	bool ShowWaitCursorWhileDrawingView(void)
	{
		for(auto& mapViewDescTop : GetCombinedMapHandler()->getMapViewDescTops())
		{
			if(mapViewDescTop->AnimationDataRef().IsAnimationRunning() || mapViewDescTop->AnimationDataRef().ShowTimesOnTimeControl())
				return false;
		}
		return true;
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
		NFmiMapViewDescTop *descTop = GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex);
		if(descTop)
		{
			descTop->AnimationDataRef().ShowTimesOnTimeControl(!(descTop->AnimationDataRef().ShowTimesOnTimeControl()));
			GetCombinedMapHandler()->mapViewDirty(theDescTopIndex, false, false, true, false, false, false); // t‰m‰n pit‰isi asettaa n‰yttˆ p‰ivitys tilaan, mutta cachea ei tarvitse en‰‰ erikseen tyhjent‰‰
            ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: Show/hide animation times on map view's time control view", GetWantedMapViewIdFlag(theDescTopIndex));
		}
	}

	const NFmiPoint& TimeSerialViewSizeInPixels(void) const
	{
		return itsTimeSerialViewSizeInPixels;
	}
	void TimeSerialViewSizeInPixels(const NFmiPoint &newValue)
	{
		itsTimeSerialViewSizeInPixels = newValue;
	}

	NFmiWindTableSystem& WindTableSystem(void)
	{
		return itsWindTableSystem;
	}

    NFmiPoint GetPrintedMapAreaOnScreenSizeInPixels(unsigned int theDescTopIndex)
	{
		NFmiMapViewDescTop &descTop = *(GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex));
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

	NFmiMetTime GetNewerOriginTimeFromInfos(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfos, const NFmiMetTime &theCurrentLatestTime)
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
			std::vector<boost::shared_ptr<NFmiFastQueryInfo> > groundInfos = InfoOrganizer()->GetInfos(NFmiInfoData::kViewable, true, prodId);
			latestTime = GetNewerOriginTimeFromInfos(groundInfos, latestTime);
			std::vector<boost::shared_ptr<NFmiFastQueryInfo> > pressureInfos = InfoOrganizer()->GetInfos(NFmiInfoData::kViewable, false, prodId);
			latestTime = GetNewerOriginTimeFromInfos(pressureInfos, latestTime);
			std::vector<boost::shared_ptr<NFmiFastQueryInfo> > hybridInfos = InfoOrganizer()->GetInfos(NFmiInfoData::kHybridData, false, prodId);
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
			GetCombinedMapHandler()->makeMapViewRowDirty(theDescTopIndex, theViewRowIndex);
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
			GetCombinedMapHandler()->makeMapViewRowDirty(theDescTopIndex, theViewRowIndex);
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
		CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
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
		// Auto generoidut soundingIndex datat talletetaan local-data hakemiston cache alihakemiston alle
        return itsHelpDataInfoSystem.LocalDataCacheDirectory();
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

	void ActivateViewParamSelectorDlg(unsigned int theMapViewDescTopIndex)
	{
        ApplicationInterface::GetApplicationInterfaceImplementation()->ActivateParameterSelectionDlg(theMapViewDescTopIndex);
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
		CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
		itsMachineThreadCount = static_cast<int>(std::thread::hardware_concurrency());

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

	bool IsEditedDataInReadOnlyMode()
	{
		auto editedData = EditedInfo();
		if(editedData)
			return editedData->RefRawData()->IsReadOnly();
		// Jos editoitua dataa ei ole, palautetaan arvo ett‰ se on read-only moodissa
		return true;
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
				GetCombinedMapHandler()->mapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, true, true, false, true, false);
                ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Edited data modifications Redo", GetUpdatedViewIdMaskForEditingData());
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
				GetCombinedMapHandler()->mapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, true, true, false, true, false);
                ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Edited data modifications Undo", GetUpdatedViewIdMaskForEditingData());
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
		if(CaseStudyModeOn())
			return itsCaseStudyHelpDataInfoSystem.get();
		else
			return &itsHelpDataInfoSystem;
	}

	NFmiDataLoadingInfo& GetUsedDataLoadingInfo(void)
	{
		if(CaseStudyModeOn())
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

		for(auto& mapViewDescTop : GetCombinedMapHandler()->getMapViewDescTops())
		{
			mapViewDescTop->AnimationDataRef().AdjustTimesRelatedtoCaseStudyModeChange(theFromWallClockTime, theToWallClockTime);
			mapViewDescTop->SetCaseStudyTimes(theToWallClockTime);
        }
        CrossSectionSystem()->SetCaseStudyTimes(theToWallClockTime);
        GetMTATempSystem().SetCaseStudyTimes(theToWallClockTime);
        TrajectorySystem()->SetCaseStudyTimes(theToWallClockTime);

        itsDataLoadingInfoNormal.CaseStudyTime(theToWallClockTime);
        itsDataLoadingInfoNormal.UpdatedTimeDescriptor(false);
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
		bool oldCaseStudyModeOn = CaseStudyModeOn();
		boost::shared_ptr<NFmiHelpDataInfoSystem> oldCaseStudyHelpDataInfoSystem = itsCaseStudyHelpDataInfoSystem;
		NFmiCaseStudySystem oldLoadedCaseStudySystem = itsLoadedCaseStudySystem;
		try
		{
			// 1. Ota metadatasta polku talteen
			std::string caseStudyBasePath = NFmiFileSystem::PathFromPattern(theCaseStudyMetaFile);
			// 2. Lataa metadata
			if(itsLoadedCaseStudySystem.ReadMetaData(theCaseStudyMetaFile, ApplicationInterface::GetSmartMetViewAsCView(), true))
			{
			// 3. Luo metadatan avulla HelpDataInfoSystem (HUOM! t‰ss‰ pit‰‰ k‰ytt‰‰ suoraan itsHelpDataInfoSystem-dataosiota, kun silt‰ pyydet‰‰n static datoja)
				itsCaseStudyHelpDataInfoSystem = itsLoadedCaseStudySystem.MakeHelpDataInfoSystem(itsHelpDataInfoSystem, caseStudyBasePath);
				if(itsCaseStudyHelpDataInfoSystem)
				{
			// 3.1. Laita CaseStudy-moodi p‰‰lle
					CaseStudyModeOn(true);
			// 4. Laita CaseStudyn kellonaika sein‰kelloajaksi
			// 5. Laita kaikkien n‰yttˆjen kello CaseStudy-aikaan
                    NFmiMetTime oldWallClockTime = oldCaseStudyModeOn ? oldLoadedCaseStudySystem.Time() : NFmiMetTime();
                    SetAllSystemsToCaseStudyModeChangeTime(oldWallClockTime, itsLoadedCaseStudySystem.Time(), false);
					itsLoadedCaseStudySystem.SetUpDataLoadinInfoForCaseStudy(itsDataLoadingInfoCaseStudy, caseStudyBasePath);

			// 6. Heit‰ kaikki dynaaminen data roskiin
					InfoOrganizer()->ClearDynamicHelpData(true); // tuhoa kaikki olemassa olevat dynaamiset help-datat (ei edit-data tai sen kopiota ,eik‰ staattisia helpdatoja kuten topografia ja fraktiilit)
                    InitializeSatelImageCacheForCaseStudy();
					auto usedAbsoluteCaseStudyHakeDirectory = NFmiCaseStudySystem::MakeCaseStudyDataHakeDirectory(NFmiCaseStudySystem::MakeBaseDataDirectory(theCaseStudyMetaFile, itsLoadedCaseStudySystem.Name()));
					itsWarningCenterSystem.goIntoCaseStudyMode(usedAbsoluteCaseStudyHakeDirectory);
					// Merkit‰‰n taas aluksi luetut datat 'vanhoiksi'
					CFmiDataLoadingThread2::ResetFirstTimeGoingThroughState();

                    // Lopetetaan cache datojen lataus ja siivous
                    CFmiQueryDataCacheLoaderThread::AutoLoadNewCacheDataMode(false);

                    // 7. Laita HelpDataInfoSystem-tarvittaviin paikkoihin k‰yttˆˆn (HelpDataInfoSystem-funktio, datanlataus-threadeihin jne.) JA aloita dynaamisen datan uudelleen lataus
			// CSmartMetDoc:in CaseStudyLoadingActions myˆs likaa cachet ja p‰ivitt‰‰ lopuksi kaikki n‰ytˆt!!, joten se pit‰‰ tehd‰ viimeisen‰...
                    ApplicationInterface::GetApplicationInterfaceImplementation()->CaseStudyLoadingActions(itsLoadedCaseStudySystem.Time(), "Going into case study mode"); // itsCaseStudyHelpDataInfoSystem -pit‰‰ olla ladattuna ja fCaseStudyModeOn pit‰‰ olla asetettuna true:ksi ennen t‰m‰n kutsua

					ParameterSelectionSystem().reInitialize(ProducerSystem(), ObsProducerSystem(), SatelImageProducerSystem(), *HelpDataInfoSystem());
					PrepareForParamAddSystemUpdate();
					TryAutoStartUpLoad();
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
		CaseStudyModeOn(oldCaseStudyModeOn);
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

	void CaseStudyModeOn(bool newState)
	{
		fCaseStudyModeOn = newState;
		CFmiDataLoadingThread2::SetCaseStudyMode(fCaseStudyModeOn);
		AddParams::ProducerData::SetCaseStudyMode(fCaseStudyModeOn);
	}

	bool CaseStudyModeOn() const
	{
		return fCaseStudyModeOn;
	}

	void CaseStudyToNormalMode()
	{
		// 3.1. Laita CaseStudy-moodi p‰‰lle
		CaseStudyModeOn(false);
		fChangingCaseStudyToNormalMode = true;
        SetAllSystemsToCaseStudyModeChangeTime(itsLoadedCaseStudySystem.Time(), NFmiMetTime(), true);
		InfoOrganizer()->ClearDynamicHelpData(true); // tuhoa kaikki olemassa olevat dynaamiset help-datat (ei edit-data tai sen kopiota ,eik‰ staattisia helpdatoja kuten topografia ja fraktiilit)
        InitializeSatelImageCacheForCaseStudy();
		itsWarningCenterSystem.goIntoNormalModeFromStudyMode();
		// Merkit‰‰n taas aluksi luetut datat 'vanhoiksi'
		CFmiDataLoadingThread2::ResetFirstTimeGoingThroughState();

		// Palataan taas normaaliin cache datojen lataukseen ja siivoukseen
        CFmiQueryDataCacheLoaderThread::AutoLoadNewCacheDataMode(ApplicationWinRegistry().ConfigurationRelatedWinRegistry().AutoLoadNewCacheData());
        ApplicationInterface::GetApplicationInterfaceImplementation()->CaseStudyToNormalModeActions();
		ParameterSelectionSystem().reInitialize(ProducerSystem(), ObsProducerSystem(), SatelImageProducerSystem(), *HelpDataInfoSystem());
		PrepareForParamAddSystemUpdate();
	}

	bool StoreCaseStudyMemory(void)
	{
		if(itsCaseStudySystem.CategoriesData().size() > 0) // turha tallettaa mit‰‰n, jos CaseStudy-systeemi‰ ei ole edes alustettu
		{
            if(itsCaseStudySystem.AreStoredMetaDataChanged(itsCaseStudySystemOrig))
            {
				itsCaseStudySystemOrig = itsCaseStudySystem;
				itsCaseStudySystem.UpdateValuesBackToWinRegistry(ApplicationWinRegistry().CaseStudySettingsWinRegistry());
			    return true;
            }
		}
		return false;
	}

	int DoMessageBox(const std::string & theMessage, const std::string &theTitle, unsigned int theMessageBoxType)
	{
        return ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(theMessage.c_str()), CA2T(theTitle.c_str()), theMessageBoxType);
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

	std::vector<int>& DataLoadingProducerIndexVector(void)
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
		CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
		itsSpecialFileStoragePath = NFmiSettings::Optional<std::string>("SmartMet::SpecialFileStoragePath", ""); // oletus arvo on tyhj‰
        if(!itsSpecialFileStoragePath.empty())
        {
			itsSpecialFileStoragePath = PathUtils::makeFixedAbsolutePath(itsSpecialFileStoragePath, itsBasicConfigurations.ControlPath(), true);
			CatLog::logMessage(std::string("SpecialFileStoragePath = ") + itsSpecialFileStoragePath, CatLog::Severity::Info, CatLog::Category::Configuration);

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
		    GetCombinedMapHandler()->mapViewDirty(theMapViewDescTopIndex, true, true, true, true, false, false);
			GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->SetBorderDrawDirtyState(CountryBorderDrawDirtyState::Geometry);
			GetCombinedMapHandler()->setMapArea(theMapViewDescTopIndex, newZoomedArea);
            // Laitetaan t‰h‰n "Ohitetaan normi p‰ivit‰ kaikki n‰ytˆt ja p‰ivit‰ vain muuttunutta karttan‰yttˆ‰" -toiminto optimoinnin takia.
            // HUOM! Optimoinnin voi laittaa t‰nne, koska esim. viewMacrojen latauksissa ei k‰ytet‰ t‰t‰ metodia (jolloin tarvitsee p‰ivitt‰‰ kaikkia n‰yttˆj‰)
            CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(theMapViewDescTopIndex);
            if(GetWantedMapViewIdFlag(theMapViewDescTopIndex) == SmartMetViewId::MainMapView)
            {
                // Jos kyse p‰‰karttan‰ytˆn zoomista, pit‰‰ myˆs p‰ivitt‰‰ asemadatataulukkon‰yttˆ‰, koska siihen laitetaan
                // vain p‰kartalla n‰kyv‰t synop asemat ja Warning dialogia (HAKE sanomat n‰ytet‰‰n p‰‰kartan zoomin mukaan).
                ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::StationDataTableView | SmartMetViewId::WarningCenterDlg);
            }
        }
   }

    void PanMap(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theMapArea, const NFmiPoint &theMousePoint, const NFmiPoint &theZoomDragUpPoint)
    {
        NFmiRect maxXYRect(theMapArea->XYArea(GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->MapHandler()->TotalArea().get()));
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
        boost::shared_ptr<NFmiArea> totalMapArea = GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex)->MapHandler()->TotalArea();
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

		GetCombinedMapHandler()->storeMapViewSettingsToWinRegistry();
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
		std::string finalMpcpProcessPath;
        std::string configuredMasterProcessPath = NFmiSettings::Optional<std::string>(processSettingsKey, "");
        if(configuredMasterProcessPath.empty())
        {

            std::string usedAppPath = absoluteSmartMetAppPath;
            usedAppPath += "\\";
            usedAppPath += defaultProcessName;
			finalMpcpProcessPath = usedAppPath;
        }
        else
        {
            if(NFmiFileSystem::IsAbsolutePath(configuredMasterProcessPath))
            {
				finalMpcpProcessPath = configuredMasterProcessPath;
            }
            else
            {
				finalMpcpProcessPath = PathUtils::getAbsoluteFilePath(configuredMasterProcessPath, absoluteSmartMetAppPath);
            }
        }
		finalMpcpProcessPath = PathUtils::simplifyWindowsPath(finalMpcpProcessPath);
		return finalMpcpProcessPath;
    }

    bool MakeSureToolMasterPoolIsRunning2()
    {
        // Tarkista onko Master-prosessi jo k‰ynniss‰, jos oli, lopetetaan
        if(MasterProcessRunningCount() <= 0)
        {
            std::string usedAppPath = ApplicationDataBase().GetDecodedApplicationDirectory();

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
        std::string rootPath = ApplicationWinRegistry().FixedDrawParamsPath();
        rootPath = PathUtils::makeFixedAbsolutePath(rootPath, ControlDirectory());
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
		CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
		try
        {
            itsSatelliteImageCacheSystemPtr->Init(*HelpDataInfoSystem());
            NFmiSatelliteImageCacheSystem::StartUpdateThreads(itsSatelliteImageCacheSystemPtr);
            NFmiSatelliteImageCacheSystem::ImageUpdateCallbackFunction updateCallback = std::bind(&GeneralDocImpl::ImageCacheUpdatedCallback, this, std::placeholders::_1);
            NFmiSatelliteImageCacheSystem::ImageUpdateCallbackFunction loadCallback = std::bind(&GeneralDocImpl::ImageCacheLoadedCallback, this, std::placeholders::_1);
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

    void ImageCacheUpdatedCallback(ImageCacheUpdateData &theUpdatedData)
    {
        std::lock_guard<std::mutex> lock(itsImageCacheUpdateDataMutex);
        itsImageCacheUpdateData.splice(itsImageCacheUpdateData.end(), theUpdatedData);
    }

    void ImageCacheLoadedCallback(ImageCacheUpdateData &theLoadedData)
    {
        std::lock_guard<std::mutex> lock(itsImageCacheUpdateDataMutex);
        itsImageCacheUpdateData.splice(itsImageCacheUpdateData.end(), theLoadedData);
    }

    int DoImageCacheUpdates(const ImageCacheUpdateData &theImageCacheUpdateData)
    {
        if(theImageCacheUpdateData.size())
        {
            bool foundUpdates = false;
			auto& mapViewDescTops = GetCombinedMapHandler()->getMapViewDescTops();
            for(unsigned int i = 0; i < static_cast<unsigned int>(mapViewDescTops.size()); i++)
                foundUpdates |= DoImageCacheUpdates(i, theImageCacheUpdateData);

            if(foundUpdates)
            {
                ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: satellite images from files loaded update", SmartMetViewId::AllMapViews); // t‰m‰ sitten p‰ivitt‰‰ kaikki ruudut
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
        NFmiMapViewDescTop *descTop = GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex);

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
            ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: Reload failed satellite images update", SmartMetViewId::AllMapViews, true, false);
        }
    }

    void OnAcceleratorSetHomeTime(int theMapViewDescTopIndex)
    {
        NFmiMetTime currentTime(1);
        if(CaseStudyModeOn())
            currentTime = itsLoadedCaseStudySystem.Time(); // CaseStudy moodissa sein‰kelloksi otetaan CasStudyn oma aika
		GetCombinedMapHandler()->centerTimeControlView(theMapViewDescTopIndex, currentTime, true);
        RefreshApplicationViewsAndDialogs("Map view: Selected map time is set to wall clock time", true, false, theMapViewDescTopIndex);
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

		auto dataTriggerList = GetDataTriggerListOwnership();
        if(BetaProductionSystem().DoNeededBetaAutomation(dataTriggerList, *InfoOrganizer()))
        {
			// P‰ivitet‰‰n beta-dialogia, sill‰ jos se on auki ja beta-automaatio tabi on p‰‰ll‰, 
			// haluamme ett‰ n‰emme automaatio listalla p‰ivittyv‰t next ja last ajo ajat
			ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Beta-automation triggered, update beta-dialog", SmartMetViewId::BetaProductionDlg);
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
        boost::ireplace_first(relativePath, itsRootViewMacroPath, "");
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
			bool dataWasDeleted = false;
			AddQueryData(data.release(), filePath, filePattern, NFmiInfoData::kObservations, "", false, dataWasDeleted);
        }
    }

    // Tarkistetaan onko tehty uutta Hake/Kaha sanoma pohjaista queryDataa ja laitetaan se infoOrganizeriin
    void CheckForNewWarningMessageData()
    {
#ifndef DISABLE_CPPRESTSDK
        AddMessageBasedData(itsWarningCenterSystem.getHakeQueryData(), "Hake Messages", "Hake Messages"); // "New warning center message data");
        AddMessageBasedData(itsWarningCenterSystem.getKahaQueryData(), "KaHa Messages", "KaHa Messages"); // "New Kansalais Havainto message data");
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
    boost::shared_ptr<NFmiFastQueryInfo> GetModelClimatologyData(const NFmiLevel &theLevel)
    {
        auto infoVector = itsSmartInfoOrganizer->GetInfos(NFmiInfoData::kClimatologyData);
        for(auto &info : infoVector)
        {
            if(NFmiFastInfoUtils::IsModelClimatologyData(info))
            {
				if(theLevel.GetIdent() != 0)
				{
					// Jos level ei ole ns. default level (id = 0)
					if(info->Level(theLevel))
						return info; // returns the first suitable data with suitable level
					else
						info->FirstLevel(); // Jos kyse oli pintadatasta, josta ei lˆydy haluttua leveli‰, pit‰‰ info laittaa osoittamaan taas pinta-leveliin
				}
				else
				{
					if(info->SizeLevels() == 1)
		                return info; // returns the first suitable surface (level count 1) data
				}
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

    boost::shared_ptr<NFmiFastQueryInfo> SeekMostFavoriteProducerData(std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &dataVector, const std::vector<unsigned long> &favoriteProducers, const NFmiLevel* actualLevel, const NFmiProducer &actualProducer)
    {
        if(dataVector.size())
        {
			boost::shared_ptr<NFmiFastQueryInfo> backupData = nullptr;
			size_t backupDataIndex = 999;
			// Katsotaan ensin lˆytyykˆ ihan t‰ysosuma dataa
			for(auto& info : dataVector)
			{
				auto isLevelDataWanted = actualLevel != nullptr;
				auto isLevelData = info->SizeLevels() > 1;
				auto isMatchingLevel = (isLevelDataWanted && isLevelData && info->Level(*actualLevel));
				if(!isLevelDataWanted || isMatchingLevel)
				{
					auto currentInfoProducerId = info->Producer()->GetIdent();
					if(actualProducer.GetIdent() == currentInfoProducerId)
						return info;
					auto favoriteIter = std::find(favoriteProducers.begin(), favoriteProducers.end(), currentInfoProducerId);
					if(favoriteIter != favoriteProducers.end())
					{
						size_t diff = std::distance(favoriteProducers.begin(), favoriteIter);
						if(diff < backupDataIndex)
						{
							backupDataIndex = diff;
							backupData = info;
						}
					}
				}
			}

			return backupData;
        }
        // Return empty if there is no data on vector
        return boost::shared_ptr<NFmiFastQueryInfo>();
    }
	
    boost::shared_ptr<NFmiFastQueryInfo> GetBestSuitableModelFractileData(boost::shared_ptr<NFmiFastQueryInfo> &usedOriginalInfo)
    {
        auto infoVector = itsSmartInfoOrganizer->GetInfos(NFmiInfoData::kClimatologyData);
        auto filteredInfoVector = FilterOnlyGridSurfaceNonYearLongData(infoVector);
        // Seek first if there is data from these producers, in this order. 
        // If none found, then return first data in filtered list.
        // 199 = harmonie, 54 = gfs, 260 = MEPS
        std::vector<unsigned long> favoriteProducers{kFmiMTAECMWF, 260, 54, kFmiMTAHIRLAM, 199 };
		const NFmiLevel* level = nullptr;
		// Jos ei ole annettu usedOriginalInfo:a, niin silloin haetaan p‰‰s‰‰ntˆisesti Ec:n fraktiilidataa
		NFmiProducer wantedProducer(kFmiMTAECMWF, "Ecmwf");
		if(usedOriginalInfo)
		{
			wantedProducer = *usedOriginalInfo->Producer();
			if(usedOriginalInfo->SizeLevels() > 1)
			{
				level = usedOriginalInfo->Level();
			}
		}
        return SeekMostFavoriteProducerData(filteredInfoVector, favoriteProducers, level, wantedProducer);
    }

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
		CombinedMapHandlerInterface::doVerboseFunctionStartingLogReporting(__FUNCTION__);
		try
        {
            auto customCategories = CustomMenuFolders();
            auto levelBag = SoundingPlotLevels().Levels();
            parameterSelectionSystem.setSoundingLevels(*levelBag);
            
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
#ifndef DISABLE_CPPRESTSDK
			if (GetCombinedMapHandler()->wmsSupportAvailable())
			{
				auto wmsCallBackFunction = [this]() {return this->GetCombinedMapHandler()->getWmsSupport(); };
				parameterSelectionSystem.setWmsCallback(wmsCallBackFunction);
			}
#endif // DISABLE_CPPRESTSDK

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
                GetCombinedMapHandler()->timeSerialViewDirty(true);
                ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews | SmartMetViewId::TimeSerialView);
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

    void InitTimeSerialParameters()
    {
        try
        {
            itsTimeSerialParameters.initializeFromConfigurations();
        }
        catch(std::exception& e)
        {
            LogAndWarnUser(e.what(), "Error with TimeSerialParameter configuration", CatLog::Severity::Error, CatLog::Category::Configuration, true, false, true);
        }
    }

    TimeSerialParameters& GetTimeSerialParameters()
    {
        return itsTimeSerialParameters;
    }

    NFmiColorContourLegendSettings& ColorContourLegendSettings()
    {
        return itsColorContourLegendSettings;
    }

	CombinedMapHandlerInterface* GetCombinedMapHandler()
	{
		return &itsCombinedMapHandler;
	}

	int CurrentCrossSectionRowIndex()
	{
		return itsCurrentCrossSectionRowIndex;
	}

	NFmiParameterInterpolationFixer& ParameterInterpolationFixer()
	{
		return itsParameterInterpolationFixer;
	}

	NFmiSeaLevelPlumeData& SeaLevelPlumeData()
	{
		return itsSeaLevelPlumeData;
	}

	NFmiLedLightStatusSystem& LedLightStatusSystem()
	{
		return itsLedLightStatusSystem;
	}

	int GetApplicationRunnningTimeInSeconds()
	{
		auto t1 = BasicSmartMetConfigurations().SmartMetStartingTime().UTCTime().EpochTime();
		auto t2 = time(0);
		double diff = difftime(t2, t1);
		return boost::math::iround(diff);
	}

	// FixDataElapsedTimeInSeconds funktio korjaa yhden mahdollisen querydatoihin tehdyn aikav‰‰rennˆksen.
	// QueryDatoihin talletetaan timeri joka laskee sen latauksesta k‰yttˆˆn kuluneen ajan.
	// Kyseisen ajan avulla voidaan merkit‰ k‰yttˆliittym‰ss‰ alle 5 minuuuttia sitten ladatatut datat uusiksi
	// erilaisin korostuksin.
	// Mutta kun smartmet lataa dataa heti k‰ynnistyksen yhteydess‰ lokaali cachesta, ei datoja
	// haluttu olevan 'uusia' kuin minuutin, joten niihin lis‰ttiin 4 minuuttia latausaikaa.
	// Nyt kun tutkitaan onko uutta dataa tullut tarpeeksi nopeasti, t‰m‰ 4:n minuutin aika pit‰‰ poistaa,
	// silloin kun latausaika on suurempi kuin smartmetin k‰ynniss‰oloaika.
	int FixDataElapsedTimeInSeconds(int elapsedDataTimeInSeconds)
	{
		auto runninTimeInSeconds = GetApplicationRunnningTimeInSeconds();
		if(elapsedDataTimeInSeconds > runninTimeInSeconds)
		{
			elapsedDataTimeInSeconds -= 4*60;
		}
		return elapsedDataTimeInSeconds;
	}

	void AddSpaceToNonEmptyString(std::string &str)
	{
		if(!str.empty())
		{
			str += " ";
		}
	}

	std::string MakeElapsedTimeString(int timeInSeconds)
	{
		const int secondsInDay = 3600 * 24;
		const int secondsInHour = 3600;
		std::string str;
		int days = (int)(timeInSeconds / secondsInDay);
		int remainingTimeInSeconds = timeInSeconds - (days * secondsInDay);
		int hours = (int)(remainingTimeInSeconds / secondsInHour);
		remainingTimeInSeconds = remainingTimeInSeconds - (hours * secondsInHour);
		int minutes = (int)(remainingTimeInSeconds / 60);
		if(days > 0)
		{
			AddSpaceToNonEmptyString(str);
			str += NFmiStringTools::Convert<int>(days);
			str += " d";
		}

		if(hours > 0)
		{
			AddSpaceToNonEmptyString(str);
			str += NFmiStringTools::Convert<int>(hours);
			str += " h";
		}
		
		if(minutes > 0)
		{
			AddSpaceToNonEmptyString(str);
			str += NFmiStringTools::Convert<int>(minutes);
			str += " min";
		}
		return str;
	}

	void DoIsAnyQueryDataLateChecks()
	{
		// Katsotaan onko miss‰‰n queryDatassa konffattuna ns. myˆh‰stymisaikaraja.
		// Jos on, katsotaan milloin kyseist‰ dataa on ladattu viimeksi. 
		// Jos aikaa on kulunut enemm‰n kuin konffeissa on s‰‰detty, laitetaan 
		// StatusBarin ledissysteemiin varoitus asiasta.
		const auto& helpDataInfoVector = HelpDataInfoSystem()->DynamicHelpDataInfos();
		for(const auto& helpInfo : helpDataInfoVector)
		{
			if(helpInfo.IsAgingTimeLimitUsed())
			{
				auto fileFilter = helpInfo.UsedFileNameFilter(*HelpDataInfoSystem());
				auto& infos = InfoOrganizer()->GetInfos(fileFilter);
				if(!infos.empty())
				{
					auto& info = infos.front();
					auto elapsedTimeInSeconds = FixDataElapsedTimeInSeconds(boost::math::iround(info->ElapsedTimeFromLoadInSeconds()));
					if(elapsedTimeInSeconds > helpInfo.AgingTimeLimitInMinutes() * 60)
					{
						std::string logMessage = "No new data for ";
						logMessage += info->DataFileName();
						logMessage += " in ";
						logMessage += MakeElapsedTimeString(elapsedTimeInSeconds);
						logMessage += " (limit = ";
						logMessage += MakeElapsedTimeString(helpInfo.AgingTimeLimitInMinutes()*60);
						logMessage += ")";
						NFmiLedLightStatusSystem::ReportToChannelFromThread(NFmiLedChannel::DataIsLate, fileFilter, logMessage, CatLog::Severity::Info);
					}
				}
			}
		}
	}

	NFmiMouseClickUrlActionData& MouseClickUrlActionData()
	{
		return itsMouseClickUrlActionData;
	}

	void ToggleVirtualTimeMode()
	{
		itsVirtualTimeData.ToggleVirtualTimeMode(CaseStudyModeOn(), CaseStudySystem().Time());
	}

	bool VirtualTimeUsed() const 
	{ 
		return itsVirtualTimeData.VirtualTimeUsed(); 
	}

	const NFmiMetTime& VirtualTime() const
	{
		return CaseStudyModeOn() ? itsVirtualTimeData.CaseStudyVirtualTime() : itsVirtualTimeData.NormalVirtualTime();
	}

	void VirtualTime(const NFmiMetTime& virtualTime)
	{
		itsVirtualTimeData.VirtualTime(virtualTime, CaseStudyModeOn());
	}

	std::string GetVirtualTimeTooltipText() const
	{
		return itsVirtualTimeData.GetVirtualTimeTooltipText(CaseStudyModeOn());
	}

	NFmiVirtualTimeData itsVirtualTimeData;
	NFmiMouseClickUrlActionData itsMouseClickUrlActionData;
	std::vector<std::string> itsLoadedDataTriggerList;
	NFmiLedLightStatusSystem itsLedLightStatusSystem;
	NFmiSeaLevelPlumeData itsSeaLevelPlumeData;
	NFmiParameterInterpolationFixer itsParameterInterpolationFixer;
	NFmiCombinedMapHandler itsCombinedMapHandler;
	bool fChangingCaseStudyToNormalMode = false;
    NFmiColorContourLegendSettings itsColorContourLegendSettings;
    TimeSerialParameters itsTimeSerialParameters;
    NFmiMacroParamDataCache itsMacroParamDataCache;
	// T‰t‰ nime‰ k‰ytet‰‰n smartmet:in p‰‰ikkunan title tekstiss‰ (jotta k‰ytt‰j‰ n‰kee mik‰ viewMacro on ladattuna)
	std::string itsLastLoadedViewMacroName; 
    Warnings::CapDataSystem capDataSystem;
    Q2ServerInfo itsQ2ServerInfo;
    AddParams::ParameterSelectionSystem parameterSelectionSystem;
	// T‰h‰n ladataan SmartMetin tyhj‰ alkutilanne, jotta SHIFT + F12 pikan‰pp‰imell‰ p‰‰see takaisin alkutilaan milloin tahansa ajon aikana.
	NFmiViewSettingMacro itsStartupViewMacro; 
	// Jos konffeissa on niin m‰‰r‰tty, t‰h‰n luetaan editoijien tunnukset, ja t‰t‰ listaa k‰ytet‰‰n L‰het‰-data-tietokantaan dialogissa
	NFmiEditDataUserList itsEditDataUserList; 
	// T‰h‰n on listattu rekursiivisesti kaikki perus smarttool hakemistosta haetut tiedostot, joissa on oikea vmr -p‰‰te. 
	// Tiedoston nimill‰ on relatiivienn polku (suhteessa perus hakemistoon)
	std::vector<std::string> itsSmartToolFileNames; 
	// T‰h‰n talletetaan havaitut korruptoituneet n‰yttˆmakro nimet, ett‰ niiden avulla voidaan varoittaa k‰ytt‰ji‰ niist‰ viewMacro-dialogissa 
	std::vector<std::string> itsCorruptedViewMacroFileList; 
    std::vector<NFmiLightWeightViewSettingMacro> itsViewMacroDescriptionList;
	// T‰h‰n on listattu rekursiivisesti kaikki perus viewMacro hakemistosta haetut tiedostot, 
	// joissa on oikea vmr -p‰‰te. Tiedoston nimill‰ on relatiivienn polku (suhteessa perus hakemistoon)
	std::vector<std::string> itsViewMacroFileNames; 
	// Milloin on viimeksi l‰hetetty editoitu data tietokantaan
	NFmiMetTime itsLastEditedDataSendTime; 
	// Onko viimeisen editoidun datan tietokantaan l‰hetyksen j‰lkeen tullut operatiivista dataa k‰yttˆˆn?
	bool fLastEditedDataSendHasComeBack; 
	// T‰h‰n asetetaan beta-tuotannossa se juuri nyt generoitava beta-tuote, muuten t‰ss‰ pit‰‰ olla nullptr (ei omista, ei tuhoa)
	const NFmiBetaProduct *itsCurrentGeneratedBetaProduct; 
    NFmiBetaProductionSystem itsBetaProductionSystem;
	// T‰h‰n tulee tietoa p‰ivitett‰vist‰ satel/muut parametrien kuvista ja niiden ajoista
	ImageCacheUpdateData itsImageCacheUpdateData; 
	// t‰ll‰ lukitaan itsImageCacheUpdateData, koska siihen tulee lis‰yksi‰ worker-threadista ja siit‰ siit‰ luetaan p‰‰-threadista
	std::mutex itsImageCacheUpdateDataMutex; 
    std::shared_ptr<NFmiSatelliteImageCacheSystem> itsSatelliteImageCacheSystemPtr;
	// Aina kun tehd‰‰n hiirell‰ kartalta vasen click valintaa, laitetaan t‰h‰n tarkat koordinaatit
	NFmiPoint itsPreciseTimeSerialLatlonPoint; 
	// T‰m‰ viewMacro saadaan ladattua k‰yttˆˆn SmartMetissa pikan‰pp‰imell‰ CTRL + F12. 
	// Kyseess‰ on edellisen SmartMet istunnon aikana talletettu viewMacro, t‰t‰ talletetaan m‰‰r‰ajoin (joka 1.5 minuutti) 
	// crashBackupViewMacro.wmr tiedostoon. T‰m‰ siis ladataan k‰ynnistyksen yhteydess‰ ja sitten SmartMet alkaa tekem‰‰n kyseiseen tiedostoon uusia talletuksia.
	NFmiViewSettingMacro itsCrashBackupViewMacro; 
	// T‰h‰n on tallletettu kaikki ns. tehdasasetus drawParamit. K‰ytt‰j‰t voivat k‰ytt‰‰ n‰it‰, mutta eiv‰t voi muuttaa.
	NFmiFixedDrawParamSystem itsFixedDrawParamSystem; 
	// Kuinka paljon levytilaa pit‰‰ olla minimiss‰‰n, jotta SmartMet suostuu tekem‰‰n konffi/muita asetus tiedosto talletuksia
	float itsHardDriveFreeLimitForConfSavesInMB; 
	// Kuinka paljon levytilaa pit‰‰ olla minimiss‰‰n, jotta SmartMet suostuu tekem‰‰n editoidun datan backup tiedosto talletuksia
    float itsHardDriveFreeLimitForEditedDataSavesInMB; 
    NFmiMultiProcessPoolOptions itsMultiProcessPoolOptions;
	// t‰m‰n avulla initialisoidaan boost-log
	std::string itsMultiProcessLogFilePath; 
	// t‰m‰n avulla siivotaan loki hakemistosta vanhat lokit pois
	std::string itsMultiProcessLogFilePattern; 
    MultiProcessClientData itsMultiProcessClientData;
	// joskus on tarpeen ett‰ SmartMet tallettaa viimeisen ladatun tiedoston nimen talteen, mutta oletuksena se ei ole tarpeen.
    bool fStoreLastLoadedFileNameToFile; 
	// SmartMetin voi s‰‰t‰‰ halutessa tallettamaan tietyt tiedostot pois SmartMetin kontrollihakemistosta. 
	// T‰t‰ tarvitaan erityisesti ulkomaisissa Dropbox-asennuksissa (ks. SMARTMET-393).
	// Jos asetusta ei ole s‰‰detty, k‰ytet‰‰n polkuna normaalia kontrollihakemistoa.
    std::string itsSpecialFileStoragePath; 
    NFmiApplicationWinRegistry itsApplicationWinRegistry;
	// Jos kontrollipiste muokkaukset halutaan rajoittaa tietyn ali-hilan alueelle, 
	// k‰ytet‰‰n t‰t‰ hilapiste-rect:i‰. T‰‰ll‰ on siis bottom-left ja top-right editoidun datan hila-indeksit.
	// T‰m‰n arvoa p‰ivitet‰‰n kun zoomataan p‰‰ikkunaa.
	NFmiRect itsCPGridCropRect; 
	// flagi ett‰ k‰ytet‰‰nkˆ croppia vai ei. T‰m‰ menee p‰‰lle aikasarjaikkunasta.
	bool fUseCPGridCrop; 
	// T‰ss‰ on tallessa latlon kulmapisteet croppi laatikosta (kartalle piirtoa varten)
	boost::shared_ptr<NFmiArea> itsCPGridCropLatlonArea; 
	// T‰ss‰ on tallessa latlon kulmapisteet sisemm‰st‰ croppi laatikosta (kartalle piirtoa varten)
	boost::shared_ptr<NFmiArea> itsCPGridCropInnerLatlonArea; 
	// t‰ss‰ kerrotaan kuinka monta hilapistett‰ x- ja y-suunnassa on ulko- ja sis‰-laatikon v‰liss‰. 
	// T‰ll‰ alueella muutokset menev‰t t‰ydest‰ l‰hes nollaan, jotta reunoista tulisi mahdollisimman pehme‰t.
	NFmiPoint itsCPGridCropMargin; 
	// haluan piirt‰‰ maalattaessa valitut pisteet piirto ikkunaan, t‰m‰ on sit‰ varten tehty kikka vitonen
	bool fDrawSelectionOnThisView; 
	// onko muutettu editoinnissa valittuja pisteit‰, jos on, niin ruutua pit‰‰ p‰ivitt‰‰ tietyll‰ tavalla.
	bool fEditedPointsSelectionChanged; 
	// T‰‰ll‰ on tietyt perus asetukset, mitk‰ alustetaan jo ennen GenDocin alustusta
	// CSmartMetApp:in InitInstance-metodissa. Uusi CrashRpt-systeemi vaatii tiettyjen tietojen asetuksia
	// ennen kuin GenDoc saadaan alustetuksia, ja siksi t‰m‰ piti tehd‰ n‰in osissa ja ehk‰ hieman hankalasti.
	NFmiBasicSmartMetConfigurations itsBasicConfigurations; 
	// uudet datan modifiointi rutiinit (FmiModifyEditdData -namespacessa) saattavat h‰iriinty‰ multi-thread koodin tai 
	// erillisess‰ threadissa ajosta johtuen (progress/Cancel vaatii erillist‰ s‰iett‰). T‰st‰ johtuen kokeilen, 
	// josko muokkausten aikan SmartMetin p‰ivityksen esto auttaisi asiaa. T‰m‰ esto toimii ainakin printtauksien 
	// yhteydess‰ koettujen ongelmien kanssa.
	bool fDataModificationInProgress;	
	// k‰ytt‰j‰ voi asettaa erikseen sallitaanko multi-threaddaavan koodin k‰yttˆ editoitaessa
	bool fUseMultiThreaddingWithEditingtools; 
	boost::shared_ptr<TimeSerialModificationDataInterface> itsGenDocDataAdapter;
	// eri apudatojen tiedot ovat t‰ss‰ ja ne luetaan tiedostosta
	NFmiHelpDataInfoSystem itsHelpDataInfoSystem; 
	// K‰ytˆss‰ olevan CaseStudyn eri apudatojen tiedot ovat t‰ss‰
	boost::shared_ptr<NFmiHelpDataInfoSystem> itsCaseStudyHelpDataInfoSystem; 
	bool fCaseStudyModeOn;

	NFmiAviationStationInfoSystem itsWmoStationInfoSystem;
	bool fWmoStationInfoSystemInitialized;
	// t‰ll‰ tehd‰‰n uusia CaseStudy-datoja
	NFmiCaseStudySystem itsCaseStudySystem; 
	// Lopussa kun SmartMetia suljetaan, tarkistetaan onko tehty muutoksia suhteessa 
	// originaali olioon ja vasta sitten tehd‰‰n talletukset asetuksiin
	NFmiCaseStudySystem itsCaseStudySystemOrig; 
	// T‰h‰n ladataan olemassa oleva CaseStudy-data k‰yttˆˆn
	NFmiCaseStudySystem itsLoadedCaseStudySystem; 
	// Kun todetaan ett‰ editoitua dataa pit‰‰ alkaa kytt‰‰m‰‰n, k‰ynnistet‰‰n t‰m‰ timeri, ett‰ tiedet‰‰n kuinka kauan ollaan odotettu. 
	// Jotta k‰ytt‰j‰‰ voidaan informoida asiasta ja lis‰ksi ehk‰ tietyn ajan yritt‰misen j‰lkeen voidaan lopettaa homma toivottomana.
	NFmiMilliSecondTimer itsEditedDataNeedsToBeLoadedTimer;
	// Jos auto startup ep‰onnistuu lataamasta mit‰‰ editoitua dataa, laitetaan t‰m‰ lippu p‰‰lle.
	// Sen j‰lkeen pit‰‰ MainFramessa timerill‰ k‰yd‰ v‰liajoin katsomassa onko tullut sopivaa dataa ladattavaksi, kunnes sit‰ lˆytyy.
	bool fEditedDataNeedsToBeLoaded; 

	NFmiMacroPathSettings itsMacroPathSettings;
	NFmiIgnoreStationsData itsIgnoreStationsData;
	// T‰m‰ on brute force ratkaisu ei queryData pohjaisten datojen (mm. satel kuvat, k‰site analyysit)
	// ruudunp‰ivityksen varmistamiseen. T‰m‰ kertoo kuinka usein tarkasteluja tehd‰‰n. Jos arvo on <= 0, ei tarkastella ollenkaan.
	int itsSatelDataRefreshTimerInMinutes; 
	// kertoo kuinka paljon koneessa on theaddaus mahdollisuutta (CPU x core x hyper-threading)
	int itsMachineThreadCount; 
	NFmiDataQualityChecker itsDataQualityChecker;
	NFmiAnalyzeToolData itsAnalyzeToolData;
	NFmiModelDataBlender itsModelDataBlender;
	// Erilaisissa paikoissa on aiemmin k‰ytetty luotausn‰ytˆn v‰risetti‰ hyv‰kseen kuvaamaan eri k‰yri‰ (aikasarja, trajektori, jne)
	// Nyt teen oman v‰ri setin t‰ll‰iseen k‰yttˆˆn, koska esim. LENiss‰ on v‰ritetty kaikki luotaukset mustaksi
	std::vector<NFmiColor> itsGeneralColors; 

	ObsDataLoadedReporter itsObsDataLoadedReporter;
	NFmiAutoComplete itsAutoComplete;
	std::deque<NFmiViewSettingMacro> itsUndoRedoViewMacroList;
	// T‰m‰ osoittaa nykyiseen kohtaan yll‰ olevaan undo-listaan.
	// T‰m‰ indeksi osoittaa siis siihen viewMacroon, johon seuraava undo kohdistuu.
	// Jos arvo on -1, ollaan listan alun ohi ja undo:ta ei voi tehd‰
	int itsUndoRedoViewMacroListPointer; 
	// Kun SmartMet lataa datan alussa automaattisesti, se ei tarkista luettua dataa mitenk‰‰n.
	// Jos editoidaan dataa, pit‰‰ datan olla oikeanlaista, konfiguraatioiden mukaista. Tein vivun,josta
	// n‰kee, ett‰ pit‰‰kˆ data tarkistaa viel‰ kun editoidaan tai dataa l‰hetet‰‰n tietokantaan.
	bool fStartUpDataLoadCheckDone; 
	NFmiWindTableSystem itsWindTableSystem;
	NFmiPoint itsTimeSerialViewSizeInPixels;
	// t‰m‰ on aikasarja ikkunassa n‰kyv‰ aikav‰li
	NFmiTimeBag itsTimeSerialViewTimeBag; 
	NFmiMapViewTimeLabelInfo itsMapViewTimeLabelInfo;
	bool itsShowToolTipTimeView;
	bool itsShowToolTipTempView;
	bool itsShowToolTipTrajectoryView;
	// t‰m‰ on m‰‰r‰tty asetuksissa helpdatainfo.conf-tiedostossa.
	// t‰m‰ pit‰‰ asettaa aina kun joku n‰yttˆ piirt‰‰ isoviiva/contoureja. T‰m‰n avulla piirret‰‰n
	// l‰pin‰kyv‰t parametrit pinell‰ kikalla ja Gdiplus-piirtoa hyv‰ksi k‰ytt‰en.
	std::vector<std::string> itsSynopDataFilePatternSortOrderVector; 
	CWnd *itsTransparencyContourDrawView; 
	// rintama piirto ominaisuudet on t‰‰ll‰
	NFmiConceptualModelData itsConceptualModelData;
	NFmiPoint itsOutOfEditedAreaTimeSerialPoint;
	// luotauksia piirret‰‰n karttan‰ytˆlle vain tiettyihin vakio painepinnoille
	NFmiVPlaceDescriptor itsSoundingPlotLevels; 
	// kun dataa l‰hetet‰‰n tietokantaan, kysyt‰‰n l‰hetys dialogissa, mit‰ tarkastuus metodia
	// halutaan k‰ytt‰‰. 1=datavalidation, 2= niin kuin asetuksissa sanotaan
	// DBChecker ohitetaan t‰ss‰.
	int itsDataToDBCheckMethod; 
	// settingseissa on lista mahdollisista ylim‰‰r‰isist‰ tuottajista,
	// joita on tarkoitus katsella luotaus-n‰ytˆll‰
	std::vector<NFmiProducer> itsExtraSoundingProducerList; 
	NFmiQ2Client itsQ2Client;
	// miss‰ hilassa asema/piste datan hilaus lasketaan
	NFmiPoint itsStationDataGridSize; 
	// tietyss‰ tilanteessa CDokumentti voidaan pakottaa piirt‰m‰‰n aikasarjan‰yttˆ
	bool fMustDrawTimeSerialView; 
	// tietyss‰ tilanteessa CDokumentti voidaan pakottaa piirt‰m‰‰n poikkileikkausn‰yttˆn‰yttˆ
	bool fMustDrawCrossSectionView; 
	// tietyss‰ tilanteessa CDokumentti voidaan pakottaa piirt‰m‰‰n luotausn‰yttˆ
	bool fMustDrawTempView; 
	bool fUseOnePressureLevelDrawParam;
	bool fRawTempRoundSynopTimes;
	NFmiPoint itsRawTempUnknownStartLonLat;
	// kaikilla konfiguraatioilla ei voi tallettaa tyˆtiedostoa, eik‰ siit‰ tarvitse varoittaa, t‰m‰ luetaan settingseist‰
	bool fWarnIfCantSaveWorkingFile; 
	// poikkileikkausn‰yttˆ ikkunan clientti osan koko pikseleiss‰
	NFmiPoint itsCrossSectionViewSizeInPixels; 
	NFmiHelpEditorSystem itsHelpEditorSystem;

	NFmiProducerSystem itsProducerSystem;
	NFmiProducerSystem itsObsProducerSystem;
	// satelliitti/tutka kuvien tuottajat laitetaan t‰h‰n (k‰ytet‰‰n mm. parametri pop-up valikoiden tekoon)
	NFmiProducerSystem itsSatelImageProducerSystem; 
#ifndef DISABLE_CPPRESTSDK
    HakeMessage::Main itsWarningCenterSystem;
#endif // DISABLE_CPPRESTSDK
	NFmiFileCleanerSystem itsFileCleanerSystem;

	bool fIsTEMPCodeSoundingDataAlsoCopiedToEditedData;
	// t‰h‰n on talletettu aina viimeksi k‰ytetty TEMP koodin purussa ollut stringi
	std::string itsLastTEMPDataStr; 
	// jos true, CreateLoadedData-metodissa k‰ytet‰‰n alla olevaa parambagi‰, muuten se otetaan 1. source datsta
	bool fUseEditedDataParamDescriptor; 
	// editor.conf-tiedostossa voidaan halutessa m‰‰ritt‰ rakennettava
	// ladatun datan parambagi. Jos ei m‰‰ritelty, ottaa parambagin 1. source-datsta
	NFmiParamDescriptor itsEditedDataParamDescriptor; 

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

    CtrlViewUtils::FmiSmartMetEditingMode itsSmartMetEditingMode;

	string itsViewMacroPath;
	string itsRootViewMacroPath; // jos ollaan 'rootti' hakemistossa, ei laiteta ..-hakemistoa viewparamlistaan
    std::shared_ptr<NFmiViewSettingMacro> itsCurrentViewMacro;
    NFmiViewSettingMacro itsHelperViewMacro; // t‰m‰ on apuna n‰yttˆ-makrojen latauksessa ja talletuksessa. Todelliset makrot ovat makrolistassa
											// mutta esim. windows asetukset talletetaan t‰m‰n avulla ennen kuin todellinen makro on edes olemassa ja kopioidaan
											// t‰st‰ lopulliseen makroon makro listaan.

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

	bool fCPDataBackupUsed; // Tarkistus bitti sille onko jostain syyst‰ pit‰nyt lukea kˇnnistyksen yhteydess‰ CP-datan backup. Tiedon avulla ei tarvitse lukea uudestaan Init vaiheessa CP-pisteit‰ (ja tuhota muutos k‰yri‰).

	std::vector<int> itsDataLoadingProducerIndexVector; // t‰ss‰ on tiedot siit‰, mit‰ tuottajaa k‰ytet‰‰n mill‰kin ajan hetkell‰ latauksen yhteydess‰

	bool Printing(void){return fPrinting;};

	void Printing(bool newStatus){fPrinting = newStatus;}
	bool fPrinting;

    void SetPrintedDescTopIndex(int nowPrintedDescTopIndex) { itsPrintedDescTopIndex = nowPrintedDescTopIndex; }
    int GetPrintedDescTopIndex() { return itsPrintedDescTopIndex; }
    void ResetPrintedDescTopIndex() { itsPrintedDescTopIndex = CtrlViewUtils::kFmiDescTopIndexNone; }

    int itsPrintedDescTopIndex = CtrlViewUtils::kFmiDescTopIndexNone;

	NFmiEditMapDataListHandler* itsListHandler; //laura lis‰si 30081999
	NFmiDataLoadingInfo itsDataLoadingInfoNormal;
	NFmiDataLoadingInfo itsDataLoadingInfoCaseStudy;

	NFmiInfoOrganizer *itsSmartInfoOrganizer; // holds different kind of smartinfo's

	bool fOpenPopup;
	std::unique_ptr<NFmiMenuItemList> itsPopupMenu;
	unsigned long itsPopupMenuStartId;

	NFmiLocationSelectionTool *itsLocationSelectionTool2;

	std::string itsFileDialogDirectoryMemory;

	bool fUseMasksInTimeSerialViews;

	int itsToolTipRealRowIndex; // hiiren kursorin lepopaikan n‰yttˆrivi tooltippi‰ varten
	int itsToolTipColumnIndex; // hiiren kursorin lepopaikan n‰yttˆrivin sarake numero tooltippi‰ varten
	NFmiPoint itsToolTipLatLonPoint; // hiiren kursorin lepopaikka tooltippi‰ varten (kartan koordinaatistossa latlon-piste)
	NFmiMetTime itsToolTipTime; // tooltipill‰ voi olla mik‰ tahansa ruudukon aika, ja se talletetaan t‰h‰n
	int itsToolTipMapViewDescTopIndex; // Mist‰ karttan‰ytˆlt‰ tooltip otettiin
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
	int itsLastBrushedViewRealRowIndex;

	NFmiMetTime itsTimeFilterStartTime;
	NFmiMetTime itsTimeFilterEndTime;

	int itsFilterDialogUpdateStatus; // 0 = ei tarvii tehd‰ mit‰‰n, 1 = p‰ivit‰ aikakontrolli-ikkunaa
									 // 2 = p‰ivit‰ koko ruutu
	bool fUseTimeInterpolation;

	NFmiMetEditorOptionsData itsMetEditorOptionsData;
	NFmiCPManagerSet itsCPManagerSet;

	AnimationProfiler profiler;
	bool profiling;
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
    try
    {
        SetGeneralDataDocInterfaceCallbacks();
        bool status = pimpl->Init(theBasicConfigurations, mapViewsPositionMap, otherViewsPositionPosMap);
        CrossSectionSystem()->SetDocumentInterface(&GetCtrlViewDocumentInterface());
        return status;
    }
	catch(AbortSmartMetInitializationGracefullyException&)
	{
		std::string message = "User has stopped application due to an error occured";
		LogMessage(message, CatLog::Severity::Info, CatLog::Category::Configuration, true);
		throw; // Heitet‰‰n poikkeus edelleen, se napataan CSmartMetApp::InitGeneralDataDoc -metodissa
	}
	catch(std::exception& e)
    {
        std::string errorMessage = "Uncaught exeption was thrown from SmartMet's initialization: ";
        errorMessage += e.what();
        LogAndWarnUser(errorMessage, "SmartMet initialization failed", CatLog::Severity::Error, CatLog::Category::Configuration, false, true, true);
    }
	catch(...)
    {
        std::string errorMessage = "Unknown uncaught exeption was thrown from SmartMet's initialization";
        LogAndWarnUser(errorMessage, "SmartMet initialization failed", CatLog::Severity::Error, CatLog::Category::Configuration, false, true, true);
    }
    return false;
}



void NFmiEditMapGeneralDataDoc::AddQueryData(NFmiQueryData* theData, const std::string& theDataFileName, const std::string& theDataFilePattern,
										int theType, const std::string& theNotificationStr, bool loadFromFileState, bool& dataWasDeleted)
{
	pimpl->AddQueryData(theData, theDataFileName, theDataFilePattern, NFmiInfoData::Type(theType), theNotificationStr, loadFromFileState, dataWasDeleted);
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

bool NFmiEditMapGeneralDataDoc::StoreOptionsData(void)
{
	return pimpl->StoreOptionsData();
}

NFmiParamBag& NFmiEditMapGeneralDataDoc::AllStaticParams(void)
{
	return pimpl->AllStaticParams();
}

bool NFmiEditMapGeneralDataDoc::CreateParamSelectionPopup(unsigned int theDescTopIndex, int theRowIndex)
{
	pimpl->itsCurrentViewRowIndex = theRowIndex;
	return pimpl->CreateParamSelectionPopup(theDescTopIndex);
}

bool NFmiEditMapGeneralDataDoc::CreateMapViewTimeBoxPopup(unsigned int theDescTopIndex)
{
	return pimpl->CreateMapViewTimeBoxPopup(theDescTopIndex);
}

bool NFmiEditMapGeneralDataDoc::CreateViewParamsPopup(unsigned int theDescTopIndex, int theRowIndex, int layerIndex, double layerIndexRealValue)
{
	return pimpl->CreateViewParamsPopup(theDescTopIndex, theRowIndex, layerIndex, layerIndexRealValue);
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

bool NFmiEditMapGeneralDataDoc::DoAreaFiltering()
{
	return pimpl->DoAreaFiltering();
}

bool NFmiEditMapGeneralDataDoc::DoTimeFiltering(void)
{
	return pimpl->DoTimeFiltering();
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

bool NFmiEditMapGeneralDataDoc::MakeDataValiditation(void)
{
	return pimpl->MakeDataValiditation();
}


bool NFmiEditMapGeneralDataDoc::CheckAndValidateAfterModifications(NFmiMetEditorTypes::FmiUsedSmartMetTool theModifyingTool, bool fMakeDataSnapshotAction, unsigned int theLocationMask, FmiParameterName theParam)
{
	return pimpl->CheckAndValidateAfterModifications(theModifyingTool, fMakeDataSnapshotAction, theLocationMask, theParam);
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
	return pimpl->itsPopupMenu.get();
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
int NFmiEditMapGeneralDataDoc::ToolTipRealRowIndex(void)
{
	return pimpl->itsToolTipRealRowIndex;
}

void NFmiEditMapGeneralDataDoc::ToolTipRealRowIndex(int newRealRowIndex)
{
	pimpl->itsToolTipRealRowIndex = newRealRowIndex;
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
int NFmiEditMapGeneralDataDoc::ToolTipMapViewDescTopIndex() const { return pimpl->itsToolTipMapViewDescTopIndex; }
void NFmiEditMapGeneralDataDoc::ToolTipMapViewDescTopIndex(int newIndex) { pimpl->itsToolTipMapViewDescTopIndex = newIndex; }
void NFmiEditMapGeneralDataDoc::ToolTipTime(const NFmiMetTime& theTime){pimpl->itsToolTipTime = theTime;};
const NFmiMetTime& NFmiEditMapGeneralDataDoc::ToolTipTime(void){return pimpl->itsToolTipTime;};
int NFmiEditMapGeneralDataDoc::FilterFunction(void){return pimpl->itsFilterFunction;};
void NFmiEditMapGeneralDataDoc::FilterFunction(int newFilter){pimpl->itsFilterFunction = newFilter;};
const NFmiMetTime& NFmiEditMapGeneralDataDoc::ActiveViewTime(void){return pimpl->itsActiveViewTime;};
void NFmiEditMapGeneralDataDoc::ActiveViewTime(const NFmiMetTime& theTime){pimpl->itsActiveViewTime = theTime;};

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
int NFmiEditMapGeneralDataDoc::LastBrushedViewRealRowIndex(void){return pimpl->itsLastBrushedViewRealRowIndex;};
void NFmiEditMapGeneralDataDoc::LastBrushedViewRealRowIndex(int newRealRowIndex){pimpl->itsLastBrushedViewRealRowIndex = newRealRowIndex;};
const NFmiMetTime& NFmiEditMapGeneralDataDoc::TimeFilterStartTime(void){return pimpl->itsTimeFilterStartTime;};
const NFmiMetTime& NFmiEditMapGeneralDataDoc::TimeFilterEndTime(void){return pimpl->itsTimeFilterEndTime;};
int NFmiEditMapGeneralDataDoc::FilterDialogUpdateStatus(void){return pimpl->FilterDialogUpdateStatus();};
void NFmiEditMapGeneralDataDoc::FilterDialogUpdateStatus(int newState){pimpl->FilterDialogUpdateStatus(newState);};
bool NFmiEditMapGeneralDataDoc::UseTimeInterpolation(void){return pimpl->fUseTimeInterpolation;};
void NFmiEditMapGeneralDataDoc::UseTimeInterpolation(bool newState){pimpl->fUseTimeInterpolation = newState;};
bool NFmiEditMapGeneralDataDoc::DoTimeSeriesValuesModifying(boost::shared_ptr<NFmiDrawParam> &theModifiedDrawParam, int theUsedMask, NFmiTimeDescriptor& theTimeDescriptor, std::vector<float> &theModificationFactorCurvePoints, NFmiMetEditorTypes::FmiUsedSmartMetTool theEditorTool, bool fUseSetForDiscreteData, int theUnchangedValue)
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

bool NFmiEditMapGeneralDataDoc::IsToolMasterAvailable() const
{return pimpl->IsToolMasterAvailable();}
void NFmiEditMapGeneralDataDoc::ToolMasterAvailable(bool newValue)
{pimpl->ToolMasterAvailable(newValue);}

AnimationProfiler& NFmiEditMapGeneralDataDoc::GetProfiler() { return pimpl->profiler; }

void NFmiEditMapGeneralDataDoc::StartProfiling() {

	pimpl->StartProfiling();

}

void NFmiEditMapGeneralDataDoc::StopProfiling() {

	pimpl->StopProfiling();
}

// tallettaa mm. CP pisteet, muutosk‰yr‰t jne.
void NFmiEditMapGeneralDataDoc::StoreSupplementaryData(void)
{pimpl->StoreSupplementaryData();}

void NFmiEditMapGeneralDataDoc::LogMessage(const std::string& theMessage, CatLog::Severity severity, CatLog::Category category, bool flushLogger)
{ pimpl->LogMessage(theMessage, severity, category, flushLogger); }

void NFmiEditMapGeneralDataDoc::LogAndWarnUser(const std::string &theMessageStr, const std::string &theDialogTitleStr, CatLog::Severity severity, CatLog::Category category, bool justLog, bool addAbortOption, bool flushLogger)
{ pimpl->LogAndWarnUser(theMessageStr, theDialogTitleStr, severity, category, justLog, addAbortOption, flushLogger); }

void NFmiEditMapGeneralDataDoc::SetDataLoadingProducerIndexVector(const std::vector<int>& theIndexVector)
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
void NFmiEditMapGeneralDataDoc::StoreViewMacro(const std::string & theAbsoluteMacroFilePath, const std::string &theDescription)
{
	pimpl->StoreViewMacro(theAbsoluteMacroFilePath, theDescription);
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

NFmiHelpEditorSystem& NFmiEditMapGeneralDataDoc::HelpEditorSystem(void)
{
	return pimpl->HelpEditorSystem();
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

boost::shared_ptr<NFmiFastQueryInfo> NFmiEditMapGeneralDataDoc::GetNearestSynopStationInfo(const NFmiLocation &theLocation, const NFmiMetTime &theTime, bool ignoreTime, std::vector<boost::shared_ptr<NFmiFastQueryInfo> > *thePossibleInfoVector, double maxDistanceInMeters)
{
	return pimpl->GetNearestSynopStationInfo(theLocation, theTime, ignoreTime, thePossibleInfoVector, maxDistanceInMeters);
}

void NFmiEditMapGeneralDataDoc::OnButtonRefresh(const std::string& message)
{
	pimpl->OnButtonRefresh(message);
}

void NFmiEditMapGeneralDataDoc::OnChangeMapType(unsigned int theDescTopIndex, bool fForward)
{
	pimpl->OnChangeMapType(theDescTopIndex, fForward);
}

void NFmiEditMapGeneralDataDoc::OnShowMasksOnMap(unsigned int theDescTopIndex)
{
	pimpl->OnShowMasksOnMap(theDescTopIndex);
}

void NFmiEditMapGeneralDataDoc::OnShowProjectionLines(void)
{
	pimpl->OnShowProjectionLines();
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

std::vector<boost::shared_ptr<NFmiFastQueryInfo> > NFmiEditMapGeneralDataDoc::GetSortedSynopInfoVector(int theProducerId, int theProducerId2, int theProducerId3, int theProducerId4)
{
	return pimpl->GetSortedSynopInfoVector(theProducerId, theProducerId2, theProducerId3, theProducerId4);
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

void NFmiEditMapGeneralDataDoc::TimeSerialViewTimeBag(const NFmiTimeBag &theTimeBag)
{
	pimpl->TimeSerialViewTimeBag(theTimeBag);
}

const NFmiTimeBag& NFmiEditMapGeneralDataDoc::TimeSerialViewTimeBag(void) const
{
	return pimpl->TimeSerialViewTimeBag();
}

int NFmiEditMapGeneralDataDoc::DoAllAnimations(void)
{
	return pimpl->DoAllAnimations();
}

int NFmiEditMapGeneralDataDoc::DoAllProfilingAnimations(void)
{
	return pimpl->DoAllProfilingAnimations();
}

bool NFmiEditMapGeneralDataDoc::ShowWaitCursorWhileDrawingView(void)
{
	return pimpl->ShowWaitCursorWhileDrawingView();
}

bool NFmiEditMapGeneralDataDoc::DoAutoLoadDataAtStartUp(void) const
{
	return pimpl->DoAutoLoadDataAtStartUp();
}
void NFmiEditMapGeneralDataDoc::DoAutoLoadDataAtStartUp(bool newValue)
{
	pimpl->DoAutoLoadDataAtStartUp(newValue);
}

void NFmiEditMapGeneralDataDoc::ToggleTimeControlAnimationView(unsigned int theDescTopIndex)
{
	pimpl->ToggleTimeControlAnimationView(theDescTopIndex);
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

void NFmiEditMapGeneralDataDoc::ActivateViewParamSelectorDlg(unsigned int theMapViewDescTopIndex)
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

bool NFmiEditMapGeneralDataDoc::IsEditedDataInReadOnlyMode()
{
	return pimpl->IsEditedDataInReadOnlyMode();
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

std::vector<int>& NFmiEditMapGeneralDataDoc::DataLoadingProducerIndexVector(void)
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

boost::shared_ptr<NFmiFastQueryInfo> NFmiEditMapGeneralDataDoc::GetModelClimatologyData(const NFmiLevel& theLevel)
{
    return pimpl->GetModelClimatologyData(theLevel);
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiEditMapGeneralDataDoc::GetBestSuitableModelFractileData(boost::shared_ptr<NFmiFastQueryInfo>& usedOriginalInfo)
{
    return pimpl->GetBestSuitableModelFractileData(usedOriginalInfo);
}

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

Q2ServerInfo& NFmiEditMapGeneralDataDoc::GetQ2ServerInfo()
{
    return pimpl->GetQ2ServerInfo();
}

Warnings::CapDataSystem& NFmiEditMapGeneralDataDoc::GetCapDataSystem()
{
    return pimpl->GetCapDataSystem();
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

bool NFmiEditMapGeneralDataDoc::DoMapViewOnSize(int mapViewDescTopIndex, const NFmiPoint &clientPixelSize, CDC* pDC)
{
    return pimpl->DoMapViewOnSize(mapViewDescTopIndex, clientPixelSize, pDC);
}

TimeSerialParameters& NFmiEditMapGeneralDataDoc::GetTimeSerialParameters()
{
    return pimpl->GetTimeSerialParameters();
}

NFmiColorContourLegendSettings& NFmiEditMapGeneralDataDoc::ColorContourLegendSettings()
{
    return pimpl->ColorContourLegendSettings();
}

void NFmiEditMapGeneralDataDoc::SetPrintedDescTopIndex(int nowPrintedDescTopIndex) 
{ 
    pimpl->SetPrintedDescTopIndex(nowPrintedDescTopIndex);
}

int NFmiEditMapGeneralDataDoc::GetPrintedDescTopIndex() 
{ 
    return pimpl->GetPrintedDescTopIndex();
}

void NFmiEditMapGeneralDataDoc::ResetPrintedDescTopIndex() 
{ 
    pimpl->ResetPrintedDescTopIndex();
}

CombinedMapHandlerInterface* NFmiEditMapGeneralDataDoc::GetCombinedMapHandler()
{
	return pimpl->GetCombinedMapHandler();
}

void NFmiEditMapGeneralDataDoc::SetCPCropGridSettings(const boost::shared_ptr<NFmiArea>& newArea, unsigned int mapViewDescTopIndex)
{
	pimpl->SetCPCropGridSettings(newArea, mapViewDescTopIndex);
}

int NFmiEditMapGeneralDataDoc::CurrentCrossSectionRowIndex()
{
	return pimpl->CurrentCrossSectionRowIndex();
}

NFmiParameterInterpolationFixer& NFmiEditMapGeneralDataDoc::ParameterInterpolationFixer()
{
	return pimpl->ParameterInterpolationFixer();
}

void NFmiEditMapGeneralDataDoc::UpdateMacroParamDataGridSizeAfterVisualizationOptimizationsChanged()
{
	pimpl->UpdateMacroParamDataGridSizeAfterVisualizationOptimizationsChanged();
}

NFmiSeaLevelPlumeData& NFmiEditMapGeneralDataDoc::SeaLevelPlumeData()
{
	return pimpl->SeaLevelPlumeData();
}

NFmiLedLightStatusSystem& NFmiEditMapGeneralDataDoc::LedLightStatusSystem()
{
	return pimpl->LedLightStatusSystem();
}

std::shared_ptr<NFmiViewSettingMacro> NFmiEditMapGeneralDataDoc::CurrentViewMacro()
{
	return pimpl->CurrentViewMacro();
}

void NFmiEditMapGeneralDataDoc::DoIsAnyQueryDataLateChecks()
{
	pimpl->DoIsAnyQueryDataLateChecks();
}

NFmiMouseClickUrlActionData& NFmiEditMapGeneralDataDoc::MouseClickUrlActionData()
{
	return pimpl->MouseClickUrlActionData();
}

void NFmiEditMapGeneralDataDoc::ToggleVirtualTimeMode()
{
	pimpl->ToggleVirtualTimeMode();
}

bool NFmiEditMapGeneralDataDoc::VirtualTimeUsed() const
{
	return pimpl->VirtualTimeUsed();
}

const NFmiMetTime& NFmiEditMapGeneralDataDoc::VirtualTime() const
{
	return pimpl->VirtualTime();
}

void NFmiEditMapGeneralDataDoc::VirtualTime(const NFmiMetTime& virtualTime)
{
	pimpl->VirtualTime(virtualTime);
}

std::string NFmiEditMapGeneralDataDoc::GetVirtualTimeTooltipText() const
{
	return pimpl->GetVirtualTimeTooltipText();
}
