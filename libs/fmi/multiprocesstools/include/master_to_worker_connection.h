#pragma once

#ifdef WIN32
#if !defined(_WIN32_WINNT) && !defined(_CHICAGO_)
#define _WIN32_WINNT 0x0600   // Specifies that the minimum required platform is Windows Vista.
#endif
#endif

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>

#include "tcp_tools.h"

namespace tcp_tools
{
    struct work_result_structure;
    struct task_structure;
}

namespace boost
{
    namespace asio
    {
        class io_service;
    }
}

/** Connection class between server (Master) and client (Worker or real client like SmartMet) process:
    - all connections are initiated by the client: client asks, server answers
    - logs in just with username (no password), if login-name contains certain word, actual_client -flag is set to true.
    - server disconnects any client that hasn't pinged for for certain timeout

    Possible client requests (worker):
    - ping: the server answers either with "ping ok" or "new_task"
    - work_result: the server answers either with "ping ok" or "new_task"
    Possible client requests (when actual_client -flag is true):
    - ping: the server answers either with "ping ok" or "new_results"
    - new_tasks: the server answers either with "ping ok" or "new_results"
*/
class master_to_worker_connection : public boost::enable_shared_from_this<master_to_worker_connection>
                     , boost::noncopyable 
{
    typedef master_to_worker_connection self_type;

    master_to_worker_connection();
public:
    typedef boost::system::error_code error_code;
    typedef boost::shared_ptr<master_to_worker_connection> connection_ptr;
    typedef boost::function<int(void)> keyboard_input_interface;

    void start();
    static connection_ptr new_();
    void stop();
    bool started() const { return started_; }
    boost::asio::ip::tcp::socket & sock() { return sock_;}
    std::string username() const { return username_; }
    static bool use_verbose_logging() {return use_verbose_logging_;}
    static void use_verbose_logging(bool new_value) {use_verbose_logging_ = new_value;}

    static boost::asio::io_service& master_service();
    static tcp_tools::concurrent_queue<tcp_tools::task_structure>& work_queue();
    static tcp_tools::concurrent_queue<tcp_tools::work_result_structure>& work_result_queue();
    static void master_to_worker_connection::handle_accept_first_connection();
    static void handle_accept_connection(connection_ptr client, const error_code &err);
    static void initialize_master(bool verbose_logging);
    static void startup_workers(int worker_count, const std::string &worker_base_name, const std::string &worker_executable_absolute_path, const std::string &worker_executable_name, const std::string &worker_project_name, const std::vector<std::string> &extra_args, bool add_command_line_flags);
    static void startup_worker(int worker_index);

    static void post_check_keyboard();

    static void on_check_stopping_service(const error_code & error);
    static void post_check_stopping_service();

    static void on_check_idle_timeout(const error_code & error);
    static void post_check_idle_timeout();
    static void on_clean_old_results_timeout(const error_code & error);
    static void post_clean_old_results_timeout();
    static void on_check_workers_running_timeout(const error_code & error);
    static void post_check_workers_running_timeout();
    // Aina kun joku tekee jotain työtä tai kun saadaan uusia töitä, tätä pitää kutsua.
    static void set_last_working_action_time();
    static void set_keyboard_check_function(const keyboard_input_interface &keyboard_check_function);
    static void do_time_out_checks(bool new_value) {do_time_out_checks_ = new_value;}

    static bool stop_master_server;
    static boost::posix_time::ptime last_working_action_time;
private:
    void on_read(const error_code & err, size_t bytes);
    
    void on_error_result(const std::string & msg);
    void on_result_from_worker(const std::string & msg);
    void on_new_tasks(const std::string & msg);
    void on_login(const std::string & msg);
    void define_connection();
    void on_ping();

    void log_result(const tcp_tools::work_result_structure &result);
    void log_task(const tcp_tools::task_structure &task);

    void do_ping();
    void do_process_result_from_worker(std::istream &in);
    void do_process_binary_result_from_worker(const std::string & msg);
    void do_process_new_tasks(std::istream &in);
    void do_next_task(void);
    void do_next_results(void);
    void do_send_task(const tcp_tools::task_structure &task);
    std::list<tcp_tools::work_result_structure> get_new_work_results();
    void do_send_new_work_results_to_client(const std::list<tcp_tools::work_result_structure> &new_results);

    void on_check_ping();
    void post_check_ping();

    static int dos_check_if_key_pressed();
    static void on_check_keyboard(const error_code & error);

    void on_write(const error_code & err, size_t bytes);
    void do_read();
    void do_write(const std::string & msg, bool allow_logging = true);
    size_t read_complete(const boost::system::error_code & err, size_t bytes);
    size_t binary_read_complete(const boost::system::error_code & err, size_t bytes);
private:
    boost::asio::ip::tcp::socket sock_;
    std::vector<char> fixed_size_read_buffer_; // Asio:n kautta luetaan fiksatun kokoisissa paketeissa (ei saa muuttaa kokoa konstruktorin jälkeen)
    std::string complete_message_from_read_; // kokonainen luettu viesti talletetaan tähän (tarvittaessa monessa osassa luettuna)
    bool transmission_incomplete_; // tämän avulla tiedetään onko saapuva lähetys luettu loppuun, vai pitääkö fixed_size_read_buffer_ lisätä välillä complete_message_from_read:iin
    std::vector<char> write_buffer_; // tämän kokoa muutetaan tarvittaessa, jos lähetettävä viesti olisi muuten liian iso
    bool started_;
    std::string username_;
    bool actual_client_connection_; // Jos tämä on false, ollaan yhteydessä oikeaan workeriin ja jos true, ollaan yhteydessä oikeaan clientiin (esim. SmartMet)
    std::string client_guid_; // eri clientit ja niiden työt tunnistetaan tämän avulla (saadaan username:sta)
    boost::asio::deadline_timer timer_; // jokaisella yhteydellä on oma timer
    boost::posix_time::ptime last_ping_;
    static bool use_verbose_logging_;
    static keyboard_input_interface keyboard_check_function_;
    static boost::asio::deadline_timer static_stopping_service_timer_; // tehdään luokan staattisille funktioille oma yhteinen timer (stopping_service)
    static boost::asio::deadline_timer static_check_keyboard_timer_; // tehdään luokan staattisille funktioille omat timerit (check_keyboard)
    static boost::asio::deadline_timer static_check_idle_timer_; // tehdään luokan staattisille funktioille omat timerit (idle)
    static boost::asio::deadline_timer static_clean_old_results__timer_; // tehdään luokan staattisille funktioille omat timerit (idle)
    static boost::asio::deadline_timer static_check_workers_running_timer_; // tehdään luokan staattisille funktioille omat timerit (idle)
    static bool do_time_out_checks_; // Kun debugataan systeemiä, ei haluta tehdä timeout testejä, jolloin tämä laitetaan false:ksi.
    
    // Näitä staattisia muutujia käytetään käynnistämään kuollutta workeriä uudestaan eloon.
    // Nämä on tarkoitus asettaa startup_workers -metodilla (joka myös käynnistää halutun määrän workereitä alkuun).
    static int worker_count_; // Kuinka monta workeriä halutaan työskentelemään (koneen säikeiden määrä on hyvä arvaus)
    static std::string worker_base_name_; // Tästä tulee workerin useaname:n pohja esim. worker -> worker#1
    static std::string worker_executable_absolute_path_;
    static std::string worker_executable_name_;
    static std::string worker_project_name_;
    static std::vector<std::string> extra_args_; // workerille annettava erillinen argumentti lista.
    static bool add_command_line_flags_; // Dos worker ohjelmassa ei ole optioiden purkua, joten sinne annetaan argumentit peräkkäin. MFC-versiolle pitää argumenteissa olla kirjain optiot, jotka lisätään erikseen tarvittaessa.

    tcp_tools::binary_read_state binary_reading_state_; // Tätä tilaa käytetään binaari sanomien lukuun ja sen eri tilojen merkitsemiseen (jos luettavan sanoman 1. sana on kokonaisluku ja space on sen perässä)
    size_t binary_read_buffer_size_; // Binary sanoman alussa kerrotaan sanoman koko tavuissa, tähän talletetaan tuo koko
    std::vector<char> binary_read_buffer_; // Binary luvussa käytetään puskurina tätä, ja sen koon voi laittaa suoraan binary_read_buffer_size_:en mukaan
    bool worker_is_doing_task_; 
    size_t on_check_ping_counter; // lasketaan kuinka monta kertaa ollaan on_check_ping -metodissa ja tehdään vain joka n. tarkistukselle lokitus (muuten niitä voi tulla kerran sekunnissa n kpl workeriltä)
    const static int no_work_client_ping_time_out_in_ms_ = 5*1000; // kuinka kauan maksimissaan odotetaan clientin pingiä ennen kuin lopetetaan yhteys (vaihtelee no-work 5000 vs. work tiloissa 50 * 1000)
    const static int working_client_ping_time_out_in_ms_ = 50*1000;
    const static int no_work_server_time_out_in_ms_ = 5*60*1000; // kuinka kauan maksimissaan odotetaan uusia töitä ennen kuin lopetetaan serverin käynnissä olo (5 minuuttia)
};

