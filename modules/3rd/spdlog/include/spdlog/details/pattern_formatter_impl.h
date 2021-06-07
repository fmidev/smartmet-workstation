//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "spdlog/formatter.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/details/os.h"
#include "fmt/format.h"

#include <chrono>
#include <ctime>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <array>

namespace
{
    void append_to_buffer(fmt::memory_buffer& formatted, const std::string& str)
    {
        formatted.append(str.data(), str.data() + str.size());
    }
}

namespace spdlog
{
namespace details
{
class flag_formatter
{
public:
    virtual ~flag_formatter()
    {}
    virtual void format(details::log_msg& msg, const std::tm& tm_time) = 0;
};

///////////////////////////////////////////////////////////////////////
// name & level pattern appenders
///////////////////////////////////////////////////////////////////////
namespace
{
class name_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm&) override
    {
        ::append_to_buffer(msg.formatted, *msg.logger_name);
    }
};
}

// log level appender
class level_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm&) override
    {
        ::append_to_buffer(msg.formatted, level::to_str(msg.level));
    }
};

// short log level appender
class short_level_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm&) override
    {
        ::append_to_buffer(msg.formatted, level::to_short_str(msg.level));
    }
};

///////////////////////////////////////////////////////////////////////
// Date time pattern appenders
///////////////////////////////////////////////////////////////////////

static const char* ampm(const tm& t)
{
    return t.tm_hour >= 12 ? "PM" : "AM";
}

static int to12h(const tm& t)
{
    return t.tm_hour > 12 ? t.tm_hour - 12 : t.tm_hour;
}

//Abbreviated weekday name
using days_array = std::array<std::string, 7>;
static const days_array& days()
{
    static const days_array arr{ { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" } };
    return arr;
}
class a_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        ::append_to_buffer(msg.formatted, days()[tm_time.tm_wday]);
    }
};
// message counter formatter
class i_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm&) override
    {
        fmt::format_to(msg.formatted, "#{}", msg.msg_id);
    }
};
//Full weekday name
static const days_array& full_days()
{
    static const days_array arr{ { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" } };
    return arr;
}
class A_formatter SPDLOG_FINAL :public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        ::append_to_buffer(msg.formatted, full_days()[tm_time.tm_wday]);
    }
};

//Abbreviated month
using months_array = std::array<std::string, 12>;
static const months_array& months()
{
    static const months_array arr{ { "Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sept", "Oct", "Nov", "Dec" } };
    return arr;
}
class b_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        ::append_to_buffer(msg.formatted, months()[tm_time.tm_mon]);
    }
};

//Full month name
static const months_array& full_months()
{
    static const months_array arr{ { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" } };
    return arr;
}
class B_formatter SPDLOG_FINAL :public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        ::append_to_buffer(msg.formatted, full_months()[tm_time.tm_mon]);
    }
};


//write 2 ints seperated by sep with padding of 2
static fmt::memory_buffer& pad_n_join(fmt::memory_buffer& w, int v1, int v2, char sep)
{
    auto str = fmt::format("{:02d}{}{:02d}", v1, sep, v2);
    ::append_to_buffer(w, str);
    return w;
}

//write 3 ints seperated by sep with padding of 2
static fmt::memory_buffer& pad_n_join(fmt::memory_buffer& w, int v1, int v2, int v3, char sep)
{
    auto str = fmt::format("{:02d}{}{:02d}{}{:02d}", v1, sep, v2, sep, v3);
    ::append_to_buffer(w, str);
    return w;
}


//Date and time representation (Thu Aug 23 15:35:46 2014)
class c_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        auto str = fmt::format("{} {} {}", days()[tm_time.tm_wday], months()[tm_time.tm_mon], tm_time.tm_mday);
        ::append_to_buffer(msg.formatted, str);
        pad_n_join(msg.formatted, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, ':');
        auto str2 = fmt::format(" {}", tm_time.tm_year + 1900);
        ::append_to_buffer(msg.formatted, str2);
    }
};


// year - 2 digit
class C_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        auto str = fmt::format("{:02d}", tm_time.tm_year % 100);
        ::append_to_buffer(msg.formatted, str);
    }
};



// Short MM/DD/YY date, equivalent to %m/%d/%y 08/23/01
class D_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        pad_n_join(msg.formatted, tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_year % 100, '/');
    }
};


// year - 4 digit
class Y_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        auto str = fmt::format("{}", tm_time.tm_year + 1900);
        ::append_to_buffer(msg.formatted, str);
    }
};

// month 1-12
class m_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        auto str = fmt::format("{:02d}", tm_time.tm_mon + 1);
        ::append_to_buffer(msg.formatted, str);
    }
};

// day of month 1-31
class d_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        auto str = fmt::format("{:02d}", tm_time.tm_mday);
        ::append_to_buffer(msg.formatted, str);
    }
};

// hours in 24 format  0-23
class H_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        auto str = fmt::format("{:02d}", tm_time.tm_hour);
        ::append_to_buffer(msg.formatted, str);
    }
};

// hours in 12 format  1-12
class I_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        auto str = fmt::format("{:02d}", to12h(tm_time));
        ::append_to_buffer(msg.formatted, str);
    }
};

// minutes 0-59
class M_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        auto str = fmt::format("{:02d}", tm_time.tm_min);
        ::append_to_buffer(msg.formatted, str);
    }
};

// seconds 0-59
class S_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        auto str = fmt::format("{:02d}", tm_time.tm_sec);
        ::append_to_buffer(msg.formatted, str);
    }
};

// milliseconds
class e_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm&) override
    {
        auto duration = msg.time.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;
        auto str = fmt::format("{:03d}", static_cast<int>(millis));
        ::append_to_buffer(msg.formatted, str);
    }
};

// microseconds
class f_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm&) override
    {
        auto duration = msg.time.time_since_epoch();
        auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % 1000000;
        auto str = fmt::format("{:06d}", static_cast<int>(micros));
        ::append_to_buffer(msg.formatted, str);
    }
};

// nanoseconds
class F_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm&) override
    {
        auto duration = msg.time.time_since_epoch();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() % 1000000000;
        auto str = fmt::format("{:09d}", static_cast<int>(ns));
        ::append_to_buffer(msg.formatted, str);
    }
};

// AM/PM
class p_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        ::append_to_buffer(msg.formatted, ampm(tm_time));
    }
};


// 12 hour clock 02:55:02 pm
class r_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        pad_n_join(msg.formatted, to12h(tm_time), tm_time.tm_min, tm_time.tm_sec, ':');
        ::append_to_buffer(msg.formatted, ampm(tm_time));
    }
};

// 24-hour HH:MM time, equivalent to %H:%M
class R_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        pad_n_join(msg.formatted, tm_time.tm_hour, tm_time.tm_min, ':');
    }
};

// ISO 8601 time format (HH:MM:SS), equivalent to %H:%M:%S
class T_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        pad_n_join(msg.formatted, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, ':');
    }
};

// ISO 8601 offset from UTC in timezone (+-HH:MM)
class z_formatter SPDLOG_FINAL:public flag_formatter
{
public:
    const std::chrono::seconds cache_refresh = std::chrono::seconds(5);

    z_formatter():_last_update(std::chrono::seconds(0)), _offset_minutes(0)
    {}
    z_formatter(const z_formatter&) = delete;
    z_formatter& operator=(const z_formatter&) = delete;

    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
#ifdef _WIN32
        int total_minutes = get_cached_offset(msg, tm_time);
#else
        // No need to chache under gcc,
        // it is very fast (already stored in tm.tm_gmtoff)
        int total_minutes = os::utc_minutes_offset(tm_time);
#endif
        bool is_negative = total_minutes < 0;
        char sign;
        if (is_negative)
        {
            total_minutes = -total_minutes;
            sign = '-';
        }
        else
        {
            sign = '+';
        }

        int h = total_minutes / 60;
        int m = total_minutes % 60;
        auto str = fmt::format("{}", sign);
        ::append_to_buffer(msg.formatted, str);
        pad_n_join(msg.formatted, h, m, ':');
    }
private:
    log_clock::time_point _last_update;
    int _offset_minutes;
    std::mutex _mutex;

    int get_cached_offset(const log_msg& msg, const std::tm& tm_time)
    {
        using namespace std::chrono;
        std::lock_guard<std::mutex> l(_mutex);
        if (msg.time - _last_update >= cache_refresh)
        {
            _offset_minutes = os::utc_minutes_offset(tm_time);
            _last_update = msg.time;
        }
        return _offset_minutes;
    }
};



// Thread id
class t_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm&) override
    {
        auto str = fmt::format("{}", msg.thread_id);
        ::append_to_buffer(msg.formatted, str);
    }
};

// Current pid
class pid_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm&) override
    {
        auto str = fmt::format("{}", details::os::pid());
        ::append_to_buffer(msg.formatted, str);
    }
};

class v_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm&) override
    {
        std::string str(msg.raw.data(), msg.raw.size());
        ::append_to_buffer(msg.formatted, str);
    }
};

class ch_formatter SPDLOG_FINAL:public flag_formatter
{
public:
    explicit ch_formatter(char ch): _ch(ch)
    {}
    void format(details::log_msg& msg, const std::tm&) override
    {
        std::string str(1, _ch);
        ::append_to_buffer(msg.formatted, str);
    }
private:
    char _ch;
};


//aggregate user chars to display as is
class aggregate_formatter SPDLOG_FINAL:public flag_formatter
{
public:
    aggregate_formatter()
    {}
    void add_ch(char ch)
    {
        _str += ch;
    }
    void format(details::log_msg& msg, const std::tm&) override
    {
        ::append_to_buffer(msg.formatted, _str);
    }
private:
    std::string _str;
};

// Full info formatter
// pattern: [%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v
class full_formatter SPDLOG_FINAL:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
#ifndef SPDLOG_NO_DATETIME
        auto duration = msg.time.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;

        /* Slower version(while still very fast - about 3.2 million lines/sec under 10 threads),
        msg.formatted.write("[{:d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d}] [{}] [{}] {} ",
        tm_time.tm_year + 1900,
        tm_time.tm_mon + 1,
        tm_time.tm_mday,
        tm_time.tm_hour,
        tm_time.tm_min,
        tm_time.tm_sec,
        static_cast<int>(millis),
        msg.logger_name,
        level::to_str(msg.level),
        msg.raw.str());*/


        // Faster (albeit uglier) way to format the line (5.6 million lines/sec under 10 threads)

        auto str = fmt::format("[{}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d}]", 
            static_cast<unsigned int>(tm_time.tm_year + 1900), 
            static_cast<unsigned int>(tm_time.tm_mon + 1),
            static_cast<unsigned int>(tm_time.tm_mday),
            static_cast<unsigned int>(tm_time.tm_hour),
            static_cast<unsigned int>(tm_time.tm_min),
            static_cast<unsigned int>(tm_time.tm_sec),
            static_cast<unsigned int>(millis));
        ::append_to_buffer(msg.formatted, str);

        //no datetime needed
#else
        (void)tm_time;
#endif

#ifndef SPDLOG_NO_NAME
        str = fmt::format("[{}]", *msg.logger_name);
        ::append_to_buffer(msg.formatted, str);
#endif

        str = fmt::format("[{}]", level::to_str(msg.level));
        ::append_to_buffer(msg.formatted, str);
        std::string str2(msg.raw.data(), msg.raw.size());
        ::append_to_buffer(msg.formatted, str2);
    }
};



}
}
///////////////////////////////////////////////////////////////////////////////
// pattern_formatter inline impl
///////////////////////////////////////////////////////////////////////////////
inline spdlog::pattern_formatter::pattern_formatter(const std::string& pattern, pattern_time_type pattern_time)
    : _pattern_time(pattern_time)
{
    compile_pattern(pattern);
}

inline void spdlog::pattern_formatter::compile_pattern(const std::string& pattern)
{
    auto end = pattern.end();
    std::unique_ptr<details::aggregate_formatter> user_chars;
    for (auto it = pattern.begin(); it != end; ++it)
    {
        if (*it == '%')
        {
            if (user_chars) //append user chars found so far
                _formatters.push_back(std::move(user_chars));

            if (++it != end)
                handle_flag(*it);
            else
                break;
        }
        else // chars not following the % sign should be displayed as is
        {
            if (!user_chars)
                user_chars = std::unique_ptr<details::aggregate_formatter>(new details::aggregate_formatter());
            user_chars->add_ch(*it);
        }
    }
    if (user_chars) //append raw chars found so far
    {
        _formatters.push_back(std::move(user_chars));
    }

}
inline void spdlog::pattern_formatter::handle_flag(char flag)
{
    switch (flag)
    {
    // logger name
    case 'n':
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::name_formatter()));
        break;

    case 'l':
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::level_formatter()));
        break;

    case 'L':
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::short_level_formatter()));
        break;

    case('t'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::t_formatter()));
        break;

    case('v'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::v_formatter()));
        break;

    case('a'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::a_formatter()));
        break;

    case('A'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::A_formatter()));
        break;

    case('b'):
    case('h'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::b_formatter()));
        break;

    case('B'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::B_formatter()));
        break;
    case('c'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::c_formatter()));
        break;

    case('C'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::C_formatter()));
        break;

    case('Y'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::Y_formatter()));
        break;

    case('D'):
    case('x'):

        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::D_formatter()));
        break;

    case('m'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::m_formatter()));
        break;

    case('d'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::d_formatter()));
        break;

    case('H'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::H_formatter()));
        break;

    case('I'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::I_formatter()));
        break;

    case('M'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::M_formatter()));
        break;

    case('S'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::S_formatter()));
        break;

    case('e'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::e_formatter()));
        break;

    case('f'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::f_formatter()));
        break;
    case('F'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::F_formatter()));
        break;

    case('p'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::p_formatter()));
        break;

    case('r'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::r_formatter()));
        break;

    case('R'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::R_formatter()));
        break;

    case('T'):
    case('X'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::T_formatter()));
        break;

    case('z'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::z_formatter()));
        break;

    case ('+'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::full_formatter()));
        break;

    case ('P'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::pid_formatter()));
        break;

#if defined(SPDLOG_ENABLE_MESSAGE_COUNTER)
    case ('i'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::i_formatter()));
        break;
#endif

    default: //Unknown flag appears as is
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::ch_formatter('%')));
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::ch_formatter(flag)));
        break;
    }
}

inline std::tm spdlog::pattern_formatter::get_time(details::log_msg& msg)
{
    if (_pattern_time == pattern_time_type::local)
        return details::os::localtime(log_clock::to_time_t(msg.time));
    else
        return details::os::gmtime(log_clock::to_time_t(msg.time));
}

inline void spdlog::pattern_formatter::format(details::log_msg& msg)
{

#ifndef SPDLOG_NO_DATETIME
    auto tm_time = get_time(msg);
#else
    std::tm tm_time;
#endif
    for (auto &f : _formatters)
    {
        f->format(msg, tm_time);
    }
    //write eol
    std::string str(details::os::eol, details::os::eol_size);
    ::append_to_buffer(msg.formatted, str);
}
