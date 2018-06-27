# smartmet-workstation
SmartMet Workstation

First time build instructions:
1. Get master from github in bash:
  <br>git clone git@github.com:fmidev/smartmet-workstation.git
2. Get 3rd party binaries from link below:
 <br>https://github.com/fmidev/smartmet-workstation/releases/download/5.12.12.1b/libs.zip
 <br>Should be extracted into repo root for compilation.
3. Run smartmet-workstation/scripts/move_scripts_here.sh script
4. Run cmake build script smartmet-workstation/scripts/cmake/cmake_msvc_build.sh
5. Get Fmi related configurations from github in bash:
  <br>git clone git@github.com:fmidev/smartmet-workstation-conf-fmi.git
6. MS Visual C++ solution is in smartmet-workstation_msvc directory, open SmartMet.sln from there
7. SmartMet project should use smartmet_sap_scand.conf main configuration file. 
 <br>Command arguments: -d -p ../../../smartmet-workstation-conf-fmi/control/smartmet_sap_scand.conf
 <br>It has cap.conf and wms.conf files disabled.
8. Set SmartMet project as start up project in Visual C++
9. Press F7 to Build solution (key mappings may be different depending on used key mapping schemes)
10. Press F5 to Run SmartMet in debugger..
