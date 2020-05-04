#pragma once

#include "NFmiRect.h"

class CDC;

// TrueViewSizeInfo luokkaaan lasketaan oikeat tiedot erilaisista kartta-alue koista.
// Nykyisin lasketut eri arvot ovat vanhoja ja eri tilanteissa erilailla laskettuja 
// bugisia juttuja, joita toistaiseksi yll�pidet��n, jotta legacy asiat eiv�t menisi rikki.
// Tarkoitus on ett� t�m� luokan arvot otetaan k�ytt��n kaikkialla ja bugiset jutut poistetaan k�yt�st�.
// T�m� luokka my�s laskee todelliset mm/pixel ratiot, koska se ei k�yt� loogisia 
// GetDeviceCaps(pDC, LOGPIXELSX) funktioita, jotka antavat samat arvot eri koneissa (120 pixel/inch kotikoneessa ja kannettavassa).
class TrueMapViewSizeInfo
{
	// Karttan�yt�n koko alueen koko pikseleiss� (= kartat + aikakontrolli)
	NFmiPoint clientAreaSizeInPixels_;
	// Karttan�yt�n karttaosion koko pikseleiss� (= pelk�st��n kartat, yksi kartta/ruudukko)
	NFmiPoint totalMapSectionSizeInPixels_;
	// Yhden alikartan koko pikseleiss� (jos vain yksi kartta kerrallaan n�kyviss�, sama kuin itsTotalMapSectionSizeInPixels)
	NFmiPoint singleMapSizeInPixels_;
	// Yhden alikartan koko millimetreiss�, t�t� k�ytet��n mm. erilaisissa n�yt�n harvennus ja hatching koodeissa
	NFmiPoint singleMapSizeInMM_;
	// Kuinka monta pikseli� mahtuu millimetrin pituisella alueelle x- ja y-suunnassa.
	// T�m� on laskettu monitorin koon ja resuluution mukaan, toisin kuin GraphicalInfo:ssa se on 
	// laskettu loogisesta arvosta...
	NFmiPoint pixelsPerMilliMeter_;
	NFmiPoint monitorSizeInMilliMeters_;
	NFmiPoint monitorSizeInPixels_;
	int mapViewDescTopIndex_ = 0;
public:
	TrueMapViewSizeInfo(int mapViewDescTopIndex);
	TrueMapViewSizeInfo(const TrueMapViewSizeInfo&);
	TrueMapViewSizeInfo& operator=(const TrueMapViewSizeInfo&);
	void onSize(const NFmiPoint& clientPixelSize, CDC* pDC, const NFmiPoint& viewGridSize, bool isTimeControlViewVisible);
	void onViewGridSizeChange(const NFmiPoint& viewGridSize, bool isTimeControlViewVisible);
	static double calculateTimeControlViewHeightInPixels(double pixelsPerMilliMeterX);
	const NFmiPoint& singleMapSizeInMM() const { return singleMapSizeInMM_; }
	const NFmiPoint& pixelsPerMilliMeter() const { return pixelsPerMilliMeter_; }
private:
	void calculateViewSizeInfo(CDC* pDC, const NFmiPoint& viewGridSize, bool isTimeControlViewVisible);
	void updatePixelsPerMilliMeterValues();
	void updateMapSizes(const NFmiPoint& viewGridSize, bool isTimeControlViewVisible);
};
