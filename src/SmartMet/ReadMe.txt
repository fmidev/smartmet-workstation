================================================================================
    MICROSOFT FOUNDATION CLASS LIBRARY : SmartMet Project Overview
===============================================================================

The application wizard has created this SmartMet application for
you.  This application not only demonstrates the basics of using the Microsoft
Foundation Classes but is also a starting point for writing your application.

This file contains a summary of what you will find in each of the files that
make up your SmartMet application.

SmartMet.vcproj
    This is the main project file for VC++ projects generated using an application wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    application wizard.

SmartMet.h
    This is the main header file for the application.  It includes other
    project specific headers (including Resource.h) and declares the
    CSmartMetApp application class.

SmartMet.cpp
    This is the main application source file that contains the application
    class CSmartMetApp.

SmartMet.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
    Visual C++. Your project resources are in 1033.

res\SmartMet.ico
    This is an icon file, which is used as the application's icon.  This
    icon is included by the main resource file SmartMet.rc.

res\SmartMet.rc2
    This file contains resources that are not edited by Microsoft
    Visual C++. You should place all resources not editable by
    the resource editor in this file.

SmartMet.reg
    This is an example .reg file that shows you the kind of registration
    settings the framework will set for you.  You can use this as a .reg
    file to go along with your application or just delete it and rely
    on the default RegisterShellFileTypes registration.

SmartMet.idl
    This file contains the Interface Description Language source code for the
    type library of your application.

/////////////////////////////////////////////////////////////////////////////

For the main frame window:
    The project includes a standard MFC interface.

MainFrm.h, MainFrm.cpp
    These files contain the frame class CMainFrame, which is derived from
    CFrameWnd and controls all SDI frame features.

res\Toolbar.bmp
    This bitmap file is used to create tiled images for the toolbar.
    The initial toolbar and status bar are constructed in the CMainFrame
    class. Edit this toolbar bitmap using the resource editor, and
    update the IDR_MAINFRAME TOOLBAR array in SmartMet.rc to add
    toolbar buttons.
/////////////////////////////////////////////////////////////////////////////

The application wizard creates one document type and one view:

SmartMetDoc.h, SmartMetDoc.cpp - the document
    These files contain your CSmartMetDoc class.  Edit these files to
    add your special document data and to implement file saving and loading
    (via CSmartMetDoc::Serialize).
    The Document will have the following strings:
        File extension:      sqd
        File type ID:        SmartMet.Document
        Main frame caption:  SmartMet
        Doc type name:       SmartMet
        Filter name:         SmartMet Files (*.sqd)
        File new short name: SmartMet
        File type long name: SmartMet.Document

SmartMetView.h, SmartMetView.cpp - the view of the document
    These files contain your CSmartMetView class.
    CSmartMetView objects are used to view CSmartMetDoc objects.




/////////////////////////////////////////////////////////////////////////////

Help Support:

hlp\SmartMet.hhp
    This file is a help project file. It contains the data needed to
    compile the help files into a .chm file.

hlp\SmartMet.hhc
    This file lists the contents of the help project.

hlp\SmartMet.hhk
    This file contains an index of the help topics.

hlp\afxcore.htm
    This file contains the standard help topics for standard MFC
    commands and screen objects. Add your own help topics to this file.

hlp\afxprint.htm
    This file contains the help topics for the printing commands.

makehtmlhelp.bat
    This file is used by the build system to compile the help files.

hlp\Images\*.gif
    These are bitmap files required by the standard help file topics for
    Microsoft Foundation Class Library standard commands.


/////////////////////////////////////////////////////////////////////////////

Other Features:

ActiveX Controls
    The application includes support to use ActiveX controls.

Printing and Print Preview support
    The application wizard has generated code to handle the print, print setup, and print preview
    commands by calling member functions in the CView class from the MFC library.

MAPI support
    The generated project contains the code needed to create, manipulate, transfer and
    store mail messages.

Windows Sockets
    The application has support for establishing communications over TCP/IP networks.

/////////////////////////////////////////////////////////////////////////////

Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named SmartMet.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Visual C++ reads and updates this file.

SmartMet.manifest
	Application manifest files are used by Windows XP to describe an applications
	dependency on specific versions of Side-by-Side assemblies. The loader uses this
	information to load the appropriate assembly from the assembly cache or private
	from the application. The Application manifest  maybe included for redistribution
	as an external .manifest file that is installed in the same folder as the application
	executable or it may be included in the executable in the form of a resource.
/////////////////////////////////////////////////////////////////////////////

Other notes:

The application wizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

If your application uses MFC in a shared DLL, you will need
to redistribute the MFC DLLs. If your application is in a language
other than the operating system's locale, you will also have to
redistribute the corresponding localized resources MFC90XXX.DLL.
For more information on both of these topics, please see the section on
redistributing Visual C++ applications in MSDN documentation.

/////////////////////////////////////////////////////////////////////////////
