// Linux entry point for SmartMet workstation.
// Replaces the MFC CSmartMetApp with a Qt6-based application.
// Supports loading real FMI querydata (.sqd) files via --data option.

#ifdef UNIX

#include "qt_main_window.h"
#include "weather_renderer.h"
#include "NFmiBasicSmartMetConfigurations.h"
#include "NFmiSettings.h"
#include "ToolMasterHelperFunctions.h"
#include "catlog/catlog.h"

#include <newbase/NFmiQueryData.h>
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

    // Loaded querydata (persists for the lifetime of the application)
    std::unique_ptr<NFmiQueryData> gQueryData;
    std::unique_ptr<NFmiFastQueryInfo> gQueryInfo;

    // Cached grid data extracted from querydata
    std::vector<float> gGridData;
    int gGridWidth = 0;
    int gGridHeight = 0;
    std::string gParamName;
    std::string gTimeStr;
    float gDataMin = 0;
    float gDataMax = 0;

    std::string findControlPath(const std::string& explicitPath)
    {
        if(!explicitPath.empty())
            return explicitPath;
        if(std::filesystem::exists("control_scand_edit_local_conf"))
            return "control_scand_edit_local_conf";
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

    // Load querydata from a .sqd file and extract the first parameter at first time step.
    // Returns true on success.
    bool loadQueryData(const std::string& dataFilePath)
    {
        if(!std::filesystem::exists(dataFilePath))
        {
            std::cerr << "Data file not found: " << dataFilePath << std::endl;
            return false;
        }

        std::cerr << "Loading querydata: " << dataFilePath << std::endl;

        try
        {
            gQueryData = std::make_unique<NFmiQueryData>(dataFilePath);
            gQueryInfo = std::make_unique<NFmiFastQueryInfo>(gQueryData.get());

            // Position to first parameter, first level, first time
            gQueryInfo->FirstParam();
            gQueryInfo->FirstLevel();
            gQueryInfo->FirstTime();

            // Get grid dimensions
            gGridWidth = static_cast<int>(gQueryInfo->GridXNumber());
            gGridHeight = static_cast<int>(gQueryInfo->GridYNumber());

            if(gGridWidth <= 0 || gGridHeight <= 0)
            {
                std::cerr << "Invalid grid dimensions: " << gGridWidth << "x" << gGridHeight << std::endl;
                return false;
            }

            // Get parameter name and time
            gParamName = std::string(gQueryInfo->Param().GetParamName().CharPtr());
            gTimeStr = std::string(gQueryInfo->Time().ToStr(NFmiString("YYYY.MM.DD HH:mm")).CharPtr());

            std::cerr << "Parameter: " << gParamName << std::endl;
            std::cerr << "Time: " << gTimeStr << std::endl;
            std::cerr << "Grid: " << gGridWidth << "x" << gGridHeight << std::endl;

            // Extract grid values using Values() -> NFmiDataMatrix<float>
            // NFmiDataMatrix is indexed as [x][y] with NX() columns and NY() rows
            NFmiDataMatrix<float> matrix = gQueryInfo->Values();

            std::cerr << "Matrix dimensions: NX=" << matrix.NX() << " NY=" << matrix.NY() << std::endl;

            // Flatten the matrix to a row-major vector for the renderer.
            // The renderer expects grid[y * width + x] layout.
            gGridData.resize(gGridWidth * gGridHeight);
            gDataMin = std::numeric_limits<float>::max();
            gDataMax = std::numeric_limits<float>::lowest();
            int validCount = 0;

            for(int y = 0; y < gGridHeight; ++y)
            {
                for(int x = 0; x < gGridWidth; ++x)
                {
                    float val = matrix[x][y];
                    // Replace missing values with NaN so the renderer skips them
                    if(val == kFloatMissing || val >= 32000.0f)
                    {
                        gGridData[y * gGridWidth + x] = std::numeric_limits<float>::quiet_NaN();
                    }
                    else
                    {
                        gGridData[y * gGridWidth + x] = val;
                        gDataMin = std::min(gDataMin, val);
                        gDataMax = std::max(gDataMax, val);
                        ++validCount;
                    }
                }
            }

            std::cerr << "Data range: [" << gDataMin << ", " << gDataMax << "] (" << validCount << " valid values)" << std::endl;

            if(validCount == 0)
            {
                std::cerr << "Warning: no valid data values found" << std::endl;
                return false;
            }

            // Report area info if available
            const NFmiArea* area = gQueryInfo->Area();
            if(area)
            {
                std::cerr << "Area class: " << area->ClassName() << std::endl;
            }

            return true;
        }
        catch(const std::exception& e)
        {
            std::cerr << "Failed to load querydata: " << e.what() << std::endl;
            return false;
        }
    }

    // Refresh gGridData from current gQueryInfo position.
    // Call after changing param/time/level.
    bool refreshGridData()
    {
        if(!gQueryInfo) return false;
        try
        {
            gGridWidth = static_cast<int>(gQueryInfo->GridXNumber());
            gGridHeight = static_cast<int>(gQueryInfo->GridYNumber());
            gParamName = std::string(gQueryInfo->Param().GetParamName().CharPtr());
            gTimeStr = std::string(gQueryInfo->Time().ToStr(NFmiString("YYYY.MM.DD HH:mm")).CharPtr());

            NFmiDataMatrix<float> matrix = gQueryInfo->Values();
            gGridData.resize(gGridWidth * gGridHeight);
            gDataMin = std::numeric_limits<float>::max();
            gDataMax = std::numeric_limits<float>::lowest();

            for(int y = 0; y < gGridHeight; ++y)
                for(int x = 0; x < gGridWidth; ++x)
                {
                    float val = matrix[x][y];
                    if(val == kFloatMissing || val >= 32000.0f)
                        gGridData[y * gGridWidth + x] = std::numeric_limits<float>::quiet_NaN();
                    else
                    {
                        gGridData[y * gGridWidth + x] = val;
                        gDataMin = std::min(gDataMin, val);
                        gDataMax = std::max(gDataMax, val);
                    }
                }
            return gDataMin <= gDataMax;
        }
        catch(...) { return false; }
    }

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

        // Use the actual data min/max for color mapping
        float dMin = gDataMin;
        float dMax = gDataMax;
        auto colorFunc = [dMin, dMax](float v) -> unsigned int { return adaptiveColor(v, dMin, dMax); };

        // Render color-mapped field
        auto colorLayer = WeatherRenderer::renderColorGrid(
            gGridData, gGridWidth, gGridHeight, w, h, colorFunc);

        // Compute isoline values based on data range
        double interval = niceIsolineInterval(gDataMin, gDataMax);
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
        auto result = WeatherRenderer::compositeLayers(w, h,
            {colorLayer, isolineLayer, refLine});

        // Draw to window via QPainter overlay (GUI elements)
        QPainter* painter = window.beginDrawing();
        if(painter)
        {
            // Background
            painter->fillRect(0, 0, w, h, QColor(240, 240, 245));

            // Weather data layers
            painter->drawImage(0, 0, result);

            // GUI overlay: title with parameter name and time
            painter->setPen(QPen(Qt::black, 1));
            QFont titleFont("Sans", 16, QFont::Bold);
            painter->setFont(titleFont);
            QString title = QString("SmartMet Linux - %1 [%2]")
                .arg(QString::fromStdString(gParamName))
                .arg(QString::fromStdString(gTimeStr));
            painter->drawText(20, 30, title);

            // Legend
            QFont legendFont("Sans", 10);
            painter->setFont(legendFont);
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

            // Grid info + controls help
            painter->drawText(20, h - 35,
                QString("Grid: %1x%2 | Image: %3x%4 | Range: [%5, %6] | Interval: %7")
                    .arg(gGridWidth).arg(gGridHeight).arg(w).arg(h)
                    .arg(static_cast<double>(gDataMin), 0, 'f', 1)
                    .arg(static_cast<double>(gDataMax), 0, 'f', 1)
                    .arg(interval, 0, 'f', 1));
            painter->setPen(QColor(100, 100, 100));
            painter->drawText(20, h - 15,
                QString::fromUtf8("\u2190\u2192 Time | \u2191\u2193 Parameter | PgUp/PgDn Level"));
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

    // Initialize settings persistence
    initSettings();

    // Initialize basic configurations (optional - demo mode works without config)
    bool configOk = false;
    std::string controlPath = findControlPath(parser.value("control-path").toStdString());
    if(!controlPath.empty())
        gBasicSmartMetConfigurations.SetControlPath(controlPath);

    try
    {
        configOk = gBasicSmartMetConfigurations.Init(Toolmaster::MakeAvsToolmasterVersionString());
    }
    catch(const std::exception& e)
    {
        std::cerr << "SmartMet: configuration init failed: " << e.what() << std::endl;
        std::cerr << "Running in demo mode (no weather data)" << std::endl;
    }

    if(parser.isSet("verbose"))
        gBasicSmartMetConfigurations.Verbose(true);

    // Try to load real querydata if --data was specified
    bool dataLoaded = false;
    std::string dataPath = parser.value("data").toStdString();
    if(!dataPath.empty())
    {
        dataLoaded = loadQueryData(dataPath);
        if(!dataLoaded)
            std::cerr << "Falling back to demo mode" << std::endl;
    }

    std::cerr << "SmartMet Linux starting"
              << (dataLoaded ? " (real data)" : (configOk ? "" : " (demo mode)"))
              << std::endl;

    // Create main window
    SmartMetMainWindow mainWindow;
    if(dataLoaded)
    {
        mainWindow.setWindowTitle(
            QString("SmartMet - %1 [%2]")
                .arg(QString::fromStdString(gParamName))
                .arg(QString::fromStdString(gTimeStr)));
    }
    else
    {
        mainWindow.setWindowTitle("SmartMet (demo)");
    }
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
    auto updateTitle = [&mainWindow]()
    {
        mainWindow.setWindowTitle(
            QString("SmartMet - %1 [%2]")
                .arg(QString::fromStdString(gParamName))
                .arg(QString::fromStdString(gTimeStr)));
    };

    QObject::connect(&mainWindow, &SmartMetMainWindow::keyPressed,
        [&mainWindow, &doRender, &updateTitle, dataLoaded](int key, int modifiers)
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
                default: break;
            }
            if(changed)
            {
                updateTitle();
                doRender();
            }
        });

    mainWindow.show();

    // Initial render after the window is shown
    QTimer::singleShot(100, doRender);

    CatLog::logMessage("SmartMet Linux main window shown", CatLog::Severity::Info, CatLog::Category::Configuration, true);

    int result = app.exec();

    // Clean up querydata before static destructors
    gQueryInfo.reset();
    gQueryData.reset();

    // Save settings on exit
    NFmiSettings::Save();

    return result;
}

#endif // UNIX
