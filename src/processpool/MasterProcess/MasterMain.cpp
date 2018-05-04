
// S‰‰ntˆj‰, mit‰ olen huomannut Master-prosessin ja Worker-prosessien (=child) v‰lill‰:
// 1. child-prosessia ei saa ottaa talteen, koska sen destruktorin kutsu myˆhemmin kaataa ohjelman.
// 2. Master prosessi ei lopu, jos workerit jatkavat olemassa oloaan, vaan se j‰‰ odottamaan niiden lopetusta.
// 3. workerit eiv‰t saa omaa dos-ikkunaa (hyv‰), vaan ne j‰‰v‰t tavallaan masterin alaisuuteen.
// 4. Jos master kaatuu, eiv‰t workerit lopeta, vaan kuollut Master prosessi odottaa workereiden loppumista (sama kuin 2.).

#include "NFmiFastQueryInfo.h"

#include "work_queue.h"
#include "logging.h"
#include "process_tools.h"
#include "heartbeat_checker.h"
#include "worker_process_info.h"
#include "process_helpers.h"

#include <boost/filesystem.hpp>
#include <boost/thread/thread.hpp> 
#include <boost/math/special_functions/round.hpp>

#ifdef WIN32
#include <conio.h>
#endif

const std::string worker_base_name = "WorkerProcess";
const std::string worker_executable_name = "WorkerProcess.exe";
const std::string worker_project_name = "WorkerProcess";

static size_t global_task_index = 0;

typedef process_helpers::master_process_stuff<task_queue_t, result_queue_t> master_process_stuff_t;

// Tarkistaa onko mit‰‰n n‰pp‰int‰ painettu. 
// Jos ei ole, palauttaa 0:n.
// Ei toimi Linuxilla.
int check_if_key_pressed()
{
    int ch = 0;
#ifdef WIN32
    if(::_kbhit()) // _kbhit tarkistaa onko painettu mit‰‰n n‰pp‰int‰, ei j‰‰ odottamaan
        ch = _getch(); // jos oli painettu jotain n‰pp‰int‰, pit‰‰ tarkistaa oliko se mahdollisesti q, jolloin k‰ytt‰j‰ haluaa pakotetun lopetuksen
#endif
    return ch;
}

int main(int /* argc */ , const char * /* argv */ )
{
    int exit_status_code = 0; // 0 = ok, 1 = error
    int worker_count = 4;
    bool close_program_flag = false; // master_loop vaatii rajapinnassa t‰ll‰ist‰, t‰t‰ k‰ytet‰‰n l‰hinn‰ interaktiivisten ohjelmien kanssa
    base_worker_process_start_info base_process_info(worker_executable_name, worker_project_name, worker_base_name);
    process_helpers::multi_process_pool_options mpp_options;

    init_logger("Master", logging::trivial::debug, default_logging_file, true);
    work_queue_verbose_logging(mpp_options.verbose_logging); // laitetaan myˆs work_queue:n verbose-log tila p‰‰lle
    log_message("Started", logging::trivial::info);

    try
    {
//        master_process_stuff_t master_process_data(worker_count, global_task_index, base_process_info, true);
        master_process_stuff_t master_process_data(worker_count, base_process_info, mpp_options);
        fill_work_queue2(master_process_data);
        process_helpers::master_loop(master_process_data, check_if_key_pressed, close_program_flag, true);
    }
    catch(boost::interprocess::interprocess_exception &ex)
    {
        exit_status_code = 1;
        log_message(ex.what(), logging::trivial::error);
    }
    catch(std::exception &ex)
    {
        exit_status_code = 1;
        log_message(ex.what(), logging::trivial::error);
    }
    catch(...)
    {
        exit_status_code = 1;
        log_message("Unknown exception thrown", logging::trivial::error);
    }

    log_message("Stopping", logging::trivial::info);
    return exit_status_code;
}

//#include <windows.h>
//SetErrorMode(SetErrorMode(0) | SEM_NOGPFAULTERRORBOX);
