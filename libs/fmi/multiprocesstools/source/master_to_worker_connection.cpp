#include "master_to_worker_connection.h"
#include "logging.h"
#include "NFmiStaticTime.h"
#include "process_helpers.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <fstream>
#include <set>

#ifdef WIN32
#include <conio.h>
#endif

namespace 
{
    boost::asio::io_service g_service;
    boost::asio::ip::tcp::acceptor acceptor(g_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), tcp_tools::g_local_ip_port));

    typedef std::vector<master_to_worker_connection::connection_ptr> worker_array;
    worker_array g_workers;
    boost::mutex g_workers_mutex_; // kaikki g_workers -k‰sittelyt pit‰‰ sulkea t‰m‰n mutexin lukon taakse

    // Kaikille Masterin yhteyksille on yhteyset globaalit tyˆ- ja tulos jonot
    tcp_tools::concurrent_queue<tcp_tools::task_structure> g_work_queue;
    tcp_tools::concurrent_queue<tcp_tools::work_result_structure> g_work_result_queue;
    tcp_tools::concurrent_queue<std::string> g_work_binary_result_queue; // t‰h‰n talletetaan purkamatta binary sanoma, joka saatiin workerilt‰, ett‰ sevoidaan laittaa sellaisenaan clientille
}

bool master_to_worker_connection::use_verbose_logging_ = false;
bool master_to_worker_connection::stop_master_server = false;
bool master_to_worker_connection::do_time_out_checks_ = true;
boost::posix_time::ptime master_to_worker_connection::last_working_action_time = boost::posix_time::microsec_clock::local_time();
boost::asio::deadline_timer master_to_worker_connection::static_stopping_service_timer_(g_service);
boost::asio::deadline_timer master_to_worker_connection::static_check_keyboard_timer_(g_service);
boost::asio::deadline_timer master_to_worker_connection::static_check_idle_timer_(g_service);
boost::asio::deadline_timer master_to_worker_connection::static_clean_old_results__timer_(g_service);
boost::asio::deadline_timer master_to_worker_connection::static_check_workers_running_timer_(g_service);

int master_to_worker_connection::worker_count_ = 0;
std::string master_to_worker_connection::worker_base_name_ = "worker";
std::string master_to_worker_connection::worker_executable_absolute_path_ = "";
std::string master_to_worker_connection::worker_executable_name_ = "";
std::string master_to_worker_connection::worker_project_name_ = "";
std::vector<std::string> master_to_worker_connection::extra_args_;
bool master_to_worker_connection::add_command_line_flags_ = false;

// oletus k‰sittely funktio on dos-versio
master_to_worker_connection::keyboard_input_interface master_to_worker_connection::keyboard_check_function_ = master_to_worker_connection::dos_check_if_key_pressed;

master_to_worker_connection::master_to_worker_connection() 
:sock_(g_service)
,fixed_size_read_buffer_(tcp_tools::g_fixed_size_read_buffer_size)
,complete_message_from_read_()
,transmission_incomplete_(false)
,write_buffer_() // write bufferin kokoa s‰‰det‰‰n lennossa ajon aikana
,started_(false)
,actual_client_connection_(false)
,client_guid_()
,timer_(g_service) 
,binary_read_buffer_size_(0)
,binary_read_buffer_()
,worker_is_doing_task_(false)
,on_check_ping_counter(0)
{
    binary_reading_state_ = tcp_tools::get_binary_reading_reset_state(); // asetetaan tila seuraavaa sanoman lukua varten kohdalleen 1. kerran konstruktorissa
}

void master_to_worker_connection::start() 
{
    started_ = true;
    {
        boost::mutex::scoped_lock lock(g_workers_mutex_);
        g_workers.push_back( shared_from_this());
    }
    last_ping_ = boost::posix_time::microsec_clock::local_time();
    post_check_ping();
    // first, we wait for client to login
    do_read();
}

master_to_worker_connection::connection_ptr master_to_worker_connection::new_() 
{
    master_to_worker_connection::connection_ptr new_ptr(new master_to_worker_connection);
    return new_ptr;
}

void master_to_worker_connection::stop() 
{
    if ( !started_) return;
    started_ = false;
    sock_.close();

    connection_ptr self = shared_from_this();
    {
        boost::mutex::scoped_lock lock(g_workers_mutex_);
        worker_array::iterator it = std::find(g_workers.begin(), g_workers.end(), self);
        g_workers.erase(it);
    }
}

boost::asio::io_service& master_to_worker_connection::master_service()
{
    return g_service;
}

tcp_tools::concurrent_queue<tcp_tools::task_structure>& master_to_worker_connection::work_queue()
{
    return g_work_queue;
}

tcp_tools::concurrent_queue<tcp_tools::work_result_structure>& master_to_worker_connection::work_result_queue()
{
    return g_work_result_queue;
}

void master_to_worker_connection::handle_accept_first_connection()
{
    connection_ptr client = new_();
    acceptor.async_accept(client->sock(), boost::bind(handle_accept_connection, client, _1));
}

void master_to_worker_connection::handle_accept_connection(connection_ptr client, const error_code & /* err */ )
{
    client->start();
    master_to_worker_connection::connection_ptr new_client = master_to_worker_connection::new_();
    acceptor.async_accept(new_client->sock(), boost::bind(handle_accept_connection, new_client, _1));
}

void master_to_worker_connection::startup_workers(int worker_count, const std::string & worker_base_name, const std::string & worker_executable_absolute_path, const std::string & worker_executable_name, const std::string & worker_project_name, const std::vector<std::string> &extra_args, bool add_command_line_flags)
{
    // Asetetaan aluksi staattiset muuttujat kohdalleen, 
    // jotta myˆhemmin voidaan her‰tell‰ workereit‰ hwenkiin n‰iden tietojen avulla.
    worker_count_ = worker_count;
    worker_base_name_ = worker_base_name;
    worker_executable_absolute_path_ = worker_executable_absolute_path;
    worker_executable_name_ = worker_executable_name;
    worker_project_name_ = worker_project_name;
    extra_args_ = extra_args;
    add_command_line_flags_ = add_command_line_flags;

    for(int i = 1; i <= worker_count_; i++)
        startup_worker(i);
}

void master_to_worker_connection::startup_worker(int worker_index)
{
    std::string worker_name = worker_base_name_ + tcp_tools::g_worker_name_index_separator + boost::lexical_cast<std::string>(worker_index);
    tcp_tools::create_worker_process(process_helpers::make_worker_executable_full_path(worker_executable_absolute_path_, worker_executable_name_, worker_project_name_), worker_name, "Creating worker Tcp process: ", extra_args_, add_command_line_flags_);
}

void master_to_worker_connection::initialize_master(bool verbose_logging)
{
    use_verbose_logging(verbose_logging);
    handle_accept_first_connection();

    post_check_keyboard();

    set_last_working_action_time();
    post_check_idle_timeout();
    post_clean_old_results_timeout();
    post_check_workers_running_timeout();
}

void master_to_worker_connection::set_keyboard_check_function(const keyboard_input_interface &keyboard_check_function)
{
    keyboard_check_function_ = keyboard_check_function;
}

// Tarkistaa onko mit‰‰n n‰pp‰int‰ painettu. 
// Jos ei ole, palauttaa 0:n.
// Ei toimi Linuxilla.
int master_to_worker_connection::dos_check_if_key_pressed()
{
    int ch = 0;
#ifdef WIN32
    if(::_kbhit()) // _kbhit tarkistaa onko painettu mit‰‰n n‰pp‰int‰, ei j‰‰ odottamaan
        ch = _getch(); // jos oli painettu jotain n‰pp‰int‰, pit‰‰ tarkistaa oliko se mahdollisesti q, jolloin k‰ytt‰j‰ haluaa pakotetun lopetuksen
#endif
    return ch;
}

void master_to_worker_connection::post_check_keyboard() 
{
    static_check_keyboard_timer_.expires_from_now(boost::posix_time::millisec(500));
    static_check_keyboard_timer_.async_wait(master_to_worker_connection::on_check_keyboard);
}

void master_to_worker_connection::on_check_stopping_service(const error_code & /* error */ ) 
{
    if(master_to_worker_connection::stop_master_server && g_workers.empty())
    {
        log_message("Quit requested by user, stopping service after last connection was closed", logging::trivial::info);
        g_service.stop();
    }
    else
    {
        master_to_worker_connection::post_check_stopping_service(); // k‰ynnistet‰‰n muuten t‰m‰ aina uudelleen
    }
}

void master_to_worker_connection::post_check_stopping_service() 
{
    static_stopping_service_timer_.expires_from_now(boost::posix_time::millisec(1000));
    static_stopping_service_timer_.async_wait(on_check_stopping_service);
}

void master_to_worker_connection::on_check_idle_timeout(const error_code & /* error */ ) 
{
    if(stop_master_server)
        return ; // lopetus k‰sky on tullut jo toisaalta, ei jatketa

    long long time_lasted_in_ms = 0;
    if(tcp_tools::is_timeout(last_working_action_time, no_work_server_time_out_in_ms_, &time_lasted_in_ms)) 
    {
        log_message(std::string("No-work timeout (last work ") + boost::lexical_cast<std::string>(time_lasted_in_ms) + " ms ago), time to stop master server", logging::trivial::info);
        stop_master_server = true;
        master_to_worker_connection::post_check_stopping_service(); // aletaan tutkimaan, milloin kaikki yhteydet on suljettu, jotta voidaan sulkea Asio-service ja p‰‰ohjelma.
    }
    else
    {
        master_to_worker_connection::post_check_idle_timeout(); // k‰ynnistet‰‰n muuten t‰m‰ aina uudelleen
    }
}

void master_to_worker_connection::post_check_idle_timeout() 
{
    static_check_idle_timer_.expires_from_now(boost::posix_time::millisec(1000));
    static_check_idle_timer_.async_wait(on_check_idle_timeout);
}

class old_work_binary_result_matcher
{
    std::size_t current_time_;
    int wanted_expiration_time_in_seconds_;
public:
    old_work_binary_result_matcher(int wanted_expiration_time_in_seconds) 
    :wanted_expiration_time_in_seconds_(wanted_expiration_time_in_seconds) 
    {
        NFmiStaticTime localTime;
        current_time_ = localTime.EpochTime(); // t‰m‰n hetkinen aika talteen
    }

    bool operator()(const std::string &binary_result) const
    { 
        size_t check_chars_from_start = 50;
        std::istringstream in(binary_result.size() <= check_chars_from_start ? binary_result : std::string(binary_result.data(), check_chars_from_start));
        // tuloksen aikaleima on 5. sana
        std::string word1, word2, word3, word4;
        in >> word1 >> word2 >> word3 >> word4;
        std::size_t work_time;
        in >> work_time;
        if(in)
        {
            if(current_time_ - work_time > wanted_expiration_time_in_seconds_)
                return true;
            else
                return false;
        }
        else
        {
            tcp_tools::log_start_of_binary_message("Error in old_work_binary_result_matcher test, binary_result had invalid structure, removing result from result-queue: ", binary_result, 50, logging::trivial::error);
            // mit‰ tehd‰‰n v‰‰r‰nlaiselle datalle?
            return true; // siivotaan pois ainakin toistaiseksi
        }
    }
};

void master_to_worker_connection::on_clean_old_results_timeout(const error_code & /* error */ )
{
    int result_expiration_timeout_in_seconds = 120;
    std::list<std::string> removed_old_binary_result_list = g_work_binary_result_queue.remove_copy_list_if(old_work_binary_result_matcher(result_expiration_timeout_in_seconds));
    if(removed_old_binary_result_list.size())
        log_message(std::string("master_to_worker_connection::on_clean_old_results_timeout: cleaned old results from results-queue due time expiration: ") + boost::lexical_cast<std::string>(removed_old_binary_result_list.size()), logging::trivial::info);
    else
        log_message(std::string("master_to_worker_connection::on_clean_old_results_timeout: no results to be cleaned out"), logging::trivial::info);

    master_to_worker_connection::post_clean_old_results_timeout(); // k‰ynnistet‰‰n t‰m‰ timeout aina uudelleen lopuksi
}

void master_to_worker_connection::post_clean_old_results_timeout()
{
    static_clean_old_results__timer_.expires_from_now(boost::posix_time::millisec(60 * 1000));
    static_clean_old_results__timer_.async_wait(on_clean_old_results_timeout);
}

// Palauttaa kaikkien worker-yhteyksien nimet. 
// Jos yhteyden username:sta lˆytyy g_client_login_base_string -sana, 
// kyse on client-yhteydest‰ (esim. Smartmet) ja sit‰ ei oteta mukaan listaan.
// Kun g_workers -listaa k‰yd‰‰n l‰pi, pit‰‰ lukita g_workers_mutex_!!
static std::list<std::string> get_worker_name_list()
{
    std::list<std::string> name_list;

    { // t‰m‰ blokki on vain mutexia ja lukkoa varten
        boost::mutex::scoped_lock lock(g_workers_mutex_);
        for(auto iter = g_workers.begin(); iter != g_workers.end(); ++iter)
        {
            std::string username = (*iter)->username();
            if(username.find(tcp_tools::g_client_login_base_string) == std::string::npos) // clientteja ei laiteta t‰h‰n listaan
                name_list.push_back(username);
        }
    }
    return name_list;
}

// Master k‰ynnist‰‰ workereit‰, niiden nimet menev‰t seuraavan kaavan mukaisesti:
// worker#1, worker#2 ,..., worker#n
// Haetaan int -listaan nuo indeksit nimen per‰ss‰. Haetaan nimest‰ tcp_tools::g_worker_name_index_separator
// -merkki‰, ja sen merkin j‰lkeen j‰lkeen on nimen loppuun oleva osio indeksi‰.
static std::list<int> get_worker_indexies(const std::list<std::string> &worker_name_list)
{
    std::list<int> running_worker_indexies;
    for(const auto &name : worker_name_list)
    {
        size_t pos = name.find_last_of(tcp_tools::g_worker_name_index_separator);
        if(pos != std::string::npos)
        {
            try
            {
                std::string index_string(name.begin()+pos+1, name.end());
                running_worker_indexies.push_back(boost::lexical_cast<int>(index_string));
            }
            catch(...)
            {}
        }
    }
    running_worker_indexies.sort();
    return running_worker_indexies;
}

// Annettu lista indeksej‰ ja max_worker_count. Selvit‰ mitk‰ luvut puuttuvat listasta, 
// jos siin‰ pit‰isi olla luvut 1-max_worker_count. 
// Palauta t‰ll‰inen lista.
// Oletus: worker_index_list on jo sortattu.
static std::list<int> get_missing_worker_indexies(const std::list<int> &worker_index_list, int max_worker_count)
{
    std::list<int> missing_worker_index_list;
    for(int i = 1; i <= max_worker_count; i++)
    {
        auto iter = std::find(worker_index_list.begin(), worker_index_list.end(), i);
        if(iter == worker_index_list.end())
            missing_worker_index_list.push_back(i);
    }
    return missing_worker_index_list;
}

void master_to_worker_connection::on_check_workers_running_timeout(const error_code & /* error */ )
{
    if(stop_master_server)
        return ; // lopetus k‰sky on tullut jo toisaalta, ei jatketa
    if(master_to_worker_connection::worker_count_ == 0)
        return ; // ei ole haluttu ollenkaan workereit‰, ei jatketa

    if(g_workers.size()) // jos ei ole (viel‰?) ollenkaan workereit‰ elossa, ei kannata niit‰ t‰ss‰ her‰tell‰
    {
        if(g_workers.size() < master_to_worker_connection::worker_count_) // vain jos haluttu worker m‰‰r‰ on suurempi kuin lˆydettyjen m‰‰r‰, her‰tell‰‰n workereit‰ eloon
        {
            log_message("There were one or more stopped workers, trying to startup them", logging::trivial::warning);
            // 1. Tee lista k‰ynniss‰ olevista master_to_worker_connection -olioista (username-lista)
            std::list<std::string> worker_name_list = ::get_worker_name_list();
            // 2. Hae nimilistasta lista indeksej‰
            std::list<int> worker_index_list = ::get_worker_indexies(worker_name_list);
            // 3. Tutki puuttuuko joku listalta
            std::list<int> missing_worker_index_list = get_missing_worker_indexies(worker_index_list, master_to_worker_connection::worker_count_);
            // 4. Jos puuttui nro 1-n, k‰ynnist‰ se uudestaan
            for(auto index : missing_worker_index_list)
                master_to_worker_connection::startup_worker(index);
        }
        else
            log_message("There were no stopped workers", logging::trivial::debug);
    }

    master_to_worker_connection::post_check_workers_running_timeout(); // k‰ynnistet‰‰n t‰m‰ timeout aina uudelleen lopuksi
}

void master_to_worker_connection::post_check_workers_running_timeout()
{
    static_check_workers_running_timer_.expires_from_now(boost::posix_time::millisec(60 * 1000));
    static_check_workers_running_timer_.async_wait(on_check_workers_running_timeout);
}

// Aina kun joku tekee jotain tyˆt‰ tai kun saadaan uusia tˆit‰, t‰t‰ pit‰‰ kutsua.
void master_to_worker_connection::set_last_working_action_time()
{
    last_working_action_time = boost::posix_time::microsec_clock::local_time();
}

void master_to_worker_connection::on_read(const error_code & err, size_t bytes) 
{
    //if(use_verbose_logging_)
    //    log_message(std::string("on_read -function from: ") + username_, logging::trivial::trace);

    if ( err) 
    {
        if(!stop_master_server) // ei laiteta virhe ilmoitusta, jos serveri on ollut tarkoitus laittaa nurin
            log_message(std::string("Error in on_read category: ") + err.category().name() + ", message: " + err.message(), logging::trivial::error);
        stop();
    }
    if ( !started() ) 
        return;

    tcp_tools::boost_asio_async_read_behaviour_change_fix(bytes, binary_read_buffer_, &binary_reading_state_);

    last_ping_ = boost::posix_time::microsec_clock::local_time(); // viimeksi kun ollaan oltu yhteydess‰ clientiin
    if(binary_reading_state_ == tcp_tools::binary_read_state_found)
    {
        if(use_verbose_logging_)
            log_message(std::string("Binary transmission starts from: ") + username_, logging::trivial::trace);

        binary_read_buffer_.resize(binary_read_buffer_size_);
        do_read(); // jatketaan binary sanoman lukua
    }
    else if(transmission_incomplete_)
    {
        if(use_verbose_logging_)
            log_message(std::string("incomplete transmission from: ") + username_, logging::trivial::trace);

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
            on_ping(); // ping sanomia ei lokiteta koskaan
        else
        {
            if(use_verbose_logging_)
                log_message(std::string("complete message from: ") + username_, logging::trivial::trace);

            if(tcp_tools::find_from_start_of_string(msg, "login ", tcp_tools::g_search_message_start_max_chars))
                on_login(msg);
            else if(tcp_tools::find_from_start_of_string(msg, tcp_tools::g_new_tasks_from_client_string, tcp_tools::g_search_message_start_max_chars))
                on_new_tasks(msg);
            else if(tcp_tools::find_from_start_of_string(msg, tcp_tools::g_result_from_worker_string, tcp_tools::g_search_message_start_max_chars))
                on_result_from_worker(msg);
            else if(tcp_tools::find_from_start_of_string(msg, tcp_tools::g_error_string, tcp_tools::g_search_message_start_max_chars))
                on_error_result(msg);
            else 
            {
                log_message(std::string("invalid msg: ") + msg + ", continuing to on_ping...", logging::trivial::error);
                on_ping();
            }
        }
    }
}
    
void master_to_worker_connection::on_error_result(const std::string & msg) 
{
    log_message(std::string("Task sended to worker was returned with error: ") + msg, logging::trivial::error);
    do_next_task();
}

void master_to_worker_connection::on_result_from_worker(const std::string & msg) 
{
    if(tcp_tools::use_binary_transfer())
        do_process_binary_result_from_worker(msg);
    else
    {
        std::istringstream in(msg);
        std::string answer;
        in >> answer; // luetaan 1. sana pois streamista
        do_process_result_from_worker(in);
    }
}

void master_to_worker_connection::on_new_tasks(const std::string & msg)
{
    std::istringstream in(msg);
    std::string answer;
    in >> answer; // luetaan 1. sana pois streamista
    do_process_new_tasks(in);
}

// Jos username_:ssa on tietty sana, kyse on clientista, eik‰ workerist‰
void master_to_worker_connection::define_connection()
{
    size_t pos = username_.find(tcp_tools::g_client_login_base_string);
    if(pos != std::string::npos)
    { // oli client
        actual_client_connection_ = true;
        client_guid_ = std::string(username_.begin() + pos + tcp_tools::g_client_login_base_string.size(), username_.end());
        boost::algorithm::trim(client_guid_);
    }
}

void master_to_worker_connection::on_login(const std::string & msg) 
{
    size_t pos = msg.find(" "); // etsi 1. space
    if(pos == std::string::npos)
    {
        log_message("Failed to log in, no username?", logging::trivial::info);
        do_write("login error, no username? Stopping conection...\n");
        stop();
    }
    else
    {
        username_ = std::string(msg.begin()+pos, msg.end());
        boost::algorithm::trim(username_); // poistetaan spacet alusta ja lopusta, ja katsotaan mit‰ j‰i j‰ljelle
        if(username_.empty())
        {
            log_message("Failed to log in, no username?", logging::trivial::info);
            do_write("login error, no username? Stopping conection...\n");
            stop();
        }

        define_connection();
        log_message(username_ + " logged in", logging::trivial::info);
        do_write("login ok\n");
    }
}

void master_to_worker_connection::on_ping() 
{
    //log_message(std::string("on_ping from: ") + username_, logging::trivial::trace);
    if(actual_client_connection_)
        do_next_results();
    else
        do_next_task();
}

void master_to_worker_connection::log_result(const tcp_tools::work_result_structure &result)
{
    std::ostringstream out;
    out << "Received result from " << username_;
    if(master_to_worker_connection::use_verbose_logging_)
        out << ": " << result;
    log_message(out.str(), logging::trivial::debug);
}

void master_to_worker_connection::log_task(const tcp_tools::task_structure &task)
{
    std::ostringstream out;
    out << "Received task from " << username_;
    if(master_to_worker_connection::use_verbose_logging_)
        out << ": " << task;
    log_message(out.str(), logging::trivial::debug);
}

void master_to_worker_connection::do_ping() 
{
    //if(use_verbose_logging())
    //    log_message(std::string("do_ping ") + username_, logging::trivial::trace);

    worker_is_doing_task_ = false;
    do_write("ping\n", false); // false = ei lokitusta miss‰‰n tapauksessa, pingej‰ tehd‰‰n 10x sekunnissa per worker
}

void master_to_worker_connection::do_process_new_tasks(std::istream &in)
{
    try
    {
        if(!actual_client_connection_)
            throw std::runtime_error("Doing do_process_new_tasks -function while not being connected to actual client but worker process.");

        master_to_worker_connection::set_last_working_action_time();
        tcp_tools::read_queue(in, g_work_queue);

        // TƒMƒ ON TESTIƒ VARTEN; FEIKKI TY÷ TEHDƒƒN HETI TƒSSƒ
//        tcp_tools::make_some_fake_work(g_work_queue, g_work_result_queue, true);
    }
    catch(std::exception &e)
    {
        log_message(std::string("Error in do_process_new_tasks: ") + e.what(), logging::trivial::error);
    }
    catch(...)
    {
        log_message("Unknown error in do_process_new_tasks", logging::trivial::error);
    }
    do_next_results();
}

void master_to_worker_connection::do_process_result_from_worker(std::istream &in)
{
    try
    {
        if(actual_client_connection_)
            throw std::runtime_error("Doing do_process_result_from_worker -function while being connected to actual client and not to worker process.");

        master_to_worker_connection::set_last_working_action_time();
        tcp_tools::work_result_structure result;
        in >> result;
        log_result(result);
        g_work_result_queue.push(result);
    }
    catch(std::exception &e)
    {
        log_message(std::string("Error in do_process_result_from_worker: ") + e.what(), logging::trivial::error);
    }
    catch(...)
    {
        log_message("Unknown error in do_process_result_from_worker", logging::trivial::error);
    }
    do_next_task();
}

void master_to_worker_connection::do_process_binary_result_from_worker(const std::string & msg)
{
    try
    {
        if(actual_client_connection_)
            throw std::runtime_error("Doing do_process_binary_result_from_worker -function while being connected to actual client and not to worker process.");

        master_to_worker_connection::set_last_working_action_time();
        tcp_tools::log_start_of_binary_message(std::string("binary result from ") + username_ + ": ", msg, 30, logging::trivial::info);

        // Koska luettaessa binary sanomaa, se on pit‰nyt jakaa osiin, pit‰‰ se koostaa kokonaisuudessaan takaisin t‰ss‰, ennen kuin se laitetaan result-jonoon.
        std::string binary_message_start = boost::lexical_cast<std::string>(binary_read_buffer_size_);
        binary_message_start += " ";
        g_work_binary_result_queue.push(binary_message_start + msg); 
    }
    catch(std::exception &e)
    {
        log_message(std::string("Error in do_process_binary_result_from_worker: ") + e.what(), logging::trivial::error);
    }
    catch(...)
    {
        log_message("Unknown error in do_process_binary_result_from_worker", logging::trivial::error);
    }
    do_next_task();
}

void master_to_worker_connection::do_next_task(void)
{
    tcp_tools::task_structure task;
    if(g_work_queue.try_pop(task))
        do_send_task(task);
    else
        do_ping();
}

class work_result_matcher
{
   std::string wanted_guid_;
   public:
     work_result_matcher(std::string wanted_guid) : wanted_guid_(wanted_guid) {}
     bool operator()(const tcp_tools::work_result_structure &result) const
     { 
         return result.smartmet_guid_ == wanted_guid_;
     }
};

std::list<tcp_tools::work_result_structure> master_to_worker_connection::get_new_work_results()
{
    return g_work_result_queue.remove_copy_list_if(work_result_matcher(client_guid_));
}

static void force_template_code_compilation_function(void)
{
    // Template koodien k‰‰nnˆstesti (t‰t‰ ei kutsuta mist‰‰n)
    std::ifstream in("xxx");
    tcp_tools::read_queue(in, g_work_queue);
    tcp_tools::log_concurrent_queue(g_work_queue, logging::trivial::info);
    std::ofstream out("xxx");
    tcp_tools::write_concurrent_queue(out, g_work_queue);
}

void master_to_worker_connection::do_send_new_work_results_to_client(const std::list<tcp_tools::work_result_structure> &new_results)
{
    if(!actual_client_connection_)
        throw std::runtime_error("Doing do_send_new_work_results_to_client -function while not being connected to actual client but worker process.");

    log_message(std::string("Sending ") + boost::lexical_cast<std::string>(new_results.size()) + " new result(s) to " + username_, logging::trivial::info);

    master_to_worker_connection::set_last_working_action_time();
    std::ostringstream out;
    out << tcp_tools::g_new_results_to_client_string << " ";
    tcp_tools::write_list(out, new_results);
    out << "\n"; // kirjoitettava sanoma pit‰‰ lopettaa rivinvaihtoon
    do_write(out.str());
}

class binary_work_result_matcher
{
   std::string wanted_guid_;
   public:
     binary_work_result_matcher(std::string wanted_guid) : wanted_guid_(wanted_guid) {}

     bool operator()(const std::string &binary_result_string) const
     { 
         return tcp_tools::find_from_start_of_string(binary_result_string, wanted_guid_, 80); // 80 merkin sis‰ll‰ pit‰isi olla clientin guid -stringi
     }
};

void master_to_worker_connection::do_next_results(void)
{
    if(use_verbose_logging())
        log_message(std::string("do_next_results ") + username_, logging::trivial::trace);
    std::list<tcp_tools::work_result_structure> new_results = get_new_work_results();
    if(!new_results.empty())
        do_send_new_work_results_to_client(new_results);
    else
    {
        // HUOM1 t‰ss‰ binary result jutussa ei ole viel‰ kuin yhden sanoman l‰hetys kerrallaan.
        std::string binary_result;
        if(g_work_binary_result_queue.try_get_next(binary_work_result_matcher(client_guid_), binary_result))
            do_write(binary_result);
        else
            do_ping();
    }
}

void master_to_worker_connection::do_send_task(const tcp_tools::task_structure &task)
{
    master_to_worker_connection::set_last_working_action_time();
    worker_is_doing_task_ = true;
    std::ostringstream out;
    out << tcp_tools::g_new_task_to_worker_string << " " << task;
    out << "\n"; // kirjoitettava sanoma pit‰‰ lopettaa rivinvaihtoon
    do_write(out.str());
}

void master_to_worker_connection::on_check_ping() 
{
    if(stop_master_server) 
    {
        log_message(std::string("stopping ") + username_ + " -connection because user request", logging::trivial::info);
        stop();
    }
    else
    {
        int used_time_out = actual_client_connection_ ? (no_work_client_ping_time_out_in_ms_ * 10) : (worker_is_doing_task_ ? working_client_ping_time_out_in_ms_ : no_work_client_ping_time_out_in_ms_);
        long long time_lasted_in_ms = 0;
        if(master_to_worker_connection::do_time_out_checks_ && tcp_tools::is_timeout(last_ping_, used_time_out, &time_lasted_in_ms)) 
        {
            std::string stopping_log_message = "stopping " + username_;
            stopping_log_message += worker_is_doing_task_ ? " (was working)" : " (was idleing)";
            stopping_log_message += " - no ping in time (waited for " + boost::lexical_cast<std::string>(time_lasted_in_ms) + " ms)";
            log_message(stopping_log_message, logging::trivial::info);
            stop();
        }
        else
        {
            if((on_check_ping_counter++ % 10) == 0 && use_verbose_logging_) // tulostetaan vain joka 10. ping tarkastus, eli joka 10. sekunti
                log_message(std::string("on_check_ping: waited for ") + username_ + "'s ping for " + boost::lexical_cast<std::string>(time_lasted_in_ms) + " ms", logging::trivial::trace);
            post_check_ping(); // k‰ynnistet‰‰n t‰m‰ aina uudelleen
        }
    }
}

void master_to_worker_connection::post_check_ping() 
{
    timer_.expires_from_now(boost::posix_time::millisec(1000));
    timer_.async_wait( MEMBER_FUNC(on_check_ping));
}

// Tehd‰‰n n‰pp‰imistˆn k‰sittelyst‰ yksitt‰inen tapahtuma, eli kaikille
// talk_to_client -yhteyksille ei tehd‰ omaa k‰sittely‰, vaan yksi yhteinen 
// staattinen funktio k‰sittely.
void master_to_worker_connection::on_check_keyboard(const error_code & error) 
{
    if(error)
        return ;
    if(stop_master_server)
        return ; // lopetus k‰sky on tullut jo toisaalta, ei jatketa

    int ch = keyboard_check_function_();
    if(ch == 'q' || ch == 'Q')
    {
        log_message("User requested to stop master server (by hitting 'q'-key)", logging::trivial::info);
        stop_master_server = true;
        master_to_worker_connection::post_check_stopping_service(); // aletaan tutkimaan, milloin kaikki yhteydet on suljettu, jotta voidaan sulkea Asio-service ja p‰‰ohjelma.
    }
    else
    {
        master_to_worker_connection::post_check_keyboard(); // k‰ynnistet‰‰n muuten t‰m‰ aina uudelleen
    }
}

void master_to_worker_connection::on_write(const error_code & /* err */ , size_t /* bytes */ ) 
{
    do_read();
}

void master_to_worker_connection::do_read() 
{
    //if(use_verbose_logging_)
    //    log_message(std::string("do_read -function from: ") + username_, logging::trivial::trace);

    if(binary_reading_state_ == tcp_tools::binary_read_state_found)
    { // binary sanoman loppuosan luku
        async_read(sock_, boost::asio::buffer(binary_read_buffer_), 
            MEMBER_FUNC2(binary_read_complete,_1,_2), MEMBER_FUNC2(on_read,_1,_2));

        tcp_tools::trace_log_start_of_container("do_read reads following binary message: ", use_verbose_logging_, binary_read_buffer_);
    }
    else
    { // normaali tavu kerrallaan luku
        async_read(sock_, boost::asio::buffer(fixed_size_read_buffer_), 
            MEMBER_FUNC2(read_complete,_1,_2), MEMBER_FUNC2(on_read,_1,_2));
    }
}

void master_to_worker_connection::do_write(const std::string & msg, bool allow_logging) 
{
    if ( !started() ) 
        return;

    if(write_buffer_.size() < msg.size())
        write_buffer_.resize(msg.size()); // kasvatetaan write bufferin kokoa tarvittaessa

    if(allow_logging && use_verbose_logging_)
        tcp_tools::log_start_of_binary_message(std::string("do_write to ") + username_ + ": ", msg, 30, logging::trivial::trace);

    std::copy(msg.begin(), msg.end(), write_buffer_.begin());
    sock_.async_write_some( boost::asio::buffer(write_buffer_, msg.size()), 
        MEMBER_FUNC2(on_write,_1,_2));
}

size_t master_to_worker_connection::read_complete(const boost::system::error_code & err, size_t bytes) 
{
    return tcp_tools::read_complete_check_look_for_binary_message(err, bytes, fixed_size_read_buffer_, transmission_incomplete_, binary_reading_state_, binary_read_buffer_size_);
}

size_t master_to_worker_connection::binary_read_complete(const boost::system::error_code & err, size_t bytes) 
{
    return tcp_tools::binary_read_complete_check(err, bytes, binary_read_buffer_size_, binary_reading_state_);
}
