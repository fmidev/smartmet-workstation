#pragma once

#include <boost/timer/timer.hpp>
#include <boost/chrono.hpp>

class worker_running_info;

class heartbeat_checker
{
public:
    heartbeat_checker(int expire_in_ms);

    bool check_counter(worker_running_info &running_info);
    bool is_heartbeat_expired() const;
    void reset_heartbeat();
    double elapsed_time_in_seconds() const;
    boost::chrono::milliseconds elapsed_time_in_milliseconds() const;
private:
    int last_counter_value_;
    boost::timer::cpu_timer timer_;
    boost::chrono::milliseconds expire_duration_in_ms_;
};
