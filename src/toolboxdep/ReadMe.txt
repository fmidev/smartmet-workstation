========================================================================
    STATIC LIBRARY : SmartMetToolboxDep Project Overview
========================================================================
Markon kommenttia:
T‰m‰ on MFC kirjasto, joka on riippuvainen mm. toolbox-kirjastosta.
T‰m‰ riippuvuus on tarkoitus korvata GDI+ piirroilla ja jonka
j‰lkeen luokat on tarkoitus siirt‰‰ SmartMetViews tai muuhun vastaavaan 
kirjastoon.
HUOMIOITA:
1. Resursseja voi olla eri kirjastoissa irrallaan toisistaan.
2. Apukirjaston resurssi-tiedosto (buildattu *.res -tiedosto) pit‰‰ antaa erikseen 
   p‰‰ohjelman linkkerille.
3. Icon -resursseja ei voi olla kuin yhden projektin resursseiss‰, jos mit‰ tahansa 
   ikoneita on useissa resurssi tiedostoissa ja ne linkataan yhteen tulee joku 
   virhe, jolle ei voi mit‰‰n ainakaan viel‰.

========================================================================

AppWizard has created this SmartMetToolboxDep library project for you.

This file contains a summary of what you will find in each of the files that
make up your SmartMetToolboxDep application.


SmartMetToolboxDep.vcproj
    This is the main project file for VC++ projects generated using an Application Wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    Application Wizard.


/////////////////////////////////////////////////////////////////////////////

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named SmartMetToolboxDep.pch and a precompiled types file named StdAfx.obj.

/////////////////////////////////////////////////////////////////////////////
The compiler and linker switches have been modified to support MFC. Using the
MFC ClassWizard with this project requires that you add several files to the
project, including "resource.h", "SmartMetToolboxDep.rc" and a "SmartMetToolboxDep.h" that
includes resource.h. If you add an rc file to a static library, you may
experience difficulties due to the limitation that only one rc file may be
present in a Dll or Exe. This problem may be overcome by including the
library's .rc file into the parent project's .rc file.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" comments to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////
