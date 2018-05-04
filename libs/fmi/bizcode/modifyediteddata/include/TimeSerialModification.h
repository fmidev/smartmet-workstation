#pragma once

#include <vector>
#include "boost/shared_ptr.hpp"
#include "NFmiMetEditorTypes.h"
#include "NFmiInfoData.h"
#include "NFmiParameterName.h"
#include "NFmiAreaMask.h"
#include "NFmiTimeDescriptor.h"
#include "FmiSmartMetEditingMode.h"
#include "TimeSerialModificationDataInterface.h"

class NFmiThreadCallBacks;

namespace FmiModifyEditdData
{
	typedef std::vector<std::pair<std::string, NFmiInfoData::Type> > DataVector;

	// Tämän TimeSerialModificationDataAdapter-luokan avulla on siis tarkoitus ajaa konsoli testi ajoja.
	// Tämä tuottaa tarvittavat datat erilaisiin testeihin (mahdollisimman yksinkertaisesti toteutettuna).
	class TimeSerialModificationDataAdapter : public TimeSerialModificationDataInterface
	{
	public:

		TimeSerialModificationDataAdapter(void);
		~TimeSerialModificationDataAdapter(void);
		bool Initialize(DataVector &theUsedDatas, bool useMasks, bool fDoValidations);

		bool CheckValidationFromSettings(void) override;
		boost::shared_ptr<NFmiFastQueryInfo> EditedInfo(void) override;
		boost::shared_ptr<NFmiAreaMaskList> ParamMaskList(void) override;
		bool UseMasksInTimeSerialViews(void) override {return fUseMasksInTimeSerialViews;}
		NFmiAnalyzeToolData& AnalyzeToolData(void) override;
		NFmiInfoOrganizer* InfoOrganizer(void) override;
		bool EditedDataNotInPreferredState(void) override {return false;}
		void EditedDataNotInPreferredState(bool /* newValue */ ) override {}
		int DoMessageBox(const std::string & /* theMessage */ , const std::string & /* theTitle */ , unsigned int /* theMessageBoxType */ ) override {return 1;}
		void AllMapViewDescTopsTimeDirty(const NFmiMetTime &theTime) override;
		void AreaViewDirty(unsigned int theDescTopIndex, bool areaViewDirty, bool clearCache) override;
		void MapDirty(unsigned int theDescTopIndex, bool mapDirty, bool clearCache) override;
		void WindTableSystemMustaUpdateTable(bool /* newState */) override {}
		boost::shared_ptr<NFmiDrawParam> GetUsedDrawParam(const NFmiDataIdent &theDataIdent, NFmiInfoData::Type theDataType) override;
		NFmiMetEditorOptionsData& MetEditorOptionsData(void) override;
		boost::shared_ptr<NFmiEditorControlPointManager> CPManager(bool getOldSchoolCPManager = false) override;
		int TimeEditSmootherValue(void) override {return 0;}
        CtrlViewUtils::FmiSmartMetEditingMode SmartMetEditingMode(void) override {return itsSmartMetEditingMode;}
		void SmartMetEditingMode(CtrlViewUtils::FmiSmartMetEditingMode newValue, bool /* modifySettings */ ) override {itsSmartMetEditingMode = newValue;}
		int FilteringParameterUsageState(void) override {return itsFilteringParameterUsageState;} // arvot 0 (aktiivinen parametri),1 (kaikki parametrit) ja 2 (valitut parametrit)
		void FilteringParameterUsageState(int newValue) override {itsFilteringParameterUsageState = newValue;}
		boost::shared_ptr<NFmiDrawParam> ActiveDrawParam(unsigned int theDescTopIndex, int theRowIndex) override;
		int ActiveViewRow(unsigned int /* theDescTopIndex */ ) override {return 1;}
		NFmiParamBag& FilteringParamBag(void) override;
		const NFmiTimeDescriptor& EditedDataTimeDescriptor(void) override;
		const NFmiMetTime& ActiveViewTime(void) override;
		boost::shared_ptr<NFmiTimeDescriptor> CreateDataFilteringTimeDescriptor(boost::shared_ptr<NFmiFastQueryInfo> &theEditedData, bool fPasteClipBoardData = false) override;
		NFmiSmartToolInfo* SmartToolInfo(void) override;
		std::string& SmartToolEditingErrorText(void) override;
        NFmiGriddingHelperInterface* GetGriddingHelper(void) override { return 0; }
		void LastBrushedViewRow(int newRow) override;
		bool UseMasksWithFilterTool(void) override;
		int TestFilterUsedMask(void) override;
		NFmiGrid* ClipBoardData(void) override;
		int FilterFunction(void) override; // 0 = avg, 1 = max ja 2 = min filtterit
		const NFmiMetTime& TimeFilterStartTime(void) override;
		const NFmiMetTime& TimeFilterEndTime(void) override;
		const NFmiRect& AreaFilterRange(int index) override; // 1 = alku laatikko ja 2 on loppu laatikko
		bool UseTimeInterpolation(void) override;
		const NFmiPoint& TimeFilterRangeStart(void) override;
		const NFmiPoint& TimeFilterRangeEnd(void) override;
		bool IsSmoothTimeShiftPossible(void) override;
		int DataToDBCheckMethod(void) override; // 1=datavalidation, 2= niin kuin asetuksissa sanotaan
		NFmiDataLoadingInfo& GetUsedDataLoadingInfo(void) override;
		NFmiHelpEditorSystem& HelpEditorSystem(void) override;
		bool StoreData(const std::string& theFileName, boost::shared_ptr<NFmiFastQueryInfo> &theSmartInfo, bool askForSave) override;
		bool StoreData(bool fDoSaveTmpRename, const std::string& theFileName, NFmiQueryData *theData) override;
		NFmiHelpDataInfoSystem* HelpDataInfoSystem(void) override;
		bool DataLoadingOK(bool noError) override;
		void AddQueryData(NFmiQueryData* theData, const std::string& theDataFileName, const std::string& theDataFilePattern, NFmiInfoData::Type theType, const std::string& theNotificationStr, bool loadFromFileState = false) override;
		bool WarnIfCantSaveWorkingFile(void) override {return fWarnIfCantSaveWorkingFile;};
		NFmiHPlaceDescriptor* PossibleUsedDataLoadingGrid(void) override {return 0;} // oikeassa GenDoc-adapterissa palauta itsPossibleUsedDataLoadingGrid -dataosa!!!
		bool UseEditedDataParamDescriptor(void) override {return fUseEditedDataParamDescriptor;}
		NFmiParamDescriptor& EditedDataParamDescriptor(void) override;
		checkedVector<int>& DataLoadingProducerIndexVector(void) override {return itsDataLoadingProducerIndexVector;}  // oikeassa GenDoc-adapterissa palauta sen itsDataLoadingProducerIndexVector -dataosa!!!
		void EditedDataNeedsToBeLoaded(bool /* newValue */ ) override {}
		bool CheckEditedDataAfterAutoLoad(void) override {return true;}
		void PutWarningFlagTimerOn(void) override;
		NFmiModelDataBlender& ModelDataBlender(void) override;
		NFmiProducerIdLister& ProducerIdLister(void) override;
		void DoDataLoadingProblemsDlg(const std::string &theMessage) override;
		void DataModificationInProgress(bool /* newValue */ ) override {};
		bool DataModificationInProgress(void) override {return false;}
		const std::string& GetCurrentSmartToolMacro(void) override;
		NFmiMacroParamSystem& MacroParamSystem(void) override;
		void SetLatestMacroParamErrorText(const std::string& theErrorText) override;
		void SetMacroErrorText(const std::string &theErrorStr) override;
		boost::shared_ptr<NFmiArea> MapHandlerArea(bool fGetZoomedArea) override;
		FmiLanguage Language(void) override;
		float CPGriddingFactor(void) override {return 1;}
		const NFmiRect& CPGridCropRect(void) override {static NFmiRect dummy; return dummy;}
		bool UseCPGridCrop(void) override {return false;}
		const NFmiPoint& CPGridCropMargin(void) override {static NFmiPoint dummy; return dummy;}
        MultiProcessClientData& GetMultiProcessClientData(void) override;
        const std::string& GetSmartMetGuid(void) override {static std::string dummy("dummy-guid"); return dummy;}
        bool UseMultiProcessCpCalc(void) override {return false;}
        void MakeSureToolMasterPoolIsRunning(void) override {}
        bool IsWorkingDataSaved() override { return false; }
        void LogAndWarnUser(const std::string &theMessageStr, const std::string &theDialogTitleStr, CatLog::Severity severity, CatLog::Category category, bool justLog, bool addAbortOption = false) override;

	protected:
		void AddMask(FmiParameterName theParId, NFmiInfoData::Type theType, FmiMaskOperation maskOper, double limit1, double limit2 = 1);

		bool fCheckValidationFromSettings; // true -> tehdään asetuksien mukaiset validaatiot, false -> ei tehdä validaatioita ollenkaan
		boost::shared_ptr<NFmiAreaMaskList> itsParamMaskList;
		bool fUseMasksInTimeSerialViews;
		boost::shared_ptr<NFmiAnalyzeToolData> itsAnalyzeToolData;
		boost::shared_ptr<NFmiInfoOrganizer> itsInfoOrganizer;
		boost::shared_ptr<NFmiDrawParam> itsDrawParam;
		boost::shared_ptr<NFmiMetEditorOptionsData> itsMetEditorOptionsData;
		boost::shared_ptr<NFmiEditorControlPointManager> itsEditorControlPointManager;
		int itsTimeEditSmootherValue;
		CtrlViewUtils::FmiSmartMetEditingMode itsSmartMetEditingMode;
		int itsFilteringParameterUsageState;
		boost::shared_ptr<NFmiParamBag> itsFilteringParamBag;
		boost::shared_ptr<NFmiTimeDescriptor> itsEditedDataTimeDescriptor;
		boost::shared_ptr<NFmiMetTime> itsActiveViewTime;
		boost::shared_ptr<NFmiSmartToolInfo> itsSmartToolInfo;
		std::string itsSmartToolEditingErrorText;
		bool fUseMasksWithFilterTool;
		int itsTestFilterUsedMask;
		boost::shared_ptr<NFmiGrid> itsClipBoardData;
		int itsFilterFunction;
		NFmiRect itsAreaFilterStartRange;
		NFmiRect itsAreaFilterEndRange;
		bool fUseTimeInterpolation;
		NFmiPoint itsTimeFilterRangeStart;
		NFmiPoint itsTimeFilterRangeEnd;
		int itsDataToDBCheckMethod;
		boost::shared_ptr<NFmiDataLoadingInfo> itsDataLoadingInfo;
		boost::shared_ptr<NFmiHelpEditorSystem> itsHelpEditorSystem;
		boost::shared_ptr<NFmiHelpDataInfoSystem> itsHelpDataInfoSystem;
		bool fWarnIfCantSaveWorkingFile;
		bool fUseEditedDataParamDescriptor;
		boost::shared_ptr<NFmiParamDescriptor> itsEditedDataParamDescriptor;
		checkedVector<int> itsDataLoadingProducerIndexVector;
		boost::shared_ptr<NFmiModelDataBlender> itsModelDataBlender;
		boost::shared_ptr<NFmiProducerIdLister> itsProducerIdLister;
		boost::shared_ptr<NFmiMacroParamSystem> itsMacroParamSystem;
		boost::shared_ptr<NFmiArea> itsArea;
		FmiLanguage itsLanguage;
	};

	class ModifyFunctionParamHolder
	{
	public:
		ModifyFunctionParamHolder(TimeSerialModificationDataInterface &theAdapter)
		:itsAdapter(theAdapter)
		,itsModifiedDrawParam()
		,fUsedMask(NFmiMetEditorTypes::kFmiNoMask)
        ,itsEditorTool(NFmiMetEditorTypes::kFmiNoToolSelected)
		,fDoMultiThread(false)
		,itsThreadCallBacks(0)
		,fReturnValue(false)
		{}

        TimeSerialModificationDataInterface &itsAdapter;
		boost::shared_ptr<NFmiDrawParam> itsModifiedDrawParam;
		NFmiMetEditorTypes::Mask fUsedMask;
		NFmiMetEditorTypes::FmiUsedSmartMetTool itsEditorTool;
		bool fDoMultiThread;
		NFmiThreadCallBacks *itsThreadCallBacks;
		bool fReturnValue;

	private:
		ModifyFunctionParamHolder(const ModifyFunctionParamHolder &);
		ModifyFunctionParamHolder& operator=(const ModifyFunctionParamHolder &);
	};

    const std::string ForecasterIDKey = "ForecasterID";
    const std::string HelperForecasterIDKey = "HelperForecasterID";

	// Tätä funktiota VOI testata ModifyEditedDataExe-ohjelmalla
	bool DoTimeSerialModifications(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiDrawParam> &theModifiedDrawParam, NFmiMetEditorTypes::Mask fUsedMask, NFmiTimeDescriptor& theTimeDescriptor, std::vector<double> &theModificationFactorCurvePoints, NFmiMetEditorTypes::FmiUsedSmartMetTool theEditorTool, bool fUseSetForDiscreteData, int theUnchangedValue, bool fDoMultiThread, NFmiThreadCallBacks *theThreadCallBacks);
	void DoTimeSerialModifications2(ModifyFunctionParamHolder &theModifyFunctionParamHolder, NFmiTimeDescriptor& theTimeDescriptor, std::vector<double> &theModificationFactorCurvePoints, bool fUseSetForDiscreteData, int theUnchangedValue);
	// Tätä funktiota VOI testata ModifyEditedDataExe-ohjelmalla
	bool DoSmartToolEditing(TimeSerialModificationDataInterface &theAdapter, const std::string &theSmartToolText, const std::string &theRelativePathMacroName, bool fSelectedLocationsOnly, bool fDoMultiThread, NFmiThreadCallBacks *theThreadCallBacks);
	void DoSmartToolEditing2(ModifyFunctionParamHolder &theModifyFunctionParamHolder, const std::string &theSmartToolText, const std::string &theRelativePathMacroName, bool fSelectedLocationsOnly);
	// Tätä funktiota VOI testata ModifyEditedDataExe-ohjelmalla
	bool DoAreaFiltering(TimeSerialModificationDataInterface &theAdapter, bool fPasteClipBoardData, bool fDoMultiThread);
	// Tätä funktiota VOI testata ModifyEditedDataExe-ohjelmalla
	bool DoTimeFiltering(TimeSerialModificationDataInterface &theAdapter, bool fDoMultiThread);
	// Tätä funktiota EI VOI testata NFmiEditMapGeneralDataDoc-luokan ulkopuolella (ainakaan vielä)
	bool DoCombineModelAndKlapse(TimeSerialModificationDataInterface &theAdapter, bool fDoMultiThread);
	// Tätä funktiota EI VOI testata NFmiEditMapGeneralDataDoc-luokan ulkopuolella (ainakaan vielä)
	bool StoreDataToDataBase(TimeSerialModificationDataInterface &theAdapter, const std::string &theForecasterId, const std::string &theHelperForecasterId, bool fDoMultiThread);
	// Tätä funktiota EI VOI testata NFmiEditMapGeneralDataDoc-luokan ulkopuolella (ainakaan vielä)
	bool LoadData(TimeSerialModificationDataInterface &theAdapter, bool fRemoveThundersOnLoad, bool fDoMultiThread);
	// Tätä funktiota EI VOI testata NFmiEditMapGeneralDataDoc-luokan ulkopuolella (ainakaan vielä)
	float CalcMacroParamMatrix(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiDataMatrix<float> &theValues, bool fCalcTooltipValue, bool fDoMultiThread, const NFmiMetTime &theTime, const NFmiPoint &theTooltipLatlon, boost::shared_ptr<NFmiFastQueryInfo> &theUsedMacroInfoOut, bool &theUseCalculationPoints, boost::shared_ptr<NFmiFastQueryInfo> possibleSpacedOutMacroInfo = nullptr);
	// Tätä funktiota EI VOI testata NFmiEditMapGeneralDataDoc-luokan ulkopuolella (ainakaan vielä)
	bool TryAutoStartUpLoad(TimeSerialModificationDataInterface &theAdapter, bool fDoMultiThread);
	// Tätä funktiota VOI testata ModifyEditedDataExe-ohjelmalla
	bool CheckAndValidateAfterModifications(TimeSerialModificationDataInterface &theAdapter, NFmiMetEditorTypes::FmiUsedSmartMetTool theModifyingTool, bool fMakeDataSnapshotAction, unsigned int theLocationMask, FmiParameterName theParam, bool fDoMultiThread, bool fPasteAction);
	// Tätä funktiota EI VOI testata NFmiEditMapGeneralDataDoc-luokan ulkopuolella (ainakaan vielä)
	void SetActiveParamMissingValues(TimeSerialModificationDataInterface &theAdapter, double theValue, bool fDoMultiThread);
	// Tätä funktiota VOI testata ModifyEditedDataExe-ohjelmalla
	bool MakeDataValiditation(TimeSerialModificationDataInterface &theAdapter, bool fDoMultiThread);
	// Tätä funktiota VOI testata ModifyEditedDataExe-ohjelmalla
	void SnapShotData(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiDataIdent &theDataIdent, const std::string &theModificationText
					, const NFmiMetTime &theStartTime, const NFmiMetTime &theEndTime);
	bool UndoData(TimeSerialModificationDataInterface &theAdapter);
	bool RedoData(TimeSerialModificationDataInterface &theAdapter);
    std::string GetModifiedParamsForLog(NFmiParamBag & modifiedParams);
    std::string GetWantedSmartToolStr(NFmiMacroParamSystem &macroParamSystem, boost::shared_ptr<NFmiDrawParam> &theDrawParam);
    std::string DataFilterToolsParamsForLog(TimeSerialModificationDataInterface &theAdapter);
}
