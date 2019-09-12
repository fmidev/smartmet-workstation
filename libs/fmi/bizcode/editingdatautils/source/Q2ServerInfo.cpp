#include "Q2ServerInfo.h"
#include "SettingsFunctions.h"
#include "NFmiSettings.h"

Q2ServerInfo::Q2ServerInfo()
:q2ServerURLStr_("http://localhost")
,q2ServerDecimalCount_(1)
,q2ServerUsedZipMethod_(0)
,useQ2Server_(false)
,q2ServerGridSize_(50, 50)
,logQ2Requests_(false)
,q3ServerUrl_()
{
}

void Q2ServerInfo::InitFromSettings()
{
    q2ServerURLStr_ = SettingsFunctions::GetUrlFromSettings("MetEditor::Q2ServerURLStr");
    q2ServerDecimalCount_ = NFmiSettings::Require<int>("MetEditor::Q2ServerDecimalCount");
    q2ServerUsedZipMethod_ = NFmiSettings::Require<int>("MetEditor::Q2ServerUsedZipMethod");
    useQ2Server_ = NFmiSettings::Require<bool>("MetEditor::UseQ2Server");
    q2ServerGridSize_ = SettingsFunctions::GetCommaSeparatedPointFromSettings("MetEditor::Q2ServerGridSize");
    q3ServerUrl_ = SettingsFunctions::GetUrlFromSettings("SmartMet::Q3ServerUrl");
}

void Q2ServerInfo::StoreToSettings()
{
    SettingsFunctions::SetUrlToSettings("MetEditor::Q2ServerURLStr", q2ServerURLStr_);
    NFmiSettings::Set("MetEditor::Q2ServerDecimalCount", NFmiStringTools::Convert<int>(q2ServerDecimalCount_), true);
    NFmiSettings::Set("MetEditor::Q2ServerUsedZipMethod", NFmiStringTools::Convert<int>(q2ServerUsedZipMethod_), true);
    NFmiSettings::Set("MetEditor::UseQ2Server", NFmiStringTools::Convert<bool>(useQ2Server_), true);
    SettingsFunctions::SetCommaSeparatedPointToSettings("MetEditor::Q2ServerGridSize", q2ServerGridSize_);
    SettingsFunctions::SetUrlToSettings("SmartMet::Q3ServerUrl", q3ServerUrl_);
}