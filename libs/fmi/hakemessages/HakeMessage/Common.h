#pragma once


#include <boost/math/special_functions/round.hpp>

#include <chrono>

namespace HakeMessage
{
    static const double minutesToMilliSecondsFactor = 1000 * 60.;

    inline double milliSecondsToMinutes(const std::chrono::milliseconds &ms)
    {
        return ms.count() / minutesToMilliSecondsFactor;
    }

    inline std::chrono::milliseconds minutesToMilliSeconds(double minutes)
    {
        return std::chrono::milliseconds(boost::math::iround(minutes * minutesToMilliSecondsFactor));
    }
}