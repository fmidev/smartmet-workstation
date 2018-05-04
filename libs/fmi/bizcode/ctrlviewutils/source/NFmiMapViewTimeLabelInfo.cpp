#include "NFmiMapViewTimeLabelInfo.h"
#include "SettingsFunctions.h"
#include "NFmiSettings.h"

NFmiMapViewTimeLabelInfo::NFmiMapViewTimeLabelInfo()
    :fUseOldTimeLabel(false)
    , itsTimeStringInfo1()
    , itsTimeStringInfo2()
    , itsBoxFillColor(1, 1, 1, 0.4f)
    , itsBoxFrameColor(0, 0, 0, 0)
    , itsBoxFrameThicknessInMM(0.1)
    , itsSizeFactor1(0.8)
    , itsSizeFactor2(1.2)
    , itsViewSize1(100)
    , itsViewSize2(300)
    , itsMinFactor(0.65)
    , itsMaxFactor(1.4)
    , itsAbsoluteMinFontSizeInMM(1.)
    , itsAbsoluteMaxFontSizeInMM(20.)
{
    itsTimeStringInfo1.itsTimeFormat = "DD Nnn YYYY";
    itsTimeStringInfo1.itsFontName = "Arial";
    itsTimeStringInfo1.fBold = true;
    itsTimeStringInfo1.itsFontSizeInMM = 3.;
    itsTimeStringInfo1.itsColor = NFmiColor(0, 0, 0);

    itsTimeStringInfo2.itsTimeFormat = "HH:mm:SS UTC";
    itsTimeStringInfo2.itsFontName = "Arial";
    itsTimeStringInfo2.fBold = true;
    itsTimeStringInfo2.itsFontSizeInMM = 3.;
    itsTimeStringInfo2.itsColor = NFmiColor(0, 0, 0);
}

static NFmiMapViewTimeLabelInfo::TimeStringInfo GetTimeStringInfoFromSettings(const std::string &theSettingKey)
{
    NFmiMapViewTimeLabelInfo::TimeStringInfo timeStringInfo;
    timeStringInfo.itsTimeFormat = NFmiSettings::Require<std::string>(std::string(theSettingKey + "::Format").c_str());
    timeStringInfo.itsFontName = NFmiSettings::Require<std::string>(std::string(theSettingKey + "::FontName").c_str());
    timeStringInfo.itsFontSizeInMM = NFmiSettings::Require<double>(std::string(theSettingKey + "::FontSizeInMM").c_str());
    timeStringInfo.itsColor = SettingsFunctions::GetColorFromSettings(theSettingKey + "::Color");
    timeStringInfo.fBold = NFmiSettings::Require<bool>(std::string(theSettingKey + "::Bold").c_str());

    return timeStringInfo;
}

void NFmiMapViewTimeLabelInfo::InitFromSettings(const std::string &theInitNameSpace)
{
    fUseOldTimeLabel = NFmiSettings::Require<bool>(std::string(theInitNameSpace + "::UseOldTimeLabel").c_str());
    itsTimeStringInfo1 = ::GetTimeStringInfoFromSettings(theInitNameSpace + "::TimeString1");
    itsTimeStringInfo2 = ::GetTimeStringInfoFromSettings(theInitNameSpace + "::TimeString2");
    itsBoxFillColor = SettingsFunctions::GetColorFromSettings(theInitNameSpace + "::Box::FillColor");
    itsBoxFrameColor = SettingsFunctions::GetColorFromSettings(theInitNameSpace + "::Box::FrameColor");
    itsBoxFrameThicknessInMM = NFmiSettings::Require<double>(std::string(theInitNameSpace + "::Box::FrameThicknessInMM").c_str());
    itsSizeFactor1 = NFmiSettings::Require<double>(std::string(theInitNameSpace + "::SizeFactor1").c_str());
    itsSizeFactor2 = NFmiSettings::Require<double>(std::string(theInitNameSpace + "::SizeFactor2").c_str());
    itsViewSize1 = NFmiSettings::Require<double>(std::string(theInitNameSpace + "::ViewSize1").c_str());
    itsViewSize2 = NFmiSettings::Require<double>(std::string(theInitNameSpace + "::ViewSize2").c_str());
    itsMinFactor = NFmiSettings::Require<double>(std::string(theInitNameSpace + "::MinFactor").c_str());
    itsMaxFactor = NFmiSettings::Require<double>(std::string(theInitNameSpace + "::MaxFactor").c_str());
    itsAbsoluteMinFontSizeInMM = NFmiSettings::Require<double>(std::string(theInitNameSpace + "::AbsoluteMinFontSizeInMM").c_str());
    itsAbsoluteMaxFontSizeInMM = NFmiSettings::Require<double>(std::string(theInitNameSpace + "::AbsoluteMaxFontSizeInMM").c_str());
}
