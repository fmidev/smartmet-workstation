#ifdef UNIX

#include "help_data_loader.h"

#include <smarttools/NFmiHelpDataInfo.h>

#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiFileSystem.h>
#include <newbase/NFmiQueryData.h>

#include <iostream>

namespace
{
    // Help data entries can also point at satellite images, map layers and other formats
    // that are not querydata at all. Those are of no use to the Linux renderer.
    bool isQueryDataType(NFmiInfoData::Type theType)
    {
        switch(theType)
        {
            case NFmiInfoData::kSatelData:
            case NFmiInfoData::kConceptualModelData:
            case NFmiInfoData::kCapData:
            case NFmiInfoData::kWmsData:
            case NFmiInfoData::kMapLayer:
                return false;
            default:
                return true;
        }
    }
}

size_t HelpDataLoader::loadFromSettings(const std::string& theControlPath)
{
    NFmiHelpDataInfoSystem helpDataSystem;
    try
    {
        helpDataSystem.InitFromSettings("MetEditor::HelpData", theControlPath);
    }
    catch(const std::exception& e)
    {
        std::cerr << "No usable help data configuration: " << e.what() << std::endl;
        return 0;
    }

    const size_t countBefore = itsLoadedData.size();

    std::cerr << "Loading help data ("
              << helpDataSystem.StaticCount() << " static, "
              << helpDataSystem.DynamicCount() << " dynamic configured)" << std::endl;

    for(const auto& info : helpDataSystem.StaticHelpDataInfos())
        loadHelpDataInfo(info, helpDataSystem);
    for(const auto& info : helpDataSystem.DynamicHelpDataInfos())
        loadHelpDataInfo(info, helpDataSystem);

    return itsLoadedData.size() - countBefore;
}

void HelpDataLoader::loadHelpDataInfo(const NFmiHelpDataInfo& theInfo,
                                      const NFmiHelpDataInfoSystem& theSystem)
{
    if(!theInfo.IsEnabled() || !::isQueryDataType(theInfo.DataType()))
        return;

    const std::string fileFilter = theInfo.UsedFileNameFilter(theSystem);
    const std::string fileName = NFmiFileSystem::NewestPatternFileName(fileFilter);
    if(fileName.empty())
    {
        std::cerr << "  " << theInfo.Name() << ": no files match " << fileFilter << std::endl;
        return;
    }

    loadFile(fileName, theInfo.Name(), theInfo.DataType());
}

bool HelpDataLoader::loadFile(const std::string& theFileName,
                              const std::string& theName,
                              NFmiInfoData::Type theDataType)
{
    try
    {
        LoadedHelpData loaded;
        loaded.name = theName;
        loaded.fileName = theFileName;
        loaded.dataType = theDataType;
        // Memory mapped, so opening a multi-gigabyte model file is cheap.
        loaded.data = std::make_shared<NFmiQueryData>(theFileName);
        loaded.info = std::make_shared<NFmiFastQueryInfo>(loaded.data.get());

        if(!loaded.info->IsGrid())
        {
            std::cerr << "  " << theName << ": " << theFileName
                      << " is station data, which the Linux renderer cannot draw yet"
                      << std::endl;
            return false;
        }

        std::cerr << "  " << theName << ": " << theFileName
                  << " (" << loaded.info->GridXNumber() << "x" << loaded.info->GridYNumber()
                  << ", " << loaded.info->SizeParams() << " params, "
                  << loaded.info->SizeTimes() << " times, "
                  << loaded.info->SizeLevels() << " levels)" << std::endl;

        itsLoadedData.push_back(std::move(loaded));
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << "  " << theName << ": cannot read " << theFileName << ": " << e.what()
                  << std::endl;
        return false;
    }
}

#endif // UNIX
