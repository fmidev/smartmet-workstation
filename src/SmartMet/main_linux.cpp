// Linux entry point for SmartMet workstation.
// Replaces the MFC CSmartMetApp with a Qt6-based application.
// Loads real FMI querydata either from the control directory's help data configuration
// (-p) or from a single file given with --data.

#ifdef UNIX

#include "help_data_loader.h"
#include "qt_main_window.h"
#include "weather_renderer.h"
#include "NFmiBasicSmartMetConfigurations.h"
#include "NFmiSettings.h"
#include "ToolMasterHelperFunctions.h"
#include "catlog/catlog.h"

#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiDataMatrix.h>
#include <newbase/NFmiGlobals.h>

#include <QApplication>
#include <QCommandLineParser>
#include <QPainter>
#include <QTimer>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <cmath>
#include <memory>
#include <algorithm>

namespace
{
    NFmiBasicSmartMetConfigurations gBasicSmartMetConfigurations;

    // All querydata loaded at startup (persists for the lifetime of the application)
    HelpDataLoader gDataLoader;

    // Currently displayed data. gQueryInfo points into gDataLoader and is never owned here.
    int gCurrentDataIndex = -1;
    NFmiFastQueryInfo* gQueryInfo = nullptr;
    std::string gDataName;

    // Cached grid data extracted from querydata
    std::vector<float> gGridData;
    int gGridWidth = 0;
    int gGridHeight = 0;
    std::string gParamName;
    std::string gTimeStr;
    float gDataMin = 0;
    float gDataMax = 0;
    int gValidValueCount = 0;

    // Returns an absolute path. The configuration Init() chdir's before it resolves
    // anything, so relative paths must be pinned down while the original cwd is still valid.
    std::string findControlPath(const std::string& explicitPath)
    {
        std::error_code ec;
        if(!explicitPath.empty())
            return std::filesystem::absolute(explicitPath, ec).string();
        // Search for control directories in common locations
        for(const auto& path : {
            "control_linux",
            "../control_linux",
            "control_scand_edit_local_conf"})
        {
            if(std::filesystem::exists(path))
                return std::filesystem::absolute(path, ec).string();
        }
        return "";
    }

    void initSettings()
    {
        try
        {
            auto configDir = std::string(std::getenv("HOME") ? std::getenv("HOME") : ".") + "/.config/smartmet";
            std::filesystem::create_directories(configDir);
            auto configFile = configDir + "/registry.conf";
            if(std::filesystem::exists(configFile) && std::filesystem::file_size(configFile) > 10)
                NFmiSettings::Read(configFile);
        }
        catch(const std::exception& e)
        {
            std::cerr << "Settings init skipped: " << e.what() << std::endl;
        }
    }

    // Refresh gGridData from the current gQueryInfo position. Call after changing
    // data/param/time/level. Returns true if the position has a usable grid; a grid with
    // nothing but missing values is still usable, gValidValueCount then tells it is empty.
    bool refreshGridData()
    {
        gValidValueCount = 0;
        if(!gQueryInfo) return false;
        try
        {
            gGridWidth = static_cast<int>(gQueryInfo->GridXNumber());
            gGridHeight = static_cast<int>(gQueryInfo->GridYNumber());
            if(gGridWidth <= 0 || gGridHeight <= 0)
                return false;

            gParamName = std::string(gQueryInfo->Param().GetParamName().CharPtr());
            gTimeStr = std::string(gQueryInfo->Time().ToStr(NFmiString("YYYY.MM.DD HH:mm")).CharPtr());

            // NFmiDataMatrix is indexed as [x][y], the renderer expects grid[y * width + x]
            NFmiDataMatrix<float> matrix = gQueryInfo->Values();
            gGridData.resize(gGridWidth * gGridHeight);
            gDataMin = std::numeric_limits<float>::max();
            gDataMax = std::numeric_limits<float>::lowest();

            for(int y = 0; y < gGridHeight; ++y)
                for(int x = 0; x < gGridWidth; ++x)
                {
                    float val = matrix[x][y];
                    // Replace missing values with NaN so the renderer skips them
                    if(val == kFloatMissing || val >= 32000.0f)
                        gGridData[y * gGridWidth + x] = std::numeric_limits<float>::quiet_NaN();
                    else
                    {
                        gGridData[y * gGridWidth + x] = val;
                        gDataMin = std::min(gDataMin, val);
                        gDataMax = std::max(gDataMax, val);
                        ++gValidValueCount;
                    }
                }

            if(gValidValueCount == 0)
            {
                gDataMin = 0;
                gDataMax = 0;
            }
            return true;
        }
        catch(...) { return false; }
    }

    // Make the given loaded data the displayed one, positioned at its first
    // parameter/level/time. theIndex wraps around both ends.
    bool selectData(int theIndex)
    {
        const auto& datas = gDataLoader.loadedData();
        if(datas.empty()) return false;

        const int count = static_cast<int>(datas.size());
        theIndex = ((theIndex % count) + count) % count;

        const auto& selected = datas[theIndex];
        gCurrentDataIndex = theIndex;
        gDataName = selected.name;
        gQueryInfo = selected.info.get();
        gQueryInfo->FirstParam();
        gQueryInfo->FirstLevel();
        gQueryInfo->FirstTime();
        return refreshGridData();
    }

    // Startup selection: the first parameter of the first data is often all missing, which
    // would look exactly like a broken load. Show the first data/parameter that has values.
    bool selectFirstDataWithValues()
    {
        const int count = static_cast<int>(gDataLoader.loadedData().size());
        for(int i = 0; i < count; ++i)
        {
            if(!selectData(i))
                continue;
            if(gValidValueCount > 0)
                return true;
            while(gQueryInfo->NextParam())
            {
                if(refreshGridData() && gValidValueCount > 0)
                    return true;
            }
        }
        // Nothing had values - still show the first data rather than the demo screen.
        return selectData(0);
    }

    bool nextData()     { return selectData(gCurrentDataIndex + 1); }
    bool prevData()     { return selectData(gCurrentDataIndex - 1); }
    bool nextTime()     { return gQueryInfo && gQueryInfo->NextTime() && refreshGridData(); }
    bool prevTime()     { return gQueryInfo && gQueryInfo->PreviousTime() && refreshGridData(); }
    bool nextParam()    { return gQueryInfo && gQueryInfo->NextParam() && refreshGridData(); }
    bool prevParam()    { return gQueryInfo && gQueryInfo->PreviousParam() && refreshGridData(); }
    bool nextLevel()    { return gQueryInfo && gQueryInfo->NextLevel() && refreshGridData(); }
    bool prevLevel()    { return gQueryInfo && gQueryInfo->PreviousLevel() && refreshGridData(); }

    // Generate sample temperature grid data for demonstration (fallback)
    std::vector<float> generateSampleGrid(int width, int height)
    {
        std::vector<float> grid(width * height);
        for(int j = 0; j < height; ++j)
        {
            for(int i = 0; i < width; ++i)
            {
                double x = static_cast<double>(i) / (width - 1);
                double y = static_cast<double>(j) / (height - 1);
                // Simulated temperature field: warm center, cold edges, some wave pattern
                double cx = 0.55, cy = 0.45;
                double dist = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
                double temp = 20.0 - 30.0 * dist
                    + 3.0 * std::sin(x * 8.0) * std::cos(y * 6.0)
                    + 2.0 * std::sin((x + y) * 5.0);
                grid[j * width + i] = static_cast<float>(temp);
            }
        }
        return grid;
    }

    // Adaptive color function: maps data values to blue->white->red gradient
    // using the actual data range
    unsigned int adaptiveColor(float value, float dataMin, float dataMax)
    {
        if(!std::isfinite(value))
            return 0x00000000; // transparent for missing

        float range = dataMax - dataMin;
        if(range < 0.001f) range = 1.0f;
        float normalized = (value - dataMin) / range; // 0..1
        normalized = std::clamp(normalized, 0.0f, 1.0f);

        int r, g, b;
        if(normalized < 0.5f)
        {
            float f = normalized * 2.0f;
            r = static_cast<int>(f * 255);
            g = static_cast<int>(f * 255);
            b = 255;
        }
        else
        {
            float f = (normalized - 0.5f) * 2.0f;
            r = 255;
            g = static_cast<int>((1.0f - f) * 255);
            b = static_cast<int>((1.0f - f) * 255);
        }
        return 0xC0000000u | (r << 16) | (g << 8) | b; // semi-transparent
    }

    // Fixed color function for demo mode: blue (cold) -> white (0) -> red (hot)
    unsigned int temperatureColor(float value)
    {
        return adaptiveColor(value, -20.0f, 30.0f);
    }

    // Compute nice isoline interval based on data range
    double niceIsolineInterval(float dataMin, float dataMax)
    {
        double range = dataMax - dataMin;
        if(range <= 0) return 1.0;

        // Aim for approximately 10 isolines
        double rawInterval = range / 10.0;

        // Round to a "nice" number: 0.1, 0.2, 0.5, 1, 2, 5, 10, 20, 50, ...
        double magnitude = std::pow(10.0, std::floor(std::log10(rawInterval)));
        double residual = rawInterval / magnitude;
        double niceResidual;
        if(residual <= 1.5)
            niceResidual = 1.0;
        else if(residual <= 3.5)
            niceResidual = 2.0;
        else if(residual <= 7.5)
            niceResidual = 5.0;
        else
            niceResidual = 10.0;

        return niceResidual * magnitude;
    }

    void renderDataView(SmartMetMainWindow& window)
    {
        int w = window.width();
        int h = window.height();
        if(w < 10 || h < 10) return;

        const bool hasValues = gValidValueCount > 0;

        // Use the actual data min/max for color mapping
        float dMin = gDataMin;
        float dMax = gDataMax;
        auto colorFunc = [dMin, dMax](float v) -> unsigned int { return adaptiveColor(v, dMin, dMax); };

        double interval = 1.0;
        QImage result(w, h, QImage::Format_ARGB32_Premultiplied);
        if(hasValues)
        {
            // Render color-mapped field
            auto colorLayer = WeatherRenderer::renderColorGrid(
                gGridData, gGridWidth, gGridHeight, w, h, colorFunc);

            // Compute isoline values based on data range
            interval = niceIsolineInterval(gDataMin, gDataMax);
            double firstIso = std::ceil(gDataMin / interval) * interval;
            std::vector<double> isoValues;
            for(double v = firstIso; v <= gDataMax; v += interval)
                isoValues.push_back(v);

            auto isolineLayer = WeatherRenderer::renderIsolines(
                gGridData, gGridWidth, gGridHeight, w, h, isoValues,
                0xFF000000, 1.5);  // black lines, 1.5px

            // Find a "round" reference value for a thicker highlight isoline
            double midValue = (gDataMin + gDataMax) / 2.0;
            double refValue = std::round(midValue / interval) * interval;
            auto refLine = WeatherRenderer::renderIsolines(
                gGridData, gGridWidth, gGridHeight, w, h, {refValue},
                0xFF0000FF, 3.0);  // blue, 3px

            // Composite all layers
            result = WeatherRenderer::compositeLayers(w, h,
                {colorLayer, isolineLayer, refLine});
        }
        else
        {
            result.fill(Qt::transparent);
        }

        // Draw to window via QPainter overlay (GUI elements)
        QPainter* painter = window.beginDrawing();
        if(painter)
        {
            // Background
            painter->fillRect(0, 0, w, h, QColor(240, 240, 245));

            // Weather data layers
            painter->drawImage(0, 0, result);

            // GUI overlay: title with data name, parameter name and time
            painter->setPen(QPen(Qt::black, 1));
            QFont titleFont("Sans", 16, QFont::Bold);
            painter->setFont(titleFont);
            QString title = QString("SmartMet Linux - %1: %2 [%3]")
                .arg(QString::fromStdString(gDataName))
                .arg(QString::fromStdString(gParamName))
                .arg(QString::fromStdString(gTimeStr));
            painter->drawText(20, 30, title);

            QFont legendFont("Sans", 10);
            painter->setFont(legendFont);

            if(hasValues)
            {
                // Legend
                int legendX = w - 170, legendY = 40;
                int legendEntries = 10;
                int legendHeight = 40 + legendEntries * 16;
                painter->fillRect(legendX - 10, legendY - 15, 160, legendHeight, QColor(255, 255, 255, 200));
                painter->drawRect(legendX - 10, legendY - 15, 160, legendHeight);
                painter->drawText(legendX, legendY, QString::fromStdString(gParamName));
                for(int i = 0; i <= legendEntries; ++i)
                {
                    float val = gDataMin + i * (gDataMax - gDataMin) / legendEntries;
                    unsigned int c = adaptiveColor(val, gDataMin, gDataMax);
                    QColor qc((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
                    painter->fillRect(legendX, legendY + 15 + i * 16, 20, 14, qc);
                    painter->drawText(legendX + 25, legendY + 27 + i * 16,
                        QString::number(static_cast<double>(val), 'f', 1));
                }
            }
            else
            {
                painter->drawText(20, 55,
                    "No values for this parameter / level / time step");
            }

            // Grid info + controls help
            painter->drawText(20, h - 35,
                hasValues
                    ? QString("Data %1/%2 | Grid: %3x%4 | Image: %5x%6 | Range: [%7, %8] | Interval: %9")
                          .arg(gCurrentDataIndex + 1)
                          .arg(static_cast<int>(gDataLoader.loadedData().size()))
                          .arg(gGridWidth).arg(gGridHeight).arg(w).arg(h)
                          .arg(static_cast<double>(gDataMin), 0, 'f', 1)
                          .arg(static_cast<double>(gDataMax), 0, 'f', 1)
                          .arg(interval, 0, 'f', 1)
                    : QString("Data %1/%2 | Grid: %3x%4 | Image: %5x%6 | no values")
                          .arg(gCurrentDataIndex + 1)
                          .arg(static_cast<int>(gDataLoader.loadedData().size()))
                          .arg(gGridWidth).arg(gGridHeight).arg(w).arg(h));
            painter->setPen(QColor(100, 100, 100));
            painter->drawText(20, h - 15,
                QString::fromUtf8("\u2190\u2192 Time | \u2191\u2193 Parameter | PgUp/PgDn Level | N/P Data"));
        }
        window.endDrawing();
    }

    void renderDemoView(SmartMetMainWindow& window)
    {
        int w = window.width();
        int h = window.height();
        if(w < 10 || h < 10) return;

        // Generate sample grid data
        const int gridW = 50, gridH = 40;
        auto gridData = generateSampleGrid(gridW, gridH);

        // Render color-mapped temperature field
        auto colorLayer = WeatherRenderer::renderColorGrid(
            gridData, gridW, gridH, w, h, temperatureColor);

        // Render isolines at every 5 degrees
        std::vector<double> isoValues;
        for(double t = -15.0; t <= 25.0; t += 5.0)
            isoValues.push_back(t);

        auto isolineLayer = WeatherRenderer::renderIsolines(
            gridData, gridW, gridH, w, h, isoValues,
            0xFF000000, 1.5);  // black lines, 1.5px

        // Render zero-degree isoline thicker in blue
        auto zeroline = WeatherRenderer::renderIsolines(
            gridData, gridW, gridH, w, h, {0.0},
            0xFF0000FF, 3.0);  // blue, 3px

        // Composite all layers
        auto result = WeatherRenderer::compositeLayers(w, h,
            {colorLayer, isolineLayer, zeroline});

        // Draw to window via QPainter overlay (GUI elements)
        QPainter* painter = window.beginDrawing();
        if(painter)
        {
            // Background
            painter->fillRect(0, 0, w, h, QColor(240, 240, 245));

            // Weather data layers
            painter->drawImage(0, 0, result);

            // GUI overlay: title
            painter->setPen(QPen(Qt::black, 1));
            QFont titleFont("Sans", 16, QFont::Bold);
            painter->setFont(titleFont);
            painter->drawText(20, 30, "SmartMet Linux - Temperature Demo");

            // Legend
            QFont legendFont("Sans", 10);
            painter->setFont(legendFont);
            int legendX = w - 150, legendY = 40;
            painter->fillRect(legendX - 10, legendY - 15, 140, 200, QColor(255, 255, 255, 200));
            painter->drawRect(legendX - 10, legendY - 15, 140, 200);
            painter->drawText(legendX, legendY, "Temperature (C)");
            for(int i = 0; i <= 10; ++i)
            {
                float t = -20.0f + i * 5.0f;
                unsigned int c = temperatureColor(t);
                QColor qc((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
                painter->fillRect(legendX, legendY + 15 + i * 16, 20, 14, qc);
                painter->drawText(legendX + 25, legendY + 27 + i * 16,
                    QString::number(static_cast<int>(t)) + QString::fromUtf8("\u00B0"));
            }

            // Grid info
            painter->drawText(20, h - 20,
                QString("Grid: %1x%2 | Image: %3x%4 | Isolines: 5\u00B0 interval")
                    .arg(gridW).arg(gridH).arg(w).arg(h));
        }
        window.endDrawing();
    }
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("SmartMet");
    app.setOrganizationName("FMI");

    QCommandLineParser parser;
    parser.setApplicationDescription("SmartMet Weather Editor");
    parser.addHelpOption();
    parser.addOption({{"p", "control-path"}, "Control directory path", "path"});
    parser.addOption({{"d", "data"}, "Path to a .sqd querydata file to load", "file"});
    parser.addOption({{"v", "verbose"}, "Enable verbose logging"});
    parser.process(app);

    // Pin the data file down while the original working directory is still current:
    // NFmiBasicSmartMetConfigurations::Init() chdir's before the data is loaded.
    std::string dataPath = parser.value("data").toStdString();
    if(!dataPath.empty())
    {
        std::error_code ec;
        dataPath = std::filesystem::absolute(dataPath, ec).string();
    }

    // Initialize settings persistence
    initSettings();

    // On Windows the working directory is taken one step up from the cwd, because since
    // version 5.4 the exe is started from its own 32/64-bit subdirectory. On Linux the app
    // is started from the project root, so that climb up must be disabled - otherwise the
    // control path is resolved against the parent directory and never found.
    gBasicSmartMetConfigurations.DeveloperModePath(true);

    // Initialize basic configurations (optional - demo mode works without config)
    bool configOk = false;
    std::string controlPath = findControlPath(parser.value("control-path").toStdString());
    bool controlPathOk = false;
    if(!controlPath.empty())
    {
        controlPathOk = gBasicSmartMetConfigurations.SetControlPath(controlPath);
        if(!controlPathOk)
            std::cerr << "Control path not usable: " << controlPath << std::endl;
    }

    if(controlPathOk)
    {
        try
        {
            configOk = gBasicSmartMetConfigurations.Init(Toolmaster::MakeAvsToolmasterVersionString());
            if(configOk)
                std::cerr << "Configuration loaded from: " << controlPath << std::endl;
            else
                std::cerr << "Configuration init returned false for: " << controlPath << std::endl;
        }
        catch(const std::exception& e)
        {
            std::cerr << "Configuration init exception: " << e.what() << std::endl;
        }
        catch(...)
        {
            std::cerr << "Configuration init: unknown exception" << std::endl;
        }
    }

    if(parser.isSet("verbose"))
        gBasicSmartMetConfigurations.Verbose(true);

    // An explicit --data file comes first, so it is the one shown at startup.
    if(!dataPath.empty())
    {
        std::cerr << "Loading querydata given with --data" << std::endl;
        gDataLoader.loadFile(dataPath,
                             std::filesystem::path(dataPath).filename().string(),
                             NFmiInfoData::kViewable);
    }

    // Then everything the control directory configuration lists as help data.
    if(configOk)
        gDataLoader.loadFromSettings(gBasicSmartMetConfigurations.ControlPath());

    bool dataLoaded = selectFirstDataWithValues();

    std::cerr << "SmartMet Linux starting";
    if(dataLoaded)
        std::cerr << " (" << gDataLoader.loadedData().size() << " data loaded, showing "
                  << gDataName << " / " << gParamName << " " << gTimeStr << ")";
    else
        std::cerr << " (demo mode)";
    std::cerr << std::endl;

    // Window title follows the current data / parameter / time
    auto makeWindowTitle = []()
    {
        return QString("SmartMet - %1: %2 [%3]")
            .arg(QString::fromStdString(gDataName))
            .arg(QString::fromStdString(gParamName))
            .arg(QString::fromStdString(gTimeStr));
    };

    // Create main window
    SmartMetMainWindow mainWindow;
    mainWindow.setWindowTitle(dataLoaded ? makeWindowTitle() : QString("SmartMet (demo)"));
    mainWindow.resize(1200, 800);

    // Choose render function based on whether real data was loaded
    auto doRender = [&mainWindow, dataLoaded]()
    {
        if(dataLoaded)
            renderDataView(mainWindow);
        else
            renderDemoView(mainWindow);
    };

    QObject::connect(&mainWindow, &SmartMetMainWindow::resized, doRender);

    // Keyboard navigation for querydata
    QObject::connect(&mainWindow, &SmartMetMainWindow::keyPressed,
        [&mainWindow, &doRender, &makeWindowTitle, dataLoaded](int key, int /* modifiers */)
        {
            if(!dataLoaded) return;
            bool changed = false;
            switch(key)
            {
                case Qt::Key_Right: changed = nextTime(); break;
                case Qt::Key_Left:  changed = prevTime(); break;
                case Qt::Key_Up:    changed = nextParam(); break;
                case Qt::Key_Down:  changed = prevParam(); break;
                case Qt::Key_PageUp:   changed = nextLevel(); break;
                case Qt::Key_PageDown: changed = prevLevel(); break;
                case Qt::Key_N:     changed = nextData(); break;
                case Qt::Key_P:     changed = prevData(); break;
                default: break;
            }
            if(changed)
            {
                mainWindow.setWindowTitle(makeWindowTitle());
                doRender();
            }
        });

    mainWindow.show();

    // Initial render after the window is shown
    QTimer::singleShot(100, doRender);

    CatLog::logMessage("SmartMet Linux main window shown", CatLog::Severity::Info, CatLog::Category::Configuration, true);

    int result = app.exec();

    // Clean up querydata before static destructors
    gQueryInfo = nullptr;
    gDataLoader.clear();

    // Save settings on exit
    NFmiSettings::Save();

    return result;
}

#endif // UNIX
