#pragma once

#include "NFmiMetTime.h"
#include "NFmiPoint.h"
#include "NFmiMTATempSystem.h"

class SoundingInformation
{
	NFmiMetTime soundingTime_ = NFmiMetTime::gMissingTime;
	NFmiMetTime dataOriginTime_ = NFmiMetTime::gMissingTime;
	NFmiPoint soundingLatlon_ = NFmiPoint::gMissingLatlon;
	NFmiMTATempSystem::SoundingProducer producer_;
public:
	SoundingInformation();
	SoundingInformation(const SoundingInformation&);
	~SoundingInformation();
	SoundingInformation(const NFmiMetTime& soundingTime, const NFmiMetTime& dataOriginTime, const NFmiPoint& soundingLatlon, const NFmiMTATempSystem::SoundingProducer& producer);

	bool operator==(const SoundingInformation& other);
	bool operator!=(const SoundingInformation& other);
};

class NFmiTempViewScrollingData
{
	int relativeScrollingIndex_ = 0;
	SoundingInformation lastSoundingInfo_;
	// Kun luotausn�ytt�� piirret��n, t�nne laitetaan talteen viimeisimm�n piirron tekstisivuikkunan arvot.
	// N�m� pit�� my�s nollata, jos luotausn�ytt� piirret��n ilman kyseist� sivuikkunaa.
	int lastTotalSoundingRowCount_ = 0;
	int lastFullRowCount_ = 0;
public:
	NFmiTempViewScrollingData();
	~NFmiTempViewScrollingData();

	void resetRelativeScrollingIndex() { relativeScrollingIndex_ = 0; }
	void resetLastTextualSideViewValues();

	bool isSoundingChanged(const SoundingInformation& newSoundingInfo);
	bool doScrolling(int scrollChange, bool drawUpwardSounding);
	int calcActualScrollingIndex(int soundingCount, int fullRowCount, bool drawUpwardSounding);
	NFmiRect calcScrollAreaRect(double scrollAreaRelativeTop, double scrollAreaRelativeWidth, const NFmiRect &textualRect, int totalSoundingRows, int fullVisibleRows, int startingRowIndex);
};
