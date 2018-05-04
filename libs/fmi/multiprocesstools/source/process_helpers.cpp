#include "process_helpers.h"
#include "process_tools.h"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/thread/thread.hpp> 
#include <boost/random.hpp>
#include <boost/math/special_functions/round.hpp>

namespace process_helpers
{

boost::random::mt19937 random_generator; // produces randomness out of thin air see pseudo-random number generators
boost::random::uniform_int_distribution<> task_grid_size_rand(3,8); 
boost::random::uniform_int_distribution<> point_count_rand(4, 15); 
boost::random::uniform_real_distribution<float> relative_place_rand(0.f, 1.f); 
boost::random::uniform_real_distribution<float> z_value_rand(0.f, 5.f); 

void remove_shared_memory_object(const std::string &name)
{
    log_message(std::string("Removing shared-memory-object with name: ") + name, logging::trivial::info);
    boost::interprocess::shared_memory_object::remove(name.c_str());
}

void starting_workers_log(int worker_count)
{
    std::string starting_workers_string = "Starting ";
    starting_workers_string += boost::lexical_cast<std::string>(worker_count);
    starting_workers_string += " worker(s)";
    log_message(starting_workers_string, logging::trivial::info);
}

worker_process_info_vector_t create_workers(int worker_count, const base_worker_process_start_info &base_info, const multi_process_pool_options &mpp_options_in)
{
    starting_workers_log(worker_count);
    worker_process_info_vector_t worker_process_infos;

    for(int i = 0; i < worker_count; i++)
    {
        create_worker_process(worker_process_infos, i, false, "Opening worker's running info", "Starting worker # ", base_info, mpp_options_in);
    }

    return worker_process_infos;
}

std::string make_worker_executable_full_path(const std::string &worker_executable_absolute_path, const std::string &worker_executable_name, const std::string &worker_project_name)
{
    if(!worker_executable_absolute_path.empty())
        return worker_executable_absolute_path;
    else if(boost::filesystem::exists(worker_executable_name))
    { // exe löytyi working hakemistosta, käytetään sitä
        return worker_executable_name;
    }
    else
    { // muuten oletetaan että exe löytyy ProcessPool-solutionin WorkerProcess-projektin rakennuspoluilta, joille asetuksien mukaan pitää rakentaa erilliset polut
        std::string executable_path = "..\\..\\..\\";
#ifdef _WIN64
        executable_path += "x64\\";
#else
        executable_path += "win32\\";
#endif
        executable_path += "ProcessPool\\";
        executable_path += worker_project_name;
        executable_path += "\\";
#ifdef _DEBUG
        executable_path += "Debug\\";
#else
        executable_path += "Release\\";
#endif
        executable_path += worker_executable_name;

        return executable_path;
    }
}

std::vector<std::string> make_worker_executable_base_args(const base_worker_process_start_info &base_info)
{
    std::vector<std::string> args;
    args.push_back("-n");
    args.push_back(base_info.worker_base_name_str_);
    args.push_back("-i");

    return args;
}

boost::shared_ptr<worker_process_info> create_worker_process(const std::string &executable_path, const std::vector<std::string> &base_args, int worker_index, const std::string &runnin_info_opening_start_message, const std::string &create_worker_start_message, const base_worker_process_start_info &base_info)
{
    std::string worker_index_string = boost::lexical_cast<std::string>(worker_index);
    std::vector<std::string> final_args(base_args);
    final_args.push_back(worker_index_string);
    if(!base_info.extra_args_.empty())
        final_args.insert(final_args.end(), base_info.extra_args_.begin(), base_info.extra_args_.end());

    log_message(runnin_info_opening_start_message, logging::trivial::info);
    boost::shared_ptr<worker_process_info> pInfo(new worker_process_info(worker_running_info_shared_base_name + worker_index_string));
    pInfo->running_info_.set_command(1); // 1= normaali työskentely
    pInfo->running_info_.set_counter(0); // nollataan counteri
    pInfo->final_name_ = base_info.worker_base_name_str_ + worker_index_string;
    pInfo->base_process_info_ = base_info;

    std::string worker_string = create_worker_start_message;
    worker_string += worker_index_string;
    log_message(worker_string, logging::trivial::info);

//    if(worker_index == 2)
//        throw std::runtime_error("Can't create worker with index 2");

    // child:ia ei saa ottaa talteen, koska sen destruktori kaataa ohjelman, kun se yrittää sulkea
    // process-handlen. Mystisesti ongelmia ei tapahdu, kun start_process-funktio palauttaa child:in
    // ja destruktoria kutsutaan heti tässä (ja kun sitä ei laiteta talteen mihinkään).
    /* pInfo->child_process_ = */ start_process(executable_path, final_args);
    return pInfo;
}

bool create_worker_process(worker_process_info_vector_t &worker_process_infos, int index, bool recreate_dead_process, const std::string &runnin_info_opening_start_message, const std::string &create_worker_start_message, const base_worker_process_start_info &base_info, const multi_process_pool_options &mpp_options_in)
{
    try
    {
        std::string executable = make_worker_executable_full_path(mpp_options_in.worker_executable_absolute_path, base_info.worker_executable_name_str_, base_info.worker_project_name_);
        std::vector<std::string> base_args = make_worker_executable_base_args(base_info);
        boost::shared_ptr<worker_process_info> process_info = create_worker_process(executable, base_args, index, runnin_info_opening_start_message, create_worker_start_message, base_info);
        if(recreate_dead_process)
            worker_process_infos[index] = process_info;
        else
            worker_process_infos.push_back(process_info);
        return true;
    }
    catch(std::exception &ex)
    {
        log_message(std::string("Failed to create worker process:\n") + ex.what(), logging::trivial::error);
    }
    catch(...)
    {
        log_message("Failed to create worker process for unknown reason", logging::trivial::error);
    }
    if(!recreate_dead_process)
        worker_process_infos.push_back(boost::shared_ptr<worker_process_info>()); // laitetaan tyhjä info tilalle, jotta workereiden indeksit sopivat paikkoihin vektorissa
    return false;
}

bool recreate_worker(worker_process_info_vector_t &worker_process_infos, int index, const std::string &runnin_info_opening_start_message, const std::string &create_worker_start_message, const multi_process_pool_options &mpp_options_in)
{
    return create_worker_process(worker_process_infos, index, true, runnin_info_opening_start_message, create_worker_start_message, worker_process_infos[index]->base_process_info_, mpp_options_in);
}

void make_heart_beat(worker_running_info_ptr_t &master_info)
{
    master_info->increase_counter();
}

void check_worker_heartbeats(worker_process_info_vector_t &worker_process_infos, const multi_process_pool_options &mpp_options_in)
{
    log_message("Checking worker heartbeats", logging::trivial::trace);
    for(size_t i = 0; i < worker_process_infos.size(); i++)
    {
        if(worker_process_infos[i])
        {
            if(!worker_process_infos[i]->has_died_)
            {
                if(!worker_process_infos[i]->heartbeat_checker_.check_counter(worker_process_infos[i]->running_info_))
                {
                    if(worker_process_infos[i]->heartbeat_checker_.is_heartbeat_expired())
                    {
                        std::string death_message = worker_process_infos[i]->final_name_;
                        death_message += " has died, no live signs in over ";
                        double elapsed_time_in_seconds = worker_process_infos[i]->heartbeat_checker_.elapsed_time_in_seconds();
                        death_message += boost::str(boost::format("%.1f") % elapsed_time_in_seconds);
                        death_message += " seconds";
                        log_message(death_message, logging::trivial::error);

                        // Yritetään luoda uusi worker prosessi
                        if(!recreate_worker(worker_process_infos, static_cast<int>(i), "Re-opening worker's running info", "Re-starting worker # ", mpp_options_in))
                        {
                            std::string mark_as_death_message = "Marking ";
                            mark_as_death_message += worker_process_infos[i]->final_name_;
                            mark_as_death_message += " as dead";
                            log_message(mark_as_death_message, logging::trivial::error);
                            worker_process_infos[i]->has_died_ = true; // jos ei onnistunut, merkataan nykyinen worker kuolleeksi
                        }
                    }
                }
            }
        }
    }
}

bool are_all_workers_dead(worker_process_info_vector_t &worker_process_infos)
{
    for(size_t i = 0; i < worker_process_infos.size(); i++)
    {
        if(worker_process_infos[i])
        {
            if(!worker_process_infos[i]->has_died_)
                return false; // jos yksikin worker on elossa, palauta false
        }
    }
    return true;
}

bool have_there_been_zero_work_in_time_limit(worker_process_info_vector_t &worker_process_infos, heartbeat_checker &no_tasks_timeout_timer)
{
    static size_t total_task_handled = 0;

    size_t current_total_task_handled = 0;
    for(size_t i = 0; i < worker_process_infos.size(); i++)
    {
        if(worker_process_infos[i])
        {
            current_total_task_handled += worker_process_infos[i]->running_info_.get_task_counter();
        }
    }

    if(total_task_handled != current_total_task_handled)
    {
        no_tasks_timeout_timer.reset_heartbeat(); // on ollut töitä, nollataan timer
        total_task_handled = current_total_task_handled; // annetaan current arvo staattiselle muuttujalle muistiin
        return false;
    }
    else
    {
        if(no_tasks_timeout_timer.is_heartbeat_expired())
            return true; // ei töitä tietyn ajan sisällä
        else
            return false; // time-out ei ole vielä päällä
    }
}

void add_point_values_to_vectors(float x, float y, float z, std::vector<float> &x_values, std::vector<float> &y_values, std::vector<float> &z_values)
{
    x_values.push_back(x);
    y_values.push_back(y);
    z_values.push_back(z);
}

void fill_work_queue(task_queue_ptr_t &workqueue, task_allocator_holder_ptr_t &allocator_holder_ptr, size_t &running_task_index)
{
    log_message("Filling work_queue with tasks", logging::trivial::info);

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

    task_structure task(*allocator_holder_ptr->float_allocator_.get(), *allocator_holder_ptr->char_allocator_.get(), 1, 1, job_time, g_relative_area_string, 4, 5, x_values, y_values, z_values, g_sample_guid, gridding_function);
    workqueue->push_task(task);
    task = task_structure(*allocator_holder_ptr->float_allocator_.get(), *allocator_holder_ptr->char_allocator_.get(), 1, 2, job_time, g_relative_area_string, 5, 3, x_values, y_values, z_values, g_sample_guid, gridding_function);
    workqueue->push_task(task);
    add_point_values_to_vectors(0.5f, 0.42f, -0.3f, x_values, y_values, z_values);
    task = task_structure(*allocator_holder_ptr->float_allocator_.get(), *allocator_holder_ptr->char_allocator_.get(), 1, 3, job_time, g_relative_area_string, 6, 4, x_values, y_values, z_values, g_sample_guid, gridding_function);
    workqueue->push_task(task);
    task = task_structure(*allocator_holder_ptr->float_allocator_.get(), *allocator_holder_ptr->char_allocator_.get(), 1, 4, job_time, g_relative_area_string, 2, 3, x_values, y_values, z_values, g_sample_guid, gridding_function);
    workqueue->push_task(task);
    add_point_values_to_vectors(0.8f, 0.72f, 0.73f, x_values, y_values, z_values);
    task = task_structure(*allocator_holder_ptr->float_allocator_.get(), *allocator_holder_ptr->char_allocator_.get(), 1, 5, job_time, g_relative_area_string, 5, 4, x_values, y_values, z_values, g_sample_guid, gridding_function);
    workqueue->push_task(task);
    task = task_structure(*allocator_holder_ptr->float_allocator_.get(), *allocator_holder_ptr->char_allocator_.get(), 1, 6, job_time, g_relative_area_string, 3, 3, x_values, y_values, z_values, g_sample_guid, gridding_function);
    workqueue->push_task(task);
    add_point_values_to_vectors(0.23f, 0.87f, 1.3f, x_values, y_values, z_values);
    task = task_structure(*allocator_holder_ptr->float_allocator_.get(), *allocator_holder_ptr->char_allocator_.get(), 1, 7, job_time, g_relative_area_string, 5, 6, x_values, y_values, z_values, g_sample_guid, gridding_function);
    workqueue->push_task(task);

    running_task_index = workqueue->size() + 1;
}

void make_random_point_values(std::vector<float> &x_values, std::vector<float> &y_values, std::vector<float> &z_values)
{
    size_t point_count = point_count_rand(random_generator);
    for(size_t i = 0; i < point_count; i++)
    {
        x_values.push_back(relative_place_rand(random_generator));
        y_values.push_back(relative_place_rand(random_generator));
        z_values.push_back(z_value_rand(random_generator));
    }
}

task_structure generate_random_task(task_allocator_holder_ptr_t &allocator_holder_ptr, size_t &running_task_index)
{
    size_t size_x = task_grid_size_rand(random_generator);
    size_t size_y = task_grid_size_rand(random_generator);
    std::vector<float> x_values;
    std::vector<float> y_values;
    std::vector<float> z_values;
    make_random_point_values(x_values, y_values, z_values);
    NFmiStaticTime localTime;
    std::time_t job_time = localTime.EpochTime();
    int gridding_function = 2; // kFmiXuGriddingLocalFitCalc

    return task_structure(*allocator_holder_ptr->float_allocator_.get(), *allocator_holder_ptr->char_allocator_.get(), 2, running_task_index++, job_time, g_relative_area_string, size_x, size_y, x_values, y_values, z_values, g_sample_guid, gridding_function);
}

void log_workqueue_size(task_queue_ptr_t &workqueue, logging::trivial::severity_level log_level)
{
    std::string queue_size_message = "There were ";
    queue_size_message += boost::lexical_cast<std::string>(workqueue->size());
    queue_size_message += " tasks in work queue";
    log_message(queue_size_message, log_level);
}

void generate_random_tasks(task_queue_ptr_t &workqueue, task_allocator_holder_ptr_t &allocator_holder_ptr, int count, size_t &running_task_index)
{
    if(count)
    {
        log_workqueue_size(workqueue, logging::trivial::info);

        std::string generate_message = "Generate ";
        generate_message += boost::lexical_cast<std::string>(count);
        generate_message += " new random tasks";
        log_message(generate_message, logging::trivial::info);
        for(int i = 0; i < count; i++)
        {
            workqueue->push_task(generate_random_task(allocator_holder_ptr, running_task_index));
        }

        log_workqueue_size(workqueue, logging::trivial::debug);
    }
}

void log_and_wait(const std::string &base_message, float wait_time_in_seconds)
{
    std::string waiting_message = base_message;
    waiting_message += " ";
    waiting_message += boost::str(boost::format("%.1f") % wait_time_in_seconds);
    waiting_message += " seconds";
    log_message(waiting_message, logging::trivial::info);
    boost::this_thread::sleep(boost::posix_time::milliseconds(boost::math::iround(wait_time_in_seconds * 1000)));
}

void report_worker_activity_on_close(worker_process_info_vector_t &worker_process_infos)
{
    for(size_t i = 0; i < worker_process_infos.size(); i++)
    {
        std::string counter_string = "Worker ";
        counter_string += boost::lexical_cast<std::string>(i);
        counter_string += " had counter value: ";
        if(worker_process_infos[i])
            counter_string += boost::lexical_cast<std::string>(worker_process_infos[i]->running_info_.get_counter());
        else
            counter_string += "-";
        log_message(counter_string, logging::trivial::info);
    }
}

void stop_workers(worker_process_info_vector_t &worker_process_infos)
{
    log_message("Give workers close command", logging::trivial::info);
    for(size_t i = 0; i < worker_process_infos.size(); i++)
    {
        if(worker_process_infos[i])
            worker_process_infos[i]->running_info_.set_command(2);
    }
}

bool log_result(result_holder_t &result, const std::string &log_message_start_string, logging::trivial::severity_level log_level, bool short_log_string)
{
    if(result)
    {
        std::string result_string = log_message_start_string;
        result_string += boost::lexical_cast<std::string>(result->job_index_);
        result_string += ", data-time-index: ";
        result_string += boost::lexical_cast<std::string>(result->data_time_index_);
        result_string += ", job-time-t: ";
        result_string += boost::lexical_cast<std::string>(result->job_time_t_);
        result_string += ", guid: ";
        result_string += std::string(result->smartmet_guid_.begin(), result->smartmet_guid_.end());
        result_string += ", x: ";
        result_string += boost::lexical_cast<std::string>(result->size_x_);
        result_string += ", y: ";
        result_string += boost::lexical_cast<std::string>(result->size_y_);
        if(!short_log_string)
        {
            result_string += ", values: \n";
            size_t counter = 0;
            for(size_t j = 0; j < result->size_y_; j++)
            {
                for(size_t i = 0; i < result->size_x_; i++)
                {
                    result_string += boost::str(boost::format("%5.1f") % result->values_[counter++]);
                    if(i < result->size_x_ - 1) // viimeisen luvun jälkeen ei laiteta pilkkua
                        result_string += ", ";
                }
                if(j < result->size_y_ - 1) // viimeisen rivin jälkeen ei laiteta rivin vaihtoa, lokitus tekee sen puolesta
                    result_string += "\n";
            }
        }
        log_message(result_string, log_level);
        return true;
    }
    else
        return false;
}

void log_results(result_queue_ptr_t &work_result_queue, const std::string &log_message_start_string, logging::trivial::severity_level log_level, bool short_log_string)
{
    for(;;)
    {
        result_holder_t result = work_result_queue->try_pop_task();
        if(!log_result(result, log_message_start_string, log_level, short_log_string))
            break;
    }
}

gridding_result::gridding_result(const result_holder_t &result_holder)
:job_index_(0)
,data_time_index_(0)
,job_time_t_(0)
,size_x_(0)
,size_y_(0)
,smartmet_guid_()
,values_()
{
    if(result_holder)
    {
        job_index_ = result_holder->job_index_;
        data_time_index_ = result_holder->data_time_index_;
        job_time_t_ = result_holder->job_time_t_;
        size_x_ = result_holder->size_x_;
        size_y_ = result_holder->size_y_;
        smartmet_guid_ = std::string(result_holder->smartmet_guid_.begin(), result_holder->smartmet_guid_.end());
        values_ = std::vector<float>(result_holder->values_.begin(), result_holder->values_.end());
    }
}

// Haetaan pois resultqueue:sta kaikki tietyn clientin työn tulokset.
// Ne jotka eivät ole samaa guid:ia, laitetaan takaisin.
gridding_results_t get_gridding_results(result_queue_ptr_t &resultqueue, const std::string &client_guid)
{
    gridding_results_t gridding_results;
    size_t queue_size = resultqueue->size();
    for(size_t i = 0; i < queue_size; i++)
    {
        result_holder_t result_holder = resultqueue->pop_task();
        if(result_holder)
        {
            std::string result_guid(result_holder->smartmet_guid_.begin(), result_holder->smartmet_guid_.end());
            if(client_guid == result_guid)
            {
                gridding_results.push_back(gridding_result_ptr_t(new gridding_result(result_holder)));
            }
            else
                resultqueue->push_task(result_holder.get()); // ei ollut halutun asiakkaan työntulos, laitetaan takaisin jonoon
        }
        else
            break; // jono on jo tyhjä, voidaan lopettaa
    }
    return gridding_results;
}

void clear_old_results_from_queue(result_queue_ptr_t &resultqueue)
{
    const time_t result_aging_limit_in_seconds = 180; // yli 3 minuutti vanhat tulokset heivataan roskiin tulosjonosta häiritsemästä
    NFmiStaticTime timeNow;
    time_t nowEpochTime = timeNow.EpochTime();
    size_t queue_size = resultqueue->size();
    for(size_t i = 0; i < queue_size; i++)
    {
        result_holder_t result_holder = resultqueue->pop_task();
        if(result_holder)
        {
            if(nowEpochTime - result_holder->job_time_t_ < result_aging_limit_in_seconds)
                resultqueue->push_task(result_holder.get()); // tulos ei ollut vielä tarpeeksi vanha, joten laitetaan se takaisin työjonoon
            else
                log_result(result_holder, "Removed old result from job-index: ", logging::trivial::info, true);
        }
        else
            break; // jono on jo tyhjä, voidaan lopettaa
    }
}

// Tehdään joku höpö työn tulos float vektoriin
work_result_t make_some_fake_result(const task_holder_t &task, result_allocator_holder_ptr_t &result_allocator_holder)
{
    size_t sizeX = task->size_x_;
    if(sizeX < 1)
        sizeX = 1;
    size_t sizeY = task->size_y_;
    if(sizeY < 1)
        sizeY = 1;
    size_t job_index = task->job_index_;
    size_t data_time_index = task->data_time_index_;
    size_t job_time_t = task->job_time_t_;
    std::string guid(task->smartmet_guid_.begin(), task->smartmet_guid_.end());
    std::vector<float> values(sizeX*sizeY, 0);
    bool even = job_index % 2 == 0;
    for(size_t i = 0; i < values.size(); i++)
    {
        if(even)
            values[i] = i * 1.5f;
        else
            values[values.size() - i - 1] = i * 1.5f;
    }
    return work_result_t(*result_allocator_holder->float_allocator_.get(), *result_allocator_holder->char_allocator_.get(), job_index, data_time_index, job_time_t, sizeX, sizeY, values, guid);
}

void log_work_queue(task_queue_ptr_t &workqueue)
{
    log_message("Following task(s) in work_queue", logging::trivial::info);
    for(;;)
    {
        task_holder_t task = workqueue->try_pop_task();
        if(task)
        {
            std::stringstream out;
            out << *task;
            log_message(out.str(), logging::trivial::info);
        }

        else
            break;
    }

}

} // namespace process_helpers
