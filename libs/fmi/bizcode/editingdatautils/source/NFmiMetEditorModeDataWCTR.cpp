//**********************************************************
// C++ Class Name : NFmiMetEditorModeDataWCTR (=With Changing Time Resolution) 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: NFmiMetEditorModeDataWCTR.cpp 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 4.0 ) 
//  - GD System Name    : nowcast starts planning 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : NFmiMetEditorModeDataWCTR 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Thu - Jan 16, 2003 
// 
//  Change Log     : 
// 
//**********************************************************

#include "NFmiMetEditorModeDataWCTR.h"
#include "NFmiTimeDescriptor.h"
#include "NFmiTimeList.h"
#include "NFmiSettings.h"
#include <cmath>

#include <boost\math\special_functions\round.hpp>

using namespace std;

static int CalcTotalSuggestedTime(const std::vector<NFmiMetEditorModeDataWCTR::TimeSectionData> &theTimeSections)
{
	int totalSuggestedTime = 0;
	for(int i = 0; i < static_cast<int>(theTimeSections.size()); i++)
		totalSuggestedTime += theTimeSections[i].itsSuggestedSectionLengthInHours;
	return totalSuggestedTime;
}

static NFmiMetTime CalcStartTime(const NFmiMetEditorModeDataWCTR::TimeSectionData &theStartSection, const NFmiMetTime &theCurrentTime)
{
	NFmiMetTime startTime(theCurrentTime);
	startTime.SetTimeStep(theStartSection.itsStartTimeResolutionInMinutes);
	if(startTime > theCurrentTime)
		startTime.PreviousMetTime();
	if(theStartSection.itsStartTimeResolutionInMinutes <= 60)
	{
		startTime.PreviousMetTime(); // laitetaan koko jutun (1. osion 1h aikaresoluutio-alue) aloitusaika aina niin, ett‰ se on yhden tunnin ennen currenttia aikaa
	}
	return startTime;
}

static NFmiMetTime CalcEndTime(const NFmiMetEditorModeDataWCTR::TimeSectionData &theEndSection, const NFmiMetTime &theStartTime, int theSuggestedTotalTime)
{
	NFmiMetTime endTime(theStartTime);
	endTime.ChangeByHours(theSuggestedTotalTime);
	endTime.SetTimeStep(theEndSection.itsStartTimeResolutionInMinutes);
	if(endTime.DifferenceInHours(theStartTime) < theSuggestedTotalTime)
		endTime.NearestMetTime();
	return endTime;
}

NFmiMetEditorModeDataWCTR::NFmiMetEditorModeDataWCTR(void)
:itsEditorMode(kNormal)
,itsTimeDescriptor(0)
,itsMaximalCoverageTimeBag()
,itsTimeSections()
,itsWantedDataLengthInHours(0)
,itsRealDataLengthInHours(0)
,fUseNormalModeForAWhile(false)
,fInNormalModeStillInDataLoadDialog(false)
{
}

NFmiMetEditorModeDataWCTR::NFmiMetEditorModeDataWCTR(const NFmiMetEditorModeDataWCTR &theOther)
:itsEditorMode(theOther.itsEditorMode)
,itsTimeDescriptor(theOther.itsTimeDescriptor ? new NFmiTimeDescriptor(*theOther.itsTimeDescriptor) : 0)
,itsMaximalCoverageTimeBag(theOther.itsMaximalCoverageTimeBag)
,itsTimeSections(theOther.itsTimeSections)
,itsWantedDataLengthInHours(theOther.itsWantedDataLengthInHours)
,itsRealDataLengthInHours(theOther.itsRealDataLengthInHours)
,fUseNormalModeForAWhile(theOther.fUseNormalModeForAWhile)
,fInNormalModeStillInDataLoadDialog(theOther.fInNormalModeStillInDataLoadDialog)
{
}

NFmiMetEditorModeDataWCTR::~NFmiMetEditorModeDataWCTR(void)
{
	Clear();
}

void NFmiMetEditorModeDataWCTR::Clear(void)
{
	std::vector<TimeSectionData>().swap(itsTimeSections); // vectorin 'guru'-tyhjennys temppu
	delete itsTimeDescriptor;
	itsTimeDescriptor = 0;
	itsWantedDataLengthInHours = 0;
	itsRealDataLengthInHours = 0;
}

#define CONFIG_DATALOADINGINFO_MODEDATA_EDITORMODE "MetEditor::DataLoadingInfo::ModeData::EditorMode"
#define CONFIG_DATALOADINGINFO_MODEDATA_TIMESECTION_STARTTIMERES "MetEditor::DataLoadingInfo::ModeData::TimeSection::%d::StartTimeRes"
#define CONFIG_DATALOADINGINFO_MODEDATA_TIMESECTION_TIMERES "MetEditor::DataLoadingInfo::ModeData::TimeSection::%d::TimeRes"
#define CONFIG_DATALOADINGINFO_MODEDATA_TIMESECTION_SUGGESTEDLEN "MetEditor::DataLoadingInfo::ModeData::TimeSection::%d::SuggestedLen"
#define CONFIG_DATALOADINGINFO_MODEDATA_TIMESECTION_ROUNDINGRULE "MetEditor::DataLoadingInfo::ModeData::TimeSection::%d::RoundingRule"

#pragma warning( push )
#pragma warning( disable : 4996 )

void NFmiMetEditorModeDataWCTR::Configure()  {
	Clear();

	int mode = NFmiSettings::Require<int>(CONFIG_DATALOADINGINFO_MODEDATA_EDITORMODE);
	itsEditorMode = static_cast<Mode>(mode);

	int i = 1;
	char roundingRuleKey[128];
	char startTimeResKey[128];
	char timeResKey[128];
	char suggestedLenKey[128];
	
	TimeSectionData tmpData;

	sprintf(roundingRuleKey, CONFIG_DATALOADINGINFO_MODEDATA_TIMESECTION_ROUNDINGRULE, i);
	sprintf(startTimeResKey, CONFIG_DATALOADINGINFO_MODEDATA_TIMESECTION_STARTTIMERES, i);
	sprintf(timeResKey, CONFIG_DATALOADINGINFO_MODEDATA_TIMESECTION_TIMERES, i);
	sprintf(suggestedLenKey, CONFIG_DATALOADINGINFO_MODEDATA_TIMESECTION_SUGGESTEDLEN, i);
	for (; NFmiSettings::IsSet(startTimeResKey); i++, 
		sprintf(roundingRuleKey, CONFIG_DATALOADINGINFO_MODEDATA_TIMESECTION_ROUNDINGRULE, i),
		sprintf(startTimeResKey, CONFIG_DATALOADINGINFO_MODEDATA_TIMESECTION_STARTTIMERES, i),
		sprintf(timeResKey, CONFIG_DATALOADINGINFO_MODEDATA_TIMESECTION_TIMERES, i),
		sprintf(suggestedLenKey, CONFIG_DATALOADINGINFO_MODEDATA_TIMESECTION_SUGGESTEDLEN, i) )
	{
		tmpData.itsStartTimeResolutionInMinutes = NFmiSettings::Require<int>(startTimeResKey);
		tmpData.itsTimeResolutionInMinutes = NFmiSettings::Require<int>(timeResKey);
		tmpData.itsSuggestedSectionLengthInHours = NFmiSettings::Require<int>(suggestedLenKey);
		tmpData.itsRoundingRule = static_cast<TimeSectionRoundDirection>(NFmiSettings::Require<int>(roundingRuleKey));
		
		itsTimeSections.push_back(tmpData);
	}
}

// Undo the above warning disabling.
#pragma warning( pop )

NFmiMetEditorModeDataWCTR::Mode NFmiMetEditorModeDataWCTR::EditorMode(void) const
{
	if(fUseNormalModeForAWhile)
		return kNormal;
	return itsEditorMode;
}

// HUOM!!! t‰m‰ mene mets‰‰n jos ei ole alustettu!!!!
const NFmiTimeDescriptor& NFmiMetEditorModeDataWCTR::TimeDescriptor(void)
{
	return *itsTimeDescriptor;
}

const NFmiTimeDescriptor& NFmiMetEditorModeDataWCTR::TimeDescriptor(const NFmiMetTime &theCurrentTime, const NFmiMetTime &theOriginTime, bool fMakeUpdate)
{
	if(fMakeUpdate)
	{
		if(itsEditorMode == kOperativeNormal)
			CreateTimeBagSystem(theCurrentTime, theOriginTime);
		else if(itsEditorMode == kOperativeWCTR)
			CreateTimeListSystem(theCurrentTime, theOriginTime);
		itsRealDataLengthInHours = itsWantedDataLengthInHours; // t‰m‰ pit‰‰ korjata, jos real-arvoa tosiaan tarvitaan
		MakeMaximalCoverageTimeBag();
	}
	return *itsTimeDescriptor;
}

bool NFmiMetEditorModeDataWCTR::CreateTimeListSystem(const NFmiMetTime &theCurrentTime, const NFmiMetTime &theOriginTime)
{
	std::vector<NFmiMetTime> borderTimes(CalcSectionBorderTimes(theCurrentTime));

	if(borderTimes.size() > 1)
	{
		NFmiTimeList timeList;
		NFmiMetTime startTime;
		NFmiMetTime endTime;
		NFmiMetTime tmpTime;
		for(int i=0; i < static_cast<int>(borderTimes.size() - 1); i++)
		{
			startTime = borderTimes[i];
			startTime.SetTimeStep(itsTimeSections[i].itsTimeResolutionInMinutes);
			tmpTime = startTime;
			endTime = borderTimes[i+1];
			if(i==0) // laitetaan vain 1. kerralla starttime listaan
				timeList.Add(new NFmiMetTime(tmpTime));
			do
			{
				tmpTime.NextMetTime();
				timeList.Add(new NFmiMetTime(tmpTime));
			}while(tmpTime < endTime);
		}
		delete itsTimeDescriptor;
		itsTimeDescriptor = new NFmiTimeDescriptor(theOriginTime, timeList);
		return true;
	}
	return false;
}

// timebag systeemiss‰ katsotaan vain 1. aikasectionia ja tehd‰‰n sen mukainen timebagi
bool NFmiMetEditorModeDataWCTR::CreateTimeBagSystem(const NFmiMetTime &theCurrentTime, const NFmiMetTime &theOriginTime)
{
	std::vector<NFmiMetTime> borderTimes(CalcSectionBorderTimes(theCurrentTime));
	if(borderTimes.size() > 1)
	{
		NFmiTimeBag times(borderTimes[0], borderTimes[1], itsTimeSections[0].itsTimeResolutionInMinutes);
		delete itsTimeDescriptor;
		itsTimeDescriptor = new NFmiTimeDescriptor(theOriginTime, times);
		return true;
	}
	return false;
}

std::vector<NFmiMetTime> NFmiMetEditorModeDataWCTR::CalcSectionBorderTimes(const NFmiMetTime &theCurrentTime)
{
	std::vector<NFmiMetTime> borderTimes;
	if(itsTimeSections.size() > 0)
	{
		int totalSuggestedTime = CalcTotalSuggestedTime(itsTimeSections);
		itsWantedDataLengthInHours = totalSuggestedTime;

		NFmiMetTime startTime(CalcStartTime(itsTimeSections[0], theCurrentTime));
		NFmiMetTime endTime(CalcEndTime(itsTimeSections[itsTimeSections.size() - 1], theCurrentTime, totalSuggestedTime));

		if(itsTimeSections.size() == 1 || itsEditorMode != kOperativeWCTR)
		{
			borderTimes.push_back(startTime);
			borderTimes.push_back(endTime);
		}
		else
		{
			borderTimes.push_back(endTime);
			NFmiMetTime sectionEndTime(endTime);
			for(int i = static_cast<int>(itsTimeSections.size() - 1); i > 0; i--)
			{
				NFmiMetTime sectionStartTime(sectionEndTime);
				int realSectionLength = 0;
				double steps = itsTimeSections[i].itsSuggestedSectionLengthInHours / (itsTimeSections[i].itsTimeResolutionInMinutes/60.);
				if(::fmod(steps, 1) == 0)
					realSectionLength = itsTimeSections[i].itsSuggestedSectionLengthInHours*60;
				else
					realSectionLength = boost::math::iround(steps)*itsTimeSections[i].itsTimeResolutionInMinutes;
				sectionStartTime.ChangeByMinutes(-realSectionLength);
				borderTimes.push_back(sectionStartTime);
				sectionEndTime = sectionStartTime; 
			}
			borderTimes.push_back(startTime);
			// lopuksi pit‰‰ ajat k‰‰nt‰‰ vectorissa, k‰yt‰n siihen apuvectoria
			std::vector<NFmiMetTime> tmpTimes(borderTimes.size()); 
			std::copy(borderTimes.rbegin(), borderTimes.rend(), tmpTimes.begin());
			borderTimes.swap(tmpTimes);
		}
	}
	
	return borderTimes;
}

bool NFmiMetEditorModeDataWCTR::MakeMaximalCoverageTimeBag(void)
{
	if(itsTimeDescriptor)
	{
		if(itsEditorMode == kOperativeWCTR)
		{
			itsTimeDescriptor->Time(0);
			NFmiMetTime firstTime(itsTimeDescriptor->ValidTime());
			itsTimeDescriptor->Time(itsTimeDescriptor->Size() - 1);
			NFmiMetTime lastTime(itsTimeDescriptor->ValidTime());
			itsMaximalCoverageTimeBag = NFmiTimeBag(firstTime, lastTime, firstTime.GetTimeStep());
			return true;
		}
		else
		{
			itsMaximalCoverageTimeBag = *itsTimeDescriptor->ValidTimeBag();
			return true;
		}
	}
	return false;
}
