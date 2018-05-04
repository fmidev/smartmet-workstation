#pragma once

#include "NFmiPoint.h"
#include <string>

class Q2ServerInfo
{
    std::string q2ServerURLStr_;
    int q2ServerDecimalCount_;
    // 0=none, 1=zip, 2=bzip2
    int q2ServerUsedZipMethod_; 
    bool useQ2Server_;
    // missä hilassa q2server data haetaan
    NFmiPoint q2ServerGridSize_; 
    // jos tämä on true, logataan kaikki q2/q3 hakujen http-kutsut
    bool logQ2Requests_; 
    std::string q3ServerUrl_;

public:
    Q2ServerInfo();
    void InitFromSettings();
    void StoreToSettings();

    const std::string& Q2ServerURLStr() const { return q2ServerURLStr_; }
    void Q2ServerURLStr(const std::string &q2ServerURLStr) { q2ServerURLStr_ = q2ServerURLStr; }
    int Q2ServerDecimalCount() const { return q2ServerDecimalCount_; }
    void Q2ServerDecimalCount(int q2ServerDecimalCount) { q2ServerDecimalCount_ = q2ServerDecimalCount; }
    int Q2ServerUsedZipMethod() const { return q2ServerUsedZipMethod_; }
    void Q2ServerUsedZipMethod(int q2ServerUsedZipMethod) { q2ServerUsedZipMethod_ = q2ServerUsedZipMethod; }
    bool UseQ2Server() const { return useQ2Server_; }
    void UseQ2Server(bool useQ2Server) { useQ2Server_ = useQ2Server; }
    const NFmiPoint& Q2ServerGridSize() const { return q2ServerGridSize_; }
    void Q2ServerGridSize(const NFmiPoint &q2ServerGridSize) { q2ServerGridSize_ = q2ServerGridSize; }
    bool LogQ2Requests() const { return logQ2Requests_; }
    void LogQ2Requests(bool logQ2Requests) { logQ2Requests_ = logQ2Requests; }
    const std::string& Q3ServerUrl() const { return q3ServerUrl_; }
    void Q3ServerUrl(const std::string &q3ServerUrl) { q3ServerUrl_ = q3ServerUrl; }
};

