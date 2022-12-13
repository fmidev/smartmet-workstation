# SmartMet Workstation

## First time build instructions

1. Starting from version 5.13.0.0 Visual C++ 2019 is required
2. Also requires CMake 3.14.4 or never
3. Get master from github in bash:
  <br>git clone git@github.com:fmidev/smartmet-workstation.git
4. Get 3rd party binaries from link below:
 <br>https://github.com/fmidev/smartmet-workstation/releases/download/smartmet_release_5_13_6_0/libs.zip
 <br>Should be extracted into repo root for compilation.
5. Run smartmet-workstation/scripts/move_scripts_here.sh script
6. Run cmake build script smartmet-workstation/scripts/cmake/cmake_msvc_build.sh
7. Get Fmi related configurations from github in bash:
  <br>FMI Private: git clone git@github.com:fmidev/smartmet-workstation-conf-fmi.git
  <br>Public: git clone git@github.com:fmidev/smartmet-workstation-conf-example.git
8. MS Visual C++ solution is in smartmet-workstation_msvc directory, open SmartMet.sln from there
9. SmartMet project should use smartmet_sap_scand.conf main configuration file. 
 <br>Command arguments: -d -p ../../../smartmet-workstation-conf-fmi/control/smartmet_sap_scand.conf
 <br>It has cap.conf and wms.conf files disabled.
10. Set SmartMet project as start up project in Visual C++
11. Press F7 to Build solution (key mappings may be different depending on used key mapping schemes)
12. Press F5 to Run SmartMet in debugger..

## How to make SmartMet release

1. When you have working version in your master branch, make version tag with git to it (e.g. smartmet_release_5_12_16_0)
2. Push this version to Github (with smartmet-workstation\scripts\git\dev_2_push_master.sh script)
3. Now in Github, go to tags page https://github.com/fmidev/smartmet-workstation/tags
4. Open newly created tag
5. Press "Edit tag"
6. Fill in release informations: Title: 5.12.16.0 and Description (if you want): For example add info about 3rd party binaries 
7. If there is changes in 3rd party binaries you have to add a new zip file that contains them.
8. Finally press Publish release.
9. Add new link to 3rd party libs into readme.

## SmartMet-workstation 3rd party binary utilities guide

1. If you want to make Case-study data sets, you need 7zip binaries
2. Go to https://www.7-zip.org/ and download the latest installation executable.
3. Install 7zip to your PC
4. Create utils directory in your SmartMet main directory
5. Copy 7z.exe and 7z.dll in utils directory

## SmartMet-workstation 3rd party libraries guide

Here are library dependencies that are needed but are not included with SmartMet-workstation project.
Build all the library binaries with the same Visual C++ compiler (VC++ 2015 or never). If you download them directly make sure that they are build with the same compiler.

### Mandatory libraries for build:
1. Boost
 - Used version 1.61.0 (or newer if no build breaking interface changes)
 - See https://www.boost.org/users/download/ and look there for the link to download "prebuild windows binaries", then you don't have to work with zlib, bzip2 and icu libraries either.
   - Find following download (or newer version): boost_1_61_0-msvc-14.0-64.exe (msvc-14.0 means VC++ 2015)
 - If you *really* need to build, find source download from https://www.boost.org/users/download/
   - Binaries must be build with following options:
     - Platform: x64 (64-bit)
     - Static build (no dll)
     - Using Visual C++ 2015 or newer
     - Include following libraries: ICU, zlib, bzip2
     - See sample Boost build bat script at the end.
2. zlib (only if you build Boost yourself)
 - Used version 1.2.8 (or newer?)
 - Build or download 64-bit static binaries.
3. bzip2 (only if you build Boost yourself)
 - Used version 1.0.6 (or newer?)
 - Build or download 64-bit static binaries.
4. Icu (only if you build Boost yourself)
 - Used version 55.1 (or newer?)
 - Build or download 64-bit static binaries.
 - Look for downloads from http://site.icu-project.org/download
5. CrashRpt
 - Used version 1.4.2 (or newer?)

### Optional libraries for build (omitting them will disable many important features):
6. AVS Uniras ToolMaster
 - Used version 7.5.2 (newer versions might have different systems)
 - Not using this will seriously handicap SmartMet's isoline and color contour visualizations.
 - You have to make contact with Advandec Viaualization Systems http://www.avs.com/contact/ and ask about using ToolMaster.
 - To disable Toolmaster set option DISABLE_UNIRAS_TOOLMASTER from smartmet-workstation\CMakeLists.txt to OFF (default is ON).
7. Extreme Toolkit Pro
 - Used version 17.1.0 (or newer?)
 - Not using this will disable ability to edit smarttool/macroParam formulas in smartTool dialog.
 - You have to make contact with codeJock company http://www.codejock.com/products/toolkitpro/ and ask about using Toolkit Pro.
 - To disable Toolmaster set option DISABLE_EXTREME_TOOLKITPRO from smartmet-workstation\CMakeLists.txt to OFF (default is ON).
8. CppRestSdk
 - Used version 1.61.0 or newer
 - Not using this will disable WMS support and certain Finnish data message support (HAKE and KaHa).
 - Go to https://github.com/Microsoft/cpprestsdk and look for help to install package to windows or build it your self.
 - To disable Toolmaster set option DISABLE_CPPRESTSDK from smartmet-workstation\CMakeLists.txt to OFF (default is ON).

### Using above libraries in smartMet-workstation project:
1. Libraries are copyed to following location:
 - smartmet-workstation\libs\3rd\
 - The directory structure at the moment is following:
		<br>boost_1_61_0
		<br>cpprestsdk
		<br>crashrpt_1_4_2_r1609
		<br>icu_55_1
		<br>uniras_7v5
		<br>xtremetoolkitpro_17_1_0
 - Naming policy is defined in smartmet-workstation\cmake\UsefulVariables.cmake file.
   - There is base names for directory names for each library. 
   - There is also used version for each library, that is in the resulting final directory name. If you use different versions, update the version number in there.
 
 
## Boost building sample script

rem build boost library bat with bjam-system (using MS VC++ 8-14 compiler)

rem Give boost version like it's in the name of boost directory
set BOOST_VERSION=1_61_0

rem different VC++ versions: 2005 = 8.0 , 2008 = 9.0 , 2010 = 10.0 , 2012 = 11.0 , 2013 = 12.0 , 2015 = 14.0
set VC_VER=14.0
set MSVC_TOOLSET=msvc-%VC_VER%
set VISUAL_STUDIO_BASE_PATH=D:\Program Files (x86)
set CPU_CORE_COUNT=24

rem ************** MSVC XX.X version build starts *******************************

rem Set MSVC environment variables on x64 platform
call "%VISUAL_STUDIO_BASE_PATH%\Microsoft Visual Studio %VC_VER%\VC\vcvarsall.bat" amd64
@echo on

rem Help library paths used to build Boost
set LIBSPATH=D:\projekti\ver200_SmartMet_release_5_11\libs

set ZLIB_SOURCE=%LIBSPATH%\zlib_libs\zlib-1.2.8
set ZLIB_INCLUDE=%LIBSPATH%\zlib_libs\zlib-1.2.8
set BZIP2_SOURCE=%LIBSPATH%\bzip2_libs\bzip2-1.0.6
set BZIP2_INCLUDE=%LIBSPATH%\bzip2_libs\bzip2-1.0.6

set BOOST_BASE_PATH=%LIBSPATH%\boost_libs\boost_%BOOST_VERSION%

chdir /D %BOOST_BASE_PATH%

rem Boost build tool Jam must be build first! Remove comment from the following line if you need to build Jam
rem call bootstrap.bat

rem Build from http://site.icu-project.org/download page suitable version's source package with VC++ e.g. icu4c-55_1-src.zip
set ICU_OPTION=-sICU_PATH=%LIBSPATH%\icu_libs\icu-55.1-vs2015

set GENERAL_BUILD_OPTIONS=--toolset=%MSVC_TOOLSET% %ICU_OPTION% -j%CPU_CORE_COUNT% link=static debug release install --without-mpi --without-python --without-wave

rem Build static, 64-bit, debug and release libraries (kaikki) to boost base directory's lib -directory
bjam address-model=64 --prefix=lib --build-dir=bin.v2_64 %GENERAL_BUILD_OPTIONS%

rem ************** MSVC XX.X version build ends *******************************

pause
