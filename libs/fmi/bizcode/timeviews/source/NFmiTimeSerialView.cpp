#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ k‰‰nt‰j‰n varoitusta
#endif

#include "NFmiTimeSerialView.h"
#include "NFmiAxisView.h"
#include "NFmiTimeScaleView.h"
#include "NFmiDrawParam.h"
#include "NFmiSmartInfo.h"
#include "NFmiAreaMaskList.h"
#include "NFmiDrawParamList.h"
#include "NFmiAxisViewWithMinFontSize.h"
#include "NFmiMetEditorOptionsData.h"
#include "NFmiToolBox.h"
#include "NFmiLine.h"
#include "NFmiValueString.h"
#include "NFmiRectangle.h"
#include "NFmiText.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiTimeScale.h"
#include "NFmiAxis.h"
#include "NFmiProducerName.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiProducerSystem.h"
#include "NFmiHelpEditorSystem.h"
#include "NFmiDataModifierMinMax.h"
#include "NFmiColorSpaces.h"
#include "NFmiAnalyzeToolData.h"
#include "CtrlViewDocumentInterface.h"
#include "CtrlViewFastInfoFunctions.h"
#include "CtrlViewFunctions.h"
#include "NFmiEditorControlPointManager.h"
#include "SpecialDesctopIndex.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "CtrlViewKeyboardFunctions.h"
#include "NFmiStepTimeScale.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "NFmiStation.h"
#include "EditedInfoMaskHandler.h"
#include "ToolBoxStateRestorer.h"
#include "NFmiLimitChecker.h"
#include "ToolBoxStateRestorer.h"
#include "NFmiFastInfoUtils.h"
#include "catlog/catlog.h"
#include "TimeSerialParameters.h"
#include "NFmiFastInfoUtils.h"
#include "ColorStringFunctions.h"
#include "NFmiSeaLevelPlumeData.h"
#include "NFmiExtraMacroParamData.h"
#include "NFmiSmartToolModifier.h"
#include "NFmiSmartToolInfo.h"
#include "NFmiMacroParamSystem.h"
#include "NFmiMacroParam.h"
#include "TimeSerialModificationDataInterface.h"

#include "boost\math\special_functions\round.hpp"

static boost::shared_ptr<NFmiAreaMaskList> classesEmptyParamMaskList(new NFmiAreaMaskList()); // jos k‰ytt‰j‰ ei ole halunnut k‰ytt‰‰ maskeja laskuissaan, k‰ytet‰‰n t‰t‰ listaa todellisen maskilistan sijasta

static int itsModifyingUnit; // ainakin v‰liaikaisesti staattisena t‰‰ll‰ (optimointia change valueta laskettaessa)

bool fCPHelpColorsInitialized = false;
std::vector<NFmiColor> gCPHelpColors; // kun piirret‰‰n muita kuin aktiivista CP:t‰, k‰ytet‰‰n n‰it‰ v‰rej‰
const int gMaxHelpCPDrawed = 20;

const double gDontDrawLineValue = -99999.9; // funktio DrawSimpleDataVectorInTimeSerial ei piirr‰ kyseisell‰ arvolla viivaa

const NFmiColor g_OfficialDataColor(0.78f, 0.082f, 0.52f); // viinin punainen kepa-datasta

using namespace std;

class FastInfoTimeLimits
{
public:
	FastInfoTimeLimits() = default;
	FastInfoTimeLimits(unsigned long startTimeIndex, unsigned long endTimeIndex)
		:startTimeIndex_(startTimeIndex)
		, endTimeIndex_(endTimeIndex)
	{}

	FastInfoTimeLimits(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, const NFmiTimeBag& theLimitingTimes)
		:FastInfoTimeLimits()
	{
		auto usedStartTime = NFmiFastInfoUtils::GetUsedTimeIfModelClimatologyData(theInfo, theLimitingTimes.FirstTime());
		theInfo->FindNearestTime(usedStartTime, kBackward);
		startTimeIndex_ = theInfo->TimeIndex();
		if(startTimeIndex_ == -1)
		{
			if(theLimitingTimes.IsInside(theInfo->TimeDescriptor().FirstTime()))
				startTimeIndex_ = 0;
		}
		auto usedLastTime = NFmiFastInfoUtils::GetUsedTimeIfModelClimatologyData(theInfo, theLimitingTimes.LastTime());
		theInfo->FindNearestTime(usedLastTime, kForward);
		endTimeIndex_ = theInfo->TimeIndex();
		if(endTimeIndex_ == -1)
		{
			if(theLimitingTimes.IsInside(theInfo->TimeDescriptor().LastTime()))
				endTimeIndex_ = static_cast<unsigned long>(theInfo->SizeTimes()) - 1;
		}
	}

	bool isValid() const
	{
		return startTimeIndex_ != gMissingIndex && endTimeIndex_ != gMissingIndex;
	}

	unsigned long startTimeIndex_ = gMissingIndex;
	unsigned long endTimeIndex_ = gMissingIndex;
};

static boost::shared_ptr<NFmiFastQueryInfo> GetWantedData(CtrlViewDocumentInterface *theCtrlViewDocumentInterface, boost::shared_ptr<NFmiDrawParam> &theViewedDrawParam, const NFmiProducer &theWantedProducer, NFmiInfoData::Type theWantedType, const NFmiPoint* possibleLatlonPtr)
{
	bool useParamIdOnly = theWantedProducer.GetIdent() == 0;
	NFmiDataIdent dataIdent(*(theViewedDrawParam->Param().GetParam()), theWantedProducer);
	NFmiInfoData::Type usedType = theWantedType;
	if(usedType == NFmiInfoData::kViewable && theViewedDrawParam->DataType() == NFmiInfoData::kHybridData)
		usedType = NFmiInfoData::kHybridData;
	const NFmiLevel *level = nullptr;
	if(theViewedDrawParam->Level().LevelValue() != kFloatMissing)
		level = &theViewedDrawParam->Level();
    auto possibleComparisonParameters = theCtrlViewDocumentInterface->GetTimeSerialParameters().getComparisonParameters(static_cast<FmiParameterName>(dataIdent.GetParamIdent()));

	boost::shared_ptr<NFmiFastQueryInfo> wantedInfo;
	if(theWantedProducer.GetIdent() == kFmiSYNOP && possibleLatlonPtr)
	{
		NFmiMetTime dummyTime;
		NFmiLocation location(*possibleLatlonPtr);
		wantedInfo = theCtrlViewDocumentInterface->GetNearestSynopStationInfo(location, dummyTime, true, nullptr);
		if(wantedInfo)
		{
			wantedInfo->Param(static_cast<FmiParameterName>(dataIdent.GetParamIdent()));
		}
	}
	else
	{
		wantedInfo = theCtrlViewDocumentInterface->InfoOrganizer()->Info(dataIdent, level, usedType, useParamIdOnly, false, 0, possibleComparisonParameters);
	}

    if(!wantedInfo)
    {
		if(usedType == NFmiInfoData::kViewable) // jos ei lˆytynyt kViewable-tyypill‰, kokeillaan viel‰ kModelHelpData
			wantedInfo = ::GetWantedData(theCtrlViewDocumentInterface, theViewedDrawParam, theWantedProducer, NFmiInfoData::kModelHelpData, possibleLatlonPtr);
    }

	return wantedInfo;
}

static bool IsMosDataUsed(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo)
{
    if(theViewedInfo->Param().GetParamIdent() == kFmiTemperature && theViewedInfo->SizeLevels() == 1)
        return true;
    else
        return false;
}

static bool HasModelClimatologyDataAnyOfGivenParameters(boost::shared_ptr<NFmiFastQueryInfo>& modelClimatologyInfo, const ModelClimatology::ParamIds& paramIds)
{
	if(modelClimatologyInfo)
	{
		for(auto paramId : paramIds)
		{
			if(modelClimatologyInfo->Param(paramId))
				return true;
		}
	}
	return false;
}

//--------------------------------------------------------
// NFmiTimeSerialView
//--------------------------------------------------------
NFmiTimeSerialView::NFmiTimeSerialView(int theMapViewDescTopIndex, const NFmiRect &theRect
						 ,NFmiToolBox *theToolBox
						 ,NFmiDrawingEnvironment * theDrawingEnvi
						 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
						 ,int theRowIndex)
:NFmiTimeView(theMapViewDescTopIndex, theRect
			 ,theToolBox
			 ,theDrawingEnvi
			 ,theDrawParam
			 ,NFmiTimeDescriptor(NFmiMetTime(60), GetCtrlViewDocumentInterface()->TimeSerialViewTimeBag()) // t‰m‰ aika-alue annetaan aika-akselin tekoa varten
             ,theRowIndex)
,itsNormalCurveEnvi(NormalStationDataCurveEnvironment())
,itsChangeCurveEnvi(ChangeStationDataCurveEnvironment())
,itsIncrementalCurveEnvi(IncrementalStationDataCurveEnvironment())
,itsChanheIncrementalCurveEnvi(ChangeIncrementalStationDataCurveEnvironment())
,itsValueView(0)
,itsModifyFactorView(0)
,itsValueAxis(0)
,itsModifyFactorAxis(0)
,itsMaxStationShowed(5)
,itsSinAmplitude(0)
,itsPhase(6)
,itsProducerModelDataColors()
,itsOperationMode(TimeSerialOperationMode::NormalDrawMode)
,fEditingMouseMotionsAllowed(false)
{
	itsProducerModelDataColors.push_back(NFmiColor(0.5f, 0.25f, 0.f));
	itsProducerModelDataColors.push_back(NFmiColor(0.729f, 0.333f, 0.827f));
	itsProducerModelDataColors.push_back(NFmiColor(0.f, 0.67f, 0.67f));
	itsProducerModelDataColors.push_back(NFmiColor(1.f, 0.f, 0.5f));
	itsProducerModelDataColors.push_back(NFmiColor(.0f, 0.5f, 0.f));
    itsProducerModelDataColors.push_back(NFmiColor(.5f, .5f, 1.f));
    itsProducerModelDataColors.push_back(NFmiColor(.82f, .82f, 0.f));
    itsProducerModelDataColors.push_back(NFmiColor(0.f, 0.87f, 0.87f));

	itsModificationFactorCurvePoints.resize(EditedDataTimeDescriptor().Size());
	for(unsigned long i = 0; i < itsModificationFactorCurvePoints.size(); i++)
		itsModificationFactorCurvePoints[i] = 0.;

	if(!fCPHelpColorsInitialized)
	{
		fCPHelpColorsInitialized = true;
		gCPHelpColors.resize(gMaxHelpCPDrawed);
		gCPHelpColors[0] = NFmiColor(0.f, 0.f, 1.f); // blue
		gCPHelpColors[1] = NFmiColor(0.f, 1.f, 1.f); // cyan
		gCPHelpColors[2] = NFmiColor(1.f, 0.f, 0.f); // red
		gCPHelpColors[3] = NFmiColor(1.f, 0.f, 1.f); // magenta
		gCPHelpColors[4] = NFmiColor(0.f, 1.f, 0.f); // green
		gCPHelpColors[5] = NFmiColor(1.f, 0.5f, 0.f); // orange
		gCPHelpColors[6] = NFmiColor(0.3f, 0.3f, 0.3f); // dark cray
		gCPHelpColors[7] = NFmiColor(0.f, 0.6f, 0.0f); // dark green
		gCPHelpColors[8] = NFmiColor(1.f, 0.f, 0.6f); // violet red
		gCPHelpColors[9] = NFmiColor(0.f, 0.55f, 1.f); // sky blue
		gCPHelpColors[10] = NFmiColor(0.f, 0.1f, 0.8f);
		gCPHelpColors[11] = NFmiColor(0.3f, 0.2f, 0.2f);
		gCPHelpColors[12] = NFmiColor(0.7f, 0.3f, 0.1f);
		gCPHelpColors[13] = NFmiColor(0.9f, 0.4f, 0.3f);
		gCPHelpColors[14] = NFmiColor(0.2f, 0.5f, 0.6f);
		gCPHelpColors[15] = NFmiColor(0.9f, 0.6f, 0.8f);
		gCPHelpColors[16] = NFmiColor(0.7f, 0.7f, 0.9f);
		gCPHelpColors[17] = NFmiColor(0.2f, 0.8f, 0.3f);
		gCPHelpColors[18] = NFmiColor(0.5f, 0.9f, 0.6f);
		gCPHelpColors[19] = NFmiColor(0.2f, 0.4f, 0.8f); // indeksi gMaxHelpCPDrawed - 1 !!!!!!
	}
}
//--------------------------------------------------------
// ~NFmiTimeSerialView
//--------------------------------------------------------
NFmiTimeSerialView::~NFmiTimeSerialView(void)
{
	delete itsValueAxis;
	delete itsValueView;
	delete itsModifyFactorAxis;
	delete itsModifyFactorView;
}
//--------------------------------------------------------
// Draw
//--------------------------------------------------------
void NFmiTimeSerialView::Draw(NFmiToolBox* theToolBox)
{
    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, __FUNCTION__);
    try
	{
		DrawBackground();
		itsOperationMode = TimeSerialOperationMode::NormalDrawMode;
		ClearSideParameterNames();
		itsInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, false, true);
		if(itsInfo == 0)
			return;
        itsModifyingUnit = true;
		UpdateCachedParameterName();
		bool editedDataDrawed = IsEditedData(itsInfo);

		CreateModifyFactorScaleView(true, 7); // ei tarvitsisi tehd‰ aina piirrett‰ess‰
	// en keksinyt t‰lle parempaa paikkaa, mutta nyt akseli ja maksimi muutos vastaavat toisiaan
		itsDrawParam->TimeSerialModifyingLimit(itsModifyFactorAxis->EndValue());

		NFmiDrawingEnvironment envi;
		CreateValueScale();
		itsDataRect = CalculateDataRect();
        ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsDataRect);

		DrawNightShades();
		envi.SetFrameColor(NFmiColor(0.5f,0.5f,0.5f));
		envi.SetFillPattern(FMI_RELATIVE_FILL);
		envi.SetRelativeFill(0.2);
		envi.SetSubLinePerUnit(10); // envi will change in DrawGrids()

		DrawGrids(envi);
		DrawData();
		DrawValueAxis();
		if(editedDataDrawed)
			DrawModifyFactorAxis();
		DrawParamName();
		if(editedDataDrawed)
			DrawModifyingUnit();	// Piirt‰‰ aikasarjaikkunaan muokkausakselin yksikˆn.

		NFmiDrawingEnvironment dataRectEnvi;
		NFmiRectangle dataRectangle(itsDataRect, 0, &dataRectEnvi);
		itsToolBox->Convert(&dataRectangle);
	}
	catch(...)
	{
	}

	itsInfo = boost::shared_ptr<NFmiFastQueryInfo>();
}

NFmiRect NFmiTimeSerialView::CalculateDataRect(void)
{
    NFmiRect valueRect = CalcValueAxisRect();
	NFmiRect timeRect = CalcTimeAxisRect();
	NFmiRect dataRect(timeRect.Left(), valueRect.Top(), timeRect.Right(), valueRect.Bottom());
	return dataRect;
}

void NFmiTimeSerialView::DrawSideParametersDataLocationInTime(const NFmiPoint& theLatlon)
{
	auto viewRowSideParameters = itsCtrlViewDocumentInterface->GetCombinedMapHandlerInterface().getTimeSerialViewSideParameters(itsViewGridRowNumber);
	if(viewRowSideParameters && viewRowSideParameters->NumberOfItems() > 0)
	{
		// GeneralColor v‰rit alkavat 0:sta, ja 1. side-parameter on tarkoitus piirt‰‰ 2. v‰rill‰, jonka indeksi on siis 1.
		int sideParameterColorIndex = 1; 
		NFmiDrawingEnvironment envi;
		envi.SetPenSize(NFmiPoint(2, 2));
		for(viewRowSideParameters->Reset(); viewRowSideParameters->Next(); sideParameterColorIndex++)
		{
			envi.SetFrameColor(itsCtrlViewDocumentInterface->GeneralColor(sideParameterColorIndex));
			auto sideParamDrawParam = viewRowSideParameters->Current();
			if(sideParamDrawParam)
			{
				auto &producer = *sideParamDrawParam->Param().GetProducer();
				auto dataType = sideParamDrawParam->DataType();
				boost::shared_ptr<NFmiFastQueryInfo> sideParamInfo = ::GetWantedData(itsCtrlViewDocumentInterface, sideParamDrawParam, producer, dataType, &theLatlon);
				if(sideParamInfo)
				{
					if(itsOperationMode == TimeSerialOperationMode::NormalDrawMode)
						AddSideParameterNames(sideParamDrawParam, sideParamInfo);

					sideParamInfo->ResetTime(); // varmuuden vuoksi asetan 1. aikaan
					DrawSimpleDataInTimeSerial(GetViewLimitingTimes(), sideParamInfo, sideParamDrawParam, envi, theLatlon, NFmiPoint(6, 6)); // 0=ei siirret‰ aikasarjaa mihink‰‰n suuntaa piirrossa
				}
			}
		}
	}
}

void NFmiTimeSerialView::DrawHelperDataLocationInTime(const NFmiPoint &theLatlon)
{
    // Smartmet on kaatunut joskus mystisesti HESSAA symbolin kanssa, joten ei piirret‰, kun on erikoisn‰ytˆst‰ kyse
    if(!IsParamWeatherSymbol3())
    {
        if(itsCtrlViewDocumentInterface->IsOperationalModeOn())
        {
            if(itsCtrlViewDocumentInterface->ShowHelperData1InTimeSerialView())
            {
                NFmiDrawingEnvironment envi;
                DrawModelDataLocationInTime(envi, theLatlon);
                DrawHelpEditorDataLocationInTime(envi, theLatlon);
                DrawKepaDataLocationInTime(envi, theLatlon);
                DrawFraktiiliDataLocationInTime(envi, theLatlon);
            }
            DrawHelperData2LocationInTime(theLatlon);
            DrawHelperData3LocationInTime(theLatlon);
            DrawHelperData4LocationInTime(theLatlon);
        }
    }
}

void NFmiTimeSerialView::DrawHelpEditorDataLocationInTime(NFmiDrawingEnvironment &envi, const NFmiPoint &theLatlon)
{
	if(itsCtrlViewDocumentInterface->HelpEditorSystem().Use())
	{
		envi.SetFrameColor(itsCtrlViewDocumentInterface->HelpEditorSystem().HelpColor());
		DrawModelDataLocationInTime(envi, theLatlon, NFmiProducer(NFmiProducerSystem::gHelpEditorDataProdId, "helpdata"), NFmiInfoData::kEditingHelpData);
	}
}

void NFmiTimeSerialView::DrawHelperData2LocationInTime(const NFmiPoint &theLatlon)
{
	if(itsCtrlViewDocumentInterface->ShowHelperData2InTimeSerialView())
	{
        DrawModelFractileDataLocationInTime(theLatlon);
        DrawPossibleSeaLevelPlumeDataLocationInTime(theLatlon);
	}
}

void NFmiTimeSerialView::DrawHelperData3LocationInTime(const NFmiPoint &theLatlon)
{
    if(itsCtrlViewDocumentInterface->ShowHelperData3InTimeSerialView())
    {
        DrawAnnualModelFractileDataLocationInTime1(static_cast<FmiParameterName>(itsDrawParam->Param().GetParamIdent()), itsCtrlViewDocumentInterface->GetModelClimatologyData(itsDrawParam->Level()), theLatlon);
    }
}

void NFmiTimeSerialView::DrawHelperData4LocationInTime(const NFmiPoint &theLatlon)
{
    if(IsMosTemperatureMinAndMaxDisplayed(itsInfo))
    {
        DrawTemperatureMinAndMaxFromHelperData(static_cast<FmiParameterName>(itsDrawParam->Param().GetParamIdent()), itsCtrlViewDocumentInterface->GetMosTemperatureMinAndMaxData(), theLatlon);
    }
}

// I have to hard code all the used fractile param id's because they are not consistently structured.
// All fractile indices goes for each param like this: F100,F99,F97.5,F95,F87.5,F50,F12.5,F5,F2.5,F1,F0
// If there is range, param id's are consecutive, if list they are not.
//
//CAPEF100 4547 - 4557
//DewPointF100 4536 - 4546
//IceCoverF100 4558 - 4568
//MaximumTemperatureF100 4580 - 4590
//MinimumTemperatureF100 4591 - 4601
//Precipitation3hF100 946, 945, 4516, 943, 4517, 935, 4518, 923, 4519, 921, 920
//PressureF100 4613 - 4623
//SnowDepthF100 4569 - 4579
//TemperatureF100 871, 4500, 4501, 4502, 4503, 874, 4504, 4505, 4506, 4507, 877
//TemperatureSeaF100 4602 - 4612
//TotalCloudCoverF100 881, 4508, 4509, 4510, 4511, 884, 4512, 4513, 4514, 4515, 887
//TotalColumnWaterF100 4624 - 4634
//WindGustF100 1117, 4528, 4529, 4530, 4531, 1120, 4532, 4533, 4534, 4535, 1123
//WindSpeedF100 1110, 4520, 4521, 4522, 4523, 1113, 4524, 4525, 4526, 4527, 1116
namespace ModelClimatology
{
    ParamMap eraParamMap;
    std::vector<NFmiColor> eraColorsForFractiles;
    std::vector<std::string> eraFractileLabels;

    ParamIds MakeRangeParamIds(unsigned long range1, unsigned long range2)
    {
        ParamIds paramIds;
        for(auto i = range1; i <= range2; i++)
            paramIds.push_back(static_cast<FmiParameterName>(i));
        return paramIds;
    }

    void AddRangeParamToMap(ParamMap &paramMap, FmiParameterName mappedParam, unsigned long range1, unsigned long range2, const std::string &paramName)
    {
        ParamIds paramIds = MakeRangeParamIds(range1, range2);
        paramMap[mappedParam] = std::make_pair(paramName, paramIds);
    }

    void AddInitializerListToMap(ParamMap &paramMap, std::initializer_list<unsigned long> mappedParamList, std::initializer_list<unsigned long> paramIdList, const std::string &paramName)
    {
        // Add first all fractile param to vector
        ParamIds paramIds;
        for(auto paramId : paramIdList)
            paramIds.push_back(static_cast<FmiParameterName>(paramId));

        // Add then all mapped params to map with their fractile par-ids
        for(auto mappedParam : mappedParamList)
            paramMap[static_cast<FmiParameterName>(mappedParam)] = std::make_pair(paramName, paramIds);
    }

    void initFractileLabelss()
    {
        // There is allways 11 different fractile classes for ERA data (if that changes, code must be adjusted)
        eraFractileLabels.push_back("F100 ");
        eraFractileLabels.push_back("F97.5");
        eraFractileLabels.push_back("F90  ");
        eraFractileLabels.push_back("F50  ");
        eraFractileLabels.push_back("F10  ");
        eraFractileLabels.push_back("F2.5 ");
        eraFractileLabels.push_back("F0   ");
    }

    void initColorsForFractiles()
    {
        // There is allways 11 different fractile classes for ERA data (if that changes, code must be adjusted)

        // High fractiles (> 50%) are red (higher are brighter)
        eraColorsForFractiles.push_back(NFmiColor(1.f, 0.f, 0.f));
        eraColorsForFractiles.push_back(NFmiColor(1.f, 0.15f, 0.15f));
        eraColorsForFractiles.push_back(NFmiColor(1.f, 0.3f, 0.3f));
        eraColorsForFractiles.push_back(NFmiColor(1.f, 0.45f, 0.45f));
        eraColorsForFractiles.push_back(NFmiColor(1.f, 0.6f, 0.6f));
        // Mid fractile (50 %) is green
        eraColorsForFractiles.push_back(NFmiColor(0.f, 0.5f, 0.f));
        // Low fractiles (< 50 %) are blue (lower are 'colder')
        eraColorsForFractiles.push_back(NFmiColor(0.45f, 0.45f, 1.f));
        eraColorsForFractiles.push_back(NFmiColor(0.35f, 0.35f, 1.f));
        eraColorsForFractiles.push_back(NFmiColor(0.25f, 0.25f, 1.f));
        eraColorsForFractiles.push_back(NFmiColor(0.15f, 0.15f, 1.f));
        eraColorsForFractiles.push_back(NFmiColor(0.f, 0.f, 1.f));
    }

    void initEraParamMap()
    {
        static bool firstTime = true;
        if(!firstTime)
            return;
        firstTime = false;

        // Init also colors for different fractiles once
        initColorsForFractiles();
        // Init labels also
        initFractileLabelss();

        // add both dewpoint params for same param range
        //AddRangeParamToMap(eraParamMap, kFmiDewPoint, 4536, 4546, "Td");
        //AddRangeParamToMap(eraParamMap, kFmiDewPoint2M, 4536, 4546, "Td");
        //AddRangeParamToMap(eraParamMap, kFmiIceCover, 4558, 4568, "IceCover");
        //AddRangeParamToMap(eraParamMap, kFmiMaximumTemperature, 4580, 4590, "T-max");
        //AddRangeParamToMap(eraParamMap, kFmiMinimumTemperature, 4591, 4601, "T-min");
        //AddRangeParamToMap(eraParamMap, kFmiPressure, 4613, 4623, "P");
        //AddRangeParamToMap(eraParamMap, kFmiSnowDepth, 4569, 4579, "SnowDepth");
        //AddRangeParamToMap(eraParamMap, kFmiTemperatureSea, 4602, 4612, "T-sea");
        //AddRangeParamToMap(eraParamMap, kFmiTotalColumnWater, 4624, 4634, "TCW");

        // Both temperatures also for same params
        AddInitializerListToMap(eraParamMap, { kFmiTemperature, kFmiTemperature2M }, { 871, 4501, 872, 874, 876, 4506, 877 }, "T");
        // all possible wind gusts added for same params
        AddInitializerListToMap(eraParamMap, { kFmiWindGust, kFmiHourlyMaximumGust, kFmiWindGust2 }, { 1117, 4529, 1118, 1120 }, "WindGust");
        AddInitializerListToMap(eraParamMap, { kFmiWindSpeedMS }, { 1110, 4521, 1111, 1113 }, "WS");
		AddInitializerListToMap(eraParamMap, { kFmiCAPE}, { 4547, 4549, 4690, 4552 }, "CAPE");
		AddInitializerListToMap(eraParamMap, { kFmiTemperatureSea }, { 4602, 4604, 4686, 4607, 4689, 4610, 4612 }, "Temperature Sea");
	}
}

void NFmiTimeSerialView::DrawAnnualModelFractileDataLocationInTime1(FmiParameterName mainParameter, boost::shared_ptr<NFmiFastQueryInfo> &climateInfo, const NFmiPoint &theLatlon)
{
    if(climateInfo)
    {
        // make sure that param map is initialized
        ModelClimatology::initEraParamMap();
        auto paramMapIter = ModelClimatology::eraParamMap.find(mainParameter);
        if(paramMapIter != ModelClimatology::eraParamMap.end())
        {
			const auto& paramIds = paramMapIter->second.second;
			if(::HasModelClimatologyDataAnyOfGivenParameters(climateInfo, paramIds))
			{
				// T‰ss‰ annetaan overrideEnvi -parametrille nullptr, koska haluamme piirt‰‰ tietyill‰ tyylill‰ (ominaisuudet 
				// tulevat erilllisest‰ taulukosta) eri fraktiili parametrit.
				DrawAnnualModelFractileDataLocationInTime2(climateInfo, theLatlon, paramIds, nullptr);
			}
        }
    }
}

void NFmiTimeSerialView::DrawAnnualModelFractileDataLocationInTime2(boost::shared_ptr<NFmiFastQueryInfo> &climateInfo, const NFmiPoint &theLatlon, const ModelClimatology::ParamIds &paramIds, NFmiDrawingEnvironment* overrideEnvi)
{
    // Seek start and end time to be drawn so that year doesn't matter. climate data doesn't care about years.
    // There may be two separate time ranges that has to be handled. This happens when time range goes from one year 
    // to other and there is switch in years at new year's eve.
    // Also calculate the time shift so that climate data can be draw on this time range.
    auto climateDataYear = climateInfo->TimeDescriptor().FirstTime().GetYear();
    auto startTime = ZoomedTimeDescriptor().FirstTime();
    int startYearDiff = startTime.GetYear() - climateDataYear;
    // pit‰‰ laittaa etsint‰ aika climatologia datan vuoteen
    startTime.SetYear(climateDataYear);
    // etsit‰‰n l‰hin aika taaksep‰in
    if(climateInfo->FindNearestTime(startTime, kBackward))
    {
        auto startClimatologyDataTime = climateInfo->Time();

        auto endTime = ZoomedTimeDescriptor().LastTime();
        int endYearDiff = endTime.GetYear() - climateDataYear;
        // pit‰‰ laittaa etsint‰ aika climatologia datan vuoteen
        endTime.SetYear(climateDataYear);
        // etsit‰‰n l‰hin aika eteenp‰in
        if(climateInfo->FindNearestTime(endTime, kForward))
        {
            auto endClimatologyDataTime = climateInfo->Time();
            if(startYearDiff != endYearDiff)
            {
                // Let's split time range in two and draw them separate
                DrawAnnualModelFractileDataLocationInTime3(climateInfo, theLatlon, paramIds, startClimatologyDataTime, climateInfo->TimeDescriptor().LastTime(), startYearDiff, overrideEnvi);
                DrawAnnualModelFractileDataLocationInTime3(climateInfo, theLatlon, paramIds, climateInfo->TimeDescriptor().FirstTime(), endClimatologyDataTime, endYearDiff, overrideEnvi);
                // Also draw possible full years between start and end year
                for(auto yearIndex = startYearDiff + 1; yearIndex < endYearDiff; yearIndex++)
                    DrawAnnualModelFractileDataLocationInTime3(climateInfo, theLatlon, paramIds, climateInfo->TimeDescriptor().FirstTime(), climateInfo->TimeDescriptor().LastTime(), yearIndex, overrideEnvi);
            }
            else
            {
                DrawAnnualModelFractileDataLocationInTime3(climateInfo, theLatlon, paramIds, startClimatologyDataTime, endClimatologyDataTime, startYearDiff, overrideEnvi);
            }
        }
    }
}

void NFmiTimeSerialView::DrawAnnualModelFractileDataLocationInTime3(boost::shared_ptr<NFmiFastQueryInfo> &climateInfo, const NFmiPoint &theLatlon, const ModelClimatology::ParamIds &paramIds, const NFmiMetTime &startTime, const NFmiMetTime &endTime, int climateDataYearDifference, NFmiDrawingEnvironment* overrideEnvi)
{
    NFmiTimeBag drawedTimes(startTime, endTime, 60); // resoluutiolla ei merkityst‰
	auto viewStartTime = startTime;
	viewStartTime.SetYear(viewStartTime.GetYear() + climateDataYearDifference);
    int timeWhenDrawedInMinutes = viewStartTime.DifferenceInMinutes(startTime);
    DrawAnnualModelFractileDataLocationInTime4(climateInfo, theLatlon, paramIds, drawedTimes, timeWhenDrawedInMinutes, overrideEnvi);
}

void NFmiTimeSerialView::DrawAnnualModelFractileDataLocationInTime4(boost::shared_ptr<NFmiFastQueryInfo> &climateInfo, const NFmiPoint &theLatlon, const ModelClimatology::ParamIds &paramIds, const NFmiTimeBag &theDrawedTimes, int theTimeOffsetWhenDrawedInMinutes, NFmiDrawingEnvironment* overrideEnvi)
{
    NFmiDrawingEnvironment localEnvi;
	localEnvi.SetFillPattern(FMI_DOT);
	localEnvi.SetPenSize(NFmiPoint(1, 1));
	auto useOverrideEnvi = (overrideEnvi != nullptr);
	NFmiDrawingEnvironment& usedEnvi = useOverrideEnvi ? *overrideEnvi : localEnvi;
    for(size_t i = 0; i < paramIds.size(); i++)
    {
        if(climateInfo->Param(paramIds[i]))
        {
			if(!useOverrideEnvi)
			{
				// piirto-ominaisuuksia muutetllaan loopissa vain jos ei oltu annettu vakio optioita overrideEnvi parametrin mukana
	            usedEnvi.SetFrameColor(ModelClimatology::eraColorsForFractiles[i]);
			}
            DrawSimpleDataInTimeSerial(theDrawedTimes, climateInfo, itsDrawParam, usedEnvi, theLatlon, NFmiPoint(6, 6), true, theTimeOffsetWhenDrawedInMinutes);
        }
    }
}

void NFmiTimeSerialView::DrawTemperatureMinAndMaxFromHelperData(FmiParameterName mainParameter, boost::shared_ptr<NFmiFastQueryInfo>& helperDataInfo, const NFmiPoint& theLatlon)
{
	if(helperDataInfo)
	{
		float colorFactor = 0.3f;
		NFmiDrawingEnvironment envi;
		envi.SetFillPattern(FMI_DASHDOTDOT);
		envi.SetPenSize(NFmiPoint(1, 1));
		envi.SetFrameColor(NFmiColor(colorFactor, colorFactor, colorFactor * 2.5f));
		auto viewLimitTimeBag = GetViewLimitingTimes();
		if(helperDataInfo->Param(kFmiMinimumTemperature))
			DrawSimpleDataInTimeSerial(viewLimitTimeBag, helperDataInfo, itsDrawParam, envi, theLatlon, NFmiPoint(4, 4), true);
		envi.SetFrameColor(NFmiColor(colorFactor * 2.5f, colorFactor, colorFactor));
		if(helperDataInfo->Param(kFmiMaximumTemperature))
			DrawSimpleDataInTimeSerial(viewLimitTimeBag, helperDataInfo, itsDrawParam, envi, theLatlon, NFmiPoint(4, 4), true);
	}
}

void NFmiTimeSerialView::DrawModelDataLegend(const std::vector<NFmiColor> &theUsedColors, const std::vector<std::string> &theFoundProducerNames)
{
	if(itsOperationMode != TimeSerialOperationMode::NormalDrawMode)
		return ;
	itsToolBox->UseClipping(false);
	if(itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode() == false) // ei piirret‰ CP legendaa ja model legendaa yht‰ aikaa
	{
		NFmiDrawingEnvironment envi;
		envi.SetPenSize(NFmiPoint(2, 2));
		FmiDirection oldAligment = itsToolBox->GetTextAlignment();
		itsToolBox->SetTextAlignment(kBaseRight); // piirret‰‰n teksti vasemmalle textPoint:ista ja viiva oikealle

		NFmiPoint fontSize(16,16);
		envi.SetFontSize(fontSize);

		boost::shared_ptr<NFmiFastQueryInfo> info = Info();
		if(info)
		{
			NFmiRect frame(GetFrame());
			NFmiPoint textPoint(frame.TopLeft());
			textPoint.X(textPoint.X() + frame.Width()/7*6);
			double heightInc = itsToolBox->SY(15);
			double endPointX1 = textPoint.X() + frame.Width()/18*1;
			textPoint.Y(textPoint.Y() + heightInc);
			int currentLineIndex = 0;

			for(size_t i = 0; (i < theFoundProducerNames.size()) && (i < theUsedColors.size()); i++)
			{
				envi.SetFrameColor(theUsedColors[i]);
				NFmiText text(textPoint, theFoundProducerNames[i].c_str(), true, 0, &envi);
				itsToolBox->Convert(&text);

				NFmiLine line1(NFmiPoint(textPoint.X(), textPoint.Y() - heightInc/2.), NFmiPoint(endPointX1, textPoint.Y() - heightInc/2.), 0, &envi);
				itsToolBox->Convert(&line1);

				textPoint.Y(textPoint.Y() + heightInc);
			}

			// laitetaan tarvittaessa myˆs help editor data legend‰ n‰kyviin
			if(itsCtrlViewDocumentInterface->HelpEditorSystem().Use())
			{
				NFmiProducer prod(NFmiProducerSystem::gHelpEditorDataProdId, "helpdata");
                DrawExistingDataLegend(prod, NFmiInfoData::kEditingHelpData, itsDrawParam, itsCtrlViewDocumentInterface->HelpEditorSystem().HelpColor(), heightInc, endPointX1, textPoint, envi);
			}
            // Virallisen operatiivisen datan legenda
            NFmiProducer prod(0, "operational");
            DrawExistingDataLegend(prod, NFmiInfoData::kKepaData, itsDrawParam, g_OfficialDataColor, heightInc, endPointX1, textPoint, envi);
        }
		itsToolBox->SetTextAlignment(oldAligment);
	}
}

void NFmiTimeSerialView::DrawExistingDataLegend(const NFmiProducer &producer, NFmiInfoData::Type dataType, boost::shared_ptr<NFmiDrawParam> &drawParam, const NFmiColor &color, double heightIncrement, double endPointX, NFmiPoint &legendPlaceInOut, NFmiDrawingEnvironment &drawingEnvironmentInOut)
{
    boost::shared_ptr<NFmiFastQueryInfo> info = ::GetWantedData(itsCtrlViewDocumentInterface, drawParam, producer, dataType, nullptr);
    if(info)
    {
        drawingEnvironmentInOut.SetFrameColor(color);
        NFmiText text(legendPlaceInOut, producer.GetName(), true, 0, &drawingEnvironmentInOut);
        itsToolBox->Convert(&text);

        NFmiLine line1(NFmiPoint(legendPlaceInOut.X(), legendPlaceInOut.Y() - heightIncrement / 2.), NFmiPoint(endPointX, legendPlaceInOut.Y() - heightIncrement / 2.), 0, &drawingEnvironmentInOut);
        itsToolBox->Convert(&line1);

        legendPlaceInOut.Y(legendPlaceInOut.Y() + heightIncrement);
    }
}

void NFmiTimeSerialView::DrawModelDataLocationInTime(NFmiDrawingEnvironment &envi, const NFmiPoint &theLatlon)
{
	if(itsDrawParam->DataType() == NFmiInfoData::kHybridData)
		return ; // hybrid-datalle ei kannata piirt‰‰ apumallidatoja, koska hybrid-levelit eiv‰t vastaa eri malleissa toisiaan...
	envi.SetPenSize(NFmiPoint(1, 1));
	std::vector<NFmiProducerInfo> &producers = itsCtrlViewDocumentInterface->ProducerSystem().Producers();
	std::vector<string> foundProducerNames;
	int foundDataCounter = 0;
	for(unsigned int i=0; i < producers.size(); i++) // k‰yd‰‰n l‰pi kaikki tuottajat, ja katsotaan kuinka moneen p‰‰data osui (param + level ja tyyppi)
	{
		envi.SetFrameColor(itsProducerModelDataColors[foundDataCounter]);
        if(DrawModelDataLocationInTime(envi, theLatlon, producers[i].GetProducer())) // i+1 merkitsee ett‰ producersystemiss‰ on 1:ll‰ alkavat indeksit
		{
			foundProducerNames.push_back(producers[i].Name());
			foundDataCounter++;
		}
		if(foundProducerNames.size() >= itsProducerModelDataColors.size())
			break; // ei piirret‰ enemp‰‰ kuin on m‰‰ritelty apuv‰rej‰
	}
	DrawModelDataLegend(itsProducerModelDataColors, foundProducerNames);

	envi.SetPenSize(NFmiPoint(1, 1)); // laitetaan viel‰ varmuuden vuoksi ohut viiva takaisin
}

// Originaali parametria ei lˆytynyt, katsotaan lˆytyykˆ 'sijais' parametreja datasta
static bool DataHasComparisonParameter(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, FmiParameterName wantedParamId, CtrlViewDocumentInterface* ctrlViewDocumentInterface)
{
    auto timeSerialParameters = ctrlViewDocumentInterface->GetTimeSerialParameters().getComparisonParameters(wantedParamId);
    if(timeSerialParameters)
    {
        auto iter = std::find_if(timeSerialParameters->begin(), timeSerialParameters->end(), 
            [&](auto paramId) {return theInfo->Param(paramId); });

        if(iter != timeSerialParameters->end())
            return true;
    }

    return false;
}

static bool DataHasNeededParameters(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, unsigned long wantedParamId, CtrlViewDocumentInterface *ctrlViewDocumentInterface)
{
    NFmiFastInfoUtils::MetaWindParamUsage metaWindParamUsage = NFmiFastInfoUtils::CheckMetaWindParamUsage(theInfo);
    if(metaWindParamUsage.ParamNeedsMetaCalculations(wantedParamId))
        return true;
    else
    {
        auto wantedParam = static_cast<FmiParameterName>(wantedParamId);
        if(theInfo->Param(wantedParam))
            return true;
        else
            return ::DataHasComparisonParameter(theInfo, wantedParam, ctrlViewDocumentInterface);
    }
}

bool NFmiTimeSerialView::DrawModelDataLocationInTime(NFmiDrawingEnvironment &envi, const NFmiPoint &theLatlon, const NFmiProducer &theProducer, NFmiInfoData::Type theDataType)
{
	itsToolBox->UseClipping(true);
    boost::shared_ptr<NFmiFastQueryInfo> info = ::GetWantedData(itsCtrlViewDocumentInterface, itsDrawParam, theProducer, theDataType, &theLatlon);
	if(info)
	{
		info->ResetTime(); // varmuuden vuoksi asetan 1. aikaan
		{
			if(::DataHasNeededParameters(info, itsDrawParam->Param().GetParamIdent(), itsCtrlViewDocumentInterface)) // parametrikin pit‰‰ asettaa
			{
				DrawSimpleDataInTimeSerial(GetViewLimitingTimes(), info, itsDrawParam, envi, theLatlon, NFmiPoint(6, 6));
				return true;
			}
		}
	}
	return false;
}

void NFmiTimeSerialView::DrawKepaDataLocationInTime(NFmiDrawingEnvironment &envi, const NFmiPoint &theLatlon)
{
	NFmiProducer prod(0, "none"); // 0 producer ignoorataan GetWantedData-funktiossa
	boost::shared_ptr<NFmiFastQueryInfo> kepaInfo = ::GetWantedData(itsCtrlViewDocumentInterface, itsDrawParam, prod, NFmiInfoData::kKepaData, nullptr);
	if(kepaInfo)
	{
		envi.SetFrameColor(g_OfficialDataColor);
		envi.SetPenSize(NFmiPoint(2, 2));
		DrawSimpleDataInTimeSerial(GetViewLimitingTimes(), kepaInfo, itsDrawParam, envi, theLatlon, NFmiPoint(6, 6));
		envi.SetPenSize(NFmiPoint(1, 1)); // ohut viiva takaisin
	}
}

static pair<int, double> FindClosestLocationWithData(std::vector<pair<int, double> > &theNearestLocations, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiTimeBag& theCheckedTimes)
{
	int size = static_cast<int>(theNearestLocations.size());
	for(int i=0; i<size; i++)
	{
		if(theInfo->LocationIndex(theNearestLocations[i].first))
		{
            if(theInfo->FindNearestTime(theCheckedTimes.FirstTime(), kForward))
			{
                do
                {
                    if(theCheckedTimes.IsInside(theInfo->Time()))
				    {
					    if(theInfo->FloatValue() != kFloatMissing)
						    return theNearestLocations[i]; // heti kun jostain lˆytyy jotain muuta kuin puuttuvaa, palautetaan kyseinen paikka
				    }
                    else
                        break;
                } while(theInfo->NextTime());
			}
		}
	}
	return make_pair(-1, kFloatMissing);
}

bool NFmiTimeSerialView::SetObsDataToNearestLocationWhereIsData(boost::shared_ptr<NFmiFastQueryInfo> &theObsInfo, const NFmiPoint &theLatlon, std::pair<int, double> &theLocationWithDataOut)
{
    NFmiTimeBag checkedTimes(GetViewLimitingTimes()); // kiinnostaa vain n‰kyv‰n aika-alueen datat
	std::vector<pair<int, double> > nearestLocations = theObsInfo->NearestLocations(theLatlon, 5);
	theLocationWithDataOut = FindClosestLocationWithData(nearestLocations, theObsInfo, checkedTimes);
	bool drawJustLegend = theLocationWithDataOut.first == -1; // jos ei lˆytynyt asemaa, jolle on dataa, otetaan vain l‰hin asema ja piirret‰‰n sen nimi
	if(drawJustLegend)
	{
		if(nearestLocations.size() > 0)
			theObsInfo->LocationIndex(nearestLocations[0].first);
		else
			return false;
	}
	else
		theObsInfo->LocationIndex(theLocationWithDataOut.first);
	return true;
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiTimeSerialView::GetObservationInfo(const NFmiParam &theParam, const NFmiPoint &theLatlon)
{
	boost::shared_ptr<NFmiFastQueryInfo> obsInfo = itsCtrlViewDocumentInterface->GetNearestSynopStationInfo(NFmiLocation(theLatlon), NFmiMetTime(), true, 0);
    if(!obsInfo || ::DataHasNeededParameters(obsInfo, theParam.GetIdent(), itsCtrlViewDocumentInterface) == false)
        obsInfo = GetNonSynopObservation(theParam);
    return obsInfo;
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiTimeSerialView::GetNonSynopObservation(const NFmiParam &theParam)
{
    std::set<long> excludedProducerIds;
    excludedProducerIds.insert(kFmiSYNOP);
    excludedProducerIds.insert(kFmiTestBed);
    excludedProducerIds.insert(kFmiSHIP);
    excludedProducerIds.insert(kFmiBUOY);

    std::vector<boost::shared_ptr<NFmiFastQueryInfo> > obsInfos = itsCtrlViewDocumentInterface->InfoOrganizer()->GetInfos(NFmiInfoData::kObservations);
    for(size_t i= 0; i < obsInfos.size(); i++)
    {
        boost::shared_ptr<NFmiFastQueryInfo> &currentInfo = obsInfos[i];
        std::set<long>::iterator it = excludedProducerIds.find(currentInfo->Producer()->GetIdent());
        if(it == excludedProducerIds.end())
        {
            if(::DataHasNeededParameters(currentInfo, theParam.GetIdent(), itsCtrlViewDocumentInterface))
                return currentInfo;
        }
    }
    return boost::shared_ptr<NFmiFastQueryInfo>();
}

void NFmiTimeSerialView::DrawObservationDataLocationInTime(NFmiDrawingEnvironment& envi, const NFmiPoint& theLatlon)
{
	if(itsDrawParam->Level().LevelValue() != kFloatMissing)
		return; // havainto apu piirrot tehd‰‰n vain pintadatalle
	boost::shared_ptr<NFmiFastQueryInfo> obsInfo = GetObservationInfo(*itsDrawParam->Param().GetParam(), theLatlon);
	if(obsInfo)
	{
		obsInfo->FirstLevel(); // varmuuden vuoksi asetan 1. leveliin
		if(::DataHasNeededParameters(obsInfo, itsDrawParam->Param().GetParamIdent(), itsCtrlViewDocumentInterface)) // parametrikin pit‰‰ asettaa
		{
			std::pair<int, double> locationWithData;
			if(SetObsDataToNearestLocationWhereIsData(obsInfo, theLatlon, locationWithData))
			{
				const NFmiLocation* loc = obsInfo->Location();
				NFmiPoint place(CalcParamTextPosition());
				NFmiPoint fontSize(CalcFontSize());
				fontSize *= NFmiPoint(0.9, 0.9); // pienennet‰‰n fonttia hieman

				// pit‰‰ laske kuinka monta pistett‰ on piirretty aikasarjaan. Huom! v‰hint‰‰n yksi on piirretty, jos ollaan t‰‰ll‰!
				unsigned long displayCount = CtrlViewFastInfoFunctions::GetMaskedCount(itsCtrlViewDocumentInterface->EditedSmartInfo(), NFmiMetEditorTypes::kFmiDisplayedMask, itsCtrlViewDocumentInterface->AllowRightClickDisplaySelection());
				if(displayCount == 0)
					displayCount = 1;

				// displayCount+1 pit‰‰ laittaa kertoimeksi, koska myˆs havainto asema on piirretty
				place.Y(place.Y() + 0.9 * (displayCount + 1) * itsToolBox->SY(static_cast<long>(fontSize.Y())));
				NFmiString locString(::GetDictionaryString("TimeSerialViewObservationStation"));
				locString += " ";
				envi.SetFrameColor(NFmiColor(0, 0, 0));
				DrawStationNameLegend(loc, envi, fontSize, place, locString, kTop, locationWithData.second);

				envi.SetFrameColor(NFmiColor(0.956f, 0.282f, 0.05f)); // vihert‰v‰‰ havainto datasta
				envi.SetPenSize(NFmiPoint(3, 3)); // paksunnetaan viivaa
				DrawSimpleDataInTimeSerial(GetViewLimitingTimes(), obsInfo, itsDrawParam, envi, theLatlon, NFmiPoint(9, 9), true);
				envi.SetPenSize(NFmiPoint(1, 1)); // ohut viiva takaisin
			}
		}
	}
}

static bool CalcIntersection(const NFmiTimeBag & baseBag, const NFmiTimeDescriptor & theTimes2, NFmiTimeBag & refBag, bool resolOfTimes2)
{
	NFmiMetTime begTime(baseBag.FirstTime() > theTimes2.FirstTime() ? baseBag.FirstTime() : theTimes2.FirstTime());
	NFmiMetTime endTime(baseBag.LastTime() < theTimes2.LastTime() ? baseBag.LastTime() : theTimes2.LastTime());

	if(begTime > endTime)
		return false;

	if(resolOfTimes2)
		refBag = NFmiTimeBag(begTime, endTime, theTimes2.Resolution());
	else
		refBag = NFmiTimeBag(begTime, endTime, baseBag.Resolution());
	return true;
}


static const double gMaxDistanceToFractileStation = 500*1000; // fraktiili asemat n‰ytet‰‰n vain alle 500 km et‰isyydelt‰ osoitetusta paikasta

void NFmiTimeSerialView::DrawFraktiiliDataLocationInTime(NFmiDrawingEnvironment &envi, const NFmiPoint &theLatlon)
{
	if(itsOperationMode == TimeSerialOperationMode::MinMaxScanMode)
		return ; // havainto fraktiili datat skipataan scannauksessa

	if(itsDrawParam->Level().LevelValue() != kFloatMissing)
		return ; // havainto fraktiili apu piirrot tehd‰‰n vain pintadatalle
	if(Info()->Param().GetParamIdent() == kFmiTemperature) // fraktiileja lˆytyy vain l‰mpˆtilalle!!!
	{
		boost::shared_ptr<NFmiFastQueryInfo> fraktiiliInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->FindInfo(NFmiInfoData::kClimatologyData);
		if(fraktiiliInfo)
		{
			NFmiPoint penSize(2, 2);
			envi.SetPenSize(penSize);
			fraktiiliInfo->FirstLevel(); // varmuuden vuoksi asetan 1. leveliin
			NFmiTimeBag timesInView(itsCtrlViewDocumentInterface->TimeSerialViewTimeBag());
			if(timesInView.FirstTime().GetYear() == timesInView.LastTime().GetYear()) // pit‰‰ mietti se erikseen, miten hoidetaan kun aikaikkunan alku ja loppu ovat eri vuosilla (esim. vuoden vaihteessa)
			{
				auto firstViewTime = timesInView.FirstTime();
				int viewYear = timesInView.FirstTime().GetYear();
				NFmiTimeDescriptor fraktiiliTimes(fraktiiliInfo->TimeDescriptor());
				int fraktiiliDataYear = fraktiiliTimes.FirstTime().GetYear(); // fraktiili data on jollekin vuodelle tehty pˆtkˆ, otetaan vuosi talteen, ett‰ voidaan rakentaa sopiva timebagi datan l‰pik‰ymiseen
				auto similarFractileTime = firstViewTime;
				similarFractileTime.SetYear(fraktiiliDataYear);
				NFmiMetTime startTime(timesInView.FirstTime());
				startTime.SetYear(fraktiiliDataYear);
				NFmiMetTime endTime(timesInView.LastTime());
				endTime.SetYear(fraktiiliDataYear);
				NFmiTimeBag newTimesInView(startTime, endTime, timesInView.Resolution());
				NFmiTimeBag drawedTimes; // n‰m‰ ajat sitten piirret‰‰n, kunhan otetaan selville ensin mitk‰ ne ovat
				bool status = ::CalcIntersection(newTimesInView, fraktiiliTimes, drawedTimes, true);
				if(status)
				{
					// viel‰ pit‰‰ muokata timebagia, niin ett‰ tunnit menee 12:een
					NFmiMetTime st2(drawedTimes.FirstTime());
					fraktiiliInfo->TimeToNearestStep(st2, kCenter);
					int fraktileHour = fraktiiliInfo->Time().GetHour();
					st2.ChangeByDays(-1); // pit‰‰ vied‰ p‰iv‰ eteen ja sitten nollata
					st2.SetHour(fraktileHour);
					NFmiMetTime et2(drawedTimes.LastTime());
					et2.ChangeByDays(2); // pit‰‰ vied‰ kaksi p‰iv‰‰ eteen ja sitten nollata
					et2.SetHour(fraktileHour);
					drawedTimes = NFmiTimeBag(st2, et2, drawedTimes.Resolution());

					if(fraktiiliInfo->NearestPoint(theLatlon)) // asetetaan kepadata l‰himp‰‰n pisteeseen (t‰ss‰ kaupunkiin) piirtoa varten
					{
						int timeOffsetWhenDrawedInMinutes = firstViewTime.DifferenceInMinutes(similarFractileTime);
						const NFmiLocation *loc = fraktiiliInfo->Location();
						double distance = 999999999.;
						if(loc)
							distance = loc->Distance(theLatlon);
						if(distance < gMaxDistanceToFractileStation) // fraktiili tiedot vain jos tarpeeksi l‰helt‰ lˆytyi asema
						{
							NFmiPoint place(CalcParamTextPosition());
							NFmiPoint fontSize(CalcFontSize());
							fontSize *= NFmiPoint(0.9, 0.9); // pienennet‰‰n fonttia hieman

							// pit‰‰ laske kuinka monta pistett‰ on piirretty aikasarjaan. Huom! v‰hint‰‰n yksi on piirretty, jos ollaan t‰‰ll‰!
							unsigned long displayCount = CtrlViewFastInfoFunctions::GetMaskedCount(itsCtrlViewDocumentInterface->EditedSmartInfo(), NFmiMetEditorTypes::kFmiDisplayedMask, itsCtrlViewDocumentInterface->AllowRightClickDisplaySelection());
							if(displayCount == 0)
								displayCount = 1;

							// displayCount+2 pit‰‰ laittaa kertoimeksi, koska myˆs havainto asema on piirretty
							place.Y(place.Y() + (0.9*(displayCount+2) * itsToolBox->SY(static_cast<long>(fontSize.Y()))));
							std::string locString(::GetDictionaryString("TimeSerialViewClimatologyStation"));
							locString += " ";
							envi.SetFrameColor(NFmiColor(0, 0, 0));
							DrawStationNameLegend(loc, envi, fontSize, place, locString, kTop);

							if(fraktiiliInfo->Param(kFmiNormalMinTemperatureF02))
							{
								envi.SetFrameColor(NFmiColor(0.541f,0.54f,0.95f)); // 'minimin minimi' arvo vaalen siniseksi (koska v‰hemm‰n kiinnostava)
								DrawSimpleDataInTimeSerial(drawedTimes, fraktiiliInfo, itsDrawParam, envi, fraktiiliInfo->LatLon(), NFmiPoint(6, 6), false, timeOffsetWhenDrawedInMinutes);
							}
							if(fraktiiliInfo->Param(kFmiNormalMinTemperatureF50))
							{
								envi.SetFrameColor(NFmiColor(0.1f,0.f,0.95f)); // 'mean minimi' arvo siniseksi
								DrawSimpleDataInTimeSerial(drawedTimes, fraktiiliInfo, itsDrawParam, envi, fraktiiliInfo->LatLon(), NFmiPoint(6, 6), false, timeOffsetWhenDrawedInMinutes);
							}
							if(fraktiiliInfo->Param(kFmiNormalMeanTemperature))
							{
								envi.SetFrameColor(NFmiColor(0.1f,0.89f,0.15f)); // 'avg' arvo vihre‰ksi
								DrawSimpleDataInTimeSerial(drawedTimes, fraktiiliInfo, itsDrawParam, envi, fraktiiliInfo->LatLon(), NFmiPoint(6, 6), false, timeOffsetWhenDrawedInMinutes);
							}
							if(fraktiiliInfo->Param(kFmiNormalMaxTemperatureF50))
							{
								envi.SetFrameColor(NFmiColor(0.95f,0.1f,0.05f)); // 'mean maksimi' arvo punaiseksi
								DrawSimpleDataInTimeSerial(drawedTimes, fraktiiliInfo, itsDrawParam, envi, fraktiiliInfo->LatLon(), NFmiPoint(6, 6), false, timeOffsetWhenDrawedInMinutes);
							}
							if(fraktiiliInfo->Param(kFmiNormalMaxTemperatureF98))
							{
								envi.SetFrameColor(NFmiColor(0.95f,0.54f,0.54f)); // 'maksimin maksimi' arvo haalean punaiseksi, koska kiinnostaa v‰hemm‰n
								DrawSimpleDataInTimeSerial(drawedTimes, fraktiiliInfo, itsDrawParam, envi, fraktiiliInfo->LatLon(), NFmiPoint(6, 6), false, timeOffsetWhenDrawedInMinutes);
							}
						}
					}
				}
			}
		}
	}
}

void NFmiTimeSerialView::DrawParamInTime(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiDrawingEnvironment &theEnvi, const NFmiPoint &theLatlon, FmiParameterName theParam, const NFmiColor &theColor, const NFmiPoint &theEmptyPointSize)
{
	if(::DataHasNeededParameters(theInfo, theParam, itsCtrlViewDocumentInterface))
	{
		theEnvi.SetFrameColor(theColor);
		DrawSimpleDataInTimeSerial(GetViewLimitingTimes(), theInfo, itsDrawParam, theEnvi, theLatlon, theEmptyPointSize);
	}
}

// theParamIndexIncrement kertoo mihin suuntaan param-indeksi‰ juoksutetaan kun menn‰‰n F100:sta F0:aan.
// T‰m‰ siksi ett‰ kFmiTemperatureF100, kFmiTotalPrecipitationF100 ja kFmiTotalCloudCoverF100 -sarjat menev‰t F100:sta F0:aan,
// theParamIndexIncrement -parametri on j‰‰nne menneisyydest‰, jolloin tuulen nopeus parametrit meniv‰t toiseen suuntaan ja incrementiksi piti antaa -1,
// nyky‰‰n kaikki parametri menev‰t F100 -> F0:aan.
void NFmiTimeSerialView::DrawModelFractileDataLocationInTime(boost::shared_ptr<NFmiFastQueryInfo> &theFractileData, long theStartParamIndex, const NFmiPoint &theLatlon, long theParamIndexIncrement)
{
	NFmiDrawingEnvironment envi;
	envi.SetFillPattern(FMI_DASHDOTDOT);

	DrawParamInTime(theFractileData, envi, theLatlon, static_cast<FmiParameterName>(theStartParamIndex), NFmiColor(1.f, 0.f, 0.f), NFmiPoint(1, 1));
	DrawParamInTime(theFractileData, envi, theLatlon, static_cast<FmiParameterName>(theStartParamIndex += theParamIndexIncrement), NFmiColor(1.f, 0.25f, 0.25f), NFmiPoint(1, 1));
	DrawParamInTime(theFractileData, envi, theLatlon, static_cast<FmiParameterName>(theStartParamIndex += theParamIndexIncrement), NFmiColor(1.f, 0.5f, 0.5f), NFmiPoint(1, 1));
	DrawParamInTime(theFractileData, envi, theLatlon, static_cast<FmiParameterName>(theStartParamIndex += theParamIndexIncrement), NFmiColor(0.f, 0.5f, 0.f), NFmiPoint(1, 1));
	DrawParamInTime(theFractileData, envi, theLatlon, static_cast<FmiParameterName>(theStartParamIndex += theParamIndexIncrement), NFmiColor(0.4f, 0.4f, 1.f), NFmiPoint(1, 1));
	DrawParamInTime(theFractileData, envi, theLatlon, static_cast<FmiParameterName>(theStartParamIndex += theParamIndexIncrement), NFmiColor(0.25f, 0.25f, 1.f), NFmiPoint(1, 1));
	DrawParamInTime(theFractileData, envi, theLatlon, static_cast<FmiParameterName>(theStartParamIndex += theParamIndexIncrement), NFmiColor(0.f, 0.f, 1.f), NFmiPoint(1, 1));
}

void NFmiTimeSerialView::DrawModelFractileDataLocationInTime(const NFmiPoint &theLatlon)
{
	boost::shared_ptr<NFmiFastQueryInfo> fractileData = itsCtrlViewDocumentInterface->GetBestSuitableModelFractileData(itsInfo);
	if(fractileData)
	{
		if(Info()->Param().GetParamIdent() == kFmiTemperature && fractileData->Param(kFmiTemperatureF100)) 
			DrawModelFractileDataLocationInTime(fractileData, kFmiTemperatureF100, theLatlon);
		else if(Info()->Param().GetParamIdent() == kFmiPrecipitation1h && fractileData->Param(kFmiTotalPrecipitationF100)) 
			DrawModelFractileDataLocationInTime(fractileData, kFmiTotalPrecipitationF100, theLatlon);
		else if(Info()->Param().GetParamIdent() == kFmiTotalCloudCover && fractileData->Param(kFmiTotalCloudCoverF100)) 
			DrawModelFractileDataLocationInTime(fractileData, kFmiTotalCloudCoverF100, theLatlon);
		else if(Info()->Param().GetParamIdent() == kFmiWindSpeedMS && fractileData->Param(kFmiWindSpeedF100)) 
			DrawModelFractileDataLocationInTime(fractileData, kFmiWindSpeedF100, theLatlon);
        else if(Info()->Param().GetParamIdent() == kFmiHourlyMaximumGust && fractileData->Param(kFmiWindGustF100)) 
			DrawModelFractileDataLocationInTime(fractileData, kFmiWindGustF100, theLatlon);
	}
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiTimeSerialView::GetSeaLevelPlumeData(const NFmiProducer& usedProducer)
{
    return itsCtrlViewDocumentInterface->InfoOrganizer()->FindInfo(NFmiInfoData::kViewable, usedProducer, true);
}

// Jos on ollut ShowHelperData2InTimeSerialView p‰‰ll‰ (= n‰yt‰ EC:n lyhyit‰ fraktiileja pluumina)
// Jos kyseess‰ on Sea-level parametri (id = 60), katsotaan lˆytyykˆ siihen liittyv‰‰
// Hansenin meri fraktiili dataa (*_waterlevel_hansen_EPS.sqd).
void NFmiTimeSerialView::DrawPossibleSeaLevelPlumeDataLocationInTime(const NFmiPoint &theLatlon)
{
	const auto *seaLevelPlumeData = itsCtrlViewDocumentInterface->SeaLevelPlumeData().getSeaLevelPlumeData(itsDrawParam->Param().GetParamIdent());
    if(seaLevelPlumeData)
    {
        auto seaLevelFractileData = GetSeaLevelPlumeData(seaLevelPlumeData->producer());
        if(seaLevelFractileData)
        {
            NFmiDrawingEnvironment envi;
            envi.SetFillPattern(FMI_DASHDOTDOT);

            // Piirr‰ sea-level fraktiilit
			const auto& fractileParams = seaLevelPlumeData->fractileParams();
			const auto& fractileParamColors = seaLevelPlumeData->fractileParamColors();
            for(size_t i = 0; i < fractileParams.size(); i++)
            {
                DrawParamInTime(seaLevelFractileData, envi, theLatlon, static_cast<FmiParameterName>(fractileParams[i].GetIdent()), fractileParamColors[i], NFmiPoint(1, 1));
            }
            DrawSeaLevelProbLines(theLatlon);
        }
    }
    else
        DrawPossibleSeaLevelForecastProbLimitDataPlume(theLatlon);
}

// Toinen erikoistapaus meris‰‰palvelulle:
// Jos k‰ytˆss‰ jokin Hansenin meri fraktiili datan ProbLimit1-4 parametreista, piirret‰‰n ne kaikki 
// parveen samoilla v‰reill‰ kuin DrawSeaLevelProbLines. Huom! kyse ei ole samojen vakia viivojen piirrosta, 
// vaan eri limittien todenn‰kˆisyys arvoista.
void NFmiTimeSerialView::DrawPossibleSeaLevelForecastProbLimitDataPlume(const NFmiPoint &theLatlon)
{
	const auto* seaLevelPlumeData = itsCtrlViewDocumentInterface->SeaLevelPlumeData().getSeaLevelPlumeData(itsDrawParam->Param().GetParamIdent());
	if(seaLevelPlumeData)
	{
        auto seaLevelFractileData = GetSeaLevelPlumeData(seaLevelPlumeData->producer());
        if(seaLevelFractileData)
        {
            NFmiDrawingEnvironment envi;
            envi.SetPenSize(NFmiPoint(2, 2));

            // Piirr‰ sea-level fraktiilit
			const auto& probLimitParams = seaLevelPlumeData->probLimitParams();
			const auto& probabilityLineColors = seaLevelPlumeData->probabilityLineColors();
			for(size_t i = 0; i < probLimitParams.size(); i++)
            {
                DrawParamInTime(seaLevelFractileData, envi, theLatlon, static_cast<FmiParameterName>(probLimitParams[i].GetIdent()), probabilityLineColors[i], NFmiPoint(1, 1));
            }
        }
    }
}

// Oletus: T‰t‰ kutsutaan vasta kun on piirretty fraktiili parvi DrawPossibleSeaLevelPlumeDataLocationInTime metodista, 
// eli kaikki seaLevel fraktiili tarkastelut on jo tehty.
// Oletus 2: piirrett‰v‰n datan pit‰‰ olla asemadataa, jotta voidaan etsi‰ hiiren kursorin l‰himm‰n aseman id
// 1. Katsotaan onko piirrett‰v‰n seaLevel datan l‰hin piste g_SeaLevelProbabilityMaxSearchRangeInMetres sis‰ll‰ hiiren kursorista
// 2. Katso lˆytyyko kyseinen asema g_SeaLevelProbabilityStationData listasta.
// Jos lˆytyy, piirr‰ n‰ytˆn yli vaakasuoraan eri prob viivat halutuilla v‰reill‰.
void NFmiTimeSerialView::DrawSeaLevelProbLines(const NFmiPoint& theLatlon)
{
	if(itsOperationMode != TimeSerialOperationMode::NormalDrawMode)
		return;

	const auto* seaLevelPlumeData = itsCtrlViewDocumentInterface->SeaLevelPlumeData().getSeaLevelPlumeData(itsDrawParam->Param().GetParamIdent());
	if(seaLevelPlumeData)
	{
		auto oldLocationIndex = itsInfo->LocationIndex();
		if(itsInfo->NearestLocation(theLatlon, seaLevelPlumeData->probabilityMaxSearchRangeInMetres()))
		{
			auto probStationData = seaLevelPlumeData->FindSeaLevelProbabilityStationData(itsInfo->Location(), itsInfo->LatLon());
			const auto& probabilityLineColors = seaLevelPlumeData->probabilityLineColors();
			if(probStationData && probabilityLineColors.size() >= 4)
			{
				NFmiDrawingEnvironment envi;
				envi.SetPenSize(NFmiPoint(2, 2));
				DrawSeaLevelProbLine(envi, probabilityLineColors[0], probStationData->prob1_);
				DrawSeaLevelProbLine(envi, probabilityLineColors[1], probStationData->prob2_);
				DrawSeaLevelProbLine(envi, probabilityLineColors[2], probStationData->prob3_);
				DrawSeaLevelProbLine(envi, probabilityLineColors[3], probStationData->prob4_);
			}
		}
		itsInfo->LocationIndex(oldLocationIndex);
	}
}

void NFmiTimeSerialView::DrawSeaLevelProbLine(NFmiDrawingEnvironment &theEnvi, const NFmiColor &theLineColor, float theProbValue)
{
    theEnvi.SetFrameColor(theLineColor);
    NFmiPoint point1;
    point1.X(itsDataRect.Left());
    point1.Y(Value2AxisPosition(theProbValue));
    NFmiPoint point2;
    point2.X(itsDataRect.Right());
    point2.Y(Value2AxisPosition(theProbValue));
    NFmiLine line(point1, point2, 0, &theEnvi);
    itsToolBox->Convert(&line);
}

void NFmiTimeSerialView::DrawStationNameLegend(const NFmiLocation* theLocation, NFmiDrawingEnvironment &theEnvi, const NFmiPoint& theFontSize, const NFmiPoint& theTextPos, const NFmiString& thePreLocationString, FmiDirection theTextAligment, double theDistanceInMeters)
{
	if(itsOperationMode != TimeSerialOperationMode::NormalDrawMode)
		return ;
	itsToolBox->UseClipping(false);
	if(theLocation)
	{
		theEnvi.SetFontSize(theFontSize);
		NFmiString tmpStr(thePreLocationString);
		tmpStr += theLocation->GetName();
		tmpStr += " (";
		tmpStr += std::to_string(theLocation->GetIdent());
		tmpStr += ")";

		if(theDistanceInMeters != kFloatMissing)
		{
			tmpStr += " (dist ";
			NFmiValueString distStr(theDistanceInMeters/1000., "%.1f");
			tmpStr += distStr;
			tmpStr += " km)";
		}

		NFmiText text(theTextPos, tmpStr, true, 0, &theEnvi);
		FmiDirection oldAligment = itsToolBox->GetTextAlignment();
		itsToolBox->SetTextAlignment(theTextAligment);
		itsToolBox->Convert(&text);
		itsToolBox->SetTextAlignment(oldAligment);
	}
}

static double GetTimeSerialValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, bool interpolateValues, const NFmiPoint &theLatlon, const NFmiFastInfoUtils::MetaWindParamUsage &metaWindParamUsage, unsigned long wantedParamId)
{
    if(metaWindParamUsage.ParamNeedsMetaCalculations(wantedParamId))
    {
        if(interpolateValues)
            return NFmiFastInfoUtils::GetMetaWindValue(theInfo, theLatlon, metaWindParamUsage, wantedParamId);
        else
            return NFmiFastInfoUtils::GetMetaWindValue(theInfo, metaWindParamUsage, wantedParamId);
    }
    else
    {
        if(interpolateValues)
            return theInfo->InterpolatedValue(theLatlon);
        else
            return theInfo->FloatValue();
    }
}

// Tooltip arvoja haettaessa pit‰‰ ottaa huomioon seuraavia asioita:
// 1. Hiladataa interpoloidaan ajan ja paikan suhteen
// 2. Havaintodataa ei interpoloida, oletetaan ett‰ paikka ja aika on jo asennettu (tuli puuttuvaa tai ei)
// 3. MetaWindParamUsage pit‰‰ tarkistaa, jos kyseess‰ on tuulen meta-parametri tapaus
float NFmiTimeSerialView::GetTooltipValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, boost::shared_ptr<NFmiDrawParam>& theDrawParam)
{
	auto wantedParamId = theDrawParam->Param().GetParamIdent();
	auto usedTime = NFmiFastInfoUtils::GetUsedTimeIfModelClimatologyData(theInfo, theTime);
    bool doInterpolation = theInfo->IsGrid();
    NFmiFastInfoUtils::MetaWindParamUsage metaWindParamUsage = NFmiFastInfoUtils::CheckMetaWindParamUsage(theInfo);
    if(metaWindParamUsage.ParamNeedsMetaCalculations(wantedParamId))
    {
		if(doInterpolation)
            return NFmiFastInfoUtils::GetMetaWindValue(theInfo, usedTime, theLatlon, metaWindParamUsage, wantedParamId);
        else
            return NFmiFastInfoUtils::GetMetaWindValue(theInfo, metaWindParamUsage, wantedParamId);
    }
    else
    {
		if(theInfo && theInfo->DataType() == NFmiInfoData::kTimeSerialMacroParam)
		{
			return GetMacroParamTooltipValue(theInfo, theDrawParam, theLatlon, theTime);
		}
		else if(doInterpolation)
            return theInfo->InterpolatedValue(theLatlon, usedTime);
        else
        {
            theInfo->Time(usedTime);
            return theInfo->FloatValue();
        }
    }
}

void NFmiTimeSerialView::DrawSimpleDataInTimeSerial(const NFmiTimeBag &theDrawedTimes, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiDrawParam>& theDrawParam, NFmiDrawingEnvironment &theEnvi, const NFmiPoint &theLatLonPoint, const NFmiPoint& theSinglePointSize, bool drawConnectingLines, int theTimeOffsetWhenDrawedInMinutes)
{
    auto metaWindParamUsage = NFmiFastInfoUtils::CheckMetaWindParamUsage(theInfo);
    auto paramId = theDrawParam->Param().GetParamIdent();

	if(itsOperationMode != TimeSerialOperationMode::NormalDrawMode)
	{
		ScanDataForSpecialOperation(theInfo, theDrawParam, theLatLonPoint, itsAutoAdjustScanTimes, itsAutoAdjustMinMaxValues, metaWindParamUsage, paramId, theTimeOffsetWhenDrawedInMinutes);
		return ;
	}

	itsToolBox->UseClipping(true);
	if(theInfo && theInfo->DataType() == NFmiInfoData::kTimeSerialMacroParam)
	{
		// MacroParam datan piirrot
		DrawTimeSerialMacroParam(theInfo, theDrawParam, theLatLonPoint, theEnvi);
		return;
	}

	// Normaali datan piirrot
	FastInfoTimeLimits timeLimits(theInfo, theDrawedTimes);
	if(timeLimits.isValid())
	{
		bool interpolateValues = theInfo->IsGrid();
		NFmiLocation wantedLocation(theLatLonPoint);
		if(interpolateValues == false && (theInfo->NearestLocation(wantedLocation, gMaxDistanceToFractileStation) == false))
			return; // jos asema dataa ei lˆydy 500 km sis‰lt‰ haluttua pistett‰, ei k‰ytet‰ sit‰
		NFmiDrawingEnvironment blackLineEnvi;
		theInfo->TimeIndex(timeLimits.startTimeIndex_); // pit‰‰ asettaa 1. aika p‰‰lle
		NFmiPoint pointSize(1, 1);
		double realValue1 = ::GetTimeSerialValue(theInfo, interpolateValues, theLatLonPoint, metaWindParamUsage, paramId)
			  ,realValue2 = kFloatMissing;
		double lastNonMissingValue = kFloatMissing;
		NFmiMetTime time1(theInfo->Time())
				   ,time2;
		NFmiMetTime lastNonMissingValueTime;
		time1.ChangeByMinutes(theTimeOffsetWhenDrawedInMinutes);
		if(timeLimits.startTimeIndex_ == timeLimits.endTimeIndex_)
		{
			DrawSinglePointData(realValue1, time1, theEnvi, theSinglePointSize);
		}
		else
		{
			for(auto timeIndex = timeLimits.startTimeIndex_ + 1; timeIndex <= timeLimits.endTimeIndex_; timeIndex++)
			{
				theInfo->TimeIndex(timeIndex);
				time2 = theInfo->Time();
				time2.ChangeByMinutes(theTimeOffsetWhenDrawedInMinutes);
				realValue2 = ::GetTimeSerialValue(theInfo, interpolateValues, theLatLonPoint, metaWindParamUsage, paramId);
				if(drawConnectingLines && realValue1 == kFloatMissing && realValue2 != kFloatMissing && lastNonMissingValue != kFloatMissing)
				{
					// piirret‰‰n ohut musta yhteysviiv‰ katkonaisiin kohtiin, jos niin on s‰‰detty
					// ja piirret‰‰n se t‰p‰n alle, jokat tehd‰‰n kun on toinen arvoista on puuttuvaa.
					DrawDataLine(lastNonMissingValueTime, time2, lastNonMissingValue, realValue2, blackLineEnvi, pointSize, theSinglePointSize, true);
				}
				DrawDataLine(time1, time2, realValue1, realValue2, theEnvi, pointSize, theSinglePointSize, true);
				realValue1 = realValue2;
				time1 = time2;
				if(realValue2 != kFloatMissing)
				{
					lastNonMissingValue = realValue2;
					lastNonMissingValueTime = time2;
				}
			}
		}
	}
}

void NFmiTimeSerialView::DrawSinglePointData(double value, const NFmiMetTime &time, NFmiDrawingEnvironment &theEnvi, const NFmiPoint& theSinglePointSize)
{
    // Piirret‰‰n se ainoa datasta lˆytynyt aika kuitenkin aikasarjaan
    NFmiPoint relativePoint = CalcRelativeValuePosition(time, value);
    double penSize = itsCtrlViewDocumentInterface->Printing() ? (theSinglePointSize.X() * 5.) : theSinglePointSize.X();
    auto oldPenSize = theEnvi.GetPenSize();
    theEnvi.SetPenSize(NFmiPoint(penSize, penSize));
    DrawPointInDataRect(theEnvi, relativePoint, theSinglePointSize);
    theEnvi.SetPenSize(oldPenSize);
}

//--------------------------------------------------------
// CreateValueScale
//--------------------------------------------------------
void NFmiTimeSerialView::CreateValueScale (void)
{
	CreateValueScaleView();
}
//--------------------------------------------------------
// DrawDataLine
//--------------------------------------------------------
// ottaa huomioon myˆs, jos viiva on leikattava kun toinen arvoista menee akselin yli
void NFmiTimeSerialView::DrawDataLine (const NFmiMetTime& theTime1,  const NFmiMetTime& theTime2
									  ,double value1, double value2, NFmiDrawingEnvironment & envi
									  ,const NFmiPoint& thePointSize, const NFmiPoint& theSinglePointSize, bool fUseValueAxis)
{
	if(value1 == kFloatMissing && value2 == kFloatMissing) // kumpikaan ei ole piirrett‰viss‰, ei piirret‰ sitten ollenkaan!
		return;

	NFmiMetTime time1 = theTime1;
	NFmiMetTime time2 = theTime2;
	if(value1 == kFloatMissing) // vain 2. pisteell‰ on arvo, piirret‰‰n sitten piste!
	{
		value1 = value2;
		time1 = time2;
	}
	if(value2 == kFloatMissing) // vain 1. pisteell‰ on arvo, piirret‰‰n sitten piste!
	{
		value2 = value1;
		time2 = time1;
	}
	bool isInAxis1 = fUseValueAxis ? ValueInsideValueAxis(value1) : ValueInsideModifyFactorAxis(value1)
		,isInAxis2 = fUseValueAxis ? ValueInsideValueAxis(value2) : ValueInsideModifyFactorAxis(value2);
	NFmiPoint point1 = fUseValueAxis ? CalcRelativeValuePosition(time1, value1) : CalcRelativeModifyFactorPosition(time1, value1)
		,point2 = fUseValueAxis ? CalcRelativeValuePosition(time2, value2) : CalcRelativeModifyFactorPosition(time2, value2);
	if(!(isInAxis1 && isInAxis2))
	{
		if(!CheckIsPointsDrawable(point1, isInAxis1, point2, isInAxis2, fUseValueAxis))
			return;
	}

	NFmiLine line(point1, point2, 0, &envi);
	itsToolBox->Convert(&line);
	if(point1 == point2)
	{
		DrawPointInDataRect(envi, point1, theSinglePointSize);
	}
	else if(thePointSize.X() > 1. && thePointSize.Y() > 1.)
	{
		DrawPointInDataRect(envi, point1, thePointSize);
		DrawPointInDataRect(envi, point2, thePointSize);
	}
}

void NFmiTimeSerialView::DrawLineInDataRect(NFmiPoint& relativeStartPoint, NFmiPoint& relativeEndPoint, NFmiDrawingEnvironment & envi)
{
	NFmiRect valueRect = CalcValueAxisRect();
	valueRect.Size(NFmiPoint(1., valueRect.Size().Y())); // t‰m‰ ei ole oikea datarect x-suunnassa, mutta riitt‰‰ t‰ss‰ toistaiseksi
	bool isInAxis1 = valueRect.IsInside(relativeStartPoint)
			  ,isInAxis2 = valueRect.IsInside(relativeEndPoint);
	if(!(isInAxis1 && isInAxis2))
	{
		if(!CheckIsPointsDrawable(relativeStartPoint, isInAxis1, relativeEndPoint, isInAxis2, true))
			return;
	}

	if(relativeStartPoint == relativeEndPoint)
		relativeEndPoint.X(relativeEndPoint.X() + itsToolBox->SY(4)); // jos piirret‰‰n vain piste, tehd‰‰n siit‰ 4 pikseli‰ pitk‰
	NFmiLine line(relativeStartPoint, relativeEndPoint, 0, &envi);
	itsToolBox->Convert(&line);
}

//--------------------------------------------------------
// CheckIsPointsDrawable
//--------------------------------------------------------
// tarkistaa voidaanko annetut pisteet mitenk‰‰n piirt‰‰ arvoasteikolle, jos voidaan, niit‰ muokataan tarvittavalla tavalla
// muuten palauttaa false:n (HUOM: laskuissa k‰ytet‰‰n jo valmiiksi laskettuja suhteellisia paikkoja (pisteit‰)
// sen sijaan ett‰ k‰ytett‰isiin parametrin todellisia arvoja ja kellonaikoja sen takia, ett‰ kellonajat voisivat tuottaa ongelmia
// laskettaessa leikkaus pisteit‰!?!)
bool NFmiTimeSerialView::CheckIsPointsDrawable(NFmiPoint& point1, bool fPoint1In, NFmiPoint& point2, bool fPoint2In, bool fUseValueAxis)
{
// HUOM!!! kaikki Top vs. point.Y() vertailut vaikuttavat olevan v‰‰rinp‰in, mutta rect alkaakin ylh‰‰lt‰ ja menee alas
	NFmiRect valueRect = fUseValueAxis ? CalcValueAxisRect() : CalcModifyFactorAxisRect();
	if(!fPoint1In && !fPoint2In) // molemmat pisteet asteikon ulkona, tarkistetaan, olivatko pisteet eri puolilla asteikkoa, jolloin ne voidaan osittain piirt‰‰
	{
		if(valueRect.Top() > point1.Y() && valueRect.Bottom() < point2.Y())
		{ // point1 oli yli asteikon ja point2 oli sen ali
			CutLinePoint2YPlane(point2, point1, valueRect.Top());
			CutLinePoint2YPlane(point1, point2, valueRect.Bottom());
			return true;
		}
		else if(valueRect.Top() > point2.Y() && valueRect.Bottom() < point1.Y())
		{ // point2 oli yli asteikon ja point1 oli sen ali
			CutLinePoint2YPlane(point1, point2, valueRect.Top());
			CutLinePoint2YPlane(point2, point1, valueRect.Bottom());
			return true;
		}
		// else pisteet ovat olleet ulkona samalla puolella asteikkoa ja viivaa ei piirret‰
	}
	else // toinen piste asteikon sis‰ll‰ ja toinen ulkona
	{
		if(valueRect.Top() > point1.Y())
		{ // point1 oli yli asteikon
			CutLinePoint2YPlane(point2, point1, valueRect.Top());
			return true;
		}
		else if(valueRect.Top() > point2.Y())
		{ // point2 oli yli asteikon
			CutLinePoint2YPlane(point1, point2, valueRect.Top());
			return true;
		}
		else if(valueRect.Bottom() < point1.Y())
		{ // point1 oli alle asteikon
			CutLinePoint2YPlane(point2, point1, valueRect.Bottom());
			return true;
		}
		else if(valueRect.Bottom() < point2.Y())
		{ // point2 oli alle asteikon
			CutLinePoint2YPlane(point1, point2, valueRect.Bottom());
			return true;
		}
	}
	return false;
}

//--------------------------------------------------------
// EvaluateValue
//--------------------------------------------------------

//   Asettaa uuden arvon datalle muutosk‰yr‰‰
//   piirrett‰ess‰. Esim. kok.pilv. 90 + 10 ->
//   100
//   ja WD 350 + 20 -> 10, jne.
void NFmiTimeSerialView::EvaluateChangedValue (double& theValue)
{
 // OTA DRAWPARAMISTA minimi ja maksimi tarkastelu!!!!!!
}

//--------------------------------------------------------
// DrawParamName
//--------------------------------------------------------
void NFmiTimeSerialView::DrawParamName(void)
{
	itsToolBox->UseClipping(false);
	if(itsDrawParam)
	{
		itsDrawingEnvironment->SetFrameColor(CtrlViewUtils::GetParamTextColor(itsDrawParam->DataType(), itsDrawParam->UseArchiveModelData()));

		itsDrawingEnvironment->SetFontSize(CalcFontSize());

		bool doNewDataHighlight = !itsCtrlViewDocumentInterface->BetaProductGenerationRunning();
		auto str = CtrlViewUtils::GetParamNameString(itsDrawParam, false, false, true, 0, true, doNewDataHighlight, true, nullptr);
		if(IsNewDataParameterName(str))
		{
			itsDrawingEnvironment->BoldFont(true);
		}

		if(itsDrawParam->DataType() == NFmiInfoData::kEditable)
		{
			int displayed = CtrlViewFastInfoFunctions::GetMaskedCount(Info(), NFmiMetEditorTypes::kFmiDisplayedMask, itsCtrlViewDocumentInterface->AllowRightClickDisplaySelection());
			str += " (";
			if(displayed > 1)
				str += "displayed locations";
			else
				str += ::GetDictionaryString("TimeSerialViewSelectedPoints");
			str += " ";
			int selected = CtrlViewFastInfoFunctions::GetMaskedCount(Info(), NFmiMetEditorTypes::kFmiSelectionMask, itsCtrlViewDocumentInterface->AllowRightClickDisplaySelection());
			NFmiValueString displayedCountStr(displayed, "%d");
			NFmiValueString selectedCountStr(selected, "%d");
			if(displayed > 1)
				str += displayedCountStr;
			else
				str += selectedCountStr;
			str += ")";
		}

		NFmiPoint place(CalcParamTextPosition());
		NFmiText text(place, str, true, 0, itsDrawingEnvironment);
		FmiDirection oldDir = itsToolBox->GetTextAlignment();
		itsToolBox->SetTextAlignment(kTop);
		itsToolBox->Convert(&text);
		DrawSideParameterNames(str);
		itsToolBox->SetTextAlignment(oldDir);
		itsDrawingEnvironment->BoldFont(false);
	}
}

static void MoveTextPointByDrawnText(NFmiPoint& textPointInOut, const NFmiString& drawnString, NFmiToolBox* toolbox)
{
	const double measureTextFactor = 1.1;
	auto textLegth = toolbox->MeasureText(drawnString) * measureTextFactor;
	textPointInOut.X(textPointInOut.X() + textLegth);
}

void NFmiTimeSerialView::DrawSideParameterNames(const NFmiString& mainParamString)
{
	if(!itsSideParameterNames.empty())
	{
		NFmiPoint place(CalcParamTextPosition());
		auto usedMainParamString = mainParamString;
		usedMainParamString += "   ";
		::MoveTextPointByDrawnText(place, usedMainParamString, itsToolBox);
		// Piirret‰‰n p‰‰parametrin per‰‰n eri v‰reill‰ side-paramit
		itsDrawingEnvironment->SetFrameColor(NFmiColor(0, 0, 0));
		NFmiString drawnText = "Side-Params:  ";
		NFmiText headerText(place, drawnText, true, 0, itsDrawingEnvironment);
		itsToolBox->Convert(&headerText);
		::MoveTextPointByDrawnText(place, drawnText, itsToolBox);

		// GeneralColor v‰rit alkavat 0:sta, ja 1. side-parameter on tarkoitus piirt‰‰ 2. v‰rill‰, jonka indeksi on siis 1.
		int sideParameterColorIndex = 1;
		for(const auto& sideParamName : itsSideParameterNames)
		{
			itsDrawingEnvironment->SetFrameColor(itsCtrlViewDocumentInterface->GeneralColor(sideParameterColorIndex));
			std::string sideParamString = std::to_string(sideParameterColorIndex) + ")" + sideParamName + "  ";
			drawnText = sideParamString;
			NFmiText paramText(place, drawnText, true, 0, itsDrawingEnvironment);
			itsToolBox->Convert(&paramText);
			::MoveTextPointByDrawnText(place, drawnText, itsToolBox);
			sideParameterColorIndex++;
		}
	}
}

NFmiPoint NFmiTimeSerialView::CalcParamTextPosition(void)
{
	NFmiPoint place(GetFrame().TopLeft());
	place.X(place.X() + itsToolBox->SX(5)); // siirret‰‰n 5 pikseli‰ oikealle reunasta
	return place;
}

//--------------------------------------------------------
// DrawModifyingUnit M.K. 22.4.99
//--------------------------------------------------------
void NFmiTimeSerialView::DrawModifyingUnit(void)
{
	itsToolBox->UseClipping(false);
	if(itsCtrlViewDocumentInterface->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal) // jos ns. edit-moodi p‰‰ll‰, piiret‰‰n aikarajoitin viivat
	{
		itsDrawingEnvironment->SetFrameColor(NFmiColor(0.f,0.f,0.f));

		itsModifyingUnitTextRect = CalcModifyingUnitRect();
		NFmiRectangle rec(itsModifyingUnitTextRect, 0, itsDrawingEnvironment);
		itsToolBox->Convert(&rec);

		NFmiPoint fontSize = CalcFontSize();						// Pikaratkaisu.
		itsDrawingEnvironment->SetFontSize(fontSize);				// Pikaratkaisu.

		NFmiString str("");

		if(itsCtrlViewDocumentInterface->TimeSerialViewDrawParamList()->Find(itsDrawParam->Param(), &itsDrawParam->Level(), itsDrawParam->DataType()))
		{
			{
				str += NFmiString(itsCtrlViewDocumentInterface->TimeSerialViewDrawParamList()->Current()->Unit());
				std::string locString(::GetDictionaryString("TimeSerialViewUnitString"));
				str += NFmiString(locString);
			}
		}

		NFmiPoint place(itsModifyingUnitTextRect.BottomLeft());
		place.Y(place.Y() - 0.005);
		NFmiText text(place, str, false, 0, itsDrawingEnvironment);
		itsToolBox->Convert(&text);
	}
	return;
}

//--------------------------------------------------------
// CalcFontSize
//--------------------------------------------------------
NFmiPoint NFmiTimeSerialView::CalcFontSize(void)
{
	long x = itsToolBox->HX(CalcTimeAxisRect().Width()/30.);		// Tekstin koko riippuu aikasarjaeditorin leveydest‰.
	long y = itsToolBox->HY(CalcTimeAxisRect().Height()/5.);		// Tekstin koko riippuu aikasarjaeditorin leveydest‰.
	long fontSize = boost::math::iround((x + y)/2.2);
	fontSize = FmiMin(18, FmiMax(16, static_cast<int>(fontSize)));
	return NFmiPoint(fontSize, fontSize);
}

//--------------------------------------------------------
// DrawData
//--------------------------------------------------------
void NFmiTimeSerialView::DrawData(void)
{
	itsToolBox->UseClipping(true);

	FmiParameterName param = FmiParameterName(itsDrawParam->Param().GetParamIdent());
	if(param == kFmiPrecipitation1h || param == kFmiPrecipitation3h || param == kFmiPrecipitation6h) // t‰h‰n pit‰‰ muuttaa incremental tarkistus!!!
		DrawSelectedStationDataIncrementally();
	DrawSelectedStationData();
	if(itsDrawParam->DataType() == NFmiInfoData::kEditable)
		DrawModifyFactorPoints();
	DrawTimeLine();
}

// piirt‰‰ selityksen ruudun yl‰reunaan ja piirt‰‰ datan kertyv‰n‰
// eli laskee kertym‰n datan arvoista
void NFmiTimeSerialView::DrawSelectedStationDataIncrementally(void)
{
	if(itsOperationMode != TimeSerialOperationMode::NormalDrawMode)
		return ;
	DrawIncrementalDataLegend();
	boost::shared_ptr<NFmiFastQueryInfo> info = Info();
    // Incremental draw can't be used witn non-edited data, because these drawing 
    // functions are using mask-system that are only usable with edited data.
	if(!info || !IsEditedData(info))
		return;
    EditedInfoMaskHandler editedInfoMaskHandler(info, CtrlViewFastInfoFunctions::GetProperMaskTypeFromEditeInfo(info, itsCtrlViewDocumentInterface->AllowRightClickDisplaySelection()));
	info->ResetLocation();
	if(info->NextLocation())
		DrawLocationDataIncrementally();

    if(info->MaskType() == NFmiMetEditorTypes::kFmiDisplayedMask && itsCtrlViewDocumentInterface->AllowRightClickDisplaySelection())
	{
		while(info->NextLocation())
			DrawLocationDataIncrementally();
	}

	return;
}

void NFmiTimeSerialView::DrawLocationDataIncrementally(void)
{
	float value1, value2, modifiedValue1,modifiedValue2, realValue1, realValue2;
	NFmiMetTime time1, time2;

	boost::shared_ptr<NFmiAreaMaskList> paramMaskList = itsCtrlViewDocumentInterface->ParamMaskListMT();
	if(!paramMaskList)
		return ;
	if(!itsCtrlViewDocumentInterface->IsMasksUsedInTimeSerialViews())
		paramMaskList = classesEmptyParamMaskList;
	paramMaskList->CheckIfMaskUsed();

	Info()->ResetTime();
	Info()->NextTime();
	time1 = Info()->Time();

	paramMaskList->SyncronizeMaskTime(time1);
	float maskFactor1 = (float)paramMaskList->MaskValue(Info()->LatLon()),
		   maskFactor2 = 0;
	realValue1 = value1 = value2 = Info()->FloatValue() * Info()->TimeResolution() / 60.f; // pit‰‰ kertoa tuntim‰‰r‰ll‰, koska nyky‰‰n vain 1h sateita datassa
	modifiedValue1 = modifiedValue2 = CalcModifiedValue(realValue1, 0, maskFactor1);
	bool fDrawChangeLines2 = value1 != modifiedValue1;
	NFmiPoint pointSize(4,4);
	long timeCount = Info()->SizeTimes();
	for(long i=1; i < timeCount; i++) // HUOM! ei piirr‰ jos vain yksi aika.
	{
		Info()->NextTime();
		time2 = Info()->Time();

		paramMaskList->SyncronizeMaskTime(time2);
		maskFactor2 = (float)paramMaskList->MaskValue(Info()->LatLon());

		value2 += Info()->FloatValue() * Info()->TimeResolution() / 60.f; // pit‰‰ kertoa tuntim‰‰r‰ll‰, koska nyky‰‰n vain 1h sateita datassa
		realValue2 = Info()->FloatValue() * Info()->TimeResolution() / 60.f; // pit‰‰ kertoa tuntim‰‰r‰ll‰, koska nyky‰‰n vain 1h sateita datassa
		float modValue = CalcModifiedValue(realValue2, i, maskFactor2);
		if(modValue < 0.)
			modValue = 0.;
		modifiedValue2 += modValue;
		bool fDrawChangeLines3 = value2 != modifiedValue2;
		DrawDataLine(time1, time2, value1, value2, itsIncrementalCurveEnvi, pointSize, pointSize, true);

		if (fDrawChangeLines2 && fDrawChangeLines3)
		{
			DrawDataLine(time1, time2, modifiedValue1, modifiedValue2, itsChanheIncrementalCurveEnvi, pointSize, pointSize, true);
		}
		else if(fDrawChangeLines2 && !fDrawChangeLines3)
		{
			DrawDataLine(time1, time2, modifiedValue1, value2, itsChanheIncrementalCurveEnvi, pointSize, pointSize, true);
		}
		else if(!fDrawChangeLines2 && fDrawChangeLines3)
		{
			DrawDataLine(time1, time2, value1, modifiedValue2, itsChanheIncrementalCurveEnvi, pointSize, pointSize, true);
		}

		value1 = value2;
		modifiedValue1 = modifiedValue2;
		time1 = time2;
		fDrawChangeLines2 = fDrawChangeLines3;
		maskFactor1 = maskFactor2;
	}
	return;
}

void NFmiTimeSerialView::DrawIncrementalDataLegend(void)
{
	double yShiftChange = itsToolBox->SY(itsIncrementalCurveEnvi.GetFontHeight());
	NFmiPoint place(GetFrame().TopLeft());
	place.X(place.X() + GetFrame().Width() * 0.6);
	place.Y(place.Y() + yShiftChange * 0.06);

	FmiDirection oldAlign = itsToolBox->GetTextAlignment();
	itsToolBox->SetTextAlignment(kLeft);
	NFmiString str("cumulative ---");
	NFmiText text(place, str, false, 0, &itsIncrementalCurveEnvi);
	itsToolBox->Convert(&text);
	itsToolBox->SetTextAlignment(oldAlign);
}

static std::string GetLatlonString(const NFmiPoint &theLatlon)
{
	string str;
	str += CtrlViewUtils::GetLatitudeMinuteStr(theLatlon.Y(), 0);
	str += ",";
	str += CtrlViewUtils::GetLongitudeMinuteStr(theLatlon.X(), 0);
	return str;
}

void NFmiTimeSerialView::DrawStationDataStationNameLegend(boost::shared_ptr<NFmiFastQueryInfo> &info, const NFmiPoint &theLatlon, int counter, NFmiDrawingEnvironment &envi)
{
	if(itsOperationMode != TimeSerialOperationMode::NormalDrawMode)
		return ;
	if(info)
	{
		itsToolBox->UseClipping(false);
		NFmiLocation loc = *info->Location();
		if(info->Grid())
		{
			string locName;
			locName += "loc: ";
			locName += CtrlViewUtils::GetLatitudeMinuteStr(theLatlon.Y(), 0);
			locName += ",";
			locName += CtrlViewUtils::GetLongitudeMinuteStr(theLatlon.X(), 0);
			loc.SetName(locName);
		}
		NFmiPoint place(CalcParamTextPosition());
		NFmiPoint fontSize(CalcFontSize());
		fontSize *= NFmiPoint(0.9, 0.9); // pienennet‰‰n fonttia hieman
		place.Y(place.Y() + 0.9 * counter * itsToolBox->SY(static_cast<long>(fontSize.Y())));
		NFmiString locString; //(::GetDictionaryString("TimeSerialViewObservationStation"));
		DrawStationNameLegend(&loc, envi, fontSize, place, locString, kTop);
		itsToolBox->UseClipping(true);
	}
}

void NFmiTimeSerialView::DrawSelectedStationData(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, const NFmiPoint &theLatlon, int &theDrawedLocationCounter)
{
	auto drawHelperData = DrawHelperData();
	// vain 1. lokaatiolle piirret‰‰n helper-data ja side-parametrit
	if(theDrawedLocationCounter == 1)
	{
		// Piirret‰‰n ensin muut helper datat
		if(drawHelperData)
		{
			DrawHelperDataLocationInTime(theLatlon);
		}
		// Piirret‰‰n sitten mahdolliset side-parametrit
		DrawSideParametersDataLocationInTime(theLatlon);
	}

    itsNormalCurveEnvi.SetFrameColor(itsCtrlViewDocumentInterface->GeneralColor(theDrawedLocationCounter - 1));
    DrawLocationInTime(theLatlon, itsNormalCurveEnvi, itsChangeCurveEnvi, true);

    if(drawHelperData)
    {
        if(theDrawedLocationCounter == 1) // vain 1. lokaatiolle piirret‰‰n havainto-data
        {
            // Piirret‰‰n mahdolliset apu havainnot viimeiseksi, jotta erilaiset parvet eiv‰t peitt‰isi niit‰ (t‰st‰ tulee aina vain yksi k‰yr‰, joten se ei peit‰ paljoa)
            DrawHelperObservationData(theLatlon);
        }
        DrawStationDataStationNameLegend(theViewedInfo, theLatlon, theDrawedLocationCounter++, itsNormalCurveEnvi);
    }
}

void NFmiTimeSerialView::DrawHelperObservationData(const NFmiPoint &theLatlon)
{
    if(itsCtrlViewDocumentInterface->IsOperationalModeOn() && itsCtrlViewDocumentInterface->ShowHelperData1InTimeSerialView())
    {
        // Ei piirret‰ jos valittu data on havainto tyyppista ja synop tuottajalta, koska k‰yr‰ on jo piirrettyn‰ valittuna datana edell‰.
        // Jos se piirret‰‰n uudestaan originaali (sininen) k‰yr‰ peittyisi nyt punaisella k‰yr‰ll‰
        if(!IsSynopticObservationData())
        {
            // Smartmet on kaatunut joskus mystisesti HESSAA symbolin kanssa, joten ei piirret‰, kun on erikoisn‰ytˆst‰ kyse
            if(!IsParamWeatherSymbol3())
            {
                NFmiDrawingEnvironment envi;
                DrawObservationDataLocationInTime(envi, theLatlon);
            }
        }
    }
}

bool NFmiTimeSerialView::IsSynopticObservationData()
{
    return itsDrawParam->Param().GetProducer()->GetIdent() == kFmiSYNOP && itsDrawParam->DataType() == NFmiInfoData::kObservations;
}

bool NFmiTimeSerialView::IsParamWeatherSymbol3()
{
    return itsDrawParam->Param().GetParamIdent() == kFmiWeatherSymbol3;
}

bool NFmiTimeSerialView::DoControlPointModeDrawing() const
{
    return (!itsCtrlViewDocumentInterface->AnalyzeToolData().AnalyzeToolMode()) && itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode();
}

bool NFmiTimeSerialView::IsAnalyzeRelatedToolUsed() const
{
    return itsCtrlViewDocumentInterface->AnalyzeToolData().AnalyzeToolMode() || itsCtrlViewDocumentInterface->AnalyzeToolData().ControlPointObservationBlendingData().UseBlendingTool();
}

//--------------------------------------------------------
// DrawSelectedStationData
//--------------------------------------------------------
// Jos vain vasemmalla hiiren n‰pp‰imell‰ on valittu asemia,
// piirret‰‰n niist‰ yksi aikasarjaeditoriin, muuten piirret‰‰n
// oikealla hiiren n‰pp‰imell‰ valitut asemat.
void NFmiTimeSerialView::DrawSelectedStationData(void)
{
	boost::shared_ptr<NFmiFastQueryInfo> info = Info();
	if(!info)
		return;

	if(itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode())
	{
		itsCtrlViewDocumentInterface->CPManager()->Param(itsDrawParam->Param());
	}

	if(!IsEditedData(info))
		DrawSelectedStationDataForNonEditedData();
	else
	{
		if(DoControlPointModeDrawing())
		{ // piirr‰ controlli pisteet vertailun vuoksi ruutuun
			DrawCPReferenceLines();
		}
		else
		{
            EditedInfoMaskHandler editedInfoMaskHandler(info, CtrlViewFastInfoFunctions::GetProperMaskTypeFromEditeInfo(info, itsCtrlViewDocumentInterface->AllowRightClickDisplaySelection()));

			int counter = 1;
			NFmiColor stationDataColor;
			info->ResetLocation();
			if(info->NextLocation())
			{
				DrawSelectedStationData(info, info->LatLon(), counter);
				if(IsAnalyzeRelatedToolUsed())
					DrawAnalyzeToolEndTimeLine(); // piirret‰‰n vain ensimm‰isell‰ kerralla pystyviiva, joka kuvaa analyysityˆkalun lopetusajan kohdan
			}

            if(info->MaskType() == NFmiMetEditorTypes::kFmiDisplayedMask && itsCtrlViewDocumentInterface->AllowRightClickDisplaySelection())
			{
				while(info->NextLocation())
				{
					DrawSelectedStationData(info, info->LatLon(), counter);
				}
			}

			// piirrret‰‰n viel‰ editoidun alueen ulkopuolelta mahd. valittu piste mutta vain jos muuta ei ole piirretty
			if(counter == 1)
			{
				auto& latlon = itsCtrlViewDocumentInterface->OutOfEditedAreaTimeSerialPoint();
				if(latlon != NFmiPoint::gMissingLatlon)
				{
					DrawSelectedStationData(info, latlon, counter);
				}
			}
		}
	}
}

// hae oikeasti k‰ytetty smartInfo ja aseta smartinfo osoittamaan l‰hint‰ asemaa.
// t‰m‰ pit‰‰ tehd‰ n‰in koska synop-datoja voi olla useita ja oikea info pit‰‰ hakea aina paikka kohtaisesti.
static boost::shared_ptr<NFmiFastQueryInfo> GetUsedSmartInfo(CtrlViewDocumentInterface *theCtrlViewDocumentInterface, const NFmiPoint &theLatlon, boost::shared_ptr<NFmiFastQueryInfo> &theCurrentInfo, const NFmiMetTime &theTime, boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
	if(theCurrentInfo == 0)
		return boost::shared_ptr<NFmiFastQueryInfo>();
	FmiProducerName prodId = static_cast<FmiProducerName>(theCurrentInfo->Producer()->GetIdent());
	boost::shared_ptr<NFmiFastQueryInfo> info = theCurrentInfo;
	if(prodId == kFmiSYNOP || prodId == kFmiSHIP || prodId == kFmiBUOY || prodId == kFmiTestBed)
		info = theCtrlViewDocumentInterface->GetNearestSynopStationInfo(theLatlon, theTime, true, 0);
	else
		info->NearestLocation(theLatlon);

	info->Param(static_cast<FmiParameterName>(theDrawParam->Param().GetParamIdent()));

	return info;
}

// ei editoitu data piirret‰‰n v‰h‰n erilailla
void NFmiTimeSerialView::DrawSelectedStationDataForNonEditedData(void)
{
	itsInfo = Info();
	boost::shared_ptr<NFmiFastQueryInfo> editedInfo = itsCtrlViewDocumentInterface->EditedSmartInfo();
	if(itsInfo == 0 || editedInfo == 0)
		return;
    NFmiPoint primaryLatlon = GetTooltipLatlon();

    EditedInfoMaskHandler editedInfoMaskHandler(editedInfo, CtrlViewFastInfoFunctions::GetProperMaskTypeFromEditeInfo(editedInfo, itsCtrlViewDocumentInterface->AllowRightClickDisplaySelection()));

	int counter = 1;
	FmiParameterName param = FmiParameterName(itsDrawParam->Param().GetParamIdent());
	NFmiColor stationDataColor;
    itsInfo = ::GetUsedSmartInfo(itsCtrlViewDocumentInterface, primaryLatlon, itsInfo, itsTime, itsDrawParam);
    if(itsInfo)
        DrawSelectedStationData(itsInfo, primaryLatlon, counter);

    // Draw possible additional dispaying selected points
    if(!itsCtrlViewDocumentInterface->IsPreciseTimeSerialLatlonPointUsed())
    {
        if(editedInfo->MaskType() == NFmiMetEditorTypes::kFmiDisplayedMask && itsCtrlViewDocumentInterface->AllowRightClickDisplaySelection())
        {
            editedInfo->FirstLocation();
            while(editedInfo->NextLocation())
            {
                itsInfo = ::GetUsedSmartInfo(itsCtrlViewDocumentInterface, editedInfo->LatLon(), itsInfo, itsTime, itsDrawParam);
                if(itsInfo)
                    DrawSelectedStationData(itsInfo, editedInfo->LatLon(), counter);
            }
        }
    }
}

// Jos ollaan CP-moodissa ja ollaan myˆs Obs-blender moodissa ja kyse on editoidusta datasta.
// Haaraantuu t‰nne DrawCPReferenceLines metodista.
void NFmiTimeSerialView::DrawObservationBlenderDataInCpMode()
{
    DrawCPReferenceLines_DrawAllCps(false); // Ei piirret‰ CP muokkausk‰yr‰‰ (false parametri)
    DrawAnalyzeToolEndTimeLine();
}

NFmiDrawingEnvironment NFmiTimeSerialView::MakeNormalCpLineDrawOptions() const
{
    NFmiDrawingEnvironment drawingEnvironment;
    drawingEnvironment.SetFrameColor(NFmiColor(0.5f, 0.5f, 0.5f));
    NFmiPoint fontSize(20, 20);
    drawingEnvironment.SetFontSize(fontSize);
    return drawingEnvironment;
}

NFmiDrawingEnvironment NFmiTimeSerialView::MakeChangeCpLineDrawOptions() const
{
    NFmiDrawingEnvironment drawingEnvironment(ChangeStationDataCurveEnvironment(0.3, 150));
    drawingEnvironment.SetFrameColor(NFmiColor(0.5f, 0.5f, 0.5f));
    return drawingEnvironment;
}

CpDrawingOptions::CpDrawingOptions(const NFmiDrawingEnvironment &currentLine, const NFmiDrawingEnvironment &changeLine, const NFmiRect &viewFrame, const NFmiToolBox *toolbox)
    :currentDataEnvi(currentLine)
    , changeDataEnvi(changeLine)
{
    textPoint = viewFrame.TopLeft();
    textPoint.X(textPoint.X() + viewFrame.Width() / 5 * 4);
    heightInc = toolbox->SY(15);
    endPointX1 = textPoint.X() + viewFrame.Width() / 25 * 1;
    endPointX2 = textPoint.X() + viewFrame.Width() / 20 * 2;
    textPoint.Y(textPoint.Y() + heightInc);
}

void NFmiTimeSerialView::DrawCPReferenceLines_SetLineOptions(boost::shared_ptr<NFmiEditorControlPointManager> &cpManager, CpDrawingOptions &cpDrawingOptions)
{
    if(cpManager->IsActivateCP())
    {
        cpDrawingOptions.currentDataEnvi.SetPenSize(cpDrawingOptions.thickLine);
        cpDrawingOptions.changeDataEnvi.SetPenSize(cpDrawingOptions.thickChangeLine);
    }
    else if(cpManager->ShowCPAllwaysOnTimeView())
    {
        cpDrawingOptions.currentDataEnvi.SetPenSize(cpDrawingOptions.normalLine);
        cpDrawingOptions.changeDataEnvi.SetPenSize(cpDrawingOptions.normalChangeLine);
    }
    cpDrawingOptions.currentDataEnvi.SetFrameColor(gCPHelpColors[cpDrawingOptions.currentLineIndex]);
    cpDrawingOptions.changeDataEnvi.SetFrameColor(gCPHelpColors[cpDrawingOptions.currentLineIndex]);
}

bool NFmiTimeSerialView::DrawCPReferenceLines_IsCpDrawn(boost::shared_ptr<NFmiEditorControlPointManager> &cpManager)
{
    if(cpManager->IsActivateCP() || cpManager->ShowCPAllwaysOnTimeView())
        return true;
    else
        return false;
}

void NFmiTimeSerialView::DrawCPReferenceLines_AdvanceDrawingOptions(CpDrawingOptions &cpDrawingOptions)
{
    cpDrawingOptions.currentLineIndex++;
    cpDrawingOptions.textPoint.Y(cpDrawingOptions.textPoint.Y() + cpDrawingOptions.heightInc);
}

bool NFmiTimeSerialView::IsThisFirstEditedParamRow()
{
    auto drawParamList = itsCtrlViewDocumentInterface->TimeSerialViewDrawParamList();
    if(drawParamList)
    {
        for(drawParamList->Reset(); drawParamList->Next(); )
        {
            auto drawParam = drawParamList->Current();
            // Katsotaan mist‰ kohtaa lˆytyy 1. editoitava parametri
            if(drawParam->DataType() == NFmiInfoData::kEditable)
            {
                // Katsotaan onko listalla oleva DrawParam sama kuin t‰m‰n olion oma drawParam (suora pointer vertailu!)
                if(drawParam.get() == itsDrawParam.get())
                {
                    return true;
                }
                break; // Jos lˆydetty t‰m‰n olion paikka listasta, mutta se ei ollut editable, lopetetaan
            }
        }
    }
    return false;
}

void NFmiTimeSerialView::DrawCPReferenceLines_DrawLegend(boost::shared_ptr<NFmiEditorControlPointManager> &cpManager, CpDrawingOptions &cpDrawingOptions)
{
    // Laitetaan viivav‰ri legenda vain 1. editoitavan parametrin aikasarja ikkunaan!
    if(IsThisFirstEditedParamRow())
    {
        itsToolBox->UseClipping(false);
        NFmiValueString indexStr(cpManager->CPIndex() + 1, "%d"); // +1 koska indeksit alkavat 0:sta
        NFmiText text(cpDrawingOptions.textPoint, indexStr, false, 0, &cpDrawingOptions.currentDataEnvi);
        itsToolBox->Convert(&text);

        NFmiLine line1(NFmiPoint(cpDrawingOptions.textPoint.X(), cpDrawingOptions.textPoint.Y() - cpDrawingOptions.heightInc / 2.), NFmiPoint(cpDrawingOptions.endPointX1, cpDrawingOptions.textPoint.Y() - cpDrawingOptions.heightInc / 2.), 0, &cpDrawingOptions.currentDataEnvi);
        itsToolBox->Convert(&line1);
        NFmiLine line2(NFmiPoint(cpDrawingOptions.endPointX1, cpDrawingOptions.textPoint.Y() - cpDrawingOptions.heightInc / 2.), NFmiPoint(cpDrawingOptions.endPointX2, cpDrawingOptions.textPoint.Y() - cpDrawingOptions.heightInc / 2.), 0, &cpDrawingOptions.changeDataEnvi);
        itsToolBox->Convert(&line2);
    }
}

void NFmiTimeSerialView::DrawCPReferenceLines_DrawCpLocation(boost::shared_ptr<NFmiEditorControlPointManager> &cpManager, CpDrawingOptions &cpDrawingOptions, bool drawModificationLines)
{
    auto isActiveCp = cpManager->IsActivateCP();
    if(isActiveCp)
        DrawHelperDataLocationInTime(cpManager->LatLon()); // piirret‰‰n aktiivisen CP-pisteen apu datat myˆs ruudulle (=kepa, obs ja clim datat)
    DrawLocationInTime(cpManager->LatLon(), cpDrawingOptions.currentDataEnvi, cpDrawingOptions.changeDataEnvi, drawModificationLines);
    if(isActiveCp)
    {
        // Piirret‰‰n mahdolliset apu havainnot viimeiseksi, jotta erilaiset parvet eiv‰t peitt‰isi niit‰ (t‰st‰ tulee aina vain yksi k‰yr‰, joten se ei peit‰ paljoa)
        DrawHelperObservationData(cpManager->LatLon());
    }
}

// Piirt‰‰ yhteen CP-pisteeseen liittyv‰t jutut.
void NFmiTimeSerialView::DrawCPReferenceLines_ForCurrentCp(boost::shared_ptr<NFmiEditorControlPointManager> &cpManager, boost::shared_ptr<NFmiFastQueryInfo> &info, CpDrawingOptions &cpDrawingOptions, bool drawModificationLines)
{
    if(DrawCPReferenceLines_IsCpDrawn(cpManager))
    {
        DrawCPReferenceLines_SetLineOptions(cpManager, cpDrawingOptions);

        info->Location(cpManager->LatLon()); // asetetaan infon location kohdalleen
        itsModificationFactorCurvePoints = cpManager->CPChangeValues();
        DrawCPReferenceLines_DrawCpLocation(cpManager, cpDrawingOptions, drawModificationLines);
        DrawCPReferenceLines_DrawLegend(cpManager, cpDrawingOptions);
        DrawCPReferenceLines_AdvanceDrawingOptions(cpDrawingOptions);
    }
}

void NFmiTimeSerialView::DrawCPReferenceLines_DrawAllCps(bool drawModificationLines)
{
    CpDrawingOptions cpDrawingOptions(MakeNormalCpLineDrawOptions(), MakeChangeCpLineDrawOptions(), GetFrame(), itsToolBox);
    // piirret‰‰n teksti vasemmalle textPoint:ista ja viiva oikealle
    ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, kBaseRight, itsToolBox->UseClipping());

    boost::shared_ptr<NFmiEditorControlPointManager> CPMan = itsCtrlViewDocumentInterface->CPManager();
    boost::shared_ptr<NFmiFastQueryInfo> info = Info();
    if(CPMan && info)
    {
        for(CPMan->ResetCP(); CPMan->NextCP();)
        {
            if(cpDrawingOptions.currentLineIndex >= gMaxHelpCPDrawed)
                break; // ei piirret‰ enemp‰‰ referenssi viivoja

            DrawCPReferenceLines_ForCurrentCp(CPMan, info, cpDrawingOptions, drawModificationLines);
        }
        //lopuksi asetetaan info aktiiviseen cp:hen
        info->Location(CPMan->ActiveCPLatLon()); // asetetaan infon location kohdalleen
        itsModificationFactorCurvePoints = CPMan->ActiveCPChangeValues();
    }
}

void NFmiTimeSerialView::DrawCPReferenceLines()
{
    if(itsCtrlViewDocumentInterface->AnalyzeToolData().ControlPointObservationBlendingData().UseBlendingTool())
        DrawObservationBlenderDataInCpMode();
    else
        DrawCPReferenceLines_DrawAllCps(true);
}

//--------------------------------------------------------
// ValueInsideValueAxis
//--------------------------------------------------------
bool NFmiTimeSerialView::ValueInsideValueAxis(double theValue)
{
	if(itsValueAxis)
	{
		float start = itsValueAxis->StartValue();
		float end = itsValueAxis->EndValue();
		if((theValue >= start) && (theValue <= end))
			return true;
	}
	return false;
}

bool NFmiTimeSerialView::ValueInsideModifyFactorAxis(double theValue)
{
	if(itsModifyFactorAxis)
	{
		float start = itsModifyFactorAxis->StartValue();
		float end = itsModifyFactorAxis->EndValue();
		if((theValue >= start) && (theValue <= end))
			return true;
	}
	return false;
}

//--------------------------------------------------------
// NormalStationDataCurveEnvironment
//--------------------------------------------------------
NFmiDrawingEnvironment NFmiTimeSerialView::NormalStationDataCurveEnvironment(void)
{
	NFmiDrawingEnvironment envi;
	envi.DisableFill();
	envi.SetFrameColor(NFmiColor(0.0f,0.4f,0.8f));
	envi.SetPenSize(NFmiPoint(3,3));
	return envi;
}

//--------------------------------------------------------
// ChangeStationDataCurveEnvironment
//--------------------------------------------------------
NFmiDrawingEnvironment NFmiTimeSerialView::ChangeStationDataCurveEnvironment(double theRelativeFillFactor, double theSublinesPerUnitFactor) const
{
	NFmiDrawingEnvironment envi;
	envi.DisableFill();
	envi.SetFrameColor(NFmiColor(1.0f,0.f,0.f));
	envi.SetFillPattern(FMI_RELATIVE_FILL);
	envi.SetRelativeFill(theRelativeFillFactor);
	envi.SetSubLinePerUnit(theSublinesPerUnitFactor/itsRect.Width());
	return envi;
}

NFmiDrawingEnvironment NFmiTimeSerialView::IncrementalStationDataCurveEnvironment(void)
{
	NFmiDrawingEnvironment envi;
	envi.DisableFill();
	envi.SetFrameColor(NFmiColor(1.0f,0.f,1.f));
	envi.SetFontSize(NFmiPoint(18,18));
	envi.BoldFont(true);
	return envi;
}

NFmiDrawingEnvironment NFmiTimeSerialView::ChangeIncrementalStationDataCurveEnvironment(void)
{
	NFmiDrawingEnvironment envi;
	envi.DisableFill();
	envi.SetFrameColor(NFmiColor(1.f,0.3f,0.8f));
	envi.SetFillPattern(FMI_RELATIVE_FILL);
	envi.SetRelativeFill(0.6);
	envi.SetSubLinePerUnit(50./itsRect.Width());
	return envi;
}

//--------------------------------------------------------
// IsModifiedValueLineDrawn
//--------------------------------------------------------
// desides if the possible modified curve is draw (if modifications won't make any
// difference, then help line won't be drawn)
bool NFmiTimeSerialView::IsModifiedValueLineDrawn(long theEndPointIndex)
{
	if(!IsAnalyzeRelatedToolUsed()) // analyysi modessa ei piirret‰ n‰it‰ muutosk‰yri‰!!!
	{
		if(itsDrawParam->TimeSerialModifyingLimit())
		{
			if(theEndPointIndex >= 0 && static_cast<unsigned long>(theEndPointIndex) < itsModificationFactorCurvePoints.size())
			{
				// one of the change factor values (start or end -point) must differ from zero or there will not be any change!
				if(itsModificationFactorCurvePoints[theEndPointIndex-1] ||
					itsModificationFactorCurvePoints[theEndPointIndex])
				{
					return true;
				}
			}
		}
	}
	return false;
}
//--------------------------------------------------------
// CalcModifiedValuePoint
//--------------------------------------------------------
NFmiPoint NFmiTimeSerialView::CalcModifiedValuePoint(float theRealValue, long theIndex, float theMaskFactor)
{
	return CalcRelativeValuePosition(Info()->Time(), CalcModifiedValue(theRealValue, theIndex, theMaskFactor));
}

//--------------------------------------------------------
// CalcModifiedValue
//--------------------------------------------------------
float NFmiTimeSerialView::CalcModifiedValue(float theRealValue, long theIndex, float theMaskFactor)
{
	if(theRealValue == kFloatMissing || theMaskFactor == kFloatMissing)
		return kFloatMissing;
	float returnValue = kFloatMissing;
	if(theIndex >= 0 && static_cast<unsigned long>(theIndex) < itsModificationFactorCurvePoints.size())
	{
		if (itsModifyingUnit == 0)
			returnValue = ((theRealValue * itsModificationFactorCurvePoints[theIndex] * theMaskFactor / 100) + theRealValue);
		else
			returnValue = theRealValue + itsModificationFactorCurvePoints[theIndex] * theMaskFactor;
	}
	return returnValue;
}
//--------------------------------------------------------
// DrawModifyFactorPoints
//--------------------------------------------------------
void NFmiTimeSerialView::DrawModifyFactorPoints(void)
{
	if(itsOperationMode != TimeSerialOperationMode::NormalDrawMode)
		return ;
	itsToolBox->UseClipping(true);
	if(itsCtrlViewDocumentInterface->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal) // jos ns. edit-moodi p‰‰ll‰, piiret‰‰n aikarajoitin viivat
	{
		if(!IsAnalyzeRelatedToolUsed()) // muutos k‰yr‰t piirret‰‰n vain ei-analyysi tilassa
		{
			DrawModifyFactorPointGrids();
			if(itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode())
			{
				auto& modFacVec = itsCtrlViewDocumentInterface->CPManager()->ActiveCPChangeValues();
				itsModificationFactorCurvePoints = modFacVec;
			}

			DrawModifyFactorPointsManual();
		}
	}
}

//--------------------------------------------------------
// DrawModifyFactorPointsManual
//--------------------------------------------------------
void NFmiTimeSerialView::DrawModifyFactorPointsManual(void)
{
	NFmiPoint pointSize(6.,6.);

	NFmiDrawingEnvironment envi;
	envi.EnableFill();
	envi.SetFillColor(NFmiColor(1.f,0.f,0.f));
	envi.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	std::vector<NFmiMetTime> times;
	FillTimeSerialTimesFromInfo(*Info(), times);
	PlotTimeSerialData(itsModificationFactorCurvePoints, times, envi, pointSize, pointSize, false);
}

//--------------------------------------------------------
// DrawModifyFactorPointGrids
//--------------------------------------------------------
// now draws only zero-line
void NFmiTimeSerialView::DrawModifyFactorPointGrids(void)
{
	NFmiDrawingEnvironment envi;
	envi.DisableFill();
	envi.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	NFmiTimeBag timeBag = ZoomedTimeBag();
	NFmiPoint point1(CalcRelativeModifyFactorPosition(timeBag.FirstTime(),0))
			 ,point2(CalcRelativeModifyFactorPosition(timeBag.LastTime(),0));
	NFmiLine line(point1, point2, 0, &envi);
	itsToolBox->Convert(&line);
}

//--------------------------------------------------------
// CalcRelativeValuePosition
//--------------------------------------------------------
NFmiPoint NFmiTimeSerialView::CalcRelativeValuePosition(const NFmiMetTime& theTime, double theValue)
{
	NFmiPoint point;
	point.X(Time2Value(theTime));
	point.Y(Value2AxisPosition((float)theValue));
	return point;
}

//--------------------------------------------------------
// CalcRelativeModifyFactorPosition
//--------------------------------------------------------
NFmiPoint NFmiTimeSerialView::CalcRelativeModifyFactorPosition(const NFmiMetTime& theTime, double theValue)
{
	NFmiPoint point;
	point.X(Time2Value(theTime));
	point.Y(ModifyFactor2AxisPosition((float)theValue));
	return point;
}

//--------------------------------------------------------
// DrawPoint
//--------------------------------------------------------
void NFmiTimeSerialView::DrawPoint(NFmiDrawingEnvironment& envi, const NFmiPoint& theRelativePoint, const NFmiPoint& thePointSize)
{
	NFmiPoint topLeft;
	NFmiPoint bottomRight;
	topLeft.Set(theRelativePoint.X() - itsToolBox->SX(long(thePointSize.X()/2.))
			 ,theRelativePoint.Y() - itsToolBox->SY(long(thePointSize.Y()/2.)));
	bottomRight.Set(theRelativePoint.X() + itsToolBox->SX(long(thePointSize.X()/2.))
			 ,theRelativePoint.Y() + itsToolBox->SY(long(thePointSize.Y()/2.)));
	NFmiRectangle rect(topLeft, bottomRight, 0,&envi);
	itsToolBox->Convert(&rect);
}

//--------------------------------------------------------
// DrawPoint
//--------------------------------------------------------
void NFmiTimeSerialView::DrawPointInDataRect(NFmiDrawingEnvironment& envi, const NFmiPoint& theRelativePoint, const NFmiPoint& thePointSize)
{
	NFmiRect valueRect = CalcValueAxisRect();
	valueRect.Size(NFmiPoint(1., valueRect.Size().Y())); // t‰m‰ ei ole oikea datarect x-suunnassa, mutta riitt‰‰ t‰ss‰ toistaiseksi
	bool isInDataRect = valueRect.IsInside(theRelativePoint);
	if(isInDataRect)
		DrawPoint(envi, theRelativePoint, thePointSize);
}

//--------------------------------------------------------
// CalcMaxValueAxisRect
//--------------------------------------------------------
NFmiRect NFmiTimeSerialView::CalcMaxValueAxisRect(void)
{
	NFmiRect axisRect(GetFrame());
	axisRect.Inflate(0., -itsToolBox->SY(1));
	axisRect.Right(axisRect.Left());
	axisRect.Top(CalcTimeAxisRect().Bottom());
	return axisRect;
}
//--------------------------------------------------------
// CalcValueAxisRect
//--------------------------------------------------------
NFmiRect NFmiTimeSerialView::CalcValueAxisRect(void)
{
	NFmiRect axisRect(GetFrame());
	double emptySpace = axisRect.Height()/50.; // how much is slack at the top and bottom of the scale to the whole view
	axisRect.Top(axisRect.Top() + 6. * emptySpace); // at the top there is also the time axis (in the future not!)
	axisRect.Bottom(axisRect.Bottom() - emptySpace);
	axisRect.Right(axisRect.Width()/12.);
	return axisRect;
}

//--------------------------------------------------------
// CalcModifyFactorAxisRect
//--------------------------------------------------------
NFmiRect NFmiTimeSerialView::CalcModifyFactorAxisRect(void)
{
	NFmiRect axisRect(CalcValueAxisRect()); // tiet‰‰ korkeuden ja leveyden, positio lasketaan uudelleen
	double width = GetFrame().Width();
	axisRect.Left(axisRect.Left() + width - width/25.);
	axisRect.Right(axisRect.Right() + width - width/20.);
	return axisRect;
}

//--------------------------------------------------------
// DrawValueAxis
//--------------------------------------------------------
void NFmiTimeSerialView::DrawValueAxis(void)
{
	itsToolBox->UseClipping(false);

	FmiDirection oldAlign = itsToolBox->GetTextAlignment();
	NFmiDrawingEnvironment envi;
	envi.SetFrameColor(NFmiColor(0.5f,0.5f,0.5f));

	envi.DisableFrame();
	NFmiRect cat = CalcValueAxisRect();
	cat.Inflate(0.,-itsToolBox->SY(1));
	NFmiRectangle rect(cat.TopLeft()
					  ,cat.BottomRight()
					  ,0
					  ,&envi);
	itsToolBox->Convert(&rect);
	if(itsValueView)
		itsValueView->DrawAll();
	itsToolBox->SetTextAlignment(oldAlign);

}

//--------------------------------------------------------
// DrawModifyFactorAxis
//--------------------------------------------------------
void NFmiTimeSerialView::DrawModifyFactorAxis(void)
{
	itsToolBox->UseClipping(false);
	if(itsCtrlViewDocumentInterface->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal) // jos ns. edit-moodi p‰‰ll‰, piiret‰‰n aikarajoitin viivat
	{
		if(itsModifyFactorView && (!IsAnalyzeRelatedToolUsed())) // muutos asteikko piirret‰‰n vain ei-analyysi tilassa
		{
			NFmiDrawingEnvironment envi;
			envi.SetFrameColor(NFmiColor(0.f,0.f,0.f));

			envi.DisableFrame();
			NFmiRect cat = CalcModifyFactorAxisRect();
			cat.Inflate(0.,-itsToolBox->SY(1));
			NFmiRectangle rect(cat.TopLeft()
							,cat.BottomRight()
							,0
							,&envi);
			itsToolBox->Convert(&rect);
			itsModifyFactorView->DrawAll();
		}
	}
}

bool NFmiTimeSerialView::IsEditedData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo) const
{
    return theInfo->DataType() == NFmiInfoData::kEditable;
}

//--------------------------------------------------------
// LeftButtonUp
//--------------------------------------------------------
bool NFmiTimeSerialView::LeftButtonUp(const NFmiPoint &thePlace
											   ,unsigned long theKey)
{
    bool tmpEditingMouseMotionsAllowed = fEditingMouseMotionsAllowed; // pit‰‰ ottaa talteen originaali arvo, jotta se voidaan asettaa heti 
                                                                    // alkuun false tilaan, koska sit‰ ei voi asettaa lopuksi, koska metodissa on niin monta return -kohtaa.
    fEditingMouseMotionsAllowed = false; // astetaan originaali muuttuja siis heti false tilaan

    itsCtrlViewDocumentInterface->MouseCapturedInTimeWindow(false);

	if(IsIn(thePlace))
	{
        itsCtrlViewDocumentInterface->TimeSerialViewDirty(true);
		itsCtrlViewDocumentInterface->SetLastActiveDescTopAndViewRow(CtrlViewUtils::kFmiTimeSerialView, itsViewGridRowNumber);

        // kun monta alin‰yttˆ‰ yht‰aikaa, pit‰‰ ensimm‰isell‰ klikkauksella asettaa kyseinen n‰yttˆ 'editointitilaan'
		if(itsDrawParam && (!itsDrawParam->IsParamEdited()))
		{
			itsCtrlViewDocumentInterface->TimeSerialViewDrawParamList()->DisableEditing();
			itsDrawParam->EditParam(true);
			return true;
		}
		itsInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, false, false);
		if(itsInfo == 0)
			return false;
		if(IsModifyFactorViewClicked(thePlace))
		{
			if(IsEditedData(itsInfo))
				return ChangeModifyFactorView(-1.);
			else
				return false;
		}
		if(itsValueView && itsValueView->GetFrame().IsInside(thePlace))
		{
			// t‰m‰ seuraava tarkastus johtuu itsValueView:in 'oudosta' leveydest‰
			// n‰ytt‰‰ ett‰ itsValueView:in pit‰‰ olla todella leve‰,
			// ennenkuin se suostuu piirt‰m‰‰n fontit tarpeeksi isolla
			// nyt tarkistetaan ettei klikkaus mene itsTimeView:in alueelle,
			// koska sen tarkastelu on t‰rke‰mp‰‰ ja oikeampaa t‰ss‰ tapauksessa
			if(thePlace.X() < itsTimeView->GetFrame().Left())
			{
				if(theKey & kCtrlKey)
				{
					return MoveValueView(-1.);
				}
				else
				{
					if(Position2ModifyFactor(thePlace) > 0.)
						return ChangeValueView(-1., true); // true liikuttaa asteikon yl‰p‰‰t‰ ja false alap‰‰t‰
					else
						return ChangeValueView(-1., false);
				}
			}
		}

        {
            if(tmpEditingMouseMotionsAllowed) // testataan tmp-muuttujaa, koska originaali on jo asetettu false:ksi
            {
	            // kuinka l‰helt‰ pit‰‰ aika-akselia klikata ennenkuin ohjelma suostuu 'lˆyt‰m‰‰n'
	            // klikkauksen paikan (nyt lineaariselle laitetaan isommat 'reunat' hakua varten)
			    if(!IsEditedData(itsInfo))
				    return false;

			    double proximityFactor = CalcMouseClickProximityFactor();
			    if(IsAnalyzeRelatedToolUsed())
			    {
				    NFmiMetTime analyzeEndTime(Value2Time(thePlace));
                    itsCtrlViewDocumentInterface->AnalyzeToolData().AnalyzeToolEndTime(analyzeEndTime);
				    return true;
			    }
			    else
			    {
				    int index;
				    if(FindTimeIndex(thePlace.X(), proximityFactor, index))
				    {
						float value = kFloatMissing;
					    if(itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode())
					    { 
							// 13.11.2002/Marko Muutos CP-tyˆkalun k‰ytˆkseen siten, ett‰ piirret‰‰n lopullista arvok‰yr‰‰ haluttuun pisteeseen.
						    value = Position2Value(thePlace);
					    }
					    else
					    {
						    value = Position2ModifyFactor(thePlace);
						    FixModifyFactorValue(value);
					    }
					    bool status = ModifyFactorPointsManual(value, index);

					    if(itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode())
					    {
                            itsCtrlViewDocumentInterface->CPManager()->Param(itsDrawParam->Param());
						    auto& modFacVec = itsCtrlViewDocumentInterface->CPManager()->ActiveCPChangeValues();
						    modFacVec = itsModificationFactorCurvePoints; // laitetaan kyseisen parametrin ja kyseisen CP:n muutokset talteen
					    }
					    return status;
				    }
                }
			}
		}
	}
	return false;
}

void NFmiTimeSerialView::ScanDataForNacroParamCase(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, boost::shared_ptr<NFmiDrawParam>& theMacroParamDrawParam, const NFmiPoint& theLatlon, NFmiDataModifierMinMax& theAutoAdjustMinMaxValuesOut)
{
	std::vector<NFmiMetTime> times = MakeMacroParamTimeVector();
	std::vector<float> values(times.size(), kFloatMissing);
	FillTimeSerialMacroParamData(theLatlon, values, times, theInfo, theMacroParamDrawParam, nullptr, nullptr);
	bool doCsvDataGeneration = (itsOperationMode == TimeSerialOperationMode::CsvDataGeneration);
	std::list<NFmiMetTime> csvGenerationTimes;
	std::list<float> csvGenerationParameterValues;
	for(size_t index = 0; index < times.size(); index++)
	{
		DoScanningPhaseValueAdding(doCsvDataGeneration, values[index], times[index], 0, theAutoAdjustMinMaxValuesOut, csvGenerationTimes, csvGenerationParameterValues);
	}

	DoScanningPhaseTimeSerialAdding(doCsvDataGeneration, theInfo, theLatlon, csvGenerationTimes, csvGenerationParameterValues, theMacroParamDrawParam);
}

void NFmiTimeSerialView::ScanDataForSpecialOperation(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, boost::shared_ptr<NFmiDrawParam>& theDrawParam, const NFmiPoint& theLatlon, const NFmiTimeBag& theLimitingTimes, NFmiDataModifierMinMax& theAutoAdjustMinMaxValuesOut, const NFmiFastInfoUtils::MetaWindParamUsage& metaWindParamUsage, unsigned long wantedParamId, int theTimeWhenDrawedInMinutes)
{
	if(theInfo && theInfo->DataType() == NFmiInfoData::kTimeSerialMacroParam)
	{
		ScanDataForNacroParamCase(theInfo, theDrawParam, theLatlon, theAutoAdjustMinMaxValuesOut);
		return;
	}

	bool interpolateLocation = theInfo->IsGrid();
	FastInfoTimeLimits timeLimits(theInfo, theLimitingTimes);
	if(timeLimits.isValid())
	{
		bool doCsvDataGeneration = (itsOperationMode == TimeSerialOperationMode::CsvDataGeneration);
		std::list<NFmiMetTime> csvGenerationTimes;
		std::list<float> csvGenerationParameterValues;
		for(auto timeIndex = timeLimits.startTimeIndex_; timeIndex <= timeLimits.endTimeIndex_; timeIndex++)
		{
			if(theInfo->TimeIndex(timeIndex))
			{
				float value = static_cast<float>(::GetTimeSerialValue(theInfo, interpolateLocation, theLatlon, metaWindParamUsage, wantedParamId));
				DoScanningPhaseValueAdding(doCsvDataGeneration, value, theInfo->ValidTime(), theTimeWhenDrawedInMinutes, theAutoAdjustMinMaxValuesOut, csvGenerationTimes, csvGenerationParameterValues);
			}
		}

		DoScanningPhaseTimeSerialAdding(doCsvDataGeneration, theInfo, theLatlon, csvGenerationTimes, csvGenerationParameterValues);
	}
}

void NFmiTimeSerialView::DoScanningPhaseTimeSerialAdding(bool doCsvDataGeneration, boost::shared_ptr<NFmiFastQueryInfo>& theInfo, const NFmiPoint& theLatlon, std::list<NFmiMetTime>& csvGenerationTimes, std::list<float>& csvGenerationParameterValues, boost::shared_ptr<NFmiDrawParam> possibleMacroParamDrawParam)
{
	if(doCsvDataGeneration && !csvGenerationParameterValues.empty())
	{
		auto parameterNameString = MakeCsvFullParameterNameString(theInfo, possibleMacroParamDrawParam);
		auto parameterAlreadyIncludedIter = std::find(itsCsvGenerationParameterNames.begin(), itsCsvGenerationParameterNames.end(), parameterNameString);
		if(parameterAlreadyIncludedIter == itsCsvGenerationParameterNames.end())
		{
			// Estet‰‰n ett‰ sama parametri (prod+par+level) ei mene kahdesti dataan
			itsCsvGenerationParameterNames.push_back(parameterNameString);
			itsCsvGenerationTimes.emplace_back(std::move(csvGenerationTimes));
			itsCsvGenerationParameterValues.emplace_back(std::move(csvGenerationParameterValues));
			if(itsCsvGenerationLatlon == NFmiPoint::gMissingLatlon)
				itsCsvGenerationLatlon = theLatlon;
		}
	}
}

void NFmiTimeSerialView::DoScanningPhaseValueAdding(bool doCsvDataGeneration, float value, NFmiMetTime validTimeCopy, int theTimeWhenDrawedInMinutes, NFmiDataModifierMinMax& theAutoAdjustMinMaxValuesOut, std::list<NFmiMetTime> &csvGenerationTimesOut, std::list<float> &csvGenerationParameterValuesOut)
{
	if(doCsvDataGeneration)
	{
		// Datasta saatu aika pit‰‰ viel‰ mahdollisesti siirt‰‰ aikasarjassa olevaan aikaan
		validTimeCopy.ChangeByMinutes(theTimeWhenDrawedInMinutes);
		csvGenerationTimesOut.push_back(validTimeCopy);
		csvGenerationParameterValuesOut.push_back(value);
	}
	else
	{
		theAutoAdjustMinMaxValuesOut.Calculate(value);
	}

}

std::vector<NFmiPoint> NFmiTimeSerialView::GetViewedLatlonPoints(void)
{
	std::vector<NFmiPoint> latlons;
	int counter = 1;

	boost::shared_ptr<NFmiFastQueryInfo> editedInfo = itsCtrlViewDocumentInterface->EditedSmartInfo();
	if(editedInfo)
	{
        EditedInfoMaskHandler editedInfoMaskHandler(editedInfo, CtrlViewFastInfoFunctions::GetProperMaskTypeFromEditeInfo(editedInfo, itsCtrlViewDocumentInterface->AllowRightClickDisplaySelection()));

		editedInfo->ResetLocation();
		if(editedInfo->NextLocation())
			latlons.push_back(editedInfo->LatLon());

        if(editedInfo->MaskType() == NFmiMetEditorTypes::kFmiDisplayedMask && itsCtrlViewDocumentInterface->AllowRightClickDisplaySelection())
		{
			while(editedInfo->NextLocation())
				latlons.push_back(editedInfo->LatLon());
		}
	}
	// piirrret‰‰n viel‰ editoidun alueen ulkopuolelta mahd. valittu piste mutta vain jos muuta ei ole piirretty
	if(counter == 1)
	{
		if(itsCtrlViewDocumentInterface->OutOfEditedAreaTimeSerialPoint() != NFmiPoint::gMissingLatlon)
			latlons.push_back(itsCtrlViewDocumentInterface->OutOfEditedAreaTimeSerialPoint());
	}
	return latlons;
}

// Funktio joka etsii asteikon min ja max arvojen avulla
// sopivimman stepin. Stepin pit‰‰ olla luku joka on 1, 2 tai 5.
// Tai jokin niiden 10. potenssi eli esim. 0.2, 0.02, ... tai 20, 200, ...
// oletus, max ja min eiv‰t ole kFloatMissing
static float GetAxisStepValue(float theMinValue, float theMaxValue)
{
	if(theMinValue == kFloatMissing || theMaxValue == kFloatMissing)
		return 1.f;

	float diff = theMaxValue - theMinValue;
	if(diff == 0)
		return 1;
	float step = diff / 20.f;
	float powPlus = 0.f;
	for(; step < 1;)
	{
		step *= 10.f;
		powPlus++;
	}
	float powMinus = 0.f;
	for(; step > 10;)
	{
		step /= 10.f;
		powMinus++;
	}
	// Nyt stepin pit‰isi olla 1 ja 10 v‰lill‰
	if(step < 1.5f)
		step = 1.f;
	else if(step < 3.5f)
		step = 2.f;
	else if(step < 7.5f)
		step = 5.f;
	else
	{
		step = 1.f;
		powMinus++;
	}

	// palutetaan stepin suuruus luokka
	if(powMinus)
		step = step * std::powf(10.f, powMinus);
	if(powPlus)
		step = step / std::powf(10.f, powPlus);

	return step;
}

static float RoundValue(float value, float rounder)
{
	return ::round(value/rounder) * rounder;
}

#ifdef min
#undef min
#undef max
#endif

static NFmiTimeBag GetScannedTimes(const NFmiTimeBag &theViewTimes)
{
	long stepInMinutes = theViewTimes.Resolution();
	long lengthInMinutes = theViewTimes.LastTime().DifferenceInMinutes(theViewTimes.FirstTime());
	float extensionLengthInMinutes = lengthInMinutes/2.f; // lis‰t‰‰n molempiin p‰ihin ajallisesti puolet aikan‰ytˆn pituudesta
	extensionLengthInMinutes = std::min(extensionLengthInMinutes, 2.f*24*60); // minimiss‰‰n kuitenkin haarukkaan tulee kaksi p‰iv‰‰
	extensionLengthInMinutes = std::max(extensionLengthInMinutes, 7.f*24*60); // maksimissaan haarukkaan tulee viikko
	float roundedExtension = ::RoundValue(extensionLengthInMinutes, static_cast<float>(stepInMinutes));
	NFmiMetTime newFirstTime(theViewTimes.FirstTime());
	newFirstTime.ChangeByMinutes(static_cast<long>(-roundedExtension));
	NFmiMetTime newLastTime(theViewTimes.LastTime());
	newLastTime.ChangeByMinutes(static_cast<long>(roundedExtension));
	return NFmiTimeBag(newFirstTime, newLastTime, stepInMinutes);
}

bool NFmiTimeSerialView::AutoAdjustValueScale(void)
{
	itsOperationMode = TimeSerialOperationMode::MinMaxScanMode;
	itsAutoAdjustMinMaxValues.Clear();
	itsScannedLatlonPoints = GetViewedLatlonPoints();
	itsAutoAdjustScanTimes = ::GetScannedTimes(GetViewLimitingTimes());
	itsInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, false, true);
	DrawSelectedStationData(); // skannataan piirto-systeemi l‰pi ilman piirtoa etsien min/max arvoja eri datoista
	itsToolBox->UseClipping(false);
	itsDrawingEnvironment->EnableFill();
	
	itsOperationMode = TimeSerialOperationMode::NormalDrawMode;

	float minValue = itsAutoAdjustMinMaxValues.MinValue();
	float maxValue = itsAutoAdjustMinMaxValues.MaxValue();
	if(minValue != kFloatMissing && maxValue != kFloatMissing)
	{
		float step = GetAxisStepValue(minValue, maxValue);

		float oldAxisStartValue = static_cast<float>(itsDrawParam->TimeSeriesScaleMin());
		float oldAxisEndValue = static_cast<float>(itsDrawParam->TimeSeriesScaleMax());
	// 3. Jos min ja max arvot mahtivat vanhan akselin sis‰‰n ja niiden abs. ero akseleihin on tietyn rajan 
	// sis‰ll‰ esim. step * 3, ei akselia tarvitse s‰‰t‰‰
		if(!((minValue >= oldAxisStartValue) && (maxValue <= oldAxisEndValue) && (::fabs(minValue - oldAxisStartValue) <= step*5) && (::fabs(maxValue - oldAxisEndValue) <= step*5)))
		{

	// 4. jos min ja max ovat ei-puuttuvia, laske sopiva pyˆristys alas min-arvosta ja sopiva pyˆristys ylˆs max-arvosta
			float newAxisStartValue = (::round(minValue/step) * step) - 3*step;
			float newAxisEndValue = (::round(maxValue/step) * step) + 3*step;

	// 5. Aseta uudet arvot itsDrawParamille
			itsDrawParam->TimeSeriesScaleMin(newAxisStartValue);
			itsDrawParam->TimeSeriesScaleMax(newAxisEndValue);
			itsDrawParam->TimeSerialModifyingLimit(step);
	// 6. Luo uusi arvoasteikko
			CreateValueScaleView();
			return true;
		}
	}
	// 8. jos ei, palauta false
	return false;
}

bool NFmiTimeSerialView::LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey)
{
    fEditingMouseMotionsAllowed = false;
	if(IsIn(thePlace))
	{
		if(itsDrawParam && itsDrawParam->IsParamEdited())
		{
			itsCtrlViewDocumentInterface->MouseCapturedInTimeWindow(true);
		}
        if(itsDataRect.IsInside(thePlace))
            fEditingMouseMotionsAllowed = true;
	}
	return false;
}

bool NFmiTimeSerialView::MouseMove(const NFmiPoint &thePlace, unsigned long theKey)
{
	if(!itsCtrlViewDocumentInterface->MouseCapturedInTimeWindow())
		return false;
	if(IsIn(thePlace))
	{
        itsCtrlViewDocumentInterface->TimeSerialViewDirty(true);
		if(itsDrawParam && itsDrawParam->IsParamEdited())
		{
			itsInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, false, false);
			if(itsInfo == 0)
				return false;

			NFmiRect valueRect = CalcValueAxisRect();
			NFmiRect timeRect = CalcTimeAxisRect();
			NFmiRect editAreaRect(timeRect.Left(), valueRect.Top(), timeRect.Right(), valueRect.Bottom());
			if(!editAreaRect.IsInside(thePlace))
				return false;

            if(EditingMouseMotionsAllowed())
            {
                // kuinka l‰helt‰ pit‰‰ aikaakselia klikata ennenkuin ohjelma suostuu 'lˆyt‰m‰‰n'
	            // klikkauksen paikan (nyt lineaariselle laitetaan isommat 'reunat' hakua varten)
			    double proximityFactor = CalcMouseClickProximityFactor();
			    if(IsAnalyzeRelatedToolUsed())
			    {
				    NFmiMetTime analyzeEndTime(Value2Time(thePlace));
                    itsCtrlViewDocumentInterface->AnalyzeToolData().AnalyzeToolEndTime(analyzeEndTime);
				    return true;
			    }
			    else
			    {
				    int index = 0;
				    if(FindTimeIndex(thePlace.X(), proximityFactor, index))
				    {
						float value = kFloatMissing;
					    if(itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode())
					    { // 13.11.2002/Marko Muutos CP-tyˆkalun k‰ytˆkseen siten, ett‰ piirret‰‰n lopullista arvok‰yr‰‰ haluttuun pisteeseen.
						    value = Position2Value(thePlace);
					    }
					    else
					    {
						    value = Position2ModifyFactor(thePlace);
							FixModifyFactorValue(value);
					    }

					    bool status = ModifyFactorPointsManual(value, index);

					    if(itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode())
					    {
                            itsCtrlViewDocumentInterface->CPManager()->Param(itsDrawParam->Param());
						    auto& modFacVec = itsCtrlViewDocumentInterface->CPManager()->ActiveCPChangeValues();
						    modFacVec = itsModificationFactorCurvePoints; // laitetaan kyseisen parametrin ja kyseisen CP:n muutokset talteen
					    }
					    return status;
				    }
                }
			}
		}
	}
	return false;
}

double NFmiTimeSerialView::CalcMouseClickProximityFactor(void)
{
	// LAITA TƒMƒ UUSIKSI WCTR:n TAKIA!!!!!!!!
	double value = (itsTimeView->GetFrame().Width() / (ZoomedTimeDescriptor().Size()-1)) / 2.;
	return value;
}

// Tarkistaa, ollaanko klikattu hiiren oikealla napilla otsikkoalueelta.
// Jos on, avataan timeserialView-popup, jolla voidaan manipuloida suoraan kyseist‰ n‰yttˆ‰.
// otsikko alue on Frame:n Top:in alla ja itsValueView:in yll‰.
bool NFmiTimeSerialView::OpenOverViewPopUp(const NFmiPoint &thePlace, unsigned long /* theKey */ )
{
	if(itsValueView && itsValueView->GetFrame().Top() > thePlace.Y() && GetFrame().Top() < thePlace.Y())
	{
		itsCtrlViewDocumentInterface->CreateTimeSerialDialogOnViewPopup(itsViewGridRowNumber);
		return true;
	}
	return false;
}

//--------------------------------------------------------
// RightButtonUp
//--------------------------------------------------------
bool NFmiTimeSerialView::RightButtonUp(const NFmiPoint &thePlace
										  ,unsigned long theKey)
{
	if(IsIn(thePlace))
	{
		itsCtrlViewDocumentInterface->TimeSerialViewDirty(true);
		// kun monta alin‰yttˆ‰ yht‰aikaa, pit‰‰ ensimm‰isell‰ klikkauksella asettaa kyseinen n‰yttˆ 'editointitilaan'
		if(itsDrawParam && (!itsDrawParam->IsParamEdited()))
		{
            itsCtrlViewDocumentInterface->TimeSerialViewDrawParamList()->DisableEditing();
			itsDrawParam->EditParam(true);
			return OpenOverViewPopUp(thePlace, theKey);
		}
		itsInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, false, false);
		if(itsInfo == 0)
			return false;

		if(OpenOverViewPopUp(thePlace, theKey))
			return true;
		if(IsModifyFactorViewClicked(thePlace))
		{
			return ChangeModifyFactorView(1.);
		}
		if(itsValueView && itsValueView->GetFrame().IsInside(thePlace))
		{
			// t‰m‰ seuraava tarkastus johtuu itsValueView:in 'oudosta' leveydest‰
			// n‰ytt‰‰ ett‰ itsValueView:in pit‰‰ olla todella leve‰,
			// ennenkuin se suostuu piirt‰m‰‰n fontit tarpeeksi isolla
			// nyt tarkistetaan ettei klikkaus mene itsTimeView:in alueelle,
			// koska sen tarkastelu on t‰rke‰mp‰‰ ja oikeampaa t‰ss‰ tapauksessa
			if(thePlace.X() < itsTimeView->GetFrame().Left())
			{
				if(theKey & kCtrlKey)
				{
					return MoveValueView(1.);
				}
				else
				{
					if(Position2ModifyFactor(thePlace) > 0.)
						return ChangeValueView(1., true); // true liikuttaa asteikon yl‰p‰‰t‰ ja false alap‰‰t‰
					else
						return ChangeValueView(1., false);
				}
			}
		}
		return true; // jos n‰ytˆn sis‰ll‰, palauttaa true (ainakin v‰liaikaisesti, muuten is‰nt‰ avaa v‰‰r‰n popupin)
	}
	return false;
}

//--------------------------------------------------------
// ModifyFactorPointsManual
//--------------------------------------------------------
bool NFmiTimeSerialView::ModifyFactorPointsManual(float theValue, int theIndex)
{
    if(itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode())
	{ 
		// 13.11.2002/Marko Muutos CP-tyˆkalun k‰ytˆkseen siten, ett‰ piirret‰‰n lopullista arvok‰yr‰‰ haluttuun pisteeseen.
		NFmiPoint latlon(itsCtrlViewDocumentInterface->CPManager()->ActiveCPLatLon());
		Info()->TimeIndex(theIndex);
		float value = Info()->InterpolatedValue(latlon);
		if(theValue != kFloatMissing && value != kFloatMissing)
			itsModificationFactorCurvePoints[theIndex] = (theValue - value);
	}
	else
	{
		if(theValue != kFloatMissing)
			itsModificationFactorCurvePoints[theIndex] = theValue;
	}
	return true;
}

//--------------------------------------------------------
// FixModifyFactorValue
//--------------------------------------------------------
void NFmiTimeSerialView::FixModifyFactorValue(float& theValue)
{
	float limit = (float)itsDrawParam->TimeSerialModifyingLimit();
	if(theValue < -limit)
		theValue = -limit;
	if(theValue > limit)
		theValue = limit;
}

//--------------------------------------------------------
// MaxStationShowed
//--------------------------------------------------------
void NFmiTimeSerialView::MaxStationShowed(unsigned int newCount)
{
	itsMaxStationShowed = newCount;
}

//--------------------------------------------------------
// RightButtonDown
//--------------------------------------------------------
bool NFmiTimeSerialView::RightButtonDown(const NFmiPoint& thePlace ,unsigned long /* theKey */)
{
	if(IsIn(thePlace))
	{
		return true;
	}
	return false;
}

//--------------------------------------------------------
// DrawBackground
//--------------------------------------------------------
void NFmiTimeSerialView::DrawBackground(void)
{
	NFmiDrawingEnvironment envi;
	envi.EnableFill();
	envi.EnableFrame();
	envi.SetFillColor(NFmiColor(1.f,1.f,0.97f));
	if(itsDrawParam && itsDrawParam->IsParamEdited())
	{
		envi.SetPenSize(NFmiPoint(2.,2.));
		envi.SetFrameColor(NFmiColor(1.f,0.f,0.f));
	}
	else
	{
		envi.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	}
	NFmiRectangle rect(itsRect.TopLeft()
					  ,itsRect.BottomRight()
					  ,0
					  ,&envi);
	itsToolBox->Convert(&rect);
	envi.DisableFill();
	NFmiPoint newTopLeft(itsRect.TopLeft().X() + itsToolBox->SX(1), itsRect.TopLeft().Y() + itsToolBox->SY(1));
	NFmiPoint newBottomRight(itsRect.BottomRight().X() - itsToolBox->SX(1), itsRect.BottomRight().Y() - itsToolBox->SY(1));
	NFmiRectangle rect2(newTopLeft
					  ,newBottomRight
					  ,0
					  ,&envi);
	itsToolBox->Convert(&rect2);
}

void NFmiTimeSerialView::DrawShade(NFmiDrawingEnvironment &theEnvi, const NFmiMetTime &theTime, short theChangeByMinutes, double theYShift)
{
	NFmiMetTime time2(theTime);
	time2.ChangeByMinutes(theChangeByMinutes);

	NFmiPoint topLeft(itsRect.TopLeft());
	topLeft.X(Time2Value(theTime));
	topLeft.Y(topLeft.Y() + theYShift);
	NFmiPoint bottomRight(itsRect.BottomRight());
	bottomRight.X(Time2Value(time2));
	bottomRight.Y(bottomRight.Y() - theYShift);
	NFmiRectangle rect(topLeft
					  ,bottomRight
					  ,0
					  ,&theEnvi);
	itsToolBox->Convert(&rect);
}

// piirt‰‰ yˆhˆn 00-06 utc varjostuksen
// piirt‰‰ myˆs 12 utc:n kohdalle viivan
void NFmiTimeSerialView::DrawNightShades(void)
{
	itsToolBox->RelativeClipRect(itsDataRect, true);
	if(Info())
	{
		NFmiDrawingEnvironment enviNight;
		enviNight.EnableFill();
		enviNight.DisableFrame();
		enviNight.SetFillColor(NFmiColor(0.92f,0.92f,0.995f));
		NFmiDrawingEnvironment enviDay;
		enviDay.EnableFill();
		enviDay.DisableFrame();
		enviDay.SetFillColor(NFmiColor(0.995f,0.92f,0.92f));
		double shiftInYDirection = itsToolBox->SY(3);

		short usedTimeStep = 6*60;
		NFmiTimeBag timesInView(itsCtrlViewDocumentInterface->TimeSerialViewTimeBag());
		NFmiMetTime startTime = timesInView.FirstTime();
		startTime.SetTimeStep(usedTimeStep);
		startTime.PreviousMetTime(); // laitetaan aloitus varmasti ennen n‰ytˆn alkua
		
		NFmiMetTime endTime = timesInView.LastTime();
		endTime.SetTimeStep(usedTimeStep);
		endTime.NextMetTime(); // laitetaan loppu aika varmasti ohi n‰ytˆn lopun

		NFmiTimeBag usedTimes(startTime, endTime, usedTimeStep);

		for(usedTimes.Reset(); usedTimes.Next(); )
		{
			NFmiMetTime time1 = usedTimes.CurrentTime();
			if(time1.GetHour() == 0)
				DrawShade(enviNight, time1, usedTimeStep, shiftInYDirection);
			else if(time1.GetHour() == 12)
				DrawShade(enviDay, time1, usedTimeStep, shiftInYDirection);
		}
	}
}

//--------------------------------------------------------
// CreateValueScaleView
//--------------------------------------------------------
void NFmiTimeSerialView::CreateValueScaleView(void)
{
	delete itsValueAxis;
	delete itsValueView;
	double lowLimit = itsDrawParam->TimeSeriesScaleMin()
		  ,highLimit = itsDrawParam->TimeSeriesScaleMax();

	NFmiStepScale scale((float)lowLimit
					  ,(float)highLimit
					  ,kFloatMissing	// step value
					  ,0				// start gap
					  ,kForward			// scale direction
					  ,false			// steps on borders (onko akselin reunat aina stepin kohdalla)
					  ,6); // oletus on 5, 6:lla tehd‰‰n arvoja-ticksej‰ tihe‰mmin
	itsValueAxis = new NFmiAxis(scale, NFmiTitle("Y"));
	itsValueView = new NFmiAxisViewWithMinFontSize(CalcValueAxisRect()
													,itsToolBox
													,itsValueAxis
													,kDown
													,kLeft
													,true
													,true
													,false
													,0.1f);
}

//--------------------------------------------------------
// CreateModifyFactorScaleView
//--------------------------------------------------------
//void NFmiTimeSerialView::CreateModifyFactorScaleView(void)
void NFmiTimeSerialView::CreateModifyFactorScaleView(bool fSetScalesDirectlyWithLimits, double theValue)
{
	delete itsModifyFactorAxis;
	delete itsModifyFactorView;
	float lowerLimit= float(-itsDrawParam->TimeSerialModifyingLimit())
		, upperLimit= float(itsDrawParam->TimeSerialModifyingLimit());
	if(fSetScalesDirectlyWithLimits)
	{
		NFmiStepScale scale(lowerLimit, upperLimit, kFloatMissing, 0., kForward, true, int(theValue));
		itsModifyFactorAxis = new NFmiAxis (scale, NFmiTitle("Y"));
	}
	else
		itsModifyFactorAxis = new NFmiAxis(lowerLimit,upperLimit, NFmiTitle("Y"));
	itsModifyFactorView = new NFmiAxisViewWithMinFontSize(CalcModifyFactorAxisRect()
														,itsToolBox
														,itsModifyFactorAxis
														,kDown
														,kLeft
														,true
														,true
														,false
														,0.1f);
// en keksinyt t‰lle parempaa paikkaa, mutta nyt akseli ja maksimi muutos vastaavat toisiaan
//	itsDrawParam->TimeSerialModifyingLimit(itsModifyFactorAxis->EndValue());
}

//--------------------------------------------------------
// Value2AxisPosition
//--------------------------------------------------------
double NFmiTimeSerialView::Value2AxisPosition(float theValue)
{
	if(itsValueAxis)
	{
		float value = itsValueAxis->Location(theValue);
		NFmiRect rect(CalcValueAxisRect());
		double finalValue = rect.Bottom() - value * rect.Height(); // HUOMM!! toimii vain jos value on y-akselilla ja alkaa alhaalta ylˆs!!!
		return finalValue;
	}

	return kFloatMissing;
}

//--------------------------------------------------------
// ModifyFactor2AxisPosition
//--------------------------------------------------------
double NFmiTimeSerialView::ModifyFactor2AxisPosition(float theValue)
{
	float value = itsModifyFactorAxis->Location(theValue);
	NFmiRect rect(CalcModifyFactorAxisRect());
	double finalValue = rect.Bottom() - value * rect.Height(); // HUOMM!! toimii vain jos value on y-akselilla ja alkaa alhaalta ylˆs!!!
	return finalValue;
}

//--------------------------------------------------------
// Position2Value
//--------------------------------------------------------
float NFmiTimeSerialView::Position2Value(const NFmiPoint& thePos)
{
	if(itsValueView)
		return (float)itsValueView->Value(thePos);
	else
		return 0.f;
}

//--------------------------------------------------------
// Position2ModifyFactor
//--------------------------------------------------------
float NFmiTimeSerialView::Position2ModifyFactor(const NFmiPoint& thePos)
{
	if(itsModifyFactorView)
		return (float)itsModifyFactorView->Value(thePos);
	else
		return 0.f;
}

//--------------------------------------------------------
// DrawGrids
//--------------------------------------------------------
void NFmiTimeSerialView::DrawGrids(NFmiDrawingEnvironment& envi)
{
	itsToolBox->UseClipping(false);

    CalcValueAxisRect();
	NFmiRect timeRect = CalcTimeAxisRect();
	double lowTimeGridPos = 0;
	double lineperunit = CalcLinePerUnitValue(&lowTimeGridPos);
	if(lineperunit > 200.) // ei piirret‰ turhaa pipellyst‰!!!
		lineperunit = 200.;

	double relativeFill = (1./300.)*lineperunit;
	double oldRelativeFill = envi.GetRelativeFill();
	envi.SetRelativeFill(relativeFill);
	envi.SetSubLinePerUnit(lineperunit);
	envi.SetFrameColor(NFmiColor(0.5f,0.5f,0.5f)); // time grid v‰ri
	envi.SetSubLinePerUnit(CalcValueLinePerUnitValue());
	envi.SetRelativeFill(0.75);
	envi.SetFrameColor(NFmiColor(0.2f, 0.45f, 0.9f)); // value grid v‰ri
	DrawValueGrids(envi,timeRect.Left(),timeRect.Right());
	envi.SetRelativeFill(oldRelativeFill);
}

double NFmiTimeSerialView::CalcLinePerUnitValue(double *theLowTimeGridPos)
{
// ***** n‰m‰ laskut jakavat pysty viivat sopiviin v‰leihin arvoihin n‰hden, eli tekevat asteikon ******
	double lowValue = 0; // arvo hatusta
	double highValue = 10;// arvo hatusta
	if(itsDrawParam)
	{
		lowValue = itsDrawParam->TimeSeriesScaleMin();
		highValue = itsDrawParam->TimeSeriesScaleMax();
	}
	if(lowValue == kFloatMissing)
		lowValue = 0;
	if(highValue == kFloatMissing)
		lowValue = 10;
	double lowAxis = Value2AxisPosition(float(lowValue));
	double highAxis = Value2AxisPosition(float(highValue));
	double axisLength = lowAxis - highAxis;
	double minMaxDiff = highValue - lowValue;
	double jakaja = 1;
	if(minMaxDiff > 1000)
		jakaja = 100.;
	else if(minMaxDiff > 100)
		jakaja = 10.;
	else if(minMaxDiff > 70)
		jakaja = 5.;
	minMaxDiff /= jakaja;
	double lineperunit = (1 / axisLength)*(minMaxDiff);
	double lowUnitValue = (((int)(lowValue/jakaja)))*jakaja + jakaja;
	*theLowTimeGridPos = Value2AxisPosition(float(lowUnitValue));
	return lineperunit;
}

double NFmiTimeSerialView::CalcValueLinePerUnitValue(void)
{
// ***** n‰m‰ laskut jakavat vaaka-viivat sopiviin v‰leihin aika-askeleisiin n‰hden, eli tekevat asteikon ******
	int timeCount = itsZoomedTimeDescriptor.Size();
	double lowAxis = Time2Value(itsZoomedTimeDescriptor.FirstTime());
	double highAxis = Time2Value(itsZoomedTimeDescriptor.LastTime());
	double axisLength = highAxis - lowAxis;
	double minMaxDiff = timeCount-1;
	if(minMaxDiff > 200)
		minMaxDiff /= 10.;
	else if(minMaxDiff > 100)
		minMaxDiff /= 5.;
	else if(minMaxDiff < 10)
		minMaxDiff *= 10.;
	else if(minMaxDiff < 20)
		minMaxDiff *= 5.;
	double lineperunit = (1 / axisLength)*(minMaxDiff);
	return lineperunit;
}

//--------------------------------------------------------
// DrawValueGrids
//--------------------------------------------------------
// Piirt‰‰ nyky‰‰n minor ja major tick markit tai apuviivat
void NFmiTimeSerialView::DrawValueGrids(NFmiDrawingEnvironment& envi,double minPos,double maxPos)
{ // min- and maxpos are the other axis relative positions (timeaxis in this case)
	if(itsValueAxis)
	{
		// piirret‰‰n ensin minor tickmarkit apu viivoiksi himme‰ll‰
		itsValueAxis->StepNumber();
		double minValue = itsValueAxis->StartValue();
		double maxValue = itsValueAxis->EndValue();
		double majorTickStep = itsValueAxis->StepValue();
		double minorTickStep = majorTickStep/5.; // 1, 5, 10, 50, jne. menee sopivasti viidell‰, mutta 2, 20, 200 jne menee nelj‰ll‰
		if(majorTickStep == 0.002 || majorTickStep == 0.02 || majorTickStep == 0.2 || majorTickStep == 2. || majorTickStep == 20. || majorTickStep == 200. || majorTickStep == 2000. || majorTickStep == 20000. || majorTickStep == 200000. || majorTickStep == 2000000.)
			minorTickStep = majorTickStep/4.;
		TFmiColor majorTickColor(envi.GetFrameColor());
		envi.SetFrameColor(TFmiColor(0.75f, 0.75f, 0.75f)); // haalean harmaat apuviivat

		double yPos = 0;
		double currentValue = static_cast<int>(minValue/minorTickStep) * minorTickStep;
		for( ; currentValue < maxValue; currentValue += minorTickStep)
		{
			yPos = Value2AxisPosition(static_cast<float>(currentValue));
			NFmiLine line(NFmiPoint(minPos, yPos) // HUOM!! toimii vain jos arvoasteikko-viivat menev‰t vaakasuuntaan
						 ,NFmiPoint(maxPos, yPos)
						 ,0
						 ,&envi);
			itsToolBox->Convert(&line);
		}


		envi.SetFrameColor(majorTickColor);
		for(itsValueAxis->Reset(); itsValueAxis->Next(); )
		{
			yPos = Value2AxisPosition(itsValueAxis->Value());
			NFmiLine line(NFmiPoint(minPos, yPos) // HUOM!! toimii vain jos arvoasteikko-viivat menev‰t vaakasuuntaan
						 ,NFmiPoint(maxPos, yPos)
						 ,0
						 ,&envi);
			itsToolBox->Convert(&line);
		}
	}
}

//--------------------------------------------------------
// ChangeTimeSeriesValues
//--------------------------------------------------------
 // laskee aikasarjan muutokset ja p‰ivitt‰‰ infon arvot ja nollaa korjaus k‰yr‰n
void NFmiTimeSerialView::ChangeTimeSeriesValues(void)
{
	if(itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode() || IsAnalyzeRelatedToolUsed() || IsModifyFactorValuesNonZero())
	{
		NFmiMetEditorTypes::Mask maskType = NFmiMetEditorTypes::kFmiSelectionMask;
		if(itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode()) // kontrollipiste tyˆkalulla muutokset tehd‰‰n aina kaikkiin pisteisiin, eli = nomask
			maskType = NFmiMetEditorTypes::kFmiNoMask;
        bool status = itsCtrlViewDocumentInterface->DoTimeSeriesValuesModifying(itsDrawParam, maskType, itsEditedDataTimeDescriptor, itsModificationFactorCurvePoints, NFmiMetEditorTypes::kFmiTimeSerialModification, false);
		if(!itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode() && status)
			ResetModifyFactorValues();
	}
}

bool NFmiTimeSerialView::IsModifyFactorValuesNonZero(void)
{
	for(int i=0; i < static_cast<int>(itsModificationFactorCurvePoints.size()); i++)
		if(itsModificationFactorCurvePoints[i]) // jos jokin poikkeaa 0-arvosta, palautetaan true
			return true;
	return false;
}

//--------------------------------------------------------
// ResetModifyFactorValues
//--------------------------------------------------------
void NFmiTimeSerialView::ResetModifyFactorValues(void)
{
	for(int i=0; i < static_cast<int>(itsModificationFactorCurvePoints.size()); i++)
		itsModificationFactorCurvePoints[i] = 0.;
	itsSinAmplitude = 0.;
	itsPhase = 0;
}

//--------------------------------------------------------
// ChangeModifyFactorView
//--------------------------------------------------------
bool NFmiTimeSerialView::ChangeModifyFactorView(double theChangeDirectionFactor)
{
	double originalLimit = itsDrawParam->TimeSerialModifyingLimit();
	for(int i = 0; i < 50; i++)//50 = Ei tehd‰ ikiluuppia
	{
		if(itsDrawParam)
		{
			double currentLimit = itsDrawParam->TimeSerialModifyingLimit();
			double step = itsDrawParam->ModifyingStep();
			if(currentLimit > step || theChangeDirectionFactor > 0.)
			{
				double changeValue = step * theChangeDirectionFactor;
				itsDrawParam->TimeSerialModifyingLimit(itsDrawParam->TimeSerialModifyingLimit() + changeValue);
				// HUOM! t‰ss‰ on parametrit outoja, koska toteutus on aikoinaan tehty aikasarjalle vinksalleen, mieti jos korjattaisiin koodia paremmaksi!!
                itsCtrlViewDocumentInterface->UpdateToModifiedDrawParam(CtrlViewUtils::kFmiTimeSerialView, itsDrawParam, CtrlViewUtils::kFmiTimeSerialView);
			}
			else
				return false;
		}
		CreateModifyFactorScaleView(true, 7);
		if(theChangeDirectionFactor < 0.)
		{
			if(originalLimit > itsModifyFactorAxis->EndValue())
				return true;
		}
		else
		{
			if(originalLimit < itsModifyFactorAxis->EndValue())
				return true;
		}
	}
	return false;
}

// Pyˆrist‰‰ luvun l‰himp‰‰n 1, 2 tai 5 arvoon, tarkoittaen ett‰ theStep:in suuruus luokasta riippuen
// luku pyˆristet‰‰n esim. 1:een, 2:een tai 5:een (jos originaali theStep oli n. 0.8 - 8). 
// Pyˆristet‰‰n 0.1:een, 0.2:een tai 0.5:een (jos originaali theStep oli n. 0.08 - 0.8). 
// Pyˆristet‰‰n 10:een, 20:een tai 50:een (jos originaali theStep oli n. 8 - 80).  jne.
static double RoundToNearest_1_2_5(const double theStep)
{
	double step = ::fabs(theStep);
	int expCount = 0;
	if(step < 0.8)
	{
		for(; step < 0.8; )
		{
			step *= 10.;
			expCount--;
		}
	}
	else
	{
		for(; step > 8; )
		{
			step /= 10.;
			expCount++;
		}
	}

	// nyt stepin pit‰isi olla 0.8:n ja 8:n v‰lill‰
	if(step < 1.5)
		step = 1;
	else if(step < 3.5)
		step = 2;
	else
		step = 5;

	if(expCount < 0)
	{
		for(int i=0; i < abs(expCount); i++)
			step /= 10;
	}
	else if(expCount > 0)
	{
		for(int i=0; i < expCount; i++)
			step *= 10;
	}
	return step;
}

static double GetSuitableValueScaleModifyingStep(boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
	double scaleMax = theDrawParam->TimeSeriesScaleMax();
	double scaleMin = theDrawParam->TimeSeriesScaleMin();
	double step = (scaleMax - scaleMin) / 40.;
	double roundedStep = ::RoundToNearest_1_2_5(step);
	return roundedStep;
}

//--------------------------------------------------------
// ChangeValueView
//--------------------------------------------------------
bool NFmiTimeSerialView::ChangeValueView(double theChangeDirectionFactor, bool fMoveTop)
{
	if(itsValueAxis)
	{
		for(int i = 0; i < 50; i++)//50 = Ei tehd‰ ikiluuppia
		{
			if(itsDrawParam)
			{
				double step = ::GetSuitableValueScaleModifyingStep(itsDrawParam);
				double scaleMax = itsDrawParam->TimeSeriesScaleMax();
				double scaleMin = itsDrawParam->TimeSeriesScaleMin();
				if(((scaleMax - scaleMin) > step * 3) || theChangeDirectionFactor > 0.)
				{
					double changeValue = step * theChangeDirectionFactor;
					if(fMoveTop)
						itsDrawParam->TimeSeriesScaleMax(itsDrawParam->TimeSeriesScaleMax() + changeValue);
					else
						itsDrawParam->TimeSeriesScaleMin(itsDrawParam->TimeSeriesScaleMin() - changeValue);
				}
				else
					return false;
			}

			double ruunanPaa = itsValueAxis->StartValue();
			double ruunanHanta = itsValueAxis->EndValue();
			CreateValueScaleView();
			if(ruunanPaa != itsValueAxis->StartValue() || ruunanHanta != itsValueAxis->EndValue())
			{
				// HUOM! t‰ss‰ on parametrit outoja, koska toteutus on aikoinaan tehty aikasarjalle vinksalleen, mieti jos korjattaisiin koodia paremmaksi!!
				itsCtrlViewDocumentInterface->UpdateToModifiedDrawParam(CtrlViewUtils::kFmiTimeSerialView, itsDrawParam, CtrlViewUtils::kFmiTimeSerialView);
				return true;
			}
		}
	}

	return false;
}

//--------------------------------------------------------
// MoveValueView
//--------------------------------------------------------
bool NFmiTimeSerialView::MoveValueView(double theChangeDirectionFactor)
{
	if(itsDrawParam)
	{
		double changeValue = ::GetSuitableValueScaleModifyingStep(itsDrawParam) * theChangeDirectionFactor;
		itsDrawParam->TimeSeriesScaleMax(itsDrawParam->TimeSeriesScaleMax() + changeValue);
		itsDrawParam->TimeSeriesScaleMin(itsDrawParam->TimeSeriesScaleMin() + changeValue);
		return true;
	}
	return false;
}


//--------------------------------------------------------
// CutLinePoint2YPlane
//--------------------------------------------------------
void NFmiTimeSerialView::CutLinePoint2YPlane(const NFmiPoint& theOtherPoint, NFmiPoint& theChangedPoint, double theYPlane)
{
	// line function is y = ax + b
	// we calculate here new x value: x = (y-b)/a
	// if two points make line, then a and b are the following
	// b = (y2*x1-y1*x2)/(x1-x2)
	// a = (y1-b)/x1
	double x1 = theOtherPoint.X()
		 , x2 = theChangedPoint.X()
		 , y1 = theOtherPoint.Y()
		 , y2 = theChangedPoint.Y()
		 , y = theYPlane;
	double b = (y2 * x1 - y1 * x2) / (x1 - x2);
	double a = (y1 - b) / x1;
	double newX = (y-b)/a;
	NFmiPoint newChangedPoint(newX, y);
	theChangedPoint = newChangedPoint;
}
//--------------------------------------------------------
// DrawTimeLine
//--------------------------------------------------------
void NFmiTimeSerialView::DrawTimeLine()
{
	if(itsOperationMode != TimeSerialOperationMode::NormalDrawMode)
		return ;
	NFmiRect wholeArea(GetFrame());
	NFmiRect timeArea(CalcTimeAxisRect());
	double scale = timeArea.Width()/wholeArea.Width();

	//otetaan aika dokumentilta
    NFmiMetTime time = itsCtrlViewDocumentInterface->ActiveMapTime();
	NFmiDrawingEnvironment envi;
	envi.SetFrameColor(NFmiColor(1.f,0.f,0.f));
	double x = ((NFmiTimeScale*)itsTimeAxis->Scale())->RelTimeLocation(time);
	x = timeArea.Left() + scale * x;
	NFmiLine tmpline(NFmiPoint(x,itsRect.Top()),
			 NFmiPoint(x,itsRect.Bottom()),
			 0,
			 &envi);
	itsToolBox->Convert(&tmpline);
}

NFmiRect NFmiTimeSerialView::CalcModifyingUnitRect(void)
{
	NFmiRect timeAxisRect(CalcTimeAxisRect());
	NFmiPoint place(timeAxisRect.TopRight());
	place.X(place.X() - timeAxisRect.Width() * 0.02);
	double rectHeight = timeAxisRect.Width()/30.;
	NFmiPoint size(0, rectHeight);
	NFmiRect rect;
	rect.Place(place);
	rect.Size(size);
	rect.Right(GetFrame().Right());
	return rect;
}

// piirt‰‰ analyysi moodin lopetus ajan kohdalle pystyviivan merkiksi
void NFmiTimeSerialView::DrawAnalyzeToolEndTimeLine(void)
{
	if(itsOperationMode != TimeSerialOperationMode::NormalDrawMode)
		return ;
	NFmiDrawingEnvironment envi(ChangeStationDataCurveEnvironment());
	envi.SetFrameColor(NFmiColor(0.91f,0.34f,0.34f));
	envi.SetPenSize(NFmiPoint(3, 3));
	NFmiRect valueRect(CalcValueAxisRect());
	double xPos = Time2Value(itsCtrlViewDocumentInterface->AnalyzeToolData().AnalyzeToolEndTime());
	NFmiLine line(NFmiPoint(xPos, valueRect.Top()), NFmiPoint(xPos, valueRect.Bottom()), 0, &envi);
	itsToolBox->Convert(&line);
}

void NFmiTimeSerialView::DrawObsBlenderChangeLine(const NFmiPoint &theLatLonPoint)
{
    NFmiDrawingEnvironment envi;
    std::vector<std::string> messages;
    auto editedInfo = Info();
    boost::shared_ptr<NFmiFastQueryInfo> usedObsBlenderInfo;
    auto &controlPointObservationBlendingData = itsCtrlViewDocumentInterface->AnalyzeToolData().ControlPointObservationBlendingData();
    NFmiMetTime firstEditedTime;
    float obsBlenderValue = kFloatMissing;
    try
    {
        if(itsCtrlViewDocumentInterface->SetupObsBlenderData(theLatLonPoint, *itsDrawParam->Param().GetParam(), NFmiInfoData::kObservations, true, controlPointObservationBlendingData.SelectedProducer(), firstEditedTime, usedObsBlenderInfo, obsBlenderValue, messages))
        {
            // 4. Mik‰ on lopetusaika(helppo)
            // 5. Piirr‰ muutosk‰ppyr‰(helppoa, kun kohdat 1 - 2 hoidettu)
            DrawAnalyzeToolRelatedChangeLineFinal(true, obsBlenderValue, theLatLonPoint, envi, editedInfo, usedObsBlenderInfo, firstEditedTime, messages);
        }
        else
            messages.push_back("No suitable obs. data for this CP point"s);
    }
    catch(std::exception &e)
    {
        messages.push_back(e.what());
        itsCtrlViewDocumentInterface->LogAndWarnUser(std::string("Error in ") + __FUNCTION__ + ": " + e.what(), "", CatLog::Severity::Warning, CatLog::Category::Visualization, true);
    }

    DrawAnalyzeToolRelatedMessages(messages, envi);
}

static std::string GetMaskFactorEffectText(float maskFactor)
{
    if(maskFactor < 0.05)
        return " (no change)";
    else if(maskFactor < 0.3)
        return " (mild change)";
    else if(maskFactor < 0.7)
        return " (moderate change)";
    else if(maskFactor < 0.9)
        return " (strong change)";
    else
        return " (full change)";
}

void NFmiTimeSerialView::DrawAnalyzeToolRelatedChangeLineFinal(bool useObservationData, float usedAnalyzeValue, const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment &envi, boost::shared_ptr<NFmiFastQueryInfo> &editedInfo, boost::shared_ptr<NFmiFastQueryInfo> &usedToolInfo, const NFmiMetTime &startTime, std::vector<std::string> &messages)
{
    DrawAnalyzeToolDataLocationInTime(theLatLonPoint, envi, usedToolInfo); // piirret‰‰n ensin analyysi data ja sitten sen aiheuttama muutosk‰yr‰
    NFmiMetTime endTime(itsCtrlViewDocumentInterface->AnalyzeToolData().AnalyzeToolEndTime());
    if(startTime < endTime)
    {
        // Jos hila-analyysidatan aikaa ei lˆydy editoitavasta datasta, ei kannata jatkaa
        if(useObservationData ? editedInfo->FindNearestTime(startTime, kCenter, NFmiControlPointObservationBlendingData::ExpirationTimeInMinutes()) : editedInfo->Time(startTime))
        {
            auto firstEditDataValue = useObservationData ? editedInfo->InterpolatedValue(theLatLonPoint) : editedInfo->FloatValue();
            if(usedAnalyzeValue != kFloatMissing && firstEditDataValue != kFloatMissing)
            {
                NFmiLimitChecker limitChecker(static_cast<float>(itsDrawParam->AbsoluteMinValue()), static_cast<float>(itsDrawParam->AbsoluteMaxValue()), static_cast<FmiParameterName>(itsDrawParam->Param().GetParamIdent()));
                auto maskList = NFmiAnalyzeToolData::GetUsedTimeSerialMaskList(itsCtrlViewDocumentInterface->GenDocDataAdapter());
                bool useMask = maskList->UseMask();
                auto changeValue = useObservationData ? usedAnalyzeValue : (firstEditDataValue - usedAnalyzeValue);
                // Muutosarvo pit‰‰ kertoa -1:ll‰, jotta selitt‰v‰‰n tekstiin saadaan oikean suuntainen muutosarvo, en tied‰ miksi logiikka menee n‰in
                auto helpText = "Selected point change value: "s + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(-changeValue, 2));
                messages.push_back(helpText);
                NFmiTimeBag times(startTime, endTime, editedInfo->TimeDescriptor().Resolution());
                auto timeSize = times.GetSize();
                auto timeIndex = 0;
                NFmiDrawingEnvironment envi2(ChangeStationDataCurveEnvironment());
                envi2.SetFrameColor(NFmiColor(0.98f, 0.134f, 0.14f));
                envi2.SetPenSize(NFmiPoint(2, 2));
                std::vector<pair<double, NFmiMetTime> > dataVector;
                auto maskHelpText = "Starting mask factor: "s;
                for(times.Reset(); times.Next(); timeIndex++)
                {
                    auto editDataValue = kFloatMissing;
                    float maskFactor = 1.f;
                    if(editedInfo->Time(times.CurrentTime()))
                    {
                        editDataValue = useObservationData ? editedInfo->InterpolatedValue(theLatLonPoint) : editedInfo->FloatValue();
                        if(useMask)
                        {
                            maskList->SyncronizeMaskTime(editedInfo->Time());
                            maskFactor = static_cast<float>(maskList->MaskValue(editedInfo->LatLonFast()));
                        }
                    }
                    if(timeIndex == 0)
                    {
                        maskHelpText += NFmiValueString::GetStringWithMaxDecimalsSmartWay(maskFactor, 1);
                        maskHelpText += GetMaskFactorEffectText(maskFactor);
                    }
                    auto modifiedValue = NFmiControlPointObservationBlendingData::BlendData(editDataValue, changeValue, maskFactor, timeSize, timeIndex, limitChecker);
                    dataVector.push_back(std::make_pair(modifiedValue, times.CurrentTime()));
                }
                messages.push_back(maskHelpText);
                DrawSimpleDataVectorInTimeSerial(dataVector, envi2, NFmiPoint(2, 2), NFmiPoint(6, 6));
            }
            else
            {
                messages.push_back("Missing value in analyze data"s);
            }
        }
        else
            messages.push_back("Suitable start time not in edited data"s);
    }
    else
        messages.push_back("Start/end editing time range illegal"s);
}

// piirt‰‰ katkoviivalla k‰yr‰n, joka kuvaa valitun pisteen kohdalla tapahtuvaa muutosta.
// HUOM!!! Piirt‰‰ myˆs analyysidatan ruutuun!!!!!
void NFmiTimeSerialView::DrawAnalyzeToolChangeLine(const NFmiPoint &theLatLonPoint)
{
    if(itsOperationMode != TimeSerialOperationMode::NormalDrawMode)
        return;

    if(itsCtrlViewDocumentInterface->AnalyzeToolData().ControlPointObservationBlendingData().UseBlendingTool())
    {
        DrawObsBlenderChangeLine(theLatLonPoint);
        return;
    }

    NFmiDrawingEnvironment envi;

    std::vector<std::string> messages;
    boost::shared_ptr<NFmiFastQueryInfo> analyzeDataInfo = GetAnalyzeToolData();
    if(analyzeDataInfo)
    {
        analyzeDataInfo->FirstLevel(); // varmuuden vuoksi asetan 1. leveliin
        auto editedInfo = Info();
        if(analyzeDataInfo->Param(*itsDrawParam->Param().GetParam())) // parametrikin pit‰‰ asettaa kohdalleen
        {
            analyzeDataInfo->LastTime();
            NFmiMetTime startTime = analyzeDataInfo->Time();
            auto analyzeValue = analyzeDataInfo->InterpolatedValue(theLatLonPoint);
            DrawAnalyzeToolRelatedChangeLineFinal(false, analyzeValue, theLatLonPoint, envi, editedInfo, analyzeDataInfo, startTime, messages);
        }
        else
            messages.push_back("No param in selected data"s);
    }
    else
    {
        messages.push_back("No suitable analyze data"s);
    }
    DrawAnalyzeToolRelatedMessages(messages, envi);
}

// Piirt‰‰ tietyt analyysityˆkaluun liittyv‰t lyhyet sanomat keskelle (ja keskitetysti) n‰yttˆrivi‰ otsikko-osioon.
void NFmiTimeSerialView::DrawAnalyzeToolRelatedMessages(const std::vector<std::string> &messages, NFmiDrawingEnvironment &envi)
{
    ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, kCenter, false);
    envi.SetFontSize(NFmiPoint(16, 16));
    auto textPoint = GetFrame().Center();
    double heightInc = itsToolBox->SY(15);
    textPoint.Y(GetFrame().Top() + heightInc/2.);
    for(const auto &message : messages)
    {
        NFmiText text(textPoint, message.c_str(), false, 0, &envi);
        itsToolBox->Convert(&text);
        textPoint.Y(textPoint.Y() + heightInc);
    }
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiTimeSerialView::GetAnalyzeToolData()
{
    return itsCtrlViewDocumentInterface->InfoOrganizer()->Info(NFmiDataIdent(*(itsDrawParam->Param().GetParam()), itsCtrlViewDocumentInterface->AnalyzeToolData().SelectedProducer1()), 0, NFmiInfoData::kAnalyzeData);
}

static std::vector<boost::shared_ptr<NFmiFastQueryInfo>> GetInfosWithWantedParam(std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &infoVectorIn, FmiParameterName wantedParamId)
{
    std::vector<boost::shared_ptr<NFmiFastQueryInfo>> infoVectorResult;
    std::copy_if(infoVectorIn.begin(), infoVectorIn.end(), std::back_inserter(infoVectorResult),
        [wantedParamId](const auto &info)
    {
        // Datassa ei saa olla myˆsk‰‰n ship/buoy dataa (= moving station data)
        if(!info->HasLatlonInfoInData())
        {
            return info->Param(wantedParamId);
        }
        return false;
    });
    return infoVectorResult;
}

std::vector<boost::shared_ptr<NFmiFastQueryInfo>> NFmiTimeSerialView::GetObsBlenderDataVector()
{
    auto selectedProducerId = itsCtrlViewDocumentInterface->AnalyzeToolData().ControlPointObservationBlendingData().SelectedProducer().GetIdent();
    auto infoVector = itsCtrlViewDocumentInterface->InfoOrganizer()->GetInfos(NFmiInfoData::kObservations, true, selectedProducerId);
    return ::GetInfosWithWantedParam(infoVector, static_cast<FmiParameterName>(itsDrawParam->Param().GetParamIdent()));
}

void NFmiTimeSerialView::DrawAnalyzeToolDataLocationInTime(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment &envi, boost::shared_ptr<NFmiFastQueryInfo> &analyzeDataInfo)
{
	if(analyzeDataInfo)
	{
        analyzeDataInfo->FirstLevel(); // varmuuden vuoksi asetan 1. leveliin
		if(::DataHasNeededParameters(analyzeDataInfo, itsDrawParam->Param().GetParamIdent(), itsCtrlViewDocumentInterface)) // parametrikin pit‰‰ asettaa kohdalleen
		{
			NFmiMetTime firstTime(Value2Time(NFmiPoint(0,0))); // haetaan aika, joka on ruudun alussa
            NFmiTimeDescriptor infoTimes(analyzeDataInfo->TimeDescriptor());
			if(infoTimes.FindNearestTime(firstTime))
                firstTime = infoTimes.Time();
			NFmiMetTime lastTime(analyzeDataInfo->TimeDescriptor().LastTime());
			NFmiTimeBag drawedTimes(firstTime, lastTime, analyzeDataInfo->TimeDescriptor().Resolution()); // n‰m‰ ajat sitten piirret‰‰n, kunhan otetaan selville ensin mitk‰ ne ovat
			if(analyzeDataInfo->NearestPoint(theLatLonPoint)) // asetetaan osoittamaan l‰himp‰‰n pisteeseen (pit‰isi olla oikeasti sama hila molemmisssa datoissa)
			{
				envi.SetFrameColor(NFmiColor(0.f, 0.f, 0.f)); // musta analyysi k‰yr‰
				envi.SetPenSize(NFmiPoint(3, 3)); // paksunnetaan viivaa
				DrawSimpleDataInTimeSerial(drawedTimes, analyzeDataInfo, itsDrawParam, envi, theLatLonPoint, NFmiPoint(9, 9));
				envi.SetPenSize(NFmiPoint(1, 1)); // ohut viiva takaisin
			}
		}
	}
}

void NFmiTimeSerialView::DrawSimpleDataVectorInTimeSerial(std::vector<pair<double, NFmiMetTime> > &theDataVector, NFmiDrawingEnvironment &theEnvi, const NFmiPoint& thePenSize, const NFmiPoint& theSinglePointSize)
{
	itsToolBox->UseClipping(true);
	int size = static_cast<int>(theDataVector.size());
	for(int i=1; i<size; i++)
	{
		if(theDataVector[i-1].first != gDontDrawLineValue && theDataVector[i].first != gDontDrawLineValue) // kumpikaan arvoista ei saa olla dontDraw-arvoinen
			DrawDataLine(theDataVector[i-1].second
						,theDataVector[i].second
						,theDataVector[i-1].first
						,theDataVector[i].first
						,theEnvi
						,thePenSize
						,theSinglePointSize, true);
	}
}

bool NFmiTimeSerialView::IsActivated(void) const
{
	if(itsDrawParam && itsDrawParam->IsParamEdited())
		return true;
	return false;
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiTimeSerialView::Info(void) const
{
	return itsInfo;
}

// funktio piirt‰‰ annetun arvojoukon annettuina aikoina n‰yttˆˆn halutuilla piirtooptioilla
void NFmiTimeSerialView::PlotTimeSerialData(const std::vector<float> &theValues, const std::vector<NFmiMetTime> &theTimes, NFmiDrawingEnvironment &theEnvi, const NFmiPoint& thePointSize, const NFmiPoint& theSinglePointSize, bool fUseValueAxis, bool drawConnectingLines)
{
	int valuesSize = static_cast<int>(theValues.size());
	int timesSize = static_cast<int>(theTimes.size());
	if(valuesSize == 1)
	{
		NFmiPoint onlyPointSize(20.,6.);
		DrawDataLine(theTimes[0], theTimes[0], theValues[0], kFloatMissing, theEnvi, onlyPointSize, onlyPointSize, fUseValueAxis);
	}
	else if(valuesSize > 1)
	{
		NFmiDrawingEnvironment blackLineEnvi;
		int lastNonMissingIndex = -1;
		double realValue1 = kFloatMissing; // t‰m‰ on indeksill‰ i-1 saatava arvo
		double realValue2 = kFloatMissing; // t‰m‰ on indeksill‰ i saatava arvo
		for(int i=1; (i < valuesSize) && (i < timesSize); i++) // i < timesSize est‰‰ kaatumisen jos values ja times vectorit erikokoisia, esim. jos deletoidaan CP-piste, palautetaan default std::vector, jonka koko on 200!!!
		{
			realValue1 = theValues[i-1];
			realValue2 = theValues[i];

			if(drawConnectingLines && realValue1 == kFloatMissing && realValue2 != kFloatMissing && lastNonMissingIndex != -1)
			{ // piirret‰‰n ohut musta yhteysviiv‰ katkonaisiin kohtiin, jos niin on s‰‰detty
				// ja piirret‰‰n se t‰p‰n alle, jokat tehd‰‰n kun on toinen arvoista on puuttuvaa.
				DrawDataLine(theTimes[lastNonMissingIndex], theTimes[i], theValues[lastNonMissingIndex], realValue2, blackLineEnvi, thePointSize, theSinglePointSize, true);
			}
			DrawDataLine(theTimes[i-1], theTimes[i], realValue1, realValue2, theEnvi, thePointSize, theSinglePointSize, fUseValueAxis);

			if(realValue2 != kFloatMissing)
				lastNonMissingIndex = i;
		}
	}
}

NFmiTimeBag NFmiTimeSerialView::GetViewLimitingTimes(void)
{
	return itsCtrlViewDocumentInterface->TimeSerialViewTimeBag();
}

void NFmiTimeSerialView::FillTimeSerialDataFromInfo(boost::shared_ptr<NFmiFastQueryInfo> &theSourceInfo, const NFmiPoint &theLatLonPoint, std::vector<float> &theValues, const NFmiFastInfoUtils::MetaWindParamUsage &metaWindParamUsage, unsigned long wantedParamId)
{
	// Onko t‰m‰ oikein????, linitingtimesit otetaan sourceInfosta ja niill‰ rajoitetaan FillTimeSerialDataFromInfo-metodissa
	// taas sourceInfosta haettavia aikoja.
	NFmiTimeBag limitingTimes(theSourceInfo->TimeDescriptor().FirstTime(), theSourceInfo->TimeDescriptor().LastTime(), 60); // resoluutiolla ei ole merkityst‰, 60 vain heitet‰‰n siihen
	FillTimeSerialDataFromInfo(theSourceInfo, theLatLonPoint, limitingTimes, theValues, metaWindParamUsage, wantedParamId);
}

// HUOM! yritt‰‰ ottaa myˆs yhdet ajat ja arvot aikareunojen yli
void NFmiTimeSerialView::FillTimeSerialDataFromInfo(boost::shared_ptr<NFmiFastQueryInfo>& theSourceInfo, const NFmiPoint& theLatLonPoint, const NFmiTimeBag& theLimitTimes, std::vector<float>& theValues, const NFmiFastInfoUtils::MetaWindParamUsage& metaWindParamUsage, unsigned long wantedParamId)
{
	if(theSourceInfo->TimeToNearestStep(theLimitTimes.FirstTime(), kBackward))
	{
		bool interpolateValues = theSourceInfo->IsGrid();
		NFmiLocation wantedLocation(theLatLonPoint);
		if(interpolateValues == false && (theSourceInfo->NearestLocation(wantedLocation, gMaxDistanceToFractileStation) == false))
		{
			return; // jos asema dataa ei lˆydy 500 km sis‰lt‰ haluttua pistett‰, ei k‰ytet‰ sit‰
		}
		float value = static_cast<float>(::GetTimeSerialValue(theSourceInfo, interpolateValues, theLatLonPoint, metaWindParamUsage, wantedParamId));
		theValues.push_back(value);
		NFmiMetTime currentInfoTime;
		for(; theSourceInfo->NextTime(); )
		{
			value = static_cast<float>(::GetTimeSerialValue(theSourceInfo, interpolateValues, theLatLonPoint, metaWindParamUsage, wantedParamId));
			theValues.push_back(value);
			if(!theLimitTimes.IsInside(theSourceInfo->Time()))
			{
				break; // oletus, ett‰ ajat j‰rjestyksess‰ ja loopitus voidaan lopettaa
			}
		}
	}
}

void NFmiTimeSerialView::FillTimeSerialTimesFromInfo(NFmiFastQueryInfo &theSourceInfo, std::vector<NFmiMetTime> &theTimes)
{
	NFmiTimeBag limitingTimes(theSourceInfo.TimeDescriptor().FirstTime(), theSourceInfo.TimeDescriptor().LastTime(), 60); // resoluutiolla ei ole merkityst‰, 60 vain heitet‰‰n siihen
	FillTimeSerialTimesFromInfo(theSourceInfo, limitingTimes, theTimes);
}

void NFmiTimeSerialView::FillTimeSerialTimesFromInfo(NFmiFastQueryInfo &theSourceInfo, const NFmiTimeBag &theLimitTimes, std::vector<NFmiMetTime> &theTimes)
{
	if(theSourceInfo.TimeToNearestStep(theLimitTimes.FirstTime(), kBackward))
	{
		theTimes.push_back(theSourceInfo.Time());
		NFmiMetTime currentInfoTime;
		for( ; theSourceInfo.NextTime(); )
		{
			currentInfoTime = theSourceInfo.Time();
			theTimes.push_back(currentInfoTime);
			if(!theLimitTimes.IsInside(currentInfoTime))
				break; // oletus, ett‰ ajat j‰rjestyksess‰ ja loopitus voidaan lopettaa
		}
	}
}

// t‰ytt‰‰ haluttuihin aikoihin maskien arvot (jos k‰ytet‰‰n maskeja).
void NFmiTimeSerialView::FillTimeSerialMaskValues(const std::vector<NFmiMetTime> &theTimes, const NFmiPoint &theLatLonPoint, std::vector<float> &theMaskValues)
{
	boost::shared_ptr<NFmiAreaMaskList> paramMaskList = itsCtrlViewDocumentInterface->ParamMaskListMT();
	if(paramMaskList)
	{
		if(itsCtrlViewDocumentInterface->IsMasksUsedInTimeSerialViews())
		{
			if(paramMaskList->CheckIfMaskUsed())
			{
				int timeSize = static_cast<int>(theTimes.size());
				for(int i=0; i < timeSize; i++)
				{
					paramMaskList->SyncronizeMaskTime(theTimes[i]);
					theMaskValues.push_back((float)paramMaskList->MaskValue(theLatLonPoint));
				}
			}
		}
	}
}

// t‰ytt‰‰ haluttuihin aikoihin muutos arvot, joilla piirret‰‰n muutos k‰yr‰.
void NFmiTimeSerialView::FillTimeSerialChangedValues(const std::vector<float> &theValues, const std::vector<float> &theMaskValues, std::vector<float> &theChangedValues)
{
	int timeSize = static_cast<int>(theValues.size());
	int maskSize = static_cast<int>(theMaskValues.size());
	for(int i=0; i < timeSize; i++)
		theChangedValues.push_back(CalcModifiedValue(theValues[i], i, maskSize ? theMaskValues[i] : 1));
}

void NFmiTimeSerialView::DrawModelRunsPlume(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment &theCurrentDataLineStyle, boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
	int wantedModelRunCount = theDrawParam->TimeSerialModelRunCount();
	if(wantedModelRunCount)
	{
		NFmiDrawingEnvironment usedEnvi = theCurrentDataLineStyle;
		NFmiColor origColor = theCurrentDataLineStyle.GetFrameColor();

		NFmiPoint pointNormal(1.,1.);
		NFmiPoint pointSingle(4.,4.);

		NFmiDrawParam origDrawParam(*itsDrawParam);
		try
		{
			int startIndex = origDrawParam.ModelRunIndex() - itsDrawParam->TimeSerialModelRunCount();

			for(int i = startIndex; i < origDrawParam.ModelRunIndex(); i++)
			{
				*itsDrawParam = origDrawParam; // joka kierroksella pit‰‰ palauttaa originaali optiot takaisin
				double brightningFactor = CtrlView::CalcBrightningFactor(theDrawParam->ModelRunIndex(), wantedModelRunCount, i); // mit‰ isompi luku, sit‰ enemm‰n vaalenee (0-100), vanhemmat malliajot vaaleammalla
				NFmiColor modelRunColor = NFmiColorSpaces::GetBrighterColor(origColor, brightningFactor);
				usedEnvi.SetFrameColor(modelRunColor);
				std::vector<float> values;
				itsDrawParam->ModelRunIndex(i);
				boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, false, false);
				if(info)
				{
                    auto metaWindParamUsage = NFmiFastInfoUtils::CheckMetaWindParamUsage(info);
                    auto paramId = itsDrawParam->Param().GetParamIdent();

					if(itsOperationMode != TimeSerialOperationMode::NormalDrawMode)
					{
						ScanDataForSpecialOperation(info, itsDrawParam, theLatLonPoint, itsAutoAdjustScanTimes, itsAutoAdjustMinMaxValues, metaWindParamUsage, paramId, 0);
					}
					else
					{
						DrawSimpleDataInTimeSerial(GetViewLimitingTimes(), info, itsDrawParam, usedEnvi, theLatLonPoint, pointSingle);
					}
				}
			}
		}
		catch(...)
		{
		}
		*itsDrawParam = origDrawParam; // palutetaan lopuksi originaali piirto-ominaisuudet
	}
}

static int CalcTimeOffsetInMinutes(boost::shared_ptr<NFmiFastQueryInfo>& info, const NFmiMetTime& viewStartTime)
{
	int timeOffsetInMinutes = 0;
	if(NFmiFastInfoUtils::IsModelClimatologyData(info))
	{
		auto dataTime = viewStartTime;
		dataTime.SetYear(info->TimeDescriptor().FirstTime().GetYear());
		timeOffsetInMinutes = viewStartTime.DifferenceInMinutes(dataTime);
	}
	return timeOffsetInMinutes;
}

//--------------------------------------------------------
// DrawLocationInTime
//--------------------------------------------------------

// Toiminnat mit‰ tehd‰‰n DrawEditedDataLocationInTime metodin alussa.
// Palauttaa true, jos on tarkoitus jatkaa normaali piirtoja viel‰ eteenp‰in, 
// ja palauttaa false, jos lopetetaan.
bool NFmiTimeSerialView::DrawEditedDataLocationInTime_PreliminaryActions(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle)
{
    itsToolBox->UseClipping(true);
    // piirret‰‰n edellisten malliajojen pluumi ensin eli alle (jos niit‰ edes piirret‰‰n)
    DrawModelRunsPlume(theLatLonPoint, theCurrentDataLineStyle, itsDrawParam); 

    if(itsOperationMode != TimeSerialOperationMode::NormalDrawMode)
    {
		int timeWhenDrawedInMinutes = ::CalcTimeOffsetInMinutes(itsInfo, itsAutoAdjustScanTimes.FirstTime());
        auto metaWindParamUsage = NFmiFastInfoUtils::CheckMetaWindParamUsage(Info());
        auto paramId = itsDrawParam->Param().GetParamIdent();
        ScanDataForSpecialOperation(Info(), itsDrawParam, theLatLonPoint, itsAutoAdjustScanTimes, itsAutoAdjustMinMaxValues, metaWindParamUsage, paramId, timeWhenDrawedInMinutes);
        return false;
    }

    return true;
}

bool NFmiTimeSerialView::IsModificationLineDrawn() const
{
    // Jos kyse on editoidun datan piirrosta
    if(IsEditedData(Info()))
    {
        // ja jos ollaan normaali editointi moodissa
        if(itsCtrlViewDocumentInterface->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal)
            return true; // voidaan muutos k‰yr‰ piirt‰‰
    }
    return false;
}

NFmiPoint g_PointNormal(1., 1.);
NFmiPoint g_PointSingle(4., 4.);

void NFmiTimeSerialView::DrawEditedDataLocationInTime_ModificationLine(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theModifiedDataLineStyle, const std::vector<NFmiMetTime> &theTimes, const std::vector<float> &values)
{
    if(IsModificationLineDrawn())
    {
        std::vector<float> maskValues;
        FillTimeSerialMaskValues(theTimes, theLatLonPoint, maskValues);
        std::vector<float> changeValues;
        FillTimeSerialChangedValues(values, maskValues, changeValues);
        PlotTimeSerialData(changeValues, theTimes, theModifiedDataLineStyle, g_PointNormal, g_PointSingle, true);
    }
}

void NFmiTimeSerialView::DrawEditedDataLocationInTime_MainActions(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle, const std::vector<NFmiMetTime> &theTimes, bool drawModificationLines)
{
    auto metaWindParamUsage = NFmiFastInfoUtils::CheckMetaWindParamUsage(Info());
    auto paramId = itsDrawParam->Param().GetParamIdent();
	auto drawedInfo = Info();
    std::vector<float> values;
    FillTimeSerialDataFromInfo(drawedInfo, theLatLonPoint, values, metaWindParamUsage, paramId);
    if(drawModificationLines && drawedInfo->DataType() == NFmiInfoData::kEditable)
        DrawEditedDataLocationInTime_ModificationLine(theLatLonPoint, theModifiedDataLineStyle, theTimes, values);
    PlotTimeSerialData(values, theTimes, theCurrentDataLineStyle, g_PointNormal, g_PointSingle, true, true);
}

void NFmiTimeSerialView::DrawEditedDataLocationInTime(const NFmiPoint& theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle, bool drawModificationLines)
{
	if(DrawEditedDataLocationInTime_PreliminaryActions(theLatLonPoint, theCurrentDataLineStyle))
	{
		auto info = Info();
		if(info && info->DataType() == NFmiInfoData::kTimeSerialMacroParam)
		{
			DrawTimeSerialMacroParam(info, itsDrawParam, theLatLonPoint, theCurrentDataLineStyle);
		}
		else if(NFmiFastInfoUtils::IsModelClimatologyData(info))
		{
			ModelClimatology::ParamIds drawedParamVector{ static_cast<FmiParameterName>(itsDrawParam->Param().GetParamIdent()) };
			DrawAnnualModelFractileDataLocationInTime2(info, theLatLonPoint, drawedParamVector, &theCurrentDataLineStyle);
		}
		else
		{
			std::vector<NFmiMetTime> times;
			FillTimeSerialTimesFromInfo(*Info(), times);
			DrawEditedDataLocationInTime_MainActions(theLatLonPoint, theCurrentDataLineStyle, theModifiedDataLineStyle, times, drawModificationLines);
		}
	}
}

void NFmiTimeSerialView::DrawLocationInTime(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle, bool drawModificationLines)
{
	DrawEditedDataLocationInTime(theLatLonPoint, theCurrentDataLineStyle, theModifiedDataLineStyle, drawModificationLines);
	if(IsEditedData(Info()))
	{
		if(IsAnalyzeRelatedToolUsed())
			DrawAnalyzeToolChangeLine(theLatLonPoint);
	}
}

// Katsoo onko kyseinen drawParamin data hybrid/painepinta tai muuta level dataa.
// Jos on, yritt‰‰ vaihtaa leveli‰ hiiren rullauksen mukaan ylˆs/alas.
// Palauttaa true, jos level vaihtui.
bool NFmiTimeSerialView::ChangeDataLevel(boost::shared_ptr<NFmiDrawParam> &theDrawParam, short theDelta)
{
	if(theDrawParam)
	{
		boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(theDrawParam, false, true);
		if(info)
		{
			if(info->SizeLevels() > 1)
			{
				if(theDelta < 0)
				{
					if(info->NextLevel())
					{
						theDrawParam->Level(*info->Level());
						return true;
					}
					else // pyˆr‰ytet‰‰n levelit ymp‰ri jos ollaan tultu loppuun
					{
						info->FirstLevel();
						theDrawParam->Level(*info->Level());
						return true;
					}
				}
				else
				{
					if(info->PreviousLevel())
					{
						theDrawParam->Level(*info->Level());
						return true;
					}
					else // pyˆr‰ytet‰‰n levelit ymp‰ri jos ollaan tultu alkuun
					{
						info->LastLevel();
						theDrawParam->Level(*info->Level());
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool NFmiTimeSerialView::IsModifyFactorViewClicked(const NFmiPoint &thePlace) const
{
    return (!IsAnalyzeRelatedToolUsed()) && itsModifyFactorView && itsModifyFactorView->GetFrame().IsInside(thePlace);
}

bool NFmiTimeSerialView::MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta)
{
	bool status = false;
	if(IsIn(thePlace))
	{
		if(IsModifyFactorViewClicked(thePlace))
		{
			status = ChangeModifyFactorView((theDelta < 0) ? -1. : 1.);
		}
		else if(itsValueView && itsValueView->GetFrame().IsInside(thePlace))
		{
			// t‰m‰ seuraava tarkastus johtuu itsValueView:in 'oudosta' leveydest‰
			// n‰ytt‰‰ ett‰ itsValueView:in pit‰‰ olla todella leve‰,
			// ennenkuin se suostuu piirt‰m‰‰n fontit tarpeeksi isolla
			// nyt tarkistetaan ettei klikkaus mene itsTimeView:in alueelle,
			// koska sen tarkastelu on t‰rke‰mp‰‰ ja oikeampaa t‰ss‰ tapauksessa
			if(thePlace.X() < itsTimeView->GetFrame().Left())
			{
				if(theKey & kCtrlKey)
				{
					status = MoveValueView((theDelta < 0) ? -1. : 1.);
				}
				else
				{
					if(Position2ModifyFactor(thePlace) > 0.)
						status = ChangeValueView((theDelta < 0) ? -1. : 1., true); // true liikuttaa asteikon yl‰p‰‰t‰ ja false alap‰‰t‰
					else
						status = ChangeValueView((theDelta < 0) ? 1. : -1., false);
				}
			}
		}
		else if(theKey & kCtrlKey)
		{ // Jos ollaan asteikkojen ulkopuolella ja rullaa liikutetaan CTRL-nappi pohjassa, vaihdetaan n‰ytett‰v‰‰ parametria joko seuraavaan tai edelliseen
			return itsCtrlViewDocumentInterface->ChangeActiveMapViewParam(CtrlViewUtils::kFmiTimeSerialView, itsViewGridRowNumber, itsViewGridRowNumber, theDelta > 0 ? true : false, false);
		}
		else if(theKey & kShiftKey) // jos shift-nappi pohjassa muutetaan hybrid/pressure -datojen leveli‰ ylˆs/alas
		{
			status = ChangeDataLevel(itsDrawParam, theDelta); // muuttaa mallipinta ja painepintaa
		}
	}
	if(status)
        itsCtrlViewDocumentInterface->TimeSerialViewDirty(true);
	return status;
}

static std::string Value2ToolTipString(float theValue, int theDigitCount, FmiInterpolationMethod theInterpolationMethod, FmiParamType theParamType)
{
	if(theValue == kFloatMissing)
		return std::string("-");
	else
	{
		NFmiString str;
		if(theDigitCount == 0 && theInterpolationMethod != kLinearly && theParamType != kContinuousParam)
			str = NFmiValueString(boost::math::iround(theValue), "%d");
		else
			str = NFmiValueString::GetStringWithMaxDecimalsSmartWay(theValue, ((theValue > 1) ? 1 : 2));

		return std::string(str);
	}
}

static void AddValueLineString(std::string &theStr, const std::string &theTitle, const NFmiColor &theTitleColor, float theValue, boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool fAddEndl, int theTabulatorCount = 1)
{
	FmiInterpolationMethod interpMethod = theDrawParam->Param().GetParam()->InterpolationMethod();
	FmiParamType parType = (FmiParamType)theDrawParam->Param().Type();
	int digitCount = theDrawParam->IsoLineLabelDigitCount(); // tekstille pit‰isi saada oma lukunsa ja isoviivoille oma

	theStr += "<font color=";
	theStr += ColorString::Color2HtmlColorStr(theTitleColor);
	theStr += ">";
	theStr += theTitle;
	theStr += "</font>";
	for(int i=0; i<theTabulatorCount; i++)
		theStr += "\t"; // tabulaattori
	theStr += "<b><font color=";
	NFmiColor tmpColor(0,0,1);
	theStr += ColorString::Color2HtmlColorStr(tmpColor);
	theStr += ">";
	theStr += ::Value2ToolTipString(theValue, digitCount, interpMethod, parType);
	theStr += "</font></b>";
	if(fAddEndl)
		theStr += "\n";
}

static void AddProducerString(std::string &theStr, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool fAddEndl)
{
	if(!theDrawParam->IsMacroParamCase(true))
	{
		theStr += " (";
		theStr += theInfo->Producer()->GetName();
		if(theDrawParam->Level().GetIdent() != 0) // jos ident on 0, on kyseess‰ 'pinta data' eli ei ilmoitetan level tietoja
		{
			if(theDrawParam->Level().LevelType() == kFmiHybridLevel)
				theStr += ", level ";
			else
				theStr += ", ";
			theStr += NFmiValueString(static_cast<int>(theDrawParam->Level().LevelValue()), "%d");
			if(theDrawParam->Level().LevelType() == kFmiPressureLevel)
				theStr += " hPa";
			else if(theDrawParam->Level().LevelType() == kFmiHeight)
				theStr += " m";
		}
		theStr += ") ";
	}

	if(fAddEndl)
		theStr += "\n";
}

std::string NFmiTimeSerialView::GetSideParametersToolTipText(const NFmiPoint& theLatlon, const NFmiMetTime& theTime, bool addNewlineAtStart)
{
	std::string str;

	auto viewRowSideParameters = itsCtrlViewDocumentInterface->GetCombinedMapHandlerInterface().getTimeSerialViewSideParameters(itsViewGridRowNumber);
	if(viewRowSideParameters && viewRowSideParameters->NumberOfItems() > 0)
	{
		if(addNewlineAtStart)
		{
			str += "\n";
		}
		str += "<hr color=red><br>";
		str += "Side Parameter(s):\n";

		// GeneralColor v‰rit alkavat 0:sta, ja 1. side-parameter on tarkoitus piirt‰‰ 2. v‰rill‰, jonka indeksi on siis 1.
		int sideParameterColorIndex = 1;
		for(viewRowSideParameters->Reset(); viewRowSideParameters->Next(); sideParameterColorIndex++)
		{
			// Joskus esim. side-parameterin lis‰ys ei laukaise aikasarjaikkunan piirtoa, ja t‰llˆin jos
			// tooltip piirto tulee ensin, ei listasta lˆydy kaikkia parametri nimi‰
			auto usedSideParamNameIndex = sideParameterColorIndex - 1;
			if(usedSideParamNameIndex < itsSideParameterNamesForTooltip.size())
			{
				const auto& paramNameInTooltip = itsSideParameterNamesForTooltip[usedSideParamNameIndex];
				auto sideParamColor = itsCtrlViewDocumentInterface->GeneralColor(sideParameterColorIndex);
				auto sideParamDrawParam = viewRowSideParameters->Current();
				if(sideParamDrawParam)
				{
					auto& producer = *sideParamDrawParam->Param().GetProducer();
					auto dataType = sideParamDrawParam->DataType();
					boost::shared_ptr<NFmiFastQueryInfo> sideParamInfo = ::GetWantedData(itsCtrlViewDocumentInterface, sideParamDrawParam, producer, dataType, &theLatlon);
					if(sideParamInfo)
					{
						float sideParameterValue = GetTooltipValue(sideParamInfo, theLatlon, theTime, sideParamDrawParam);
						::AddValueLineString(str, paramNameInTooltip, sideParamColor, sideParameterValue, sideParamDrawParam, true, 1);
					}
				}
			}
		}
	}

	if(!str.empty() && str.back() == '\n')
		str.pop_back();

	return str;
}

std::string NFmiTimeSerialView::GetModelDataToolTipText(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, const NFmiPoint &theLatlon, const NFmiMetTime &theTime)
{
	if(itsDrawParam->DataType() == NFmiInfoData::kHybridData)
		return std::string(); // hybrid-datalle ei kannata piirt‰‰ apumallidatoja, koska hybrid-levelit eiv‰t vastaa eri malleissa toisiaan...

	std::string str;
	// Sitten kirjataan malli datojen arvot
	bool horizontalBarAdded = false;

	std::vector<NFmiProducerInfo> &producers = itsCtrlViewDocumentInterface->ProducerSystem().Producers();
	size_t foundDataCounter = 0;
	for(unsigned int i=0; i < producers.size(); i++) // k‰yd‰‰n l‰pi kaikki tuottajat, ja katsotaan kuinka moneen p‰‰data osui (param + level ja tyyppi)
	{
		boost::shared_ptr<NFmiFastQueryInfo> modelInfo = ::GetWantedData(itsCtrlViewDocumentInterface, itsDrawParam, producers[i].GetProducer(), NFmiInfoData::kViewable, &theLatlon);
		if(modelInfo)
		{
			float modelValue = GetTooltipValue(modelInfo, theLatlon, theTime, itsDrawParam);
			if(horizontalBarAdded == false)
			{
				str += "<br><hr color=red><br>";
				horizontalBarAdded = true;
			}
			else
				str += "\n";
			std::string tmpNameStr = producers[i].Name();
			int tabulatorCount = 1;
			if(tmpNameStr.size() < 5)
				tabulatorCount = 2;
			::AddValueLineString(str, tmpNameStr, itsProducerModelDataColors[foundDataCounter], modelValue, itsDrawParam, false, tabulatorCount);
			foundDataCounter++;
			if(foundDataCounter >= itsProducerModelDataColors.size())
				break;
		}
	}
	return str;
}

// Huom! T‰‰ll‰ ei tarvitse v‰litt‰‰ tuulen meta parametreista, koska kyse on tietyist‰ fraktiili parametreista
std::string NFmiTimeSerialView::GetEcFraktileParamToolTipText(boost::shared_ptr<NFmiFastQueryInfo>& theViewedInfo, long theStartParamIndex, const std::string &theParName, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor, long theParamIndexIncrement)
{
	std::string str;
	boost::shared_ptr<NFmiFastQueryInfo> modelFractileInfo = itsCtrlViewDocumentInterface->GetBestSuitableModelFractileData(theViewedInfo);
	if(modelFractileInfo && modelFractileInfo->Param(static_cast<FmiParameterName>(theStartParamIndex)))
	{
		str += "<br><hr color=red><br>";
		str += modelFractileInfo->Producer()->GetName() + " " + theParName + "\n";
		long currentParamId = theStartParamIndex;

		modelFractileInfo->Param(static_cast<FmiParameterName>(currentParamId));
		::AddValueLineString(str, theParName + "-F100", theColor, modelFractileInfo->InterpolatedValue(theLatlon, theTime), itsDrawParam, true);
		currentParamId += theParamIndexIncrement;
		modelFractileInfo->Param(static_cast<FmiParameterName>(currentParamId));
		::AddValueLineString(str, theParName + "-F90", theColor, modelFractileInfo->InterpolatedValue(theLatlon, theTime), itsDrawParam, true);
		currentParamId += theParamIndexIncrement;
		modelFractileInfo->Param(static_cast<FmiParameterName>(currentParamId));
		::AddValueLineString(str, theParName + "-F75", theColor, modelFractileInfo->InterpolatedValue(theLatlon, theTime), itsDrawParam, true);
		currentParamId += theParamIndexIncrement;
		modelFractileInfo->Param(static_cast<FmiParameterName>(currentParamId));
		::AddValueLineString(str, theParName + "-F50", theColor, modelFractileInfo->InterpolatedValue(theLatlon, theTime), itsDrawParam, true);
		currentParamId += theParamIndexIncrement;
		modelFractileInfo->Param(static_cast<FmiParameterName>(currentParamId));
		::AddValueLineString(str, theParName + "-F25", theColor, modelFractileInfo->InterpolatedValue(theLatlon, theTime), itsDrawParam, true);
		currentParamId += theParamIndexIncrement;
		modelFractileInfo->Param(static_cast<FmiParameterName>(currentParamId));
		::AddValueLineString(str, theParName + "-F10", theColor, modelFractileInfo->InterpolatedValue(theLatlon, theTime), itsDrawParam, true);
		currentParamId += theParamIndexIncrement;
		modelFractileInfo->Param(static_cast<FmiParameterName>(currentParamId));
		int usedTabCount = 1;
		if(theParName.size() < 2)
			usedTabCount = 2; // jos param nimi stringi on yhden merkin mittainen, pit‰‰ olla 2 tabulaattoria, mutta jos se on pidempi, pit‰‰ olla 1 tabulaattori
		::AddValueLineString(str, theParName + "-F0", theColor, modelFractileInfo->InterpolatedValue(theLatlon, theTime), itsDrawParam, true, usedTabCount);
	}
	return str;
}

// Huom! T‰‰ll‰ ei tarvitse v‰litt‰‰ tuulen meta parametreista, koska kyse on tietyist‰ ERA interim erikoisparametreista
std::string NFmiTimeSerialView::GetModelClimatologyParamToolTipText(const ModelClimatology::ParamMapItem &paramItem, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor)
{
    std::string str;
    boost::shared_ptr<NFmiFastQueryInfo> modelClimatologyInfo = itsCtrlViewDocumentInterface->GetModelClimatologyData(itsDrawParam->Level());
    if(::HasModelClimatologyDataAnyOfGivenParameters(modelClimatologyInfo, paramItem.second))
    {
        str += "<br><hr color=red><br>";
        str += modelClimatologyInfo->Producer()->GetName() + " " + paramItem.first + "\n";

        for(size_t i = 0; i < paramItem.second.size(); i++)
        {
            auto paramName = paramItem.first + "-" + ModelClimatology::eraFractileLabels[i];
            if(modelClimatologyInfo->Param(paramItem.second[i]))
            {
                auto usedInterpolationTime = theTime;
                // Time must be transferred to year that climatology data has so that it can be used in any given year.
                usedInterpolationTime.SetYear(modelClimatologyInfo->TimeDescriptor().FirstTime().GetYear());
                ::AddValueLineString(str, paramName, theColor, modelClimatologyInfo->InterpolatedValue(theLatlon, usedInterpolationTime), itsDrawParam, true);
            }
            else
                ::AddValueLineString(str, paramName, theColor, kFloatMissing, itsDrawParam, true);
        }
    }
    return str;
}

std::string NFmiTimeSerialView::GetEcFraktileDataToolTipText(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor)
{
	std::string str;
	// nyt ec fraktiileja on vain 5 pintaparametrille (T, RR, N, WS ja WG)
	if(theViewedInfo->SizeLevels() == 1 && itsCtrlViewDocumentInterface->GetBestSuitableModelFractileData(theViewedInfo) && itsCtrlViewDocumentInterface->ShowHelperData2InTimeSerialView())
	{
		if(itsDrawParam->Param().GetParamIdent() == kFmiTemperature)
			str += GetEcFraktileParamToolTipText(theViewedInfo, kFmiTemperatureF100, "T", theLatlon, theTime, theColor);
		else if(itsDrawParam->Param().GetParamIdent() == kFmiPrecipitation1h)
			str += GetEcFraktileParamToolTipText(theViewedInfo, kFmiTotalPrecipitationF100, "RR", theLatlon, theTime, theColor);
		else if(itsDrawParam->Param().GetParamIdent() == kFmiTotalCloudCover)
			str += GetEcFraktileParamToolTipText(theViewedInfo, kFmiTotalCloudCoverF100, "N", theLatlon, theTime, theColor);
		else if(itsDrawParam->Param().GetParamIdent() == kFmiWindSpeedMS) 
			str += GetEcFraktileParamToolTipText(theViewedInfo, kFmiWindSpeedF100, "WS", theLatlon, theTime, theColor);
        else if(itsDrawParam->Param().GetParamIdent() == kFmiHourlyMaximumGust) 
            str += GetEcFraktileParamToolTipText(theViewedInfo, kFmiWindGustF100, "WG", theLatlon, theTime, theColor);

	} // end of help data 2}
	return str;
}

// Huom! T‰‰ll‰ ei tarvitse v‰litt‰‰ tuulen meta parametreista, koska kyse on erikoisfraktiili parametreista
std::string NFmiTimeSerialView::GetSeaLevelPlumeDataToolTipText(boost::shared_ptr<NFmiFastQueryInfo>& theViewedInfo, const NFmiPoint& theLatlon, const NFmiMetTime& theTime, const NFmiColor& theColor)
{
	std::string str;
	const auto* seaLevelPlumeData = itsCtrlViewDocumentInterface->SeaLevelPlumeData().getSeaLevelPlumeData(itsDrawParam->Param().GetParamIdent());
	if(seaLevelPlumeData)
	{
		auto seaLevelFractileData = GetSeaLevelPlumeData(seaLevelPlumeData->producer());
		if(itsCtrlViewDocumentInterface->ShowHelperData2InTimeSerialView() && seaLevelFractileData)
		{
			const auto& fractileParams = seaLevelPlumeData->fractileParams();
			// Jos lˆytyy 1. parametreista ja l‰hin asema piste on g_SeaLevelProbabilityMaxSearchRangeInMetres rajan sis‰ll‰
			if(seaLevelFractileData->Param(fractileParams[0]) && seaLevelFractileData->NearestLocation(theLatlon, seaLevelPlumeData->probabilityMaxSearchRangeInMetres()))
			{
				std::string paramName = "SeaLevel";
				str += "<br><hr color=red><br>";
				str += seaLevelFractileData->Producer()->GetName() + " " + paramName + "\n";

				for(size_t i = 0; i < fractileParams.size(); i++)
				{
					seaLevelFractileData->Param(fractileParams[i]);
					::AddValueLineString(str, paramName + "-" + std::string(fractileParams[i].GetName()), theColor, seaLevelFractileData->InterpolatedValue(theTime), itsDrawParam, true);
				}
			}
			str += GetSeaLevelProbDataToolTipText(theViewedInfo, seaLevelFractileData, theLatlon, theTime, theColor);
		}
		else if(seaLevelPlumeData->IsSeaLevelProbLimitParam(itsDrawParam->Param()))
		{
			const auto& probLimitParams = seaLevelPlumeData->probLimitParams();
			// Jos lˆytyy 1. parametreista ja l‰hin asema piste on g_SeaLevelProbabilityMaxSearchRangeInMetres rajan sis‰ll‰
			if(seaLevelFractileData->Param(probLimitParams[0]) && seaLevelFractileData->NearestLocation(theLatlon, seaLevelPlumeData->probabilityMaxSearchRangeInMetres()))
			{
				std::string paramName = "ProbLimit";
				str += "<br><hr color=red><br>";
				str += seaLevelFractileData->Producer()->GetName() + " " + paramName + "\n";

				for(size_t i = 0; i < probLimitParams.size(); i++)
				{
					seaLevelFractileData->Param(probLimitParams[i]);
					::AddValueLineString(str, paramName + std::to_string(i + 1), theColor, seaLevelFractileData->InterpolatedValue(theTime), itsDrawParam, true);
				}
			}
		}
	}
	return str;
}

static std::string GetSeaLevelProbLocationName(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
    auto location = theInfo->Location();
    if(location)
    {
        return std::string(location->GetName());
    }
    else
    {
        auto latlon = theInfo->LatLon();
        std::string latlonStr;
        latlonStr += CtrlViewUtils::GetLatitudeMinuteStr(latlon.X(), 2);
        latlonStr += ",";
        latlonStr += CtrlViewUtils::GetLongitudeMinuteStr(latlon.Y(), 2);
        return latlonStr;
    }
}

// Oletus: Tietyt tarkastelut on jo tehty NFmiTimeSerialView::GetSeaLevelPlumeDataToolTipText metodissa
// Huom! T‰‰ll‰ ei tarvitse v‰litt‰‰ tuulen meta parametreista, koska kyse on erikoisfraktiili parametreista
std::string NFmiTimeSerialView::GetSeaLevelProbDataToolTipText(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, boost::shared_ptr<NFmiFastQueryInfo> &theSeaLevelFractileData, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor)
{
    std::string str;
	const auto* seaLevelPlumeData = itsCtrlViewDocumentInterface->SeaLevelPlumeData().getSeaLevelPlumeData(itsDrawParam->Param().GetParamIdent());
	if(seaLevelPlumeData)
	{
		auto oldLocationIndex = theViewedInfo->LocationIndex();
		if(theViewedInfo->NearestLocation(theLatlon, seaLevelPlumeData->probabilityMaxSearchRangeInMetres()))
		{
			auto probStationData = seaLevelPlumeData->FindSeaLevelProbabilityStationData(theViewedInfo->Location(), theViewedInfo->LatLon());
			if(probStationData)
			{
				str += "<br><hr color=red><br>";
				str += "Fixed sea level ProbLimits 1-4 for ";
				str += probStationData->station_.GetName();
				str += "\n";

				std::string paramName = "ProbLimit";
				::AddValueLineString(str, paramName + "1", theColor, probStationData->prob1_, itsDrawParam, true);
				::AddValueLineString(str, paramName + "2", theColor, probStationData->prob2_, itsDrawParam, true);
				::AddValueLineString(str, paramName + "3", theColor, probStationData->prob3_, itsDrawParam, true);
				::AddValueLineString(str, paramName + "4", theColor, probStationData->prob4_, itsDrawParam, true);
			}

			str += "<br><hr color=red><br>";
			str += "Forecasted ProbLimit 1-4 values for ";
			str += ::GetSeaLevelProbLocationName(theViewedInfo);
			str += "\n";

			const auto& probParams = seaLevelPlumeData->probLimitParams();
			for(const auto& probParam : probParams)
			{
				theSeaLevelFractileData->Param(probParam);
				::AddValueLineString(str, std::string(probParam.GetName()), theColor, theSeaLevelFractileData->InterpolatedValue(theTime), itsDrawParam, true);
			}
		}
		theViewedInfo->LocationIndex(oldLocationIndex);
	}
    return str;
}

std::string NFmiTimeSerialView::GetModelClimatologyDataToolTipText(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor)
{
    if(itsCtrlViewDocumentInterface->GetModelClimatologyData(itsDrawParam->Level()) && itsCtrlViewDocumentInterface->ShowHelperData3InTimeSerialView())
    {
        auto paramMapIter = ModelClimatology::eraParamMap.find(static_cast<FmiParameterName>(itsDrawParam->Param().GetParamIdent()));
        if(paramMapIter != ModelClimatology::eraParamMap.end())
            return GetModelClimatologyParamToolTipText(paramMapIter->second, theLatlon, theTime, theColor);
    }
    return "";
}

bool NFmiTimeSerialView::IsMosTemperatureMinAndMaxDisplayed(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo)
{
    if(itsCtrlViewDocumentInterface->ShowHelperData4InTimeSerialView() && ::IsMosDataUsed(theViewedInfo))
        return true;
    else
        return false;
}

// Huom! T‰‰ll‰ ei tarvitse v‰litt‰‰ tuulen meta parametreista, koska kyse on tietyist‰ l‰mpˆtila parametreista
std::string NFmiTimeSerialView::GetMosTemperatureMinAndMaxDataToolTipText(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor)
{
    std::string str;
    if(IsMosTemperatureMinAndMaxDisplayed(theViewedInfo))
    {
        auto mosData = itsCtrlViewDocumentInterface->GetMosTemperatureMinAndMaxData();
        if(mosData)
        {
            str += "<br><hr color=red><br>";
            str += mosData->Producer()->GetName() + "\n";
            mosData->Param(kFmiMaximumTemperature);
            ::AddValueLineString(str, "T-max", theColor, mosData->InterpolatedValue(theLatlon, theTime), itsDrawParam, true);
            mosData->Param(kFmiMinimumTemperature);
            ::AddValueLineString(str, "T-min", theColor, mosData->InterpolatedValue(theLatlon, theTime), itsDrawParam, true);
        }
    }
    return str;
}

// Huom! T‰‰ll‰ ei tarvitse v‰litt‰‰ tuulen meta parametreista, koska kyse on tietyist‰ l‰mpˆtila parametreista
std::string NFmiTimeSerialView::GetObsFraktileDataToolTipText(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor)
{
	std::string str;
	if(theViewedInfo->SizeLevels() == 1 && itsDrawParam->Param().GetParamIdent() == kFmiTemperature) // fraktiileja lˆytyy vain pinta l‰mpˆtilalle!!!
	{
		boost::shared_ptr<NFmiFastQueryInfo> obsFractileInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->FindInfo(NFmiInfoData::kClimatologyData);
		if(obsFractileInfo)
		{
			if(obsFractileInfo->NearestLocation(NFmiLocation(theLatlon), gMaxDistanceToFractileStation)) // asetetaan kepadata l‰himp‰‰n pisteeseen (t‰ss‰ kaupunkiin) piirtoa varten
			{
				NFmiMetTime fractileTime1(obsFractileInfo->TimeDescriptor().FirstTime()); // fraktile data on omassa vuosiluvussaan, kun etsit‰‰n osoitettua aikaa, pit‰‰ tehd‰ vuosiluku kikka
				NFmiMetTime fractileTime2(theTime);
				fractileTime2.SetYear(fractileTime1.GetYear());
				str += "<br><hr color=red><br>";

				str += obsFractileInfo->Location()->GetName();
				str += " (fractile)\n";
				obsFractileInfo->Param(kFmiNormalMaxTemperatureF98);
				::AddValueLineString(str, "T-MaxF98", theColor, obsFractileInfo->InterpolatedValue(fractileTime2), itsDrawParam, true);
				obsFractileInfo->Param(kFmiNormalMaxTemperatureF50);
				::AddValueLineString(str, "T-MaxF50", theColor, obsFractileInfo->InterpolatedValue(fractileTime2), itsDrawParam, true);
				obsFractileInfo->Param(kFmiNormalMeanTemperature);
				::AddValueLineString(str, "T-mean", theColor, obsFractileInfo->InterpolatedValue(fractileTime2), itsDrawParam, true);
				obsFractileInfo->Param(kFmiNormalMinTemperatureF50);
				::AddValueLineString(str, "T-MinF50", theColor, obsFractileInfo->InterpolatedValue(fractileTime2), itsDrawParam, true);
				obsFractileInfo->Param(kFmiNormalMinTemperatureF02);
				::AddValueLineString(str, "T-MinF02", theColor, obsFractileInfo->InterpolatedValue(fractileTime2), itsDrawParam, true);

			}
		}
	}
	return str;
}

std::string NFmiTimeSerialView::GetObservationToolTipText(boost::shared_ptr<NFmiFastQueryInfo>& theViewedInfo, const NFmiPoint& theLatlon, const NFmiMetTime& theTime, const NFmiColor& theColor)
{
	std::string str;
	if(itsDrawParam->Level().LevelValue() == kFloatMissing) // kepa ja havainto laiteaan apudatoiksi vain pintadatoille!
	{
		// sitten havainto data
		boost::shared_ptr<NFmiFastQueryInfo> obsInfo = GetObservationInfo(*itsDrawParam->Param().GetParam(), theLatlon);
		if(obsInfo)
		{
			obsInfo->FirstLevel(); // varmuuden vuoksi asetan 1. leveliin
			if(::DataHasNeededParameters(obsInfo, itsDrawParam->Param().GetParamIdent(), itsCtrlViewDocumentInterface)) // parametrikin pit‰‰ asettaa
			{
				std::pair<int, double> locationWithData;
				if(SetObsDataToNearestLocationWhereIsData(obsInfo, theLatlon, locationWithData))
				{
					obsInfo->Time(theTime);
					::AddValueLineString(str, static_cast<char*>(obsInfo->Location()->GetName()), theColor, obsInfo->FloatValue(), itsDrawParam, false);
				}
			}
		}
	}
	return str;
}

static std::string GetColoredLocationTooltipStr(CtrlViewDocumentInterface *theCtrlViewDocumentInterface, const NFmiPoint &theLatlon, int theSelectedLocationCounter)
{
	std::string str;
	str += "<font color=";
	str += ColorString::Color2HtmlColorStr(theCtrlViewDocumentInterface->GeneralColor(theSelectedLocationCounter));
	str += ">";
	str += "Loc: ";
	str += ::GetLatlonString(theLatlon);
	str += "</font>";
	return str;
}

std::string NFmiTimeSerialView::MultiModelRunToolTip(boost::shared_ptr<NFmiDrawParam> &theDrawParam, const NFmiMetTime &theTime, const NFmiPoint &theLatlon)
{ 
    // lis‰t‰‰n lyhyesti muut malliajo arvot eri riveille kaytt‰en vain [-1]  =    2.3  -tyyppist‰ notaatiota
	boost::shared_ptr<NFmiDrawParam> tmpDrawParam(new NFmiDrawParam(*theDrawParam));
	int endIndex = tmpDrawParam->ModelRunIndex() - tmpDrawParam->TimeSerialModelRunCount();
	std::string str = "\n";
	for(int i = theDrawParam->ModelRunIndex() - 1; i >= endIndex; i--)
	{
		str += "\t[";
		str += NFmiStringTools::Convert(i);
		str += "]\t\t\t";
		str += "<b><font color=blue>";
		tmpDrawParam->ModelRunIndex(i);
		boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(tmpDrawParam, false, false);
		if(info)
		{
            float value = GetTooltipValue(info, theLatlon, theTime, tmpDrawParam);
			if(value == kFloatMissing)
				str += "-";
			else
			{
				FmiParamType parType = (FmiParamType)theDrawParam->Param().Type();
				str += Value2ToolTipString(value, theDrawParam->IsoLineLabelDigitCount(), theDrawParam->Param().GetParam()->InterpolationMethod(), parType);
			}
		}
		else
		{
			str += "-";
		}
		str += "</font></b>";
		str += "\n";
	}
	return str;
}

std::string NFmiTimeSerialView::ComposeToolTipText(const NFmiPoint& theRelativePoint)
{
	std::string str;

	boost::shared_ptr<NFmiFastQueryInfo> editedInfo = itsCtrlViewDocumentInterface->EditedSmartInfo();
	boost::shared_ptr<NFmiFastQueryInfo> viewedInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, false, false);
	if(editedInfo && viewedInfo)
	{
        NFmiPoint primaryLocationLatlon(GetTooltipLatlon());
        NFmiMetTime aTime = Value2Time(theRelativePoint, true); // haetaan aluksi tarkka aika

        // T‰m‰ varmistaa ett‰ myˆs erikoistapaus eli synop-data menee oikein
        viewedInfo = ::GetUsedSmartInfo(itsCtrlViewDocumentInterface, primaryLocationLatlon, viewedInfo, aTime, itsDrawParam);

		auto mainDataTimeResolution = static_cast<long>(viewedInfo->TimeResolution());
		if(mainDataTimeResolution > 60)
			mainDataTimeResolution = 60;
		aTime.SetTimeStep(mainDataTimeResolution); // asetetaan tooltipin ajaksi joko p‰‰datatan aikaresoluutio tai maksimissaan 60 minuuttia

	    NFmiString timeStr1 = aTime.ToStr("Nnnn DD. YYYY\nWwww HH:mm [UTC]", itsCtrlViewDocumentInterface->Language());
	    str += timeStr1;
	    str += "<br><hr color=red><br>";

		str += MakePossibleVirtualTimeTooltipText();

        viewedInfo->Param(static_cast<FmiParameterName>(itsDrawParam->Param().GetParamIdent())); // parametri pit‰‰ asettaa
		// n‰in saadaan selville 1. piirretty piste, ik‰v‰‰ koodia, mutta ei voi mit‰‰n...
		bool composeAllSelectedLocations = false;
        auto maskType = CtrlViewFastInfoFunctions::GetProperMaskTypeFromEditeInfo(editedInfo, itsCtrlViewDocumentInterface->AllowRightClickDisplaySelection());
        EditedInfoMaskHandler editedInfoMaskHandler(editedInfo, maskType);
        if(maskType == NFmiMetEditorTypes::kFmiDisplayedMask)
		{
			composeAllSelectedLocations = true;
		}

		bool showExtraInfo = CtrlView::IsKeyboardKeyDown(VK_CONTROL); // jos CTRL-n‰pp‰in on pohjassa, laitetaan lis‰‰ infoa n‰kyville
		string parNameStr = CtrlViewUtils::GetParamNameString(itsDrawParam, false, showExtraInfo, true, 0, true, true, true, nullptr);
		parNameStr = DoBoldingParameterNameTooltipText(parNameStr);
		auto fontColor = CtrlViewUtils::GetParamTextColor(itsDrawParam->DataType(), itsDrawParam->UseArchiveModelData());
		parNameStr = AddColorTagsToString(parNameStr, fontColor, true);
		NFmiColor stationDataColor;
		editedInfo->FirstLocation();
		int selectedLocationCounter = 0;
        float value = GetTooltipValue(viewedInfo, primaryLocationLatlon, aTime, itsDrawParam);
		str += GetColoredLocationTooltipStr(itsCtrlViewDocumentInterface, primaryLocationLatlon, selectedLocationCounter);
		str += "\n";
		selectedLocationCounter++;

		NFmiColor normalTitleColor(0, 0, 0);
		::AddValueLineString(str, parNameStr, normalTitleColor, value, itsDrawParam, false);
	    if(itsDrawParam->TimeSerialModelRunCount() > 0)
            str += MultiModelRunToolTip(itsDrawParam, aTime, primaryLocationLatlon);
		::AddProducerString(str, viewedInfo, itsDrawParam, true);
        std::string tmpLatestObsStr = CtrlViewUtils::GetLatestObservationTimeString(itsDrawParam, itsCtrlViewDocumentInterface, ::GetDictionaryString("YYYY.MM.DD HH:mm"), false);
        if(!tmpLatestObsStr.empty())
            tmpLatestObsStr += "\n";
        str += tmpLatestObsStr;
        // editoitu/viewable arvo loppuu t‰h‰n

		if(composeAllSelectedLocations)
		{
			for( ; editedInfo->NextLocation(); ) // k‰yd‰‰n l‰pi loput pisteet, jos oli moni paikka valinta
			{
				value = GetTooltipValue(viewedInfo, editedInfo->LatLon(), aTime, itsDrawParam);
				str += GetColoredLocationTooltipStr(itsCtrlViewDocumentInterface, editedInfo->LatLon(), selectedLocationCounter);
				str += " ";
				::AddValueLineString(str, parNameStr, normalTitleColor, value, itsDrawParam, false);
				str += "\n";
				selectedLocationCounter++;
			}
		}

		if(itsCtrlViewDocumentInterface->ShowHelperData1InTimeSerialView())
		{
			str += GetObservationToolTipText(viewedInfo, primaryLocationLatlon, aTime, normalTitleColor);

			if(itsDrawParam->Level().LevelValue() == kFloatMissing) // kepa ja havainto laiteaan apudatoiksi vain pintadatoille!
			{
				// sitten laitetaan virallinen data jos se lˆytyy
				boost::shared_ptr<NFmiFastQueryInfo> kepaInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->FindInfo(NFmiInfoData::kKepaData);
				if(kepaInfo)
				{
					if(::DataHasNeededParameters(kepaInfo, itsDrawParam->Param().GetParamIdent(), itsCtrlViewDocumentInterface)) // parametrikin pit‰‰ asettaa
					{
						str += "\n";
						::AddValueLineString(str, "Official edited ", normalTitleColor, GetTooltipValue(kepaInfo, primaryLocationLatlon, aTime, itsDrawParam), itsDrawParam, false);
					}
				}
			}
		}

		str += GetSideParametersToolTipText(primaryLocationLatlon, aTime, str.back() != '\n');

		if(itsCtrlViewDocumentInterface->ShowHelperData1InTimeSerialView())
		{
			str += GetModelDataToolTipText(viewedInfo, primaryLocationLatlon, aTime);
			str += GetEditingRelatedDataToolTipText(primaryLocationLatlon, aTime);
			str += GetObsFraktileDataToolTipText(viewedInfo, primaryLocationLatlon, aTime, normalTitleColor);
		} // end of help data 1

		str += GetEcFraktileDataToolTipText(viewedInfo, primaryLocationLatlon, aTime, normalTitleColor);
        str += GetSeaLevelPlumeDataToolTipText(viewedInfo, primaryLocationLatlon, aTime, normalTitleColor);
        str += GetModelClimatologyDataToolTipText(viewedInfo, primaryLocationLatlon, aTime, normalTitleColor);
        str += GetMosTemperatureMinAndMaxDataToolTipText(viewedInfo, primaryLocationLatlon, aTime, normalTitleColor);
    }

	return str;
}

std::string NFmiTimeSerialView::MakeToolTipTextForData(const NFmiProducer &theProducer, NFmiInfoData::Type theDataType, const NFmiColor &theTitleColor, const NFmiPoint& theLatlon, const NFmiMetTime& theTime)
{
	std::string str;
	boost::shared_ptr<NFmiFastQueryInfo> info = ::GetWantedData(itsCtrlViewDocumentInterface, itsDrawParam, theProducer, theDataType, &theLatlon);
	if(info)
	{
		str += "\n";
		float value = GetTooltipValue(info, theLatlon, theTime, itsDrawParam);
		::AddValueLineString(str, static_cast<char*>(theProducer.GetName()), theTitleColor, value, itsDrawParam, false);
	}
	return str;
}

std::string NFmiTimeSerialView::GetEditingRelatedDataToolTipText(const NFmiPoint &theLatlon, const NFmiMetTime &theTime)
{
	std::string str;
	if(itsCtrlViewDocumentInterface->HelpEditorSystem().Use())
	{
		NFmiProducer helpDataProducer(NFmiProducerSystem::gHelpEditorDataProdId, "help-data");
		str += MakeToolTipTextForData(helpDataProducer, NFmiInfoData::kEditingHelpData, itsCtrlViewDocumentInterface->HelpEditorSystem().HelpColor(), theLatlon, theTime);
	}
	return str;
}

NFmiPoint NFmiTimeSerialView::GetTooltipLatlon() const
{
    // 1. If control-point mode, return active CP latlon
    if(DoControlPointModeDrawing())
        return itsCtrlViewDocumentInterface->CPManager()->ActiveCPLatLon();

    // 2. If non-edited data selected
    if(itsDrawParam->DataType() != NFmiInfoData::kEditable)
    {
        // 1.1 If rigth-click accurate mode, return accurate latlon point
        if(itsCtrlViewDocumentInterface->IsPreciseTimeSerialLatlonPointUsed())
            return itsCtrlViewDocumentInterface->PreciseTimeSerialLatlonPoint();
    }

    // 3. Else return first selected location
    return GetFirstSelectedLatlonFromEditedData();
}

NFmiPoint NFmiTimeSerialView::GetFirstSelectedLatlonFromEditedData() const
{
    boost::shared_ptr<NFmiFastQueryInfo> editedInfo = itsCtrlViewDocumentInterface->EditedSmartInfo();
    if(editedInfo)
    {
        // Check if there is selected mask point to return
        EditedInfoMaskHandler editedInfoMaskHandler(editedInfo, CtrlViewFastInfoFunctions::GetProperMaskTypeFromEditeInfo(editedInfo, itsCtrlViewDocumentInterface->AllowRightClickDisplaySelection()));
        if(editedInfo->FirstLocation())
        {
            return editedInfo->LatLon();
        }
    }
    // If all else fails return possible out-of-edited-area point (or missing point)
    auto outOfEditedAreaTimeSerialPoint = itsCtrlViewDocumentInterface->OutOfEditedAreaTimeSerialPoint();
	if(outOfEditedAreaTimeSerialPoint != NFmiPoint::gMissingLatlon)
		return outOfEditedAreaTimeSerialPoint;
	else
		return itsCtrlViewDocumentInterface->ToolTipLatLonPoint();
}

void NFmiTimeSerialView::UpdateCachedParameterName()
{
	CachedParameterName(CtrlViewUtils::GetParamNameString(itsDrawParam, false, false, false, 0, true, true, true, itsInfo), false);
	CachedParameterName(CtrlViewUtils::GetParamNameString(itsDrawParam, false, false, true, 0, true, true, true, itsInfo), true);
}

void NFmiTimeSerialView::AddSideParameterNames(boost::shared_ptr<NFmiDrawParam>& drawParam, boost::shared_ptr<NFmiFastQueryInfo>& fastInfo)
{
	itsSideParameterNames.push_back(CtrlViewUtils::GetParamNameString(drawParam, false, false, false, 0, true, true, true, fastInfo));
	itsSideParameterNamesForTooltip.push_back(CtrlViewUtils::GetParamNameString(drawParam, false, false, true, 0, true, true, true, fastInfo));
}

void NFmiTimeSerialView::ClearSideParameterNames()
{
	itsSideParameterNames.clear();
	itsSideParameterNamesForTooltip.clear();
}

std::string NFmiTimeSerialView::MakeCsvDataString()
{
	itsCsvDataString.clear();
	itsCsvGenerationTimes.clear();
	itsCsvGenerationParameterValues.clear();
	itsCsvGenerationParameterNames.clear();
	itsCsvGenerationLatlon = NFmiPoint::gMissingLatlon;

	itsInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, false, true);
	if(itsInfo)
	{
		itsOperationMode = TimeSerialOperationMode::CsvDataGeneration;
		itsAutoAdjustScanTimes = GetViewLimitingTimes();
		DrawSelectedStationData();
		itsOperationMode = TimeSerialOperationMode::NormalDrawMode;
		if(!itsCsvGenerationTimes.empty())
		{
			auto headerString = MakeTimeSerialCsvHeaderString();
			auto timeSerialDataString = MakeTimeSerialCsvString();
			if(!headerString.empty() && !timeSerialDataString.empty())
			{
				itsCsvDataString = headerString;
				itsCsvDataString += timeSerialDataString;
			}
		}
	}
	return itsCsvDataString;
}

static std::set<NFmiMetTime> MakeUniqueAscendingTimeSet(const std::list<std::list<NFmiMetTime>>& availableTimes)
{
	std::set<NFmiMetTime> uniqueTimes;
	for(const auto& timeList : availableTimes)
	{
		uniqueTimes.insert(timeList.begin(), timeList.end());
	}
	return uniqueTimes;
}

const NFmiString gCsvTimeFormatString = "YYYY-MM-DD HH:mm:SS";

static std::string MakeCsvParameterNameSectionString(const std::list<std::string>& csvParameterNames)
{
	std::string paramNameLineString = "time";
	for(const auto& parameterName : csvParameterNames)
	{
		paramNameLineString += ",";
		paramNameLineString += parameterName;
	}
	paramNameLineString += "\n";
	return paramNameLineString;
}

static std::string MakeCsvValueLineString(const NFmiMetTime& time, const std::list<std::list<NFmiMetTime>>& csvGenerationTimes,
	const std::list<std::list<float>>& csvGenerationParameterValues)
{
	std::string csvDataString = time.ToStr(gCsvTimeFormatString, kEnglish);
	csvDataString += ",";
	auto paramValuesListIter = csvGenerationParameterValues.begin();
	bool firstValueInLoop = true;
	for(const auto& parameterTimeList : csvGenerationTimes)
	{
		if(!firstValueInLoop)
		{
			csvDataString += ",";
		}
		firstValueInLoop = false;

		auto timeListIter = std::find(parameterTimeList.begin(), parameterTimeList.end(), time);
		if(timeListIter == parameterTimeList.end())
		{
			csvDataString += "-";
		}
		else
		{
			auto diffFromStart = std::distance(parameterTimeList.begin(), timeListIter);
			auto valueIter = paramValuesListIter->begin();
			std::advance(valueIter, diffFromStart);
			auto value = *valueIter;
			if(value == kFloatMissing)
			{
				csvDataString += "-";
			}
			else
			{
				csvDataString += NFmiValueString::GetStringWithMaxDecimalsSmartWay(value, 2);
			}
		}
		++paramValuesListIter;
	}
	csvDataString += "\n";
	return csvDataString;
}

std::string NFmiTimeSerialView::MakeTimeSerialCsvString()
{
	std::string csvDataString;
	// 1. Tarkista ett‰ n‰yttˆrivilt‰ tulee sama m‰‰r‰ aikalistoja, parametrilistoja ja parametrien nimi‰
	auto expectedSize = itsCsvGenerationTimes.size();
	if(expectedSize == itsCsvGenerationParameterValues.size() && expectedSize == itsCsvGenerationParameterNames.size())
	{
		csvDataString += ::MakeCsvParameterNameSectionString(itsCsvGenerationParameterNames);
		// 2. Tee aikalista, miss‰ on mukana kaikkien aikalistojen kaikki eri ajat nousevassa j‰rjestyksess‰
		auto uniqueTimeSet = ::MakeUniqueAscendingTimeSet(itsCsvGenerationTimes);
		for(const auto& time : uniqueTimeSet)
		{
			csvDataString += ::MakeCsvValueLineString(time, itsCsvGenerationTimes, itsCsvGenerationParameterValues);
		}
		csvDataString += "\n";
	}
	return csvDataString;
}

std::string NFmiTimeSerialView::MakeTimeSerialCsvHeaderString()
{
	std::string rowLabel = "Row-";
	rowLabel += std::to_string(itsViewGridRowNumber);
	std::string csvDataString;
	csvDataString += rowLabel;
	csvDataString += ",";
	csvDataString += "lon-lat-point";
	csvDataString += ",";
	csvDataString += std::to_string(itsCsvGenerationLatlon.X());
	csvDataString += ",";
	csvDataString += std::to_string(itsCsvGenerationLatlon.Y());
	csvDataString += ",";
	csvDataString += "start-time";
	csvDataString += ",";
	csvDataString += itsAutoAdjustScanTimes.FirstTime().ToStr(gCsvTimeFormatString, kEnglish);
	csvDataString += ",";
	csvDataString += "end-time";
	csvDataString += ",";
	csvDataString += itsAutoAdjustScanTimes.LastTime().ToStr(gCsvTimeFormatString, kEnglish);
	csvDataString += "\n";

	return csvDataString;
}

// Tuottaja nime‰ ei haluta mielell‰‰n datan tuottajasta, vaan lyhyt geneerinen nimi josatain producerSystem:ista.
// Jos malli/havainto tuottajanimi systeemeist‰ ei sitten lˆydy sellaista, silloin otetaan datasta lˆytynyt nimi.
static std::string MakeCsvProducerName(NFmiProducerSystem& modelProducerSystem, NFmiProducerSystem& obsProducerSystem, boost::shared_ptr<NFmiFastQueryInfo>& theInfo)
{
	auto dataType = theInfo->DataType();
	const auto& producer = *theInfo->Producer();
	if(dataType == NFmiInfoData::kEditable || dataType == NFmiInfoData::kCopyOfEdited)
		return "";
	if(dataType == NFmiInfoData::kSingleStationRadarData)
		return std::string(producer.GetName());
	if(dataType == NFmiInfoData::kEditingHelpData)
		return "Help";

	auto producerIndex1Based = modelProducerSystem.FindProducerInfo(producer);
	if(producerIndex1Based != 0)
	{
		return modelProducerSystem.Producer(producerIndex1Based).ShortName();
	}

	producerIndex1Based = obsProducerSystem.FindProducerInfo(producer);
	if(producerIndex1Based != 0)
	{
		return obsProducerSystem.Producer(producerIndex1Based).ShortName();
	}

	return std::string(producer.GetName());
}

static std::string MakeCsvParameterNameSectionString(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, unsigned long wantedParamId)
{
	NFmiFastInfoUtils::QueryInfoParamStateRestorer restorer(*theInfo);
	if(theInfo->Param(static_cast<FmiParameterName>(wantedParamId)))
	{
		// jos haluttu parametri lˆytyi datasta, palautetaan sen nimi
		return std::string(theInfo->Param().GetParamName());
	}

	// Jos kyse on meta-parametrista, palautetaan par + id string esim. par21
	std::string paramNameStr = "par";
	paramNameStr += std::to_string(wantedParamId);
	return paramNameStr;
}

static std::string MakeCsvPossibleLevelSectionString(boost::shared_ptr<NFmiFastQueryInfo>& theInfo)
{
	const auto* level = theInfo->Level();
	if(level && theInfo->SizeLevels() > 1)
	{
		auto levelType = level->LevelType();
		switch(levelType)
		{
		case kFmiHybridLevel:
		{
			std::string levelStr = ":lev";
			levelStr += std::to_string(boost::math::iround(level->LevelValue()));
			return levelStr;
		}
		case kFmiPressureLevel:
		{
			std::string levelStr = ":p";
			levelStr += std::to_string(boost::math::iround(level->LevelValue()));
			return levelStr;
		}
		case kFmiHeight:
		{
			std::string levelStr = ":z";
			levelStr += std::to_string(boost::math::iround(level->LevelValue()));
			return levelStr;
		}
		default:
			break;
		}
	}

	// Ei ole level tietoa tai sill‰ ei ole v‰li‰ (= pinta dataa)
	return "";
}

// Nimi tulee kolmesta mahdollisesta osiosta producer-parameter-level
std::string NFmiTimeSerialView::MakeCsvFullParameterNameString(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, boost::shared_ptr<NFmiDrawParam> possibleMacroParamDrawParam) //, unsigned long wantedParamId)
{
	CtrlViewDocumentInterface* ctrlViewDocumentInterface = CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation();
	if(ctrlViewDocumentInterface && theInfo)
	{
		boost::shared_ptr<NFmiDrawParam> drawParam = possibleMacroParamDrawParam ? possibleMacroParamDrawParam : ctrlViewDocumentInterface->InfoOrganizer()->CreateDrawParam(theInfo->Param(), theInfo->Level(), theInfo->DataType());
		if(drawParam)
		{
			auto parameterNameStr = CtrlViewUtils::GetParamNameString(drawParam, false, false, false, 0, true, false, true, nullptr);
			if(!theInfo->IsGrid())
			{
				parameterNameStr += "(";
				parameterNameStr += theInfo->Location()->GetName();
				parameterNameStr += ")";
			}
			return parameterNameStr;
		}
/*
		std::string parameterNameStr;
		auto& modelProducerSystem = ctrlViewDocumentInterface->ProducerSystem();
		auto& observationProducerSystem = ctrlViewDocumentInterface->ObsProducerSystem();
		parameterNameStr += ::MakeCsvProducerName(modelProducerSystem, observationProducerSystem, theInfo);
		parameterNameStr += ":";
		parameterNameStr += ::MakeCsvParameterNameSectionString(theInfo, wantedParamId);
		parameterNameStr += ::MakeCsvPossibleLevelSectionString(theInfo);
		return parameterNameStr;
*/
	}
	return "Unknown_param_name";
}

void NFmiTimeSerialView::FillTimeSerialMacroParamData(const NFmiPoint& latlon, std::vector<float>& values, const std::vector<NFmiMetTime>& times, boost::shared_ptr<NFmiFastQueryInfo>& macroParamInfo, boost::shared_ptr<NFmiDrawParam>& theMacroParamDrawParam, TimeSerialTooltipData* possibleTooltipData, NFmiExtraMacroParamData* possibleExtraMacroParamData)
{
	if(!possibleTooltipData)
	{
		if(!macroParamInfo)
			return;

		macroParamInfo->First(); // asetetaan varmuuden vuoksi First:iin
		// laitetaan myˆs t‰m‰ matriisi aluksi puuttuvaksi, ett‰ sit‰ ei virhetilanteissa tarvitse erikseen s‰‰dell‰
		values = std::vector<float>(macroParamInfo->SizeLocations(), kFloatMissing);
		// nollataan infossa ollut data missing-arvoilla, ett‰ saadaan puhdas kentt‰ laskuihin
		for(macroParamInfo->ResetLocation(); macroParamInfo->NextLocation(); )
		{
			macroParamInfo->FloatValue(kFloatMissing);
		}
		macroParamInfo->First(); // asetetaan varmuuden vuoksi viel‰ First:iin
	}

	NFmiMacroParamSystem& mpSystem = itsCtrlViewDocumentInterface->MacroParamSystem();
	NFmiSmartToolModifier smartToolModifier(itsCtrlViewDocumentInterface->InfoOrganizer());
	try // ensin tulkitaan macro
	{
		smartToolModifier.IncludeDirectory(itsCtrlViewDocumentInterface->SmartToolInfo()->LoadDirectory());

		auto macroParamPtr = mpSystem.GetWantedMacro(theMacroParamDrawParam->InitFileName());
		if(macroParamPtr)
		{
			smartToolModifier.InitSmartTool(macroParamPtr->MacroText(), true);
		}
		else
			throw runtime_error(string("NFmiTimeSerialView::FillTimeSerialMacroParamData: Error, couldn't find macroParam:") + theMacroParamDrawParam->ParameterAbbreviation());
	}
	catch(exception& e)
	{
		std::string errorText = CtrlViewUtils::MakeMacroParamRelatedFinalErrorMessage("Error: Time-serial Macro Parameter intepretion failed", &e, theMacroParamDrawParam, mpSystem.RootPath());
		CtrlViewUtils::SetMacroParamErrorMessage(errorText, *itsCtrlViewDocumentInterface, possibleTooltipData ? &possibleTooltipData->macroParamErrorMessage : nullptr);
	}

	try // suoritetaan macro sitten
	{
		if(possibleTooltipData)
		{
			smartToolModifier.CalcTimeSerialSmartToolValues(possibleTooltipData->values, possibleTooltipData->latlon, possibleTooltipData->times);
		}
		else
		{
			smartToolModifier.CalcTimeSerialSmartToolValues(values, latlon, times);
		}

		if(possibleExtraMacroParamData)
		{
			*possibleExtraMacroParamData = smartToolModifier.ExtraMacroParamData();
		}
	}
	catch(exception& e)
	{
		std::string errorText = CtrlViewUtils::MakeMacroParamRelatedFinalErrorMessage("Error: Time-serial MacroParam calculation failed", &e, itsDrawParam, mpSystem.RootPath());
		CtrlViewUtils::SetMacroParamErrorMessage(errorText, *itsCtrlViewDocumentInterface, possibleTooltipData ? &possibleTooltipData->macroParamErrorMessage : nullptr);
	}
}

// Tehd‰‰n aluksi macroParam laskuille lista aikoja, jotka menev‰t tunnin 
// v‰lein aikaikkunan alusta loppuun, oli ikkunen pituus kuinka pitk‰ hyv‰ns‰.
std::vector<NFmiMetTime> NFmiTimeSerialView::MakeMacroParamTimeVector()
{
	std::vector<NFmiMetTime> times;
	auto limitingTimes = itsCtrlViewDocumentInterface->TimeSerialViewTimeBag();
	for(limitingTimes.Reset(); limitingTimes.Next(); )
	{
		times.push_back(limitingTimes.CurrentTime());
	}
	return times;
}

void NFmiTimeSerialView::DrawTimeSerialMacroParam(boost::shared_ptr<NFmiFastQueryInfo>& macroParamInfo, boost::shared_ptr<NFmiDrawParam>& theMacroParamDrawParam, const NFmiPoint& latLonPoint, NFmiDrawingEnvironment& dataLineStyle)
{
	std::vector<NFmiMetTime> times = MakeMacroParamTimeVector();
	std::vector<float> values(times.size(), kFloatMissing);
	FillTimeSerialMacroParamData(latLonPoint, values, times, macroParamInfo, theMacroParamDrawParam, nullptr, nullptr);
	PlotTimeSerialData(values, times, dataLineStyle, g_PointNormal, g_PointSingle, true);
}

float NFmiTimeSerialView::GetMacroParamTooltipValue(boost::shared_ptr<NFmiFastQueryInfo>& macroParamInfo, boost::shared_ptr<NFmiDrawParam>& theMacroParamDrawParam, const NFmiPoint& latlon, const NFmiMetTime& time)
{
	vector<float> dummyValues{ kFloatMissing };
	vector<NFmiMetTime> dummyTimes{ NFmiMetTime::gMissingTime };
	TimeSerialTooltipData tooltipData;
	tooltipData.latlon = latlon;
	tooltipData.times[0] = time;
	tooltipData.values[0] = kFloatMissing;
	FillTimeSerialMacroParamData(latlon, dummyValues, dummyTimes, macroParamInfo, theMacroParamDrawParam, &tooltipData, nullptr);
	return tooltipData.values[0];
}
