#include "LocalCacheHelpDataSystem.h"

LocalCacheHelpDataSystem::LocalCacheHelpDataSystem() = default;

void LocalCacheHelpDataSystem::InitHelpDataInfoSystem(const NFmiHelpDataInfoSystem& helpDataInfoSystem)
{
    std::lock_guard<std::mutex> lock(settingsMutex);
    workerHelpDataSystemPtr = std::make_shared<NFmiHelpDataInfoSystem>(helpDataInfoSystem);
}

void LocalCacheHelpDataSystem::UpdateHelpDataInfoSystem(NFmiHelpDataInfoSystem& helpDataInfoSystem)
{
    InitHelpDataInfoSystem(helpDataInfoSystem);
}

std::shared_ptr<NFmiHelpDataInfoSystem> LocalCacheHelpDataSystem::GetHelpDataInfoSystemPtr()
{
    std::lock_guard<std::mutex> lock(settingsMutex);
    return workerHelpDataSystemPtr;
}
