#pragma once

#include "NFmiRect.h"

#ifdef max
#undef max
#undef min
#endif


class NFmiArea;
class NFmiFastQueryInfo;

class NFmiWorldXyBoundingBoxCalculator
{
	NFmiRect mapWorldXyRect_;
	double boundingLeft_ = std::numeric_limits<double>::max();
	double boundingRight_ = std::numeric_limits<double>::lowest();
	// Tämä on world-xy maailman kartalla pohjoisessa oleva laskettu yläraja (eri päin kuin NFmiRect:issä)
	double boundingTop_ = std::numeric_limits<double>::lowest();
	// Tämä on world-xy maailman kartalla etelässä oleva laskettu alaraja (eri päin kuin NFmiRect:issä)
	double boundingBottom_ = std::numeric_limits<double>::max();

public:
	NFmiWorldXyBoundingBoxCalculator();

	static NFmiRect calcDataWorldXyBoundingBoxOverMapArea(NFmiFastQueryInfo& fastInfo, NFmiArea& mapArea);
	NFmiRect calcGridDataAreaOverMapAreaWorldXyBoundingBox(NFmiFastQueryInfo& fastInfo, NFmiArea& mapArea);

private:
	void clear();
	bool doBoundingBoxWithSameKindAreas(NFmiFastQueryInfo& fastInfo, NFmiArea& mapArea, NFmiRect& worldXyBoundingBox);
	bool doBoundingBoxWithDataCornerPoints(NFmiFastQueryInfo& fastInfo, NFmiArea& mapArea, NFmiRect& worldXyBoundingBox);
	NFmiRect doBoundingBoxWithDataEdgePoints(NFmiFastQueryInfo& fastInfo, NFmiArea& mapArea);
	void checkXyPoint(const NFmiPoint& worlXyPoint);
};
