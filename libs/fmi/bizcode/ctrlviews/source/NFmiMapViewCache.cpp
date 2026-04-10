// ======================================================================
/*!
 * \file NFmiMapViewCache.cpp
 * \brief Implementation of class NFmiMapViewCache
 */
// ======================================================================

#include <algorithm>
#include <functional>

#include "NFmiMapViewCache.h"

#ifndef UNIX
#include "stdafx.h"
#endif // UNIX

#ifndef UNIX
static double CalcBitmapSizeInMB(CBitmap* theBitmap)
{
	const static double bytesInMB = 1024 * 1024;
	if(theBitmap)
	{
		BITMAP theBMInfo;
		int status = theBitmap->GetObject(sizeof(BITMAP), &theBMInfo);
		if(status)
		{
			int totalBytes = theBMInfo.bmHeight * theBMInfo.bmWidthBytes;
			return totalBytes/bytesInMB;
		}
	}
	return 0;
}

NFmiMapViewCache::CacheSlot::CacheSlot(void)
:itsMapImage(0)
,itsTime()
,itsSizeInMB(0)
,itsDrawTime() // otetaan sein�kello aika vain piirtoajaksi
{
}

NFmiMapViewCache::CacheSlot::CacheSlot(const NFmiMetTime &theTime, CBitmap* theMapImage)
:itsMapImage(theMapImage)
,itsTime(theTime)
,itsSizeInMB(0)
,itsDrawTime() // otetaan sein�kello aika vain piirtoajaksi (t�m� on se metodi, mill� annetaan kuva cacheen)
{
	itsSizeInMB = CalcBitmapSizeInMB(itsMapImage);
}
NFmiMapViewCache::CacheSlot::~CacheSlot(void)
{ // ei tee mitaan tarkoituksella, dynaamisesti varattu bitmap tuhotaan erikseen jos tarpeen
}

void NFmiMapViewCache::CacheSlot::DestroyData(void)
{
	if(itsMapImage)
	{
		itsMapImage->DeleteObject();
		delete itsMapImage;
		itsMapImage = 0;
	}
	itsSizeInMB = 0;
}

NFmiMapViewCache::CacheRow::CacheRow(void)
:itsFifoCache()
{
}

NFmiMapViewCache::CacheRow::~CacheRow(void)
{
	Clear();
}

void NFmiMapViewCache::CacheRow::Clear(void)
{
	std::for_each(itsFifoCache.begin(), itsFifoCache.end(), [&](auto& cacheSlot) { cacheSlot.DestroyData(); });
	itsFifoCache.clear();
}

struct FindImageTime
{
	FindImageTime(const NFmiMetTime &theTime):itsTime(theTime){}
	bool operator()(const NFmiMapViewCache::CacheSlot &theSlot)
	{
		return itsTime == theSlot.itsTime;
	}
	NFmiMetTime itsTime;
};

void NFmiMapViewCache::CacheRow::ClearTime(const NFmiMetTime &theTime)
{
	std::list<CacheSlot>::iterator it = std::find_if(itsFifoCache.begin(), itsFifoCache.end(), FindImageTime(theTime));
	if(it != itsFifoCache.end())
	{
		(*it).DestroyData();
		itsFifoCache.erase(it);
	}
}

void NFmiMapViewCache::CacheRow::MakeTimesDirty(const NFmiMetTime &theMinTime, const NFmiMetTime &theMaxTime)
{
	if(itsFifoCache.size())
	{
		std::list<CacheSlot>::iterator it = itsFifoCache.begin();
		for( ; it != itsFifoCache.end(); )
		{
			if((*it).itsTime >= theMinTime && (*it).itsTime <= theMaxTime)
			{
				(*it).DestroyData();
				it = itsFifoCache.erase(it);
			}
			else
				++it; // jos ei poistettu mit��n, pit�� iteraattoria edist��
		}
	}
}

CBitmap* NFmiMapViewCache::CacheRow::MapImage(const NFmiMetTime &theTime)
{
	std::list<CacheSlot>::iterator it = Find(theTime);
	if(it != itsFifoCache.end())
	{
		return (*it).itsMapImage;
	}
	return 0;
}

void NFmiMapViewCache::CacheRow::MapImage(const NFmiMetTime &theTime, CBitmap* theMapImage)
{
	itsFifoCache.push_back(NFmiMapViewCache::CacheSlot(theTime, theMapImage));
}

std::list<NFmiMapViewCache::CacheSlot>::iterator NFmiMapViewCache::CacheRow::Find(const NFmiMetTime &theTime)
{
	return std::find_if(itsFifoCache.begin(), itsFifoCache.end(), FindImageTime(theTime));
}

struct ImageSizeSum
{
	ImageSizeSum(void):itsSum(0){}
	void operator()(const NFmiMapViewCache::CacheSlot &theSlot)
	{
		itsSum += theSlot.itsSizeInMB;
	}
	double itsSum;
};

double NFmiMapViewCache::CacheRow::CalcUsedSize(void)
{
	ImageSizeSum summer;
	summer = std::for_each(itsFifoCache.begin(), itsFifoCache.end(), summer);
	return summer.itsSum;
}

double NFmiMapViewCache::CacheRow::MakeRoom(double theMinCleareSizeMB)
{
	double freedSizeMB = 0;
	for( ; !itsFifoCache.empty(); )
	{
		CacheSlot &tmp(itsFifoCache.front());
		freedSizeMB += tmp.itsSizeInMB;
		tmp.DestroyData();
		itsFifoCache.pop_front();
		if(freedSizeMB >= theMinCleareSizeMB)
			break;
	}
	return freedSizeMB;
}

void NFmiMapViewCache::CacheRow::Swap(CacheRow& otherCacheRow)
{
    itsFifoCache.swap(otherCacheRow.itsFifoCache);
}

NFmiMapViewCache::NFmiMapViewCache(int theRowCount)
:itsCacheRows(theRowCount)
{
}

NFmiMapViewCache::~NFmiMapViewCache(void)
{
}

void NFmiMapViewCache::MakeDirty(void) // likaa koko cache eli tyhjenna koko cache
{
	int ssize = static_cast<int>(itsCacheRows.size());
	for(int i=0 ; i<ssize; i++)
		ClearRow(i);
}

void NFmiMapViewCache::MakeRowDirty(int theRowIndex)
{
	ClearRow(theRowIndex);
}

void NFmiMapViewCache::MakeTimeDirty(const NFmiMetTime &theTime)
{
	int ssize = static_cast<int>(itsCacheRows.size());
	for(int i=0 ; i<ssize; i++)
		itsCacheRows[i].ClearTime(theTime);
}

void NFmiMapViewCache::MakeTimesDirty(const NFmiMetTime &theMinTime, const NFmiMetTime &theMaxTime, int theRowIndex)
{
	if(IsCacheRowIndexOk(theRowIndex))
		itsCacheRows[theRowIndex].MakeTimesDirty(theMinTime, theMaxTime);
}

CBitmap* NFmiMapViewCache::MapImage(const NFmiMetTime &theTime, int theRowIndex)
{
	if(itsMaxSizeMB == 0)
		return 0;
	if(IsCacheRowIndexOk(theRowIndex))
		return itsCacheRows[theRowIndex].MapImage(theTime);
	else
		return 0;
}

void NFmiMapViewCache::MapImage(const NFmiMetTime &theTime, int theRowIndex, CBitmap* theMapImage)
{
	if(itsMaxSizeMB == 0)
		return ; // tama voi vuotaa, jos ulkoa ei tarkisteta ensin, onko cache kaytossa, nyt bitmap pitaisi tuhota ulkopuolella
	double usedSizeMB = CalcUsedSize();
	if(usedSizeMB > itsMaxSizeMB)
		MakeRoom(usedSizeMB - itsMaxSizeMB);
	if(IsCacheRowIndexOk(theRowIndex))
		itsCacheRows[theRowIndex].MapImage(theTime, theMapImage);
}

bool NFmiMapViewCache::IsCacheRowIndexOk(int theRowIndex)
{
    return static_cast<size_t>(theRowIndex) < itsCacheRows.size();
}

void NFmiMapViewCache::SwapRows(int theRowIndex1, int theRowIndex2)
{
    if(IsCacheRowIndexOk(theRowIndex1) && IsCacheRowIndexOk(theRowIndex2))
    {
        itsCacheRows[theRowIndex1].Swap(itsCacheRows[theRowIndex2]);
    }
}

void NFmiMapViewCache::ClearRow(int theRowIndex)
{
	if(IsCacheRowIndexOk(theRowIndex))
		itsCacheRows[theRowIndex].Clear();
}

double NFmiMapViewCache::CalcUsedSize(void)
{
	double usedSizeMB = 0;
	int ssize = static_cast<int>(itsCacheRows.size());
	for(int i=0 ; i<ssize; i++)
		usedSizeMB += itsCacheRows[i].CalcUsedSize();
	return usedSizeMB;
}

double NFmiMapViewCache::MakeRoom(double theMinCleareSizeMB)
{
	double freedSizeMB = MakeRoomFromNonUsedRows(theMinCleareSizeMB);
	if(freedSizeMB > theMinCleareSizeMB)
		return freedSizeMB;
	else
	{ // viela pitaa vapauttaa tilaa
		double freedSizeMB2 = MakeRoomFromUsedRows(theMinCleareSizeMB - freedSizeMB); 
		return freedSizeMB + freedSizeMB2;
	}
}

double NFmiMapViewCache::MakeRoomFromNonUsedRows(double theMinCleareSizeMB)
{
	double clearedSizeMB = 0;
	double leftToClear = theMinCleareSizeMB;
	int ssize = static_cast<int>(itsCacheRows.size());
	for(int i=ssize-1 ; i>=0; i--)
	{
		auto it = std::find(itsUsedRowIndexies.begin(), itsUsedRowIndexies.end(), i);
		if(it == itsUsedRowIndexies.end()) // jos ei loytynyt kaytossa olevien listalat, yritetaan siivota rivia
			clearedSizeMB += itsCacheRows[i].MakeRoom(leftToClear);
		leftToClear = theMinCleareSizeMB - clearedSizeMB;
		if(leftToClear <= 0)
			break;
	}
	return clearedSizeMB;
}

double NFmiMapViewCache::MakeRoomFromUsedRows(double theMinCleareSizeMB)
{
	double clearedSizeMB = 0;
	double leftToClear = theMinCleareSizeMB;
	int ssize = static_cast<int>(itsCacheRows.size());
	for(int i=ssize-1 ; i>=0; i--)
	{
		auto it = std::find(itsUsedRowIndexies.begin(), itsUsedRowIndexies.end(), i);
		if(it != itsUsedRowIndexies.end()) // jos loytyy kaytossa olevien listalta, yritetaan siivota rivia
			clearedSizeMB += itsCacheRows[i].MakeRoom(leftToClear);
		leftToClear = theMinCleareSizeMB - clearedSizeMB;
		if(leftToClear <= 0)
			break;
	}
	return clearedSizeMB;
}
#else // UNIX - stub implementations

NFmiMapViewCache::CacheSlot::CacheSlot(void)
:itsTime()
,itsSizeInMB(0)
,itsDrawTime()
{
}

NFmiMapViewCache::CacheSlot::~CacheSlot(void)
{
}

void NFmiMapViewCache::CacheSlot::DestroyData(void)
{
	itsSizeInMB = 0;
}

NFmiMapViewCache::CacheRow::CacheRow(void)
:itsFifoCache()
{
}

NFmiMapViewCache::CacheRow::~CacheRow(void)
{
	Clear();
}

void NFmiMapViewCache::CacheRow::Clear(void)
{
	std::for_each(itsFifoCache.begin(), itsFifoCache.end(), [&](auto& cacheSlot) { cacheSlot.DestroyData(); });
	itsFifoCache.clear();
}

void NFmiMapViewCache::CacheRow::ClearTime(const NFmiMetTime &theTime)
{
	auto it = std::find_if(itsFifoCache.begin(), itsFifoCache.end(),
		[&](const CacheSlot& slot) { return theTime == slot.itsTime; });
	if(it != itsFifoCache.end())
	{
		it->DestroyData();
		itsFifoCache.erase(it);
	}
}

void NFmiMapViewCache::CacheRow::MakeTimesDirty(const NFmiMetTime &theMinTime, const NFmiMetTime &theMaxTime)
{
	for(auto it = itsFifoCache.begin(); it != itsFifoCache.end(); )
	{
		if(it->itsTime >= theMinTime && it->itsTime <= theMaxTime)
		{
			it->DestroyData();
			it = itsFifoCache.erase(it);
		}
		else
			++it;
	}
}

double NFmiMapViewCache::CacheRow::CalcUsedSize(void)
{
	double sum = 0;
	for(const auto& slot : itsFifoCache)
		sum += slot.itsSizeInMB;
	return sum;
}

double NFmiMapViewCache::CacheRow::MakeRoom(double theMinCleareSizeMB)
{
	double freedSizeMB = 0;
	while(!itsFifoCache.empty())
	{
		auto& tmp = itsFifoCache.front();
		freedSizeMB += tmp.itsSizeInMB;
		tmp.DestroyData();
		itsFifoCache.pop_front();
		if(freedSizeMB >= theMinCleareSizeMB)
			break;
	}
	return freedSizeMB;
}

void NFmiMapViewCache::CacheRow::Swap(CacheRow& otherCacheRow)
{
	itsFifoCache.swap(otherCacheRow.itsFifoCache);
}

std::list<NFmiMapViewCache::CacheSlot>::iterator NFmiMapViewCache::CacheRow::Find(const NFmiMetTime &theTime)
{
	return std::find_if(itsFifoCache.begin(), itsFifoCache.end(),
		[&](const CacheSlot& slot) { return theTime == slot.itsTime; });
}

NFmiMapViewCache::NFmiMapViewCache(int theRowCount)
:itsCacheRows(theRowCount)
,itsMaxSizeMB(0)
{
}

NFmiMapViewCache::~NFmiMapViewCache(void)
{
}

void NFmiMapViewCache::MakeDirty(void)
{
	int ssize = static_cast<int>(itsCacheRows.size());
	for(int i = 0; i < ssize; i++)
		ClearRow(i);
}

void NFmiMapViewCache::MakeRowDirty(int theRowIndex)
{
	ClearRow(theRowIndex);
}

void NFmiMapViewCache::MakeTimeDirty(const NFmiMetTime &theTime)
{
	int ssize = static_cast<int>(itsCacheRows.size());
	for(int i = 0; i < ssize; i++)
		itsCacheRows[i].ClearTime(theTime);
}

void NFmiMapViewCache::MakeTimesDirty(const NFmiMetTime &theMinTime, const NFmiMetTime &theMaxTime, int theRowIndex)
{
	if(IsCacheRowIndexOk(theRowIndex))
		itsCacheRows[theRowIndex].MakeTimesDirty(theMinTime, theMaxTime);
}

bool NFmiMapViewCache::IsCacheRowIndexOk(int theRowIndex)
{
	return static_cast<size_t>(theRowIndex) < itsCacheRows.size();
}

void NFmiMapViewCache::SwapRows(int theRowIndex1, int theRowIndex2)
{
	if(IsCacheRowIndexOk(theRowIndex1) && IsCacheRowIndexOk(theRowIndex2))
	{
		itsCacheRows[theRowIndex1].Swap(itsCacheRows[theRowIndex2]);
	}
}

void NFmiMapViewCache::ClearRow(int theRowIndex)
{
	if(IsCacheRowIndexOk(theRowIndex))
		itsCacheRows[theRowIndex].Clear();
}

double NFmiMapViewCache::CalcUsedSize(void)
{
	double usedSizeMB = 0;
	int ssize = static_cast<int>(itsCacheRows.size());
	for(int i = 0; i < ssize; i++)
		usedSizeMB += itsCacheRows[i].CalcUsedSize();
	return usedSizeMB;
}

double NFmiMapViewCache::MakeRoom(double theMinCleareSizeMB)
{
	double freedSizeMB = MakeRoomFromNonUsedRows(theMinCleareSizeMB);
	if(freedSizeMB > theMinCleareSizeMB)
		return freedSizeMB;
	else
	{
		double freedSizeMB2 = MakeRoomFromUsedRows(theMinCleareSizeMB - freedSizeMB);
		return freedSizeMB + freedSizeMB2;
	}
}

double NFmiMapViewCache::MakeRoomFromNonUsedRows(double theMinCleareSizeMB)
{
	double clearedSizeMB = 0;
	double leftToClear = theMinCleareSizeMB;
	int ssize = static_cast<int>(itsCacheRows.size());
	for(int i = ssize - 1; i >= 0; i--)
	{
		auto it = std::find(itsUsedRowIndexies.begin(), itsUsedRowIndexies.end(), i);
		if(it == itsUsedRowIndexies.end())
			clearedSizeMB += itsCacheRows[i].MakeRoom(leftToClear);
		leftToClear = theMinCleareSizeMB - clearedSizeMB;
		if(leftToClear <= 0)
			break;
	}
	return clearedSizeMB;
}

double NFmiMapViewCache::MakeRoomFromUsedRows(double theMinCleareSizeMB)
{
	double clearedSizeMB = 0;
	double leftToClear = theMinCleareSizeMB;
	int ssize = static_cast<int>(itsCacheRows.size());
	for(int i = ssize - 1; i >= 0; i--)
	{
		auto it = std::find(itsUsedRowIndexies.begin(), itsUsedRowIndexies.end(), i);
		if(it != itsUsedRowIndexies.end())
			clearedSizeMB += itsCacheRows[i].MakeRoom(leftToClear);
		leftToClear = theMinCleareSizeMB - clearedSizeMB;
		if(leftToClear <= 0)
			break;
	}
	return clearedSizeMB;
}

#endif // UNIX
