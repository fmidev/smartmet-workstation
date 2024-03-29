#pragma once

#include "NFmiCachedRegistryValue.h"
#include "NFmiPoint.h"
#include "NFmiRect.h"

class NFmiFastQueryInfo;
class NFmiArea;
class NFmiGrid;

// Luokka joka pitää sisällään kahta isoviiva/contour piirtoihin liittyvää säätöä:
// 1. Isoviivoihin liittyvä hilankoko vs näytön pikselikoko suhde 
//   - Tällä säädöllä on tarkoitus estää ToolMaster visualisointi pakettiin liittyvä kaatobugi.
// 2. Isoviiva/contour piirtoon liittyvä optimoidun piirron kerroin
//   - Jos tämä on käytössä, lasketaan visualisointeja varten sopiva hilakoko kullekin datalle/karttapohjalle erikseen
class NFmiVisualizationSpaceoutSettings
{
	// Jos isoviivoja piirtää liian tiheällä hilalla suhteessa näytöllä oleviin pikseleihin
	// alkaa ToolMaster systeemi hajoamaan satunnaisesti. Systeemi joko lakkaa toimimasta tai kaatuu.
	// Satunnaisuutta lisää että ongelmat esiintyvät enemmän tietyillä datoilla ja parametreilla kuin toisilla.
	// Käytössä on todettu että pixel/gridpoint => ~4 on siedettävä (ongelmia on harvemmin).
	// Käytännössä pixelToGridPointRatio_ arvolla 4 tarkoittaa, että 4 pikseliä vastaa yhtä hilapistettä koon suhteen
	// minimissään, kun dataa piirretään (eli tarkempi data interpoloidaan harvempaan hilaan tarvittaessa).
	const double criticalPixelToGridPointRatioLimit_ = 4.0;
	// Tämä voidaan asettaa minimissään 1:een, koska yleensä piirrossa ei ole hyötyä käyttää dataa joka on tarkempaa
	// kuin näytöllä on pikseleitä.
	// Jos arvo asetettu pienemmäksi kuin criticalPixelToGridPointRatioLimit_, tulee asiasta varoituksia lokiin ja karttanäytön tooltippeihin.
	boost::shared_ptr<CachedRegDouble> pixelToGridPointRatio_;
	const double minPixelToGridPointRatioValue_ = 1.0;
	const double maxPixelToGridPointRatioValue_ = 20.0;
	// Tämän turvaharvennus option voi ottaa päältä, mutta asetusta ei voi tallentaa mihinkään, kun
	// SmartMet käynnistyy uudestaan, on asetus taas päällä (true tilassa). Lisäksi jos tämä pois päältä,
	// tehdään asiasta varoituksia lokiin ja karttanäytön tooltippeihin.
	bool usePixelToGridPointRatioSafetyFeature_ = true;
	
	// Visualisointeja halutaan nopeuttaa, käyttämällä tilanteesta riippuen harvempaa hiladataa, kuin käytössä oikeasti olisi.
	// baseSpaceoutGridSize_ on arvo joka menee välillä 20 - 400 (?).
	// Minimi perus hilakoko on siis 20x20 hilapistettä ja maksi on 400x400.
	// Lopullisen käytetyn hilakoon laskuissa on kuitenkin lukuisia pieniä säätöpoikkeamia:
	// 1) 20-400 arvot tulevat vain kartta-alueen sille kantille kummassa on pidempi reuna projektion omassa maailmassa (km).
	//    - Lyhyemmälle reunalle tulee lyhyempi_reuna_km/pidempi_reuna_km kertoimella oleva määrä hilapisteitä.
	// 2) Jos data ei peita koko kartta-aluetta, saa datasta laskettu hila vain sen suhteellisen osion koko ruudun saamasta hilakoosta.
	// 3) Jos karttanäytöllä on yhden ruudun sijasta ruudukko, pienenee käytetty hilamäärä tietyn käänteisluvun potenssikaavan mukaan:
	//    1 -> 1.0, 2 -> 0.812252,.., 4 -> 0.659754,.., 6 -> 0.584191,.., 9 -> 0.517282,.., 16 -> 0.435275,.., 50 -> 0.309249
	// 4) Jos laskettu harvennettu hila on lähellä datan oikeaa hilatarkkuutta (~10 % päässä), käytetään datan oikeaa hilaa.
	// 5) Jos saatu hila on liian tarkka pixelToGridPointRatio:lle, lasketaan tietenkin datasta vielä lopullinen harvempi hila.
	boost::shared_ptr<CachedRegInt> baseSpaceoutGridSize_;
	const int minBaseSpaceoutGridSize_ = 20;
	const int maxBaseSpaceoutGridSize_ = 400;
	// Halutaanko että harvennussysteemiä käytetään ollenkaan.
	boost::shared_ptr<CachedRegBool> useGlobalVisualizationSpaceoutFactorOptimization_;
	// Millä mekanismilla haetaan dataa harvennettuun hilaan:
	// 0 = Lineaarisesti interpoloimalla (nopea mutta huono)
	// 1 = Median filtterillä (hitaampi mutta paras tulos)
	boost::shared_ptr<CachedRegInt> spaceoutDataGatheringMethod_;
	// Contoureja on turha piirtää tarkasti, jos piirrettävä data hilatarkkuus lähestyy näytön pikseli tasoa.
	// Jos piirrettävä hila menee tämän rajan alle, piirretään data quick-contour tavalla (nopea, eikä menetetä juuri näköä).
	const double criticalPixelToGridPointRatioLimitForContours_ = 1.4;
	// Halutessa harvennusoptimoita voi käyttää myös Beta-tuote tuotannossa, mutta oletuksena se on pois päältä.
	boost::shared_ptr<CachedRegBool> useSpaceoutOptimizationsForBetaProducts_;

	// Windows rekisterin käyttöön liittyviä muuttujia
	bool initialized_ = false;
	std::string baseRegistryPath_;
	std::string sectionName_;
public:
	NFmiVisualizationSpaceoutSettings();
	bool Init(const std::string& baseRegistryPath);

	bool updateFromDialog(double newPixelToGridPointRatio, bool newUsePixelToGridPointRatioSafetyFeature, int newBaseSpaceoutGridSize, bool newUseGlobalVisualizationSpaceoutFactorOptimization, int newSpaceoutDataGatheringMethod, bool newUseSpaceoutOptimizationsForBetaProducts);
	void doViewUpdateWarningLogsIfNeeded();
	bool checkIsOptimizationsUsed(NFmiFastQueryInfo& fastInfo, const NFmiArea& mapArea, NFmiGrid &optimizedGridOut, int viewSubGridSize, bool betaProductRunning) const;
	NFmiPoint getCheckedPossibleOptimizedGridSize(const NFmiPoint& suggestedGridSize, NFmiArea& mapArea, int viewSubGridSize, bool betaProductRunning) const;
	NFmiPoint calcAreaGridSize(NFmiArea& area, int viewSubGridSize) const;
	std::string composePossibleTooltipWarningText(NFmiArea& area, int viewSubGridSize) const;
	bool preventOptimizationsForBetaProducts(bool betaProductRunning) const;

	double criticalPixelToGridPointRatioLimit() const { return criticalPixelToGridPointRatioLimit_; }
	double pixelToGridPointRatio() const;
	void pixelToGridPointRatio(double newValue, bool firstInitialization = false);
	double minPixelToGridPointRatioValue() const { return minPixelToGridPointRatioValue_; }
	double maxPixelToGridPointRatioValue() const { return maxPixelToGridPointRatioValue_; }
	bool usePixelToGridPointRatioSafetyFeature() const { return usePixelToGridPointRatioSafetyFeature_; }
	void usePixelToGridPointRatioSafetyFeature(bool newState) { usePixelToGridPointRatioSafetyFeature_ = newState; }
	int baseSpaceoutGridSize() const;
	void baseSpaceoutGridSize(int newValue);
	int minBaseSpaceoutGridSize() const { return minBaseSpaceoutGridSize_; }
	int maxBaseSpaceoutGridSize() const { return maxBaseSpaceoutGridSize_; }
	bool useGlobalVisualizationSpaceoutFactorOptimization() const;
	void useGlobalVisualizationSpaceoutFactorOptimization(bool newState);
	int spaceoutDataGatheringMethod() const;
	void spaceoutDataGatheringMethod(int newValue);
	double criticalPixelToGridPointRatioLimitForContours() const;
	bool useSpaceoutOptimizationsForBetaProducts() const;
	void useSpaceoutOptimizationsForBetaProducts(bool newState);

	static double calcViewSubGridFactor(int viewSubGridSize);

private:
	NFmiRect calcInfoAreaOverMapAreaWorldXyBoundingBox(NFmiFastQueryInfo& fastInfo, NFmiArea& mapArea) const;
};
