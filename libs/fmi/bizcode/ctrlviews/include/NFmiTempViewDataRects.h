#pragma once
#include "NFmiRect.h"

class CtrlViewDocumentInterface;
class NFmiToolBox;

class NFmiTempViewDataRects
{
	// Interface pointteri, josta saadaan monet alueiden laskentoihin liittyv�t tiedot.
	// Ei omista, eik� tuhoa...
	CtrlViewDocumentInterface* ctrlViewDocumentInterface_;
	// T�nne asetetaan kulloinkin k�yt�ss� oleva toolbox olion pointteri ,ei omista, ei tuhoa...
	NFmiToolBox* usedToolBox_ = nullptr;
	// T�m� on perusleveysyksikk�, joka laitetaan eri sivun�ytt�jen v�liin horisontaalisuunnassa.
	double baseMarginRelativeWidth_ = 0.;
	// T�m� arvo annetaan luotausn�yt�lt�. Sen tarkoitus on olla arvossa (1,1) normaalipiirrossa,
	// mutta sille lasketaan uusi suhteellinen arvo kun luotausn�ytt�� printataan, jotta
	// normi n�yt�lle piirron ja printtauksen v�lille ei tulisi suhteellisia eroja.
	NFmiPoint drawSizeFactor_;
	// Mik� on luotausn�yt�n vasemmassa reunassa olevan paineasteikon suhteellinen leveys
	double pressureScaleWidth_ = 0;
	// T�m� annetaan ulkopuolelta ja se sis�lt�� koko luotausn�yt�n suhteellisen piirtoalueen.
	NFmiRect totalSoundingViewRect_;
	// Alue mink� sis��n lasketaan muuta dataosioiden alueet.
	// T�m�n laskuissa on otettu huomioon koko piirtoalueen reunoilla olevat asteikot ja marginaalit.
	NFmiRect totalDataRect_;
	// T�m� stability-index alue lasketaan 1. ja on aina oikeassa laidassa (jos n�kyvill�).
	// T�m� on suhteellisessa koordinaatistossa oleva alue, johon piirret��n eri stabiilisuus indeksit.
	NFmiRect stabilityIndexSideViewRect_;
	// Lasketaan erikseen stability indeksi tekstin fonttikoko pikseleiss�.
	// Jos kyseinen n�ytt� on piilossa, laitetaan sen fontin kooksi 0.
	int stabilityIndexFontSize_ = 0;
	// T�m� lasketaan 2. ja on 2. oikeasta laidassa (jos n�kyvill�).
	// T�m� on suhteellisessa koordinaatistossa oleva alue, johon piirret��n luotauksen arvot tekstimuodossa.
	NFmiRect textualSoundingDataSideViewRect_;
	// Lasketaan erikseen tekstimuotoisen luotausdatan tekstin fonttikoko pikseleiss�.
	// Jos kyseinen n�ytt� on piilossa, laitetaan sen fontin kooksi 0.
	int textualSoundingDataFontSize_ = 0;
	// Animaationappien paikat lasketaan 3. ja ne laitetaan joko stabilityIndexSideViewRect_ 
	// (prioriteetti 1) tai textualSoundingDataSideViewRect_:in  sis�lle ja sen alareunaan kiinni.
	// N�ihin piirret��n animaatio kontrollit napit ja n�iden p��ll� suoritetaan animaatio mousewheel operaatiot.
	NFmiRect animationButtonRect_;
	NFmiRect animationStepButtonRect_;
	// Apudata alue lasketaan 4. ja se tulee textualSoundingDataSideViewRect_:in vasemmalle puolelle (jos n�kyvill�).
	// T�m� m��ritt�� siis sen suhteellisen alueen, mihin piirret��n 0 - 100 asteikko (vaakasuunnassa) ja siihen 
	// piirret��n mm. seuraavia parametreja (jos datasta l�ytyy niit�) WS, N, RH.
	NFmiRect secondaryDataFrame_;
	// Varsinainen k�yrien p��piiirtoalue lasketaan viimeisen� ja se tulee secondaryDataFrame_:n 
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
