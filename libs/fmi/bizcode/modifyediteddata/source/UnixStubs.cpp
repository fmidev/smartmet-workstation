// Unix stub implementations for functions defined in
// TimeSerialModification.cpp and NFmiDataParamModifier.cpp
// which cannot be compiled on Linux due to Windows SDK dependencies.

#ifdef UNIX

#include "TimeSerialModification.h"
#include "NFmiDataParamModifier.h"
#include "NFmiSmartToolModifier.h"
#include "NFmiExtraMacroParamData.h"

namespace FmiModifyEditdData
{

bool DoTimeSerialModifications(TimeSerialModificationDataInterface& /*theAdapter*/, std::shared_ptr<NFmiDrawParam>& /*theModifiedDrawParam*/, NFmiMetEditorTypes::Mask /*fUsedMask*/, NFmiTimeDescriptor& /*theTimeDescriptor*/, std::vector<float>& /*theModificationFactorCurvePoints*/, NFmiMetEditorTypes::FmiUsedSmartMetTool /*theEditorTool*/, bool /*fUseSetForDiscreteData*/, int /*theUnchangedValue*/, bool /*fDoMultiThread*/, NFmiThreadCallBacks* /*theThreadCallBacks*/)
{
    return false;
}

bool DoAreaFiltering(TimeSerialModificationDataInterface& /*theAdapter*/, bool /*fDoMultiThread*/)
{
    return false;
}

bool DoTimeFiltering(TimeSerialModificationDataInterface& /*theAdapter*/, bool /*fDoMultiThread*/)
{
    return false;
}

bool DoCombineModelAndKlapse(TimeSerialModificationDataInterface& /*theAdapter*/, bool /*fDoMultiThread*/)
{
    return false;
}

bool StoreDataToDataBase(TimeSerialModificationDataInterface& /*theAdapter*/, const std::string& /*theForecasterId*/, const std::string& /*theHelperForecasterId*/, bool /*fDoMultiThread*/)
{
    return false;
}

bool LoadData(TimeSerialModificationDataInterface& /*theAdapter*/, bool /*fRemoveThundersOnLoad*/, bool /*fDoMultiThread*/)
{
    return false;
}

float CalcMacroParamMatrix(TimeSerialModificationDataInterface& /*theAdapter*/, int /*theMapViewDescTopIndex*/, std::shared_ptr<NFmiDrawParam>& /*theDrawParam*/, NFmiDataMatrix<float>& /*theValues*/, bool /*fCalcTooltipValue*/, bool /*fDoMultiThread*/, const NFmiMetTime& /*theTime*/, const NFmiPoint& /*theTooltipLatlon*/, std::shared_ptr<NFmiFastQueryInfo>& /*theUsedMacroInfoOut*/, bool& /*theUseCalculationPoints*/, bool /*doProbing*/, const NFmiPoint& /*spaceOutSkipFactors*/, std::shared_ptr<NFmiFastQueryInfo> /*possibleSpacedOutMacroInfo*/, NFmiExtraMacroParamData* /*possibleExtraMacroParamData*/)
{
    return kFloatMissing;
}

bool TryAutoStartUpLoad(TimeSerialModificationDataInterface& /*theAdapter*/, bool /*fDoMultiThread*/)
{
    return false;
}

bool CheckAndValidateAfterModifications(TimeSerialModificationDataInterface& /*theAdapter*/, NFmiMetEditorTypes::FmiUsedSmartMetTool /*theModifyingTool*/, bool /*fMakeDataSnapshotAction*/, unsigned int /*theLocationMask*/, FmiParameterName /*theParam*/, bool /*fDoMultiThread*/)
{
    return false;
}

bool MakeDataValiditation(TimeSerialModificationDataInterface& /*theAdapter*/, bool /*fDoMultiThread*/)
{
    return false;
}

void SnapShotData(TimeSerialModificationDataInterface& /*theAdapter*/, std::shared_ptr<NFmiFastQueryInfo>& /*theInfo*/, const NFmiDataIdent& /*theDataIdent*/, const std::string& /*theModificationText*/, const NFmiMetTime& /*theStartTime*/, const NFmiMetTime& /*theEndTime*/)
{
}

bool UndoData(TimeSerialModificationDataInterface& /*theAdapter*/)
{
    return false;
}

bool RedoData(TimeSerialModificationDataInterface& /*theAdapter*/)
{
    return false;
}

bool SetupObsBlenderData(TimeSerialModificationDataInterface& /*theAdapter*/, const NFmiPoint& /*theLatlon*/, const NFmiParam& /*theParam*/, NFmiInfoData::Type /*theDataType*/, bool /*fGroundData*/, const NFmiProducer& /*theProducer*/, NFmiMetTime& /*firstEditedTimeOut*/, std::shared_ptr<NFmiFastQueryInfo>& /*usedObsBlenderInfoOut*/, float& /*analyzeValueOut*/, std::vector<std::string>& /*messagesOut*/)
{
    return false;
}

void InitializeSmartToolModifierForMacroParam(NFmiSmartToolModifier& /*theSmartToolModifier*/, TimeSerialModificationDataInterface& /*theAdapter*/, std::shared_ptr<NFmiDrawParam>& /*theDrawParam*/, int /*theMapViewDescTopIndex*/, std::shared_ptr<NFmiFastQueryInfo>& /*possibleSpacedOutMacroInfo*/, bool /*doProbing*/, const NFmiPoint& /*spaceOutSkipFactors*/)
{
}

} // namespace FmiModifyEditdData

// Stub for NFmiDataParamControlPointModifier::DoDataGridding
void NFmiDataParamControlPointModifier::DoDataGridding(std::vector<float>& /*xValues*/, std::vector<float>& /*yValues*/, std::vector<float>& /*zValues*/, int /*arraySize*/, NFmiDataMatrix<float>& /*gridData*/, const NFmiRect& /*theRelativeRect*/, const NFmiGriddingProperties& /*griddingProperties*/, NFmiObsDataGridding* /*theObsDataGridding*/, float /*theObservationRadiusRelative*/)
{
}

#endif // UNIX
