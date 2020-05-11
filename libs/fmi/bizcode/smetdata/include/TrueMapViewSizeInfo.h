#pragma once

#include "NFmiRect.h"

class CDC;

// TrueViewSizeInfo luokkaaan lasketaan oikeat tiedot erilaisista kartta-alue koista.
// Nykyisin lasketut eri arvot ovat vanhoja ja eri tilanteissa erilailla laskettuja 
// bugisia juttuja, joita toistaiseksi ylläpidetään, jotta legacy asiat eivät menisi rikki.
// Tarkoitus on että tämä luokan arvot otetaan käyttöön kaikkialla ja bugiset jutut poistetaan käytöstä.
// Tämä luokka myös laskee todelliset mm/pixel ratiot, koska se ei käytä loogisia 
// GetDeviceCaps(pDC, LOGPIXELSX) funktioita, jotka antavat samat arvot eri koneissa (120 pixel/inch kotikoneessa ja kannettavassa).
class TrueMapViewSizeInfo
{
	// Karttanäytön koko alueen koko pikseleissä (= kartat + aikakontrolli)
	NFmiPoint clientAreaSizeInPixels_;
	// Karttanäytön karttaosion koko pikseleissä (= pelkästään kartat, yksi kartta/ruudukko)
	NFmiPoint totalMapSectionSizeInPixels_;
	// Yhden alikartan koko pikseleissä (jos vain yksi kartta kerrallaan näkyvissä, sama kuin itsTotalMapSectionSizeInPixels)
	NFmiPoint singleMapSizeInPixels_;
	// Yhden alikartan koko millimetreissä, tätä käytetään mm. erilaisissa näytön harvennus ja hatching koodeissa
	NFmiPoint singleMapSizeInMM_;
	// Kuinka monta pikseliä mahtuu millimetrin pituisella alueelle x- ja y-suunnassa.
	// Tämä on laskettu monitorin koon ja resuluution mukaan, toisin kuin GraphicalInfo:ssa se on 
	// laskettu loogisesta arvosta...
	NFmiPoint pixelsPerMilliMeter_;
	// Pakko sittenkin käyttää loogisia suhteita, koska Windows näytön skaalaus sotkee oikeat pikseli suhteet.
	NFmiPoint logicalPixelsPerMilliMeter_;
	NFmiPoint monitorSizeInMilliMeters_;
	// Tässä on monitorin pikseli resoluutio, MUTTA älä laske niiden varaan mitään, koska Windows näyttö voi 
	// olla skaalattu ja silloin oikeiden pikselien lukumäärät eivät enää pidä paikkaansa normaalissa mielessä.
	NFmiPoint monitorSizeInPixels_;
	int mapViewDescTopIndex_ = 0;
public:
	TrueMapViewSizeInfo(int mapViewDescTopIndex);
	TrueMapViewSizeInfo(const TrueMapViewSizeInfo&);
	TrueMapViewSizeInfo& operator=(const TrueMapViewSizeInfo&);
	void onSize(const NFmiPoint& clientPixelSize, CDC* pDC, const NFmiPoint& viewGridSize, bool isTimeControlViewVisible, double drawObjectScaleFactor);
	void onViewGridSizeChange(const NFmiPoint& viewGridSize, bool isTimeControlViewVisible);
	static double calculateTimeControlViewHeightInPixels(double pixelsPerMilliMeterX);
	const NFmiPoint& singleMapSizeInMM() const { return singleMapSizeInMM_; }
	const NFmiPoint& logicalPixelsPerMilliMeter() const { return logicalPixelsPerMilliMeter_; }
private:
	void calculateViewSizeInfo(CDC* pDC, const NFmiPoint& viewGridSize, bool isTimeControlViewVisible, double drawObjectScaleFactor);
	void updatePixelsPerMilliMeterValues(CDC* pDC, double drawObjectScaleFactor);
	void updateMapSizes(const NFmiPoint& viewGridSize, bool isTimeControlViewVisible);
};
