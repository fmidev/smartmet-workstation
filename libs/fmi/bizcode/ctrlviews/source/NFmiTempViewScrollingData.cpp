#include "NFmiTempViewScrollingData.h"

SoundingInformation::SoundingInformation() = default;
SoundingInformation::SoundingInformation(const SoundingInformation&) = default;
SoundingInformation::~SoundingInformation() = default;

SoundingInformation::SoundingInformation(const NFmiMetTime &soundingTime, const NFmiMetTime &dataOriginTime, const NFmiPoint &soundingLatlon, const NFmiMTATempSystem::SoundingProducer &producer)
		:soundingTime_(soundingTime)
		,dataOriginTime_(dataOriginTime)
		,soundingLatlon_(soundingLatlon)
		,producer_(producer)
{}

bool SoundingInformation::operator==(const SoundingInformation& other)
{
	return soundingTime_ == other.soundingTime_ && dataOriginTime_ == other.dataOriginTime_ && soundingLatlon_ == other.soundingLatlon_ && producer_ == other.producer_;
}

bool SoundingInformation::operator!=(const SoundingInformation& other)
{
	return !operator==(other);
}



NFmiTempViewScrollingData::NFmiTempViewScrollingData() = default;
NFmiTempViewScrollingData::~NFmiTempViewScrollingData() = default;

bool NFmiTempViewScrollingData::isSoundingChanged(const SoundingInformation& newSoundingInfo)
{
	auto hasChanged = lastSoundingInfo_ != newSoundingInfo;
	if(hasChanged)
	{
		lastSoundingInfo_ = newSoundingInfo;
		relativeScrollingIndex_ = 0;
	}
	return hasChanged;
}

bool NFmiTempViewScrollingData::doScrolling(int scrollChange, bool drawUpwardSounding)
{
	int soundingCount = lastTotalSoundingRowCount_;
	int fullRowCount = lastFullRowCount_;
	if(soundingCount == 0 || fullRowCount == 0)
		return false;

	int originalRelativeScrollingIndex = relativeScrollingIndex_;
	relativeScrollingIndex_ += scrollChange;
	if(drawUpwardSounding)
	{
		if(relativeScrollingIndex_ > 0)
			relativeScrollingIndex_ = 0;
		else
		{
			if(fullRowCount >= soundingCount)
				relativeScrollingIndex_ = 0;
			else
			{
				// relativeScrollingIndex_ on negatiivinen luku, joten pitää laittaa '-' merkki yhteenlaskuun if:issa
				if(fullRowCount - relativeScrollingIndex_ > soundingCount)
				{
					relativeScrollingIndex_ = -(soundingCount - fullRowCount);
				}
			}
		}
	}
	else
	{
		if(relativeScrollingIndex_ < 0)
			relativeScrollingIndex_ = 0;
		else
		{
			if(fullRowCount >= soundingCount)
				relativeScrollingIndex_ = 0;
			else
			{
				if(fullRowCount + relativeScrollingIndex_ > soundingCount)
				{
					relativeScrollingIndex_ = soundingCount - fullRowCount;
				}
			}
		}
	}
	return originalRelativeScrollingIndex != relativeScrollingIndex_;
}

int NFmiTempViewScrollingData::calcActualScrollingIndex(int soundingCount, int fullRowCount, bool drawUpwardSounding)
{
	lastTotalSoundingRowCount_ = soundingCount;
	lastFullRowCount_ = fullRowCount;
	// Varmistetaan että relativeScrollingIndex_ on oikeissa rajoissa tekemällä sille 0 muutos
	doScrolling(0, drawUpwardSounding);

	if(fullRowCount >= soundingCount)
		return 0;

	if(drawUpwardSounding)
	{
		return soundingCount - fullRowCount + relativeScrollingIndex_;
	}
	else
	{
		return relativeScrollingIndex_;
	}
}

void NFmiTempViewScrollingData::resetLastTextualSideViewValues() 
{ 
	lastTotalSoundingRowCount_ = 0; 
	lastFullRowCount_ = 0; 
}

NFmiRect NFmiTempViewScrollingData::calcScrollAreaRect(double scrollAreaRelativeTop, double scrollAreaRelativeWidth, const NFmiRect& textualRect, int totalSoundingRows, int fullVisibleRows, int startingRowIndex)
{
	auto right = textualRect.Right();
	auto left = right - scrollAreaRelativeWidth;
	auto totalScrollAreaHeight = textualRect.Bottom() - scrollAreaRelativeTop;
	auto topFactor = double(startingRowIndex) / totalSoundingRows;
	auto top = scrollAreaRelativeTop + (topFactor * totalScrollAreaHeight);
	auto heightFactor = double(fullVisibleRows) / totalSoundingRows;
	auto bottom = top + (heightFactor * totalScrollAreaHeight);
	return NFmiRect(left, top, right, bottom);
}
