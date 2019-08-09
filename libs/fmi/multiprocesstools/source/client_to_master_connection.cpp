#include "client_to_master_connection.h"
#include "logging.h"

namespace 
{
    boost::asio::io_service g_client_service;

    // Kaikille client yhteyksille on yhteyset globaalit tyˆ- ja tulos jonot (yleens‰ on kai kuitenkin vain yksi client olemassa)
    tcp_tools::concurrent_queue<tcp_tools::task_structure> g_task_queue;
    tcp_tools::concurrent_queue<tcp_tools::work_result_structure> g_result_queue;
    tcp_tools::concurrent_queue<std::string> g_work_binary_result_queue; // t‰h‰n talletetaan purkamatta binary sanoma, joka saatiin workerilt‰, ett‰ sevoidaan laittaa sellaisenaan clientille

    // Clientilla on tarkoitus olla vain yksi ja ainoa yhteys serveriin kerrallaan, t‰h‰n on talletettu sen pointteri.
    client_to_master_connection::connection_ptr g_currently_used_connection_ptr;
}

client_to_master_connection::client_to_master_connection(const std::string & username, bool use_verbose_logging) 
:sock_(g_client_service)
,fixed_size_read_buffer_(tcp_tools::g_fixed_size_read_buffer_size)
,complete_message_from_read_()
,transmission_incomplete_(false)
,write_buffer_() // write bufferin kokoa s‰‰det‰‰n lennossa ajon aikana
,started_(true)
,username_(username)
,timer_(g_client_service) 
,use_verbose_logging_(use_verbose_logging)
,stopped_(false)
,binary_read_buffer_size_(0)
,binary_read_buffer_()
{
    binary_reading_state_ = tcp_tools::get_binary_reading_reset_state(); // asetetaan tila seuraavaa sanoman lukua varten kohdalleen 1. kerran konstruktorissa
}

void client_to_master_connection::start(boost::asio::ip::tcp::endpoint ep) 
{
    sock_.async_connect(ep, MEMBER_FUNC1(on_connect,_1));
}

void client_to_master_connection::start_client(const std::string & username, bool use_verbose_logging)
{
    boost::asio::ip::tcp::endpoint ep( boost::asio::ip::address::from_string(tcp_tools::g_local_ip_address), tcp_tools::g_local_ip_port);
    g_currently_used_connection_ptr = client_to_master_connection::start(ep, username, use_verbose_logging);
}

client_to_master_connection::connection_ptr client_to_master_connection::start(boost::asio::ip::tcp::endpoint ep, const std::string & username, bool use_verbose_logging) 
{
    connection_ptr new_(new client_to_master_connection(username, use_verbose_logging));
    new_->start(ep);
    return new_;
}

boost::asio::io_service& client_to_master_connection::client_service()
{
    return g_client_service;
}

tcp_tools::concurrent_queue<tcp_tools::task_structure>& client_to_master_connection::task_queue()
{
    return g_task_queue;
}

tcp_tools::concurrent_queue<tcp_tools::work_result_structure>& client_to_master_connection::result_queue()
{
    return g_result_queue;
}

void client_to_master_connection::stop() 
{
    stopped_ = true;
    g_currently_used_connection_ptr.reset();
    if ( !started_)
        return;
    log_message("stopping", logging::trivial::info);
    started_ = false;
    sock_.close();
}

client_to_master_connection::connection_ptr client_to_master_connection::currently_used_connection()
{
    return g_currently_used_connection_ptr;
}

void client_to_master_connection::on_connect(const error_code & err) 
{
    if ( !err)
        do_write("login " + username_ + "\n");
    else
        stop();
}

void client_to_master_connection::on_read(const error_code & err, size_t bytes) 
{
    if ( err) 
    {
        log_message(std::string("Error in on_read category: ") + err.category().name() + ", message: " + err.message(), logging::trivial::error);
        stop();
    }
    if ( !started() ) 
        return;

    tcp_tools::boost_asio_async_read_behaviour_change_fix(bytes, binary_read_buffer_, &binary_reading_state_);

    if(binary_reading_state_ == tcp_tools::binary_read_state_found)
    {
        if(use_verbose_logging_)
            log_message("Binary transmission starts from server", logging::trivial::trace);

        binary_read_buffer_.resize(binary_read_buffer_size_);
        do_read(); // jatketaan binary sanoman lukua
    }
    else if(transmission_incomplete_)
    {
        complete_message_from_read_ += std::string(fixed_size_read_buffer_.data(), bytes);
        do_read(); // jatketaan sanoman lukua
    }
    else
    {
        // process the transmission message
        if(binary_reading_state_ == tcp_tools::binary_read_state_finished)
            complete_message_from_read_ += std::string(binary_read_buffer_.data(), bytes); // liitet‰‰n binary luku puskuri kokonais-sanomaan
        else
            complete_message_from_read_ += std::string(fixed_size_read_buffer_.data(), bytes); // liitet‰‰n loput viestista kokonais-sanomaan
        std::string msg;
        msg.swap(complete_message_from_read_); // t‰m‰ tyhjent‰‰ samalla complete_message_from_read -puskurin seuraavaa lukua varten

        binary_reading_state_ = tcp_tools::get_binary_reading_reset_state(); // resetoidan binary luku  tila seuraavaa sanoman lukua varten

        if(tcp_tools::find_from_start_of_string(msg, "ping", tcp_tools::g_search_message_start_max_chars))
            on_ping(msg); // ping sanomia ei lokiteta koskaan
        else
        {
            if(use_verbose_logging_)
                tcp_tools::log_start_of_binary_message(std::string("on_read from server: "), msg, 30, logging::trivial::trace);

            if(tcp_tools::find_from_start_of_string(msg, "login ", tcp_tools::g_search_message_start_max_chars))
                on_login();
            else if(tcp_tools::find_from_start_of_string(msg, tcp_tools::g_result_from_worker_string, tcp_tools::g_search_message_start_max_chars))
                on_new_results(msg);
            else if(tcp_tools::find_from_start_of_string(msg, tcp_tools::g_new_results_to_client_string, tcp_tools::g_search_message_start_max_chars))
                on_new_results(msg);
            else
            {
                log_message(std::string("invalid msg, continuing: ") + msg, logging::trivial::error);
                on_ping(msg);
            }
        }
    }
}
    
void client_to_master_connection::on_login() 
{
    log_message("Logged in", logging::trivial::info);
    do_next_tasks();
}

void client_to_master_connection::on_ping(const std::string & /* msg */ ) 
{
    //if(use_verbose_logging_)
    //    log_message("on_ping -function", logging::trivial::trace);
    do_next_tasks();
}

void client_to_master_connection::do_next_tasks()
{
    //if(use_verbose_logging_)
    //    log_message("do_next_tasks -function", logging::trivial::trace);

    tcp_tools::concurrent_queue<tcp_tools::task_structure>& tasks = task_queue();
    if(tasks.size())
        do_send_next_tasks();
    else
        postpone_ping();
}

void client_to_master_connection::do_send_next_tasks()
{
    if(use_verbose_logging_)
        log_message("do_send_next_tasks -function", logging::trivial::trace);
    tcp_tools::concurrent_queue<tcp_tools::task_structure>& tasks = task_queue();
    std::ostringstream out;
    out << tcp_tools::g_new_tasks_from_client_string << " ";
    tcp_tools::task_structure task;
    for( ; tasks.try_pop(task); )
    {
        out << task << " ";
    }
    out << "\n"; // kirjoitettava sanoma pit‰‰ lopettaa rivinvaihtoon
    do_write(out.str());
}

void client_to_master_connection::on_new_results(const std::string & msg) 
{
    if(tcp_tools::use_binary_transfer())
        do_process_binary_result(msg);
    else
    {
        std::istringstream in(msg);
        std::string answer;
        in >> answer; // luetaan 1. sana pois streamista
        do_process_results(in);
    }
}

void client_to_master_connection::do_process_results(std::istream &in)
{
    tcp_tools::concurrent_queue<tcp_tools::work_result_structure>& results = result_queue();
    try
    {
        tcp_tools::read_queue(in, results);
        if(use_verbose_logging_)
            tcp_tools::log_concurrent_queue(results, logging::trivial::debug);
        else
            log_message("Received new results", logging::trivial::debug);

        do_ping(); // tehd‰‰n saapuneiden resulttien j‰lkeen heti ping uudestaan, jotta saamme ehk‰ lis‰‰ resultteja
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
    do_write(tcp_tools::g_error_string + "\n");
}

void client_to_master_connection::do_process_binary_result(const std::string &msg)
{
    try
    {
        tcp_tools::log_start_of_binary_message("binary result from server: ", msg, 30, logging::trivial::info);

        tcp_tools::work_result_structure result;
        result.from_binary_string(msg); // Oletus: binary moodissa vain 1 result per sanoma
        result_queue().push(result);

        do_ping(); // tehd‰‰n saapuneiden resulttien j‰lkeen heti ping uudestaan, jotta saamme ehk‰ lis‰‰ resultteja
        return ;
    }
    catch(std::exception &e)
    {
        log_message(std::string("Error in do_process_binary_result: ") + e.what(), logging::trivial::error);
    }
    catch(...)
    {
        log_message("Unknown error in do_process_binary_result", logging::trivial::error);
    }
    do_write(tcp_tools::g_error_string + "\n");
}

void client_to_master_connection::do_ping() 
{
    do_write("ping\n", false); // false = ei lokiteta t‰t‰ miss‰‰n tapauksessa 
}

void client_to_master_connection::postpone_ping() 
{
    static int counter = 0; // testej‰ varten countteri
    counter++;

    int millis = tcp_tools::g_postpone_ping_timeout_in_ms; // tietyin v‰liajoin kysell‰‰n pingill‰ onko teht‰vi‰
    //if(counter == 5)
    //    millis = 7000;
    //if(use_verbose_logging_)
    //    log_message(std::string("Postponing ping ") + boost::lexical_cast<std::string>(millis) + " millis", logging::trivial::trace);
    timer_.expires_from_now(boost::posix_time::millisec(millis));
    timer_.async_wait( MEMBER_FUNC(do_ping));
}

void client_to_master_connection::on_write(const error_code & /* err */ , size_t /* bytes */ ) 
{
    do_read();
}

void client_to_master_connection::do_read() 
{
    if(binary_reading_state_ == tcp_tools::binary_read_state_found)
    { // binary sanoman loppuosan luku
        async_read(sock_, boost::asio::buffer(binary_read_buffer_), 
            MEMBER_FUNC2(binary_read_complete,_1,_2), MEMBER_FUNC2(on_read,_1,_2));
    }
    else
    { // normaali tavu kerrallaan luku
        async_read(sock_, boost::asio::buffer(fixed_size_read_buffer_), 
            MEMBER_FUNC2(read_complete,_1,_2), MEMBER_FUNC2(on_read,_1,_2));
    }
}

void client_to_master_connection::do_write(const std::string & msg, bool allow_logging) 
{
    if ( !started() ) 
        return;
    if(write_buffer_.size() < msg.size())
        write_buffer_.resize(msg.size()); // kasvatetaan write bufferin kokoa tarvittaessa

    if(allow_logging && use_verbose_logging_)
        tcp_tools::log_start_of_binary_message("do_write: ", msg, 30, logging::trivial::trace);

    std::copy(msg.begin(), msg.end(), write_buffer_.begin());
    sock_.async_write_some( boost::asio::buffer(write_buffer_, msg.size()), 
        MEMBER_FUNC2(on_write,_1,_2));
}

size_t client_to_master_connection::read_complete(const boost::system::error_code & err, size_t bytes) 
{
    return tcp_tools::read_complete_check_look_for_binary_message(err, bytes, fixed_size_read_buffer_, transmission_incomplete_, binary_reading_state_, binary_read_buffer_size_);
}

size_t client_to_master_connection::binary_read_complete(const boost::system::error_code & err, size_t bytes) 
{
    return tcp_tools::binary_read_complete_check(err, bytes, binary_read_buffer_size_, binary_reading_state_);
}
