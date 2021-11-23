#pragma once

#include "NFmiPoint.h"
#include "NFmiDataMatrix.h"
#include <list>

class SparseData
{
	float value_ = kFloatMissing;
	NFmiPoint drawGridPoint_ = NFmiPoint::gMissingLatlon;
	NFmiPoint peekIndex_ = NFmiPoint::gMissingLatlon;
	unsigned long visualizedLocationIndex_ = gMissingIndex;
	bool insideZoomedArea_ = false;
public:
	SparseData();
	SparseData(float value, const NFmiPoint& drawGridPoint, const NFmiPoint &peekIndex, unsigned long visualizedLocationIndex);
	
	void clearData();
	bool hasValue() const;
	bool peekIndexIsCloserToCenter(const SparseData& otherData) const;
	double calcPeekDistance() const;
	FmiDirection peekIndexDirection() const;
	const NFmiPoint& peekIndex() const { return peekIndex_; }
	float value() const { return value_; }
	unsigned long visualizedLocationIndex() const { return visualizedLocationIndex_; }
	bool insideZoomedArea() const { return insideZoomedArea_; }
	static FmiDirection horizontalElimination(const SparseData &leftData, const SparseData &rightData);
	static FmiDirection verticalElimination(const SparseData& lowerData, const SparseData& upperData);
	static double drawGridPointDistance(const SparseData &data1, const SparseData &data2);
};

class SparseDataGrid
{
	NFmiDataMatrix<SparseData> sparseDataMatrix_;
public:
	SparseDataGrid();
	SparseDataGrid(int xSize, int ySize);
	
	void setData(int xIndex, int yIndex, const SparseData &sparseData);
	void cleanTooCloseHits();
	const NFmiDataMatrix<SparseData>& sparseDataMatrix() const { return sparseDataMatrix_; }
	int calcNonMissingValues() const;
};
