#pragma once

#include "NFmiDataMatrix.h"
#include "NFmiMetTime.h"
#include "NFmiPoint.h"

class NFmiProducer;

// pluumi eli yksi yksitt�isen partikkelin eteneminen
// t�h�n talletetaan vain trajectorin pisteet ja alku aika ja paikka.
// Muut tiedot ovat ylemp�n� eli NFmiTrajectory-luokassa
class NFmiSingleTrajector
{
public:
    NFmiSingleTrajector(void);
    NFmiSingleTrajector(const NFmiPoint &theLatLon, const NFmiMetTime &theTime, double thePressureLevel);
    ~NFmiSingleTrajector(void);

    std::string ToXMLStr(int TimeStepInMinutes, NFmiProducer &Producer, FmiDirection Direction);
    const std::vector<NFmiPoint>& Points(void) const { return itsPoints; }
    const std::vector<float>& Pressures(void) const { return itsPressures; }
    const std::vector<float>& HeightValues(void) const { return itsHeightValues; }
    const NFmiPoint& StartLatLon(void) const { return itsStartLatLon; }
    void StartLatLon(const NFmiPoint &newValue) { itsStartLatLon = newValue; }
    const NFmiMetTime& StartTime(void) const { return itsStartTime; }
    void StartTime(const NFmiMetTime &newValue) { itsStartTime = newValue; }
    void ClearPoints(void);
    void AddPoint(const NFmiPoint &theLatLon);
    void AddPoint(const NFmiPoint &theLatLon, float thePressure, float theHeightValue);
    double StartPressureLevel(void) const { return itsStartPressureLevel; }
    void StartPressureLevel(double newValue) { itsStartPressureLevel = newValue; }
    bool Is3DTrajectory(void) const;
    double IsentropicTpotValue(void) const { return itsIsentropicTpotValue; }
    void IsentropicTpotValue(double newValue) { itsIsentropicTpotValue = newValue; }
    void SetRandomValues(double theWD, double theWS, double thew);
    double RandWSdiff(void) const { return itsRandWSdiff; }
    double RandWDdiff(void) const { return itsRandWDdiff; }
    double Randwdiff(void) const { return itsRandwdiff; }
private:
    std::vector<NFmiPoint> itsPoints;
    std::vector<float> itsPressures;
    std::vector<float> itsHeightValues;
    NFmiPoint itsStartLatLon;
    NFmiMetTime itsStartTime;
    double itsStartPressureLevel; // yks. hPa
    double itsIsentropicTpotValue; // kun lasketaan trajektoreita, t�h�n talletetaan haluttu potentiaali l�mp�tila, mit� on k�ytetty

    double itsRandWSdiff; // satunnainen WS muutos
    double itsRandWDdiff; // satunnainen WD muutos
    double itsRandwdiff; // satunnainen w muutos
};
