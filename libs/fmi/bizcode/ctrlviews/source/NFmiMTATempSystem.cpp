#include "NFmiMTATempSystem.h"
#include "NFmiSettings.h"
#include "NFmiProducerName.h"
#include "NFmiProducerSystem.h"
#include "NFmiDataStoringHelpers.h"
#include "SettingsFunctions.h"
#include "NFmiApplicationWinRegistry.h"
#include "catlog/catlog.h"
#include "ModelDataServerConfiguration.h"
#include <boost/algorithm/string.hpp>


/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/

#ifdef max
#undef max
#undef min
#endif

double NFmiMTATempSystem::itsLatestVersionNumber = 1.0;

// ************************************************************
// ** Tästä alkaa NFmiMTATempSystem::HodografViewData osio ****
// ************************************************************

void NFmiMTATempSystem::HodografViewData::AdjustScaleMaxValue(short theDelta)
{
	// säädetään arvoalueen kokoa riippuen annetusta hiiren rullan suunnasta (theDelta)
	if(theDelta > 0)
		itsScaleMaxValue -= 10;
	else
		itsScaleMaxValue += 10;

	// Rajataan muuttujan koko 10 ja 150 välille
	if(itsScaleMaxValue < 10)
		itsScaleMaxValue = 10;
	if(itsScaleMaxValue > 150)
		itsScaleMaxValue = 150;
}

void NFmiMTATempSystem::HodografViewData::AdjustRelativiHeightFactor(short theDelta)
{
	// säädetään suhteellista kokoa riippuen annetusta hiiren rullan suunnasta (theDelta)
	if(theDelta > 0)
		itsRelativiHeightFactor -= 0.05;
	else
		itsRelativiHeightFactor += 0.05;

	// Rajataan muuttujan koko 0.2 ja 0.6 välille
	if(itsRelativiHeightFactor < 0.2)
		itsRelativiHeightFactor = 0.2;
	if(itsRelativiHeightFactor > 0.6)
		itsRelativiHeightFactor = 0.6;
}

void NFmiMTATempSystem::HodografViewData::SetCenter(const NFmiPoint& thePlace)
{
	itsRect.Center(thePlace);
}

std::string NFmiMTATempSystem::HodografViewData::GenerateSettingsString() const
{
	std::stringstream out;
	out << itsRect << itsScaleMaxValue << " " << itsRelativiHeightFactor << std::endl;

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi vielä mahdollinen extra data
	out << extraData;

	return out.str();
}

void NFmiMTATempSystem::HodografViewData::InitializeFromSettingsString(const std::string& settingsString)
{
	std::stringstream in(settingsString);
	in >> itsRect >> itsScaleMaxValue >> itsRelativiHeightFactor;

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi vielä mahdollinen extra data
	in >> extraData;
}

// **********************************************************
// ** Tästä alkaa NFmiMTATempSystem::ServerProducer osio ****
// **********************************************************

NFmiMTATempSystem::ServerProducer::ServerProducer()
{}

NFmiMTATempSystem::ServerProducer::ServerProducer(const NFmiProducer &producer, bool useServer)
    :NFmiProducer(producer)
    ,useServer_(useServer)
{}

bool NFmiMTATempSystem::ServerProducer::ProducersAreEqual(const NFmiProducer &first, const NFmiProducer &second)
{
    return first.GetIdent() == second.GetIdent() && first.GetName() == second.GetName();
}

bool NFmiMTATempSystem::ServerProducer::operator==(const ServerProducer &other) const
{
    return ProducersAreEqual(*this, other) && useServer() == other.useServer();
}

bool NFmiMTATempSystem::ServerProducer::operator==(const NFmiProducer &other) const
{
    return ProducersAreEqual(*this, other);
}

bool NFmiMTATempSystem::ServerProducer::operator!=(const ServerProducer &other) const
{
    return !operator==(other);
}

bool NFmiMTATempSystem::ServerProducer::operator!=(const NFmiProducer &other) const
{
    return !ProducersAreEqual(*this, other);
}

bool NFmiMTATempSystem::ServerProducer::operator<(const ServerProducer &other) const
{
    if(GetIdent() != other.GetIdent())
        return GetIdent() < other.GetIdent();
    if(GetName() != other.GetName())
        return GetName() < other.GetName();

	return useServer() < other.useServer();
}


// ****************************************************
// ** Tästä alkaa NFmiMTATempSystem::TempInfo osio ****
// ****************************************************

bool NFmiMTATempSystem::TempInfo::operator<(const TempInfo &other) const
{
    if(Latlon() != other.Latlon())
        return Latlon() < other.Latlon();
    if(Time() != other.Time())
        return Time() < other.Time();
    else
        return Producer() < other.Producer();
}

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


NFmiMTATempSystem::SoundingDataCacheMapKey::SoundingDataCacheMapKey(const TempInfo &tempInfo, const ServerProducer &serverProducer, int modelRunIndex)
    :tempInfo_(tempInfo)
    ,serverProducer_(serverProducer)
    ,modelRunIndex_(modelRunIndex)
{}

bool NFmiMTATempSystem::SoundingDataCacheMapKey::operator<(const SoundingDataCacheMapKey &other) const
{
    if(serverProducer_ != other.serverProducer_)
        return serverProducer_ < other.serverProducer_;
    if(modelRunIndex_ != other.modelRunIndex_)
        return modelRunIndex_ < other.modelRunIndex_;
    else
        return tempInfo_ < other.tempInfo_;
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
,itsModelRunCount(0)
,fDrawSecondaryData(false)
,itsSecondaryDataFrameWidthFactor(gDefaultSecondaryDataFrameWidthFactor)
// Näille viiva asetuksille pitää antaa valmiit oletusarvot, koska näitä ei välttämättä löydy asetuksista
,itsWSLineInfo(NFmiColor(), 2, FMI_SOLID, true)
,itsNLineInfo(NFmiColor(), 2, FMI_DASH, true)
,itsRHLineInfo(NFmiColor(), 2, FMI_DOT, true)
,itsSoundingViewSettingsFromWindowsRegisty()
,itsSoundingDataServerConfigurations()
{
}

NFmiMTATempSystem::~NFmiMTATempSystem(void)
{
}

void NFmiMTATempSystem::Init(NFmiProducerSystem &theProducerSystem, const std::vector<NFmiProducer>& theExtraSoundingProducers, const SoundingViewSettingsFromWindowsRegisty& soundingViewSettingsFromWindowsRegisty)
{
	fInitializationOk = false;
	itsSoundingViewSettingsFromWindowsRegisty = soundingViewSettingsFromWindowsRegisty;
    InitializeSoundingDataServerConfigurations();
    InitializeSoundingColors();
    InitPossibleProducerList(theProducerSystem, theExtraSoundingProducers);
	fInitializationOk = true; // jos ei poikkeuksia lentänyt laitetaan true:ksi
}

void NFmiMTATempSystem::InitializeSoundingDataServerConfigurations()
{
    try
    {
        itsSoundingDataServerConfigurations.init(NFmiApplicationWinRegistry::MakeBaseRegistryPath(), "SmartMet::SoundingDataServerConfigurations");
    }
    catch(std::exception &e)
    {
        std::string errorMessage = "Problem with SoundingDataServerConfigurations: ";
        errorMessage += e.what();
        CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Configuration, true);
    }
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

static std::string MakeValueStr(const std::vector<double> &theValues)
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
	itsPressureValues = NFmiStringTools::Split<std::vector<double> >(valuesStr);
	GetHelpLineInfoSettings("MetEditor::TempView::HelpLine::Pressure::Line", itsPressureLineInfo);
	GetHelpLabelInfoSettings("MetEditor::TempView::HelpLine::Pressure::Label", itsPressureLabelInfo);

	valuesStr = NFmiSettings::Require<std::string>("MetEditor::TempView::HelpLine::MixingRatio::Values");
	itsMixingRatioValues = NFmiStringTools::Split<std::vector<double> >(valuesStr);
	GetHelpLineInfoSettings("MetEditor::TempView::HelpLine::MixingRatio::Line", itsMixingRatioLineInfo);
	GetHelpLabelInfoSettings("MetEditor::TempView::HelpLine::MixingRatio::Label", itsMixingRatioLabelInfo);

	valuesStr = NFmiSettings::Require<std::string>("MetEditor::TempView::HelpLine::MoistAdiabatic::Values");
	itsMoistAdiabaticValues = NFmiStringTools::Split<std::vector<double> >(valuesStr);
	GetHelpLineInfoSettings("MetEditor::TempView::HelpLine::MoistAdiabatic::Line", itsMoistAdiabaticLineInfo);
	GetHelpLabelInfoSettings("MetEditor::TempView::HelpLine::MoistAdiabatic::Label", itsMoistAdiabaticLabelInfo);

	valuesStr = NFmiSettings::Require<std::string>("MetEditor::TempView::HelpLine::DryAdiabatic::Values");
	itsDryAdiabaticValues = NFmiStringTools::Split<std::vector<double> >(valuesStr);
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
void NFmiMTATempSystem::InitPossibleProducerList(NFmiProducerSystem &theProducerSystem, const std::vector<NFmiProducer>& theExtraSoundingProducers)
{
	itsPossibleProducerList.clear();
    AddSpecialDataToPossibleProducerList(itsPossibleProducerList);
    AddVerticalModelDataToPossibleProducerList(itsPossibleProducerList, theProducerSystem);
    AddSoundingDataFromServerToPossibleProducerList(itsPossibleProducerList);
    AddExtraSoundingDataToPossibleProducerList(itsPossibleProducerList, theExtraSoundingProducers);
}

void NFmiMTATempSystem::AddSoundingDataFromServerToPossibleProducerList(SelectedProducerContainer &possibleProducerList)
{
    // 3. tuottajat, joiden data haetaan serverilta
    for(const auto &modelSoundingDataFromServerConfiguration : itsSoundingDataServerConfigurations.modelConfigurations())
    {
        const std::string serverDataNamePrefix = "(S) ";
        NFmiProducer producer(modelSoundingDataFromServerConfiguration.producerId(), serverDataNamePrefix + modelSoundingDataFromServerConfiguration.producerName());
        possibleProducerList.push_back(SoundingProducer(producer, true));
    }
}

void NFmiMTATempSystem::AddExtraSoundingDataToPossibleProducerList(SelectedProducerContainer &possibleProducerList, const std::vector<NFmiProducer>& theExtraSoundingProducers)
{
    // 4. extra tuottajat, jotka saadaan myös docilta
    for(const auto &producer : theExtraSoundingProducers)
        possibleProducerList.push_back(SoundingProducer(producer, false));
}

void NFmiMTATempSystem::AddVerticalModelDataToPossibleProducerList(SelectedProducerContainer &possibleProducerList, NFmiProducerSystem &theProducerSystem)
{
    // 2. malli datat jotka on määritelty docin producer listassa
    const auto &producers = theProducerSystem.Producers();
    for(auto producerIndex = 0u; producerIndex < static_cast<unsigned int>(producers.size()); producerIndex++)
    {
        if(theProducerSystem.Producer(producerIndex + 1).HasRealVerticalData())
        {
            NFmiProducerInfo &producerInfo = theProducerSystem.Producer(producerIndex + 1); // +1 johtuu producersystemin 1-pohjaisesta indeksi systeemistä
            NFmiProducer producer = producerInfo.GetProducer();
            const unsigned long nameLengthLimit = 12; // Jos tuottajan normaalinimi menee yli tämä rajan, käytetään ShortName:a
            if(producer.GetName().GetLen() > nameLengthLimit)
                producer.SetName(producerInfo.ShortName());
            itsPossibleProducerList.push_back(SoundingProducer(producer, false));
        }
    }
}

void NFmiMTATempSystem::AddSpecialDataToPossibleProducerList(SelectedProducerContainer &possibleProducerList)
{
    // 1. edited + havainto + TEMP (=raaka-luotaukset)
    possibleProducerList.push_back(SoundingProducer(NFmiProducer(kFmiMETEOR, "Edited"), false));
    possibleProducerList.push_back(SoundingProducer(NFmiProducer(kFmiTEMP, "Sounding"), false));
    possibleProducerList.push_back(SoundingProducer(NFmiProducer(kFmiBufrTEMP, "BufrSounding"), false));
    possibleProducerList.push_back(SoundingProducer(NFmiProducer(kFmiRAWTEMP, "TEMP"), false));
}

const NFmiProducer& NFmiMTATempSystem::CurrentProducer(void) const
{
	if(itsSelectedProducer>=0 && itsSelectedProducer < static_cast<int>(itsPossibleProducerList.size()))
		return itsPossibleProducerList[itsSelectedProducer];
    else
    {
        static const SoundingProducer dummy(NFmiProducer(0, "Error Producer"), false);
    	return dummy;
    }
}

void NFmiMTATempSystem::CurrentProducer(const NFmiProducer &newValue)
{
	itsSelectedProducer = -1;
	for(int i=0; i < static_cast<int>(itsPossibleProducerList.size()); i++)
	{
		if(itsPossibleProducerList[i] == newValue)
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

static std::vector<NFmiProducer> MakeLegacyProducerContainer(const NFmiMTATempSystem::SelectedProducerContainer &producerContainer)
{
    std::vector<NFmiProducer> legacyContainer;
    for(const auto &producer : producerContainer)
    {
        legacyContainer.push_back(producer);
    }

    return legacyContainer;
}

static std::string MakeProducerContainerServerUsageString(const NFmiMTATempSystem::SelectedProducerContainer &producerContainer)
{
    std::string serverUsageString;
    for(const auto &producer : producerContainer)
    {
        if(!serverUsageString.empty())
            serverUsageString.push_back(' ');
        serverUsageString.push_back(producer.useServer() ? '1' : '0');
    }

    return serverUsageString;
}

static const NFmiMTATempSystem::SelectedProducerContainer MakeSoundingComparisonProducersFromLegacyData(const std::vector<NFmiProducer> &legacyProducerContainer, const std::string &serverDataUsageString)
{
    std::istringstream in(serverDataUsageString);
    NFmiMTATempSystem::SelectedProducerContainer finalComparisonProducers;
    bool serverUsage = false;
    for(const auto &producer : legacyProducerContainer)
    {
        in >> serverUsage;
        if(in)
            finalComparisonProducers.push_back(NFmiMTATempSystem::SoundingProducer(producer, serverUsage));
        else
            finalComparisonProducers.push_back(NFmiMTATempSystem::SoundingProducer(producer, false));
    }

    return finalComparisonProducers;
}

int NFmiMTATempSystem::GetSelectedProducerIndex(bool getLimitCheckedIndex) const 
{ 
	if(getLimitCheckedIndex)
	{
		if(itsSelectedProducerIndex >= itsSoundingComparisonProducers.size())
		{
			if(itsSoundingComparisonProducers.empty())
				return 0;
			else
				return static_cast<int>(itsSoundingComparisonProducers.size() - 1);
		}
	}

	return itsSelectedProducerIndex; 
}

void NFmiMTATempSystem::SetSelectedProducerIndex(int newValue, bool ignoreHighLimit)
{ 
	if(newValue < 0 || itsSoundingComparisonProducers.empty())
		newValue = 0;
	
	// ignoreHighLimit tapaus kiinnostaa vain näyttömakron latauksen yhteydessä
	if(!ignoreHighLimit && newValue >= itsSoundingComparisonProducers.size())
	{
		newValue = static_cast<int>(itsSoundingComparisonProducers.size() - 1);
	}

	itsSelectedProducerIndex = newValue; 
}

void NFmiMTATempSystem::ToggleSelectedProducerIndex(FmiDirection direction)
{
	if(direction == kUp)
		itsSelectedProducerIndex++;
	else
		itsSelectedProducerIndex--;

	// Jos indeksi menee ali tai yli rajojen, mennään ympäri toiseen päähän
	if(itsSelectedProducerIndex < 0)
	{
		if(itsSoundingComparisonProducers.empty())
			itsSelectedProducerIndex = 0;
		else
			itsSelectedProducerIndex = static_cast<int>(itsSoundingComparisonProducers.size() - 1);
	}
	else if(itsSelectedProducerIndex >= itsSoundingComparisonProducers.size())
		itsSelectedProducerIndex = 0;
}

static double SetInsideLimits(double newValue, double minValue, double maxValue)
{
	if(newValue == kFloatMissing)
	{
		return 0;
	}
	else
	{
		double value = std::max(minValue, newValue);
		value = std::min(value, maxValue);
		return value;
	}
}

const double gIntegrationRangeInKmMin = 0;
const double gIntegrationRangeInKmMax = 250;
const double gIntegrationTimeOffsetInHoursMin = -50;
const double gIntegrationTimeOffsetInHoursMax = 50;

void NFmiMTATempSystem::IntegrationRangeInKm(double newValue) 
{ 
	itsIntegrationRangeInKm = ::SetInsideLimits(newValue, gIntegrationRangeInKmMin, gIntegrationRangeInKmMax);
}

void NFmiMTATempSystem::IntegrationTimeOffset1InHours(double newValue) 
{ 
	itsIntegrationTimeOffset1InHours = ::SetInsideLimits(newValue, gIntegrationTimeOffsetInHoursMin, gIntegrationTimeOffsetInHoursMax);
}

void NFmiMTATempSystem::IntegrationTimeOffset2InHours(double newValue) 
{ 
	itsIntegrationTimeOffset2InHours = ::SetInsideLimits(newValue, gIntegrationTimeOffsetInHoursMin, gIntegrationTimeOffsetInHoursMax);
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

	os << "// Selected producers std::vector<NFmiProducer>" << std::endl;
	NFmiDataStoringHelpers::WriteContainer(::MakeLegacyProducerContainer(itsSoundingComparisonProducers), os, std::string("\n"));

	os << "// SelectedProducer index + tempViewOn + SkewTDegree" << std::endl;
	os << itsSelectedProducer << " " << fTempViewOn << " " << itsSkewTDegree << std::endl;

	os << "// SoundingColors std::vector<NFmiColor>" << std::endl;
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
	os << ShowIndexiesViewMacroLegacy() << " " << itsIndexiesFontSize << " " << itsSoundingTextFontSize << std::endl;

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
    extraData.Add(static_cast<double>(ShowSideViewViewMacroLegacy())); // fShowSideView on 2. uusi double arvo
    extraData.Add(static_cast<double>(itsModelRunCount)); // itsModelRunCount on 3. uusi double arvo
    extraData.Add(static_cast<double>(fDrawSecondaryData)); // fDrawSecondaryData on 4. uusi double arvo
    extraData.Add(itsSecondaryDataFrameWidthFactor); // itsSecondaryDataFrameWidthFactor on 5. uusi double arvo
    extraData.Add(static_cast<double>(itsSoundingViewSettingsFromWindowsRegisty.SoundingTextUpward())); // fSoundingTextUpwardWinReg on 6. uusi double arvo
	extraData.Add(static_cast<double>(itsSoundingViewSettingsFromWindowsRegisty.SoundingTimeLockWithMapView())); // fSoundingTimeLockWithMapViewWinReg on 7. uusi double arvo
	extraData.Add(static_cast<double>(itsSoundingViewSettingsFromWindowsRegisty.ShowStabilityIndexSideView())); // ShowStabilityIndexSideView on 8. uusi double arvo
	extraData.Add(static_cast<double>(itsSoundingViewSettingsFromWindowsRegisty.ShowTextualSoundingDataSideView())); // ShowTextualSoundingDataSideView on 9. uusi double arvo
	extraData.Add(itsIntegrationRangeInKm); // IntegrationRangeInKm on 10. uusi double arvo
	extraData.Add(itsIntegrationTimeOffset1InHours); // IntegrationTimeOffset1InHours on 11. uusi double arvo
	extraData.Add(itsIntegrationTimeOffset2InHours); // IntegrationTimeOffset2InHours on 12. uusi double arvo

    extraData.Add(MakeSecondaryDataLineInfoString()); // WS + N + RH lineInfor yhtenä stringinä on 1. uusi string arvo extroissa
    extraData.Add(::MakeProducerContainerServerUsageString(itsSoundingComparisonProducers)); // 2. uusi string arvo extroissa on valittujen tuottajien server/local data käyttötila tyyliin "0 1 0 0"
	extraData.Add(itsHodografViewData.GenerateSettingsString()); // 3. uusi string arvo extroissa on hodografi säädöt
	extraData.Add(MakeSelectedProducerStringForViewMacro()); // 4. uusi string arvo extroissa on valitun tuottajan indeksiin liittyvät arvot

	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw std::runtime_error("NFmiMTATempSystem::Write failed");
}

// Tehdään stringi, jossa on valitun tuottajan producer-id ja valitun tuottajan index pilkuilla eroteltuna.
// Oletetaan että itsSelectedProducerIndex on 2 (eli 3. valituista multivalinta tuottajista) ja se on Gfs
// tuottaja (id = 54), tällöin tehdään seuraava stringi: "2,54"
// Huom! Jos valittu index on suurempi kuin on valittuja tuottajia, käytetään silloin listan viimeistä tuottajaa,
// mutta originaali indeksiä, joka osoittaa tuottaja listan ulkopuolelle.
std::string NFmiMTATempSystem::MakeSelectedProducerStringForViewMacro() const
{
	auto usedProducerIndex = GetSelectedProducerIndex(true);
	auto actualProducerIndex = GetSelectedProducerIndex(false);
	auto pointedProducerId = 0;
	if(!itsSoundingComparisonProducers.empty())
	{
		pointedProducerId = static_cast<int>(itsSoundingComparisonProducers[usedProducerIndex].GetIdent());
	}
	std::string producerStr = std::to_string(actualProducerIndex);
	producerStr += ",";
	producerStr += std::to_string(pointedProducerId);
	return producerStr;
}

void NFmiMTATempSystem::SetSelectedProducerFromViewMacroString(const std::string& str)
{
	std::vector<std::string> parts;
	boost::split(parts, str, boost::is_any_of(","));
	if(parts.size() == 2)
	{
		try
		{
			auto producerIndex = std::stoi(parts[0]);
			auto producerId = std::stoi(parts[1]);
			auto iter = std::find_if(itsSoundingComparisonProducers.begin(), itsSoundingComparisonProducers.end(),
				[=](const auto& producerInfo) { return static_cast<int>(producerInfo.GetIdent()) == producerId; });
			if(iter != itsSoundingComparisonProducers.end())
			{
				itsSelectedProducerIndex = static_cast<int>(iter - itsSoundingComparisonProducers.begin());
			}
			else
			{
				itsSelectedProducerIndex = producerIndex;
			}
		}
		catch(...)
		{ }
	}
}

// Jos textual-sounding sivuikkuna on auki, avataan se myös legacy näyttömakroissa, 
// eli sen prioriteetti on valittu suuremmaksi kuin stability-index sivuikkunan, 
// jos molemmat 'teksti' sivuikkunoista ovat auki.
bool NFmiMTATempSystem::ShowIndexiesViewMacroLegacy() const
{
	return !itsSoundingViewSettingsFromWindowsRegisty.ShowTextualSoundingDataSideView();
}

// Jos jompikumpi tai molemmat 'teksti' sivuikkunoista on auki, on vanhassa systeemissä ShowSideView = true
bool NFmiMTATempSystem::ShowSideViewViewMacroLegacy() const
{
	return itsSoundingViewSettingsFromWindowsRegisty.ShowStabilityIndexSideView() || itsSoundingViewSettingsFromWindowsRegisty.ShowTextualSoundingDataSideView();
}

void NFmiMTATempSystem::SetupSideViewsFromLegacyViewMacroValues(bool showIndexiesLegacyValue, bool showSideViewLegacyValue)
{
	if(!showSideViewLegacyValue)
	{
		itsSoundingViewSettingsFromWindowsRegisty.ShowStabilityIndexSideView(false);
		itsSoundingViewSettingsFromWindowsRegisty.ShowTextualSoundingDataSideView(false);
	}
	else
	{
		if(showIndexiesLegacyValue)
		{
			itsSoundingViewSettingsFromWindowsRegisty.ShowStabilityIndexSideView(true);
			itsSoundingViewSettingsFromWindowsRegisty.ShowTextualSoundingDataSideView(false);
		}
		else
		{
			itsSoundingViewSettingsFromWindowsRegisty.ShowStabilityIndexSideView(false);
			itsSoundingViewSettingsFromWindowsRegisty.ShowTextualSoundingDataSideView(true);
		}
	}
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
    std::vector<NFmiProducer> legacySoundingComparisonProducers;
	NFmiDataStoringHelpers::ReadContainer(legacySoundingComparisonProducers, is);

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

	bool showIndexiesLegacyValue = false;
	is >> showIndexiesLegacyValue >> itsIndexiesFontSize >> itsSoundingTextFontSize;

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
	bool showSideViewLegacyValue = true;
    if(extraData.itsDoubleValues.size() > 1)
        showSideViewLegacyValue = extraData.itsDoubleValues[1] != 0.;
	itsModelRunCount = 0;
    if(extraData.itsDoubleValues.size() > 2)
        itsModelRunCount = static_cast<int>(extraData.itsDoubleValues[2]);
    fDrawSecondaryData = false;
    if(extraData.itsDoubleValues.size() > 3)
        fDrawSecondaryData = extraData.itsDoubleValues[3] != 0;
    itsSecondaryDataFrameWidthFactor = gDefaultSecondaryDataFrameWidthFactor;
    if(extraData.itsDoubleValues.size() > 4)
        itsSecondaryDataFrameWidthFactor = extraData.itsDoubleValues[4];
    bool soundingTextUpwardTmp = true;
    if(extraData.itsDoubleValues.size() > 5)
        soundingTextUpwardTmp = extraData.itsDoubleValues[5] != 0;
	itsSoundingViewSettingsFromWindowsRegisty.SoundingTextUpward(soundingTextUpwardTmp);
	bool soundingTimeLockWithMapViewTmp = false;
    if(extraData.itsDoubleValues.size() > 6)
        soundingTimeLockWithMapViewTmp = extraData.itsDoubleValues[6] != 0;
	itsSoundingViewSettingsFromWindowsRegisty.SoundingTimeLockWithMapView(soundingTimeLockWithMapViewTmp);

	if(extraData.itsDoubleValues.size() > 8)
	{
		// Version 5.13.12.0 jälkeisillä versioilla tehty näyttömakro hanskataan käyttämällä extraData arvoja suoraan
		itsSoundingViewSettingsFromWindowsRegisty.ShowStabilityIndexSideView(extraData.itsDoubleValues[7] != 0);
		itsSoundingViewSettingsFromWindowsRegisty.ShowTextualSoundingDataSideView(extraData.itsDoubleValues[8] != 0);
	}
	else
	{
		// Versiolla 5.13.12.0 ja sitä edeltävillä versioilla tehty näyttömakro pitää tehdä erityis legacy metodeilla
		SetupSideViewsFromLegacyViewMacroValues(showIndexiesLegacyValue, showSideViewLegacyValue);
	}

	itsIntegrationRangeInKm = 0;
	if(extraData.itsDoubleValues.size() > 9)
		itsIntegrationRangeInKm = static_cast<int>(extraData.itsDoubleValues[9]);
	itsIntegrationTimeOffset1InHours = 0;
	if(extraData.itsDoubleValues.size() > 10)
		itsIntegrationTimeOffset1InHours = static_cast<int>(extraData.itsDoubleValues[10]);
	itsIntegrationTimeOffset2InHours = 0;
	if(extraData.itsDoubleValues.size() > 11)
		itsIntegrationTimeOffset2InHours = static_cast<int>(extraData.itsDoubleValues[11]);

    if(extraData.itsStringValues.size() > 0)
        ReadSecondaryDataLineInfoFromString(extraData.itsStringValues[0]);
    std::string producerContainerServerUsageString;
	if(extraData.itsStringValues.size() > 1)
	{
		producerContainerServerUsageString = extraData.itsStringValues[1];
	}
    itsSoundingComparisonProducers = ::MakeSoundingComparisonProducersFromLegacyData(legacySoundingComparisonProducers, producerContainerServerUsageString);
	if(extraData.itsStringValues.size() > 2)
		itsHodografViewData.InitializeFromSettingsString(extraData.itsStringValues[2]);
	itsSelectedProducerIndex = 0;
	if(extraData.itsStringValues.size() > 3)
		SetSelectedProducerFromViewMacroString(extraData.itsStringValues[3]);

	if(is.fail())
		throw std::runtime_error("NFmiMTATempSystem::Read failed");

	itsCurrentVersionNumber = itsLatestVersionNumber; // aina jatketaan viimeisellä versio numerolla
}

void NFmiMTATempSystem::InitFromViewMacro(const NFmiMTATempSystem &theOther, bool disableWindowManipulations)
{
	itsCurrentVersionNumber = theOther.itsCurrentVersionNumber;
	itsTempInfos = theOther.itsTempInfos;
	itsMaxTempsShowed = theOther.itsMaxTempsShowed;
	itsSoundingComparisonProducers = theOther.itsSoundingComparisonProducers;
	itsSelectedProducer = theOther.itsSelectedProducer;
	if(!disableWindowManipulations)
	{
		fTempViewOn = theOther.fTempViewOn;
	}
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
    itsModelRunCount = theOther.itsModelRunCount;
    fDrawSecondaryData = theOther.fDrawSecondaryData;
    itsSecondaryDataFrameWidthFactor = theOther.itsSecondaryDataFrameWidthFactor;
    itsWSLineInfo = theOther.itsWSLineInfo;
    itsNLineInfo = theOther.itsNLineInfo;
    itsRHLineInfo = theOther.itsRHLineInfo;
	itsSoundingViewSettingsFromWindowsRegisty = theOther.itsSoundingViewSettingsFromWindowsRegisty;
	// Pitäisikö myös itsSoundingDataServerConfigurations asetukset kopioida tässä???
	// itsSoundingDataServerConfigurations = theOther.itsSoundingDataServerConfigurations;
	itsHodografViewData = theOther.itsHodografViewData;
	itsSelectedProducerIndex = theOther.itsSelectedProducerIndex;
	itsIntegrationRangeInKm = theOther.itsIntegrationRangeInKm;
	itsIntegrationTimeOffset1InHours = theOther.itsIntegrationTimeOffset1InHours;
	itsIntegrationTimeOffset2InHours = theOther.itsIntegrationTimeOffset2InHours;
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

// Kun luotaus-dialogista annetaan takaisin valitut tuottajat, ne annetaan pelkkänä tuottaja listana,
// ilman tietoa onko kyseessä serveriltä haettava data vai ei.
// Täällä päätellää tuottajanimien avulla, mitkä tuottajat oikeasti on kyseessä.
void NFmiMTATempSystem::SoundingComparisonProducers(const SelectedProducerLegacyContainer &selectedProducersLegacyContainer)
{
    SelectedProducerContainer selectedProducers;
    for(const auto &producer : selectedProducersLegacyContainer)
    {
        auto producerIterator = std::find_if(itsPossibleProducerList.begin(), itsPossibleProducerList.end(),
            [&producer](const auto &possibleProducer) { return possibleProducer == producer; });
        if(producerIterator != itsPossibleProducerList.end())
            selectedProducers.push_back(*producerIterator);
    }

    itsSoundingComparisonProducers = selectedProducers;
}
