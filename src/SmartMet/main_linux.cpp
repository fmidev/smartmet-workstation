// Linux entry point for SmartMet workstation.
// Replaces the MFC CSmartMetApp with a Qt6-based application.

#ifdef UNIX

#include "qt_main_window.h"
#include "weather_renderer.h"
#include "NFmiBasicSmartMetConfigurations.h"
#include "NFmiSettings.h"
#include "ToolMasterHelperFunctions.h"
#include "catlog/catlog.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QPainter>
#include <QTimer>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <cmath>

namespace
{
    NFmiBasicSmartMetConfigurations gBasicSmartMetConfigurations;

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

    // Generate sample temperature grid data for demonstration
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

    // Color function for temperature: blue (cold) -> white (0) -> red (hot)
    unsigned int temperatureColor(float value)
    {
        float t = std::clamp(value, -20.0f, 30.0f);
        float normalized = (t + 20.0f) / 50.0f; // 0..1

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

    std::cerr << "SmartMet Linux starting" << (configOk ? "" : " (demo mode)") << std::endl;

    // Create main window
    SmartMetMainWindow mainWindow;
    mainWindow.setWindowTitle("SmartMet");
    mainWindow.resize(1200, 800);

    // Render demo on show and resize
    auto doRender = [&mainWindow]() { renderDemoView(mainWindow); };

    QObject::connect(&mainWindow, &SmartMetMainWindow::resized, doRender);

    mainWindow.show();

    // Initial render after the window is shown
    QTimer::singleShot(100, doRender);

    CatLog::logMessage("SmartMet Linux main window shown", CatLog::Severity::Info, CatLog::Category::Configuration, true);

    int result = app.exec();

    // Save settings on exit
    NFmiSettings::Save();

    return result;
}

#endif // UNIX
