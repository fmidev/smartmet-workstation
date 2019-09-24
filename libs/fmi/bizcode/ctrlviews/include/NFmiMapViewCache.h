// ======================================================================
/*!
 * \file NFmiMapViewCache.h
 * \brief Interface of class NFmiMapViewCache
 */
// ======================================================================

#pragma once

#include "NFmiMetTime.h"
#include "NFmiDataMatrix.h" // t‰‰lt‰ tulee checkedVector

#include <list>

class CBitmap;

class NFmiMapViewCache
{
public:
	struct CacheSlot
	{
		CacheSlot(void);
		CacheSlot(const NFmiMetTime &theTime, CBitmap* theMapImage);
		~CacheSlot(void);
		void DestroyData(void);

		CBitmap* itsMapImage; // ei omista, ei tuhoa, siksi erillinen DestroyData-metodi
							  // bitmap on tavallaan CacheRow-luokan omistuksessa, koska se paattaa tuhoamisesta
		NFmiMetTime itsTime;
		double itsSizeInMB;
		NFmiTime itsDrawTime; // milloin cache kuva on piirretty, k‰ytet‰‰n kun tutkitaan, pit‰‰kˆ esim. uuden satel-kuvan takia piirt‰‰ kuva uudestaan
	};

	class CacheRow
	{
	public:
		CacheRow(void);
		~CacheRow(void);

		void Clear(void);
		void ClearTime(const NFmiMetTime &theTime);
		CBitmap* MapImage(const NFmiMetTime &theTime);
		void MapImage(const NFmiMetTime &theTime, CBitmap* theMapImage);
		double CalcUsedSize(void);
		double MakeRoom(double theMinCleareSizeMB);
		void MakeTimesDirty(const NFmiMetTime &theMinTime, const NFmiMetTime &theMaxTime);
        void Swap(CacheRow& otherCacheRow);

	private:
		std::list<CacheSlot>::iterator Find(const NFmiMetTime &theTime);

		std::list<CacheSlot> itsFifoCache;
	};

	NFmiMapViewCache(int theRowCount = 1);
	~NFmiMapViewCache(void);

	size_t RowSize(void) const {return itsCacheRows.size();}
	CacheRow& GetCacheRow(int theRowIndex) {return itsCacheRows[theRowIndex];}
	void MakeDirty(void); // likaa koko cache
	void MakeRowDirty(int theRowIndex);
	void MakeTimeDirty(const NFmiMetTime &theTime);
	void MakeTimesDirty(const NFmiMetTime &theMinTime, const NFmiMetTime &theMaxTime, int theRowIndex);
	CBitmap* MapImage(const NFmiMetTime &theTime, int theRowIndex);
	void MapImage(const NFmiMetTime &theTime, int theRowIndex, CBitmap* theMapImage);
	double MaxSizeMB(void) const {return itsMaxSizeMB;}
	void MaxSizeMB(double newValue) 
	{itsMaxSizeMB = newValue; if(itsMaxSizeMB == 0) MakeDirty();}
	bool IsCacheUsed(void) const {return itsMaxSizeMB > 0;}
    void SwapRows(int theRowIndex1, int theRowIndex2);

private:
	void ClearRow(int theRowIndex);
	double CalcUsedSize(void);
	double MakeRoom(double theMinCleareSizeMB);
	double MakeRoomFromNonUsedRows(double theMinCleareSizeMB);
	double MakeRoomFromUsedRows(double theMinCleareSizeMB);
    bool IsCacheRowIndexOk(int theRowIndex);

	checkedVector<CacheRow> itsCacheRows;
	checkedVector<int> itsUsedRowIndexies; // mitka rivit ovat editorin kartalla nakyvissa
	double itsMaxSizeMB; // kuinka suuri cachen kokonais koko saa olla maksimissaan eli jos koko ylittaa taman, pitaa alkaa vapauttamaan tilaa
};

