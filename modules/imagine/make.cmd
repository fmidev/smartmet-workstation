@REM
@REM make.cmd [debug|release] [Boost_path] [Precompiled_Visual_C++_Libraries_path]
@REM
@REM Requires:
@REM     SCons       installed and available ('scons -v' works)
@REM     VC++ 2008   command line tools available (run 'vcvars32.bat')
@REM     Precompiled Cairomm etc. ('svn checkout http://svn.fmi.fi/svn/tie/VisualC++Libraries')
@REM                 available at D:\IL\svn\VisualC++Libraries
@REM     Boost 1.36  available at D:\Boost\1_36_0 (installed by BoostPro Computing installer)
@REM
@if "%1"=="" %0 debug
@if "%2"=="" %0 %1 D:\Boost\boost_1_36_0
@if "%3"=="" %0 %1 %2 D:\IL\svn\VisualC++Libraries

@if not "%WindowsSdkDir%"=="" goto VC_OK
@echo.
@echo *** Run '%VC90COMNTOOLS%\vcvars32.bat'
@echo *** to set up Visual C++ 2008 command line tools.
@echo.
@goto EXIT

:VC_OK
@REM 'objdir' on local disk to speed up compilation if using a remote file system
@REM (s.a. ssh mapped to G: via SFtpDrive)
@REM
scons %1=1 windows_boost_path=%2 windows_prebuilt_path=%3 smartmet_imagine_%1.lib objdir=%TEMP%\imagine-%1

:EXIT
