#include "HakeMsg.h"
#include "NFmiMetTime.h"
#include "NFmiPoint.h"

using namespace std;

namespace HakeMessage
{
    const HakeMsg HakeMsg::unInitialized = HakeMsg{};

    const NFmiMetTime& HakeMsg::StartTime(void) const
    { 
        return itsStartTime;
    }

    void HakeMsg::StartTime(const NFmiMetTime &newValue) 
    { 
        itsStartTime = newValue;
    }

    const NFmiMetTime& HakeMsg::SendingTime(void) const 
    { 
        return itsSendingTime;
    }

    void HakeMsg::SendingTime(const NFmiMetTime &newValue) 
    { 
        itsSendingTime = newValue;
    }

    const string& HakeMsg::Number(void) const
    { 
        return itsNumber;
    }

    void HakeMsg::Number(const string &newValue) 
    { 
        itsNumber = newValue;
    }

    const string& HakeMsg::TypeStr(void) const 
    { 
        return itsTypeStr;
    }

    void HakeMsg::TypeStr(const string &newValue) 
    { 
        itsTypeStr = newValue;
    }

    int HakeMsg::Category(void) const 
    { 
        return itsCategory;
    }

    void HakeMsg::Category(int newValue) 
    {
        itsCategory = newValue;
    }

    int HakeMsg::MessageLevel(void) const 
    {
        return itsMessageLevel;
    }

    void HakeMsg::MessageLevel(int newValue) 
    {
        itsMessageLevel = newValue;
    }

    const string& HakeMsg::MessageStr(void) const  
    {
        return itsMessageStr;
    }

    void HakeMsg::MessageStr(const string &newValue) 
    {
        itsMessageStr = newValue;
    }

    const string& HakeMsg::ReasonStr(void) const 
    {
        return itsReasonStr;
    }

    void HakeMsg::ReasonStr(const string &newValue) 
    {
        itsReasonStr = newValue;
    }

    const string& HakeMsg::MessageCenterId(void) const
    {
        return itsMessageCenterId;
    }

    void HakeMsg::MessageCenterId(const string& newValue)
    {
        itsMessageCenterId = newValue;
    }

    const string& HakeMsg::CountyStr(void) const 
    {
        return itsCountyStr;
    }

    void HakeMsg::CountyStr(const string &newValue) 
    {
        itsCountyStr = newValue;
    }

    const string& HakeMsg::CityDistrictStr(void) const 
    {
        return itsCityDistrictStr;
    }

    void HakeMsg::CityDistrictStr(const string &newValue) 
    {
        itsCityDistrictStr = newValue;
    }

    const string& HakeMsg::Address(void) const 
    {
        return itsAddress;
    }

    void HakeMsg::Address(const string &newValue) 
    {
        itsAddress = newValue;
    }

    const string& HakeMsg::Address2(void) const 
    {
        return itsAddress2;
    }

    void HakeMsg::Address2(const string &newValue) 
    {
        itsAddress2 = newValue;
    }

    const NFmiPoint& HakeMsg::LatlonPoint(void) const 
    { 
        return itsLatlonPoint;
    }

    void HakeMsg::LatlonPoint(const NFmiPoint &newValue) 
    {
        itsLatlonPoint = newValue;
    }

    const string& HakeMsg::TotalMessageStr(void) const 
    {
        return itsTotalMessageStr;
    }

    void HakeMsg::TotalMessageStr(const string &newValue) 
    {
        itsTotalMessageStr = newValue;
    }

    bool HakeMsg::IsFromXmlFormat() const
    {
        return fIsFromXmlFormat;
    }

    void HakeMsg::IsFromXmlFormat(bool newValue)
    {
        fIsFromXmlFormat = newValue;
    }
}
