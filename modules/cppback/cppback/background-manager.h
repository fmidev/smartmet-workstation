#ifndef BACKGROUND_MANAGER_SINGLETON_H
#define BACKGROUND_MANAGER_SINGLETON_H

#include "cppback/error.h"
#include "catlog/catlog.h"

#include <future>
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
        std::future<void> shouldDie_;
        std::promise<void> killBackgroundTasks_;
        mutable std::atomic_bool forcedWakeUp_ = false;
    public:
        BackgroundManager()
        {
            shouldDie_ = killBackgroundTasks_.get_future();
        }

        template<typename LambdaWithNoArgs>
        decltype(auto) addTask(LambdaWithNoArgs&& func)
        {
            return spin([&, func = std::move(func)]() mutable
            {
                if(isKillSignalSet())
                {
                    throw AlreadyKilled("Can't add task after kill signal is already set.");
                }
                return addTaskImpl(std::move(func));
            });
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
            ++running_;
            try
            {
                result = func();
            }
            catch(const std::exception& e)
            {
                logException(e, "BackgroundManager::addTaskImpl->result");
                --running_;
                throw e;
            }
            --running_;
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
            ++running_;
            try
            {
                func();
            }
            catch(const std::exception& e)
            {
                logException(e, "BackgroundManager::addTaskImpl->void");
                --running_;
                throw e;
            }
            --running_;
        }

        void kill()
        {
            if(isKillSignalSet())
            {
                return;
            }
            killBackgroundTasks_.set_value();
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
