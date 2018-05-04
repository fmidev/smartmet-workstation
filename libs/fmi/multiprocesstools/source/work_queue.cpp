
#include "work_queue.h"

namespace
{
    bool verbose_logging_ = false;
}

bool work_queue_verbose_logging(void)
{
    return verbose_logging_;
}

void work_queue_verbose_logging(bool newValue)
{
    verbose_logging_ = newValue;
}

std::ostream& operator <<(std::ostream &out, const struct task_structure &object)
{
    out << "task job-index: " << object.job_index_ << ", data-time-index: " << object.data_time_index_ ;
    if(verbose_logging_)
        out << ", job-time-t: " << object.job_time_t_ << ", relative-area: " << object.relative_area_string_ << ", size-X: " << object.size_x_ << ", size-Y: " << object.size_y_ << ", smet-guid: " << object.smartmet_guid_ << ", gridding-func: " << object.gridding_function_;
    return out;
}

std::ostream& operator <<(std::ostream &out, const struct work_result_structure &object)
{
    out << "work_result job-index: " << object.job_index_ << object.job_index_ << ", data-time-index: " << object.data_time_index_ << ", job-time-t: " << object.job_time_t_ << ", smet-guid: " << object.smartmet_guid_ << ", x: " << object.size_x_ << ", y: " << object.size_y_ << ", values: " << std::endl;
    if(object.values_.size() < 150)
        std::copy(object.values_.begin(), object.values_.end(), std::ostream_iterator<float>(out, ","));
    else
    { // jos vektori on iso, tulostetaan muutamia alusta, ..., muutamia lopusta
        const size_t count = 15;
        for(size_t i = 0; i < count; i++)
            out << object.values_[i] << ",";
        out << "...,";
        for(size_t i = object.values_.size() - count; i < object.values_.size(); i++)
            out << object.values_[i] << ",";
    }
    return out;
}
