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
#include "NFmiProducer.h"

class NFmiThreadCallBacks;

namespace FmiModifyEditdData
{
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

	bool DoTimeSerialModifications(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiDrawParam> &theModifiedDrawParam, NFmiMetEditorTypes::Mask fUsedMask, NFmiTimeDescriptor& theTimeDescriptor, std::vector<double> &theModificationFactorCurvePoints, NFmiMetEditorTypes::FmiUsedSmartMetTool theEditorTool, bool fUseSetForDiscreteData, int theUnchangedValue, bool fDoMultiThread, NFmiThreadCallBacks *theThreadCallBacks);
	void DoTimeSerialModifications2(ModifyFunctionParamHolder &theModifyFunctionParamHolder, NFmiTimeDescriptor& theTimeDescriptor, std::vector<double> &theModificationFactorCurvePoints, bool fUseSetForDiscreteData, int theUnchangedValue);
	void DoSmartToolEditing2(ModifyFunctionParamHolder &theModifyFunctionParamHolder, const std::string &theSmartToolText, const std::string &theRelativePathMacroName, bool fSelectedLocationsOnly);
	bool DoAreaFiltering(TimeSerialModificationDataInterface &theAdapter, bool fPasteClipBoardData, bool fDoMultiThread);
	bool DoTimeFiltering(TimeSerialModificationDataInterface &theAdapter, bool fDoMultiThread);
	bool DoCombineModelAndKlapse(TimeSerialModificationDataInterface &theAdapter, bool fDoMultiThread);
	bool StoreDataToDataBase(TimeSerialModificationDataInterface &theAdapter, const std::string &theForecasterId, const std::string &theHelperForecasterId, bool fDoMultiThread);
	bool LoadData(TimeSerialModificationDataInterface &theAdapter, bool fRemoveThundersOnLoad, bool fDoMultiThread);
	float CalcMacroParamMatrix(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiDataMatrix<float> &theValues, bool fCalcTooltipValue, bool fDoMultiThread, const NFmiMetTime &theTime, const NFmiPoint &theTooltipLatlon, boost::shared_ptr<NFmiFastQueryInfo> &theUsedMacroInfoOut, bool &theUseCalculationPoints, boost::shared_ptr<NFmiFastQueryInfo> possibleSpacedOutMacroInfo = nullptr, std::string *possibleSymbolTooltipFile = nullptr);
	bool TryAutoStartUpLoad(TimeSerialModificationDataInterface &theAdapter, bool fDoMultiThread);
	bool CheckAndValidateAfterModifications(TimeSerialModificationDataInterface &theAdapter, NFmiMetEditorTypes::FmiUsedSmartMetTool theModifyingTool, bool fMakeDataSnapshotAction, unsigned int theLocationMask, FmiParameterName theParam, bool fDoMultiThread, bool fPasteAction);
	void SetActiveParamMissingValues(TimeSerialModificationDataInterface &theAdapter, double theValue, bool fDoMultiThread);
	bool MakeDataValiditation(TimeSerialModificationDataInterface &theAdapter, bool fDoMultiThread);
	void SnapShotData(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiDataIdent &theDataIdent, const std::string &theModificationText
					, const NFmiMetTime &theStartTime, const NFmiMetTime &theEndTime);
	bool UndoData(TimeSerialModificationDataInterface &theAdapter);
	bool RedoData(TimeSerialModificationDataInterface &theAdapter);
    std::string GetModifiedParamsForLog(NFmiParamBag & modifiedParams);
    std::string GetMacroParamFormula(NFmiMacroParamSystem &macroParamSystem, boost::shared_ptr<NFmiDrawParam> &theDrawParam);
    std::string DataFilterToolsParamsForLog(TimeSerialModificationDataInterface &theAdapter);
    checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> GetAnalyzeToolInfos(NFmiInfoOrganizer &infoOrganizer, const NFmiParam &theParam, NFmiInfoData::Type theType, bool fGroundData, int theProducerId, int theProducerId2 = -1);
    bool SetupObsBlenderData(TimeSerialModificationDataInterface &theAdapter, const NFmiPoint &theLatlon, const NFmiParam &theParam, NFmiInfoData::Type theDataType, bool fGroundData, const NFmiProducer &theProducer, NFmiMetTime &firstEditedTimeOut, boost::shared_ptr<NFmiFastQueryInfo> &usedObsBlenderInfoOut, float &analyzeValueOut, std::vector<std::string> &messagesOut);
}
