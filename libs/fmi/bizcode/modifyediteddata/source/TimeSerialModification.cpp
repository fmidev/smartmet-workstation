
#include "targetver.h"
#include "TimeSerialModification.h"
#include "NFmiDrawParam.h"
#include "NFmiAreaMaskList.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiAnalyzeToolData.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiSmartInfo.h"
#include "NFmiMetEditorOptionsData.h"
#include "NFmiEditorControlPointManager.h"
#include "NFmiDataParamModifier.h"
#include "NFmiDataModifierClasses.h"
#include "NFmiDataModifierMetEditClasses.h"
#include "NFmiSettings.h"
#include "NFmiFileSystem.h"
#include "NFmiQueryData.h"
#include "NFmiInfoAreaMask.h"
#include "NFmiSmartToolInfo.h"
#include "NFmiSmartToolModifier.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiDataLoadingInfo.h"
#include "NFmiHelpEditorSystem.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiProducerIdLister.h"
#include "NFmiProducerSystem.h"
#include "NFmiFileString.h"
#include "NFmiMilliSecondTimer.h"
#include "NFmiValueString.h"
#include "NFmiMetEditorModeDataWCTR.h"
#include "NFmiModelDataBlender.h"
#include "NFmiTotalWind.h"
#include "NFmiMacroParamSystem.h"
#include "NFmiMacroParam.h"
#include "NFmiTimeList.h"
#include "NFmiLatLonArea.h"
#include "MultiProcessClientData.h"
#include "NFmiExtraMacroParamData.h"
#include "CtrlViewFunctions.h"
#include "NFmiPathUtils.h"
#include "EditedInfoMaskHandler.h"
#include "catlog/catlog.h"
#include <fstream>
#include <newbase/NFmiEnumConverter.h>
#include "NFmiControlPointObservationBlender.h"
#include "ApplicationInterface.h"
#include "CtrlViewTimeConsumptionReporter.h"


#ifdef _MSC_VER
#pragma warning (disable : 4244 4267 4512) // boost:in thread kirjastosta tulee ik‰v‰sti 4244 varoituksia
#endif
#include <boost/thread.hpp>
#include "boost/math/special_functions/round.hpp"
#ifdef _MSC_VER
#pragma warning (default : 4244 4267 4512) // laitetaan 4244 takaisin p‰‰lle, koska se on t‰rke‰ (esim. double -> int auto castaus varoitus)
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4505) // warning C4505: 'func-name' : unreferenced local function has been removed
#endif

using namespace std::literals::string_literals;

using QueryDataVector = std::vector<std::unique_ptr<NFmiQueryData>>;
using FastInfoVector = std::vector<std::unique_ptr<NFmiFastQueryInfo>>;

static void LogMessage(TimeSerialModificationDataInterface &theAdapter, const std::string& theMessage, CatLog::Severity severity, CatLog::Category category)
{
    theAdapter.LogAndWarnUser(theMessage, "", severity, category, true);
}

// piti tehd‰ winkkarista irralliset messagebox button/toiminto definet (HUOM! FMI-etuliite)
#define FMI_MB_OK			0x00000000L
#define FMI_MB_ICONERROR	0x00000010L


// ***** Koodia kopsattu NFmiQueryInfo-luokasta *******
// Kopsasin koodia NFmiQueryInfo-luokasta, koska tavitsen syvemm‰n kopion NFmiFastInfo-luokasta.
// Eli jos kyseess‰ on NFmiSmartInfo-luokka, tavitsen siit‰ shallow-kopion (vain iteraattori osio ja 
// mm. erilaset maskit, mutta ei itse dataa, joka tulisi Clone-metodilla). Koska NFmiQueryInfo on 
// newbase-kirjastossa ja NFmiSmartInfo on smarttools-kirjastossa, kopsasin koodit t‰nne kuin ett‰ olisin
// siirt‰nyt n. 5-10 luokkaa smarttool-kirjastosta newbase:en.
// ***** Koodia kopsattu NFmiQueryInfo-luokasta *******

static boost::shared_ptr<NFmiFastQueryInfo> DoDynamicShallowCopy(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	if(theInfo)
	{
		NFmiSmartInfo *smartInfo = dynamic_cast<NFmiSmartInfo *>(theInfo.get());
		if(smartInfo)
			return boost::shared_ptr<NFmiFastQueryInfo>(new NFmiSmartInfo(*smartInfo)); // smartinfo kopio tarvitaan ett‰ editoidun datan locationMaskit tulevat otettua huomioon

		// HUOM! en kokeile/tee NFmiOwnerInfo-kopiota, koska siit‰ ei ole toiminnallista hyˆty‰

		return boost::shared_ptr<NFmiFastQueryInfo>(new NFmiFastQueryInfo(*theInfo)); // jos ei saatu smartinfoa, palautetaan fastInfo:n kopio
	}
	return boost::shared_ptr<NFmiFastQueryInfo>();
}

class TimeToModifyCalculator
{
public:
	typedef boost::shared_mutex MutexType;
	typedef boost::shared_lock<MutexType> ReadLock; // Read-lockia ei oikeasti tarvita, mutta laitan sen t‰h‰n, jos joskus tarvitaankin
	typedef boost::unique_lock<MutexType> WriteLock;

	TimeToModifyCalculator(const NFmiTimeDescriptor &theTimeDescriptor)
	:itsTimeDescriptor(theTimeDescriptor)
	,fNoMoreWork(false)
	{
		itsTimeDescriptor.First();
	}

#ifdef GetCurrentTime
#undef GetCurrentTime // t‰ll‰inen Windows macro pit‰‰ poistaa, muuten koodi ei k‰‰nny
#endif

	bool GetCurrentTime(NFmiMetTime &theTime)
	{
		WriteLock lock(itsMutex);
		if(fNoMoreWork)
			return false;
		theTime = itsTimeDescriptor.Time();
		if(itsTimeDescriptor.Next() == false)
			fNoMoreWork = true;
		return true;
	}

private:
	NFmiTimeDescriptor itsTimeDescriptor;
	MutexType itsMutex;
	bool fNoMoreWork;

	TimeToModifyCalculator(const TimeToModifyCalculator & ); // ei toteuteta kopio konstruktoria
};

static void ModifySingleTimeGridInThread(NFmiFastQueryInfo& theModifiedInfo,
    TimeToModifyCalculator& theTimeToModifyCalculator,
    NFmiDataModifier* theModifier)
{
    try
    {
        NFmiMetTime aTime;
        for(; theTimeToModifyCalculator.GetCurrentTime(aTime); )
        {
            if(theModifiedInfo.Time(aTime))
            {
                theModifier->SetTimeIndex(theModifiedInfo.TimeIndex());
                for(theModifiedInfo.ResetLocation(); theModifiedInfo.NextLocation(); )
                {
                    theModifier->SetLocationIndex(theModifiedInfo.LocationIndex());
                    theModifiedInfo.FloatValue(theModifier->FloatOperation(theModifiedInfo.FloatValue()));
                }
            }
        }
    }
    catch(std::exception& e)
    {
        std::string errorMessage = __FUNCTION__;
        errorMessage += ": ";
        errorMessage += e.what();
        CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Editing, true);
    }
}

static void ModifyTimesLocationData_FullMT(boost::shared_ptr<NFmiFastQueryInfo> &theModifiedData, NFmiDataModifier * theModifier, NFmiTimeDescriptor & theTimeDescriptor)
{
	unsigned int usedThreadCount = NFmiQueryDataUtil::GetReasonableWorkingThreadCount(75);
	unsigned long timeCount = theTimeDescriptor.Size();
	if(usedThreadCount > timeCount)
		usedThreadCount = timeCount;

	theModifiedData->LatLon(); // multi-thread koodin varmistus, ett‰ latlon-cachet on alustettu
	theModifier->InitLatlonCache();
	std::vector<boost::shared_ptr<NFmiFastQueryInfo> > modifiedInfoVector(usedThreadCount);
	std::vector<boost::shared_ptr<NFmiDataModifier> > dataModifierVector(usedThreadCount);
	for(unsigned int i = 0; i < usedThreadCount; i++)
	{
		// HUOM! pakko tehd‰ fastQueryInfo copioita, ett‰ ei menetet‰ nopeutta, eik‰ saa tehd‰ Clone:ja, koska t‰llˆin kopioituu myˆs data-osio!!!
		modifiedInfoVector[i] = ::DoDynamicShallowCopy(theModifiedData);
		dataModifierVector[i] = boost::shared_ptr<NFmiDataModifier>(theModifier->Clone());
	}


	TimeToModifyCalculator timeIndexCalculator(theTimeDescriptor);
	boost::thread_group calcParts;
	for(unsigned int i = 0; i < usedThreadCount; i++)
		calcParts.add_thread(new boost::thread(::ModifySingleTimeGridInThread, boost::ref(*modifiedInfoVector[i]), boost::ref(timeIndexCalculator), dataModifierVector[i].get()));
	calcParts.join_all(); // odotetaan ett‰ threadit lopettavat
}

// ****************************************************
// ***** Koodia kopsattu NFmiQueryInfo-luokasta *******
// ****************************************************



// Heitt‰‰ poikkeuksen, jos on tehty varoitus
static void PreventEditingIfProblemWithEditedData(TimeSerialModificationDataInterface &theAdapter)
{
	if(theAdapter.IsEditedDataInReadOnlyMode())
	{
		std::string msgStr("Current edited data is not suitable for editing or to be sent to the database.\n\n");
		msgStr += "SmartMet won't let you to do the editing you were going to do here now, nor later.\n\n";
		msgStr += "YOU SHOULD do the data loading properly:\n\n => Press Load Data -button 'after' pressing OK button here.";
		std::string dlgTitleStr("Current edited data not editable!");
		theAdapter.DoMessageBox(msgStr, dlgTitleStr, FMI_MB_OK | FMI_MB_ICONERROR);
		throw std::runtime_error("Cancelled editing action due not suitable data, data was not loaded correctly");
	}
}

// t‰ss‰ tehd‰‰n yhteiset perus toiminnot kaikille snapShot-funktioille
// Heitt‰‰ poikkeuksen, jos on tehty varoitus
static void SnapShotDataBaseAction(TimeSerialModificationDataInterface& theAdapter, boost::shared_ptr<NFmiFastQueryInfo>& theInfo, const std::string& theModificationText
	, const NFmiMetTime& theStartTime, const NFmiMetTime& theEndTime)
{
	::PreventEditingIfProblemWithEditedData(theAdapter);
	NFmiSmartInfo* smartInfo = dynamic_cast<NFmiSmartInfo*>(theInfo.get());
	if(smartInfo)
	{
		smartInfo->SnapShotData(theModificationText); // otetaan kuva datan nykytilasta
		smartInfo->Dirty(true);
		theAdapter.WindTableSystemMustaUpdateTable(true);
	}
	else
		throw std::runtime_error("Given edited data was not NFmiSmartInfo, internal SmartMet error.");
}

// funktio, joka hoitaa kaikkien parametrien likaamisen ja tallentaa nykytilan mahdollista undo/redo toimintoja varten:
// 1. ottaa valokuvan datan nykytilasta undo/redo toimintoja varten
// 2. asettaa editoidun (theInfo) datan likaiseksi,
// 5. asettaa editoitavan datan halutun parametrin likaiseksi
// Heitt‰‰ poikkeuksen, jos on tehty varoitus
static void SnapShotData(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const std::string &theModificationText
				 , const NFmiMetTime &theStartTime, const NFmiMetTime &theEndTime)
{
	::SnapShotDataBaseAction(theAdapter, theInfo, theModificationText, theStartTime, theEndTime);
}

// funktio, joka hoitaa kaikkien parametrien likaamisen ja tallentaa nykytilan mahdollista undo/redo toimintoja varten:
// 1. ottaa valokuvan datan nykytilasta undo/redo toimintoja varten
// 2. asettaa editoidun (theInfo) datan likaiseksi,
// 5. asettaa editoitavan datan halutun parametrin likaiseksi
// Heitt‰‰ poikkeuksen, jos on tehty varoitus
static void SnapShotData(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiParamBag &theParams, const std::string &theModificationText
				 , const NFmiMetTime &theStartTime, const NFmiMetTime &theEndTime)
{
	::SnapShotDataBaseAction(theAdapter, theInfo, theModificationText, theStartTime, theEndTime);
}

// funktio, joka hoitaa kaikkien parametrien likaamisen ja tallentaa nykytilan mahdollista undo/redo toimintoja varten:
// 1. ottaa valokuvan datan nykytilasta undo/redo toimintoja varten
// 2. asettaa editoidun (theInfo) datan likaiseksi,
// 5. asettaa editoitavan datan halutun parametrin likaiseksi
// Heitt‰‰ poikkeuksen, jos on tehty varoitus
static void SnapShotData(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiDataIdent &theDataIdent, const std::string &theModificationText
				 , const NFmiMetTime &theStartTime, const NFmiMetTime &theEndTime)
{
	::SnapShotDataBaseAction(theAdapter, theInfo, theModificationText, theStartTime, theEndTime);
}

static bool DoAnalyzeModificationsForParam(TimeSerialModificationDataInterface &theAdapter, const NFmiParam & theParam, boost::shared_ptr<NFmiFastQueryInfo> &theAnalyzeData, boost::shared_ptr<NFmiFastQueryInfo> &theModifiedData, NFmiTimeDescriptor& theTimes, boost::shared_ptr<NFmiAreaMaskList> &theMaskList)
{
    if(theParam.InterpolationMethod() != kLinearly)
        return false; // Tehd‰‰n vain lineaarisille parametreille muokkaus, en osaa muille laskea muutoksia oikein

	NFmiDataIdent dataIdent(theParam);
	boost::shared_ptr<NFmiDrawParam> drawParamForLimits = theAdapter.GetUsedDrawParamForEditedData(dataIdent);

	if(drawParamForLimits)
	{
        auto useMask = theMaskList->UseMask();
		NFmiLimitChecker limitChecker(static_cast<float>(drawParamForLimits->AbsoluteMinValue()), static_cast<float>(drawParamForLimits->AbsoluteMaxValue()), static_cast<FmiParameterName>(theParam.GetIdent()));
		auto timeSize = theTimes.Size();
		theAnalyzeData->LastTime(); // vain viimeinen aika kiinnostaa analyysist‰
		if(theAnalyzeData->Param(theParam) && theModifiedData->Param(theParam))
		{
            for(theAnalyzeData->ResetLevel(), theModifiedData->ResetLevel(); theAnalyzeData->NextLevel() && theModifiedData->NextLevel(); )
            {
                for(theModifiedData->ResetLocation(); theModifiedData->NextLocation(); )
                {
                    NFmiPoint latlon(theModifiedData->LatLon());
                    float analyzeValue = theAnalyzeData->InterpolatedValue(latlon);
                    if(theModifiedData->Time(theTimes.FirstTime()))
                    {
                        float firstEditDataValue = theModifiedData->FloatValue();
                        if(analyzeValue != kFloatMissing && firstEditDataValue != kFloatMissing)
                        {
                            float changeValue = firstEditDataValue - analyzeValue;
                            auto timeIndex = 0;
                            for(theTimes.Reset(); theTimes.Next(); timeIndex++)
                            {
                                if(theModifiedData->Time(theTimes.Time())) // vain editoitavaa dataa juoksutetaan ajassa (ajan pit‰isi lˆyty‰!)
                                {
                                    theMaskList->SyncronizeMaskTime(theTimes.Time());
                                    float maskFactor = 1.f;
                                    if(useMask)
                                        maskFactor = static_cast<float>(theMaskList->MaskValue(latlon));
                                    if(maskFactor)
                                    {
                                        float editDataValue = theModifiedData->FloatValue();
                                        if(editDataValue != kFloatMissing)
                                        {
                                            auto modifiedValue = NFmiControlPointObservationBlendingData::BlendData(editDataValue, changeValue, maskFactor, timeSize, timeIndex, limitChecker);
                                            theModifiedData->FloatValue(modifiedValue);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
			// kopioidaan viel‰ analyysi datan alku editoitavan datan alkuun, ett‰ tulee jatkuvaa dataa (ei hyppy‰ mallista analyysiin ja sitten liuku analyysista malliin)
            for( ; theAnalyzeData->PreviousTime(); )
			{
				if(theModifiedData->Time(theAnalyzeData->Time()))
				{
                    theMaskList->SyncronizeMaskTime(theModifiedData->Time());
                    for(theAnalyzeData->ResetLevel(), theModifiedData->ResetLevel(); theAnalyzeData->NextLevel() && theModifiedData->NextLevel(); )
					{
						for(theModifiedData->ResetLocation(); theModifiedData->NextLocation(); )
						{
							NFmiPoint latlon(theModifiedData->LatLon());
                            float maskFactor = 1.f;
                            if(useMask)
                                maskFactor = static_cast<float>(theMaskList->MaskValue(latlon));
                            if(maskFactor)
                            {
                                float value = limitChecker.CheckValue(theAnalyzeData->InterpolatedValue(latlon));
								if(value != kFloatMissing)
									theModifiedData->FloatValue(value);
							}
						}
					}
				}
			}
		}
	}
	return false;
}

static bool DoAnalyseModifications(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiFastQueryInfo> &theEditedInfo, boost::shared_ptr<NFmiFastQueryInfo> &theAnalyzeDataInfo, boost::shared_ptr<NFmiAreaMaskList> &theUsedMaskList, NFmiTimeDescriptor &theTimes, NFmiParam &theParam)
{
	return ::DoAnalyzeModificationsForParam(theAdapter, theParam, theAnalyzeDataInfo, theEditedInfo, theTimes, theUsedMaskList);
}

std::vector<boost::shared_ptr<NFmiFastQueryInfo>> FmiModifyEditdData::GetAnalyzeToolInfos(NFmiInfoOrganizer &infoOrganizer, const NFmiParam &theParam, NFmiInfoData::Type theType,
    bool fGroundData, int theProducerId, int theProducerId2)
{
    auto infoVector = infoOrganizer.GetInfos(theType, fGroundData, theProducerId, theProducerId2);
    std::vector<boost::shared_ptr<NFmiFastQueryInfo>> finalInfos;
    for(const auto &info : infoVector)
    {
        if(info->Param(theParam))
            finalInfos.push_back(info);
    }
    return finalInfos;
}

static NFmiTimeDescriptor GetAnalyzeToolModificationTimes(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiFastQueryInfo> &editedInfo, const NFmiMetTime &firstTime)
{
    NFmiTimeDescriptor times = editedInfo->TimeDescriptor();
    times = times.GetIntersection(firstTime, theAdapter.AnalyzeToolData().AnalyzeToolEndTime());
    return times;
}

static bool DoFinalAnalyzeToolModifications(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiFastQueryInfo> &editedInfo, boost::shared_ptr<NFmiFastQueryInfo> &analyzeInfo1, NFmiParam &theParam, NFmiMetEditorTypes::Mask fUsedMask, boost::shared_ptr<NFmiAreaMaskList> &maskList, NFmiTimeDescriptor &analyzeToolTimes, NFmiInfoData::Type dataType)
{
    bool status = ::DoAnalyseModifications(theAdapter, editedInfo, analyzeInfo1, maskList, analyzeToolTimes, theParam);
    if(theAdapter.AnalyzeToolData().UseBothProducers())
    {
        boost::shared_ptr<NFmiFastQueryInfo> analyzeInfo2 = theAdapter.InfoOrganizer()->Info(NFmiDataIdent(theParam, theAdapter.AnalyzeToolData().SelectedProducer2()), 0, dataType);
        if(analyzeInfo2)
        {
            dynamic_cast<NFmiSmartInfo*>(editedInfo.get())->InverseMask(fUsedMask);
            status = ::DoAnalyseModifications(theAdapter, editedInfo, analyzeInfo2, maskList, analyzeToolTimes, theParam);
            dynamic_cast<NFmiSmartInfo*>(editedInfo.get())->InverseMask(fUsedMask); // lopuksi pit‰‰ palauttaa alkuper‰inen maski (hoituu uudella inverse:ll‰)
        }
    }
    return status;
}

static bool DoFinalObservationBlenderToolModifications(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiFastQueryInfo> &editedInfo, std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &observationInfos, NFmiParam &theParam, NFmiMetEditorTypes::Mask fUsedMask, boost::shared_ptr<NFmiAreaMaskList> &maskList, NFmiTimeDescriptor &analyzeToolTimes, NFmiInfoData::Type dataType, const NFmiMetTime &actualFirstTime)
{
    auto drawParam = theAdapter.GetUsedDrawParamForEditedData(editedInfo->Param());
    NFmiControlPointObservationBlender dataModifier(editedInfo, drawParam, maskList, fUsedMask,
        theAdapter.CPManager(), theAdapter.CPGridCropRect(),
        theAdapter.UseCPGridCrop(), theAdapter.CPGridCropMargin(), observationInfos, actualFirstTime, theAdapter.GetGriddingHelper()->GriddingProperties(true));

    return dataModifier.ModifyTimeSeriesDataUsingMaskFactors(analyzeToolTimes, nullptr);
}

// 1. Mik‰ on editoidun datan 1. muokkausaika
// 2. Hae CP - pisteeseen sopivimmasta datasta sopivimpaan aikaan oleva arvo
// 3. Mik‰ on k‰ytetyn datan k‰ytetty location, jotta voidaan piirt‰‰ kyseinen k‰yr‰ sellaisenaan aikasarjaan
bool FmiModifyEditdData::SetupObsBlenderData(TimeSerialModificationDataInterface &theAdapter, const NFmiPoint &theLatlon, const NFmiParam &theParam, NFmiInfoData::Type theDataType, bool fGroundData, const NFmiProducer &theProducer, NFmiMetTime &firstEditedTimeOut, boost::shared_ptr<NFmiFastQueryInfo> &usedObsBlenderInfoOut, float &analyzeValueOut, std::vector<std::string> &messagesOut)
{
    auto infoOrganizer = theAdapter.InfoOrganizer();
    if(infoOrganizer)
    {
        auto editedInfo = theAdapter.EditedInfo();
        if(editedInfo)
        {
            if(editedInfo->Param(theParam))
            {
                auto obsBlenderInfos = FmiModifyEditdData::GetAnalyzeToolInfos(*infoOrganizer, theParam, theDataType, fGroundData, theProducer.GetIdent());
                if(!obsBlenderInfos.empty())
                {
                    auto usedAreaPtr = NFmiAnalyzeToolData::GetUsedAreaForAnalyzeTool(theAdapter, editedInfo);
                    NFmiMetTime actualFirstTime;
                    std::tie(actualFirstTime, firstEditedTimeOut) = NFmiAnalyzeToolData::GetLatestSuitableAnalyzeToolInfoTime(obsBlenderInfos, editedInfo, usedAreaPtr, true, std::string(theProducer.GetName()));
                    auto allowedTimes = NFmiControlPointObservationBlender::CalcAllowedObsBlenderTimes(actualFirstTime, firstEditedTimeOut, NFmiControlPointObservationBlendingData::ExpirationTimeInMinutes());
                    NFmiLocation cpLocation(theLatlon);
                    return NFmiControlPointObservationBlender::SeekClosestObsBlenderData(cpLocation, editedInfo, obsBlenderInfos, allowedTimes, analyzeValueOut, usedObsBlenderInfoOut);
                }
                else
                    messagesOut.push_back("No obs. data for Obs-blender");
            }
            else
                messagesOut.push_back("Edited data didn't have edited param");
        }
        else
            messagesOut.push_back("No Edited data?");
    }
    return false;
}

static bool DoAnalyzeToolRelatedModifications(bool useObservationBlenderTool, TimeSerialModificationDataInterface &theAdapter, NFmiParam &theParam, NFmiMetEditorTypes::Mask fUsedMask, const std::string &normalLogMessage, const NFmiProducer &producer, NFmiInfoData::Type dataType)
{
    boost::shared_ptr<NFmiFastQueryInfo> editedInfo = theAdapter.EditedInfo();
    if(editedInfo)
    {
        if(editedInfo->Param(theParam))
        {
            auto maskList = NFmiAnalyzeToolData::GetUsedTimeSerialMaskList(theAdapter);
            NFmiInfoOrganizer *infoOrganizer = theAdapter.InfoOrganizer();
            if(infoOrganizer)
            {
                auto analyzeToolInfos = FmiModifyEditdData::GetAnalyzeToolInfos(*infoOrganizer, theParam, dataType, true, producer.GetIdent());
                if(!analyzeToolInfos.empty())
                {
                    auto usedAreaPtr = NFmiAnalyzeToolData::GetUsedAreaForAnalyzeTool(theAdapter, editedInfo);
                    NFmiMetTime actualFirstTime, firstTime;
                    std::tie(actualFirstTime, firstTime) = NFmiAnalyzeToolData::GetLatestSuitableAnalyzeToolInfoTime(analyzeToolInfos, editedInfo, usedAreaPtr, useObservationBlenderTool, std::string(producer.GetName()));
                    if(theAdapter.AnalyzeToolData().AnalyzeToolEndTime() > firstTime)
                    {
                        auto times = ::GetAnalyzeToolModificationTimes(theAdapter, editedInfo, firstTime);
                        try
                        {
                            ::SnapShotData(theAdapter, editedInfo, editedInfo->Param(), normalLogMessage, times.FirstTime(), times.LastTime());
                        }
                        catch(...)
                        {
                            // heitetty poikkeus eli halutaan lopettaa toiminto
                            ::LogMessage(theAdapter, "Unknown error occured while trying to use Analyze tools related editing", CatLog::Severity::Error, CatLog::Category::Editing);
                            return false;
                        }

                        EditedInfoMaskHandler editedInfoMaskHandler(editedInfo, fUsedMask);
                        ::LogMessage(theAdapter, normalLogMessage, CatLog::Severity::Info, CatLog::Category::Editing);
                        if(useObservationBlenderTool)
                            return ::DoFinalObservationBlenderToolModifications(theAdapter, editedInfo, analyzeToolInfos, theParam, fUsedMask, maskList, times, dataType, actualFirstTime);
                        else
                            return ::DoFinalAnalyzeToolModifications(theAdapter, editedInfo, analyzeToolInfos[0], theParam, fUsedMask, maskList, times, dataType);
                    }
                    else
                        ::LogMessage(theAdapter, "Analyze tool's end time was greater than calculated start time for analyze related editing", CatLog::Severity::Error, CatLog::Category::Editing);
                }
                else
                    ::LogMessage(theAdapter, "Couldn't find any data while trying to use Analyze tools related editing", CatLog::Severity::Error, CatLog::Category::Editing);
            }
        }
        else
            ::LogMessage(theAdapter, std::string("Edited data didn't contain param '") + theParam.GetName().CharPtr() + "'", CatLog::Severity::Error, CatLog::Category::Editing);
    }
    else
        ::LogMessage(theAdapter, std::string("No edited data available?"), CatLog::Severity::Error, CatLog::Category::Editing);
    return false;
}

static void DoTimeSeriesValuesModifyingWithCPs(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiDrawParam> &theModifiedDrawParam, NFmiMetEditorTypes::Mask fUsedMask, NFmiTimeDescriptor& theTimeDescriptor, boost::shared_ptr<NFmiAreaMaskList> &theMaskList, NFmiThreadCallBacks *theThreadCallBacks)
{
	// HUOM!! muokkaukset pit‰‰ tehd‰ kaikille pisteille!!!
	if(theModifiedDrawParam)
	{
		NFmiInfoOrganizer *infoOrganizer = theAdapter.InfoOrganizer();
		if(infoOrganizer)
		{
			boost::shared_ptr<NFmiFastQueryInfo> info = infoOrganizer->Info(theModifiedDrawParam, false, false);
			boost::shared_ptr<NFmiFastQueryInfo> fastInfo = NFmiAreaMask::DoShallowCopy(info);
			if(fastInfo)
			{
				theAdapter.CPManager()->Param(theModifiedDrawParam->Param());

				NFmiDataParamControlPointModifier dataModifier(fastInfo, theModifiedDrawParam, theMaskList, fUsedMask, 
															theAdapter.CPManager(), theAdapter.CPGridCropRect(), 
                                                            theAdapter.UseCPGridCrop(), theAdapter.CPGridCropMargin(), theAdapter.GetGriddingHelper()->GriddingProperties(true));

                if(theAdapter.UseMultiProcessCpCalc())
                {
                    if(theAdapter.MakeSureToolMasterPoolIsRunning())
                    {
                        try
                        {
                            dataModifier.DoProcessPoolCpModifyingTcp(theAdapter.GetMultiProcessClientData(), theTimeDescriptor, theAdapter.GetSmartMetGuid(), theThreadCallBacks);
                        }
                        catch(std::exception &e)
                        {
                            CatLog::logMessage(std::string("Error with MP-CP editing: ") + e.what(), CatLog::Severity::Error, CatLog::Category::Editing);
                        }
                    }
                    else
                    {
                        CatLog::logMessage(std::string("Error with MP-CP editing: Unable to start distributed calculation system"), CatLog::Severity::Error, CatLog::Category::Editing);
                    }
                }
                else
				    dataModifier.ModifyTimeSeriesDataUsingMaskFactors(theTimeDescriptor, theThreadCallBacks);
			}
		}
	}
}

static bool MustMakeValidationCheckAfterModifyingThisParam(TimeSerialModificationDataInterface &theAdapter, FmiParameterName theParam)
{
	if(theAdapter.MetEditorOptionsData().UseDataValiditation_PrForm_T())
	{
		switch (theParam)
		{
		case kFmiTemperature: // T-DP tarkasteluissa
		case kFmiPrecipitation1h:
#ifdef USE_POTENTIAL_VALUES_IN_EDITING
        case kFmiPotentialPrecipitationForm:
#else
        case kFmiPrecipitationForm:
#endif
			return true;
		}
	}

	if(theAdapter.MetEditorOptionsData().UseDataValiditation_T_DP())
	{
		switch (theParam)
		{
		case kFmiTemperature:
		case kFmiDewPoint:
		case kFmiHumidity:
			return true;
		}
	}

	return false;
}

static bool MustMakeValidationCheckAfterModifyingTheseParams(TimeSerialModificationDataInterface &theAdapter, NFmiParamBag& theParams, bool fSeeOnlyActiveParams)
{
	for(theParams.Reset(); theParams.Next(); )
	{
		if(!theParams.Current()->HasDataParams())
		{
			if(!fSeeOnlyActiveParams || theParams.Current()->IsActive())
			{
				if(::MustMakeValidationCheckAfterModifyingThisParam(theAdapter, FmiParameterName(theParams.Current()->GetParam()->GetIdent())))
					return true;
			}
		}
		else // k‰yd‰‰n aliparametritkin l‰pi
		{
			NFmiParamBag subParamBag(*theParams.Current()->GetDataParams());
			for(subParamBag.Reset(); subParamBag.Next(); )
			{
				if(!fSeeOnlyActiveParams || subParamBag.Current()->IsActive())
				{
					if(subParamBag.Current()->Type() != kSymbolicParam)
					{
						if(::MustMakeValidationCheckAfterModifyingThisParam(theAdapter, FmiParameterName(subParamBag.Current()->GetParam()->GetIdent())))
							return true;
					}
				}
			}
		}
	}
	return false;
}

static bool MakeDataValiditation_PrForm_T(TimeSerialModificationDataInterface &theAdapter, NFmiTimeDescriptor* theTimeDescriptor, unsigned int theLocationMask, bool fDoMultiThread)
{
	if(theAdapter.MetEditorOptionsData().SnowTemperatureLimit() != kFloatMissing && theAdapter.MetEditorOptionsData().RainTemperatureLimit() != kFloatMissing)
	{
		boost::shared_ptr<NFmiFastQueryInfo> editedData = theAdapter.EditedInfo();
		if(editedData)
		{
			boost::shared_ptr<NFmiFastQueryInfo> prFormInfo(new NFmiFastQueryInfo(*editedData));
#ifdef USE_POTENTIAL_VALUES_IN_EDITING
            if(!prFormInfo->Param(kFmiPotentialPrecipitationForm))
#else
            if(!prFormInfo->Param(kFmiPrecipitationForm))
#endif
				return false;
			boost::shared_ptr<NFmiFastQueryInfo> temperatureInfo(new NFmiFastQueryInfo(*editedData));
			if(!temperatureInfo->Param(kFmiTemperature))
				return false;
			prFormInfo->MaskType(theLocationMask);
			temperatureInfo->MaskType(theLocationMask);

			NFmiDataModifierValidateData_PrForm_T modifier(temperatureInfo, theAdapter.MetEditorOptionsData().SnowTemperatureLimit(), theAdapter.MetEditorOptionsData().RainTemperatureLimit());
			if(fDoMultiThread)
				::ModifyTimesLocationData_FullMT(prFormInfo, &modifier, *theTimeDescriptor);
			else
				prFormInfo->ModifyTimesLocationData(&modifier, temperatureInfo.get(), *theTimeDescriptor);
			return true;
		}
	}
	return false;
}

static bool MakeDataValiditation_T_DP(TimeSerialModificationDataInterface &theAdapter, NFmiTimeDescriptor* theTimeDescriptor, unsigned int theLocationMask, bool fDoMultiThread)
{
	boost::shared_ptr<NFmiFastQueryInfo> editedData = theAdapter.EditedInfo();
	if(editedData)
	{
		boost::shared_ptr<NFmiFastQueryInfo> DPInfo(new NFmiFastQueryInfo(*editedData));
		if(!DPInfo->Param(kFmiDewPoint))
			return false;
		boost::shared_ptr<NFmiFastQueryInfo> temperatureInfo(new NFmiFastQueryInfo(*editedData));
		if(!temperatureInfo->Param(kFmiTemperature))
			return false;
		DPInfo->MaskType(theLocationMask);
		temperatureInfo->MaskType(theLocationMask);
		NFmiDataModifierValidateData_T_DP_ver2 modifier(temperatureInfo);
		if(fDoMultiThread)
			::ModifyTimesLocationData_FullMT(DPInfo, &modifier, *theTimeDescriptor);
		else
			DPInfo->ModifyTimesLocationData(&modifier, temperatureInfo.get(), *theTimeDescriptor);

		return true;
	}
	return false;
}

static bool MakeDataValiditation(TimeSerialModificationDataInterface &theAdapter, NFmiTimeDescriptor* theTimeDescriptor, bool fMakeDataSnapshotAction, unsigned int theLocationMask, bool fDoMultiThread)
{
	boost::shared_ptr<NFmiFastQueryInfo> editedData = theAdapter.EditedInfo();
	if(editedData)
	{
		if(fMakeDataSnapshotAction)
		{
			try
			{
				::SnapShotData(theAdapter, editedData, editedData->Param(), "Data validation", editedData->TimeDescriptor().FirstTime(), editedData->TimeDescriptor().LastTime());
			}
			catch(...)
			{
				// heitetty poikkeus eli halutaan lopettaa toiminto
				return false;
			}
		}
		bool status = true;
		if(theAdapter.MetEditorOptionsData().UseDataValiditation_PrForm_T())
			status &= ::MakeDataValiditation_PrForm_T(theAdapter, theTimeDescriptor, theLocationMask, fDoMultiThread);

		if(theAdapter.MetEditorOptionsData().UseDataValiditation_T_DP())
			status &= ::MakeDataValiditation_T_DP(theAdapter, theTimeDescriptor, theLocationMask, fDoMultiThread);
		return status;
	}
	return false;
}

static void MakeBasicViewUpdatePreparationsAfterDataModifications(TimeSerialModificationDataInterface& theAdapter, bool clearEditedDataRelatedCaches = true)
{
    // 1. Kaikkien karttan‰yttˆjen image cachet tyhjiksi, pakotetaan piirto ja editoituun dataan riippuvaisten cachejen (macroParam) tyhjennys
    theAdapter.MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, true, true, false, clearEditedDataRelatedCaches, false);
    // 2. Optimoidaan smartmetin n‰yttˆjen p‰ivityst‰, vain ne n‰ytˆt pit‰‰ p‰ivitt‰‰, jotka voivat k‰ytt‰‰ editoitua dataa
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(::GetUpdatedViewIdMaskForEditingData());
}

static bool MakeDataValiditation(TimeSerialModificationDataInterface &theAdapter, bool fDoMultiThread)
{
	boost::shared_ptr<NFmiFastQueryInfo> editedData = theAdapter.EditedInfo();
	if(editedData)
	{
        ::MakeBasicViewUpdatePreparationsAfterDataModifications(theAdapter);
		NFmiTimeDescriptor timeDescriptor(theAdapter.EditedDataTimeDescriptor());
		return ::MakeDataValiditation(theAdapter, &timeDescriptor, true, 1, fDoMultiThread);
	}
	return false;
}

// t‰t‰ metodia voidaan k‰ytt‰‰ aina kaikkialla kun tehd‰‰n muokkauksia dataan
// theModifyingTool 1 = muokkausdialogi, 2 = pensseli ja 3 = aikasarjaeditori
// kun tyˆkalu on 2 tai 3, annetaan mukana myˆs editoitava parametri
static bool CheckAndValidateAfterModifications(TimeSerialModificationDataInterface &theAdapter, NFmiMetEditorTypes::FmiUsedSmartMetTool theModifyingTool, bool fMakeDataSnapshotAction, unsigned int theLocationMask, FmiParameterName theParam, bool fDoMultiThread, NFmiParamBag *theModifiedParamBag = 0)
{
	if(theAdapter.CheckValidationFromSettings() == false)
		return false;

	if(theAdapter.SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ns. view-moodi p‰‰ll‰, ei tehd‰ mit‰‰n
		return false;

	boost::shared_ptr<NFmiFastQueryInfo> editedInfo = theAdapter.EditedInfo();
	if(editedInfo)
	{
		if(theAdapter.MetEditorOptionsData().MakeDataValiditationAutomatic())
		{
			bool paramStatus = false;
			switch (theModifyingTool)
			{
            case NFmiMetEditorTypes::kFmiDataModificationTool:
				{ // muokkaustyˆkalulla on kolme moodia, miss‰ voidaan tehd‰ muutoksia joko aktiiviseen parametriin, valittuun joukkoon parametreista tai kaikkiin parametreihen kerrallaan
					switch(theAdapter.FilteringParameterUsageState())
					{
					case 0: // 0=tehd‰‰n operaatio vain aktiiviselle parametrille
						{
							boost::shared_ptr<NFmiDrawParam> drawParam = theAdapter.ActiveDrawParamFromActiveRow(0); // HUOM! muokkausta voi tehd‰ vain 1. (eli indeksi 0) desctopist‰
							if(drawParam)
								paramStatus = ::MustMakeValidationCheckAfterModifyingThisParam(theAdapter, FmiParameterName(drawParam->Param().GetParam()->GetIdent()));
						}
						break;
					case 1: // 1=tehd‰‰n operaatio kaikille parametreille
						paramStatus = ::MustMakeValidationCheckAfterModifyingTheseParams(theAdapter, theAdapter.FilteringParamBag(), false);
						break;
					case 2: // 2=tehd‰‰n operaatio valituille parametreille
						paramStatus = ::MustMakeValidationCheckAfterModifyingTheseParams(theAdapter, theAdapter.FilteringParamBag(), true);
						break;
					}
				}
				break;
                // N‰ill‰ tyˆkaluilla tehd‰‰n muutoksia vain yhdelle parametrille kerrallaan
            case NFmiMetEditorTypes::kFmiBrush:
            case NFmiMetEditorTypes::kFmiTimeSerialModification:
				paramStatus = ::MustMakeValidationCheckAfterModifyingThisParam(theAdapter, theParam);
				break;
                // Smarttoolilla voidaan tehd‰ muutoksia kaikille parametreille yhdell‰ kertaa
            case NFmiMetEditorTypes::kFmiSmarttool:
                if(theModifiedParamBag) // pit‰isi kait heitt‰‰ poikkeus jos nullptr
    				paramStatus = ::MustMakeValidationCheckAfterModifyingTheseParams(theAdapter, *theModifiedParamBag, false);
				break;
                // Datan lataus tekee muutoksia kaikille parametreille yhdell‰ kertaa
            case NFmiMetEditorTypes::kFmiDataLoading:
				paramStatus = ::MustMakeValidationCheckAfterModifyingTheseParams(theAdapter, theAdapter.FilteringParamBag(), false);
				break;
			}

			if(paramStatus)
			{
                boost::shared_ptr<NFmiTimeDescriptor> timeDesc;
				switch (theModifyingTool)
				{
                case NFmiMetEditorTypes::kFmiDataModificationTool:
					timeDesc = theAdapter.CreateDataFilteringTimeDescriptor(editedInfo);
					break;
                case NFmiMetEditorTypes::kFmiBrush:
					timeDesc = boost::shared_ptr<NFmiTimeDescriptor>(new NFmiTimeDescriptor(theAdapter.EditedDataTimeDescriptor().OriginTime(), NFmiTimeBag(theAdapter.ActiveViewTime(), theAdapter.ActiveViewTime(), editedInfo->TimeResolution())));
					break;
                case NFmiMetEditorTypes::kFmiTimeSerialModification:
						timeDesc = boost::shared_ptr<NFmiTimeDescriptor>(new NFmiTimeDescriptor(theAdapter.EditedDataTimeDescriptor()));
					break;
                case NFmiMetEditorTypes::kFmiSmarttool:
						timeDesc = theAdapter.CreateDataFilteringTimeDescriptor(editedInfo);
					break;
                case NFmiMetEditorTypes::kFmiDataLoading:
						timeDesc = boost::shared_ptr<NFmiTimeDescriptor>(new NFmiTimeDescriptor(theAdapter.EditedDataTimeDescriptor()));
					break;
				}

                bool status = ::MakeDataValiditation(theAdapter, timeDesc.get(), fMakeDataSnapshotAction, theLocationMask, fDoMultiThread);
				return status;
			}
		}
	}

	return false;
}

std::string TimeSeriesModifiedParamForLog(boost::shared_ptr<NFmiDrawParam> &theModifiedDrawParam)
{
    //std::string paramName = theModifiedDrawParam->Param().GetParamName();
    std::string paramName = theModifiedDrawParam->ParameterAbbreviation();
    return "[" + paramName + "]";
}

static bool DoTimeSeriesValuesModifying(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiDrawParam> &theModifiedDrawParam, NFmiMetEditorTypes::Mask fUsedMask, NFmiTimeDescriptor& theTimeDescriptor, std::vector<float> &theModificationFactorCurvePoints, NFmiMetEditorTypes::FmiUsedSmartMetTool theEditorTool, bool fUseSetForDiscreteData, int theUnchangedValue, bool fDoMultiThread, NFmiThreadCallBacks *theThreadCallBacks)
{
	if(theModifiedDrawParam && theModifiedDrawParam->IsParamEdited())
	{
		auto& modifiedParam = *theModifiedDrawParam->Param().GetParam();
        if(theAdapter.AnalyzeToolData().ControlPointObservationBlendingData().UseBlendingTool()) // haaraudutaan, jos on observation-blender tyˆkalu k‰ytˆss‰!
            ::DoAnalyzeToolRelatedModifications(true, theAdapter, modifiedParam, fUsedMask, "Using Observation-blender tool to modify edited data.", theAdapter.AnalyzeToolData().ControlPointObservationBlendingData().SelectedProducer(), NFmiInfoData::kObservations);
        else if(theAdapter.AnalyzeToolData().AnalyzeToolMode()) // haaraudutaan, jos on analyysi tyˆkalu k‰ytˆss‰!
			::DoAnalyzeToolRelatedModifications(false, theAdapter, modifiedParam, fUsedMask, "Using Analyze tool to modify edited data.", theAdapter.AnalyzeToolData().SelectedProducer1(), NFmiInfoData::kAnalyzeData);
		else
		{
			NFmiInfoOrganizer *infoOrganizer = theAdapter.InfoOrganizer();
            std::string paramName = modifiedParam.GetName();
			if(infoOrganizer)
			{
				boost::shared_ptr<NFmiFastQueryInfo> info = infoOrganizer->Info(theModifiedDrawParam, false, false);
				if(!info)
				{
                    ::LogMessage(theAdapter, "Trying to do Time Serial modifications, but there is no edited data.", CatLog::Severity::Warning, CatLog::Category::Editing);
					return false;
				}

				NFmiMetTime dummyTime;
				try
				{
					::SnapShotData(theAdapter, info, theModifiedDrawParam->Param(), "Time Serial modifications", dummyTime, dummyTime);
				}
				catch(...)
				{
					// heitetty poikkeus eli halutaan lopettaa toiminto
					return false;
				}

                auto maskList = NFmiAnalyzeToolData::GetUsedTimeSerialMaskList(theAdapter);
				if(theAdapter.MetEditorOptionsData().ControlPointMode())
				{
                    ::LogMessage(theAdapter, TimeSeriesModifiedParamForLog(theModifiedDrawParam) + " - modified with Control Point tool.", CatLog::Severity::Info, CatLog::Category::Editing);
					::DoTimeSeriesValuesModifyingWithCPs(theAdapter, theModifiedDrawParam, NFmiMetEditorTypes::kFmiNoMask, theTimeDescriptor, maskList, theThreadCallBacks);
				}
				else
				{
                    ::LogMessage(theAdapter, TimeSeriesModifiedParamForLog(theModifiedDrawParam) + " - modified with Time Serial tool.", CatLog::Severity::Info, CatLog::Category::Editing);
					NFmiDataParamModifier dataModifier(info, theModifiedDrawParam, maskList, fUsedMask);
					if(fUseSetForDiscreteData)
						dataModifier.SetTimeSeriesData(theTimeDescriptor, &theModificationFactorCurvePoints[0], theUnchangedValue);
					else
						dataModifier.ModifyTimeSeriesDataUsingMaskFactors(theTimeDescriptor, &theModificationFactorCurvePoints[0]);
				}
			}
		}
		::CheckAndValidateAfterModifications(theAdapter, theEditorTool, false, fUsedMask, FmiParameterName(modifiedParam.GetIdent()), fDoMultiThread, false);
        ::MakeBasicViewUpdatePreparationsAfterDataModifications(theAdapter);

		return true;
	}
	return false;
}

std::string ModifiedTimesForLog(TimeSerialModificationDataInterface &theAdapter)
{
    boost::shared_ptr<NFmiFastQueryInfo> editedData = theAdapter.EditedInfo();
    boost::shared_ptr<NFmiTimeDescriptor> modifiedTimes = theAdapter.CreateDataFilteringTimeDescriptor(editedData);
    auto firstTime = modifiedTimes->FirstTime().ToStr("DD.MM. HH:mm");
    auto lastTime = modifiedTimes->LastTime().ToStr("DD.MM. HH:mm");
    std::string modifiedTimespan = (firstTime == lastTime) ? (" " + firstTime) : (" " + firstTime + " - " + lastTime);
    return modifiedTimespan;
}

void LogSmartToolModifications(TimeSerialModificationDataInterface &theAdapter, NFmiParamBag &modifiedParams, const std::string &theLogMessage, bool showLoadedSmartTool = false)
{
    std::string editedParams = FmiModifyEditdData::GetModifiedParamsForLog(modifiedParams);
    std::string loadedSmartTool = (showLoadedSmartTool) ? " (" + theAdapter.SmartToolInfo()->LoadDirectory() + theAdapter.SmartToolInfo()->CurrentScriptName() + ")" : "";

    ::LogMessage(theAdapter, editedParams + ModifiedTimesForLog(theAdapter) + theLogMessage + loadedSmartTool, CatLog::Severity::Info, CatLog::Category::Editing);
}

void TraceLogUsedSmartToolThreadCount(TimeSerialModificationDataInterface& theAdapter, const std::set<int> &usedThreadCounts)
{
	std::string logMessage;
	auto multiValueThreadcount = (usedThreadCounts.size() > 1);
	if(multiValueThreadcount)
	{
		logMessage = "Used working thread counts for this smarttools calculations were ";
		logMessage += std::to_string(*usedThreadCounts.begin());
		logMessage += "-";
		// set's rbegin points to last element, but not past it...
		logMessage += std::to_string(*usedThreadCounts.rbegin());
	}
	else
	{
		logMessage = "Used working thread count for this smarttools calculation was ";
		logMessage += std::to_string(*usedThreadCounts.begin());
	}
	::LogMessage(theAdapter, logMessage, CatLog::Severity::Trace, CatLog::Category::Editing);
}

bool DoSmartToolEditing(TimeSerialModificationDataInterface &theAdapter, const std::string &theSmartToolText, const std::string &theRelativePathMacroName, bool fSelectedLocationsOnly, NFmiTimeDescriptor &theTimes,
						const std::string &theLogMessage, const std::string &theIncludeDir, bool fDoDataValidations, bool fDoMultiThread, NFmiThreadCallBacks *theThreadCallBacks, 
                        bool showLoadedSmartTool = false)
{
	theAdapter.SmartToolEditingErrorText().clear();
	boost::shared_ptr<NFmiFastQueryInfo> editedData = theAdapter.EditedInfo();
	if(editedData)
	{
		// HUOM! SmartToolin kanssa ei aseteta meteorologisia,
		// koska ne sis‰lt‰v‰t meteorologiset maskit itse.

		NFmiSmartToolModifier smartToolModifier(theAdapter.InfoOrganizer());
		try // ensin tulkitaan macro
		{
			smartToolModifier.SetGriddingHelper(theAdapter.GetGriddingHelper());
			smartToolModifier.IncludeDirectory(theIncludeDir);
			smartToolModifier.InitSmartTool(theSmartToolText);
		}
		catch(std::exception &e)
		{
            std::string errorString = e.what();
            errorString += ", in macro '";
            errorString += theRelativePathMacroName;
            errorString += "'";
			theAdapter.SmartToolEditingErrorText() = errorString;
            ::LogMessage(theAdapter, std::string("SmartTool-macro interpretation failed: ") + errorString, CatLog::Severity::Error, CatLog::Category::Editing);
			return false;
		}

		// Metkun editorin SmartTool-dialogissa jos painaa "Suorita makro"-nappia, tehd‰‰n
		// ruudulla olevan skriptin mukaiset muutokset dataan. MUTTA jos kyseess‰ on
		// ns. macroParam eli skriptiss‰ on lausekkeit‰ jossa sijoitetaan RETURN muuttujaan,
		// t‰llˆin ei haluta ajaa makro muutoksia, koska tarkoitus on vain katsella
		// macroParam:ia karttan‰ytˆll‰.
		// T‰nne asti tullaan ett‰ saadaan virhe ilmoitus tulkinta vaiheessa talteen.
		// Mutta suoritus vaiheen virheet menev‰t t‰ll‰ hetkell‰ vain loki tiedostoon.
		if(smartToolModifier.IsInterpretedSkriptMacroParam())
		{
            ::MakeBasicViewUpdatePreparationsAfterDataModifications(theAdapter, false);
            return true;
		}

        NFmiParamBag modifiedParams = smartToolModifier.ModifiedParams();
		{
			try // suoritetaan macro sitten
			{
				try
				{
					::SnapShotData(theAdapter, editedData, modifiedParams, "SmartTool modifications", theTimes.FirstTime(), theTimes.LastTime());
				}
				catch(...)
				{
					// heitetty poikkeus eli halutaan lopettaa toiminto
					return false;
				}

				if(fDoMultiThread)
				{
					smartToolModifier.ModifyData_ver2(&theTimes, fSelectedLocationsOnly, false, theThreadCallBacks);
				}
				else
				{
					smartToolModifier.ModifyData(&theTimes, fSelectedLocationsOnly, false, theThreadCallBacks);
				}

				TraceLogUsedSmartToolThreadCount(theAdapter, smartToolModifier.UsedThreadCounts());

                if(!smartToolModifier.LastExceptionMessageFromThreads().empty())
                    ::LogMessage(theAdapter, smartToolModifier.LastExceptionMessageFromThreads(), CatLog::Severity::Error, CatLog::Category::Editing);
            }
			catch(std::exception &e)
			{
				theAdapter.SmartToolEditingErrorText() = e.what();
                ::LogMessage(theAdapter, std::string("Error: SmartTool-macro execution failed: ") + theAdapter.SmartToolEditingErrorText(), CatLog::Severity::Warning, CatLog::Category::Editing);
				return false;
			}
		}

		if(fDoDataValidations)
		{
			// tehd‰‰n j‰rkevyystarkastelu operaatio kaikille parametreille
            ::CheckAndValidateAfterModifications(theAdapter, NFmiMetEditorTypes::kFmiSmarttool, false, fSelectedLocationsOnly ? NFmiMetEditorTypes::kFmiSelectionMask : NFmiMetEditorTypes::kFmiNoMask, kFmiLastParameter, fDoMultiThread, &modifiedParams);
		}

        LogSmartToolModifications(theAdapter, modifiedParams, theLogMessage, showLoadedSmartTool);
        ::MakeBasicViewUpdatePreparationsAfterDataModifications(theAdapter);
    }
	else
	{
		theAdapter.SmartToolEditingErrorText() = "Error: there were no edited data available.";
        ::LogMessage(theAdapter, std::string("SmartTool-macro wont be executed: ") + theAdapter.SmartToolEditingErrorText(), CatLog::Severity::Warning, CatLog::Category::Editing);
		return false;
	}
	return true;
}

static const std::string& GetUsedSmarttoolIncludeDirectory(TimeSerialModificationDataInterface& theAdapter)
{
	return theAdapter.SmartToolInfo()->RootLoadDirectory();
}

static bool DoSmartToolEditing(TimeSerialModificationDataInterface &theAdapter, const std::string &theSmartToolText, const std::string &theRelativePathMacroName, bool fSelectedLocationsOnly, bool fDoMultiThread, NFmiThreadCallBacks *theThreadCallBacks)
{
	bool status = false;
	boost::shared_ptr<NFmiFastQueryInfo> editedData = theAdapter.EditedInfo();
	if(editedData)
	{
		boost::shared_ptr<NFmiTimeDescriptor> times = theAdapter.CreateDataFilteringTimeDescriptor(editedData);

		CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(nullptr, __FUNCTION__, CatLog::Severity::Debug);

		status = ::DoSmartToolEditing(theAdapter, theSmartToolText, theRelativePathMacroName, fSelectedLocationsOnly, *times,
							std::string(" - modified with SmartTool."),
							::GetUsedSmarttoolIncludeDirectory(theAdapter),
							true,
							fDoMultiThread, 
							theThreadCallBacks,
                            true);
	}
	return status;
}

// T‰m‰ luo SmartInfosta pinta kopioin, eli vain ns. iteraattori (info osuus) kopioituu, data pysyy alkuper‰isen kanssa jaettuna.
static boost::shared_ptr<NFmiFastQueryInfo> CreateShallowInfoCopy(boost::shared_ptr<NFmiFastQueryInfo> &theUsedInfo, const NFmiDataIdent& theDataIdent, const NFmiLevel* theLevel, NFmiInfoData::Type theType)
{
	if(theUsedInfo && theUsedInfo->DataType() == theType && theUsedInfo->Param(*theDataIdent.GetParam()) && (!theLevel || (theLevel && theUsedInfo->Level(*theLevel))))
	{
		boost::shared_ptr<NFmiFastQueryInfo> copyOfInfo(new NFmiFastQueryInfo(*theUsedInfo));
		return copyOfInfo;
	}
	return boost::shared_ptr<NFmiFastQueryInfo>();
}

// K‰yttˆesimerkki: Halutaan aluemuokkauksen yhteydess‰ k‰ytt‰‰ maskeja, mutta
// koska editoitava data saattaa muuttua muokkauksen yhteydessa, pit‰‰ maskien
// laskuissa k‰ytt‰‰ kopiota alkuper‰isest‰ datasta. Parametrina annetaan
// haluttu maskilista ja kopio editoitavasta datasta. Metodi k‰y l‰pi annetun listan
// ja tekee kopiot maskeista ja laittaa ne uuteen luotavaan maskilistaan ja jos
// maski k‰ytt‰‰ editoitavaa dataa, korvataan kopiomaskiin dataksi parametrina annettu
// kopio datasta. (ongelma esiintyy esim. jos haluan tasoittaa kokonaispilvisyytt‰
// mutta en halua levitt‰‰ pilvialuetta, joten k‰yt‰n kokonaispilvisyytt‰ maskina.)
static boost::shared_ptr<NFmiAreaMaskList> CreateNewParamMaskListWithReplacedEditedInfo(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiAreaMaskList> &theOriginalMaskList, boost::shared_ptr<NFmiFastQueryInfo> &theUsedEditInfo)
{
	if(theOriginalMaskList && theUsedEditInfo)
	{
		boost::shared_ptr<NFmiAreaMaskList> newMaskList(new NFmiAreaMaskList());
		for(theOriginalMaskList->Reset(); theOriginalMaskList->Next();)
		{
			boost::shared_ptr<NFmiAreaMask> mask = theOriginalMaskList->Current();
			if(!mask)
				continue; // virhetilanne!!!! pit‰isi olla assert
			boost::shared_ptr<NFmiFastQueryInfo> info;
			NFmiInfoData::Type type = mask->GetDataType();
			if(type == NFmiInfoData::kCalculatedValue)
			{
				boost::shared_ptr<NFmiAreaMask> cloneMask(mask->Clone());
				newMaskList->Add(cloneMask);
			}
			else
			{
				if(type == NFmiInfoData::kEditable)
					info = ::CreateShallowInfoCopy(theUsedEditInfo, *mask->DataIdent(), mask->Level(), mask->GetDataType());
				else
				{
					boost::shared_ptr<NFmiFastQueryInfo> tmpInfo = theAdapter.InfoOrganizer()->Info(*mask->DataIdent()
																		,mask->Level()
																		,mask->GetDataType());
					info = ::CreateShallowInfoCopy(tmpInfo, *mask->DataIdent(), mask->Level(), mask->GetDataType());
				}
				boost::shared_ptr<NFmiAreaMask> newMask(new NFmiInfoAreaMask(mask->Condition()
															,mask->MaskType()
															,mask->GetDataType()
															,info
                                                            ,kFmiBadParameter
															,NFmiAreaMask::kNoValue));
				newMaskList->Add(newMask);
			}
		}
		return newMaskList;
	}
	return boost::shared_ptr<NFmiAreaMaskList>();
}

static boost::shared_ptr<NFmiAreaMaskList> CreateFilteringMaskList(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiFastQueryInfo> &theCopyOfEditedData)
{
	boost::shared_ptr<NFmiAreaMaskList> maskList;
	if(theAdapter.UseMasksWithFilterTool())
	{
		boost::shared_ptr<NFmiAreaMaskList> tmpMaskList(theAdapter.ParamMaskList());
		maskList = ::CreateNewParamMaskListWithReplacedEditedInfo(theAdapter, tmpMaskList, theCopyOfEditedData);
	}
	if(maskList == 0)
		maskList = boost::shared_ptr<NFmiAreaMaskList>(new NFmiAreaMaskList()); // luodaan aina tyhj‰ lista, vaikka maskeja ei olisi k‰ytˆss‰
	if(maskList)
		maskList->CheckIfMaskUsed();
	return maskList;
}

static NFmiDataModifier* CreateFilteringHelperModifier(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiFastQueryInfo> &theCopyOfEditedData)
{
	NFmiDataModifier *helpModifier = 0;
	if(theCopyOfEditedData)
	{
		FmiInterpolationMethod interp = theCopyOfEditedData->Param().GetParam()->InterpolationMethod();
		switch(theAdapter.FilterFunction())
		{
		case 0:
		case 3: // t‰m‰ on painotettu keskiarvo, mutta se lasketaan sen ulomman modifierin avulla k‰ytt‰en normaaleja avg-laskuja!
			if(interp == kByCombinedParam || interp == kNearestPoint)
				helpModifier = new NFmiDataModifierMode(); // ep‰jatkuville lasketaankin moodi!!!
			else
				helpModifier = new NFmiDataModifierAvg();
			break;
		case 1:
			helpModifier = new NFmiDataModifierMax();
			break;
		case 2:
			helpModifier = new NFmiDataModifierMin();
			break;
		case 4:
			helpModifier = new NFmiDataModifierMedian();
			break;
		default: // pit‰isi olla 2!!!
			helpModifier = new NFmiDataModifierMin();
			break;
		}
	}
	return helpModifier;
}

static std::vector<NFmiRect> CreateAreaFilterRangeArray(TimeSerialModificationDataInterface &theAdapter)
{
	std::vector<NFmiRect> returnRectArray;
	boost::shared_ptr<NFmiFastQueryInfo> editedInfo = theAdapter.EditedInfo();

	if(editedInfo && editedInfo->TimeToNearestStep(theAdapter.TimeFilterStartTime(), kCenter))
	{
		int arraySize = editedInfo->SizeTimes();
		int startIndex = editedInfo->TimeIndex();
		returnRectArray.resize(arraySize);
		int filledArraySize = 0;
		if(editedInfo->TimeToNearestStep(theAdapter.TimeFilterEndTime(), kCenter))
			filledArraySize = editedInfo->TimeIndex() - startIndex + 1;
		if(filledArraySize + startIndex <= static_cast<int>(editedInfo->SizeTimes())) // periaatteessa t‰m‰ on turha tarkistus, mutta eip‰ pit‰isi tulla taulukon yli kirjoitusta mill‰‰n
		{
			NFmiRect areaFilterRangeStart = theAdapter.AreaFilterRange(1);
			NFmiRect areaFilterRangeEnd = theAdapter.AreaFilterRange(2);
			if(filledArraySize == 1)
			{
				int left = boost::math::iround(areaFilterRangeStart.Left());
				int top = boost::math::iround(areaFilterRangeStart.Top());
				int right = boost::math::iround(areaFilterRangeStart.Right());
				int bottom = boost::math::iround(areaFilterRangeStart.Bottom());
				returnRectArray[startIndex] = NFmiRect(left, top, right, bottom);
			}
			else
			{
				for(int i = startIndex; i < filledArraySize+startIndex; i++)
				{
					double ratio = 1. - (i - startIndex)/static_cast<double>(filledArraySize);
					int left = boost::math::iround(areaFilterRangeStart.Left() * ratio + areaFilterRangeEnd.Left() * (1. - ratio));
					int top = boost::math::iround(areaFilterRangeStart.Top() * ratio + areaFilterRangeEnd.Top() * (1. - ratio));
					int right = boost::math::iround(areaFilterRangeStart.Right() * ratio + areaFilterRangeEnd.Right() * (1. - ratio));
					int bottom = boost::math::iround(areaFilterRangeStart.Bottom() * ratio + areaFilterRangeEnd.Bottom() * (1. - ratio));
					returnRectArray[i] = NFmiRect(left, top, right, bottom);
				}
			}
		}
	}
	return returnRectArray;

}
static boost::shared_ptr<NFmiDataModifier> CreateAreaFilteringModifier(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiAreaMaskList> &theMaskList, boost::shared_ptr<NFmiFastQueryInfo> &theCopyOfEditedData)
{
	NFmiDataModifier *helpModifier = ::CreateFilteringHelperModifier(theAdapter, theCopyOfEditedData);
	boost::shared_ptr<NFmiDataModifier> areaModifier;
	if(helpModifier)
	{
		std::vector<NFmiRect> areaFilterArray = ::CreateAreaFilterRangeArray(theAdapter);
		if(theAdapter.UseTimeInterpolation())
			areaModifier = boost::shared_ptr<NFmiDataModifier>(new NFmiDataModifierTimeInterpolationWithAreaModifiers(theCopyOfEditedData, helpModifier, theMaskList, areaFilterArray, theAdapter.TimeFilterStartTime(), theAdapter.TimeFilterEndTime(), theCopyOfEditedData->TimeResolution()));
		else if(theAdapter.FilterFunction() != 3)
			areaModifier = boost::shared_ptr<NFmiDataModifier>(new NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime(theCopyOfEditedData, helpModifier, theMaskList, areaFilterArray));
		else // tehd‰‰n painotetulle keskiarvolle oma olio
			areaModifier = boost::shared_ptr<NFmiDataModifier>(new NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTimeUsingWAvg(theCopyOfEditedData, helpModifier, theMaskList, areaFilterArray));
	}
	return areaModifier;
}

static bool ModifyLocationData(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiAreaMaskList> &theMaskList, boost::shared_ptr<NFmiFastQueryInfo> &theEditedData, boost::shared_ptr<NFmiFastQueryInfo> &theCopyOfEditedData, boost::shared_ptr<NFmiTimeDescriptor> &theTimes, bool fSetParamToActiveParam, bool fDoMultiThread)
{
	boost::shared_ptr<NFmiDrawParam> drawParam = theAdapter.ActiveDrawParamFromActiveRow(0); // n‰m‰ moukkaustyˆkalulla (filtteri tyˆkalu) tehdyt muokkaukset tehd‰‰n aina p‰‰karttaikkunan aktiiviselle datalle
	auto& param = *drawParam->Param().GetParam();
	if(!fSetParamToActiveParam || (drawParam && drawParam->DataType() == NFmiInfoData::kEditable && theEditedData->Param(param)))
	{
		if(drawParam && drawParam->Param().Type() == kSymbolicParam) // turha tehd‰ symbolisille parametreille
			return false;
		if(fSetParamToActiveParam && drawParam)
			if(!theCopyOfEditedData->Param(param))
				return false;

		boost::shared_ptr<NFmiDataModifier> areaModifier = CreateAreaFilteringModifier(theAdapter, theMaskList, theCopyOfEditedData);
		bool status = false;
		if(areaModifier)
		{
			status = true;
			if(fDoMultiThread)
				::ModifyTimesLocationData_FullMT(theEditedData, areaModifier.get(), *theTimes);
			else
				theEditedData->ModifyTimesLocationData(areaModifier.get(), theCopyOfEditedData.get(), *theTimes);
		}
		return status;
	}
	return false;
}

static bool DoAreaFilteringToAllParams(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiAreaMaskList> &theMaskList, boost::shared_ptr<NFmiFastQueryInfo> &theEditedData, boost::shared_ptr<NFmiFastQueryInfo> &theCopyOfEditedData, boost::shared_ptr<NFmiTimeDescriptor> &theTimes, bool fDoMultiThread)
{
	if(theEditedData && theCopyOfEditedData)
	{
		for(theEditedData->ResetParam(); theEditedData->NextParam(); )
		{
			if(!theEditedData->Param().HasDataParams())
			{
				if(theCopyOfEditedData->Param(*theEditedData->Param().GetParam()))
				{
					::ModifyLocationData(theAdapter, theMaskList, theEditedData, theCopyOfEditedData, theTimes, false, fDoMultiThread);
				}
			}
			else // k‰yd‰‰n aliparametritkin l‰pi
			{
				NFmiParamBag subParamBag(*theEditedData->Param().GetDataParams());
				for(subParamBag.Reset(); subParamBag.Next(); )
				{
					if(subParamBag.Current()->Type() != kSymbolicParam)
					{
						auto& subParam = *subParamBag.Current()->GetParam();
						if(theEditedData->Param(subParam))
						{
							if(theCopyOfEditedData->Param(subParam))
							{
								::ModifyLocationData(theAdapter, theMaskList, theEditedData, theCopyOfEditedData, theTimes, false, fDoMultiThread);
							}
						}
					}
				}
			}

		}
		return true;
	}
	return false;
}

static bool DoAreaFilteringToSelectedParams(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiAreaMaskList> &theMaskList, boost::shared_ptr<NFmiFastQueryInfo> &theEditedData, boost::shared_ptr<NFmiFastQueryInfo> &theCopyOfEditedData, boost::shared_ptr<NFmiTimeDescriptor> &theTimes, bool fDoMultiThread)
{
	if(theCopyOfEditedData && theEditedData)
	{
		NFmiParamBag &filteringParamBag = theAdapter.FilteringParamBag();
		for(filteringParamBag.Reset(); filteringParamBag.Next(); )
		{
			if(!filteringParamBag.Current()->HasDataParams())
			{
				if(filteringParamBag.Current()->IsActive()) // vain aktiiviset k‰yd‰‰n l‰pi
				{
					auto& filteringParam = *filteringParamBag.Current()->GetParam();
					if(theEditedData->Param(filteringParam))
					{
						if(theCopyOfEditedData->Param(filteringParam))
						{
							::ModifyLocationData(theAdapter, theMaskList, theEditedData, theCopyOfEditedData, theTimes, false, fDoMultiThread);
						}
					}
				}
			}
			else // k‰yd‰‰n aliparametritkin l‰pi
			{
				NFmiParamBag subParamBag(*filteringParamBag.Current()->GetDataParams());
				for(subParamBag.Reset(); subParamBag.Next(); )
				{
					if(subParamBag.Current()->IsActive()) // vain aktiiviset k‰yd‰‰n l‰pi
					{
						if(subParamBag.Current()->Type() != kSymbolicParam)
						{
							auto subParam = *subParamBag.Current()->GetParam();
							if(theEditedData->Param(subParam))
							{
								if(theCopyOfEditedData->Param(subParam))
								{
									::ModifyLocationData(theAdapter, theMaskList, theEditedData, theCopyOfEditedData, theTimes, false, fDoMultiThread);
								}
							}
						}
					}
				}
			}
		}
		return true;
	}
	return false;
}

static bool DoAreaFiltering(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiAreaMaskList> &theMaskList, boost::shared_ptr<NFmiFastQueryInfo> &theEditedData, boost::shared_ptr<NFmiFastQueryInfo> &theCopyOfEditedData, boost::shared_ptr<NFmiTimeDescriptor> &theTimes, bool fDoMultiThread)
{
	switch(theAdapter.FilteringParameterUsageState())
	{
	case 0: // 0=tehd‰‰n operaatio vain aktiiviselle parametrille
		{
			return ::ModifyLocationData(theAdapter, theMaskList, theEditedData, theCopyOfEditedData, theTimes, true, fDoMultiThread);
		}
	case 1: // 1=tehd‰‰n operaatio kaikille parametreille
		{
		return ::DoAreaFilteringToAllParams(theAdapter, theMaskList, theEditedData, theCopyOfEditedData, theTimes, fDoMultiThread);
		}
	case 2: // 2=tehd‰‰n operaatio valituille parametreille
		{
		return ::DoAreaFilteringToSelectedParams(theAdapter, theMaskList, theEditedData, theCopyOfEditedData, theTimes, fDoMultiThread);
		}
	}
	return false;
}

static bool UndoData(TimeSerialModificationDataInterface &theAdapter)
{
	boost::shared_ptr<NFmiFastQueryInfo> info = theAdapter.EditedInfo();
	if(info)
	{
		NFmiSmartInfo *smartInfo = dynamic_cast<NFmiSmartInfo*>(info.get()); // pakko tehd‰ down-casti editoitavalle datalle toistaiseksi n‰in, smartInfo-pointteri on k‰ytett‰viss‰ info -shared_ptr on olemassa
		theAdapter.LastBrushedViewRealRowIndex(-1); // sivellint‰ varten pit‰‰ 'nollata' t‰m‰
        std::string modificationDescription = "";
		bool status = smartInfo->UndoData(modificationDescription);
		if(status)
		{
			theAdapter.WindTableSystemMustaUpdateTable(true);
            ::LogMessage(theAdapter, "Undo " + modificationDescription + ".", CatLog::Severity::Info, CatLog::Category::Editing);
		}
        ::MakeBasicViewUpdatePreparationsAfterDataModifications(theAdapter);
        return status;
	}
	return false;
}

static bool RedoData(TimeSerialModificationDataInterface &theAdapter)
{
	boost::shared_ptr<NFmiFastQueryInfo> info = theAdapter.EditedInfo();
	if(info)
	{
		NFmiSmartInfo *smartInfo = dynamic_cast<NFmiSmartInfo*>(info.get()); // pakko tehd‰ down-casti editoitavalle datalle toistaiseksi n‰in, smartInfo-pointteri on k‰ytett‰viss‰ info -shared_ptr on olemassa
		theAdapter.LastBrushedViewRealRowIndex(-1); // sivellint‰ varten pit‰‰ 'nollata' t‰m‰
        std::string modificationDescription = "";
		bool status = smartInfo->RedoData(modificationDescription);
		if(status)
		{
			theAdapter.WindTableSystemMustaUpdateTable(true);
            ::LogMessage(theAdapter, "Redo " + modificationDescription + ".", CatLog::Severity::Info, CatLog::Category::Editing);
		}
        ::MakeBasicViewUpdatePreparationsAfterDataModifications(theAdapter);
        return status;
	}
	return false;
}

void LogDataFilterToolsModifications(TimeSerialModificationDataInterface &theAdapter, bool areaModification)
{
    std::string toolText = (areaModification) ? " - modified with Data filter tool's area modification." : " - modified with Data filter tool's time modification.";

    ::LogMessage(theAdapter, FmiModifyEditdData::DataFilterToolsParamsForLog(theAdapter) + ModifiedTimesForLog(theAdapter) + toolText, CatLog::Severity::Info, CatLog::Category::Editing);
}

static bool DoAreaFiltering(TimeSerialModificationDataInterface &theAdapter, bool fDoMultiThread)
{
	boost::shared_ptr<NFmiFastQueryInfo> editedData = theAdapter.EditedInfo();
	if(editedData)
	{
        NFmiMetTime time = editedData->Time();
		{
			boost::shared_ptr<NFmiTimeDescriptor> times = theAdapter.CreateDataFilteringTimeDescriptor(editedData);
			try
			{
				::SnapShotData(theAdapter, editedData, editedData->Param(), "Area modifications", times->FirstTime(), times->LastTime());
			}
			catch(...)
			{
				// heitetty poikkeus eli halutaan lopettaa toiminto
				return false;
			}

			boost::shared_ptr<NFmiFastQueryInfo> copyOfEditedData = boost::shared_ptr<NFmiFastQueryInfo>(dynamic_cast<NFmiFastQueryInfo*>(editedData->Clone()));
            auto usedMaskType = theAdapter.TestFilterUsedMask();
            EditedInfoMaskHandler editedInfoMaskHandler(editedData, usedMaskType);
			bool status = false;
			if(copyOfEditedData)
			{
				boost::shared_ptr<NFmiAreaMaskList> maskList = ::CreateFilteringMaskList(theAdapter, copyOfEditedData);
				copyOfEditedData->MaskType(usedMaskType);
				status = ::DoAreaFiltering(theAdapter, maskList, editedData, copyOfEditedData, times, fDoMultiThread);
				if(!status)
					::UndoData(theAdapter);
				else
				{
					::CheckAndValidateAfterModifications(theAdapter, NFmiMetEditorTypes::kFmiDataModificationTool, false, theAdapter.TestFilterUsedMask(), kFmiLastParameter, fDoMultiThread);
                    ::MakeBasicViewUpdatePreparationsAfterDataModifications(theAdapter);
                }
			}
			editedData->Time(time);
            LogDataFilterToolsModifications(theAdapter, true);
			return status;
		}
	}
	return false;
}

// huom t‰m‰ ei ota huomioon vaihtuvia aika-askelia, vaan laskee haluttuja juttuja niist‰ v‰litt‰m‰tt‰
std::vector<double> CreateTimeFilterShiftArrayWCTR(TimeSerialModificationDataInterface &theAdapter)
{
	std::vector<double> returnFactors;
	boost::shared_ptr<NFmiFastQueryInfo> editedInfo = theAdapter.EditedInfo();

	if(editedInfo && editedInfo->TimeToNearestStep(theAdapter.TimeFilterStartTime(), kCenter))
	{
		int arraySize = editedInfo->SizeTimes();
		int startIndex = editedInfo->TimeIndex();
		returnFactors.resize(arraySize);
		int filledArraySize = 0;
		if(editedInfo->TimeToNearestStep(theAdapter.TimeFilterEndTime(), kCenter))
			filledArraySize = editedInfo->TimeIndex() - startIndex + 1;
		if(filledArraySize + startIndex <= static_cast<int>(editedInfo->SizeTimes())) // periaatteessa t‰m‰ on turha tarkistus, mutta eip‰ pit‰isi tulla taulukon yli kirjoitusta mill‰‰n
		{
			if(filledArraySize == 1)
				returnFactors[startIndex] = theAdapter.TimeFilterRangeStart().X();
			else
			{
				for(int i = startIndex; i < filledArraySize+startIndex; i++)
				{
					double ratio = 1. - (i - startIndex)/static_cast<double>(filledArraySize);
					double x = round(theAdapter.TimeFilterRangeStart().X() * ratio + theAdapter.TimeFilterRangeEnd().X() * (1. - ratio));
					returnFactors[i] = x;
				}
			}
		}
	}
	return returnFactors;
}

static std::vector<double> CreateTimeFilterShiftArray(TimeSerialModificationDataInterface &theAdapter)
{
	return ::CreateTimeFilterShiftArrayWCTR(theAdapter);
}

// huom t‰m‰ ei ota huomioon vaihtuvia aika-askelia, vaan laskee haluttuja juttuja niist‰ v‰litt‰m‰tt‰
static std::vector<NFmiPoint> CreateTimeFilterRangeArrayWCTR(TimeSerialModificationDataInterface &theAdapter)
{
	std::vector<NFmiPoint> returnPointArray;
	boost::shared_ptr<NFmiFastQueryInfo> editedInfo = theAdapter.EditedInfo();

	if(editedInfo && editedInfo->TimeToNearestStep(theAdapter.TimeFilterStartTime(), kCenter))
	{
		int arraySize = editedInfo->SizeTimes();
		int startIndex = editedInfo->TimeIndex();
		returnPointArray.resize(arraySize);
		int filledArraySize = 0;
		if(editedInfo->TimeToNearestStep(theAdapter.TimeFilterEndTime(), kCenter))
			filledArraySize = editedInfo->TimeIndex() - startIndex + 1;
		if(filledArraySize + startIndex <= static_cast<int>(editedInfo->SizeTimes())) // periaatteessa t‰m‰ on turha tarkistus, mutta eip‰ pit‰isi tulla taulukon yli kirjoitusta mill‰‰n
		{
			if(filledArraySize == 1)
				returnPointArray[startIndex] = theAdapter.TimeFilterRangeStart();
			else
			{
				for(int i = startIndex; i < filledArraySize+startIndex; i++)
				{
					double ratio = 1. - (i - startIndex)/static_cast<double>(filledArraySize);
					double x = round(theAdapter.TimeFilterRangeStart().X() * ratio + theAdapter.TimeFilterRangeEnd().X() * (1. - ratio));
					double y = round(theAdapter.TimeFilterRangeStart().Y() * ratio + theAdapter.TimeFilterRangeEnd().Y() * (1. - ratio));
					returnPointArray[i] = NFmiPoint(x, y);
				}
			}
		}
	}
	return returnPointArray;
}

static std::vector<NFmiPoint> CreateTimeFilterRangeArray(TimeSerialModificationDataInterface &theAdapter)
{
	return ::CreateTimeFilterRangeArrayWCTR(theAdapter);
}

static boost::shared_ptr<NFmiDataModifier> CreateTimeFilteringModifier(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiAreaMaskList> &theMaskList, boost::shared_ptr<NFmiFastQueryInfo> &theCopyOfEditedData)
{
// HUOM!! Maskilistaa ei viel‰ k‰ytet‰.
	boost::shared_ptr<NFmiDataModifier> timeModifier;
	if(theMaskList && theCopyOfEditedData)
	{
		if(theAdapter.IsSmoothTimeShiftPossible())
		{
			std::vector<double> timeShiftFactors = ::CreateTimeFilterShiftArray(theAdapter);
			timeModifier = boost::shared_ptr<NFmiDataModifier>(new NFmiDataModifierTimeShifter(theCopyOfEditedData, timeShiftFactors, theMaskList));
		}
		else
		{
			NFmiDataModifier* helpModifier = ::CreateFilteringHelperModifier(theAdapter, theCopyOfEditedData);
			std::vector<NFmiPoint> timeRangeArray = ::CreateTimeFilterRangeArray(theAdapter);
			if(theAdapter.FilterFunction() != 3)
				timeModifier = boost::shared_ptr<NFmiDataModifier>(new NFmiDataModifierTimeShifterWithChangingRanges(theCopyOfEditedData, timeRangeArray, helpModifier, theMaskList));
			else
				timeModifier = boost::shared_ptr<NFmiDataModifier>(new NFmiDataModifierTimeShifterWithChangingRangesUsingWAvg(theCopyOfEditedData, timeRangeArray, helpModifier, theMaskList));
		}
	}
	return timeModifier;
}

static bool ModifyTimesLocationData(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiAreaMaskList> &theMaskList, boost::shared_ptr<NFmiFastQueryInfo> &theEditedData, boost::shared_ptr<NFmiFastQueryInfo> &theCopyOfEditedData, boost::shared_ptr<NFmiTimeDescriptor> &theTimes, bool fSetParamToActiveParam, bool fDoMultiThread)
{
	if(theEditedData && theCopyOfEditedData)
	{
		boost::shared_ptr<NFmiDrawParam> drawParam = theAdapter.ActiveDrawParamFromActiveRow(0);  // n‰m‰ moukkaustyˆkalulla (filtteri tyˆkalu) tehdyt muokkaukset tehd‰‰n aina p‰‰karttaikkunan aktiiviselle datalle
		if(drawParam)
		{
			auto& editedParam = *drawParam->Param().GetParam();
			if(!fSetParamToActiveParam || (drawParam && drawParam->DataType() == NFmiInfoData::kEditable && theEditedData->Param(editedParam)))
			{
				if(fSetParamToActiveParam)
				{

					if(!theCopyOfEditedData->Param(editedParam))
						return false;
				}
				boost::shared_ptr<NFmiDataModifier> timeModifier = ::CreateTimeFilteringModifier(theAdapter, theMaskList, theCopyOfEditedData);
				if(timeModifier)
				{
					if(fDoMultiThread)
						::ModifyTimesLocationData_FullMT(theEditedData, timeModifier.get(), *theTimes);
					else
						theEditedData->ModifyTimesLocationData(timeModifier.get(), theCopyOfEditedData.get(), *theTimes);
					return true;
				}
			}
		}
	}
	return false;
}

static bool DoTimeFilteringToAllParams(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiAreaMaskList> &theMaskList, boost::shared_ptr<NFmiFastQueryInfo> &theEditedData, boost::shared_ptr<NFmiFastQueryInfo> &theCopyOfEditedData, boost::shared_ptr<NFmiTimeDescriptor> &theTimes, bool fDoMultiThread)
{
	if(theEditedData && theCopyOfEditedData)
	{
		for(theEditedData->ResetParam(); theEditedData->NextParam(); )
		{
			if(!theEditedData->Param().HasDataParams())
			{
				if(theCopyOfEditedData->Param(*theEditedData->Param().GetParam()))
				{
					::ModifyTimesLocationData(theAdapter, theMaskList, theEditedData, theCopyOfEditedData, theTimes, false, fDoMultiThread);
				}
			}
			else // k‰yd‰‰n aliparametritkin l‰pi
			{
				NFmiParamBag subParamBag(*theEditedData->Param().GetDataParams());
				for(subParamBag.Reset(); subParamBag.Next(); )
				{
					if(subParamBag.Current()->Type() != kSymbolicParam)
					{
						auto& subParam = *subParamBag.Current()->GetParam();
						if(theEditedData->Param(subParam))
						{
							if(theCopyOfEditedData->Param(subParam))
							{
								::ModifyTimesLocationData(theAdapter, theMaskList, theEditedData, theCopyOfEditedData, theTimes, false, fDoMultiThread);
							}
						}
					}
				}
			}

		}
		return true;
	}
	return false;
}

static bool DoTimeFilteringToSelectedParams(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiAreaMaskList> &theMaskList, boost::shared_ptr<NFmiFastQueryInfo> &theEditedData, boost::shared_ptr<NFmiFastQueryInfo> &theCopyOfEditedData, boost::shared_ptr<NFmiTimeDescriptor> &theTimes, bool fDoMultiThread)
{
	if(theEditedData && theCopyOfEditedData)
	{
		NFmiParamBag &filteringParamBag = theAdapter.FilteringParamBag();
		for(filteringParamBag.Reset(); filteringParamBag.Next(); )
		{
			if(!filteringParamBag.Current()->HasDataParams())
			{
				if(filteringParamBag.Current()->IsActive()) // vain aktiiviset k‰yd‰‰n l‰pi
				{
					auto& filterinParam = *filteringParamBag.Current()->GetParam();
					if(theEditedData->Param(filterinParam))
					{
						if(theCopyOfEditedData->Param(filterinParam))
						{
							::ModifyTimesLocationData(theAdapter, theMaskList, theEditedData, theCopyOfEditedData, theTimes, false, fDoMultiThread);
						}
					}
				}
			}
			else // k‰yd‰‰n aliparametritkin l‰pi
			{
				NFmiParamBag subParamBag(*filteringParamBag.Current()->GetDataParams());
				for(subParamBag.Reset(); subParamBag.Next(); )
				{
					if(subParamBag.Current()->IsActive()) // vain aktiiviset k‰yd‰‰n l‰pi
					{
						if(subParamBag.Current()->Type() != kSymbolicParam)
						{
							auto& subParam = *subParamBag.Current()->GetParam();
							if(theEditedData->Param(subParam))
							{
								if(theCopyOfEditedData->Param(subParam))
								{
									::ModifyTimesLocationData(theAdapter, theMaskList, theEditedData, theCopyOfEditedData, theTimes, false, fDoMultiThread);
								}
							}
						}
					}
				}
			}
		}
		return true;
	}
	return false;
}

static bool DoTimeFiltering(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiAreaMaskList> &theMaskList, boost::shared_ptr<NFmiFastQueryInfo> &theEditedData, boost::shared_ptr<NFmiFastQueryInfo> &theCopyOfEditedData, boost::shared_ptr<NFmiTimeDescriptor> &theTimes, bool fDoMultiThread)
{
	bool status = true;
	switch(theAdapter.FilteringParameterUsageState())
	{
	case 0: // 0=tehd‰‰n operaatio vain aktiiviselle parametrille
		{
		status = ::ModifyTimesLocationData(theAdapter, theMaskList, theEditedData, theCopyOfEditedData, theTimes, true, fDoMultiThread);
		break;
		}
	case 1: // 1=tehd‰‰n operaatio kaikille parametreille
		{
		status = ::DoTimeFilteringToAllParams(theAdapter, theMaskList, theEditedData, theCopyOfEditedData, theTimes, fDoMultiThread);
		break;
		}
	case 2: // 2=tehd‰‰n operaatio valituille parametreille
		{
		status = ::DoTimeFilteringToSelectedParams(theAdapter, theMaskList, theEditedData, theCopyOfEditedData, theTimes, fDoMultiThread);
		break;
		}
	}
	return status;
}

static bool DoTimeFiltering(TimeSerialModificationDataInterface &theAdapter, bool fDoMultiThread)
{
	boost::shared_ptr<NFmiFastQueryInfo> editedData = theAdapter.EditedInfo();
	if(editedData)
	{
		NFmiMetTime time = editedData->Time();
		boost::shared_ptr<NFmiTimeDescriptor> times = theAdapter.CreateDataFilteringTimeDescriptor(editedData);
		try
		{
			::SnapShotData(theAdapter, editedData, editedData->Param(), "Time modifications", times->FirstTime(), times->LastTime());
		}
		catch(...)
		{
			// heitetty poikkeus eli halutaan lopettaa toiminto
			return false;
		}

		boost::shared_ptr<NFmiFastQueryInfo> copyOfEditedData = boost::shared_ptr<NFmiFastQueryInfo>(dynamic_cast<NFmiFastQueryInfo*>(editedData->Clone()));
		if(!copyOfEditedData)
			return false;

		boost::shared_ptr<NFmiAreaMaskList> maskList = ::CreateFilteringMaskList(theAdapter, copyOfEditedData);
        auto usedMaskType = theAdapter.TestFilterUsedMask();
        EditedInfoMaskHandler editedInfoMaskHandler(editedData, usedMaskType);
		copyOfEditedData->MaskType(usedMaskType);
		bool status = ::DoTimeFiltering(theAdapter, maskList, editedData, copyOfEditedData, times, fDoMultiThread);

		if(!status)
			::UndoData(theAdapter);
		else
		{
            ::CheckAndValidateAfterModifications(theAdapter, NFmiMetEditorTypes::kFmiDataModificationTool, false, usedMaskType, kFmiLastParameter, fDoMultiThread, false);
            ::MakeBasicViewUpdatePreparationsAfterDataModifications(theAdapter);
        }

		editedData->Time(time);
        LogDataFilterToolsModifications(theAdapter, false);
		return status;
	}
	return false;
}

std::vector<float> CreateFilteringCombineFactorTable(TimeSerialModificationDataInterface &theAdapter)
{
	float filteringCombineFactorStart = 0.f;
	float filteringCombineFactorEnd = 1.f;
	std::vector<float> factorArray;
	boost::shared_ptr<NFmiFastQueryInfo> editedInfo = theAdapter.EditedInfo();

	if(editedInfo && editedInfo->TimeToNearestStep(theAdapter.TimeFilterStartTime(), kCenter))
	{
		int arraySize = editedInfo->SizeTimes();
		int startIndex = editedInfo->TimeIndex();
		factorArray.resize(arraySize);
		int filledArraySize = 0;
		if(editedInfo->TimeToNearestStep(theAdapter.TimeFilterEndTime(), kCenter))
			filledArraySize = editedInfo->TimeIndex() - startIndex + 1;
		if(filledArraySize + startIndex <= static_cast<int>(editedInfo->SizeTimes())) // periaatteessa t‰m‰ on turha tarkistus, mutta eip‰ pit‰isi tulla taulukon yli kirjoitusta mill‰‰n
		{
			if(filledArraySize == 1)
				factorArray[startIndex] = filteringCombineFactorStart;
			else
			{
				for(int i = startIndex; i < filledArraySize+startIndex; i++)
				{
					double ratio = 1. - (i - startIndex)/static_cast<double>(filledArraySize);
					double value = filteringCombineFactorStart * ratio + filteringCombineFactorEnd * (1. - ratio);
					factorArray[i] = static_cast<float>(value);
				}
			}
		}
	}
	return factorArray;
}

static boost::shared_ptr<NFmiDataModifier> CreateKlapseCombiningModifier(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiAreaMaskList> &theMaskList, boost::shared_ptr<NFmiFastQueryInfo> &theCopyOfEditedData)
{
	std::vector<NFmiRect> areaFilterArray = ::CreateAreaFilterRangeArray(theAdapter);
	boost::shared_ptr<NFmiFastQueryInfo> precipitationParamInfo(new NFmiFastQueryInfo(*theCopyOfEditedData));
	if(!precipitationParamInfo)
		return boost::shared_ptr<NFmiDataModifier>();
	precipitationParamInfo->SetDescriptors(theCopyOfEditedData.get());

	NFmiParam param(353, "Rain1h");
	NFmiProducer producer(1014, "NRD");
	NFmiDataIdent klapseParam(param, producer);

	boost::shared_ptr<NFmiFastQueryInfo> tempInfo = theAdapter.InfoOrganizer()->Info(klapseParam, 0, NFmiInfoData::kViewable, false, false);
	if(tempInfo == 0)
		return boost::shared_ptr<NFmiDataModifier>();
	boost::shared_ptr<NFmiFastQueryInfo> klapseParamInfo(new NFmiFastQueryInfo(*tempInfo));
	klapseParamInfo->First();

	float firstParamBase = 0;
	float secondParamBase = 0;
	float firstParamFactor = 1;
	float secondParamFactor = 1;
	float firstParamAdditionalMissingValue = kFloatMissing;
	float secondParamAdditionalMissingValue = kFloatMissing;

	NFmiDataModifier *helpModifier = ::CreateFilteringHelperModifier(theAdapter, theCopyOfEditedData);
	std::vector<float> factorArray = ::CreateFilteringCombineFactorTable(theAdapter);
	boost::shared_ptr<NFmiDataModifier> modifier(new NFmiDataModifierParamCombiner(
										 theCopyOfEditedData
										,helpModifier
										,theMaskList
										,areaFilterArray
										,precipitationParamInfo
										,klapseParamInfo
										,factorArray
										,firstParamBase
										,secondParamBase
										,firstParamFactor
										,secondParamFactor
										,firstParamAdditionalMissingValue
										,secondParamAdditionalMissingValue));
	return modifier;
}

static bool DoCombineModelAndKlapse(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiAreaMaskList> &theMaskList, boost::shared_ptr<NFmiFastQueryInfo> &theEditedData, boost::shared_ptr<NFmiFastQueryInfo> &theCopyOfEditedData, boost::shared_ptr<NFmiTimeDescriptor> &theTimes, bool fDoMultiThread)
{
	if(theEditedData && theEditedData->DataType() == NFmiInfoData::kEditable && theEditedData->Param(kFmiPrecipitation1h))
	{
		if(theCopyOfEditedData)
			if(!theCopyOfEditedData->Param(kFmiPrecipitation1h))
				return false;

		boost::shared_ptr<NFmiDataModifier> klapseCombiningModifier = ::CreateKlapseCombiningModifier(theAdapter, theMaskList, theCopyOfEditedData);

		bool status = false;
		if(klapseCombiningModifier)
		{
			status = true;
			if(fDoMultiThread)
				::ModifyTimesLocationData_FullMT(theEditedData, klapseCombiningModifier.get(), *theTimes);
			else
				theEditedData->ModifyTimesLocationData(klapseCombiningModifier.get(), theCopyOfEditedData.get(), *theTimes);
		}
		return status;
	}
	return false;
}

static bool DoCombineModelAndKlapse(TimeSerialModificationDataInterface &theAdapter, bool fDoMultiThread)
{
	boost::shared_ptr<NFmiFastQueryInfo> editedData = theAdapter.EditedInfo();
	if(editedData)
	{
        ::LogMessage(theAdapter, "Radar-prediction to model-precipitation combination.", CatLog::Severity::Info, CatLog::Category::Editing);
		NFmiMetTime time(editedData->Time());

		boost::shared_ptr<NFmiTimeDescriptor> times = theAdapter.CreateDataFilteringTimeDescriptor(editedData);
		try
		{
			::SnapShotData(theAdapter, editedData, editedData->Param(), "Radar-prediction to model-precipitation combination", times->FirstTime(), times->LastTime());
		}
		catch(...)
		{
			// heitetty poikkeus eli halutaan lopettaa toiminto
			return false;
		}
		boost::shared_ptr<NFmiFastQueryInfo> copyOfEditedData = boost::shared_ptr<NFmiFastQueryInfo>(dynamic_cast<NFmiFastQueryInfo*>(editedData->Clone()));
		if(!copyOfEditedData)
			return false;

		boost::shared_ptr<NFmiAreaMaskList> maskList = ::CreateFilteringMaskList(theAdapter, copyOfEditedData);
        auto usedMaskType = theAdapter.TestFilterUsedMask();
        EditedInfoMaskHandler editedInfoMaskHandler(editedData, usedMaskType);
		copyOfEditedData->MaskType(usedMaskType);
		bool status = ::DoCombineModelAndKlapse(theAdapter, maskList, editedData, copyOfEditedData, times, fDoMultiThread);
		if(!status)
			::UndoData(theAdapter);
		else
		{
            ::CheckAndValidateAfterModifications(theAdapter, NFmiMetEditorTypes::kFmiDataModificationTool, false, theAdapter.TestFilterUsedMask(), kFmiLastParameter, fDoMultiThread, false);
            ::MakeBasicViewUpdatePreparationsAfterDataModifications(theAdapter);
        }

		editedData->Time(time);

		return status;
	}
	return false;
}

class NFmiDataModifierSet : public NFmiDataModifier
{
	public:
	virtual float FloatOperation(float /* theValue */ )
	{
		return itsSetValue;
	}
	NFmiDataModifierSet(float theSetValue)
	:itsSetValue(theSetValue)
	{}

	NFmiDataModifierSet(const NFmiDataModifierSet &theOther)
	:NFmiDataModifier(theOther)
	,itsSetValue(theOther.itsSetValue)
	{
	}

	NFmiDataModifier* Clone(void) const
	{
		return new NFmiDataModifierSet(*this);
	}

	private:
	float itsSetValue;
};

static bool MakeDBCheckerDataValiditation(TimeSerialModificationDataInterface &theAdapter, bool fDoMultiThread)
{
	if(theAdapter.EditedInfo()) // parempi tarkistaa, onko editoitavaa dataa ollenkaan
	{
		NFmiTimeDescriptor timeDesc(theAdapter.EditedDataTimeDescriptor());

		return ::DoSmartToolEditing(theAdapter,
						theAdapter.SmartToolInfo()->DBCheckerText(),
                        "DBChecker",
						false,
						timeDesc,
						" - modified with DBChecker",
						::GetUsedSmarttoolIncludeDirectory(theAdapter),
						true, // =data validoidaan
						fDoMultiThread,
						0);
	}
	return false;
}

static std::string CreateHelpEditorFileNameWithPath(TimeSerialModificationDataInterface &theAdapter)
{
	std::string fileName(theAdapter.HelpEditorSystem().DataPath());
	fileName += "\\"; // varmuudeksi laitetaan polku deliminator per‰‰n
	NFmiMetTime aTime(1); // currentti aika aikaleimaa varten
	fileName += aTime.ToStr(kYYYYMMDDHHMM);
	fileName += "_";
	fileName += theAdapter.HelpEditorSystem().FileNameBase();
	return fileName;
}

// jos timebag pointteri on 0, otetaan timebagi smartista ja resoluutio annetusta parametrista,
// mutta jos timebag pointteri osoittaa johonkin, otetaan se suoraan k‰yttˆˆn.
static boost::shared_ptr<NFmiFastQueryInfo> CreateSimilarSmartInfoWithTimeInterpolation(boost::shared_ptr<NFmiFastQueryInfo> &theSmartInfo, long theResolutionInMinutes, NFmiTimeBag* theWantedTimeBag)
{
	boost::shared_ptr<NFmiFastQueryInfo> newSmartInfo;
	if(theSmartInfo && theSmartInfo->RefQueryData())
	{
		NFmiQueryData *data = const_cast<NFmiQueryData *>(theSmartInfo->RefQueryData());
		NFmiQueryData* newData = NFmiQueryDataUtil::InterpolateTimes(data, theResolutionInMinutes, theResolutionInMinutes, theWantedTimeBag, 360, kLagrange);
		newSmartInfo = boost::shared_ptr<NFmiFastQueryInfo>(new NFmiSmartInfo(newData, theSmartInfo->DataType(), "", ""));
		NFmiProducerIdLister prodIdsLister(*theSmartInfo);
		if(!prodIdsLister.IsEmpty())
		{
			prodIdsLister.ChangeTimeResolution(theResolutionInMinutes);
			newSmartInfo->AddKey(NFmiQueryDataUtil::GetOfficialQueryDataProdIdsKey(), prodIdsLister.MakeProducerIdString(), true);
		}
	}
	return newSmartInfo;
}

static void AddKeyValueToData(boost::shared_ptr<NFmiFastQueryInfo> &data, const std::string &key, const std::string &value)
{
    data->RemoveAllKeys(key); // putsataan ensin pois mahd. vanhat avain arvot
    if(!value.empty()) // jos value ei ollut tyhj‰, laitetaan sen arvo annettuun key:hin
        data->AddKey(key, value);
}

static bool StoreDataToDataBase(TimeSerialModificationDataInterface &theAdapter, const std::string &theForecasterId, const std::string &theHelperForecasterId, bool fDoMultiThread)
{
    bool status = true;
    ::LogMessage(theAdapter, "Sending data to server...", CatLog::Severity::Info, CatLog::Category::Editing);
    boost::shared_ptr<NFmiFastQueryInfo> storableSmart = theAdapter.EditedInfo();
    if(storableSmart == 0)
    {
        ::LogMessage(theAdapter, "There were no data to be sent to server.", CatLog::Severity::Warning, CatLog::Category::Editing);
        return false;
    }

    // en ollut varma, mihin v‰liin t‰m‰ datan validointi tietokantaan viennin yhteydess‰ pit‰isi laittaa (Marko)
    if(theAdapter.DataToDBCheckMethod() == 1 || (theAdapter.DataToDBCheckMethod() == 2 && theAdapter.MetEditorOptionsData().MakeDataValiditationAtSendingToDB()))
    {
        ::LogMessage(theAdapter, "Data validations before sending data to server.", CatLog::Severity::Info, CatLog::Category::Editing);
        ::MakeDataValiditation(theAdapter, fDoMultiThread);
    }

    if(theAdapter.SmartToolInfo()->MakeDBCheckAtSend())
    {
        ::LogMessage(theAdapter, "Data validations with DBChecker before sending data to server.", CatLog::Severity::Info, CatLog::Category::Editing);
        ::MakeDBCheckerDataValiditation(theAdapter, fDoMultiThread);
    }

    NFmiDataLoadingInfo &dataLoadingInfo = theAdapter.GetUsedDataLoadingInfo();
    dataLoadingInfo.InitFileNameLists();
    int length = dataLoadingInfo.DataLengthInHours();
    int version = dataLoadingInfo.LatestDataBaseInVersion(length) + 1;
    std::string dataBaseFileName(theAdapter.HelpEditorSystem().UsedHelpEditorStatus() ? ::CreateHelpEditorFileNameWithPath(theAdapter) : dataLoadingInfo.CreateDataBaseOutFileName(version).CharPtr());

    ::AddKeyValueToData(storableSmart, FmiModifyEditdData::ForecasterIDKey, theForecasterId);
    if(!theForecasterId.empty()) // Lis‰t‰‰n helper editoijan id vain jos varsinaisen editoijan id ei ole puuttuvaa
        ::AddKeyValueToData(storableSmart, FmiModifyEditdData::HelperForecasterIDKey, theHelperForecasterId);
    status = theAdapter.StoreData(dataBaseFileName, storableSmart, false);
    if(status)
        ::LogMessage(theAdapter, std::string("Edited data sent to server. '") + dataBaseFileName + std::string("'"), CatLog::Severity::Info, CatLog::Category::Editing);
    else
    {
        std::string logStr(::GetDictionaryString("Unable to store data file to database"));
        logStr += ". ";
        logStr += ::GetDictionaryString("File");
        logStr += "\n'";
        logStr += dataBaseFileName;
        logStr += "'\n ";
        logStr += ::GetDictionaryString("couldn't be created");
        logStr += ".";
        ::LogMessage(theAdapter, logStr, CatLog::Severity::Error, CatLog::Category::Editing);
        theAdapter.DoMessageBox(logStr, "Problem when sending data!", FMI_MB_OK);
    }
    return status;
}

static std::string GetViewModeSpeedLoadFile(TimeSerialModificationDataInterface &theAdapter, int thePrimaryProducer, NFmiDataLoadingInfo* theLoadingInfo)
{
	std::string apuFileFilter;
	switch(thePrimaryProducer)
	{
	case 0: // hirlam
		apuFileFilter = theLoadingInfo->Model1FilePattern();
		break;
	case 1: // ecmwf
		apuFileFilter = theLoadingInfo->Model2FilePattern();
		break;
	case 2: // oma tyo-tiedosto
		apuFileFilter = theLoadingInfo->CreateWorkingFileNameFilter();
		break;
	case 3: // editoitu-tietokanta-data
		apuFileFilter = theLoadingInfo->CreateDataBaseInFileNameFilter(1);
		break;
	}
	std::string latestFileName;
	/* time_t timeStamp = */ NFmiFileSystem::FindFile(apuFileFilter, true, &latestFileName);
	std::string wholeFileName;
	switch(thePrimaryProducer)
	{
	case 0: // hirlam
		wholeFileName = theLoadingInfo->Model1Path();
		break;
	case 1: // ecmwf
		wholeFileName = theLoadingInfo->Model2Path();
		break;
	case 2: // oma tyo-tiedosto
		wholeFileName = theLoadingInfo->WorkingPath();
		break;
	case 3: // editoitu-tietokanta-data
		wholeFileName = theLoadingInfo->DataBaseInPath();
		break;
	}
	wholeFileName += latestFileName;
	return wholeFileName;
}

static NFmiQueryData* ReadDataFromFile(TimeSerialModificationDataInterface &theAdapter, const std::string& fileName)
{
	NFmiQueryData* data = NULL;
	NFmiMilliSecondTimer timer;
	timer.StartTimer();

	if (data == NULL)
	{
		if(fileName.empty() == false && NFmiFileSystem::FileExists(fileName) && NFmiFileSystem::FileEmpty(fileName) == false)
		{
			data = new NFmiQueryData(fileName, true); // t‰m‰ laittaa memorymappauksen p‰‰lle
		}
	}

	timer.StopTimer();
	if(data)
	{
		std::string logStr("Data file: '");
		logStr += fileName;
		logStr += "' reading lasted: ";
		logStr += NFmiValueString::GetStringWithMaxDecimalsSmartWay(timer.TimeDiffInMSeconds() / 1000, 0);
		logStr += ".";
		logStr += NFmiValueString::GetStringWithMaxDecimalsSmartWay(timer.TimeDiffInMSeconds() % 1000, 0);
		logStr += " s.";
        ::LogMessage(theAdapter, logStr, CatLog::Severity::Info, CatLog::Category::Editing);
	}

	return data;
}

void RemoveThunders(NFmiQueryData* theData, bool fDoMultiThread)
{
	if(theData)
	{
		boost::shared_ptr<NFmiFastQueryInfo> fastInfo(new NFmiFastQueryInfo(theData));
		NFmiTimeDescriptor times = fastInfo->TimeDescriptor();
		if(fastInfo->Param(kFmiProbabilityThunderstorm))
		{
			NFmiDataModifierSet mod(0);
			if(fDoMultiThread)
				::ModifyTimesLocationData_FullMT(fastInfo, &mod, times); // poistetaan ukkoset 
			else
				fastInfo->ModifyTimesLocationData(&mod, 0, times); // poistetaan ukkoset 
		}
	}
}

static bool UseLoadedDataAndLoadAccessoryData(TimeSerialModificationDataInterface& theAdapter, NFmiDataLoadingInfo* theDataLoadingInfo, NFmiQueryData *theLoadedQueryData, bool fRemoveThundersOnLoad, bool fDoMultiThread, bool fLoadedFromFile, bool& dataWasDeleted, std::string *possibleReadOnlyFileName = nullptr)
{
	if(theDataLoadingInfo && theAdapter.DataLoadingOK(theLoadedQueryData != nullptr))
	{
		//talletetaan ja ladataan uusi data, jos annettu startup latauksen yhteydest‰ oikea ladattu read-only datan tiedostopolku, k‰ytet‰‰n sit‰
		std::string newFileName = possibleReadOnlyFileName ? *possibleReadOnlyFileName : theDataLoadingInfo->NewFileName();
		theLoadedQueryData->First();
		if(fRemoveThundersOnLoad)
			::RemoveThunders(theLoadedQueryData, fDoMultiThread);
		theLoadedQueryData->First();
		theAdapter.AddQueryData(theLoadedQueryData, newFileName, "", NFmiInfoData::kEditable, "", fLoadedFromFile, dataWasDeleted);
		return true;
	}
	return false;
}

static bool WriteDataToFile(NFmiString fileName, NFmiQueryData& data)
{
	std::ofstream out(fileName, std::ios::binary);
	if(out)
	{
		data.UseBinaryStorage(true);
		out << data;
		out.close();
		return true;
	}
	return false;
}

static bool ContinueCreatingLoadedData(TimeSerialModificationDataInterface &theAdapter, NFmiQueryData* theLoadedQueryData, NFmiDataLoadingInfo* theLoadingInfo, bool fRemoveThundersOnLoad, bool fDoMultiThread, bool fLoadedFromFile, std::string* possibleReadOnlyFileName = nullptr)
{
	bool dataWasDeleted = false;
	bool status= ::UseLoadedDataAndLoadAccessoryData(theAdapter, theLoadingInfo, theLoadedQueryData, fRemoveThundersOnLoad, fDoMultiThread, fLoadedFromFile, dataWasDeleted, possibleReadOnlyFileName);
	if(dataWasDeleted)
		return false;

    ::CheckAndValidateAfterModifications(theAdapter, NFmiMetEditorTypes::kFmiDataLoading, false, NFmiMetEditorTypes::kFmiNoMask, kFmiLastParameter, fDoMultiThread, false);

	if(theAdapter.IsWorkingDataSaved())
	{
		//talletetaan uusi data
		NFmiString newFileName = theLoadingInfo->NewFileName();
		if((::WriteDataToFile(newFileName, *theLoadedQueryData) == false) && theAdapter.WarnIfCantSaveWorkingFile())
		{
			std::string logStr;
			logStr += "Unable to create working data file: \n";
			logStr += newFileName.CharPtr();
			logStr += "\nCheck if directory exist or that you have rights to write there.";
			theAdapter.DoMessageBox(logStr, "Problem storing working data!", FMI_MB_OK);
            ::LogMessage(theAdapter, logStr, CatLog::Severity::Error, CatLog::Category::Editing);
			// Ei aseteta status:ta false:ksi, ettei tule uusia virheilmoituksia!
		}
	}
	return status;
}

// jos ollaan viewMode tai startupload moodeissa, k‰ytet‰‰n t‰t‰ lataamaan data nopeasti ja huomaamattomasti
static bool SpeedLoadModeDataLoading(TimeSerialModificationDataInterface &theAdapter, int theProducer, NFmiDataLoadingInfo* theLoadingInfo, bool fDoMultiThread)
{
	bool status = false;
	std::string speedLoadFile(::GetViewModeSpeedLoadFile(theAdapter, theProducer, theLoadingInfo));

//	if(CheckIfSpeedLoadFileHasSimilaGridAsWanted(speedLoadFile)) // ei oikeastaan kiinnosta t‰ss‰ tilassa onko hila mik‰ hyv‰ns‰
	{
		NFmiQueryData* data = 0;
		try
		{
			data = ::ReadDataFromFile(theAdapter, speedLoadFile);
		}
		catch(...)
		{ // ei tehd‰ viel‰ mit‰‰n, t‰h‰n voisi lis‰t‰ lokitusta, tai messagebox-varoitus tms
		}

		if(data)
		{
			// Jos ladattu suoraan tiedosto read-onlyna, ei poisteta ukkosia, ja annetaan speedLoadFile parametrina, 
			// jotta oikeaa tiedostopolkua k‰ytet‰‰n, kun data laitetaan sis‰iseen qdata-tietokantaan.
			status = ::ContinueCreatingLoadedData(theAdapter, data, theLoadingInfo, false, fDoMultiThread, false, &speedLoadFile);
		}
	}
	return status;
}

static bool CheckIfSpeedLoadFileHasSimilaGridAsWanted(TimeSerialModificationDataInterface &theAdapter, const std::string &theLoadedFileName)
{
	if(theAdapter.PossibleUsedDataLoadingGrid() == 0)
		return true; // jos ei ollut m‰‰ritelty haluttua hilaa+projektiota, palauta true
	else
	{
		if(!theLoadedFileName.empty())
		{
			std::ifstream in(theLoadedFileName.c_str(), std::ios::binary);
			if(in)
			{
				NFmiQueryInfo info;
				in >> info;
				if(info.HPlaceDescriptor() == *theAdapter.PossibleUsedDataLoadingGrid())
					return true;
			}
		}
	}
	return false;
}

static bool SpeedLoadDBDataOnlyIfPossible(TimeSerialModificationDataInterface &theAdapter, int thePrimaryProducer, const std::string &theLoadedFile, std::vector<int>& theModelIndexVector, NFmiDataLoadingInfo* theLoadingInfo, bool fRemoveThundersOnLoad, bool fDoMultiThread)
{
	bool status = false;
	if(theAdapter.SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa TAI startUp-loading moodissa, voidaan lataus tehda helpommin
	{ // View-moodissa ei valiteta mistaan vaan ladataan primaari data sellaisenaan
		// koska view-moodissa ei lataus-dialogia avattu, pitaa ladattavan datan tiedoston nimi hakea toista kautta
		// vaihe 1: yritet‰‰n ladata prim‰‰ri dataa
		status = ::SpeedLoadModeDataLoading(theAdapter, thePrimaryProducer, theLoadingInfo, fDoMultiThread);
		if(status == false)
		{ // ei mennyt l‰pi prim‰‰ri datan latauksella, kokeillaan sitten l‰pi eri datat j‰rjestyksess‰ 3:sta 0:een (ei ole v‰li‰, vaikka yritet‰‰n lukea prim‰‰ri dataa uudestaan)
			for(int producerNro = 3; producerNro >= 0; producerNro--)
			{
				status = ::SpeedLoadModeDataLoading(theAdapter, producerNro, theLoadingInfo, fDoMultiThread);
				if(status)
					break;
			}
		}
		return status;
	}

	// etsit‰‰n 1. prim‰‰rituottajasta poikkeava, jos eri suuri kuin end, lˆytyi muitakin tuottajia ja pika latausta ei voida suorittaa
	if(thePrimaryProducer == 2 || thePrimaryProducer == 3) // 2 == oma tyˆdata ja 3 == virallinen db data
	{
        std::vector<int>::iterator pos = std::find_if(theModelIndexVector.begin(), theModelIndexVector.end(), [=](auto modelIndex) { return modelIndex != thePrimaryProducer; });
		if(pos == theModelIndexVector.end())
		{
			if(::CheckIfSpeedLoadFileHasSimilaGridAsWanted(theAdapter, theLoadedFile))
			{
				NFmiQueryData* data = 0;
				try
				{
					data = ::ReadDataFromFile(theAdapter, theLoadedFile);
				}
				catch(...)
				{ // ei tehd‰ viel‰ mit‰‰n, t‰h‰n voisi lis‰t‰ lokitusta, tai messagebox-varoitus tms
				}
				if(data)
				{
					if(!theAdapter.UseEditedDataParamDescriptor() || theAdapter.EditedDataParamDescriptor() == data->Info()->ParamDescriptor())
					{
						NFmiTimeDescriptor timeDescriptor(theLoadingInfo->MetEditorModeDataWCTR()->TimeDescriptor());
						if(timeDescriptor == data->Info()->TimeDescriptor()) // jos viel‰ ladattavat ajat ovat samat kuin ladattavassa datassa, ei tarvitse kuin ottaa data k‰yttˆˆn
						{
							// producer id:t tulevat tiedostosta ladatstusta datasta sellaisenaan
							status = ::ContinueCreatingLoadedData(theAdapter, data, theLoadingInfo, fRemoveThundersOnLoad, fDoMultiThread, false);
							status = true;
						}
						else
						{
							NFmiTimeDescriptor dataTimeDesc(data->Info()->TimeDescriptor());
							NFmiTimeDescriptor testTimeDesc(dataTimeDesc.GetIntersection(timeDescriptor.FirstTime(), timeDescriptor.LastTime()));
							if(timeDescriptor == testTimeDesc) // jos viel‰ ladattavat ajat ovat riitt‰v‰n samat kuin ladattavassa datassa, voidaan pikalataus data rakentaa
							{
								NFmiQueryData *data2 = NFmiQueryDataUtil::ExtractTimes(data, testTimeDesc);
								if(data2) // ExtractTimes hoiti samalla myos producer id:t
								{
									status = ::ContinueCreatingLoadedData(theAdapter, data2, theLoadingInfo, fRemoveThundersOnLoad, fDoMultiThread, false);
									status = true;
								}
							}
						}
					}
				}
			}
		}
	}
	return status;
}

static bool TryAutoStartUpLoad(TimeSerialModificationDataInterface &theAdapter, bool fDoMultiThread)
{
	bool status = false;
    ::LogMessage(theAdapter, "TryAutoStartUpLoad checking for data...", CatLog::Severity::Debug, CatLog::Category::Editing);
    CtrlViewUtils::FmiSmartMetEditingMode oldMode = theAdapter.SmartMetEditingMode();
	theAdapter.SmartMetEditingMode(CtrlViewUtils::kFmiEditingModeStartUpLoading, false); // ei muutet‰ asetusta setting:eihin asti (false), koska t‰m‰ on vain v‰liaikainen muutos, lopussa palautetaan arvo takaisin
	try
	{
		bool fRemoveThundersOnLoad = false; // auto start-upissa ei poistella turhaan ukkosia...
		NFmiDataLoadingInfo &dataLoadingInfo = theAdapter.GetUsedDataLoadingInfo();
		dataLoadingInfo.UpdateSelectedFileNamesVector();
		const std::vector<NFmiString>& selectedFileNames = dataLoadingInfo.SelectedFileNamesVector();
		if(dataLoadingInfo.MetEditorModeDataWCTR())
		{
			int primaryProducer = dataLoadingInfo.PrimaryProducerSetting();
			status = ::SpeedLoadDBDataOnlyIfPossible(theAdapter, primaryProducer, std::string(selectedFileNames[primaryProducer]), theAdapter.DataLoadingProducerIndexVector(), &dataLoadingInfo, fRemoveThundersOnLoad, fDoMultiThread);
			if(status && theAdapter.EditedInfo())
			{
				theAdapter.EditedDataNeedsToBeLoaded(false);
                ::LogMessage(theAdapter, "TryAutoStartUpLoad found some data.", CatLog::Severity::Debug, CatLog::Category::Editing);
				if(theAdapter.IsEditedDataInReadOnlyMode())
					theAdapter.PutWarningFlagTimerOn();
			}
		}
	}
	catch(...)
	{
	}
	theAdapter.SmartMetEditingMode(oldMode, false);
	return status;
}

static QueryDataVector LoadAndCreateSelectedQueryData(TimeSerialModificationDataInterface &theAdapter, const std::vector<NFmiString>& theSelectedFileNames)
{
	QueryDataVector dataVector;
	int size = static_cast<int>(theSelectedFileNames.size());
	for(int i=0; i<size; i++)
	{
		std::string fileName(theSelectedFileNames[i]);
		try
		{
			dataVector.emplace_back(std::unique_ptr<NFmiQueryData>(::ReadDataFromFile(theAdapter, fileName)));
		}
		catch(std::exception &e)
		{
            ::LogMessage(theAdapter, std::string("Coudn't load data file '") + fileName + "' when building edited data.\n" + e.what(), CatLog::Severity::Error, CatLog::Category::Editing);
			// Pakko laittaa tyhj‰ listaan, jatko olettaa ett‰ sellainen lˆytyy
			dataVector.push_back(nullptr);
		}
		catch(...)
		{
            ::LogMessage(theAdapter, std::string("Coudn't load data file '") + fileName + "' when building edited data.", CatLog::Severity::Error, CatLog::Category::Editing);
			// Pakko laittaa tyhj‰ listaan, jatko olettaa ett‰ sellainen lˆytyy
			dataVector.push_back(nullptr);
		}
	}
	return dataVector;
}

static FastInfoVector CreateQueryDataIterators(const QueryDataVector& theDatas)
{
	FastInfoVector fastInfos;
	int size = static_cast<int>(theDatas.size());
	for(int i=0; i<size; i++)
	{
		if(theDatas[i])
		{
			fastInfos.emplace_back(std::make_unique<NFmiFastQueryInfo>(theDatas[i].get()));
		}
		else
			fastInfos.push_back(nullptr);
	}
	return fastInfos;
}

static bool FillDataWithSourceDataWCTR(FastInfoVector & theSourceInfos, std::vector<int> & theModelIndexVector, NFmiQueryData* theDestData, bool fDoMultiThread)
{
	if(theModelIndexVector.size() && theDestData)
	{
		// Katso yhten‰isi‰ tuottaja p‰tki‰ theModelIndexVector:ista, ota p‰tkist‰ aina alku ja loppu indeksi.
		unsigned long startTimeIndex = 0; //gMissingIndex;
		unsigned long endTimeIndex = gMissingIndex;
		int currentProducerIndex = theModelIndexVector[0];
		size_t i = 0;
		for(;;) // iki-looppi
		{
			for(i = endTimeIndex+1; i < theModelIndexVector.size(); i++)
			{
				if(currentProducerIndex != theModelIndexVector[i])
					break;
			}
			endTimeIndex = static_cast<unsigned long>(i-1);
			if(currentProducerIndex >= 0 && currentProducerIndex < static_cast<int>(theSourceInfos.size()))
			{
				auto &sourceInfo = theSourceInfos[currentProducerIndex];
				if(sourceInfo && sourceInfo->RefQueryData())
				{
					if(fDoMultiThread)
						NFmiQueryDataUtil::FillGridDataFullMT(const_cast<NFmiQueryData*>(sourceInfo->RefQueryData()), theDestData, startTimeIndex, endTimeIndex, nullptr);
					else
						NFmiQueryDataUtil::FillGridData(const_cast<NFmiQueryData*>(sourceInfo->RefQueryData()), theDestData, startTimeIndex, endTimeIndex, nullptr, true);
				}
			}
			if(i >= theModelIndexVector.size())
				break;
			startTimeIndex = endTimeIndex+1;
			currentProducerIndex = theModelIndexVector[startTimeIndex];
		}
		return true;
	}
	return false;
}

float BlendValues(float value1, float value2, float factor, FmiParameterName parId , double infoVersion)
{
	bool isCombinedParam = ((parId == kFmiTotalWindMS) || (parId == kFmiWeatherAndCloudiness));
	if(isCombinedParam)
	{
		if(value1 == kTCombinedWeatherFloatMissing && value2 == kTCombinedWeatherFloatMissing)
			return kTCombinedWeatherFloatMissing;
		else if(value1 == kTCombinedWeatherFloatMissing)
			return value2;
		else if(value2 == kTCombinedWeatherFloatMissing)
			return value1;
		else
		{
			if(parId == kFmiTotalWindMS)
			{
				NFmiTotalWind tempWind1(value1, kFmiPackedWind, infoVersion);
				NFmiTotalWind tempWind2(value2, kFmiPackedWind, infoVersion);
				NFmiTotalWind resultWind;
				resultWind.SetToWeightedMean(&tempWind1, 1-factor,
											 &tempWind2, factor,
											 &tempWind1, 0,
											 &tempWind1, 0);
				return resultWind.TransformedFloatValue();
			}
			else
			{
				NFmiWeatherAndCloudiness tempWaC1(value1, kFmiPackedWeather, kFloatMissing, infoVersion);
				NFmiWeatherAndCloudiness tempWaC2(value2, kFmiPackedWeather, kFloatMissing, infoVersion);
				NFmiWeatherAndCloudiness resultWaC;
				resultWaC.SetToWeightedMean(&tempWaC1, 1-factor,
											 &tempWaC2, factor,
											 &tempWaC1, 0,
											 &tempWaC1, 0);
				return resultWaC.TransformedFloatValue();
			}
		}
	}
	else
	{
		if(value1 == kFloatMissing && value2 == kFloatMissing)
			return kFloatMissing;
		else if(value1 == kFloatMissing)
			return value2;
		else if(value2 == kFloatMissing)
			return value1;
		else
		{
			float finalValue = (1.f - factor)*value1 + factor * value2;
			return finalValue;
		}
	}
}

// T‰‰ll‰ siis blendataan kaksi mallia (prim‰‰ri ja secund‰‰ri) halutulla aikav‰lill‰.
// Blendi‰ ei tehd‰, jos prim‰‰ri ja sekund‰‰ri ovat samoja, tai jos kaikilla aika-askelilla
// theModelIndexVector:iss‰ on sama arvo ja jos kaikilla paitsi viimeisell‰ aika-askeleella on
// sama arvo. N‰ill‰ tarkistuksilla yritet‰‰n est‰‰ tahaton blendaus.
void DoDataLoadingBlending(TimeSerialModificationDataInterface &theAdapter, FastInfoVector & theSourceInfos, std::vector<int> & theModelIndexVector, NFmiQueryData* theDestData, NFmiDataLoadingInfo* theLoadingInfo)
{
	if(theAdapter.ModelDataBlender().Use() == false)
		return ;

	if(theLoadingInfo->PrimaryProducerSetting() == theLoadingInfo->SecondaryProducerSetting())
		return ;
	int firstModelIndex = theModelIndexVector[0];
	size_t k = 1;
	for( ; k < theModelIndexVector.size()-1; k++) // looppi toisesta toiseksi viimeiseen indeksiin...
	{
		if(firstModelIndex != theModelIndexVector[k])
			break;
	}
	if(k >= theModelIndexVector.size()-1) // jos siis mentiin looppi l‰pi ilman break:ia eli kaikki oli samaa mallia (paitsi viimeinen aika-askel)
		return  ; // niin lopetetaan...

	auto &info1 = theSourceInfos[theLoadingInfo->PrimaryProducerSetting()];
	auto& info2 = theSourceInfos[theLoadingInfo->SecondaryProducerSetting()];
	if(info1 && info2) // niin ja primaarui ja sekond‰‰ri datat pit‰‰ lˆyty‰, muuten ei blendi‰
	{
		info1->First();
		info2->First();
		NFmiFastQueryInfo destInfo(theDestData);
		double infoVersion = destInfo.InfoVersion();
		NFmiModelDataBlender &modelDataBlender = theAdapter.ModelDataBlender();
		for(size_t i = modelDataBlender.GetStarTimeIndex(); i <= modelDataBlender.GetEndTimeIndex(); i++)
		{ // k‰yd‰‰n blendaus ajat l‰pi
			destInfo.TimeIndex(static_cast<unsigned long>(i));
//			for(level)  // HUOM! oletus viel‰ nyt ett‰ ei k‰yd‰ leveleit‰ l‰pi!!!! Nyt levelit osoittavat kaikissa infoissa ensimm‰iseen.
			for(destInfo.ResetParam(); destInfo.NextParam(); )
			{
				FmiParameterName parId = static_cast<FmiParameterName>(destInfo.Param().GetParamIdent());
				info1->Param(parId);
				info2->Param(parId);
				for(destInfo.ResetLocation(); destInfo.NextLocation(); )
				{
					// tehd‰‰n viel‰ ilman optimointia eli aika ja paikka interpolointi aina
					NFmiPoint latlon = destInfo.LatLon();
					float value1 = info1->InterpolatedValue(latlon, destInfo.Time(), 360);
					float value2 = info2->InterpolatedValue(latlon, destInfo.Time(), 360);
					destInfo.FloatValue(BlendValues(value1, value2, modelDataBlender.GetBlendFactor(i), parId, infoVersion));
				}
			}
		}
	}
}

static bool GetProducerIdsLister(NFmiQueryInfo *theInfo, NFmiProducerIdLister &theProducerIdsLister)
{
	if(theInfo)
		if(theInfo->FindFirstKey(NFmiQueryDataUtil::GetOfficialQueryDataProdIdsKey()))
			return theProducerIdsLister.IntepretProducerIdString(std::string(theInfo->GetCurrentKeyValue()));
	return false;
}

// Tekee producerIdListan ladatun datan mukaisesti.
// Tallettaa sen datan infon headeriin ProdIds-avaimen alle
void PutProducerIdListInDataHeader(TimeSerialModificationDataInterface &theAdapter, NFmiQueryData *data, FastInfoVector &theSourceInfos, std::vector<int> &theModelIndexVector)
{
	NFmiFastQueryInfo fastInfo(data);
	NFmiProducerIdLister workingDataIds;
	bool workingDataIdsExist = ::GetProducerIdsLister(theSourceInfos[2].get(), workingDataIds); // 2=working data indeksi
	NFmiProducerIdLister officialDataIds;
	bool officialDataIdsExist = ::GetProducerIdsLister(theSourceInfos[3].get(), officialDataIds); // 3=virallinen data indeksi
	NFmiProducerIdLister prodIdLister(fastInfo.TimeDescriptor(), -1);
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
			if(fastInfo.TimeIndex(i))
			{
				if(theModelIndexVector[i] == 2)
				{
					if(workingDataIdsExist)
					{
						prodIdLister.ProducerId(i, workingDataIds.ProducerId(fastInfo.Time(), true));
						prodIdLister.ModelOriginTime(i, workingDataIds.ModelOriginTime(fastInfo.Time(), true));
					}
				}
				else if(theModelIndexVector[i] == 3)
				{
					if(officialDataIdsExist)
					{
						prodIdLister.ProducerId(i, officialDataIds.ProducerId(fastInfo.Time(), true));
						prodIdLister.ModelOriginTime(i, officialDataIds.ModelOriginTime(fastInfo.Time(), true));
					}
				}
			}
		}
	}

	fastInfo.AddKey(NFmiQueryDataUtil::GetOfficialQueryDataProdIdsKey(), prodIdLister.MakeProducerIdString(), true);
	theAdapter.ProducerIdLister() = prodIdLister;

}

static bool CreateLoadedData(TimeSerialModificationDataInterface &theAdapter, FastInfoVector& theInfos, std::vector<int>& theModelIndexVector, NFmiDataLoadingInfo* theLoadingInfo, bool fRemoveThundersOnLoad, int thePrimaryProducer, bool fDoMultiThread)
{
	bool dataExist = false;
	int size = static_cast<int>(theInfos.size());
	int firstIndex = -1;
	for(int i=0; i<size; i++)
		if(theInfos[i] != 0 && firstIndex == -1) // jos edes 1 info lˆytyy, jatketaan
		{
			dataExist = true;
			firstIndex = i;
			break;
		}

	if(!dataExist)
		return false;
	auto &firstInfo = theInfos[firstIndex];

	//luodaan uusi data, jolla timedescriptor on loaddatainfosta
	if(!theLoadingInfo->MetEditorModeDataWCTR())
		return false;
	NFmiTimeDescriptor timeDescriptor(theLoadingInfo->MetEditorModeDataWCTR()->TimeDescriptor());

	NFmiParamDescriptor paramDescriptor(theAdapter.UseEditedDataParamDescriptor() ? theAdapter.EditedDataParamDescriptor() : firstInfo->ParamDescriptor());
	paramDescriptor.SetProducer(theLoadingInfo->Producer());

	NFmiQueryInfo tempQueryInfo(paramDescriptor, timeDescriptor, theAdapter.PossibleUsedDataLoadingGrid() ? *theAdapter.PossibleUsedDataLoadingGrid() : firstInfo->HPlaceDescriptor(), firstInfo->VPlaceDescriptor());
	double selectedInfoVersion = 7;
	if(theInfos[thePrimaryProducer])
		selectedInfoVersion = theInfos[thePrimaryProducer]->InfoVersion();
	else if(dataExist && theInfos[firstIndex])
		selectedInfoVersion = theInfos[firstIndex]->InfoVersion();

	tempQueryInfo.InfoVersion(selectedInfoVersion);
	NFmiQueryData* newData = NFmiQueryDataUtil::CreateEmptyData(tempQueryInfo);
	bool status = ::FillDataWithSourceDataWCTR(theInfos, theModelIndexVector, newData, fDoMultiThread);

	if(!status)
	{
		delete newData;
		return false;
	}

	// T‰h‰n v‰liin tehd‰‰n blendaus operaatio, jos tarpeen
	::DoDataLoadingBlending(theAdapter, theInfos, theModelIndexVector, newData, theLoadingInfo);

	::PutProducerIdListInDataHeader(theAdapter, newData, theInfos, theModelIndexVector);
	status &= ::ContinueCreatingLoadedData(theAdapter, newData, theLoadingInfo, fRemoveThundersOnLoad, fDoMultiThread, false);
	return status;
}

static bool LoadEditedData(TimeSerialModificationDataInterface& theAdapter, bool fRemoveThundersOnLoad, bool fDoMultiThread)
{
	if(theAdapter.SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeStartUpLoading)
		return ::TryAutoStartUpLoad(theAdapter, fDoMultiThread);

	NFmiMilliSecondTimer timer;
	timer.StartTimer();
	::LogMessage(theAdapter, "Loading data from selected model(s).", CatLog::Severity::Info, CatLog::Category::Editing);

	NFmiDataLoadingInfo& dataLoadingInfo = theAdapter.GetUsedDataLoadingInfo();
	dataLoadingInfo.UpdateSelectedFileNamesVector();
	const std::vector<NFmiString>& selectedFileNames = dataLoadingInfo.SelectedFileNamesVector();
	if(!dataLoadingInfo.MetEditorModeDataWCTR())
		return false;
	int primaryProducer = dataLoadingInfo.PrimaryProducerSetting();
	auto dataVector = ::LoadAndCreateSelectedQueryData(theAdapter, selectedFileNames);
	auto infos = ::CreateQueryDataIterators(dataVector);
	auto status = ::CreateLoadedData(theAdapter, infos, theAdapter.DataLoadingProducerIndexVector(), &dataLoadingInfo, fRemoveThundersOnLoad, primaryProducer, fDoMultiThread);

	if(!status)
	{
		if(theAdapter.SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeStartUpLoading)
		{
			theAdapter.DoDataLoadingProblemsDlg("Error occured GeneralDocImpl::LoadEditedData.");
		}
		return false;
	}
	timer.StopTimer();
	std::string timerStr("Editing base data loading lasted:");
	timerStr += timer.EasyTimeDiffStr();
	::LogMessage(theAdapter, timerStr, CatLog::Severity::Debug, CatLog::Category::Editing);

	return status;
}

static bool LoadData(TimeSerialModificationDataInterface &theAdapter, bool fRemoveThundersOnLoad, bool fDoMultiThread)
{
	bool status = false;
	try
	{
		status = ::LoadEditedData(theAdapter, fRemoveThundersOnLoad, fDoMultiThread);
		if(!status)
            ::LogMessage(theAdapter, "Loading the edited data failed.", CatLog::Severity::Error, CatLog::Category::Editing);
	}
	catch(std::exception &e)
	{
        ::LogMessage(theAdapter, std::string("Loading the edited data failed.\n") + e.what(), CatLog::Severity::Error, CatLog::Category::Editing);
	}
	return status;
}

// Pit‰‰ tehd‰ alustuksia laskuissa k‰ytetyn fastInfon ja datamatriisin v‰lill‰.
static void InitializeMacroParamData(const NFmiTimeDescriptor &theTimes, boost::shared_ptr<NFmiFastQueryInfo> &theMacroInfo, NFmiDataMatrix<float> &theValues, bool fCalcTooltipValue)
{
    if(!fCalcTooltipValue)
    {
        // laitetaan myˆs t‰m‰ matriisi aluksi puuttuvaksi, ett‰ sit‰ ei virhetilanteissa tarvitse erikseen s‰‰dell‰
        NFmiExtraMacroParamData::AdjustValueMatrixToMissing(theMacroInfo, theValues);
    }

	if(theMacroInfo)
	{
		theMacroInfo->First(); // asetetaan varmuuden vuoksi First:iin
		theMacroInfo->SetValues(theValues); // nollataan infossa ollut data missing-arvoilla, ett‰ saadaan puhdas kentt‰ laskuihin
		theMacroInfo->SetTimeDescriptor(theTimes); // asetetaan makroData-infon aikasysteemi currentin kartan kohtaan (feikki datassa vain yksi aika ja se pit‰‰ s‰‰t‰‰ kohdalleen, ett‰ laskut onnistuvat)
	}
}

typedef std::pair<int, int> GridPoint;

static bool GetGridPoint(boost::shared_ptr<NFmiFastQueryInfo> &theMacroInfo, GridPoint &theGridPoint)
{
    if(theMacroInfo->LocationIndex() != gMissingIndex)
    {
        theGridPoint.first = theMacroInfo->LocationIndex() % theMacroInfo->GridXNumber();
        theGridPoint.second = theMacroInfo->LocationIndex() / theMacroInfo->GridXNumber();
        return true;
    }

    return false;
}

void FmiModifyEditdData::InitializeSmartToolModifierForMacroParam(NFmiSmartToolModifier &theSmartToolModifier, TimeSerialModificationDataInterface& theAdapter, boost::shared_ptr<NFmiDrawParam>& theDrawParam, int theMapViewDescTopIndex, boost::shared_ptr<NFmiFastQueryInfo>& possibleSpacedOutMacroInfo, bool doProbing, const NFmiPoint& spaceOutSkipFactors)
{
	theSmartToolModifier.SetGriddingHelper(theAdapter.GetGriddingHelper());
	auto macroParamSystemPtr = theAdapter.MacroParamSystem();
	theSmartToolModifier.IncludeDirectory(macroParamSystemPtr->RootPath());

	std::string macroParamStr = CtrlViewUtils::GetMacroParamFormula(*macroParamSystemPtr, theDrawParam);
	theSmartToolModifier.InitSmartToolForMacroParam(macroParamStr, possibleSpacedOutMacroInfo, theAdapter.GetUsedMapViewArea(theMapViewDescTopIndex), doProbing, spaceOutSkipFactors);
}

static void SetMacroParamErrorMessage(const std::string &theErrorText, TimeSerialModificationDataInterface& theAdapter, NFmiExtraMacroParamData* possibleExtraMacroParamData)
{
	// Lokitetaan virheviesti
	::LogMessage(theAdapter, theErrorText, CatLog::Severity::Error, CatLog::Category::Macro);
	// Jos kyse toolpit laskuista, laitetaan viesti talteen ExtraMacroParamData:an, jotta viesti voidaan laittaa tooltippiin
	if(possibleExtraMacroParamData)
		possibleExtraMacroParamData->MacroParamErrorMessage(theErrorText);

	// talletetaan virheteksti aikaleimalla, ett‰ k‰ytt‰j‰ voi tarkastella sit‰ sitten smarttool dialogissa
	NFmiTime aTime;
	std::string timeString = aTime.ToStr("YYYY.MM.DD HH:mm:SS\n");
	auto dialogErrorString = timeString + theErrorText;
	theAdapter.SetLatestMacroParamErrorText(dialogErrorString);
	theAdapter.SetMacroErrorText(dialogErrorString);
}

static void SetupPossibleextraMacroParamData(NFmiExtraMacroParamData* possibleExtraMacroParamData, NFmiSmartToolModifier& smartToolModifier)
{
	if(possibleExtraMacroParamData)
	{
		*possibleExtraMacroParamData = smartToolModifier.ExtraMacroParamData();
	}
}

static float CalcMacroParamMatrix(TimeSerialModificationDataInterface &theAdapter, int theMapViewDescTopIndex, boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiDataMatrix<float> &theValues, bool fCalcTooltipValue, bool fDoMultiThread, const NFmiMetTime &theTime, const NFmiPoint &theTooltipLatlon, boost::shared_ptr<NFmiFastQueryInfo> &theUsedMacroInfoOut, bool &theUseCalculationPoints, boost::shared_ptr<NFmiFastQueryInfo> &possibleSpacedOutMacroInfo, NFmiExtraMacroParamData *possibleExtraMacroParamData, bool doProbing, const NFmiPoint& spaceOutSkipFactors)
{
	float value = kFloatMissing;
	NFmiSmartToolModifier smartToolModifier(theAdapter.InfoOrganizer());
	const auto& macroParamRootPath = theAdapter.MacroParamSystem()->RootPath();
	try // ensin tulkitaan macro
	{
		FmiModifyEditdData::InitializeSmartToolModifierForMacroParam(smartToolModifier, theAdapter, theDrawParam, theMapViewDescTopIndex, possibleSpacedOutMacroInfo, doProbing, spaceOutSkipFactors);
	}
	catch(std::exception &e)
	{
		std::string errorText = CtrlViewUtils::MakeMacroParamRelatedFinalErrorMessage("Error: Macro Parameter intepretion failed", &e, theDrawParam, macroParamRootPath);
		::SetMacroParamErrorMessage(errorText, theAdapter, possibleExtraMacroParamData);
		return value;
	}

	if(doProbing && !smartToolModifier.CalculationPoints().empty())
	{
		theUseCalculationPoints = true;
		SetupPossibleextraMacroParamData(possibleExtraMacroParamData, smartToolModifier);
		return value;
	}

	try // suoritetaan macro sitten
	{
		NFmiTimeBag validTimes(theTime, theTime, 60);
		NFmiTimeDescriptor times(theTime, validTimes); // luodaan 'feikki' timedescriptor juuri t‰lle kartan ajalle, joka lasketaan smarttool-systeemiss‰
        theUsedMacroInfoOut = smartToolModifier.UsedMacroParamData();
        ::InitializeMacroParamData(times, theUsedMacroInfoOut, theValues, fCalcTooltipValue);
		if(fCalcTooltipValue)
			value = smartToolModifier.CalcSmartToolValue(theTime, theTooltipLatlon); // false tarkoittaa ett‰ laskut tehd‰‰n kaikkiin pisteisiin eik‰ vain valittuihin pisteisiin
		else
		{
			if(fDoMultiThread)
			{
				smartToolModifier.ModifyData_ver2(&times, false, true, 0); // false tarkoittaa ett‰ laskut tehd‰‰n kaikkiin pisteisiin eik‰ vain valittuihin pisteisiin
			}
			else
			{
				smartToolModifier.ModifyData(&times, false, true, 0); // false tarkoittaa ett‰ laskut tehd‰‰n kaikkiin pisteisiin eik‰ vain valittuihin pisteisiin
			}
			TraceLogUsedSmartToolThreadCount(theAdapter, smartToolModifier.UsedThreadCounts());
		}

        if(!smartToolModifier.LastExceptionMessageFromThreads().empty())
            ::LogMessage(theAdapter, smartToolModifier.LastExceptionMessageFromThreads(), CatLog::Severity::Error, CatLog::Category::Editing);

		theUsedMacroInfoOut->First(); // asetetaan varmuuden vuoksi First:iin
		theUsedMacroInfoOut->Values(theValues);
        if(!smartToolModifier.CalculationPoints().empty())
            theUseCalculationPoints = true;
		SetupPossibleextraMacroParamData(possibleExtraMacroParamData, smartToolModifier);
	}
	catch(std::exception &e)
	{
		std::string errorText = CtrlViewUtils::MakeMacroParamRelatedFinalErrorMessage("Error: Macro Parameter calculation failed", &e, theDrawParam, macroParamRootPath);
		::SetMacroParamErrorMessage(errorText, theAdapter, possibleExtraMacroParamData);
	}
	catch(...)
	{
		std::string errorText = CtrlViewUtils::MakeMacroParamRelatedFinalErrorMessage("Error: Macro Parameter calculation failed: Unknown error!", nullptr, theDrawParam, macroParamRootPath);
		::SetMacroParamErrorMessage(errorText, theAdapter, possibleExtraMacroParamData);
	}
	return value;
}

float FmiModifyEditdData::CalcMacroParamMatrix(TimeSerialModificationDataInterface &theAdapter, int theMapViewDescTopIndex, boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiDataMatrix<float> &theValues, bool fCalcTooltipValue, bool fDoMultiThread, const NFmiMetTime &theTime, const NFmiPoint &theTooltipLatlon, boost::shared_ptr<NFmiFastQueryInfo> &theUsedMacroInfoOut, bool &theUseCalculationPoints, bool doProbing, const NFmiPoint& spaceOutSkipFactors, boost::shared_ptr<NFmiFastQueryInfo> possibleSpacedOutMacroInfo, NFmiExtraMacroParamData *possibleExtraMacroParamData)
{
    return ::CalcMacroParamMatrix(theAdapter, theMapViewDescTopIndex, theDrawParam, theValues, fCalcTooltipValue, fDoMultiThread, theTime, theTooltipLatlon, theUsedMacroInfoOut, theUseCalculationPoints, possibleSpacedOutMacroInfo, possibleExtraMacroParamData, doProbing, spaceOutSkipFactors);
}

static void SetForInfiniteValueCheck(TimeSerialModificationDataInterface &theAdapter)
{
	boost::shared_ptr<NFmiFastQueryInfo> editedInfo = theAdapter.EditedInfo();
	if(editedInfo)
		editedInfo->HasNonFiniteValueSet(false); // asetetaan t‰m‰ aina editointien aluksi false:ksi ett‰ editoinnin j‰lkeen tied‰mme onko kyseinen editointi yritt‰nyt laittaa inf/nan arvoja editoitavaan dataan
}

static void ReportError_InfiniteValueCheck(TimeSerialModificationDataInterface &theAdapter, const std::string &theFunctionNameStr)
{
	boost::shared_ptr<NFmiFastQueryInfo> editedInfo = theAdapter.EditedInfo();
	if(editedInfo)
	{
		if(editedInfo->HasNonFiniteValueSet())
		{
			std::string errorStr;
			errorStr += "In function ";
			errorStr += theFunctionNameStr;
			errorStr += " there was attempt to set inf/nan value to edited data (denied).";
            ::LogMessage(theAdapter, errorStr, CatLog::Severity::Error, CatLog::Category::Editing);
		}
	}
}

static void PrepareForProgressDialogAction(FmiModifyEditdData::ModifyFunctionParamHolder &theModifyFunctionParamHolder)
{
	theModifyFunctionParamHolder.itsAdapter.DataModificationInProgress(true);
	if(theModifyFunctionParamHolder.itsThreadCallBacks)
	{
		// threadien kanssa pit‰‰ kikkailla. t‰ll‰ on odotettava ett‰ p‰‰-threadissa saadaan alustettuaprogress-cancel 
		// dialogi, muuten liian aikaisista funktio kutsuista voi tulla fataaleja
		theModifyFunctionParamHolder.itsThreadCallBacks->WaitUntilInitialized();
	}
}

static void PostProcessForProgressDialogAction(FmiModifyEditdData::ModifyFunctionParamHolder &theModifyFunctionParamHolder, bool theStatus, unsigned int theMessage, bool fSetModificationInProogressStatus = true)
{
	theModifyFunctionParamHolder.fReturnValue = theStatus;
	if(theModifyFunctionParamHolder.itsThreadCallBacks)
		theModifyFunctionParamHolder.itsThreadCallBacks->DoPostMessage(theMessage); // t‰m‰ sulkee mahdollisen progress-dialogin
	if(fSetModificationInProogressStatus)
		theModifyFunctionParamHolder.itsAdapter.DataModificationInProgress(false);
}

// T‰m‰ tarkistaa onko k‰ynniss‰ datan modifikaatiota erillisess‰ s‰ikeess‰ (mm. kontrollipiste-muokkaus ja smarttool-muokkaus).
// Jos on, raportoi asiasta loggeriin ja palauttaa false:n (t‰llˆin t‰t‰ kutsuvan funktion pit‰isi lopettaa, eik‰ edet‰ datan muokkaukseen), 
// muuten palauttaa true:n.
static bool IsDataModificationInProgress(TimeSerialModificationDataInterface &theAdapter, const std::string &theCallingFunctionName)
{
	if(theAdapter.DataModificationInProgress())
	{
		std::string problemStr("Stopped doing the function ");
		problemStr += theCallingFunctionName;
		problemStr += " because there were some data modification in progress.";
        ::LogMessage(theAdapter, problemStr, CatLog::Severity::Warning, CatLog::Category::Editing);
		return true;
	}
	else
		return false;
}

bool FmiModifyEditdData::DoTimeSerialModifications(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiDrawParam> &theModifiedDrawParam, NFmiMetEditorTypes::Mask fUsedMask, NFmiTimeDescriptor& theTimeDescriptor, std::vector<float> &theModificationFactorCurvePoints, NFmiMetEditorTypes::FmiUsedSmartMetTool theEditorTool, bool fUseSetForDiscreteData, int theUnchangedValue, bool fDoMultiThread, NFmiThreadCallBacks *theThreadCallBacks)
{
    auto status = false;
    try
    {
        if(::IsDataModificationInProgress(theAdapter, __FUNCTION__))
            return false;
        ::SetForInfiniteValueCheck(theAdapter);
        status = ::DoTimeSeriesValuesModifying(theAdapter, theModifiedDrawParam, fUsedMask, theTimeDescriptor, theModificationFactorCurvePoints, theEditorTool, fUseSetForDiscreteData, theUnchangedValue, fDoMultiThread, theThreadCallBacks);
        ::ReportError_InfiniteValueCheck(theAdapter, __FUNCTION__);
    }
    catch(std::exception &e)
    {
        auto errorString = "Unable to do time-serial modifications: "s;
        errorString += e.what();
        ::LogMessage(theAdapter, errorString, CatLog::Severity::Error, CatLog::Category::Editing);
    }
	return status;
}

// N‰m‰ on oikeasti m‰‰ritelty SmartMet\SmartMetToolboxDep\SmartMetToolboxDep_resource.h -tiedostossa, mutta en halua t‰‰ll‰ riippuvuutta sinne
#define ID_MESSAGE_WORKING_THREAD_COMPLETED 33088
#define ID_MESSAGE_WORKING_THREAD_CANCELED 33089

void FmiModifyEditdData::DoTimeSerialModifications2(ModifyFunctionParamHolder &theModifyFunctionParamHolder, NFmiTimeDescriptor& theTimeDescriptor, std::vector<float> &theModificationFactorCurvePoints, bool fUseSetForDiscreteData, int theUnchangedValue)
{
	if(::IsDataModificationInProgress(theModifyFunctionParamHolder.itsAdapter, __FUNCTION__))
	{
		::PostProcessForProgressDialogAction(theModifyFunctionParamHolder, false, ID_MESSAGE_WORKING_THREAD_CANCELED, false);
		return ;
	}
	::SetForInfiniteValueCheck(theModifyFunctionParamHolder.itsAdapter);
	try
	{
		::PrepareForProgressDialogAction(theModifyFunctionParamHolder);
		bool status = ::DoTimeSeriesValuesModifying(theModifyFunctionParamHolder.itsAdapter, theModifyFunctionParamHolder.itsModifiedDrawParam, 
			theModifyFunctionParamHolder.fUsedMask, theTimeDescriptor, theModificationFactorCurvePoints, 
			theModifyFunctionParamHolder.itsEditorTool, fUseSetForDiscreteData, theUnchangedValue, 
			theModifyFunctionParamHolder.fDoMultiThread, theModifyFunctionParamHolder.itsThreadCallBacks);
		::PostProcessForProgressDialogAction(theModifyFunctionParamHolder, status, ID_MESSAGE_WORKING_THREAD_COMPLETED);
		::ReportError_InfiniteValueCheck(theModifyFunctionParamHolder.itsAdapter, __FUNCTION__);
		return ;
		// HUOM! ei saa menn‰ onnistuneessa ajossa t‰st‰ ohi, pit‰‰ olla return edell‰!
	}
	catch(std::exception &e)
	{
        ::LogMessage(theModifyFunctionParamHolder.itsAdapter, e.what(), CatLog::Severity::Error, CatLog::Category::Editing);
    }
	catch(...)
	{
	}
	::UndoData(theModifyFunctionParamHolder.itsAdapter); // toiminto on luultavasti keskeytetty k‰ytt‰j‰n toimesta, joten pit‰‰ tehd‰ undo-toiminto t‰ss‰ ja sitten palauttaa false
	::PostProcessForProgressDialogAction(theModifyFunctionParamHolder, false, ID_MESSAGE_WORKING_THREAD_CANCELED);
	::ReportError_InfiniteValueCheck(theModifyFunctionParamHolder.itsAdapter, std::string(__FUNCTION__) + " - canceled");
}

void FmiModifyEditdData::DoSmartToolEditing2(ModifyFunctionParamHolder &theModifyFunctionParamHolder, const std::string &theSmartToolText, const std::string &theRelativePathMacroName, bool fSelectedLocationsOnly)
{
	if(::IsDataModificationInProgress(theModifyFunctionParamHolder.itsAdapter, __FUNCTION__))
	{
		::PostProcessForProgressDialogAction(theModifyFunctionParamHolder, false, ID_MESSAGE_WORKING_THREAD_CANCELED, false);
		return ;
	}
	::SetForInfiniteValueCheck(theModifyFunctionParamHolder.itsAdapter);
	try
	{
		::PrepareForProgressDialogAction(theModifyFunctionParamHolder);
		bool status = ::DoSmartToolEditing(theModifyFunctionParamHolder.itsAdapter, theSmartToolText, theRelativePathMacroName, fSelectedLocationsOnly, theModifyFunctionParamHolder.fDoMultiThread, theModifyFunctionParamHolder.itsThreadCallBacks);
		::PostProcessForProgressDialogAction(theModifyFunctionParamHolder, status, ID_MESSAGE_WORKING_THREAD_COMPLETED);
		::ReportError_InfiniteValueCheck(theModifyFunctionParamHolder.itsAdapter, __FUNCTION__);
		return ;
		// HUOM! ei saa menn‰ onnistuneessa ajossa t‰st‰ ohi, pit‰‰ olla return edell‰!
	}
	catch(NFmiStopThreadException & )
	{
	}
	catch(...)
	{
	}
	::UndoData(theModifyFunctionParamHolder.itsAdapter); // toiminto on luultavasti keskeytetty k‰ytt‰j‰n toimesta, joten pit‰‰ tehd‰ undo-toiminto t‰ss‰ ja sitten palauttaa false
	::PostProcessForProgressDialogAction(theModifyFunctionParamHolder, false, ID_MESSAGE_WORKING_THREAD_CANCELED);
	::ReportError_InfiniteValueCheck(theModifyFunctionParamHolder.itsAdapter, std::string(__FUNCTION__) + " - canceled");
}

bool FmiModifyEditdData::DoAreaFiltering(TimeSerialModificationDataInterface &theAdapter, bool fDoMultiThread)
{
	if(::IsDataModificationInProgress(theAdapter, __FUNCTION__))
		return false;
	::SetForInfiniteValueCheck(theAdapter);
	bool status = ::DoAreaFiltering(theAdapter, fDoMultiThread);
	::ReportError_InfiniteValueCheck(theAdapter, __FUNCTION__);
	return status;
}

bool FmiModifyEditdData::DoTimeFiltering(TimeSerialModificationDataInterface &theAdapter, bool fDoMultiThread)
{
	if(::IsDataModificationInProgress(theAdapter, __FUNCTION__))
		return false;
	::SetForInfiniteValueCheck(theAdapter);
	bool status = ::DoTimeFiltering(theAdapter, fDoMultiThread);
	::ReportError_InfiniteValueCheck(theAdapter, __FUNCTION__);
	return status;
}

bool FmiModifyEditdData::DoCombineModelAndKlapse(TimeSerialModificationDataInterface &theAdapter, bool fDoMultiThread)
{
	if(::IsDataModificationInProgress(theAdapter, __FUNCTION__))
		return false;
	::SetForInfiniteValueCheck(theAdapter);
	bool status = ::DoCombineModelAndKlapse(theAdapter, fDoMultiThread);
	::ReportError_InfiniteValueCheck(theAdapter, __FUNCTION__);
	return status;
}

bool FmiModifyEditdData::StoreDataToDataBase(TimeSerialModificationDataInterface &theAdapter, const std::string &theForecasterId, const std::string &theHelperForecasterId, bool fDoMultiThread)
{
	if(::IsDataModificationInProgress(theAdapter, __FUNCTION__))
		return false;
	::SetForInfiniteValueCheck(theAdapter);
	bool status = ::StoreDataToDataBase(theAdapter, theForecasterId, theHelperForecasterId, fDoMultiThread);
	::ReportError_InfiniteValueCheck(theAdapter, __FUNCTION__);
	return status;
}

bool FmiModifyEditdData::LoadData(TimeSerialModificationDataInterface &theAdapter, bool fRemoveThundersOnLoad, bool fDoMultiThread)
{
	if(::IsDataModificationInProgress(theAdapter, __FUNCTION__))
		return false;
	::SetForInfiniteValueCheck(theAdapter);
	bool status = ::LoadData(theAdapter, fRemoveThundersOnLoad, fDoMultiThread);
	::ReportError_InfiniteValueCheck(theAdapter, __FUNCTION__);
	return status;
}

bool FmiModifyEditdData::TryAutoStartUpLoad(TimeSerialModificationDataInterface &theAdapter, bool fDoMultiThread)
{
	if(::IsDataModificationInProgress(theAdapter, __FUNCTION__))
		return false;
	::SetForInfiniteValueCheck(theAdapter);
	bool status = ::TryAutoStartUpLoad(theAdapter, fDoMultiThread);
	::ReportError_InfiniteValueCheck(theAdapter, __FUNCTION__);
	return status;
}

bool FmiModifyEditdData::CheckAndValidateAfterModifications(TimeSerialModificationDataInterface &theAdapter, NFmiMetEditorTypes::FmiUsedSmartMetTool theModifyingTool, bool fMakeDataSnapshotAction, unsigned int theLocationMask, FmiParameterName theParam, bool fDoMultiThread)
{
	if(::IsDataModificationInProgress(theAdapter, __FUNCTION__))
		return false;
	::SetForInfiniteValueCheck(theAdapter);
	bool status = ::CheckAndValidateAfterModifications(theAdapter, theModifyingTool, fMakeDataSnapshotAction, theLocationMask, theParam, fDoMultiThread);
	::ReportError_InfiniteValueCheck(theAdapter, __FUNCTION__);
	return status;
}

bool FmiModifyEditdData::MakeDataValiditation(TimeSerialModificationDataInterface &theAdapter, bool fDoMultiThread)
{
	if(::IsDataModificationInProgress(theAdapter, __FUNCTION__))
		return false;
	::SetForInfiniteValueCheck(theAdapter);
	bool status = ::MakeDataValiditation(theAdapter, fDoMultiThread);
	::ReportError_InfiniteValueCheck(theAdapter, __FUNCTION__);
	return status;
}

void FmiModifyEditdData::SnapShotData(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiDataIdent &theDataIdent, const std::string &theModificationText
									, const NFmiMetTime &theStartTime, const NFmiMetTime &theEndTime)
{
	if(::IsDataModificationInProgress(theAdapter, __FUNCTION__))
		return ;
	::SnapShotData(theAdapter, theInfo, theDataIdent, theModificationText, theStartTime, theEndTime);
}

bool FmiModifyEditdData::UndoData(TimeSerialModificationDataInterface &theAdapter)
{
	if(::IsDataModificationInProgress(theAdapter, __FUNCTION__))
		return false;
	return ::UndoData(theAdapter);
}

bool FmiModifyEditdData::RedoData(TimeSerialModificationDataInterface &theAdapter)
{
	if(::IsDataModificationInProgress(theAdapter, __FUNCTION__))
		return false;
	return ::RedoData(theAdapter);
}

std::string FmiModifyEditdData::GetModifiedParamsForLog(NFmiParamBag &modifiedParams)
{
    NFmiEnumConverter converter;
    std::string paramNames = "[";
    for(auto param : modifiedParams.ParamsVector())
    {
        if(!converter.ToString(param.GetParamIdent()).empty()) //ToString is used to figure out whether the parameter is actually an editable parameter.
        {
            if(param.GetParamName() != modifiedParams.GetFirst()->GetParamName())
                paramNames += ", ";
            paramNames += param.GetParamName();
        }
    }
    paramNames += "]";

    std::transform(paramNames.begin(), paramNames.end(), paramNames.begin(), ::toupper);
    return paramNames;
}

std::string FmiModifyEditdData::DataFilterToolsParamsForLog(TimeSerialModificationDataInterface &theAdapter)
{
    std::string desc;

    switch(theAdapter.FilteringParameterUsageState())
    {
        case 0: // Only active parameter
        {
            boost::shared_ptr<NFmiDrawParam> drawParam = theAdapter.ActiveDrawParamFromActiveRow(0);
            if(drawParam)
            {
                desc = drawParam->ParameterAbbreviation();
                return "[" + desc + "]";
            }
            else
                return "[No active param to modify]";
        }
        case 1: // All parameters
        {
            desc = "All parameters";
            return "[" + desc + "]";
        }
        case 2: // selected parameters
        {
            NFmiParamBag &filteringParamBag = theAdapter.FilteringParamBag();
            for(filteringParamBag.Reset(); filteringParamBag.Next(); )
            {
                if(!filteringParamBag.Current()->HasDataParams())
                {
                    if(filteringParamBag.Current()->IsActive()) // Get all active parameters
                        desc += filteringParamBag.Current()->GetParamName() + ", ";
                }
                else // Get also possible sub parameters (wind)
                {
                    NFmiParamBag subParamBag(*filteringParamBag.Current()->GetDataParams());
                    for(subParamBag.Reset(); subParamBag.Next(); )
                    {
                        if(subParamBag.Current()->IsActive())
                        {
                            desc += subParamBag.Current()->GetParamName() + ", ";
                        }
                    }
                }
            }
            desc = desc.substr(0, desc.size() - 2);
            return "[" + desc + "]";
        }
    }
    return "";
}
