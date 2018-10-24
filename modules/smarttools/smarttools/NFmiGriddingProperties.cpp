#include "NFmiGriddingProperties.h"
#include "NFmiArea.h"

NFmiGriddingProperties::NFmiGriddingProperties(bool toolMasterAvailable)
    :toolMasterAvailable_(toolMasterAvailable)
{}

FmiGriddingFunction NFmiGriddingProperties::function(void) const
{
    if(toolMasterAvailable_)
        return function_;
    return kFmiMarkoGriddingFunction;
}

// Palauta annettu pituus [km] annetun arean suhteellisessa koordinaatistossa. Suhteellinen pituus lasketaan x ja y suhteen.
// Jos area on 0-pointer, palautetaan puuttuvaa.
double NFmiGriddingProperties::ConvertLengthInKmToRelative(double lengthInKm, const NFmiArea *area)
{
    if(lengthInKm > 0)
    {
        if(area)
        {
            const float kmInMeters = 1000.f;
            auto x = kmInMeters * lengthInKm / area->WorldXYWidth();
            auto y = kmInMeters * lengthInKm / area->WorldXYHeight();
            return std::sqrt(x*x + y*y);
        }
    }

    return kFloatMissing;
}
