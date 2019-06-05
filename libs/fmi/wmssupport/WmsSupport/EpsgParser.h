#pragma once

#include "NFmiArea.h"

#include <cpprest/details/basic_types.h>
#undef U // This fixes cpprest's U -macro clash with boost library move code (really dangerous to give macro name like U !!!!)


namespace Wms
{
    class EpsgParser
    {
        std::string stereo00_;
        std::string stereo10_;
        std::string stereo20_;
    public:
        EpsgParser(const std::string& stereo00, const std::string& stereo10, const std::string& stereo20);
        std::string parse(const NFmiArea& area) const;
    private:
        std::string stereographicToEpsg(const NFmiArea& area) const;
    };
}
