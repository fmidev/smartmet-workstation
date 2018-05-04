#include "catlog/catlog.h"
#include "spdlog/spdlog.h"

#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/filesystem.hpp>

#include <vector>
#include <mutex>

using namespace std;
using namespace CatLog;

namespace
{
    std::once_flag initLoggerFlag_;
    std::shared_ptr<spdlog::logger> logger_;
    std::vector<std::shared_ptr<LogData>> loggedMessages_;
    // In certain intervals loggedMessages_ container is cleared from the oldest messages, if it exceeds maximumMessagesKept_ limit
    size_t maximumMessagesKept_;
    mutex messageMutex_;
    // full path + base log filename, it's used to calculate currently used log file with dates
    std::string baseLogFilePath_;
    // If this callback function is set, it's is called after every log purpose to update view which is showing the log messages to user
    std::function<void()> logViewerUpdateCallback_;

    string categoryToString(Category category)
    {
        switch(category)
        {
        case Category::Configuration:
            return "Conf";
        case Category::Data:
            return "Data";
        case Category::Editing:
            return "Edit";
        case Category::Visualization:
            return "Vis";
        case Category::Macro:
            return "Macro";
        case Category::Operational:
            return "Oper";
        case Category::NetRequest:
            return "Net";
        default:
            return "NoCat";
        }
    }

    string makeFinalLogMessage(Category category, const std::string &message)
    {
        return ::categoryToString(category) + ": " + message;
    }

    // There is no need for mutex and locks here because logger_ object itself is thread safe
    void logMessageToFile(Category category, Severity severity, const std::string &message, bool flushLogger)
    {
        // If logger is initialized, we can do file logging
        if(logger_)
        {
            string finalLogMessage = ::makeFinalLogMessage(category, message);
            switch(severity)
            {
            case Severity::Trace:
                logger_->trace(finalLogMessage);
                break;
            case Severity::Debug:
                logger_->debug(finalLogMessage);
                break;
            case Severity::Info:
                logger_->info(finalLogMessage);
                break;
            case Severity::Warning:
                logger_->warn(finalLogMessage);
                break;
            case Severity::Error:
                logger_->error(finalLogMessage);
                break;
            case Severity::Critical:
                logger_->critical(finalLogMessage);
                break;
            default:
                break;
            }

            if(flushLogger)
                logger_->flush();
        }
    }

    void logPiledMessagesToFile()
    {
        if(logger_)
        {
            lock_guard<mutex> lock(messageMutex_);
            for(const auto &logData : loggedMessages_)
            {
                ::logMessageToFile(logData->category_, logData->severity_, logData->message_, false);
            }
            logger_->flush();
        }
    }

    bool containsStringCaseInsensitive(const string &searchThis, const string &findThis)
    {
        auto findRange = boost::algorithm::ifind_first(searchThis, findThis);
        return !findRange.empty();
    }

    bool isExcludeWord(const string &word)
    {
        return word.size() && word[0] == '-';
    }

    bool containsAllSearchedWordsCaseInsensitive(const string &searchThis, const std::vector<std::string> &searchedWords)
    {
        for(const auto &word : searchedWords)
        {
            if(::isExcludeWord(word))
            {
                // Presumes that all word's given here are longer than 1 character
                std::string excludeWord(word.begin() + 1, word.end());
                if(::containsStringCaseInsensitive(searchThis, excludeWord))
                    return false;
            }
            else
            {
                if(!::containsStringCaseInsensitive(searchThis, word))
                    return false;
            }
        }
        return true;
    }

    bool isMessageMatch(const std::shared_ptr<LogData> &logData, const std::vector<std::string> & searchedWords, Category categoryLimit, Severity severityLimit)
    {
        if(logData)
        {
            if(categoryLimit == logData->category_ || categoryLimit == Category::NoCategory)
            {
                if(logData->severity_ >= severityLimit || severityLimit == Severity::NoSeverity)
                {
                    if(searchedWords.empty() || ::containsAllSearchedWordsCaseInsensitive(logData->message_, searchedWords))
                        return true;
                }
            }
        }
        return false;
    }

    void createDirectoryFromFilePath(const std::string &filePath)
    {
        boost::filesystem::path p(filePath);
        boost::filesystem::path fileDirectory = p.parent_path();
        if(!boost::filesystem::exists(fileDirectory))
        {
            boost::filesystem::create_directories(fileDirectory);
        }
    }

    // At the end of initLogger_impl function, all the messages logged so far are logged to log file.
    void initLogger_impl(const std::string &filePath, size_t maximumMessagesKept, Severity logLevel)
    {
        baseLogFilePath_ = filePath;
        maximumMessagesKept_ = maximumMessagesKept;
        // spdlog user must make sure that destination directory exists, spdlog insist that.
        ::createDirectoryFromFilePath(filePath);
        logger_ = spdlog::daily_logger_mt("daily_logger", filePath);
        logger_->flush_on(spdlog::level::warn);
        logger_->set_level(static_cast<spdlog::level::level_enum>(logLevel));
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e]'%l' %v");

        // Have to log those messages which are not yet logged to the file
        ::logPiledMessagesToFile();
    }

    std::vector<std::string> getSearchedWords(const std::string &searchString)
    {
        std::vector<std::string> words;
        // This will generate, in Debug configuration, about 100 lines of compiler warnings (boost not using secure functions blaa blaa). 
        // Couldn't disable those in any other way but to disable that warning from catlog library's CMakeLists.txt with add_compile_options with /wd4996
        boost::split(words, searchString, boost::is_any_of("\t "));
        // E.g. empty string generates vector with 1 empty element, have to remove all empty values from vector
        words.erase(std::remove_if(words.begin(), words.end(), [](const auto &word) {return word.empty(); }), words.end());
        // Minus character at start of word means excluded word, if user has started to write excluded word with '-' character, ignore it 
        // And also igone word if there is only single character after - sign like "-a", so that not all the messages that have 'a' would be ignored. 
        words.erase(std::remove_if(words.begin(), words.end(), [](const auto &word) {return word == "-"; }), words.end());
        words.erase(std::remove_if(words.begin(), words.end(), [](const auto &word) {return word.size() == 2 && word[0] == '-'; }), words.end());

        return words;
    }


    // Limited number of returned messages is tricky. You have to go through messages in reverse order
    // and then put them in returned vector in reverse order. Because loggedMessages_ contains messages in oldest to newest arder 
    // and with limited returns we want to return only the newest messages, but they must be returned in oldest to newest order.
    std::vector<std::shared_ptr<LogData>> limitedMessageSearch(const std::string &searchString, size_t messageLimit, Category categoryLimit, Severity severityLimit)
    {
        std::vector<std::shared_ptr<LogData>> foundMessages;
        {
            auto searchedWords = ::getSearchedWords(searchString);
            lock_guard<mutex> lock(messageMutex_);
            // This little trick makes range for loop go in reverse order
            for(const auto &logData : boost::adaptors::reverse(loggedMessages_))
            {
                if(::isMessageMatch(logData, searchedWords, categoryLimit, severityLimit))
                {
                    foundMessages.push_back(logData);
                    if(foundMessages.size() >= messageLimit)
                        break;
                }
            }
        } // mutex is released here
        // let's return reversed vector
        return std::vector<std::shared_ptr<LogData>>(foundMessages.crbegin(), foundMessages.crend());
    }

    std::vector<std::shared_ptr<LogData>> unlimitedMessageSearch(const std::string &searchString, Category categoryLimit, Severity severityLimit)
    {
        std::vector<std::shared_ptr<LogData>> foundMessages;
        auto searchedWords = ::getSearchedWords(searchString);
        lock_guard<mutex> lock(messageMutex_);
        for(const auto &logData : loggedMessages_)
        {
            if(::isMessageMatch(logData, searchedWords, categoryLimit, severityLimit))
            {
                foundMessages.push_back(logData);
            }
        }
        return foundMessages;
    }

    class LogDataTrimmer
    {
    private:
        // How many messages has been already removed
        size_t removedCounter_ = 0;
        // How many meassages user want's to be removed
        size_t removeCount_;
        // What is the maximum Severity level that user want's to be removed
        CatLog::Severity maxSeverity_;
    public:
        LogDataTrimmer(size_t removeCount, CatLog::Severity maxSeverity)
            :removeCount_(removeCount)
            ,maxSeverity_(maxSeverity)
        {
        }

        bool operator()(const std::shared_ptr<LogData> &logData)
        {
            if(removedCounter_ < removeCount_)
            {
                if(logData->severity_ <= maxSeverity_)
                {
                    removedCounter_++;
                    return true;
                }
            }
            return false;
        }
    };
}

namespace CatLog
{
    LogData::LogData(Category category, Severity severity, const std::string &message, const std::chrono::system_clock::time_point &logTime)
            :category_(category)
            , severity_(severity)
            , message_(message)
            , logTime_(logTime)
        {}

    // Init implementation called only once
    void initLogger(const std::string &filePath, size_t maximumMessagesKept, Severity logLevel)
    {
        std::call_once(initLoggerFlag_, ::initLogger_impl, filePath, maximumMessagesKept, logLevel);
    }

    void logMessage(const std::string &message, Severity severity, Category category, bool flushLogger)
    {
        if(severity >= logLevel())
        {
            // Take time-point before trying logging to file
            std::chrono::system_clock::time_point logTime = std::chrono::system_clock::now();
            // No need for lock-guards when logging to file, that system is made thread safe otherwise
            ::logMessageToFile(category, severity, message, flushLogger);

            {
                lock_guard<mutex> lock(messageMutex_);
                loggedMessages_.emplace_back(make_shared<LogData>(category, severity, message, logTime));
            }
            if(logViewerUpdateCallback_)
                logViewerUpdateCallback_();
        }
    }

    // messageLimit is max number messages included to returned vector. If it's value is 0 (default there is no limit)
    std::vector<std::shared_ptr<LogData>> searchMessages(const std::string &searchString, size_t messageLimit, Category categoryLimit, Severity severityLimit)
    {
        if(messageLimit)
            return ::limitedMessageSearch(searchString, messageLimit, categoryLimit, severityLimit);
        else
            return ::unlimitedMessageSearch(searchString, categoryLimit, severityLimit);
    }

    // Trimming away oldest messages if the container is exceeding the limits.
    // Trimmed messages must be at level of maxSeverity or lower to be trimmed.
    void trimmOldestMessages(Severity maxSeverity)
    {
        auto messageCount = loggedMessages_.size();
        if(messageCount > maximumMessagesKept_)
        {
            lock_guard<mutex> lock(messageMutex_);
            auto trimmedMessagecount = messageCount - maximumMessagesKept_;
            loggedMessages_.erase(std::remove_if(loggedMessages_.begin(), loggedMessages_.end(), LogDataTrimmer(trimmedMessagecount, maxSeverity)), loggedMessages_.end());
            int x = 0;
        }
    }

    std::string currentLogFilePath()
    {
        return logger_->current_filename();
    }

    std::string baseLogFilePath()
    {
        return baseLogFilePath_;
    }

    Severity logLevel()
    {
        // Severity enum has the same values as does spdlog's log-level enum so all you need is static_cast
        if(logger_)
            return static_cast<Severity>(logger_->level());
        else
            return Severity::Debug; // when logger is not yet initialized (before configurations are read), using default level: Debug
    }

    void logLevel(Severity logLevel)
    {
        // Severity enum has the same values as does spdlog's log-level enum so all you need is static_cast
        logger_->set_level(static_cast<spdlog::level::level_enum>(logLevel));
    }

    void setLogViewerUpdateCallback(std::function<void()> updateCallback)
    {
        logViewerUpdateCallback_ = updateCallback;
    }

    bool doTraceLevelLogging()
    {
        return logLevel() == Severity::Trace;
    }
}

