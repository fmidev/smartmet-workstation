#include "NFmiOnceLoadedDataFiles.h"
#include "NFmiMissingDataOnServerReporter.h"
#include "NFmiCachedDataFileInfo.h"

namespace
{
    // Nollataan ladattujen datojen lista kerran esim. 6 tunnissa
    const double gResetIntervalInSeconds = 6 * 60 * 60;
}

NFmiOnceLoadedDataFiles::NFmiOnceLoadedDataFiles() = default;

bool NFmiOnceLoadedDataFiles::checkIfFileHasBeenLoadedEarlier(NFmiCachedDataFileInfo& cachedDataFileInfo)
{
    doClearanceChecks();

    std::lock_guard<std::mutex> lock(onceLoadedFilePathsOnServerMutex_);
    if(onceLoadedFilePathsOnServer_.find(cachedDataFileInfo.itsTotalServerFileName) == onceLoadedFilePathsOnServer_.end())
    {
        onceLoadedFilePathsOnServer_.insert(cachedDataFileInfo.itsTotalServerFileName);
        return false;
    }
    else
        return true;
}

void NFmiOnceLoadedDataFiles::doClearanceChecks()
{
    if(onceLoadedFilePathsOnServerTimer_.elapsedTimeInSeconds() > gResetIntervalInSeconds)
    {
        std::lock_guard<std::mutex> lock(onceLoadedFilePathsOnServerMutex_);
        // Tyhjennet‰‰n tiedostonimilista aika ajoin, jotta sen k‰sittely ei rupea hidastelemaan
        // t‰t‰ toimintaa, jos SmartMet on k‰ynniss‰ vaikka viikkoja (jos sill‰ ei tee juuri mit‰‰n ei kaatumisia juuri tapahdu).
        onceLoadedFilePathsOnServer_.clear();
        // timer pit‰‰ myˆs k‰ynnist‰‰ uudestaan
        onceLoadedFilePathsOnServerTimer_.restart();
    }
}
