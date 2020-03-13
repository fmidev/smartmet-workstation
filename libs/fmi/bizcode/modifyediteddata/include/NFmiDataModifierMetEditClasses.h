//**********************************************************
// C++ Class Name : - (many NFmiDataModifier-child classes)
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: NFmiDataModifierMetEditClasses.h 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 2.1.4 ) 
//  - GD System Name    : mask and filter business 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : mask and filter 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Mon - Mar 29, 1999 
// 
// 
//  Description: 
//  Tähän tiedostoon on kerätty useita NFmiDataModifier-luokan
//	lapsiluokkia, jotka tekevät erilaisia asioita.
// 
//  Change Log: 
// 
//**********************************************************

#pragma once

#include "NFmiDataModifier.h"
#include "NFmiMetTime.h"
#include "boost/shared_ptr.hpp"
#include <vector>

class NFmiQueryInfo;
class NFmiAreaMaskList;
class NFmiRect;
class NFmiPoint;
class NFmiGrid;
class NFmiFastQueryInfo;

//************** NFmiDataModifierMissingValueSet ************************
class NFmiDataModifierMissingValueSet : public NFmiDataModifier
{
 public:
	NFmiDataModifierMissingValueSet(double theMissingValue, double theValue);
	NFmiDataModifierMissingValueSet(const NFmiDataModifierMissingValueSet &theOther);
	NFmiDataModifier* Clone(void) const;
	virtual float FloatOperation(float theValue);

 private:
	 double itsMissingValue;
	 double itsValue;
};
//************** NFmiDataModifierMissingValueSet ************************

//************** NFmiDataModifierTimeShifter ************************
class NFmiDataModifierTimeShifter : public NFmiDataModifier
{ 
 public:
	NFmiDataModifierTimeShifter(boost::shared_ptr<NFmiFastQueryInfo> theQueryInfoCopy
								,const std::vector<double> &theTimeShiftFactorTable
								,boost::shared_ptr<NFmiAreaMaskList> &theMaskList)
								:itsLastTimeIndex(-1)
								,itsMaskList(theMaskList)
								,itsQueryInfoCopy(theQueryInfoCopy)
								,itsTimeShiftFactorTable(theTimeShiftFactorTable)
	{}

	NFmiDataModifierTimeShifter(const NFmiDataModifierTimeShifter &theOther);
	NFmiDataModifier* Clone(void) const;
	virtual ~NFmiDataModifierTimeShifter(void){};
	virtual float FloatOperation(float theValue);

	// multi-threaddaus alustus/apu funktiota
	void SetLocationIndex(unsigned long theIndex);
	void SetTimeIndex(unsigned long theIndex);
	void InitLatlonCache(void);
 protected:
	void SyncronizeMasks(void);

	int itsLastTimeIndex; // käytetään optimoinnissa
	boost::shared_ptr<NFmiAreaMaskList> itsMaskList;
	boost::shared_ptr<NFmiFastQueryInfo> itsQueryInfoCopy;
	std::vector<double> itsTimeShiftFactorTable;// taulu, jossa on kaikille ajankohdille siirtoaika 
									// datan aikaresoluutiossa (esim. arvo 1.5 voi olla 
									// 1.5 tuntia tai 9 tuntia resoluutiosta riippuen)
};
//************** NFmiDataModifierTimeShifter ************************

//************** NFmiDataModifierTimeShifterWithChangingRanges ************************
class NFmiDataModifierTimeShifterWithChangingRanges : public NFmiDataModifierTimeShifter
{ 
 public:
	NFmiDataModifierTimeShifterWithChangingRanges(boost::shared_ptr<NFmiFastQueryInfo> theQueryInfoCopy
												,const std::vector<NFmiPoint> &theTimeRangeTable
												,NFmiDataModifier* theDataModifier
												,boost::shared_ptr<NFmiAreaMaskList> &theMaskList)
												:NFmiDataModifierTimeShifter(theQueryInfoCopy, std::vector<double>(), theMaskList)
								,itsNegRange(0)
								,itsPosRange(0)
								,itsModifier(theDataModifier)
								,itsTimeRangeTable(theTimeRangeTable)
								{ }
	NFmiDataModifierTimeShifterWithChangingRanges(const NFmiDataModifierTimeShifterWithChangingRanges &theOther);
	NFmiDataModifier* Clone(void) const;
	virtual ~NFmiDataModifierTimeShifterWithChangingRanges(void);
	virtual float FloatOperation(float theValue);

	// multi-threaddaus alustus/apu funktiota
	void SetLocationIndex(unsigned long theIndex);
	void SetTimeIndex(unsigned long theIndex);
	void InitLatlonCache(void);

 protected:
	int itsNegRange, itsPosRange;
	NFmiDataModifier* itsModifier; // tämä tuhotaan destruktorissa!!
	std::vector<NFmiPoint> itsTimeRangeTable;// taulu, jossa on kaikille ajankohdille aikarajat datan laskua varten
									// datan aikaresoluutiossa (esim. arvo 1.5 voi olla 
									// 1.5 tuntia tai 9 tuntia resoluutiosta riippuen)
};
//************** NFmiDataModifierTimeShifterWithChangingRanges ************************

//************** NFmiDataModifierTimeShifterWithChangingRangesUsingWAvg ************************
class NFmiDataModifierTimeShifterWithChangingRangesUsingWAvg : public NFmiDataModifierTimeShifterWithChangingRanges
{ 
 public:
	NFmiDataModifierTimeShifterWithChangingRangesUsingWAvg(boost::shared_ptr<NFmiFastQueryInfo> theQueryInfoCopy
												,const std::vector<NFmiPoint> &theTimeRangeTable
												,NFmiDataModifier* theDataModifier
												,boost::shared_ptr<NFmiAreaMaskList> &theMaskList)
								:NFmiDataModifierTimeShifterWithChangingRanges(theQueryInfoCopy, theTimeRangeTable, theDataModifier, theMaskList)
								{}
	NFmiDataModifierTimeShifterWithChangingRangesUsingWAvg(const NFmiDataModifierTimeShifterWithChangingRangesUsingWAvg &theOther);
	NFmiDataModifier* Clone(void) const;
	virtual ~NFmiDataModifierTimeShifterWithChangingRangesUsingWAvg(void);
	virtual float FloatOperation(float theValue);

 private:
};
//************** NFmiDataModifierTimeShifterWithChangingRanges ************************


//************** NFmiDataModifierWithModifierWithMasks ************************
class NFmiDataModifierWithModifierWithMasks : public NFmiDataModifier
{ 
 public:
	NFmiDataModifierWithModifierWithMasks(boost::shared_ptr<NFmiFastQueryInfo> theQueryInfoCopy,
											NFmiDataModifier *theDataModifier
											,boost::shared_ptr<NFmiAreaMaskList> &theMaskList
											,int theNegXRange, int theNegYRange
											,int thePosXRange, int thePosYRange)
											:itsLastTimeIndex(-1),
											itsQueryInfoCopy(theQueryInfoCopy),
											itsModifier(theDataModifier),
											itsMaskList(theMaskList),
											itsNegXRange(theNegXRange), 
											itsNegYRange(theNegYRange),
											itsPosXRange(thePosXRange),
											itsPosYRange(thePosYRange)
											{}
	NFmiDataModifierWithModifierWithMasks(const NFmiDataModifierWithModifierWithMasks &theOther);
	NFmiDataModifier* Clone(void) const;
	virtual ~NFmiDataModifierWithModifierWithMasks(void);
	virtual float FloatOperation(float theValue);

	// multi-threaddaus alustus/apu funktiota
	void SetLocationIndex(unsigned long theIndex);
	void SetTimeIndex(unsigned long theIndex);
	void InitLatlonCache(void);
 protected:
	int itsLastTimeIndex; // käytetään optimoinnissa
	boost::shared_ptr<NFmiFastQueryInfo> itsQueryInfoCopy;
	NFmiDataModifier *itsModifier; // tämä tuhotaan destruktorissa!!
	boost::shared_ptr<NFmiAreaMaskList> itsMaskList;
	int itsNegXRange, itsNegYRange, itsPosXRange, itsPosYRange;
};
//************** NFmiDataModifierWithModifierWithMasks ************************

//************** NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime ************************
class NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime : public NFmiDataModifierWithModifierWithMasks
{ 
 public:
	 NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime(boost::shared_ptr<NFmiFastQueryInfo> theQueryInfoCopy,
		NFmiDataModifier *theDataModifier
		,boost::shared_ptr<NFmiAreaMaskList> &theMaskList
		,const std::vector<NFmiRect> &theRangeArray)
		:NFmiDataModifierWithModifierWithMasks(theQueryInfoCopy, theDataModifier, theMaskList,0,0,0,0)
		,itsRangeArray(theRangeArray){};
	NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime(const NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime &theOther);
	NFmiDataModifier* Clone(void) const;
	virtual ~NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime(void);
	virtual float FloatOperation(float theValue);

 protected:
	 std::vector<NFmiRect> itsRangeArray; // tämä tuhotaan destruktorissa!!! // tämän pitää olla yhtä suuri kuin queryinfossa on aikoja
};
//************** NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime ************************

//************** NFmiDataModifierTimeInterpolationWithAreaModifiers ************************
class NFmiDataModifierTimeInterpolationWithAreaModifiers : public NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime
{ 
 public:
	 NFmiDataModifierTimeInterpolationWithAreaModifiers(
						boost::shared_ptr<NFmiFastQueryInfo> theQueryInfoCopy,
						NFmiDataModifier *theDataModifier
						,boost::shared_ptr<NFmiAreaMaskList> &theMaskList
						,const std::vector<NFmiRect> &theRangeArray
						,const NFmiMetTime& theStartTime
						,const NFmiMetTime& theEndTime
						,int theTimeResolutionInMinutes);
	NFmiDataModifierTimeInterpolationWithAreaModifiers(const NFmiDataModifierTimeInterpolationWithAreaModifiers &theOther);
	NFmiDataModifier* Clone(void) const;
	virtual ~NFmiDataModifierTimeInterpolationWithAreaModifiers(void){};
	virtual float FloatOperation(float theValue);

 protected:
	 NFmiMetTime itsStartTime;
	 NFmiMetTime itsEndTime;
	 int itsMaxTimeSteps; // = itsEndTime - itsStartTime / timeresolution
	 int itsCurrentTimeStep; // 1 on alkuajan steppi ja itsMaxStep on loppuajan steppi
	 int itsTimeResolutionInMinutes;

	// tuulen suuntaa varten pitää tehdä virityksiä, että esim. 350 ja 20  interpolointi olisi n. 10 eikä n. 185 (eli maksimi) jne.
	bool fFixWDValue;
	void CheckIfWDParameter(void);
};
//************** NFmiDataModifierTimeInterpolationWithAreaModifiers ************************

//************** NFmiDataModifierParamCombiner ************************
class NFmiDataModifierParamCombiner : public NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime
{ 
 public:
	 NFmiDataModifierParamCombiner(boost::shared_ptr<NFmiFastQueryInfo> theQueryInfoCopy,
		NFmiDataModifier *theDataModifier
		,boost::shared_ptr<NFmiAreaMaskList> &theMaskList
		,const std::vector<NFmiRect> &theRangeArray
		,boost::shared_ptr<NFmiFastQueryInfo> theFirstParamInfo
		,boost::shared_ptr<NFmiFastQueryInfo> theSecondParamInfo
		,const std::vector<float> &theCombineFactorArray
		,float theFirstParamBase=0.f
		,float theSecondParamBase=0.f
		,float theFirstParamFactor=1.f
		,float theSecondParamFactor=1.f
		,float theFirstParamAdditionalMissingValue=kFloatMissing
		,float theSecondParamAdditionalMissingValue=kFloatMissing);
	NFmiDataModifierParamCombiner(const NFmiDataModifierParamCombiner &theOther);
	NFmiDataModifier* Clone(void) const;
	 ~NFmiDataModifierParamCombiner(void);
	virtual float FloatOperation(float theValue);

	// multi-threaddaus alustus/apu funktiota
	void InitLatlonCache(void);
 protected:
	 float Dz2Rain(float theDz, float& theAdditionalMissingValue);
	 float itsRainConstantC; // c = log10 * a / b
	 float itsRainConstantA; // a = 200
	 float itsRainConstantB; // a = 1.6
	 bool fFirstParamIsRadar;
	 bool fSecondParamIsRadar;
	 bool fFirstParamInfoIsSameGridAsModifiedData;
	 bool fSecondParamInfoIsSameGridAsModifiedData;
	 bool fFirstParamUsed; // ehkä parametria ei olekaan asetettu, jolloin ei sitä käytetä
	 bool fSecondParamUsed; // ehkä parametria ei olekaan asetettu, jolloin ei sitä käytetä
	 std::vector<float> itsCombineFactorArray;
	 boost::shared_ptr<NFmiFastQueryInfo> itsFirstParamInfo; // omistaa (yhdistettäville parametreille omat infot, jotta niitä ei tarvitsisi asetella jatkuvasti osoittamaan haluttuun parametriin)
	 boost::shared_ptr<NFmiFastQueryInfo> itsSecondParamInfo; // omistaa
	 float itsFirstParamBase;
	 float itsSecondParamBase;
	 float itsFirstParamFactor; // jos halutaan skaalata eri parametreja, se tapahtuu näiden kertoimien avulla
	 float itsSecondParamFactor;
	 float itsFirstParamAdditionalMissingValue; // esim. tutka heijastuksella tämä on -32
	 float itsSecondParamAdditionalMissingValue;
};
//************** NFmiDataModifierParamCombiner ************************

//************** NFmiDataModelDataCombiner ************************
class NFmiDataModelDataCombiner : public NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime
{ 
 public:
	 NFmiDataModelDataCombiner(boost::shared_ptr<NFmiFastQueryInfo> theQueryInfoCopy,
		NFmiDataModifier *theDataModifier
		,boost::shared_ptr<NFmiAreaMaskList> &theMaskList
		,const std::vector<NFmiRect> &theRangeArray
		,boost::shared_ptr<NFmiFastQueryInfo> theFirstModelInfo
		,boost::shared_ptr<NFmiFastQueryInfo> theSecondModelInfo
		,const std::vector<float> &theCombineFactorArray);
	NFmiDataModelDataCombiner(const NFmiDataModelDataCombiner &theOther);
	NFmiDataModifier* Clone(void) const;
	 ~NFmiDataModelDataCombiner(void);
	virtual float FloatOperation(float theValue);

	// multi-threaddaus alustus/apu funktiota
	void InitLatlonCache(void);
 protected:
	 bool fFirstParamInfoIsSameGridAsModifiedData;
	 bool fSecondParamInfoIsSameGridAsModifiedData;
	 bool fFirstModelInfoHasSameTimeResolutionAsModifiedData;
	 bool fSecondModelInfoHasSameTimeResolutionAsModifiedData;
	 const std::vector<float> itsCombineFactorArray;
	 boost::shared_ptr<NFmiFastQueryInfo> itsFirstModelInfo; // omistaa (yhdistettäville parametreille omat infot, jotta niitä ei tarvitsisi asetella jatkuvasti osoittamaan haluttuun parametriin)
	 boost::shared_ptr<NFmiFastQueryInfo> itsSecondModelInfo; // omistaa
	 NFmiMetTime itsCurrentTime; // käytetään aikainterpolaatiossa
	 int itsFirstModelTimeResolution;
	 int itsSecondModelTimeResolution;
};
//************** NFmiDataModelDataCombiner ************************


//************** NFmiDataModifierValidateData_PrForm_T ************************
// muuttaa Sateen olomuotoa lämpötilan mukaan
class NFmiDataModifierValidateData_PrForm_T : public NFmiDataModifier
{ 
 public:
	NFmiDataModifierValidateData_PrForm_T(boost::shared_ptr<NFmiFastQueryInfo> theTemperatureInfo, float theSnowTemperatureLimit, float theRainTemperatureLimit);
	NFmiDataModifierValidateData_PrForm_T(const NFmiDataModifierValidateData_PrForm_T &theOther);
	NFmiDataModifier* Clone(void) const;
	virtual float FloatOperation(float theValue);

	// multi-threaddaus alustus/apu funktiota
	void SetLocationIndex(unsigned long theIndex);
	void SetTimeIndex(unsigned long theIndex);
	void InitLatlonCache(void);
 protected:
	boost::shared_ptr<NFmiFastQueryInfo> itsTemperatureInfo;
	float itsSnowTemperatureLimit;
	float itsRainTemperatureLimit;
};
//************** NFmiDataModifierValidateData_PrForm_T ************************

//************** NFmiDataModifierValidateData_T_DP ************************
// muuttaa kastepistettä (DP) lämpötilan (T) ja suht. kosteuden (RH) mukaan
class NFmiDataModifierValidateData_T_DP : public NFmiDataModifier
{ 
 public:
	NFmiDataModifierValidateData_T_DP(boost::shared_ptr<NFmiFastQueryInfo> theTemperatureInfo, boost::shared_ptr<NFmiFastQueryInfo> theRHInfo)
										:NFmiDataModifier()
										,itsTemperatureInfo(theTemperatureInfo)
										,itsRHInfo(theRHInfo)
										,itsLastTimeIndex(-1)
										{};
	virtual float FloatOperation(float theValue);

 protected:
	boost::shared_ptr<NFmiFastQueryInfo> itsTemperatureInfo; // sama kuin kopio info muissa modifiereissa ja tätä juoksutetaan ajassa jo valmiiksi
	boost::shared_ptr<NFmiFastQueryInfo> itsRHInfo;
	int itsLastTimeIndex; // käytetään optimoinnissa
};
//************** NFmiDataModifierValidateData_T_DP ************************

//************** NFmiDataModifierValidateData_T_DP_ver2 ************************
// muuttaa kastepistettä (DP) lämpötilan (T) ja suht. kosteuden (RH) mukaan
class NFmiDataModifierValidateData_T_DP_ver2 : public NFmiDataModifier
{ 
 public:
	NFmiDataModifierValidateData_T_DP_ver2(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiDataModifierValidateData_T_DP_ver2(const NFmiDataModifierValidateData_T_DP_ver2 &theOther);
	NFmiDataModifier* Clone(void) const;
	float FloatOperation(float theValue);

	// multi-threaddaus alustus/apu funktiota
	void SetLocationIndex(unsigned long theIndex);
	void SetTimeIndex(unsigned long theIndex);
	void InitLatlonCache(void);
protected:
	boost::shared_ptr<NFmiFastQueryInfo> itsInfo;
	unsigned long itsTIndex;
	unsigned long itsRHIndex;
};
//************** NFmiDataModifierValidateData_T_DP_ver2 ************************

//************** NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTimeUsingWAvg ************************
class NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTimeUsingWAvg : public NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime
{ 
 public:
	 NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTimeUsingWAvg(boost::shared_ptr<NFmiFastQueryInfo> theQueryInfoCopy,
		NFmiDataModifier *theDataModifier
		,boost::shared_ptr<NFmiAreaMaskList> theMaskList
		,const std::vector<NFmiRect> &theRangeArray)
		:NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime(theQueryInfoCopy, theDataModifier, theMaskList,theRangeArray)
		{};
	NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTimeUsingWAvg(const NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTimeUsingWAvg &theOther);
	NFmiDataModifier* Clone(void) const;
	virtual ~NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTimeUsingWAvg(void);
	virtual float FloatOperation(float theValue);

 protected:
};
//************** NFmiDataModifierWithModifierWithMasksAndRangeShiftedWithTime ************************

