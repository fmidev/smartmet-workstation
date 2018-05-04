#pragma once

#ifdef WIN32
#if !defined(_WIN32_WINNT) && !defined(_CHICAGO_)
#define _WIN32_WINNT 0x0600   // Specifies that the minimum required platform is Windows Vista.
#endif
#endif

#include <vector>

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>

#include "tcp_tools.h"

namespace boost
{
    namespace asio
    {
        class io_service;
    }
}




/** simple connection to server:
    - logs in just with username (no password)
    - all connections are initiated by the client
    - server disconnects any client that hasn't pinged for in certain timeout (depending if working or idleing)
    - there is only ONE connection to the server per process because this was intended to be used with multiple worker processes

    Possible requests:
    - ping: the server answers either with "ping ok" or "new_task"
    - work_result: the server answers with "ping ok" or "new_task"
*/
class worker_to_master_connection : public boost::enable_shared_from_this<worker_to_master_connection>
                  , boost::noncopyable 
{
    typedef worker_to_master_connection self_type;
    typedef boost::function<bool(const tcp_tools::task_structure &task_in, tcp_tools::work_result_structure &result_out, bool use_verbose_logging)> working_function_interface;

    worker_to_master_connection(const std::string & username, working_function_interface used_work_function, bool use_verbose_logging);
    void start(boost::asio::ip::tcp::endpoint ep);
public:
    typedef boost::system::error_code error_code;
    typedef boost::shared_ptr<worker_to_master_connection> connection_ptr;

    static void start_worker(const std::string & username, working_function_interface used_work_function, bool use_verbose_logging);
    static connection_ptr start(boost::asio::ip::tcp::endpoint ep, const std::string & username, working_function_interface used_work_function, bool use_verbose_logging);
    static boost::asio::io_service& worker_service();
    void stop();
    bool started() { return started_; }
private:
    void on_connect(const error_code & err);
    void on_read(const error_code & err, size_t bytes);
    
    void on_login();
    void on_ping(const std::string &msg);
    void on_new_task(const std::string & msg);

    void do_process_task(std::istream &in);

    void do_ping();

    void postpone_ping();

    void on_write(const error_code &err, size_t bytes);
    void do_read();
    void do_write(const std::string & msg);
    size_t read_complete(const boost::system::error_code & err, size_t bytes);

private:
    boost::asio::ip::tcp::socket sock_;
    std::vector<char> fixed_size_read_buffer_; // Asio:n kautta luetaan fiksatun kokoisissa paketeissa (ei saa muuttaa kokoa konstruktorin jälkeen)
    std::string complete_message_from_read; // kokonainen luettu viesti talletetaan tähän (tarvittaessa monessa osassa luettuna)
    bool transmission_incomplete_; // tämän avulla tiedetään onko saapuva lähetys luettu loppuun, vai pitääkö fixed_size_read_buffer_ lisätä välillä complete_message_from_read:iin
    std::vector<char> write_buffer_; // tämän kokoa muutetaan tarvittaessa, jos lähetettävä viesti olisi muuten liian iso
    bool started_;
    std::string username_;
    boost::asio::deadline_timer timer_;
    working_function_interface used_work_function_;
    bool use_verbose_logging_;
};
