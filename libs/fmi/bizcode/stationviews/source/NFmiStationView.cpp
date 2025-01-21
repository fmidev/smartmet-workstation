//© Ilmatieteenlaitos/Marko
//  Original 21.09.1998
//
//
// Changed 1999.09.10/Marko Tästä lähtien haluttu näytettävä aika annetaan ulkoa päin.
//
//-------------------------------------------------------------------- NFmiStationView.cpp
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ kääntäjän varoitusta
#endif

#include "NFmiStationView.h"
#include "NFmiToolBox.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiSmartInfo.h"
#include "NFmiDrawParam.h"
#include "NFmiAreaMaskList.h"
#include "NFmiLocationSelectionTool.h"
#include "NFmiDataModifierClasses.h"
#include "NFmiEditorControlPointManager.h"
#include "NFmiArea.h"
#include "NFmiRectangle.h"
#include "NFmiText.h"
#include "NFmiGrid.h"
#include "NFmiValueString.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiLatlonArea.h"

#include "NFmiEditMapDataListHandler.h"
#include "NFmiStringList.h"
#include "NFmiSmartToolModifier.h"
#include "NFmiMacroParamSystem.h"
#include "NFmiMacroParam.h"
#include "NFmiObsComparisonInfo.h"
#include "NFmiProducerName.h"
#include "NFmiStringTools.h"
#include "NFmiLagrange.h"
#include "NFmiMetEditorOptionsData.h"
#include "NFmiMTATempSystem.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiDataParamModifier.h"
#include "NFmiObsDataGridding.h"
#include "NFmiIgnoreStationsData.h"
#include "NFmiIsoLineView.h"
#include "TimeSerialModification.h"
#include "NFmiGriddingHelperInterface.h"
#include "NFmiFastInfoUtils.h"
#include "CtrlViewDocumentInterface.h"
#include "CtrlViewFunctions.h"
#include "NFmiQueryDataUtil.h"
#include "CtrlViewKeyboardFunctions.h"
#include "MapHandlerInterface.h"
#include "CtrlViewFastInfoFunctions.h"
#include "ToolMasterColorCube.h"
#include "catlog/catlog.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "Q2ServerInfo.h"
#include "NFmiHelpDataInfo.h"
#include "EditedInfoMaskHandler.h"
#include "NFmiApplicationWinRegistry.h"
#include "NFmiCommentStripper.h"
#include "ToolBoxStateRestorer.h"
#include "NFmiMacroParamDataCache.h"
#include "Utf8ConversionFunctions.h"
#include "NFmiMacroParamfunctions.h"
#include "NFmiIsoLineData.h"
#include "ToolMasterDrawingFunctions.h"
#include "NFmiCountryBorderDrawUtils.h"
#include "CombinedMapHandlerInterface.h"
#include "GdiplusStationBulkDraw.h"
#include "SparseDataGrid.h"
#include "NFmiSymbolTextMapping.h"
#include "ColorStringFunctions.h"
#include "NFmiSmartToolIntepreter.h"

#include <cmath>
#include <stdexcept>
#include <fstream>
#include "boost\math\special_functions\round.hpp"

using namespace std;

namespace
{
	NFmiSymbolTextMappingCache g_SymbolMappingsCache;
	NFmiColor g_MacroParamSymbolTextColor(0, .75f, 0.2f);
	NFmiColor g_MacroParamMultiParamBaseTextColor(0.1f, .65f, 0.28f);
}

bool LogMessage(const NFmiString& fileName, const NFmiString& text)
{
	static int counter = 0;
	using namespace std;
	ofstream out(fileName, ios::app);
	if(out)
	{
		counter++;
		out << counter << " " << (char*) text << endl;
		out.close();
		return true;
	}
	return false;
}

static bool NearestShipLocation(NFmiFastQueryInfo &theInfo, const NFmiLocation &theLocation)
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

static boost::shared_ptr<NFmiFastQueryInfo> FindInfoWithNearestLocation(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector, const NFmiPoint& theWantedLatLon)
{
	NFmiLocation wantedLocation(theWantedLatLon);
	if(theInfoVector.empty() == false)
	{ // etsitään useasta infosta lähintä asemaa
		const double defaultDist = 999999999.;
		double minDist = defaultDist;
		int wantedInfoIndex = -1;
		int index = 0;
		unsigned long minLocationIndex = static_cast<unsigned long>(-1);
		for(size_t i = 0; i < theInfoVector.size(); i++)
		{
			boost::shared_ptr<NFmiFastQueryInfo> info = theInfoVector[i];
			FmiProducerName prod = static_cast<FmiProducerName>(info->Producer()->GetIdent());
			bool useLocationFromData = false;
			if(prod == kFmiSHIP || prod == kFmiBUOY)
				useLocationFromData = true;
			if(useLocationFromData ? ::NearestShipLocation(*info, wantedLocation) : info->NearestLocation(wantedLocation))
			{
				double currentDistance = wantedLocation.Distance(info->LatLon());
				if(currentDistance < minDist)
				{
					minDist = currentDistance;
					wantedInfoIndex = index;
					minLocationIndex = info->LocationIndex();
				}
			}
			index++;
		}
		if(wantedInfoIndex >=0)
		{
			boost::shared_ptr<NFmiFastQueryInfo> info = theInfoVector[wantedInfoIndex];
			info->LocationIndex(minLocationIndex);
			return info;
		}
	}
	return boost::shared_ptr<NFmiFastQueryInfo>();
}

bool NFmiStationView::IsQ2ServerUsed(void)
{
	return itsCtrlViewDocumentInterface->UseQ2Server();
}

NFmiStationView::NFmiStationView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
								,NFmiToolBox *theToolBox
								,boost::shared_ptr<NFmiDrawParam> &theDrawParam
								,FmiParameterName theParamId
								,NFmiPoint theOffSet
								,NFmiPoint theSize
                                ,int viewGridRowNumber
                                ,int viewGridColumnNumber)
: NFmiAreaView(theMapViewDescTopIndex, theArea
			  ,theToolBox
			  ,theDrawParam
              ,viewGridRowNumber
              ,viewGridColumnNumber)
,itsOriginalDataInfo()
,itsObjectOffSet(theOffSet)
,itsObjectSize(theSize)
,fDrawText(false)
,itsGeneralStationRect()
,itsParamId(theParamId)
,itsInfoVector()
,fDoTimeInterpolation(false)
,itsBackupDrawParamForDifferenceDrawing()
,fDoDifferenceDrawSwitch(false)
,fDoMovingStationDataLocations(false)
,fUseMacroParamSpecialCalculations(false)
,itsMacroParamSpecialCalculationsValues()
,fGetSynopDataFromQ2(false)
,itsSynopDataValuesFromQ2()
,itsSynopDataFromQ2StationIndexies()
,itsToolTipDiffValue1(kFloatMissing)
,itsToolTipDiffValue2(kFloatMissing)
,itsSpecialMatrixData()
,fUseCalculationPoints(false)
,fUseAlReadySpacedOutData(false)
,itsTimeInterpolationRangeInMinutes(kLongMissing)
,fAllowNearestTimeInterpolation(false)
{
}

NFmiStationView::~NFmiStationView(void)
{
}

double NFmiStationView::MaximumFontSizeFactor() const
{
    return itsCtrlViewDocumentInterface->Registry_MaximumFontSizeFactor();
}

NFmiRect NFmiStationView::CalcSymbolRelativeRect(const NFmiPoint &theLatlon, double theSymbolSizeInMM) const
{
    NFmiPoint viewPoint(LatLonToViewPoint(theLatlon));
    long pixelWidth = boost::math::iround(theSymbolSizeInMM * itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_x);
    long pixelHeight = boost::math::iround(theSymbolSizeInMM * itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_y);
    NFmiPoint relativeSize(itsToolBox->SX(pixelWidth), itsToolBox->SY(pixelHeight));
    NFmiRect symbolRect(0, 0, relativeSize.X(), relativeSize.Y());
    symbolRect.Center(viewPoint);
    return symbolRect;
}

// Käytetäänkö Q2-serveriä hakemaan synop-dataa karttanäytölle?
// Jos prioriteetti 1. synop-datan (euro-synop) alkuaika on uudempi kuin 
// itsTime, tehdään Q2-haku, jos se on sallittu.
bool NFmiStationView::UseQ2ForSynopData(boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
	if(theDrawParam->Param().GetProducer()->GetIdent() == kFmiSYNOP && IsQ2ServerUsed())
	{
		if(itsInfoVector.size() == 0)
			MakeDrawedInfoVector(); // kokeillaan, ollaanko tilassa, missä pitää rakentaa info vektori uudestaan

		if(itsInfoVector.size())
		{
			// nyt kiinnostaa siis nimenomaan prioriteetti 1 data, mikä on euro-synop-data, koska juuri sitä laitetaan
			// tällä hetkellä tietokantaan, jolta q2 hakee havainto datansa
			NFmiMetTime firstQdTime = (*itsInfoVector.begin())->TimeDescriptor().FirstTime();
			if(firstQdTime > itsTime)
				return true;
		}
	}
	return false;
}

bool NFmiStationView::GetDataFromLocalInfo() const
{
    if(fGetSynopDataFromQ2 || fGetCurrentDataFromQ2Server || fUseMacroParamSpecialCalculations)
        return false;
    else
        return true;
}

void NFmiStationView::SetupPossibleWindMetaParamData()
{
    if(itsInfo && GetDataFromLocalInfo())
        metaWindParamUsage = NFmiFastInfoUtils::CheckMetaWindParamUsage(itsInfo);
    else
        metaWindParamUsage = NFmiFastInfoUtils::MetaWindParamUsage(); // reset this if local data not used
}

// jos palauttaa true, tehdään piirto, muuten ei
bool NFmiStationView::PrepareForStationDraw(void)
{
    fUseCalculationPoints = false;
    fUseAlReadySpacedOutData = false;
	fUseMacroParamSpecialCalculations = false;
	fDoTimeInterpolation = false;
	if(!itsInfo) // tämä alustetaan jo SetMapViewSettings-metodissa
		return false;
    SetupPossibleWindMetaParamData();
	bool dummyBoolNotUsed = false;
	if(IsMacroParamCase())
	{
		if(itsDrawParam->IsParamHidden() == false)
		{
			unsigned long currentLocationIndex = itsInfo->LocationIndex();
			fUseMacroParamSpecialCalculations = true;
			CalcViewFloatValueMatrix(itsMacroParamSpecialCalculationsValues, 0, 0, 0, 0, dummyBoolNotUsed); // datahila pitää laskea jo tässä
			itsInfo->LocationIndex(currentLocationIndex); // CalcViewFloatValueMatrix nollaa paikan, joten se pitää asettaa tässä takaisin
		}
	}

	if(UseQ2ForSynopData(itsDrawParam))
	{
		if(fGetSynopDataFromQ2 == false)
			GetQ2SynopData();
	}
	else if(fGetCurrentDataFromQ2Server)
	{
		CalcViewFloatValueMatrix(itsQ2ServerDataValues, 0, 0, 0, 0, dummyBoolNotUsed);
	}
	else if(!itsInfo->Time(itsTime))
	{
		auto flashTypeData = NFmiFastInfoUtils::IsLightningTypeData(itsInfo);
		if(itsInfo->DataType() == NFmiInfoData::kStationary)
			itsInfo->FirstTime();
		else if(itsInfo->TimeDescriptor().IsInside(itsTime) || NFmiFastInfoUtils::IsModelClimatologyData(itsInfo) || flashTypeData)
		{
			if(itsInfo->IsGrid()) // hila dataa voi interpoloida ajassa, mutta ei asema dataa eli havaintoje
				fDoTimeInterpolation = true; // nyt voidaan piirtää dataa käyttäen aikainterpolaatiota
			else if(flashTypeData)
			{
				// Ei tehdä mitään, mutta sallitaan piirto lopuksi, koska piirrettävältä aikaväliltä voi löytyä dataa
			}
			else
				return false;
		}
		else if(fUseMacroParamSpecialCalculations)
			return true;
		else
			return false;
	}
	return true;
}

bool NFmiStationView::IsParamDrawn()
{
    if(!itsDrawParam || itsDrawParam->IsParamHidden())
        return false;
    else
        return true;
}

void NFmiStationView::Draw(NFmiToolBox* theGTB)
{
	if(!IsParamDrawn())
	{
		CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging("NFmiStationView doesn't draw anything, param was hidden", this);
		return;
	}

	CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, __FUNCTION__);

	fUseMacroParamSpecialCalculations = false;
	fGetSynopDataFromQ2 = false; // aluksi laitetaan falseksi, haku tehdään kerran PrepareForStationDraw-metodissa jossa onnistumisen kanssa lippu laitetaan päälle
	itsOptimizedGridPtr.reset();
	itsProbingExtraMacroParamData.Clear();
	itsMacroParamPhase = MacroParamPhase::NoPhase;

	if(!theGTB)
		return;

	itsToolBox = theGTB;

	if(itsDrawParam->DataType() == NFmiInfoData::kMapLayer)
	{
		// Rajaviivat piirretään tässä kartan piirtopinoon, jos kyse on erillisestä "country border" -layeristä
		DrawCountryBordersToMapView();
		return; // Muuta ei saakaan sitten tehdä
	}

	ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsArea->XYArea());
	CalculateGeneralStationRect();
	itsSymbolBulkDrawData.clear();
	itsCachedSpaceOutFactors = NFmiPoint::gMissingLatlon;
	itsEnlargedDrawArea = SbdCalcEnlargedDrawArea();

	MakeDrawedInfoVector();
	for(auto& fastInfo : itsInfoVector)
	{
		// Varmistetaan että osoitetaan johon validiin asemaan/pisteeseen, muuten tulee ongelmia nan -pohjaisten point-olioiden kanssa
		fastInfo->FirstLocation();
		SetMapViewSettings(fastInfo);
		UpdateCachedParameterName();
		FmiFontType oldFont = itsDrawingEnvironment.GetFontType();

		ModifyTextEnvironment();
		if(!PrepareForStationDraw())
			continue;
		SbdCollectSymbolDrawData(false);
		DrawObsComparison();

		itsDrawingEnvironment.SetFontType(oldFont);
		fDoTimeInterpolation = false;
		itsInfo = nullptr;
	}
	SbdDoSymbolDraw(false);
}

void NFmiStationView::DrawCountryBordersToMapView()
{
	if(CombinedMapHandlerInterface::IsBorderLayerDrawn(itsDrawParam.get()))
		NFmiCountryBorderDrawUtils::drawCountryBordersToMapView(this, itsToolBox, itsDrawParam.get());
}

void NFmiStationView::MakeDrawedInfoVector(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector, boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
	itsCtrlViewDocumentInterface->MakeDrawedInfoVectorForMapView(theInfoVector, theDrawParam, GetArea());
}

void NFmiStationView::MakeDrawedInfoVector(void)
{
	fGetCurrentDataFromQ2Server = false;
	MakeDrawedInfoVector(itsInfoVector, itsDrawParam);
	if(itsInfoVector.empty() && itsDrawParam->ModelRunIndex() < 0)
	{ // jos ei löytynyt dataa ja kyse on arkistodatahausta, koetetaan hakea viimeisin löytyvä data. Jos se löytyy, merkitään että tämä pitää lopuksi hakea q2serveriltä.
		// Näin saadaan itsInfoVector:iin datan eräänlainen kopio, jolle voidaan tehdä erilaisia tarkasteluja ennen itse piirtoa.
		boost::shared_ptr<NFmiDrawParam> tmpDrawParam(new NFmiDrawParam(*itsDrawParam));
		tmpDrawParam->ModelRunIndex(0);
		MakeDrawedInfoVector(itsInfoVector, tmpDrawParam);
		if(itsInfoVector.empty() == false)
			fGetCurrentDataFromQ2Server = true;
	}
}

static int GetGridDataIndex(int xSize, int xInd, int yInd)
{
	return (xSize * yInd) + xInd;
}

static std::vector<unsigned long> FillLocationIndexies(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	if(theInfo->IsGrid())
	{ // hiladatalle palautetaan seuraavat paikkaindeksit
		int xSize = theInfo->Grid()->XNumber();
		int ySize = theInfo->Grid()->YNumber();
		std::vector<unsigned long> locationIndexies;
		if(xSize > 0 && ySize > 0)
		{
			locationIndexies.push_back(GetGridDataIndex(xSize, xSize/2, ySize/2)); // laitetaan ensimmäiseksi hilan puoliväli, koska reunoista puuttuu helposti dataa
			locationIndexies.push_back(0); // laitetaan 1. hilapiste
			locationIndexies.push_back(theInfo->SizeLocations()-1); // laitetaan mukaan vielä viimeinen hilapiste
			if(xSize > 4 && ySize > 4)
			{ // laitetaan sitten 'neljännes' kehä pisteet ks. kaava kuva
			  // --------
			  // -*-*-*--
			  // --------
			  // -*---*--
			  // --------
			  // -*-*-*--
			  // --------

				locationIndexies.push_back(GetGridDataIndex(xSize, xSize/4, ySize/4));
				locationIndexies.push_back(GetGridDataIndex(xSize, xSize/4, ySize/2));
				locationIndexies.push_back(GetGridDataIndex(xSize, xSize/4, 3*ySize/4));
				locationIndexies.push_back(GetGridDataIndex(xSize, xSize/2, ySize/4));
				locationIndexies.push_back(GetGridDataIndex(xSize, xSize/2, 3*ySize/4));
				locationIndexies.push_back(GetGridDataIndex(xSize, 3*xSize/4, ySize/4));
				locationIndexies.push_back(GetGridDataIndex(xSize, 3*xSize/4, ySize/2));
				locationIndexies.push_back(GetGridDataIndex(xSize, 3*xSize/4, 3*ySize/4));

                // Tehdään isommille hiloille vielä kattavampi otos
                auto totalGridSize = theInfo->SizeLocations();
                if(totalGridSize >= 500)
                {
                    // Tehdään hilakokoon suhteutettu otos
                    auto indexSkipCount = static_cast<unsigned long>(std::sqrt(totalGridSize));
                    for(unsigned long checkedLocationIndex = 0; checkedLocationIndex < theInfo->SizeLocations(); checkedLocationIndex += indexSkipCount)
                    {
                        locationIndexies.push_back(checkedLocationIndex);
                    }
                }
			}
		}
		return locationIndexies;
	}
	return std::vector<unsigned long>();
}

static std::vector<float> MatrixToVector(const NFmiDataMatrix<float>& matrix)
{
	std::vector<float> vec(matrix.NX() * matrix.NY(), kFloatMissing);
	size_t totalIndex = 0;
	for(size_t yIndex = 0; yIndex < matrix.NY(); yIndex++)
	{
		for(size_t xIndex = 0; xIndex < matrix.NX(); xIndex++)
		{
			vec[totalIndex] = matrix[xIndex][yIndex];
			totalIndex++;
		}
	}
	return vec;
}

// käy läpi muutaman pisteen datassa ja etsii sieltä edustavimman teksti pituuden
int NFmiStationView::GetApproxmationOfDataTextLength(std::vector<float> *sampleValues)
{
	if(IsSingleSymbolView())
        return 1;
    else
    {
		if(itsDrawParam->IsFixedTextSymbolDrawLengthUsed())
			return itsDrawParam->FixedTextSymbolDrawLength();
        if(sampleValues)
            return CalcApproxmationOfDataTextLength(*sampleValues).first;
		else
		{
			auto firstApproximation = CalcApproxmationOfDataTextLength(GetSampleDataForDataTextLengthApproxmation());
			if(itsDrawParam->DoSparseSymbolVisualization() && itsInfo->IsGrid())
			{
				if(!firstApproximation.second)
				{
					if(IsMacroParamCase())
					{
						// MacroParam tapaukset ovat erikoistapaus, jos niille ei löydy hyviä arvioita, 
						// ei dataa aleta enempää laskemaan ja käydä sitä läpi. Palautetaa vain ns. oletusarvo.
						// Tällä estetään tietyt ikiloopit, kun lasketusta probe-datasta ei löydy yhtään non-missing arvoa.
						return firstApproximation.first;
					}
					// Jos alkuarvauksesta ei löytynyt non-missing arvoja, pitää käydä koko data läpi
					bool dummyBoolNotUsed = false;
					NFmiDataMatrix<float> valueMatrix;
					if(CalcViewFloatValueMatrix(valueMatrix, 0, 0, 0, 0, dummyBoolNotUsed))
					{
						std::vector<float> fullFieldValues = ::MatrixToVector(valueMatrix);
						return CalcApproxmationOfDataTextLength(fullFieldValues).first;
					}
				}
			}
			// Tähän tullaan normaali tilanteessa ja silloin kun sparse-visualisointi tapauksessa löytyi ei-puuttuvia arvoja pienestä näytehilasta
			return firstApproximation.first;
		}
    }
}

// Jos laskuissa saatu non-missing arvoihin perustuva arvio tekstin pituudesta, silloin std::pair:issa palautuu true second:ina.
// Jos kaikki oli puuttuvaa, palautetaan <1,false>
std::pair<int, bool> NFmiStationView::CalcApproxmationOfDataTextLength(const std::vector<float> &sampleValues)
{
    NFmiDataModifierMinMax minmaxCalc;
    NFmiDataModifierAvg avgCalc;
    for(auto value : sampleValues)
    {
        if(value != kFloatMissing)
        {
            NFmiString text(GetPrintedText(value));
            minmaxCalc.Calculate(static_cast<float>(text.GetLen()));
            avgCalc.Calculate(static_cast<float>(text.GetLen()));
        }
    }

    if(minmaxCalc.MaxValue() != kFloatMissing)
    {
        if(minmaxCalc.MaxValue() - avgCalc.CalculationResult() >= 4)
            return std::make_pair(boost::math::iround(avgCalc.CalculationResult()), true); // jos maksimi pituuden ja keski teksti pituuden ero oli 4 tai yli, palauta keskiarvon pyöristys
        else // muuten palauta maksimi arvo
            return std::make_pair(static_cast<int>(minmaxCalc.MaxValue()), true);
    }
    return std::make_pair(1, false);
}

std::vector<float> NFmiStationView::GetSampleDataForDataTextLengthApproxmation()
{
	if(IsMacroParamCase())
	{
		return GetSampleDataFrmoMacroParamForDataTextLengthApproxmation();
	}

    unsigned long oldLocationIndex = itsInfo->LocationIndex();
    std::vector<unsigned long> locationIndexies(FillLocationIndexies(itsInfo));
    std::vector<float> values;
    for(auto locationIndex : locationIndexies)
    {
        itsInfo->LocationIndex(locationIndex);
        values.push_back(ViewFloatValue(false));
    }

    itsInfo->LocationIndex(oldLocationIndex);
    return values;
}

static bool GetNonMissingSampleVectorFromMacroParamCache(const NFmiMacroParamLayerCacheDataType& cacheData, std::vector<float> &sampleVectorOut)
{
	const auto& matrix = cacheData.getDataMatrix();
	size_t maxIndex = matrix.NX() * matrix.NY();
	size_t step = 1;
	for(size_t index = 0; index < maxIndex; index += step)
	{
		size_t xIndex = index % matrix.NX();
		size_t yIndex = index / matrix.NX();
		auto value = matrix[xIndex][yIndex];
		if(value != kFloatMissing)
		{
			sampleVectorOut.push_back(value);
			if(sampleVectorOut.size() % 10)
			{
				// Joka 10:n arvon jälkeen harvennetaan hakusteppiä,
				// näin pienestä matriisista saa tarvittavan tiedon,
				// mutta jos matriisi on iso, haluataan arvoja sieltä 
				// täältä (kattavampi otos kartan eri osista), mutta 
				// ei kaikkia miljoonaa arvoa.
				step *= 2;
			}
		}
	}
	return !sampleVectorOut.empty();
}

std::vector<float> NFmiStationView::GetSampleDataFrmoMacroParamForDataTextLengthApproxmation()
{
	std::vector<float> sampleFromCache;
	NFmiMacroParamDataCacheLayer macroParamTotalCache;
	auto realRowIndex = CalcRealRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber);
	if(itsCtrlViewDocumentInterface->MacroParamDataCache().getTotalCache(itsMapViewDescTopIndex, realRowIndex, itsViewRowLayerNumber, itsDrawParam->InitFileName(), macroParamTotalCache))
	{
		for(const auto& singleTimeCache : macroParamTotalCache.layerCache())
		{
			if(::GetNonMissingSampleVectorFromMacroParamCache(singleTimeCache.second, sampleFromCache))
				return sampleFromCache;
		}
	}

	if(!itsMacroParamProbingValues.empty())
	{
		return itsMacroParamProbingValues;
	}

	return sampleFromCache;
}

// näillä kertoimilla vielä tehdään viimeistely
NFmiPoint NFmiStationView::GetSpaceOutFontFactor(void)
{
	return NFmiPoint(0.5, 1);
}

static double AdjustSymbolSizeFactorWithDensity(double sizeFactor, double symbolDrawDensity)
{
	if(symbolDrawDensity != DefaultSymbolDrawDensity)
	{
		sizeFactor *= symbolDrawDensity;
	}
	return sizeFactor;
}

void NFmiStationView::DoSymboldrawDensityAdjustments(double &xSizeFactorInOut, double &ySizeFactorInOut)
{
	xSizeFactorInOut = ::AdjustSymbolSizeFactorWithDensity(xSizeFactorInOut, itsDrawParam->SymbolDrawDensityX());
	ySizeFactorInOut = ::AdjustSymbolSizeFactorWithDensity(ySizeFactorInOut, itsDrawParam->SymbolDrawDensityY());
}

static boost::shared_ptr<NFmiFastQueryInfo> GetUsedSpaceOutCalculationInfo(boost::shared_ptr<NFmiFastQueryInfo>& normalInfo, NFmiExtraMacroParamData& probingExtraMacroParamData)
{
	if(probingExtraMacroParamData.IsFixedSpacedOutDataCase() && probingExtraMacroParamData.FixedBaseDataInfo())
	{
		return probingExtraMacroParamData.FixedBaseDataInfo();
	}
	return normalInfo;
}

#ifdef max
#undef max
#endif


// OLETUS!! itsInfo on grid-dataa.
// HUOM! Pitää varmistaa että tulos on minimissään 1.
NFmiPoint NFmiStationView::CalcUsedSpaceOutFactors(int theSpaceOutFactor)
{
	if(theSpaceOutFactor == 0)
	{
		// 0. Return no space-out factor value
		return NFmiPoint(1, 1);
	}
	else
    {
		auto usedInfo = ::GetUsedSpaceOutCalculationInfo(itsInfo, itsProbingExtraMacroParamData);
        unsigned long centerX = usedInfo->GridXNumber() / 2;
        unsigned long centerY = usedInfo->GridYNumber() / 2;
        unsigned long centerLocationIndex = centerY * usedInfo->GridXNumber() + centerX;
		usedInfo->LocationIndex(centerLocationIndex); // nyt tutkitaan hilan keskipistettä (ennen tutkittiin alukulmaa, mutta globaali datassa ala- ja yläreuna pisteet ovat napapisteitä, eikä laskut toimi tällöin) 
        // 1. get font size
        NFmiPoint fontSize(itsDrawingEnvironment.GetFontSize());
        double fontXSize = itsToolBox->SX(static_cast<long>(fontSize.X()));
        double fontYSize = itsToolBox->SY(static_cast<long>(fontSize.Y()));
        // 2. get one value string for estimate string length
        int textLength = GetApproxmationOfDataTextLength();
        NFmiPoint latlon1(usedInfo->LatLon());
        NFmiPoint latlon2(usedInfo->PeekLocationLatLon(1, 0));
        NFmiPoint latlon3(usedInfo->PeekLocationLatLon(0, 1));
        NFmiPoint p1(LatLonToViewPoint(latlon1));
        NFmiPoint p2(LatLonToViewPoint(latlon2));
        NFmiPoint p3(LatLonToViewPoint(latlon3));
        // 3. Calc relative dist of two parallel neighbor grid point in x dir
        double distX = ::fabs(p1.X() - p2.X());
        // 4. Calc relative dist of two vertical neighbor grid point in y dir
        double distY = ::fabs(p1.Y() - p3.Y());
        // 5. calc x and y skippingFactors
        NFmiPoint fontFactor(GetSpaceOutFontFactor());
        double xFactor = fontFactor.X() * fontXSize * textLength / distX;
        double yFactor = fontFactor.Y() * fontYSize / distY;
		DoSymboldrawDensityAdjustments(xFactor, yFactor);
		double finalFactorX = 1;
		double finalFactorY = 1;
        // 6. case theSpaceOutFactor 1 -> ceil, 2 -> ceil + 1
		if(theSpaceOutFactor == 1)
		{
			finalFactorX = std::ceil(xFactor);
			finalFactorY = std::ceil(yFactor);
		}
        else //if(theSpaceOutFactor == 2)
        {
            if(xFactor < 0.5 && yFactor < 0.5) // jos ollaan zoomattu jo tarpeeksi lähellä, näytetään kaikki pisteet
			{
				finalFactorX = std::ceil(xFactor);
				finalFactorY = std::ceil(yFactor);
			}
            else
			{
				finalFactorX = std::ceil(xFactor) + 1;
				finalFactorY = std::ceil(yFactor) + 1;
			}
        }

		finalFactorX = std::max(finalFactorX, 1.);
		finalFactorY = std::max(finalFactorY, 1.);
		return NFmiPoint(finalFactorX, finalFactorY);
    }
}

static std::vector<float> matrixToVector(const NFmiDataMatrix<float> &matrix)
{
    std::vector<float> values;
    for(const auto &subVector : matrix)
    {
        values.insert(values.end(), subVector.begin(), subVector.end());
    }
    return values;
}

// Kun macroParamia piirretään symboleilla, ei kannata laskea arvoja normaalisti, vaan vain niihin kohtiin, mihin symboli tulee piirtää.
// Tässä lasketaan että kuinka tiheä hila symboleja voidaan piirtää annetun data samplen mukaan.
// Data samplella on merkitystä vain jos piirto tapahtuu numero tekstinä. 1-merkkisten symbolien piirrossa ratkaisee vain 
// symboli koko ja käytössä oleva piirtotila.
// theSpaceOutFactor kertoo symboli piirrossa käytetyn harvennustilan, tänne ei pitäisi tulla jos sen arvo on 0, 1 = tiheämpi symboliväli ja 2 harvempi.
NFmiPoint NFmiStationView::CalcSymbolDrawedMacroParamSpaceOutGridSize(int theSpaceOutFactor)
{
    // 1. get relative font size
    NFmiPoint fontSize(itsDrawingEnvironment.GetFontSize());
    double fontXSize = itsToolBox->SX(static_cast<long>(fontSize.X()));
    double fontYSize = itsToolBox->SY(static_cast<long>(fontSize.Y()));
    // 2. Calc estimate for string lengths for sample values
    int textLength = GetApproxmationOfDataTextLength(&itsMacroParamProbingValues);
    // 3. Get space out font factor
    NFmiPoint fontFactor(GetSpaceOutFontFactor());
    // 4. Yhden symbolin koko suhteellisella kartta-alueella
    const double spaceOutExtraFactor = 1.12; // Pitää vielä harventaa hieman lisää symboli kokoja, jotta menee oikein
    double xRelativeSize = fontFactor.X() * fontXSize * textLength * spaceOutExtraFactor;
    double yRelativeSize = fontFactor.Y() * fontYSize * spaceOutExtraFactor;
	DoSymboldrawDensityAdjustments(xRelativeSize, yRelativeSize);
	// 5. Kuinka monta symboli piirtoa mahtuu ruudulle
    double xCount = std::floor(itsArea->XYArea().Width() / xRelativeSize);
    double yCount = std::floor(itsArea->XYArea().Height() / yRelativeSize);

    if(theSpaceOutFactor == 2)
    {
        xCount = std::floor(xCount * 0.9);
        yCount = std::floor(yCount * 0.9);
    }
    if(xCount < 2)
        xCount = 2;
    if(yCount < 2)
        yCount = 2;
    return NFmiPoint(xCount, yCount);
}

void NFmiStationView::SetStationPointDrawingEnvi(NFmiDrawingEnvironment &envi)
{
	envi.SetFrameColor(itsCtrlViewDocumentInterface->StationPointColor(itsMapViewDescTopIndex));
	envi.SetFillColor(itsCtrlViewDocumentInterface->StationPointColor(itsMapViewDescTopIndex));
	envi.EnableFill();
}

// Jos kyseinen drawParam sisältää sellaisen macroParamin, joka lasketaan
// vain havainto datan lähimpiin pisteisiin, sitä ei saa harventaa spacingOutFactor:in 
// mukaan NFmiStationView::DrawSymbols -metodissa.
bool NFmiStationView::IsStationDataMacroParam(void)
{
    try
    {
        if(fUseCalculationPoints)
            return true;
    	std::string macroParamStr = CtrlViewUtils::GetMacroParamFormula(*(itsCtrlViewDocumentInterface->MacroParamSystem()), itsDrawParam);
        if(MacroParam::ci_find_substr(macroParamStr, std::string("closestvalue")) != MacroParam::ci_string_not_found)
            return true;
    }
    catch(...)
    {
    }
    return false;
}

bool NFmiStationView::IsSpaceOutDrawingUsed()
{
	// Jos kyse macroParam tapauksesta, ei tehdä mitään harvennuksia,
	// koska macroParam data on tarkoitus laskea aina lopulliseen hilaan.
	if(IsMacroParamCase())
		return false;

	auto spaceOutFactors = CalcUsedSpaceOutFactors();
	if(spaceOutFactors.X() > 1 || spaceOutFactors.Y() > 1)
	{
		if(itsInfo->Grid()) // asema dataa ei yritetä harventaa
		{
			return true;
		}
	}

	return false;
}

NFmiPoint NFmiStationView::CalcUsedSpaceOutFactors()
{
	auto spacingOutFactor = itsCtrlViewDocumentInterface->Registry_SpacingOutFactor(itsMapViewDescTopIndex);
	if(IsMacroParamCase())
	{
		// MacroParam laskut ovat nyt niin kompleksisia, että vain tietyissä tilanteissa
		// lasketaan kertoimet, muuten palautetaan vain (1,1). Cache arvoja ei lasketa tai käytetä.
	    // Myös calculationpoint tyyppisiä datoja ei harvenneta
		if(itsMacroParamPhase != MacroParamPhase::Calculation || IsStationDataMacroParam())
		{
			return NFmiPoint(1, 1);
		}
		else
		{
			return CalcUsedSpaceOutFactors(spacingOutFactor);
		}
	}

	if(itsCachedSpaceOutFactors == NFmiPoint::gMissingLatlon)
	{
		itsCachedSpaceOutFactors = CalcUsedSpaceOutFactors(spacingOutFactor);
	}
	return itsCachedSpaceOutFactors;
}

CtrlViewUtils::GraphicalInfo& NFmiStationView::GetGraphicalInfo() const
{
	return itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
}

NFmiVisualizationSpaceoutSettings& NFmiStationView::GetVisualizationSettings() const
{
	return itsCtrlViewDocumentInterface->ApplicationWinRegistry().VisualizationSpaceoutSettings();
}

// Seek-rangeja laskettaessa halutaan että alihilat eivät mene päälekkäin vierekkäisissä piirrettävissä pisteissä.
// Rajoitetaan mieluummin kurkkauksia alas ja vasemmalle (x/y tapaukset).
// Parin first on kurkkaus vasemmalle/alas (0 tai negatiivinen luku) ja second on kurkkaus oikealle/ylös (0 tai positiivinen luku).
static std::pair<double, double> CalcSparsePeekRange(int spaceOutFactor)
{
	int halfFactor = spaceOutFactor / 2;
	if(spaceOutFactor % 2 == 0)
	{
		// Pariton tapaus
		return std::make_pair(-(halfFactor - 1.), halfFactor);
	}
	else
	{
		return std::make_pair(-halfFactor, halfFactor);
	}
}

// Spase symbol draw tapauksessa pitää laskea harvennuksen mukainen alihila, jolla
// etsitään piirrettävän hilapisteen ympäriltä peekxy komentojen avulla ei-puuttuvaa arvoa.
static NFmiRect CalcSparsePeekRect(int spaceOutFactorX, int spaceOutFactorY)
{
	auto peekRangeX = ::CalcSparsePeekRange(spaceOutFactorX);
	auto peekRangeY = ::CalcSparsePeekRange(spaceOutFactorY);
	return NFmiRect(peekRangeX.first, peekRangeY.second, peekRangeX.second, peekRangeY.first);
}

static int CalcSparseGridSize(int spaceOutFactor, unsigned long gridSize)
{
	if(gridSize % spaceOutFactor == 0)
		return gridSize / spaceOutFactor;
	else
		return (gridSize / spaceOutFactor) + 1;
}

void NFmiStationView::SbdCollectSpaceOutSymbolDrawData(bool doStationPlotOnly)
{
	auto spaceOutFactors = CalcUsedSpaceOutFactors();
	int spaceOutFactorX = static_cast<int>(spaceOutFactors.X());
	int spaceOutFactorY = static_cast<int>(spaceOutFactors.Y());
	auto sparseDrawPeekRect = ::CalcSparsePeekRect(spaceOutFactorX, spaceOutFactorY);
	auto doSparseCase = itsDrawParam->DoSparseSymbolVisualization() && !doStationPlotOnly;
	SparseDataGrid sparseDataGrid;
	if(doSparseCase)
	{
		int xSize = ::CalcSparseGridSize(spaceOutFactorX, itsInfo->GridXNumber());
		int ySize = ::CalcSparseGridSize(spaceOutFactorY, itsInfo->GridYNumber());
		sparseDataGrid = SparseDataGrid(xSize, ySize);
	}

	int skippinCounter = 0;
	int currentLine = 0;
	int currentSkipColumn = 0;
	int currentSkipRow = 0;
	int lastLine = 0;
	int gridXSize = itsInfo->Grid()->XNumber(); // gridin olemassaa olo on tarkitettu jo aiemmin
	if(gridXSize == 0)
		return; // virhetilanne, ei tehdä mitään, pitäisi heittää poikkeus

	for(itsInfo->ResetLocation(); itsInfo->NextLocation();)
	{
		currentLine = itsInfo->LocationIndex() / gridXSize;
		if(currentLine != lastLine) // jos rivi vaihtuu
		{
			skippinCounter = 0;
		}

		if(currentLine % spaceOutFactorY == 0) // jos spaceOutFactorY on 1, tehdään joka rivi
		{
			if(skippinCounter % spaceOutFactorX == 0) // jos spaceOutFactorX on 1, tehdään joka sarake
			{
				if(SbdIsInsideEnlargedDrawArea())
				{
					currentSkipColumn = itsInfo->LocationIndex() % gridXSize / spaceOutFactorX;
					currentSkipRow = itsInfo->LocationIndex() / gridXSize / spaceOutFactorY;
					// Tämä tehdään vain luotaus datalle: tämä level pitää asettaa joka pisteelle erikseen, koska vakio painepinnat eivät ole kaikille luotaus parametreille samoilla leveleillä
					NFmiFastInfoUtils::SetSoundingDataLevel(itsDrawParam->Level(), *itsInfo);
					if(doSparseCase)
						SbdSearchForSparseSymbolDrawData(doStationPlotOnly, sparseDrawPeekRect, sparseDataGrid, currentSkipColumn, currentSkipRow);
					else
						SbdCollectStationData(doStationPlotOnly);
				}
			}
		}
		skippinCounter++;
		lastLine = currentLine;
	}

	if(doSparseCase)
		SbdDoFinalSparseCaseWork(doStationPlotOnly, sparseDataGrid);
}

void NFmiStationView::SbdDoFinalSparseCaseWork(bool doStationPlotOnly, SparseDataGrid& sparseDataGrid)
{
	auto nonMissingCounter1 = sparseDataGrid.calcNonMissingValues();
	sparseDataGrid.cleanTooCloseHits();
	auto nonMissingCounter2 = sparseDataGrid.calcNonMissingValues();
	const auto& sparseDataMatrix = sparseDataGrid.sparseDataMatrix();
	for(size_t yIndex = 0; yIndex < sparseDataMatrix.NY(); yIndex++)
	{
		for(size_t xIndex = 0; xIndex < sparseDataMatrix.NX(); xIndex++)
		{
			const auto& sparseData = sparseDataMatrix[xIndex][yIndex];
			if(sparseData.insideZoomedArea())
			{
				itsInfo->LocationIndex(sparseData.visualizedLocationIndex());
				SbdCollectStationData(doStationPlotOnly, sparseData.value());
			}
		}
	}
}

NFmiPoint NFmiStationView::SbdCalcOldSchoolSymbolScaleFix(const NFmiPoint& symbolScale) const
{
	// Kokeillaan tiettyjen vanhojen symbol piirtojen litistymisen estoa, kun käytössä on karttaruudukko, 
	// missä ruudukon dimensiot ovat eri kokoisia (esim. 1x3 tai 3x2). Tarkoittaa lähinnä nuolta ja simple-weather-symbol piirtoja.
	// Tämä on siis aikamoinen viritys, mutta muuten pitäisi koko symbolipiirto juttua pitäisi vetää tämän suhteen uusiksi ja 
	// siitä voisi olla outoja seuraamuksia.
	auto viewGridSize = itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex);
	// Jos karttaruudukon dimensiot ovat samoja, ei tehdä mitään.
	if(viewGridSize.X() == viewGridSize.Y())
	{
		return symbolScale;
	}
	else
	{
		auto scale = symbolScale;
		const double fixedFactor1 = 0.75;
		const double fixedFactor2 = 0.875;
		double xPerYRatio = viewGridSize.X() / viewGridSize.Y();
		// Mitä isommaksi/pienemmäksi ratio-luku muuttaa, sitä enemmän liian suuriksi symbolit kasvavat, pitää hillitä ratiota potenssilaskulla
		xPerYRatio = std::pow(xPerYRatio, 0.7);
		if(xPerYRatio > 1)
		{
			scale.X(scale.X() * xPerYRatio * fixedFactor1);
			scale.Y(scale.Y() * fixedFactor2);
		}
		else
		{
			scale.X(scale.X() * fixedFactor2);
			scale.Y(scale.Y() * fixedFactor1 / xPerYRatio);
		}
		return scale;
	}
}

static bool IsCurrentStationBlocked(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiIgnoreStationsData &ignoreStationData)
{
    if(theInfo->IsGrid())
        return false;
    return ignoreStationData.IsStationBlocked(*(theInfo->Location()), true);
}

void NFmiStationView::SbdCollectSparseSymbolDrawData(bool doStationPlotOnly)
{
	for(itsInfo->ResetLocation(); itsInfo->NextLocation();)
	{
		if(SbdIsInsideEnlargedDrawArea())
		{
			NFmiFastInfoUtils::SetSoundingDataLevel(itsDrawParam->Level(), *itsInfo); // Tämä tehdään vain luotaus datalle: tämä level pitää asettaa joka pisteelle erikseen, koska vakio painepinnat eivät ole kaikille luotaus parametreille samoilla leveleillä
			// Harvan datan symboli piirto vain niille pisteille missä on ei-puuttuva arvo,
			// ja SbdCollectStationData metodi toimii juuri niin oletusarvoisesti.
			SbdCollectStationData(doStationPlotOnly);
		}
	}
}

void NFmiStationView::DrawAllAccessoryStationData(void)
{
	if(!itsInfo)
		return;
	CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, "NFmiStationView: Drawing data's station/grid point markers");

	itsSymbolBulkDrawData.clear();
	itsEnlargedDrawArea = SbdCalcEnlargedDrawArea();
	NFmiDrawingEnvironment stationPointEnvi;
	SetStationPointDrawingEnvi(stationPointEnvi);
	ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsArea->XYArea());

	for(auto& fastInfo : itsInfoVector)
	{
		// Varmistetaan että osoitetaan johon validiin asemaan/pisteeseen, muuten tulee ongelmia nan -pohjaisten point-olioiden kanssa
		fastInfo->FirstLocation();
		SetMapViewSettings(fastInfo);
		if(!PrepareForStationDraw())
			continue;
		SbdCollectSymbolDrawData(true);
		itsInfo = nullptr;
	}
	SbdDoSymbolDraw(true);
}

bool NFmiStationView::GetLocation(const NFmiPoint& thePoint, NFmiLocation &theLocation)
{
	return false;
}

void NFmiStationView::DrawStation(NFmiDrawingEnvironment &theStationPointEnvi)
{
	auto plotRect(SbdCalcBaseStationRelativeRect());
	plotRect.Center(CurrentStationPosition());

	NFmiRectangle tmp(plotRect, 0, &theStationPointEnvi);
	itsToolBox->Convert(&tmp);
}

NFmiRect NFmiStationView::CalcInvertStationRectSize(double theMinXSize, double theMinYSize, double theMaxXSize, double theMaxYSize, double sizeFactor)
{
	NFmiRect stationRect(CurrentStationRect(sizeFactor));
	NFmiPoint size(stationRect.Size());
	if(size.X() < theMinXSize || size.Y() < theMinYSize)
	{
		stationRect.Size(NFmiPoint(theMinXSize, theMinYSize));
	}
	if(size.X() > theMaxXSize || size.Y() > theMaxYSize)
	{
		stationRect.Size(NFmiPoint(theMaxXSize, theMaxYSize));
	}
	return stationRect;
}

// used with mouse selection (don't have to draw screen all over again)
void NFmiStationView::DrawInvertStationRect(NFmiRect &theRect)
{
	theRect.Center(CurrentStationPosition()); // huom! theRect saa muuttua ihan rauhassa
	NFmiRectangle tmp(theRect,
						0,
						&itsDrawingEnvironment);
	itsToolBox->Convert(&tmp);
}

static void SetCRectCenter(CRect &theRect, const CPoint &theCenterPoint)
{
	theRect.MoveToXY(theCenterPoint.x - theRect.Width()/2, theCenterPoint.y - theRect.Height()/2);
}

bool NFmiStationView::CanToolmasterBeUsed(void)
{
	if(itsCtrlViewDocumentInterface->IsToolMasterAvailable() == false)
		return false;

    boost::shared_ptr<NFmiArea> infoArea(itsInfo->Area()->Clone());
	return NFmiQueryDataUtil::AreAreasSameKind(itsArea.get(), infoArea.get());
}

void NFmiStationView::DrawWithIsolineView(const NFmiDataMatrix<float> &theMatrix, boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
	// Käytetään hiladatan piirrossa isolineView-luokkaa
	NFmiIsoLineView isolineView(itsMapViewDescTopIndex, GetArea(), itsToolBox, theDrawParam, static_cast<FmiParameterName>(NFmiInfoData::kFmiSpMatrixDataDraw), itsObjectOffSet, itsObjectSize, itsViewGridRowNumber, itsViewGridColumnNumber);
	isolineView.Time(itsTime);
	isolineView.SpecialMatrixData(theMatrix);
	isolineView.Draw(itsToolBox);
}

static NFmiRect CalcWantedDisplayRect(CtrlViewDocumentInterface *theCtrlViewDocumentInterface, NFmiToolBox *theToolBox, int theMapViewDescTopIndex, double wantedDisplayRectSizeInMM)
{
    int pixelSizeX = boost::math::iround(wantedDisplayRectSizeInMM * theCtrlViewDocumentInterface->GetGraphicalInfo(theMapViewDescTopIndex).itsPixelsPerMM_x);
    int pixelSizeY = boost::math::iround(wantedDisplayRectSizeInMM * theCtrlViewDocumentInterface->GetGraphicalInfo(theMapViewDescTopIndex).itsPixelsPerMM_y);
    double xWidth = theToolBox->SX(pixelSizeX);
    double yWidth = theToolBox->SY(pixelSizeY);
    return NFmiRect(0, 0, xWidth, yWidth);
}

static CPoint CalcWantedPixelSize(CtrlViewDocumentInterface *theCtrlViewDocumentInterface, NFmiToolBox *theToolBox, int theMapViewDescTopIndex, double wantedSizeInMM)
{
    int pixelSizeX = boost::math::iround(wantedSizeInMM * theCtrlViewDocumentInterface->GetGraphicalInfo(theMapViewDescTopIndex).itsPixelsPerMM_x);
    int pixelSizeY = boost::math::iround(wantedSizeInMM * theCtrlViewDocumentInterface->GetGraphicalInfo(theMapViewDescTopIndex).itsPixelsPerMM_y);
    return CPoint(pixelSizeX, pixelSizeY);
}

void NFmiStationView::DrawMouseSelectionMarker(const NFmiPoint &theLatlon, bool fDrawBiggerMarker, CRect &theBiggerBaseMfcRect, CRect &theSmallerBaseMfcRect, CDC *theUsedDC)
{
    if(itsArea->IsInside(theLatlon))
    {
        CRect mfcRect2;
        CPoint absolutePoint;
        itsToolBox->ConvertPoint(LatLonToViewPoint(theLatlon), absolutePoint);
        if(fDrawBiggerMarker)
        {
            ::SetCRectCenter(theBiggerBaseMfcRect, absolutePoint);
            theUsedDC->InvertRect(&theBiggerBaseMfcRect);
            mfcRect2.SetRect(theBiggerBaseMfcRect.left + 1, theBiggerBaseMfcRect.top + 1, theBiggerBaseMfcRect.right - 1, theBiggerBaseMfcRect.bottom - 1);
        }
        else
        {
            ::SetCRectCenter(theSmallerBaseMfcRect, absolutePoint);
            theUsedDC->InvertRect(&theSmallerBaseMfcRect);
            mfcRect2.SetRect(theSmallerBaseMfcRect.left + 1, theSmallerBaseMfcRect.top + 1, theSmallerBaseMfcRect.right - 1, theSmallerBaseMfcRect.bottom - 1);
        }
        // piirretään pienempi laatikko sisään, jotta saadaan ei fillattu laatikko
        theUsedDC->InvertRect(&mfcRect2);
    }
}

//------------------------------------------------------------------------
//	DrawAllSelectedDisplayedStationsWithInvertStationRect
//------------------------------------------------------------------------
// jos data on editoitavaa, funktio piirtää valitut/näytettävät asemat ja palauttaa true,
// jolloin kutsuja voi päätellä, että ei tarvitse enää piirtää valittuja asemia uudelleen
// mutta jos data ei ole editoitavaa, ei piirretä asemia, ja palautetaan false, että
// kutsuja tietää, että asemanäyttöluokkia voidaan käydä edelleen läpi ja etsiä editoitavaa dataa.
bool NFmiStationView::DrawAllSelectedStationsWithInvertStationRect(unsigned long theMaskType)
{
	// Muutin koodin niin että se tehdään aina editoitavalle datalle, tällöin
	// valitut pisteet piirretään, vaikka ruudulla ei olisikaan editoitavaa dataa.
	itsInfo = itsCtrlViewDocumentInterface->EditedSmartInfo();
	if(!itsInfo)
		return false;
	NFmiInfoData::Type dataType = itsInfo->DataType();
	if(dataType == NFmiInfoData::kEditable)
	{
        ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsArea->XYArea());
		const int kSelectionRectPixelSize = 3;
		double selectionRectXSize = itsToolBox->SX(kSelectionRectPixelSize);
		double selectionRectYSize = itsToolBox->SY(kSelectionRectPixelSize);
		CalculateGeneralStationRect();
        NFmiRect displayRect = ::CalcWantedDisplayRect(itsCtrlViewDocumentInterface, itsToolBox, itsMapViewDescTopIndex, 2.0);
        CPoint firstSelectedPointPixelSize = ::CalcWantedPixelSize(itsCtrlViewDocumentInterface, itsToolBox, itsMapViewDescTopIndex, 1.6);
        EditedInfoMaskHandler editedInfoMaskHandler(itsInfo, theMaskType);
		itsDrawingEnvironment.EnableInvert();
		CDC *usedDC = itsToolBox->GetDC();
		if(theMaskType == NFmiMetEditorTypes::kFmiSelectionMask)
		{
			if(itsInfo->IsGrid() && CtrlViewFastInfoFunctions::GetMaskedCount(itsInfo, NFmiMetEditorTypes::kFmiSelectionMask, itsCtrlViewDocumentInterface->AllowRightClickDisplaySelection()) > itsCtrlViewDocumentInterface->SelectedGridPointLimit() && CanToolmasterBeUsed())
			{ 
//static int counter = 0;
//TRACE(_T("NFmiStationView::DrawAllSelectedStationsWithInvertStationRect %d\n"), counter++);
				// jos valittuja pisteitä on yli tietyn määrän ja toolmaster piirto on käytössä, piirretään valittujen pisteiden maski toolmasterilla
				// koska sen piirto on nopeampaa ja isojen alueiden ollessa kyseessä hienomman näköinen.
				unsigned long locationIndex = 0;
				unsigned long gridSizeX = itsInfo->GridXNumber();
				// Rakennetaan hila, missä ei valitut pisteet ovat 0:n arvoisia ja valitut 1:n arvoisia.
				NFmiDataMatrix<float> selectionMatrix(gridSizeX, itsInfo->GridYNumber(), 0.5f);
				for(itsInfo->ResetLocation(); itsInfo->NextLocation(); )
				{
					locationIndex = itsInfo->LocationIndex();
					selectionMatrix[locationIndex % gridSizeX][locationIndex / gridSizeX] = 1.2f;
				}

				DrawWithIsolineView(selectionMatrix, itsCtrlViewDocumentInterface->GetSelectedGridPointDrawParam());
			}
            else
            {
                // Valittujen pisteiden piirto on hidastunut invertRect:illä kun siirrytään XP:stä Win7:aan.
                // tästä syystä piirretään valitun pisteen ympyrä nyt kahdella CDC:n InvertRect -kutsulla toolboxin 4:n sijasta.
                // Lisäksi ohitetaan toolbox tässä kokonaan. Valitun pisteen muoto ja koko muuttuivat hieman. Koko 4 -> 3. ja muoto
                // 'ympyrästä' neliöksi.
                // Omalla koneella vanha piirsi editoidun datan valitut pisteet 1.5 sekunnissa ja uudella piirrolla 0.6 sekunnissa.
                CRect mfcRect(0, 0, kSelectionRectPixelSize, kSelectionRectPixelSize);
                CRect firstLocationMfcRect(0, 0, firstSelectedPointPixelSize.x, firstSelectedPointPixelSize.y); // 1. valittu piste piirretään tuplakokoisena, koska se osoittaa myös aikasarjassa piirrettävän valitun pisteen paikan
                if(itsCtrlViewDocumentInterface->IsPreciseTimeSerialLatlonPointUsed())
                    DrawMouseSelectionMarker(itsCtrlViewDocumentInterface->PreciseTimeSerialLatlonPoint(), true, firstLocationMfcRect, mfcRect, usedDC);
                else
                {
                    int counter = 0;
                    for(itsInfo->ResetLocation(); itsInfo->NextLocation();)
                    {
                        DrawMouseSelectionMarker(itsInfo->LatLon(), (counter == 0), firstLocationMfcRect, mfcRect, usedDC);
                        counter++;
                    }
                }
            }
		}
		else if(theMaskType == NFmiMetEditorTypes::kFmiDisplayedMask)
		{
            if(itsCtrlViewDocumentInterface->AllowRightClickDisplaySelection())
            {
				itsDrawingEnvironment.EnableFill();
                NFmiRect rec1 = displayRect; //(CalcInvertStationRectSize(minSelectedRectXSize, minSelectedRectYSize, maxSelectedRectXSize, maxSelectedRectYSize, 0.1));
                for(itsInfo->ResetLocation(); itsInfo->NextLocation();)
                    DrawInvertStationRect(rec1);
            }
		}

		// piirrretään vielä editoidun alueen ulkopuolelta mahd. valittu piste mutta vain selectionMask yhteydessä
		if(theMaskType == NFmiMetEditorTypes::kFmiSelectionMask)
		{
			if(itsCtrlViewDocumentInterface->OutOfEditedAreaTimeSerialPoint() != NFmiPoint(kFloatMissing, kFloatMissing))
			{
				NFmiRect rec1 = displayRect; //(CalcInvertStationRectSize(minSelectedRectXSize, minSelectedRectYSize, maxSelectedRectXSize, maxSelectedRectYSize, 0.1));
				rec1.Center(LatLonToViewPoint(itsCtrlViewDocumentInterface->OutOfEditedAreaTimeSerialPoint()));
				NFmiRectangle tmp(rec1, 0, &itsDrawingEnvironment);
				itsToolBox->Convert(&tmp);
			}
		}
		itsDrawingEnvironment.DisableInvert();
		return true;
	}
	return false;
}

NFmiPoint NFmiStationView::LatLonToViewPoint(const NFmiPoint& theLatLon) const
{
	return itsArea->ToXY(theLatLon);
}

NFmiPoint NFmiStationView::ViewPointToLatLon(const NFmiPoint& theViewPoint) const
{
	return itsArea->ToLatLon(theViewPoint);
}

// HUOM!! lähes sama koodi löytyy myös NFmiStationViewHandler:ista, koska haluttiin
// että paikkoja voi valita esim. luotaus näyttöön, vaikka karttanäytöllä ei ole mitään.
// Mutta tätä kutsutaan jos on yksikin parametri kartalla (täällä on valittujen pisteiden piirto).
bool NFmiStationView::LeftButtonUp(const NFmiPoint& thePlace, unsigned long theKey)
{
	// Muutin koodia niin, että kartalle hiiren klikkaus vaikuttaa aina, vaikka kyseessä olisi ei editoitava data.
	// Muutokset tehdään aina editoitavaan dataan.
	boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->EditedSmartInfo();
	if(info)
	{
		if(itsCtrlViewDocumentInterface->ModifyToolMode() != CtrlViewUtils::kFmiEditorModifyToolModeBrush && !itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode()) // siveltimen kanssa ei voi valita asemia???
		{
			dynamic_cast<NFmiSmartInfo*>(info.get())->LocationSelectionSnapShot();

			NFmiPoint latlon = itsArea->ToLatLon(thePlace);
			if(theKey & kCtrlKey)
				SelectLocations(info, latlon, kFmiSelectionCombineAdd, NFmiMetEditorTypes::kFmiSelectionMask, false);
			else if(theKey & kShiftKey)
				SelectLocations(info, latlon, kFmiSelectionCombineRemove, NFmiMetEditorTypes::kFmiSelectionMask, false);
			else
				SelectLocations(info, latlon, kFmiSelectionCombineClearFirst, NFmiMetEditorTypes::kFmiSelectionMask, true);
		}
		else if(itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode())
			SelectControlPointLocation(info, kFmiSelectionCombineClearFirst, NFmiMetEditorTypes::kFmiSelectionMask);

		// piirretään tietyissä tilanteissa valitut pisteet myös ei oikean yläkulman kartta ruutuun, mutta ei piirretä tässä oikeaan yläkulmaan koska sinne piirretään muutenkin
		if(itsCtrlViewDocumentInterface->DrawSelectionOnThisView())
		{
			DrawAllSelectedStationsWithInvertStationRect(NFmiMetEditorTypes::kFmiSelectionMask);
		}
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------------------
//	RightButtonUp					M.K. 9.4.99
//---------------------------------------------------------------------------------------
// HUOM!! lähes sama koodi löytyy myös NFmiStationViewHandler:ista, koska haluttiin
// että paikkoja voi valita esim. luotaus näyttöön, vaikka karttanäytöllä ei ole mitään.
// Mutta tätä kutsutaan jos on yksikin parametri kartalla (täällä on valittujen pisteiden piirto).
bool NFmiStationView::RightButtonUp(const NFmiPoint& thePlace, unsigned long theKey)
{
	// Muutin koodia niin, että kartalle hiiren klikkaus vaikuttaa aina, vaikka kyseessä olisi ei editoitava data.
	// Muutokset tehdään aina editoitavaan dataan.
	boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->EditedSmartInfo();
	if(info)
	{
		if((theKey & kCtrlKey) && (theKey & kShiftKey))
		{
			// ctrl+shift+right-mouseclick:in avulla voidaan valita aktiivinen näyttörivi ja aika ilman, että valinnat muuttuvat
		}
		else if(itsCtrlViewDocumentInterface->ModifyToolMode() != CtrlViewUtils::kFmiEditorModifyToolModeBrush) // siveltimen kanssa ei voi valita asemia???
		{
            dynamic_cast<NFmiSmartInfo*>(info.get())->LocationSelectionSnapShot();

            NFmiPoint latlon = itsArea->ToLatLon(thePlace);
            if(theKey & kCtrlKey)
                SelectLocations(info, latlon, kFmiSelectionCombineAdd, NFmiMetEditorTypes::kFmiDisplayedMask, false);
            else if(theKey & kShiftKey)
                SelectLocations(info, latlon, kFmiSelectionCombineRemove, NFmiMetEditorTypes::kFmiDisplayedMask, false);
            else
                SelectLocations(info, latlon, kFmiSelectionCombineClearFirst, NFmiMetEditorTypes::kFmiDisplayedMask, true);
		}
	}
	return true;
}

// kutsutaan kerran Draw():ssa
// HUOM!! Tämä metodi pitäisi järkeistää niin, että jokaisessa ehtohaarassa lasketaan size ja
// sitä käytetään lopussa laatikon laskuissa yhteisellä tavalla
void NFmiStationView::CalculateGeneralStationRect(void)
{ // laskee vain laatikon koon, tätä laatikkoa siirtämällä saadaan kulloisenkin aseman oma laatikko
	NFmiPoint place; // just 0,0 place
	// float size = 0.;
	double sizeX = 0.;
	double sizeY = 0.;
	{
        // generaldatadociin pitäisi laittaa sellainen metodi, joka palauttaa zoomatun alueen suhteellisen koon
        // jotta saataisiin tästä maphandlerin käyttö pois ja include-lause pois tiedoston alusta
        auto mapHandlerInterface = GetMapHandlerInterface();
        NFmiRect zoomedAreaRect = mapHandlerInterface->TotalArea()->XYArea(mapHandlerInterface->Area().get());
		double sizeFactor = 1.;
		if(zoomedAreaRect.Width() < 0.7 || zoomedAreaRect.Height() < 0.7)
		{// zoomausta on tehty tarpeeksi perusalueesta, isonnetaan symboleja
			sizeFactor = 1/((zoomedAreaRect.Width() + zoomedAreaRect.Height())/2);

            auto viewGridSize = itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex);
			// line function is y = ax + b
			// we calculate here new x value: x = (y-b)/a
			// if two points make line, then a and b are the following
			// b = (y2*x1-y1*x2)/(x1-x2)
			// a = (y1-b)/x1
			double x1 = 3.5
				 , x2 = 12
				 , y1 = 2
				 , y2 = 25
				 , y = viewGridSize.X() * viewGridSize.Y();
			double b = (y2 * x1 - y1 * x2) / (x1 - x2);
			double a = (y1 - b) / x1;
			double newX = (y-b)/a;


			double maxSizeFactor = FmiMin(FmiMax(newX, 1.3), 2.5);
			if(y == 1)
				maxSizeFactor = 1.3;
			sizeFactor = FmiMin(sizeFactor, maxSizeFactor);
		}

		sizeX = float(GetFrame().Width()/(itsObjectSize.X()*2.));
		sizeY = float(GetFrame().Height()/(itsObjectSize.Y()*2.));
		sizeX *= sizeFactor;
		sizeY *= sizeFactor;
		auto relativeOffsetX = itsToolBox->SX(itsToolBox->HX(sizeX));
		auto relativeOffsetY = itsToolBox->SY(itsToolBox->HY(sizeY));
		NFmiPoint topLeft(place.X() - relativeOffsetX, place.Y() - relativeOffsetY);
		NFmiPoint bottomRight(place.X() + relativeOffsetX, place.Y() + relativeOffsetY);
		itsGeneralStationRect = NFmiRect(topLeft,bottomRight);
	}
}

const NFmiRect& NFmiStationView::GeneralStationRect() const
{
	return itsGeneralStationRect;
}

NFmiRect NFmiStationView::CurrentStationRect() const
{
	NFmiRect rect(GeneralStationRect());
	rect.Center(CurrentStationPosition());
	return rect;
}

NFmiRect NFmiStationView::CurrentStationRect(double theSizeFactor)
{
	NFmiRect rect(GeneralStationRect());
	NFmiPoint size(rect.Size().X()*theSizeFactor, rect.Size().Y()*theSizeFactor); // väliaikaisesti pienempi recti
	rect.Size(size);
	rect.Center(CurrentStationPosition());
	return rect;
}

NFmiRect NFmiStationView::CurrentDataRect() const
{
	NFmiRect stationRect(CurrentStationRect());
	NFmiRect dataRect(stationRect.Left()
					 ,stationRect.Top()
					 ,stationRect.Left() + stationRect.Width() * itsDrawParam->OnlyOneSymbolRelativeSize().X()
					 ,stationRect.Top() + stationRect.Height() * itsDrawParam->OnlyOneSymbolRelativeSize().Y());
	NFmiPoint center(stationRect.Center());

    center = NFmiPoint(center.X() + stationRect.Width() * itsDrawParam->OnlyOneSymbolRelativePositionOffset().X()		// tähän voisi tulla onlyonesymbol-offset, mikä on drawparamin ominaisuus
        , center.Y() + stationRect.Height() * itsDrawParam->OnlyOneSymbolRelativePositionOffset().Y());  // LISÄKSI: voisi datarectin koon laittaa riippuvaiseksi onlyonesymbolsize:sta, mikä on myös drawparamilla
	dataRect.Center(center);
	return dataRect;
}

NFmiPoint NFmiStationView::CurrentStationPosition() const
{
	NFmiPoint xy(LatLonToViewPoint(CurrentLatLon()));
	return xy;
}

float NFmiStationView::GetSynopValueFromQ2Archive(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	if(theInfo)
	{
		itsNearestTooltipLocation = *(theInfo->Location());
		if(GetQ2SynopData(theInfo->Location()->GetIdent()))
			return itsSynopDataValuesFromQ2[0][0]; // jos haku onnistui, on data 1x1 taulukon ainoassa alkiossa
		else
			return kFloatMissing;
	}
	return kFloatMissing;
}

// Oletus: theInfo ei ole nullptr
// Oletus 2: annetun theInfo:n aikainterpolaatio arvot ovat samoja kuin NFmiStationView-luokan itsInfo:n vastaavat.
float NFmiStationView::CalcTimeInterpolatedValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiMetTime &theTime)
{
    if(fDoTimeInterpolation && itsTimeInterpolationRangeInMinutes == 0)
        return kFloatMissing;
    else
    {
        auto paramId = itsDrawParam->Param().GetParamIdent();
        if(metaWindParamUsage.ParamNeedsMetaCalculations(paramId))
        {
            return NFmiFastInfoUtils::GetMetaWindValue(itsInfo, theTime, metaWindParamUsage, paramId);
        }
        else
        {
            float currentValue = theInfo->InterpolatedValue(theTime, itsTimeInterpolationRangeInMinutes);
            if(currentValue == kFloatMissing && fAllowNearestTimeInterpolation)
            {
                auto oldTimeIndex = theInfo->TimeIndex();
                if(theInfo->FindNearestTime(theTime, kCenter, itsTimeInterpolationRangeInMinutes))
                {
                    currentValue = theInfo->FloatValue();
                }
                theInfo->TimeIndex(oldTimeIndex);
            }
            return currentValue;
        }
    }
}

float NFmiStationView::GetSynopDataValueFromq2()
{
    StationIdSeekContainer::iterator it = itsSynopDataFromQ2StationIndexies.find(itsInfo->Location()->GetIdent());
    if(it != itsSynopDataFromQ2StationIndexies.end())
        return itsSynopDataValuesFromQ2[0][(*it).second];
    else
        return kFloatMissing;
}

float NFmiStationView::GetMacroParamSpecialCalculationsValue()
{
    unsigned long locIndex = itsInfo->LocationIndex();
    unsigned long columnIndex = locIndex % itsInfo->GridXNumber();
    unsigned long rowIndex = locIndex / itsInfo->GridXNumber();
    if(columnIndex < itsMacroParamSpecialCalculationsValues.NX() && rowIndex < itsMacroParamSpecialCalculationsValues.NY())
        return itsMacroParamSpecialCalculationsValues[columnIndex][rowIndex];
    else
        return kFloatMissing;
}

float NFmiStationView::GetDifferenceToOriginalDataValue()
{
    float currentValue = CalcTimeInterpolatedValue(itsInfo, itsTime);
    float origValue = currentValue;
    if(itsOriginalDataInfo)
    {
        itsOriginalDataInfo->LocationIndex(itsInfo->LocationIndex());
        origValue = CalcTimeInterpolatedValue(itsOriginalDataInfo, itsTime);
    }

    if(currentValue == kFloatMissing || origValue == kFloatMissing)
        return kFloatMissing;
    else
        return currentValue - origValue;
}

float NFmiStationView::GetValueFromLocalInfo()
{
    if(fDoTimeInterpolation)
    {
        auto usedTime = NFmiFastInfoUtils::GetUsedTimeIfModelClimatologyData(itsInfo, itsTime);
        return CalcTimeInterpolatedValue(itsInfo, usedTime);
    }
    else
    {
        auto paramId = itsDrawParam->Param().GetParamIdent();
        if(metaWindParamUsage.ParamNeedsMetaCalculations(paramId))
        {
            return NFmiFastInfoUtils::GetMetaWindValue(itsInfo, metaWindParamUsage, paramId);
        }
        else
            return itsInfo->FloatValue();
    }
}

// tämä hakee näytettävän datan riippuen asetuksista
float NFmiStationView::ViewFloatValue(bool )
{
    if(itsInfo)
        NFmiFastInfoUtils::SetSoundingDataLevel(itsDrawParam->Level(), *itsInfo);  // pitää varmistaa että jos kyse on sounding datasta, että level on kohdallaan
    if(fGetSynopDataFromQ2)
    {
        return GetSynopDataValueFromq2();
    }
    else if(fGetCurrentDataFromQ2Server)
    {
        return itsQ2ServerDataValues.InterpolatedValue(LatLonToViewPoint(itsInfo->LatLon()), itsArea->XYArea(), itsParamId);
    }
    else if(fUseMacroParamSpecialCalculations)
    {
        return GetMacroParamSpecialCalculationsValue();
    }
    else if(itsDrawParam->ShowDifferenceToOriginalData())
    {
        return GetDifferenceToOriginalDataValue();
    }
    else if(itsInfo)
    {
        return GetValueFromLocalInfo();
    }
    else
        return kFloatMissing;
}

static bool AreMatricesEqual(const NFmiDataMatrix<float> & m1, const NFmiDataMatrix<float> & m2)
{
	if(m1.NX() == m2.NX() && m1.NY() == m2.NY())
	{
		for (size_t j = 0; j < m1.NY() ; j++)
		{
			for (size_t i = 0; i < m1.NX(); i++)
				if(m1[i][j] != m2[i][j])
					return false;
		}
		return true;
	}
	return false;
}

// Resoluutio datalla tarkoitetaan tässä joko FixedBaseData:a tai Resolution dataa.
// FixedBaseDatalla on korkeampi prioriteetti.
static boost::shared_ptr<NFmiFastQueryInfo> CalcPossibleResolutionInfoFromMacroParam(TimeSerialModificationDataInterface& theAdapter, boost::shared_ptr<NFmiDrawParam>& theDrawParam, int theMapViewDescTopIndex, boost::shared_ptr<NFmiFastQueryInfo>& possibleSpacedOutMacroInfo, const NFmiPoint& spaceOutSkipFactors)
{
	float value = kFloatMissing;
	NFmiSmartToolModifier smartToolModifier(theAdapter.InfoOrganizer());
	try // ensin tulkitaan macro
	{
		FmiModifyEditdData::InitializeSmartToolModifierForMacroParam(smartToolModifier, theAdapter, theDrawParam, theMapViewDescTopIndex, possibleSpacedOutMacroInfo, false, spaceOutSkipFactors);
		auto possibleFixedBaseMacroParamData = smartToolModifier.PossibleFixedBaseMacroParamData();
		if(possibleFixedBaseMacroParamData)
			return possibleFixedBaseMacroParamData;
		if(smartToolModifier.ExtraMacroParamData().UseSpecialResolution())
			return smartToolModifier.UsedMacroParamData();
	}
	catch(...)
	{
	}
	return nullptr;
}

static std::string DownSizeGridString(const NFmiPoint& gridSize)
{
	std::string str = std::to_string(boost::math::iround(gridSize.X()));
	str += "x";
	str += std::to_string(boost::math::iround(gridSize.Y()));

	return str;
}

static void DoDataDownSizeLogging(NFmiCtrlView* view, const std::string& operationName, const NFmiPoint& originalSize, const NFmiPoint& newSize)
{
	if(CatLog::doTraceLevelLogging())
	{
		std::string finalMessage = "Down sizing data grid when ";
		finalMessage += operationName;
		finalMessage += "': ";
		finalMessage += ::DownSizeGridString(originalSize);
		finalMessage += " => ";
		finalMessage += ::DownSizeGridString(newSize);
		CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(finalMessage, view);
	}
}

static void SetupIsolineData(const boost::shared_ptr<NFmiFastQueryInfo>& possibleMacroParamResolutionInfo, NFmiIsoLineData& theIsoLineDataOut)
{
	if(possibleMacroParamResolutionInfo)
	{
		theIsoLineDataOut.itsInfo = possibleMacroParamResolutionInfo;
		theIsoLineDataOut.itsXNumber = possibleMacroParamResolutionInfo->GridXNumber();
		theIsoLineDataOut.itsYNumber = possibleMacroParamResolutionInfo->GridYNumber();
	}
}

bool NFmiStationView::IsMacroParamIsolineDataDownSized(NFmiPoint& newGridSizeOut, boost::shared_ptr<NFmiFastQueryInfo>& possibleMacroParamResolutionInfoOut)
{
	if(itsDrawParam)
	{
		boost::shared_ptr<NFmiFastQueryInfo> possibleSpaceOutInfoIsEmpty;
		possibleMacroParamResolutionInfoOut = CalcPossibleResolutionInfoFromMacroParam(itsCtrlViewDocumentInterface->GenDocDataAdapter(), itsDrawParam, itsMapViewDescTopIndex, possibleSpaceOutInfoIsEmpty, CalcUsedSpaceOutFactors());
		if(possibleMacroParamResolutionInfoOut)
		{
			NFmiIsoLineData isoLineData;
			::SetupIsolineData(possibleMacroParamResolutionInfoOut, isoLineData);
			NFmiPoint pixelToGridRatio = CalcPixelToGridRatio(isoLineData, NFmiRect(0, 0, 1, 1));
			NFmiPoint downSizeFactor;
			if(IsolineDataDownSizingNeeded(isoLineData, pixelToGridRatio, downSizeFactor, itsDrawParam))
			{
				// Tehdään tässä floor, koska muuten myöhemmin (tm_utils\source\ToolMasterDrawingFunctions.cpp:ssä) saatetaan luulla että tarvitsee harventaa lisää
				auto newSizeX = std::floor(isoLineData.itsXNumber / downSizeFactor.X());
				auto newSizeY = std::floor(isoLineData.itsYNumber / downSizeFactor.Y());
				newGridSizeOut = NFmiPoint(newSizeX, newSizeY);
				::DoDataDownSizeLogging(this, "calculating macroParam isoline values", NFmiPoint(isoLineData.itsXNumber, isoLineData.itsYNumber), newGridSizeOut);
				return true;
			}
		}
	}
	return false;
}

bool NFmiStationView::IsMacroParamContourDataDownSized(const boost::shared_ptr<NFmiFastQueryInfo>& possibleMacroParamResolutionInfo, NFmiPoint& newGridSizeOut)
{
	if(itsDrawParam && possibleMacroParamResolutionInfo)
	{
		// Oikeastaan kFmiColorContourIsoLineView on jo tarkastettu edella isoviiva tapauksien kanssa ja 
		// siellä on suuremmat harvennuskertoimet, mutta tehdään se vielä tässä varmuuden vuoksi.
		auto style = itsDrawParam->GridDataPresentationStyle();
		if(style == NFmiMetEditorTypes::View::kFmiColorContourView || style == NFmiMetEditorTypes::View::kFmiQuickColorContourView || style == NFmiMetEditorTypes::View::kFmiColorContourIsoLineView)
		{
			NFmiIsoLineData isoLineData;
			::SetupIsolineData(possibleMacroParamResolutionInfo, isoLineData);
			NFmiPoint pixelToGridRatio = CalcPixelToGridRatio(isoLineData, NFmiRect(0, 0, 1, 1));
			NFmiPoint downSizeFactor;
			if(IsDownSizingNeeded(pixelToGridRatio, GetVisualizationSettings().criticalPixelToGridPointRatioLimitForContours(), downSizeFactor))
			{
				// Tehdään tässä ceil, koska muuten myöhemmin (tm_utils\source\ToolMasterDrawingFunctions.cpp:ssä) luullaan että ei tarvitse tarvitse laittaa quick-contour optiota päälle ollenkaan
				auto newSizeX = std::ceil(isoLineData.itsXNumber / downSizeFactor.X());
				auto newSizeY = std::ceil(isoLineData.itsYNumber / downSizeFactor.Y());
				newGridSizeOut = NFmiPoint(newSizeX, newSizeY);
				::DoDataDownSizeLogging(this, "calculating macroParam contour values", NFmiPoint(isoLineData.itsXNumber, isoLineData.itsYNumber), newGridSizeOut);
				return true;
			}
		}
	}
	return false;
}

// Lasketaan käytetyn datan hilan ja näytön pikseleiden suhdeluku x- ja y-suunnassa.
// Jos kyse ei hiladatasta, tai esim. makrosta (smarttool/q3), lasketaan isolinedatan ja arean avulla kertoimet.
// Jos x/y arvo on 0, jätetään tämä huomiotta.
NFmiPoint NFmiStationView::CalcPixelToGridRatio(NFmiIsoLineData& theIsoLineData, const NFmiRect& zoomedAreaRect)
{
	auto areasAreSameKind = NFmiQueryDataUtil::AreAreasSameKind(itsArea.get(), theIsoLineData.itsInfo->Area());
	NFmiPoint grid2PixelRatio(0, 0);
	if((!areasAreSameKind || theIsoLineData.fUseOriginalDataInPixelToGridRatioCalculations) && theIsoLineData.itsInfo && theIsoLineData.itsInfo->Grid())
	{
		NFmiFastQueryInfo& usedInfo = *(theIsoLineData.itsInfo);
		usedInfo.FirstLocation(); // laitetaan 1. hilapiste eli vasen alanurkka kohdalle

		NFmiPoint latlon1(usedInfo.LatLon());
		NFmiPoint latlon2(usedInfo.PeekLocationLatLon(1, 0));
		NFmiPoint latlon3(usedInfo.PeekLocationLatLon(0, 1));
		NFmiPoint p1(LatLonToViewPoint(latlon1));
		NFmiPoint p2(LatLonToViewPoint(latlon2));
		NFmiPoint p3(LatLonToViewPoint(latlon3));
		// 3. Calc relative dist of two parallel neighbor grid point in x dir
		double relGridPoinWidth = ::fabs(p1.X() - p2.X());
		double onePixelWidth = itsToolBox->SX(1);
		grid2PixelRatio.X(relGridPoinWidth / onePixelWidth);
		// 4. Calc relative dist of two vertical neighbor grid point in y dir
		double relGridPoinHeight = ::fabs(p1.Y() - p3.Y());
		double onePixelHeigth = itsToolBox->SY(1);
		grid2PixelRatio.Y(relGridPoinHeight / onePixelHeigth);
	}
	else
	{
		double relGridPoinWidth = (zoomedAreaRect.Width() * itsArea->Width()) / (theIsoLineData.itsXNumber - 1.0);
		grid2PixelRatio.X(itsToolBox->HXs(relGridPoinWidth));
		double relGridPoinHeight = (zoomedAreaRect.Height() * itsArea->Height()) / (theIsoLineData.itsYNumber - 1.0);
		grid2PixelRatio.Y(itsToolBox->HYs(relGridPoinHeight));
	}

	return grid2PixelRatio;
}

// 'Probing' macroParam data on alueeltaan hieman pienempi kuin kartan alue, tällöin reunoille ei toivottavasti tule mitään erikoisia
// arvoja kuten puuttuvaa tai 0:aa. Hilana on 4x4 eli 16 testipistettä, jolla saadaan aavistus, kuinka monta numeroa on luku 
// tekstissä keskimäärin. Sen avulla voidaan laskea lopullisen harvennetun datan symboli tiheys ruudulla.
static boost::shared_ptr<NFmiFastQueryInfo> CreateProbingMacroParamData(boost::shared_ptr<NFmiArea> &mapArea)
{
    // Lasketaan uudet kulmapisteet kolmanneksen päähän reunoista ja tehdään siihen 4x4 hila.
    if(mapArea)
    {
        auto bottomLeftXyPoint = mapArea->BottomLeft();
        auto topRightXyPoint = mapArea->TopRight();
        auto xShift = (topRightXyPoint.X() - bottomLeftXyPoint.X()) / 5.;
        // Huom! xy-maailma on y-suunnassa käännetty eli origo on karttanäytön yläosassa ja kasvaa alaspäin.
        auto yShift = (bottomLeftXyPoint.Y() - topRightXyPoint.Y()) / 5.;
        NFmiPoint newBottomLeftXy(bottomLeftXyPoint.X() + xShift, bottomLeftXyPoint.Y() - yShift);
        NFmiPoint newTopRightXy(topRightXyPoint.X() - xShift, topRightXyPoint.Y() + yShift);
        boost::shared_ptr<NFmiArea> newArea(mapArea->CreateNewArea(NFmiRect(newBottomLeftXy, newTopRightXy)));
        return NFmiInfoOrganizer::CreateNewMacroParamData_checkedInput(4, 4, NFmiInfoData::kMacroParam, newArea.get());
    }

    return boost::shared_ptr<NFmiFastQueryInfo>();
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiStationView::CreateNewResizedMacroParamData(const NFmiPoint &newGridSize, const NFmiArea* usedArea)
{
	fUseAlReadySpacedOutData = true;
	int gridSizeX = boost::math::iround(newGridSize.X());
	int gridSizeY = boost::math::iround(newGridSize.Y());
	return NFmiInfoOrganizer::CreateNewMacroParamData_checkedInput(gridSizeX, gridSizeY, NFmiInfoData::kMacroParam, usedArea);
}

static bool IsSymbolDrawing(boost::shared_ptr<NFmiDrawParam>& theDrawParam)
{
	if(theDrawParam)
	{
		auto gridDataDrawStyle = theDrawParam->GridDataPresentationStyle();
		// Tyylit 2-5 ovat isoline, contour, isoline+contour, quickcontour
		if(gridDataDrawStyle >= NFmiMetEditorTypes::View::kFmiIsoLineView && gridDataDrawStyle <= NFmiMetEditorTypes::View::kFmiQuickColorContourView)
			return false;
		else
			return true;
	}
	return false;
}

bool NFmiStationView::IsGridDataDrawnWithSpaceOutSymbols()
{
	if(itsDrawParam)
	{
		if(!::IsSymbolDrawing(itsDrawParam))
			return false;
		else
		{
			// Jos macroParam laskee ns. 'harvaa' dataa ja se piirretään harvan-datan symboli piirrolla, ei pidä yrittää optimoida hilan kokoa
			if(itsDrawParam->DoSparseSymbolVisualization())
				return false;
            if(itsCtrlViewDocumentInterface->Registry_SpacingOutFactor(itsMapViewDescTopIndex) != 0)
                return true;
		}
	}
	return false;
}

void NFmiStationView::UpdateOptimizedVisualizationMacroParamData()
{
	auto areaBasedGridSize = GetVisualizationSettings().calcAreaGridSize(*itsArea, CalcViewGridSize());
	itsCtrlViewDocumentInterface->InfoOrganizer()->UpdateOptimizedVisualizationMacroParamDataSize(boost::math::iround(areaBasedGridSize.X()), boost::math::iround(areaBasedGridSize.Y()), itsArea);
}

// Tähän ei oteta mukaan VisualizationOptimization hilakokolaskuja, koska tässä lasketaan
// jo harvennettuun laskentahilaan mahdollisen symbolipiirron takia.
boost::shared_ptr<NFmiFastQueryInfo> NFmiStationView::CreatePossibleSpaceOutMacroParamData()
{
	// Huom! Tehdään valitettava kaksoistoiminto, mutta päivitetään aluksi myös OptimizedVisualizationMacroParamData:n
	// area ja koko, jota tarvitaan myöhemmin.
	UpdateOptimizedVisualizationMacroParamData();
    if(IsGridDataDrawnWithSpaceOutSymbols())
    {
		if(DoMacroParamProbing())
        {
            auto gridsize = CalcSymbolDrawedMacroParamSpaceOutGridSize(itsCtrlViewDocumentInterface->Registry_SpacingOutFactor(itsMapViewDescTopIndex));
			return CreateNewResizedMacroParamData(gridsize, itsArea.get());
        }
    }
	else if(::IsSymbolDrawing(itsDrawParam))
	{
		// Symbolipiirtoja varten pitää kuitenkin laskea joku testisetti dataa, jotta
		// voidaan arvioida symbolin teksti pituuksia.
		// Jos näin ei tehdä, jää seuraavanlainen data iki-looppiin:
		// macroParam, jossa symbolipiirto ja jossa sparse-data-visualization päällä (sitä ei harvenneta)
		DoMacroParamProbing();
		return nullptr;
	}


	NFmiPoint possibleNewGridSize;
	boost::shared_ptr<NFmiFastQueryInfo> possibleMacroParamResolutionInfo;
	if(IsMacroParamIsolineDataDownSized(possibleNewGridSize, possibleMacroParamResolutionInfo))
		return CreateNewResizedMacroParamData(possibleNewGridSize, possibleMacroParamResolutionInfo->Area());

	if(IsMacroParamContourDataDownSized(possibleMacroParamResolutionInfo, possibleNewGridSize))
		return CreateNewResizedMacroParamData(possibleNewGridSize, possibleMacroParamResolutionInfo->Area());

    return boost::shared_ptr<NFmiFastQueryInfo>();
}

// Tekee määrätyn kokoisen macroParam matriisin laskut ja asettaa
// arvot haluttuihin dataosiin.
// Palauttaa true, jos probe action tehtiin onnistuneesti, muuten false.
bool NFmiStationView::DoMacroParamProbing()
{
	// CalculationPoint laskuja ei tarvitse tarkistella
	if(!fUseCalculationPoints)
	{
		auto probingData = ::CreateProbingMacroParamData(itsArea);
		if(probingData)
		{
			itsMacroParamPhase = MacroParamPhase::Probing;
			NFmiDataMatrix<float> probingMatrix(probingData->GridXNumber(), probingData->GridYNumber(), kFloatMissing);
			FmiModifyEditdData::CalcMacroParamMatrix(itsCtrlViewDocumentInterface->GenDocDataAdapter(), itsMapViewDescTopIndex, itsDrawParam, probingMatrix, false, itsCtrlViewDocumentInterface->UseMultithreaddingWithModifyingFunctions(), itsTime, NFmiPoint::gMissingLatlon, probingData, fUseCalculationPoints, true, CalcUsedSpaceOutFactors(), probingData, &itsProbingExtraMacroParamData);
			itsMacroParamProbingValues = ::matrixToVector(probingMatrix);
			return true;
		}
	}
	return false;
}


static void TraceLogForMacroParamCalculationSize(boost::shared_ptr<NFmiFastQueryInfo> &macroParamInfo, NFmiCtrlView *view)
{
    if(macroParamInfo)
    {
        auto gridSizeX = macroParamInfo->GridXNumber();
        auto gridSizeY = macroParamInfo->GridYNumber();
        std::string gridSizeStr = std::to_string(gridSizeX) + "x" + std::to_string(gridSizeY);
		std::string finalLogStr = "MacroParam was calculated  in grid size of " + gridSizeStr;
        CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(finalLogStr, view);
    }
}

// Lasketaan macroParamin arvot annettuun theValues matriisiin.
// Laskuissa käytetty hila laitetaan theUsedGridOut:in arvoksi.
void NFmiStationView::CalcMacroParamMatrix(NFmiDataMatrix<float> &theValues, NFmiGrid *theUsedGridOut)
{
    NFmiMacroParamLayerCacheDataType macroParamLayerCacheDataType;
    auto realRowIndex = CalcRealRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber);
    if(itsCtrlViewDocumentInterface->MacroParamDataCache().getCache(itsMapViewDescTopIndex, realRowIndex, itsViewRowLayerNumber, itsTime, itsDrawParam->InitFileName(), macroParamLayerCacheDataType))
    {
        macroParamLayerCacheDataType.getCacheValues(theValues, fUseCalculationPoints, fUseAlReadySpacedOutData, itsInfo);
        if(theUsedGridOut)
        {
            *theUsedGridOut = NFmiGrid(itsArea.get(), static_cast<unsigned long>(theValues.NX()), static_cast<unsigned long>(theValues.NY()));
        }
        CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(std::string("MacroParam data was retrieved from cache (=fast)"), this);
    }
    else
    {
        CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, std::string(__FUNCTION__) + ": macroParam calculations");
        auto possibleSpaceOutData = CreatePossibleSpaceOutMacroParamData();
		itsMacroParamPhase = MacroParamPhase::Calculation;
		FmiModifyEditdData::CalcMacroParamMatrix(itsCtrlViewDocumentInterface->GenDocDataAdapter(), itsMapViewDescTopIndex, itsDrawParam, theValues, false, itsCtrlViewDocumentInterface->UseMultithreaddingWithModifyingFunctions(), itsTime, NFmiPoint::gMissingLatlon, itsInfo, fUseCalculationPoints, false, CalcUsedSpaceOutFactors(), possibleSpaceOutData);
        if(fUseCalculationPoints)
            CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(std::string("MacroParam was calculated only in set CalculationPoint's"), this);
		else
			::TraceLogForMacroParamCalculationSize(itsInfo, this);
		if(theUsedGridOut && itsInfo && itsInfo->Grid())
            *theUsedGridOut = *itsInfo->Grid();

        CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(std::string("MacroParam data was put into cache for future fast retrievals"), this);
        macroParamLayerCacheDataType.setCacheValues(theValues, fUseCalculationPoints, fUseAlReadySpacedOutData, itsInfo->Area());
        itsCtrlViewDocumentInterface->MacroParamDataCache().setCache(itsMapViewDescTopIndex, realRowIndex, itsViewRowLayerNumber, itsTime, itsDrawParam->InitFileName(), macroParamLayerCacheDataType);
    }
	itsMacroParamPhase = MacroParamPhase::Drawing;
}

const float g_MacroParamValueWasNotInCache = -987654321.123456789f;

float NFmiStationView::GetMacroParamTooltipValueFromCache(const NFmiExtraMacroParamData& extraMacroParamData)
{
	return GetMacroParamValueFromCache(extraMacroParamData, itsCtrlViewDocumentInterface->ToolTipLatLonPoint(), itsCtrlViewDocumentInterface->ToolTipTime());
}

float NFmiStationView::GetMacroParamValueFromCache(const NFmiExtraMacroParamData& extraMacroParamData, const NFmiPoint& latlon, const NFmiMetTime& aTime)
{
	NFmiMacroParamLayerCacheDataType macroParamLayerCacheDataType;
	auto realRowIndex = CalcRealRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber);
	if(itsCtrlViewDocumentInterface->MacroParamDataCache().getCache(itsMapViewDescTopIndex, realRowIndex, itsViewRowLayerNumber, aTime, itsDrawParam->InitFileName(), macroParamLayerCacheDataType))
	{
		const auto& dataMatrix = macroParamLayerCacheDataType.getDataMatrix();
		NFmiGrid grid = itsMacroParamCalculationGrid ? *itsMacroParamCalculationGrid :
			NFmiGrid(itsArea.get(), static_cast<unsigned long>(dataMatrix.NX()), static_cast<unsigned long>(dataMatrix.NY()));
		grid.Area()->SetXYArea(NFmiRect(0, 0, 1, 1));
		auto gridPoint = grid.LatLonToGrid(latlon);
		if(extraMacroParamData.CalculationType() == MacroParamCalculationType::Index)
		{
			auto xIndex = boost::math::iround(gridPoint.X());
			auto yIndex = boost::math::iround(gridPoint.Y());
			if(xIndex >= 0 && xIndex < dataMatrix.NX() && yIndex >= 0 && yIndex < dataMatrix.NY())
			{
				return dataMatrix[xIndex][yIndex];
			}
		}
		else
		{
			auto xyPoint = grid.GridToXY(gridPoint);
			// normi reaaliluku interpolaatio lämpötila parametri on vain dummy arvo tavalliselle reaaliluvulle
			auto interpolatedValue = dataMatrix.InterpolatedValue(xyPoint, grid.Area()->XYArea(), kFmiTemperature);
			if(interpolatedValue != kFloatMissing)
				return interpolatedValue;

			// Monissa macroParameissa on paljon puuttuvia arvoja, ja normaali interpolaatio ei toimi, kokeillaan vielä saadaanko nearest menetelmällä arvoa
			return dataMatrix.InterpolatedValue(xyPoint, grid.Area()->XYArea(), kFmiTemperature, false, kNearestPoint);
		}
	}

	return g_MacroParamValueWasNotInCache;
}

// Tooltip laskuja varten tehdään oma 'Probing' macroParam data on alueeltaan seuraava:
// Tooltipin latlon piste tulee sen bottom-left-kulmaksi ja siitä koilliseen 100 metriä on top-right-kulma.
// Laskentahilaksi 4x4, joihin ei oikeasti edes lasketa kuin yhteen pisteeseen.
static boost::shared_ptr<NFmiFastQueryInfo> CreateTooltipProbingMacroParamData(boost::shared_ptr<NFmiArea>& mapArea, const NFmiPoint &tooltipLatlon)
{
	// Lasketaan uudet kulmapisteet suhteessa tooltip sijaintiin (=bottomLeft) ja tehdään siihen 4x4 hila.
	if(mapArea)
	{
		NFmiLocation topRightLocation(tooltipLatlon);
		// Siirrytään tooltip kohdasta koilliseen (= 45 astetta kun 0 astetta on pohjoiseen) 100 metriä.
		topRightLocation.SetLocation(45, 100, mapArea->PacificView());
		boost::shared_ptr<NFmiArea> newArea(mapArea->CreateNewArea(tooltipLatlon, topRightLocation.GetLocation()));
		return NFmiInfoOrganizer::CreateNewMacroParamData_checkedInput(4, 4, NFmiInfoData::kMacroParam, newArea.get());
	}

	return boost::shared_ptr<NFmiFastQueryInfo>();
}

// Pelkän tooltipin lasku macroParamista.
float NFmiStationView::CalcMacroParamTooltipValue(NFmiExtraMacroParamData &extraMacroParamData, boost::shared_ptr<NFmiDrawParam>& theUsedDrawParam)
{
    NFmiPoint latlon = itsCtrlViewDocumentInterface->ToolTipLatLonPoint();
    NFmiMetTime usedTime = itsCtrlViewDocumentInterface->ToolTipTime();
    NFmiDataMatrix<float> fakeMatrixValues;
	// Ei ole hyötyä käyttää monta threadia kun lasketaan yhtä tooltip arvoa
	bool doMultiThread = false;
	// Luodaan mahdollisimman pieni data, jotta tooltippien rakentelu menee joutuisasti.
	// Ainoa asia mikä menetetään on, että jos joku laskee muuttujaan jotain ja haluaa 
	// laskea siitä jotain alueellisia keskiarvoja tms., koska sitä ei voi tehdä probe hilalla.
	auto probeData = ::CreateTooltipProbingMacroParamData(itsArea, latlon);
	return FmiModifyEditdData::CalcMacroParamMatrix(itsCtrlViewDocumentInterface->GenDocDataAdapter(), itsMapViewDescTopIndex, theUsedDrawParam, fakeMatrixValues, true, doMultiThread, usedTime, latlon, itsInfo, fUseCalculationPoints, true, CalcUsedSpaceOutFactors(), probeData, &extraMacroParamData);
}

static void MakeDrawedInfoVector(NFmiGriddingHelperInterface *theGriddingHelper, const boost::shared_ptr<NFmiArea> &theArea, std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector, boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
    theGriddingHelper->MakeDrawedInfoVectorForMapView(theInfoVector, theDrawParam, theArea);
}

static void SetXYZValues(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiArea &theArea, float theValue, std::vector<float> &theXValues, std::vector<float> &theYValues, std::vector<float> &theZValues)
{
	if(theValue != kFloatMissing)
	{
		NFmiPoint p(theArea.ToXY(theInfo->LatLon()));
		const NFmiRect &bsRect(theArea.XYArea());
		double top = bsRect.Top();
		double height = bsRect.Height();
		double invertY = top  + ((top+height)-p.Y()); // ikävä kyllä taas pitää tehdä y-akselin kääntöä
		theXValues.push_back(static_cast<float>(p.X()));
		theYValues.push_back(static_cast<float>(invertY));
		theZValues.push_back(theValue);
	}
}

static void DoFinalGridding(const NFmiGriddingProperties &griddingProperties, const boost::shared_ptr<NFmiArea> &theArea, std::vector<float> &theXValues, std::vector<float> &theYValues, std::vector<float> &theZValues, NFmiDataMatrix<float> &theValues)
{
    auto stationRadiusRelative = static_cast<float>(NFmiGriddingProperties::ConvertLengthInKmToRelative(griddingProperties.rangeLimitInKm(), theArea.get()));
    std::unique_ptr<NFmiObsDataGridding> obsDataGridding(new NFmiObsDataGridding());
	NFmiDataParamControlPointModifier::DoDataGridding(theXValues, theYValues, theZValues, static_cast<int>(theZValues.size()), theValues, theArea->XYArea(), griddingProperties, obsDataGridding.get(), stationRadiusRelative);
}

void NFmiStationView::GridStationData(NFmiGriddingHelperInterface *theGriddingHelper, const boost::shared_ptr<NFmiArea> &theArea, boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiDataMatrix<float> &theValues, const NFmiMetTime &theTime, const NFmiGriddingProperties &griddingProperties)
{
	std::vector<float> xValues;
	std::vector<float> yValues;
	std::vector<float> zValues;

	std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector;
	::MakeDrawedInfoVector(theGriddingHelper, theArea, infoVector, theDrawParam);
	std::vector<boost::shared_ptr<NFmiFastQueryInfo> >::iterator iter = infoVector.begin();
	if(iter != infoVector.end())
	{
		NFmiIgnoreStationsData &ignorestationdata = theGriddingHelper->IgnoreStationsData();
		for( ; iter != infoVector.end(); ++iter)
		{
			boost::shared_ptr<NFmiFastQueryInfo> &info = *iter;
			if(info->Time(theTime))
			{
				bool useShipLocations = NFmiFastInfoUtils::IsInfoShipTypeData(*info);
				if(info->Param(static_cast<FmiParameterName>(theDrawParam->Param().GetParamIdent())))
				{
					for(info->ResetLocation(); info->NextLocation(); )
					{
						if(ignorestationdata.IsStationBlocked(*(info->Location()), false) == false)
						{
							NFmiPoint latlon(useShipLocations ? info->GetLatlonFromData() : info->LatLon());
							if(theArea->IsInside(latlon))
							{
                                NFmiFastInfoUtils::SetSoundingDataLevel(theDrawParam->Level(), *info); // Tämä tehdään vain luotaus datalle: tämä level pitää asettaa joka pisteelle erikseen, koska vakio painepinnat eivät ole kaikille luotaus parametreille samoilla leveleillä
								float value = info->FloatValue();
								::SetXYZValues(info, *theArea, value, xValues, yValues, zValues);
							}
						}
					}
				}
			}
		}
	}
	::DoFinalGridding(griddingProperties, theArea, xValues, yValues, zValues, theValues);
}

class CtrlViewDocumentInterfaceGridding : public NFmiGriddingHelperInterface
{
    CtrlViewDocumentInterface *itsCtrlViewDocumentInterface;
public:
    CtrlViewDocumentInterfaceGridding(CtrlViewDocumentInterface *theCtrlViewDocumentInterface)
        :itsCtrlViewDocumentInterface(theCtrlViewDocumentInterface)
    {}

    void MakeDrawedInfoVectorForMapView(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector, boost::shared_ptr<NFmiDrawParam> &theDrawParam, const boost::shared_ptr<NFmiArea> &theArea) override
    {
        itsCtrlViewDocumentInterface->MakeDrawedInfoVectorForMapView(theInfoVector, theDrawParam, theArea);
    }
    NFmiIgnoreStationsData& IgnoreStationsData() override
    {
        return itsCtrlViewDocumentInterface->IgnoreStationsData();
    }
    const NFmiGriddingProperties& GriddingProperties(bool getEditingRelatedProperties) override
    {
        return itsCtrlViewDocumentInterface->ApplicationWinRegistry().GriddingProperties(getEditingRelatedProperties);
    }

};


void NFmiStationView::GridStationDataToMatrix(NFmiDataMatrix<float> &theValues, const NFmiMetTime &theTime)
{
	if(UseQ2ForSynopData(itsDrawParam))
		GridStationDataFromQ2(theValues, theTime);
    else
    {
        CtrlViewDocumentInterfaceGridding ctrlViewGriddingInterface(itsCtrlViewDocumentInterface);
        NFmiStationView::GridStationData(&ctrlViewGriddingInterface, GetArea(), itsDrawParam, theValues, theTime, ctrlViewGriddingInterface.GriddingProperties(false));
    }
}

void NFmiStationView::GridStationDataFromQ2(NFmiDataMatrix<float> &theValues, const NFmiMetTime &theTime)
{
	std::vector<float> xValues;
	std::vector<float> yValues;
	std::vector<float> zValues;

	std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector;
	MakeDrawedInfoVector(infoVector, itsDrawParam);
	std::vector<boost::shared_ptr<NFmiFastQueryInfo> >::iterator iter = infoVector.begin();
	if(iter != infoVector.end())
	{
		NFmiIgnoreStationsData &ignorestationdata = itsCtrlViewDocumentInterface->IgnoreStationsData();
		if(fGetSynopDataFromQ2 == false)
			GetQ2SynopData();
		if(fGetSynopDataFromQ2)
		{ // jos haku onnistui jatketaan
			boost::shared_ptr<NFmiFastQueryInfo> &info = *iter; // käydään läpi vain pää eli 1. info
			for(info->ResetLocation(); info->NextLocation(); )
			{
				if(ignorestationdata.IsStationBlocked(*(info->Location()), false) == false)
				{
					StationIdSeekContainer::iterator it = itsSynopDataFromQ2StationIndexies.find(info->Location()->GetIdent());
					if(it != itsSynopDataFromQ2StationIndexies.end())
					{
						float value = itsSynopDataValuesFromQ2[0][(*it).second];
						::SetXYZValues(info, *itsArea, value, xValues, yValues, zValues);
					}
				}
			}
		}
	}

    CtrlViewDocumentInterfaceGridding ctrlViewGriddingInterface(itsCtrlViewDocumentInterface);
    ::DoFinalGridding(ctrlViewGriddingInterface.GriddingProperties(false), itsArea, xValues, yValues, zValues, theValues);
}

static void FixDataWithMaskValues(const NFmiMetTime &theTime, NFmiDataMatrix<float> &theValues, const NFmiGrid &theGrid, boost::shared_ptr<NFmiDrawParam> &theDrawParam, boost::shared_ptr<NFmiAreaMaskList> &theParamMaskList)
{
    // theValues matrix migth be empty in certain kind of errors (e.g. macroParam couldn't be calculated at all)
	// and the matrix and given grid objects must be the same size in both directions.
    if(theValues.NX() && theValues.NY() && theValues.NX() == theGrid.XNumber() && theValues.NY() == theGrid.YNumber())
    {
        if(theParamMaskList && theDrawParam->DrawOnlyOverMask())
        {
            theParamMaskList->CheckIfMaskUsed();
            if(theParamMaskList->UseMask())
            {
                theParamMaskList->SyncronizeMaskTime(theTime);
                NFmiGrid aGrid = theGrid;
                int xSize = aGrid.XNumber();
                int index = 0;
                for(aGrid.Reset(); aGrid.Next(); index++)
                {
                    if(theParamMaskList->IsMasked(aGrid.LatLon()) == false)
                    {
                        int xNum = index % xSize;
                        int yNum = index / xSize;
                        theValues[xNum][yNum] = kFloatMissing;
                    }
                }
            }
        }
    }
}

static string GetQ2ParamString(boost::shared_ptr<NFmiDrawParam> &theDrawParam) 	
{
	string str("paramId=");
	str += NFmiStringTools::Convert<unsigned long>(theDrawParam->Param().GetParamIdent());
	return str;
}

static string GetQ2ParamString(std::vector<FmiParameterName> &theWantedParamVector) 	
{
	string str("paramId=");
	for(size_t i = 0; i < theWantedParamVector.size(); i++)
	{
		if(i > 0)
			str += ",";
		str += NFmiStringTools::Convert<unsigned long>(theWantedParamVector[i]);
	}
	return str;
}

static string GetQ2ProducerString(boost::shared_ptr<NFmiDrawParam> &theDrawParam) 	
{
	string str("&producerId=");
	str += NFmiStringTools::Convert<unsigned long>(theDrawParam->Param().GetProducer()->GetIdent());
	return str;
}

static string GetQ2DataTypeString(boost::shared_ptr<NFmiDrawParam> &theDrawParam) 	
{
	string str("&dataType=");
	str += NFmiStringTools::Convert<NFmiInfoData::Type>(theDrawParam->DataType());
	return str;
}

static string GetQ2DecimalCountString(int theDecimalCount) 	
{
	string str("&maxDecimals=");
	str += NFmiStringTools::Convert<int>(theDecimalCount);
	return str;
}

static string GetQ2LevelString(boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
	string str;
	// level dataa pyyntöä varten pitää laittaa (HUOM! jos ident on 0, ei kyse ole level datasta)
	if(theDrawParam->Level().GetIdent() != 0)
	{
		// &levelType=100 tai 109 (100 pressuretype ja 109 on hybridi)
		str += "&levelType=";
		str += NFmiStringTools::Convert<int>(theDrawParam->Level().LevelType());
		// &levelValue=850
		str += "&levelValue=";
		str += NFmiStringTools::Convert<float>(theDrawParam->Level().LevelValue());
	}
	return str;
}

static std::unique_ptr<NFmiArea> GetQ2ToolTipArea(CtrlViewDocumentInterface *theCtrlViewDocumentInterface)
{
	NFmiPoint bl(theCtrlViewDocumentInterface->ToolTipLatLonPoint());
	NFmiPoint tr(bl.X() + 0.001, bl.Y() + 0.001);
	return std::make_unique<NFmiLatLonArea>(bl, tr);
}

static NFmiGrid GetQ3ArchiveDataGrid(CtrlViewDocumentInterface *theCtrlViewDocumentInterface, boost::shared_ptr<NFmiArea> &theArea, bool doToolTipCalculation, int viewGridSize)
{
	if(doToolTipCalculation)
	{
		NFmiPoint gridSize(2, 2); // 2 x 2 on mini hila mitä voidaan hakea, vaikka tooltippiin tarvitaankin vain yksi arvo
		std::unique_ptr<NFmiArea> areaPtr = ::GetQ2ToolTipArea(theCtrlViewDocumentInterface);
		return NFmiGrid(areaPtr.get(), static_cast<unsigned long>(gridSize.X()), static_cast<unsigned long>(gridSize.Y()));
	}
	else
	{
		auto& visSettings = theCtrlViewDocumentInterface->ApplicationWinRegistry().VisualizationSpaceoutSettings();
		NFmiPoint gridSize = visSettings.getCheckedPossibleOptimizedGridSize(theCtrlViewDocumentInterface->GetQ2ServerInfo().Q2ServerGridSize(), *theArea, viewGridSize, theCtrlViewDocumentInterface->BetaProductGenerationRunning());
        return NFmiGrid(theArea.get(), static_cast<unsigned long>(gridSize.X()), static_cast<unsigned long>(gridSize.Y()));
	}
}


static string GetQ2ProjectionString(const boost::shared_ptr<NFmiArea> &theFinalArea)
{
	string str("&projection=");
	str += theFinalArea->AreaStr();
	return str;
}

static string GetQ2StationIdString(unsigned long theStationId)
{
	string str("&stationId=");
	str += NFmiStringTools::Convert(theStationId);
	return str;
}

static string GetQ2GridSizeString(const NFmiGrid &theGrid)
{
	string str("&gridSize=");
	str += NFmiStringTools::Convert(theGrid.XNumber());
	str += ",";
	str += NFmiStringTools::Convert(theGrid.YNumber());
	return str;
}

static string GetQ2ValidTimeString(const NFmiMetTime &theTime)
{
	string str("&validTime=");
	str += theTime.ToStr(kYYYYMMDDHHMMSS);
	return str;
}

static string GetQ2ModelOrigTimeString(CtrlViewDocumentInterface *theCtrlViewDocumentInterface, boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
	string str("&");
	str += theCtrlViewDocumentInterface->GetModelOrigTimeString(theDrawParam);
	return str;
}

// luodaan theSynopDataFromQ2StationIndexiesOut:iin indeksi lista, joka sitoo
// asema Id:n ja indeksin theSynopDataValuesFromQ2-matriisin välille.
// Kun sitten käydään piirtämässä dataa kartalle, kysytään querydatan locatiolta asemaId ja
// sen avulla kysytään StationSeekcontainerilta indeksi lopulliseen parametrin arvon
// kohtaan matriisissa.
static void GetSynopStationIds(const std::string &theExtraInfoStr, NFmiStationView::StationIdSeekContainer &theSynopDataFromQ2StationIndexiesOut, NFmiDataMatrix<float> &theSynopDataValuesFromQ2)
{
	theSynopDataFromQ2StationIndexiesOut.clear();
	std::vector<unsigned long> stationIndexies = NFmiStringTools::Split<std::vector<unsigned long> >(theExtraInfoStr);
	for(size_t i = 0; i < stationIndexies.size(); i++)
	{
		theSynopDataFromQ2StationIndexiesOut[stationIndexies[i]] = static_cast<unsigned long>(i);
	}
}

static void SetSynopParamIds(NFmiStationView::ParamIdSeekContainer &theSynopPlotParamIndexies, std::vector<FmiParameterName> &theWantedParamVector)
{
	theSynopPlotParamIndexies.clear();
	for(size_t i = 0; i < theWantedParamVector.size(); i++)
		theSynopPlotParamIndexies[theWantedParamVector[i]] = static_cast<unsigned long>(i);
}

// Tämä hakee asema dataa Q2-serveriltä
// Esimerkki haku:
// http://brainstormgw.fmi.fi/q2?requestType=stationData&paramId=4,1&producerId=1001&dataType=5&validTime=TODAY+3&projection=stereographic,25:21,62,27,66&maxDecimals=1
// Lisäksi tooltippiä varten pitää tehdä haku (tehdään jos theStationId on eri kuin 0):
// http://brainstormgw.fmi.fi/q2?requestType=stationData&paramId=4,1&producerId=1001&dataType=5&validTime=TODAY-12&stationId=2978,2929&maxDecimals=1
bool NFmiStationView::GetQ2SynopData(unsigned long theStationId, std::vector<FmiParameterName> theWantedParamVector)
{
    auto &q2ServerInfo = itsCtrlViewDocumentInterface->GetQ2ServerInfo();
	string urlStr = q2ServerInfo.Q2ServerURLStr();
	string baseParStr;
	if(theWantedParamVector.size() == 0)
		baseParStr += ::GetQ2ParamString(itsDrawParam);
	else
		baseParStr += ::GetQ2ParamString(theWantedParamVector);
	baseParStr += ::GetQ2ProducerString(itsDrawParam);
	baseParStr += ::GetQ2DataTypeString(itsDrawParam);
	baseParStr += ::GetQ2DecimalCountString(q2ServerInfo.Q2ServerDecimalCount());
	baseParStr += ::GetQ2LevelString(itsDrawParam);
	string requestTypeStr("&requestType=stationData");
	string projectionStr = ::GetQ2ProjectionString(itsArea);
	if(theStationId)
		projectionStr = ::GetQ2StationIdString(theStationId);
	string timeStr = ::GetQ2ValidTimeString(itsTime);

	bool useBinaryData = true; // binääri data on nopeampaa
	int usedCompression = q2ServerInfo.Q2ServerUsedZipMethod(); // 0=none, 1=zip, 2=bzip2
	std::string extraInfoStr;

	std::string finalParStr(baseParStr);
	finalParStr += requestTypeStr;
	finalParStr += timeStr;
	finalParStr += projectionStr;

	try
	{
		itsCtrlViewDocumentInterface->GetDataFromQ2Server(urlStr, finalParStr, useBinaryData, usedCompression, itsSynopDataValuesFromQ2, extraInfoStr);

		::GetSynopStationIds(extraInfoStr, itsSynopDataFromQ2StationIndexies, itsSynopDataValuesFromQ2);
		if(theWantedParamVector.size())
			::SetSynopParamIds(itsSynopPlotParamIndexies, theWantedParamVector);

		fGetSynopDataFromQ2 = true; // merkataan true:ksi vasta kun dataa on oikeasti saatu
	}
	catch(std::exception & /* e */ )
	{
		// liatetaan varmuuden vuoksi matriisi 0 kokoiseksi
		itsSynopDataValuesFromQ2.Resize(0, 0);

		return false;
	}
	return true;

}

static std::string MakeQ3ErrorStr(std::exception *theException, FmiLanguage theLanguage)
{
    NFmiTime currentTime;
    std::string errStr("Last Q3 error: ");
    errStr += currentTime.ToStr("HH:mm:SS (local time)", theLanguage);
    errStr += "\n";
    if(theException)
        errStr += theException->what();
    else
        errStr += "Unknown error with q3 query...";

    return errStr;
}

static string GetQ3ModelOrigTimeString(CtrlViewDocumentInterface *theCtrlViewDocumentInterface, boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
    return theCtrlViewDocumentInterface->GetModelOrigTimeString(theDrawParam);
}

static string MakeQ3GridSizeStr(NFmiGrid &theUsedGrid)
{
    string gridsizeStr("gridsize=xy(");
    gridsizeStr += NFmiStringTools::Convert<int>(static_cast<int>(theUsedGrid.XNumber()));
    gridsizeStr += ",";
    gridsizeStr += NFmiStringTools::Convert<int>(static_cast<int>(theUsedGrid.YNumber()));
    gridsizeStr += ")";
    return gridsizeStr;

}

static string GetQ3ProducerString(boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
    long producer = theDrawParam->Param().GetProducer()->GetIdent();
    if(producer == kFmiMTAECMWF)
        return "EC";
    else if(producer == kFmiMTAHIRLAM)
        return "HIR";

    throw std::runtime_error(string("Unsupported producer (") + NFmiStringTools::Convert(producer) + ") wanted from q3 server archives, stopping data retrieval...");
}

static string GetQ3LevelString(boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
    string str;
    // level dataa pyyntöä varten pitää laittaa (HUOM! jos ident on 0, ei kyse ole level datasta)
    if(theDrawParam->Level().GetIdent() != 0)
    {
        if(theDrawParam->Level().LevelType() == kFmiPressureLevel)
        {
            str += "{hpa=" + NFmiStringTools::Convert(theDrawParam->Level().LevelValue()) + "}";
        }
        else if(theDrawParam->Level().LevelType() == kFmiHybridLevel)
        {
            str += "{hybrid=true}{hybrid=" + NFmiStringTools::Convert(theDrawParam->Level().LevelValue()) + "}";
        }
        else
            throw std::runtime_error(string("Unsupported level type (") + NFmiStringTools::Convert(theDrawParam->Level().LevelType()) + ") wanted from q3 server archives, stopping data retrieval...");
    }
    return str;

}

static string GetQ3ParamString(boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
    return string("[\":") + NFmiStringTools::Convert(theDrawParam->Param().GetParamIdent()) + "\"]";
}

// hirlam pintalämpötila : HIR.T
// hirlam lämpötila painepinnalla 925 : HIR{hpa=925}.T
// hirlam lämpötila mallipinnalla 60 : HIR{hybrid=true}{hybrid=60}.T
//
// ecmwf vastaavasti korvaamalla 'HIR' 'EC' : llä, esim.EC.T
// T on lämpötilaparam. 'global' nimi, voidaan hakea myös param.nro : lla;
//
// HIR[":4"], HIR{hpa=925}[":4"], HIR{hybrid=true}{hybrid=60}[":4"]

static string MakeQ3WantedDataStr(boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
    string dataStr = ::GetQ3ProducerString(theDrawParam);
    dataStr += ::GetQ3LevelString(theDrawParam);
    dataStr += ::GetQ3ParamString(theDrawParam);
    return dataStr;
}

static string MakeQ3ArchiveModelDataCodeStr(CtrlViewDocumentInterface *theCtrlViewDocumentInterface, boost::shared_ptr<NFmiDrawParam> &theDrawParam, const NFmiMetTime &theValidTime, NFmiGrid &theUsedGrid)
{
    std::string codeStrStart = "code="; // Tätä alkua ei saa encodata

    std::string codeStr = ::GetQ3ModelOrigTimeString(theCtrlViewDocumentInterface, theDrawParam);

    codeStr += " validtime=";
    codeStr += theValidTime.ToStr(kYYYYMMDDHHMMSS);

    codeStr += " projection=\"";
    codeStr += theUsedGrid.Area()->AreaStr();
    codeStr += "\" ";

    codeStr += ::MakeQ3GridSizeStr(theUsedGrid);

    codeStr += " return ";
    codeStr += ::MakeQ3WantedDataStr(theDrawParam);

    codeStr = NFmiStringTools::UrlEncode(codeStr);

    return codeStrStart + codeStr;
}

// Q3 arkistohaku esimerkkiä ja ohjeita:
// http://smartmet.fmi.fi/q3 on nykyinen osoite (2016 Marraskuu), crash-osoite on testiympäristö.
//
// hirlam pintalämpötila :
// http://crash.fmi.fi:8080/q3?output=bin&maxDecimals=2&code=origintime=20161107000000 validtime=20161107180000 projection=%22stereographic,20,90,60:6,51.3,49,70.2%22 gridsize=xy(100,100) return HIR.T
//
// hirlam lämpötila painepinnalla 925 :
// http ://crash.fmi.fi:8080/q3?output=bin&maxDecimals=2&code=origintime=20161107000000 validtime=20161107180000 projection=%22stereographic,20,90,60:6,51.3,49,70.2%22 gridsize=xy(100,100) return HIR{hpa=850}{hpa=925}.T
//
// hirlam lämpötila mallipinnalla 60 :
// http ://crash.fmi.fi:8080/q3?output=bin&maxDecimals=2&code=origintime=20161107000000 validtime=20161107180000 projection=%22stereographic,20,90,60:6,51.3,49,70.2%22 gridsize=xy(100,100) return HIR{hybrid=true}{hybrid=60}.T

bool NFmiStationView::GetArchiveDataFromQ3Server(NFmiDataMatrix<float> &theValues, NFmiGrid &theUsedGridOut, bool doToolTipCalculation)
{
    try
    {
        auto &q2ServerInfo = itsCtrlViewDocumentInterface->GetQ2ServerInfo();
        bool useBinaryData = true; // binääri data on nopeampaa
        int usedCompression = q2ServerInfo.Q2ServerUsedZipMethod(); // 0=none, 1=zip, 2=bzip2
        theUsedGridOut = ::GetQ3ArchiveDataGrid(itsCtrlViewDocumentInterface, itsArea, doToolTipCalculation, CalcViewGridSize());

        string urlStr = q2ServerInfo.Q3ServerUrl();

        string baseParStr;

        int decimalcount = q2ServerInfo.Q2ServerDecimalCount();
        baseParStr += "maxdecimals="; // Ei & -merkkiä alkuun, koska kyseessä 1. parametri
        baseParStr += NFmiStringTools::Convert<int>(decimalcount);

        baseParStr += "&" + ::MakeQ3ArchiveModelDataCodeStr(itsCtrlViewDocumentInterface, itsDrawParam, itsTime, theUsedGridOut);

        std::string dummyExtraInfoStr; // Tähän ei tule mitään näissä haussa (arkisto synop datahaussa tulee asematietoa)
        itsCtrlViewDocumentInterface->GetDataFromQ2Server(urlStr, baseParStr, useBinaryData, usedCompression, theValues, dummyExtraInfoStr);
    }
    catch(std::exception & e)
    {
        // liatetaan varmuuden vuoksi matriisi 0 kokoiseksi
        theValues.Resize(0, 0);
        itsCtrlViewDocumentInterface->LogAndWarnUser(::MakeQ3ErrorStr(&e, itsCtrlViewDocumentInterface->Language()), "", CatLog::Severity::Error, CatLog::Category::NetRequest, true);
        return false;
    }
    catch(...)
    {
        // liatetaan varmuuden vuoksi matriisi 0 kokoiseksi
        theValues.Resize(0, 0);
        itsCtrlViewDocumentInterface->LogAndWarnUser(::MakeQ3ErrorStr(nullptr, itsCtrlViewDocumentInterface->Language()), "", CatLog::Severity::Error, CatLog::Category::NetRequest, true);
        return false;
    }
    return true;
}

// Q3 skriptin datahaku q3-serveriltä
// http://smartmet.fmi.fi/q3 on nykyinen osoite (2016 Marraskuu)
// http://crash.fmi.fi:8080/q3?code=return%20WS_HIR&validtime=TODAY12&projection=stereographic,20,90,60:6,51.3,49,70.2&gridsize=50,60&maxdecimals=1
// Sama osiin jaettuna 
//
// http://crash.fmi.fi:8080/q3?
// code=return%20WS_HIR
// &validtime=TODAY12
// &projection=stereographic,20,90,60:6,51.3,49,70.2
// &gridsize=50,60
// &maxdecimals=1
bool NFmiStationView::GetQ3ScriptData(NFmiDataMatrix<float> &theValues, NFmiGrid &theUsedGrid, const std::string &theUsedBaseUrlStr)
{
    try
    {
		auto& visSettings = GetVisualizationSettings();
		NFmiPoint usedGridSize = visSettings.getCheckedPossibleOptimizedGridSize(itsCtrlViewDocumentInterface->InfoOrganizer()->GetMacroParamDataGridSize(), *itsArea, CalcViewGridSize(), itsCtrlViewDocumentInterface->BetaProductGenerationRunning());
        theUsedGrid = NFmiGrid(itsArea.get(), static_cast<unsigned long>(usedGridSize.X()), static_cast<unsigned long>(usedGridSize.Y()));

        string urlStr = theUsedBaseUrlStr;

        string baseParStr;
        baseParStr += "code=";

        std::string macroParamStr = CtrlViewUtils::GetMacroParamFormula(*(itsCtrlViewDocumentInterface->MacroParamSystem()), itsDrawParam);
        baseParStr += NFmiStringTools::UrlEncode(macroParamStr);

        string projectionStr("&projection=");
        projectionStr += itsArea->AreaStr();
        baseParStr += projectionStr;

        string gridsizeStr("&gridsize=");
        gridsizeStr += NFmiStringTools::Convert<int>(static_cast<int>(usedGridSize.X()));
        gridsizeStr += ",";
        gridsizeStr += NFmiStringTools::Convert<int>(static_cast<int>(usedGridSize.Y()));
        baseParStr += gridsizeStr;

        string timeStr("&validtime=");
        timeStr += itsTime.ToStr(kYYYYMMDDHHMMSS);
        baseParStr += timeStr;

        int decimalcount = itsCtrlViewDocumentInterface->GetQ2ServerInfo().Q2ServerDecimalCount();
        baseParStr += "&maxdecimals=";
        baseParStr += NFmiStringTools::Convert<int>(decimalcount);

        bool getOriginalGrid = false;
        bool useBinaryData = true; // binääri data on nopeampaa
        int usedCompression = 0; // 0=none, 1=zip, 2=bzip2
        std::string extraInfoStr;

        itsCtrlViewDocumentInterface->GetDataFromQ2Server(urlStr, baseParStr, useBinaryData, usedCompression, theValues, extraInfoStr);
    }
	catch(std::exception & e)
	{
		// liatetaan varmuuden vuoksi matriisi 0 kokoiseksi
		theValues.Resize(0, 0);
        itsCtrlViewDocumentInterface->SetMacroErrorText(::MakeQ3ErrorStr(&e, itsCtrlViewDocumentInterface->Language()));
		return false;
	}
	catch(...)
	{
		// liatetaan varmuuden vuoksi matriisi 0 kokoiseksi
		theValues.Resize(0, 0);
        itsCtrlViewDocumentInterface->SetMacroErrorText(::MakeQ3ErrorStr(nullptr, itsCtrlViewDocumentInterface->Language()));
		return false;
	}
	return true;
}

long NFmiStationView::GetTimeInterpolationRangeInMinutes(const NFmiHelpDataInfo *theHelpDataInfo)
{
    if(theHelpDataInfo)
        return theHelpDataInfo->TimeInterpolationRangeInMinutes();
    else
        return 6 * 60; // Default will be 6 hours
}

bool NFmiStationView::AllowNearestTimeInterpolation(long theTimeInterpolationRangeInMinutes)
{
    if(theTimeInterpolationRangeInMinutes > 0)
        return theTimeInterpolationRangeInMinutes <= 30;
    else
        return false;
}

float NFmiStationView::CalcUsedLegendSizeFactor(double singleMapViewHeightInMM, int visibleViewRowCount)
{
	float sizeFactor = ::CalcMMSizeFactor(static_cast<float>(singleMapViewHeightInMM), 1.1f);
	if(sizeFactor < 1)
		sizeFactor = std::pow(sizeFactor, 2.f);
	if(visibleViewRowCount > 1)
	{
		// Halutaan kasvattaa kertointa riippuen näyttörivien määrästä, 
		// sqrt olisi kasvattanut desimaaliosiota liian vähän, joten tässä otetaan 1.6:s juuri (pow(x, 1/y) -> y's juuri x:stä)
		float decimalPart = std::pow(static_cast<float>(visibleViewRowCount), 1.f/1.6f) / 10.f;
		float smallViewFactor = 1.f + decimalPart;
		sizeFactor *= smallViewFactor;
	}
	return sizeFactor;
}

NFmiHelpDataInfo* NFmiStationView::GetHelpDataInfo(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
    return GetCtrlViewDocumentInterface()->HelpDataInfoSystem()->FindHelpDataInfo(theInfo->DataFilePattern());
}

void NFmiStationView::FinalFillDataMatrix(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiDataMatrix<float> &theValues, const NFmiMetTime &usedTime, bool useCropping, int x1, int y1, int x2, int y2, NFmiGrid* optimizedDataGrid)
{
	if(optimizedDataGrid)
		theInfo->GridValues(theValues, *optimizedDataGrid, usedTime, itsTimeInterpolationRangeInMinutes);
	else if(useCropping)
        theInfo->CroppedValues(theValues, usedTime, x1, y1, x2, y2, itsTimeInterpolationRangeInMinutes, fAllowNearestTimeInterpolation);
    else
        theInfo->Values(theValues, usedTime, itsTimeInterpolationRangeInMinutes, fAllowNearestTimeInterpolation);
}

// Kannattaako yrittää piirtää annetun datan annettua aikaa karttanäyttöön?
bool NFmiStationView::DataIsDrawable(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiMetTime &usedTime)
{
    // Jos annettu aika on datan aikarakenteen sisällä, yritetään piirtoa
    if(itsInfo->TimeDescriptor().IsInside(usedTime))
        return true;
    if(fAllowNearestTimeInterpolation)
    {
        auto oldTimeIndex = theInfo->TimeIndex();
        bool foundNearTime = theInfo->FindNearestTime(usedTime, kCenter, itsTimeInterpolationRangeInMinutes);
        theInfo->TimeIndex(oldTimeIndex);
        return foundNearTime;
    }
    return false;
}

void NFmiStationView::FinalFillWindMetaDataMatrix(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiDataMatrix<float> &theValues, const NFmiMetTime &usedTime, bool useCropping, int x1, int y1, int x2, int y2, unsigned long wantedParamId, NFmiGrid* optimizedDataGrid)
{
    NFmiFastInfoUtils::QueryInfoParamStateRestorer restorer(*theInfo);
    if(metaWindParamUsage.HasWsAndWd())
    {
        theInfo->Param(kFmiWindSpeedMS);
        NFmiDataMatrix<float> WS;
        FinalFillDataMatrix(theInfo, WS, usedTime, useCropping, x1, y1, x2, y2, optimizedDataGrid);
        theInfo->Param(kFmiWindDirection);
        NFmiDataMatrix<float> WD;
        FinalFillDataMatrix(theInfo, WD, usedTime, useCropping, x1, y1, x2, y2, optimizedDataGrid);
        switch(wantedParamId)
        {
        case kFmiWindVectorMS:
        {
            NFmiFastInfoUtils::CalcMatrixWindVectorFromSpeedAndDirection(WS, WD, theValues);
            break;
        }
        case kFmiWindUMS:
        {
            NFmiFastInfoUtils::CalcMatrixUcomponentFromSpeedAndDirection(WS, WD, theValues);
            break;
        }
        case kFmiWindVMS:
        {
            NFmiFastInfoUtils::CalcMatrixVcomponentFromSpeedAndDirection(WS, WD, theValues);
            break;
        }
        default:
        {
            CatLog::logMessage(std::string("Meta wind parameter calculation had logical error with parameter: ") + std::to_string(wantedParamId), CatLog::Severity::Warning, CatLog::Category::Visualization);
            break;
        }
        }
    }
    else if(metaWindParamUsage.HasWindComponents())
    {
        theInfo->Param(kFmiWindUMS);
        NFmiDataMatrix<float> u;
        FinalFillDataMatrix(theInfo, u, usedTime, useCropping, x1, y1, x2, y2, optimizedDataGrid);
        theInfo->Param(kFmiWindVMS);
        NFmiDataMatrix<float> v;
        FinalFillDataMatrix(theInfo, v, usedTime, useCropping, x1, y1, x2, y2, optimizedDataGrid);
        switch(wantedParamId)
        {
        case kFmiWindVectorMS:
            return NFmiFastInfoUtils::CalcMatrixWindVectorFromWindComponents(u, v, theValues);
        case kFmiWindDirection:
            return NFmiFastInfoUtils::CalcMatrixWdFromWindComponents(u, v, theValues);
        case kFmiWindSpeedMS:
            return NFmiFastInfoUtils::CalcMatrixWsFromWindComponents(u, v, theValues);
        default:
        {
            CatLog::logMessage(std::string("Meta wind parameter calculation had logical error with parameter: ") + std::to_string(wantedParamId), CatLog::Severity::Warning, CatLog::Category::Visualization);
            break;
        }
        }
    }
}

void NFmiStationView::FillDataMatrix(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiDataMatrix<float> &theValues, const NFmiMetTime &theTime, bool fUseCropping, int x1, int y1, int x2, int y2, NFmiGrid* optimizedDataGrid)
{
	if(theInfo == 0)
		theValues = kFloatMissing;
    else
    {
        auto usedTime = NFmiFastInfoUtils::GetUsedTimeIfModelClimatologyData(theInfo, theTime);
        auto paramId = itsDrawParam->Param().GetParamIdent();
        if(metaWindParamUsage.ParamNeedsMetaCalculations(paramId))
        {
            FinalFillWindMetaDataMatrix(theInfo, theValues, usedTime, fUseCropping, x1, y1, x2, y2, paramId, optimizedDataGrid);
        }
        else
        {
            FinalFillDataMatrix(theInfo, theValues, usedTime, fUseCropping, x1, y1, x2, y2, optimizedDataGrid);
        }
    }
}

static void CalcDiffMatrix(NFmiDataMatrix<float> &theValuesInOut, const NFmiDataMatrix<float> &theValues2In)
{
	if(theValuesInOut.NX() == theValues2In.NX() && theValuesInOut.NY() == theValues2In.NY())
		theValuesInOut -= theValues2In;
	else
		theValuesInOut = kFloatMissing;
}

static NFmiGrid GetUsedGrid(const NFmiGrid &theCurrentGrid, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiDataMatrix<float> &theValues, bool fUseCropping, int x1, int y1, int x2, int y2)
{
	if(theCurrentGrid.Area() != 0)
		return theCurrentGrid;

	NFmiGrid usedGrid;
	if(fUseCropping)
	{
		NFmiGrid tmpGrid(*theInfo->Grid());
		NFmiPoint bl = tmpGrid.GridToLatLon(NFmiPoint(x1, y1));
		NFmiPoint tr = tmpGrid.GridToLatLon(NFmiPoint(x2, y2));
		NFmiArea *tmpArea = tmpGrid.Area()->CreateNewArea(bl, tr);
		if(tmpArea)
		{
			usedGrid = NFmiGrid(tmpArea, static_cast<unsigned long>(theValues.NX()), static_cast<unsigned long>(theValues.NY()));
			delete tmpArea;
		}
		else
			usedGrid = *theInfo->Grid(); // hätä paska ratkaisu, jos cropastusta alueesta tuleekin 0-pointteri
	}
	else
		usedGrid = *theInfo->Grid();

	return usedGrid;
}

bool NFmiStationView::GetCurrentDataMatrixFromQ2Server(NFmiDataMatrix<float> &theValues, NFmiGrid &usedGrid)
{
    if(IsQ2ServerUsed())
    {
        return GetArchiveDataFromQ3Server(theValues, usedGrid, false);
    }
    else
        return false;
}

bool NFmiStationView::IsStationDataGridded()
{
    if(itsDrawParam->StationDataViewType() != NFmiMetEditorTypes::View::kFmiTextView && itsInfo->IsGrid() == false)
        return true;
    else
        return false;
}

void NFmiStationView::CalculateGriddedStationData(NFmiDataMatrix<float> &theValues, NFmiGrid &usedGrid)
{
	auto& visSettings = GetVisualizationSettings();
	NFmiPoint usedGridSize = visSettings.getCheckedPossibleOptimizedGridSize(itsCtrlViewDocumentInterface->StationDataGridSize(), *itsArea, CalcViewGridSize(), itsCtrlViewDocumentInterface->BetaProductGenerationRunning());
    usedGrid = NFmiGrid(itsArea.get(), static_cast<unsigned long>(usedGridSize.X()), static_cast<unsigned long>(usedGridSize.Y()));
    theValues.Resize(static_cast<unsigned long>(usedGridSize.X()), static_cast<unsigned long>(usedGridSize.Y()), kFloatMissing);
    GridStationDataToMatrix(theValues, itsTime);
}

void NFmiStationView::CalculateDifferenceToOriginalDataMatrix(NFmiDataMatrix<float> &theValues, int x1, int y1, int x2, int y2, bool useCropping, NFmiGrid* optimizedDataGrid)
{
    FillDataMatrix(itsInfo, theValues, itsTime, useCropping, x1, y1, x2, y2, optimizedDataGrid);
    if(itsOriginalDataInfo)
    {
        NFmiDataMatrix<float> values2;
        FillDataMatrix(itsOriginalDataInfo, values2, itsTime, useCropping, x1, y1, x2, y2, optimizedDataGrid);
        ::CalcDiffMatrix(theValues, values2);
    }
    else
        theValues = 0; // tehdään 0-arvoiset luvut tulosmatriisiin, koska ei ole originaali arvoja käytössä
}

// Otetaan visualizationOptimazation:in harvennetut hilakoot tarvittavissa kohdissa käyttöön
bool NFmiStationView::CalcViewFloatValueMatrix(NFmiDataMatrix<float> &theValues, int x1, int y1, int x2, int y2, bool& useOriginalDataInPixelToGridRatioCalculations, NFmiGrid* optimizedDataGrid)
{
	itsMacroParamCalculationGrid.reset(); // nollataan aina tämä aluksi
	bool status = true;
    NFmiGrid usedGrid; // tämän avulla lasketaan maski laskut

	if(IsSpecialMatrixDataDraw())
	{
		// Tätä käytetään vain visualisoimaan editoidun datan valittuja pisteitä, täällä ei tehdä visualizationOptimazation harvennusta
		theValues = itsSpecialMatrixData;
        usedGrid = NFmiGrid(itsArea.get(), static_cast<unsigned long>(itsSpecialMatrixData.NX()), static_cast<unsigned long>(itsSpecialMatrixData.NY()));
	}
	else if(fGetCurrentDataFromQ2Server)
	{
        status = GetCurrentDataMatrixFromQ2Server(theValues, usedGrid);
	}
    else if(itsDrawParam->DataType() == NFmiInfoData::kQ3MacroParam) // HUOM! dataTypeä pitää kysyä drawParamilta, pitää tehdä makrolaskelmat ja antaa ne theValues-matriisille
    {
        status = GetQ3ScriptData(theValues, usedGrid, itsCtrlViewDocumentInterface->GetQ2ServerInfo().Q3ServerUrl());
    }
    else if(itsInfo)
	{
		if(IsStationDataGridded())
		{
            CalculateGriddedStationData(theValues, usedGrid);
		}
		else if(itsInfo->DataType() == NFmiInfoData::kMacroParam) // pitää tehdä makrolaskelmat ja antaa ne theValues-matriisille
		{
			CalcMacroParamMatrix(theValues, &usedGrid);
			itsMacroParamCalculationGrid.reset(new NFmiGrid(usedGrid));
		}
		else
		{
			bool useCropping = (x2 - x1 >= 1) && (y2 - y1 >= 1);
			if(itsDrawParam->ShowDifferenceToOriginalData())
			{
                CalculateDifferenceToOriginalDataMatrix(theValues, x1, y1, x2, y2, useCropping, optimizedDataGrid);
			}
			else
			{
				if(itsInfo->DataType() != NFmiInfoData::kStationary)
				{
                    FillDataMatrix(itsInfo, theValues, itsTime, useCropping, x1, y1, x2, y2, optimizedDataGrid);
				}
				else
				{
                    // Staattisille datoille (terrain datat jms.) ei tarvitse tehdä meta parametri tarkasteluja
					if(optimizedDataGrid)
					{
						// GridValues metodista ei löydy kuin aikainterpolaatio menetelmä, joten pitää pyytää eka aika datasta
						const auto& firstTime = itsInfo->TimeDescriptor().FirstTime();
						itsInfo->GridValues(theValues, *optimizedDataGrid, firstTime);
					}
					else if(useCropping)
						itsInfo->CroppedValues(theValues, x1, y1, x2, y2); // stat data on jo ajallisesti kohdallaan
					else
						itsInfo->Values(theValues); // stat data on jo ajallisesti kohdallaan
				}
			}

			if(optimizedDataGrid)
				usedGrid = *optimizedDataGrid;
			else
			{
				usedGrid = GetUsedGrid(usedGrid, itsInfo, theValues, useCropping, x1, y1, x2, y2);
				useOriginalDataInPixelToGridRatioCalculations = true;
			}
		}
	}

	if(status)
	{
		// piirrä vain maski alueelle juttu datan fixaus niin että ei maskatulle alueelle tulee puuttuvaa arvoa
		::FixDataWithMaskValues(itsTime, theValues, usedGrid, itsDrawParam, itsCtrlViewDocumentInterface->ParamMaskListMT());
	}
	return status;
}

void NFmiStationView::DoTimeInterpolationSettingChecks(boost::shared_ptr<NFmiFastQueryInfo>& theInfo)
{
	itsTimeInterpolationRangeInMinutes = NFmiStationView::GetTimeInterpolationRangeInMinutes(GetHelpDataInfo(theInfo));
	fAllowNearestTimeInterpolation = NFmiStationView::AllowNearestTimeInterpolation(itsTimeInterpolationRangeInMinutes);
}

// tämä asettaa tarvittavat jutut optimointia varten
void NFmiStationView::SetMapViewSettings(boost::shared_ptr<NFmiFastQueryInfo> &theUsedInfo)
{
	itsInfo = theUsedInfo;
	if(itsInfo == 0)
		return ;
	FmiProducerName prod = static_cast<FmiProducerName>(itsInfo->Producer()->GetIdent());
	auto shipTypeData = (itsInfo->IsGrid() == false && (prod == kFmiSHIP || prod == kFmiBUOY));
	auto flashTypeData = NFmiFastInfoUtils::IsLightningTypeData(itsInfo);
	if(shipTypeData || flashTypeData)
		fDoMovingStationDataLocations = true;
	else
		fDoMovingStationDataLocations = false;
	if(!IsMacroParamCase())
		if(!itsInfo->Param(static_cast<FmiParameterName>(itsDrawParam->Param().GetParamIdent())))
			return ;

	DoTimeInterpolationSettingChecks(itsInfo);

	if(itsDrawParam->ShowDifferenceToOriginalData() && itsDrawParam->DataType() == NFmiInfoData::kEditable)
	{
		//verrataan editdataa sittenkin vertailu dataan, jolloin voi ottaa 'valokuvia', ja vertailua voi suorittaa askel askeleelta
		itsOriginalDataInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->FindInfo(NFmiInfoData::kCopyOfEdited);
		if(itsOriginalDataInfo && itsInfo)
		{
			itsOriginalDataInfo->TimeIndex(itsInfo->TimeIndex());
			itsOriginalDataInfo->Param(itsInfo->Param());
			itsOriginalDataInfo->LevelIndex(itsInfo->LevelIndex());
		}
	}
}

#ifdef min
#undef min
#endif

static NFmiColor GetCustomClassColor(float value, const boost::shared_ptr<NFmiDrawParam> &drawParam)
{
    const auto &classes = drawParam->SpecialIsoLineValues();
    const auto &colorIndices = drawParam->SpecialIsoLineColorIndexies();
    if(classes.size() && colorIndices.size())
    {
        auto maxCheckedIndex = std::min(classes.size(), colorIndices.size());
        for(size_t i = 0; i < maxCheckedIndex; i++)
        {
            if(value <= classes[i])
                return ToolMasterColorCube::ColorIndexToRgb(colorIndices[i]);
        }

        // If loop goes through, return the last possible color
        if(classes.size() >= colorIndices.size())
            return ToolMasterColorCube::ColorIndexToRgb(colorIndices[colorIndices.size()-1]);
        else
            return ToolMasterColorCube::ColorIndexToRgb(colorIndices[maxCheckedIndex]);
    }
    return NFmiColor();
}

NFmiColor NFmiStationView::GetColoredNumberColor(float theValue) const
{ 
    if(!itsDrawParam->UseSimpleIsoLineDefinitions())
        return ::GetCustomClassColor(theValue, itsDrawParam);

	NFmiColor color(0, 0, 0); // musta on default väri
	float high = itsDrawParam->StationSymbolColorShadeHighValue();
	float mid = itsDrawParam->StationSymbolColorShadeMidValue();
	float low = itsDrawParam->StationSymbolColorShadeLowValue();
	if(theValue >= high)
		color = itsDrawParam->StationSymbolColorShadeHighValueColor();
	else if(theValue == mid)
		color = itsDrawParam->StationSymbolColorShadeMidValueColor();
	else if(theValue <= low)
		color = itsDrawParam->StationSymbolColorShadeLowValueColor();
	else if(itsDrawParam->StationSymbolColorShadeClassCount() <= 3) // jos ei ole haluttu jaotella väri alueita, annetaan välissä olevien arvojen väriksi middle väri
		color = itsDrawParam->StationSymbolColorShadeMidValueColor();
	else if(theValue < mid) // tällöin ollaan low ja mid:in välissä ja halutaan laskea väri tarkemmin
	{
		float lowColorFactor = (mid - theValue)/(mid - low);
		NFmiColor wantedColor(itsDrawParam->StationSymbolColorShadeMidValueColor());
		wantedColor.Mix(itsDrawParam->StationSymbolColorShadeLowValueColor(), lowColorFactor);
		color = wantedColor;
	}
	else // tällöin ollaan mid:in ja high:n välissä ja halutaan laskea väri tarkemmin
	{
		float midColorFactor = (high - theValue)/(high - mid);
		NFmiColor wantedColor(itsDrawParam->StationSymbolColorShadeHighValueColor());
		wantedColor.Mix(itsDrawParam->StationSymbolColorShadeMidValueColor(), midColorFactor);
		color = wantedColor;
	}
	return color;
}

static float GetSecondParameterColorValue(const boost::shared_ptr<NFmiFastQueryInfo> &primaryInfo, const boost::shared_ptr<NFmiFastQueryInfo>& colorValueInfo, const NFmiMetTime &theTime)
{
	if(NFmiFastInfoUtils::IsInfoShipTypeData(*colorValueInfo))
	{
		// Laivatyypeille ei tehdä raskasta lähimmän paikan etsimistä, vaan
		// katsotaan että jos molemmat datat ovat samoja, laitetaan locationIndex ja aikaIndex 
		// samoiksi ja pyydetään arvo siitä. Jos ei samoja datoja, palautetaan puuttuvaa.
		if(primaryInfo->DataFileName() == colorValueInfo->DataFileName())
		{
			colorValueInfo->LocationIndex(primaryInfo->LocationIndex());
			colorValueInfo->TimeIndex(primaryInfo->TimeIndex());
			return colorValueInfo->FloatValue();
		}
		return kFloatMissing;
	}

	return colorValueInfo->InterpolatedValue(primaryInfo->LatLon(), theTime);
}

// Jos symboli halutaan värittää toisen parametrin arvojen mukaisesti, mutta se
// sen toisen parametrin arvo on puuttuvaa, pitää tapaukselle keksiä joku puuttuva väri.
// Ajattelin, että pelkkä musta kuvaa parhaiten ongelmatilannetta, koska piirtoa ei haluta
// poistaa, mutta ei sitä kannata myöskään korostaa jollai räväkällä värilläkään.
const NFmiColor gSecondParameterMissingColor(0, 0, 0);

NFmiColor NFmiStationView::GetSymbolColor(float theValue) const
{
	if(itsDrawParam->ShowColoredNumbers())
	{
		// Vaihtelevanväriset numerot tekstinäyttöön
		if(itsPossibleColorValueInfo)
		{
			auto colorParamValue = ::GetSecondParameterColorValue(itsInfo, itsPossibleColorValueInfo, itsTime);
			if(colorParamValue == kFloatMissing)
				return gSecondParameterMissingColor;
			else
				theValue = colorParamValue;
		}
		return GetColoredNumberColor(theValue);
	}
	else
	{
		// "Tavalliset" värit näyttöön
		return GetBasicParamRelatedSymbolColor(theValue);
	}
}

NFmiColor NFmiStationView::GetBasicParamRelatedSymbolColor(float theValue) const
{
    return itsDrawParam->FrameColor();
}

NFmiString NFmiStationView::GetPrintedText(float theValue)
{
	NFmiString text;
	if(theValue == kFloatMissing)
	{
		text = " - ";
	}
	else
	{
		int digitCount = itsDrawParam->IsoLineLabelDigitCount(); // tekstille pitäisi saada oma lukunsa ja isoviivoille oma
		if(digitCount == 0)
            text = NFmiValueString(boost::math::iround(theValue), "%d");
		else
		{
			NFmiString format("%0.");
			format += NFmiValueString(digitCount, "%d");
			format += "f";
			text = NFmiValueString(theValue, format);
		}
	}
	return text;
}

void NFmiStationView::ModifyTextEnvironment(void)
{
}

// suoran kaksi pistettä on annettu ja x:n arvo, laske y:n arvo
static double LaskeYSuoralla(double x, double x1, double x2, double y1, double y2)
{
	double k = (y2-y1)/(x2-x1);
	double b = (x1*y2 - y1*x2)/(x1-x2);
	double y = k*x + b;
	return y;
}

NFmiPoint NFmiStationView::CalcFontSize(int theMinSize, int theMaxSize, bool fPrinting) const
{
	if(fPrinting)
	{
		// tämä on kikka vitonen printtaus piirtoon:
		// Ruudulle on piirretyy tähän mennessä symboleja ja tekstejä pikseli koossa.
		// Tämä on toiminut myös printtauksen yhteydessä, koska tähän mennessä on vain valmis kuva lähetetty
		// sellaisenaan printterille (-> WYSIWYG heh heh).
		// Nyt kun karttanäytölle pitää piirtää kunnolla esim. synop-plottia ja isoille lakanoille,
		// pitää piirtoa muuttaa ja hommat tehdään millimetri maailmassa.
		// Viritin tähän fontin laskuun sellaisen ratkaisun että lasketaan fontti koko suoraan
		// suhteellisen symboli koon mukaan. Mittasin että näytöllä 0.37 koko saa aikaan n. 1.9 mm
		// fontti korkeuden. 1.0 saa aikaan n. 5 mm fontin.
		double relSize = itsDrawParam->OnlyOneSymbolRelativeSize().Y();
		double fontSizeInMM = ::LaskeYSuoralla(relSize, 0.37, 1, 1.9, 5.0); // lineaarinen muunnos relSize 0.37 -> 1.9 mm ja relSize 1.0 -> 5.0 mm
		// LISÄKSI min ja max koot pitää laskea vielä
		// olemassa olevista ruudulle sopivista pikselikoista, kokeilemalla olen saanut
		// muunnoskaavan 12pix -> 1.9mm ja 30pix -> 5.0 mm
		double minSizeInMM = ::LaskeYSuoralla(theMinSize, 12, 30, 1.9, 5.0);
		double maxSizeInMM = ::LaskeYSuoralla(theMaxSize, 12, 30, 1.9, 5.0);

		fontSizeInMM = FmiMax(minSizeInMM, fontSizeInMM);
		fontSizeInMM = FmiMin(maxSizeInMM, fontSizeInMM);

        int y = boost::math::iround(fontSizeInMM * itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_y * 1.88);

		return NFmiPoint(y, y);
	}
	else
	{
		int x = static_cast<int>(itsToolBox->HX(CurrentDataRect().Width()/1.2));
		int y = static_cast<int>(itsToolBox->HY(CurrentDataRect().Height()/1.2));
		double tmp = (y + x) / 2.;
		tmp = tmp * ::LaskeYSuoralla(tmp, theMinSize, theMaxSize, 1, 0.9); // tämä funktio yrittää laittaa fontin kooksi lähelle optimia

		y = FmiMax(theMinSize, boost::math::iround(tmp));
		y = FmiMin(theMaxSize, y);
		return NFmiPoint(y, y);
	}
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiStationView::GetNearestQ2SynopStation(const NFmiLocation &theWantedLocation)
{
	if(itsInfoVector.size() == 0)
		MakeDrawedInfoVector(); // joskus pitää varmistaa, että info vektor on rakennettu

	// haetaan vain 2 ensimmäisestä datasta tooltip arvoja eli euro ja maailma datoista
	std::vector<boost::shared_ptr<NFmiFastQueryInfo> > tempInfoVec;
	if(itsInfoVector.size() >= 1)
		tempInfoVec.push_back(*itsInfoVector.begin());
	if(itsInfoVector.size() >= 2)
		tempInfoVec.push_back(*(itsInfoVector.begin()+1));
	boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->GetNearestSynopStationInfo(theWantedLocation, itsTime, true, &tempInfoVec);
	return info;
}

float NFmiStationView::ToolTipValue(const NFmiPoint& theRelativePoint, boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	if(fGetCurrentDataFromQ2Server)
	{
		if(IsQ2ServerUsed())
		{
			NFmiDataMatrix<float> values; // pyydetään data 2 x 2 hilaan ja otetaan 1. arvo tooltippiin
			NFmiGrid dummyGrid; // tämä vain pitää antaa, ei käyttöä
			GetArchiveDataFromQ3Server(values, dummyGrid, true);
			if(values.NX() && values.NX())
				return values[0][0];
		}
		return kFloatMissing;
	}
	else
	{
		if(theInfo == 0)
			return kFloatMissing;

		itsNearestTooltipLocation = NFmiLocation();
		NFmiLocation wantedLocation(ViewPointToLatLon(theRelativePoint));

		FmiProducerName prodId = static_cast<FmiProducerName>(theInfo->Producer()->GetIdent());
		boost::shared_ptr<NFmiFastQueryInfo> info = theInfo;
		if(prodId == kFmiSYNOP || prodId == kFmiSHIP || prodId == kFmiBUOY || prodId == kFmiTestBed)
		{
			info = itsCtrlViewDocumentInterface->GetNearestSynopStationInfo(wantedLocation, itsTime, true, 0);
			if(info == 0 && UseQ2ForSynopData(itsDrawParam))
			{
				info = GetNearestQ2SynopStation(wantedLocation);
				return GetSynopValueFromQ2Archive(info);
			}
		}
		else
			info->NearestLocation(wantedLocation);
		if(info == 0)
			return kFloatMissing;
		SetMapViewSettings(info); // tämän voisi varmaan optimoida
		PrepareForStationDraw(); // tämä pitää kutsua, että mm. parametrit on asetettu oikein itsInfo-olioon
		if(NFmiFastInfoUtils::IsLightningTypeData(itsInfo))
		{
			return GetTooltipValueForFlashTypeData(wantedLocation);
		}
		else
		{
			itsNearestTooltipLocation = *(itsInfo->Location());
			return ViewFloatValue(true);
		}
	}
}

#ifdef max
#undef max
#undef min
#endif

float NFmiStationView::GetTooltipValueForFlashTypeData(const NFmiLocation& theCursorLocation)
{
	double minimumDistance = std::numeric_limits<double>::max(); // iso luku tähän
	unsigned long minDistTimeIndex = gMissingIndex;
	if(FindNearestFlashTypeObservation(itsInfo, theCursorLocation, minimumDistance, minDistTimeIndex))
	{
		if(minimumDistance <= 100 * 1000)
		{
			itsInfo->TimeIndex(minDistTimeIndex);
			itsNearestTooltipLocation = NFmiLocation(itsInfo->GetLatlonFromData());
			std::string locationNameStr = CtrlViewUtils::GetFixedLatlonStr(itsNearestTooltipLocation.GetLocation());
			locationNameStr += " at ";
			locationNameStr += itsInfo->Time().ToStr("HH:mm:SS");
			itsNearestTooltipLocation.SetName(locationNameStr);
			return itsInfo->FloatValue();
		}
	}
	return kFloatMissing;
}

bool NFmiStationView::FindNearestFlashTypeObservation(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, const NFmiLocation& theCursorLocation, double& theCurrentMinDistInOut, unsigned long& theMinDistTimeIndexOut)
{
	bool status = false;
	unsigned long timeIndex1 = 0;
	unsigned long timeIndex2 = 0;
	if(GetTimeSpanIndexies(theInfo, timeIndex1, timeIndex2))
	{
		for(unsigned long i = timeIndex1; i <= timeIndex2; i++)
		{
			theInfo->TimeIndex(i);
			NFmiLocation flashLoc(theInfo->GetLatlonFromData());
			double dist = flashLoc.Distance(theCursorLocation);
			if(dist < theCurrentMinDistInOut)
			{
				theCurrentMinDistInOut = dist;
				theMinDistTimeIndexOut = i;
				status = true;
			}
		}
	}
	return status;
}

bool NFmiStationView::IsActiveParam(void)
{
	if(itsDrawParam)
		return itsDrawParam->IsActive();
	return false;
}

// kysytään näytön datalta, voidaanko dataa editoida
bool NFmiStationView::IsEditedDataParamView(void)
{
	if(itsDrawParam)
		return (itsDrawParam->DataType() == NFmiInfoData::kEditable);
	return false;
}

void NFmiStationView::DrawControlPointData(void)
{
	boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, false, false);
	if(info && IsActiveParam()) // onko näytön parametri aktiivinen?
	{
		boost::shared_ptr<NFmiEditorControlPointManager> CPMan = itsCtrlViewDocumentInterface->CPManager();
		if(CPMan && CPMan->Time(itsTime) && CPMan->Param(itsDrawParam->Param()))
		{
			bool macroParamCase = itsDrawParam->IsMacroParamCase(true);
			info->Time(itsTime); // pitää asettaa aika kohdalleen
			// lasketaan fontti koko
			int pixels = 1 + 2*(itsToolBox->HY(sqrt(itsArea->Width() * itsArea->Height()))/55);
			pixels = min(22, pixels);
			pixels = max(13, pixels);

			float height = static_cast<float>(itsToolBox->SY(pixels));
			// float width = itsToolBox->SX(pixels);
			itsDrawingEnvironment.SetFontType(kArial);
			NFmiPoint fontSize(pixels, pixels);
			NFmiPoint oldFontSize(itsDrawingEnvironment.GetFontWidth(), itsDrawingEnvironment.GetFontHeight());
			NFmiColor oldFillColor(itsDrawingEnvironment.GetFillColor());
			itsDrawingEnvironment.SetFillColor(NFmiColor(1,1,1)); // valkoinen tausta CP-teksti-lätkille
			itsDrawingEnvironment.SetFontSize(fontSize);
			bool oldBoldStatus = itsDrawingEnvironment.BoldFont();
			itsDrawingEnvironment.BoldFont(true);
			boost::shared_ptr<NFmiArea> zoomedArea = GetMapHandlerInterface()->Area();

			for(CPMan->ResetCP(); CPMan->NextCP();)
			{
				itsDrawingEnvironment.SetFontSize(fontSize);
				NFmiPoint latLonPoint(CPMan->LatLon());
				if(!zoomedArea->IsInside(latLonPoint))
					continue;
				NFmiPoint xy(itsArea->ToXY(latLonPoint));

				float changeValue = static_cast<float>(CPMan->ChangeValue());
				float actualValue = info->InterpolatedValue(latLonPoint);
				if(macroParamCase)
				{
					NFmiExtraMacroParamData extraMPData;
					auto macroParamValue = GetMacroParamValueFromCache(extraMPData, latLonPoint, itsTime);
					if(macroParamValue == g_MacroParamValueWasNotInCache)
					{
						macroParamValue = kFloatMissing;
					}
					actualValue = macroParamValue;
				}
				float modifiedValue = (actualValue != kFloatMissing) ? (changeValue + actualValue) : kFloatMissing;
				NFmiValueString str(changeValue, "%0.1f");
				NFmiValueString modifiedStr(modifiedValue, "%0.1f");
				NFmiValueString changeStr;
				if(modifiedValue > 0) // jos positiivinen luku, laitetaan varmuuden vuoksi space eteen, että saadaan mahdollinen '-'-merkki peittoon (piirron optimoinnista johtuva juttu)
					modifiedStr = NFmiString("  ") + modifiedStr;
				else if(modifiedValue > -10) // jos positiivinen luku, laitetaan varmuuden vuoksi space eteen, että saadaan mahdollinen '-'-merkki peittoon (piirron optimoinnista johtuva juttu)
					modifiedStr = NFmiString(" ") + modifiedStr;
				if(actualValue == kFloatMissing)
				{
					modifiedStr = "  -  ";
				}
				if(changeValue>0)
					changeStr = "+";
				changeStr += str;
				NFmiPoint textPoint(xy);
				textPoint.Y(textPoint.Y() - .30 * height);
				if(changeValue>0)
					itsDrawingEnvironment.SetFrameColor(NFmiColor(1.f,0.f,0.f));
				else if(changeValue<0)
					itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,1.f));
				else
					itsDrawingEnvironment.SetFrameColor(NFmiColor(0.3f,0.7f,0.2f));
				NFmiText text1(textPoint, changeStr, false, 0, &itsDrawingEnvironment);
				FmiDirection oldAligment = itsToolBox->GetTextAlignment();
				itsToolBox->SetTextAlignment((FmiDirection)(int(kBottomCenter) + 1000)); // + 1000 SUPERPIKAVIRITYS!!!!
				itsToolBox->Convert(&text1);  // muutos teksti (+0.5)
				textPoint.Y(textPoint.Y() + 0.6 * height);
				itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,0.f));
				NFmiText text2(textPoint, modifiedStr, false, 0, &itsDrawingEnvironment);
				itsToolBox->SetTextAlignment((FmiDirection)(int(kTopCenter) + 1000)); // + 1000 SUPERPIKAVIRITYS!!!!
				itsToolBox->Convert(&text2); // tuleva arvo teksti (12.4)

				// piirretään vielä CP:n indeksi näkyviin, mutta vain jos se halutaan
				if(CPMan->IsActivateCP() || CPMan->ShowCPAllwaysOnTimeView())
				{
					NFmiPoint fontSize2(pixels - 2, pixels - 2);
					itsDrawingEnvironment.SetFontSize(fontSize2);
					NFmiValueString indexStr(CPMan->CPIndex() + 1, "%d"); // +1 koska indeksit alkavat 0:sta
					textPoint.Y(textPoint.Y() - 0.72 * height);
					textPoint.X(textPoint.X() + 1.2 * height);
					itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,0.f));
					NFmiText text3(textPoint, indexStr, false, 0, &itsDrawingEnvironment);
					itsToolBox->Convert(&text3);
				}
				itsToolBox->SetTextAlignment(oldAligment);
			}
			itsDrawingEnvironment.SetFillColor(oldFillColor);
			itsDrawingEnvironment.SetFontSize(oldFontSize);
			itsDrawingEnvironment.BoldFont(oldBoldStatus);
		}
	}
}


bool NFmiStationView::SelectControlPointLocation(boost::shared_ptr<NFmiFastQueryInfo> &theInfo
													 ,int theSelectionCombineFunction
													 ,unsigned long theMask)
{
 // valitaan aktiivista CP-pistettä lähin piste (en keksi parempaakaan)
	NFmiLocationSelectionTool *tool = itsCtrlViewDocumentInterface->LocationSelectionTool2();
	FmiLocationSearchTool oldTool = tool->SelectedTool(); // Onko tämä oldTool jupina turhaa????
	tool->SelectLocations(theInfo
						 , itsCtrlViewDocumentInterface->CPManager()->ActiveCPLatLon()
						 ,itsArea
						 ,FmiSelectionCombineFunction(theSelectionCombineFunction)
						 ,theMask
						 , itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex));
	tool->SelectedTool(oldTool);
	return true;
}

void NFmiStationView::SelectLocations(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiPoint& theLatLon
									 ,int theSelectionCombineFunction
									 ,unsigned long theMask
									 ,bool fMakeMTAModeAdd // vain tietyistä paikoista kun tätä metodia kutsutaan, saa luotauksen lisätä (left buttom up karttanäytöllä lähinnä)
									 ,bool fDoOnlyMTAModeAdd)
{
	itsCtrlViewDocumentInterface->SelectLocations(itsMapViewDescTopIndex, theInfo, itsArea, theLatLon, itsTime, theSelectionCombineFunction, theMask, fMakeMTAModeAdd, fDoOnlyMTAModeAdd);
}

std::string NFmiStationView::Value2ToolTipString(float theValue, int theDigitCount, FmiInterpolationMethod theInterpolationMethod, FmiParamType theParamType)
{
	if(theValue == kFloatMissing)
		return "-";

	NFmiString str;
	if(theDigitCount == 0 && theInterpolationMethod != kLinearly && theParamType != kContinuousParam)
        str = NFmiValueString(boost::math::iround(theValue), "%d");
	else
    {
        int usedDecimals = theDigitCount;
        if(usedDecimals == 0)
            usedDecimals = (theValue > 1) ? 1 : 2;
        float rounder = 1;
        for(int i = 0; i < usedDecimals; i++)
            rounder /= 10.f;
        str = NFmiValueString::GetStringWithMaxDecimalsSmartWay(theValue, usedDecimals);
    }

	return std::string(str);
}

static void AddFilePathToTooltip(std::string &theTooltipStr, const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, bool getCurrentDataFromQ2Server)
{
    // Jos sekä CTRL- että SHIFT näppäimet on pohjassa, laitetaan tiedoston koko polku näkyviin omalle rivilleen
    if(CtrlView::IsKeyboardKeyDown(VK_CONTROL) && CtrlView::IsKeyboardKeyDown(VK_SHIFT))
    {
        if(getCurrentDataFromQ2Server)
            theTooltipStr += "\nData is retrieved from q2server (if possible)";
        else
        {
            std::string totalFilePath(NFmiFastInfoUtils::GetTotalDataFilePath(theInfo));
            if(!totalFilePath.empty())
            {
                theTooltipStr += "\n";
                theTooltipStr += totalFilePath;
            }
        }
    }
}

std::string NFmiStationView::GetLocationTooltipString()
{
    std::string locationStr = " (";
	if(!NFmiFastInfoUtils::IsLightningTypeData(itsInfo))
	{
		locationStr += std::to_string(itsNearestTooltipLocation.GetIdent());
		locationStr += " ";
	}
    locationStr += itsNearestTooltipLocation.GetName();
    locationStr += ")";
    // Location string must be xml encoded, because it might contain characters that will mess up with html output (like '<' and '>' characters)
    return CtrlViewUtils::XmlEncode(locationStr);
}

std::string NFmiStationView::MakeMacroParamErrorTooltipText(const std::string& macroParamErrorMessage)
{
	std::string str = "\"";
	str += "<font color = firebrick>";
	str += macroParamErrorMessage;
	str += "</font>";
	str += "\"";
	return str;
}

std::string NFmiStationView::MakeMacroParamDescriptionTooltipText(const NFmiExtraMacroParamData &extraMacroParamData)
{
	std::string str;
	if(!extraMacroParamData.MacroParamDescription().empty())
	{
		str += "<font color = magenta> (";
		// MacroParamDescription string must be xml encoded, because it might contain characters that will mess up with html output (like '<' and '>' characters)
		str += CtrlViewUtils::XmlEncode(extraMacroParamData.MacroParamDescription());
		str += ")</font>";
	}
	return str;
}

std::string NFmiStationView::MakeMacroParamTotalTooltipString(boost::shared_ptr<NFmiFastQueryInfo> &usedInfo, const std::string &paramName)
{
    NFmiExtraMacroParamData extraMacroParamData;
    itsInfo = usedInfo;
    float value = CalcMacroParamTooltipValue(extraMacroParamData, itsDrawParam);
	if(!extraMacroParamData.MacroParamErrorMessage().empty())
		return MakeMacroParamErrorTooltipText(extraMacroParamData.MacroParamErrorMessage());
	else
	{
		usedInfo = itsInfo;
		std::string str;
		float cacheValue = GetMacroParamTooltipValueFromCache(extraMacroParamData);
		if(cacheValue == g_MacroParamValueWasNotInCache)
		{
			auto valueStr = GetToolTipValueStr(value, usedInfo, itsDrawParam);
			str += valueStr;
			str += " (crude) ";
			str += GetPossibleMacroParamSymbolText(value, valueStr, extraMacroParamData);
		}
		else
		{
			auto cacheValueStr = GetToolTipValueStr(cacheValue, usedInfo, itsDrawParam);
			str += cacheValueStr;
			str += " (cache) ";
			str += GetPossibleMacroParamSymbolText(cacheValue, cacheValueStr, extraMacroParamData);
		}
		str += MakeMacroParamDescriptionTooltipText(extraMacroParamData);
		return str;
	}
}

static std::string MakeMapLayerTooltipText(CtrlViewDocumentInterface* ctrlViewDocumentInterface, const boost::shared_ptr<NFmiDrawParam>& drawParam)
{
	std::string str = "<b><font color=";
	str += ColorString::Color2HtmlColorStr(CtrlViewUtils::GetParamTextColor(NFmiInfoData::kMapLayer, false));
	str += ">";
	str += drawParam->ParameterAbbreviation();
	str += "</font></b>";
	return str;
}

std::string NFmiStationView::ComposeToolTipText(const NFmiPoint& theRelativePoint)
{
	itsToolTipDiffValue1 = kFloatMissing;
	itsToolTipDiffValue2 = kFloatMissing;
	string tabStr = "	";
	string str;
	if(itsDrawParam)
	{
		auto drawParamDataType = itsDrawParam->DataType();
		if(drawParamDataType == NFmiInfoData::kMapLayer)
			return ::MakeMapLayerTooltipText(itsCtrlViewDocumentInterface, itsDrawParam);
        bool showExtraInfo = CtrlView::IsKeyboardKeyDown(VK_CONTROL); // jos CTRL-näppäin on pohjassa, laitetaan lisää infoa näkyville
        bool macroParamCase = (drawParamDataType == NFmiInfoData::kMacroParam);
        auto parameterStr = CtrlViewUtils::GetParamNameString(itsDrawParam, false, showExtraInfo, true, 0, false, true, true, nullptr);
		parameterStr = DoBoldingParameterNameTooltipText(parameterStr);
		auto fontColor = CtrlViewUtils::GetParamTextColor(itsDrawParam->DataType(), itsDrawParam->UseArchiveModelData());
		parameterStr = AddColorTagsToString(parameterStr, fontColor, true);
		str += parameterStr + tabStr;
		NFmiLocation loc(itsCtrlViewDocumentInterface->ToolTipLatLonPoint());
		MakeDrawedInfoVector();
		boost::shared_ptr<NFmiFastQueryInfo> info = itsInfoVector.empty() ? boost::shared_ptr<NFmiFastQueryInfo>() : *itsInfoVector.begin();
		if(info) // satelliitti kuvilla ei ole infoa
		{
            if(fGetCurrentDataFromQ2Server || drawParamDataType == NFmiInfoData::kQ3MacroParam || info->NearestLocation(loc))
            {
                fDoTimeInterpolation = false;
                if(info->DataType() != NFmiInfoData::kStationary) // stationaari datalle ei tarvitse ajan osua, koska data on joka ajalle aina sama
                {
                    fDoTimeInterpolation = !(info->Time(itsCtrlViewDocumentInterface->ToolTipTime()));
                }
                if(macroParamCase)
                {
                    str += MakeMacroParamTotalTooltipString(info, str);
                }
                else
                {
                    float value = ToolTipValue(theRelativePoint, info);
                    str += GetToolTipValueStr(value, info, itsDrawParam);
                }
            }
			else
				str += "?";

            if(!macroParamCase)
            {
                auto paramType = static_cast<FmiParamType>(info->Param().Type());
                if(info->IsLocation())
                {
                    str += GetLocationTooltipString();
                }
                else if(paramType == kContinuousParam || paramType == kNumberParam)// gridistä otetaan myös interpoloitu arvo
                {
                    // Arkisto datasta ei tarvitse interpoloitua arvoa, koska nyt jo käytetään sitä
                    if(fGetCurrentDataFromQ2Server == false) 
                    {
                        NFmiPoint latlon = itsCtrlViewDocumentInterface->ToolTipLatLonPoint();
                        auto usedTime = NFmiFastInfoUtils::GetUsedTimeIfModelClimatologyData(info, itsTime);
                        float interpValue = InterpolatedToolTipValue(usedTime, latlon, info);
                        str += " (intp: ";
                        str += Value2ToolTipString(interpValue, itsDrawParam->IsoLineLabelDigitCount(), itsDrawParam->Param().GetParam()->InterpolationMethod(), paramType);
                        str += ")";
                    }
                }

                if(fGetCurrentDataFromQ2Server == false) // TODO arkisto datasta ei vielä lasketa havainto vertailua (LISÄÄ OMINAISUUS!)
                    str += GetCompareObservationToolTipString(info);

                AddLatestObsInfoToString(str);

                str += CtrlViewUtils::GetArchiveOrigTimeString(itsDrawParam, itsCtrlViewDocumentInterface, info, fGetCurrentDataFromQ2Server, "TempViewLegendTimeFormat");

                ::AddFilePathToTooltip(str, info, fGetCurrentDataFromQ2Server);
            }
        }
		else
			str += " Selected parameter or data is not currently available.";
	}
	return str;
}

std::string NFmiStationView::GetPossibleMacroParamSymbolText(float value, const std::string& valueStr, const NFmiExtraMacroParamData& extraMacroParamData)
{
	if(extraMacroParamData.IsMultiParamCase())
	{
		return GetMacroParamMultiParamText(value, valueStr, extraMacroParamData);
	}

	auto str = g_SymbolMappingsCache.getPossibleMacroParamSymbolText(value, extraMacroParamData.SymbolTooltipFile());
	if(str.empty())
		return str;
	else
	{
		// Fiksataan saatu pohjateksti niin että <,>, jne. merkit korvataan xml enkoodatuilla jutuilla (esim. '<' -> '&lt')
		str = CtrlViewUtils::XmlEncode(str);

		std::string decoratedStr = "(<b><font color=";
		decoratedStr += ColorString::Color2HtmlColorStr(g_MacroParamSymbolTextColor);
		decoratedStr += ">";
		decoratedStr += str;
		decoratedStr += "</font></b>)";
		return decoratedStr;
	}
}

float NFmiStationView::GetMultiParamValue(const MultiParamData& multiParam)
{
	if(multiParam.IsInUse())
	{
		if(multiParam.IsMacroParamCase())
		{
			NFmiExtraMacroParamData extraMacroParamData;
			auto usedDrawParamPtr = boost::make_shared<NFmiDrawParam>();
			usedDrawParamPtr->InitFileName(multiParam.possibleMacroParamFullPath());
			return CalcMacroParamTooltipValue(extraMacroParamData, usedDrawParamPtr);
		}
		else
		{
			const auto& paramData = multiParam.possibleParamData();
			NFmiDataIdent dataIdent(paramData.param_, paramData.producer_);
			auto* level = paramData.levelPtr_.get();
			auto dataType = paramData.dataType_;
			bool useParIdOnly = (dataType == NFmiInfoData::kEditable) || (dataType == NFmiInfoData::kCopyOfEdited);

			auto usedInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(dataIdent, level, dataType, useParIdOnly);
			if(usedInfo)
			{
				NFmiPoint latlon = itsCtrlViewDocumentInterface->ToolTipLatLonPoint();
				NFmiMetTime usedTime = itsCtrlViewDocumentInterface->ToolTipTime();
				return usedInfo->InterpolatedValue(latlon, usedTime);
			}
		}
	}
	return kFloatMissing;
}

std::string NFmiStationView::GetMacroParamMultiParamText(float multiParamValue1, const std::string& multiParamValue1Str, const NFmiExtraMacroParamData& extraMacroParamData)
{
	// MultiParam2 on pakollinen käydä läpi
	const auto& multiParam2 = extraMacroParamData.MultiParam2();
	auto multiParamValue2 = GetMultiParamValue(multiParam2);
	std::string multiParamValue2Str = (multiParamValue2 == kFloatMissing) ? " - " : NFmiValueString::GetStringWithMaxDecimalsSmartWay(multiParamValue2, 1);
	std::vector<float> multiParamValues{ multiParamValue1, multiParamValue2 };
	const auto& multiParam3 = extraMacroParamData.MultiParam3();
	std::string multiParamValue3Str;
	if(multiParam3.IsInUse())
	{
		auto multiParamValue3 = GetMultiParamValue(multiParam3);
		multiParamValues.push_back(multiParamValue3);
		multiParamValue3Str = (multiParamValue3 == kFloatMissing) ? " - " : NFmiValueString::GetStringWithMaxDecimalsSmartWay(multiParamValue3, 1);
	}
	auto str = g_SymbolMappingsCache.getPossibleMacroParamMultiParamText(multiParamValues, extraMacroParamData.MultiParamTooltipFile());
	if(str.empty())
		return str;
	else
	{
		// Fiksataan saatu pohjateksti niin että <,>, jne. merkit korvataan xml enkoodatuilla jutuilla (esim. '<' -> '&lt')
		str = CtrlViewUtils::XmlEncode(str);

		boost::replace_all(str, "%1", multiParamValue1Str);
		boost::replace_all(str, "%2", multiParamValue2Str);
		if(multiParam3.IsInUse())
		{
			boost::replace_all(str, "%3", multiParamValue3Str);
		}
		std::string decoratedStr = "(<b><font color=";
		decoratedStr += ColorString::Color2HtmlColorStr(g_MacroParamMultiParamBaseTextColor);
		decoratedStr += ">";
		decoratedStr += str;
		decoratedStr += "</font></b>)";
		return decoratedStr;
	}
}

void NFmiStationView::AddLatestObsInfoToString(std::string &tooltipString)
{
    std::string tmpLatestObsStr = CtrlViewUtils::GetLatestObservationTimeString(itsDrawParam, itsCtrlViewDocumentInterface, ::GetDictionaryString("YYYY.MM.DD HH:mm"), false);
    if(!tmpLatestObsStr.empty())
        tooltipString += "\n";
    tooltipString += tmpLatestObsStr;
}

float NFmiStationView::InterpolatedToolTipValue(const NFmiMetTime &theUsedTime, const NFmiPoint& theLatlon, boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
    float interpolatedValue = kFloatMissing;
    if(!(fDoTimeInterpolation && itsTimeInterpolationRangeInMinutes == 0))
    {
        auto paramId = itsDrawParam->Param().GetParamIdent();
        if(metaWindParamUsage.ParamNeedsMetaCalculations(paramId))
            interpolatedValue = NFmiFastInfoUtils::GetMetaWindValue(theInfo, theUsedTime, theLatlon, metaWindParamUsage, paramId);
        else
            interpolatedValue = fDoTimeInterpolation ? theInfo->InterpolatedValue(theLatlon, theUsedTime, itsTimeInterpolationRangeInMinutes) : theInfo->InterpolatedValue(theLatlon);
    }
    return interpolatedValue;
}

std::string NFmiStationView::GetToolTipValueStr(float theValue, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
	std::string str = "<b><font color=blue>";
	if(theValue != kFloatMissing)
	{
		NFmiString string2("");
		FmiParameterName param = FmiParameterName(theInfo->Param().GetParam()->GetIdent());
		NFmiStringList* list = 0;
		try
		{
			list = itsCtrlViewDocumentInterface->DataLists()->ParamValueList(param);
		}
		catch(std::exception & /* e */ )
		{
			// ei tehdä mitään, vaan jatketaan, huonoa koodia kaikin puolin
		}
		if(list)
		{
			if(list->FindWithStatus((int)theValue))
				string2 += NFmiString(*list->Current());
			str += string2;
		}
		else
		{
			FmiParamType parType = (FmiParamType)theDrawParam->Param().Type();
			str += Value2ToolTipString(theValue, theDrawParam->IsoLineLabelDigitCount(), theDrawParam->Param().GetParam()->InterpolationMethod(), parType);
		}
	}
	else
		str += "-";

	str += "</font></b>";

	return str;
}

// Hakee annetusta infosta annetusta paikasta, levelistä ja parametrista
// viimeiset n kpl ei-puuttuvia arvoja aikoineen. Jos arvoja ei saada
// kokoon ennen kuin theMaxTimeStepsBackWard lukema tulee vastaan, lopetetaan
// ja palautetaan false. Jos onnistuu, palautetaan true.
static bool GetValuesAndTimesFromBack(NFmiFastQueryInfo & theInfo, std::vector<double> &theValues, std::vector<NFmiMetTime> &theTimes, int theMaxTimeStepsBackWard, int theWantedValues)
{
	unsigned int oldTimeIndex = theInfo.TimeIndex();
	theInfo.LastTime();
	int timeIndexCounter = 1;
	int valuesFound = 0;
	do
	{
		float value = theInfo.FloatValue();
		if(value != kFloatMissing)
		{
			valuesFound++;

			// sijoitetaan arvot aika järjestyksessä taulukkoihin (HUOM! taulukkoihin on varattu etukäteen tarpeeksi tilaa!!)
			int index = theWantedValues-valuesFound;
			if(index < 0 || index >= static_cast<int>(theValues.size()))
				throw runtime_error("Virhe havaintojen extrapoloinnissa - GetValuesAndTimesFromBack index taulukon ulkopuolella.");
			theValues[index] = value;
			theTimes[index] = theInfo.Time();
			if(valuesFound >= theWantedValues)
			{
				theInfo.TimeIndex(oldTimeIndex); // palautetaan varmuden vuoksi vanha aika
				return true;
			}
		}
		timeIndexCounter++;
		if(!theInfo.PreviousTime())
			break;
	}while(timeIndexCounter < theMaxTimeStepsBackWard);

	theInfo.TimeIndex(oldTimeIndex); // palautetaan varmuden vuoksi vanha aika
	return false;
}

// muuttaa kaikki ajat sopiviksi etäisyys kertoimiksi Lagrange funktiota varten.
// theTimes-vektorin 1. aika on 0, theMapTime on 1, loput ajat lasketaan näiden suhteen
static void ConvertTimesToDistanceFactors(std::vector<NFmiMetTime> &theTimes, std::vector<double> &theDistanceFactors, const NFmiMetTime &theMapTime, double &theMapTimeDistanceFactor)
{
	theDistanceFactors[0] = 0.;
	theMapTimeDistanceFactor = 1.;
	double totalDiffInMinutes = static_cast<double>(theMapTime.DifferenceInMinutes(theTimes[0]));
	if(totalDiffInMinutes == 0)
		throw runtime_error("Virhe havaintojen extrapoloinnissa - ConvertTimesToDistanceFactors : nollalla jako.");
	int vecSize = static_cast<int>(theTimes.size());
	for(int i=1; i<vecSize; i++)
	{
		double diffInMinutes = static_cast<double>(theTimes[i].DifferenceInMinutes(theTimes[0]));
		theDistanceFactors[i] = diffInMinutes/totalDiffInMinutes;
	}
}

// normaali tapauksessa pyytää arvon obsInfosta.
// extrapolointi tapauksesta extrapoloi havainnoista arvon.
// Arvot skaalataan halutulla kertoimella lopuksi jos ei puuttuva arvo (lähinnä N-parametrin (kok. pilv.) takia).
static float GetObsComparisonValue(NFmiFastQueryInfo & theObsInfo, const NFmiObsComparisonInfo::Param& theParam, const NFmiMetTime &theMapTime, bool fExtrapolateObs)
{
	float obsValue = kFloatMissing;
	if(!fExtrapolateObs)
		obsValue = theObsInfo.FloatValue();
	else
	{
		const int kWantedValuesCount = 3;
		double maxMinutesBackWard = 12 * 60; // kuinka kauas kurkataan maksimissaan taaksepäin
		double timeStepInMinutes = theObsInfo.TimeResolution();
        int maxTimeStepsBackWard = boost::math::iround(maxMinutesBackWard / timeStepInMinutes); // huom! oletus että havainnot tasavälein esim. 1h resoluutio
		std::vector<double> values(kWantedValuesCount);
		std::vector<NFmiMetTime> times(kWantedValuesCount);
		try
		{
			if(GetValuesAndTimesFromBack(theObsInfo, values, times, maxTimeStepsBackWard, kWantedValuesCount))
			{
				if(theMapTime.DifferenceInHours(times[kWantedValuesCount-1]) <= 6) // tuoreimman löydetyn havainnon pitää olla vähintään 6h etäisyydellä extrapoloitavasta ajasta
				{
					std::vector<double> distanceFactors(kWantedValuesCount);
					double mapTimeDistanceFactor = 0;
					ConvertTimesToDistanceFactors(times, distanceFactors, theMapTime, mapTimeDistanceFactor);

					// oletus, lagrange otukselle annetaan vain kunnon arvoja, eikä puuttuvia, joten niitä ei tarvitse enää tarkistaa.
					NFmiLagrange lagrange;
					lagrange.Init(&distanceFactors[0], &values[0], static_cast<unsigned long>(values.size()));
					obsValue = static_cast<float>(lagrange.Interpolate(mapTimeDistanceFactor));
				}
			}
		}
		catch(exception & /* e */)
		{
			int x = 0; // tähän saa break pointin jos tarvis
			x++;
			// ei tehdä mitään, pitäisi tulla hälytys ohjelmoijalle
		}
	}

	if(obsValue != kFloatMissing)
		obsValue *= theParam.itsConversionFactor;

	if(fExtrapolateObs)
	{ // jos aproksimoitu arvo, pitää se tietyille parametreille laittaa rajoihinsa
		switch(theParam.itsId)
		{
		case kFmiHumidity:
		case kFmiTotalCloudCover:
			if(obsValue < 0)
				obsValue =  0;
			if(obsValue > 100)
				obsValue =  100;
			break;
		case kFmiWindSpeedMS:
		case kFmiPrecipitation1h:
			if(obsValue < 0)
				obsValue =  0;
			break;
		case kFmiWindDirection:
			if(obsValue < 0)
				obsValue =  360 + obsValue;
			if(obsValue > 360)
				obsValue -=  360;
			break;
		}
	}

	return obsValue;
}

// oletus eivät voi olla puuttuvia arvoja, on jo tarkastettu edellä
static float CalculateDiffValue(float theForValue, float theObsValue, FmiParameterName theParam)
{
	float diff = theForValue - theObsValue;
	if(theParam == kFmiWindDirection && diff > 180) // tuulen suunnassa pitää ottaa huomioon että tuuli kiertää 360 yli
		diff = theForValue - (theObsValue + 360);
	else if(theParam == kFmiWindDirection && diff < -180) // tuulen suunnassa pitää ottaa huomioon että tuuli kiertää 360 yli
		diff = theForValue - (theObsValue - 360);
	return diff;
}

void NFmiStationView::DrawObsComparison(void)
{
	NFmiObsComparisonInfo & obsComparisonInfo = itsCtrlViewDocumentInterface->ObsComparisonInfo();
	if((obsComparisonInfo.ComparisonMode() > 0 && itsMapViewDescTopIndex == 0) || (itsMapViewDescTopIndex > 0 && itsCtrlViewDocumentInterface->ShowObsComparisonOnMap(itsMapViewDescTopIndex)))
	{
		FmiParameterName param = static_cast<FmiParameterName>(itsDrawParam->Param().GetParamIdent());
		const NFmiObsComparisonInfo::Param& tmpParam = obsComparisonInfo.GetParam(param);
		if(tmpParam.itsId != kFmiBadParameter)
		{
			std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector = itsCtrlViewDocumentInterface->GetSortedSynopInfoVector(kFmiSYNOP, kFmiTestBed, kFmiSHIP, kFmiBUOY);
			for(size_t i = 0; i<infoVector.size(); i++)
			{
				boost::shared_ptr<NFmiFastQueryInfo> &obsInfo = infoVector[i];
				FmiProducerName prod = static_cast<FmiProducerName>(obsInfo->Producer()->GetIdent());
				bool useGetLocationFromData = false;
				if(obsInfo->IsGrid() == false && (prod == kFmiSHIP || prod == kFmiBUOY))
					useGetLocationFromData = true;
				boost::shared_ptr<NFmiFastQueryInfo> info = itsInfo;
				if(info && info->IsGrid() && info->SizeLevels() == 1 && obsInfo) // SizeLevels == 1 eli on pinta kamaa
				{
					obsInfo->FirstLevel(); // varmuuden vuoksi asetan 1. leveliin
					if(obsInfo->Param(param))
					{
						bool obsFound = obsInfo->Time(itsTime);
						bool extrapolateObs = false;
						if(!obsFound)
							extrapolateObs = IsThisTimeExtrapolated(itsTime, *obsInfo);
						if(obsFound || (extrapolateObs && obsComparisonInfo.ComparisonMode() == 2))
						{
							const int boxSizeXInPixels = boost::math::iround(obsComparisonInfo.SymbolSize().X());
							const int boxSizeYInPixels = boost::math::iround(obsComparisonInfo.SymbolSize().Y());
							double relativeBoxWidth = itsToolBox->SX(boxSizeXInPixels);
							double relativeBoxHeight = itsToolBox->SY(boxSizeYInPixels);
							if(obsComparisonInfo.DrawBorders())
								itsDrawingEnvironment.EnableFrame();
							else
								itsDrawingEnvironment.DisableFrame();
							NFmiColor oldFrameColor(itsDrawingEnvironment.GetFrameColor());
							itsDrawingEnvironment.SetFrameColor(obsComparisonInfo.FrameColor());
							itsDrawingEnvironment.EnableFill();
							NFmiRect aRect(0, 0, relativeBoxWidth, relativeBoxHeight);
							for(obsInfo->ResetLocation(); obsInfo->NextLocation(); )
							{
								NFmiPoint usedObsLatlon(useGetLocationFromData ? obsInfo->GetLatlonFromData() : obsInfo->LatLon());
								if(itsArea->IsInside(usedObsLatlon))
								{
									float obsValue = GetObsComparisonValue(*obsInfo, tmpParam, itsTime, extrapolateObs);
									float forValue = info->InterpolatedValue(usedObsLatlon, itsTime);
									if(obsValue != kFloatMissing && forValue != kFloatMissing)
									{
										if(!(param == kFmiWindDirection && obsValue == 0)) // ei tehdä WD vertailua jos tuulen suunta on 0, eli pläkä keli
										{
											aRect.Center(this->LatLonToViewPoint(usedObsLatlon));
											float diff = CalculateDiffValue(forValue, obsValue, param);
											NFmiColor compColor(extrapolateObs ? obsComparisonInfo.ExtrapolationOkColor() : obsComparisonInfo.OkColor());
											if(diff > tmpParam.itsHighLimit)
												compColor = extrapolateObs ? obsComparisonInfo.ExtrapolationOverHighColor() : obsComparisonInfo.OverHighColor();
											else if(diff < tmpParam.itsLowLimit)
												compColor = extrapolateObs ? obsComparisonInfo.ExtrapolationUnderLowColor() : obsComparisonInfo.UnderLowColor();
											itsDrawingEnvironment.SetFillColor(compColor);
											if(obsComparisonInfo.SymbolType() == 1)
											{
												NFmiRectangle aRec(aRect, 0, &itsDrawingEnvironment);
												itsToolBox->Convert(&aRec);
											}
											else
											{
												itsToolBox->DrawEllipse(aRect, &itsDrawingEnvironment);
											}
										}
									}
								}
							}
							itsDrawingEnvironment.EnableFrame();
							itsDrawingEnvironment.SetFrameColor(oldFrameColor);
						}
					}
				}
			}
		}
	}
}

bool NFmiStationView::IsThisTimeExtrapolated(const NFmiMetTime &theMapTime, NFmiFastQueryInfo & theObsInfo)
{
	int timeDiffInHours = theMapTime.DifferenceInHours(theObsInfo.TimeDescriptor().LastTime());
	if(timeDiffInHours >= 1 && timeDiffInHours <= 2)
		return true;
	return false;
}

std::string NFmiStationView::GetCompareObservationToolTipString(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	NFmiObsComparisonInfo & obsComparisonInfo = itsCtrlViewDocumentInterface->ObsComparisonInfo();
	string str;
	if(obsComparisonInfo.ComparisonMode() > 0)
	{
		FmiParameterName param = static_cast<FmiParameterName>(itsDrawParam->Param().GetParamIdent());
		const NFmiObsComparisonInfo::Param& tmpParam = obsComparisonInfo.GetParam(param);
		if(tmpParam.itsId != kFmiBadParameter)
		{
			NFmiLocation wantedLocation(itsCtrlViewDocumentInterface->ToolTipLatLonPoint());
			boost::shared_ptr<NFmiFastQueryInfo> obsInfo = itsCtrlViewDocumentInterface->GetNearestSynopStationInfo(wantedLocation, itsTime, false, 0);
			if(theInfo && theInfo->IsGrid() && theInfo->SizeLevels() == 1  && obsInfo) // SizeLevels == 1 eli on pinta kamaa
			{
				obsInfo->FirstLevel(); // varmuuden vuoksi asetan 1. leveliin
				if(obsInfo->Param(param))
				{
					bool obsFound = obsInfo->Time(itsTime);
					bool extrapolateObs = false;
					if(!obsFound)
						extrapolateObs = IsThisTimeExtrapolated(itsTime, *obsInfo);
					if(obsFound || (extrapolateObs && obsComparisonInfo.ComparisonMode() == 2))
					{
						if(obsInfo->NearestLocation(wantedLocation))
						{
							float obsValue = GetObsComparisonValue(*obsInfo, tmpParam, itsTime, extrapolateObs);
							float forValue = theInfo->InterpolatedValue(obsInfo->LatLon());
							float diff = CalculateDiffValue(forValue, obsValue, param);
							str += "\n# f:";
							if(forValue == kFloatMissing)
								str += "-";
							else
								str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(forValue, tmpParam.itsDecimalCount);
							if(extrapolateObs)
								str += " apx-o:";
							else
								str += " o:";
							if(obsValue == kFloatMissing)
								str += "-";
							else
								str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(obsValue, tmpParam.itsDecimalCount);
							str += " d:";
							if(obsValue == kFloatMissing || forValue == kFloatMissing)
								str += "-";
							else if(param == kFmiWindDirection && obsValue == 0) // ei tehdä WD vertailua jos tuulen suunta on 0, eli pläkä keli
								str += "-";
							else
								str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(diff, tmpParam.itsDecimalCount);
							str += " s:";
							str += NFmiStringTools::Convert(obsInfo->Location()->GetIdent());
							str += " ";
							str += obsInfo->Location()->GetName();
							str += "#";
						}
					}
				}
			}
		}
	}
	return str;
}

const NFmiPoint NFmiStationView::CurrentLatLon() const
{
    return CurrentLatLon(itsInfo);
}

const NFmiPoint NFmiStationView::CurrentLatLon(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo) const
{
	if(fDoMovingStationDataLocations)
        return theInfo->GetLatlonFromData();
	else
		return theInfo->LatLon();
}

bool NFmiStationView::IsSpecialMatrixDataDraw(void) const
{
	return itsParamId == static_cast<FmiParameterName>(NFmiInfoData::kFmiSpMatrixDataDraw);
}

bool NFmiStationView::IsAccessoryStationDataDrawn()
{
    return itsCtrlViewDocumentInterface->Registry_ShowStationPlot(itsMapViewDescTopIndex) && IsActiveParam() && (IsSpecialMatrixDataDraw() == false);
}

int NFmiStationView::CalcViewGridSize()
{
	auto viewGridSize = itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex);
	return int(viewGridSize.X() * viewGridSize.Y());
}

// ******** Symbol-Bulk-Draw toimintojen alku *********

// Tässä metodissa tehdään kaikki mahdolliset S-B-D asetukset, mitkä 
// tehdään vain kerran ja ennen kuin datoja aletaan käydä läpi.
void NFmiStationView::SbdDoFixedSymbolDrawSettings()
{
	itsSymbolBulkDrawData.relativePositionOffset(SbdCalcDrawObjectOffset());
	itsSymbolBulkDrawData.isChangingSymbolColorsUsed(SbdIsChangingSymbolColorsUsed());
	SbdGetStationDrawSettings();
	SbdSetPossibleFixedSymbolColor();
	SbdSetPossibleFixedSymbolSize();
	SbdSetDrawType();
	SbdSetFontName();
	itsSymbolBulkDrawData.penSize(SbdCalcFixedPenSize());
	itsSymbolBulkDrawData.printing(itsCtrlViewDocumentInterface->Printing());
	itsSymbolBulkDrawData.mapViewSizeInPixels(itsCtrlViewDocumentInterface->MapViewSizeInPixels(itsMapViewDescTopIndex));
	SetupPossibleColorValueInfo();
}

void NFmiStationView::SbdCollectSymbolDrawData(bool doStationPlotOnly)
{
	try
	{
		// Nämä haluttaisiin tehdä vain kerran, mutta koska monet asetukset vaativat 
		// että itsInfo on jo asetettu, tämä joudutaan tekemään potentiaalisesti monen 
		// fastInfon kanssa (synop data tapauksessa).
		SbdDoFixedSymbolDrawSettings();

		if(doStationPlotOnly)
		{
			// Tänne tullaan (doStationPlotOnly = true) kun piirretään isoviiva/contour juttuja
			// ja silloin halutaan piirtää kaikki datan pisteet näkyviin.
			if(itsOptimizedGridPtr)
				SbdCollectOptimizedGridStationPoints();
			else
				SbdCollectNormalSymbolDrawData(doStationPlotOnly);
		}
		else if(IsSpaceOutDrawingUsed())
		{
			SbdCollectSpaceOutSymbolDrawData(doStationPlotOnly);
		}
		else
		{
			if(NFmiFastInfoUtils::IsLightningTypeData(itsInfo))
				SbdCollectFlashTypeSymbolDrawData(doStationPlotOnly);
			else
				SbdCollectNormalSymbolDrawData(doStationPlotOnly);
		}
	}
	catch(...)
	{
	}
}

NFmiMetTime NFmiStationView::CalcStartTimeOfTimeSpan() const
{
	auto startTime(itsTime);
	startTime.SetTimeStep(1);
	auto usedTimeStepInMinutes = itsCtrlViewDocumentInterface->TimeControlTimeStepInMinutes(itsMapViewDescTopIndex);
	startTime.ChangeByMinutes(-usedTimeStepInMinutes);
	return startTime;
}

// Palauttaa aika ikkunalle: (curret-time - time-step) - current-time , ne aika-indeksit, jotka sopivat mainitulle välille
bool NFmiStationView::GetTimeSpanIndexies(const boost::shared_ptr<NFmiFastQueryInfo>& theInfo, unsigned long& theStartIndexOut, unsigned long& theEndIndexOut)
{
	auto startTime = CalcStartTimeOfTimeSpan();
	auto usedTimeStepInMinutes = itsCtrlViewDocumentInterface->TimeControlTimeStepInMinutes(itsMapViewDescTopIndex);
	return NFmiFastInfoUtils::FindTimeIndicesForGivenTimeRange(theInfo, startTime, usedTimeStepInMinutes, theStartIndexOut, theEndIndexOut);
}

void NFmiStationView::SbdCollectFlashTypeSymbolDrawData(bool doStationPlotOnly)
{
	unsigned long startIndex = gMissingIndex;
	unsigned long endIndex = gMissingIndex;
	if(GetTimeSpanIndexies(itsInfo, startIndex, endIndex))
	{
		for(unsigned long i = startIndex; i <= endIndex; i++)
		{
			itsInfo->TimeIndex(i);
			if(SbdIsInsideEnlargedDrawArea())
			{
				NFmiFastInfoUtils::SetSoundingDataLevel(itsDrawParam->Level(), *itsInfo); // Tämä tehdään vain luotaus datalle: tämä level pitää asettaa joka pisteelle erikseen, koska vakio painepinnat eivät ole kaikille luotaus parametreille samoilla leveleillä
				SbdCollectStationData(doStationPlotOnly);
			}
		}
	}
}

void NFmiStationView::SbdCollectNormalSymbolDrawData(bool doStationPlotOnly)
{
	NFmiIgnoreStationsData& ignoreStationData = itsCtrlViewDocumentInterface->IgnoreStationsData();
	for(itsInfo->ResetLocation(); itsInfo->NextLocation();)
	{
		if(SbdIsInsideEnlargedDrawArea())
		{
			if(!::IsCurrentStationBlocked(itsInfo, ignoreStationData))
			{
				NFmiFastInfoUtils::SetSoundingDataLevel(itsDrawParam->Level(), *itsInfo); // Tämä tehdään vain luotaus datalle: tämä level pitää asettaa joka pisteelle erikseen, koska vakio painepinnat eivät ole kaikille luotaus parametreille samoilla leveleillä
				SbdCollectStationData(doStationPlotOnly);
			}
		}
	}
}

void NFmiStationView::SbdCollectOptimizedGridStationPoints()
{
	if(itsOptimizedGridPtr)
	{
		for(itsOptimizedGridPtr->Reset(); itsOptimizedGridPtr->Next(); )
		{
			NFmiPoint xy(LatLonToViewPoint(itsOptimizedGridPtr->LatLon()));
			itsSymbolBulkDrawData.addRelativeStationPointPosition(xy);
		}
	}
}

void NFmiStationView::SbdGetStationDrawSettings()
{
	itsSymbolBulkDrawData.drawStationPoint(IsAccessoryStationDataDrawn());
	if(itsSymbolBulkDrawData.drawStationPoint())
	{
		itsSymbolBulkDrawData.stationPointColor(itsCtrlViewDocumentInterface->StationPointColor(itsMapViewDescTopIndex));
		itsSymbolBulkDrawData.baseStationPointRect(SbdCalcBaseStationRelativeRect());
	}
}

NFmiRect NFmiStationView::SbdCalcBaseStationRelativeRect()
{
	// Perus asemapisteen koon lasku
	NFmiPoint pointSize(itsCtrlViewDocumentInterface->StationPointSize(itsMapViewDescTopIndex));
	if(itsCtrlViewDocumentInterface->Printing())
	{
		// Mikä on näyttöruudulla olevan pikselin koko [mm]:ssä?
		// Arvioidaan se karkeasti mittaamalla esimerkki näytöstä ja lasketaan sen avulla printtauksessa käytettyjen pikseleiden määrä.
		const double monitorPixelHeightInMM = 0.2;
		auto monitorPixelCount = pointSize.Y();
		auto plotHeightInMM = monitorPixelCount * monitorPixelHeightInMM;
		auto printerPixelCount = plotHeightInMM * GetGraphicalInfo().itsPixelsPerMM_y;
		pointSize = NFmiPoint(printerPixelCount, printerPixelCount);
	}
	return NFmiRect(0, 0, itsToolBox->SX(boost::math::iround(pointSize.X())), itsToolBox->SY(boost::math::iround(pointSize.Y())));
}

// Symbolipiirto halutaan laajentaa hieman zoomatun kartta-alueen ulkopuolelle, jotta
// pikkuisen zoomatun alueen ulkopuolelle olevien asemien/pisteiden arvoista saadaan jotain näkyviin.
NFmiRect NFmiStationView::SbdCalcEnlargedDrawArea()
{
	NFmiRect enlargedRect = itsArea->XYArea();
	const double fractionValue = 0.02;
	enlargedRect.Inflate(enlargedRect.Width() * fractionValue, enlargedRect.Height() * fractionValue);
	return enlargedRect;

}

bool NFmiStationView::SbdIsInsideEnlargedDrawArea() const
{
	return itsEnlargedDrawArea.IsInside(CurrentStationPosition());
}

void NFmiStationView::SbdCollectStationData(bool doStationPlotOnly, float overrideValue)
{
	itsSymbolBulkDrawData.addRelativeStationPointPosition(CurrentStationPosition());
	if(!doStationPlotOnly)
	{
		float value = (overrideValue == kFloatMissing) ? ViewFloatValue(false) : overrideValue;
		itsSymbolBulkDrawData.addValue(value);

		// Jos arvo puuttuvaa, lisätään tekstiksi tyhjä stringi, jota ei piirretä.
		// Puuttuvat on kuitenkin lisättävä listaan, koska mahdollinen asemapiste 
		// piirretään myös puuttuvien arvojen kohdalle.
		if(value == kFloatMissing)
			itsSymbolBulkDrawData.addDrawnText(std::string(""));
		else
		{
			NFmiString text(GetPrintedText(value));
			itsSymbolBulkDrawData.addDrawnText(std::string(text));
		}

		itsSymbolBulkDrawData.addStationPointLatlons(CurrentLatLon());

		if(itsSymbolBulkDrawData.isChangingSymbolColorsUsed())
		{
			// Vaihtelevan väriset numerot tekstipiirtoon
			itsSymbolBulkDrawData.addColor(GetSymbolColor(value));
		}

		if(!SbdIsFixedSymbolSize())
		{
			// Symbolin arvosta riippuvan fontti koon lasku
			itsSymbolBulkDrawData.addSymbolSize(SbdCalcChangingSymbolSize(value));
		}
	}
}

#ifdef max
#undef max
#undef min
#endif max

void NFmiStationView::SbdPeekSparseValue(int peekIndexX, int peekIndexY, const NFmiRect& peekRect, PeekSparseValueDistanceList& nonMissingValuesWithDistance)
{
	NFmiPoint peekIndexPoint(peekIndexX, peekIndexY);
	if(peekRect.IsInside(peekIndexPoint))
	{
		auto oldLocationIndex = itsInfo->LocationIndex();
		auto peekedLocationIndex = itsInfo->PeekLocationIndex(peekIndexX, peekIndexY);
		if(itsInfo->LocationIndex(peekedLocationIndex))
		{
			auto value = ViewFloatValue(false);
			if(value != kFloatMissing)
			{
				double distanceApproximation = peekIndexX * peekIndexX + peekIndexY * peekIndexY;
				nonMissingValuesWithDistance.push_back(std::make_tuple(value, distanceApproximation, NFmiPoint(peekIndexX, peekIndexY)));
			}
		}
		itsInfo->LocationIndex(oldLocationIndex);
	}
}

static NFmiPoint CalcGridPoint(boost::shared_ptr<NFmiFastQueryInfo>& fastInfo)
{
	auto locationIndex = fastInfo->LocationIndex();
	if(fastInfo->IsGrid())
	{
		long xIndex = locationIndex % fastInfo->GridXNumber();
		long yIndex = locationIndex / fastInfo->GridXNumber();
		return NFmiPoint(xIndex, yIndex);
	}
	else
	{
		return NFmiPoint(locationIndex, 1);
	}
}

void NFmiStationView::SbdSearchForSparseSymbolDrawData(bool doStationPlotOnly, const NFmiRect& peekRect, SparseDataGrid& sparseDataGrid, int currentSkipColumn, int currentSkipLine)
{
	auto value = ViewFloatValue(false);
	auto drawedGridPoint = ::CalcGridPoint(itsInfo);
	if(value == kFloatMissing)
	{
		// piirtopisteestä ei löytynyt arvoa, sitten etsitään ei-puuttuvaa arvoa 
		// hakualihilasta kunnes sellainen löytyy.
		int maxIndex = std::max(int(peekRect.Right()), int(peekRect.Bottom()));
		// Käydään etsintäalihila läpi sisimmästä 'renkaasta' uloimpaan eli alkaen 1:stä. 0,0 -pistettä ei tarvitse käydä enää läpi.
		for(int ringIndex = 1; ringIndex <= maxIndex; ringIndex++)
		{
			PeekSparseValueDistanceList nonMissingValuesWithDistance;
			// Kierretään kukin 'rengas' läpi alkaen vasen-alakulmasta vastapäivään.
			// Käydään koko rengas läpi ja katsotaan löytyikö ei puuttuvia arvoja. 
			// Katsotaan vielä mikä piste (jos ei-puuttuvia oli useassa pisteessä) oli lähimpänä keskipistettä ja valitaan sen arvo.
			for(int index = -ringIndex; index <= ringIndex; index++)
			{
				// Alareuna tapaus
				SbdPeekSparseValue(index, -ringIndex, peekRect, nonMissingValuesWithDistance);
				// Yläreuna tapaus
				SbdPeekSparseValue(index, ringIndex, peekRect, nonMissingValuesWithDistance);
				// Kulmatapauksia ei tarvitse käydä läpi kahdesti
				if(index != -ringIndex && index != ringIndex)
				{
					// Vasen reuna tapaus
					SbdPeekSparseValue(-ringIndex, index, peekRect, nonMissingValuesWithDistance);
					// Oikea reuna tapaus
					SbdPeekSparseValue(ringIndex, index, peekRect, nonMissingValuesWithDistance);
				}
			}
			if(!nonMissingValuesWithDistance.empty())
			{
				nonMissingValuesWithDistance.sort([](const auto& valueDist1, const auto& valueDist2) {return std::get<1>(valueDist1) < std::get<1>(valueDist2); });
				auto minDistance = std::get<1>(nonMissingValuesWithDistance.front());
				auto value = std::get<0>(nonMissingValuesWithDistance.front());
				std::list<NFmiPoint> peekIndexList;
				for(const auto& tupleValue : nonMissingValuesWithDistance)
				{
					if(minDistance >= std::get<1>(tupleValue))
						peekIndexList.push_back(std::get<2>(tupleValue));
				}
				SparseData sparseData(value, drawedGridPoint, peekIndexList, itsInfo->LocationIndex());
				sparseDataGrid.setData(currentSkipColumn, currentSkipLine, sparseData);
				return;
			}
		}
	}

	// Ei löytynyt arvoja alihilasta, laitetaan puuttuvan arvon tietoja sitten datapakettiin (tarvitaan ainakin station point piirroissa)
	std::list<NFmiPoint> peekIndexList{ NFmiPoint(0, 0) };
	SparseData sparseData(value, drawedGridPoint, peekIndexList, itsInfo->LocationIndex());
	sparseDataGrid.setData(currentSkipColumn, currentSkipLine, sparseData);
}

NFmiPoint NFmiStationView::SbdCalcDrawObjectOffset() const
{
	NFmiPoint offset = CurrentDataRect().Center();
	// CurrentDataRect:issa on mukana symbolipiirtoon liittyvät offsetit.
	offset -= CurrentStationPosition();

	// Seuraava vertikaalisuunnassa tehtävä korjaus voidaan tehdä vain vakio kokoiselle tekstisymbolille.
	// Muuttuva kokoinen tekstisymboli pitää korjata erikseen symbolikohtaisesti piirrossa.
	if(SbdIsFixedSymbolSize())
	{
		// Normi tekstille pelkkä toolbox-alignmentti center (eikä mikään muukaan) vie tekstiä 
		// keskelle y-suunnassa, joten tämä siirros siirtää tekstin ihan keskelle
		auto moveInY = itsToolBox->SY(boost::math::iround(SbdCalcFixedSymbolSize().Y()/2.f));
		offset.Y(offset.Y() - moveInY);
	}
	return offset;
}

void NFmiStationView::SbdSetPossibleFixedSymbolColor()
{
	if(!itsSymbolBulkDrawData.isChangingSymbolColorsUsed())
		itsSymbolBulkDrawData.setColor(itsDrawParam->FrameColor());
}

bool NFmiStationView::SbdIsChangingSymbolColorsUsed() const
{
	switch(SbdGetSymbolColorChangingType())
	{
	case NFmiSymbolColorChangingType::Never:
	{
		return false;
	}
	case NFmiSymbolColorChangingType::OnlyWithOtherParameterValues:
	{
		return itsDrawParam->ShowColoredNumbers() && itsDrawParam->IsPossibleColorValueParameterValid();
	}
	case NFmiSymbolColorChangingType::Mixed:
	{
		return true;
	}
	case NFmiSymbolColorChangingType::DrawParamSet:
	{
		return itsDrawParam->ShowColoredNumbers();
	}
	default:
	{
		return false;
	}
	}
}

bool NFmiStationView::SbdIsFixedSymbolSize() const
{
	return true;
}

NFmiPoint NFmiStationView::SbdCalcFixedSymbolSize() const
{
	// Tämä emoluokan toteutus on dummy, palautetaan iso luku, niin mahd. bugit on helpompi huomata.
	return NFmiPoint(100, 100);
}

NFmiPoint NFmiStationView::SbdCalcFixedRelativeDrawObjectSize() const
{
	return CurrentDataRect().Size();
}

int NFmiStationView::SbdCalcFixedPenSize() const
{
	// Oletus on aina 1:n kokoinen pen size, ei ruveta laittamaan sekopäisiä arvoja.
	// Tätä arvoa käytetään lähinnä tuulinuoli ja windbarb piirroissa
	return 1;
}

NFmiPoint NFmiStationView::SbdCalcChangingSymbolSize(float value) const
{
	// Tämä emoluokan toteutus on dummy, palautetaan vain fiksattu fontti koko.
	return SbdCalcFixedSymbolSize();
}

void NFmiStationView::SbdSetPossibleFixedSymbolSize()
{
	if(SbdIsFixedSymbolSize())
	{
		itsSymbolBulkDrawData.setSymbolSize(SbdCalcFixedSymbolSize());
	}

	itsSymbolBulkDrawData.relativeDrawObjectSize(SbdCalcFixedRelativeDrawObjectSize());
}

NFmiPoint NFmiStationView::SbdBasicSymbolSizeCalculation(int minSize, int maxSize) const
{
	auto fontSize = CalcFontSize(minSize, boost::math::iround(MaximumFontSizeFactor() * maxSize), itsCtrlViewDocumentInterface->Printing()).Y();
	return NFmiPoint(fontSize, fontSize);
}

void NFmiStationView::SbdSetDrawType()
{
	itsSymbolBulkDrawData.drawType(SbdGetDrawType());
}

NFmiSymbolBulkDrawType NFmiStationView::SbdGetDrawType() const
{
	return NFmiSymbolBulkDrawType::Text;
}

// Jotkut symbolit piirretään monivärisinä ja jotkut ei. Tämän tyypin 
// perusteella voidaan päätellä symbolityypin tapaukset erikseen ja lopullinen
// värien käyttö päätellä SbdIsChangingSymbolColorsUsed metodissa.
NFmiSymbolColorChangingType NFmiStationView::SbdGetSymbolColorChangingType() const
{
	return NFmiSymbolColorChangingType::DrawParamSet;
}
 
void NFmiStationView::SbdSetFontName()
{
	itsSymbolBulkDrawData.fontName(L"Arial");
}

void NFmiStationView::SbdDoImageBasedSymbolDraw()
{
	try
	{
		// Varmistetaan itsGdiPlusGraphics:in alustus
		if(itsGdiPlusGraphics == nullptr)
			InitializeGdiplus(itsToolBox, &itsRect);
		GdiplusStationBulkDraw::Draw(itsSymbolBulkDrawData, itsGdiPlusGraphics, *itsToolBox);
	}
	catch(std::exception &e)
	{
		std::string errorMessage = "Error in ";
		errorMessage += __FUNCTION__;
		errorMessage += " : ";
		errorMessage += e.what();
		CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Visualization, true);
	}
	catch(...)
	{
		std::string errorMessage = "Unknown error in ";
		errorMessage += __FUNCTION__;
		CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Visualization, true);
	}
	CleanGdiplus();
	// Image pohjaisille symboleille pitää piirtää erikseen asemapisteet jos niin säädetty.
	if(itsSymbolBulkDrawData.drawStationPoint())
		itsToolBox->DoSymbolBulkDraw(itsSymbolBulkDrawData, true);
}

void NFmiStationView::SbdDoSymbolDraw(bool doStationPlotOnly)
{
	if(doStationPlotOnly)
		itsToolBox->DoSymbolBulkDraw(itsSymbolBulkDrawData, true);
	else if(itsSymbolBulkDrawData.isDataOk())
	{
		if(itsSymbolBulkDrawData.hasAnyData())
		{
			if(itsSymbolBulkDrawData.drawType() < NFmiSymbolBulkDrawType::BitmapSymbol1)
			{
				itsToolBox->DoSymbolBulkDraw(itsSymbolBulkDrawData, doStationPlotOnly);
			}
			else
			{
				SbdDoImageBasedSymbolDraw();
			}
		}
	}
	else
	{
		std::string errorMessage = __FUNCTION__;
		errorMessage += " had error with SymbolBulkDrawData: ";
		errorMessage += itsSymbolBulkDrawData.errorMessage();
		CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Visualization, true);
	}
}

// ******** Symbol-Bulk-Draw toimintojen loppu *********

static bool IsIsolinesDrawn(const NFmiIsoLineData& theOrigIsoLineData)
{
	if(theOrigIsoLineData.fUseIsoLines)
		return true;
	return false;
}

static bool IsIsolinesDrawn(const boost::shared_ptr<NFmiDrawParam>& thePossibleDrawParam)
{
	auto gridDataDrawStyle = thePossibleDrawParam->GridDataPresentationStyle();
	return gridDataDrawStyle == NFmiMetEditorTypes::View::kFmiIsoLineView || gridDataDrawStyle == NFmiMetEditorTypes::View::kFmiColorContourIsoLineView;
}

bool NFmiStationView::IsolineDataDownSizingNeeded(const NFmiIsoLineData& theIsoLineData, const NFmiPoint& thePixelToGridPointRatio, NFmiPoint& theDownSizeFactorOut, const boost::shared_ptr<NFmiDrawParam>& thePossibleDrawParam)
{
	auto& visSettings = GetVisualizationSettings();
	if(!visSettings.usePixelToGridPointRatioSafetyFeature())
		return false;

	if(thePossibleDrawParam)
	{
		if(!::IsIsolinesDrawn(thePossibleDrawParam))
			return false;
	}
	else if(!::IsIsolinesDrawn(theIsoLineData))
		return false;

	double usedPixelToGridPointRatio = visSettings.pixelToGridPointRatio();
	return IsDownSizingNeeded(thePixelToGridPointRatio, usedPixelToGridPointRatio, theDownSizeFactorOut);
}

static double CalcFinalDownSizeRatio(double criticalRatio, double currentRatio)
{
	if(currentRatio)
	{
		if(currentRatio < criticalRatio)
			return criticalRatio / currentRatio;
	}
	return 1.;
}

bool NFmiStationView::IsDownSizingNeeded(const NFmiPoint& thePixelToGridPointRatio, double usedPixelToGridPointRatio, NFmiPoint& theDownSizeFactorOut)
{
	const NFmiPoint zeroChangeFactor(1, 1);
	theDownSizeFactorOut.X(::CalcFinalDownSizeRatio(usedPixelToGridPointRatio, thePixelToGridPointRatio.X()));
	theDownSizeFactorOut.Y(::CalcFinalDownSizeRatio(usedPixelToGridPointRatio, thePixelToGridPointRatio.Y()));
	return theDownSizeFactorOut != zeroChangeFactor;
}

void NFmiStationView::UpdateOptimizedGridValues(const NFmiRect& dataAreaXyRect, int gridSizeX, int gridSizeY)
{
	NFmiPoint bottomLeftLatlon = itsArea->ToLatLon(dataAreaXyRect.BottomLeft());
	NFmiPoint topRightLatlon = itsArea->ToLatLon(dataAreaXyRect.TopRight());
	std::unique_ptr<NFmiArea> dataGridArea(itsArea->CreateNewArea(bottomLeftLatlon, topRightLatlon));
	itsOptimizedGridPtr.reset(new NFmiGrid(dataGridArea.get(), gridSizeX, gridSizeY));
}

bool NFmiStationView::IsMacroParamCase()
{
	return NFmiDrawParam::IsMacroParamCase(itsInfo->DataType());
}

// Haetaan colorValueInfoa vielä vain samasta datasta kuin its arvo data
static boost::shared_ptr<NFmiFastQueryInfo> GetBestMatchingData(CtrlViewDocumentInterface& ctrlViewDocumentInterface, boost::shared_ptr<NFmiDrawParam>& baseDrawParam, FmiParameterName parameterName, boost::shared_ptr<NFmiFastQueryInfo> &valueInfo)
{
	if(baseDrawParam->Param().GetProducer()->GetIdent() == kFmiSYNOP)
	{
		// Synop tapauksissa pitää hakea tiedoston nimen patternin mukaan, koska synop 
		// data käsittelee kolmea eri synop dataa (suomi/euro/maailma) ja lisäksi ship ja poiju datat.
		// Niiden haku normaalilla nearest-location jutulla ei toimi aina halutulla tavalla.
		auto infos = ctrlViewDocumentInterface.InfoOrganizer()->GetInfos(valueInfo->DataFilePattern());
		if(!infos.empty())
		{
			auto& info = infos.front();
			if(info->Param(parameterName))
			{
				return info;
			}
		}
	}
	else
	{
		boost::shared_ptr<NFmiDrawParam> wantedDrawParamPtr(new NFmiDrawParam(*baseDrawParam));
		wantedDrawParamPtr->Param().SetParam(NFmiParam(parameterName, "wantedParamName"));
		return ctrlViewDocumentInterface.InfoOrganizer()->Info(wantedDrawParamPtr, false, true);
	}
	return nullptr;
}

void NFmiStationView::SetupPossibleColorValueInfo()
{
	itsPossibleColorValueInfo = nullptr;
	if(!itsDrawParam->PossibleColorValueParameter().empty())
	{
		try
		{
			auto variableData = NFmiSmartToolIntepreter::CheckForVariableDataType(itsDrawParam->PossibleColorValueParameter());
			if(variableData.first && variableData.second.IsInUse())
			{
				const auto& defineWantedData = variableData.second;
				auto wantedParameterName = static_cast<FmiParameterName>(defineWantedData.param_.GetIdent());
				itsPossibleColorValueInfo = ::GetBestMatchingData(*itsCtrlViewDocumentInterface, itsDrawParam, wantedParameterName, itsInfo);
			}
		}
		catch(std::exception&)
		{
		}
	}
}
