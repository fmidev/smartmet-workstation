// ======================================================================
/*!
 * \file NFmiDataQualityChecker.cpp
 * \brief Implementation of NFmiDataQualityChecker-class.
 */
// ======================================================================

#include "NFmiDataQualityChecker.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiQueryData.h"
#include "NFmiSettings.h"
#include "NFmiQueryDataUtil.h"

const NFmiPoint NFmiGridValuesCheck::gMissingGridPoint = NFmiPoint(-1, -1);

// *****************************************************
// ********* NFmiGridValuesCheck ***********************
// *****************************************************

NFmiGridValuesCheck::NFmiGridValuesCheck(void)
:itsMissTime(NFmiMetTime::gMissingTime)
,itsMinTime(NFmiMetTime::gMissingTime)
,itsMaxTime(NFmiMetTime::gMissingTime)
,itsParam()
,itsLocationCheckingStep(0)
,itsMissingValueProsent(0)
,itsCheckedLocationCount(0)
,itsMissingValueCount(0)
,itsMinValue(kFloatMissing)
,itsMaxValue(kFloatMissing)
,fChecksDone(false)
,fParamFound(false)
,itsMinValueLatlon(NFmiPoint::gMissingLatlon)
,itsMinValueGridPoint(gMissingGridPoint)
,itsMaxValueLatlon(NFmiPoint::gMissingLatlon)
,itsMaxValueGridPoint(gMissingGridPoint)
,itsMissingValueLatlon(NFmiPoint::gMissingLatlon)
,itsMissingValueGridPoint(gMissingGridPoint)
{
}

// theLatLonInfo-oliota k‰ytet‰‰n vain ett‰ saataisiin latlon-pisteet talteen
void NFmiGridValuesCheck::CheckGrid(const NFmiDataMatrix<float> &theValues, const NFmiMetTime &theTime, const NFmiParam &theParam, NFmiFastQueryInfo &theLatLonInfo)
{
	Clear();
	itsMissTime = theTime;
	itsMinTime = theTime;
	itsMaxTime = theTime;
	itsParam = theParam;
	fParamFound = true;
	theLatLonInfo.FirstLocation(); // asetetaan info (0, 0) indeksiin, ett‰ voidaan ted‰ peeklatlon-kutsuja hila pisteiden avulla

	for(size_t j=0; j<theValues.NY(); j++)
	{
		for(size_t i=0; i<theValues.NX(); i++)
		{
			float value = theValues[i][j];
			itsCheckedLocationCount++;
			if(value == kFloatMissing)
			{
				itsMissingValueCount++;
				if(itsMissingValueGridPoint == gMissingGridPoint)
				{ // laitetaan siis 1. missing arvon hila paikka talteen
					itsMissingValueGridPoint = NFmiPoint(static_cast<double>(i), static_cast<double>(j));
					itsMissingValueLatlon = theLatLonInfo.PeekLocationLatLon(static_cast<int>(i), static_cast<int>(j));
				}
			}
			else
			{ // oletus, nyt arvo ei voi olla en‰‰ puuttuva
				if(itsMinValue == kFloatMissing || value < itsMinValue)
				{
					itsMinValue = value;
					itsMinValueGridPoint = NFmiPoint(static_cast<double>(i), static_cast<double>(j));
					itsMinValueLatlon = theLatLonInfo.PeekLocationLatLon(static_cast<int>(i), static_cast<int>(j));
				}

				if(itsMaxValue == kFloatMissing || value > itsMaxValue)
				{
					itsMaxValue = value;
					itsMaxValueGridPoint = NFmiPoint(static_cast<double>(i), static_cast<double>(j));
					itsMaxValueLatlon = theLatLonInfo.PeekLocationLatLon(static_cast<int>(i), static_cast<int>(j));
				}
			}
		}
	}
	if(itsCheckedLocationCount)
	{
		fChecksDone = true;
		itsMissingValueProsent = CalculateProcent(itsMissingValueCount, itsCheckedLocationCount);
	}
}

double NFmiGridValuesCheck::CalculateProcent(size_t theHitCount, size_t theTotalCount)
{
	if(theTotalCount)
		return static_cast<double>(theHitCount)/theTotalCount * 100.;
	else
		return 0;
}

void NFmiGridValuesCheck::Clear(void)
{
	*this = NFmiGridValuesCheck();
}

// lasketaan this-olioon kahden NFmiGridValuesCheck-olion kumulatiinen tulos.
// Lis‰ksi pit‰‰ ottaa talteen kolme aikaa.
// 1. ei puuttuva missing-time.
// pienemm‰n min-arvon aika
// suuremman max-arvon aika
void NFmiGridValuesCheck::Add(const NFmiGridValuesCheck &theOther)
{
	if(fChecksDone == false && theOther.fChecksDone == false)
		return ;
	else if(fChecksDone == true && theOther.fChecksDone == false)
		return ;
	else if(fChecksDone == false && theOther.fChecksDone == true)
	{
		*this = theOther;
		return ;
	}
	else
	{

		// itsTime // aikaa ei lasketa
		// itsParam // parametrin pit‰isi olla molemmissa sama
		// itsLocationCheckingStep // t‰ll‰ ei ole v‰li‰
		fParamFound = (fParamFound || theOther.fParamFound);

		itsCheckedLocationCount += theOther.itsCheckedLocationCount;
		itsMissingValueCount += theOther.itsMissingValueCount;
		itsMissingValueProsent = CalculateProcent(itsMissingValueCount, itsCheckedLocationCount);

		if(itsMissingValueLatlon == NFmiPoint::gMissingLatlon)
		{ // jos this-oliolta puuttuu ensimm‰isenk‰‰n puuttuvan paikan sijainti, otetaan kylm‰sti arvot toisesta (oli ne puuttuvaa tai ei)
			itsMissingValueLatlon = theOther.itsMissingValueLatlon;
			itsMissingValueGridPoint = theOther.itsMissingValueGridPoint;
			itsMissTime = theOther.itsMissTime;
		}

		if(itsMinValue != kFloatMissing && theOther.itsMinValue != kFloatMissing)
		{
			if(itsMinValue > theOther.itsMinValue)
			{ // jos this:in min arvo on suurempi kuin otherin, sijoitetaan kaikki min-arvot otherista this:iin
				itsMinTime = theOther.itsMinTime;
				itsMinValue = theOther.itsMinValue;
				itsMinValueLatlon = theOther.itsMinValueLatlon;
				itsMinValueGridPoint = theOther.itsMinValueGridPoint;
			}
		}
		else if(theOther.itsMinValue != kFloatMissing)
		{ // jos this;in min-arvo oli puuttuva ja otherin ei, pit‰‰ otherista siirt‰‰ kaikki min-arvot this:iin
			itsMinTime = theOther.itsMinTime;
			itsMinValue = theOther.itsMinValue;
			itsMinValueLatlon = theOther.itsMinValueLatlon;
			itsMinValueGridPoint = theOther.itsMinValueGridPoint;
		}

		if(itsMaxValue != kFloatMissing && theOther.itsMaxValue != kFloatMissing)
		{
			if(itsMaxValue < theOther.itsMaxValue)
			{ // jos this:in max arvo on pienempi kuin otherin, sijoitetaan kaikki max-arvot otherista this:iin
				itsMaxTime = theOther.itsMaxTime;
				itsMaxValue = theOther.itsMaxValue;
				itsMaxValueLatlon = theOther.itsMaxValueLatlon;
				itsMaxValueGridPoint = theOther.itsMaxValueGridPoint;
			}
		}
		else if(theOther.itsMaxValue != kFloatMissing)
		{ // jos this;in max-arvo oli puuttuva ja otherin ei, pit‰‰ otherista siirt‰‰ kaikki max-arvot this:iin
			itsMaxTime = theOther.itsMaxTime;
			itsMaxValue = theOther.itsMaxValue;
			itsMaxValueLatlon = theOther.itsMaxValueLatlon;
			itsMaxValueGridPoint = theOther.itsMaxValueGridPoint;
		}
	}
}

// *****************************************************
// ********* NFmiDataParamCheckingInfo *****************
// *****************************************************

NFmiDataParamCheckingInfo::NFmiDataParamCheckingInfo(void)
:itsCheckedParam()
,itsMissingValueErrorLimit(kFloatMissing)
,itsMissingValueWarningLimit(kFloatMissing)
,itsMinValueErrorLimit(kFloatMissing)
,itsMinValueWarningLimit(kFloatMissing)
,itsMaxValueErrorLimit(kFloatMissing)
,itsMaxValueWarningLimit(kFloatMissing)
,itsBaseNameSpace()
{
}

void NFmiDataParamCheckingInfo::InitFromSettings(const std::string &theBaseNameSpace)
{
	itsBaseNameSpace = theBaseNameSpace;

	unsigned long parId = NFmiSettings::Require<unsigned long>(std::string(itsBaseNameSpace + "::ParId"));
	std::string parName = NFmiSettings::Require<std::string>(std::string(itsBaseNameSpace + "::ParName"));
	itsCheckedParam = NFmiParam(parId, parName);

	itsMissingValueErrorLimit = NFmiSettings::Require<double>(std::string(itsBaseNameSpace + "::MissingValueErrorLimit"));
	itsMissingValueWarningLimit = NFmiSettings::Require<double>(std::string(itsBaseNameSpace + "::MissingValueWarningLimit"));
	itsMinValueErrorLimit = NFmiSettings::Require<double>(std::string(itsBaseNameSpace + "::MinValueErrorLimit"));
	itsMinValueWarningLimit = NFmiSettings::Require<double>(std::string(itsBaseNameSpace + "::MinValueWarningLimit"));
	itsMaxValueErrorLimit = NFmiSettings::Require<double>(std::string(itsBaseNameSpace + "::MaxValueErrorLimit"));
	itsMaxValueWarningLimit = NFmiSettings::Require<double>(std::string(itsBaseNameSpace + "::MaxValueWarningLimit"));
}

void NFmiDataParamCheckingInfo::StoreToSettings(void)
{
	if(itsBaseNameSpace.empty() == false)
	{
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::ParId"), NFmiStringTools::Convert(itsCheckedParam.GetIdent()), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::ParName"), itsCheckedParam.GetName().CharPtr(), true);

		NFmiSettings::Set(std::string(itsBaseNameSpace + "::MissingValueErrorLimit"), NFmiStringTools::Convert(itsMissingValueErrorLimit), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::MissingValueWarningLimit"), NFmiStringTools::Convert(itsMissingValueWarningLimit), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::MinValueErrorLimit"), NFmiStringTools::Convert(itsMinValueErrorLimit), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::MinValueWarningLimit"), NFmiStringTools::Convert(itsMinValueWarningLimit), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::MaxValueErrorLimit"), NFmiStringTools::Convert(itsMaxValueErrorLimit), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::MaxValueWarningLimit"), NFmiStringTools::Convert(itsMaxValueWarningLimit), true);
	}
	else
		throw std::runtime_error("Error in NFmiDataParamCheckingInfo::StoreToSettings, unable to store setting.");
}


// *****************************************************
// ********* NFmiDataQualityChecker ********************
// *****************************************************

NFmiDataQualityChecker::NFmiDataQualityChecker(void)
:itsValueCheckMatrix()
,itsDataParamCheckingInfos()
,itsCombinedParamChecks()
,fUse(false)
,fAutomatic(false)
,fViewOn(false)
{
}

NFmiDataQualityChecker::~NFmiDataQualityChecker(void)
{
}

void NFmiDataQualityChecker::InitFromSettings(const std::string & theBaseNameSpace)
{
	Clear();
	itsDataParamCheckingInfos.clear();

	itsBaseNameSpace = theBaseNameSpace;

	fUse = NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::Use"));
	fAutomatic = NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::Automatic"));
	// initialisoidaan viel‰ tarkastettavien parametrien tiedot
	std::string paramBaseKey = itsBaseNameSpace + "::Parameters";
	std::vector<std::string> dataKeys = NFmiSettings::ListChildren(paramBaseKey);
	std::vector<std::string>::iterator iter = dataKeys.begin();
	for( ; iter != dataKeys.end(); ++iter)
	{
		NFmiDataParamCheckingInfo checkInfo;
		checkInfo.InitFromSettings(paramBaseKey + "::" + *iter);
		itsDataParamCheckingInfos.push_back(checkInfo);
	}
}

void NFmiDataQualityChecker::StoreToSettings(void)
{
	if(itsBaseNameSpace.empty() == false)
	{
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::Use"), NFmiStringTools::Convert(fUse), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::Automatic"), NFmiStringTools::Convert(fAutomatic), true);
		for(size_t i=0; i<itsDataParamCheckingInfos.size(); i++)
			itsDataParamCheckingInfos[i].StoreToSettings();
	}
	else
		throw std::runtime_error("Error in NFmiDataQualityChecker::StoreToSettings, unable to store setting.");
}

void NFmiDataQualityChecker::SetCheckedData(std::unique_ptr<NFmiQueryData> theDataPtr)
{
	itsDataPtr.swap(theDataPtr); // otetaan queryData omistukseen
}

static void DoStepIt(NFmiThreadCallBacks *theThreadCallBacks)
{
	if(theThreadCallBacks)
		theThreadCallBacks->StepIt();
}

static void CheckIfStopped(NFmiThreadCallBacks *theThreadCallBacks)
{
	if(theThreadCallBacks)
		theThreadCallBacks->CheckIfStopped(); // heitt‰‰ poikkeiksen jos halutaan lopettaa
}

void NFmiDataQualityChecker::CheckData(NFmiThreadCallBacks *theThreadCallBacks)
{
	Clear();
	if(itsDataPtr.get())
	{
		NFmiFastQueryInfo fInfo(itsDataPtr.get());
		InitCheckMatrix(fInfo);
		NFmiDataMatrix<float> values;
		NFmiDataMatrix<float> values2; // mm. T>Td tarkasteluja varten
		if(theThreadCallBacks)
			theThreadCallBacks->SetRange(0, static_cast<int>(itsValueCheckMatrix.NX()*itsValueCheckMatrix.NY()), 1);

		for(size_t i=0; i<itsValueCheckMatrix.NX(); i++) // parametrit l‰pi
		{
            // Ensin tarkistetaan erikois tilanne eli T>=Td tarkistus, datassa pit‰‰ t‰llˆin myˆs olla T ja Td parametrit
            if(itsDataParamCheckingInfos[i].CheckedParam().GetIdent() == NFmiInfoData::kFmiSpTvsTdQualityCheckParam && fInfo.Param(kFmiTemperature) && fInfo.Param(kFmiDewPoint))
            {
				for(size_t j=0; j<itsValueCheckMatrix.NY(); j++) // ajat l‰pi
				{
					::CheckIfStopped(theThreadCallBacks);
					::DoStepIt(theThreadCallBacks);
					fInfo.TimeIndex(static_cast<unsigned long>(j));
                    fInfo.Param(kFmiTemperature);
					fInfo.Values(values);
                    fInfo.Param(kFmiDewPoint);
					fInfo.Values(values2);
                    values -= values2;
					itsValueCheckMatrix[i][j].CheckGrid(values, fInfo.Time(), *(fInfo.Param().GetParam()), fInfo);
				}
            }
			else if(fInfo.Param(itsDataParamCheckingInfos[i].CheckedParam()))
			{
				for(size_t j=0; j<itsValueCheckMatrix.NY(); j++) // ajat l‰pi
				{
					::CheckIfStopped(theThreadCallBacks);
					::DoStepIt(theThreadCallBacks);
					fInfo.TimeIndex(static_cast<unsigned long>(j));
					fInfo.Values(values);
					itsValueCheckMatrix[i][j].CheckGrid(values, fInfo.Time(), *(fInfo.Param().GetParam()), fInfo);
				}
			}
			else
			{ // pit‰‰ laittaa t‰m‰n parametrin checkeihin joka aikaan ett‰ chekkaus-data on k‰yty l‰pi, mutta parametria ei vain lˆytynyt (se on jo oletus arvona)
				::CheckIfStopped(theThreadCallBacks);
				for(size_t j=0; j<itsValueCheckMatrix.NY(); j++) // ajat l‰pi
				{
					::DoStepIt(theThreadCallBacks);
					itsValueCheckMatrix[i][j].ChecksDone(true);
				}
			}
		}
		CalcCombinedParamChecks();
	}
}

void NFmiDataQualityChecker::CalcCombinedParamChecks(void)
{
	for(size_t i=0; i<itsValueCheckMatrix.NX(); i++) // parametrit l‰pi
	{
		for(size_t j=0; j<itsValueCheckMatrix.NY(); j++) // ajat l‰pi
		{
			itsCombinedParamChecks[i].Add(itsValueCheckMatrix[i][j]);
		}
	}
}

void NFmiDataQualityChecker::InitCheckMatrix(NFmiFastQueryInfo &theInfo)
{
	itsValueCheckMatrix = NFmiDataMatrix<NFmiGridValuesCheck>(); // t‰m‰ tekee totaali tyhjennyksen matriisiin
	size_t timeSize = theInfo.SizeTimes();
	size_t paramSize = itsDataParamCheckingInfos.size();
	itsValueCheckMatrix.Resize(paramSize, timeSize); // t‰ss‰ matriisi alustetaan 'puuttuvilla' arvoilla eli default kontruktorin muodostamilla olioilla

	itsCombinedParamChecks = std::vector<NFmiGridValuesCheck>(); // t‰m‰ tekee totaali tyhjennyksen vektoriin
	itsCombinedParamChecks.resize(paramSize);
}

void NFmiDataQualityChecker::Clear(void)
{ 
	itsValueCheckMatrix.clear();
	itsCombinedParamChecks.clear();

	// HUOM! ei nollata itsDataParamCheckingInfos -dataosaa
}
