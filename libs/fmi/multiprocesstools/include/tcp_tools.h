#pragma once

#include "logging.h"

#include <vector>
#include <list>

#include <boost/thread/mutex.hpp> 
#include <boost/random.hpp>
#include <boost/bind.hpp>

// N�m� apu funktiot on per�isin "Boost.Asio C++ Networking Programming" -kirjan
// esimerkeist�. Niiden tarkoitus pakottaa k�ytt�m��n Asio:n asynkronisia kutsuja
// 'oikein'. Eli esim. shared_from_this tekee kutsuvasta oliosta shared_ptr -otuksen
// ja siten olio pakotetaan olemaan elossa asynkroonisen kutsun ajan. En sin�ns� 
// ymm�rr� t�t� tekniikkaa, mutta joku on miettinyt n�it�.
#define MEMBER_FUNC(func)    boost::bind(&self_type::func, shared_from_this())
#define MEMBER_FUNC1(func,par1)    boost::bind(&self_type::func, shared_from_this(),par1)
#define MEMBER_FUNC2(func,par1,par2)    boost::bind(&self_type::func, shared_from_this(),par1,par2)

namespace tcp_tools
{
    enum binary_read_state
    {
        binary_read_state_no = 0, // ei k�ytet� ollenkaan binary siirtoja
        binary_read_state_searching, // etsit��n sanomasta binary osiota, eli 1. sana on kokonaisluku ja sen per�ss� on space
        binary_read_state_not_found, // ei l�ytynyt kokonaisluku + space -yhdistelm��, jatketaan normaalia lukua
        binary_read_state_found, // l�ytyi kokonaisluku + space -yhdistelm�, jatketaan binary luvulla
        binary_read_state_finished, // luettu binary sanoma onnistuneesti
        binary_read_state_failed // binary sanoman luku ep�onnistui
    };

    const std::string g_sample_guid("9991842d-9afb-4f85-832f-f0522d28dbe9");
    const std::string g_relative_area_string("0,0,1,1");
    const std::string g_new_task_to_worker_string("new_task:");
    const std::string g_new_tasks_from_client_string("new_tasks:");
    const std::string g_result_from_worker_string("result:");
    const std::string g_new_results_to_client_string("new_results:");
    const std::string g_error_string("error");
    const std::string g_client_login_base_string("client:");
    const std::string g_local_ip_address = "127.0.0.1";
    const char g_worker_name_index_separator = '#'; // master rakentaa eri workereiden nimet seuraavasti: base_name + separator + index eli esim. worker#1
    const int g_local_ip_port = 8001;
    const int g_postpone_ping_timeout_in_ms = 100;
    const uint32_t g_missing_random_seed = static_cast<uint32_t>(-1);
    const size_t g_fixed_size_read_buffer_size = 128*1024;
    const size_t g_search_message_start_max_chars = 20;

    void create_worker_process(const std::string &executable_path, const std::string &worker_name, const std::string &create_worker_start_message, const std::vector<std::string> &extra_args, bool add_command_line_flags);
    bool use_binary_transfer();
    void use_binary_transfer(bool new_value);
    bool find_from_start_of_string(const std::string &message, const std::string &searched_string, size_t search_max_chars);

    struct task_structure {

        task_structure(void);
        task_structure(std::size_t job_index, std::size_t data_time_index, std::size_t job_time_t, const std::string &relative_area_string, std::size_t size_x, std::size_t size_y, std::vector<float> &x_values, std::vector<float> &y_values, std::vector<float> &z_values, const std::string &smartmet_guid, int gridding_function);

        std::string to_string(void) const;

        std::size_t job_index_; // smartmetin sis�inen ty�indeksi
        std::size_t data_time_index_; // editoitavan datan aika-indeksi
        std::size_t job_time_t_; // milloin ty� on annettu, eli sekunteja kulunut sitten 1.1. 1970 (t�m�n avulla voidaan poistaa vanhentuneita roikkumaan j��neit� tuloksia, jos esim. ty�n antanut smartmet on kaatunut, tai lopettanut)
        std::string relative_area_string_; // tuloshilan relatiivinen alue stringin�, muotoa: left,top,right,bottom (HUOM! ei saa sis�lt�� spaceja!)
        std::size_t size_x_; // halutun tuloshilan koko x-suunnassa
        std::size_t size_y_; // halutun tuloshilan koko y-suunnassa
        int gridding_function_; // haluttu griddaus funktio
        std::vector<float> x_values_; // hilattavien pisteiden relatiiviset x-sijainnit
        std::vector<float> y_values_; // hilattavien pisteiden relatiiviset y-sijainnit
        std::vector<float> z_values_; // hilattavien pisteiden arvot eli ns. 'korkeuskentt�'
        std::string smartmet_guid_; // ty�n antaneen smartmet instanssin guid, esim. "9999842d-9afb-4f85-832f-f0522d28dbe9" (HUOM! ei saa sis�lt�� spaceja!)
    };

    std::ostream& operator <<(std::ostream &out, const task_structure &object);
    std::istream& operator >>(std::istream &in, task_structure &object);

    struct work_result_structure {

        work_result_structure(void);
        work_result_structure(std::size_t job_index, std::size_t data_time_index, std::size_t job_time_t, std::size_t size_x, std::size_t size_y, const std::vector<float> &values, const std::string &smartmet_guid);

        void set_results(std::size_t job_index, std::size_t data_time_index, std::size_t job_time_t, std::size_t size_x, std::size_t size_y, const std::vector<float> &values, const std::string &smartmet_guid);
        std::string to_string(void) const;
        std::string to_binary_string(const std::string &start_of_str) const;
        void from_binary_string(const std::string &binary_str);

        std::size_t job_index_; // smartmetin sis�inen ty�indeksi
        std::size_t data_time_index_; // editoitavan datan aika-indeksi
        std::size_t job_time_t_; // milloin ty� on annettu, eli sekunteja kulunut sitten 1.1. 1970 (t�m�n avulla voidaan poistaa vanhentuneita roikkumaan j��neit� tuloksia, jos esim. ty�n antanut smartmet on kaatunut, tai lopettanut)
        std::size_t size_x_; // halutun tuloshilan koko x-suunnassa
        std::size_t size_y_; // halutun tuloshilan koko y-suunnassa
        std::vector<float> values_; // hilauksen lopputulos vektorissa
        std::string smartmet_guid_; // ty�n antaneen smartmet instanssin guid

    };

    std::ostream& operator <<(std::ostream &out, const work_result_structure &object);
    std::istream& operator >>(std::istream &in, work_result_structure &object);

    template<typename Data>
    class concurrent_queue : public boost::noncopyable
    {
    public:
        typedef typename std::list<Data>::value_type value_type;

        concurrent_queue(void)
        :mutex_()
        ,data_queue_()
        {}

        size_t size(void) const
        {
            boost::mutex::scoped_lock lock(mutex_);
            return data_queue_.size();
        }

        void push(const Data &data_in)
        {
            boost::mutex::scoped_lock lock(mutex_);
            data_queue_.push_back(data_in);
        }

        bool try_pop(Data& popped_value)
        {
            boost::mutex::scoped_lock lock(mutex_);
            if(data_queue_.empty())
                return false;
        
            popped_value = data_queue_.front();
            data_queue_.pop_front();
            return true;
        }

        std::list<Data> get_basic_list_copy(bool clear_list = false)
        {
            boost::mutex::scoped_lock lock(mutex_);
            std::list<Data> list_copy(data_queue_);
            if(clear_list)
                data_queue_.clear();
            return list_copy; // en tied� olisinko voinut palauttaa t�ss� suoraan data_queue_:n, koska en tied� mit� tapahtuu (tehd��nk� ensin kopio palautettavasta queue:sta ja sitten lukko aukeaa ja palataan metodista, vai jotenkin eri j�rjestyksess�)
        }

        // Funktio, joka poistaa originaali listasta (data_queue_) kaikki halutut datat
        //  ja laittaa ne palautettavaan listaan (match_list).
        template<typename Pred>
        std::list<Data> remove_copy_list_if(const Pred &condition)
        {
            boost::mutex::scoped_lock lock(mutex_);
            std::list<Data> match_list;
            for(auto iter = data_queue_.begin(); iter != data_queue_.end(); )
            {
                if(condition(*iter))
                {
                    match_list.push_back(*iter);
                    iter = data_queue_.erase(iter);
                }
                else
                    ++iter;
            }
            return match_list;
        }

        // Funktio, joka poistaa originaali listasta (data_queue_) seuraavan datan
        // joka sopii predikaatti condition:in ehtoihin ja palauttaa sen
        // popped_data -parametrissa. Palauttaa true, jos l�ytyi data ja false jos ei.
        template<typename Pred>
        bool try_get_next(const Pred &condition, Data &popped_data)
        {
            boost::mutex::scoped_lock lock(mutex_);
            std::list<Data> match_list;
            for(auto iter = data_queue_.begin(); iter != data_queue_.end(); )
            {
                if(condition(*iter))
                {
                    popped_data = *iter; // l�ytyi haluttu data, otetaan se talteen
                    iter = data_queue_.erase(iter); // poistetaan l�ydetty listasta
                    return true;
                }
                else
                    ++iter;
            }
            return false;
        }

    private:
        mutable boost::mutex mutex_;
        std::list<Data> data_queue_;
    };

    void fill_work_queue(concurrent_queue<task_structure> &workqueue, size_t &running_task_index);
    task_structure generate_random_task(size_t &running_task_index);
    void generate_random_tasks(concurrent_queue<task_structure> &workqueue, int count, size_t &running_task_index);
    boost::random::mt19937& get_random_generator(void);
    void seed_random_generator(uint32_t base_seed = g_missing_random_seed, std::string used_hash_string = "");
    void add_point_values_to_vectors(float x, float y, float z, std::vector<float> &x_values, std::vector<float> &y_values, std::vector<float> &z_values);
    void make_random_point_values(std::vector<float> &x_values, std::vector<float> &y_values, std::vector<float> &z_values);

    template<typename List>
    static void log_list(const List &alist, logging::trivial::severity_level level)
    {
        if(alist.size() == 0)
            log_message("Logging given list: it was empty", level);
        else
        {
            log_message("Logging given list:", level);
            int i = 1;
            for(auto iter = alist.begin(); iter != alist.end(); ++iter)
            {
                std::stringstream out;
                out << i++ << ": " << *iter;
                log_message(out.str(), level);
            }
        }
    }

    template<typename Data>
    static void log_concurrent_queue(concurrent_queue<Data> &queue, logging::trivial::severity_level level)
    {
        log_list(queue.get_basic_list_copy(), level);
    }

    template<typename List>
    static void write_list(std::ostream &out, const List &alist)
    {
        if(alist.size())
        {
            int i = 0;
            for(auto iter = alist.begin(); iter != alist.end(); ++iter)
            {
                if(i++)
                    out << " "; // laitetaan space otusten v�liin
                out << *iter;
            }
        }
    }

    template<typename Data>
    static void write_concurrent_queue(std::ostream &out, concurrent_queue<Data> &queue)
    {
        write_list(out, queue.get_basic_list_copy());
    }

    template<typename Queue>
    static void read_queue(std::istream &in, Queue &queue)
    {
        for( ; ; )
        {
            try
            {
                Queue::value_type queue_item;
                in >> queue_item;
                if(in)
                    queue.push(queue_item);
                else
                    break;
            }
            catch(std::exception &)
            {
                break; // queue_item -luku heitt�� poikkeuksen, kun tullaan streamin loppuun
            }
            catch(...)
            {
                break; // queue_item -luku heitt�� poikkeuksen, kun tullaan streamin loppuun
            }
        }
    }

    bool make_some_fake_result(const task_structure &task_in, work_result_structure &work_result_out, bool use_verbose_logging);
    void make_some_fake_work(concurrent_queue<task_structure> &task_queue_in, concurrent_queue<work_result_structure> &result_queue_out, bool use_verbose_logging);
    bool is_timeout(const boost::posix_time::ptime checked_time, long long used_time_out, long long *time_lasted_in_ms_ = 0);
    size_t read_complete_check_one_char_at_time(const boost::system::error_code & err, size_t bytes, const std::vector<char> &fixed_size_buffer, bool &transmission_incomplete);
    size_t read_complete_check_look_for_binary_message(const boost::system::error_code & err, size_t bytes, const std::vector<char> &fixed_size_buffer, bool &transmission_incomplete, binary_read_state &binary_read_state, size_t &binary_read_byte_count);
    size_t binary_read_complete_check(const boost::system::error_code & err, size_t bytes, size_t binary_read_byte_count_fixed, binary_read_state &binary_read_state);
    void log_point_values(const tcp_tools::task_structure &task_in, const std::vector<float> &x_values, const std::vector<float> &y_values, const std::vector<float> &z_values);
    binary_read_state get_binary_reading_reset_state();
    void log_start_of_binary_message(const std::string &log_message_start, const std::string &real_binary_message, size_t max_chars_from_message, logging::trivial::severity_level log_level);

struct multi_process_tcp_pool_options
{
	multi_process_tcp_pool_options(void)
    :worker_name("default-worker-name")
    ,total_wait_time_limit_in_seconds(90)
    ,verbose_logging(false)
    ,worker_executable_absolute_path()
    {}

    std::string worker_name;
    double total_wait_time_limit_in_seconds; // Kuinka kauan SmartMet odottaa maksimissaan ett� joku annetty ty�-sarja (= kokonainen CP-editointi) valmistuu
    bool verbose_logging; // k�ytet��nk� multi-process lokituksessa pidempi� vai lyhyempi� lokitus tekstej�
    std::string worker_executable_absolute_path; // jos t�ss� on arvo, k�ytet��n t�t� worker-prosessin k�ynnistykseen, muuten polku rakennetaan toista kautta
};

} // tcp_tools
