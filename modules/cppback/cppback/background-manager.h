#ifndef BACKGROUND_MANAGER_SINGLETON_H
#define BACKGROUND_MANAGER_SINGLETON_H

#include "cppback/error.h"
#include "catlog/catlog.h"

#include <future>
#include <atomic>
#include <thread>
#include <mutex>
#include <type_traits>

namespace cppback
{
    namespace
    {
        template<typename LambdaWithNoArgs>
        auto spin(LambdaWithNoArgs&& func)
        {
            auto task = std::packaged_task<std::result_of_t<LambdaWithNoArgs()>()>{ std::forward<LambdaWithNoArgs>(func) };
            auto res = task.get_future();
            std::thread{ std::move(task) }.detach();
            return res;
        }

        void logException(const std::exception &e, const std::string &functionName)
        {
            // Have to log errors here, because otherwise exceptions are lost in std::function/future inner works...
            std::string errorMessage = "Error in ";
            errorMessage += functionName;
            errorMessage += ": ";
            errorMessage += e.what();
            CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::NetRequest, true);
        }
    }

    template <class...>
    using void_t = void;

    class AlreadyKilled : public std::runtime_error
    {
    public:
        AlreadyKilled(const std::string msg)
            : runtime_error(msg)
        {};
    };

    class BackgroundManager
    {
        std::atomic_uint running_ = 0;
        std::shared_future<void> shouldDie_;
        std::once_flag killOnce_;
        std::promise<void> killBackgroundTasks_;
        mutable std::atomic_bool forcedWakeUp_ = false;
    public:
        BackgroundManager()
        {
            shouldDie_ = killBackgroundTasks_.get_future().share();
        }

        template<typename LambdaWithNoArgs>
        decltype(auto) addTask(LambdaWithNoArgs&& func)
        {
            // Count the task before launching: isDead() must include pending starts.
            ++running_;
            try
            {
                return spin([this, func = std::move(func)]() mutable
                {
                    struct Completion
                    {
                        std::atomic_uint& count;
                        ~Completion() { --count; }
                    } completion{running_};
                    if(isKillSignalSet())
                        throw AlreadyKilled("Can't add task after kill signal is already set.");
                    return addTaskImpl(std::move(func));
                });
            }
            catch(...)
            {
                --running_;
                throw;
            }
        }

        template<typename LambdaWithNoArgs>
        auto addTaskImpl(LambdaWithNoArgs&& func)
            ->std::enable_if_t
            <
            !(std::is_same_v<std::result_of_t<LambdaWithNoArgs()>, void_t<>>),
            std::result_of_t<LambdaWithNoArgs()>
            >
        {
            std::result_of_t<LambdaWithNoArgs()> result;
            try
            {
                result = func();
            }
            catch(const std::exception& e)
            {
                logException(e, "BackgroundManager::addTaskImpl->result");
                throw;
            }
            return result;
        }

        template<typename LambdaWithNoArgs>
        auto addTaskImpl(LambdaWithNoArgs&& func)
            ->std::enable_if_t
            <
            (std::is_same_v<std::result_of_t<LambdaWithNoArgs()>, void_t<>>),
            void
            >
        {
            try
            {
                func();
            }
            catch(const std::exception& e)
            {
                logException(e, "BackgroundManager::addTaskImpl->void");
                throw;
            }
        }

        void kill()
        {
            std::call_once(killOnce_, [this]() { killBackgroundTasks_.set_value(); });
        }

        bool isKillSignalSet() const
        {
            using namespace std::literals;
            return isKillSignalSet(0ms);
        }

        bool isKillSignalSet(const std::chrono::milliseconds &waitTime) const
        {
            using namespace std::literals;
            return shouldDie_.wait_for(waitTime) == std::future_status::ready;
        }

        bool isDead(const std::chrono::milliseconds &waitTime) const
        {
            if(running_ == 0)
            {
                return true;
            }
            std::this_thread::sleep_for(waitTime);
            return running_ == 0;
        }

        void sleepInIntervals(const std::chrono::milliseconds &sleepDuration, const std::chrono::milliseconds &interval, const std::string& taskName = "") const
        {
            using namespace std::literals;
            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            auto durationSlept = 0ms;
            while(durationSlept < sleepDuration)
            {
                if(isKillSignalSet(interval))
                {
                    throw TaskStoppedByKillSignal{ taskName };
                }
                if(isForcedWakeUpTime())
                    return;
                durationSlept = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
            }
        }

        void doForcedWakeUp()
        {
            forcedWakeUp_ = true;
        }

        bool isForcedWakeUpTime() const
        {
            if(forcedWakeUp_)
            {
                forcedWakeUp_ = false;
                return true;
            }
            return false;
        }
    };
}
#endif
