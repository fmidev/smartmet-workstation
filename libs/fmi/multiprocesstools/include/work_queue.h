#pragma once

#include <boost/interprocess/managed_shared_memory.hpp>

#include <boost/interprocess/containers/deque.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include <boost/optional.hpp>

#ifdef _MSC_VER
#pragma warning (disable : 4503)
#endif

// Tällä versio stringillä erotellaan eri versioita käyttävät shared_memory:t (voi olla eri SmartMet versioita rinnan käytössä), 
// eli jos task_structure, work_result_structure tai jokin merkittävä rakenne muuttuu, kasvata numeroa.
const std::string work_queue_version_string = "ver1.0";

const std::string work_queue_shared_name = std::string("Processpool_work-queue_") + work_queue_version_string;
const std::string work_result_queue_shared_name = std::string("Processpool_work-result-queue_") + work_queue_version_string;
const std::string worker_running_info_shared_base_name = std::string("Processpool_worker-running-info_") + work_queue_version_string;
const std::string master_running_info_shared_base_name = std::string("Processpool_master-running-info_") + work_queue_version_string;
const std::string killer_task_name = "CRASH#&@'N'BURN"; // tämän niminen tehtävä aiheuttaa workerin äkillisen kuoleman

bool work_queue_verbose_logging(void);
void work_queue_verbose_logging(bool newValue);

struct task_structure {

typedef boost::interprocess::managed_shared_memory::segment_manager segment_manager_t;
typedef boost::interprocess::allocator<char, segment_manager_t> char_allocator_type; 
typedef boost::interprocess::basic_string<char, std::char_traits<char>, char_allocator_type> shared_memory_string;
typedef boost::interprocess::allocator<float, segment_manager_t> float_allocator_type; 
typedef boost::interprocess::vector<float, float_allocator_type> shared_memory_vector;

    task_structure(float_allocator_type &float_allocator, char_allocator_type &char_allocator, std::size_t job_index, std::size_t data_time_index, std::size_t job_time_t, const std::string &relative_area_string, std::size_t size_x, std::size_t size_y, std::vector<float> &x_values, std::vector<float> &y_values, std::vector<float> &z_values, const std::string &smartmet_guid, int gridding_function)
    :job_index_(job_index)
    ,data_time_index_(data_time_index)
    ,job_time_t_(job_time_t)
    ,relative_area_string_(relative_area_string.begin(), relative_area_string.end(), char_allocator)
    ,size_x_(size_x)
    ,size_y_(size_y)
    ,gridding_function_(gridding_function)
    ,x_values_(x_values.begin(), x_values.end(), float_allocator)
    ,y_values_(y_values.begin(), y_values.end(), float_allocator)
    ,z_values_(z_values.begin(), z_values.end(), float_allocator)
    ,smartmet_guid_(smartmet_guid.begin(), smartmet_guid.end(), char_allocator)
    {}

    std::size_t job_index_; // smartmetin sisäinen työindeksi
    std::size_t data_time_index_; // editoitavan datan aika-indeksi
    std::size_t job_time_t_; // milloin työ on annettu, eli sekunteja kulunut sitten 1.1. 1970 (tämän avulla voidaan poistaa vanhentuneita roikkumaan jääneitä tuloksia, jos esim. työn antanut smartmet on kaatunut, tai lopettanut)
    shared_memory_string relative_area_string_; // tuloshilan relatiivinen alue stringinä, muotoa: left,top,right,bottom
    std::size_t size_x_; // halutun tuloshilan koko x-suunnassa
    std::size_t size_y_; // halutun tuloshilan koko y-suunnassa
    int gridding_function_; // haluttu griddaus funktio
    shared_memory_vector x_values_; // hilattavien pisteiden relatiiviset x-sijainnit
    shared_memory_vector y_values_; // hilattavien pisteiden relatiiviset y-sijainnit
    shared_memory_vector z_values_; // hilattavien pisteiden arvot eli ns. 'korkeuskenttä'
    shared_memory_string smartmet_guid_; // työn antaneen smartmet instanssin guid
};

std::ostream& operator <<(std::ostream &out, const struct task_structure &object);

struct work_result_structure {

typedef boost::interprocess::managed_shared_memory::segment_manager segment_manager_t;
typedef boost::interprocess::allocator<char, segment_manager_t> char_allocator_type; 
typedef boost::interprocess::basic_string<char, std::char_traits<char>, char_allocator_type> shared_memory_string;
typedef boost::interprocess::allocator<float, segment_manager_t> float_allocator_type; 
typedef boost::interprocess::vector<float, float_allocator_type> shared_memory_vector;

    work_result_structure(float_allocator_type &float_allocator, char_allocator_type &char_allocator, std::size_t job_index, std::size_t data_time_index, std::size_t job_time_t, std::size_t size_x, std::size_t size_y, const std::vector<float> &values, const std::string &smartmet_guid)
    :job_index_(job_index)
    ,data_time_index_(data_time_index)
    ,job_time_t_(job_time_t)
    ,size_x_(size_x)
    ,size_y_(size_y)
    ,smartmet_guid_(smartmet_guid.begin(), smartmet_guid.end(), char_allocator)
    ,values_(values.begin(), values.end(), float_allocator)
    {}

    std::size_t job_index_; // smartmetin sisäinen työindeksi
    std::size_t data_time_index_; // editoitavan datan aika-indeksi
    std::size_t job_time_t_; // milloin työ on annettu, eli sekunteja kulunut sitten 1.1. 1970 (tämän avulla voidaan poistaa vanhentuneita roikkumaan jääneitä tuloksia, jos esim. työn antanut smartmet on kaatunut, tai lopettanut)
    std::size_t size_x_; // halutun tuloshilan koko x-suunnassa
    std::size_t size_y_; // halutun tuloshilan koko y-suunnassa
    shared_memory_string smartmet_guid_; // työn antaneen smartmet instanssin guid
    shared_memory_vector values_; // hilauksen lopputulos vektorissa

};

std::ostream& operator <<(std::ostream &out, const struct work_result_structure &object);

template<typename Task>
class work_queue {
public:
    typedef Task task_type;
    typedef boost::interprocess::managed_shared_memory managed_shared_memory_t;
    typedef boost::interprocess::allocator<task_type, managed_shared_memory_t::segment_manager> allocator_t;
    typedef boost::optional<task_type> optional_holder_t;

private:
    std::string base_shared_name_;
    managed_shared_memory_t segment_;
    const allocator_t allocator_;

    typedef boost::interprocess::deque<task_type, allocator_t>
        deque_t;

    typedef boost::interprocess::interprocess_mutex mutex_t;
    typedef boost::interprocess::interprocess_condition
        condition_t;
    typedef boost::interprocess::scoped_lock<mutex_t>
        scoped_lock_t;

    deque_t& tasks_;
    mutex_t& mutex_;
    boost::interprocess::interprocess_condition& cond_;

public:
    // Master process käyttää tätä (open or create)
    work_queue(const std::string &base_shared_name, const boost::interprocess::open_or_create_t &creation_mode, size_t segment_size)
        : base_shared_name_(base_shared_name)
        , segment_(
              creation_mode,
              base_shared_name.c_str(),
              segment_size
        )
        , allocator_(segment_.get_segment_manager())
        , tasks_(
            *segment_.find_or_construct<deque_t>
            (get_deque_name(base_shared_name).c_str())(allocator_)
        )
        , mutex_(
            *segment_.find_or_construct<mutex_t>
            (get_mutex_name(base_shared_name).c_str())()
        )
        , cond_(
            *segment_.find_or_construct<condition_t>
            (get_condition_name(base_shared_name).c_str())()
        )
    {}

    // Worker processit käyttää tätä (open only)
    work_queue(const std::string &base_shared_name, const boost::interprocess::open_only_t &creation_mode)
        : base_shared_name_(base_shared_name)
        , segment_(
              creation_mode,
              base_shared_name.c_str()
        )
        , allocator_(segment_.get_segment_manager())
        , tasks_(
            *segment_.find_or_construct<deque_t>
            (get_deque_name(base_shared_name).c_str())(allocator_)
        )
        , mutex_(
            *segment_.find_or_construct<mutex_t>
            (get_mutex_name(base_shared_name).c_str())()
        )
        , cond_(
            *segment_.find_or_construct<condition_t>
            (get_condition_name(base_shared_name).c_str())()
        )
    {}

    void push_task(const task_type& task) {
        scoped_lock_t lock(mutex_);
        tasks_.push_back(task);
        lock.unlock();
        cond_.notify_one();
    }

    optional_holder_t try_pop_task() {
        optional_holder_t ret;
        scoped_lock_t lock(mutex_);
        if (!tasks_.empty()) {
            ret = tasks_.front();
            tasks_.pop_front();
        }

        return ret;
    }

    task_type pop_task() {
        scoped_lock_t lock(mutex_);
        while (tasks_.empty()) {
            cond_.wait(lock);
        }

        task_type ret = tasks_.front();
        tasks_.pop_front();

        return ret;
    }

    bool empty(void) const
    {
        scoped_lock_t lock(mutex_);
        return tasks_.empty();
    }

    size_t size(void) const
    {
        scoped_lock_t lock(mutex_);
        return tasks_.size();
    }

    managed_shared_memory_t* get_segment(){
        return &segment_;
    }

    static std::string get_deque_name(const std::string &base_shared_name)
    {
        return base_shared_name+":deque";
    }
    static std::string get_mutex_name(const std::string &base_shared_name)
    {
        return base_shared_name+":mutex";
    }
    static std::string get_condition_name(const std::string &base_shared_name)
    {
        return base_shared_name+":condition";
    }

};


class worker_running_info {
public:
    typedef boost::interprocess::managed_shared_memory managed_shared_memory_t;

private:
    std::string base_shared_name_;
    managed_shared_memory_t segment_;

    int& command_; // Tämä tulee Masterilta workerille: 0=ei alustettu, 1=tee työtä, 2=lopeta prosessi
    int& counter_; // Worker kasvattaa tätä counteria esim. sekunnin välein, jos luku muuttuu, tämä kertoo Masterille että worker on vielä elossa
    int& task_counter_; // Worker kasvattaa tätä counteria kun se on saanut tehtävän, tämän avulla Master voi päätellä onko kukaan workereistä tehnyt mitään tiettyyn aikaan ja jos ei ole ollut tekemistä, se voi lopettaa kaiken toiminnan

public:
    // Master process käyttää tätä (open or create)
    worker_running_info(const std::string &base_shared_name, const boost::interprocess::open_or_create_t &creation_mode, size_t segment_size)
        : base_shared_name_(base_shared_name)
        , segment_(
              creation_mode,
              base_shared_name.c_str(),
              segment_size
        )
        , command_(
            *segment_.find_or_construct<int>
            (get_command_name(base_shared_name).c_str())()
        )
        , counter_(
            *segment_.find_or_construct<int>
            (get_counter_name(base_shared_name).c_str())()
        )
        , task_counter_(
            *segment_.find_or_construct<int>
            (get_task_counter_name(base_shared_name).c_str())()
        )
    {}

    // Worker processit käyttää tätä (open only)
    worker_running_info(const std::string &base_shared_name, const boost::interprocess::open_only_t &creation_mode)
        : base_shared_name_(base_shared_name)
        , segment_(
              creation_mode,
              base_shared_name.c_str()
        )
        , command_(
            *segment_.find_or_construct<int>
            (get_command_name(base_shared_name).c_str())()
        )
        , counter_(
            *segment_.find_or_construct<int>
            (get_counter_name(base_shared_name).c_str())()
        )
        , task_counter_(
            *segment_.find_or_construct<int>
            (get_task_counter_name(base_shared_name).c_str())()
        )
    {}

    void set_command(int command)
    {
        command_ = command;
    }
    int get_command(void) const
    {
        return command_;
    }
    void set_counter(int counter)
    {
        counter_ = counter;
    }
    int get_counter(void) const
    {
        return counter_;
    }
    int increase_counter(void) const
    {
        return counter_++;
    }
    int get_task_counter(void) const
    {
        return task_counter_;
    }
    int increase_task_counter(void) const
    {
        return task_counter_++;
    }

    static std::string get_command_name(const std::string &base_shared_name)
    {
        return base_shared_name+":command";
    }
    static std::string get_counter_name(const std::string &base_shared_name)
    {
        return base_shared_name+":counter";
    }
    static std::string get_task_counter_name(const std::string &base_shared_name)
    {
        return base_shared_name+":task_counter";
    }

};

typedef work_queue<task_structure> task_queue_t;
typedef boost::shared_ptr<task_queue_t> task_queue_ptr_t;
typedef work_queue<work_result_structure> result_queue_t;
typedef boost::shared_ptr<result_queue_t> result_queue_ptr_t;
typedef result_queue_ptr_t::element_type::task_type work_result_t;
typedef result_queue_ptr_t::element_type::optional_holder_t result_holder_t;
typedef task_queue_ptr_t::element_type::optional_holder_t task_holder_t;
typedef boost::shared_ptr<worker_running_info> worker_running_info_ptr_t;

/*
int main() {
    try {
        static const std::size_t max_tasks_count = 10000000;
        work_queue queue;

        boost::optional<task_structure> task = queue.try_pop_task();
        if (!task) {
            std::cout << "Filling data" << std::endl;
            task_structure ts;
            for (std::size_t i = 0; i < max_tasks_count; ++i) {
                ts.value_ = i;
                queue.push_task(ts);
            }
        } else {
            std::cout << "Gettinging data" << std::endl;
            assert(task->value_ == 0);
            for (std::size_t i = 1; i < max_tasks_count; ++i) {
                assert(queue.pop_task().value_ == i);
            }

            assert(!queue.try_pop_task());
        }
    } catch (const std::exception& e) {
        std::cout << e.what();
    }
}
*/
