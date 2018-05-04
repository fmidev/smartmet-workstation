// ======================================================================
/*!
 * \file NFmiAnalyzeToolData.h
 * \brief Class that bundles together information that involves the analyze
 *  tool in timeserialview.
 */
// ======================================================================

#pragma once

#include "NFmiProducer.h"
#include "NFmiMetTime.h"
#include "NFmiDataMatrix.h"
#include "NFmiHelpDataInfo.h"

#include <boost/function.hpp>


class NFmiInfoOrganizer;
class NFmiHelpDataInfoSystemtem;

class NFmiAnalyzeToolData
{
public:

	typedef boost::function<void (NFmiProducer theProd, bool status) > ProdSetter;

	NFmiAnalyzeToolData(void);
	~NFmiAnalyzeToolData(void);

	void InitFromSettings(const std::string &theBaseNameSpace);
	void StoreToSettings(void);
	void SeekPotentialProducersFileFilters(const NFmiHelpDataInfoSystem &theHelpDataInfoSystem);
	void SeekProducers(NFmiInfoOrganizer &theInfoOrganizer);

	const NFmiProducer& SelectedProducer1(void) const {return itsSelectedProducer1;}
	void SelectedProducer1(const NFmiProducer &theProducer, bool handSelected);
	const NFmiProducer& SelectedProducer2(void) const {return itsSelectedProducer2;}
	void SelectedProducer2(const NFmiProducer &theProducer, bool handSelected);
	const NFmiMetTime& AnalyzeToolEndTime(void) const {return itsAnalyzeToolEndTime;}
	void AnalyzeToolEndTime(const NFmiMetTime &theTime) {itsAnalyzeToolEndTime = theTime;}
	bool AnalyzeToolWithAllParams(void) const {return fAnalyzeToolWithAllParams;}
	void AnalyzeToolWithAllParams(bool newValue) {fAnalyzeToolWithAllParams = newValue;}
	bool AnalyzeToolMode(void) const {return fAnalyzeToolMode;}
	void AnalyzeToolMode(bool newValue) {fAnalyzeToolMode = newValue;}
	bool UseBothProducers(void) const {return fUseBothProducers;}
	void UseBothProducers(bool newValue) {fUseBothProducers = newValue;}
	const checkedVector<std::string>& PotentialProducersFileFilters(void) const {return itsPotentialProducersFileFilters;}
	const checkedVector<NFmiProducer>& Producers(void) const {return itsProducers;}
	bool IsSelectionsMadeYet(void) const {return fIsSelectionsMadeYet;}
	bool SelectProducer1ByName(const std::string &theProducerName);
	bool SelectProducer2ByName(const std::string &theProducerName);
	bool EnableAnalyseTool(NFmiInfoOrganizer &theInfoOrganizer, const NFmiParam &theParam);

private:
	bool SelectProducerByName(const std::string &theProducerName, ProdSetter theSetter);

	NFmiProducer itsSelectedProducer1;  // Tämä on nyt valittu tuottaja (niistä mitkä ovat juuri nyt käytössä).
	NFmiProducer itsLastSessionProducer1; // Tämä luetaan konffeista ja tämä pyritään saamaan valituksi myös itsSelectedProducer1
										// niin kauan kun fIsSelectionsMadeYet on false.
	NFmiProducer itsSelectedProducer2; // tätä käytetään vain jos fUseBothProducers on true
	NFmiProducer itsLastSessionProducer2;
	NFmiMetTime itsAnalyzeToolEndTime; // tähän ajanhetkeen analyysi työkalu liu'uttaa analyysia ennusteeseen
	bool fAnalyzeToolWithAllParams; // tehdäänkö analyysin liu'utus jokaiselle parametrille, vaiko vain aktiiviselle
	bool fAnalyzeToolMode; // onko editori analyysi työkalu tilassa vai ei.
	bool fUseBothProducers; // Käytetäänkö analyysi muokkauksessa kahta siten että primääri data valituille pisteille ja
							// sekundääri data ei valituille pisteille.
	bool fIsSelectionsMadeYet; // Kun SmartMet käynnistetään, alkaa Aikasarja-työkalu potentiaalisesti pitämään
								// kirjaa valitusta parametrista. Jos SmartMet on ehtinyt esim. lukea vasta yhden analyysi datan
								// mikä ei ollut valittuna viime seeiossa, valitaan se tuottajaksi. Mutta myöhemmin
								// kun lisää dataa luetaan, tulee myös käyttöön viimeksi valittu, voidaan tämäbn muuttujan
								// avulla säätää se käyttöön. Mutta jos joku on tehnyt valintoja jo, ei kosketa listojen valintaan.
	checkedVector<NFmiProducer> itsProducers; // tähän kerätään filefilttereiden avullä ladatut ja siis käytössä olevat tuottajat
	checkedVector<std::string> itsPotentialProducersFileFilters; // tähän kerätään kaikki potentiaaliset tuottajat helpdata-info konffeista. Mutta koska konffeista ei saa tuottajaa
																// joudumme käyttämää tunnisteenä fileFilteriä
	std::string itsBaseNameSpace;
};

