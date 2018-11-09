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

#include "boost\math\special_functions\round.hpp"

static boost::shared_ptr<NFmiAreaMaskList> classesEmptyParamMaskList(new NFmiAreaMaskList()); // jos k‰ytt‰j‰ ei ole halunnut k‰ytt‰‰ maskeja laskuissaan, k‰ytet‰‰n t‰t‰ listaa todellisen maskilistan sijasta

static int itsModifyingUnit; // ainakin v‰liaikaisesti staattisena t‰‰ll‰ (optimointia change valueta laskettaessa)

bool fCPHelpColorsInitialized = false;
checkedVector<NFmiColor> gCPHelpColors; // kun piirret‰‰n muita kuin aktiivista CP:t‰, k‰ytet‰‰n n‰it‰ v‰rej‰
const int gMaxHelpCPDrawed = 20;

const double gDontDrawLineValue = -99999.9; // funktio DrawSimpleDataVectorInTimeSerial ei piirr‰ kyseisell‰ arvolla viivaa

const NFmiColor g_OfficialDataColor(0.78f, 0.082f, 0.52f); // viinin punainen kepa-datasta

using namespace std;

static boost::shared_ptr<NFmiFastQueryInfo> GetWantedData(CtrlViewDocumentInterface *theCtrlViewDocumentInterface, boost::shared_ptr<NFmiDrawParam> &theViewedDrawParam, const NFmiProducer &theWantedProducer, NFmiInfoData::Type theWantedType, bool fIgnoreParam = false)
{
	bool useParamIdOnly = theWantedProducer.GetIdent() == 0;
	NFmiDataIdent dataIdent(*(theViewedDrawParam->Param().GetParam()), theWantedProducer);
	NFmiInfoData::Type usedType = theWantedType;
	if(usedType == NFmiInfoData::kViewable && theViewedDrawParam->DataType() == NFmiInfoData::kHybridData)
		usedType = NFmiInfoData::kHybridData;
	const NFmiLevel *level = 0;
	if(theViewedDrawParam->Level().LevelValue() != kFloatMissing)
		level = &theViewedDrawParam->Level();

	boost::shared_ptr<NFmiFastQueryInfo> wantedInfo;
	if(fIgnoreParam)
	{
		bool groundData = (level == 0);
		checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > infos = theCtrlViewDocumentInterface->InfoOrganizer()->GetInfos(usedType, groundData, theWantedProducer.GetIdent());
		if(infos.size())
			wantedInfo = infos[0]; // palauteataan vain 1. info vektorista
	}
	else
	{
		wantedInfo = theCtrlViewDocumentInterface->InfoOrganizer()->Info(dataIdent, level, usedType, useParamIdOnly, false);
	}

	if(wantedInfo == 0)
		if(usedType == NFmiInfoData::kViewable) // jos ei lˆytynyt kViewable-tyypill‰, kokeillaan viel‰ kModelHelpData
			wantedInfo = ::GetWantedData(theCtrlViewDocumentInterface, theViewedDrawParam, theWantedProducer, NFmiInfoData::kModelHelpData, fIgnoreParam);

	return wantedInfo;
}

static bool IsMosDataUsed(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo)
{
    if(theViewedInfo->Param().GetParamIdent() == kFmiTemperature && theViewedInfo->SizeLevels() == 1)
        return true;
    else
        return false;
}

//--------------------------------------------------------
// NFmiTimeSerialView
//--------------------------------------------------------
NFmiTimeSerialView::NFmiTimeSerialView(const NFmiRect &theRect
						 ,NFmiToolBox *theToolBox
						 ,NFmiDrawingEnvironment * theDrawingEnvi
						 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
						 ,int theRowIndex
						 ,double theManualModifierLength)
:NFmiTimeView(0, theRect
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
,itsManualModifierLength(theManualModifierLength)
,itsMaxStationShowed(5)
,itsSinAmplitude(0)
,itsPhase(6)
,itsProducerModelDataColors()
,fJustScanningForMinMaxValues(false)
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
		fJustScanningForMinMaxValues = false;
		itsInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, false, true);
		if(itsInfo == 0)
			return;
        itsModifyingUnit = true;

		bool editedDataDrawed = IsEditedData(itsInfo);
		DrawBackground();

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
		DrawWantedDataLocationInTime(envi, theLatlon, itsDrawParam, NFmiInfoData::kEditingHelpData, NFmiProducer(NFmiProducerSystem::gHelpEditorDataProdId, "helpdata"), true);
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
        DrawAnnualModelFractileDataLocationInTime(static_cast<FmiParameterName>(itsDrawParam->Param().GetParamIdent()), itsCtrlViewDocumentInterface->GetModelClimatologyData(), theLatlon);
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
        eraFractileLabels.push_back("F99  ");
        eraFractileLabels.push_back("F97.5");
        eraFractileLabels.push_back("F95  ");
        eraFractileLabels.push_back("F87.5");
        eraFractileLabels.push_back("F50  ");
        eraFractileLabels.push_back("F12.5");
        eraFractileLabels.push_back("F5   ");
        eraFractileLabels.push_back("F2.5 ");
        eraFractileLabels.push_back("F1   ");
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

        AddRangeParamToMap(eraParamMap, kFmiCAPE, 4547, 4557, "Cape");
        // add both dewpoint params for same param range
        AddRangeParamToMap(eraParamMap, kFmiDewPoint, 4536, 4546, "Td");
        AddRangeParamToMap(eraParamMap, kFmiDewPoint2M, 4536, 4546, "Td");
        AddRangeParamToMap(eraParamMap, kFmiIceCover, 4558, 4568, "IceCover");
        AddRangeParamToMap(eraParamMap, kFmiMaximumTemperature, 4580, 4590, "T-max");
        AddRangeParamToMap(eraParamMap, kFmiMinimumTemperature, 4591, 4601, "T-min");
        AddRangeParamToMap(eraParamMap, kFmiPressure, 4613, 4623, "P");
        AddRangeParamToMap(eraParamMap, kFmiSnowDepth, 4569, 4579, "SnowDepth");
        AddRangeParamToMap(eraParamMap, kFmiTemperatureSea, 4602, 4612, "T-sea");
        AddRangeParamToMap(eraParamMap, kFmiTotalColumnWater, 4624, 4634, "TCW");

        // Add 1h and 3h precipitations both for same fraktile indecies
        AddInitializerListToMap(eraParamMap, { kFmiPrecipitation1h, kFmiPrecipitation3h, kFmiPrecipitationRate }, { 946, 945, 4516, 943, 4517, 935, 4518, 923, 4519, 921, 920 }, "rr3h");
        // Both temperatures also for same params
        AddInitializerListToMap(eraParamMap, { kFmiTemperature, kFmiTemperature2M }, { 871, 4500, 4501, 4502, 4503, 874, 4504, 4505, 4506, 4507, 877 }, "T");
        AddInitializerListToMap(eraParamMap, { kFmiTotalCloudCover }, { 881, 4508, 4509, 4510, 4511, 884, 4512, 4513, 4514, 4515, 887 }, "N");
        // all possible wind gusts added for same params
        AddInitializerListToMap(eraParamMap, { kFmiWindGust, kFmiHourlyMaximumGust, kFmiWindGust2 }, { 1117, 4528, 4529, 4530, 4531, 1120, 4532, 4533, 4534, 4535, 1123 }, "WindGust");
        AddInitializerListToMap(eraParamMap, { kFmiWindSpeedMS }, { 1110, 4520, 4521, 4522, 4523, 1113, 4524, 4525, 4526, 4527, 1116 }, "WS");
    }
}

void NFmiTimeSerialView::DrawAnnualModelFractileDataLocationInTime(FmiParameterName mainParameter, boost::shared_ptr<NFmiFastQueryInfo> &climateInfo, const NFmiPoint &theLatlon)
{
    if(climateInfo)
    {
        // make sure that param map is initialized
        ModelClimatology::initEraParamMap();
        auto paramMapIter = ModelClimatology::eraParamMap.find(mainParameter);
        if(paramMapIter != ModelClimatology::eraParamMap.end())
        {
            DrawAnnualModelFractileDataLocationInTime(climateInfo, theLatlon, paramMapIter->second.second);
        }
    }
}

void NFmiTimeSerialView::DrawAnnualModelFractileDataLocationInTime(boost::shared_ptr<NFmiFastQueryInfo> &climateInfo, const NFmiPoint &theLatlon, const ModelClimatology::ParamIds &paramIds)
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
                DrawAnnualModelFractileDataLocationInTime(climateInfo, theLatlon, paramIds, startClimatologyDataTime, climateInfo->TimeDescriptor().LastTime(), startYearDiff);
                DrawAnnualModelFractileDataLocationInTime(climateInfo, theLatlon, paramIds, climateInfo->TimeDescriptor().FirstTime(), endClimatologyDataTime, endYearDiff);
                // Also draw possible full years between start and end year
                for(auto yearIndex = startYearDiff + 1; yearIndex < endYearDiff; yearIndex++)
                    DrawAnnualModelFractileDataLocationInTime(climateInfo, theLatlon, paramIds, climateInfo->TimeDescriptor().FirstTime(), climateInfo->TimeDescriptor().LastTime(), yearIndex);
            }
            else
            {
                DrawAnnualModelFractileDataLocationInTime(climateInfo, theLatlon, paramIds, startClimatologyDataTime, endClimatologyDataTime, startYearDiff);
            }
        }
    }
}

void NFmiTimeSerialView::DrawAnnualModelFractileDataLocationInTime(boost::shared_ptr<NFmiFastQueryInfo> &climateInfo, const NFmiPoint &theLatlon, const ModelClimatology::ParamIds &paramIds, const NFmiMetTime &startTime, const NFmiMetTime &endTime, int climateDataYearDifference)
{
    NFmiTimeBag drawedTimes(startTime, endTime, 60); // resoluutiolla ei merkityst‰
    int timeWhenDrawedInMinutes = climateDataYearDifference * 365 * 24 * 60;
    DrawAnnualModelFractileDataLocationInTime(climateInfo, theLatlon, paramIds, drawedTimes, timeWhenDrawedInMinutes);
}

void NFmiTimeSerialView::DrawAnnualModelFractileDataLocationInTime(boost::shared_ptr<NFmiFastQueryInfo> &climateInfo, const NFmiPoint &theLatlon, const ModelClimatology::ParamIds &paramIds, const NFmiTimeBag &theDrawedTimes, int theTimeWhenDrawedInMinutes)
{
    NFmiDrawingEnvironment envi;
    envi.SetFillPattern(FMI_DOT);
    envi.SetPenSize(NFmiPoint(1, 1));
    for(size_t i = 0; i < paramIds.size(); i++)
    {
        if(climateInfo->Param(paramIds[i]))
        {
            envi.SetFrameColor(ModelClimatology::eraColorsForFractiles[i]);
            DrawSimpleDataInTimeSerial(theDrawedTimes, climateInfo, envi, theLatlon, theTimeWhenDrawedInMinutes, NFmiPoint(6, 6), true);
        }
    }
}

void NFmiTimeSerialView::DrawTemperatureMinAndMaxFromHelperData(FmiParameterName mainParameter, boost::shared_ptr<NFmiFastQueryInfo> &helperDataInfo, const NFmiPoint &theLatlon)
{
    if(helperDataInfo)
    {
        NFmiTimeBag drawedTimes;
        if(GetDrawedTimes(helperDataInfo, drawedTimes))
        {
            float colorFactor = 0.3f;
            NFmiDrawingEnvironment envi;
            envi.SetFillPattern(FMI_DASHDOTDOT);
            envi.SetPenSize(NFmiPoint(1, 1));
            envi.SetFrameColor(NFmiColor(colorFactor, colorFactor, colorFactor*2.5f));
            if(helperDataInfo->Param(kFmiMinimumTemperature))
                DrawSimpleDataInTimeSerial(drawedTimes, helperDataInfo, envi, theLatlon, 0, NFmiPoint(4, 4), true);
            envi.SetFrameColor(NFmiColor(colorFactor*2.5f, colorFactor, colorFactor));
            if(helperDataInfo->Param(kFmiMaximumTemperature))
                DrawSimpleDataInTimeSerial(drawedTimes, helperDataInfo, envi, theLatlon, 0, NFmiPoint(4, 4), true);
        }
    }
}

void NFmiTimeSerialView::DrawModelDataLegend(const std::vector<NFmiColor> &theUsedColors, const std::vector<std::string> &theFoundProducerNames)
{
	if(fJustScanningForMinMaxValues)
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
				NFmiText text(textPoint, theFoundProducerNames[i].c_str(), 0, &envi);
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
    boost::shared_ptr<NFmiFastQueryInfo> info = ::GetWantedData(itsCtrlViewDocumentInterface, drawParam, producer, dataType);
    if(info)
    {
        drawingEnvironmentInOut.SetFrameColor(color);
        NFmiText text(legendPlaceInOut, producer.GetName(), 0, &drawingEnvironmentInOut);
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

bool NFmiTimeSerialView::DrawModelDataLocationInTime(NFmiDrawingEnvironment &envi, const NFmiPoint &theLatlon, const NFmiProducer &theProducer)
{
	itsToolBox->UseClipping(true);
    boost::shared_ptr<NFmiFastQueryInfo> info = ::GetWantedData(itsCtrlViewDocumentInterface, itsDrawParam, theProducer, NFmiInfoData::kViewable);
	if(info)
	{
		info->ResetTime(); // varmuuden vuoksi asetan 1. aikaan
//		if(info->Grid() && info->Area()->IsInside(theLatlon))
		{
			if(info->Param(*Info()->Param().GetParam())) // parametrikin pit‰‰ asettaa
			{
				DrawSimpleDataInTimeSerial(info, envi, theLatlon, 0, NFmiPoint(6, 6)); // 0=ei siirret‰ aikasarjaa mihink‰‰n suuntaa piirrossa
				return true;
			}
		}
	}
	return false;
}

void NFmiTimeSerialView::DrawWantedDataLocationInTime(NFmiDrawingEnvironment &envi, const NFmiPoint &theLatlon, boost::shared_ptr<NFmiDrawParam> &theViewedDrawParam, NFmiInfoData::Type theDataType, const NFmiProducer &theProducer, bool fGroundData)
{
	boost::shared_ptr<NFmiFastQueryInfo> info = ::GetWantedData(itsCtrlViewDocumentInterface, theViewedDrawParam, theProducer, theDataType);
	if(info)
	{
		info->ResetTime(); // varmuuden vuoksi asetan 1. leveliin
		if(info->Param(*Info()->Param().GetParam())) // parametrikin pit‰‰ asettaa
		{
			envi.SetPenSize(NFmiPoint(1, 1));
			DrawSimpleDataInTimeSerial(info, envi, theLatlon, 0, NFmiPoint(6, 6)); // 0=ei siirret‰ aikasarjaa mihink‰‰n suuntaa piirrossa
			envi.SetPenSize(NFmiPoint(1, 1)); // ohut viiva takaisin
		}
	}
}

void NFmiTimeSerialView::DrawKepaDataLocationInTime(NFmiDrawingEnvironment &envi, const NFmiPoint &theLatlon)
{
	NFmiProducer prod(0, "none"); // 0 producer ignoorataan GetWantedData-funktiossa
	boost::shared_ptr<NFmiFastQueryInfo> kepaInfo = ::GetWantedData(itsCtrlViewDocumentInterface, itsDrawParam, prod, NFmiInfoData::kKepaData);
	if(kepaInfo)
	{
		kepaInfo->ResetTime(); // varmuuden vuoksi asetan 1. aikaan
		envi.SetFrameColor(g_OfficialDataColor);
		envi.SetPenSize(NFmiPoint(2, 2));
		DrawSimpleDataInTimeSerial(kepaInfo, envi, theLatlon, 0, NFmiPoint(6, 6)); // 0=ei siirret‰ aikasarjaa mihink‰‰n suuntaa piirrossa
		envi.SetPenSize(NFmiPoint(1, 1)); // ohut viiva takaisin
	}
}

static pair<int, double> FindClosestLocationWithData(checkedVector<pair<int, double> > &theNearestLocations, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiTimeBag& theCheckedTimes)
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

// kun piirret‰‰n havaintoja aikasarjaan editoidun datan lis‰ksi, halutaan etsi‰
// Kiinnostavaa pistett‰ l‰hin havainto. 'Kiinnostava' piste on normaalisti editoidusta
// datasta valittu piste. Mutta Controllipiste tyˆkalun ollessa k‰ynniss‰ se onkin
// aktiivinen CP-piste.
const NFmiPoint& NFmiTimeSerialView::GetUsedLatlon(void)
{
	if(itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode())
		return itsCtrlViewDocumentInterface->CPManager()->ActiveCPLatLon();
	else
		return Info()->LatLon();
}

bool NFmiTimeSerialView::GetDrawedTimes(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiTimeBag &theTimesOut)
{
	NFmiMetTime lastTime(theInfo->TimeDescriptor().LastTime());
	NFmiMetTime firstViewTime(Value2Time(NFmiPoint(0,0))); // haetaan aika, joka on ruudun alussa
	if(theInfo->TimeDescriptor().IsInside(firstViewTime) ? theInfo->FindNearestTime(firstViewTime, kBackward) : theInfo->FindNearestTime(firstViewTime, kCenter))
	{
		NFmiMetTime firstTime(theInfo->Time());
		NFmiTimeBag drawedTimes(firstTime, lastTime, theInfo->TimeDescriptor().Resolution());
		theTimesOut = drawedTimes;
		return true;
	}
	return false;
}

bool NFmiTimeSerialView::SetObsDataToNearestLocationWhereIsData(boost::shared_ptr<NFmiFastQueryInfo> &theObsInfo, const NFmiPoint &theLatlon, std::pair<int, double> &theLocationWithDataOut)
{
    NFmiTimeBag checkedTimes(GetViewLimitingTimes()); // kiinnostaa vain n‰kyv‰n aika-alueen datat
	checkedVector<pair<int, double> > nearestLocations = theObsInfo->NearestLocations(theLatlon, 5);
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
    if(!obsInfo || obsInfo->Param(theParam) == false)
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

    checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > obsInfos = itsCtrlViewDocumentInterface->InfoOrganizer()->GetInfos(NFmiInfoData::kObservations);
    for(size_t i= 0; i < obsInfos.size(); i++)
    {
        boost::shared_ptr<NFmiFastQueryInfo> &currentInfo = obsInfos[i];
        std::set<long>::iterator it = excludedProducerIds.find(currentInfo->Producer()->GetIdent());
        if(it == excludedProducerIds.end())
        {
            if(currentInfo->Param(theParam))
                return currentInfo;
        }
    }
    return boost::shared_ptr<NFmiFastQueryInfo>();
}

void NFmiTimeSerialView::DrawObservationDataLocationInTime(NFmiDrawingEnvironment &envi, const NFmiPoint &theLatlon)
{
	if(itsDrawParam->Level().LevelValue() != kFloatMissing)
		return ; // havainto apu piirrot tehd‰‰n vain pintadatalle
	boost::shared_ptr<NFmiFastQueryInfo> obsInfo = GetObservationInfo(*Info()->Param().GetParam(), theLatlon);
	if(obsInfo)
	{
		obsInfo->FirstLevel(); // varmuuden vuoksi asetan 1. leveliin
		if(obsInfo->Param(*Info()->Param().GetParam())) // parametrikin pit‰‰ asettaa
		{
			NFmiTimeBag drawedTimes;
			if(GetDrawedTimes(obsInfo, drawedTimes))
			{
				std::pair<int, double> locationWithData;
				if(SetObsDataToNearestLocationWhereIsData(obsInfo, theLatlon, locationWithData))
				{
					const NFmiLocation *loc = obsInfo->Location();
					NFmiPoint place(CalcParamTextPosition());
					NFmiPoint fontSize(CalcFontSize());
					fontSize *= NFmiPoint(0.9, 0.9); // pienennet‰‰n fonttia hieman

					// pit‰‰ laske kuinka monta pistett‰ on piirretty aikasarjaan. Huom! v‰hint‰‰n yksi on piirretty, jos ollaan t‰‰ll‰!
					unsigned long displayCount = CtrlViewFastInfoFunctions::GetMaskedCount(itsCtrlViewDocumentInterface->EditedSmartInfo(), NFmiMetEditorTypes::kFmiDisplayedMask, itsCtrlViewDocumentInterface->AllowRightClickDisplaySelection());
					if(displayCount == 0)
						displayCount = 1;

					// displayCount+1 pit‰‰ laittaa kertoimeksi, koska myˆs havainto asema on piirretty
					place.Y(place.Y() + 0.9*(displayCount+1)*itsToolBox->SY(static_cast<long>(fontSize.Y())));
					NFmiString locString(::GetDictionaryString("TimeSerialViewObservationStation"));
					locString += " ";
					envi.SetFrameColor(NFmiColor(0, 0, 0));
					DrawStationNameLegend(loc, envi, fontSize, place, locString, kTop, locationWithData.second);

					envi.SetFrameColor(NFmiColor(0.956f, 0.282f, 0.05f)); // vihert‰v‰‰ havainto datasta
					envi.SetPenSize(NFmiPoint(3, 3)); // paksunnetaan viivaa
					DrawSimpleDataInTimeSerial(drawedTimes, obsInfo, envi, theLatlon, 0, NFmiPoint(9, 9), true, locationWithData.first); // 0=ei siirret‰ aikasarjaa mihink‰‰n suuntaa piirrossa
					envi.SetPenSize(NFmiPoint(1, 1)); // ohut viiva takaisin
				}
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
	if(fJustScanningForMinMaxValues)
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
				int viewYear = timesInView.FirstTime().GetYear();
				NFmiTimeDescriptor fraktiiliTimes(fraktiiliInfo->TimeDescriptor());
				int fraktiiliDataYear = fraktiiliTimes.FirstTime().GetYear(); // fraktiili data on jollekin vuodelle tehty pˆtkˆ, otetaan vuosi talteen, ett‰ voidaan rakentaa sopiva timebagi datan l‰pik‰ymiseen
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
						int changeTimeWhenDrawedInMinutes = (viewYear - fraktiiliDataYear) * 365 * 24 * 60;
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
								DrawSimpleDataInTimeSerial(drawedTimes, fraktiiliInfo, envi, fraktiiliInfo->LatLon(), changeTimeWhenDrawedInMinutes, NFmiPoint(6, 6));
							}
							if(fraktiiliInfo->Param(kFmiNormalMinTemperatureF50))
							{
								envi.SetFrameColor(NFmiColor(0.1f,0.f,0.95f)); // 'mean minimi' arvo siniseksi
								DrawSimpleDataInTimeSerial(drawedTimes, fraktiiliInfo, envi, fraktiiliInfo->LatLon(), changeTimeWhenDrawedInMinutes, NFmiPoint(6, 6));
							}
							if(fraktiiliInfo->Param(kFmiNormalMeanTemperature))
							{
								envi.SetFrameColor(NFmiColor(0.1f,0.89f,0.15f)); // 'avg' arvo vihre‰ksi
								DrawSimpleDataInTimeSerial(drawedTimes, fraktiiliInfo, envi, fraktiiliInfo->LatLon(), changeTimeWhenDrawedInMinutes, NFmiPoint(6, 6));
							}
							if(fraktiiliInfo->Param(kFmiNormalMaxTemperatureF50))
							{
								envi.SetFrameColor(NFmiColor(0.95f,0.1f,0.05f)); // 'mean maksimi' arvo punaiseksi
								DrawSimpleDataInTimeSerial(drawedTimes, fraktiiliInfo, envi, fraktiiliInfo->LatLon(), changeTimeWhenDrawedInMinutes, NFmiPoint(6, 6));
							}
							if(fraktiiliInfo->Param(kFmiNormalMaxTemperatureF98))
							{
								envi.SetFrameColor(NFmiColor(0.95f,0.54f,0.54f)); // 'maksimin maksimi' arvo haalean punaiseksi, koska kiinnostaa v‰hemm‰n
								DrawSimpleDataInTimeSerial(drawedTimes, fraktiiliInfo, envi, fraktiiliInfo->LatLon(), changeTimeWhenDrawedInMinutes, NFmiPoint(6, 6));
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
	if(theInfo->Param(theParam))
	{
		theEnvi.SetFrameColor(theColor);
		DrawSimpleDataInTimeSerial(theInfo, theEnvi, theLatlon, 0, theEmptyPointSize); // 0=ei siirret‰ aikasarjaa mihink‰‰n suuntaa piirrossa
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
	boost::shared_ptr<NFmiFastQueryInfo> fractileData = itsCtrlViewDocumentInterface->GetFavoriteSurfaceModelFractileData();
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

namespace
{
    // SeaLevelProbData luokka pit‰‰ sis‰ll‰‰n sea-level parametriin liittyvien probability rajojen vakio arvoja.
    // Jokaiselle tunnetulle seaLevel asemalle annetaan erikseen siihen liittyv‰t rajat.
    // Kun Kartalta valitaan paikka, joka halutaan piirt‰‰ aikasarjaan, etsit‰‰n 300 km l‰heisyydest‰ l‰hin asema. Jos kyseisen aseman 
    // stationId lˆytyy oliosta, k‰ytet‰‰n sen prob rajoja piirt‰m‰‰n rajat aikasarjaan.
    class SeaLevelProbData
    {
    public:
        SeaLevelProbData() = default;
        SeaLevelProbData(const NFmiStation &station, float prob1, float prob2, float prob3, float prob4)
            :station_(station),
            prob1_(prob1),
            prob2_(prob2),
            prob3_(prob3),
            prob4_(prob4)
        {}

        NFmiStation station_;
        float prob1_ = 0;
        float prob2_ = 0;
        float prob3_ = 0;
        float prob4_ = 0;
    };

    const NFmiProducer g_SeaLevelPlumeProducer(2168, "Hansen EPS");
    std::vector<FmiParameterName> g_SeaLevelPlumeFractileParams{ static_cast<FmiParameterName>(1309), static_cast<FmiParameterName>(1310), static_cast<FmiParameterName>(1311) , static_cast<FmiParameterName>(1312) , static_cast<FmiParameterName>(1313) , static_cast<FmiParameterName>(1314) , static_cast<FmiParameterName>(1315) };
    std::vector<std::string> g_SeaLevelPlumeFractileParamLabels{ "F95", "F90", "F75", "F50", "F25", "F10", "F5" };
    std::vector<NFmiColor> g_SeaLevelPlumeFractileParamColors{ NFmiColor(1.f, 0.f, 0.f), NFmiColor(1.f, 0.25f, 0.25f), NFmiColor(1.f, 0.5f, 0.5f) , NFmiColor(0.f, 0.5f, 0.f), NFmiColor(0.4f, 0.4f, 1.f), NFmiColor(0.25f, 0.25f, 1.f), NFmiColor(0.f, 0.f, 1.f) };
    std::vector<SeaLevelProbData> g_SeaLevelProbabilityStationData;
    // ProbLimit parametrit 1-4
    std::vector<FmiParameterName> g_SeaLevelProbLimitParams{ static_cast<FmiParameterName>(1305), static_cast<FmiParameterName>(1306), static_cast<FmiParameterName>(1307) , static_cast<FmiParameterName>(1308) };
    // ProbLimit 1-4 viiva v‰rit: keltainen, keltainen, oranssi ja punainen
    std::vector<NFmiColor> g_SeaLevelProbabilityLineColors{ NFmiColor(0.7f, 0.7f, 0.f), NFmiColor(0.7f, 0.7f, 0.f), NFmiColor(1.f, 0.5f, 0.f) , NFmiColor(1.f, 0.f, 0.f) };
    double g_SeaLevelProbabilityMaxSearchRangeInMetres = 250 * 1000;

    void InitSeaLevelProbabilityStationData()
    {
        if(g_SeaLevelProbabilityStationData.size() == 0)
        {
            // Hamina kuuluu Suomenlahden it‰osaan (prob arvot sielt‰)
            g_SeaLevelProbabilityStationData.push_back(SeaLevelProbData(NFmiStation(100051, "Hamina", 27.2, 60.56), -70, 110, 145, 175));
            // Porvoo ja Helsinki (Suomenlahden l‰nsiosaan)
            g_SeaLevelProbabilityStationData.push_back(SeaLevelProbData(NFmiStation(100066, "Porvoo", 25.6251, 60.2058), -60, 80, 115, 130));
            g_SeaLevelProbabilityStationData.push_back(SeaLevelProbData(NFmiStation(100052, "Helsinki", 24.95, 60.13), -60, 80, 115, 130));
            // Turku ja Hanko (Saaristomeri ja Suomenlahden l‰nsiosa)
            g_SeaLevelProbabilityStationData.push_back(SeaLevelProbData(NFmiStation(100054, "Turku", 22.22, 60.44), -50, 70, 95, 110));
            g_SeaLevelProbabilityStationData.push_back(SeaLevelProbData(NFmiStation(100053, "Hanko", 22.95, 59.82), -50, 70, 95, 110));
            // Degerby (Ahvenanmeri ja Saaristomeri)
            g_SeaLevelProbabilityStationData.push_back(SeaLevelProbData(NFmiStation(100055, "Degerby", 20.38, 60.03), -50, 65, 85, 100));
            // Rauma, M‰ntyluoto ja Kaskinen (Selk‰meri)
            g_SeaLevelProbabilityStationData.push_back(SeaLevelProbData(NFmiStation(100056, "Rauma", 21.49, 61.13), -50, 75, 100, 120));
            g_SeaLevelProbabilityStationData.push_back(SeaLevelProbData(NFmiStation(100057, "M‰ntyluoto", 21.49, 61.59), -50, 75, 100, 120));
            g_SeaLevelProbabilityStationData.push_back(SeaLevelProbData(NFmiStation(100058, "Kaskinen", 21.21, 62.34), -50, 75, 100, 120));
            // Vaasa (Merenkurkku)
            g_SeaLevelProbabilityStationData.push_back(SeaLevelProbData(NFmiStation(100059, "Vaasa", 21.61, 63.1), -50, 85, 110, 130));
            // Pietarsaari (Per‰meren etel‰osa)
            g_SeaLevelProbabilityStationData.push_back(SeaLevelProbData(NFmiStation(100060, "Pietarsaari", 22.67, 63.68), -65, 85, 110, 130));
            // Raahe, Oulu ja Kemi (Per‰meren pohjoisosa)
            g_SeaLevelProbabilityStationData.push_back(SeaLevelProbData(NFmiStation(100061, "Raahe", 24.48, 64.68), -65, 85, 110, 130));
            g_SeaLevelProbabilityStationData.push_back(SeaLevelProbData(NFmiStation(100062, "Oulu", 25.49, 65.02), -80, 115, 140, 170));
            g_SeaLevelProbabilityStationData.push_back(SeaLevelProbData(NFmiStation(100063, "Kemi", 24.53, 65.68), -80, 115, 140, 170));
        }
    }

    const SeaLevelProbData* FindSeaLevelProbabilityStationData(const NFmiLocation *location, const NFmiPoint &latlon)
    {
        if(location)
        {
            // Etsi joko tarkka location-id pohjaisen aseman data
            for(const auto &probStationData : g_SeaLevelProbabilityStationData)
            {
                if(location->GetIdent() == probStationData.station_.GetIdent())
                    return &probStationData;
            }
        }
        else
        {
            // Tai etsi l‰himm‰n latlon pisteesen liittyv‰n aseman data, edellytt‰en ett‰ latlon piste on g_SeaLevelProbabilityMaxSearchRangeInMetres sis‰ll‰.
            // T‰t‰ k‰ytet‰‰n, jos seaLevel data sattuu olemaan hiladataa.
            double minDistanceInMetres = 999999999;
            const SeaLevelProbData *minDistanceProbDataPtr = nullptr;
            NFmiLocation searchedLocation(latlon);
            for(const auto &probStationData : g_SeaLevelProbabilityStationData)
            {
                auto currentDistanceInMeters = searchedLocation.Distance(probStationData.station_);
                if(currentDistanceInMeters < minDistanceInMetres)
                {
                    minDistanceInMetres = currentDistanceInMeters;
                    minDistanceProbDataPtr = &probStationData;
                }
            }
            if(minDistanceInMetres <= g_SeaLevelProbabilityMaxSearchRangeInMetres)
                return minDistanceProbDataPtr;
        }
        return nullptr;
    }
}

bool NFmiTimeSerialView::IsSeaLevelPlumeParam()
{
    return itsDrawParam->Param().GetParamIdent() == kFmiSeaLevel;
}

bool NFmiTimeSerialView::IsSeaLevelProbLimitParam()
{
    auto iter = std::find(g_SeaLevelProbLimitParams.begin(), g_SeaLevelProbLimitParams.end(), itsDrawParam->Param().GetParamIdent());
    return iter != g_SeaLevelProbLimitParams.end();
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiTimeSerialView::GetSeaLevelPlumeData()
{
    return itsCtrlViewDocumentInterface->InfoOrganizer()->FindInfo(NFmiInfoData::kViewable, g_SeaLevelPlumeProducer, true);
}

// Jos on ollut ShowHelperData2InTimeSerialView p‰‰ll‰ (= n‰yt‰ EC:n lyhyit‰ fraktiileja pluumina)
// Jos kyseess‰ on Sea-level parametri (id = 60), katsotaan lˆytyykˆ siihen liittyv‰‰
// Hansenin meri fraktiili dataa (*_waterlevel_hansen_EPS.sqd).
void NFmiTimeSerialView::DrawPossibleSeaLevelPlumeDataLocationInTime(const NFmiPoint &theLatlon)
{
    if(IsSeaLevelPlumeParam())
    {
        auto seaLevelFractileData = GetSeaLevelPlumeData();
        if(seaLevelFractileData)
        {
            NFmiDrawingEnvironment envi;
            envi.SetFillPattern(FMI_DASHDOTDOT);

            // Piirr‰ sea-level fraktiilit
            for(size_t i = 0; i < g_SeaLevelPlumeFractileParams.size(); i++)
            {
                DrawParamInTime(seaLevelFractileData, envi, theLatlon, g_SeaLevelPlumeFractileParams[i], g_SeaLevelPlumeFractileParamColors[i], NFmiPoint(1, 1));
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
    if(IsSeaLevelProbLimitParam())
    {
        auto seaLevelFractileData = GetSeaLevelPlumeData();
        if(seaLevelFractileData)
        {
            NFmiDrawingEnvironment envi;
            envi.SetPenSize(NFmiPoint(2, 2));

            // Piirr‰ sea-level fraktiilit
            for(size_t i = 0; i < g_SeaLevelProbLimitParams.size(); i++)
            {
                DrawParamInTime(seaLevelFractileData, envi, theLatlon, g_SeaLevelProbLimitParams[i], g_SeaLevelProbabilityLineColors[i], NFmiPoint(1, 1));
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
void NFmiTimeSerialView::DrawSeaLevelProbLines(const NFmiPoint &theLatlon)
{
    if(fJustScanningForMinMaxValues)
        return;

    auto oldLocationIndex = itsInfo->LocationIndex();
    if(itsInfo->NearestLocation(theLatlon, g_SeaLevelProbabilityMaxSearchRangeInMetres))
    {
        // Varmistetaan ett‰ taulukko on olemassa, t‰m‰ on ‰‰ri nopea toiminto, kun se on kerran alustettu
        InitSeaLevelProbabilityStationData();
        auto probStationData = ::FindSeaLevelProbabilityStationData(itsInfo->Location(), itsInfo->LatLon());
        if(probStationData)
        {
            NFmiDrawingEnvironment envi;
            envi.SetPenSize(NFmiPoint(2, 2));
            DrawSeaLevelProbLine(envi, g_SeaLevelProbabilityLineColors[0], probStationData->prob1_);
            DrawSeaLevelProbLine(envi, g_SeaLevelProbabilityLineColors[1], probStationData->prob2_);
            DrawSeaLevelProbLine(envi, g_SeaLevelProbabilityLineColors[2], probStationData->prob3_);
            DrawSeaLevelProbLine(envi, g_SeaLevelProbabilityLineColors[3], probStationData->prob4_);
        }
    }
    itsInfo->LocationIndex(oldLocationIndex);
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
	if(fJustScanningForMinMaxValues)
		return ;
	itsToolBox->UseClipping(false);
	if(theLocation)
	{
		theEnvi.SetFontSize(theFontSize);
		NFmiString tmpStr(thePreLocationString);
		tmpStr += theLocation->GetName();

		if(theDistanceInMeters != kFloatMissing)
		{
			tmpStr += " (dist ";
			NFmiValueString distStr(theDistanceInMeters/1000., "%.1f");
			tmpStr += distStr;
			tmpStr += " km)";
		}

		NFmiText text(theTextPos, tmpStr, 0, &theEnvi);
		FmiDirection oldAligment = itsToolBox->GetTextAlignment();
		itsToolBox->SetTextAlignment(theTextAligment);
		itsToolBox->Convert(&text);
		itsToolBox->SetTextAlignment(oldAligment);
	}
}

static float GetWantedValue(NFmiFastQueryInfo &theInfo, const NFmiPoint &theLatLonPoint, int theWantedLocationIndex)
{
	if(theWantedLocationIndex < 0)
		return theInfo.InterpolatedValue(theLatLonPoint);
	else
	{
		theInfo.LocationIndex(theWantedLocationIndex);
		return theInfo.FloatValue();
	}
}

void NFmiTimeSerialView::DrawSimpleDataInTimeSerial(const NFmiTimeBag &theDrawedTimes, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiDrawingEnvironment &theEnvi, const NFmiPoint &theLatLonPoint, int theTimeWhenDrawedInMinutes, const NFmiPoint& theSinglePointSize, bool drawConnectingLines, int theWantedLocationIndex)
{
	if(fJustScanningForMinMaxValues)
	{
		ScanDataForMinAndMaxValues(theInfo, theLatLonPoint, itsAutoAdjustScanTimes, itsAutoAdjustMinMaxValues);
		return ;
	}

	itsToolBox->UseClipping(true);
	int size = theDrawedTimes.GetSize();
	if(size >= 1)
	{
		NFmiDrawingEnvironment blackLineEnvi;
		theInfo->Time(theDrawedTimes.FirstTime()); // pit‰‰ asettaa 1. aika p‰‰lle, mutta ei tarvitse tarkistaa sit‰
		NFmiPoint pointSize(1, 1);
		double realValue1 = ::GetWantedValue(*theInfo, theLatLonPoint, theWantedLocationIndex)
			  ,realValue2 = kFloatMissing;
		double lastNonMissingValue = kFloatMissing;
		NFmiMetTime time1(theDrawedTimes.FirstTime())
				   ,time2;
		NFmiMetTime lastNonMissingValueTime;
		time1.ChangeByMinutes(theTimeWhenDrawedInMinutes);
        NFmiMetTime lastTimeOnView = GetViewLimitingTimes().LastTime();
		for( ; theInfo->NextTime(); )
		{
			time2 = theInfo->Time();
			time2.ChangeByMinutes(theTimeWhenDrawedInMinutes);
			realValue2 = ::GetWantedValue(*theInfo, theLatLonPoint, theWantedLocationIndex);
			if(drawConnectingLines && realValue1 == kFloatMissing && realValue2 != kFloatMissing && lastNonMissingValue != kFloatMissing)
			{ // piirret‰‰n ohut musta yhteysviiv‰ katkonaisiin kohtiin, jos niin on s‰‰detty
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
            if(time2 > lastTimeOnView)
                break;
		}

        if(size == 1)
        {
            DrawSinglePointData(realValue1, time1, theEnvi, theSinglePointSize);
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

void NFmiTimeSerialView::DrawSimpleDataInTimeSerial(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiDrawingEnvironment &theEnvi, const NFmiPoint &theLatlon, int theTimeWhenDrawedInMinutes, const NFmiPoint& theSinglePointSize, bool fUseValueAxis)
{
	if(fJustScanningForMinMaxValues)
	{
		ScanDataForMinAndMaxValues(theInfo, theLatlon, itsAutoAdjustScanTimes, itsAutoAdjustMinMaxValues);
		return ;
	}
	itsToolBox->UseClipping(true);
	int size = theInfo->SizeTimes();
	bool interpolateValues = theInfo->IsGrid(); // jos asema dataa, pit‰‰ laittaa nearest
	NFmiLocation wantedLocation(theLatlon);
	if(interpolateValues == false && (theInfo->NearestLocation(wantedLocation, gMaxDistanceToFractileStation) == false))
		return ; // jos asema dataa ei lˆydy 500 km sis‰lt‰ haluttua pistett‰, ei k‰ytet‰ sit‰
	if(size >= 1)
	{
		NFmiPoint pointSize(1, 1);
		theInfo->FirstTime();
		double realValue1 = interpolateValues ? theInfo->InterpolatedValue(theLatlon) : theInfo->FloatValue()
			  ,realValue2 = kFloatMissing;
		NFmiMetTime time1(theInfo->TimeDescriptor().FirstTime())
				   ,time2;
		time1.ChangeByMinutes(theTimeWhenDrawedInMinutes);
		for(int i=0; theInfo->NextTime(); i++)
		{
			time2 = theInfo->Time();
			time2.ChangeByMinutes(theTimeWhenDrawedInMinutes);
			realValue2 = interpolateValues ? theInfo->InterpolatedValue(theLatlon) : theInfo->FloatValue();
			DrawDataLine(time1, time2, realValue1, realValue2, theEnvi, pointSize, theSinglePointSize, fUseValueAxis);
			realValue1 = realValue2;
			time1 = time2;
		}

        if(size == 1)
        {
            DrawSinglePointData(realValue1, time1, theEnvi, theSinglePointSize);
        }
    }
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

// Timebag-optimoitu piirto rutiini. Ei k‰yt‰ mettime:ia ollenkaan vaan suoraan x-positiota.
// ottaa huomioon myˆs, jos viiva on leikattava kun toinen arvoista menee akselin yli
void NFmiTimeSerialView::DrawDataLineOpt(double xpos1,  double xpos2
										,double value1, double value2, NFmiDrawingEnvironment & envi
										,const NFmiPoint& thePointSize, const NFmiPoint& theSinglePointSize, bool fUseValueAxis)
{
	if(value1 == kFloatMissing && value2 == kFloatMissing) // kumpikaan ei ole piirrett‰viss‰, ei piirret‰ sitten ollenkaan!
		return;

	if(value1 == kFloatMissing) // vain 2. pisteell‰ on arvo, piirret‰‰n sitten piste!
	{
		value1 = value2;
		xpos1 = xpos2;
	}
	if(value2 == kFloatMissing) // vain 1. pisteell‰ on arvo, piirret‰‰n sitten piste!
	{
		value2 = value1;
		xpos2 = xpos1;
	}
	bool isInAxis1 = fUseValueAxis ? ValueInsideValueAxis(value1) : ValueInsideModifyFactorAxis(value1)
		,isInAxis2 = fUseValueAxis ? ValueInsideValueAxis(value2) : ValueInsideModifyFactorAxis(value2);

	NFmiPoint point1 = fUseValueAxis ? NFmiPoint(xpos1, Value2AxisPosition((float)value1)) : NFmiPoint(xpos1, ModifyFactor2AxisPosition((float)value1));
	NFmiPoint point2 = fUseValueAxis ? NFmiPoint(xpos2, Value2AxisPosition((float)value2)) : NFmiPoint(xpos2, ModifyFactor2AxisPosition((float)value2));
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
		itsDrawingEnvironment->SetFrameColor(CtrlViewUtils::GetParamTextColor(itsDrawParam, itsCtrlViewDocumentInterface));

		itsDrawingEnvironment->SetFontSize(CalcFontSize());

		NFmiString str = CtrlViewUtils::GetParamNameString(itsDrawParam, itsCtrlViewDocumentInterface, ::GetDictionaryString("MapViewToolTipOrigTimeNormal"), ::GetDictionaryString("MapViewToolTipOrigTimeMinute"), false, false, false);

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
			str += NFmiString(")");
		}

		NFmiPoint place(CalcParamTextPosition());
		NFmiText text(place, str, 0, itsDrawingEnvironment);
		FmiDirection oldDir = itsToolBox->GetTextAlignment();
		itsToolBox->SetTextAlignment(kTop);
		itsToolBox->Convert(&text);
		itsToolBox->SetTextAlignment(oldDir);
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
		NFmiText text(place, str, 0, itsDrawingEnvironment);
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
	if(fJustScanningForMinMaxValues)
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
	double value1, value2, modifiedValue1,modifiedValue2, realValue1, realValue2;
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
	double maskFactor1 = paramMaskList->MaskValue(Info()->LatLon()),
		   maskFactor2 = 0;
	realValue1 = value1 = value2 = Info()->FloatValue() * Info()->TimeResolution() / 60.; // pit‰‰ kertoa tuntim‰‰r‰ll‰, koska nyky‰‰n vain 1h sateita datassa
	modifiedValue1 = modifiedValue2 = CalcModifiedValue(realValue1, 0, maskFactor1);
	bool fDrawChangeLines2 = value1 != modifiedValue1;
	NFmiPoint pointSize(4,4);
	long timeCount = Info()->SizeTimes();
	for(long i=1; i < timeCount; i++) // HUOM! ei piirr‰ jos vain yksi aika.
	{
		Info()->NextTime();
		time2 = Info()->Time();

		paramMaskList->SyncronizeMaskTime(time2);
		maskFactor2 = paramMaskList->MaskValue(Info()->LatLon());

		value2 += Info()->FloatValue() * Info()->TimeResolution() / 60.; // pit‰‰ kertoa tuntim‰‰r‰ll‰, koska nyky‰‰n vain 1h sateita datassa
		realValue2 = Info()->FloatValue() * Info()->TimeResolution() / 60.; // pit‰‰ kertoa tuntim‰‰r‰ll‰, koska nyky‰‰n vain 1h sateita datassa
		double modValue = CalcModifiedValue(realValue2, i, maskFactor2);
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
	NFmiText text(place, str, 0, &itsIncrementalCurveEnvi);
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
	if(fJustScanningForMinMaxValues)
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
    if(theDrawedLocationCounter == 1) // vain 1. lokaatiolle piirret‰‰n helper-data
    {
        DrawHelperDataLocationInTime(theLatlon);
    }
    itsNormalCurveEnvi.SetFrameColor(itsCtrlViewDocumentInterface->GeneralColor(theDrawedLocationCounter - 1));
    DrawLocationInTime(theLatlon, itsNormalCurveEnvi, itsChangeCurveEnvi, true);
    if(theDrawedLocationCounter == 1) // vain 1. lokaatiolle piirret‰‰n havainto-data
    {
        // Piirret‰‰n mahdolliset apu havainnot viimeiseksi, jotta erilaiset parvet eiv‰t peitt‰isi niit‰ (t‰st‰ tulee aina vain yksi k‰yr‰, joten se ei peit‰ paljoa)
        DrawHelperObservationData(theLatlon);
    }
    DrawStationDataStationNameLegend(theViewedInfo, theLatlon, theDrawedLocationCounter++, itsNormalCurveEnvi);
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
				if(itsCtrlViewDocumentInterface->OutOfEditedAreaTimeSerialPoint() != NFmiPoint::gMissingLatlon)
				{
                    auto &latlon = itsCtrlViewDocumentInterface->OutOfEditedAreaTimeSerialPoint();
        			DrawHelperDataLocationInTime(latlon);
                    // Piirret‰‰n mahdolliset apu havainnot viimeiseksi, jotta erilaiset parvet eiv‰t peitt‰isi niit‰ (t‰st‰ tulee aina vain yksi k‰yr‰, joten se ei peit‰ paljoa)
                    DrawHelperObservationData(latlon);
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
        NFmiText text(cpDrawingOptions.textPoint, indexStr, 0, &cpDrawingOptions.currentDataEnvi);
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
NFmiPoint NFmiTimeSerialView::CalcModifiedValuePoint(double theRealValue, long theIndex, double theMaskFactor)
{
	return CalcRelativeValuePosition(Info()->Time(), CalcModifiedValue(theRealValue, theIndex, theMaskFactor));
}

//--------------------------------------------------------
// CalcModifiedValue
//--------------------------------------------------------
double NFmiTimeSerialView::CalcModifiedValue(double theRealValue, long theIndex, double theMaskFactor)
{
	if(theRealValue == kFloatMissing || theMaskFactor == kFloatMissing)
		return kFloatMissing;
	double returnValue = kFloatMissing;
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
	if(fJustScanningForMinMaxValues)
		return ;
	itsToolBox->UseClipping(true);
	if(itsCtrlViewDocumentInterface->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal) // jos ns. edit-moodi p‰‰ll‰, piiret‰‰n aikarajoitin viivat
	{
		if(!IsAnalyzeRelatedToolUsed()) // muutos k‰yr‰t piirret‰‰n vain ei-analyysi tilassa
		{
			DrawModifyFactorPointGrids();
			if(itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode())
			{
				checkedVector<double>& modFacVec = itsCtrlViewDocumentInterface->CPManager()->ActiveCPChangeValues();
				itsModificationFactorCurvePoints = modFacVec;
			}

			switch (itsEditingMode)
			{
			case kFmiTimeEditSinCurve:
				DrawModifyFactorPointsSin();
				break;
			case kFmiTimeEditLinear:
				DrawModifyFactorPointsLinear();
				break;
			case kFmiTimeEditManual:
				DrawModifyFactorPointsManual();
				break;
			default:
				break;
			}
		}
	}
}

//--------------------------------------------------------
// DrawModifyFactorPointsSin
//--------------------------------------------------------
void NFmiTimeSerialView::DrawModifyFactorPointsSin(void)
{
	NFmiDrawingEnvironment envi;
	envi.DisableFill();
	envi.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	NFmiPoint point1,point2;
	NFmiTimeDescriptor timeDescriptor = EditedDataTimeDescriptor();
	timeDescriptor.Reset();
	timeDescriptor.Next();
	NFmiMetTime time = timeDescriptor.Time();
	point1 = CalcRelativeModifyFactorPosition(time,itsModificationFactorCurvePoints[0]);
	for(unsigned long i=1; i < itsModificationFactorCurvePoints.size(); i++) // HUOM! ei piirr‰ jos vain yksi data.
	{
		timeDescriptor.Next();
		time = timeDescriptor.Time();
		point2 = CalcRelativeModifyFactorPosition(time, itsModificationFactorCurvePoints[i]);
		// tehd‰‰n pointeista kopiot, koska ne voivat muuttua
		NFmiPoint p1(point1);
		NFmiPoint p2(point2);
		DrawLineInDataRect(p1, p2, envi);
		point1 = point2;
	}
}

//--------------------------------------------------------
// DrawModifyFactorPointsLinear
//--------------------------------------------------------
void NFmiTimeSerialView::DrawModifyFactorPointsLinear(void)
{
	if(itsModificationFactorCurvePoints.size() > 0)
	{
		NFmiDrawingEnvironment envi;
		envi.SetFillColor(NFmiColor(1.f,0.f,0.f));
		envi.SetFrameColor(NFmiColor(0.f,0.f,0.f));
		envi.EnableFill();
		NFmiPoint pointSize(5.,5.);

		NFmiTimeBag timeBag = EditedDataTimeBag();
		NFmiPoint point1 = CalcRelativeModifyFactorPosition(timeBag.FirstTime(),itsModificationFactorCurvePoints[0]);
		NFmiPoint point2 = CalcRelativeModifyFactorPosition(timeBag.LastTime(), itsModificationFactorCurvePoints[itsModificationFactorCurvePoints.size()-1]);

		DrawLineInDataRect(point1,point2, envi);
		DrawPointInDataRect(envi, point1, pointSize);
		DrawPointInDataRect(envi, point2, pointSize);
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
	checkedVector<NFmiMetTime> times;
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

			    double proximityFactor = CalcMouseClickProximityFactor();//0.02;
			    if(IsAnalyzeRelatedToolUsed())
			    {
				    NFmiMetTime analyzeEndTime(Value2Time(thePlace));
                    itsCtrlViewDocumentInterface->AnalyzeToolData().AnalyzeToolEndTime(analyzeEndTime);
				    return true;
			    }
			    else
			    {
				    if(itsEditingMode == kFmiTimeEditLinear)
					    proximityFactor = 0.2;
				    int index;
				    if(FindTimeIndex(thePlace.X(), proximityFactor, index))
				    {
					    double value = kFloatMissing;
					    if(itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode() && itsEditingMode == kFmiTimeEditManual)
					    { // 13.11.2002/Marko Muutos CP-tyˆkalun k‰ytˆkseen siten, ett‰ piirret‰‰n lopullista arvok‰yr‰‰ haluttuun pisteeseen.
						    value = Position2Value(thePlace);
					    }
					    else
					    {
						    value = Position2ModifyFactor(thePlace);
					    FixModifyFactorValue(value);
					    }
					    bool status = false;
					    switch (itsEditingMode)
					    {
					    case kFmiTimeEditSinCurve:
						    status = ModifyFactorPointsSin(value, index, thePlace);
						    break;
					    case kFmiTimeEditLinear:
						    status = ModifyFactorPointsLinear(value, index);
						    break;
					    case kFmiTimeEditManual:
						    status = ModifyFactorPointsManual(value, index);
						    break;
					    }

					    if(itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode())
					    {
                            itsCtrlViewDocumentInterface->CPManager()->Param(itsDrawParam->Param());
						    checkedVector<double>& modFacVec = itsCtrlViewDocumentInterface->CPManager()->ActiveCPChangeValues();
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

bool NFmiTimeSerialView::ScanDataForMinAndMaxValues(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiPoint &theLatlon, const NFmiTimeBag &theLimitingTimes, NFmiDataModifierMinMax &theAutoAdjustMinMaxValuesOut)
{
//	if(theInfo->Location(theLatlon))
	{
		bool interpolateLocation = (theInfo->Grid() != 0);
		theInfo->FindNearestTime(theLimitingTimes.FirstTime(), kBackward);
		int timeIndex1 = theInfo->TimeIndex();
		if(timeIndex1 == -1)
		{
			if(theLimitingTimes.IsInside(theInfo->TimeDescriptor().FirstTime()))
				timeIndex1 = 0;
		}
		theInfo->FindNearestTime(theLimitingTimes.LastTime(), kForward);
		int timeIndex2 = theInfo->TimeIndex();
		if(timeIndex2 == -1)
		{
			if(theLimitingTimes.IsInside(theInfo->TimeDescriptor().LastTime()))
				timeIndex2 = static_cast<int>(theInfo->SizeTimes()) - 1;
		}

		for(int i = timeIndex1; i <= timeIndex2; i++)
		{
			if(theInfo->TimeIndex(i))
			{
				float value = interpolateLocation ? theInfo->InterpolatedValue(theLatlon) : theInfo->FloatValue();
				theAutoAdjustMinMaxValuesOut.Calculate(value);
			}
		}
		return true;
	}
	return false;
}

checkedVector<NFmiPoint> NFmiTimeSerialView::GetViewedLatlonPoints(void)
{
	checkedVector<NFmiPoint> latlons;
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
	int powPlus = 0;
	for(; step < 1;)
	{
		step *= 10.f;
		powPlus++;
	}
	int powMinus = 0;
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
		step = step * ::pow(10.f, powMinus);
	if(powPlus)
		step = step / ::pow(10.f, powPlus);

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
	fJustScanningForMinMaxValues = true;
	itsAutoAdjustMinMaxValues.Clear();
	itsScannedLatlonPoints = GetViewedLatlonPoints();
	itsAutoAdjustScanTimes = ::GetScannedTimes(GetViewLimitingTimes());
	itsInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, false, true);
	DrawSelectedStationData(); // skannataan piirto-systeemi l‰pi ilman piirtoa etsien min/max arvoja eri datoista
	itsToolBox->UseClipping(false);
	itsDrawingEnvironment->EnableFill();
	
	fJustScanningForMinMaxValues = false;

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
			newAxisStartValue = std::max(newAxisStartValue, static_cast<float>(itsDrawParam->AbsoluteMinValue()));
			newAxisEndValue = std::min(newAxisEndValue, static_cast<float>(itsDrawParam->AbsoluteMaxValue()));
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
			    double proximityFactor = CalcMouseClickProximityFactor();//0.02;
			    if(IsAnalyzeRelatedToolUsed())
			    {
				    NFmiMetTime analyzeEndTime(Value2Time(thePlace));
                    itsCtrlViewDocumentInterface->AnalyzeToolData().AnalyzeToolEndTime(analyzeEndTime);
				    return true;
			    }
			    else
			    {
				    if(itsEditingMode == kFmiTimeEditLinear)
					    proximityFactor = 0.2;
				    int index;
				    if(FindTimeIndex(thePlace.X(), proximityFactor, index))
				    {
					    double value = kFloatMissing;
					    if(itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode() && itsEditingMode == kFmiTimeEditManual)
					    { // 13.11.2002/Marko Muutos CP-tyˆkalun k‰ytˆkseen siten, ett‰ piirret‰‰n lopullista arvok‰yr‰‰ haluttuun pisteeseen.
						    value = Position2Value(thePlace);
					    }
					    else
					    {
						    value = Position2ModifyFactor(thePlace);
					    FixModifyFactorValue(value);
					    }

					    bool status = false;
					    switch (itsEditingMode)
					    {
					    case kFmiTimeEditSinCurve:
						    status = ModifyFactorPointsSin(value, index, thePlace);
						    break;
					    case kFmiTimeEditLinear:
						    status = ModifyFactorPointsLinear(value, index);
						    break;
					    case kFmiTimeEditManual:
						    status = ModifyFactorPointsManual(value, index);
						    break;
					    }

					    if(itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode())
					    {
                            itsCtrlViewDocumentInterface->CPManager()->Param(itsDrawParam->Param());
						    checkedVector<double>& modFacVec = itsCtrlViewDocumentInterface->CPManager()->ActiveCPChangeValues();
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
// ModifyFactorPointsSin
//--------------------------------------------------------
bool NFmiTimeSerialView::ModifyFactorPointsSin(double theValue, int /* theIndex */, const NFmiPoint&  thePlace)
{
	itsSinAmplitude = theValue;
	NFmiMetTime time2 = Value2Time(thePlace);
	itsPhase = time2.GetHour()-6; // 6 tarkoittaa vaihesiirtoa, mik‰ tarvitaan, ett‰ klikattu kohta saadaan maksimiksi (vahinko, ettei voi piirt‰‰ kuvaa t‰h‰n)
	CalcSinModifyFactorPoints();
	return true;
}

//--------------------------------------------------------
// ModifyFactorPointsLinear
//--------------------------------------------------------
bool NFmiTimeSerialView::ModifyFactorPointsLinear(double theValue, int theIndex)
{
	if(theIndex == 0 || theIndex == itsModificationFactorCurvePoints.size()-1)
	{
		itsModificationFactorCurvePoints[theIndex] = theValue;
		CalcLinearModifyFactorPoints();
		return true;
	}

	if(theIndex < static_cast<int>(itsModificationFactorCurvePoints.size() - theIndex))
		itsModificationFactorCurvePoints[0] = theValue;
	else
		itsModificationFactorCurvePoints[itsModificationFactorCurvePoints.size()-1] = theValue;
	CalcLinearModifyFactorPoints();
	return true;
}

//--------------------------------------------------------
// ModifyFactorPointsManual
//--------------------------------------------------------
bool NFmiTimeSerialView::ModifyFactorPointsManual(double theValue, int theIndex)
{
    const double toPowerFactor = 2.3; // t‰m‰n avulla s‰‰det‰‰n hiiren klikkauksen muokkaus vaikutusalueen 'k‰rjen' ter‰vyyteen aikasarjaikkunassa editoitaessa (suurempi arvo -> laakeampi muutosk‰yr‰ klikatun kohdan ymp‰rill‰)
    double manualModifierFactor = pow(itsManualModifierLength, 0.2); // t‰ss‰ s‰‰det‰‰n vaikutusalueen leveytt‰ ensin toisella kertoimella (pienempi arvo -> leve‰mpi vaikutus)
    if(itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode())
	{ // 13.11.2002/Marko Muutos CP-tyˆkalun k‰ytˆkseen siten, ett‰ piirret‰‰n lopullista arvok‰yr‰‰ haluttuun pisteeseen.
		NFmiPoint latlon(itsCtrlViewDocumentInterface->CPManager()->ActiveCPLatLon());
		Info()->TimeIndex(theIndex);
		float value = kFloatMissing;
		for(int i = 0; i < static_cast<int>(itsModificationFactorCurvePoints.size()); i++)
		{
			Info()->TimeIndex(i);
			value = Info()->InterpolatedValue(latlon);
			double timeFactor = FmiMax(0., FmiMin(1., double(::abs(i - theIndex)) * (1-manualModifierFactor)));
            timeFactor = 1. - fabs(pow(timeFactor, toPowerFactor));
			itsModificationFactorCurvePoints[i] =
					(theValue - value) * timeFactor +
					itsModificationFactorCurvePoints[i]  * (1 - timeFactor);
		}
	}
	else
	{
		for(int i = 0; i < static_cast<int>(itsModificationFactorCurvePoints.size()); i++)
		{
			double timeFactor = FmiMax(0., FmiMin(1., double(abs(i - theIndex)) * (1-manualModifierFactor)));
            timeFactor = 1. - fabs(pow(timeFactor, toPowerFactor));
			itsModificationFactorCurvePoints[i] =
				(theValue - itsModificationFactorCurvePoints[i]) * timeFactor +
				itsModificationFactorCurvePoints[i] ;
		}
	}
	return true;
}

//--------------------------------------------------------
// FixModifyFactorValue
//--------------------------------------------------------
void NFmiTimeSerialView::FixModifyFactorValue(double& theValue)
{
	double limit = itsDrawParam->TimeSerialModifyingLimit();
	if(theValue < -limit)
		theValue = -limit;
	if(theValue > limit)
		theValue = limit;
}

//--------------------------------------------------------
// CalcLinearModifyFactorPoints
//--------------------------------------------------------
void NFmiTimeSerialView::CalcLinearModifyFactorPoints(void)
{
	double low = itsModificationFactorCurvePoints[0];
	double high = itsModificationFactorCurvePoints[itsModificationFactorCurvePoints.size()-1];
	for(int i = 1; i < static_cast<int>(itsModificationFactorCurvePoints.size())-1; i++)
		itsModificationFactorCurvePoints[i] = (high - low) * i/float(itsModificationFactorCurvePoints.size()) + low;
}

//--------------------------------------------------------
// CalcSinModifyFactorPoints
//--------------------------------------------------------
void NFmiTimeSerialView::CalcSinModifyFactorPoints(void)
{
	int i = 0;
	for(itsEditedDataTimeDescriptor.Reset(); itsEditedDataTimeDescriptor.Next(); i++)
	{
		double hour = itsEditedDataTimeDescriptor.Time().GetHour();
		double temp = ((hour - itsPhase)/24*2*kPii);
		itsModificationFactorCurvePoints[i] = sin(temp)*itsSinAmplitude;
	}
}

//--------------------------------------------------------
// CalcModifyFactorPoints
//--------------------------------------------------------
void NFmiTimeSerialView::CalcModifyFactorPoints(void)
{
		switch (itsEditingMode)
		{
		case kFmiTimeEditLinear:
			CalcLinearModifyFactorPoints();
			break;
		case kFmiTimeEditSinCurve: // not implemented yet
			CalcSinModifyFactorPoints();
			break;
		case kFmiTimeEditManual: // nothing to do here
		default:
			break;
		}
}

//--------------------------------------------------------
// EditingMode
//--------------------------------------------------------
void NFmiTimeSerialView::EditingMode(int newMode)
{
	itsEditingMode = FmiTimeEditMode(newMode);
	CalcModifyFactorPoints();
}

//--------------------------------------------------------
// MaxStationShowed
//--------------------------------------------------------
void NFmiTimeSerialView::MaxStationShowed(unsigned int newCount)
{
	itsMaxStationShowed = newCount;
}

void NFmiTimeSerialView::ManualModifierLength(double newValue)
{
	itsManualModifierLength = newValue;
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
// CalcSuitableLowAndHighLimits
//--------------------------------------------------------
void NFmiTimeSerialView::CalcSuitableLowAndHighLimits(double& low, double& high, double step)
{
	low = LowerScaleValueRound(low,step);
	high = HigherScaleValueRound(high,step);
}

//--------------------------------------------------------
// LowerScaleValueRound
//--------------------------------------------------------
double NFmiTimeSerialView::LowerScaleValueRound(double value, double step)
{
	double returnValue = value/step;
	returnValue = floor(returnValue);
	returnValue *= step;
	return returnValue;
}

//--------------------------------------------------------
// HigherScaleValueRound
//--------------------------------------------------------
double NFmiTimeSerialView::HigherScaleValueRound(double value, double step)
{
	double returnValue = value/step;
	returnValue = ceil(returnValue);
	returnValue *= step;
	return returnValue;
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
	float value = itsValueAxis->Location(theValue);
	NFmiRect rect(CalcValueAxisRect());
	double finalValue = rect.Bottom() - value * rect.Height(); // HUOMM!! toimii vain jos value on y-akselilla ja alkaa alhaalta ylˆs!!!
	return finalValue;
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
double NFmiTimeSerialView::Position2Value(const NFmiPoint& thePos)
{
	if(itsValueView)
		return itsValueView->Value(thePos);
	else
		return 0.;
}

//--------------------------------------------------------
// Position2ModifyFactor
//--------------------------------------------------------
double NFmiTimeSerialView::Position2ModifyFactor(const NFmiPoint& thePos)
{
	if(itsModifyFactorView)
		return itsModifyFactorView->Value(thePos);
	else
		return 0.;
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
                itsCtrlViewDocumentInterface->UpdateModifiedDrawParamMarko(itsDrawParam);
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

	for(int i = 0; i < 50; i++)//50 = Ei tehd‰ ikiluuppia
	{
		if(itsDrawParam)
		{
			double step = ::GetSuitableValueScaleModifyingStep(itsDrawParam);
			double scaleMax = itsDrawParam->TimeSeriesScaleMax();
			double scaleMin = itsDrawParam->TimeSeriesScaleMin();
			if(((scaleMax - scaleMin) > step*3) || theChangeDirectionFactor > 0.)
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
			itsCtrlViewDocumentInterface->UpdateModifiedDrawParamMarko(itsDrawParam);
		   return true;
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
	if(fJustScanningForMinMaxValues)
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
	if(fJustScanningForMinMaxValues)
		return ;
	NFmiDrawingEnvironment envi(ChangeStationDataCurveEnvironment());
	envi.SetFrameColor(NFmiColor(0.91f,0.34f,0.34f));
	envi.SetPenSize(NFmiPoint(3, 3));
	NFmiRect valueRect(CalcValueAxisRect());
	double xPos = Time2Value(itsCtrlViewDocumentInterface->AnalyzeToolData().AnalyzeToolEndTime());
	NFmiLine line(NFmiPoint(xPos, valueRect.Top()), NFmiPoint(xPos, valueRect.Bottom()), 0, &envi);
	itsToolBox->Convert(&line);
}

bool NFmiTimeSerialView::SetAnalyzeToolRelatedInfoToLastSuitableTime(boost::shared_ptr<NFmiFastQueryInfo> &analyzeDataInfo, bool isObservationData)
{
    if(!isObservationData)
    {
        analyzeDataInfo->LastTime();
        return true;
    }
    else
    {
        try
        {
            checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> analyzeToolInfos = GetObsBlenderDataVector();
            auto usedAreaPtr = NFmiAnalyzeToolData::GetUsedAreaForAnalyzeTool(itsCtrlViewDocumentInterface->GenDocDataAdapter(), Info());
            auto useObservationBlenderTool = itsCtrlViewDocumentInterface->AnalyzeToolData().ControlPointObservationBlendingData().UseBlendingTool();
            std::string producerName = itsCtrlViewDocumentInterface->AnalyzeToolData().ControlPointObservationBlendingData().SelectedProducer().GetName();
            NFmiMetTime actualFirstTime, usedFirstTime;
            std::tie(actualFirstTime, usedFirstTime) = NFmiAnalyzeToolData::GetLatestSuitableAnalyzeToolInfoTime(analyzeToolInfos, Info(), usedAreaPtr, useObservationBlenderTool, producerName);
            if(usedFirstTime > analyzeDataInfo->TimeDescriptor().LastTime())
                return analyzeDataInfo->Time(actualFirstTime);
            else
                return analyzeDataInfo->Time(usedFirstTime);
        }
        catch(std::exception &)
        {
            analyzeDataInfo->ResetTime();
            return false;
        }
    }
}

// piirt‰‰ katkoviivalla k‰yr‰n, joka kuvaa valitun pisteen kohdalla tapahtuvaa muutosta.
// HUOM!!! Piirt‰‰ myˆs analyysidatan ruutuun!!!!!
void NFmiTimeSerialView::DrawAnalyzeToolChangeLine(const NFmiPoint &theLatLonPoint)
{
    if(fJustScanningForMinMaxValues)
        return;
    NFmiDrawingEnvironment envi;

    auto useObservationBlender = itsCtrlViewDocumentInterface->AnalyzeToolData().ControlPointObservationBlendingData().UseBlendingTool();
    std::vector<std::string> messages;
    boost::shared_ptr<NFmiFastQueryInfo> analyzeDataInfo = GetMostSuitableAnalyzeToolRelatedData(theLatLonPoint);
    if(analyzeDataInfo)
    {
        bool isObservationData = !analyzeDataInfo->IsGrid();
        DrawAnalyzeToolDataLocationInTime(theLatLonPoint, envi, analyzeDataInfo); // piirret‰‰n ensin analyysi data ja sitten sen aiheuttama muutosk‰yr‰
        analyzeDataInfo->FirstLevel(); // varmuuden vuoksi asetan 1. leveliin
        auto editedInfo = Info();
        if(analyzeDataInfo->Param(*editedInfo->Param().GetParam())) // parametrikin pit‰‰ asettaa kohdalleen
        {
            NFmiMetTime startTime(analyzeDataInfo->TimeDescriptor().LastTime());
            NFmiMetTime endTime(itsCtrlViewDocumentInterface->AnalyzeToolData().AnalyzeToolEndTime());
            if(startTime < endTime)
            {
                if(SetAnalyzeToolRelatedInfoToLastSuitableTime(analyzeDataInfo, isObservationData))
                {
                    // Havainto datan tapauksessa etsit‰‰n l‰hin aika, mutta jos hila-analyysidatan aikaa ei lˆydy editoitavasta datasta, ei kannata jatkaa
                    if(isObservationData ? editedInfo->FindNearestTime(startTime) : editedInfo->Time(startTime))
                    {
                        if(isObservationData)
                            startTime = editedInfo->Time(); // Asetetaan havaintoaikaa l‰hin editoitava aika alkuajaksi (havainnoissa voi olla pienempi aika-askel, ja se pit‰‰ sovittaa editoituun dataan)
                        // Havaintodatalle ei tehd‰ paikka interpolaatiota
                        auto analyzeValue = isObservationData ? analyzeDataInfo->FloatValue() : analyzeDataInfo->InterpolatedValue(theLatLonPoint);
                        auto firstEditDataValue = editedInfo->FloatValue();
                        if(analyzeValue != kFloatMissing && firstEditDataValue != kFloatMissing)
                        {
                            NFmiLimitChecker limitChecker(static_cast<float>(itsDrawParam->AbsoluteMinValue()), static_cast<float>(itsDrawParam->AbsoluteMaxValue()), static_cast<FmiParameterName>(itsInfo->Param().GetParamIdent()));
                            auto maskList = NFmiAnalyzeToolData::GetUsedTimeSerialMaskList(itsCtrlViewDocumentInterface->GenDocDataAdapter());
                            bool useMask = maskList->UseMask();
                            auto changeValue = firstEditDataValue - analyzeValue;
                            auto helpText = "Selected point change value: "s + std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(changeValue, 2));
                            messages.push_back(helpText);
                            NFmiTimeBag times(startTime, endTime, editedInfo->TimeDescriptor().Resolution());
                            auto timeSize = times.GetSize();
                            auto timeIndex = 0;
                            NFmiDrawingEnvironment envi2(ChangeStationDataCurveEnvironment());
                            envi2.SetFrameColor(NFmiColor(0.98f, 0.134f, 0.14f));
                            envi2.SetPenSize(NFmiPoint(2, 2));
                            checkedVector<pair<double, NFmiMetTime> > dataVector;
                            auto maskHelpText = "Starting mask factor: "s;
                            for(times.Reset(); times.Next(); timeIndex++)
                            {
                                auto editDataValue = kFloatMissing;
                                float maskFactor = 1.f;
                                if(editedInfo->Time(times.CurrentTime()))
                                {
                                    editDataValue = editedInfo->FloatValue();
                                    if(useMask)
                                    {
                                        maskList->SyncronizeMaskTime(editedInfo->Time());
                                        maskFactor = static_cast<float>(maskList->MaskValue(editedInfo->LatLonFast()));
                                    }
                                }
                                if(timeIndex == 0)
                                    maskHelpText += NFmiValueString::GetStringWithMaxDecimalsSmartWay(maskFactor, 1);
                                auto modifiedValue = NFmiControlPointObservationBlendingData::BlendData(editDataValue, changeValue, maskFactor, timeSize, timeIndex, limitChecker);
                                dataVector.push_back(std::make_pair(modifiedValue, times.CurrentTime()));
                            }
                            messages.push_back(maskHelpText);
                            DrawSimpleDataVectorInTimeSerial(dataVector, envi2, NFmiPoint(2, 2), NFmiPoint(6, 6));
                        }
                        else
                        {
                            if(useObservationBlender)
                                messages.push_back("Selected point is zero change CP"s);
                            else
                                messages.push_back("Missing value in analyze data"s);
                        }
                    }
                    else
                        messages.push_back("Suitable start time not in edited data"s);
                }
                else
                    messages.push_back("No suitable time for obs-blender"s);
            }
            else
                messages.push_back("Start/end editing time range illegal"s);
        }
        else
            messages.push_back("No param in selected data"s);
    }
    else
    {
        if(useObservationBlender)
            messages.push_back("No suitable obs-blender data"s);
        else
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
        NFmiText text(textPoint, message.c_str(), 0, &envi);
        itsToolBox->Convert(&text);
        textPoint.Y(textPoint.Y() + heightInc);
    }
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiTimeSerialView::GetMostSuitableAnalyzeToolRelatedData(const NFmiPoint &theLatLonPoint)
{
    if(itsCtrlViewDocumentInterface->AnalyzeToolData().ControlPointObservationBlendingData().UseBlendingTool())
        return GetMostSuitableObsBlenderData(theLatLonPoint);
    else
        return itsCtrlViewDocumentInterface->InfoOrganizer()->Info(NFmiDataIdent(*(Info()->Param().GetParam()), itsCtrlViewDocumentInterface->AnalyzeToolData().SelectedProducer1()), 0, NFmiInfoData::kAnalyzeData);
}

static checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> GetInfosWithWantedParam(checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> &infoVectorIn, FmiParameterName wantedParamId)
{
    checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> infoVectorResult;
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

boost::shared_ptr<NFmiFastQueryInfo> NFmiTimeSerialView::GetMostSuitableObsBlenderData(const NFmiPoint &theLatLonPoint)
{
    auto infosWithParamVector = GetObsBlenderDataVector();
    // Haetaan data, jossa on l‰hin asema, jonka pit‰‰ olla myˆs m‰‰r‰tyn hakus‰teen sis‰ll‰
    auto maxDistanceInMeters = NFmiControlPointObservationBlendingData::MaxAllowedDistanceToStationInKm() * 1000.;
    return itsCtrlViewDocumentInterface->GetNearestSynopStationInfo(NFmiLocation(theLatLonPoint), NFmiMetTime::gMissingTime, true, &infosWithParamVector, maxDistanceInMeters);
}

checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> NFmiTimeSerialView::GetObsBlenderDataVector()
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
		if(analyzeDataInfo->Param(*Info()->Param().GetParam())) // parametrikin pit‰‰ asettaa kohdalleen
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
				DrawSimpleDataInTimeSerial(drawedTimes, analyzeDataInfo, envi, theLatLonPoint, 0, NFmiPoint(9, 9)); // 0=ei siirret‰ aikasarjaa mihink‰‰n suuntaa piirrossa
//				DrawSimpleDataInTimeSerial(*analyzeDataInfo, envi, 0, NFmiPoint(9, 9)); // 0=ei siirret‰ aikasarjaa mihink‰‰n suuntaa piirrossa
				envi.SetPenSize(NFmiPoint(1, 1)); // ohut viiva takaisin
			}
		}
	}
}

void NFmiTimeSerialView::DrawSimpleDataVectorInTimeSerial(checkedVector<pair<double, NFmiMetTime> > &theDataVector, NFmiDrawingEnvironment &theEnvi, const NFmiPoint& thePenSize, const NFmiPoint& theSinglePointSize)
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
void NFmiTimeSerialView::PlotTimeSerialData(const checkedVector<double> &theValues, const checkedVector<NFmiMetTime> &theTimes, NFmiDrawingEnvironment &theEnvi, const NFmiPoint& thePointSize, const NFmiPoint& theSinglePointSize, bool fUseValueAxis, bool drawConnectingLines)
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
		for(int i=1; (i < valuesSize) && (i < timesSize); i++) // i < timesSize est‰‰ kaatumisen jos values ja times vectorit erikokoisia, esim. jos deletoidaan CP-piste, palautetaan default checkedVector, jonka koko on 200!!!
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

// t‰m‰ funktio on timebag optimoitu!
// funktio piirt‰‰ annetun arvojoukon annettuina aikoina n‰yttˆˆn halutuilla piirtooptioilla
void NFmiTimeSerialView::PlotTimeSerialDataOpt(const checkedVector<double> &theValues, double xPosStart, double xStep, NFmiDrawingEnvironment &theEnvi, const NFmiPoint& thePointSize, const NFmiPoint& theSinglePointSize, bool fUseValueAxis, bool drawConnectingLines)
{
	int valuesSize = static_cast<int>(theValues.size());
	if(valuesSize == 1)
	{
		NFmiPoint onlyPointSize(20.,6.);
		DrawDataLineOpt(xPosStart, xPosStart, theValues[0], kFloatMissing, theEnvi, onlyPointSize, onlyPointSize, fUseValueAxis);
	}
	else if(valuesSize > 1)
	{
		NFmiDrawingEnvironment blackLineEnvi;
		NFmiPoint missingValuePointSize(6,6);
		double lastNonMissingPosition = kFloatMissing;
		double lastNonMissingValue = kFloatMissing;
		double realValue1 = kFloatMissing; // t‰m‰ on indeksill‰ i-1 saatava arvo
		double realValue2 = kFloatMissing; // t‰m‰ on indeksill‰ i saatava arvo
		double currentPos = xPosStart;
		for(int i=1; i < valuesSize; i++, currentPos += xStep)
		{
			realValue1 = theValues[i-1];
			realValue2 = theValues[i];

			if(drawConnectingLines && realValue1 == kFloatMissing && realValue2 != kFloatMissing && lastNonMissingValue != kFloatMissing)
			{ // piirret‰‰n ohut musta yhteysviiv‰ katkonaisiin kohtiin, jos niin on s‰‰detty
				// ja piirret‰‰n se t‰p‰n alle, jokat tehd‰‰n kun on toinen arvoista on puuttuvaa.
				DrawDataLineOpt(lastNonMissingPosition, currentPos+xStep, lastNonMissingValue, realValue2, blackLineEnvi, missingValuePointSize, missingValuePointSize, true);
			}

			DrawDataLineOpt(currentPos, currentPos+xStep, theValues[i-1], theValues[i], theEnvi, thePointSize, theSinglePointSize, fUseValueAxis);

			if(realValue2 != kFloatMissing)
			{
				lastNonMissingValue = realValue2;
				lastNonMissingPosition = currentPos+xStep;
			}
		}
	}
}

NFmiTimeBag NFmiTimeSerialView::GetViewLimitingTimes(void)
{
	return itsCtrlViewDocumentInterface->TimeSerialViewTimeBag();
}

void NFmiTimeSerialView::FillTimeSerialDataFromInfo(NFmiFastQueryInfo &theSourceInfo, const NFmiPoint &theLatLonPoint, checkedVector<double> &theValues)
{
	// Onko t‰m‰ oikein????, linitingtimesit otetaan sourceInfosta ja niill‰ rajoitetaan FillTimeSerialDataFromInfo-metodissa
	// taas sourceInfosta haettavia aikoja.
	NFmiTimeBag limitingTimes(theSourceInfo.TimeDescriptor().FirstTime(), theSourceInfo.TimeDescriptor().LastTime(), 60); // resoluutiolla ei ole merkityst‰, 60 vain heitet‰‰n siihen
	FillTimeSerialDataFromInfo(theSourceInfo, theLatLonPoint, limitingTimes, theValues);
}

// HUOM! yritt‰‰ ottaa myˆs yhdet ajat ja arvot aikareunojen yli
void NFmiTimeSerialView::FillTimeSerialDataFromInfo(NFmiFastQueryInfo &theSourceInfo, const NFmiPoint &theLatLonPoint, const NFmiTimeBag &theLimitTimes, checkedVector<double> &theValues)
{
	if(theSourceInfo.TimeToNearestStep(theLimitTimes.FirstTime(), kBackward))
	{
		bool interpolateValues = theSourceInfo.IsGrid();
		NFmiLocation wantedLocation(theLatLonPoint);
		if(interpolateValues == false && (theSourceInfo.NearestLocation(wantedLocation, gMaxDistanceToFractileStation) == false))
			return ; // jos asema dataa ei lˆydy 500 km sis‰lt‰ haluttua pistett‰, ei k‰ytet‰ sit‰
		float value = interpolateValues ? theSourceInfo.InterpolatedValue(theLatLonPoint) : theSourceInfo.FloatValue();
		theValues.push_back(value);
		NFmiMetTime currentInfoTime;
		for( ; theSourceInfo.NextTime(); )
		{
			value = interpolateValues ? theSourceInfo.InterpolatedValue(theLatLonPoint) : theSourceInfo.FloatValue();
			theValues.push_back(value);
			if(!theLimitTimes.IsInside(theSourceInfo.Time()))
				break; // oletus, ett‰ ajat j‰rjestyksess‰ ja loopitus voidaan lopettaa
		}
	}
}

void NFmiTimeSerialView::FillTimeSerialTimesFromInfo(NFmiFastQueryInfo &theSourceInfo, checkedVector<NFmiMetTime> &theTimes)
{
	NFmiTimeBag limitingTimes(theSourceInfo.TimeDescriptor().FirstTime(), theSourceInfo.TimeDescriptor().LastTime(), 60); // resoluutiolla ei ole merkityst‰, 60 vain heitet‰‰n siihen
	FillTimeSerialTimesFromInfo(theSourceInfo, limitingTimes, theTimes);
}

void NFmiTimeSerialView::FillTimeSerialTimesFromInfo(NFmiFastQueryInfo &theSourceInfo, const NFmiTimeBag &theLimitTimes, checkedVector<NFmiMetTime> &theTimes)
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
void NFmiTimeSerialView::FillTimeSerialMaskValues(const checkedVector<NFmiMetTime> &theTimes, const NFmiPoint &theLatLonPoint, checkedVector<double> &theMaskValues)
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
					theMaskValues.push_back(paramMaskList->MaskValue(theLatLonPoint));
				}
			}
		}
	}
}

// t‰ytt‰‰ haluttuihin aikoihin muutos arvot, joilla piirret‰‰n muutos k‰yr‰.
void NFmiTimeSerialView::FillTimeSerialChangedValues(const checkedVector<double> &theValues, const checkedVector<double> &theMaskValues, checkedVector<double> &theChangedValues)
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
				checkedVector<double> values;
				itsDrawParam->ModelRunIndex(i);
				boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, false, false);
				if(info)
				{
					if(fJustScanningForMinMaxValues)
					{
						ScanDataForMinAndMaxValues(info, theLatLonPoint, itsAutoAdjustScanTimes, itsAutoAdjustMinMaxValues);
					}
					else
					{
						FillTimeSerialDataFromInfo(*info, theLatLonPoint, values);
						checkedVector<NFmiMetTime> times;
						FillTimeSerialTimesFromInfo(*info, times);

						PlotTimeSerialData(values, times, usedEnvi, pointNormal, pointSingle, true, true);
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

void NFmiTimeSerialView::CalcOptimizedDrawingValues(const NFmiTimeBag &theTimesIn, double &theXStartPosOut, double &theXStepOut)
{
	NFmiMetTime aTime(theTimesIn.FirstTime());
	theXStartPosOut = Time2Value(aTime);
	aTime.ChangeByMinutes(theTimesIn.Resolution());
	double xPos2 = Time2Value(aTime);
	theXStepOut = xPos2 - theXStartPosOut;
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

    if(fJustScanningForMinMaxValues)
    {
        ScanDataForMinAndMaxValues(Info(), theLatLonPoint, itsAutoAdjustScanTimes, itsAutoAdjustMinMaxValues);
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

void NFmiTimeSerialView::DrawEditedDataLocationInTime_ModificationLineOptimized(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theModifiedDataLineStyle, const checkedVector<NFmiMetTime> &theTimes, const checkedVector<double> &values)
{
    if(IsModificationLineDrawn())
    {
        double xStartPos = 0;
        double xStep = 0.1;
        CalcOptimizedDrawingValues(*(Info()->TimeDescriptor().ValidTimeBag()), xStartPos, xStep);
        checkedVector<double> maskValues;
        FillTimeSerialMaskValues(theTimes, theLatLonPoint, maskValues);
        checkedVector<double> changeValues;
        FillTimeSerialChangedValues(values, maskValues, changeValues);
        PlotTimeSerialDataOpt(changeValues, xStartPos, xStep, theModifiedDataLineStyle, g_PointNormal, g_PointSingle, true);
    }
}

void NFmiTimeSerialView::DrawEditedDataLocationInTime_ModificationLine(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theModifiedDataLineStyle, const checkedVector<NFmiMetTime> &theTimes, const checkedVector<double> &values)
{
    if(IsModificationLineDrawn())
    {
        checkedVector<double> maskValues;
        FillTimeSerialMaskValues(theTimes, theLatLonPoint, maskValues);
        checkedVector<double> changeValues;
        FillTimeSerialChangedValues(values, maskValues, changeValues);
        PlotTimeSerialData(changeValues, theTimes, theModifiedDataLineStyle, g_PointNormal, g_PointSingle, true);
    }
}

void NFmiTimeSerialView::DrawEditedDataLocationInTime_TimebagOptimized(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle, const checkedVector<NFmiMetTime> &theTimes, bool drawModificationLines)
{
    checkedVector<double> values;
    FillTimeSerialDataFromInfo(*Info(), theLatLonPoint, values);

    // timebagin tapauksessa (tasa askel aina) voidaan k‰ytt‰‰ optimoitua koodia
    double xStartPos = 0;
    double xStep = 0.1;
    CalcOptimizedDrawingValues(*(Info()->TimeDescriptor().ValidTimeBag()), xStartPos, xStep);
    if(drawModificationLines)
        DrawEditedDataLocationInTime_ModificationLineOptimized(theLatLonPoint, theModifiedDataLineStyle, theTimes, values);
    PlotTimeSerialDataOpt(values, xStartPos, xStep, theCurrentDataLineStyle, g_PointNormal, g_PointSingle, true, true);
}

void NFmiTimeSerialView::DrawEditedDataLocationInTime_Timelist(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle, const checkedVector<NFmiMetTime> &theTimes, bool drawModificationLines)
{
    checkedVector<double> values;
    FillTimeSerialDataFromInfo(*Info(), theLatLonPoint, values);
    if(drawModificationLines)
        DrawEditedDataLocationInTime_ModificationLine(theLatLonPoint, theModifiedDataLineStyle, theTimes, values);
    PlotTimeSerialData(values, theTimes, theCurrentDataLineStyle, g_PointNormal, g_PointSingle, true, true);
}

void NFmiTimeSerialView::DrawEditedDataLocationInTime(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle, bool drawModificationLines)
{
    if(DrawEditedDataLocationInTime_PreliminaryActions(theLatLonPoint, theCurrentDataLineStyle))
    {
        checkedVector<NFmiMetTime> times;
        FillTimeSerialTimesFromInfo(*Info(), times);

        // timebagin tapauksessa (tasa askel aina) voidaan k‰ytt‰‰ optimoitua koodia
        if(Info()->TimeDescriptor().ValidTimeBag())
        {
            DrawEditedDataLocationInTime_TimebagOptimized(theLatLonPoint, theCurrentDataLineStyle, theModifiedDataLineStyle, times, drawModificationLines);
        }
        else
        {
            DrawEditedDataLocationInTime_Timelist(theLatLonPoint, theCurrentDataLineStyle, theModifiedDataLineStyle, times, drawModificationLines);
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
	theStr += CtrlViewUtils::Color2HtmlColorStr(theTitleColor);
	theStr += ">";
	theStr += theTitle;
	theStr += "</font>";
	for(int i=0; i<theTabulatorCount; i++)
		theStr += "\t"; // tabulaattori
	theStr += "<b><font color=";
	NFmiColor tmpColor(0,0,1);
	theStr += CtrlViewUtils::Color2HtmlColorStr(tmpColor);
	theStr += ">";
	theStr += ::Value2ToolTipString(theValue, digitCount, interpMethod, parType);
	theStr += "</font></b>";
	if(fAddEndl)
		theStr += "\n";
}

static void AddProducerString(std::string &theStr, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool fAddEndl)
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

	if(fAddEndl)
		theStr += "\n";
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
		boost::shared_ptr<NFmiFastQueryInfo> modelInfo = ::GetWantedData(itsCtrlViewDocumentInterface, itsDrawParam, producers[i].GetProducer(), NFmiInfoData::kViewable);
		if(modelInfo)
		{
			float modelValue = modelInfo->InterpolatedValue(theLatlon, theTime);
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

std::string NFmiTimeSerialView::GetEcFraktileParamToolTipText(long theStartParamIndex, const std::string &theParName, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor, long theParamIndexIncrement)
{
	std::string str;
	boost::shared_ptr<NFmiFastQueryInfo> modelFractileInfo = itsCtrlViewDocumentInterface->GetFavoriteSurfaceModelFractileData();
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

std::string NFmiTimeSerialView::GetModelClimatologyParamToolTipText(const ModelClimatology::ParamMapItem &paramItem, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor)
{
    std::string str;
    boost::shared_ptr<NFmiFastQueryInfo> modelClimatologyInfo = itsCtrlViewDocumentInterface->GetModelClimatologyData();
    if(modelClimatologyInfo)
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
	if(theViewedInfo->SizeLevels() == 1 && itsCtrlViewDocumentInterface->GetFavoriteSurfaceModelFractileData() && itsCtrlViewDocumentInterface->ShowHelperData2InTimeSerialView())
	{
		if(itsDrawParam->Param().GetParamIdent() == kFmiTemperature)
			str += GetEcFraktileParamToolTipText(kFmiTemperatureF100, "T", theLatlon, theTime, theColor);
		else if(itsDrawParam->Param().GetParamIdent() == kFmiPrecipitation1h)
			str += GetEcFraktileParamToolTipText(kFmiTotalPrecipitationF100, "RR", theLatlon, theTime, theColor);
		else if(itsDrawParam->Param().GetParamIdent() == kFmiTotalCloudCover)
			str += GetEcFraktileParamToolTipText(kFmiTotalCloudCoverF100, "N", theLatlon, theTime, theColor);
		else if(itsDrawParam->Param().GetParamIdent() == kFmiWindSpeedMS) 
			str += GetEcFraktileParamToolTipText(kFmiWindSpeedF100, "WS", theLatlon, theTime, theColor);
        else if(itsDrawParam->Param().GetParamIdent() == kFmiHourlyMaximumGust) 
            str += GetEcFraktileParamToolTipText(kFmiWindGustF100, "WG", theLatlon, theTime, theColor);

	} // end of help data 2}
	return str;
}

std::string NFmiTimeSerialView::GetSeaLevelPlumeDataToolTipText(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor)
{
    std::string str;
    auto seaLevelFractileData = GetSeaLevelPlumeData();
    if(itsCtrlViewDocumentInterface->ShowHelperData2InTimeSerialView() && seaLevelFractileData)
    {
        if(IsSeaLevelPlumeParam())
        {
            // Jos lˆytyy 1. parametreista ja l‰hin asema piste on g_SeaLevelProbabilityMaxSearchRangeInMetres rajan sis‰ll‰
            if(seaLevelFractileData->Param(g_SeaLevelPlumeFractileParams[0]) && seaLevelFractileData->NearestLocation(theLatlon, g_SeaLevelProbabilityMaxSearchRangeInMetres))
            {
                std::string paramName = "SeaLevel";
                str += "<br><hr color=red><br>";
                str += seaLevelFractileData->Producer()->GetName() + " " + paramName + "\n";

                for(size_t i = 0; i < g_SeaLevelPlumeFractileParams.size(); i++)
                {
                    seaLevelFractileData->Param(g_SeaLevelPlumeFractileParams[i]);
                    ::AddValueLineString(str, paramName + "-" + g_SeaLevelPlumeFractileParamLabels[i], theColor, seaLevelFractileData->InterpolatedValue(theTime), itsDrawParam, true);
                }
            }
            str += GetSeaLevelProbDataToolTipText(theViewedInfo, seaLevelFractileData, theLatlon, theTime, theColor);
        }
        else if(IsSeaLevelProbLimitParam())
        {
            // Jos lˆytyy 1. parametreista ja l‰hin asema piste on g_SeaLevelProbabilityMaxSearchRangeInMetres rajan sis‰ll‰
            if(seaLevelFractileData->Param(g_SeaLevelProbLimitParams[0]) && seaLevelFractileData->NearestLocation(theLatlon, g_SeaLevelProbabilityMaxSearchRangeInMetres))
            {
                std::string paramName = "ProbLimit";
                str += "<br><hr color=red><br>";
                str += seaLevelFractileData->Producer()->GetName() + " " + paramName + "\n";

                for(size_t i = 0; i < g_SeaLevelProbLimitParams.size(); i++)
                {
                    seaLevelFractileData->Param(g_SeaLevelProbLimitParams[i]);
                    ::AddValueLineString(str, paramName + std::to_string(i+1), theColor, seaLevelFractileData->InterpolatedValue(theTime), itsDrawParam, true);
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
std::string NFmiTimeSerialView::GetSeaLevelProbDataToolTipText(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, boost::shared_ptr<NFmiFastQueryInfo> &theSeaLevelFractileData, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor)
{
    std::string str;
    std::string paramName = "ProbLimit";
    auto oldLocationIndex = theViewedInfo->LocationIndex();
    if(theViewedInfo->NearestLocation(theLatlon, g_SeaLevelProbabilityMaxSearchRangeInMetres))
    {
        // Varmistetaan ett‰ taulukko on olemassa, t‰m‰ on ‰‰ri nopea toiminto, kun se on kerran alustettu
        InitSeaLevelProbabilityStationData();
        auto probStationData = ::FindSeaLevelProbabilityStationData(theViewedInfo->Location(), theViewedInfo->LatLon());
        if(probStationData)
        {
            str += "<br><hr color=red><br>";
            str += "Fixed sea level ProbLimits 1-4 for ";
            str += probStationData->station_.GetName();
            str += "\n";

            ::AddValueLineString(str, paramName + "1", theColor, probStationData->prob1_, itsDrawParam, true);
            ::AddValueLineString(str, paramName + "2", theColor, probStationData->prob2_, itsDrawParam, true);
            ::AddValueLineString(str, paramName + "3", theColor, probStationData->prob3_, itsDrawParam, true);
            ::AddValueLineString(str, paramName + "4", theColor, probStationData->prob4_, itsDrawParam, true);
        }

        str += "<br><hr color=red><br>";
        str += "Forecasted ProbLimit 1-4 values for ";
        str += ::GetSeaLevelProbLocationName(theViewedInfo);
        str += "\n";

        theSeaLevelFractileData->Param(static_cast<FmiParameterName>(1305));
        ::AddValueLineString(str, paramName + "1", theColor, theSeaLevelFractileData->InterpolatedValue(theTime), itsDrawParam, true);
        theSeaLevelFractileData->Param(static_cast<FmiParameterName>(1306));
        ::AddValueLineString(str, paramName + "2", theColor, theSeaLevelFractileData->InterpolatedValue(theTime), itsDrawParam, true);
        theSeaLevelFractileData->Param(static_cast<FmiParameterName>(1307));
        ::AddValueLineString(str, paramName + "3", theColor, theSeaLevelFractileData->InterpolatedValue(theTime), itsDrawParam, true);
        theSeaLevelFractileData->Param(static_cast<FmiParameterName>(1308));
        ::AddValueLineString(str, paramName + "4", theColor, theSeaLevelFractileData->InterpolatedValue(theTime), itsDrawParam, true);
    }
    theViewedInfo->LocationIndex(oldLocationIndex);
    return str;
}

std::string NFmiTimeSerialView::GetModelClimatologyDataToolTipText(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor)
{
    if(theViewedInfo->SizeLevels() == 1 && itsCtrlViewDocumentInterface->GetModelClimatologyData() && itsCtrlViewDocumentInterface->ShowHelperData3InTimeSerialView())
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

std::string NFmiTimeSerialView::GetObservationToolTipText(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor)
{
	std::string str;
	if(itsDrawParam->Level().LevelValue() == kFloatMissing) // kepa ja havainto laiteaan apudatoiksi vain pintadatoille!
	{
		// sitten havainto data
    	boost::shared_ptr<NFmiFastQueryInfo> obsInfo = GetObservationInfo(*theViewedInfo->Param().GetParam(), theLatlon);
		if(obsInfo)
		{
			obsInfo->FirstLevel(); // varmuuden vuoksi asetan 1. leveliin
			if(obsInfo->Param(*(theViewedInfo->Param().GetParam()))) // parametrikin pit‰‰ asettaa
			{
				NFmiTimeBag drawedTimes;
				if(GetDrawedTimes(obsInfo, drawedTimes))
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
	}
	return str;
}

static std::string GetColoredLocationTooltipStr(CtrlViewDocumentInterface *theCtrlViewDocumentInterface, const NFmiPoint &theLatlon, int theSelectedLocationCounter)
{
	std::string str;
	str += "<font color=";
	str += CtrlViewUtils::Color2HtmlColorStr(theCtrlViewDocumentInterface->GeneralColor(theSelectedLocationCounter));
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
            float value = info->InterpolatedValue(theLatlon, theTime);
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
        short mainDataTimeResolution = static_cast<short>(viewedInfo->TimeResolution());
        if(mainDataTimeResolution > 60)
            mainDataTimeResolution = 60;
        aTime.SetTimeStep(mainDataTimeResolution); // asetetaan tooltipin ajaksi joko p‰‰datatan aikaresoluutio tai maksimissaan 60 minuuttia
	    NFmiString timeStr1 = aTime.ToStr("Nnnn DD. YYYY\nWwww HH:mm [UTC]", itsCtrlViewDocumentInterface->Language());
	    str += timeStr1;
	    str += "<br><hr color=red><br>";

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
		string parNameStr = CtrlViewUtils::GetParamNameString(itsDrawParam, itsCtrlViewDocumentInterface, ::GetDictionaryString("MapViewToolTipOrigTimeNormal"), ::GetDictionaryString("MapViewToolTipOrigTimeMinute"), false, showExtraInfo, true);
		NFmiColor stationDataColor;
		editedInfo->FirstLocation();
		int selectedLocationCounter = 0;
        float value = viewedInfo->InterpolatedValue(primaryLocationLatlon, aTime);
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
				value = viewedInfo->InterpolatedValue(editedInfo->LatLon(), aTime);
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
					if(kepaInfo->Param(*(viewedInfo->Param().GetParam()))) // parametrikin pit‰‰ asettaa
					{
						str += "\n";
						::AddValueLineString(str, "Official edited ", normalTitleColor, kepaInfo->InterpolatedValue(primaryLocationLatlon, aTime), itsDrawParam, false);
					}
				}
			}

			str += GetModelDataToolTipText(viewedInfo, primaryLocationLatlon, aTime);
			str += GetHelpDataToolTipText(primaryLocationLatlon, aTime);
			str += GetObsFraktileDataToolTipText(viewedInfo, primaryLocationLatlon, aTime, normalTitleColor);
		} // end of help data 1

		str += GetEcFraktileDataToolTipText(viewedInfo, primaryLocationLatlon, aTime, normalTitleColor);
        str += GetSeaLevelPlumeDataToolTipText(viewedInfo, primaryLocationLatlon, aTime, normalTitleColor);
        str += GetModelClimatologyDataToolTipText(viewedInfo, primaryLocationLatlon, aTime, normalTitleColor);
        str += GetMosTemperatureMinAndMaxDataToolTipText(viewedInfo, primaryLocationLatlon, aTime, normalTitleColor);
    }

	return str;
}

std::string NFmiTimeSerialView::GetHelpDataToolTipText(const NFmiPoint &theLatlon, const NFmiMetTime &theTime)
{
	std::string str;
	if(itsCtrlViewDocumentInterface->HelpEditorSystem().Use())
	{
		NFmiProducer prod(NFmiProducerSystem::gHelpEditorDataProdId, "help-data");
		boost::shared_ptr<NFmiFastQueryInfo> helpInfo = ::GetWantedData(itsCtrlViewDocumentInterface, itsDrawParam, prod, NFmiInfoData::kEditingHelpData);
		if(helpInfo)
		{
			str += "\n";
			::AddValueLineString(str, static_cast<char*>(prod.GetName()), itsCtrlViewDocumentInterface->HelpEditorSystem().HelpColor(), helpInfo->InterpolatedValue(theLatlon, theTime), itsDrawParam, false);
		}
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
    return itsCtrlViewDocumentInterface->OutOfEditedAreaTimeSerialPoint();
}
