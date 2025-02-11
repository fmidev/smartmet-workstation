//© Ilmatieteenlaitos/software by Marko Pietarinen
//  Original 19.02.2008
//
//
//-------------------------------------------------------------------- NFmiConceptualDataView.h

#pragma once

#include "NFmiStationView.h"
#include "NFmiParameterName.h"
#include "xmlliteutils/XMLite.h"
#include "NFmiImageMap.h"

class NFmiToolBox;
class NFmiArea;
namespace Gdiplus
{
	class PointF;
    class Bitmap;
    class GraphicsPath;
    class Font;
}

enum FrontType
{
	kFmiFrontTypeError=-1,
	kFmiFrontTypeWarm=0,
	kFmiFrontTypeCold=1,
	kFmiFrontTypeOcclusion=2,
	kFmiFrontTypeThrough=3,
	kFmiFrontTypeJet=4,
	kFmiFrontTypeSlowJet=5,
	kFmiFrontTypeWind=6,
	kFmiFrontTypeCloudArea=7,
	kFmiFrontTypeStreamLine=8, // tämä tulee NFmiStreamLineView-luokan tarpeista
	kFmiFrontTypeCloudAreaPartArc=1001, // pilvi alue koostuu kahdesta osasta, kaaresta ja läpinäkyvästä puolipallosta
	kFmiFrontTypeCloudAreaPartPie1=1002, // tämä on tummempi ja sisään päin
	kFmiFrontTypeCloudAreaPartPie2=1003 // tämä on vaaleampi ja ulospäin
};

class ConceptualObjectData
{
public:
	ConceptualObjectData(void);
	void Clear(void);
	void InitializeFromPartNode(LPXNode theNode);
	void InitializeFromWomlNode(LPXNode theNode);
	void InitLatlonValues(bool fLonLatOrder);
	void InitDataFromRawStrings(void);
	void InitDataFromWomlStrings(LPXNode theNode);
	void SetDataFromObjectType(LPXNode theNode);
    void CalcParameterValues(LPXNode theNode);

	std::string itsPartTypeStr; // conceptual tai symbol
	bool fConceptualType; // true jos on conceptual ja false jos symbol type
	std::string itsFrontNameStr; // eri rintamat (esim. COLDFRONT) ja PLAIN on alue ja symbolilla ei ole mitään arvoa
	bool fPlainArea; // onko kyseessä alueesta (sadealue tms.)
	std::string itsValidTimeStr; // muotoa 2008-02-18T06:00:00+0000
	NFmiMetTime itsValidTime; // tulkittu itsValidTimeStr:ista
	std::string itsInfoTextStr; // meteorologin temeä valinnainen teksti joka menee tooltippiin
	std::string itsGmlPosListStr; // olion koordinaatit lon-lat pareina
	std::vector<NFmiPoint> itsLatlonPoints; // tulkittu itsGmlPosListStr:ista
	std::string itsOrientationStr; // 1 / 2 (left/right?)
	bool fLeft; // tulkittu itsOrientationStr:ista
	std::string itsAppearanceStr; // viivan paksuus (näyttöruudun pikseleissä) ja R G B
	double itsLineWidthInMM; // tulkittu itsAppearanceStr:ista
	NFmiColor itsConceptualColor; // tulkittu itsAppearanceStr:ista
	std::string itsRainphaseStr; // alueen sadetyyppi esim snow/sleet jne.
	std::string itsFontNameStr; // symbolissa käytetyn fontin nimi
	std::string itsFontColorStr; // symbolin väri R G B
	NFmiColor itsFontColor; // tulkittu itsFontColorStr:ista
	std::string itsFontScaleFactorStr; // suhteellinen fontin koko
	float itsFontScaleFactor; // edellinen stringi lukuna
	std::string itsSymbolCodeStr; // symbolin koodi annetussa fontissa
	int itsSymbolCode; // tulkittu itsSymbolCodeStr:ista
	std::string itsParameterValueStr; // tähän tulee mm. lämpötila tai lämpötilaväli stringi (esim. "-12" tai -12..-8)
									  // jos itsParameterValueStr ei ole tyhjä, käytetään sitä symboli piirrossa
	std::string itsReferenceStr; // mikä on womlqty:reference arvo?
    float itsParameterValue; // tähän tulee arvo itsParameterValueStr:istä, tai missing, jos NaN tai puuttuu
    float itsParameterValue2; // jos itsParameterValueStr:issä oli arvohaarukka esim. 10...15, 1. arvo laitetaan itsParameterValue:een ja toinen tähän
	std::string itsGmlPosStr; // symbolin lat-lon
	NFmiPoint itsSymbolLatlon;
	std::string itsWomlMemberNameStr; // WOML:issa olevan otuksen nimi/tyyppi
	std::string itsWomlSymbolInfoStr; // WOML:issa olevan symbolin tietoa (esim. mirricode@49)
    int itsFillHatchPattern; // -1 tarkoittaa ei hatch:iä ja HatchStyle -enum löytyy gdiplusenums.h -tiedostosta
    bool fIsWindArrow;
	bool fWomlOk;

	double itsPixelSizeInMM; // tämä annetaan ulkopuolelta (pitää olla näytön pikseli koko, ei printterin)
};


class NFmiConceptualDataView : public NFmiStationView
{

public:
	NFmiConceptualDataView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
							,NFmiToolBox * theToolBox
							,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							,FmiParameterName theParamId
							,int theRowIndex
                            ,int theColumnIndex);
    ~NFmiConceptualDataView(void);
	void Draw (NFmiToolBox * theGTB) override;
	std::string ComposeToolTipText(const NFmiPoint& theRelativePoint) override;

    // HUOM! tätä pitää kutsua ennen kuin itse luokkaa saa käyttää!!!!
    static void InitMirwaSymbolMap(const std::string &theWomlDirectory);

    // Tähän muutamia staattisia apufunktioita, joita voi käyttää luokan ulkoa (esim. NFmiStreamLineView:ssa)
    static float CalcScreenSizeFactor(CtrlViewDocumentInterface &theCtrlViewDocumentInterface, int theMapViewDesctopIndex); // laskee tähän näyttöön ja tähän tilaan sopivan mm-koko kertoimen
    static float CalcUsedFontSizeInPixels(float theWantedFontSizeInMM, CtrlViewDocumentInterface &theCtrlViewDocumentInterface, int theMapViewDesctopIndex, float theFontScaleFactor);
    static void CalculatePathLengths(float pixelLengthInMM, const std::vector<Gdiplus::PointF> &xyPoints, std::vector<float> &lengthsInMM, float &totalLengthInMM);
    static int CalculateUsedSymbolCountAndMeasures(float totalLengthInMM, float &startGapInMM, float &objectSizeInMM, float &gapInMM);
    static void GetDecorationPath(float symbolSizeInPixels, FrontType frontType, Gdiplus::GraphicsPath &thePathOut);
    static void CalcPathObjectPoints(const std::vector<Gdiplus::PointF> &frontPoints, size_t objectCount, float startGapInMM, float objectSizeInMM,
        float gapInMM, const std::vector<float> &lengthsInMM, float totalLengthInMM, std::vector<Gdiplus::PointF> &objectPoints,
        std::vector<float> &rotationAngles);
    static void CalculateIdealPathObjectMeasures(float lineWidthInMM, FrontType frontType, float &startGapInMM, float &objectSizeInMM, float &gapInMM);

protected:
	void DrawCloudArea(ConceptualObjectData &theData, float theLineWidthInMM, const std::vector<Gdiplus::PointF> &theXyPoints, float theLineWidthInPixels);
	void DrawFrontDecorations(ConceptualObjectData &theData, float theLineWidthInMM, const std::vector<Gdiplus::PointF> &theXyPoints, float theLineWidthInPixels, FrontType theFrontType);
	void DrawConseptualData(ConceptualObjectData &theData);
	void DrawConseptualObject(ConceptualObjectData &theData);
	void DrawSymbol(ConceptualObjectData &theData);
	void DrawTextArea(ConceptualObjectData &theData);
	void GetConceptualData(const NFmiMetTime &theTime); // tämä on yleis haku funktio, joka haarautuu konfiguraatioiden mukaisesti erilaisiin hakuihin
	void GetConceptualsFromDB(const NFmiMetTime &theTime);
	void GetConceptualsFromFile(const NFmiMetTime &theTime);
    void DrawSymbol(ConceptualObjectData &theData, double wantedSymbolSizeInMM);
    void DrawTemperatureRange(ConceptualObjectData &theData, const Gdiplus::PointF &thePlace, const Gdiplus::Font &theFont);

	void GetConceptualsDataStrFromDB(const NFmiMetTime &theTime, std::string &theResultStr);
    void GetConceptualsDataStrFromWomlDB(const NFmiMetTime &theTime, std::string &theResultStr);
	void GetConceptualsDataStrFromFile(const NFmiMetTime &theTime, std::string &theResultStr);
	void DecodeConceptualDataFromStr(const std::string &theConceptualDataStr);
	void DecodeConceptualWomlDataFromStr(const std::string &theConceptualDataStr);
	void DecodeConceptualPreWomlDataFromStr(const std::string &theConceptualDataStr);
    void GetConceptualDataStrFromDB(std::string &url, std::string &theResultStr);

	int itsRowIndex; // monennessako rivissä näyttöruudukossa tämä näyttö sijaitsee
	std::vector<ConceptualObjectData> itsConceptualObjectDatas;

	double itsScreenPixelSizeInMM; // tähän pitää saada talteen pikselin koko mm:eissä silloin kun ei olla printtaamassa, 
								// koska viivan paksuu on annettu ruudulla näkyvissä pikselikoossa.
    static NFmiImageMap itsMirwaSymbolMap;
};

