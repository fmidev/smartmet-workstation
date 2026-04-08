// ======================================================================
/*!
 * \file NFmiAnalyzeToolData.cpp
 * \brief NFmiAnalyzeToolData implementation
 */
// ======================================================================

#include "NFmiAnalyzeToolData.h"
#include "NFmiInfoOrganizer.h"
#include <memory>
#include "NFmiSettings.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiLimitChecker.h"
#include "TimeSerialModificationDataInterface.h"
#include "NFmiAreaMaskList.h"
#include <boost/bind.hpp>

using namespace std::literals::string_literals;

long NFmiControlPointObservationBlendingData::itsExpirationTimeInMinutes = 30;
double NFmiControlPointObservationBlendingData::itsMaxAllowedDistanceToStationInKm = 20;
float NFmiControlPointObservationBlendingData::itsNonMissingObservationValueRatioLimit = 10;

void NFmiControlPointObservationBlendingData::SeekProducers(NFmiInfoOrganizer &theInfoOrganizer)
{
    NFmiInfoData::Type usedInfoData = NFmiInfoData::kObservations;
    // Ker�t��n lista producer-id arvoja nimineen, joista tehd��n popup valikko
    // Yhdelt� tuottajalta tulee vain yksi kohta (esim. synopilla voi olla jopa 3 eri tiedostoa)
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
    itsLastSessionProducer = itsSelectedProducer; // t�t� pit�� my�s p�ivitt��, koska se talletetaan sitten lopuksi konffeihin
    return true;
}

void NFmiControlPointObservationBlendingData::OverrideSelection(bool newValue)
{ 
    fOverrideSelection = newValue; 
    // Jos overide-moodi halutaan resetoida, pit�� my�s fIsSelectionMadeYet laittaa p��lle
    if(!newValue)
        fIsSelectionMadeYet = true;
}

bool NFmiControlPointObservationBlendingData::IsGoodObservationDataForCpPointConversion(std::shared_ptr<NFmiFastQueryInfo> &info)
{
    if(info)
    {
        // Pit�� olla asemadata
        if(!info->IsGrid())
        {
            // Ei saa olla level dataa
            if(info->SizeLevels() == 1)
            {
                // Ei saa olla liikkuva asemadata
                if(!info->HasLatlonInfoInData())
                {
                    // Asemia pit�� olla kokonaisuudessaan v�hint�in 10 kpl (ettei tule mukaan kaikenlaisia 'h�p�' datoja)
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

    // HUOM!! ei lueta eik� talleteta fAnalyzeToolMode-muuttujaa, koska oletusarvoisesti t�m� arvo on SmartMetin k�ynnistyess� aina false
    unsigned long prodId = NFmiSettings::Optional<unsigned long>(std::string(itsBaseNameSpace + "::ProdId"), kFmiSYNOP);
    std::string prodName = NFmiSettings::Optional<std::string>(std::string(itsBaseNameSpace + "::ProdName"), "Synop"s);
    itsSelectedProducer = itsLastSessionProducer = NFmiProducer(prodId, prodName);

    NFmiControlPointObservationBlendingData::itsExpirationTimeInMinutes = NFmiSettings::Optional<long>(std::string(itsBaseNameSpace + "::ExpirationTimeInMinutes"), NFmiControlPointObservationBlendingData::itsExpirationTimeInMinutes);
    NFmiControlPointObservationBlendingData::itsMaxAllowedDistanceToStationInKm = NFmiSettings::Optional<double>(std::string(itsBaseNameSpace + "::MaxAllowedDistanceToStationInKm"), NFmiControlPointObservationBlendingData::itsMaxAllowedDistanceToStationInKm);
    NFmiControlPointObservationBlendingData::itsNonMissingObservationValueRatioLimit = NFmiSettings::Optional<float>(std::string(itsBaseNameSpace + "::NonMissingObservationValueRatioLimit"), NFmiControlPointObservationBlendingData::itsNonMissingObservationValueRatioLimit);
}

void NFmiControlPointObservationBlendingData::StoreToSettings(void)
{
    if(itsBaseNameSpace.empty() == false)
    {
        // t�ss� k�ytet��n lastsession-versiota, koska jos tuottajaan ei ole koskettu k�sin, niiss� on oikeat tuottajat ja jos s��detty k�sin niin my�s
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

float NFmiControlPointObservationBlendingData::NonMissingObservationValueRatioLimit() 
{ 
    return itsNonMissingObservationValueRatioLimit; 
}

void NFmiControlPointObservationBlendingData::NonMissingObservationValueRatioLimit(float newValue) 
{ 
    itsNonMissingObservationValueRatioLimit = newValue; 
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

	// HUOM!! ei lueta eik� talleteta fAnalyzeToolMode-muuttujaa, koska oletusarvoisesti t�m� arvo on SmartMetin k�ynnistyess� aina false
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
		// t�ss� k�ytet��n lastsession-versioita, koska jos tuottajiin ei ole koskettu k�sin, niiss� on oikeat tuottajat ja jos s��detty k�sin niin my�s
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
	const std::vector<NFmiHelpDataInfo> &dynamicHelpDataInfos = theHelpDataInfoSystem.DynamicHelpDataInfos();
	itsPotentialProducersFileFilters.clear();
	for(size_t i = 0; i < dynamicHelpDataInfos.size(); i++)
	{
		if(dynamicHelpDataInfos[i].DataType() == NFmiInfoData::kAnalyzeData)
		{
			if(dynamicHelpDataInfos[i].IsCombineData() == false)
				itsPotentialProducersFileFilters.push_back(dynamicHelpDataInfos[i].UsedFileNameFilter(theHelpDataInfoSystem));
			else
				itsPotentialProducersFileFilters.push_back(dynamicHelpDataInfos[i].CombinedResultDataFileFilter());
		}
	}
}

void NFmiAnalyzeToolData::SeekProducers(NFmiInfoOrganizer &theInfoOrganizer)
{
	itsProducers.clear();
	for(size_t i = 0; i < itsPotentialProducersFileFilters.size(); i++)
	{
		std::vector<std::shared_ptr<NFmiFastQueryInfo> > infos = theInfoOrganizer.GetInfos(itsPotentialProducersFileFilters[i]);
		if(infos.size() && infos[0]->Producer())
		{
			if(infos[0]->SizeLevels() == 1) // ainakin aluksi t�h�n hyv�ksyt��n vain ns. pintadatat
				itsProducers.push_back(*(infos[0]->Producer()));
		}
	}
}

void NFmiAnalyzeToolData::SelectedProducer1(const NFmiProducer &theProducer, bool handSelected)
{
	if(fIsSelectionsMadeYet && handSelected == false)
		return ; //t�ll�ist� tilannetta ei kai pit�isi tulla, mutta varaudutaan siihen kuitenkin

	itsSelectedProducer1 = theProducer;
	if(handSelected)
	{
		fIsSelectionsMadeYet = true;
		itsLastSessionProducer1 = itsSelectedProducer1; // t�t� pit�� my�s p�ivitt��, koska se talletetaan sitten lopuksi konffeihin
	}
}

void NFmiAnalyzeToolData::SelectedProducer2(const NFmiProducer &theProducer, bool handSelected)
{
	if(fIsSelectionsMadeYet && handSelected == false)
		return ; //t�ll�ist� tilannetta ei kai pit�isi tulla, mutta varaudutaan siihen kuitenkin

	itsSelectedProducer2 = theProducer;
	if(handSelected)
	{
		fIsSelectionsMadeYet = true;
		itsLastSessionProducer2 = itsSelectedProducer2; // t�t� pit�� my�s p�ivitt��, koska se talletetaan sitten lopuksi konffeihin
	}
}

bool NFmiAnalyzeToolData::SelectProducer1ByName(const std::string &theProducerName)
{
	// HUOM! t�ss� olen k�ytt�nyt boost:in bind ja function templaatteja
	// niin ett� voin antaa olion member-function:in parametrina luokan metodille.
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
		return true; // jos analyysi ty�kalu ei ole k�yt�ss�, enabloi aina
	else
	{
		std::shared_ptr<NFmiFastQueryInfo> analyzeDataInfo1 = theInfoOrganizer.Info(NFmiDataIdent(theParam, SelectedProducer1()), 0, NFmiInfoData::kAnalyzeData);
		std::shared_ptr<NFmiFastQueryInfo> editedInfo = theInfoOrganizer.FindInfo(NFmiInfoData::kEditable);
		if(editedInfo && analyzeDataInfo1)
		{
			if(analyzeDataInfo1->TimeDescriptor().LastTime() >= editedInfo->TimeDescriptor().FirstTime())
			{
				if(fUseBothProducers)
				{
					std::shared_ptr<NFmiFastQueryInfo> analyzeDataInfo2 = theInfoOrganizer.Info(NFmiDataIdent(theParam, SelectedProducer2()), 0, NFmiInfoData::kAnalyzeData);
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

// Tarkista ett� valitulle ajalle ja parametrille l�ytyy riitt�v� m��r� ei-puuttuvia havaintoja.
// Jos havaintoja ei ole riitt�v�sti, palauta false, muuten true.
// Oletuksia: 1. info ei ole nullptr, 2. checkedObservationArea ei ole nullptr, 3. info:ssa on asemadataa
static bool CheckForExistingObservationsOnUsedArea(std::shared_ptr<NFmiFastQueryInfo> &info, const std::shared_ptr<NFmiArea> &checkedObservationArea)
{
    auto stationsOnAreaCount = 0.f;
    auto nonMissingObservationsOnAreaCount = 0.f;
    for(info->ResetLocation(); info->NextLocation(); )
    {
        if(checkedObservationArea->IsInside(info->LatLon()))
        {
            stationsOnAreaCount++;
            if(info->FloatValue() != kFloatMissing)
                nonMissingObservationsOnAreaCount++;
        }
    }
    if(stationsOnAreaCount == 0)
        return false; // Jos alueella ei ollut yht��n asemaa => false
    auto nonMissingValueRatioOnArea = 100.f * nonMissingObservationsOnAreaCount / stationsOnAreaCount;
    return nonMissingValueRatioOnArea >= NFmiControlPointObservationBlendingData::NonMissingObservationValueRatioLimit();
}

// Haetaan datasta viimeisin aika, jolla on riitt�v�sti havaintoja.
// Jos kyse on hiladatasta, palautetaan vain viimeinen aika.
static NFmiMetTime FindLatestAcceptableTime(std::shared_ptr<NFmiFastQueryInfo> &info, const std::shared_ptr<NFmiArea> &checkedObservationArea)
{
    if(info)
    {
        if(checkedObservationArea && !info->IsGrid())
        {
            // Aletaan k�yd� datan aikoja l�pi lopusta taaksep�in
            info->LastTime();
            // Ei k�yd� l�pi kuin maksimissan n kpl viimeist� aikaa
            auto timeCounter = 0;
            const auto timeCounterLimit = 5;
            do
            {
                if(::CheckForExistingObservationsOnUsedArea(info, checkedObservationArea))
                    return info->Time();
                timeCounter++;
            } while(info->PreviousTime() && timeCounter <= timeCounterLimit);

            // Sopivaa aikaa ei l�ytynyt datan lopusta, palautetaan puuttuva aika
            return NFmiMetTime::gMissingTime;
        }
        // Kyseess� oli hiladataa tai ei haluta tehd� area tarkasteluja, palauta viimeinen aika
        return info->TimeDescriptor().LastTime();
    }
    // Ei infoa, t�h�n ei pit�isi tulla
    return NFmiMetTime::gMissingTime;
}

static NFmiMetTime GetLatestInfoTime(std::vector<std::shared_ptr<NFmiFastQueryInfo>> &infos, const std::shared_ptr<NFmiArea> &checkedObservationArea)
{
    NFmiMetTime latestTime = NFmiMetTime::gMissingTime;
    for(auto &info : infos)
    {
        // Ajan etsinn�iss� k�yd��n l�pi kaikki asemat, mutta haluamme palauttaa t�m�n asetuksen lopuksi
        auto oldLocationIndex = info->LocationIndex();
        const auto latestInfoTime = ::FindLatestAcceptableTime(info, checkedObservationArea);
        if(latestTime == NFmiMetTime::gMissingTime || latestInfoTime > latestTime)
        {
            latestTime = latestInfoTime;
        }
        info->LocationIndex(oldLocationIndex);
    }
    return latestTime;
}

static std::string MakeErrorTextForNotFindingSuitableAnalyzeToolTime(const std::string &toolName, const std::string &dataTypeName, const NFmiMetTime &notFoundTime, const std::string &usedProducerName, long maxDifferenceInMinutes)
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

static NFmiMetTime GetSuitableAnalyzeToolInfoTime(const NFmiMetTime &latestInfoTime, std::shared_ptr<NFmiFastQueryInfo> &editedInfo, bool useObservationBlenderTool, const std::string &usedProducerName)
{
    // 1. Jos latestInfoTime l�ytyy editedInfo:sta, palautetaan se
    if(editedInfo->Time(latestInfoTime))
        return latestInfoTime;
    else
    {
        // Jos aikaa ei ollut suoraan editoidussa datassa, katsotaan kelpaako l�hin aika.
        // Normaali analyysi laskennassa k�ytet��n 1h haarukkaa, mutta obs-blenderin kanssa k�ytet��n tarkempaa
        // rqjaa, mik� l�ytyy 
        long maxDifferenceInMinutes = useObservationBlenderTool ? NFmiControlPointObservationBlendingData::ExpirationTimeInMinutes() : 60;
        if(editedInfo->FindNearestTime(latestInfoTime, kCenter, maxDifferenceInMinutes))
            return editedInfo->Time();
        else
        {
            // Ei l�ytynyt aikarajojen sis�lt� sopivaa aikaa editoidusta datasta, tee joku selitt�v� teksti lokiin molemmille ty�kaluille erikseen
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

// Palauttaa sek� todellisen viimeisen ajan (esim. 10.20), ett� py�ristetyn editoituun datan sopivan ajan (esim. 10.00)
std::pair<NFmiMetTime, NFmiMetTime> NFmiAnalyzeToolData::GetLatestSuitableAnalyzeToolInfoTime(std::vector<std::shared_ptr<NFmiFastQueryInfo>> &infos, std::shared_ptr<NFmiFastQueryInfo> &editedInfo, const std::shared_ptr<NFmiArea> &checkedObservationArea, bool useObservationBlenderTool, const std::string &usedProducerName)
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

std::shared_ptr<NFmiArea> NFmiAnalyzeToolData::GetUsedAreaForAnalyzeTool(TimeSerialModificationDataInterface &theAdapter, std::shared_ptr<NFmiFastQueryInfo> &editedInfo)
{
    if(editedInfo && editedInfo->IsGrid())
    {
        const auto &editAreaGrid = *editedInfo->Grid();
        // 1. Jos "Zoomed CP" optio ei ole p��ll� tai zoomaus on niin v�h�inen ett� ei tuota erillist� zoomattua aluetta,
        // palautetaan editoidun datan area.
        const auto &zoomedAreaGridPointRect = theAdapter.CPGridCropRect();
        const auto emptyRect = NFmiRect();
        auto useZoomedArea = theAdapter.UseCPGridCrop() && (zoomedAreaGridPointRect != emptyRect);
        if(!useZoomedArea)
            return std::shared_ptr<NFmiArea>(editAreaGrid.Area()->Clone());
        else
        {
            // Luodaan uusi area zoomialueen hilapisteiden avulla
            auto bottomLeftLatlon = editAreaGrid.GridToLatLon(zoomedAreaGridPointRect.BottomLeft());
            auto topRightLatlon = editAreaGrid.GridToLatLon(zoomedAreaGridPointRect.TopRight());
            return std::shared_ptr<NFmiArea>(editAreaGrid.Area()->CreateNewArea(bottomLeftLatlon, topRightLatlon));
        }
    }
    return std::shared_ptr<NFmiArea>();
}

std::shared_ptr<NFmiAreaMaskList> NFmiAnalyzeToolData::GetUsedTimeSerialMaskList(TimeSerialModificationDataInterface &theAdapter)
{
    std::shared_ptr<NFmiAreaMaskList> maskList = theAdapter.ParamMaskList();
    std::shared_ptr<NFmiAreaMaskList> emptyMaskList(new NFmiAreaMaskList());
    if(!theAdapter.UseMasksInTimeSerialViews())
        maskList = emptyMaskList;
    maskList->CheckIfMaskUsed();
    return maskList;
}
