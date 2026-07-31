// Linux entry point for SmartMet workstation.
// Replaces the MFC CSmartMetApp with a Qt6-based application.
// Loads real FMI querydata either from the control directory's help data configuration
// (-p) or from a single file given with --data.

#ifdef UNIX

#include "qt_application_interface.h"
#include "qt_document_view.h"
#include "qt_main_window.h"
#include "qt_map_view.h"
#include "weather_data_model.h"
#include "NFmiBasicSmartMetConfigurations.h"
#include "NFmiEditMapGeneralDataDoc.h"
#include "NFmiSettings.h"
#include "ToolMasterHelperFunctions.h"
#include "catlog/catlog.h"

#include <newbase/NFmiInfoData.h>

#include <QApplication>
#include <QCommandLineParser>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace
{
    NFmiBasicSmartMetConfigurations gBasicSmartMetConfigurations;

    // All loaded querydata plus the current view state
    WeatherDataModel gDataModel;

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

    // The real SmartMet document. Only created when --document is given: bringing it up
    // on Linux is work in progress, and the Qt window does not render its views yet.
    std::unique_ptr<NFmiEditMapGeneralDataDoc> gGeneralDataDoc;
    DocumentMapView gDocumentMapView;

    // Returns true if the document initialized. Reports what happened either way -
    // this is the entry point the rest of the view stack hangs off.
    bool initGeneralDataDoc()
    {
        // The document calls back into the application through ApplicationInterface,
        // so the implementation must be installed before Init().
        QtApplicationInterface::install();

        try
        {
            // The popup menu command id range is a Windows resource concept; the Qt port
            // has no menu resources, so start from a value that cannot clash with them.
            gGeneralDataDoc = std::make_unique<NFmiEditMapGeneralDataDoc>(40000);

            std::map<std::string, std::string> mapViewPositions;
            std::map<std::string, std::string> otherViewPositions;
            const bool ok = gGeneralDataDoc->Init(gBasicSmartMetConfigurations,
                                                  mapViewPositions, otherViewPositions);
            std::cerr << "Document init " << (ok ? "succeeded" : "returned false") << std::endl;
            if(!ok)
                gGeneralDataDoc.reset();
            return ok;
        }
        catch(const std::exception& e)
        {
            std::cerr << "Document init exception: " << e.what() << std::endl;
        }
        catch(...)
        {
            std::cerr << "Document init: unknown exception" << std::endl;
        }
        gGeneralDataDoc.reset();
        return false;
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
    parser.addOption({"document", "Experimental: bring up the real SmartMet document"});
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

    if(parser.isSet("document"))
    {
        if(!configOk)
            std::cerr << "--document needs a working control directory configuration" << std::endl;
        else
            initGeneralDataDoc();
    }

    // An explicit --data file comes first, so it is the one shown at startup.
    if(!dataPath.empty())
    {
        std::cerr << "Loading querydata given with --data" << std::endl;
        gDataModel.loader().loadFile(dataPath,
                                     std::filesystem::path(dataPath).filename().string(),
                                     NFmiInfoData::kViewable);
    }

    // Then everything the control directory configuration lists as help data.
    if(configOk)
        gDataModel.loader().loadFromSettings(gBasicSmartMetConfigurations.ControlPath());

    const bool dataLoaded = gDataModel.selectFirstDataWithValues();

    std::cerr << "SmartMet Linux starting";
    if(dataLoaded)
        std::cerr << " (" << gDataModel.dataCount() << " data loaded, showing "
                  << gDataModel.dataName() << " / " << gDataModel.paramName() << " "
                  << gDataModel.timeString() << ")";
    else
        std::cerr << " (demo mode)";
    std::cerr << std::endl;

    SmartMetMainWindow mainWindow(gDataModel);

    // With --document, let SmartMet's own map view draw the canvas. It tears itself
    // down on the first failure, after which the standalone renderer takes over.
    if(gGeneralDataDoc && !gDocumentMapView.create(0))
        std::cerr << "Real map view not available: " << gDocumentMapView.lastError()
                  << "\nUsing the standalone renderer instead." << std::endl;

    if(gDocumentMapView.isReady())
    {
        mainWindow.mapView()->setDocumentRenderer(
            [](QPainter& painter, int width, int height) -> bool
            {
                gDocumentMapView.setViewSize(width, height);
                return gDocumentMapView.draw(painter);
            });
        QtApplicationInterface::install().setRepaintCallback(
            [&mainWindow]() { mainWindow.mapView()->refresh(); });
    }

    mainWindow.show();

    CatLog::logMessage("SmartMet Linux main window shown", CatLog::Severity::Info, CatLog::Category::Configuration, true);

    int result = app.exec();

    // Release the memory mapped querydata before static destructors run
    gDataModel.clear();

    // Save settings on exit
    NFmiSettings::Save();

    return result;
}

#endif // UNIX
