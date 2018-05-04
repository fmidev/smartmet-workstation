#include "NFmiMTATempSystem.h"
#include "NFmiSettings.h"
#include "NFmiProducerName.h"
#include "NFmiProducerSystem.h"
#include "NFmiDataStoringHelpers.h"
#include "SettingsFunctions.h"

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/

double NFmiMTATempSystem::itsLatestVersionNumber = 1.0;

// ****************************************************
// ** Tästä alkaa NFmiMTATempSystem::TempInfo osio ****
// ****************************************************

void NFmiMTATempSystem::TempInfo::Write(std::ostream& os) const
{
	os << "// NFmiMTATempSystem::TempInfo::Write..." << std::endl;
	os << "// latlon-point" << std::endl;
	os << itsLatlon << std::endl;
	os << "// producer" << std::endl;
	os << itsProducer << std::endl;
	os << "// time with utc hour + minute + day shift to current day" << std::endl;
	NFmiMetTime usedViewMacroTime = NFmiDataStoringHelpers::GetUsedViewMacroTime();
	NFmiDataStoringHelpers::WriteTimeWithOffsets(usedViewMacroTime, itsTime, os);

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi vielä mahdollinen extra data
	// Kun tulee uusia muuttujia, tee tähän extradatan täyttöä, jotta se saadaan talteen tiedopstoon siten että
	// edelliset versiot eivät mene solmuun vaikka on tullut uutta dataa.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw std::runtime_error("NFmiMTATempSystem::TempInfo::Write failed");
}

void NFmiMTATempSystem::TempInfo::Read(std::istream& is)
{ // toivottavasti olet poistanut kommentit luettavasta streamista!!

	is >> itsLatlon;
	is >> itsProducer;
	// time with utc hour + day shift to current day
	NFmiMetTime usedViewMacroTime = NFmiDataStoringHelpers::GetUsedViewMacroTime();
	NFmiDataStoringHelpers::ReadTimeWithOffsets(usedViewMacroTime, itsTime, is);
	if(is.fail())
		throw std::runtime_error("NFmiMTATempSystem::TempInfo::Read failed");
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi vielä mahdollinen extra data
	is >> extraData;
	// Tässä sitten otetaaan extradatasta talteen uudet muuttujat, mitä on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, käsittele tässä.

	if(is.fail())
		throw std::runtime_error("NFmiMTATempSystem::TempInfo::Read failed");
}

const double gDefaultSecondaryDataFrameWidthFactor = 0.15;

// ******************************************
// ** Tästä alkaa NFmiMTATempSystem osio ****
// ******************************************

NFmiMTATempSystem::NFmiMTATempSystem(void)
:itsCurrentVersionNumber(itsLatestVersionNumber)
,itsTempInfos()
,itsMaxTempsShowed(5)
,itsPossibleProducerList()
,itsSoundingComparisonProducers()
,itsSelectedProducer(-1)
,fTempViewOn(false)
,itsSkewTDegree(45)
,fShowMapMarkers(false)
,itsSoundingColors()
,itsAnimationTimeStepInMinutes(60)
,fLeftMouseDown(false)
,fRightMouseDown(false)
,fInitializationOk(false)
,fUpdateFromViewMacro(false)
,itsWindBarbSpaceOutFactor(0)
,fShowSideView(true)
,itsModelRunCount(0)
,fDrawSecondaryData(false)
,itsSecondaryDataFrameWidthFactor(gDefaultSecondaryDataFrameWidthFactor)
// Näille viiva asetuksille pitää antaa valmiit oletusarvot, koska näitä ei välttämättä löydy asetuksista
,itsWSLineInfo(NFmiColor(), 2, FMI_SOLID, true)
,itsNLineInfo(NFmiColor(), 2, FMI_DASH, true)
,itsRHLineInfo(NFmiColor(), 2, FMI_DOT, true)
,fSoundingTextUpwardWinReg(true)
,fSoundingTimeLockWithMapViewWinReg(false)
{
}

NFmiMTATempSystem::~NFmiMTATempSystem(void)
{
}

void NFmiMTATempSystem::Init(NFmiProducerSystem &theProducerSystem, const std::vector<NFmiProducer>& theExtraSoundingProducers, bool theSoundingTextUpward, bool theSoundingTimeLockWithMapView)
{
	fInitializationOk = false;
    fSoundingTextUpwardWinReg = theSoundingTextUpward;
    fSoundingTimeLockWithMapViewWinReg = theSoundingTimeLockWithMapView;
    InitializeSoundingColors();
    InitProducerList(theProducerSystem, theExtraSoundingProducers);
	fInitializationOk = true; // jos ei poikkeuksia lentänyt laitetaan true:ksi
}

void NFmiMTATempSystem::AddTemp(const TempInfo &theTempInfo)
{
	if(static_cast<int>(itsTempInfos.size()) >= itsMaxTempsShowed)
		ClearTemps();
	itsTempInfos.push_back(theTempInfo);
}

void NFmiMTATempSystem::ClearTemps(void)
{
	itsTempInfos.clear();
}

static std::string MakeValueStr(const checkedVector<double> &theValues)
{
	std::string str;
	int ssize = static_cast<int>(theValues.size());
	for(int i = 0; i < ssize; i++)
	{
		str += NFmiStringTools::Convert<double>(theValues[i]);
		if(i<ssize-1) // viimeisen perään ei laiteta pilkkua
			str += ",";
	}
	return str;
}

static void GetOptionalHelpLineInfoSettings(const std::string &theSettingsBaseKey, NFmiTempLineInfo &theLineInfo)
{
    try
    { // yritetään hakea asetuksista arvoja, mutta jos niitä ei löydy, otetaan poikkeus kiinni ja jätetään oletusarvot
        theLineInfo.Color(SettingsFunctions::GetColorFromSettings(theSettingsBaseKey + "::Color"));
        theLineInfo.LineType(static_cast<FmiPattern>(NFmiSettings::Require<int>(std::string(theSettingsBaseKey + "::Type").c_str())));
        theLineInfo.Thickness(NFmiSettings::Require<int>(std::string(theSettingsBaseKey + "::Thickness").c_str()));
        theLineInfo.DrawLine(NFmiSettings::Require<bool>(std::string(theSettingsBaseKey + "::Draw").c_str()));
    }
    catch(...)
    {
    }
}

static void GetHelpLineInfoSettings(const std::string &theSettingsBaseKey, NFmiTempLineInfo &theLineInfo)
{
	theLineInfo.Color(SettingsFunctions::GetColorFromSettings(theSettingsBaseKey+"::Color"));
	theLineInfo.LineType(static_cast<FmiPattern>(NFmiSettings::Require<int>(std::string(theSettingsBaseKey+"::Type").c_str())));
	theLineInfo.Thickness(NFmiSettings::Require<int>(std::string(theSettingsBaseKey+"::Thickness").c_str()));
	theLineInfo.DrawLine(NFmiSettings::Require<bool>(std::string(theSettingsBaseKey+"::Draw").c_str()));
}

static void SetHelpLineInfoToSettings(const std::string &theSettingsBaseKey, NFmiTempLineInfo &theLineInfo)
{
    SettingsFunctions::SetColorToSettings(theSettingsBaseKey+"::Color", theLineInfo.Color());
	NFmiSettings::Set(theSettingsBaseKey+"::Type", NFmiStringTools::Convert<int>(theLineInfo.LineType()), true);
	NFmiSettings::Set(theSettingsBaseKey+"::Thickness", NFmiStringTools::Convert<int>(theLineInfo.Thickness()), true);
	NFmiSettings::Set(theSettingsBaseKey+"::Draw", NFmiStringTools::Convert<bool>(theLineInfo.DrawLine()), true);
}

static void GetHelpLabelInfoSettings(const std::string &theSettingsBaseKey, NFmiTempLabelInfo &theLabelInfo)
{
	theLabelInfo.TextAlignment(static_cast<FmiDirection>(NFmiSettings::Require<int>(std::string(theSettingsBaseKey+"::Alignment").c_str())));
	theLabelInfo.ClipWithDataRect(NFmiSettings::Require<bool>(std::string(theSettingsBaseKey+"::ClipInDataRect").c_str()));
	theLabelInfo.DrawLabelText(NFmiSettings::Require<bool>(std::string(theSettingsBaseKey+"::DrawLabel").c_str()));
	theLabelInfo.FontSize(NFmiSettings::Require<int>(std::string(theSettingsBaseKey+"::Size").c_str()));
	theLabelInfo.StartPointPixelOffSet(SettingsFunctions::GetPointFromSettings(std::string(theSettingsBaseKey+"::Offset")));
}

static void SetHelpLabelInfoToSettings(const std::string &theSettingsBaseKey, NFmiTempLabelInfo &theLabelInfo)
{
	NFmiSettings::Set(theSettingsBaseKey+"::Alignment", NFmiStringTools::Convert<int>(theLabelInfo.TextAlignment()), true);
	NFmiSettings::Set(theSettingsBaseKey+"::ClipInDataRect", NFmiStringTools::Convert<bool>(theLabelInfo.ClipWithDataRect()), true);
	NFmiSettings::Set(theSettingsBaseKey+"::DrawLabel", NFmiStringTools::Convert<bool>(theLabelInfo.DrawLabelText()), true);
	NFmiSettings::Set(theSettingsBaseKey+"::Size", NFmiStringTools::Convert<int>(theLabelInfo.FontSize()), true);
    SettingsFunctions::SetPointToSettings(theSettingsBaseKey+"::Offset", theLabelInfo.StartPointPixelOffSet());
}

void NFmiMTATempSystem::InitializeSoundingColors(void)
{

	itsMaxTempsShowed = NFmiSettings::Require<int>("MetEditor::TempView::MTAMaxShowed");
	itsSkewTDegree = NFmiSettings::Require<double>("MetEditor::TempView::SkewTDegree");
	int colorCount = NFmiSettings::Require<int>("MetEditor::TempView::ColorCount");
	std::string colorText("MetEditor::TempView::Color");
	for(int i=0; i<colorCount; i++)
	{
		// värin luku vaatii hieman kikkailua stringstreamin kanssa
		std::string settingStr(colorText + NFmiStringTools::Convert<int>(i+1)); // colorText-stringiin pitää saada järjestys numero perään
		itsSoundingColors.push_back(SettingsFunctions::GetColorFromSettings(settingStr));
	}

	// ********* initialisoidaan vielä luotaus näytön kaikki viiva systeemit ********
	// nämä otetaan lopuksi konffi filestä, mutta nyt alustetaan tästä

	itsTAxisStart0Degree = NFmiSettings::Require<double>("MetEditor::TempView::TAxisZero::Start");
	itsTAxisEnd0Degree = NFmiSettings::Require<double>("MetEditor::TempView::TAxisZero::End");
	itsTAxisStart45Degree = NFmiSettings::Require<double>("MetEditor::TempView::TAxisPos::Start");
	itsTAxisEnd45Degree = NFmiSettings::Require<double>("MetEditor::TempView::TAxisPos::End");
	itsTAxisStartNegDegree = NFmiSettings::Require<double>("MetEditor::TempView::TAxisNeg::Start");
	itsTAxisEndNegDegree = NFmiSettings::Require<double>("MetEditor::TempView::TAxisNeg::End");
	itsTemperatureHelpLineStart = NFmiSettings::Require<double>("MetEditor::TempView::THelpLine::Start");
	itsTemperatureHelpLineEnd = NFmiSettings::Require<double>("MetEditor::TempView::THelpLine::End");
	itsTemperatureHelpLineStep = NFmiSettings::Require<double>("MetEditor::TempView::THelpLine::Step");
	GetHelpLineInfoSettings("MetEditor::TempView::HelpLine::Temperature::Line", itsTemperatureHelpLineInfo);
	GetHelpLabelInfoSettings("MetEditor::TempView::HelpLine::Temperature::Label", itsTemperatureHelpLabelInfo);


	itsPAxisStart = NFmiSettings::Require<double>("MetEditor::TempView::PAxis::Start");
	itsPAxisEnd = NFmiSettings::Require<double>("MetEditor::TempView::PAxis::End");
	std::string valuesStr = NFmiSettings::Require<std::string>("MetEditor::TempView::HelpLine::Pressure::Values");
	itsPressureValues = NFmiStringTools::Split<checkedVector<double> >(valuesStr);
	GetHelpLineInfoSettings("MetEditor::TempView::HelpLine::Pressure::Line", itsPressureLineInfo);
	GetHelpLabelInfoSettings("MetEditor::TempView::HelpLine::Pressure::Label", itsPressureLabelInfo);

	valuesStr = NFmiSettings::Require<std::string>("MetEditor::TempView::HelpLine::MixingRatio::Values");
	itsMixingRatioValues = NFmiStringTools::Split<checkedVector<double> >(valuesStr);
	GetHelpLineInfoSettings("MetEditor::TempView::HelpLine::MixingRatio::Line", itsMixingRatioLineInfo);
	GetHelpLabelInfoSettings("MetEditor::TempView::HelpLine::MixingRatio::Label", itsMixingRatioLabelInfo);

	valuesStr = NFmiSettings::Require<std::string>("MetEditor::TempView::HelpLine::MoistAdiabatic::Values");
	itsMoistAdiabaticValues = NFmiStringTools::Split<checkedVector<double> >(valuesStr);
	GetHelpLineInfoSettings("MetEditor::TempView::HelpLine::MoistAdiabatic::Line", itsMoistAdiabaticLineInfo);
	GetHelpLabelInfoSettings("MetEditor::TempView::HelpLine::MoistAdiabatic::Label", itsMoistAdiabaticLabelInfo);

	valuesStr = NFmiSettings::Require<std::string>("MetEditor::TempView::HelpLine::DryAdiabatic::Values");
	itsDryAdiabaticValues = NFmiStringTools::Split<checkedVector<double> >(valuesStr);
	GetHelpLineInfoSettings("MetEditor::TempView::HelpLine::DryAdiabatic::Line", itsDryAdiabaticLineInfo);
	GetHelpLabelInfoSettings("MetEditor::TempView::HelpLine::DryAdiabatic::Label", itsDryAdiabaticLabelInfo);

	GetHelpLineInfoSettings("MetEditor::TempView::HelpLine::AirParcel1::Line", itsAirParcel1LineInfo);
	GetHelpLabelInfoSettings("MetEditor::TempView::HelpLine::AirParcel1::Label", itsAirParcel1LabelInfo);
	GetHelpLineInfoSettings("MetEditor::TempView::HelpLine::AirParcel2::Line", itsAirParcel2LineInfo);
	GetHelpLabelInfoSettings("MetEditor::TempView::HelpLine::AirParcel2::Label", itsAirParcel2LabelInfo);
	GetHelpLineInfoSettings("MetEditor::TempView::HelpLine::AirParcel3::Line", itsAirParcel3LineInfo);
	GetHelpLabelInfoSettings("MetEditor::TempView::HelpLine::AirParcel3::Label", itsAirParcel3LabelInfo);
	GetHelpLineInfoSettings("MetEditor::TempView::HelpLine::WindModificationArea::Line", itsWindModificationAreaLineInfo);

	fDrawWinds = NFmiSettings::Require<bool>("MetEditor::TempView::WindVector::Draw");
	itsWindvectorSizeInPixels = SettingsFunctions::GetPointFromSettings("MetEditor::TempView::WindVector::Size");

	fDrawLegendText = NFmiSettings::Require<bool>("MetEditor::TempView::Legend::Draw");
	itsLegendTextSize = SettingsFunctions::GetPointFromSettings("MetEditor::TempView::Legend::Size");

	itsTemperatureLineInfo.LineType(static_cast<FmiPattern>(NFmiSettings::Require<int>("MetEditor::TempView::Temperature::Line::Type")));
	itsTemperatureLineInfo.Thickness(NFmiSettings::Require<int>("MetEditor::TempView::Temperature::Line::Thickness"));
	itsDewPointLineInfo.LineType(static_cast<FmiPattern>(NFmiSettings::Require<int>("MetEditor::TempView::DewPoint::Line::Type")));
	itsDewPointLineInfo.Thickness(NFmiSettings::Require<int>("MetEditor::TempView::DewPoint::Line::Thickness"));

	GetHelpLabelInfoSettings("MetEditor::TempView::HeightValue::Label", itsHeightValueLabelInfo);
	fDrawOnlyHeightValuesOfFirstDrawedSounding = NFmiSettings::Require<bool>("MetEditor::TempView::HeightValue::OnlyFirstDrawed");

	fShowIndexies = NFmiSettings::Require<bool>("MetEditor::TempView::ShowIndexies");

	itsIndexiesFontSize = NFmiSettings::Require<int>("MetEditor::TempView::IndexiesFontSize");
	itsSoundingTextFontSize = NFmiSettings::Require<int>("MetEditor::TempView::SoundingTextFontSize");
	fShowHodograf = NFmiSettings::Require<bool>("MetEditor::TempView::ShowHodograf");
	fShowCondensationTrailProbabilityLines = NFmiSettings::Require<bool>("MetEditor::TempView::ShowCondensationTrailProbabilityLines");

	fShowKilometerScale = NFmiSettings::Require<bool>("MetEditor::TempView::ShowKilometerScale");
	fShowFlightLevelScale = NFmiSettings::Require<bool>("MetEditor::TempView::ShowFlightLevelScale");
	fShowOnlyFirstSoundingInHodograf = NFmiSettings::Require<bool>("MetEditor::TempView::ShowOnlyFirstSoundingInHodograf");

	itsResetScalesStartP = NFmiSettings::Require<double>("MetEditor::TempView::ResetScalesStartP");
	itsResetScalesEndP = NFmiSettings::Require<double>("MetEditor::TempView::ResetScalesEndP");
	itsResetScalesStartT = NFmiSettings::Require<double>("MetEditor::TempView::ResetScalesStartT");
	itsResetScalesEndT = NFmiSettings::Require<double>("MetEditor::TempView::ResetScalesEndT");
	itsResetScalesSkewTStartT = NFmiSettings::Require<double>("MetEditor::TempView::ResetScalesSkewTStartT");
	itsResetScalesSkewTEndT = NFmiSettings::Require<double>("MetEditor::TempView::ResetScalesSkewTEndT");
	itsWindBarbSpaceOutFactor = NFmiSettings::Require<int>("MetEditor::TempView::WindBarbSpaceOutFactor");
	fShowSideView = NFmiSettings::Require<bool>("MetEditor::TempView::ShowSideView");
	itsModelRunCount = NFmiSettings::Require<int>("MetEditor::TempView::ModelRunCount");

    fDrawSecondaryData = NFmiSettings::Optional<bool>("MetEditor::TempView::DrawSecondaryData", false);
    itsSecondaryDataFrameWidthFactor = NFmiSettings::Optional<double>("MetEditor::TempView::SecondaryDataFrameWidthFactor", 0.15);
    GetOptionalHelpLineInfoSettings("MetEditor::TempView::HelpLine::WS::Line", itsWSLineInfo);
    GetOptionalHelpLineInfoSettings("MetEditor::TempView::HelpLine::N::Line", itsNLineInfo);
    GetOptionalHelpLineInfoSettings("MetEditor::TempView::HelpLine::RH::Line", itsRHLineInfo);
}


// initialisoinnin (InitializeSoundingColors-metodin) vastakohta eli
// tallettaa asetukset NFmiSettings-systeemiin.
void NFmiMTATempSystem::StoreSettings(void)
{
	if(fInitializationOk == false)
		return; // jos alustus on epäonnistunut, ei talletuksia tehdä, koska tuloksena voi olla roskaa

	NFmiSettings::Set("MetEditor::TempView::MTAMaxShowed", NFmiStringTools::Convert<int>(itsMaxTempsShowed), true);
	NFmiSettings::Set("MetEditor::TempView::SkewTDegree", NFmiStringTools::Convert<double>(itsSkewTDegree), true);

	int colorCount = static_cast<int>(itsSoundingColors.size());
	NFmiSettings::Set("MetEditor::TempView::ColorCount", NFmiStringTools::Convert<int>(colorCount), true);
	std::string colorText("MetEditor::TempView::Color");
	for(int i=0; i<colorCount; i++)
	{
		// värin talletus vaatii hieman kikkailua stringstreamin kanssa
		std::string settingStr(colorText + NFmiStringTools::Convert<int>(i+1)); // colorText-stringiin pitää saada järjestys numero perään
        SettingsFunctions::SetColorToSettings(settingStr, itsSoundingColors[i]);
	}

	NFmiSettings::Set("MetEditor::TempView::TAxisZero::Start", NFmiStringTools::Convert<double>(itsTAxisStart0Degree), true);
	NFmiSettings::Set("MetEditor::TempView::TAxisZero::End", NFmiStringTools::Convert<double>(itsTAxisEnd0Degree), true);
	NFmiSettings::Set("MetEditor::TempView::TAxisPos::Start", NFmiStringTools::Convert<double>(itsTAxisStart45Degree), true);
	NFmiSettings::Set("MetEditor::TempView::TAxisPos::End", NFmiStringTools::Convert<double>(itsTAxisEnd45Degree), true);
	NFmiSettings::Set("MetEditor::TempView::TAxisNeg::Start", NFmiStringTools::Convert<double>(itsTAxisStartNegDegree), true);
	NFmiSettings::Set("MetEditor::TempView::TAxisNeg::End", NFmiStringTools::Convert<double>(itsTAxisEndNegDegree), true);
	NFmiSettings::Set("MetEditor::TempView::THelpLine::Start", NFmiStringTools::Convert<double>(itsTemperatureHelpLineStart), true);
	NFmiSettings::Set("MetEditor::TempView::THelpLine::End", NFmiStringTools::Convert<double>(itsTemperatureHelpLineEnd), true);
	NFmiSettings::Set("MetEditor::TempView::THelpLine::Step", NFmiStringTools::Convert<double>(itsTemperatureHelpLineStep), true);

	SetHelpLineInfoToSettings("MetEditor::TempView::HelpLine::Temperature::Line", itsTemperatureHelpLineInfo);
	SetHelpLabelInfoToSettings("MetEditor::TempView::HelpLine::Temperature::Label", itsTemperatureHelpLabelInfo);

	NFmiSettings::Set("MetEditor::TempView::PAxis::Start", NFmiStringTools::Convert<double>(itsPAxisStart), true);
	NFmiSettings::Set("MetEditor::TempView::PAxis::End", NFmiStringTools::Convert<double>(itsPAxisEnd), true);

	NFmiSettings::Set("MetEditor::TempView::HelpLine::Pressure::Values", MakeValueStr(itsPressureValues), true);

	SetHelpLineInfoToSettings("MetEditor::TempView::HelpLine::Pressure::Line", itsPressureLineInfo);
	SetHelpLabelInfoToSettings("MetEditor::TempView::HelpLine::Pressure::Label", itsPressureLabelInfo);

	NFmiSettings::Set("MetEditor::TempView::HelpLine::MixingRatio::Values", MakeValueStr(itsMixingRatioValues), true);
	SetHelpLineInfoToSettings("MetEditor::TempView::HelpLine::MixingRatio::Line", itsMixingRatioLineInfo);
	SetHelpLabelInfoToSettings("MetEditor::TempView::HelpLine::MixingRatio::Label", itsMixingRatioLabelInfo);

	NFmiSettings::Set("MetEditor::TempView::HelpLine::MoistAdiabatic::Values", MakeValueStr(itsMoistAdiabaticValues), true);
	SetHelpLineInfoToSettings("MetEditor::TempView::HelpLine::MoistAdiabatic::Line", itsMoistAdiabaticLineInfo);
	SetHelpLabelInfoToSettings("MetEditor::TempView::HelpLine::MoistAdiabatic::Label", itsMoistAdiabaticLabelInfo);

	NFmiSettings::Set("MetEditor::TempView::HelpLine::DryAdiabatic::Values", MakeValueStr(itsDryAdiabaticValues), true);
	SetHelpLineInfoToSettings("MetEditor::TempView::HelpLine::DryAdiabatic::Line", itsDryAdiabaticLineInfo);
	SetHelpLabelInfoToSettings("MetEditor::TempView::HelpLine::DryAdiabatic::Label", itsDryAdiabaticLabelInfo);

	SetHelpLineInfoToSettings("MetEditor::TempView::HelpLine::AirParcel1::Line", itsAirParcel1LineInfo);
	SetHelpLabelInfoToSettings("MetEditor::TempView::HelpLine::AirParcel1::Label", itsAirParcel1LabelInfo);
	SetHelpLineInfoToSettings("MetEditor::TempView::HelpLine::AirParcel2::Line", itsAirParcel2LineInfo);
	SetHelpLabelInfoToSettings("MetEditor::TempView::HelpLine::AirParcel2::Label", itsAirParcel2LabelInfo);
	SetHelpLineInfoToSettings("MetEditor::TempView::HelpLine::AirParcel3::Line", itsAirParcel3LineInfo);
	SetHelpLabelInfoToSettings("MetEditor::TempView::HelpLine::AirParcel3::Label", itsAirParcel3LabelInfo);
	SetHelpLineInfoToSettings("MetEditor::TempView::HelpLine::WindModificationArea::Line", itsWindModificationAreaLineInfo);

	NFmiSettings::Set("MetEditor::TempView::WindVector::Draw", NFmiStringTools::Convert<bool>(fDrawWinds), true);
    SettingsFunctions::SetPointToSettings("MetEditor::TempView::WindVector::Size", itsWindvectorSizeInPixels);

	NFmiSettings::Set("MetEditor::TempView::Legend::Draw", NFmiStringTools::Convert<bool>(fDrawLegendText), true);
    SettingsFunctions::SetPointToSettings("MetEditor::TempView::Legend::Size", itsLegendTextSize);

	SetHelpLineInfoToSettings("MetEditor::TempView::Temperature::Line", itsTemperatureLineInfo);
	SetHelpLineInfoToSettings("MetEditor::TempView::DewPoint::Line", itsDewPointLineInfo);

	SetHelpLabelInfoToSettings("MetEditor::TempView::HeightValue::Label", itsHeightValueLabelInfo);
	NFmiSettings::Set("MetEditor::TempView::HeightValue::OnlyFirstDrawed", NFmiStringTools::Convert<bool>(fDrawOnlyHeightValuesOfFirstDrawedSounding), true);

	NFmiSettings::Set("MetEditor::TempView::ShowIndexies", NFmiStringTools::Convert<bool>(fShowIndexies), true);
	NFmiSettings::Set("MetEditor::TempView::IndexiesFontSize", NFmiStringTools::Convert<int>(itsIndexiesFontSize), true);
	NFmiSettings::Set("MetEditor::TempView::SoundingTextFontSize", NFmiStringTools::Convert<int>(itsSoundingTextFontSize), true);
	NFmiSettings::Set("MetEditor::TempView::ShowHodograf", NFmiStringTools::Convert<bool>(fShowHodograf), true);
	NFmiSettings::Set("MetEditor::TempView::ShowCondensationTrailProbabilityLines", NFmiStringTools::Convert<bool>(fShowCondensationTrailProbabilityLines), true);

	NFmiSettings::Set("MetEditor::TempView::ShowKilometerScale", NFmiStringTools::Convert<bool>(fShowKilometerScale), true);
	NFmiSettings::Set("MetEditor::TempView::ShowFlightLevelScale", NFmiStringTools::Convert<bool>(fShowFlightLevelScale), true);
	NFmiSettings::Set("MetEditor::TempView::ShowOnlyFirstSoundingInHodograf", NFmiStringTools::Convert<bool>(fShowOnlyFirstSoundingInHodograf), true);

	NFmiSettings::Set("MetEditor::TempView::ResetScalesStartP", NFmiStringTools::Convert<double>(itsResetScalesStartP), true);
	NFmiSettings::Set("MetEditor::TempView::ResetScalesEndP", NFmiStringTools::Convert<double>(itsResetScalesEndP), true);
	NFmiSettings::Set("MetEditor::TempView::ResetScalesStartT", NFmiStringTools::Convert<double>(itsResetScalesStartT), true);
	NFmiSettings::Set("MetEditor::TempView::ResetScalesEndT", NFmiStringTools::Convert<double>(itsResetScalesEndT), true);
	NFmiSettings::Set("MetEditor::TempView::ResetScalesSkewTStartT", NFmiStringTools::Convert<double>(itsResetScalesSkewTStartT), true);
	NFmiSettings::Set("MetEditor::TempView::ResetScalesSkewTEndT", NFmiStringTools::Convert<double>(itsResetScalesSkewTEndT), true);
	NFmiSettings::Set("MetEditor::TempView::WindBarbSpaceOutFactor", NFmiStringTools::Convert<int>(itsWindBarbSpaceOutFactor), true);
	NFmiSettings::Set("MetEditor::TempView::ShowSideView", NFmiStringTools::Convert<bool>(fShowSideView), true);
	NFmiSettings::Set("MetEditor::TempView::ModelRunCount", NFmiStringTools::Convert<int>(itsModelRunCount), true);

    try
    {
        NFmiSettings::Set("MetEditor::TempView::DrawSecondaryData", NFmiStringTools::Convert<bool>(fDrawSecondaryData));
        NFmiSettings::Set("MetEditor::TempView::SecondaryDataFrameWidthFactor", NFmiStringTools::Convert<double>(itsSecondaryDataFrameWidthFactor));
        SetHelpLineInfoToSettings("MetEditor::TempView::HelpLine::WS::Line", itsWSLineInfo);
        SetHelpLineInfoToSettings("MetEditor::TempView::HelpLine::N::Line", itsNLineInfo);
        SetHelpLineInfoToSettings("MetEditor::TempView::HelpLine::RH::Line", itsRHLineInfo);
    }
    catch(...)
    {
    }
}

const NFmiColor& NFmiMTATempSystem::SoundingColor(int theIndex)
{
	static NFmiColor dummy(0.f,0.f,0.f);  // musta on dummy väri
	if(theIndex >= 0 && theIndex < static_cast<int>(itsSoundingColors.size()))
		return itsSoundingColors[theIndex];
	else
		return dummy;
}

void NFmiMTATempSystem::SoundingColor(int theIndex, const NFmiColor &theColor)
{
	if(theIndex >= 0 && theIndex < static_cast<int>(itsSoundingColors.size()))
		itsSoundingColors[theIndex] = theColor;
}

double NFmiMTATempSystem::TAxisStart(void)
{
	if(itsSkewTDegree > 0)
		return itsTAxisStart45Degree;
	else if(itsSkewTDegree == 0)
		return itsTAxisStart0Degree;
	else
		return itsTAxisStartNegDegree;
}

double NFmiMTATempSystem::TAxisEnd(void)
{
	if(itsSkewTDegree > 0)
		return itsTAxisEnd45Degree;
	else if(itsSkewTDegree == 0)
		return itsTAxisEnd0Degree;
	else
		return itsTAxisEndNegDegree;
}

void NFmiMTATempSystem::TAxisStart(double newValue)
{
	if(itsSkewTDegree > 0)
	{
		if(newValue < itsTAxisEnd45Degree)
			itsTAxisStart45Degree = newValue;
	}
	else if(itsSkewTDegree == 0)
	{
		if(newValue < itsTAxisEnd0Degree)
			itsTAxisStart0Degree = newValue;
	}
	else
	{
		if(newValue < itsTAxisEndNegDegree)
			itsTAxisStartNegDegree = newValue;
	}
}

void NFmiMTATempSystem::TAxisEnd(double newValue)
{
	if(itsSkewTDegree > 0)
	{
		if(newValue > itsTAxisStart45Degree)
			itsTAxisEnd45Degree = newValue;
	}
	else if(itsSkewTDegree == 0)
	{
		if(newValue > itsTAxisStart0Degree)
			itsTAxisEnd0Degree = newValue;
	}
	else
	{
		if(newValue > itsTAxisStartNegDegree)
			itsTAxisEndNegDegree = newValue;
	}
}

void NFmiMTATempSystem::PAxisStart(double newValue)
{
	if(newValue > itsPAxisEnd)
		itsPAxisStart = newValue;
}

void NFmiMTATempSystem::PAxisEnd(double newValue)
{
	if(newValue < itsPAxisStart)
	{
		if(newValue >= 1)
			itsPAxisEnd = newValue;
	}
}

void NFmiMTATempSystem::ChangeSoundingsInTime(FmiDirection theDirection)
{
	for(Container::iterator it = itsTempInfos.begin(); it != itsTempInfos.end(); ++it)
	{
		NFmiMetTime tim((*it).Time());
		tim.SetTimeStep(1);
		if(theDirection == kForward)
			tim.ChangeByMinutes(itsAnimationTimeStepInMinutes);
		else
			tim.ChangeByMinutes(-itsAnimationTimeStepInMinutes);
		(*it).Time(tim);
	}
}

void NFmiMTATempSystem::NextAnimationStep(void)
{
	if(itsAnimationTimeStepInMinutes < 30)
		itsAnimationTimeStepInMinutes = 30;
	else if(itsAnimationTimeStepInMinutes < 60)
		itsAnimationTimeStepInMinutes = 60;
	else if(itsAnimationTimeStepInMinutes < 120)
		itsAnimationTimeStepInMinutes = 120;
	else if(itsAnimationTimeStepInMinutes < 180)
		itsAnimationTimeStepInMinutes = 180;
	else if(itsAnimationTimeStepInMinutes < 240)
		itsAnimationTimeStepInMinutes = 240;
	else if(itsAnimationTimeStepInMinutes < 360)
		itsAnimationTimeStepInMinutes = 360;
	else if(itsAnimationTimeStepInMinutes < 720)
		itsAnimationTimeStepInMinutes = 720;
	else
		itsAnimationTimeStepInMinutes = 1440;
}

void NFmiMTATempSystem::PreviousAnimationStep(void)
{
	if(itsAnimationTimeStepInMinutes > 1440)
		itsAnimationTimeStepInMinutes = 1440;
	else if(itsAnimationTimeStepInMinutes > 720)
		itsAnimationTimeStepInMinutes = 720;
	else if(itsAnimationTimeStepInMinutes > 360)
		itsAnimationTimeStepInMinutes = 360;
	else if(itsAnimationTimeStepInMinutes > 240)
		itsAnimationTimeStepInMinutes = 240;
	else if(itsAnimationTimeStepInMinutes > 180)
		itsAnimationTimeStepInMinutes = 180;
	else if(itsAnimationTimeStepInMinutes > 120)
		itsAnimationTimeStepInMinutes = 120;
	else if(itsAnimationTimeStepInMinutes > 60)
		itsAnimationTimeStepInMinutes = 60;
	else
		itsAnimationTimeStepInMinutes = 30;
}

// initialisoi tuottaja lista
void NFmiMTATempSystem::InitProducerList(NFmiProducerSystem &theProducerSystem, const std::vector<NFmiProducer>& theExtraSoundingProducers)
{
	itsPossibleProducerList.clear();
	// 1. edited + havainto + TEMP (=raaka-luotaukset)
	itsPossibleProducerList.push_back(NFmiProducer(kFmiMETEOR, "Edited"));
    itsPossibleProducerList.push_back(NFmiProducer(kFmiTEMP, "Sounding"));
    itsPossibleProducerList.push_back(NFmiProducer(kFmiBufrTEMP, "BufrSounding"));
	itsPossibleProducerList.push_back(NFmiProducer(kFmiRAWTEMP, "TEMP"));

	size_t i=0;
	// 2. malli datat jotka on määritelty docin producer listassa
	size_t modelCount = theProducerSystem.Producers().size();
	for(i=0; i<modelCount; i++)
	{
		if(theProducerSystem.Producer(static_cast<unsigned int>(i+1)).HasRealVerticalData())
        {
            NFmiProducerInfo &producerInfo = theProducerSystem.Producer(static_cast<unsigned int>(i+1)); // +1 johtuu producersystemin 1-pohjaisesta indeksi systeemistä
            NFmiProducer producer = producerInfo.GetProducer();
            const unsigned long nameLengthLimit = 12; // Jos tuottajan normaalinimi menee yli tämä rajan, käytetään ShortName:a
            if(producer.GetName().GetLen() > nameLengthLimit)
                producer.SetName(producerInfo.ShortName());
			itsPossibleProducerList.push_back(producer);
        }
	}
	// 3. extra tuottajat, jotka saadaan myös docilta
    for(i = 0; i < theExtraSoundingProducers.size(); i++)
        itsPossibleProducerList.push_back(theExtraSoundingProducers[i]);
}

const NFmiProducer& NFmiMTATempSystem::CurrentProducer(void) const
{
	static NFmiProducer dummy(0, "Error Producer");
	if(itsSelectedProducer>=0 && itsSelectedProducer < static_cast<int>(itsPossibleProducerList.size()))
		return itsPossibleProducerList[itsSelectedProducer];
	return dummy;
}

void NFmiMTATempSystem::CurrentProducer(const NFmiProducer &newValue)
{
	itsSelectedProducer = -1;
	for(int i=0; i<static_cast<int>(itsPossibleProducerList.size()); i++)
	{
		if(itsPossibleProducerList[i].GetIdent() == newValue.GetIdent())
		{
			itsSelectedProducer = i;
			break;
		}
	}
}

void NFmiMTATempSystem::WindBarbSpaceOutFactor(int newValue)
{
	itsWindBarbSpaceOutFactor = newValue;
	if(itsWindBarbSpaceOutFactor > 2)
		itsWindBarbSpaceOutFactor = 0;
}

void NFmiMTATempSystem::ChangeWindBarbSpaceOutFactor(void)
{
	WindBarbSpaceOutFactor(WindBarbSpaceOutFactor()+1);
}

void NFmiMTATempSystem::Write(std::ostream& os) const
{
	os << "// NFmiMTATempSystem::Write..." << std::endl;

	os << "// version number" << std::endl;
	itsCurrentVersionNumber = itsLatestVersionNumber; // aina kirjoitetaan viimeisellä versio numerolla
	os << itsCurrentVersionNumber << std::endl;

	os << "// Container<TempInfos>" << std::endl;
	NFmiDataStoringHelpers::WriteContainer(itsTempInfos, os, std::string("\n"));

	os << "// MaxTempsShowed" << std::endl;
	os << itsMaxTempsShowed << std::endl;

	os << "// Selected producers checkedVector<NFmiProducer>" << std::endl;
	NFmiDataStoringHelpers::WriteContainer(itsSoundingComparisonProducers, os, std::string("\n"));

	os << "// SelectedProducer index + tempViewOn + SkewTDegree" << std::endl;
	os << itsSelectedProducer << " " << fTempViewOn << " " << itsSkewTDegree << std::endl;

	os << "// SoundingColors checkedVector<NFmiColor>" << std::endl;
	NFmiDataStoringHelpers::WriteContainer(itsSoundingColors, os, std::string("\n"));

	os << "// TAxisStart0Degree + TAxisEnd0Degree + TAxisStart45Degree + TAxisEnd45Degree + TAxisStartNegDegree + TAxisEndNegDegree" << std::endl;
	os << itsTAxisStart0Degree << " " << itsTAxisEnd0Degree << " " << itsTAxisStart45Degree << " " << itsTAxisEnd45Degree << " " << itsTAxisStartNegDegree << " " << itsTAxisEndNegDegree << std::endl;

	os << "// TemperatureHelpLineStart + TemperatureHelpLineEnd + TemperatureHelpLineStep" << std::endl;
	os << itsTemperatureHelpLineStart << " " << itsTemperatureHelpLineEnd << " " << itsTemperatureHelpLineStep << std::endl;

	os << "// TemperatureHelpLineInfo" << std::endl;
	os << itsTemperatureHelpLineInfo << std::endl;
	os << "// TemperatureHelpLabelInfo" << std::endl;
	os << itsTemperatureHelpLabelInfo << std::endl;

	os << "// PAxisStart + PAxisEnd" << std::endl;
	os << itsPAxisStart << " " << itsPAxisEnd << std::endl;

	os << "// PressureValues" << std::endl;
	NFmiDataStoringHelpers::WriteContainer(itsPressureValues, os, std::string(" "));
	os << std::endl;
	os << "// PressureLineInfo" << std::endl;
	os << itsPressureLineInfo << std::endl;
	os << "// PressureLabelInfo" << std::endl;
	os << itsPressureLabelInfo << std::endl;

	os << "// MixingRatioValues" << std::endl;
	NFmiDataStoringHelpers::WriteContainer(itsMixingRatioValues, os, std::string(" "));
	os << std::endl;
	os << "// MixingRatioLineInfo" << std::endl;
	os << itsMixingRatioLineInfo << std::endl;
	os << "// MixingRatioLabelInfo" << std::endl;
	os << itsMixingRatioLabelInfo << std::endl;

	os << "// MoistAdiabaticValues" << std::endl;
	NFmiDataStoringHelpers::WriteContainer(itsMoistAdiabaticValues, os, std::string(" "));
	os << std::endl;
	os << "// MoistAdiabaticLineInfo" << std::endl;
	os << itsMoistAdiabaticLineInfo << std::endl;
	os << "// MoistAdiabaticLabelInfo" << std::endl;
	os << itsMoistAdiabaticLabelInfo << std::endl;

	os << "// DryAdiabaticValues" << std::endl;
	NFmiDataStoringHelpers::WriteContainer(itsDryAdiabaticValues, os, std::string(" "));
	os << std::endl;
	os << "// DryAdiabaticLineInfo" << std::endl;
	os << itsDryAdiabaticLineInfo << std::endl;
	os << "// DryAdiabaticLabelInfo" << std::endl;
	os << itsDryAdiabaticLabelInfo << std::endl;

	os << "// AirParcel1LineInfo" << std::endl;
	os << itsAirParcel1LineInfo << std::endl;
	os << "// AirParcel1LabelInfo" << std::endl;
	os << itsAirParcel1LabelInfo << std::endl;

	os << "// AirParcel2LineInfo" << std::endl;
	os << itsAirParcel2LineInfo << std::endl;
	os << "// AirParcel2LabelInfo" << std::endl;
	os << itsAirParcel2LabelInfo << std::endl;

	os << "// AirParcel3LineInfo" << std::endl;
	os << itsAirParcel3LineInfo << std::endl;
	os << "// AirParcel3LabelInfo" << std::endl;
	os << itsAirParcel3LabelInfo << std::endl;

	os << "// WindModificationAreaLineInfo" << std::endl;
	os << itsWindModificationAreaLineInfo << std::endl;

	os << "// DrawWinds + WindvectorSizeInPixels" << std::endl;
	os << fDrawWinds << " " << itsWindvectorSizeInPixels;
	os << "// DrawLegendText + LegendTextSize" << std::endl;
	os << fDrawLegendText << " " << itsLegendTextSize << std::endl;

	os << "// TemperatureLineInfo" << std::endl;
	os << itsTemperatureLineInfo << std::endl;
	os << "// DewPointLineInfo" << std::endl;
	os << itsDewPointLineInfo << std::endl;

	os << "// HeightValueLabelInfo" << std::endl;
	os << itsHeightValueLabelInfo << std::endl;
	os << "// DrawOnlyHeightValuesOfFirstDrawedSounding" << std::endl;
	os << fDrawOnlyHeightValuesOfFirstDrawedSounding << std::endl;

	os << "// ShowIndexies + IndexiesFontSize + SoundingTextFontSize" << std::endl;
	os << fShowIndexies << " " << itsIndexiesFontSize << " " << itsSoundingTextFontSize << std::endl;

	os << "// ShowHodograf + ShowCondensationTrailProbabilityLines + ShowKilometerScale + ShowFlightLevelScale + fShowMapMarkers + ShowOnlyFirstSoundingInHodograf" << std::endl;
	os << fShowHodograf << " " << fShowCondensationTrailProbabilityLines << " " << fShowKilometerScale << " " << fShowFlightLevelScale << " " << fShowMapMarkers << " " << fShowOnlyFirstSoundingInHodograf << std::endl;

	os << "// ResetScalesStartP + ResetScalesEndP + ResetScalesStartT + ResetScalesEndT + ResetScalesSkewTStartT + ResetScalesSkewTEndT" << std::endl;
	os << itsResetScalesStartP << " " << itsResetScalesEndP << " " << itsResetScalesStartT << " " << itsResetScalesEndT << " " << itsResetScalesSkewTStartT << " " << itsResetScalesSkewTEndT << std::endl;

	os << "// AnimationTimeStepInMinutes" << std::endl;
	os << itsAnimationTimeStepInMinutes << std::endl;

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi vielä mahdollinen extra data
	// Kun tulee uusia muuttujia, tee tähän extradatan täyttöä, jotta se saadaan talteen tiedopstoon siten että
	// edelliset versiot eivät mene solmuun vaikka on tullut uutta dataa.
    extraData.Add(static_cast<double>(itsWindBarbSpaceOutFactor)); // itsWindBarbSpaceOutFactor on 1. uusi double arvo
    extraData.Add(static_cast<double>(fShowSideView)); // fShowSideView on 2. uusi double arvo
    extraData.Add(static_cast<double>(itsModelRunCount)); // itsModelRunCount on 3. uusi double arvo
    extraData.Add(static_cast<double>(fDrawSecondaryData)); // fDrawSecondaryData on 4. uusi double arvo
    extraData.Add(itsSecondaryDataFrameWidthFactor); // itsSecondaryDataFrameWidthFactor on 5. uusi double arvo
    extraData.Add(static_cast<double>(fSoundingTextUpwardWinReg)); // fSoundingTextUpwardWinReg on 6. uusi double arvo
    extraData.Add(static_cast<double>(fSoundingTimeLockWithMapViewWinReg)); // fSoundingTimeLockWithMapViewWinReg on 7. uusi double arvo

    extraData.Add(MakeSecondaryDataLineInfoString()); // WS + N + RH lineInfor yhtenä stringinä on 1. uusi string arvo extroissa

	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw std::runtime_error("NFmiMTATempSystem::Write failed");
}

std::string NFmiMTATempSystem::MakeSecondaryDataLineInfoString() const
{
    std::stringstream out;
    itsWSLineInfo.WriteBasicData(out);
    out << " ";
    itsNLineInfo.WriteBasicData(out);
    out << " ";
    itsRHLineInfo.WriteBasicData(out);
    return out.str();
}

void NFmiMTATempSystem::ReadSecondaryDataLineInfoFromString(const std::string &theStr)
{
    std::stringstream in(theStr);
    in >> itsWSLineInfo >> itsNLineInfo >> itsRHLineInfo;
}

void NFmiMTATempSystem::Read(std::istream& is)
{ // toivottavasti olet poistanut kommentit luettavasta streamista!!

	is >> itsCurrentVersionNumber;
	if(itsCurrentVersionNumber > itsLatestVersionNumber)
		throw std::runtime_error("NFmiMTATempSystem::Read failed the version number war newer than program can handle.");

	if(is.fail())
		throw std::runtime_error("NFmiMTATempSystem::Read failed");
	NFmiDataStoringHelpers::ReadContainer(itsTempInfos, is);

	is >> itsMaxTempsShowed;

	if(is.fail())
		throw std::runtime_error("NFmiMTATempSystem::Read failed");
	NFmiDataStoringHelpers::ReadContainer(itsSoundingComparisonProducers, is);

	is >> itsSelectedProducer >> fTempViewOn >> itsSkewTDegree;

	if(is.fail())
		throw std::runtime_error("NFmiMTATempSystem::Read failed");
	NFmiDataStoringHelpers::ReadContainer(itsSoundingColors, is);

	is >> itsTAxisStart0Degree >> itsTAxisEnd0Degree >> itsTAxisStart45Degree >> itsTAxisEnd45Degree >> itsTAxisStartNegDegree >> itsTAxisEndNegDegree;

	is >> itsTemperatureHelpLineStart >> itsTemperatureHelpLineEnd >> itsTemperatureHelpLineStep;

	is >> itsTemperatureHelpLineInfo;
	is >> itsTemperatureHelpLabelInfo;

	is >> itsPAxisStart >> itsPAxisEnd;

	if(is.fail())
		throw std::runtime_error("NFmiMTATempSystem::Read failed");
	NFmiDataStoringHelpers::ReadContainer(itsPressureValues, is);

	is >> itsPressureLineInfo;
	is >> itsPressureLabelInfo;

	if(is.fail())
		throw std::runtime_error("NFmiMTATempSystem::Read failed");
	NFmiDataStoringHelpers::ReadContainer(itsMixingRatioValues, is);

	is >> itsMixingRatioLineInfo;
	is >> itsMixingRatioLabelInfo;

	if(is.fail())
		throw std::runtime_error("NFmiMTATempSystem::Read failed");
	NFmiDataStoringHelpers::ReadContainer(itsMoistAdiabaticValues, is);

	is >> itsMoistAdiabaticLineInfo;
	is >> itsMoistAdiabaticLabelInfo;

	if(is.fail())
		throw std::runtime_error("NFmiMTATempSystem::Read failed");
	NFmiDataStoringHelpers::ReadContainer(itsDryAdiabaticValues, is);

	is >> itsDryAdiabaticLineInfo;
	is >> itsDryAdiabaticLabelInfo;

	is >> itsAirParcel1LineInfo;
	is >> itsAirParcel1LabelInfo;

	is >> itsAirParcel2LineInfo;
	is >> itsAirParcel2LabelInfo;

	is >> itsAirParcel3LineInfo;
	is >> itsAirParcel3LabelInfo;

	is >> itsWindModificationAreaLineInfo;

	is >> fDrawWinds >> itsWindvectorSizeInPixels;
	is >> fDrawLegendText >> itsLegendTextSize;

	if(is.fail())
		throw std::runtime_error("NFmiMTATempSystem::Read failed");

	is >> itsTemperatureLineInfo;
	is >> itsDewPointLineInfo;

	is >> itsHeightValueLabelInfo;
	is >> fDrawOnlyHeightValuesOfFirstDrawedSounding;

	is >> fShowIndexies >> itsIndexiesFontSize >> itsSoundingTextFontSize;

	is >> fShowHodograf >> fShowCondensationTrailProbabilityLines >> fShowKilometerScale >> fShowFlightLevelScale >> fShowMapMarkers >> fShowOnlyFirstSoundingInHodograf;

	is >> itsResetScalesStartP >> itsResetScalesEndP >> itsResetScalesStartT >> itsResetScalesEndT >> itsResetScalesSkewTStartT >> itsResetScalesSkewTEndT;

	is >> itsAnimationTimeStepInMinutes;

	if(is.fail())
		throw std::runtime_error("NFmiMTATempSystem::Read failed");
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi vielä mahdollinen extra data
	is >> extraData;
	// Tässä sitten otetaaan extradatasta talteen uudet muuttujat, mitä on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, käsittele tässä.
	itsWindBarbSpaceOutFactor = 0;
    if(extraData.itsDoubleValues.size() > 0)
        itsWindBarbSpaceOutFactor = static_cast<int>(extraData.itsDoubleValues[0]);
	fShowSideView = true;
    if(extraData.itsDoubleValues.size() > 1)
        fShowSideView = extraData.itsDoubleValues[1] != 0.;
	itsModelRunCount = 0;
    if(extraData.itsDoubleValues.size() > 2)
        itsModelRunCount = static_cast<int>(extraData.itsDoubleValues[2]);
    fDrawSecondaryData = false;
    if(extraData.itsDoubleValues.size() > 3)
        fDrawSecondaryData = extraData.itsDoubleValues[3] != 0;
    itsSecondaryDataFrameWidthFactor = gDefaultSecondaryDataFrameWidthFactor;
    if(extraData.itsDoubleValues.size() > 4)
        itsSecondaryDataFrameWidthFactor = extraData.itsDoubleValues[4];
    fSoundingTextUpwardWinReg = true;
    if(extraData.itsDoubleValues.size() > 5)
        fSoundingTextUpwardWinReg = extraData.itsDoubleValues[5] != 0;
    fSoundingTimeLockWithMapViewWinReg = false;
    if(extraData.itsDoubleValues.size() > 6)
        fSoundingTimeLockWithMapViewWinReg = extraData.itsDoubleValues[6] != 0;

    if(extraData.itsStringValues.size() > 0)
        ReadSecondaryDataLineInfoFromString(extraData.itsStringValues[0]);

	if(is.fail())
		throw std::runtime_error("NFmiMTATempSystem::Read failed");

	itsCurrentVersionNumber = itsLatestVersionNumber; // aina jatketaan viimeisellä versio numerolla
}

void NFmiMTATempSystem::InitFromViewMacro(const NFmiMTATempSystem &theOther)
{
	itsCurrentVersionNumber = theOther.itsCurrentVersionNumber;
	itsTempInfos = theOther.itsTempInfos;
	itsMaxTempsShowed = theOther.itsMaxTempsShowed;
	itsSoundingComparisonProducers = theOther.itsSoundingComparisonProducers;
	itsSelectedProducer = theOther.itsSelectedProducer;
	fTempViewOn = theOther.fTempViewOn;
	itsSkewTDegree = theOther.itsSkewTDegree;
	itsSoundingColors = theOther.itsSoundingColors;
	itsTAxisStart0Degree = theOther.itsTAxisStart0Degree;
	itsTAxisEnd0Degree = theOther.itsTAxisEnd0Degree;
	itsTAxisStart45Degree = theOther.itsTAxisStart45Degree;
	itsTAxisEnd45Degree = theOther.itsTAxisEnd45Degree;
	itsTAxisStartNegDegree = theOther.itsTAxisStartNegDegree;
	itsTAxisEndNegDegree = theOther.itsTAxisEndNegDegree;
	itsTemperatureHelpLineStart = theOther.itsTemperatureHelpLineStart;
	itsTemperatureHelpLineEnd = theOther.itsTemperatureHelpLineEnd;
	itsTemperatureHelpLineStep = theOther.itsTemperatureHelpLineStep;
	itsTemperatureHelpLineInfo = theOther.itsTemperatureHelpLineInfo;
	itsTemperatureHelpLabelInfo = theOther.itsTemperatureHelpLabelInfo;
	itsPAxisStart = theOther.itsPAxisStart;
	itsPAxisEnd = theOther.itsPAxisEnd;
	itsPressureValues = theOther.itsPressureValues;
	itsPressureLineInfo = theOther.itsPressureLineInfo;
	itsPressureLabelInfo = theOther.itsPressureLabelInfo;
	itsMixingRatioValues = theOther.itsMixingRatioValues;
	itsMixingRatioLineInfo = theOther.itsMixingRatioLineInfo;
	itsMixingRatioLabelInfo = theOther.itsMixingRatioLabelInfo;
	itsMoistAdiabaticValues = theOther.itsMoistAdiabaticValues;
	itsMoistAdiabaticLineInfo = theOther.itsMoistAdiabaticLineInfo;
	itsMoistAdiabaticLabelInfo = theOther.itsMoistAdiabaticLabelInfo;
	itsDryAdiabaticValues = theOther.itsDryAdiabaticValues;
	itsDryAdiabaticLineInfo = theOther.itsDryAdiabaticLineInfo;
	itsDryAdiabaticLabelInfo = theOther.itsDryAdiabaticLabelInfo;
	itsAirParcel1LineInfo = theOther.itsAirParcel1LineInfo;
	itsAirParcel1LabelInfo = theOther.itsAirParcel1LabelInfo;
	itsAirParcel2LineInfo = theOther.itsAirParcel2LineInfo;
	itsAirParcel2LabelInfo = theOther.itsAirParcel2LabelInfo;
	itsAirParcel3LineInfo = theOther.itsAirParcel3LineInfo;
	itsAirParcel3LabelInfo = theOther.itsAirParcel3LabelInfo;
	itsWindModificationAreaLineInfo = theOther.itsWindModificationAreaLineInfo;
	fDrawWinds = theOther.fDrawWinds;
	itsWindvectorSizeInPixels = theOther.itsWindvectorSizeInPixels;
	fDrawLegendText = theOther.fDrawLegendText;
	itsLegendTextSize = theOther.itsLegendTextSize;
	itsTemperatureLineInfo = theOther.itsTemperatureLineInfo;
	itsDewPointLineInfo = theOther.itsDewPointLineInfo;
	itsHeightValueLabelInfo = theOther.itsHeightValueLabelInfo;
	fDrawOnlyHeightValuesOfFirstDrawedSounding = theOther.fDrawOnlyHeightValuesOfFirstDrawedSounding;
	fShowIndexies = theOther.fShowIndexies;
	itsIndexiesFontSize = theOther.itsIndexiesFontSize;
	itsSoundingTextFontSize = theOther.itsSoundingTextFontSize;
	fShowHodograf = theOther.fShowHodograf;
	fShowCondensationTrailProbabilityLines = theOther.fShowCondensationTrailProbabilityLines;
	fShowKilometerScale = theOther.fShowKilometerScale;
	fShowFlightLevelScale = theOther.fShowFlightLevelScale;
	fShowMapMarkers = theOther.fShowMapMarkers;
	fShowOnlyFirstSoundingInHodograf = theOther.fShowOnlyFirstSoundingInHodograf;
	itsResetScalesStartP = theOther.itsResetScalesStartP;
	itsResetScalesEndP = theOther.itsResetScalesEndP;
	itsResetScalesStartT = theOther.itsResetScalesStartT;
	itsResetScalesEndT = theOther.itsResetScalesEndT;
	itsResetScalesSkewTStartT = theOther.itsResetScalesSkewTStartT;
	itsResetScalesSkewTEndT = theOther.itsResetScalesSkewTEndT;
	itsAnimationTimeStepInMinutes = theOther.itsAnimationTimeStepInMinutes;
	itsWindBarbSpaceOutFactor = theOther.itsWindBarbSpaceOutFactor;
	fShowSideView = theOther.fShowSideView;
    itsModelRunCount = theOther.itsModelRunCount;
    fDrawSecondaryData = theOther.fDrawSecondaryData;
    itsSecondaryDataFrameWidthFactor = theOther.itsSecondaryDataFrameWidthFactor;
    itsWSLineInfo = theOther.itsWSLineInfo;
    itsNLineInfo = theOther.itsNLineInfo;
    itsRHLineInfo = theOther.itsRHLineInfo;
    fSoundingTextUpwardWinReg = theOther.fSoundingTextUpwardWinReg;
    fSoundingTimeLockWithMapViewWinReg = theOther.fSoundingTimeLockWithMapViewWinReg;
}

// Säädetään kaikki aikaa liittyvät jutut parametrina annettuun aikaan, että SmartMet säätyy ladattuun CaseStudy-dataan mahdollisimman hyvin.
void NFmiMTATempSystem::SetCaseStudyTimes(const NFmiMetTime &theCaseStudyTime)
{
	if(itsTempInfos.size() > 0)
	{
		Container::iterator firstSoundingIter = itsTempInfos.begin();
		long diffInMinutes = theCaseStudyTime.DifferenceInMinutes(firstSoundingIter->Time());
		for(Container::iterator it = itsTempInfos.begin(); it != itsTempInfos.end(); ++it)
		{
			NFmiMetTime newTime((*it).Time());
			newTime.ChangeByMinutes(diffInMinutes);
			(*it).Time(newTime);
		}
	}
}

double NFmiMTATempSystem::UsedSecondaryDataFrameWidthFactor()
{
    if(fDrawSecondaryData)
        return itsSecondaryDataFrameWidthFactor;
    else
        return 0;
}

void NFmiMTATempSystem::SetAllTempTimes(const NFmiMetTime &theTime)
{
    for(auto &tempInfo : itsTempInfos)
    {
        tempInfo.Time(theTime);
    }
}
