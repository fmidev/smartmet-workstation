// Linux entry point for SmartMet workstation.
// Replaces the MFC CSmartMetApp with a Qt6-based application.

#ifdef UNIX

#include "qt_main_window.h"
#include "NFmiBasicSmartMetConfigurations.h"
#include "NFmiSettings.h"
#include "ToolMasterHelperFunctions.h"
#include "catlog/catlog.h"

#include <QApplication>
#include <QCommandLineParser>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

namespace
{
    NFmiBasicSmartMetConfigurations gBasicSmartMetConfigurations;

    std::string findControlPath(const std::string& explicitPath)
    {
        if(!explicitPath.empty())
            return explicitPath;
        // Default: look for control directory relative to executable
        if(std::filesystem::exists("control_scand_edit_local_conf"))
            return "control_scand_edit_local_conf";
        return "";
    }

    void initSettings()
    {
        // Initialize NFmiSettings with a config file for persistent settings
        auto configDir = std::string(std::getenv("HOME") ? std::getenv("HOME") : ".") + "/.config/smartmet";
        std::filesystem::create_directories(configDir);
        auto configFile = configDir + "/registry.conf";
        // Create file if it doesn't exist
        if(!std::filesystem::exists(configFile))
        {
            std::ofstream(configFile).close();
        }
        NFmiSettings::Read(configFile);
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

    // Initialize basic configurations
    std::string controlPath = findControlPath(parser.value("control-path").toStdString());
    if(!controlPath.empty())
        gBasicSmartMetConfigurations.SetControlPath(controlPath);

    if(!gBasicSmartMetConfigurations.Init(Toolmaster::MakeAvsToolmasterVersionString()))
    {
        std::cerr << "SmartMet: failed to initialize basic configurations" << std::endl;
        return 1;
    }

    // Initialize logging
    if(parser.isSet("verbose"))
        gBasicSmartMetConfigurations.Verbose(true);

    CatLog::logMessage("SmartMet Linux starting", CatLog::Severity::Info, CatLog::Category::Configuration, true);

    // Create main window
    SmartMetMainWindow mainWindow;
    mainWindow.setWindowTitle("SmartMet");
    mainWindow.resize(1200, 800);
    mainWindow.show();

    CatLog::logMessage("SmartMet Linux main window shown", CatLog::Severity::Info, CatLog::Category::Configuration, true);

    int result = app.exec();

    // Save settings on exit
    NFmiSettings::Save();

    return result;
}

#endif // UNIX
