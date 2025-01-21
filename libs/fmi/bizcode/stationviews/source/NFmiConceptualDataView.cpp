//© Ilmatieteenlaitos/Marko Pietarinen
//  Original 19.02.2008
//
//
//-------------------------------------------------------------------- NFmiConceptualDataView.cpp
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ k‰‰nt‰j‰n varoitusta (liian pitk‰ nimi >255 merkki‰ joka johtuu 'puretuista' STL-template nimist‰)
#endif

#include "NFmiConceptualDataView.h"
#include "NFmiToolBox.h"
#include "NFmiDrawParam.h"
#include "NFmiConceptualModelData.h"
#include "NFmiArea.h"
#include "NFmiColorSpaces.h"
#include "NFmiFileSystem.h"
#include "NFmiSettings.h"
#include "CtrlViewDocumentInterface.h"
#include "GraphicalInfo.h"
#include "MapHandlerInterface.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "GdiPlusLineInfo.h"
#include "CtrlViewFunctions.h"
#include "xmlliteutils/XmlHelperFunctions.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "NFmiMilliSecondTimer.h"
#include "NFmiValueString.h"
#include "NFmiMacroParamfunctions.h"
#include "catlog/catlog.h"
#include "MathHelper.h"

#include <gdiplus.h>
#include <list>
#include "boost\algorithm\string\predicate.hpp"

using namespace std;
using namespace Gdiplus;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const std::string g_RangeSplitterString = "..."; // t‰m‰ on erotin kun piirret‰‰n temperature rangea

ConceptualObjectData::ConceptualObjectData(void)
:itsPartTypeStr()
,fConceptualType(true)
,itsFrontNameStr()
,fPlainArea(false)
,itsValidTimeStr()
,itsValidTime()
,itsInfoTextStr()
,itsGmlPosListStr()
,itsLatlonPoints()
,itsOrientationStr()
,fLeft(false)
,itsAppearanceStr()
,itsLineWidthInMM(0)
,itsConceptualColor()
,itsRainphaseStr()
,itsFontNameStr()
,itsFontColorStr()
,itsFontColor()
,itsSymbolCodeStr()
,itsSymbolCode(0)
,itsParameterValueStr()
,itsReferenceStr()
,itsParameterValue(kFloatMissing)
,itsParameterValue2(kFloatMissing)
,itsGmlPosStr()
,itsSymbolLatlon(NFmiPoint::gMissingLatlon)
,itsPixelSizeInMM(0)
,itsWomlMemberNameStr()
,itsWomlSymbolInfoStr()
,itsFillHatchPattern(-1)
,fIsWindArrow(false)
,fWomlOk(false)
{}

void ConceptualObjectData::Clear(void)
{
	*this = ConceptualObjectData();
}

void ConceptualObjectData::InitializeFromPartNode(LPXNode theNode)
{
	itsPartTypeStr = XmlHelper::AttributeValue(theNode, "type"); // conceptual tai symbol
	itsFrontNameStr = XmlHelper::ChildNodeValue(theNode, "frontName"); // eri rintamat ja PLAIN on alue ja symbolilla ei ole mit‰‰n
	itsValidTimeStr = XmlHelper::ChildNodeValue(theNode, "validTime");
	itsInfoTextStr = XmlHelper::ChildNodeValue(theNode, "infoText");
	itsGmlPosListStr = XmlHelper::ChildNodeValue(theNode, "gml:posList"); // olion koordinaatit lon-lat pareina
	itsOrientationStr = XmlHelper::ChildNodeValue(theNode, "orientation"); // 1 / 2 (left/right?)
	itsAppearanceStr = XmlHelper::ChildNodeValue(theNode, "appearance"); // viivan paksuus ja R G B
	itsRainphaseStr = XmlHelper::ChildNodeValue(theNode, "rainphase"); // alueen sadetyyppi esim snow/sleet jne.
	itsFontNameStr = XmlHelper::ChildNodeValue(theNode, "fontName"); // symbolissa k‰ytetyn fontin nimi
	itsFontColorStr = XmlHelper::ChildNodeValue(theNode, "fontColor"); // symbolin v‰ri R G B
	itsFontScaleFactorStr = XmlHelper::ChildNodeValue(theNode, "scaleFactor"); // suhteellinen fontin koko skaalaus luku
	itsSymbolCodeStr = XmlHelper::ChildNodeValue(theNode, "symbolCode"); // symbolin koodi annetussa fontissa
	itsGmlPosStr = XmlHelper::ChildNodeValue(theNode, "gml:pos"); // lat-lon positio

	InitDataFromRawStrings();
}

static std::string GetWomlMemberNameOrType(LPXNode theNode)
{
	static const std::string searchStr1 = "womlswo:"; // joillekin otuksille (mm. rintamat, symbolit, sadealueet, jetit, solat)
	static const std::string searchStr2 = "womlqty:"; // joillekin taas t‰m‰ (ainakin parametri-'laatikot' esim. l‰mpˆtila ja l‰mpˆtila v‰li) 
	
	if(theNode->childs.size())
	{
		std::string usedSearchStr = searchStr1;
		std::string childName = CT2A(theNode->childs[0]->name);
		std::string::size_type pos = childName.find(searchStr1);
		if(pos == std::string::npos)
		{
			usedSearchStr = searchStr2;
			pos = childName.find(searchStr2);
		}

		if(pos != std::string::npos)
		{
			std::string retValueStr(childName.begin() + pos + usedSearchStr.size(), childName.end());
			NFmiStringTools::LowerCase(retValueStr);
			return retValueStr;
		}
	}
	return "";
}

// WOML:issa ei n‰yt‰ olevan oikeastaan kuin pari kolme hyˆdyllist‰ tietoa:
// 1. Otuksen member-section 1. lapsi-noden nimess‰ on otuksen tyyppi "womlswo:"-kohdan per‰ss‰ (esim. womlswo:WarmFront).
// 2. Samaisessa esim. womlswo:WarmFront -kohdassa voi olla attribuuttina orientation="+" (mit‰ sitten + ja ehk‰ - tarkoittavatkaan)
// 3. gml:posList-nodessa on lon-lat parit
void ConceptualObjectData::InitializeFromWomlNode(LPXNode theNode)
{
/*
	itsPartTypeStr = ConceptualObjectData::AttributeValue(theNode, "type"); // conceptual tai symbol
	itsFrontNameStr = ConceptualObjectData::ChildNodeValue(theNode, "frontName"); // eri rintamat ja PLAIN on alue ja symbolilla ei ole mit‰‰n
	itsValidTimeStr = ConceptualObjectData::ChildNodeValue(theNode, "validTime");
	itsInfoTextStr = ConceptualObjectData::ChildNodeValue(theNode, "infoText");
	itsGmlPosListStr = ConceptualObjectData::ChildNodeValue(theNode, "gml:posList"); // olion koordinaatit lon-lat pareina
	itsOrientationStr = ConceptualObjectData::ChildNodeValue(theNode, "orientation"); // 1 / 2 (left/right?)
	itsAppearanceStr = ConceptualObjectData::ChildNodeValue(theNode, "appearance"); // viivan paksuus ja R G B
	itsRainphaseStr = ConceptualObjectData::ChildNodeValue(theNode, "rainphase"); // alueen sadetyyppi esim snow/sleet jne.
	itsFontNameStr = ConceptualObjectData::ChildNodeValue(theNode, "fontName"); // symbolissa k‰ytetyn fontin nimi
	itsFontColorStr = ConceptualObjectData::ChildNodeValue(theNode, "fontColor"); // symbolin v‰ri R G B
	itsFontScaleFactorStr = ConceptualObjectData::ChildNodeValue(theNode, "scaleFactor"); // suhteellinen fontin koko skaalaus luku
	itsSymbolCodeStr = ConceptualObjectData::ChildNodeValue(theNode, "symbolCode"); // symbolin koodi annetussa fontissa
*/
	itsWomlMemberNameStr = ::GetWomlMemberNameOrType(theNode);

    // Rintamille ja vastaaville viivoille halutaan gml:LineString -nodessa oleva gml:posList, eik‰ ennen niit‰ olevia bezier poslistoja
	itsGmlPosListStr = XmlHelper::ChildsChildNodeValue(theNode, "gml:LineString", "gml:posList"); // lat-lon positio
    if(itsGmlPosListStr.empty())
    { // jos ei lˆytynyt, koetetaan viel‰ hakea alueellisiin olioihin liittyv‰ gml:LinearRing -nodessa oleva gml:posList, eik‰ ennen niit‰ olevia bezier poslistoja
    	itsGmlPosListStr = XmlHelper::ChildsChildNodeValue(theNode, "gml:LinearRing", "gml:posList"); // lat-lon positio
    }
	itsGmlPosStr = XmlHelper::ChildNodeValue(theNode, "gml:pos"); // lat-lon positio
	itsWomlSymbolInfoStr = XmlHelper::ChildNodeValue(theNode, "gml:name");
    itsReferenceStr = NFmiStringTools::LowerCase(XmlHelper::ChildNodeValue(theNode, "womlqty:reference"));
    LPXNode childNode = XmlHelper::GetChildNode(theNode, 0);
    if(childNode)
    {
        itsOrientationStr = XmlHelper::AttributeValue(childNode, "orientation");
        itsSymbolCodeStr = XmlHelper::ChildsChildNodeValue(theNode, "womlswo:MeteorologicalSymbol", "womlswo:definitionReference"); // toinen symboli nimike, mik‰ lˆytyy kaikista PointMeteorologicalSymbol -otuksista
    }

	InitDataFromWomlStrings(theNode);
}

// jouduin lis‰‰m‰‰n fLonLatOrder -parametrin koska woml:issa on n‰emm‰ vaihdettu lat-lon j‰rjestykseen vanhan lon-lat j‰rjestyksen sijasta.
void ConceptualObjectData::InitLatlonValues(bool fLonLatOrder)
{
    // Siivotaan itsGmlPosListStr -dataosio, koska Mirwan Woml:iin oli tullut space stringin per‰‰n, se johti NFmiStringTools::Split -funktiossa poikkeuksen heittoon.
    NFmiStringTools::TrimR(itsGmlPosListStr, ' ');
	// t‰h‰n tulee lon-lat pareja
	vector<double> coordinatesVector = NFmiStringTools::Split<vector<double> >(itsGmlPosListStr, " ");
	size_t latlonSize = coordinatesVector.size()/2;
	itsLatlonPoints.clear();
	for(size_t i = 0; i<latlonSize; i++)
	{
		if(fLonLatOrder)
			itsLatlonPoints.push_back(NFmiPoint(coordinatesVector[i*2], coordinatesVector[i*2+1]));
		else
			itsLatlonPoints.push_back(NFmiPoint(coordinatesVector[i*2+1], coordinatesVector[i*2]));
	}
}

static NFmiPoint GetLatlonPointFromString(const std::string &theLatlonStr)
{
	if(theLatlonStr.empty())
        return NFmiPoint::gMissingLatlon;
    else
	{
		vector<float> symbolLatlonVec = NFmiStringTools::Split<vector<float> >(theLatlonStr, " ");
		if(symbolLatlonVec.size() != 2)
			throw runtime_error("Error in conceptual model data, gml:pos was illegal.");
		return NFmiPoint(symbolLatlonVec[1], symbolLatlonVec[0]); // WOML:issa on lon-lat j‰rjestys
	}
}

void ConceptualObjectData::InitDataFromWomlStrings(LPXNode theNode)
{
	fConceptualType = true;
	InitLatlonValues(false); // lat-lon j‰rjestys
	fLeft = false;
	if(itsOrientationStr == std::string("+"))
		fLeft = true;

	if(itsPixelSizeInMM == 0)
		itsPixelSizeInMM = 0.3;

	if(itsGmlPosStr.empty() == false)
        itsSymbolLatlon = ::GetLatlonPointFromString(itsGmlPosStr);

	SetDataFromObjectType(theNode);
}

static float GetParamValue(std::string theParamValueStr)
{
    if(theParamValueStr.empty())
        return kFloatMissing;
    else if(NFmiStringTools::LowerCase(theParamValueStr) == "nan")
        return kFloatMissing;
    else
    {
        float value = NFmiStringTools::Convert<float>(theParamValueStr);
        value = static_cast<float>(FmiRound(value));
        return value;
    }
}

static std::string GetParamValueStr(float theParamValue)
{
    if(theParamValue == kFloatMissing)
        return "-";
    else
        return NFmiStringTools::Convert<float>(theParamValue);
}

static float GetWindDir(const std::string &windDirStr)
{
    if(windDirStr == "n")
        return 360.f;
    else if(windDirStr == "e")
        return 90.f;
    else if(windDirStr == "s")
        return 180.f;
    else if(windDirStr == "w")
        return 270.f;
    else if(windDirStr == "e")
        return 90.f;
    else if(windDirStr == "ne")
        return 45.f;
    else if(windDirStr == "se")
        return 135.f;
    else if(windDirStr == "sw")
        return 225.f;
    else if(windDirStr == "nw")
        return 315.f;
    else if(windDirStr == "nne")
        return 22.5f;
    else if(windDirStr == "ene")
        return 67.5f;
    else if(windDirStr == "ese")
        return 112.5f;
    else if(windDirStr == "sse")
        return 157.5f;
    else if(windDirStr == "ssw")
        return 202.5f;
    else if(windDirStr == "wsw")
        return 247.5f;
    else if(windDirStr == "wnw")
        return 292.5f;
    else if(windDirStr == "nnw")
        return 337.5f;
    else
        return kFloatMissing;
}

// Asetetaan arvot seuraaville dataosille:
// itsParameterValueStr jos space, "xx" tai "xx...yy"
// itsParameterValue joko miss, arvo tai v‰lin ala-raja
// itsParameterValue2 joko miss tai v‰lin yl‰-raja
void ConceptualObjectData::CalcParameterValues(LPXNode theNode)
{
    itsParameterValueStr = "";
    itsParameterValue = kFloatMissing;
    itsParameterValue2 = kFloatMissing;
	std::string paramValueStr;
	itsFontColor = NFmiColor(1,0,0); // tehd‰‰n aluksi kaikista punaisia
	if(theNode)
	{
        std::string parValueStr = XmlHelper::ChildNodeValue(theNode, "womlqty:numericalValue"); // yksitt‰inen parametri arvo
        itsParameterValue = ::GetParamValue(parValueStr);
		if(parValueStr.empty())
		{
            std::string lowerLimitValueStr = XmlHelper::ChildNodeValue(theNode, "womlqty:numericalValueLowerLimit"); // parametrin ala-arvo
            itsParameterValue = ::GetParamValue(lowerLimitValueStr);
            std::string upperLimitValueStr = XmlHelper::ChildNodeValue(theNode, "womlqty:numericalValueUpperLimit"); // parametrin yl‰-arvo
            itsParameterValue2 = ::GetParamValue(upperLimitValueStr);

            itsParameterValueStr = ::GetParamValueStr(itsParameterValue);
			itsParameterValueStr += g_RangeSplitterString;
			itsParameterValueStr += ::GetParamValueStr(itsParameterValue2);
            if(itsParameterValue < 0 && itsParameterValue2 < 0)
            	itsFontColor = NFmiColor(0,0,1); // tehd‰‰n negatiivisista arvoista sinisi‰, HUOM! en jaa osiin piirtoa negatiiviselle ja positiiviselle v‰lille, vaan kaikki piirret‰‰n aina samalla v‰rill‰
                                                // Eli jos molemmat on negatiivisia, silloin sininen v‰ritys, muuten aina punainen
		}
        else
        {
            itsParameterValueStr = ::GetParamValueStr(itsParameterValue);
            if(itsParameterValue < 0)
            	itsFontColor = NFmiColor(0,0,1); // tehd‰‰n negatiivisista arvoista sinisi‰
        }
	}

    if(itsParameterValueStr.empty())
	    itsParameterValueStr = " "; // jos oli tyhj‰ stringi, laitetaan sinne space, muuten printtaa itsSymbolCode-arvon ruudulle 
		    						// (t‰m‰ on virhe tilanne ja hanskaan sen nyt piirt‰m‰ll‰ ruudulle tyhj‰‰)

    const std::string windReferenceSearchStr = "wind@";
    if(itsReferenceStr.find(windReferenceSearchStr) != std::string::npos)
    { // kyse oli tuulinuolesta
        fIsWindArrow = true;
		itsFontScaleFactor = 0.6f; // tehd‰‰n tuuli tekstist‰ pient‰
        std::vector<std::string> parts = NFmiStringTools::Split(itsReferenceStr, "@");
        if(parts.size() != 2)
            throw std::runtime_error("Error in ConceptualObjectData::CalcParameterValues: reference string was ilformatted, should be Wind@XXX");
        else
        {
            std::string windDirStr = NFmiStringTools::LowerCase(XmlHelper::ChildNodeValue(theNode, "gml:CompassPoint"));
            itsParameterValue2 = ::GetWindDir(windDirStr); // tuulinuolien tapauksessa mahd. nopeus talletetaan itsParameterValue:seen ja suunta itsParameterValue2:seen
            std::string windArrowTypeStr = parts[1];
            if(windArrowTypeStr == "seaflowwind")
                itsConceptualColor = NFmiColor(0.04f, 0.04f, 0.04f); // musta
            else if(windArrowTypeStr == "groundflowwind")
                itsConceptualColor = NFmiColor(0.8f,0.8f,0.8f); // harmaa
            else if(windArrowTypeStr == "warmflowwind")
                itsConceptualColor = NFmiColor(0.98f, 0.04f, 0.04f); // punainen
            else if(windArrowTypeStr == "coldflowwind")
                itsConceptualColor = NFmiColor(0.04f ,0.51f, 0.8f); // sininen
            else if(windArrowTypeStr == "flowwind")
                itsConceptualColor = NFmiColor(0.49f ,0.06f ,0.9f); // violetti
        }
    }
}

static std::string GetRainPhaseSymbolKey(const std::string &theRainPhaseStr)
{
    if(theRainPhaseStr.empty())
        return "";
    else if(theRainPhaseStr == "rain")
        return "PresentWeather@60";
    else if(theRainPhaseStr == "snow")
        return "PresentWeather@70";
    else if(theRainPhaseStr == "fog")
        return "PresentWeather@45";
    else if(theRainPhaseStr == "sleet")
        return "PresentWeather@68";
    else if(theRainPhaseStr == "hail")
        return "PresentWeather@85";
    else if(theRainPhaseStr == "freezing-precipitation")
        return "PresentWeather@66";
    else if(theRainPhaseStr == "drizzle")
        return "PresentWeather@50";
    else if(theRainPhaseStr == "mixed")
        return "PresentWeather@69";
    else if(theRainPhaseStr == "unknown")
        return "";
    else
        return "";
}

static NFmiPoint CalcAreaCenter(std::vector<NFmiPoint> &theLatlonPoints)
{
    if(theLatlonPoints.size())
    {
        double lonSum = 0;
        double latSum = 0;
        for(size_t i = 0; i < theLatlonPoints.size(); i++)
        {
            lonSum += theLatlonPoints[i].X();
            latSum += theLatlonPoints[i].Y();
        }
        return NFmiPoint(lonSum / theLatlonPoints.size(), latSum / theLatlonPoints.size());
    }
    return NFmiPoint::gMissingLatlon;
}

        
static NFmiColor GetWarningAreaColor(const std::string &theReferenceStr)
{
    const float alpha = 0.7f;
    if(theReferenceStr == "warninglevel@level-1")
        return NFmiColor(0.f, 0.7f, 0.f, alpha); // vaalean vihre‰
    else if(theReferenceStr == "warninglevel@level-2")
        return NFmiColor(0.99f, 0.97f, 0.12f, alpha); // keltainen
    else if(theReferenceStr == "warninglevel@level-3")
        return NFmiColor(0.99f, 0.55f, 0.12f, alpha); // oranssi
    else if(theReferenceStr == "warninglevel@level-4")
        return NFmiColor(0.97f, 0.27f, 0.21f, alpha); // punainen
    else
        return NFmiColor(0.1f, 0.8f, 0.1f, alpha); // vaalean vihre‰
}

void ConceptualObjectData::SetDataFromObjectType(LPXNode theNode)
{
	double usedLineWidthInMM = 1.2;
	if(itsWomlMemberNameStr == "warmfront") // muista vertailu sanojen pit‰‰ olla lower casessa!
	{
		fWomlOk = true;
		itsFrontNameStr = "WARMFRONT";
		fConceptualType = true; // true jos on conceptual ja false jos symbol type
		fPlainArea = false; // onko kyseess‰ alueesta (sadealue tms.)
		itsLineWidthInMM = usedLineWidthInMM;
		itsConceptualColor = NFmiColor(0.96f, 0.2f, 0.04f);
	}
	else if(itsWomlMemberNameStr == "coldfront") // muista vertailu sanojen pit‰‰ olla lower casessa!
	{
		fWomlOk = true;
		itsFrontNameStr = "COLDFRONT";
		fConceptualType = true; // true jos on conceptual ja false jos symbol type
		fPlainArea = false; // onko kyseess‰ alueesta (sadealue tms.)
		itsLineWidthInMM = usedLineWidthInMM;
		itsConceptualColor = NFmiColor(0.06f, 0.57f, 1.f);
	}
	else if(itsWomlMemberNameStr == "occludedfront") // muista vertailu sanojen pit‰‰ olla lower casessa!
	{
		fWomlOk = true;
		itsFrontNameStr = "OCCLUSION";
		fConceptualType = true; // true jos on conceptual ja false jos symbol type
		fPlainArea = false; // onko kyseess‰ alueesta (sadealue tms.)
		itsLineWidthInMM = usedLineWidthInMM;
		itsConceptualColor = NFmiColor(1.f, 0.f, 1.f);
	}
	else if(itsWomlMemberNameStr == "trough") // muista vertailu sanojen pit‰‰ olla lower casessa!
	{
		fWomlOk = true;
		itsFrontNameStr = "TROUGH";
		fConceptualType = true; // true jos on conceptual ja false jos symbol type
		fPlainArea = false; // onko kyseess‰ alueesta (sadealue tms.)
		itsLineWidthInMM = usedLineWidthInMM / 1.6;
		itsConceptualColor = NFmiColor(0.6f, 0.f, 0.47f);
	}
	else if(itsWomlMemberNameStr == "uppertrough") // muista vertailu sanojen pit‰‰ olla lower casessa!
	{
		fWomlOk = true;
		itsFrontNameStr = "UPPER_TROUGH";
		fConceptualType = true; // true jos on conceptual ja false jos symbol type
		fPlainArea = false; // onko kyseess‰ alueesta (sadealue tms.)
		itsLineWidthInMM = usedLineWidthInMM / 1.6;
		itsConceptualColor = NFmiColor(0.06f, 0.57f, 1.f);
	}
	else if(itsWomlMemberNameStr == "jetstream") // muista vertailu sanojen pit‰‰ olla lower casessa!
	{
		fWomlOk = true;
		itsFrontNameStr = "JET_ARROW";
		fConceptualType = true; // true jos on conceptual ja false jos symbol type
		fPlainArea = false; // onko kyseess‰ alueesta (sadealue tms.)
		itsLineWidthInMM = usedLineWidthInMM / 2.0;
		itsConceptualColor = NFmiColor(1.f, 0.f, 1.f);
	}
	else if(itsWomlMemberNameStr == "surfaceprecipitationarea") // muista vertailu sanojen pit‰‰ olla lower casessa!
	{
		fWomlOk = true;
		fConceptualType = true; // true jos on conceptual ja false jos symbol type
		fPlainArea = true; // onko kyseess‰ alueesta (sadealue tms.)
		itsLineWidthInMM = usedLineWidthInMM;
		itsConceptualColor = NFmiColor(0.f, 0.7f, 0.f, 0.5f); // tehd‰‰n vihre‰ kun ei ole tietoa v‰rist‰ (ja 50 % alpha)
        itsFillHatchPattern = HatchStyle30Percent;

        std::string rainPhaseStr = NFmiStringTools::LowerCase(XmlHelper::ChildNodeValue(theNode, "womlswo:rainPhase"));
        itsSymbolCodeStr = ::GetRainPhaseSymbolKey(rainPhaseStr);
        if(!itsSymbolCodeStr.empty())
        {
            // sadealueella oli faasi, pit‰‰ laskea yhdelle symbolille paikka, otetaan alueen keskipiste
            itsSymbolLatlon = ::CalcAreaCenter(itsLatlonPoints);
        }
	}
	else if(itsWomlMemberNameStr == "parametervaluesetarea") // muista vertailu sanojen pit‰‰ olla lower casessa!
    {
		fWomlOk = true;
		fConceptualType = true; // true jos on conceptual ja false jos symbol type
		fPlainArea = true; // onko kyseess‰ alueesta (sadealue tms.)
		itsLineWidthInMM = usedLineWidthInMM;
        itsConceptualColor = ::GetWarningAreaColor(itsReferenceStr);

        std::string categoryStr = XmlHelper::ChildNodeValue(theNode, "womlqty:category");
        if(!categoryStr.empty() && categoryStr != "Category@Unknow")
        {
            itsSymbolCodeStr = categoryStr;
            // varoitusalueella oli kategoria, pit‰‰ laskea yhdelle symbolille paikka, otetaan alueen keskipiste
            itsSymbolLatlon = ::CalcAreaCenter(itsLatlonPoints);
        }
    }
	else if(itsWomlMemberNameStr == "cloudarea") // muista vertailu sanojen pit‰‰ olla lower casessa!
	{
		fWomlOk = true;
		itsFrontNameStr = "CLOUDAREA";
		fConceptualType = true; // true jos on conceptual ja false jos symbol type
		fPlainArea = true; // onko kyseess‰ alueesta (sadealue tms.)
		itsLineWidthInMM = usedLineWidthInMM;
		itsConceptualColor = NFmiColor(0.6f, 0.6f, 0.6f, 0.3f); // tehd‰‰n vihre‰ kun ei ole tietoa v‰rist‰ (ja 70 % alpha)
	}
	else if(itsWomlMemberNameStr == "pointmeteorologicalsymbol") // muista vertailu sanojen pit‰‰ olla lower casessa!
	{
		fWomlOk = true;
		fConceptualType = false; // true jos on conceptual ja false jos symbol type
		fPlainArea = false; // onko kyseess‰ alueesta (sadealue tms.)
	}
	else if(itsWomlMemberNameStr == "parametervaluesetpoint") // muista vertailu sanojen pit‰‰ olla lower casessa!
	{
		fWomlOk = true;
		fConceptualType = false; // true jos on conceptual ja false jos symbol type
		fPlainArea = false; // onko kyseess‰ alueesta (sadealue tms.)

		itsFontNameStr = "arial";
		itsFontScaleFactor = 0.8f; // tehd‰‰n kaikista aluksi jonkun kokoisia, koska ei ole tietoa
        CalcParameterValues(theNode);
	}
	else if(itsWomlMemberNameStr == "lowpressurecenter" || itsWomlMemberNameStr == "polarlow" || itsWomlMemberNameStr == "highpressurecenter") // muista vertailu sanojen pit‰‰ olla lower casessa!
	{
		fWomlOk = true;
		fConceptualType = false; // true jos on conceptual ja false jos symbol type
		fPlainArea = false; // onko kyseess‰ alueesta (sadealue tms.)
		itsFontNameStr = "mirri"; // jos t‰ll‰ on arvo, k‰ytet‰‰n symboli piirrossa fontti piirtoa
		itsFontScaleFactor = 1.6f; // tehd‰‰n kaikista aluksi jonkun kokoisia, koska ei ole tietoa
        if(itsWomlMemberNameStr == "lowpressurecenter")
        {
		    itsFontColor = NFmiColor(1,0,0); // punainen
            itsSymbolCode = 53;
        }
        else if(itsWomlMemberNameStr == "polarlow")
        {
		    itsFontColor = NFmiColor(1,0,0); // punainen
            itsSymbolCode = 56;
        }
        else if(itsWomlMemberNameStr == "highpressurecenter")
        {
		    itsFontColor = NFmiColor(0,0,1); // sininen
            itsSymbolCode = 49;
        }
	}
	
	
}

void ConceptualObjectData::InitDataFromRawStrings(void)
{
	fConceptualType = boost::iequals(itsPartTypeStr, std::string("conceptual"));

	// itsValidTime = <- itsValidTimeStr ??? // T‰m‰ on jo tiedosssa, ei tulkita sit‰ ainakaan nyt
	InitLatlonValues(true); // lon-lat j‰rjestys

	fLeft = false;
	if(itsOrientationStr == std::string("2"))
		fLeft = true;

	// itsAppearanceStr sis‰lt‰‰ 4 lukua: viivan paksuus (n‰yttˆruudun pikseleiss‰) ja R G B
	// Pit‰‰ olettaa ett‰ yhden pikselin koko ruudulla on jotain, ett‰ voidaan laskea viivan paksuus millimetreiss‰.
	// Otetaan se, kun ei printata, GraphicalInfosta ja annetaan t‰nne.
	if(itsPixelSizeInMM == 0)
		itsPixelSizeInMM = 0.3;

	fPlainArea = false;
	if(fConceptualType)
	{
		if(boost::iequals(itsFrontNameStr, "plain") == false)
		{
			vector<float> appearanceVec = NFmiStringTools::Split<vector<float> >(itsAppearanceStr, " ");
			size_t appearanceSize = appearanceVec.size();
			const int normalFrontalAppearanceSize = 4;

			if(appearanceSize == normalFrontalAppearanceSize)
			{
				itsLineWidthInMM = itsPixelSizeInMM * appearanceVec[0];
				itsConceptualColor = NFmiColor(appearanceVec[normalFrontalAppearanceSize-3]/255.f, appearanceVec[normalFrontalAppearanceSize-2]/255.f, appearanceVec[normalFrontalAppearanceSize-1]/255.f);
			}
			else // yl‰solalla on jotain uusia appearance tietoja joista en tied‰ mit‰ neovat
			{
				int upperTroughAppSize = static_cast<int>(appearanceVec.size());
				itsLineWidthInMM = itsPixelSizeInMM * appearanceVec[0];
				itsConceptualColor = NFmiColor(appearanceVec[upperTroughAppSize-3]/255.f, appearanceVec[upperTroughAppSize-2]/255.f, appearanceVec[upperTroughAppSize-1]/255.f);
			}
		}
		else
		{
			fPlainArea = true;
			// N‰ihin loppuihin laitetaan alphaksi 0.5
			float usedAlpha = 0.5f;
			if(boost::iequals(itsRainphaseStr, "water"))
				itsConceptualColor = NFmiColor(0.3f, 0.9f, 0.3f, usedAlpha);
			else if(boost::iequals(itsRainphaseStr, "water_with_rain_symbol"))
				itsConceptualColor = NFmiColor(0.3f, 0.9f, 0.3f, usedAlpha);
			else if(boost::iequals(itsRainphaseStr, "sleet"))
				itsConceptualColor = NFmiColor(0.35f, 0.92f, 0.35f, usedAlpha);
			else if(boost::iequals(itsRainphaseStr, "snow"))
				itsConceptualColor = NFmiColor(0.20f, 0.75f, 0.20f, usedAlpha);
			else if(boost::iequals(itsRainphaseStr, "fog"))
				itsConceptualColor = NFmiColor(1.f, 1.f, 0.6f, usedAlpha);
		}
	}
	else
	{
		vector<float> fontColorVec = NFmiStringTools::Split<vector<float> >(itsFontColorStr, " ");
		if(fontColorVec.size() < 3)
			throw runtime_error("Error in conceptual model data, fontColor was illegal.");
		itsFontColor = NFmiColor(fontColorVec[0]/255.f, fontColorVec[1]/255.f, fontColorVec[2]/255.f);
		itsSymbolCode = 0;
		try
		{
			itsSymbolCode = NFmiStringTools::Convert<int>(itsSymbolCodeStr);
		}
		catch(...)
		{
		}

		itsFontScaleFactor = NFmiStringTools::Convert<float>(itsFontScaleFactorStr);

		vector<float> symbolLatlonVec = NFmiStringTools::Split<vector<float> >(itsGmlPosStr, " ");
		if(symbolLatlonVec.size() != 2)
			throw runtime_error("Error in conceptual model data, gml:pos was illegal.");
		itsSymbolLatlon = NFmiPoint(symbolLatlonVec[0], symbolLatlonVec[1]);
	}

}

// -----------------------------------------------------------------------------------------

NFmiImageMap NFmiConceptualDataView::itsMirwaSymbolMap;

NFmiConceptualDataView::NFmiConceptualDataView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
											,NFmiToolBox *theToolBox
											,boost::shared_ptr<NFmiDrawParam> &theDrawParam
											,FmiParameterName theParamId
											,int theRowIndex
                                            ,int theColumnIndex)
: NFmiStationView(theMapViewDescTopIndex, theArea
			  ,theToolBox
			  ,theDrawParam
			  ,theParamId
			  ,NFmiPoint(0, 0)
			  ,NFmiPoint(1, 1)
			  ,theRowIndex
              ,theColumnIndex)
,itsRowIndex(theRowIndex)
,itsConceptualObjectDatas()
,itsScreenPixelSizeInMM(0)
{
}

NFmiConceptualDataView::~NFmiConceptualDataView(void)
{
}

static NFmiMetTime gMissingTime(1900,1,1,0,0);

static std::string GetWantedTimeConceptualURLStr(CtrlViewDocumentInterface *theCtrlViewDocumentInterface, const NFmiMetTime &theValidTime, const std::string &theUserStr, const std::string &theBaseUrl)
{
// http://kopla.fmi.fi:8180/mirri-file-storage-servlet-0.2-SNAPSHOT/FileStorage?operation=listfiles&storageschemeid=conceptualmodel&user=meteor&validtimelower=20080102000000&subtype=gml
//	string wantedParams = "operation=listfiles&storageschemeid=conceptualmodel&user=*&validtimelower=20080120000000";
//	string wantedParams = "operation=getfile&storageschemeid=conceptualmodel&subtype=gml&user=pietarin&validtime=20080122110000&storagetime=20080122103217";
	string wantedParams;
	wantedParams += "operation=listfiles&storageschemeid=conceptualmodel";
	wantedParams += "&user=";
	wantedParams += theUserStr;
	wantedParams += "&validtimelower=";
	std::string validTimeStr = theValidTime.ToStr("YYYYMMDDHHmmSS");;
	wantedParams += validTimeStr;
	wantedParams += "&validtimeupper=";
	wantedParams += validTimeStr;

	std::string wantedUrlStr; // t‰m‰ palutetaan funktiosta
	int x = 0;
	try
	{
		std::string usedGetStr = theBaseUrl + "?" + wantedParams;
		std::string resultStr;
        CatLog::logMessage("ConceptualDataView: http request to get conceptual data.", CatLog::Severity::Debug, CatLog::Category::NetRequest);
        theCtrlViewDocumentInterface->MakeHTTPRequest(usedGetStr, resultStr, true);
		if(resultStr.empty() == false)
		{
            CString sxmlU_(CA2T(resultStr.c_str()));
			XNode xmlRoot;
            if(xmlRoot.Load(sxmlU_) == false)
				throw std::runtime_error(std::string("GetWantedTimeConceptualURLStr - xmlRoot.Load(sxml) failed for string: \n") + resultStr);

			XNodes fileNodes = xmlRoot.GetChilds(_TEXT("file")); 

			if(fileNodes.size() > 0)
			{
				const size_t missingTimeIndex = 999999999;
				NFmiStaticTime latestStorageTime = gMissingTime;
				size_t latestStorageTimeIndex = missingTimeIndex;
				std::string wantedSubtypeStr("gml");
				for(size_t i = 0; i<fileNodes.size(); i++)
				{
					LPXNode node1 = fileNodes[i];

                    string subtypeStr = CT2A(node1->GetAttrValue(_TEXT("subtype")));
					NFmiStringTools::LowerCase(subtypeStr);
					if(subtypeStr == wantedSubtypeStr)
					{
                        string storagetimeStr = CT2A(node1->GetAttrValue(_TEXT("storagetime")));
						NFmiStaticTime aValidTime = gMissingTime; // k‰ytet‰‰n static-time, ett‰ tulee sekunnitkin mukaan
						aValidTime.FromStr(storagetimeStr, kYYYYMMDDHHMMSS);
						if(latestStorageTime == gMissingTime || latestStorageTime < aValidTime)
						{
							latestStorageTime = aValidTime;
							latestStorageTimeIndex = i;
						}
					}
				}
				if(latestStorageTimeIndex != missingTimeIndex)
                    wantedUrlStr = CT2A(fileNodes[latestStorageTimeIndex]->GetAttrValue(_TEXT("fetchurl")));
			}
		}
	}
	catch(std::exception & e)
	{
		x = 1;
		string errStr = e.what();
	}
	catch(...)
	{
		x = 2;
	}
	
	return wantedUrlStr;
}

static wchar_t ConvertIntCharCodeToWChar(int theCharCode)
{
	char chArr[1];
	chArr[0] = static_cast<char>(theCharCode);
	wchar_t wchArr[1];
	mbtowc(wchArr, chArr, 1);
	return wchArr[0];
}

// t‰m‰ on yleis haku funktio, joka haarautuu konfiguraatioiden mukaisesti erilaisiin hakuihin
void NFmiConceptualDataView::GetConceptualData(const NFmiMetTime &theTime)
{
    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, __FUNCTION__);
    itsConceptualObjectDatas.clear(); // tyhjennet‰‰n ja luetaan datat tietokannasta tai tiedostosta
	if(itsCtrlViewDocumentInterface->ConceptualModelData().UseFileData())
		GetConceptualsFromFile(theTime);
	else
		GetConceptualsFromDB(theTime);
}

void NFmiConceptualDataView::GetConceptualsFromFile(const NFmiMetTime &theTime)
{
	string resultStr;
	GetConceptualsDataStrFromFile(theTime, resultStr);
	DecodeConceptualDataFromStr(resultStr);
}

void NFmiConceptualDataView::GetConceptualDataStrFromDB(std::string &url, std::string &theResultStr)
{
	std::string startingRequestLogStr = "ConceptualDataView starting request: ";
	startingRequestLogStr += url;
	CatLog::logMessage(startingRequestLogStr, CatLog::Severity::Debug, CatLog::Category::NetRequest);
    NFmiMilliSecondTimer timer;
    timer.StartTimer();
    itsCtrlViewDocumentInterface->MakeHTTPRequest(url, theResultStr, true);
    timer.StopTimer();

    std::string logStr = "ConceptualDataView: http request to get conceptual data from DB.";
    logStr += " Reading lasted: ";
    logStr += NFmiValueString::GetStringWithMaxDecimalsSmartWay(timer.TimeDiffInMSeconds() / 1000, 0);
    logStr += ".";
    logStr += NFmiValueString::GetStringWithMaxDecimalsSmartWay(timer.TimeDiffInMSeconds() % 1000, 0);
    logStr += " s.";
    CatLog::logMessage(logStr, CatLog::Severity::Debug, CatLog::Category::NetRequest);
}

void NFmiConceptualDataView::GetConceptualsDataStrFromDB(const NFmiMetTime &theTime, std::string &theResultStr)
{
    
	// esim. "http://kopla.fmi.fi:8180/mirri-file-storage-servlet-0.1-SNAPSHOT/FileStorage";
	string baseURL = itsCtrlViewDocumentInterface->ConceptualModelData().DataBaseUrl();
	std::string wantedUser = itsDrawParam->ParameterAbbreviation(); // esim. meteor
	std::string wantedUrlStr = ::GetWantedTimeConceptualURLStr(itsCtrlViewDocumentInterface, theTime, wantedUser, baseURL);
    
	//itsCtrlViewDocumentInterface->MakeHTTPRequest(wantedUrlStr, theResultStr, true);
    GetConceptualDataStrFromDB(wantedUrlStr, theResultStr);
}

void NFmiConceptualDataView::GetConceptualsDataStrFromWomlDB(const NFmiMetTime &theTime, std::string &theResultStr)
{
    // esim. http://kopla.fmi.fi:8282/fmi-common-woml-storage-unified-webapp-conceptualmodel/woml/xml/conceptualmodel/analysis/load/latest/byCreator/varosaa?validTime=2014-01-10T12:00:00Z
	string usedURL = itsCtrlViewDocumentInterface->ConceptualModelData().DataBaseUrl(); // DataBaseUrl ~ http://kopla.fmi.fi:8282
    if(usedURL.size() && usedURL[usedURL.size()-1] != '/')
        usedURL += '/';
    //usedURL += "fmi-common-woml-storage-unified-webapp-conceptualmodel/woml/xml/conceptualmodel/analysis/load/latest/byCreator/";
	std::string wantedUser = itsDrawParam->ParameterAbbreviation(); // esim. varosaa
    usedURL += wantedUser;
    usedURL += "?validTime=";
    // aika muotoa 2014-01-10T12:00:00Z
    NFmiString timeStr = theTime.ToStr("YYYY-MM-DDTHH:mm:SSZ", kEnglish);
    usedURL += timeStr;

    GetConceptualDataStrFromDB(usedURL, theResultStr);
    //itsCtrlViewDocumentInterface->MakeHTTPRequest(usedURL, theResultStr, true);
}

void NFmiConceptualDataView::GetConceptualsFromDB(const NFmiMetTime &theTime)
{
	string resultStr;
	if(itsCtrlViewDocumentInterface->ConceptualModelData().UseWOML())
    	GetConceptualsDataStrFromWomlDB(theTime, resultStr);
    else
    	GetConceptualsDataStrFromDB(theTime, resultStr);
	DecodeConceptualDataFromStr(resultStr);
}

void NFmiConceptualDataView::DecodeConceptualDataFromStr(const std::string &theConceptualDataStr)
{
	if(itsCtrlViewDocumentInterface->ConceptualModelData().UseWOML())
		DecodeConceptualWomlDataFromStr(theConceptualDataStr);
	else
		DecodeConceptualPreWomlDataFromStr(theConceptualDataStr);
}

void NFmiConceptualDataView::GetConceptualsDataStrFromFile(const NFmiMetTime &theTime, std::string &theResultStr)
{
	// 1. hae file-filtterill‰ tiedosto lista
	std::list<std::string> fileList = NFmiFileSystem::PatternFiles(itsCtrlViewDocumentInterface->ConceptualModelData().FileFilter());
	// 2. tee halutun ajan timestamp-string (tiedoston time stamp on joko YYYYMMDDHHmm- tai YYYYMMDDHHmmSS -muodossa, joten tehd‰‰n minuutin tarkkuudella oleva ja etsit‰‰n sit‰)
	std::string timeStamp = theTime.ToStr(kYYYYMMDDHHMM);
	// 3. hae file-listasta tiedosto nimi, jossa on haluttu time-stamp
	std::string wantedFileName;
	for(std::list<std::string>::iterator it = fileList.begin(); it != fileList.end(); ++it)
	{
		if(it->find(timeStamp) != std::string::npos)
		{
            if(it->find(itsDrawParam->ParameterAbbreviation()) != std::string::npos)
		    {
			    wantedFileName = *it;
			    break;
		    }
        }
	}
	// 4. lue halutun tiedoston sis‰ltˆ theResultStr-parametriin
	if(wantedFileName.empty() == false)
	{
		std::string totalFileName = NFmiFileSystem::PathFromPattern(itsCtrlViewDocumentInterface->ConceptualModelData().FileFilter());
		totalFileName += wantedFileName;
		NFmiFileSystem::ReadFile2String(totalFileName, theResultStr);
	}
}

void NFmiConceptualDataView::DecodeConceptualWomlDataFromStr(const std::string &theConceptualDataStr)
{
	if(theConceptualDataStr.empty() == false)
	{
        CString sxmlU_(CA2T(theConceptualDataStr.c_str()));
		XNode xmlRoot;
        if(xmlRoot.Load(sxmlU_) == false)
			throw std::runtime_error(std::string("NFmiStationViewHandler::DrawConceptuals - xmlRoot.Load(sxml) failed for string: \n") + theConceptualDataStr);

		XNodes partNodes = xmlRoot.GetChilds(_TEXT("womlcore:member")); 
		for(size_t j=0; j<partNodes.size(); j++)
		{
			LPXNode aNode = partNodes[j];

			ConceptualObjectData tmpData;
			tmpData.itsPixelSizeInMM = itsScreenPixelSizeInMM;
			try
			{
				tmpData.InitializeFromWomlNode(aNode);
				if(tmpData.fWomlOk)
					itsConceptualObjectDatas.push_back(tmpData);
			}
			catch(...)
			{
				// jos oli virheellinen node, ei lopeteta kuitenkaan
			}
		}
	}}

void NFmiConceptualDataView::DecodeConceptualPreWomlDataFromStr(const std::string &theConceptualDataStr)
{
	if(theConceptualDataStr.empty() == false)
	{
        CString sxmlU_(CA2T(theConceptualDataStr.c_str()));
		XNode xmlRoot;
        if(xmlRoot.Load(sxmlU_) == false)
			throw std::runtime_error(std::string("NFmiStationViewHandler::DrawConceptuals - xmlRoot.Load(sxml) failed for string: \n") + theConceptualDataStr);

		XNodes partNodes = xmlRoot.GetChilds(_TEXT("part")); 
		for(size_t j=0; j<partNodes.size(); j++)
		{
			LPXNode aNode = partNodes[j];

			ConceptualObjectData tmpData;
			tmpData.itsPixelSizeInMM = itsScreenPixelSizeInMM;
			try
			{
				tmpData.InitializeFromPartNode(aNode);
				itsConceptualObjectDatas.push_back(tmpData);
			}
			catch(...)
			{
				// jos oli virheellinen node, ei lopeteta kuitenkaan
			}
		}
	}
}

void NFmiConceptualDataView::Draw(NFmiToolBox *theGTB)
{
    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, __FUNCTION__);
    if(!theGTB)
		return;
	itsToolBox = theGTB;
	if(!IsParamDrawn())
		return;
	if(itsCtrlViewDocumentInterface->ConceptualModelData().Use() == false)
		return;

	if(itsToolBox->GetDC()->IsPrinting() == FALSE)
		itsScreenPixelSizeInMM = 1./ itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_x;

	try
	{
		InitializeGdiplus(itsToolBox, &GetFrame());
		itsGdiPlusGraphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias); // Huom. antialiasointi saattaa hidastaa yll‰tt‰v‰n paljon piirtoa
		GetConceptualData(itsTime);
		for(size_t j=0; j<itsConceptualObjectDatas.size(); j++)
			DrawConseptualData(itsConceptualObjectDatas[j]);
	}
	catch(...)
	{
	}
	CleanGdiplus(); // t‰t‰ pit‰‰ kutsua piirron lopuksi InitializeGdiplus -metodin vastin parina.
}

void NFmiConceptualDataView::DrawConseptualData(ConceptualObjectData &theData)
{ 
	if(theData.fConceptualType)
		DrawConseptualObject(theData);
	else
		DrawSymbol(theData);
}

static double CalcAreaSizeFactor(const boost::shared_ptr<NFmiArea> &theArea)
{
	// suomi kartan leveys metreiss‰ on 750 km
	// skandi kartan leveys metreiss‰ on 2000 km
	// euro kartan leveys metreiss‰ on 7000 km
	// maailma kartan leveys metreiss‰ on 40000 km
	// lasketaan n‰iden avulla joku kokeellinen skaalauskerroin eri kokoisille alueuille

	double widthInMeters = theArea->WorldRect().Width();
    // Jos leveys on alle 10000 km, lasketaan eri skaalalla (jykemm‰ll‰) kuin
	double factor = MathHelper::InterpolateWithTwoPoints(widthInMeters, 10000*1000, 1000*1000, 0.4, 1., 0.4, 1.2);
    if(widthInMeters > 10000 * 1000)
	    factor = MathHelper::InterpolateWithTwoPoints(widthInMeters, 40000*1000, 10000*1000, 0.25, 0.4, 0.3, 0.4);

	return factor;
}

// laske kerroin, mill‰ kaikkia k‰sitemalli oliota pinennet‰‰n/suurennetaan riippuen
// siit‰ kuinka suuri n‰yttˆ [mm] kulloinkin on k‰ytˆss‰.
float NFmiConceptualDataView::CalcScreenSizeFactor(CtrlViewDocumentInterface &theCtrlViewDocumentInterface, int theMapViewDesctopIndex)
{
	// yhden aliruudun koko pikseleiss‰ karttan‰ytˆll‰
	NFmiPoint viewSizeInPixels(theCtrlViewDocumentInterface.ActualMapBitmapSizeInPixels(theMapViewDesctopIndex));
    auto &graphicalInfo = theCtrlViewDocumentInterface.GetGraphicalInfo(theMapViewDesctopIndex);
	float widthInMM = static_cast<float>(viewSizeInPixels.X() / graphicalInfo.itsPixelsPerMM_x);
	float heightInMM = static_cast<float>(viewSizeInPixels.Y() / graphicalInfo.itsPixelsPerMM_y);
	float lenInMM = ::sqrt(widthInMM*widthInMM + heightInMM*heightInMM);

	// Haetaan sopiva kerroin kokeilemalla erilaisia ala ja yl‰ rajoja
	float factor = static_cast<float>(MathHelper::InterpolateWithTwoPoints(lenInMM, 50, 500, 0.3, 1.3, 0.2, 1.5));

	// lasketaan viel‰ toinen kerroin, joka riippuu valitusta maantieteellisest‰ alueesta
	float areaFactor = static_cast<float>(::CalcAreaSizeFactor(theCtrlViewDocumentInterface.GetMapHandlerInterface(theMapViewDesctopIndex)->Area()));
	factor = factor * areaFactor;

	return factor;
}

void NFmiConceptualDataView::DrawTextArea(ConceptualObjectData &theData)
{
	string usedStr = NFmiStringTools::UrlDecode(theData.itsInfoTextStr);

	double fontSizeInMM = 4.5; // mist‰ koko?
    fontSizeInMM *= CalcScreenSizeFactor(*itsCtrlViewDocumentInterface, itsMapViewDescTopIndex);

	Gdiplus::Color usedColor(CtrlView::NFmiColor2GdiplusColor(theData.itsFontColor));
	Gdiplus::SolidBrush aBrush(usedColor);
	PointF aPlace1(CtrlView::ConvertLatlonToGdiPlusPoint(this, theData.itsSymbolLatlon));

    auto &graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
	float boxHeightInPixels = static_cast<float>(1.4 * fontSizeInMM * graphicalInfo.itsPixelsPerMM_y);
	float boxWidthInPixels = static_cast<float>(0.8 * usedStr.size() * fontSizeInMM * graphicalInfo.itsPixelsPerMM_x);

	RectF boxRect(aPlace1.X, aPlace1.Y, boxWidthInPixels, boxHeightInPixels);
	boxRect.Offset(-boxRect.Width/2.f, -boxRect.Height/2.f);

	{ // t‰ysin l‰pin‰kyv‰‰n boxiin laitetaan hennot reunat
		float penWidthInPixels = static_cast<float>(1.f * 1.f/ graphicalInfo.itsPixelsPerMM_x);
		Pen aPen(CtrlView::NFmiColor2GdiplusColor(NFmiColor(0.6f, 0.6f, 0.6f, 0.8f)), penWidthInPixels);
		itsGdiPlusGraphics->DrawRectangle(&aPen, boxRect);
	}
	itsGdiPlusGraphics->FillRectangle(&aBrush, boxRect);

	NFmiColor fontColor(0, 0, 0);
	NFmiPoint aPlace2 = LatLonToViewPoint(theData.itsSymbolLatlon);
	double pixelsPerMM = graphicalInfo.itsPixelsPerMM_x;
	std::wstring fontNameStr(L"Arial");
    CtrlView::DrawTextToRelativeLocation(*itsGdiPlusGraphics, fontColor, fontSizeInMM, usedStr, aPlace2, pixelsPerMM, itsToolBox, fontNameStr, kCenter);
}

float NFmiConceptualDataView::CalcUsedFontSizeInPixels(float theWantedFontSizeInMM, CtrlViewDocumentInterface &theCtrlViewDocumentInterface, int theMapViewDesctopIndex, float theFontScaleFactor)
{
	// mahdollisten printtausten takia fontin koon kanssa pit‰‰ kikkailla
	if(theFontScaleFactor)
        theWantedFontSizeInMM *= theFontScaleFactor;
    theWantedFontSizeInMM *= CalcScreenSizeFactor(theCtrlViewDocumentInterface, theMapViewDesctopIndex);
    float usedFontSizeInPixels = static_cast<float>(theWantedFontSizeInMM * theCtrlViewDocumentInterface.GetGraphicalInfo(theMapViewDesctopIndex).itsPixelsPerMM_x);
    return usedFontSizeInPixels;
}

static double ScaleSymbolSizeBySymbolCodeName(const std::string &theSymbolCodeName)
{
    double scale = 1.0;
    if(theSymbolCodeName == "Warning@Hallaa" || theSymbolCodeName == "Warning@Frost")
        scale = 1.25;
    else if(theSymbolCodeName == "Warning@NightFrost")
        scale = 1.5;

    return scale;
}

// Piirt‰‰ mapatun Mirwa symbolin toivotun kokoisena
void NFmiConceptualDataView::DrawSymbol(ConceptualObjectData &theData, double wantedSymbolSizeInMM)
{
    bool printing = itsCtrlViewDocumentInterface->Printing();
	wantedSymbolSizeInMM *= CalcScreenSizeFactor(*itsCtrlViewDocumentInterface, itsMapViewDescTopIndex);
    wantedSymbolSizeInMM *= ::ScaleSymbolSizeBySymbolCodeName(theData.itsSymbolCodeStr);
    Gdiplus::Bitmap *symbolBitmap = NFmiConceptualDataView::itsMirwaSymbolMap.GetSymbolImage(theData.itsSymbolCodeStr, printing);
    NFmiRect symbolRect(CalcSymbolRelativeRect(theData.itsSymbolLatlon, wantedSymbolSizeInMM));
    CtrlView::DrawAnimationButton(symbolRect, symbolBitmap, itsGdiPlusGraphics, *itsToolBox, printing, itsCtrlViewDocumentInterface->MapViewSizeInPixels(itsMapViewDescTopIndex), 1.f, true);
}

static void DrawFinalRangeString(Gdiplus::Graphics *theGdiPlusGraphics, const Gdiplus::Font &theFont, FmiDirection theTextAlignment, const std::wstring &theSymbolStr, const Gdiplus::Color &theColor, const Gdiplus::PointF &thePlace)
{
    Gdiplus::StringFormat stringFormat;
    CtrlView::SetGdiplusAlignment(theTextAlignment, stringFormat);
    Gdiplus::SolidBrush aBrush(theColor);
    theGdiPlusGraphics->DrawString(theSymbolStr.c_str(), static_cast<INT>(theSymbolStr.size()), &theFont, thePlace, &stringFormat, &aBrush);
}

static NFmiColor TemperatureColor(float value, float usedAlpha)
{
    if(value == kFloatMissing)
        return NFmiColor(0.5f, 0.5f, 0.5f, usedAlpha);
    else if(value < 0)
        return NFmiColor(0, 0, 1, usedAlpha);
    else
        return NFmiColor(1, 0, 0, usedAlpha);
}

void NFmiConceptualDataView::DrawTemperatureRange(ConceptualObjectData &theData, const Gdiplus::PointF &thePlace, const Gdiplus::Font &theFont)
{
    std::vector<std::string> valuesStrVec = NFmiStringTools::Split(theData.itsParameterValueStr, g_RangeSplitterString);
    if(valuesStrVec.size() != 2)
    {
        std::string errorStr("Error in ");
        errorStr += __FUNCTION__;
        errorStr += ": Given range string '";
        errorStr += theData.itsParameterValueStr + "' was illegal.";
        throw std::runtime_error(errorStr);
    }

    float usedAlpha = theData.itsFontColor.Alpha();
    float xMoveInPixels = theFont.GetSize() * 0.25f; // kuinka paljon x-suunnassa siirret‰‰n alku ja loppu arvoja suhteessa fontin kokoon

    float leftValue = theData.itsParameterValue;
    std::string leftValueStr = valuesStrVec[0];
    float rightValue = theData.itsParameterValue2;
    std::string rightValueStr = valuesStrVec[1];

    if(leftValue < 0 && rightValue < 0)
    { // Henri Nymanin haluama Mirwa kikka: jos molemmat negatiivisia, laitetaan ne k‰‰nteiseen j‰rjestykseen.
        std::swap(leftValue, rightValue);
        std::swap(leftValueStr, rightValueStr);
    }

    // 1. Laitetaan rangen "..." stringi ensin: a) v‰ri=harmaa b) alignment=center c) paikka=originaali paikka d) fontti=normaali
    std::wstring symbolStr = CtrlView::StringToWString(g_RangeSplitterString);
    Gdiplus::Color usedColor(CtrlView::NFmiColor2GdiplusColor(NFmiColor(0.5, 0.5, 0.5, usedAlpha)));
    ::DrawFinalRangeString(itsGdiPlusGraphics, theFont, kCenter, symbolStr, usedColor, thePlace);

    // 2. Laitetaan rangen vasen stringi sitten: a) v‰ri=l‰mpˆtilasta riippuen b) alignment=right c) paikka=originaali paikka - dx d) fontti=normaali
    symbolStr = CtrlView::StringToWString(leftValueStr);
    usedColor = CtrlView::NFmiColor2GdiplusColor(::TemperatureColor(leftValue, usedAlpha));
    Gdiplus::PointF leftPlace(thePlace);
    leftPlace.X -= xMoveInPixels; // siirret‰‰n alku arvoa vasemmalle
    ::DrawFinalRangeString(itsGdiPlusGraphics, theFont, kRight, symbolStr, usedColor, leftPlace);

    // 3. Laitetaan rangen oikea stringi sitten: a) v‰ri=l‰mpˆtilasta riippuen b) alignment=left c) paikka=originaali paikka + dx d) fontti=normaali
    symbolStr = CtrlView::StringToWString(rightValueStr);
    usedColor = CtrlView::NFmiColor2GdiplusColor(::TemperatureColor(rightValue, usedAlpha));
    Gdiplus::PointF rightPlace(thePlace);
    rightPlace.X += xMoveInPixels; // siirret‰‰n alku arvoa vasemmalle
    ::DrawFinalRangeString(itsGdiPlusGraphics, theFont, kLeft, symbolStr, usedColor, rightPlace);
}

void NFmiConceptualDataView::DrawSymbol(ConceptualObjectData &theData)
{ 
	if(theData.itsPartTypeStr == "textarea")
		DrawTextArea(theData);
	else
	{
        if(theData.fIsWindArrow)
        {
            // Tuulinuolen rakentamista ei voi tehd‰ erillisess‰ funktiossa, koska Gdiplus::GraphicsPath-luokan
            // copy construktori ja sijoitus operaattori on protected tasoa.
            const float arrowHeadInnerBaseX = 0.3f;
            const float arrowHeadOuterBaseX = 0.65f;
            const float arrowHeadBaseY = 0.1f;
            const float arrowHeadTipY = 1.6f;
            const float arrowHeadTipX = 0.f;
            const float arrowTailX = 0.5f;
            const float arrowTailY = -1.2f;
            Gdiplus::PointF p1(-arrowHeadInnerBaseX, arrowHeadBaseY);
            Gdiplus::PointF p2(-arrowHeadOuterBaseX, arrowHeadBaseY);
            Gdiplus::PointF p3(arrowHeadTipX, arrowHeadTipY);
            Gdiplus::PointF p4(arrowHeadOuterBaseX, arrowHeadBaseY);
            Gdiplus::PointF p5(arrowHeadInnerBaseX, arrowHeadBaseY);
            Gdiplus::PointF p6(arrowTailX, arrowTailY);
            Gdiplus::PointF p7(-arrowTailX, arrowTailY);
            Gdiplus::GraphicsPath arrowPath;
            arrowPath.AddLine(p1, p2);
            arrowPath.AddLine(p2, p3);
            arrowPath.AddLine(p3, p4);
            arrowPath.AddLine(p4, p5);
            arrowPath.AddLine(p5, p6);
            arrowPath.AddLine(p6, p7);
            arrowPath.AddLine(p7, p1);

		    Gdiplus::PointF aPlace(CtrlView::ConvertLatlonToGdiPlusPoint(this, theData.itsSymbolLatlon));

            float angle = theData.itsParameterValue2 + 180;
		    NFmiAngle ang(itsArea->TrueNorthAzimuth(theData.itsSymbolLatlon));
		    angle += static_cast<float>(ang.Value()); // kartan pohjois suunta korjaus

    		Gdiplus::Matrix rotateMatrix;
            rotateMatrix.Rotate(angle);
            arrowPath.Transform(&rotateMatrix);

            float wantedSymbolSizeInMM = 12;
		    wantedSymbolSizeInMM *= CalcScreenSizeFactor(*itsCtrlViewDocumentInterface, itsMapViewDescTopIndex);
            float originalSymbolLengthInPixels = arrowHeadTipY - arrowTailY;
            float wantedPixelLength = static_cast<float>(wantedSymbolSizeInMM * itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_x);
            float scaleFactor = wantedPixelLength / originalSymbolLengthInPixels;
    		Gdiplus::Matrix scaleMatrix;
            scaleMatrix.Scale(scaleFactor, scaleFactor);
            arrowPath.Transform(&scaleMatrix);

    		Gdiplus::Matrix moveMatrix;
            moveMatrix.Translate(aPlace.X, aPlace.Y);
            arrowPath.Transform(&moveMatrix);

            CtrlView::DrawPath(*itsGdiPlusGraphics, arrowPath, NFmiColor(0,0,0), theData.itsConceptualColor, true, false, 0);
            if(theData.itsParameterValue != kFloatMissing)
            { // piirret‰‰n nuolen p‰‰lle nopeus arvo
                float usedFontSizeInPixels = CalcUsedFontSizeInPixels(6.f, *itsCtrlViewDocumentInterface, itsMapViewDescTopIndex, theData.itsFontScaleFactor);
                NFmiColor bkColor(0.5f, 0.5f, 0.5f);
    		    Gdiplus::PointF aPlace(CtrlView::ConvertLatlonToGdiPlusPoint(this, theData.itsSymbolLatlon));
                CtrlView::DrawSimpleText(*itsGdiPlusGraphics, NFmiColor(1,1,1), usedFontSizeInPixels, theData.itsParameterValueStr, NFmiPoint(aPlace.X, aPlace.Y), L"Arial", kCenter, Gdiplus::FontStyleRegular, &bkColor);
            }
        }
        else if(theData.itsFontNameStr.empty())
        {
            DrawSymbol(theData, 14);
        }
        else
        { // muutamat symbolit piirret‰‰n mirri fonteilla tai tavallisilla fonteilla (esim. Arial)
		    std::wstring fontNameStr = CtrlView::StringToWString(theData.itsFontNameStr);
            float usedFontSizeInPixels = CalcUsedFontSizeInPixels(10.f, *itsCtrlViewDocumentInterface, itsMapViewDescTopIndex, theData.itsFontScaleFactor);
            if(theData.itsFontNameStr == "mirri")
                usedFontSizeInPixels *= 1.5f;
		    Gdiplus::Font aFont(fontNameStr.c_str(), usedFontSizeInPixels, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

            Gdiplus::PointF aPlace(CtrlView::ConvertLatlonToGdiPlusPoint(this, theData.itsSymbolLatlon));

            if(theData.itsParameterValueStr.find(g_RangeSplitterString) != std::string::npos)
                DrawTemperatureRange(theData, aPlace, aFont);
            else
            {
                Gdiplus::StringFormat stringFormat;
                CtrlView::SetGdiplusAlignment(kCenter, stringFormat);

		        Gdiplus::Color usedColor(CtrlView::NFmiColor2GdiplusColor(theData.itsFontColor));
		        Gdiplus::SolidBrush aBrush(usedColor);

		        std::wstring symbolStr;
		        if(theData.itsParameterValueStr.empty())
			        symbolStr = ::ConvertIntCharCodeToWChar(theData.itsSymbolCode);
		        else
			        symbolStr = CtrlView::StringToWString(theData.itsParameterValueStr);

		        itsGdiPlusGraphics->DrawString(symbolStr.c_str(), static_cast<INT>(symbolStr.size()), &aFont, aPlace, &stringFormat, &aBrush);
            }
        }
	}
}

static void MinDistanceSeeker(const NFmiLocation &theLoc, const NFmiPoint &theLatlon, size_t currentIndex, double &minDistance, size_t &minDistIndex)
{
	double currDist = theLoc.Distance(theLatlon);
	if(currDist < minDistance)
	{
		minDistance = currDist;
		minDistIndex = currentIndex;
	}
}

static std::string GetLatlonTooltipString(const NFmiPoint &theLatlon)
{
    std::string str;
	str += "Lat: ";
	str += CtrlViewUtils::GetLatitudeMinuteStr(theLatlon.Y(), 1);
	str += " Lon: ";
	str += CtrlViewUtils::GetLongitudeMinuteStr(theLatlon.X(), 1);
    return str;
}

static std::string GetMirwaSymbolLocationTooltipString(const ConceptualObjectData &theObject)
{
    std::string str;
    if(theObject.itsSymbolLatlon != NFmiPoint::gMissingLatlon)
    {
        str += " (";
        str += ::GetLatlonTooltipString(theObject.itsSymbolLatlon);
        str += ")";
    }
    return str;
}
    			
static std::string MakeConceptualObjectInfoString(const ConceptualObjectData &theObject)
{
    std::string str;
    if(!theObject.itsParameterValueStr.empty())
    {
        str += theObject.itsParameterValueStr;
        str += ::GetMirwaSymbolLocationTooltipString(theObject);

        if(MacroParam::ci_find_substr(theObject.itsWomlSymbolInfoStr, std::string("wind")) != MacroParam::ci_string_not_found)
        {  // tuulisymbolit hanskataan pikkuisen erilailla
            str = ""; // tyhjennet‰‰n stringi ensin
            str += theObject.itsWomlSymbolInfoStr;
            str += " ";
            str += theObject.itsParameterValueStr;
            str += ::GetMirwaSymbolLocationTooltipString(theObject);
        }
    }
    else if(!theObject.itsSymbolCodeStr.empty())
    {
        str += theObject.itsSymbolCodeStr;
        str += ::GetMirwaSymbolLocationTooltipString(theObject);
    }
    else if(!theObject.itsReferenceStr.empty())
        str += theObject.itsReferenceStr;
    else
    {
        str += theObject.itsWomlMemberNameStr;
        str += ::GetMirwaSymbolLocationTooltipString(theObject);
    }
    return str;
}

std::string NFmiConceptualDataView::ComposeToolTipText(const NFmiPoint & theRelativePoint)
{
	// Ongelma: SmartMetin optimointi on johtanut siihen ett‰ k‰siteanalyysi oliot eiv‰t ole aina
	// tallessa kun tullaan pyyt‰m‰‰n tooltip arvoja. T‰ss‰ tarkistetaan ett‰ lˆytyykˆ niit‰
	// ollenkaan tai onko olemassa olevat oikealta ajalta. Jos ei, koetetaan hakea niit‰ t‰ss‰ ja nyt.
	try
	{
		if(itsConceptualObjectDatas.size() == 0 || (itsConceptualObjectDatas.size() > 0 && itsConceptualObjectDatas[0].itsValidTime != itsTime))
			GetConceptualData(itsTime); // tarkistetaan joska lˆytyisi k‰siteanalyysi dataa
	}
	catch(...)
	{
		// ei tehd‰ mit‰‰n, poikkeus heitet‰‰n mm. jos ei ole mit‰‰n dataa kyseiseen aikaan
	}

	string str;
	auto fontColor = CtrlViewUtils::GetParamTextColor(itsDrawParam->DataType(), itsDrawParam->UseArchiveModelData());
	if(itsConceptualObjectDatas.size())
	{

		// etsit‰‰n l‰hin k‰site-olio ja laitetaan sen info n‰kyviin
		NFmiPoint latlon = ViewPointToLatLon(theRelativePoint);
		NFmiLocation pointedLocation(latlon);
		double minDistance = 99999999;
		size_t minDistInd = static_cast<size_t>(-1);
		for(size_t i=0; i<itsConceptualObjectDatas.size(); i++)
		{
			ConceptualObjectData &tmpData = itsConceptualObjectDatas[i];
			if(tmpData.fConceptualType)
			{ // rintama/muu otus, miss‰ lat-lon vectori
				vector<NFmiPoint> &latlonVec = tmpData.itsLatlonPoints;
				for(size_t j=0; j<latlonVec.size(); j++)
				{
					::MinDistanceSeeker(pointedLocation, latlonVec[j], i, minDistance, minDistInd);
				}
			}
			else // symboli miss‰ vain yksi piste
				::MinDistanceSeeker(pointedLocation, tmpData.itsSymbolLatlon, i, minDistance, minDistInd);
		}

		
		double maxDistInMetres = itsArea->WorldXYWidth() / 10.;
		if(minDistance < maxDistInMetres)
		{
            ConceptualObjectData &tmpData = itsConceptualObjectDatas[minDistInd];
			str += "Conceptual analysis info:";
			str = AddColorTagsToString(str, fontColor, true);
			str += "\n";
            if(tmpData.itsInfoTextStr.empty())
    			str += ::MakeConceptualObjectInfoString(tmpData);
            else
    			str += tmpData.itsInfoTextStr;
		}
		else
		{
			str += "No Conceptual analysis object near";
			str = AddColorTagsToString(str, fontColor, true);
		}
	}
	else
	{
		str += "No conceptual analysis info";
		str = AddColorTagsToString(str, fontColor, true);
	}
	return str;
}

// *************************************************************
// rintamien piirtoon liittyv‰‰ pallukoiden lasku/piirto koodia
// *************************************************************

// 1. laske viivan paksuuden ja rintaman tyypin avulla
   // a) toivottu pallukan koko [mm]
   // b) toivottu alku/loppu tyhj‰ v‰li [mm]
   // c) toivottu v‰li [mm]
void NFmiConceptualDataView::CalculateIdealPathObjectMeasures(float lineWidthInMM, FrontType frontType, float &startGapInMM, float &objectSizeInMM, float &gapInMM)
{
	objectSizeInMM = lineWidthInMM * 5.f;
	startGapInMM = objectSizeInMM / 1.f;
	gapInMM = objectSizeInMM * 4.f;

	if(frontType == kFmiFrontTypeOcclusion) // 2 on okluusio ja se on kaksi kertaa leve‰mpi
		objectSizeInMM *= 2.f;

	if(frontType == kFmiFrontTypeThrough) // 3 on sola ja aloitus pistett‰ lyhennet‰‰n
	{
		gapInMM = objectSizeInMM * 1.f; // solassa on v‰k‰si‰ tiuhempaan
	}

	if(frontType == kFmiFrontTypeCloudAreaPartArc || frontType == kFmiFrontTypeCloudAreaPartPie1 || frontType == kFmiFrontTypeCloudAreaPartPie2)
	{
		objectSizeInMM = lineWidthInMM * 5.f; // oikea koko
		startGapInMM = 0.01f; //objectSizeInMM / 2.f;
		gapInMM = 0.001f; //objectSizeInMM * 0.01f;
	}
}

static float CalcLength(const PointF &p1, const PointF &p2)
{
   return ::sqrt((p1.X - p2.X)*(p1.X - p2.X) + (p1.Y - p2.Y)*(p1.Y - p2.Y));
}

// 2. Laske koko viivan pituus [mm]
   // laske myˆs eri osien pituudet taulukkoon
void NFmiConceptualDataView::CalculatePathLengths(float pixelLengthInMM, const std::vector<Gdiplus::PointF> &xyPoints, std::vector<float> &lengthsInMM, float &totalLengthInMM)
{
   lengthsInMM.clear();
   float total = 0;
   for(size_t i = 1; i<xyPoints.size(); i++)
   {
       float current = ::CalcLength(xyPoints[i-1], xyPoints[i]) * pixelLengthInMM;
       lengthsInMM.push_back(current);
       total += current;
   }
   totalLengthInMM = total;
}

static void AddFrontHalfCircleToPath(float symbolSizeInPixels, float sizeDivider, GraphicsPath &thePathOut, const PointF &theStartPoint)
{
	thePathOut.AddPie(-symbolSizeInPixels/sizeDivider + theStartPoint.X, -symbolSizeInPixels/sizeDivider + theStartPoint.Y, symbolSizeInPixels/(sizeDivider/2.f), symbolSizeInPixels/(sizeDivider/2.f), 0, 180);
}

static void AddCloudAreaHalfArcToPath(float symbolSizeInPixels, float sizeDivider, GraphicsPath &thePathOut, const PointF &theStartPoint)
{
	thePathOut.AddArc(-symbolSizeInPixels/sizeDivider + theStartPoint.X, -symbolSizeInPixels/sizeDivider + theStartPoint.Y, symbolSizeInPixels/(sizeDivider/2.f), symbolSizeInPixels/(sizeDivider/2.f), 0, 180);
}

static void AddFrontTriangleToPath(float symbolSizeInPixels, float sizeDivider, GraphicsPath &thePathOut, const PointF &theStartPoint, float heightVsBaseFactor)
{
	vector<PointF> trianglePoints;
	trianglePoints.push_back(PointF(-symbolSizeInPixels/sizeDivider + theStartPoint.X, 0 + theStartPoint.Y));
	trianglePoints.push_back(PointF(symbolSizeInPixels/sizeDivider + theStartPoint.X, 0 + theStartPoint.Y));
	// teen kolmiosta tasakylkisen ja korkeamman kuin pallukasta, eli siit‰ kerroin heightVsBaseFactor (kolmin korkeus / leveys)
	trianglePoints.push_back(PointF(0 + theStartPoint.X, (heightVsBaseFactor*symbolSizeInPixels)/sizeDivider + theStartPoint.Y));
	thePathOut.AddPolygon(&trianglePoints[0], static_cast<INT>(trianglePoints.size()));
}

// Toisin kuin kylm‰n rintaman kolmion kanssa, t‰ss‰ kolmion keskipisteen pit‰‰ olla (0,0) -pisteess‰
static void AddStreamlineArroyHeadToPath(float symbolSizeInPixels, float sizeDivider, GraphicsPath &thePathOut, const PointF &theStartPoint, float heightVsBaseFactor)
{
    float finalSymbolHeight = (heightVsBaseFactor*symbolSizeInPixels)/sizeDivider;
	vector<PointF> trianglePoints;
	trianglePoints.push_back(PointF(-symbolSizeInPixels/sizeDivider + theStartPoint.X, theStartPoint.Y - finalSymbolHeight/2.f));
	trianglePoints.push_back(PointF(0 + theStartPoint.X, theStartPoint.Y - finalSymbolHeight/4.5f)); // t‰m‰ pohjan keskipiste tekee ter‰v‰mm‰t per‰v‰k‰set nuolen p‰‰h‰n
	trianglePoints.push_back(PointF(symbolSizeInPixels/sizeDivider + theStartPoint.X, theStartPoint.Y - finalSymbolHeight/2.f));
	// teen kolmiosta tasakylkisen ja korkeamman kuin pallukasta, eli siit‰ kerroin heightVsBaseFactor (kolmin korkeus / leveys)
	trianglePoints.push_back(PointF(0 + theStartPoint.X, theStartPoint.Y + finalSymbolHeight/2.f));
	thePathOut.AddPolygon(&trianglePoints[0], static_cast<INT>(trianglePoints.size()));
}

static void AddTroughBarbWireToPath(float symbolSizeInPixels, float sizeDivider, GraphicsPath &thePathOut, const PointF &theStartPoint)
{
	vector<PointF> trianglePoints;
	trianglePoints.push_back(PointF(theStartPoint.X, theStartPoint.Y));
	// viivan pituutta lis‰t‰‰n kertoimella
	trianglePoints.push_back(PointF(theStartPoint.X, 0 + 3.0f*symbolSizeInPixels/sizeDivider + theStartPoint.Y));
	// HUOM!!! OUTO feature, polussa pit‰‰ olla kolme pistett‰ ett‰ se piirtyy Gdiplussalla
	// Joten lis‰‰ viel‰ pelk‰n yhden viivan tapauksessa polkuun alkupisteen uudestaan.
	trianglePoints.push_back(PointF(theStartPoint.X, theStartPoint.Y)); // polussa pit‰‰ olla kolme pistett‰?!?!?!?!?

	thePathOut.AddPolygon(&trianglePoints[0], static_cast<INT>(trianglePoints.size()));
}

// 3. hae pallukoiden piirto polku (GraphicsPath) rintamatyypin mukaan
void NFmiConceptualDataView::GetDecorationPath(float symbolSizeInPixels, FrontType frontType, Gdiplus::GraphicsPath &thePathOut)
{
   thePathOut.Reset();
   thePathOut.SetFillMode(FillModeWinding); // t‰m‰ ei tee aukkoja polkuun, joita tulee jos esim. pallukka ja kolmio ovat v‰h‰n p‰‰llekk‰in ja k‰ytet‰‰n oletus fillmodea (FillModeAlternate)

   float sizeDivider = 2.f;
   float heightVsBaseFactor = 1.4f;
   // HUOM! n‰m‰ pit‰‰ keskitt‰‰ 0-kohtaan!
   if(frontType == kFmiFrontTypeWarm || frontType == kFmiFrontTypeCloudAreaPartPie1 || frontType == kFmiFrontTypeCloudAreaPartPie2) // l‰mmin rintama
   {
	   ::AddFrontHalfCircleToPath(symbolSizeInPixels, sizeDivider, thePathOut, PointF(0, 0));
   }
   else if(frontType == kFmiFrontTypeCloudAreaPartArc)
   {
	   ::AddCloudAreaHalfArcToPath(symbolSizeInPixels, sizeDivider, thePathOut, PointF(0, 0));
   }
   else if(frontType == kFmiFrontTypeCold) // kylm‰ rintama kolmiot
   {
       ::AddFrontTriangleToPath(symbolSizeInPixels, sizeDivider, thePathOut, PointF(0, 0), heightVsBaseFactor);
   }
   else if(frontType == kFmiFrontTypeStreamLine) // streamline nuolen p‰‰
   {
       float heightVsBaseFactor = 2.8f;
       ::AddStreamlineArroyHeadToPath(symbolSizeInPixels, sizeDivider, thePathOut, PointF(0, 0), heightVsBaseFactor);
   }
   else if(frontType == kFmiFrontTypeOcclusion) // okluusio rintama
   {    // muista ett‰ okluusio symboli oli kaksi kertaa isompi, koska siin‰ oli kaksi symbolia yhdess‰, joten
       // kokoa pit‰‰ jakaa enemm‰n kuin edell‰
		sizeDivider = 4.f;
		::AddFrontHalfCircleToPath(symbolSizeInPixels, sizeDivider, thePathOut, PointF(-symbolSizeInPixels/sizeDivider, 0));
		thePathOut.CloseFigure(); // pit‰‰ sulkea edellinen kuvio ennen kolmion lis‰‰mist‰, muuten tulee outoja juttuja ruudulle
		// pist‰n kolmion v‰h‰n puolipallon kanssa p‰‰llekk‰in, koska se n‰ytt‰‰ paremmalta (eli siirto PointF:‰‰ fiksataan kertoimella 0.8f)
		::AddFrontTriangleToPath(symbolSizeInPixels, sizeDivider, thePathOut, PointF((0.8f * symbolSizeInPixels)/sizeDivider, 0), heightVsBaseFactor);
   }
   else if(frontType == kFmiFrontTypeThrough) // sola v‰k‰nen
   {
       ::AddTroughBarbWireToPath(symbolSizeInPixels, sizeDivider, thePathOut, PointF(0, 0));
   }
}

static float CalcPreferableFrontObjectLength(int objectCount, float startGapInMM, float objectSizeInMM, float gapInMM)
{
   float tmpValue = 2*startGapInMM + objectCount * objectSizeInMM + (objectCount-1)*gapInMM;
   return tmpValue;
}

// laskee ritamassa olevan pallukoiden ja v‰lien pituussuhteen
static float CalcGapVsTotalObjectRatio(int objectCount, float startGapInMM, float objectSizeInMM, float gapInMM)
{
   float tmpValue = (2.f*startGapInMM + (objectCount-1)*gapInMM)/::CalcPreferableFrontObjectLength(objectCount, startGapInMM, objectSizeInMM, gapInMM);
   return tmpValue;
}

static float CalcObjectScalingRatio(int decoratorCount, float optimalObjectLength, float realObjectLength, float decoratorSizeInMM, bool allowDecoratorSizeChange)
{
	float scale = realObjectLength / optimalObjectLength;
	if(allowDecoratorSizeChange == false)
		scale = (realObjectLength - (decoratorSizeInMM*decoratorCount)) / (optimalObjectLength - (decoratorSizeInMM*decoratorCount));
	return scale;
}

// 4. Laske kuinka monta pallukkaa mahtuu annetun rintaman alueelle
   // jos alle 1, sovita pallukan koko, niin ett‰ mahtuu yksi (laske kerroin jolla skaalataan ja piirr‰ viivan keskelle pallukka)
   // jos alle kaksi, sovita pallukan koko, niin ett‰ mahtuu kaksi (laske skaala kerroin ja piirr‰ kahteen kohtaan pallukat)
   // muuten pallukat laske l‰hinp‰‰n kokonaislukuun sopiva skaala kerroin
   // Jos sovitus 1:een tai 2:een, sovita piirron kaikkiin osiin skaaloja.
   // Jos sovitus yli kahteen 'pallukkaan', s‰‰d‰ vain v‰lej‰.
// Jos paluttaa 0, on kyseess‰ virhe
int NFmiConceptualDataView::CalculateUsedSymbolCountAndMeasures(float totalLengthInMM, float &startGapInMM, float &objectSizeInMM, float &gapInMM)
{
   int totalCount = 0; // 0 on virhe ilmoitus

   float minDiff = 999999999.f;
   int minInd = -1;
   float closestLength = 999999999.f;
   for(int i=1; i<10000; i++) // k‰yd‰‰n max 10000 asti pallukoiden lukum‰‰r‰ laskua, ettei j‰‰ ikiluuppiin
   {
       float currentLength = ::CalcPreferableFrontObjectLength(i, startGapInMM, objectSizeInMM, gapInMM);
       float currentDiff = ::fabs(totalLengthInMM - currentLength);
       if(currentDiff < minDiff)
       {
           minDiff = currentDiff;
           minInd = i;
           closestLength = currentLength;
       }
       else
           break; // heti jos erotus alkaa kasvamaan, voidaan lopettaa
   }

   if(minInd > 0)
   {
       float tmpCount = static_cast<float>(minInd);
       tmpCount += (totalLengthInMM - closestLength)/(totalLengthInMM/tmpCount); // t‰ss‰ lis‰t‰‰n totalcounttiin j‰‰nnˆsosa, jonka avulla lasketaan mm. skaaloja
	   bool allowDecoratorSizeChange = false; // saako pallukan (=decorator) koko muuttua, vai s‰‰det‰‰nkˆ vain v‰lien kokoa
       if(tmpCount <= 1.f) // jos kertoimeksi saatu alle 1, laitetaan yksi kuvio rintamaan ja skaalataan kaikkia arvoja sen mukaisesti
       {
			allowDecoratorSizeChange = true;
			totalCount = 1;
			float tmpScale = ::CalcObjectScalingRatio(totalCount, closestLength, totalLengthInMM, objectSizeInMM, allowDecoratorSizeChange);
			startGapInMM *= tmpScale;
			objectSizeInMM *= tmpScale;
			gapInMM *= tmpScale;
       }
       else
       {
			totalCount = minInd;
			float tmpScale = ::CalcObjectScalingRatio(totalCount, closestLength, totalLengthInMM, objectSizeInMM, allowDecoratorSizeChange);
			startGapInMM *= tmpScale;
			if(allowDecoratorSizeChange)
				objectSizeInMM *= tmpScale; // ei siis suurenneta pallukoita, vain v‰lej‰
			gapInMM *= tmpScale;
       }
   }
   return totalCount;
}

// indeksi alkaa 1:st‰
static float CalcLengthFromStart(size_t objectIndex, float startGapInMM, float objectSizeInMM, float gapInMM)
{ // et‰isyys n:een pallukkaan rintama viivassa (huom! n. objecti lasketaan mukaan puolikkaana, koska halutaan objectin keskikohta)
   float tmpValue = startGapInMM + (objectIndex-1) * gapInMM + (objectIndex-1) * objectSizeInMM + objectSizeInMM / 2.f;
   return tmpValue;
}

// Jos ratio esim. 0.1, ollaan l‰hemp‰n‰ pistett‰ p1 ja jos se on 0.9 ollaan l‰hemp‰n‰ p2:sta.
static PointF InterpolatePoints(float ratio, const PointF &p1, const PointF &p2)
{
	PointF point(static_cast<float>(NFmiInterpolation::Linear(ratio, p1.X, p2.X)), 
				 static_cast<float>(NFmiInterpolation::Linear(ratio, p1.Y, p2.Y)));
	return point;
}

// Jos ratio esim. 0.1, ollaan l‰hemp‰n‰ pistett‰ p1 ja jos se on 0.9 ollaan l‰hemp‰n‰ p2:sta.
// HUOM! nyt lasketaan kulma vain pisteiden v‰lisen kulmakertoimen avulla, 
// t‰m‰n voisi tehd‰ cardinal-splini laskujen avulla hienommin.
static float CalcRotationAngle(float /* ratio */ , const PointF &p1, const PointF &p2)
{
	float dx = p2.X - p1.X;
	float dy = p2.Y - p1.Y;
	float fixAngle = 0;
	if(dy >= 0 && dx > 0)
		fixAngle = 180;
	else if(dy < 0 && dx > 0)
		fixAngle = 180;
	if(dx)
	{
		float angleInRad = ::atan(dy/dx);
		return fixAngle + (angleInRad/(2.f*static_cast<float>(kPii))*360.f);
	}
	else
	{
		if(dy > 0)
			return -90.f; // mik‰ t‰ss‰ pit‰isi olla?
		else
			return 90.f; // mik‰ t‰ss‰ pit‰isi olla?
	}
}

static void CalcObjectPlaceAndRotationAngle(float lengthFromStart, const vector<PointF> &frontPoints,
                                           const vector<float> &lengthsInMM, float /* totalLengthInMM */ ,
                                           PointF &point, float &rotationAngle)
{
   if(lengthsInMM.size() > 0 && frontPoints.size() > lengthsInMM.size())
   {
       if(lengthFromStart < lengthsInMM[0])
       { // piste sijaitsee 1. piste v‰lill‰
           float ratio = lengthFromStart / lengthsInMM[0];
           point = ::InterpolatePoints(ratio, frontPoints[0], frontPoints[1]);
           rotationAngle = ::CalcRotationAngle(ratio, frontPoints[0], frontPoints[1]);
       }
       else
       {
           float currentLengthFromStart = lengthFromStart;
           for(size_t i=1; i<lengthsInMM.size(); i++)
           {
               currentLengthFromStart -= lengthsInMM[i-1];
               if(currentLengthFromStart < lengthsInMM[i])
               { // piste sijaitsee t‰ss‰ v‰liss‰
                   float ratio = currentLengthFromStart / lengthsInMM[i];
                   point = ::InterpolatePoints(ratio, frontPoints[i], frontPoints[i+1]);
                   rotationAngle = ::CalcRotationAngle(ratio, frontPoints[i], frontPoints[i+1]);
                   break;
               }
           }
       }
   }
   else
       throw runtime_error("Error in CalcObjectPlaceAndRotationAngle");
}

// 5. Laske pallukoiden lukum‰‰r‰n ja muiden mittojen avulla pallukoiden kohdat ja talleta sijainnit taulukkoon
// 6. laske tarvittavat pallukoiden rotaatio kulmat haluttuihin kohtiin viivaa [asteissa]
void NFmiConceptualDataView::CalcPathObjectPoints(const std::vector<Gdiplus::PointF> &frontPoints, size_t objectCount, float startGapInMM, float objectSizeInMM,
                                 float gapInMM, const std::vector<float> &lengthsInMM, float totalLengthInMM, std::vector<Gdiplus::PointF> &objectPoints,
                            std::vector<float> &rotationAngles)
{
   objectPoints.clear();
   rotationAngles.clear();
   for(size_t i = 0; i < objectCount; i++)
   {
       float lengthFromStart = ::CalcLengthFromStart(i+1, startGapInMM, objectSizeInMM, gapInMM);
       float rotationAngle = 0;
       PointF currentPoint;
       ::CalcObjectPlaceAndRotationAngle(lengthFromStart, frontPoints, lengthsInMM, totalLengthInMM, currentPoint, rotationAngle);
       objectPoints.push_back(currentPoint);
       rotationAngles.push_back(rotationAngle);
   }
}

static FrontType GetFrontType(const ConceptualObjectData &theData)
{
	if(theData.fConceptualType)
	{
	    if(theData.fPlainArea == false)
	    {
		    if(theData.itsFrontNameStr == "WARMFRONT")
			    return kFmiFrontTypeWarm;
		    else if(theData.itsFrontNameStr == "COLDFRONT")
			    return kFmiFrontTypeCold;
		    else if(theData.itsFrontNameStr == "OCCLUSION")
			    return kFmiFrontTypeOcclusion;
		    else if(theData.itsFrontNameStr == "TROUGH")
			    return kFmiFrontTypeThrough;
		    else if(theData.itsFrontNameStr == "UPPER_TROUGH")
			    return kFmiFrontTypeThrough;
		    else if(theData.itsFrontNameStr == "JET_ARROW")
			    return kFmiFrontTypeJet;
		    else if(theData.itsFrontNameStr == "DASHED_JET_ARROW")
			    return kFmiFrontTypeSlowJet;
		    else if(theData.itsFrontNameStr == "WIND_ARROW")
			    return kFmiFrontTypeWind;
        }
        else
        {
		    if(theData.itsFrontNameStr == "CLOUDAREA")
			    return kFmiFrontTypeCloudArea;
	    }
    }
	return kFmiFrontTypeError;
}

static vector<float> CalcRotationAngles(const vector<PointF> &theXyPoints)
{
	vector<float> angles;
	if(theXyPoints.size() >= 2) // pit‰‰ olla v‰hint‰in 2. pistett‰ ett‰ lasketaan
	{
		for(size_t i=0; i<theXyPoints.size(); i++)
		{
			if(i == 0)
				angles.push_back(::CalcRotationAngle(0, theXyPoints[i], theXyPoints[i+1]));
			else
				angles.push_back(::CalcRotationAngle(0, theXyPoints[i-1], theXyPoints[i]));
		}
	}
	else if(theXyPoints.size() == 1)
		angles.push_back(0); // t‰m‰ on roska kulma kun on vain yksi piste viivassa

	return angles;
}

static PointF CalcPointLoc(PointF xyPoint, float angleInDeg, float lineLength)
{
	static const double trigFactor = 2 * kPii / 360;

	PointF newPoint;
	newPoint.X = xyPoint.X + (lineLength * static_cast<float>(::cos(angleInDeg * trigFactor)));
	newPoint.Y = xyPoint.Y + (lineLength * static_cast<float>(::sin(angleInDeg * trigFactor)));

	return newPoint;
}

void NFmiConceptualDataView::DrawCloudArea(ConceptualObjectData &theData, float theLineWidthInMM, const std::vector<PointF> &theXyPoints, float theLineWidthInPixels)
{
	DrawFrontDecorations(theData, theLineWidthInMM, theXyPoints, theLineWidthInPixels, kFmiFrontTypeCloudAreaPartPie2);
	DrawFrontDecorations(theData, theLineWidthInMM, theXyPoints, theLineWidthInPixels, kFmiFrontTypeCloudAreaPartPie1);
	DrawFrontDecorations(theData, theLineWidthInMM, theXyPoints, theLineWidthInPixels, kFmiFrontTypeCloudAreaPartArc);
}

static void DrawJet(Gdiplus::Graphics &theGdiPlusGraphics, ConceptualObjectData &theData, std::vector<PointF> &theXyPoints, GdiPlusLineInfo &theLineInfo, bool fPrinting, std::vector<Gdiplus::REAL> &theDashPattern, float theLineWidthInPixels, bool fDrawOutLineArrow)
{
	// Joudun tekem‰‰n oman outline koodin t‰h‰n jet-olioita varten
	std::vector<PointF> leftXyPoints; // jetin pisteet siirrettyn‰ alkuper‰isest‰ viivasta vasemmalle
	std::vector<PointF> rightXyPoints; // ... oikealle
	vector<float> angles = ::CalcRotationAngles(theXyPoints);
	if(fDrawOutLineArrow)
	{
		for(size_t i=0; i<theXyPoints.size(); i++)
		{
			leftXyPoints.push_back(::CalcPointLoc(theXyPoints[i], angles[i]-90, theLineWidthInPixels*3));
			rightXyPoints.push_back(::CalcPointLoc(theXyPoints[i], angles[i]+90, theLineWidthInPixels*3));
		}
	}

	int fillHatchStyle = -1; // -1 tarkoittaa ett‰ fillauksen yhteydess‰ ei k‰ytet‰ hatchi‰


	if(fDrawOutLineArrow)
	{
        CtrlView::DrawGdiplusCurve(theGdiPlusGraphics, leftXyPoints, theLineInfo, theData.fPlainArea, fillHatchStyle, fPrinting, &theDashPattern);
        CtrlView::DrawGdiplusCurve(theGdiPlusGraphics, rightXyPoints, theLineInfo, theData.fPlainArea, fillHatchStyle, fPrinting, &theDashPattern);
	}
	else
        CtrlView::DrawGdiplusCurve(theGdiPlusGraphics, theXyPoints, theLineInfo, theData.fPlainArea, fillHatchStyle, fPrinting, &theDashPattern);

	if(theXyPoints.size() > 1)
	{
		std::vector<PointF> arroyXyPoints; // nuolik‰rki pistein‰
		// vasen reuna nuolen k‰rjest‰
		arroyXyPoints.push_back(::CalcPointLoc(theXyPoints[theXyPoints.size()-1], angles[theXyPoints.size()-1]-90, theLineWidthInPixels*6));
		// itse k‰rkipiste
		arroyXyPoints.push_back(::CalcPointLoc(theXyPoints[theXyPoints.size()-1], angles[theXyPoints.size()-1]+180, theLineWidthInPixels*10));
		// oikea reuna k‰rjest‰
		arroyXyPoints.push_back(::CalcPointLoc(theXyPoints[theXyPoints.size()-1], angles[theXyPoints.size()-1]+90, theLineWidthInPixels*6));
		theLineInfo.Tension(0); // laitetaan tensio 0:ksi, ett‰ tulee piirretty‰ ter‰v‰k‰rkinen nuoli
        CtrlView::DrawGdiplusCurve(theGdiPlusGraphics, arroyXyPoints, theLineInfo, theData.fPlainArea, fillHatchStyle, fPrinting, &theDashPattern);
	}
}

void NFmiConceptualDataView::DrawConseptualObject(ConceptualObjectData &theData)
{
	if(theData.itsLatlonPoints.size() > 0)
	{
		FrontType frontType = ::GetFrontType(theData);
		// ****** Gdiplus-piirto alkaa ********
		std::vector<PointF> xyPoints;
		for(size_t i = 0; i<theData.itsLatlonPoints.size(); i++)
			xyPoints.push_back(CtrlView::ConvertLatlonToGdiPlusPoint(this, theData.itsLatlonPoints[i]));
        if(frontType == kFmiFrontTypeCloudArea && xyPoints.size())
            xyPoints.push_back(xyPoints[0]); // pilvialue on jossain vaiheessa muuttunut suljetuksi alueeksi, t‰m‰n takia pit‰‰ alkupiste lis‰t‰ viel‰ loppuun, ett‰ alue sulkeutuu

		float lineWidthInMM = static_cast<float>(theData.itsLineWidthInMM);
		lineWidthInMM *= CalcScreenSizeFactor(*itsCtrlViewDocumentInterface, itsMapViewDescTopIndex);
		float lineWidthInPixels = static_cast<float>(lineWidthInMM * itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_x);
		int lineStyle = 0; // 0=yhten‰inen viiva
		std::string tmpFrontName = theData.itsFrontNameStr;
		NFmiStringTools::LowerCase(tmpFrontName);
		std::vector<Gdiplus::REAL> dashPattern;
		if(tmpFrontName.find(std::string("upper")) != std::string::npos)
		{
			lineStyle = 5; // jos lˆytyi nimesta upper-osio, laitetaan custom katkoviivaa (5=custom)
			dashPattern.push_back(8); // 9 pituinen viiva
            dashPattern.push_back(2); // 1 pituinen katko
		}
		else if(frontType == kFmiFrontTypeSlowJet)
		{
			lineStyle = 5; // jos oli heikko jetti, laitetaan custom katkoviivaa (5=custom)
			dashPattern.push_back(7.f); // 8 pituinen viiva
			dashPattern.push_back(3.f); // 2 pituinen katko
		}
		GdiPlusLineInfo lineInfo(lineWidthInPixels, theData.itsConceptualColor, lineStyle);
		lineInfo.SetLineCap(Gdiplus::LineCapRound);
		lineInfo.Tension(0.2f);
		int fillHatchStyle = -1; // -1 tarkoittaa ett‰ fillauksen yhteydess‰ ei k‰ytet‰ hatchi‰
        if(!theData.itsRainphaseStr.empty())
			fillHatchStyle = HatchStyleBackwardDiagonal;

		if(frontType == kFmiFrontTypeCloudArea)
		{
			DrawCloudArea(theData, lineWidthInMM, xyPoints, lineWidthInPixels);
		}
		else if(frontType == kFmiFrontTypeJet || frontType == kFmiFrontTypeSlowJet || frontType == kFmiFrontTypeWind)
		{ // piirret‰‰n jet omalla tavalla
			bool drawOutLineArrow = true;
			if(frontType == kFmiFrontTypeWind)
				drawOutLineArrow = false;
			::DrawJet(*itsGdiPlusGraphics, theData, xyPoints, lineInfo, itsCtrlViewDocumentInterface->Printing(), dashPattern, lineWidthInPixels, drawOutLineArrow);
		}
		else
		{
            CtrlView::DrawGdiplusCurve(*itsGdiPlusGraphics, xyPoints, lineInfo, theData.fPlainArea, theData.itsFillHatchPattern, itsCtrlViewDocumentInterface->Printing(), &dashPattern);
            if(theData.fPlainArea)
            { // jos oli ns. plain alue, piirret‰‰n sille reunus v‰h‰n tummemmalla v‰rill‰
                lineInfo.Color(NFmiColorSpaces::GetBrighterColor(lineInfo.Color(), -40));
                xyPoints.push_back(xyPoints[0]); // kun suljettu alue reunustetaan viivalla, pit‰‰ lis‰t‰ alkupiste loppuun, ett‰ piiri sulkeutuu
                CtrlView::DrawGdiplusCurve(*itsGdiPlusGraphics, xyPoints, lineInfo, false, -1, itsCtrlViewDocumentInterface->Printing(), &dashPattern);

                if(!theData.itsSymbolCodeStr.empty())
                { // tietyill‰ alueilla on niihin liittyv‰ symboli, joka piirret‰‰n alueen keskelle
                    if(theData.itsWomlMemberNameStr == "parametervaluesetarea")
                        DrawSymbol(theData, 13);
                    else
                        DrawSymbol(theData, 18);
                }
            }
			DrawFrontDecorations(theData, lineWidthInMM, xyPoints, lineWidthInPixels, frontType);
		}
	}
}

void NFmiConceptualDataView::DrawFrontDecorations(ConceptualObjectData &theData, float theLineWidthInMM, const std::vector<PointF> &theXyPoints, float theLineWidthInPixels, FrontType theFrontType)
{
	if(theFrontType != kFmiFrontTypeError)
	{
	// ***** Pallukoiden piirto *******
	// 1. laske viivan paksuuden ja rintaman tyypin avulla
		// a) toivottu pallukan koko [mm]
		// b) toivottu alku/loppu tyhj‰ v‰li [mm]
		// c) toivottu v‰li [mm]
		float startGapInMM = 0;
		float objectSizeInMM = 0;
		float gapInMM = 0;
		CalculateIdealPathObjectMeasures(theLineWidthInMM, theFrontType, startGapInMM, objectSizeInMM, gapInMM);
	// 2. Laske koko viivan pituus [mm]
		// laske myˆs eri osien pituudet taulukkoon
		float pixelLengthInMM = 1.f/static_cast<float>(itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_x);
		vector<float> lengthsInMM;
		float totalLengthInMM = 0;
		CalculatePathLengths(pixelLengthInMM, theXyPoints, lengthsInMM, totalLengthInMM);
	// 3. Laske kuinka monta pallukkaa mahtuu annetun rintaman alueelle
		// jos alle 1, sovita pallukan koko, niin ett‰ mahtuu yksi (laske kerroin jolla skaalataan ja piirr‰ viivan keskelle pallukka)
		// jos alle kaksi, sovita pallukan koko, niin ett‰ mahtuu kaksi (laske skaala kerroin ja piirr‰ kahteen kohtaan pallukat)
		// muuten pallukat laske l‰hinp‰‰n kokonaislukuun sopiva skaala kerroin
		int objectCount = CalculateUsedSymbolCountAndMeasures(totalLengthInMM, startGapInMM, objectSizeInMM, gapInMM);
		if(objectCount > 0)
		{
		// 4. hae pallukoiden piirto polku (GraphicsPath) rintamatyypin mukaan
			GraphicsPath decorationPath;
			GetDecorationPath(objectSizeInMM/pixelLengthInMM, theFrontType, decorationPath);
		// 5. Laske pallukoiden lukum‰‰r‰n ja muiden mittojen avulla pallukoiden kohdat ja talleta sijainnit taulukkoon
		// 6. laske tarvittavat pallukoiden rotaatio kulmat haluttuihin kohtiin viivaa [asteissa]
			vector<PointF> decoratorPoints;
			vector<float> rotationAngles;
			CalcPathObjectPoints(theXyPoints, objectCount, startGapInMM, objectSizeInMM, gapInMM, lengthsInMM, totalLengthInMM, decoratorPoints, rotationAngles);
		// 7. Piirr‰ pallukat skaalan, sijainnin ja rotaation avulla
			// jos left/right suunta, s‰‰d‰ rotaatiota 180 asteella
			SolidBrush aBrush(CtrlView::NFmiColor2GdiplusColor(theData.itsConceptualColor));
            float usedLineWidthInPixels = theLineWidthInPixels;
			if(theFrontType == kFmiFrontTypeThrough)
                usedLineWidthInPixels *= 0.6f; // solien v‰k‰set ovat sola viivaa ohuemmat
			// Kyn‰lle laitetaan t‰ysi opacity p‰‰lle (NFmiColor alpha = 0)
			auto penColor = theData.itsConceptualColor;
			penColor.Alpha(0);
			Pen aPen(CtrlView::NFmiColor2GdiplusColor(penColor), usedLineWidthInPixels);
			aPen.SetLineCap(Gdiplus::LineCapRound, Gdiplus::LineCapRound, Gdiplus::DashCapFlat);


			Gdiplus::Matrix transMat; // originaali muutos matriisi t‰h‰n
			itsGdiPlusGraphics->GetTransform(&transMat);
			float baseRotationFix = 0;
			if(theData.fLeft == true) // t‰m‰ rintamien pallukoiden orientaatio on muuttunut kes‰kuun lopulla 2008, syyt‰ ei ole tiedossa
				baseRotationFix = 180;
			float usedRotationFix = baseRotationFix;
			if(theFrontType == kFmiFrontTypeCloudAreaPartPie2)
				usedRotationFix += 180;

			for(size_t j = 0; j < decoratorPoints.size(); j++)
			{
				if(theFrontType == kFmiFrontTypeThrough)
				{
					if(j % 2 == 0) // joka toisella kerralla sola v‰k‰st‰ v‰‰nnet‰‰n toiseen suuntaan ja joka toisella toiseen
						usedRotationFix = baseRotationFix + 60;
					else
						usedRotationFix = baseRotationFix + 120;
				}
				PointF &p1 = decoratorPoints[j];
				itsGdiPlusGraphics->TranslateTransform(p1.X, p1.Y);
				itsGdiPlusGraphics->RotateTransform(rotationAngles[j] + usedRotationFix);

				if(theFrontType == kFmiFrontTypeThrough || theFrontType == kFmiFrontTypeCloudAreaPartArc)
					itsGdiPlusGraphics->DrawPath(&aPen, &decorationPath);
				else
				{
					if(theFrontType == kFmiFrontTypeCloudAreaPartPie1 || theFrontType == kFmiFrontTypeCloudAreaPartPie2)
					{
						NFmiColor aColor = theData.itsConceptualColor;
						aColor.Alpha(0.5f);
						if(theFrontType == kFmiFrontTypeCloudAreaPartPie2)
							aColor.Alpha(0.65f); // Pie2 laitetaan 'vaaleammaksi' lis‰‰m‰ll‰ l‰pin‰kyvyytt‰
						INT pointSize = decorationPath.GetPointCount();
						std::vector<PointF> points(pointSize);
						decorationPath.GetPathPoints(&points[0], pointSize);
						int lineStyle = 0; // 0=yhten‰inen viiva
						GdiPlusLineInfo lineInfo(usedLineWidthInPixels, aColor, lineStyle);
                        CtrlView::DrawGdiplusCurve(*itsGdiPlusGraphics, points, lineInfo, true, HatchStyle40Percent, itsToolBox->GetDC()->IsPrinting() == TRUE);
					}
					else
						itsGdiPlusGraphics->FillPath(&aBrush, &decorationPath);
				}
				itsGdiPlusGraphics->SetTransform(&transMat); // palautetaan aina originaali muutos msatriisi k‰yttˆˆn, sill‰ muuten transformaatiot kumuloituvat
			}
		}
	}
}

// HUOM! t‰t‰ pit‰‰ kutsua ennen kuin itse luokkaa saa k‰ytt‰‰!!!!
void NFmiConceptualDataView::InitMirwaSymbolMap(const std::string &theWomlDirectory)
{
    if(!NFmiConceptualDataView::itsMirwaSymbolMap.Initialized())
    {
        // hae baseFolder NFmiSettings-luokan asetuksista, tee abs.-suht. polku temput
        std::string baseFolder = NFmiSettings::Optional<std::string>("ConceptualModelData::BaseSymbolFolder", "symbols"); 
        baseFolder = NFmiFileSystem::MakeAbsolutePath(baseFolder, theWomlDirectory);
        // hae initFile NFmiSettings-luokan asetuksista, tee abs.-suht. polku temput
        std::string initFile = NFmiSettings::Optional<std::string>("ConceptualModelData::SymbolMapFile", "symbolmap.txt"); 
        initFile = NFmiFileSystem::MakeAbsolutePath(initFile, theWomlDirectory);
        NFmiConceptualDataView::itsMirwaSymbolMap.Initialize(baseFolder, initFile);
    }
}

// *************************************************************
// rintamien piirtoon liittyv‰‰ pallukoiden lasku/piirto koodia
// *************************************************************

