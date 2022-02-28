#pragma once

#include "NFmiCachedRegistryValue.h"
#include "NFmiPoint.h"
#include "NFmiRect.h"

class NFmiFastQueryInfo;
class NFmiArea;

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
	// globalVisualizationSpaceoutFactor_ on kerroin joka menee välillä 1-10 (?).
	// Jos sen arvo on 1, mikä on tarkin hilanharvennuksessä käytetty arvo, tulee hilan maksimi tarkkuudeksi 400
	// ja jos arvo on 10, tulee max tarkkuudeksi 40. Tästä on kuitenkin lukuisia pieniä säätöpoikkeamia:
	// 1) 400/40 arvot tulevat vain kartta-alueen sille kantille kummassa on pidempi reuna projektion omassa maailmassa (km).
	//    - Lyhyemmälle reunalle tulee lyhyempi_reuna_km/pidempi_reuna_km kertoimella oleva määrä hilapisteitä.
	// 2) Jos data ei peita koko kartta-aluetta, saa datasta laskettu hila vain sen suhteellisen osion koko ruudun saamasta hilakoosta.
	// 3) Jos karttanäytöllä on yhden ruudun sijasta ruudukko, pienenee käytetty hilamäärä tietyn lineaarisen kaavan mukaan.
	//    - 1 ruutu => 1.0 kerroin, 2 ruutua => 0.987, 4(2x2) => 0.96, 9(3x3) -> 0.89, 16(4x4) => 0.8, minimi 50(5x10) => 0.4
	// 4) Jos laskettu harvennettu hila on lähellä datan oikeaa hilatarkkuutta (~10 % päässä), käytetään datan oikeaa hilaa.
	// 5) Jos saatu hila on liian tarkka pixelToGridPointRatio:lle, lasketaan tietenkin datasta vilä lopullinen harvempi hila.
	boost::shared_ptr<CachedRegDouble> globalVisualizationSpaceoutFactor_;
	const double minVisualizationSpaceoutFactor_ = 1.0;
	const double maxVisualizationSpaceoutFactor_ = 10.0;
	// Hilakoot menevät SpaceoutFactor:in kanssa käänteisessä järjestyksessä: minFactor -> maxGridSize ja maxFactor -> minGridSize
	const double minVisualizationSpaceoutGridSize_ = 40.0;
	const double maxVisualizationSpaceoutGridSize_ = 400.0;
	// Halutaanko että harvennussysteemiä käytetään ollenkaan.
	boost::shared_ptr<CachedRegBool> useGlobalVisualizationSpaceoutFactorOptimization_;
	// Millä mekanismilla haetaan dataa harvennettuun hilaan:
	// 0 = Lineaarisesti interpoloimalla (nopea mutta huono)
	// 1 = Median filtterillä (hitaampi mutta paras tulos)
	boost::shared_ptr<CachedRegInt> spaceoutDataGatheringMethod_;

	// Windows rekisterin käyttöön liittyviä muuttujia
	bool initialized_ = false;
	std::string baseRegistryPath_;
	std::string sectionName_;
public:
	NFmiVisualizationSpaceoutSettings();
	bool Init(const std::string& baseRegistryPath);

	bool updateFromDialog(double pixelToGridPointRatio, bool usePixelToGridPointRatioSafetyFeature, double globalVisualizationSpaceoutFactor, bool useGlobalVisualizationSpaceoutFactorOptimization, int spaceoutDataGatheringMethod);
	void doViewUpdateWarningLogsIfNeeded();
	bool checkIsOptimizationsUsed(NFmiFastQueryInfo& fastInfo, NFmiArea& mapArea) const;

	double criticalPixelToGridPointRatioLimit() const { return criticalPixelToGridPointRatioLimit_; }
	double pixelToGridPointRatio() const;
	void pixelToGridPointRatio(double newValue);
	double minPixelToGridPointRatioValue() const { return minPixelToGridPointRatioValue_; }
	double maxPixelToGridPointRatioValue() const { return maxPixelToGridPointRatioValue_; }
	bool usePixelToGridPointRatioSafetyFeature() const { return usePixelToGridPointRatioSafetyFeature_; }
	void usePixelToGridPointRatioSafetyFeature(bool newState) { usePixelToGridPointRatioSafetyFeature_ = newState; }
	double globalVisualizationSpaceoutFactor() const;
	void globalVisualizationSpaceoutFactor(double newValue);
	double minVisualizationSpaceoutFactor() const { return minVisualizationSpaceoutFactor_; }
	double maxVisualizationSpaceoutFactor() const { return maxVisualizationSpaceoutFactor_; }
	double minVisualizationSpaceoutGridSize() const { return minVisualizationSpaceoutGridSize_; }
	double maxVisualizationSpaceoutGridSize() const { return maxVisualizationSpaceoutGridSize_; }
	bool useGlobalVisualizationSpaceoutFactorOptimization() const;
	void useGlobalVisualizationSpaceoutFactorOptimization(bool newState);
	int spaceoutDataGatheringMethod() const;
	void spaceoutDataGatheringMethod(int newValue);


private:
	double calcBaseOptimizedGridSize(double usedSpaceoutFactor) const;
	NFmiPoint calcAreaGridSize(NFmiArea& area) const;
	NFmiRect calcInfoAreaOverMapAreaWorldXyBoundingBox(NFmiFastQueryInfo& fastInfo, NFmiArea& mapArea) const;
};
