#include "SparseDataGrid.h"
#include "CtrlViewFunctions.h"

SparseData::SparseData() = default;

SparseData::SparseData(float value, const NFmiPoint & drawGridPoint, const std::list<NFmiPoint>& peekIndexList, unsigned long visualizedLocationIndex)
:value_(value)
, drawGridPoint_(drawGridPoint)
,peekIndexList_(peekIndexList)
,visualizedLocationIndex_(visualizedLocationIndex)
,insideZoomedArea_(true) // Jos käytetty tätä konstruktoria, laitetaan tämä true:ksi
{
}

void SparseData::clearData()
{
	*this = SparseData();
}

bool SparseData::hasValue() const
{
	return value_ != kFloatMissing;
}

double SparseData::calcPeekDistance() const
{
	const auto& firstPeekIndex = peekIndexList_.front();
	auto x = firstPeekIndex.X();
	auto y = firstPeekIndex.Y();
	return std::sqrt(x * x + y * y);
}

bool SparseData::peekIndexIsCloserToCenter(const SparseData& otherData) const
{
	auto distance1 = calcPeekDistance();
	auto distance2 = otherData.calcPeekDistance();
	return distance1 < distance2;
}

FmiDirection SparseData::calcPeekIndexDirection(const NFmiPoint& peekIndex)
{
	auto peekIndexAngle = CtrlViewUtils::CalcAngle(peekIndex.X(), peekIndex.Y());
	if(peekIndexAngle >= 45 && peekIndexAngle <= 135)
		return kRight;
	if(peekIndexAngle >= 135 && peekIndexAngle <= 225)
		return kDown;
	if(peekIndexAngle >= 225 && peekIndexAngle <= 315)
		return kLeft;
	return kUp;
}

NFmiPoint SparseData::seekPeekIndexFromDirection(FmiDirection wantedDirection) const
{
	for(const auto& peekIndex : peekIndexList_)
	{
		if(wantedDirection == calcPeekIndexDirection(peekIndex))
			return peekIndex;
	}
	return NFmiPoint::gMissingLatlon;
}

static FmiDirection doFinalElimination(double totalDistance, double directionalPeekDistance, FmiDirection eliminationDirection)
{
	// Eliminoidaan arvo, jos se on n. 70 % etäisyydellä piirrettävien hilojen rajalta (totalDistance on 2x matka rajalle).
	// Etäisyystarkastelu pitää vielä tehdä vain x-suunnassa, jotta etäisyys vastaa total-etäisyyteen
	if(std::fabs(directionalPeekDistance) >= totalDistance * 0.2)
		return eliminationDirection;
	return kNoDirection;
}

FmiDirection SparseData::horizontalElimination(const SparseData& leftData, const SparseData& rightData)
{
	if(leftData.hasValue() && rightData.hasValue())
	{
		auto totalDistance = drawGridPointDistance(leftData, rightData);
		if(leftData.peekIndexIsCloserToCenter(rightData))
		{
			auto peekIndex = rightData.seekPeekIndexFromDirection(kLeft);
			if(peekIndex != NFmiPoint::gMissingLatlon)
				return ::doFinalElimination(totalDistance, peekIndex.X(), kRight);
		}
		else
		{
			auto peekIndex = leftData.seekPeekIndexFromDirection(kRight);
			if(peekIndex != NFmiPoint::gMissingLatlon)
				return ::doFinalElimination(totalDistance, peekIndex.X(), kLeft);
		}
	}
	return kNoDirection;
}

FmiDirection SparseData::verticalElimination(const SparseData& lowerData, const SparseData& upperData)
{
	if(lowerData.hasValue() && upperData.hasValue())
	{
		auto totalDistance = drawGridPointDistance(lowerData, upperData);
		if(lowerData.peekIndexIsCloserToCenter(upperData))
		{
			auto peekIndex = upperData.seekPeekIndexFromDirection(kDown);
			if(peekIndex != NFmiPoint::gMissingLatlon)
				return ::doFinalElimination(totalDistance, peekIndex.Y(), kUp);
		}
		else
		{
			auto peekIndex = lowerData.seekPeekIndexFromDirection(kUp);
			if(peekIndex != NFmiPoint::gMissingLatlon)
				return ::doFinalElimination(totalDistance, peekIndex.X(), kDown);
		}
	}
	return kNoDirection;
}

double SparseData::drawGridPointDistance(const SparseData& data1, const SparseData& data2)
{
	auto xDiff = data1.drawGridPoint_.X() - data2.drawGridPoint_.X();
	auto yDiff = data1.drawGridPoint_.Y() - data2.drawGridPoint_.Y();
	return std::sqrt(xDiff * xDiff + yDiff * yDiff);
}


SparseDataGrid::SparseDataGrid() = default;

SparseDataGrid::SparseDataGrid(int xSize, int ySize)
:sparseDataMatrix_(xSize, ySize)
{
}
	
void SparseDataGrid::setData(int xIndex, int yIndex, const SparseData &sparseData)
{
	if(xIndex >= 0 && xIndex < sparseDataMatrix_.NX() && yIndex >= 0 && yIndex < sparseDataMatrix_.NY())
	{
		sparseDataMatrix_[xIndex][yIndex] = sparseData;
	}
	else
	{
		int x = 0;
	}
}

void SparseDataGrid::cleanTooCloseHits()
{
	for(size_t yIndex = 0; yIndex < sparseDataMatrix_.NY() - 1; yIndex++)
	{
		for(size_t xIndex = 0; xIndex < sparseDataMatrix_.NX() - 1; xIndex++)
		{
			auto& sparseData = sparseDataMatrix_[xIndex][yIndex];
			auto& sparseDataRight = sparseDataMatrix_[xIndex + 1][yIndex];
			auto& sparseDataUp = sparseDataMatrix_[xIndex][yIndex + 1];
			auto horizontalClean = SparseData::horizontalElimination(sparseData, sparseDataRight);
			if(horizontalClean == kLeft)
				sparseData.clearData();
			if(horizontalClean == kRight)
				sparseDataRight.clearData();
			auto verticalClean = SparseData::verticalElimination(sparseData, sparseDataUp);
			if(verticalClean == kDown)
				sparseData.clearData();
			if(verticalClean == kUp)
				sparseDataUp.clearData();
		}
	}
}

int SparseDataGrid::calcNonMissingValues() const
{
	int nonMissingValueCounter = 0;
	for(size_t yIndex = 0; yIndex < sparseDataMatrix_.NY(); yIndex++)
	{
		for(size_t xIndex = 0; xIndex < sparseDataMatrix_.NX(); xIndex++)
		{
			if(sparseDataMatrix_[xIndex][yIndex].hasValue())
				nonMissingValueCounter++;
		}
	}
	return nonMissingValueCounter;
}
