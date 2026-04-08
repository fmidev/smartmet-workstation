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
#include "NFmiHelpDataInfo.h"

#include <memory>
#include <boost/function.hpp>


class NFmiInfoOrganizer;
class NFmiHelpDataInfoSystemtem;
class NFmiFastQueryInfo;
class NFmiLimitChecker;
class TimeSerialModificationDataInterface;
class NFmiAreaMaskList;

class NFmiControlPointObservationBlendingData
{
    bool fUseBlendingTool = false;
    // T�m� on valittu havainto tuottaja
    NFmiProducer itsSelectedProducer;  
    // T�m� luetaan konffeista ja t�m� pyrit��n saamaan valituksi my�s itsSelectedProducer1
    // niin kauan kun fIsSelectionMadeYet on false.
    NFmiProducer itsLastSessionProducer; 
    // Kun SmartMet k�ynnistet��n, alkaa Aikasarja-ty�kalu potentiaalisesti pit�m��n
    // kirjaa valitusta parametrista. Jos SmartMet on ehtinyt esim. lukea vasta yhden analyysi datan
    // mik� ei ollut valittuna viime seeiossa, valitaan se tuottajaksi. Mutta my�hemmin
    // kun lis�� dataa luetaan, tulee my�s k�ytt��n viimeksi valittu, voidaan t�m�bn muuttujan
    // avulla s��t�� se k�ytt��n. Mutta jos joku on tehnyt valintoja jo, ei kosketa listojen valintaan.
    bool fIsSelectionMadeYet = false; 
    bool fOverrideSelection = false;
    // T�h�n ker�t��n k�yt�ss� olevat oikean tyyppiset observation tuottajat
    std::vector<NFmiProducer> itsProducers; 
    std::string itsBaseNameSpace;
    // Kuinka vanhoja havaintoja sallitaan mukaan suhteessa aloitusaikaan itsActualFirstTime
    static long itsExpirationTimeInMinutes;
    // Mik� on maksimi sallittu et�isyys CP-pisteest� l�himp��n havaintoasemaan
    static double itsMaxAllowedDistanceToStationInKm;
    // Kun tarkastellaan mik� on viimeisin hyv�ksytty havaintoaika, jolta
    // ei saa puuttua liikaa havaintoja editointi- tai zoomatun -alueen asemista.
    static float itsNonMissingObservationValueRatioLimit;


public:
    void InitFromSettings(const std::string &theBaseNameSpace);
    void StoreToSettings(void);
    bool UseBlendingTool() const { return fUseBlendingTool; }
    void UseBlendingTool(bool newValue) { fUseBlendingTool = newValue; }
    const NFmiProducer& SelectedProducer() const { return itsSelectedProducer; }
    bool SelectProducer(const std::string &theProducerName);
    bool SelectProducer(unsigned long theProducerId);
    bool IsSelectionMadeYet(void) const { return fIsSelectionMadeYet; }
    void SeekProducers(NFmiInfoOrganizer &theInfoOrganizer);
    const std::vector<NFmiProducer>& Producers() const { return itsProducers; }
    static bool IsGoodObservationDataForCpPointConversion(std::shared_ptr<NFmiFastQueryInfo> &info);
    bool OverrideSelection() const { return fOverrideSelection; }
    void OverrideSelection(bool newValue);

    static float BlendData(float editedDataValue, float changeValue, float maskFactor, unsigned long timeSize, unsigned long timeIndex, const NFmiLimitChecker &limitChecker);
    static long ExpirationTimeInMinutes();
    static void ExpirationTimeInMinutes(long newValue);
    static double MaxAllowedDistanceToStationInKm();
    static void MaxAllowedDistanceToStationInKm(double newValue);
    static float NonMissingObservationValueRatioLimit();
    static void NonMissingObservationValueRatioLimit(float newValue);
private:
    bool UpdateProducerInfo(const NFmiProducer &producer);
};

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
	bool AnalyzeToolMode(void) const {return fAnalyzeToolMode;}
	void AnalyzeToolMode(bool newValue) {fAnalyzeToolMode = newValue;}
	bool UseBothProducers(void) const {return fUseBothProducers;}
	void UseBothProducers(bool newValue) {fUseBothProducers = newValue;}
	const std::vector<std::string>& PotentialProducersFileFilters(void) const {return itsPotentialProducersFileFilters;}
	const std::vector<NFmiProducer>& Producers(void) const {return itsProducers;}
	bool IsSelectionsMadeYet(void) const {return fIsSelectionsMadeYet;}
	bool SelectProducer1ByName(const std::string &theProducerName);
	bool SelectProducer2ByName(const std::string &theProducerName);
	bool EnableAnalyseTool(NFmiInfoOrganizer &theInfoOrganizer, const NFmiParam &theParam);
    NFmiControlPointObservationBlendingData& ControlPointObservationBlendingData() { return itsControlPointObservationBlendingData; }

    static std::pair<NFmiMetTime, NFmiMetTime> GetLatestSuitableAnalyzeToolInfoTime(std::vector<std::shared_ptr<NFmiFastQueryInfo>> &infos, std::shared_ptr<NFmiFastQueryInfo> &editedInfo, const std::shared_ptr<NFmiArea> &checkedObservationArea, bool useObservationBlenderTool, const std::string &usedProducerName);
    static std::shared_ptr<NFmiArea> GetUsedAreaForAnalyzeTool(TimeSerialModificationDataInterface &theAdapter, std::shared_ptr<NFmiFastQueryInfo> &editedInfo);
    static std::shared_ptr<NFmiAreaMaskList> GetUsedTimeSerialMaskList(TimeSerialModificationDataInterface &theAdapter);

private:
	bool SelectProducerByName(const std::string &theProducerName, ProdSetter theSetter);

	NFmiProducer itsSelectedProducer1;  // T�m� on nyt valittu tuottaja (niist� mitk� ovat juuri nyt k�yt�ss�).
	NFmiProducer itsLastSessionProducer1; // T�m� luetaan konffeista ja t�m� pyrit��n saamaan valituksi my�s itsSelectedProducer1
										// niin kauan kun fIsSelectionsMadeYet on false.
	NFmiProducer itsSelectedProducer2; // t�t� k�ytet��n vain jos fUseBothProducers on true
	NFmiProducer itsLastSessionProducer2;
	NFmiMetTime itsAnalyzeToolEndTime; // t�h�n ajanhetkeen analyysi ty�kalu liu'uttaa analyysia ennusteeseen
	bool fAnalyzeToolMode; // onko editori analyysi ty�kalu tilassa vai ei.
	bool fUseBothProducers; // K�ytet��nk� analyysi muokkauksessa kahta siten ett� prim��ri data valituille pisteille ja
							// sekund��ri data ei valituille pisteille.
	bool fIsSelectionsMadeYet; // Kun SmartMet k�ynnistet��n, alkaa Aikasarja-ty�kalu potentiaalisesti pit�m��n
								// kirjaa valitusta parametrista. Jos SmartMet on ehtinyt esim. lukea vasta yhden analyysi datan
								// mik� ei ollut valittuna viime seeiossa, valitaan se tuottajaksi. Mutta my�hemmin
								// kun lis�� dataa luetaan, tulee my�s k�ytt��n viimeksi valittu, voidaan t�m�bn muuttujan
								// avulla s��t�� se k�ytt��n. Mutta jos joku on tehnyt valintoja jo, ei kosketa listojen valintaan.
	std::vector<NFmiProducer> itsProducers; // t�h�n ker�t��n filefilttereiden avull� ladatut ja siis k�yt�ss� olevat tuottajat
	std::vector<std::string> itsPotentialProducersFileFilters; // t�h�n ker�t��n kaikki potentiaaliset tuottajat helpdata-info konffeista. Mutta koska konffeista ei saa tuottajaa
																// joudumme k�ytt�m�� tunnisteen� fileFilteri�
    NFmiControlPointObservationBlendingData itsControlPointObservationBlendingData;
	std::string itsBaseNameSpace;
};

