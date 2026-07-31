#pragma once
// Current view state over the querydata that HelpDataLoader loaded: which data,
// parameter, level and time step is being shown, and the grid values extracted for it.
//
// Kept free of Qt so that the window, the map view and main() all read the same state
// without one of them owning it.

#ifdef UNIX

#include "help_data_loader.h"

#include <string>
#include <vector>

class NFmiFastQueryInfo;

class WeatherDataModel
{
public:
    HelpDataLoader& loader() { return itsLoader; }
    const HelpDataLoader& loader() const { return itsLoader; }

    // ---- selection -------------------------------------------------------
    // theIndex wraps around both ends. Positions at first param/level/time.
    bool selectData(int theIndex);
    // The first parameter of the first data is often entirely missing values, which
    // looks exactly like a failed load. Pick the first data/parameter that has values.
    bool selectFirstDataWithValues();
    bool selectParam(int theIndex);
    bool selectLevel(int theIndex);
    bool selectTime(int theIndex);

    bool nextData()  { return selectData(itsCurrentDataIndex + 1); }
    bool prevData()  { return selectData(itsCurrentDataIndex - 1); }
    bool nextTime();
    bool prevTime();
    bool nextParam();
    bool prevParam();
    bool nextLevel();
    bool prevLevel();

    // ---- current state ---------------------------------------------------
    bool hasData() const { return itsQueryInfo != nullptr; }
    int currentDataIndex() const { return itsCurrentDataIndex; }
    int dataCount() const { return static_cast<int>(itsLoader.loadedData().size()); }
    const std::string& dataName() const { return itsDataName; }
    const std::string& paramName() const { return itsParamName; }
    const std::string& timeString() const { return itsTimeString; }
    const std::string& levelString() const { return itsLevelString; }

    int gridWidth() const { return itsGridWidth; }
    int gridHeight() const { return itsGridHeight; }
    const std::vector<float>& gridValues() const { return itsGridValues; }
    float dataMin() const { return itsDataMin; }
    float dataMax() const { return itsDataMax; }
    int validValueCount() const { return itsValidValueCount; }

    // ---- lists for the UI ------------------------------------------------
    std::vector<std::string> dataNames() const;
    std::vector<std::string> paramNames() const;   // of the current data
    std::vector<std::string> levelNames() const;   // of the current data
    std::vector<std::string> timeNames() const;    // of the current data
    int currentParamIndex() const;
    int currentLevelIndex() const;
    int currentTimeIndex() const;

    // ---- cursor readout --------------------------------------------------
    // theRelX/theRelY are 0..1 across the drawn map, theRelY measured from the top.
    // Returns false outside the grid or when the value under the cursor is missing.
    bool valueAt(double theRelX, double theRelY,
                 double& theLatOut, double& theLonOut, float& theValueOut) const;

    // Releases the memory mapped files. Call before static destructors run.
    void clear();

private:
    // Re-reads the grid for the current position. Returns true if the position has a
    // usable grid; a grid of nothing but missing values is still usable and leaves
    // validValueCount() at zero.
    bool refreshGridData();

    HelpDataLoader itsLoader;

    int itsCurrentDataIndex = -1;
    NFmiFastQueryInfo* itsQueryInfo = nullptr;  // points into itsLoader, never owned
    std::string itsDataName;
    std::string itsParamName;
    std::string itsTimeString;
    std::string itsLevelString;

    std::vector<float> itsGridValues;
    int itsGridWidth = 0;
    int itsGridHeight = 0;
    float itsDataMin = 0;
    float itsDataMax = 0;
    int itsValidValueCount = 0;
};

#endif // UNIX
