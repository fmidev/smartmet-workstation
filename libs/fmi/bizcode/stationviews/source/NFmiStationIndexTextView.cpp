#include "NFmiStationIndexTextView.h"
#include "nindmesl.h"
#include "NFmiToolBox.h"
#include "NFmiText.h"
#include "NFmiWeatherAndCloudiness.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiDrawParam.h"
#include "FmiNMeteditLibraryDefinitions.h"
#include "NFmiValueString.h"
#include "NFmiSettings.h"
#include "NFmiFileSystem.h"
#include "ToolMasterDrawingFunctions.h"
#include "CtrlViewDocumentInterface.h"
#include "CtrlViewFunctions.h"
#include "GraphicalInfo.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "SynopCodeUtils.h"
#include "ColorStringFunctions.h"
#include "MathHelper.h"

#include "boost\math\special_functions\round.hpp"

// ********************************************************************
// ***************  NFmiStationIndexTextView  *************************
// ********************************************************************

NFmiStationIndexTextView::NFmiStationIndexTextView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
												,NFmiToolBox *theToolBox
												,boost::shared_ptr<NFmiDrawParam> &theDrawParam
												,FmiParameterName theParamId
												,NFmiIndexMessageList* theIndexedWordList
												,NFmiPoint theOffSet
												,NFmiPoint theSize
												,int theRowIndex
                                                ,int theColumnIndex)
: NFmiStationTextView (theMapViewDescTopIndex, theArea
					   ,theToolBox
					   ,theDrawParam
					   ,theParamId
					   ,theOffSet
					   ,theSize
					   ,theRowIndex
                       ,theColumnIndex)
,itsIndexedWordList(theIndexedWordList)
,fUseWeatherAndCloudinessForInterpolation(false)
{
}

NFmiStationIndexTextView::~NFmiStationIndexTextView(void)
{
}

void NFmiStationIndexTextView::ModifyTextEnvironment(void)
{
	NFmiStationTextView::ModifyTextEnvironment();
    itsDrawingEnvironment.SetFontSize(CalcFontSize(16, boost::math::iround(MaximumFontSizeFactor() * 72), itsCtrlViewDocumentInterface->Printing())); 
	itsDrawingEnvironment.SetFontType(kSynop);
}

void NFmiStationIndexTextView::SbdSetFontName()
{
    itsSymbolBulkDrawData.fontName(L"Synop");
}

NFmiPoint NFmiStationIndexTextView::SbdCalcFixedSymbolSize() const
{
    // synop fontit pit‰‰ laittaa isommiksi kuin normaali teksti
    return SbdBasicSymbolSizeCalculation(16, 72);
}

NFmiString NFmiStationIndexTextView::GetPrintedText(float theValue)
{
    return NFmiString(CtrlViewUtils::GetSynopCodeAsSynopFontText(theValue));
}

int NFmiStationIndexTextView::GetApproxmationOfDataTextLength(std::vector<float>* )
{
	return 1;
}

float NFmiStationIndexTextView::ViewFloatValue(bool doTooltipValue)
{
	float value = kFloatMissing;
    bool specialValueRetrieved = false;
	if(itsInfo)
	{
		if(fDoTimeInterpolation)
		{
			if(fUseWeatherAndCloudinessForInterpolation && itsInfo->Param().GetParamIdent() == kFmiWeatherAndCloudiness) // itsInfoa ei ole aina asetettu w&c parametriin, joten se pit‰‰ tarkistaa (ik‰v‰‰ koodia)
			{
				NFmiWeatherAndCloudiness weather(itsInfo->InterpolatedValue(itsTime, 360), kFmiPackedWeather, kFloatMissing, itsInfo->InfoVersion());
				value = static_cast<float>(weather.SubValue(itsParamId));
                specialValueRetrieved = true;
			}
		}
	}
    if(!specialValueRetrieved)
        value = NFmiStationView::ViewFloatValue(doTooltipValue);
	if(itsDrawParam->Param().GetParamIdent() == kFmiPresentWeather)
		value = ::ConvertPossible_WaWa_2_WW(value);
	return value;
}

NFmiColor NFmiStationIndexTextView::GetBasicParamRelatedSymbolColor(float theValue) const
{
    return CtrlViewUtils::GetSynopCodeSymbolColor(theValue);
}

NFmiSymbolColorChangingType NFmiStationIndexTextView::SbdGetSymbolColorChangingType() const
{
    return NFmiSymbolColorChangingType::Mixed;
}

void NFmiStationIndexTextView::Draw(NFmiToolBox *theGTB)
{
    NFmiStationTextView::Draw(theGTB);
    // Tarkistetaan myˆs itsInfo pointer, koska kaatumisraportit vihjaisivat ett‰ SmartMet on kaatunut itsInfo->Param(itsParamId) -kutsuun
    // eli kyseess‰ on ilmeisesti ollut nullptr. itsInfo attribuutti kyll‰ nollataan tietyiss‰ paikoin piirtokoodia, mutta en saanut 
    // toistettua ongelmaa mitenk‰‰n.
    if(fDoTimeInterpolation && itsInfo)
        itsInfo->Param(itsParamId); // Palautetaan jostain tuntemattomasta syyst‰ originaali parametri
}

bool NFmiStationIndexTextView::PrepareForStationDraw(void)
{
    bool status = NFmiStationView::PrepareForStationDraw();
    if(fDoTimeInterpolation)
    {
        FmiParameterName parId = static_cast<FmiParameterName>(itsDrawParam->Param().GetParamIdent());
        fUseWeatherAndCloudinessForInterpolation = (parId == kFmiWeatherAndCloudiness) || (parId == kFmiWeatherSymbol1) || (parId == kFmiPrecipitationForm) || (parId == kFmiFogIntensity);
        // jos tarvitaan aikainterpolaatiota, pit‰‰ tehd‰ jippo t‰ss‰ ja asettaa weatherandcloudiness p‰‰lle
        if(fUseWeatherAndCloudinessForInterpolation && itsInfo)
        {
            if(!itsInfo->Param(kFmiWeatherAndCloudiness)) // yritet‰‰n laittaa totalwind parametri p‰‰lle jos pit‰‰ tehd‰ aikainterpolaatiota
            {
                fUseWeatherAndCloudinessForInterpolation = false;
                itsInfo->Param(itsParamId); // jos datassa ei ole totalwindi‰, paluta windvector p‰‰lle
            }
        }
    }
    return status;
}

// Html/Xml stringin leip‰tekstin tietyt merkit pit‰‰ muuttaa, ett‰ ne n‰kyv‰t html-viewerissa oikein (tooltip on t‰ss‰ html-viewer).
// Kun k‰ytet‰‰n synop-fonttia ja siihen liittyvi‰ symboleja, tˆrm‰sin ongelmaan Cl parametrin kanssa kun cloud type on 4, t‰llˆin
// tulos stringiin tulee '<'-merkki, joka on ongelma tooltipin html-visualisoinnille. Se korjataan muuttamalla leip‰teksti osion stringit
// html:n suhteen oikeaan muotoon tekem‰ll‰ seuraavat korvaukset:
//
//    &amp; -> & (ampersand, U+0026)
//    &lt; -> < (less-than sign, U+003C)
//    &gt; -> > (greater-than sign, U+003E)
//    &quot; -> " (quotation mark, U+0022)
//    &apos; ->' (apostrophe, U+0027)
static std::string HtmlXmlCharacterEncode(const std::string &theStr)
{
	const std::string ampStr = "&";
	const std::string ampReplaceStr = "&amp;";
	const std::string ltStr = "<";
	const std::string ltReplaceStr = "&lt;";
	const std::string gtStr = ">";
	const std::string gtReplaceStr = "&gt;";
	const std::string quotStr = "\"";
	const std::string quotReplaceStr = "&quot;";
	const std::string aposStr = "'";
	const std::string aposReplaceStr = "&apos;";

	std::string resultStr(theStr);
	NFmiStringTools::ReplaceAll(resultStr, ampStr, ampReplaceStr);
	NFmiStringTools::ReplaceAll(resultStr, ltStr, ltReplaceStr);
	NFmiStringTools::ReplaceAll(resultStr, gtStr, gtReplaceStr);
	NFmiStringTools::ReplaceAll(resultStr, quotStr, quotReplaceStr);
	NFmiStringTools::ReplaceAll(resultStr, aposStr, aposReplaceStr);

	return resultStr;
}

std::string NFmiStationIndexTextView::Value2ToolTipString(float theValue, int /* theDigitCount */ , FmiInterpolationMethod /* theInterpolationMethod */ , FmiParamType /* theParamType */ )
{
	std::string symbolStr(GetPrintedText(theValue));
	symbolStr = ::HtmlXmlCharacterEncode(symbolStr);
    std::string fontFaceName = "synop";
    if(itsDrawParam->StationDataViewType() == NFmiMetEditorTypes::View::kFmiRawMirriFontSymbolView)
        fontFaceName = "mirri";
	std::string str;
    str += "<font face=";
    str += fontFaceName;
    str += " size = +6";
	NFmiColor color(GetSymbolColor(theValue));
	str += " color=";
	str += ColorString::Color2HtmlColorStr(color);
	str += ">";

	str += symbolStr;
	str += "</font>";
	str += " (";
	if(theValue == kFloatMissing)
		str += "-";
	else
		str += NFmiStringTools::Convert<int>(static_cast<int>(theValue));
	str += ")";
	return str;
}

// ********************************************************************
// ***************  NFmiStationFogTextView  ***************************
// ********************************************************************

NFmiString NFmiStationFogTextView::GetPrintedText(float theValue)
{
	if(itsIndexedWordList)
	{
		if(theValue != kFloatMissing)
		{
			long index = long(FogValueToSymbolIndex(theValue));
			return itsIndexedWordList->Message(index);
		}
	}
	return NFmiString();
}

// NoFog = 0, ModerateFog = 1, DenseFog = 2
float NFmiStationFogTextView::FogValueToSymbolIndex(float theFogValue) const
{
	switch(int(theFogValue))
	{
	case 0: // NoFog
		return kFloatMissing;
	case 1:// ModerateFog
		return 12;
	case 2:// DenseFog
		return 45;
	}
	return kFloatMissing;
}

NFmiColor NFmiStationFogTextView::GetBasicParamRelatedSymbolColor(float theValue) const
{
    return NFmiStationIndexTextView::GetBasicParamRelatedSymbolColor(FogValueToSymbolIndex(theValue));
}

// ********************************************************************
// **********  NFmiTotalCloudinessSymbolTextView  *********************
// ********************************************************************

NFmiString NFmiTotalCloudinessSymbolTextView::GetPrintedText(float theValue)
{
	if(itsIndexedWordList)
	{
		if(theValue != kFloatMissing && theValue >=0 && theValue < 10)
		{
			long index = static_cast<long>(theValue);
			return itsIndexedWordList->Message(index);
		}
		else if(theValue != kFloatMissing)
			return NFmiString(NFmiValueString::GetStringWithMaxDecimalsSmartWay(theValue, itsDrawParam->IsoLineLabelDigitCount()));
	}
	return NFmiString();
}

NFmiColor NFmiTotalCloudinessSymbolTextView::GetBasicParamRelatedSymbolColor(float theValue) const
{
    // You must leap here over parent's method to grandparent's method
    return NFmiStationView::GetBasicParamRelatedSymbolColor(theValue);
}

NFmiSymbolColorChangingType NFmiTotalCloudinessSymbolTextView::SbdGetSymbolColorChangingType() const
{
    return NFmiSymbolColorChangingType::DrawParamSet;
}

// ********************************************************************
// *********  NFmiPrecipitationFormSymbolTextView  ********************
// ********************************************************************

NFmiString NFmiPrecipitationFormSymbolTextView::GetPrintedText(float theValue)
{
	if(itsIndexedWordList)
	{
		if(theValue != kFloatMissing && theValue >=0 && theValue < 9)
		{
			long index = static_cast<long>(theValue);
			return itsIndexedWordList->Message(index);
		}
		else if(theValue != kFloatMissing)
			return NFmiString(NFmiValueString::GetStringWithMaxDecimalsSmartWay(theValue, itsDrawParam->IsoLineLabelDigitCount()));
	}
	return NFmiString();
}

NFmiColor NFmiPrecipitationFormSymbolTextView::GetBasicParamRelatedSymbolColor(float theValue) const
{
    switch(int(theValue))
    {
    case 6: // hail
        return NFmiColor(0.5f, 0, 1); // violetit s‰‰t
    case 0: // drizzle
    case 1: // rain
    case 2: // sleet
    case 3: // snow
        return NFmiColor(0, 0.6f, 0.3f); // vihre‰t s‰‰t
        //		return NFmiColor(0,0.5f,0); // tumman vihre‰t s‰‰t
    case 4: // freezing drizzle
    case 5: // freezing rain
        return NFmiColor(1, 0, 0); // punaiset s‰‰t
    default:
        return NFmiColor(0, 0, 0); // mustat s‰‰t
    }
}

// ********************************************************************
// ***********  NFmiRawMirriFontSymbolTextView  ***********************
// ********************************************************************

NFmiRawMirriFontSymbolTextView::NFmiRawMirriFontSymbolTextView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
    , NFmiToolBox * theToolBox
    , boost::shared_ptr<NFmiDrawParam> &theDrawParam
    , FmiParameterName theParamIdent
    , NFmiIndexMessageList * theIndexedWordList
    , NFmiPoint theOffSet
    , NFmiPoint theSize
    , int theRowIndex
    , int theColumnIndex)
: NFmiStationIndexTextView(theMapViewDescTopIndex, theArea
    , theToolBox
    , theDrawParam
    , theParamIdent
    , theIndexedWordList
    , theOffSet
    , theSize
    , theRowIndex
    , theColumnIndex)
{
}

void NFmiRawMirriFontSymbolTextView::ModifyTextEnvironment(void)
{
    NFmiStationTextView::ModifyTextEnvironment();
    itsDrawingEnvironment.SetFontSize(CalcFontSize(16, boost::math::iround(MaximumFontSizeFactor() * 48), itsCtrlViewDocumentInterface->Printing()));
    itsDrawingEnvironment.SetFontType(kMirri);
}

NFmiPoint NFmiRawMirriFontSymbolTextView::SbdCalcFixedSymbolSize() const
{
    return SbdBasicSymbolSizeCalculation(16, 48);
}

void NFmiRawMirriFontSymbolTextView::SbdSetFontName()
{
    itsSymbolBulkDrawData.fontName(L"Mirri");
}

NFmiString NFmiRawMirriFontSymbolTextView::GetPrintedText(float theValue)
{
    if(theValue != kFloatMissing)
    {
        char ch = static_cast<char>(theValue);
        NFmiString str;
        str += ch;
        return str;
    }
    else
        return "";
}

NFmiPoint NFmiRawMirriFontSymbolTextView::GetSpaceOutFontFactor(void)
{
    return NFmiPoint(0.6, 0.7);
}

NFmiColor NFmiRawMirriFontSymbolTextView::GetBasicParamRelatedSymbolColor(float theValue) const
{
    // You must leap here over parent's method to grandparent's method
    return NFmiStationView::GetBasicParamRelatedSymbolColor(theValue);
}

NFmiSymbolColorChangingType NFmiRawMirriFontSymbolTextView::SbdGetSymbolColorChangingType() const
{
    return NFmiSymbolColorChangingType::DrawParamSet;
}

// ********************************************************************
// **************  NFmiClCmChSymbolTextView  **************************
// ********************************************************************

static NFmiString GetClStr(float theValue)
{
	if(theValue == kFloatMissing)
		return NFmiString("");
	else
	{
		if(theValue < 1 || theValue > 9)
			return NFmiString("");
		else
		{
			int code = 56 + static_cast<int>(theValue);
			std::string str;
			str += static_cast<unsigned char>(code);
			return NFmiString(str);
		}
	}
}

static NFmiString GetCmStr(float theValue)
{
	if(theValue == kFloatMissing)
		return NFmiString("");
	else
	{
		if(theValue < 1 || theValue > 9)
			return NFmiString("");
		else
		{
			int code = 66 + static_cast<int>(theValue);
			std::string str;
			str += static_cast<unsigned char>(code);
			return NFmiString(str);
		}
	}
}

static NFmiString GetChStr(float theValue)
{
	if(theValue == kFloatMissing)
		return NFmiString("");
	else
	{
		if(theValue < 1 || theValue > 9)
			return NFmiString("");
		else
		{
			int code = 76 + static_cast<int>(theValue);
			std::string str;
			str += static_cast<unsigned char>(code);
			return NFmiString(str);
		}
	}
}

NFmiString NFmiClCmChSymbolTextView::GetPrintedText(float theValue)
{
	FmiParameterName parName = static_cast<FmiParameterName>(itsDrawParam->Param().GetParamIdent());
	switch(parName)
	{
	case kFmiLowCloudType:
		return GetClStr(theValue);
	case kFmiMiddleCloudType:
		return GetCmStr(theValue);
	case kFmiHighCloudType:
		return GetChStr(theValue);

	default:
		return NFmiString();
	}
}

// ********************************************************************
// ***************  NFmiCloudSymbolTextView  **************************
// ********************************************************************

static double CalcFontSizeFactor(float theValue)
{ // Oletus: ei missin eik‰ 0 arvoja, ne on karsittu jo.
	int sizeValue = static_cast<int>(theValue) / 1000;
	return sizeValue / 3.;
}

NFmiString NFmiCloudSymbolTextView::GetPrintedText(float theValue)
{
// t‰m‰ luokka ei k‰yt‰ itsIndexedWordList kuten perinn‰n mukaan pit‰isi, tuo lista on vain feikki
	if(theValue != kFloatMissing && theValue > 0)
	{
		int layer = (static_cast<int>(theValue) % 1000) / 100;
		if(layer == 6) // l‰pi pilve‰
		{
			char ch = -102; // t‰m‰ on se zig zag h‰ss‰kk‰
			NFmiString str;
			str += ch;
			return str;
		}
		int type = static_cast<int>(theValue) % 100;
		if(layer == 3) // ala pilve‰
		{
			NFmiString str;
			str += ('8' + type);
			return str;
		}
		if(layer == 4) // keski pilve‰
		{
			NFmiString str;
			str += ('B' + type);
			return str;
		}
		if(layer == 5) // yl‰ pilve‰
		{
			NFmiString str;
			str += ('L' + type);
			return str;
		}
	}
	return NFmiString();
}

void NFmiCloudSymbolTextView::ModifyTextEnvironment(void)
{
	NFmiStationIndexTextView::ModifyTextEnvironment();
	itsGeneralFontSize = itsDrawingEnvironment.GetFontSize();
}

NFmiColor NFmiCloudSymbolTextView::GetBasicParamRelatedSymbolColor(float theValue) const
{
    // You must leap here over parent's method to grandparent's method
    return NFmiStationView::GetBasicParamRelatedSymbolColor(theValue);
}

bool NFmiCloudSymbolTextView::SbdIsFixedSymbolSize() const
{
    return false;
}

NFmiPoint NFmiCloudSymbolTextView::SbdCalcChangingSymbolSize(float value) const
{
    auto baseFontSize = NFmiStationIndexTextView::SbdCalcFixedSymbolSize();
    auto fontSizeFactor = CalcFontSizeFactor(value);
    baseFontSize *= NFmiPoint(fontSizeFactor, fontSizeFactor);
    return baseFontSize;
}

NFmiSymbolColorChangingType NFmiCloudSymbolTextView::SbdGetSymbolColorChangingType() const
{
    return NFmiSymbolColorChangingType::DrawParamSet;
}

// ********************************************************************
// *************  NFmiImageBasedSymbolView  ***************************
// ********************************************************************

NFmiImageBasedSymbolView::NFmiImageBasedSymbolView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea>& theArea
    , NFmiToolBox* theToolBox
    , boost::shared_ptr<NFmiDrawParam>& theDrawParam
    , FmiParameterName theParamIdent
    , NFmiIndexMessageList* theIndexedWordList
    , NFmiPoint theOffSet
    , NFmiPoint theSize
    , int theRowIndex
    , int theColumnIndex)
    :NFmiStationIndexTextView(theMapViewDescTopIndex, theArea
        , theToolBox
        , theDrawParam
        , theParamIdent
        , theIndexedWordList
        , theOffSet
        , theSize
        , theRowIndex
        , theColumnIndex)
{
}

NFmiPoint NFmiImageBasedSymbolView::SbdCalcFixedRelativeDrawObjectSize() const
{
    auto symbolSizeInMM = CalcSymbolSizeInMM();
    NFmiRect symbolRect(CalcSymbolRelativeRect(CurrentLatLon(), symbolSizeInMM));
    return symbolRect.Size();
}

double NFmiImageBasedSymbolView::CalcSymbolSizeInMM() const
{
    double dataRectFactor = 0.85; // Symbolia pit‰‰ hieman pienent‰‰ suhteessa DataRect:iin
    if(itsCtrlViewDocumentInterface->Printing())
        dataRectFactor = 0.6; // Printatessa pit‰‰ pienent‰‰ viel‰ lis‰‰
    auto currentDataRect = CurrentDataRect();
    double relativeSymbolSize = dataRectFactor * (currentDataRect.Width() + currentDataRect.Height()) / 2.;
    auto& graphicalInfo = GetGraphicalInfo();
    double symbolSizeInMM = itsToolBox->HY(relativeSymbolSize) / graphicalInfo.itsPixelsPerMM_y;
    symbolSizeInMM *= ::CalcMMSizeFactor(static_cast<float>(graphicalInfo.itsViewHeightInMM), 1.1f);

    // T‰ss‰ viel‰ viimeisi‰ ep‰toivoisia symboli koko s‰‰tˆj‰ erilaisille karttaruudukko (columns x rows) tyypeille.
    // Yritin tehd‰ pikkuisia hienos‰‰tˆj‰ ett‰ aiemmin tehdyt muutokset CurrentDataRect-laskuissa eiv‰t olisi liian 
    // isoja ja muutokset olisivat oikean suuntaisia.
    auto viewGridSize = itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex);
    double xPerYRatio = viewGridSize.X() / viewGridSize.Y();
    // Isonnetaan symbolia v‰h‰n normi 1x1/2x2 jne. tapauksissa
    double finalSymbolSizeFactor = 1.15;
    if(xPerYRatio == 1)
    {
        // Jos sarakkeita sama kuin rivej‰, halutaan sarake m‰‰r‰n mukaan kasvattaa enenmm‰n koko
        finalSymbolSizeFactor = MathHelper::InterpolateWithTwoPoints(viewGridSize.X(), 1, 4, finalSymbolSizeFactor, finalSymbolSizeFactor * 1.3, finalSymbolSizeFactor, finalSymbolSizeFactor * 1.3);
    }
    else if(xPerYRatio > 1)
    {
        // Jos sarakkeita enemm‰n kuin rivej‰, pienennet‰‰n hieman symboleja
        finalSymbolSizeFactor = MathHelper::InterpolateWithTwoPoints(xPerYRatio, 2, 5, 0.97, 0.92, 0.9, 0.98);
    }
    else if(xPerYRatio < 1)
    {
        // Jos sarakkeita v‰hemm‰n kuin rivej‰, suurennetaan kohtalaisesti symboleja
        finalSymbolSizeFactor = MathHelper::InterpolateWithTwoPoints(xPerYRatio, 0.3, 0.5, 1.4, 1.3, 1.27, 1.41);
    }
    symbolSizeInMM *= finalSymbolSizeFactor;
    return symbolSizeInMM;
}

NFmiPoint NFmiImageBasedSymbolView::SbdCalcFixedSymbolSize() const
{
    // K‰ytet‰‰n image symbolien bulk piirrossa symbolin haluttua [mm] kokoa.
    auto symbolSizeInMM = CalcSymbolSizeInMM();
    return NFmiPoint(symbolSizeInMM, symbolSizeInMM);
}

NFmiPoint NFmiImageBasedSymbolView::GetSpaceOutFontFactor(void)
{
    return NFmiPoint(1, 1);
}

NFmiSymbolColorChangingType NFmiImageBasedSymbolView::SbdGetSymbolColorChangingType() const
{
    return NFmiSymbolColorChangingType::Never;
}

// t‰m‰kin on huono viritys, mutta harvennuskoodi ottaa t‰ss‰ vaiheessa fontti koon huomioon kun
// se laskee miten harvennetaan hila dataa. Nyt pit‰‰ siis laskea fontti koko t‰‰ll‰kin, vaikka
// tuuli vektori ei olekaan fontti pohjainen symboli.
void NFmiImageBasedSymbolView::ModifyTextEnvironment(void)
{
    itsDrawingEnvironment.SetFontSize(CalcFontSize(12, boost::math::iround(MaximumFontSizeFactor() * 48), itsCtrlViewDocumentInterface->Printing()));
}

// ********************************************************************
// *************  NFmiBetterWeatherSymbolView  ************************
// ********************************************************************

NFmiImageMap NFmiBetterWeatherSymbolView::itsBetterWeatherSymbolMap;

NFmiBetterWeatherSymbolView::NFmiBetterWeatherSymbolView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
    , NFmiToolBox * theToolBox
    , boost::shared_ptr<NFmiDrawParam> &theDrawParam
    , FmiParameterName theParamIdent
    , NFmiIndexMessageList * theIndexedWordList
    , NFmiPoint theOffSet
    , NFmiPoint theSize
    , int theRowIndex
    , int theColumnIndex)
:NFmiImageBasedSymbolView(theMapViewDescTopIndex, theArea
        , theToolBox
        , theDrawParam
        , theParamIdent
        , theIndexedWordList
        , theOffSet
        , theSize
        , theRowIndex
        , theColumnIndex)
{
}

NFmiSymbolBulkDrawType NFmiBetterWeatherSymbolView::SbdGetDrawType() const
{
    return NFmiSymbolBulkDrawType::BitmapSymbol1;
}

// HUOM! t‰t‰ pit‰‰ kutsua (GeneralDataDocissa) ennen kuin itse luokkaa saa k‰ytt‰‰!!!!
void NFmiBetterWeatherSymbolView::InitBetterWeatherSymbolMap(const std::string &theWomlDirectory)
{
    if(!NFmiBetterWeatherSymbolView::itsBetterWeatherSymbolMap.Initialized())
    {
        // hae baseFolder NFmiSettings-luokan asetuksista, tee abs.-suht. polku temput
        std::string baseFolder = NFmiSettings::Optional<std::string>("BetterWeatherSymbol::BaseSymbolFolder", "BetterWeatherSymbols");
        baseFolder = NFmiFileSystem::MakeAbsolutePath(baseFolder, theWomlDirectory);
        // hae initFile NFmiSettings-luokan asetuksista, tee abs.-suht. polku temput
        std::string initFile = NFmiSettings::Optional<std::string>("BetterWeatherSymbol::SymbolMapFile", "betterweathersymbolmap.txt");
        initFile = NFmiFileSystem::MakeAbsolutePath(initFile, theWomlDirectory);
        NFmiBetterWeatherSymbolView::itsBetterWeatherSymbolMap.Initialize(baseFolder, initFile);
    }
}

// ************************************************************
// *************  NFmiSmartSymbolView  ************************
// ************************************************************

NFmiImageMap NFmiSmartSymbolView::itsSmartSymbolMap;

NFmiSmartSymbolView::NFmiSmartSymbolView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
    , NFmiToolBox * theToolBox
    , boost::shared_ptr<NFmiDrawParam> &theDrawParam
    , FmiParameterName theParamIdent
    , NFmiIndexMessageList * theIndexedWordList
    , NFmiPoint theOffSet
    , NFmiPoint theSize
    , int theRowIndex
    , int theColumnIndex)
    :NFmiImageBasedSymbolView(theMapViewDescTopIndex, theArea
        , theToolBox
        , theDrawParam
        , theParamIdent
        , theIndexedWordList
        , theOffSet
        , theSize
        , theRowIndex
        , theColumnIndex)
{
}

NFmiSymbolBulkDrawType NFmiSmartSymbolView::SbdGetDrawType() const
{
    return NFmiSymbolBulkDrawType::BitmapSymbol2;
}

// HUOM! t‰t‰ pit‰‰ kutsua (GeneralDataDocissa) ennen kuin itse luokkaa saa k‰ytt‰‰!!!!
void NFmiSmartSymbolView::InitSmartSymbolMap(const std::string &theWomlDirectory)
{
    if(!NFmiSmartSymbolView::itsSmartSymbolMap.Initialized())
    {
        // hae baseFolder NFmiSettings-luokan asetuksista, tee abs.-suht. polku temput
        std::string baseFolder = NFmiSettings::Optional<std::string>("SmartSymbol::BaseSymbolFolder", "SmartSymbols");
        baseFolder = NFmiFileSystem::MakeAbsolutePath(baseFolder, theWomlDirectory);
        // hae initFile NFmiSettings-luokan asetuksista, tee abs.-suht. polku temput
        std::string initFile = NFmiSettings::Optional<std::string>("SmartSymbol::SymbolMapFile", "smartsymbolmap.txt");
        initFile = NFmiFileSystem::MakeAbsolutePath(initFile, theWomlDirectory);
        NFmiSmartSymbolView::itsSmartSymbolMap.Initialize(baseFolder, initFile);
    }
}

// ************************************************************
// *************  NFmiCustomSymbolView  ************************
// ************************************************************

NFmiImageMap NFmiCustomSymbolView::itsCustomSymbolMap;

NFmiCustomSymbolView::NFmiCustomSymbolView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
    , NFmiToolBox * theToolBox
    , boost::shared_ptr<NFmiDrawParam> &theDrawParam
    , FmiParameterName theParamIdent
    , NFmiIndexMessageList * theIndexedWordList
    , NFmiPoint theOffSet
    , NFmiPoint theSize
    , int theRowIndex
    , int theColumnIndex)
    :NFmiImageBasedSymbolView(theMapViewDescTopIndex, theArea
        , theToolBox
        , theDrawParam
        , theParamIdent
        , theIndexedWordList
        , theOffSet
        , theSize
        , theRowIndex
        , theColumnIndex)
{
}

NFmiSymbolBulkDrawType NFmiCustomSymbolView::SbdGetDrawType() const
{
    return NFmiSymbolBulkDrawType::BitmapSymbol3;
}

void NFmiCustomSymbolView::InitCustomSymbolMap(const std::string &theWomlDirectory)
{
    if(!NFmiCustomSymbolView::itsCustomSymbolMap.Initialized())
    {
        // hae baseFolder NFmiSettings-luokan asetuksista, tee abs.-suht. polku temput
        std::string baseFolder = NFmiSettings::Optional<std::string>("CustomSymbol::BaseSymbolFolder", "custom_symbols");
        baseFolder = NFmiFileSystem::MakeAbsolutePath(baseFolder, theWomlDirectory);
        // hae initFile NFmiSettings-luokan asetuksista, tee abs.-suht. polku temput
        std::string initFile = NFmiSettings::Optional<std::string>("CustomSymbol::SymbolMapFile", "customsymbolmap.txt");
        initFile = NFmiFileSystem::MakeAbsolutePath(initFile, theWomlDirectory);
        NFmiCustomSymbolView::itsCustomSymbolMap.Initialize(baseFolder, initFile);
    }
}

