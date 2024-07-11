#pragma once

#include "NFmiMetTime.h"
#include "NFmiColor.h"

// Luokka joka tiet‰‰ Smartmetin virtual-time moodit ja virtual-time:t 
// normitilalle ja caseStudy tapauksille. Lis‰ksi tiet‰‰ onko virtuaali 
// ajat jo asetettu, vai pit‰‰kˆ se asettaa 1. kerran.
class NFmiVirtualTimeData
{
	bool fVirtualTimeUsed = false;
	NFmiMetTime itsNormalVirtualTime = NFmiMetTime::gMissingTime;
	bool fNormalVirtualTimeSet = false;
	NFmiMetTime itsCaseStudyVirtualTime = NFmiMetTime::gMissingTime;
	bool fCaseStudyVirtualTimeSet = false;
public:
	NFmiVirtualTimeData();

	void ToggleVirtualTimeMode(bool caseStudyModeOn, const NFmiMetTime &caseStudyWallClockTime);
	void ClearCaseStudyVirtualTime();
	bool VirtualTimeUsed() const { return fVirtualTimeUsed; }
	const NFmiMetTime& NormalVirtualTime() const { return itsNormalVirtualTime; }
	const NFmiMetTime& CaseStudyVirtualTime() const { return itsCaseStudyVirtualTime; }
	void VirtualTime(const NFmiMetTime& virtualTime, bool caseStudyModeOn);
	std::string GetVirtualTimeTooltipText(bool caseStudyModeOn) const;

	static const NFmiColor virtualTimeBaseColor;
};
