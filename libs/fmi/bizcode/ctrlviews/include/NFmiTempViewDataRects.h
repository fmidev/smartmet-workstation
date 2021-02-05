#pragma once
#include "NFmiRect.h"

class CtrlViewDocumentInterface;
class NFmiToolBox;

class NFmiTempViewDataRects
{
	// Interface pointteri, josta saadaan monet alueiden laskentoihin liittyvät tiedot.
	// Ei omista, eikä tuhoa...
	CtrlViewDocumentInterface* ctrlViewDocumentInterface_;
	// Tänne asetetaan kulloinkin käytössä oleva toolbox olion pointteri ,ei omista, ei tuhoa...
	NFmiToolBox* usedToolBox_ = nullptr;
	// Tämä on perusleveysyksikkö, joka laitetaan eri sivunäyttöjen väliin horisontaalisuunnassa.
	double baseMarginRelativeWidth_ = 0.;
	// Tämä arvo annetaan luotausnäytöltä. Sen tarkoitus on olla arvossa (1,1) normaalipiirrossa,
	// mutta sille lasketaan uusi suhteellinen arvo kun luotausnäyttöä printataan, jotta
	// normi näytölle piirron ja printtauksen välille ei tulisi suhteellisia eroja.
	NFmiPoint drawSizeFactor_;
	// Mikä on luotausnäytön vasemmassa reunassa olevan paineasteikon suhteellinen leveys
	double pressureScaleWidth_ = 0;
	// Tämä annetaan ulkopuolelta ja se sisältää koko luotausnäytön suhteellisen piirtoalueen.
	NFmiRect totalSoundingViewRect_;
	// Alue minkä sisään lasketaan muuta dataosioiden alueet.
	// Tämän laskuissa on otettu huomioon koko piirtoalueen reunoilla olevat asteikot ja marginaalit.
	NFmiRect totalDataRect_;
	// Tämä stability-index alue lasketaan 1. ja on aina oikeassa laidassa (jos näkyvillä).
	// Tämä on suhteellisessa koordinaatistossa oleva alue, johon piirretään eri stabiilisuus indeksit.
	NFmiRect stabilityIndexSideViewRect_;
	// Lasketaan erikseen stability indeksi tekstin fonttikoko pikseleissä.
	// Jos kyseinen näyttö on piilossa, laitetaan sen fontin kooksi 0.
	int stabilityIndexFontSize_ = 0;
	// Tämä lasketaan 2. ja on 2. oikeasta laidassa (jos näkyvillä).
	// Tämä on suhteellisessa koordinaatistossa oleva alue, johon piirretään luotauksen arvot tekstimuodossa.
	NFmiRect textualSoundingDataSideViewRect_;
	// Lasketaan erikseen tekstimuotoisen luotausdatan tekstin fonttikoko pikseleissä.
	// Jos kyseinen näyttö on piilossa, laitetaan sen fontin kooksi 0.
	int textualSoundingDataFontSize_ = 0;
	// Animaationappien paikat lasketaan 3. ja ne laitetaan joko stabilityIndexSideViewRect_ 
	// (prioriteetti 1) tai textualSoundingDataSideViewRect_:in  sisälle ja sen alareunaan kiinni.
	// Näihin piirretään animaatio kontrollit napit ja näiden päällä suoritetaan animaatio mousewheel operaatiot.
	NFmiRect animationButtonRect_;
	NFmiRect animationStepButtonRect_;
	// Apudata alue lasketaan 4. ja se tulee textualSoundingDataSideViewRect_:in vasemmalle puolelle (jos näkyvillä).
	// Tämä määrittää siis sen suhteellisen alueen, mihin piirretään 0 - 100 asteikko (vaakasuunnassa) ja siihen 
	// piirretään mm. seuraavia parametreja (jos datasta löytyy niitä) WS, N, RH.
	NFmiRect secondaryDataFrame_;
	// Varsinainen käyrien pääpiiirtoalue lasketaan viimeisenä ja se tulee secondaryDataFrame_:n 
	// vasemmalle puolelle ja jatkuu aina totalDataRect_:in vasempaan reunaan asti.
	NFmiRect soundingCurveDataRect_;
public:
    NFmiTempViewDataRects();
    ~NFmiTempViewDataRects();

	void calculateAllDataViewRelatedRects(const NFmiRect &totalSoundingViewRect, const NFmiPoint &drawSizeFactor, NFmiToolBox* usedToolBox);

	const NFmiRect& getSoundingCurveDataRect() const { return soundingCurveDataRect_; }
	const NFmiRect& getSecondaryDataFrame() const { return secondaryDataFrame_; }
	double getPressureScaleWidth() const { return pressureScaleWidth_; }
	const NFmiRect& getStabilityIndexSideViewRect() const { return stabilityIndexSideViewRect_; }
	const NFmiRect& getTextualSoundingDataSideViewRect() const { return textualSoundingDataSideViewRect_; }
	int getStabilityIndexFontSize() const { return stabilityIndexFontSize_; }
	int getTextualSoundingDataFontSize() const { return textualSoundingDataFontSize_; }
	const NFmiRect& getAnimationButtonRect() const { return  animationButtonRect_; }
	const NFmiRect& getAnimationStepButtonRect() const { return  animationStepButtonRect_; }
	const NFmiRect& getTotalSoundingViewRect() const { return  totalSoundingViewRect_; }
private:
	NFmiRect calcTotalDataRect();
	NFmiRect calcStabilityIndexRect();
	NFmiRect calcTextualSoundingDataRect(const NFmiRect &rightSideView);
	double calcPressureScaleWidth();
	NFmiRect calcSoundingCurveDataRect(const NFmiRect& rightSideView);
	void calculateStabilityIndexFontSizeInPixels();
	void calculatetextualSoundingDataFontSizeInPixels();
	double calculateLeftRelativeEdgeOfSideView(double rightEdge, bool showSideView, int sideViewTextWidthCount, int fontSizeInPixels);
	void makeAnimationControlRects();
	NFmiRect calcSecondaryDataRect(const NFmiRect& rightSideView);
};
