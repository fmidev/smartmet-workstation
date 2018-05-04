#pragma once

#ifdef WIN32
#if !defined(_WIN32_WINNT) && !defined(_CHICAGO_)
#define _WIN32_WINNT 0x0600   // Specifies that the minimum required platform is Windows Vista.
#endif
#endif

#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>

#include "tcp_tools.h"

namespace boost
{
    namespace asio
    {
        class io_service;
    }
}



/** simple connection client to server:
    - logs in just with username (no password)
    - all connections are initiated by the client
    - server disconnects any client that hasn't pinged for in certain timeout
    - there is only ONE connection to the server per client because this was intended to be used with possible multiple client processes

    Possible requests:
    - ping: the server answers either with "ping ok" or "new_results"
    - new_tasks: the server answers with "ping ok" or "new_results"
*/
class client_to_master_connection : public boost::enable_shared_from_this<client_to_master_connection>
                  , boost::noncopyable 
{
    typedef client_to_master_connection self_type;

    client_to_master_connection(const std::string & username, bool use_verbose_logging);
    void start(boost::asio::ip::tcp::endpoint ep);
public:
    typedef boost::system::error_code error_code;
    typedef boost::shared_ptr<client_to_master_connection> connection_ptr;

    static void start_client(const std::string & username, bool use_verbose_logging);
    static connection_ptr start(boost::asio::ip::tcp::endpoint ep, const std::string & username, bool use_verbose_logging);
    static boost::asio::io_service& client_service();
    static tcp_tools::concurrent_queue<tcp_tools::task_structure>& task_queue();
    static tcp_tools::concurrent_queue<tcp_tools::work_result_structure>& result_queue();
    void stop();
    bool started() { return started_; }
    bool stopped() { return stopped_; }
    static connection_ptr currently_used_connection();
private:
    void on_connect(const error_code & err);
    void on_read(const error_code & err, size_t bytes);
    
    void on_login();
    void on_ping(const std::string &msg);
    void on_new_results(const std::string & msg);

    void do_process_results(std::istream &in);
    void do_process_binary_result(const std::string &msg);
    void do_next_tasks();
    void do_send_next_tasks();

    void do_ping();

    void postpone_ping();

    void on_write(const error_code &err, size_t bytes);
    void do_read();
    void do_write(const std::string & msg, bool allow_logging = true);
    size_t read_complete(const boost::system::error_code & err, size_t bytes);
    size_t binary_read_complete(const boost::system::error_code & err, size_t bytes);

private:
    boost::asio::ip::tcp::socket sock_;
    std::vector<char> fixed_size_read_buffer_; // Asio:n kautta luetaan fiksatun kokoisissa paketeissa (ei saa muuttaa kokoa konstruktorin j‰lkeen)
    std::string complete_message_from_read_; // kokonainen luettu viesti talletetaan t‰h‰n (tarvittaessa monessa osassa luettuna)
    bool transmission_incomplete_; // t‰m‰n avulla tiedet‰‰n onko saapuva l‰hetys luettu loppuun, vai pit‰‰kˆ fixed_size_read_buffer_ lis‰t‰ v‰lill‰ complete_message_from_read:iin
    std::vector<char> write_buffer_; // t‰m‰n kokoa muutetaan tarvittaessa, jos l‰hetett‰v‰ viesti olisi muuten liian iso
    bool started_;
    std::string username_;
    boost::asio::deadline_timer timer_;
    bool use_verbose_logging_;
    bool stopped_; // kun yhteys suljetaan, laitetaan t‰h‰n true (tarvitaan asynkroonisen toiminnan takia asiakasohjelmassa)

    tcp_tools::binary_read_state binary_reading_state_; // T‰t‰ tilaa k‰ytet‰‰n binaari sanomien lukuun ja sen eri tilojen merkitsemiseen (jos luettavan sanoman 1. sana on kokonaisluku ja space on sen per‰ss‰)
    size_t binary_read_buffer_size_; // Binary sanoman alussa kerrotaan sanoman koko tavuissa, t‰h‰n talletetaan tuo koko
    std::vector<char> binary_read_buffer_; // Binary luvussa k‰ytet‰‰n puskurina t‰t‰, ja sen koon voi laittaa suoraan binary_read_buffer_size_:en mukaan
};
