#include "NFmiVirtualTimeData.h"

const NFmiColor NFmiVirtualTimeData::virtualTimeBaseColor(0.608f, 0.188f, 1.f);

NFmiVirtualTimeData::NFmiVirtualTimeData() = default;

void NFmiVirtualTimeData::ToggleVirtualTimeMode(bool caseStudyModeOn, const NFmiMetTime& caseStudyWallClockTime)
{
    fVirtualTimeUsed = !fVirtualTimeUsed;

    if(caseStudyModeOn)
    {
        UpdateCaseStudyVirtualTime(caseStudyWallClockTime);
        return;
    }

    UpdateNormalVirtualTime();
}

void NFmiVirtualTimeData::UpdateCaseStudyVirtualTime(const NFmiMetTime& caseStudyWallClockTime)
{
    if(!fCaseStudyVirtualTimeSet)
    {
        itsCaseStudyVirtualTime = caseStudyWallClockTime;
        fCaseStudyVirtualTimeSet = true;
    }
}

void NFmiVirtualTimeData::UpdateNormalVirtualTime()
{
    if(!fNormalVirtualTimeSet)
    {
        // Halutaan ett� 1. asetettava virtuaali aika on sein�kellosta seuraava tasatuntiaika
        NFmiMetTime usedVirtualTime(1);
        usedVirtualTime.NearestMetTime(60, kForward);
        itsNormalVirtualTime = usedVirtualTime;
        fNormalVirtualTimeSet = true;
        return;
    }
}

// T�t� metodia on tarkoitus kutsua kun menn��n CaseStudy moodiin tai kun tullaan sielt� pois.
// Jos t�m� tehd��n kun smartmet on virtual-time moodi p��ll�, pit�� nomaali ja caseStudy aikoja mahdollisesti p�ivitt��.
void NFmiVirtualTimeData::UpdateUsedVirtualTime(bool caseStudyModeOn, const NFmiMetTime& caseStudyWallClockTime)
{
    if(!fVirtualTimeUsed)
        return;

    if(caseStudyModeOn)
    {
        fCaseStudyVirtualTimeSet = false;
        UpdateCaseStudyVirtualTime(caseStudyWallClockTime);
        return;
    }

    UpdateNormalVirtualTime();
}

void NFmiVirtualTimeData::ClearCaseStudyVirtualTime()
{
    itsCaseStudyVirtualTime = NFmiMetTime::gMissingTime;
    fCaseStudyVirtualTimeSet = false;
}

bool NFmiVirtualTimeData::SignificantVirtualTimeChangeHappened(const NFmiMetTime& origVirtualTime, const NFmiMetTime& newVirtualTime)
{
    return fVirtualTimeUsed && (origVirtualTime != newVirtualTime);
}

bool NFmiVirtualTimeData::VirtualTime(const NFmiMetTime& virtualTime, bool caseStudyModeOn)
{
    bool virtualTimeChanged = false;
    if(caseStudyModeOn)
    {
        virtualTimeChanged = SignificantVirtualTimeChangeHappened(itsCaseStudyVirtualTime,virtualTime);
        itsCaseStudyVirtualTime = virtualTime;
        fCaseStudyVirtualTimeSet = true;
        return virtualTimeChanged;
    }

    virtualTimeChanged = SignificantVirtualTimeChangeHappened(itsNormalVirtualTime, virtualTime);
    itsNormalVirtualTime = virtualTime;
    fNormalVirtualTimeSet = true;
    return virtualTimeChanged;
}

std::string NFmiVirtualTimeData::GetVirtualTimeTooltipText(bool caseStudyModeOn) const
{
    std::string str = "<b><font color=purple>Virtual-Time mode (toggle CTRL+K):</font>\n";
    str += "<font color=purple>VT = ";
    const auto& usedTime = caseStudyModeOn ? itsCaseStudyVirtualTime : itsNormalVirtualTime;
    str += usedTime.ToStr("Www YYYY.MM.DD HH:mm [utc]", kEnglish);
    str += "</font>\n";
    str += "<font color=purple>Change VT left click purple bar in time control</font></b>\n";
    str += "<hr color=red><br>";
    return str;
}
