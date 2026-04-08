# Not sure which CMake version is minimum for Visual C++ 2019 support
cmake_minimum_required(VERSION 3.14.4)

# This should fix problems with Add member variable -wizard when working with dialogs when working on Windows 10.
if(WIN32)
set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING INTERNAL FORCE)
endif()

project(SmartMet)

if(NOT UNIX)
set(CMAKE_PREFIX_PATH ${LIBS_3RD}/cmakefinds )
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/runtime_output")

if(NOT UNIX)
set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo" CACHE STRING "" FORCE)
endif()

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# ---- Linux-specific auto-enables ----
if(UNIX)
    set(DISABLE_CPPRESTSDK ON CACHE BOOL "Is CppRestSdk used in build" FORCE)
    set(DISABLE_EXTREME_TOOLKITPRO ON CACHE BOOL "Is Extreme ToolkitPro used in build" FORCE)
    set(DISABLE_UNIRAS_TOOLMASTER ON CACHE BOOL "Is Uniras Toolmaster used in build" FORCE)

    # FMI packages install GDAL in versioned directories (e.g. /usr/gdal312/)
    foreach(_gdal_ver 312 310)
        if(EXISTS /usr/gdal${_gdal_ver}/include/gdal.h)
            list(APPEND CMAKE_PREFIX_PATH /usr/gdal${_gdal_ver})
            break()
        endif()
    endforeach()
endif()

# To disable/enable Cpprestsdk usage in SmartMet build, do following:
# 1. Set DISABLE_CPPRESTSDK to ON/OFF in smartmet-workstation/CMakeLists.txt
# 2. Delete the MSVC build directory smartmet-workstation_msvc
# 3. Run cmake script smartmet-workstation/scripts/cmake/cmake_msvc_build.sh
# 4. Open smartmet-workstation_msvc/SmartMet.sln in Visual C++ and do full build
option(DISABLE_CPPRESTSDK "Is CppRestSdk used in build" OFF)
# This will affect WMS maps and data image functions and also HAKE/KaHA messages.

if(DISABLE_CPPRESTSDK)
add_definitions(-DDISABLE_CPPRESTSDK)
endif()

# To disable/enable Extreme ToolkitPro in smartMet build, do following:
# 1. - 4. Same steps as with Cpprestsdk but with option DISABLE_EXTREME_TOOLKITPRO (next line)
option(DISABLE_EXTREME_TOOLKITPRO "Is Extreme ToolkitPro used in build" OFF)
# This will affect Smarttool dialog's behaviour.

if(DISABLE_EXTREME_TOOLKITPRO)
add_definitions(-DDISABLE_EXTREME_TOOLKITPRO)
endif()

# To disable/enable Uniras Toolmaster in smartMet build, do following:
# 1. - 4. Same steps as with Cpprestsdk but with option DISABLE_UNIRAS_TOOLMASTER (next line)
option(DISABLE_UNIRAS_TOOLMASTER "Is Uniras Toolmaster used in build" OFF)
# This will affect isoline/contour visualizations and ControlPoint editing.

if(DISABLE_UNIRAS_TOOLMASTER)
add_definitions(-DDISABLE_UNIRAS_TOOLMASTER)
endif()

if(NOT UNIX)
    add_definitions(
        -D_WIN64
        -D_UNICODE
        -DUNICODE
        -DUSE_POTENTIAL_VALUES_IN_EDITING
        -DJSON_SPIRIT_WVALUE_ENABLED
        )
else()
    add_definitions(
        -DUNIX
        -DUSE_POTENTIAL_VALUES_IN_EDITING
        -DJSON_SPIRIT_WVALUE_ENABLED
        )
endif()

if(NOT UNIX)
    # Enable Visual Studio parallel compilation option
    add_compile_options(
        $<$<CXX_COMPILER_ID:MSVC>:/MP>
        )

    # Enable Visual Studio "Edit And Continue" compilation option for debug build only
    add_compile_options(
        $<$<CONFIG:DEBUG>:/ZI>
        )

    # Disable globally VC++ warning about class's name truncated for being too long (over 4096 characters long)
    add_compile_options(
        $<$<CXX_COMPILER_ID:MSVC>:/wd4503>
        )

    # Disable globally VC++ warning about using deprecated functions.
    add_compile_options(
        $<$<CXX_COMPILER_ID:MSVC>:/wd4996>
        )

    # Disable globally VC++ warnings about 'something' "needs to have dll-interface to be used by clients of class".
    add_compile_options(
        $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
        )
endif()

# ---- Linux: find Qt6 (optional until devel package installed) ----
if(UNIX)
    find_package(Qt6 COMPONENTS Widgets Network PrintSupport)
    if(Qt6_FOUND)
        message(STATUS "Qt6 found: ${Qt6_DIR}")
        add_definitions(-DHAVE_QT6)
    else()
        message(WARNING "Qt6 not found. UI targets will be skipped. Install qt6-qtbase-devel to enable.")
    endif()
    # Linux: add platform compatibility header to all targets
    include_directories(${CMAKE_SOURCE_DIR}/libs/fmi/bizcode/platform)
endif()

# ---- Linux: find system Boost ----
if(UNIX)
    find_package(Boost REQUIRED COMPONENTS thread filesystem iostreams)
    find_package(boost_math_c99 REQUIRED CONFIG)
    # Make Boost libraries available globally on Linux
    link_libraries(Boost::thread Boost::filesystem Boost::iostreams Boost::math_c99)
endif()

include(
    cmake/UsefulVariables.cmake
    )

include(
    cmake/install_deps.cmake
    )

if(NOT UNIX)
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
    include(cmake/DownloadProject.cmake)
endif()

add_subdirectory(
    modules/cppext
    )

add_subdirectory(
    modules/cppback
    )

set(SPDLOG_BUILD_TESTING OFF CACHE BOOL "Do not build spdlog tests" FORCE)

if(UNIX)
add_subdirectory(
    modules/3rd/spdlog114
    )
else()
add_subdirectory(
    modules/3rd/spdlog
    )
endif()

add_subdirectory(
    libs/fmi/catlog
    )

add_subdirectory(
    libs/fmi/xmlliteutils
    )

add_subdirectory(
    libs/fmi/bizcode/stlhelp
    )

if(NOT UNIX)
add_subdirectory(
    libs/fmi/bizcode/gridctrl
    )
endif()

add_subdirectory(
    src/applicationdatabase
    )

if(NOT UNIX)
add_subdirectory(
    src/atlutils
    )
endif()

add_subdirectory(
    modules/newbase
    )

add_subdirectory(
    src/processes
    )

if(NOT UNIX)
add_subdirectory(
    libs/fmi/bizcode/win32utils
    )
endif()

add_subdirectory(
    libs/fmi/bizcode/toolbox
    )

if(NOT UNIX)
add_subdirectory(
    src/mfcutils
    )
endif()

add_subdirectory(
    libs/fmi/datautilities
    )

add_subdirectory(
    libs/fmi/bizcode/json_spirit
    )

if(NOT UNIX)
add_subdirectory(
    libs/fmi/bizcode/winregistryutils
    )
endif()

if(NOT UNIX)
add_subdirectory(
    libs/fmi/multiprocesstools
    )
endif()

add_subdirectory(
    libs/fmi/bizcode/q2clientlib
    )

add_subdirectory(
    modules/imagine
    )

add_subdirectory(
    modules/macgyver
    )

add_subdirectory(
    modules/calculator
    )

if(UNIX)
    # On Linux, use the system-installed smartmet-library-textgen package
    add_library(Textgen SHARED IMPORTED)
    set_target_properties(Textgen PROPERTIES IMPORTED_LOCATION /usr/lib64/libsmartmet-textgen.so)
    target_include_directories(Textgen INTERFACE /usr/include/smartmet)
    set(Textgen_INCLUDE_DIRS /usr/include/smartmet CACHE INTERNAL "Textgen: Include Directories" FORCE)
else()
    add_subdirectory(modules/textgen)
endif()

add_subdirectory(
    modules/smarttools
    )

add_subdirectory(
    libs/fmi/bizcode/editingdatautils
    )

add_subdirectory(
    libs/fmi/bizcode/ctrlviewutils
    )

add_subdirectory(
    libs/fmi/bizcode/addparamstoviews
    )

add_subdirectory(
    libs/fmi/bizcode/tm_utils
    )

if(NOT DISABLE_CPPRESTSDK)
add_subdirectory(
    libs/fmi/webclient
    )
endif()

if(NOT UNIX)
add_subdirectory(
    libs/fmi/hakemessages
    )
endif()

add_subdirectory(
    libs/fmi/bizcode/autocomplete
    )

add_subdirectory(
    libs/fmi/bizcode/capdata
    )

add_subdirectory(
    libs/fmi/bizcode/smetdata
    )

add_subdirectory(
    libs/fmi/bizcode/modifyediteddata
    )

add_subdirectory(
    libs/fmi/bizcode/casestudylib
    )

add_subdirectory(
    libs/fmi/bizcode/toolboxviews
    )

add_subdirectory(
    libs/fmi/bizcode/ctrlviews
    )

if(NOT UNIX)
add_subdirectory(
    libs/fmi/bitmaphandler
    )
endif()

if(NOT DISABLE_CPPRESTSDK)
add_subdirectory(
    libs/fmi/wmssupport
    )
endif()

add_subdirectory(
    libs/fmi/bizcode/timeviews
    )

add_subdirectory(
    libs/fmi/bizcode/smetutils
    )

add_subdirectory(
    src/threads
    )

add_subdirectory(
    libs/fmi/bizcode/filterdataviews
    )

if(NOT UNIX)
add_subdirectory(
    src/toolboxdep
    )
endif()

add_subdirectory(
    libs/fmi/bizcode/stationviews
    )

if(NOT UNIX)
add_subdirectory(
    src/dialogs
    )
endif()

add_subdirectory(
    libs/fmi/bizcode/generaldoc
    )

add_subdirectory(
    src/unpackdatafilesexe
    )

add_subdirectory(
    src/datacombinationexe
    )

if(NOT UNIX)
add_subdirectory(
    src/casestudyexe
    )
endif()

# Put these Master and Worker process projects in comments after
# generating a new version binaries. We don't want to generate them all the time.
if(NOT UNIX)
add_subdirectory(
    src/processpool/MultiProcessToolsMFC
    )
add_subdirectory(
    src/processpool/MasterProcessTcpMFC
    )
add_subdirectory(
    src/processpool/WorkerProcessTcpMFC
    )
# ---- Master and Worker process projects ends --------------

add_subdirectory(
    src/SmartMet
    )
endif()

#add_subdirectory(
#test
#)
