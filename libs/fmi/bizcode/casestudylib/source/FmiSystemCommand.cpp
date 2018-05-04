
#include "FmiSystemCommand.h"
#include "NFmiDictionaryFunction.h"

bool CFmiSystemCommand::DoCommand(std::string &theCommand, WORD theShowWindow, bool waitExecutionToStop, DWORD dwCreationFlags)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = theShowWindow;;

// Start the child process.
	if(!CreateProcess(
		NULL, // No module name (use command line).
		CA2T(theCommand.c_str()),
		NULL,             // Process handle not inheritable.
		NULL,             // Thread handle not inheritable.
		FALSE,            // Set handle inheritance to FALSE.
		dwCreationFlags,  // Creation flags.
		NULL,             // Use parent's environment block.
		NULL,             // Use parent's starting directory.
		&si,              // Pointer to STARTUPINFO structure.
		&pi )             // Pointer to PROCESS_INFORMATION structure.
		)
	{
        LPTSTR lpMsgBuf=0;
		FormatMessage
		(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			lpMsgBuf,
			0,
			NULL
		);

		MessageBox( NULL, lpMsgBuf, CA2T(::GetDictionaryString("Error in system call").c_str()), MB_OK|MB_ICONERROR );

		LocalFree( lpMsgBuf );

        // Add actual logging later (using e.g. CatLog system), now removing/disabling everything using old newbase NFmiLogger1 class
		//if(theLogger)
		//{
		//	std::string virheStr("CFmiSystemCommand::DoCommand: ");
		//	virheStr += ::GetDictionaryString("the command");
		//	virheStr += " '";
		//	virheStr += theCommand;
		//	virheStr += "'\n";
		//	virheStr += ::GetDictionaryString("could not be executed");
		//	virheStr += ".";
		//	theLogger->LogMessage(virheStr, kError);
		//}
		return false;
	}
	DWORD waitTimeInMS = waitExecutionToStop ? INFINITE : 0; // joko odotetaan että prosessi saadaan päätökseen, tai ei odoteta ollenkaan
	WaitForSingleObject( pi.hProcess, waitTimeInMS);

	CloseHandle( pi.hProcess ); // Close process and thread handles.
	CloseHandle( pi.hThread );
	//if(theLogger)
	//{
	//	std::string logStr("CFmiSystemCommand::DoCommand: ");
	//	logStr += ::GetDictionaryString("the command");
	//	logStr += " '";
	//	logStr += theCommand;
	//	logStr += "' ";
	//	logStr += ::GetDictionaryString("executed");
	//	logStr += ".";
	//	theLogger->LogMessage(logStr, kInfo);
	//}
	return true;
}

