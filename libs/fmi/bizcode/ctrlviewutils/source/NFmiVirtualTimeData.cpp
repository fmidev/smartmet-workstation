#include "NFmiVirtualTimeData.h"

const NFmiColor NFmiVirtualTimeData::virtualTimeBaseColor(0.608f, 0.188f, 1.f);

NFmiVirtualTimeData::NFmiVirtualTimeData() = default;

void NFmiVirtualTimeData::ToggleVirtualTimeMode(bool caseStudyModeOn, const NFmiMetTime& caseStudyWallClockTime)
{
    fVirtualTimeUsed = !fVirtualTimeUsed;

    if(caseStudyModeOn)
    {
        if(!fCaseStudyVirtualTimeSet)
        {
            itsCaseStudyVirtualTime = caseStudyWallClockTime;
            fCaseStudyVirtualTimeSet = true;
        }
        return;
    }

    if(!fNormalVirtualTimeSet)
    {
        // Halutaan että 1. asetettava virtuaali aika on seinäkellosta seuraava tasatuntiaika
        NFmiMetTime usedVirtualTime(1);
        usedVirtualTime.NearestMetTime(60, kForward);
        itsNormalVirtualTime = usedVirtualTime;
        fNormalVirtualTimeSet = true;
        return;
    }
}

void NFmiVirtualTimeData::ClearCaseStudyVirtualTime()
{
    itsCaseStudyVirtualTime = NFmiMetTime::gMissingTime;
    fCaseStudyVirtualTimeSet = false;
}

void NFmiVirtualTimeData::VirtualTime(const NFmiMetTime& virtualTime, bool caseStudyModeOn)
{
    if(caseStudyModeOn)
    {
        itsCaseStudyVirtualTime = virtualTime;
        fCaseStudyVirtualTimeSet = true;
        return;
    }

    itsNormalVirtualTime = virtualTime;
    fNormalVirtualTimeSet = true;
}
