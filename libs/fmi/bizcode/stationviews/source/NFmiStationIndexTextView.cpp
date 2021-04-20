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

#include "boost\math\special_functions\round.hpp"

// ********************************************************************
// ***************  NFmiStationIndexTextView  *************************
// ********************************************************************

NFmiStationIndexTextView::NFmiStationIndexTextView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
												,NFmiToolBox *theToolBox
												,NFmiDrawingEnvironment* theDrawingEnvi
												,boost::shared_ptr<NFmiDrawParam> &theDrawParam
												,FmiParameterName theParamId
												,NFmiIndexMessageList* theIndexedWordList
												,NFmiPoint theOffSet
												,NFmiPoint theSize
												,int theRowIndex
                                                ,int theColumnIndex)
: NFmiStationTextView (theMapViewDescTopIndex, theArea
					   ,theToolBox
					   ,theDrawingEnvi
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
    itsDrawingEnvironment->SetFontSize(CalcFontSize(16, boost::math::iround(MaximumFontSizeFactor() * 72), itsCtrlViewDocumentInterface->Printing())); // synop fontit pit‰‰ laittaa isommiksi kuin normaali teksti
	itsDrawingEnvironment->SetFontType(kSynop);
}

NFmiString NFmiStationIndexTextView::GetPrintedText(float theValue)
{
    return NFmiString(CtrlViewUtils::GetSynopCodeAsSynopFontText(theValue));
}

int NFmiStationIndexTextView::GetApproxmationOfDataTextLength(void)
{
	return 1;
}

void NFmiStationIndexTextView::DrawData(void)
{
	float value = ViewFloatValue();
	if(value == kFloatMissing)
		return;

	NFmiString text(GetPrintedText(value));
	if(text == NFmiString(""))
		return;
	NFmiRect rect(CurrentDataRect());
	NFmiPoint place(rect.Center());
	// pelkk‰ toolbox-alignmentti center (eik‰ mik‰‰n muukaan) vie teksti‰ keskelle y-suunnassa, joten t‰m‰ siirros siirt‰‰ tekstin ihan keskelle
	place.Y(place.Y() - itsToolBox->SY(static_cast<long>(itsDrawingEnvironment->GetFontSize().Y()))/2.);

	ModifyTextColor(value);
	NFmiText tmp(place, text, 0, itsDrawingEnvironment);
	itsToolBox->Convert(&tmp);
}

float NFmiStationIndexTextView::ViewFloatValue(void)
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
        value = NFmiStationView::ViewFloatValue();
	if(itsDrawParam->Param().GetParamIdent() == kFmiPresentWeather)
		value = ::ConvertPossible_WaWa_2_WW(value);
	return value;
}

NFmiColor NFmiStationIndexTextView::GetBasicParamRelatedSymbolColor(float theValue)
{
    return CtrlViewUtils::GetSynopCodeSymbolColor(theValue);
}

void NFmiStationIndexTextView::ModifyTextColor(float theValue)
{
	itsDrawingEnvironment->SetFrameColor(GetSymbolColor(theValue));
}

void NFmiStationIndexTextView::Draw(NFmiToolBox *theGTB)
{
    // ugly way to restore fonttype, other way would be that everytime
    // somewhere is text printed, you have to set the fontType
    FmiFontType oldFontType = itsDrawingEnvironment->GetFontType();
    bool oldBoldState = itsDrawingEnvironment->BoldFont();
    NFmiStationTextView::Draw(theGTB);
    itsDrawingEnvironment->BoldFont(oldBoldState);
    itsDrawingEnvironment->SetFontType(oldFontType);
    // Tarkistetaan myˆs itsInfo pointer, koska kaatumisraportit vihjaisivat ett‰ SmartMet on kaatunut itsInfo->Param(itsParamId) -kutsuun
    // eli kyseess‰ on ilmeisesti ollut nullptr. itsInfo attribuutti kyll‰ nollataan tietyiss‰ paikoin piirtokoodia, mutta en saanut 
    // toistettua ongelmaa mitenk‰‰n.
    if(fDoTimeInterpolation && itsInfo)
        itsInfo->Param(itsParamId);
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
	str += CtrlViewUtils::Color2HtmlColorStr(color);
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

void NFmiStationIndexTextView::DrawSymbolWithWantedBitmap(int minSymbolSize, int maxSymbolSize, NFmiImageMap& imageMap)
{
    float value = ViewFloatValue();
    if(value == kFloatMissing)
        return;
    bool printing = itsCtrlViewDocumentInterface->Printing();
    std::string codeStr = boost::lexical_cast<std::string>(value);

    itsDrawingEnvironment->SetFontSize(CalcFontSize(minSymbolSize, boost::math::iround(MaximumFontSizeFactor() * maxSymbolSize), itsCtrlViewDocumentInterface->Printing()));

    double dataRectFactor = 0.85; // Symbolia pit‰‰ hieman pienent‰‰ suhteessa DataRect:iin
    if(printing)
        dataRectFactor = 0.6; // Printatessa pit‰‰ pienent‰‰ viel‰ lis‰‰
    double relativeSymbolSize = dataRectFactor * (CurrentDataRect().Width() + CurrentDataRect().Height()) / 2.;
    auto& graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
    double symbolSizeInMM = itsToolBox->HY(relativeSymbolSize) / graphicalInfo.itsPixelsPerMM_y;
    symbolSizeInMM *= ::CalcMMSizeFactor(static_cast<float>(graphicalInfo.itsViewHeightInMM), 1.1f);
    double symbolSizeInPixels = graphicalInfo.itsPixelsPerMM_y * symbolSizeInMM;
    Gdiplus::Bitmap* symbolBitmap = imageMap.GetRightSizeImage(symbolSizeInPixels, printing, codeStr);
    NFmiRect symbolRect(CalcSymbolRelativeRect(CurrentLatLon(), symbolSizeInMM));
    CtrlView::DrawAnimationButton(symbolRect, symbolBitmap, itsGdiPlusGraphics, *itsToolBox, printing, itsCtrlViewDocumentInterface->MapViewSizeInPixels(itsMapViewDescTopIndex), 1.f, true);
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
float NFmiStationFogTextView::FogValueToSymbolIndex(float theFogValue)
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

void NFmiStationFogTextView::ModifyTextColor(float theValue)
{
	NFmiStationIndexTextView::ModifyTextColor(FogValueToSymbolIndex(static_cast<float>(theValue)));
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

NFmiColor NFmiTotalCloudinessSymbolTextView::GetBasicParamRelatedSymbolColor(float theValue)
{
    // You must leap here over parent's method to grandparent's method
    return NFmiStationView::GetBasicParamRelatedSymbolColor(theValue);
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

NFmiColor NFmiPrecipitationFormSymbolTextView::GetBasicParamRelatedSymbolColor(float theValue)
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
    , NFmiDrawingEnvironment * theDrawingEnvi
    , boost::shared_ptr<NFmiDrawParam> &theDrawParam
    , FmiParameterName theParamIdent
    , NFmiIndexMessageList * theIndexedWordList
    , NFmiPoint theOffSet
    , NFmiPoint theSize
    , int theRowIndex
    , int theColumnIndex)
: NFmiStationIndexTextView(theMapViewDescTopIndex, theArea
    , theToolBox
    , theDrawingEnvi
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
    itsDrawingEnvironment->SetFontSize(CalcFontSize(16, boost::math::iround(MaximumFontSizeFactor() * 48), itsCtrlViewDocumentInterface->Printing()));
    itsDrawingEnvironment->SetFontType(kMirri);
}

NFmiString NFmiRawMirriFontSymbolTextView::GetPrintedText(float theValue)
{
    {
        if(theValue != kFloatMissing)
        {
            char ch = static_cast<char>(theValue);
            NFmiString str;
            str += ch;
            return str;
        }
    }
    return NFmiString();
}

NFmiPoint NFmiRawMirriFontSymbolTextView::GetSpaceOutFontFactor(void)
{
    return NFmiPoint(0.6, 0.7);
}

NFmiColor NFmiRawMirriFontSymbolTextView::GetBasicParamRelatedSymbolColor(float theValue)
{
    // You must leap here over parent's method to grandparent's method
    return NFmiStationView::GetBasicParamRelatedSymbolColor(theValue);
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

void NFmiCloudSymbolTextView::DrawData(void)
{
	float value = ViewFloatValue();
	if(value == kFloatMissing)
		return;

	NFmiString text(GetPrintedText(value));
	if(text == NFmiString(""))
		return;
	double fontSizeFactor = CalcFontSizeFactor(value);
	NFmiPoint newFontSize(itsGeneralFontSize);
	newFontSize.X(newFontSize.X() * fontSizeFactor);
	newFontSize.Y(newFontSize.Y() * fontSizeFactor);
	itsDrawingEnvironment->SetFontSize(newFontSize);
	NFmiRect rect(CurrentDataRect());
	NFmiPoint place(rect.Center());
	place.Y(place.Y() - itsToolBox->SY(static_cast<long>(itsDrawingEnvironment->GetFontSize().Y()))/2.);

	ModifyTextColor(value);
	NFmiText tmp(place, text, 0, itsDrawingEnvironment);
	itsToolBox->Convert(&tmp);
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
	itsGeneralFontSize = itsDrawingEnvironment->GetFontSize();
}

NFmiColor NFmiCloudSymbolTextView::GetBasicParamRelatedSymbolColor(float theValue)
{
    // You must leap here over parent's method to grandparent's method
    return NFmiStationView::GetBasicParamRelatedSymbolColor(theValue);
}

// ********************************************************************
// *************  NFmiBetterWeatherSymbolView  ************************
// ********************************************************************

NFmiImageMap NFmiBetterWeatherSymbolView::itsBetterWeatherSymbolMap;

NFmiBetterWeatherSymbolView::NFmiBetterWeatherSymbolView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
    , NFmiToolBox * theToolBox
    , NFmiDrawingEnvironment * theDrawingEnvi
    , boost::shared_ptr<NFmiDrawParam> &theDrawParam
    , FmiParameterName theParamIdent
    , NFmiIndexMessageList * theIndexedWordList
    , NFmiPoint theOffSet
    , NFmiPoint theSize
    , int theRowIndex
    , int theColumnIndex)
:NFmiStationIndexTextView(theMapViewDescTopIndex, theArea
        , theToolBox
        , theDrawingEnvi
        , theDrawParam
        , theParamIdent
        , theIndexedWordList
        , theOffSet
        , theSize
        , theRowIndex
        , theColumnIndex)
{
}

void NFmiBetterWeatherSymbolView::DrawSymbols(void)
{
    try
    {
        InitializeGdiplus(itsToolBox, 0);
        NFmiStationIndexTextView::DrawSymbols();
    }
    catch(...)
    {
    }

    CleanGdiplus();
}

void NFmiBetterWeatherSymbolView::DrawData(void)
{
    DrawSymbolWithWantedBitmap(12, 48, NFmiBetterWeatherSymbolView::itsBetterWeatherSymbolMap);
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

NFmiPoint NFmiBetterWeatherSymbolView::GetSpaceOutFontFactor(void)
{
    return NFmiPoint(1, 1);
}

// t‰m‰kin on huono viritys, mutta harvennuskoodi ottaa t‰ss‰ vaiheessa fontti koon huomioon kun
// se laskee miten harvennetaan hila dataa. Nyt pit‰‰ siis laskea fontti koko t‰‰ll‰kin, vaikka
// tuuli vektori ei olekaan fontti pohjainen symboli.
void NFmiBetterWeatherSymbolView::ModifyTextEnvironment(void)
{
    itsDrawingEnvironment->SetFontSize(CalcFontSize(12, boost::math::iround(MaximumFontSizeFactor() * 48), itsCtrlViewDocumentInterface->Printing()));
}

// ************************************************************
// *************  NFmiSmartSymbolView  ************************
// ************************************************************

NFmiImageMap NFmiSmartSymbolView::itsSmartSymbolMap;

NFmiSmartSymbolView::NFmiSmartSymbolView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
    , NFmiToolBox * theToolBox
    , NFmiDrawingEnvironment * theDrawingEnvi
    , boost::shared_ptr<NFmiDrawParam> &theDrawParam
    , FmiParameterName theParamIdent
    , NFmiIndexMessageList * theIndexedWordList
    , NFmiPoint theOffSet
    , NFmiPoint theSize
    , int theRowIndex
    , int theColumnIndex)
    :NFmiStationIndexTextView(theMapViewDescTopIndex, theArea
        , theToolBox
        , theDrawingEnvi
        , theDrawParam
        , theParamIdent
        , theIndexedWordList
        , theOffSet
        , theSize
        , theRowIndex
        , theColumnIndex)
{
}

void NFmiSmartSymbolView::DrawSymbols(void)
{
    try
    {
        InitializeGdiplus(itsToolBox, 0);
        NFmiStationIndexTextView::DrawSymbols();
    }
    catch(...)
    {
    }

    CleanGdiplus();
}

void NFmiSmartSymbolView::DrawData(void)
{
    DrawSymbolWithWantedBitmap(12, 128, NFmiSmartSymbolView::itsSmartSymbolMap);
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

NFmiPoint NFmiSmartSymbolView::GetSpaceOutFontFactor(void)
{
    return NFmiPoint(1, 1);
}

void NFmiSmartSymbolView::ModifyTextEnvironment(void)
{
    itsDrawingEnvironment->SetFontSize(CalcFontSize(12, boost::math::iround(MaximumFontSizeFactor() * 48), itsCtrlViewDocumentInterface->Printing()));
}

// ************************************************************
// *************  NFmiCustomSymbolView  ************************
// ************************************************************

NFmiImageMap NFmiCustomSymbolView::itsCustomSymbolMap;

NFmiCustomSymbolView::NFmiCustomSymbolView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
    , NFmiToolBox * theToolBox
    , NFmiDrawingEnvironment * theDrawingEnvi
    , boost::shared_ptr<NFmiDrawParam> &theDrawParam
    , FmiParameterName theParamIdent
    , NFmiIndexMessageList * theIndexedWordList
    , NFmiPoint theOffSet
    , NFmiPoint theSize
    , int theRowIndex
    , int theColumnIndex)
    :NFmiStationIndexTextView(theMapViewDescTopIndex, theArea
        , theToolBox
        , theDrawingEnvi
        , theDrawParam
        , theParamIdent
        , theIndexedWordList
        , theOffSet
        , theSize
        , theRowIndex
        , theColumnIndex)
{
}

void NFmiCustomSymbolView::DrawSymbols(void)
{
    try
    {
        InitializeGdiplus(itsToolBox, 0);
        NFmiStationIndexTextView::DrawSymbols();
    }
    catch(...)
    {
    }

    CleanGdiplus();
}

void NFmiCustomSymbolView::DrawData(void)
{
    DrawSymbolWithWantedBitmap(12, 128, NFmiCustomSymbolView::itsCustomSymbolMap);
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

NFmiPoint NFmiCustomSymbolView::GetSpaceOutFontFactor(void)
{
    return NFmiPoint(1, 1);
}

void NFmiCustomSymbolView::ModifyTextEnvironment(void)
{
    itsDrawingEnvironment->SetFontSize(CalcFontSize(12, boost::math::iround(MaximumFontSizeFactor() * 48), itsCtrlViewDocumentInterface->Printing()));
}
