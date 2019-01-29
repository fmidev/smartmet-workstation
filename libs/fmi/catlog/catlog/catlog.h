#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <functional>

// CatLog is short of "category logging". This means that there is logging messages from different 
// subjects (categories) like: editing, data, configuration, visualization, etc.
// So users may search log messages with these categories to narrow down results.
// One can log messages before logger is initilaized. 


namespace CatLog
{
    enum class Category
    {
        NoCategory = 0,
        // Anything relating initialization and reading/writing configurations
        Configuration = 1,
        // Anything relating normal data handling
        Data = 2,
        // Edited data and it's editing related
        Editing = 3,
        // Data visualization related
        Visualization = 4,
        // Different macro related stuff (viewMacro, smarttools, macroParams, etc.)
        Macro = 5,
        // Anything relating to normal operational functions
        Operational = 6,
        // Anything related to net communication and data sending/requesting over net
        NetRequest = 7
    };

    // Log message level (severity). Used log-level determines if message is logged at all.
    // So if severity is set to Info, then Debug and Trace messages are ignored and all equal and above are logged.
    enum class Severity
    {
        Trace = 0,
        Debug = 1,
        Info = 2,
        Warning = 3,
        Error = 4,
        Critical = 5,
        NoSeverity = 6
    };

    struct LogData
    {
        LogData(Category category, Severity severity, const std::string &message, const std::chrono::system_clock::time_point &logTime);

        Category category_ = Category::NoCategory;
        Severity severity_ = Severity::NoSeverity;
        std::string message_;
        std::chrono::system_clock::time_point logTime_;
    };

    void initLogger(const std::string &filePath, size_t maximumMessagesKept, Severity logLevel);
    void logMessage(const std::string &message, Severity severity, Category category, bool flushLogger = false);
    std::vector<std::shared_ptr<LogData>> searchMessages(const std::string &searchString, size_t messageLimit = 0, Category categoryLimit = Category::NoCategory, Severity severityLimit = Severity::NoSeverity);
    void trimmOldestMessages(Severity maxSeverity);
    std::string currentLogFilePath();
    std::string baseLogFilePath();
    Severity logLevel();
    void logLevel(Severity logLevel);
    void setLogViewerUpdateCallback(std::function<void()> updateCallback);
    bool doTraceLevelLogging();
}
