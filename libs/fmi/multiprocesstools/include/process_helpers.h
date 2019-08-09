#pragma once

#include "worker_process_info.h"
#include "logging.h"

#include "NFmiStaticTime.h"

#include <boost/function.hpp>

#ifdef _MSC_VER
#pragma warning (disable : 4239)
#endif

namespace process_helpers
{

const size_t gKiloByte = 1024;
const size_t gMegaByte = gKiloByte*gKiloByte;
const size_t work_queue_segment_size = gMegaByte * 64;
const size_t work_result_queue_segment_size = gMegaByte * 128;
const std::string g_sample_guid("2571842d-9afb-4f85-832f-f0522d28dbe9");
const std::string g_relative_area_string("0,0,1,1");

void remove_shared_memory_object(const std::string &name);

template<typename QueueType>
QueueType make_queue(const std::string &queue_name, const std::string &queue_shared_name, size_t segment_size, bool client)
{
    if(!client)
    { // client ei saa yrittää ensin tuhota työjonoja eikä myöskään master, koska jos data on jo käytössä toisaalla, tämä tuhoaa sen ilman sen kummempia tarkasteluja
        //log_message(std::string("Remove earlier existing ") + queue_name + " Shared memory", logging::trivial::info);
        //boost::interprocess::shared_memory_object::remove(queue_shared_name.c_str());
    }

    std::string log_string("Constructing ");
    log_string += queue_name;
    log_string += ", with size of ";
    size_t size_in_mb = segment_size / gMegaByte;
    log_string += boost::lexical_cast<std::string>(size_in_mb);
    log_string += " MB";
    log_message(log_string, logging::trivial::info);
    return QueueType(new QueueType::element_type(queue_shared_name, boost::interprocess::open_or_create, segment_size));
}

struct multi_process_pool_options
{
	multi_process_pool_options(void)
    :task_queue_size_in_bytes(work_queue_segment_size)
    ,result_queue_size_in_bytes(work_result_queue_segment_size)
    ,total_wait_time_limit_in_seconds(90)
    ,verbose_logging(false)
    ,worker_executable_absolute_path()
    {}

    size_t task_queue_size_in_bytes; // kuinka suuri tila varataan task-queuelle shared-memoryyn [B]
    size_t result_queue_size_in_bytes; // kuinka suuri tila varataan result-queuelle shared-memoryyn [B]
    double total_wait_time_limit_in_seconds; // Kuinka kauan SmartMet odottaa maksimissaan että joku annetty työ-sarja (= kokonainen CP-editointi) valmistuu
    bool verbose_logging; // käytetäänkö multi-process lokituksessa pidempiä vai lyhyempiä lokitus tekstejä
    std::string worker_executable_absolute_path; // jos tässä on arvo, käytetään tätä worker-prosessin käynnistykseen, muuten polku rakennetaan toista kautta
};


void add_point_values_to_vectors(float x, float y, float z, std::vector<float> &x_values, std::vector<float> &y_values, std::vector<float> &z_values);
void starting_workers_log(int worker_count);
std::string make_worker_executable_full_path(const std::string &worker_executable_absolute_path, const std::string &worker_executable_name, const std::string &worker_project_name);

void log_workqueue_size(task_queue_ptr_t &workqueue, logging::trivial::severity_level log_level);
void log_and_wait(const std::string &base_message, float wait_time_in_seconds);
void report_worker_activity_on_close(worker_process_info_vector_t &worker_process_infos);
void stop_workers(worker_process_info_vector_t &worker_process_infos);
bool log_result(result_holder_t &result, const std::string &log_message_start_string, logging::trivial::severity_level log_level, bool short_log_string = false);
void log_results(result_queue_ptr_t &work_result_queue, const std::string &log_message_start_string, logging::trivial::severity_level log_level, bool short_log_string = false);

template<typename ResultType>
class result_allocator_holder
{
public:
    typedef typename ResultType::char_allocator_type char_allocator_type;
    typedef typename boost::shared_ptr<char_allocator_type> char_allocator_ptr_type;
    typedef typename ResultType::float_allocator_type float_allocator_type;
    typedef typename boost::shared_ptr<float_allocator_type> float_allocator_ptr_type;

    result_allocator_holder(void)
    :char_allocator_()
    ,float_allocator_()
    {}

    result_allocator_holder(char_allocator_ptr_type &char_allocator, float_allocator_ptr_type &float_allocator)
    :char_allocator_(char_allocator)
    ,float_allocator_(float_allocator)
    {}

    char_allocator_ptr_type char_allocator_;
    float_allocator_ptr_type float_allocator_;

private:
    result_allocator_holder& operator=(const result_allocator_holder &);
};

typedef result_allocator_holder<work_result_t> result_allocator_holder_t;
typedef boost::shared_ptr<result_allocator_holder_t> result_allocator_holder_ptr_t;
typedef result_allocator_holder<task_structure> task_allocator_holder_t;
typedef boost::shared_ptr<task_allocator_holder_t> task_allocator_holder_ptr_t;

void fill_work_queue(task_queue_ptr_t &workqueue, task_allocator_holder_ptr_t &allocator_holder_ptr, size_t &running_task_index);
task_structure generate_random_task(task_allocator_holder_ptr_t &allocator_holder_ptr, size_t &running_task_index);
void generate_random_tasks(task_queue_ptr_t &workqueue, task_allocator_holder_ptr_t &allocator_holder_ptr, int count, size_t &running_task_index);
void make_random_point_values(std::vector<float> &x_values, std::vector<float> &y_values, std::vector<float> &z_values);

template<typename TaskQueue, typename ResultQueue>
class master_process_stuff
{
public:
    typedef TaskQueue task_queue_type;
    typedef boost::shared_ptr<TaskQueue> task_queue_ptr_type;
    typedef typename TaskQueue::task_type task_type;
    typedef typename task_type::char_allocator_type char_allocator_type;
    typedef typename task_type::float_allocator_type float_allocator_type;
    typedef ResultQueue result_queue_type;
    typedef boost::shared_ptr<ResultQueue> result_queue_ptr_type;
    typedef result_allocator_holder<task_type> result_allocator_holder_type;
    typedef boost::shared_ptr<result_allocator_holder_type> result_allocator_holder_ptr_type;

    // Tämä on master-konstruktori
//    master_process_stuff(int worker_count, size_t &running_task_index, const base_worker_process_start_info &base_info, bool generate_start_fake_tasks)
    master_process_stuff(int worker_count, const base_worker_process_start_info &base_info, const multi_process_pool_options &mpp_options_in)
    :mpp_options(mpp_options_in)
    ,workqueue()
    ,work_result_queue()
    ,master_info()
    ,allocator_holder()
    ,worker_process_infos()
    ,i_am_client(false)
    {
        workqueue = make_queue<task_queue_ptr_type>("work-queue", work_queue_shared_name, mpp_options.task_queue_size_in_bytes, i_am_client);
        work_result_queue = make_queue<result_queue_ptr_type>("work-result-queue", work_result_queue_shared_name, mpp_options.result_queue_size_in_bytes, i_am_client);
        master_info = worker_running_info_ptr_t(new worker_running_info(master_running_info_shared_base_name, boost::interprocess::open_or_create, 1024));
        master_info->set_counter(0);
        allocator_holder = result_allocator_holder_ptr_type(new result_allocator_holder_type());
        allocator_holder->char_allocator_ = boost::shared_ptr<char_allocator_type>(new char_allocator_type(workqueue->get_segment()->get_segment_manager()));
        allocator_holder->float_allocator_ = boost::shared_ptr<float_allocator_type>(new float_allocator_type(workqueue->get_segment()->get_segment_manager()));
//        if(generate_start_fake_tasks)
//            fill_work_queue(workqueue, allocator_holder, running_task_index);
        worker_process_infos = create_workers(worker_count, base_info, mpp_options_in);
    }

    // Tämä on client-konstruktori (SmartMetille)
    master_process_stuff(const multi_process_pool_options &mpp_options_in)
    :mpp_options(mpp_options_in)
    ,workqueue()
    ,work_result_queue()
    ,master_info()
    ,allocator_holder()
    ,worker_process_infos()
    ,i_am_client(true)
    {
        workqueue = make_queue<task_queue_ptr_type>("work-queue", work_queue_shared_name, mpp_options.task_queue_size_in_bytes, i_am_client);
        work_result_queue = make_queue<result_queue_ptr_type>("work-result-queue", work_result_queue_shared_name, mpp_options.result_queue_size_in_bytes, i_am_client);
        allocator_holder = result_allocator_holder_ptr_type(new result_allocator_holder_type());
        allocator_holder->char_allocator_ = boost::shared_ptr<char_allocator_type>(new char_allocator_type(workqueue->get_segment()->get_segment_manager()));
        allocator_holder->float_allocator_ = boost::shared_ptr<float_allocator_type>(new float_allocator_type(workqueue->get_segment()->get_segment_manager()));
    }

    multi_process_pool_options mpp_options;
    task_queue_ptr_type workqueue;
    result_queue_ptr_type work_result_queue;
    worker_running_info_ptr_t master_info;
    result_allocator_holder_ptr_type allocator_holder;
    worker_process_info_vector_t worker_process_infos;
    bool i_am_client; // client-ohjelma (Smartmet) ei tarvitse kaikkea dataa
};

template<typename MasterProcessStuff>
void add_new_task(task_queue_ptr_t &workqueue, MasterProcessStuff &master_process_data, std::vector<float> &x_values, std::vector<float> &y_values, std::vector<float> &z_values, size_t grid_size_x, size_t grid_size_y, const std::string &client_guid, size_t job_index, size_t data_time_index, size_t job_time_t, const std::string &relative_area_rect, int gridding_function)
{
    task_structure task(*master_process_data.allocator_holder->float_allocator_.get(), *master_process_data.allocator_holder->char_allocator_.get(), job_index, data_time_index, job_time_t, relative_area_rect, grid_size_x, grid_size_y, x_values, y_values, z_values, client_guid, gridding_function);
    workqueue->push_task(task);
}

template<typename MasterProcessStuff>
void fill_work_queue2(MasterProcessStuff &master_process_data)
{
    log_message("Filling work_queue (2nd version) with tasks", logging::trivial::info);

    std::vector<float> x_values;
    std::vector<float> y_values;
    std::vector<float> z_values;

    add_point_values_to_vectors(0.3f, 0.6f, 2.5f, x_values, y_values, z_values);
    add_point_values_to_vectors(0.7f, 0.15f, 0.3f, x_values, y_values, z_values);
    add_point_values_to_vectors(0.9f, 0.61f, 1.13f, x_values, y_values, z_values);
    add_point_values_to_vectors(0.15f, 0.21f, 1.83f, x_values, y_values, z_values);

    NFmiStaticTime localTime;
    std::time_t job_time = localTime.EpochTime();
    int gridding_function = 2; // kFmiXuGriddingLocalFitCalc

    add_new_task(master_process_data.workqueue, master_process_data, x_values, y_values, z_values, 4, 5, g_sample_guid, 1, 1, job_time, g_relative_area_string, gridding_function);
    add_new_task(master_process_data.workqueue, master_process_data, x_values, y_values, z_values, 5, 3, g_sample_guid, 1, 2, job_time, g_relative_area_string, gridding_function);
    add_point_values_to_vectors(0.5f, 0.42f, -0.3f, x_values, y_values, z_values);
    add_new_task(master_process_data.workqueue, master_process_data, x_values, y_values, z_values, 6, 4, g_sample_guid, 1, 3, job_time, g_relative_area_string, gridding_function);
    add_new_task(master_process_data.workqueue, master_process_data, x_values, y_values, z_values, 2, 3, g_sample_guid, 1, 4, job_time, g_relative_area_string, gridding_function);
    add_point_values_to_vectors(0.8f, 0.72f, 0.73f, x_values, y_values, z_values);
    add_new_task(master_process_data.workqueue, master_process_data, x_values, y_values, z_values, 5, 4, g_sample_guid, 1, 5, job_time, g_relative_area_string, gridding_function);
    add_new_task(master_process_data.workqueue, master_process_data, x_values, y_values, z_values, 3, 3, g_sample_guid, 1, 6, job_time, g_relative_area_string, gridding_function);
    add_point_values_to_vectors(0.23f, 0.87f, 1.3f, x_values, y_values, z_values);
    add_new_task(master_process_data.workqueue, master_process_data, x_values, y_values, z_values, 5, 6, g_sample_guid, 1, 7, job_time, g_relative_area_string, gridding_function);
}

class gridding_result
{
public:
    gridding_result(const result_holder_t &result_holder);

    std::size_t job_index_; // smartmetin sisäinen työindeksi
    std::size_t data_time_index_; // editoitavan datan aika-indeksi
    std::size_t job_time_t_; // milloin työ on annettu, eli sekunteja kulunut sitten 1.1. 1970 (tämän avulla voidaan poistaa vanhentuneita roikkumaan jääneitä tuloksia, jos esim. työn antanut smartmet on kaatunut, tai lopettanut)
    std::size_t size_x_; // halutun tuloshilan koko x-suunnassa
    std::size_t size_y_; // halutun tuloshilan koko y-suunnassa
    std::string smartmet_guid_; // työn antaneen smartmet instanssin guid
    std::vector<float> values_; // hilauksen lopputulos vektorissa
};

typedef boost::shared_ptr<gridding_result> gridding_result_ptr_t;
typedef std::vector<gridding_result_ptr_t> gridding_results_t;

gridding_results_t get_gridding_results(result_queue_ptr_t &resultqueue, const std::string &client_guid);
void clear_old_results_from_queue(result_queue_ptr_t &resultqueue);

template<typename TaskQueue, typename ResultQueue>
class worker_process_stuff
{
public:
    typedef TaskQueue task_queue_type;
    typedef boost::shared_ptr<TaskQueue> task_queue_ptr_type;
    typedef typename TaskQueue::task_type task_type;
    typedef ResultQueue result_queue_type;
    typedef boost::shared_ptr<ResultQueue> result_queue_ptr_type;
    typedef typename ResultQueue::task_type result_type;

    typedef typename result_type::char_allocator_type char_allocator_type;
    typedef boost::shared_ptr<char_allocator_type> char_allocator_ptr_type;
    typedef typename result_type::float_allocator_type float_allocator_type;
    typedef boost::shared_ptr<float_allocator_type> float_allocator_ptr_type;

    typedef typename result_allocator_holder<result_type> result_allocator_holder_type;
    typedef typename boost::shared_ptr<result_allocator_holder_type> result_allocator_holder_ptr_type;

    worker_process_stuff(const std::string &index_string, const multi_process_pool_options &mpp_options_in)
    :mpp_options(mpp_options_in)
    {
        log_message("Opening work_queue", logging::trivial::info);
        workqueue = task_queue_ptr_type(new task_queue_type(work_queue_shared_name, boost::interprocess::open_only));

        log_message("Opening work_result_queue", logging::trivial::info);
        work_result_queue = result_queue_ptr_type(new result_queue_type(work_result_queue_shared_name, boost::interprocess::open_only));

        result_char_allocator = char_allocator_ptr_type(new char_allocator_type(work_result_queue->get_segment()->get_segment_manager()));
        result_float_allocator = float_allocator_ptr_type(new float_allocator_type(work_result_queue->get_segment()->get_segment_manager()));
        result_allocator_holder = result_allocator_holder_ptr_type(new result_allocator_holder_type(result_char_allocator, result_float_allocator));

        log_message("Opening worker_running_info", logging::trivial::info);
        running_info = worker_running_info_ptr_t(new worker_running_info(worker_running_info_shared_base_name + index_string, boost::interprocess::open_only));

        log_message("Opening master_running_info", logging::trivial::info);
        master_info = worker_running_info_ptr_t(new worker_running_info(master_running_info_shared_base_name, boost::interprocess::open_only));
    }

    multi_process_pool_options mpp_options;
    task_queue_ptr_type workqueue;
    result_queue_ptr_type work_result_queue;
    char_allocator_ptr_type result_char_allocator;
    float_allocator_ptr_type result_float_allocator;
    result_allocator_holder_ptr_type result_allocator_holder;
    worker_running_info_ptr_t running_info;
    worker_running_info_ptr_t master_info;
};

// Tehdään joku höpö työn tulos float vektoriin
work_result_t make_some_fake_result(const task_holder_t &task, result_allocator_holder_ptr_t &result_allocator_holder);

const std::size_t killer_work_time_in_ms = 3*5; // tämän pituinen työ tappaa workerprosessin testausmielessä do_some_fake_work-funktiossa

template<typename WorkerProcessStuff>
bool do_some_fake_work(WorkerProcessStuff &workerProcessStuff)
{
    task_holder_t &task = workerProcessStuff.workqueue->try_pop_task();
    result_queue_ptr_t &work_result_queue = workerProcessStuff.work_result_queue;
    result_allocator_holder_ptr_t &result_allocator_holder = workerProcessStuff.result_allocator_holder;
    if(task)
    {
        workerProcessStuff.running_info->increase_task_counter();
        std::stringstream out;
        out << *task;
        std::string task_string = "Starting to do task: ";
        task_string += out.str();
        log_message(task_string, logging::trivial::info);

        std::string task_estimate_string = "Estimated work time: ";
        size_t work_time_in_ms = task->size_x_ * task->size_y_;
        task_estimate_string += boost::lexical_cast<std::string>(work_time_in_ms);
        task_estimate_string += " ms";
        log_message(task_estimate_string, logging::trivial::info);

        if(work_time_in_ms == killer_work_time_in_ms)
            throw 1; // tietyn pituiset fake tehtävät kaatavat workerin

        boost::this_thread::sleep(boost::posix_time::milliseconds(work_time_in_ms));
        log_message("Task finished", logging::trivial::info);

        log_message("Putting results to result-queue", logging::trivial::info);
        work_result_queue->push_task(make_some_fake_result(task, result_allocator_holder));

        return true;
    }
    else
    {
        log_message("Given task was empty", logging::trivial::trace);
        return false;
    }
}

template<typename WorkerProcessStuff, typename WorkerFunction>
void do_work(WorkerProcessStuff &workerProcessStuff, WorkerFunction workerFunction)
{
    heartbeat_checker beat_checker(5000);
    log_message("Start working with tasks", logging::trivial::info);
    for(;;)
    {
        int command = workerProcessStuff.running_info->get_command();
        if(command != 1)
        {
            std::string command_string = "Command was not 1, it was ";
            command_string += boost::lexical_cast<std::string>(command);
            command_string += ", stopping working";
            log_message(command_string, logging::trivial::info);
            break;
        }

        if(!beat_checker.check_counter(*workerProcessStuff.master_info.get()))
        {
            if(beat_checker.is_heartbeat_expired())
            {
                log_message("Master's heartbeat is gone, stopping work", logging::trivial::info);
                break;
            }
        }

        if(!workerFunction(workerProcessStuff))
        { 
            // ei ollut töitä, nukutaan vähän
            boost::this_thread::sleep(boost::posix_time::milliseconds(50));
        }
        workerProcessStuff.running_info->increase_counter();
    }
//    log_message("All the tasks are done, stopping work", logging::trivial::info);
}

void log_work_queue(task_queue_ptr_t &workqueue);

} // namespace process_helpers
