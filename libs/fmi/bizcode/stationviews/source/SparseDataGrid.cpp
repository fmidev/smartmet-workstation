#include "SparseDataGrid.h"
#include "CtrlViewFunctions.h"

SparseData::SparseData() = default;

SparseData::SparseData(float value, const NFmiPoint & drawGridPoint, const NFmiPoint& peekIndex, unsigned long visualizedLocationIndex)
:value_(value)
, drawGridPoint_(drawGridPoint)
,peekIndex_(peekIndex)
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
	auto x = peekIndex_.X();
	auto y = peekIndex_.Y();
	return std::sqrt(x * x + y * y);
}

bool SparseData::peekIndexIsCloserToCenter(const SparseData& otherData) const
{
	auto distance1 = calcPeekDistance();
	auto distance2 = otherData.calcPeekDistance();
	return distance1 < distance2;
}

FmiDirection SparseData::peekIndexDirection() const
{
	auto peekIndexAngle = CtrlViewUtils::CalcAngle(peekIndex_.X(), peekIndex_.Y());
	if(peekIndexAngle >= 45 && peekIndexAngle <= 135)
		return kRight;
	if(peekIndexAngle >= 135 && peekIndexAngle <= 225)
		return kDown;
	if(peekIndexAngle >= 225 && peekIndexAngle <= 315)
		return kLeft;
	return kUp;
}

static FmiDirection doFinalElimination(double totalDistance, double directionalPeekDistance, FmiDirection eliminationDirection)
{
	// Eliminoidaan arvo, jos se on n. 70 % etäisyydellä piirrettävien hilojen rajalta (totalDistance on 2x matka rajalle).
	// Etäisyystarkastelu pitää vielä tehdä vain x-suunnassa, jotta etäisyys vastaa total-etäisyyteen
	if(directionalPeekDistance >= totalDistance * 0.2)
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
			auto peekDirection = rightData.peekIndexDirection();
			if(peekDirection == kLeft)
				return ::doFinalElimination(totalDistance, rightData.peekIndex().X(), kRight);
		}
		else
		{
			auto peekDirection = leftData.peekIndexDirection();
			if(peekDirection == kRight)
				return ::doFinalElimination(totalDistance, leftData.peekIndex().X(), kLeft);
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
			auto peekDirection = upperData.peekIndexDirection();
			if(peekDirection == kDown)
				return ::doFinalElimination(totalDistance, upperData.peekIndex().Y(), kUp);
		}
		else
		{
			auto peekDirection = lowerData.peekIndexDirection();
			if(peekDirection == kUp)
				return ::doFinalElimination(totalDistance, lowerData.peekIndex().X(), kDown);
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
