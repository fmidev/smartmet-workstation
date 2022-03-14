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
	// T�m� on world-xy maailman kartalla pohjoisessa oleva laskettu yl�raja (eri p�in kuin NFmiRect:iss�)
	double boundingTop_ = std::numeric_limits<double>::lowest();
	// T�m� on world-xy maailman kartalla etel�ss� oleva laskettu alaraja (eri p�in kuin NFmiRect:iss�)
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
