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
#include <boost/bind.hpp>

long NFmiControlPointObservationBlendingData::itsExpirationTimeInMinutes = 10;
double NFmiControlPointObservationBlendingData::itsMaxAllowedDistanceToStationInKm = 10;

void NFmiControlPointObservationBlendingData::SeekProducers(NFmiInfoOrganizer &theInfoOrganizer)
{
    NFmiInfoData::Type usedInfoData = NFmiInfoData::kObservations;
    // Ker‰t‰‰n lista producer-id arvoja nimineen, joista tehd‰‰n popup valikko
    // Yhdelt‰ tuottajalta tulee vain yksi kohta (esim. synopilla voi olla jopa 3 eri tiedostoa)
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
    itsLastSessionProducer = itsSelectedProducer; // t‰t‰ pit‰‰ myˆs p‰ivitt‰‰, koska se talletetaan sitten lopuksi konffeihin
    return true;
}

void NFmiControlPointObservationBlendingData::OverrideSelection(bool newValue)
{ 
    fOverrideSelection = newValue; 
    // Jos overide-moodi halutaan resetoida, pit‰‰ myˆs fIsSelectionMadeYet laittaa p‰‰lle
    if(!newValue)
        fIsSelectionMadeYet = true;
}

bool NFmiControlPointObservationBlendingData::IsGoodObservationDataForCpPointConversion(boost::shared_ptr<NFmiFastQueryInfo> &info)
{
    if(info)
    {
        // Pit‰‰ olla asemadata
        if(!info->IsGrid())
        {
            // Ei saa olla level dataa
            if(info->SizeLevels() == 1)
            {
                // Ei saa olla liikkuva asemadata
                if(!info->HasLatlonInfoInData())
                {
                    // Asemia pit‰‰ olla kokonaisuudessaan v‰hint‰in 10 kpl (ettei tule mukaan kaikenlaisia 'hˆpˆ' datoja)
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

    // HUOM!! ei lueta eik‰ talleteta fAnalyzeToolMode-muuttujaa, koska oletusarvoisesti t‰m‰ arvo on SmartMetin k‰ynnistyess‰ aina false
    unsigned long prodId = NFmiSettings::Require<unsigned long>(std::string(itsBaseNameSpace + "::ProdId"));
    std::string prodName = NFmiSettings::Require<std::string>(std::string(itsBaseNameSpace + "::ProdName"));
    itsSelectedProducer = itsLastSessionProducer = NFmiProducer(prodId, prodName);
}

void NFmiControlPointObservationBlendingData::StoreToSettings(void)
{
    if(itsBaseNameSpace.empty() == false)
    {
        // t‰ss‰ k‰ytet‰‰n lastsession-versiota, koska jos tuottajaan ei ole koskettu k‰sin, niiss‰ on oikeat tuottajat ja jos s‰‰detty k‰sin niin myˆs
        NFmiSettings::Set(std::string(itsBaseNameSpace + "::ProdId"), NFmiStringTools::Convert(itsLastSessionProducer.GetIdent()), true);
        NFmiSettings::Set(std::string(itsBaseNameSpace + "::ProdName"), itsLastSessionProducer.GetName().CharPtr(), true);

    }
    else
        throw std::runtime_error("Error in NFmiAnalyzeToolData::StoreToSettings, unable to store setting.");
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

	// HUOM!! ei lueta eik‰ talleteta fAnalyzeToolMode-muuttujaa, koska oletusarvoisesti t‰m‰ arvo on SmartMetin k‰ynnistyess‰ aina false
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
		// t‰ss‰ k‰ytet‰‰n lastsession-versioita, koska jos tuottajiin ei ole koskettu k‰sin, niiss‰ on oikeat tuottajat ja jos s‰‰detty k‰sin niin myˆs
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
			if(infos[0]->SizeLevels() == 1) // ainakin aluksi t‰h‰n hyv‰ksyt‰‰n vain ns. pintadatat
				itsProducers.push_back(*(infos[0]->Producer()));
		}
	}
}

void NFmiAnalyzeToolData::SelectedProducer1(const NFmiProducer &theProducer, bool handSelected)
{
	if(fIsSelectionsMadeYet && handSelected == false)
		return ; //t‰ll‰ist‰ tilannetta ei kai pit‰isi tulla, mutta varaudutaan siihen kuitenkin

	itsSelectedProducer1 = theProducer;
	if(handSelected)
	{
		fIsSelectionsMadeYet = true;
		itsLastSessionProducer1 = itsSelectedProducer1; // t‰t‰ pit‰‰ myˆs p‰ivitt‰‰, koska se talletetaan sitten lopuksi konffeihin
	}
}

void NFmiAnalyzeToolData::SelectedProducer2(const NFmiProducer &theProducer, bool handSelected)
{
	if(fIsSelectionsMadeYet && handSelected == false)
		return ; //t‰ll‰ist‰ tilannetta ei kai pit‰isi tulla, mutta varaudutaan siihen kuitenkin

	itsSelectedProducer2 = theProducer;
	if(handSelected)
	{
		fIsSelectionsMadeYet = true;
		itsLastSessionProducer2 = itsSelectedProducer2; // t‰t‰ pit‰‰ myˆs p‰ivitt‰‰, koska se talletetaan sitten lopuksi konffeihin
	}
}

bool NFmiAnalyzeToolData::SelectProducer1ByName(const std::string &theProducerName)
{
	// HUOM! t‰ss‰ olen k‰ytt‰nyt boost:in bind ja function templaatteja
	// niin ett‰ voin antaa olion member-function:in parametrina luokan metodille.
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
		return true; // jos analyysi tyˆkalu ei ole k‰ytˆss‰, enabloi aina
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
