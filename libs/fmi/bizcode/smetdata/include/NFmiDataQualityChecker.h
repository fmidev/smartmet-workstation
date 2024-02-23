// ======================================================================
/*!
 * \file NFmiDataQualityChecker.h
 * \brief Class that check e.g. edited data for missing values, min and max 
 *        values and makes data quality reports.
 */
// ======================================================================

#pragma once

#include "NFmiDataMatrix.h"
#include "NFmiMetTime.h"
#include "NFmiParam.h"

class NFmiQueryData;
class NFmiFastQueryInfo;
class NFmiThreadCallBacks;

// luokassa on yhden hilan (1 aika, paikka ja level) tarkastustiedot
class NFmiGridValuesCheck
{
public:
	static const NFmiPoint gMissingGridPoint; // Tätä missing grid-pistettä (-1, -1) voi käyttää, jos haluaa ilmaista että jokin grid-piste ei mahdollisesti ole initialisoitu

	NFmiGridValuesCheck(void);

	void CheckGrid(const NFmiDataMatrix<float> &theValues, const NFmiMetTime &theTime, const NFmiParam &theParam, NFmiFastQueryInfo &theLatLonInfo);
	void Clear(void);
	void Add(const NFmiGridValuesCheck &theOther);

	const NFmiMetTime& MissTime(void) const {return itsMissTime;}
	void MissTime(const NFmiMetTime &newValue) {itsMissTime = newValue;}
	const NFmiMetTime& MinTime(void) const {return itsMinTime;}
	void MinTime(const NFmiMetTime &newValue) {itsMinTime = newValue;}
	const NFmiMetTime& MaxTime(void) const {return itsMaxTime;}
	void MaxTime(const NFmiMetTime &newValue) {itsMaxTime = newValue;}

	const NFmiParam& Param(void) const {return itsParam;}
	void Param(const NFmiParam &newValue) {itsParam = newValue;}
	size_t LocationCheckingStep(void) const {return itsLocationCheckingStep;}
	void LocationCheckingStep(size_t newValue) {itsLocationCheckingStep = newValue;}
	double MissingValueProsent(void) const {return itsMissingValueProsent;}
	size_t CheckedLocationCount(void) const {return itsCheckedLocationCount;}
	size_t MissingValueCount(void) const {return itsMissingValueCount;}
	float MinValue(void) const {return itsMinValue;}
	float MaxValue(void) const {return itsMaxValue;}
	bool ChecksDone(void) const {return fChecksDone;}
	void ChecksDone(bool newValue) {fChecksDone = newValue;}
	bool ParamFound(void) const {return fParamFound;}
	void ParamFound(bool newValue) {fParamFound = newValue;}
	const NFmiPoint& MinValueLatlon(void) const {return itsMinValueLatlon;}
	const NFmiPoint& MinValueGridPoint(void) const {return itsMinValueGridPoint;}
	const NFmiPoint& MaxValueLatlon(void) const {return itsMaxValueLatlon;}
	const NFmiPoint& MaxValueGridPoint(void) const {return itsMaxValueGridPoint;}
	const NFmiPoint& MissingValueLatlon(void) const {return itsMissingValueLatlon;}
	const NFmiPoint& MissingValueGridPoint(void) const {return itsMissingValueGridPoint;}

private:
	double CalculateProcent(size_t theHitCount, size_t theTotalCount);

	NFmiMetTime itsMissTime; // näitä eri aikoja tarvitaan oikeastaan vasta kun statistiikkaa kumuloidaan
	NFmiMetTime itsMinTime;
	NFmiMetTime itsMaxTime;
	NFmiParam itsParam;
	size_t itsLocationCheckingStep; // kuinka monen hilapisteen yli hypätään tarkastuksissa, jos 0, tarkastetaan kaikki (nopeuttaa, jos ei tarkasteta kaikkia pisteitä)
	double itsMissingValueProsent;
	size_t itsCheckedLocationCount;
	size_t itsMissingValueCount;
	float itsMinValue;
	float itsMaxValue;
	bool fChecksDone;
	bool fParamFound; // löytyikö parametri tutkittavasta datasta
	NFmiPoint itsMinValueLatlon; 
	NFmiPoint itsMinValueGridPoint; // hilan x, y koordinaatti, indeksit alkavat 0:sta ja vasen ala kulma on 0,0 ja oikea yläkulma on m-1, n-1
	NFmiPoint itsMaxValueLatlon;
	NFmiPoint itsMaxValueGridPoint;
	NFmiPoint itsMissingValueLatlon; // 1. puuttuvan arvo sijainti, muiden puuttuvien pisteiden paikkaa ei talleteta
	NFmiPoint itsMissingValueGridPoint;
};

// tässä info luokassa on eri parametreille annettuja varoitus ja virhe rajoja
class NFmiDataParamCheckingInfo
{
public:
	NFmiDataParamCheckingInfo(void);

	void InitFromSettings(const std::string &theBaseNameSpace);
	void StoreToSettings(void);

	const NFmiParam& CheckedParam(void) const {return itsCheckedParam;}
	void CheckedParam(const NFmiParam &newValue) {itsCheckedParam = newValue;}
	double MissingValueErrorLimit(void) const {return itsMissingValueErrorLimit;}
	void MissingValueErrorLimit(double newValue) {itsMissingValueErrorLimit = newValue;}
	double MissingValueWarningLimit(void) const {return itsMissingValueWarningLimit;}
	void MissingValueWarningLimit(double newValue) {itsMissingValueWarningLimit = newValue;}
	double MinValueErrorLimit(void) const {return itsMinValueErrorLimit;}
	void MinValueErrorLimit(double newValue) {itsMinValueErrorLimit = newValue;}
	double MinValueWarningLimit(void) const {return itsMinValueWarningLimit;}
	void MinValueWarningLimit(double newValue) {itsMinValueWarningLimit = newValue;}
	double MaxValueErrorLimit(void) const {return itsMaxValueErrorLimit;}
	void MaxValueErrorLimit(double newValue) {itsMaxValueErrorLimit = newValue;}
	double MaxValueWarningLimit(void) const {return itsMaxValueWarningLimit;}
	void MaxValueWarningLimit(double newValue) {itsMaxValueWarningLimit = newValue;}

private:
	NFmiParam itsCheckedParam;
	double itsMissingValueErrorLimit;
	double itsMissingValueWarningLimit;
	double itsMinValueErrorLimit;
	double itsMinValueWarningLimit;
	double itsMaxValueErrorLimit;
	double itsMaxValueWarningLimit;

	std::string itsBaseNameSpace;
};

class NFmiDataQualityChecker
{
public:
	NFmiDataQualityChecker(void);
	~NFmiDataQualityChecker(void);

	void InitFromSettings(const std::string &theBaseNameSpace);
	void StoreToSettings(void);

	void CheckData(NFmiThreadCallBacks *theThreadCallBacks);
	void SetCheckedData(std::unique_ptr<NFmiQueryData> theDataPtr);
	void Clear(void);

	bool Use(void) const {return fUse;}
	void Use(bool newValue) {fUse = newValue;}
	bool Automatic(void) const {return fAutomatic;}
	void Automatic(bool newValue) {fAutomatic = newValue;}
	bool ViewOn(void) const {return fViewOn;}
	void ViewOn(bool newValue) {fViewOn = newValue;}

	const std::vector<NFmiDataParamCheckingInfo>& DataParamCheckingInfos(void) const {return itsDataParamCheckingInfos;}
	const NFmiDataMatrix<NFmiGridValuesCheck>& ValueCheckMatrix(void) const {return itsValueCheckMatrix;}
	const std::vector<NFmiGridValuesCheck>& CombinedParamChecks(void) const {return itsCombinedParamChecks;}

private:
	void InitCheckMatrix(NFmiFastQueryInfo &theInfo);
	void CalcCombinedParamChecks(void);

	std::vector<NFmiDataParamCheckingInfo> itsDataParamCheckingInfos;
	NFmiDataMatrix<NFmiGridValuesCheck> itsValueCheckMatrix; // data on täällä param, time järjestyksessä x-indeksi siis parametri ja y-indeksi on aika
	std::vector<NFmiGridValuesCheck> itsCombinedParamChecks; // tässä on koosteet jokaisesta parametrista (yli kaikkien aikojen)
	std::unique_ptr<NFmiQueryData> itsDataPtr; // data välitetään tänne ja säilytetään unique_ptr:ssa, siksi että olisi varmaa että 
											// tämä luokka omistaa annetun datan, eikä tule ongelmia eri threadien välillä.
	bool fUse; // onko checker käytössä vai ei
	bool fAutomatic; // toimiiko checker työ-threadissa automaattisesti, vaiko toimitaanko vai on-demandina eli nappia painamalla
	bool fViewOn; // Onko dialogi avattuna vai ei
	std::string itsBaseNameSpace;
};

