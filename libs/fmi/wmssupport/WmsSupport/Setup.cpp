#include "wmssupport/Setup.h"

namespace Wms
{
    // If there is some configurations given, then we must be able
    // to set this WMS system in on -mode (=> isConfigured = true).
    void Setup::checkForMeaningfulConfigurations()
    {
        if(numberOfCaches && numberOfLayersPerCache)
        {
            if(background.parsedServers.size() || dynamics.size())
            {
                isConfigured = true;
            }
        }
    }
}

