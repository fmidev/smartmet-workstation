#pragma once
// Loads the querydata files that the control directory configuration lists under the
// "MetEditor::HelpData" namespace.
//
// On Windows the help data is loaded and kept up to date by a background thread
// (NFmiEditMapGeneralDataDoc + the help data loading threads). The Linux port has no
// document/thread layer yet, so this class does the one thing the port needs: resolve
// each configured file filter to its newest matching file and memory map it once at
// startup.

#ifdef UNIX

#include <newbase/NFmiInfoData.h>

#include <memory>
#include <string>
#include <vector>

class NFmiFastQueryInfo;
class NFmiHelpDataInfo;
class NFmiHelpDataInfoSystem;
class NFmiQueryData;

// One gridded querydata file that the configuration pointed at.
struct LoadedHelpData
{
    std::string name;      // configuration name, e.g. "fmi_surface"
    std::string fileName;  // absolute path of the file that was actually loaded
    NFmiInfoData::Type dataType = NFmiInfoData::kNoDataType;
    std::shared_ptr<NFmiQueryData> data;
    std::shared_ptr<NFmiFastQueryInfo> info;
};

class HelpDataLoader
{
public:
    // Reads the "MetEditor::HelpData" settings and loads the newest file matching each
    // enabled data's file filter. The configuration must already be in NFmiSettings and
    // theControlPath must be absolute. Returns the number of files loaded.
    size_t loadFromSettings(const std::string& theControlPath);

    // Loads a single file outside of the configuration (the --data option).
    bool loadFile(const std::string& theFileName,
                  const std::string& theName,
                  NFmiInfoData::Type theDataType);

    const std::vector<LoadedHelpData>& loadedData() const { return itsLoadedData; }

    // Releases the memory mapped files. Call before static destructors run.
    void clear() { itsLoadedData.clear(); }

private:
    void loadHelpDataInfo(const NFmiHelpDataInfo& theInfo,
                          const NFmiHelpDataInfoSystem& theSystem);

    std::vector<LoadedHelpData> itsLoadedData;
};

#endif // UNIX
