#include "worker_to_master_connection.h"
#include "logging.h"

namespace 
{
    boost::asio::io_service g_service;
}

worker_to_master_connection::worker_to_master_connection(const std::string & username, working_function_interface used_work_function, bool use_verbose_logging) 
:sock_(g_service)
,fixed_size_read_buffer_(tcp_tools::g_fixed_size_read_buffer_size)
,complete_message_from_read()
,transmission_incomplete_(false)
,write_buffer_() // write bufferin kokoa s‰‰det‰‰n lennossa ajon aikana
,started_(true)
,username_(username)
,timer_(g_service) 
,used_work_function_(used_work_function)
,use_verbose_logging_(use_verbose_logging)
{
}

void worker_to_master_connection::start(boost::asio::ip::tcp::endpoint ep) 
{
    sock_.async_connect(ep, MEMBER_FUNC1(on_connect,_1));
}

void worker_to_master_connection::start_worker(const std::string & username, working_function_interface used_work_function, bool use_verbose_logging)
{
    boost::asio::ip::tcp::endpoint ep( boost::asio::ip::address::from_string(tcp_tools::g_local_ip_address), tcp_tools::g_local_ip_port);
    worker_to_master_connection::start(ep, username, used_work_function, use_verbose_logging);
}

worker_to_master_connection::connection_ptr worker_to_master_connection::start(boost::asio::ip::tcp::endpoint ep, const std::string & username, working_function_interface used_work_function, bool use_verbose_logging) 
{
    connection_ptr new_(new worker_to_master_connection(username, used_work_function, use_verbose_logging));
    new_->start(ep);
    return new_;
}

boost::asio::io_service& worker_to_master_connection::worker_service()
{
    return g_service;
}

void worker_to_master_connection::stop() 
{
    if ( !started_)
        return;
    log_message("stopping", logging::trivial::info);
    started_ = false;
    sock_.close();
}

void worker_to_master_connection::on_connect(const error_code & err) 
{
    if ( !err)
        do_write(std::string("login ") + username_ + "\n");
    else
        stop();
}

void worker_to_master_connection::on_read(const error_code & err, size_t bytes) 
{
    if ( err) 
    {
        log_message(std::string("Error in on_read category: ") + err.category().name() + ", message: " + err.message(), logging::trivial::error);
        stop();
    }
    if ( !started() ) 
        return;
    if(transmission_incomplete_)
    {
        complete_message_from_read += std::string(fixed_size_read_buffer_.data(), bytes);
        do_read(); // jatketaan sanoman lukua
    }
    else
    {
        // process the transmission message
        complete_message_from_read += std::string(fixed_size_read_buffer_.data(), bytes); // liitet‰‰n loput viestista kokonais-sanomaan
        std::string msg;
        msg.swap(complete_message_from_read); // t‰m‰ tyhjent‰‰ samalla complete_message_from_read -puskurin seuraavaa lukua varten

        if(tcp_tools::find_from_start_of_string(msg, "login ", tcp_tools::g_search_message_start_max_chars))
            on_login();
        else if(tcp_tools::find_from_start_of_string(msg, "ping", tcp_tools::g_search_message_start_max_chars))
            on_ping(msg);
        else if(tcp_tools::find_from_start_of_string(msg, tcp_tools::g_new_task_to_worker_string, tcp_tools::g_search_message_start_max_chars))
            on_new_task(msg);
        else
        {
            log_message(std::string("invalid msg: ") + msg, logging::trivial::error);
            do_ping();
        }
    }
}
    
void worker_to_master_connection::on_login() 
{
    log_message("Logged in", logging::trivial::info);
    do_ping();
}

void worker_to_master_connection::on_ping(const std::string & /* msg */ ) 
{
        postpone_ping();
}

void worker_to_master_connection::on_new_task(const std::string & msg) 
{
    std::istringstream in(msg);
    std::string answer;
    in >> answer; // luetaan 1. sana pois streamista
    do_process_task(in);
}

void worker_to_master_connection::do_process_task(std::istream &in)
{
    //const size_t critical_time_index_for_crash_tests = 5;
    //bool do_crash = false;

    tcp_tools::task_structure task;
    try
    {
        in >> task;
        std::string log_message_string = "Received new task";
        if(use_verbose_logging_)
            log_message_string += ": " + task.to_string();
        log_message(log_message_string, logging::trivial::debug);

        //if(critical_time_index_for_crash_tests == task.data_time_index_)
        //{
        //    do_crash = true;
        //    throw std::runtime_error("Crash and burn...");
        //}

        tcp_tools::work_result_structure result;
        used_work_function_(task, result, use_verbose_logging_);
        log_message_string = "Calculated result";
        if(use_verbose_logging_)
            log_message_string += ": " + result.to_string();
        log_message(log_message_string, logging::trivial::debug);

        if(tcp_tools::use_binary_transfer())
            do_write(result.to_binary_string(tcp_tools::g_result_from_worker_string + " ")); // Huom! binary stringin per‰‰n ei laiteta \n -merkki‰!
        else
        {
            std::ostringstream out;
            out << tcp_tools::g_result_from_worker_string << " " << result << "\n";
            do_write(out.str());
        }
        return ;
    }
    catch(std::exception &e)
    {
        log_message(std::string("Error in do_process_task: ") + e.what(), logging::trivial::error);
    }
    catch(...)
    {
        log_message("Unknown error in do_process_task", logging::trivial::error);
    }

    //if(!do_crash)
        do_write(tcp_tools::g_error_string + "\n");
}

void worker_to_master_connection::do_ping() 
{
    do_write("ping\n");
}

void worker_to_master_connection::postpone_ping() 
{
    static int counter = 0; // testej‰ varten countteri
    counter++;

    int millis = tcp_tools::g_postpone_ping_timeout_in_ms; // tietyin v‰liajoin kysell‰‰n pingill‰ onko teht‰vi‰
    //if(counter == 5)
    //    millis = 7000;
    //log_message(std::string("Postponing ping ") + boost::lexical_cast<std::string>(millis) + " millis", logging::trivial::trace);
    timer_.expires_from_now(boost::posix_time::millisec(millis));
    timer_.async_wait( MEMBER_FUNC(do_ping));
}

void worker_to_master_connection::on_write(const error_code & /* err */ , size_t /* bytes */ ) 
{
    do_read();
}

void worker_to_master_connection::do_read() 
{
    async_read(sock_, boost::asio::buffer(fixed_size_read_buffer_), 
        MEMBER_FUNC2(read_complete,_1,_2), MEMBER_FUNC2(on_read,_1,_2));
}

void worker_to_master_connection::do_write(const std::string & msg) 
{
    if ( !started() ) 
        return;
    if(write_buffer_.size() < msg.size())
        write_buffer_.resize(msg.size()); // kasvatetaan write bufferin kokoa tarvittaessa

    tcp_tools::trace_log_start_of_container("do_write sends following message: ", use_verbose_logging_, msg);

    std::copy(msg.begin(), msg.end(), write_buffer_.begin());
    sock_.async_write_some( boost::asio::buffer(write_buffer_, msg.size()), 
        MEMBER_FUNC2(on_write,_1,_2));
}

size_t worker_to_master_connection::read_complete(const boost::system::error_code & err, size_t bytes) 
{
    return tcp_tools::read_complete_check_one_char_at_time(err, bytes, fixed_size_read_buffer_, transmission_incomplete_);
}

