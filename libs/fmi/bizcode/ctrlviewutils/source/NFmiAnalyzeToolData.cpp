// ======================================================================
/*!
 * \file NFmiAnalyzeToolData.cpp
 * \brief NFmiAnalyzeToolData implementation
 */
// ======================================================================

#include "NFmiAnalyzeToolData.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiSettings.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiLimitChecker.h"
#include "TimeSerialModificationDataInterface.h"
#include <boost/bind.hpp>

using namespace std::literals::string_literals;

long NFmiControlPointObservationBlendingData::itsExpirationTimeInMinutes = 10;
double NFmiControlPointObservationBlendingData::itsMaxAllowedDistanceToStationInKm = 10;

void NFmiControlPointObservationBlendingData::SeekProducers(NFmiInfoOrganizer &theInfoOrganizer)
{
    NFmiInfoData::Type usedInfoData = NFmiInfoData::kObservations;
    // Kerätään lista producer-id arvoja nimineen, joista tehdään popup valikko
    // Yhdeltä tuottajalta tulee vain yksi kohta (esim. synopilla voi olla jopa 3 eri tiedostoa)
    std::map<long, NFmiProducer> observationProducerList;
    auto observationInfos = theInfoOrganizer.GetInfos(usedInfoData);
    for(auto &info : observationInfos)
    {
        if(IsGoodObservationDataForCpPointConversion(info))
        {
            const auto &producer = *info->Producer();
            observationProducerList.insert(std::make_pair(producer.GetIdent(), producer));
        }
    }

    itsProducers.clear();
    for(const auto &producerMapItem : observationProducerList)
        itsProducers.push_back(producerMapItem.second);
}

bool NFmiControlPointObservationBlendingData::SelectProducer(const std::string &theProducerName)
{
    for(const auto &producer : itsProducers)
    {
        if(producer.GetName() == theProducerName)
        {
            return UpdateProducerInfo(producer);
        }
    }
    return false;
}

bool NFmiControlPointObservationBlendingData::SelectProducer(unsigned long theProducerId)
{
    for(const auto &producer : itsProducers)
    {
        if(producer.GetIdent() == theProducerId)
        {
            OverrideSelection(true);
            return UpdateProducerInfo(producer);
        }
    }
    return false;
}

bool NFmiControlPointObservationBlendingData::UpdateProducerInfo(const NFmiProducer &producer)
{
    itsSelectedProducer = producer;
    fIsSelectionMadeYet = true;
    itsLastSessionProducer = itsSelectedProducer; // tätä pitää myös päivittää, koska se talletetaan sitten lopuksi konffeihin
    return true;
}

void NFmiControlPointObservationBlendingData::OverrideSelection(bool newValue)
{ 
    fOverrideSelection = newValue; 
    // Jos overide-moodi halutaan resetoida, pitää myös fIsSelectionMadeYet laittaa päälle
    if(!newValue)
        fIsSelectionMadeYet = true;
}

bool NFmiControlPointObservationBlendingData::IsGoodObservationDataForCpPointConversion(boost::shared_ptr<NFmiFastQueryInfo> &info)
{
    if(info)
    {
        // Pitää olla asemadata
        if(!info->IsGrid())
        {
            // Ei saa olla level dataa
            if(info->SizeLevels() == 1)
            {
                // Ei saa olla liikkuva asemadata
                if(!info->HasLatlonInfoInData())
                {
                    // Asemia pitää olla kokonaisuudessaan vähintäin 10 kpl (ettei tule mukaan kaikenlaisia 'höpö' datoja)
                    if(info->SizeLocations() >= 10)
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void NFmiControlPointObservationBlendingData::InitFromSettings(const std::string &theBaseNameSpace)
{
    itsBaseNameSpace = theBaseNameSpace;

    // HUOM!! ei lueta eikä talleteta fAnalyzeToolMode-muuttujaa, koska oletusarvoisesti tämä arvo on SmartMetin käynnistyessä aina false
    unsigned long prodId = NFmiSettings::Require<unsigned long>(std::string(itsBaseNameSpace + "::ProdId"));
    std::string prodName = NFmiSettings::Require<std::string>(std::string(itsBaseNameSpace + "::ProdName"));
    itsSelectedProducer = itsLastSessionProducer = NFmiProducer(prodId, prodName);
}

void NFmiControlPointObservationBlendingData::StoreToSettings(void)
{
    if(itsBaseNameSpace.empty() == false)
    {
        // tässä käytetään lastsession-versiota, koska jos tuottajaan ei ole koskettu käsin, niissä on oikeat tuottajat ja jos säädetty käsin niin myös
        NFmiSettings::Set(std::string(itsBaseNameSpace + "::ProdId"), NFmiStringTools::Convert(itsLastSessionProducer.GetIdent()), true);
        NFmiSettings::Set(std::string(itsBaseNameSpace + "::ProdName"), itsLastSessionProducer.GetName().CharPtr(), true);

    }
    else
        throw std::runtime_error("Error in NFmiAnalyzeToolData::StoreToSettings, unable to store setting.");
}

long NFmiControlPointObservationBlendingData::ExpirationTimeInMinutes()
{
    return itsExpirationTimeInMinutes;
}

void NFmiControlPointObservationBlendingData::ExpirationTimeInMinutes(long newValue)
{
    itsExpirationTimeInMinutes = newValue;
}

double NFmiControlPointObservationBlendingData::MaxAllowedDistanceToStationInKm()
{
    return itsMaxAllowedDistanceToStationInKm;
}

void NFmiControlPointObservationBlendingData::MaxAllowedDistanceToStationInKm(double newValue)
{
    itsMaxAllowedDistanceToStationInKm = newValue;
}

float NFmiControlPointObservationBlendingData::BlendData(float editedDataValue, float changeValue, float maskFactor, unsigned long timeSize, unsigned long timeIndex, const NFmiLimitChecker &limitChecker)
{
    if(editedDataValue == kFloatMissing || changeValue == kFloatMissing)
        return editedDataValue; // ongelma tilanteissa palautetaan arvo editoidusta datasta takaisin
    float value = editedDataValue - ((timeSize - timeIndex - 1.f) / (timeSize - 1.f) * (changeValue * maskFactor));
    return limitChecker.CheckValue(value);
}

// ===============================================================================================

NFmiAnalyzeToolData::NFmiAnalyzeToolData(void)
:itsSelectedProducer1()
,itsLastSessionProducer1()
,itsSelectedProducer2()
,itsLastSessionProducer2()
,itsAnalyzeToolEndTime()
,fAnalyzeToolMode(false)
,fUseBothProducers(false)
,fIsSelectionsMadeYet(false)
,itsPotentialProducersFileFilters()
,itsProducers()
,itsBaseNameSpace()
{
}


NFmiAnalyzeToolData::~NFmiAnalyzeToolData(void)
{
}

void NFmiAnalyzeToolData::InitFromSettings(const std::string &theBaseNameSpace)
{
	itsBaseNameSpace = theBaseNameSpace;

	// HUOM!! ei lueta eikä talleteta fAnalyzeToolMode-muuttujaa, koska oletusarvoisesti tämä arvo on SmartMetin käynnistyessä aina false
	unsigned long prodId1 = NFmiSettings::Require<unsigned long>(std::string(itsBaseNameSpace + "::ProdId1"));
	std::string prodName1 = NFmiSettings::Require<std::string>(std::string(itsBaseNameSpace + "::ProdName1"));
	itsSelectedProducer1 = itsLastSessionProducer1 = NFmiProducer(prodId1, prodName1);
	unsigned long prodId2 = NFmiSettings::Require<unsigned long>(std::string(itsBaseNameSpace + "::ProdId2"));
	std::string prodName2 = NFmiSettings::Require<std::string>(std::string(itsBaseNameSpace + "::ProdName2"));
	itsSelectedProducer2 = itsLastSessionProducer2 = NFmiProducer(prodId2, prodName2);
	fUseBothProducers = NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::UseBothProducers"));

    itsControlPointObservationBlendingData.InitFromSettings(theBaseNameSpace + "::CpObsBlendData");
}

void NFmiAnalyzeToolData::StoreToSettings(void)
{
	if(itsBaseNameSpace.empty() == false)
	{
		// tässä käytetään lastsession-versioita, koska jos tuottajiin ei ole koskettu käsin, niissä on oikeat tuottajat ja jos säädetty käsin niin myös
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::ProdId1"), NFmiStringTools::Convert(itsLastSessionProducer1.GetIdent()), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::ProdName1"), itsLastSessionProducer1.GetName().CharPtr(), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::ProdId2"), NFmiStringTools::Convert(itsLastSessionProducer2.GetIdent()), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::ProdName2"), itsLastSessionProducer2.GetName().CharPtr(), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::UseBothProducers"), NFmiStringTools::Convert(fUseBothProducers), true);

        itsControlPointObservationBlendingData.StoreToSettings();
	}
	else
		throw std::runtime_error("Error in NFmiAnalyzeToolData::StoreToSettings, unable to store setting.");
}

void NFmiAnalyzeToolData::SeekPotentialProducersFileFilters(const NFmiHelpDataInfoSystem &theHelpDataInfoSystem)
{
	const checkedVector<NFmiHelpDataInfo> &dynamicHelpDataInfos = theHelpDataInfoSystem.DynamicHelpDataInfos();
	itsPotentialProducersFileFilters.clear();
	for(size_t i = 0; i < dynamicHelpDataInfos.size(); i++)
	{
		if(dynamicHelpDataInfos[i].DataType() == NFmiInfoData::kAnalyzeData)
		{
			if(dynamicHelpDataInfos[i].IsCombineData() == false)
				itsPotentialProducersFileFilters.push_back(dynamicHelpDataInfos[i].UsedFileNameFilter(theHelpDataInfoSystem));
			else
				itsPotentialProducersFileFilters.push_back(dynamicHelpDataInfos[i].CombineDataPathAndFileName());
		}
	}
}

void NFmiAnalyzeToolData::SeekProducers(NFmiInfoOrganizer &theInfoOrganizer)
{
	itsProducers.clear();
	for(size_t i = 0; i < itsPotentialProducersFileFilters.size(); i++)
	{
		checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > infos = theInfoOrganizer.GetInfos(itsPotentialProducersFileFilters[i]);
		if(infos.size() && infos[0]->Producer())
		{
			if(infos[0]->SizeLevels() == 1) // ainakin aluksi tähän hyväksytään vain ns. pintadatat
				itsProducers.push_back(*(infos[0]->Producer()));
		}
	}
}

void NFmiAnalyzeToolData::SelectedProducer1(const NFmiProducer &theProducer, bool handSelected)
{
	if(fIsSelectionsMadeYet && handSelected == false)
		return ; //tälläistä tilannetta ei kai pitäisi tulla, mutta varaudutaan siihen kuitenkin

	itsSelectedProducer1 = theProducer;
	if(handSelected)
	{
		fIsSelectionsMadeYet = true;
		itsLastSessionProducer1 = itsSelectedProducer1; // tätä pitää myös päivittää, koska se talletetaan sitten lopuksi konffeihin
	}
}

void NFmiAnalyzeToolData::SelectedProducer2(const NFmiProducer &theProducer, bool handSelected)
{
	if(fIsSelectionsMadeYet && handSelected == false)
		return ; //tälläistä tilannetta ei kai pitäisi tulla, mutta varaudutaan siihen kuitenkin

	itsSelectedProducer2 = theProducer;
	if(handSelected)
	{
		fIsSelectionsMadeYet = true;
		itsLastSessionProducer2 = itsSelectedProducer2; // tätä pitää myös päivittää, koska se talletetaan sitten lopuksi konffeihin
	}
}

bool NFmiAnalyzeToolData::SelectProducer1ByName(const std::string &theProducerName)
{
	// HUOM! tässä olen käyttänyt boost:in bind ja function templaatteja
	// niin että voin antaa olion member-function:in parametrina luokan metodille.
	return SelectProducerByName(theProducerName, boost::bind(&NFmiAnalyzeToolData::SelectedProducer1, this, _1, _2));
}

bool NFmiAnalyzeToolData::SelectProducer2ByName(const std::string &theProducerName)
{
	return SelectProducerByName(theProducerName, boost::bind(&NFmiAnalyzeToolData::SelectedProducer2, this, _1, _2));
}

bool NFmiAnalyzeToolData::SelectProducerByName(const std::string &theProducerName, ProdSetter theSetter)
{
	for(size_t i = 0; i < itsProducers.size(); i++)
	{
		if(theProducerName == std::string(itsProducers[i].GetName().CharPtr()))
		{
			theSetter(itsProducers[i], true);
			return true;
		}
	}
	return false;
}

bool NFmiAnalyzeToolData::EnableAnalyseTool(NFmiInfoOrganizer &theInfoOrganizer, const NFmiParam &theParam)
{
	if(fAnalyzeToolMode == false)
		return true; // jos analyysi työkalu ei ole käytössä, enabloi aina
	else
	{
		boost::shared_ptr<NFmiFastQueryInfo> analyzeDataInfo1 = theInfoOrganizer.Info(NFmiDataIdent(theParam, SelectedProducer1()), 0, NFmiInfoData::kAnalyzeData);
		boost::shared_ptr<NFmiFastQueryInfo> editedInfo = theInfoOrganizer.FindInfo(NFmiInfoData::kEditable);
		if(editedInfo && analyzeDataInfo1)
		{
			if(analyzeDataInfo1->TimeDescriptor().LastTime() >= editedInfo->TimeDescriptor().FirstTime())
			{
				if(fUseBothProducers)
				{
					boost::shared_ptr<NFmiFastQueryInfo> analyzeDataInfo2 = theInfoOrganizer.Info(NFmiDataIdent(theParam, SelectedProducer2()), 0, NFmiInfoData::kAnalyzeData);
					if(analyzeDataInfo2 && (analyzeDataInfo2->TimeDescriptor().LastTime() >= editedInfo->TimeDescriptor().FirstTime()))
						return true;
				}
				else
					return true;
			}
		}
	}
	return false;
}

// Tarkista että valitulle ajalle ja parametrille löytyy riittävä määrä ei-puuttuvia havaintoja.
// Jos havaintoja ei ole riittävästi, palauta false, muuten true.
// Oletuksia: 1. info ei ole nullptr, 2. checkedObservationArea ei ole nullptr, 3. info:ssa on asemadataa
static bool CheckForExistingObservationsOnUsedArea(boost::shared_ptr<NFmiFastQueryInfo> &info, const boost::shared_ptr<NFmiArea> &checkedObservationArea)
{
    auto stationsOnAreaCount = 0.f;
    auto nonMissingObservationsOnAreaCount = 0.f;
    for(info->ResetLocation(); info->NextLocation(); )
    {
        if(checkedObservationArea->IsInside(info->LatLonFast()))
        {
            stationsOnAreaCount++;
            if(info->FloatValue() != kFloatMissing)
                nonMissingObservationsOnAreaCount++;
        }
    }
    if(stationsOnAreaCount == 0)
        return false; // Jos alueella ei ollut yhtään asemaa => false
    auto nonMissingValueRatioOnArea = 100.f * nonMissingObservationsOnAreaCount / stationsOnAreaCount;
    auto nonMissingValueRatioLimit = 30.f; // Laitetaan joku raja puuttuvien maksimimääräksi
    return nonMissingValueRatioOnArea >= nonMissingValueRatioLimit;
}

// Haetaan datasta viimeisin aika, jolla on riittävästi havaintoja.
// Jos kyse on hiladatasta, palautetaan vain viimeinen aika.
static NFmiMetTime FindLatestAcceptableTime(boost::shared_ptr<NFmiFastQueryInfo> &info, const boost::shared_ptr<NFmiArea> &checkedObservationArea)
{
    if(info)
    {
        if(checkedObservationArea && !info->IsGrid())
        {
            // Aletaan käydä datan aikoja läpi lopusta taaksepäin
            info->LastTime();
            // Ei käydä läpi kuin maksimissan n kpl viimeistä aikaa
            auto timeCounter = 0;
            const auto timeCounterLimit = 5;
            do
            {
                if(::CheckForExistingObservationsOnUsedArea(info, checkedObservationArea))
                    return info->Time();
                timeCounter++;
            } while(info->PreviousTime() && timeCounter <= timeCounterLimit);

            // Sopivaa aikaa ei löytynyt datan lopusta, palautetaan puuttuva aika
            return NFmiMetTime::gMissingTime;
        }
        // Kyseessä oli hiladataa tai ei haluta tehdä area tarkasteluja, palauta viimeinen aika
        return info->TimeDescriptor().LastTime();
    }
    // Ei infoa, tähän ei pitäisi tulla
    return NFmiMetTime::gMissingTime;
}

static NFmiMetTime GetLatestInfoTime(checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> &infos, const boost::shared_ptr<NFmiArea> &checkedObservationArea)
{
    NFmiMetTime latestTime = NFmiMetTime::gMissingTime;
    for(auto &info : infos)
    {
        const auto latestInfoTime = ::FindLatestAcceptableTime(info, checkedObservationArea);
        if(latestTime == NFmiMetTime::gMissingTime || latestInfoTime > latestTime)
        {
            latestTime = latestInfoTime;
        }
    }
    return latestTime;
}

static std::string MakeErrorTextForNotFindingSuitableAnalyzeToolTime(const std::string &toolName, const std::string &dataTypeName, const NFmiMetTime &notFoundTime, const std::string usedProducerName, long maxDifferenceInMinutes)
{
    auto errorString = toolName;
    errorString += " error, latest good '"s;
    errorString += usedProducerName;
    errorString += "'"s;
    errorString += dataTypeName;
    if(notFoundTime == NFmiMetTime::gMissingTime)
        errorString += "' (missing time) "s;
    else
        errorString += notFoundTime.ToStr("' (MM.DD HH:mm) ");
    errorString += "was not in limits ("s;
    errorString += std::to_string(maxDifferenceInMinutes);
    errorString += " minutes) to any edited time step"s;
    return errorString;
}

static NFmiMetTime GetSuitableAnalyzeToolInfoTime(const NFmiMetTime &latestInfoTime, boost::shared_ptr<NFmiFastQueryInfo> &editedInfo, bool useObservationBlenderTool, const std::string usedProducerName)
{
    // 1. Jos latestInfoTime löytyy editedInfo:sta, palautetaan se
    if(editedInfo->Time(latestInfoTime))
        return latestInfoTime;
    else
    {
        // Jos aikaa ei ollut suoraan editoidussa datassa, katsotaan kelpaako lähin aika.
        // Normaali analyysi laskennassa käytetään 1h haarukkaa, mutta obs-blenderin kanssa käytetään tarkempaa
        // rqjaa, mikä löytyy 
        long maxDifferenceInMinutes = useObservationBlenderTool ? NFmiControlPointObservationBlendingData::ExpirationTimeInMinutes() : 60;
        if(editedInfo->FindNearestTime(latestInfoTime, kCenter, maxDifferenceInMinutes))
            return editedInfo->Time();
        else
        {
            // Ei löytynyt aikarajojen sisältä sopivaa aikaa editoidusta datasta, tee joku selittävä teksti lokiin molemmille työkaluille erikseen
            if(useObservationBlenderTool)
            {
                auto errorString = ::MakeErrorTextForNotFindingSuitableAnalyzeToolTime("Observation-blender"s, "observation"s, latestInfoTime, usedProducerName, maxDifferenceInMinutes);
                throw std::runtime_error(errorString);
            }
            else
            {
                auto errorString = ::MakeErrorTextForNotFindingSuitableAnalyzeToolTime("Analyze tool"s, "analyze"s, latestInfoTime, usedProducerName, maxDifferenceInMinutes);
                throw std::runtime_error(errorString);
            }
        }
    }
}

// Palauttaa sekä todellisen viimeisen ajan (esim. 10.20), että pyöristetyn editoituun datan sopivan ajan (esim. 10.00)
std::pair<NFmiMetTime, NFmiMetTime> NFmiAnalyzeToolData::GetLatestSuitableAnalyzeToolInfoTime(checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> &infos, boost::shared_ptr<NFmiFastQueryInfo> &editedInfo, const boost::shared_ptr<NFmiArea> &checkedObservationArea, bool useObservationBlenderTool, const std::string usedProducerName)
{
    if(infos.empty())
        throw std::runtime_error(std::string("Error in ") + __FUNCTION__ + ": given infos vector was empty, can't search latest analyze tool time");
    else
    {
        auto latestActualTime = ::GetLatestInfoTime(infos, checkedObservationArea);
        auto latestTime = ::GetSuitableAnalyzeToolInfoTime(latestActualTime, editedInfo, useObservationBlenderTool, usedProducerName);
        return std::make_pair(latestActualTime, latestTime);
    }
}

boost::shared_ptr<NFmiArea> NFmiAnalyzeToolData::GetUsedAreaForAnalyzeTool(TimeSerialModificationDataInterface &theAdapter, boost::shared_ptr<NFmiFastQueryInfo> &editedInfo)
{
    if(editedInfo && editedInfo->IsGrid())
    {
        const auto &editAreaGrid = *editedInfo->Grid();
        // 1. Jos "Zoomed CP" optio ei ole päällä tai zoomaus on niin vähäinen että ei tuota erillistä zoomattua aluetta, 
        // palautetaan editoidun datan area.
        const auto &zoomedAreaGridPointRect = theAdapter.CPGridCropRect();
        const auto emptyRect = NFmiRect();
        auto useZoomedArea = theAdapter.UseCPGridCrop() && (zoomedAreaGridPointRect != emptyRect);
        if(!useZoomedArea)
            return boost::shared_ptr<NFmiArea>(editAreaGrid.Area()->Clone());
        else
        {
            // Luodaan uusi area zoomialueen hilapisteiden avulla
            auto bottomLeftLatlon = editAreaGrid.GridToLatLon(zoomedAreaGridPointRect.BottomLeft());
            auto topRightLatlon = editAreaGrid.GridToLatLon(zoomedAreaGridPointRect.TopRight());
            return boost::shared_ptr<NFmiArea>(editAreaGrid.Area()->CreateNewArea(bottomLeftLatlon, topRightLatlon));
        }
    }
    return boost::shared_ptr<NFmiArea>();
}
