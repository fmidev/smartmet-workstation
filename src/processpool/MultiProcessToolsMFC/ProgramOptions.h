#pragma once

#include <string>

class MasterProcessMFCOptions
{
public:
    MasterProcessMFCOptions();
    bool parse_options(int argc, TCHAR *argv[]);

    bool hide_;
    bool demo_mode_;
    std::string log_file_path_;
    std::string worker_exe_file_path_;
    int log_level_;
    bool verbose_log_;
    size_t work_queue_size_in_bytes; // T‰t‰ tarvitaan vain MasterProcessMFC -ohjelmassa koska siin‰ k‰ytet‰‰n boost:in IPC tekniikoita
    size_t result_queue_size_in_bytes; // T‰t‰ tarvitaan vain MasterProcessMFC -ohjelmassa koska siin‰ k‰ytet‰‰n boost:in IPC tekniikoita

    std::string cmd_args_; // komentorivi argumentit ilman itsens‰ ohjelman polkua eli 1. argumenttia
    std::string exe_path_; // ohjelman exen polku eli 1. argumentti
};

class WorkerProcessMFCOptions
{
public:
    WorkerProcessMFCOptions();
    bool parse_options(int argc, TCHAR *argv[]);

    std::string name_;
    int index_;
    bool hide_;
    std::string log_file_path_;
    int log_level_;
    bool verbose_log_;

    std::string cmd_args_; // komentorivi argumentit ilman itsens‰ ohjelman polkua eli 1. argumenttia
    std::string exe_path_; // ohjelman exen polku eli 1. argumentti
};

class CFmiWorkerCommandLineInfo : public CCommandLineInfo
{
public:
	void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);
};

class CFmiMasterCommandLineInfo : public CCommandLineInfo
{
public:
    void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);
};
