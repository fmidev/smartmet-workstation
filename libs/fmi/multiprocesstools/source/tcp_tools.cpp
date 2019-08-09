
#include "stdafx.h"
#include "tcp_tools.h"
#include "logging.h"
#include "process_tools.h"

#include "NFmiStaticTime.h"
#include "NFmiGriddingProperties.h"

#include <chrono>

#include <boost/thread/thread.hpp> 
#include <boost/format.hpp> 
#include <boost/lexical_cast.hpp> 

namespace tcp_tools
{
    namespace
    {
        bool g_use_binary_transfer = true;
    }

boost::random::uniform_int_distribution<> task_grid_size_rand(3,8); 
boost::random::uniform_int_distribution<> point_count_rand(4, 15); 
boost::random::uniform_real_distribution<float> relative_place_rand(0.f, 1.f); 
boost::random::uniform_real_distribution<float> z_value_rand(0.f, 5.f); 

bool use_binary_transfer()
{
    return g_use_binary_transfer;
}

void use_binary_transfer(bool new_value)
{
    g_use_binary_transfer = new_value;
}

std::string make_argument_string(const std::vector<std::string>& final_args)
{
    std::string str;
    for(const auto& arg : final_args)
    {
        if(!str.empty())
            str += " ";
        str += arg;
    }
    return str;
}

void create_worker_process(const std::string &executable_path, const std::string &worker_name, const std::string &create_worker_start_message, const std::vector<std::string> &extra_args, bool add_command_line_flags)
{
    std::vector<std::string> final_args;
    if(add_command_line_flags)
        final_args.push_back("-n"); // MFC versio ottaa komentorivi optioita lipuilla, dos versio ottaa ilman
    final_args.push_back(worker_name);
    if(!extra_args.empty())
        final_args.insert(final_args.end(), extra_args.begin(), extra_args.end());

    std::string worker_string = create_worker_start_message;
    worker_string += worker_name;
    log_message(worker_string, logging::trivial::info);
    log_message(std::string("Worker exe path: ") + executable_path, logging::trivial::debug);
    log_message(std::string("Worker exe args: ") + make_argument_string(final_args), logging::trivial::debug);

    // child:ia ei saa ottaa talteen, se pitää irroittaa (detach), muuten homma ei toimi uudella boost:in process -kirjastolla.
    start_process(executable_path, final_args).detach();
}

// Etsitään message -stringin alusta (tai halutusta alusta, jos search_start_pos != 0), löytyykö
// etsittyä searched_string:iä. Jos message on tarpeeksi pitkä, ei etsitä koko stringiä läpi vaan vain haluttu osio
// eli search_max_chars:in pituinen osio.
bool find_from_start_of_string(const std::string &message, const std::string &searched_string, size_t search_max_chars)
{
    if(message.size() < search_max_chars * 3) // ei nähdä vaivaa tehdä uutta tarkasteltavaa osa stringia message:sta, jos sillä ei todella säästetä työtä
        return message.find(searched_string) != std::string::npos;
    else
    {
        std::string part_of_message(message.data(), search_max_chars);
        return part_of_message.find(searched_string) != std::string::npos;
    }
}

// ********************************************
// **** task_structure rakenne ****************
// ********************************************

task_structure::task_structure(void)
:job_index_(static_cast<size_t>(-1))
,data_time_index_(static_cast<size_t>(-1))
,job_time_t_(static_cast<size_t>(-1))
,relative_area_string_()
,size_x_(static_cast<size_t>(-1))
,size_y_(static_cast<size_t>(-1))
,x_values_()
,y_values_()
,z_values_()
,smartmet_guid_()
,gridding_properties_string_()
,cp_range_limit_relative_(kFloatMissing)
{}

task_structure::task_structure(std::size_t job_index, std::size_t data_time_index, std::size_t job_time_t, const std::string &relative_area_string, std::size_t size_x, std::size_t size_y, std::vector<float> &x_values, std::vector<float> &y_values, std::vector<float> &z_values, const std::string &smartmet_guid, const std::string &gridding_properties_string, float cp_range_limit_relative)
:job_index_(job_index)
,data_time_index_(data_time_index)
,job_time_t_(job_time_t)
,relative_area_string_(relative_area_string)
,size_x_(size_x)
,size_y_(size_y)
,x_values_(x_values)
,y_values_(y_values)
,z_values_(z_values)
,smartmet_guid_(smartmet_guid)
,gridding_properties_string_(gridding_properties_string)
,cp_range_limit_relative_(cp_range_limit_relative)
{}

std::string task_structure::to_string(void) const
{
    std::ostringstream out;
    out << *this;
    return out.str();
}

void write_float_vector(std::ostream &out, const std::vector<float> &values)
{
    out << values.size();
    if(values.size())
    {
        out << ":";
        for(size_t i = 0; i < values.size(); i++)
        {
            if(i > 0)
                out << ","; // kaikkien muiden lukujen eteen laitetaan ','-merkki paitsi 1.
            out << values[i];
        }
    }
}

void read_float_vector(std::istream &in, std::vector<float> &values)
{
    size_t vector_size = 0;
    in >> vector_size;
    if(!in)
        throw std::runtime_error("Error while reading vector size from stream");
    if(vector_size == 0)
        values.clear();
    else
    {
        char separator_reader = 0; // tähän luetaan separaattoreina käytettyjä ':'- ja ',' -merkkejä (: on koon jälkeinen separaattori ja , on numeroiden välinen).
        float value = 0;
        values.resize(vector_size);
        for(size_t i = 0; i < vector_size; i++)
        {
            in >> separator_reader;
            in >> value;
            values[i] = value;
            if(!in)
                throw std::runtime_error("Error while reading vector values from stream");
        }
    }
}

// HUOM! vektorin stringeissä ei saa olla white-spaceja!!!
void WriteExtraOptions(std::ostream &out, const std::vector<std::string> &extraOptionsInSingleWords)
{
    out << extraOptionsInSingleWords.size();
    for(const auto &word : extraOptionsInSingleWords)
    {
        out << " " << word;
    }
}

void ReadExtraOptions(std::istream &in, std::vector<std::string> &extraOptionsInSingleWords)
{
    size_t wordCount = 0;
    in >> wordCount;
    if(!in)
        throw std::runtime_error("Error while reading extra options size from stream");
    for(auto index = 0; index < wordCount; index++)
    {
        std::string word;
        in >> word;
        if(!in)
            throw std::runtime_error("Error while reading extra option from stream");
        extraOptionsInSingleWords.push_back(word);
    }
}

// Tulostetaan task_structure -olio streamiin. 
// Ei saa sisältää \n -merkkejä, tämä otus laitetaan tcp:llä masterista workeriin.
std::ostream& operator<<(std::ostream &out, const task_structure &object)
{
    out << object.job_index_ << " " << object.data_time_index_ << " " << object.job_time_t_ << " ";
    out << object.relative_area_string_ << " "; // HUOM! relative_area_string_ ei saa sisältää spaceja!!
    out << object.size_x_ << " " << object.size_y_ << " ";
    write_float_vector(out, object.x_values_);
    out << " ";
    write_float_vector(out, object.y_values_);
    out << " ";
    write_float_vector(out, object.z_values_);
    out << " ";
    out << object.smartmet_guid_ << " "; // HUOM! smartmet_guid_ ei saa sisältää spaceja!!
    out << object.gridding_properties_string_ << " ";
    out << object.cp_range_limit_relative_ << " ";

    // Varaudutaan siihen että tulevaisuudessa tulee lisää parametreja (= kokonaisia sanoja).
    // Laitetaan tähän lukumäärä kuinka monta ylimääräistä sanaa tällä hetkellä on
    std::vector<std::string> extraOptionsInSingleWords;
////    extraOptionsInSingleWords.push_back("extra-word-1");
    WriteExtraOptions(out, extraOptionsInSingleWords);

    return out;
}

// Luetaan task_structure -olio streamista. 
std::istream& operator >>(std::istream &in, task_structure &object)
{
    in >> object.job_index_;
    if(!in)
        throw std::runtime_error("Error while reading job_index_ from stream");
    in >> object.data_time_index_;
    if(!in)
        throw std::runtime_error("Error while reading data_time_index_ from stream");
    in >> object.job_time_t_;
    if(!in)
        throw std::runtime_error("Error while reading job_time_t_ from stream");
    in >> object.relative_area_string_; // HUOM! relative_area_string_ ei saa sisältää spaceja, muuten se katkeaa tässä luvussa!!
    if(!in)
        throw std::runtime_error("Error while reading relative_area_string_ from stream");
    in >> object.size_x_;
    if(!in)
        throw std::runtime_error("Error while reading size_x_ from stream");
    in >> object.size_y_;
    if(!in)
        throw std::runtime_error("Error while reading size_y_ from stream");
    read_float_vector(in, object.x_values_);
    read_float_vector(in, object.y_values_);
    read_float_vector(in, object.z_values_);
    in >> object.smartmet_guid_; // HUOM! smartmet_guid_ -stringi ei saa sisältää spaceja, muuten se katkeaa tässä luvussa!!
    if(!in)
        throw std::runtime_error("Error while reading smartmet_guid_ from stream");
    in >> object.gridding_properties_string_;
    if(!in)
        throw std::runtime_error("Error while reading gridding_properties_string_ from stream");
    in >> object.cp_range_limit_relative_;
    if(!in)
        throw std::runtime_error("Error while reading cp_range_limit_relative_ from stream");

    // Lopuksi vielä luetaan ylimääräiset optiot (sanat) ja versiosta riippuen niille tehdään jotain tai ei.
    // Tämän pitäisi siis olla taaksepäin ja eteenpäin yhteensopiva optioiden lisäys.
    std::vector<std::string> extraOptionsInSingleWords;
    ReadExtraOptions(in, extraOptionsInSingleWords);

    return in;
}

// ********************************************
// **** work_result_structure rakenne *********
// ********************************************

work_result_structure::work_result_structure(void)
:job_index_(static_cast<size_t>(-1))
,data_time_index_(static_cast<size_t>(-1))
,job_time_t_(static_cast<size_t>(-1))
,size_x_(static_cast<size_t>(-1))
,size_y_(static_cast<size_t>(-1))
,values_()
,smartmet_guid_()
{}

work_result_structure::work_result_structure(std::size_t job_index, std::size_t data_time_index, std::size_t job_time_t, std::size_t size_x, std::size_t size_y, const std::vector<float> &values, const std::string &smartmet_guid)
:job_index_(job_index)
,data_time_index_(data_time_index)
,job_time_t_(job_time_t)
,size_x_(size_x)
,size_y_(size_y)
,values_(values)
,smartmet_guid_(smartmet_guid)
{}

void work_result_structure::set_results(std::size_t job_index, std::size_t data_time_index, std::size_t job_time_t, std::size_t size_x, std::size_t size_y, const std::vector<float> &values, const std::string &smartmet_guid)
{
    job_index_ = job_index;
    data_time_index_ = data_time_index;
    job_time_t_ = job_time_t;
    size_x_ = size_x;
    size_y_ = size_y;
    values_ = values;
    smartmet_guid_ = smartmet_guid;
}


std::string work_result_structure::to_string(void) const
{
    std::ostringstream out;
    out << *this;
    return out.str();
}

// Jos binary sanomaan pitää saada alkuun jotain, se annetaan start_of_str -parametrissa.
std::string work_result_structure::to_binary_string(const std::string &start_of_str) const
{
    // Tähän ostream:iin talletetaan result-structin perus asiat, jonka avulla saadaan myös tietää lopullisen sanoman binääri osion pituus tavuina
    std::ostringstream out;
    out << start_of_str; // Huom!, ei space: perään, se pitää antaa osana start_of_str:ia.
    out << job_index_ << " " << data_time_index_ << " " << job_time_t_ << " ";
    out << size_x_ << " " << size_y_ << " " << smartmet_guid_ << " "; // HUOM! smartmet_guid_ ei saa sisältää spaceja!!
    size_t size_of_values_in_bytes = values_.size() * sizeof(float);
    out << size_of_values_in_bytes << " ";
    out.write(reinterpret_cast<const char*>(values_.data()), size_of_values_in_bytes);
    std::string base_binary_str(out.str());
    // Lopullisen binääri viestin rakenne:
    // 'viestin pituus tavuina (spacen jälkeen)' space 'loppuviesti'
    std::string final_binary_str = boost::lexical_cast<std::string>(base_binary_str.size());
    final_binary_str += " " + base_binary_str;
    return final_binary_str;
}

// binary_str -parametri on muotoa: 'viestin pituus tavuina (spacen jälkeen)' space 'loppuviesti'
void work_result_structure::from_binary_string(const std::string &binary_str)
{
    std::istringstream in(binary_str);
    std::string first_word_dummy; // binary sanoman 1. sana on joko tavumäärä tai joku muu sana, eli luemme ja ohitamme 1. sanan
    in >> first_word_dummy; // luetaan vain pois alta, tällä ei tehdä mitään
    if(!in)
        throw std::runtime_error("Error in from_binary_string-function while reading first_word_dummy from stream");
    char space_char = 0;
    in.get(space_char); // luetaan myös space pois häiritsemästä
    if(!in)
        throw std::runtime_error("Error in from_binary_string-function while reading space-separator from stream");

    in >> job_index_;
    if(!in)
        throw std::runtime_error("Error in from_binary_string-function while reading job_index_ from stream");
    in >> data_time_index_;
    if(!in)
        throw std::runtime_error("Error in from_binary_string-function while reading data_time_index_ from stream");
    in >> job_time_t_;
    if(!in)
        throw std::runtime_error("Error in from_binary_string-function while reading job_time_t_ from stream");
    in >> size_x_;
    if(!in)
        throw std::runtime_error("Error in from_binary_string-function while reading size_x_ from stream");
    in >> size_y_;
    if(!in)
        throw std::runtime_error("Error in from_binary_string-function while reading size_y_ from stream");
    in >> smartmet_guid_;
    if(!in)
        throw std::runtime_error("Error in from_binary_string-function while reading smartmet_guid_ from stream");

    size_t size_of_values_in_bytes = 0;
    in >> size_of_values_in_bytes;
    if(!in)
        throw std::runtime_error("Error in from_binary_string-function while reading size_of_values_in_bytes from stream");
    size_t size_of_values_in_floats = size_of_values_in_bytes / sizeof(float);
    if(size_of_values_in_floats * sizeof(float) != size_of_values_in_bytes)
        throw std::runtime_error("Error in from_binary_string-function size_of_values_in_bytes was incorrect, not divided by 4 (sizeof float)");
    values_.resize(size_of_values_in_floats);

    in.get(space_char); // luetaan myös space pois häiritsemästä
    if(!in)
        throw std::runtime_error("Error in from_binary_string-function while reading space-separator from stream");

    in.read(reinterpret_cast<char*>(values_.data()), size_of_values_in_bytes);
    if(!in)
        throw std::runtime_error("Error in from_binary_string-function while reading bytes for values_ -vector from stream");
}

std::ostream& operator <<(std::ostream &out, const work_result_structure &object)
{
    out << object.job_index_ << " " << object.data_time_index_ << " " << object.job_time_t_ << " ";
    out << object.size_x_ << " " << object.size_y_ << " ";
    write_float_vector(out, object.values_);
    out << " ";
    out << object.smartmet_guid_; // HUOM! smartmet_guid_ ei saa sisältää spaceja!!
    return out;
}

std::istream& operator >>(std::istream &in, work_result_structure &object)
{
    in >> object.job_index_;
    if(!in)
        throw std::runtime_error("Error while reading job_index_ from stream");
    in >> object.data_time_index_;
    if(!in)
        throw std::runtime_error("Error while reading data_time_index_ from stream");
    in >> object.job_time_t_;
    if(!in)
        throw std::runtime_error("Error while reading job_time_t_ from stream");
    in >> object.size_x_;
    if(!in)
        throw std::runtime_error("Error while reading size_x_ from stream");
    in >> object.size_y_;
    if(!in)
        throw std::runtime_error("Error while reading size_y_ from stream");
    read_float_vector(in, object.values_);
    in >> object.smartmet_guid_; // HUOM! smartmet_guid_ -stringi ei saa sisältää spaceja, muuten se katkeaa tässä luvussa!!
    if(!in)
        throw std::runtime_error("Error while reading smartmet_guid_ from stream");

    return in;
}


void fill_work_queue(concurrent_queue<task_structure> &workqueue, size_t &running_task_index)
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
    NFmiGriddingProperties griddingProperties;
    auto griddingPropertiesStr = griddingProperties.toString();
    float cp_point_radius_relative = kFloatMissing;

    task_structure task(1, 1, job_time, g_relative_area_string, 4, 5, x_values, y_values, z_values, g_sample_guid, griddingPropertiesStr, cp_point_radius_relative);
    workqueue.push(task);
    task = task_structure(1, 2, job_time, g_relative_area_string, 5, 3, x_values, y_values, z_values, g_sample_guid, griddingPropertiesStr, cp_point_radius_relative);
    workqueue.push(task);
    add_point_values_to_vectors(0.5f, 0.42f, -0.3f, x_values, y_values, z_values);
    task = task_structure(1, 3, job_time, g_relative_area_string, 6, 4, x_values, y_values, z_values, g_sample_guid, griddingPropertiesStr, cp_point_radius_relative);
    workqueue.push(task);
    task = task_structure(1, 4, job_time, g_relative_area_string, 2, 3, x_values, y_values, z_values, g_sample_guid, griddingPropertiesStr, cp_point_radius_relative);
    workqueue.push(task);
    add_point_values_to_vectors(0.8f, 0.72f, 0.73f, x_values, y_values, z_values);
    task = task_structure(1, 5, job_time, g_relative_area_string, 5, 4, x_values, y_values, z_values, g_sample_guid, griddingPropertiesStr, cp_point_radius_relative);
    workqueue.push(task);
    task = task_structure(1, 6, job_time, g_relative_area_string, 3, 3, x_values, y_values, z_values, g_sample_guid, griddingPropertiesStr, cp_point_radius_relative);
    workqueue.push(task);
    add_point_values_to_vectors(0.23f, 0.87f, 1.3f, x_values, y_values, z_values);
    task = task_structure(1, 7, job_time, g_relative_area_string, 5, 6, x_values, y_values, z_values, g_sample_guid, griddingPropertiesStr, cp_point_radius_relative);
    workqueue.push(task);

    running_task_index = workqueue.size() + 1;
}

task_structure generate_random_task(size_t &running_task_index)
{
    size_t size_x = task_grid_size_rand(get_random_generator());
    size_t size_y = task_grid_size_rand(get_random_generator());
    std::vector<float> x_values;
    std::vector<float> y_values;
    std::vector<float> z_values;
    make_random_point_values(x_values, y_values, z_values);
    NFmiStaticTime localTime;
    std::time_t job_time = localTime.EpochTime();
    NFmiGriddingProperties griddingProperties;
    auto griddingPropertiesStr = griddingProperties.toString();
    float cp_point_radius_relative = kFloatMissing;

    return task_structure(2, running_task_index++, job_time, g_relative_area_string, size_x, size_y, x_values, y_values, z_values, g_sample_guid, griddingPropertiesStr, cp_point_radius_relative);
}

void make_random_point_values(std::vector<float> &x_values, std::vector<float> &y_values, std::vector<float> &z_values)
{
    size_t point_count = point_count_rand(get_random_generator());
    for(size_t i = 0; i < point_count; i++)
    {
        x_values.push_back(relative_place_rand(get_random_generator()));
        y_values.push_back(relative_place_rand(get_random_generator()));
        z_values.push_back(z_value_rand(get_random_generator()));
    }
}

void log_workqueue_size(concurrent_queue<task_structure> &workqueue, logging::trivial::severity_level log_level)
{
    std::string queue_size_message = "There were ";
    queue_size_message += boost::lexical_cast<std::string>(workqueue.size());
    queue_size_message += " tasks in work queue";
    log_message(queue_size_message, log_level);
}

void add_point_values_to_vectors(float x, float y, float z, std::vector<float> &x_values, std::vector<float> &y_values, std::vector<float> &z_values)
{
    x_values.push_back(x);
    y_values.push_back(y);
    z_values.push_back(z);
}

void generate_random_tasks(concurrent_queue<task_structure> &workqueue, int count, size_t &running_task_index)
{
    if(count)
    {
        std::string generate_message = "Generate ";
        generate_message += boost::lexical_cast<std::string>(count);
        generate_message += " new random tasks";
        log_message(generate_message, logging::trivial::info);
        for(int i = 0; i < count; i++)
        {
            workqueue.push(generate_random_task(running_task_index));
        }

        log_workqueue_size(workqueue, logging::trivial::debug);
    }
}

boost::random::mt19937& get_random_generator(void)
{
    static boost::random::mt19937 random_generator;
    return random_generator;
}

// Kutsu tätä vain kerran ohjelman aikana.
// Random generaattorin alustus voidan tehdä usealla eri tavalla:
// 1. Oletus parametreilla otetaan koneen kellosta siemen (nano sekunti tasolla, jos kone voi sellaista tarjota).
// 2. Jos base_seed = g_missing_random_seed ja used_hash_string != "", otetaan koneen kello ja yhdistetään se annetun stringin hash arvoon.
// 3. Jos base_seed != g_missing_random_seed ja used_hash_string = "", käytetään annettua base_seed arvoa sellaisenaan (näin voi antaa halutun vakion)
// 4. Jos base_seed != g_missing_random_seed ja used_hash_string != "", otetaan annettu base_seed ja yhdistetään se annetun stringin hash arvoon.
void seed_random_generator(uint32_t base_seed, std::string used_hash_string)
{
    long long used_base_seed = base_seed;
    if(base_seed == g_missing_random_seed)
    {
        using namespace std::chrono;
        auto now = high_resolution_clock::now();
        used_base_seed = duration_cast<nanoseconds>(now.time_since_epoch()).count();
    }
    if(!used_hash_string.empty())
        used_base_seed = std::hash<std::string>()(used_hash_string) * used_base_seed;

    uint32_t used_seed = static_cast<uint32_t>(used_base_seed);
    log_message(std::string("Used random seed: ") + boost::lexical_cast<std::string>(used_seed), logging::trivial::debug);
    get_random_generator().seed(used_seed);
}

void log_point_values(const tcp_tools::task_structure &task_in, const std::vector<float> &x_values, const std::vector<float> &y_values, const std::vector<float> &z_values)
{
    const logging::trivial::severity_level point_values_log_level = logging::trivial::trace;
    if(get_used_severity_level() >= point_values_log_level)
    { // turha rakentaa tätä työlästä stringiä, jos sitä ei kuitenkaan logata
        const std::string point_value_format_string = "%8.2f";
        std::string log_string = "Locations points for job-index: ";
        log_string += boost::lexical_cast<std::string>(task_in.job_index_);
        log_string += " (data-time-index: ";
        log_string += boost::lexical_cast<std::string>(task_in.data_time_index_);
        log_string += " guid: ";
        log_string += task_in.smartmet_guid_;
        log_string += ") has XYZ-points:\n";
        for(size_t i = 0; i < x_values.size(); i++)
        {
            log_string += boost::str(boost::format(point_value_format_string) % x_values[i]);
            log_string += boost::str(boost::format(point_value_format_string) % y_values[i]);
            log_string += boost::str(boost::format(point_value_format_string) % z_values[i]);
            log_string += "\n";
        }
        log_message(log_string, point_values_log_level);
    }
}

// Tehdään joku höpö työn tulos float vektoriin
bool make_some_fake_result(const task_structure &task_in, work_result_structure &work_result_out, bool use_verbose_logging)
{
    static int counter = 0; // testejä varten countteri
    counter++;

    size_t fake_work_time_in_ms = task_in.size_x_ * task_in.size_y_ * 5;
    //if(counter == 5)
    //    fake_work_time_in_ms = 60 * 1000;
    boost::this_thread::sleep(boost::posix_time::milliseconds(fake_work_time_in_ms));
    log_message(std::string("Fake working time: ") + boost::lexical_cast<std::string>(fake_work_time_in_ms) + " [ms]", logging::trivial::trace);
    if(use_verbose_logging)
        log_point_values(task_in, task_in.x_values_, task_in.y_values_, task_in.z_values_);

    size_t sizeX = task_in.size_x_;
    if(sizeX < 1)
        sizeX = 1;
    size_t sizeY = task_in.size_y_;
    if(sizeY < 1)
        sizeY = 1;
    size_t job_index = task_in.job_index_;
    size_t data_time_index = task_in.data_time_index_;
    size_t job_time_t = task_in.job_time_t_;
    std::string guid(task_in.smartmet_guid_);
    std::vector<float> values(sizeX*sizeY, 0);
    bool even = job_index % 2 == 0;
    for(size_t i = 0; i < values.size(); i++)
    {
        if(even)
            values[i] = i * 1.5f;
        else
            values[values.size() - i - 1] = i * 1.5f;
    }
    
    work_result_out.set_results(job_index, data_time_index, job_time_t, sizeX, sizeY, values, guid);
    return true;
}

void make_some_fake_work(concurrent_queue<task_structure> &task_queue_in, concurrent_queue<work_result_structure> &result_queue_out, bool use_verbose_logging)
{
    if(task_queue_in.size())
    {
        auto queue_copy = task_queue_in.get_basic_list_copy();
        for(int i = 0; !queue_copy.empty(); i++)
        {
            work_result_structure result;
            make_some_fake_result(queue_copy.front(), result, use_verbose_logging);
            result_queue_out.push(result);
            queue_copy.pop_front();
        }
    }
}

// Tarkitaa onko checked_time:n ja nykyhetken aikaerotus tarpeeksi iso (> used_time_out)
bool is_timeout(const boost::posix_time::ptime checked_time, long long used_time_out, long long *time_lasted_in_ms_)
{
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
    long long time_lasted_in_ms = (now - checked_time).total_milliseconds();
    if(time_lasted_in_ms_)
        *time_lasted_in_ms_ = time_lasted_in_ms;
    if(time_lasted_in_ms > used_time_out) 
        return true;
    else
        return false;
}

// Tutkii onko sanoman alussa kokonaisluku ja sen jälkeen space. Jos ei, palautetaan luku 1 (eli luetaan 
// vastaanotettavasta sanomasta seuraavaksi taas yksi merkki).
// Jos oli, asetetaan bin_read_state sellaiseen tilaan että binääri sanoman alku on löytynyt. Lisäksi palautetaan
// 0, joka merkitsee että sanoman luenta lopetetaan tältä osalta, jotta voidaan lukea suoraan binääri mössö kerralla erilliseen puskuriin.
size_t check_for_binary_transfer(size_t bytes, const std::vector<char> &fixed_size_buffer, binary_read_state &binary_read_state, size_t &binary_read_byte_count)
{
    if(bytes >= 2) // pienin määrä merkkejä, jotka voivat muodostaa binääri sanoman alun mahtuu 2 merkkiin, esim. "7 "
    {
        if(fixed_size_buffer[bytes-1] == ' ') // onko viimeinen luettu merkki space
        {
            for(size_t i = 0; i < bytes - 1; i++)
            {
                if(!isdigit(fixed_size_buffer[i]))
                { // jokin merkeistä ennen spacea oli ei numero (0-9)
                    binary_read_state = binary_read_state_not_found;
                    return 1; // jatketaan normaali teksti lukua
                }
            }

            // Kaikki oli numeroita ennen spacea 
            try
            {
                // Kokeillaan muuntuuko stringi vielä varmasti kokonaisluvuksi
                binary_read_byte_count = boost::lexical_cast<size_t>(fixed_size_buffer.data(), bytes-1);
                binary_read_state = binary_read_state_found;
                // Lopetetaan luku tähän, ja jatketaan binääri luvulla
                return 0;
            }
            catch(...)
            {
            }
            binary_read_state = binary_read_state_not_found;
        }
    }
    return 1;
}

// Kun Asio:n kautta luetaan toisen osapuolen lähetystä, lukupuskuria (buffer) tutkitaan yksi merkki kerrallaa ja siitä 
// etsitään '\n' eli lähetyksen lopetus merkkiä.
// Palautetaan aina seuraavaksi luettavien merkkien määrä toisen lähetyksestä (aina joko 1 tai 0).
// Jos loppumerkki löytyi tai oli jokin virhe, palauta 0, muuten jatketaan lukemista ja palautetaan 1.
// Kun fixed_size_buffer on täynnä mutta ei ole löytynyt lopetus merkkiä, laitetaan transmission_incomplete -lippu päälle.
size_t read_complete_check_one_char_at_time(const boost::system::error_code & err, size_t bytes, const std::vector<char> &fixed_size_buffer, bool &transmission_incomplete) 
{
    if(err) 
        return 0;
    if(bytes == 0)
        return 1;

    // tutkitaan vain viimeinen juuri luettu merkki puskurista
    bool found = std::find(fixed_size_buffer.data() + bytes - 1, fixed_size_buffer.data() + bytes, '\n') < fixed_size_buffer.data() + bytes;

    if(!found)
    {
        if(bytes == fixed_size_buffer.size())
            transmission_incomplete = true;
        else
            transmission_incomplete = false;
    }

    // we read one-by-one until we get to enter, no buffering
    return found ? 0 : 1;
}

// Sama kuin edellä oleva read_complete_check_one_char_at_time -funktio, mutta
// etsii myös mahdollista binary sanoman alkua,  eli jos viestin alku on kokonaisluku ja 
// sitten space, tällöin siirrytään binary-sanoman luku vaiheeseen.
size_t read_complete_check_look_for_binary_message(const boost::system::error_code & err, size_t bytes, const std::vector<char> &fixed_size_buffer, bool &transmission_incomplete, binary_read_state &binary_read_state, size_t &binary_read_byte_count) 
{
    if(err) 
        return 0;
    if(bytes == 0)
        return 1;

    // tutkitaan vain viimeinen juuri luettu merkki puskurista
    bool found = std::find(fixed_size_buffer.data() + bytes - 1, fixed_size_buffer.data() + bytes, '\n') < fixed_size_buffer.data() + bytes;

    if(!found)
    {
        if(bytes == fixed_size_buffer.size())
            transmission_incomplete = true;
        else
            transmission_incomplete = false;

        if(binary_read_state == binary_read_state_searching)
            return check_for_binary_transfer(bytes, fixed_size_buffer, binary_read_state, binary_read_byte_count);
    }

    // we read one-by-one until we get to enter, no buffering
    return found ? 0 : 1;
}

// input streamista halutaan vain lukea binary_read_byte_count_fixed määrä tavuja ja sitten lopettaa.
size_t binary_read_complete_check(const boost::system::error_code & err, size_t bytes, size_t binary_read_byte_count_fixed, binary_read_state &binary_read_state)
{
    if(err) 
        return 0;
    if(bytes >= binary_read_byte_count_fixed)
    {
        binary_read_state = binary_read_state_finished;
        return 0;
    }
    else
        return binary_read_byte_count_fixed - bytes;
}

// Jokaisen sanoman luvun alkuun pitää asettaa binary-luku asetus kohdalleen.
// Jos binaari lukua ei sallita, asetetaan binary_read_state_no -tila päälle, muuten 
// binary_read_state_searching.
binary_read_state get_binary_reading_reset_state()
{
    if(use_binary_transfer())
        return binary_read_state_searching;
    else
        return binary_read_state_no;
}

void log_start_of_binary_message(const std::string &log_message_start, const std::string &real_binary_message, size_t max_chars_from_message, logging::trivial::severity_level log_level)
{
    if(max_chars_from_message >= real_binary_message.size())
        max_chars_from_message = real_binary_message.size();
    log_message(log_message_start + std::string(real_binary_message.data(), max_chars_from_message), log_level);
}

} // tcp_tools
