// ======================================================================
/*!
 * \file NFmiModelDataBlender.cpp
 * \brief Implementation of NFmiModelDataBlender -class.
 */
// ======================================================================

#include "NFmiModelDataBlender.h"

NFmiModelDataBlender::NFmiModelDataBlender(void)
:fUse(false)
,itsStartTimeHour(30)
,itsEndTimeHour(52)
,itsStarTimeIndex(0)
,itsEndTimeIndex(0)
,itsBlendingFactors()
,itsTimeDescriptor()
{
}

NFmiModelDataBlender::~NFmiModelDataBlender(void)
{
}

static bool FindNearestHourAndIndex(NFmiTimeDescriptor &theTimeDescriptor, int theWantedHour, int &theFoundHour, size_t &theFoundTimeIndex)
{
	NFmiMetTime aTime = theTimeDescriptor.FirstTime();
	aTime.SetTimeStep(60);
	aTime.ChangeByHours(theWantedHour);
	if(theTimeDescriptor.FindNearestTime(aTime))
	{
		theFoundTimeIndex = theTimeDescriptor.Index();
		theFoundHour = theTimeDescriptor.Time().DifferenceInHours(theTimeDescriptor.FirstTime());
		return true;
	}
	else
		return false;
}

// alustetaan itsBlendingFactors uudestaan, kuitenkin yritt‰en s‰ilytt‰‰ edelliset arvot mahd. hyvin
// jos theStartHour tai theEndHour on -1, otetaan olemassa oleva arvo k‰yttˆˆn.
void NFmiModelDataBlender::InitWithNewTimes(const NFmiTimeDescriptor &theTimeDescriptor, int theStartHour, int theEndHour)
{
	if(theStartHour == -1 || theEndHour == -1)
	{ // vaihdetaan vain timeDescriptor, ja s‰ilytet‰‰n kaikki muu ennallaan jos mahdollista
		itsTimeDescriptor = theTimeDescriptor;
		itsBlendingFactors.resize(itsTimeDescriptor.Size() ,1.f);
	}
	else
	{ // laske lineaariset kertoimet kaikille kertoimille
		itsTimeDescriptor = theTimeDescriptor;
		itsBlendingFactors.resize(itsTimeDescriptor.Size() ,1.f);
		bool stat1 = ::FindNearestHourAndIndex(itsTimeDescriptor, theStartHour, itsStartTimeHour, itsStarTimeIndex);
		bool stat2 = ::FindNearestHourAndIndex(itsTimeDescriptor, theEndHour, itsEndTimeHour, itsEndTimeIndex);

		if(stat1 && stat2)
			MakeBlendLinear();
		else
			throw std::runtime_error("Error in NFmiModelDataBlender::InitWithNewTimes: given hours don't match to given timeDescriptor.");
	}
}

// laskee tunti tasolla lineaarisen kertoimen annetulle indeksille.
float NFmiModelDataBlender::CalcLinearFactor(size_t index)
{
	if(itsTimeDescriptor.Time(static_cast<unsigned long>(index)))
	{
		NFmiMetTime atime = itsTimeDescriptor.Time();
		long hourDiff = atime.DifferenceInHours(itsTimeDescriptor.FirstTime());
		if(hourDiff <= itsStartTimeHour)
			return 0.f;
		else if(hourDiff >= itsEndTimeHour)
			return 1.f;
		else
		{
			if(itsEndTimeHour == itsStartTimeHour)
				throw std::runtime_error("Error in NFmiModelDataBlender::CalcLinearFactor: division-by-zero");
			else
			{
				float factor = (hourDiff - itsStartTimeHour)/(static_cast<float>(itsEndTimeHour) - itsStartTimeHour);
				return factor;
			}
		}
	}
	else
		throw std::runtime_error("Error in NFmiModelDataBlender::CalcLinearFactor: index out-of-vector");
}

// tekee olemassa olevan blendaus v‰lin kertoimista lineaariset.
// Siten ett‰ kaikki kertoimet ennen itsStarTimeIndex:i‰ ovat 0:ia, 
// kaikki kertoimet itsendTimeIndex:in j‰lkeen ovat 1:i‰ ja
// sill‰ v‰lill‰ ne nousevat lineaarisesti 0:sta 1:een.
// HUOM! lineaarisuus pit‰‰ laskea tunti tasolla, ei indeksi tasolla.
void NFmiModelDataBlender::MakeBlendLinear(void)
{
	for(size_t i = 0; i < itsBlendingFactors.size(); i++)
		itsBlendingFactors[i] = CalcLinearFactor(i);
}

float NFmiModelDataBlender::GetBlendFactor(size_t theIndex)
{
	if(theIndex < itsBlendingFactors.size())
		return itsBlendingFactors[theIndex];
	else
		throw std::runtime_error("Error in NFmiModelDataBlender::GetBlendFactor: index out-of-vector");
}

float NFmiModelDataBlender::SetBlendFactor(size_t theIndex, float newValue)
{
	if(theIndex < itsBlendingFactors.size())
	{
		float usedValue = std::max(0.f, std::min(1.f, newValue)); // haetaan luku joka on varmasti 0:n ja 1:n v‰lilt‰
		return itsBlendingFactors[theIndex] = usedValue;
	}
	else
		throw std::runtime_error("Error in NFmiModelDataBlender::SetBlendFactor: index out-of-vector");
}

int NFmiModelDataBlender::GetTimeStepInMinutes(size_t theIndex)
{
	if(itsTimeDescriptor.Time(static_cast<unsigned long>(theIndex)))
		return itsTimeDescriptor.Resolution();
	else
		throw std::runtime_error("Error in NFmiModelDataBlender::GetTimeStepInMinutes: index out-of-vector");
}

int NFmiModelDataBlender::GetForecastHour(size_t theIndex)
{
	if(itsTimeDescriptor.Time(static_cast<unsigned long>(theIndex)))
	{
		return itsTimeDescriptor.Time().DifferenceInHours(itsTimeDescriptor.FirstTime());
	}
	else
		throw std::runtime_error("Error in NFmiModelDataBlender::GetForecastHour: index out-of-vector");
}

bool NFmiModelDataBlender::MoveStartTime(FmiDirection theDirection)
{
	if(theDirection == kBackward)
	{
		if(itsStarTimeIndex > 0)
			itsStarTimeIndex--;
		else
			return false;
	}
	else // kForward
	{
		if(itsStarTimeIndex < itsTimeDescriptor.Size() - 2)
		{
			itsStarTimeIndex++;
			if(itsStarTimeIndex >= itsEndTimeIndex)
				itsEndTimeIndex++; // pit‰‰ s‰‰t‰‰ myˆs lopun indeksi‰, ett‰ eiv‰t ole ristiss‰
		}
		else
			return false;
	}
	UpdateForecastHours();
	return true;
}

bool NFmiModelDataBlender::MoveEndTime(FmiDirection theDirection)
{
	if(theDirection == kBackward)
	{
		if(itsEndTimeIndex > 1)
		{
			itsEndTimeIndex--;
			if(itsEndTimeIndex <= itsStarTimeIndex)
				itsStarTimeIndex = itsEndTimeIndex - 1;
		}
		else
			return false;
	}
	else // kForward
	{
		if(itsEndTimeIndex < itsTimeDescriptor.Size() - 1)
			itsEndTimeIndex++;
		else
			return false;
	}
	UpdateForecastHours();
	return true;
}

// p‰ivitt‰‰ alku ja loppu tunnin voimassa olevien indeksien avulla
void NFmiModelDataBlender::UpdateForecastHours(void)
{
	itsStartTimeHour = GetForecastHour(itsStarTimeIndex);
	itsEndTimeHour = GetForecastHour(itsEndTimeIndex);
}

const NFmiMetTime& NFmiModelDataBlender::GetStartTime(void)
{
	if(itsTimeDescriptor.Time(static_cast<unsigned long>(itsStarTimeIndex)))
		return itsTimeDescriptor.Time();
	else
		return NFmiMetTime::gMissingTime;
}

const NFmiMetTime& NFmiModelDataBlender::GetEndTime(void)
{
	if(itsTimeDescriptor.Time(static_cast<unsigned long>(itsEndTimeIndex)))
		return itsTimeDescriptor.Time();
	else
		return NFmiMetTime::gMissingTime;
}
