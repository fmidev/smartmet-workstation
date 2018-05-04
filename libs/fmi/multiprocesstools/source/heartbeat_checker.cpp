#include "heartbeat_checker.h"
#include "work_queue.h"

heartbeat_checker::heartbeat_checker(int expire_in_ms)
:last_counter_value_(0)
,timer_()
,expire_duration_in_ms_(expire_in_ms)
{}

// 1. Pyytää running_info:lta counter:in arvon
// 2. Jos se on eri kuin last_counter_value_, nollataan timer_, asetetaan se uudeksi last_counter_value_:ksi ja palautetaan true
// 3. Jos se oli sama kuin last_counter_value_, palautetaan false;
bool heartbeat_checker::check_counter(worker_running_info &running_info)
{
    int current_counter = running_info.get_counter();
    if(current_counter != last_counter_value_)
    {
        last_counter_value_ = current_counter;
        reset_heartbeat();
        return true;
    }
    else
        return false;
}

// Jos timerin wall-clock osiossa on isompi kesto kuin määrätty expire-limit,
// on kulunut liikaa aikaa.
bool heartbeat_checker::is_heartbeat_expired() const
{
    boost::chrono::milliseconds elapsed_time_in_ms = elapsed_time_in_milliseconds();
    if(elapsed_time_in_ms > expire_duration_in_ms_)
        return true;
    else
        return false;
}

// Nollaa timerin tähän hetkeen
void heartbeat_checker::reset_heartbeat()
{
    timer_ = boost::timer::cpu_timer();
}

// Tämän hetkinen kulutettu aika sekunneissa
double heartbeat_checker::elapsed_time_in_seconds() const
{
    // duration<double> (tai float) tarkoittaa aina sekunteja
    boost::chrono::duration<double> elapsed_time_in_sec = elapsed_time_in_milliseconds();
    return elapsed_time_in_sec.count();
}

boost::chrono::milliseconds heartbeat_checker::elapsed_time_in_milliseconds() const
{
    return boost::chrono::duration_cast<boost::chrono::milliseconds>(boost::chrono::nanoseconds(timer_.elapsed().wall));
}
