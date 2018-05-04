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
#include <boost/bind.hpp>

NFmiAnalyzeToolData::NFmiAnalyzeToolData(void)
:itsSelectedProducer1()
,itsLastSessionProducer1()
,itsSelectedProducer2()
,itsLastSessionProducer2()
,itsAnalyzeToolEndTime()
,fAnalyzeToolWithAllParams(false)
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
	fAnalyzeToolWithAllParams = NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::AnalyzeToolWithAllParams"));
	fUseBothProducers = NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::UseBothProducers"));
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

		NFmiSettings::Set(std::string(itsBaseNameSpace + "::AnalyzeToolWithAllParams"), NFmiStringTools::Convert(fAnalyzeToolWithAllParams), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::UseBothProducers"), NFmiStringTools::Convert(fUseBothProducers), true);

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
