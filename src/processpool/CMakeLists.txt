cmake_minimum_required(VERSION 3.6.2)

project(ProcessPool)

add_subdirectory(
    ClientProcessTcp
    )

add_subdirecotry(
    MasterProcess
    )

add_subdirecotry(
    MasterProcessMFC
    )

add_subdirecotry(
    MasterProcessTcp
    )

add_subdirecotry(
    MasterProcessTcpMFC
    )

add_subdirecotry(
    MultiProcessToolsMFC
    )

add_subdirecotry(
    WorkerProcess
    )

add_subdirecotry(
   WorkerProcessMFC 
    )

add_subdirecotry(
   WorkerProcessTcp 
    )

add_subdirecotry(
   WorkerProcessTcpMFC 
    )

