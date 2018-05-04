
#include "FmiFileSync.h"
#include <synchronization.h>
#include <string>
#include <FileSyncProvider.h>
#include "afxmt.h"
#include <io.h>
#include <direct.h>

extern "C" {
  #include <sys/types.h>
  #include <sys/stat.h>
}

namespace
{
	class ComInitializer
	{
	public:
		ComInitializer(void)
		{
			CoInitialize(NULL); // starts COM system
		}

		~ComInitializer(void)
		{
			CoUninitialize(); // ends COM system
		}

	};

	CSemaphore gSessionSemaphore; // t‰m‰n avulla asetetaan ja k‰ytet‰‰n sync-sessio otusta jaetusti 'siististi'
	ISyncSession* gSyncSession = NULL;
}

static void SetSyncSession(ISyncSession* theSyncSession)
{
	CSingleLock singleLock(&gSessionSemaphore); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(singleLock.Lock(1*1000)) // Attempt to lock the shared resource for 1 second
	{
		gSyncSession = theSyncSession;
	}
}

static void ReleaseSyncSession(void)
{
	CSingleLock singleLock(&gSessionSemaphore); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(singleLock.Lock(1*1000)) // Attempt to lock the shared resource for 1 second
	{
		gSyncSession = NULL;
	}
}

static void CancelSyncSession(void)
{
	CSingleLock singleLock(&gSessionSemaphore); // muista ett‰ t‰m‰ vapauttaa semaphoren kun tuhoutuu
	if(singleLock.Lock(1*1000)) // Attempt to lock the shared resource for 1 second
	{
		if(gSyncSession)
			gSyncSession->Cancel();
	}
}

CStringW ExtractDirectory(const CStringW &path)
{
	int pos = path.ReverseFind('\\');
	if(pos != -1)
	{
		return path.Left(pos);
	}
	return CStringW();
}
/*
CStringW ExtractFilename(const CStringW &path)
{
	return path.substr( path.find_last_of( '\\' ) +1 );
}
*/


static bool MakeSureDirectoryExist(CStringW &fileName, bool isFileName)
{
	CStringW dirName = fileName;
	if(isFileName)
		dirName = ::ExtractDirectory(fileName);
	if(dirName.IsEmpty() == FALSE)
	{
		int createDirRes = ::SHCreateDirectoryExW(NULL, dirName, NULL);
		if(createDirRes == ERROR_SUCCESS || createDirRes == ERROR_FILE_EXISTS || createDirRes == ERROR_ALREADY_EXISTS)
			return true;
	}
	return false;
}

static bool CopyDirectory(const std::string & theSrcPath, const std::string & theDestPath); // pit‰‰ esitell‰ WinCopyItem -funktiolle

static bool IsWinDir(const struct _finddata_t &fileinfo)
{
	if((fileinfo.attrib & 16) == 16)
		return true;
	return false;
}

static const std::string gThisDir = ".";
static const std::string gParentDir = "..";
static bool WinCopyItem(const std::string & theSrcPath, const std::string & theDestPath, struct _finddata_t &theFileinfo)
{
	if(::IsWinDir(theFileinfo))
	{ // kopsataan hakemistot rekursiota k‰ytt‰m‰ll‰
		if(gThisDir != theFileinfo.name && gParentDir != theFileinfo.name)
		{
			std::string subSrcDir = theSrcPath + "\\" + theFileinfo.name;
			std::string subDestDir = theDestPath + "\\" + theFileinfo.name;
			return ::CopyDirectory(subSrcDir, subDestDir);
		}
		else
			return true; // ei tarvitse tehd‰ mit‰‰n '.'- ja '..'-hakemistoille
	}
	else
	{
		// kopioi tiedosto
		std::string srcFilename = theSrcPath + "\\" + theFileinfo.name;
		std::string destFilename = theDestPath + "\\" + theFileinfo.name;
		return ::CopyFileExA(srcFilename.c_str(), destFilename.c_str(), 0, 0, 0, COPY_FILE_FAIL_IF_EXISTS) == TRUE;
	}
}

#ifndef S_ISDIR
  #define S_ISDIR(mode) (mode & S_IFDIR)
#endif

static bool DirectoryExists(const std::string & theFile)
{
	if(theFile.empty() == true)
		return false;
	struct stat st;

	char ch = theFile[theFile.size()-1];
	if(ch == '/' || ch == '\\')
	{ // ainakin VC71 ei tunnista hakemistoa olevaksi jos polku p‰‰ttyy '/' tai '\' merkkiin,
		// joten poistan sen lopusta ja kutsun rekursiivisesti funktiota uudestaan.
		std::string file2(theFile.begin(), theFile.begin() + theFile.size() - 1);
		return ::DirectoryExists(file2);
	}

	if(::stat(theFile.c_str(),&st) != 0)
		return false;

	if(!S_ISDIR(st.st_mode))
	return false;

	return true;
}

  // ----------------------------------------------------------------------
  // System independent MKDIR() method
  // Unix moodi on rwxr-xr-x
  // ----------------------------------------------------------------------

#ifdef UNIX
#define MKDIR(path) mkdir(path,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)
#else
#define MKDIR(path) ::_mkdir(path)
#endif

// Code take from http://www.codeguru.com/cpp/tic/tic0180.shtml
static std::string& ReplaceAll(std::string & theString, const std::string &fromStr, const std::string &toStr)
{
	std::string::size_type found = theString.find(fromStr);
	while(found != std::string::npos) 
	{
		theString.replace(found, fromStr.length(), toStr);
		found = theString.find(fromStr, found+1);
	}
	return theString;
}

  // ----------------------------------------------------------------------
  /*!
   * Creates the given path if it does not exist beforehand.
   *
   * \param thePath The path to create
   * \return Always true, but should return false if the operation fails
   * \todo Make the operation return false on failure
   */
  // ----------------------------------------------------------------------

static bool CreateDirectory(const std::string &thePath)
{
	if(thePath.empty() || DirectoryExists(thePath))
		return true; // jos polku oli jo ei tarvitse jatkaa

	std::string tmpPath(thePath);
	::ReplaceAll(tmpPath, "\\", "/");
	std::string::size_type pos=0;
	while( (pos = tmpPath.find('/',pos)) != std::string::npos)
	{
		std::string path = tmpPath.substr(0,pos);
		// "." has been optimized here for speed
		if( (path != ".") && !DirectoryExists(path))
			MKDIR(path.c_str());
		pos += 2;
	}

	if(!DirectoryExists(tmpPath))
		MKDIR(tmpPath.c_str());

	return DirectoryExists(tmpPath);
}

// ----------------------------------------------------------------------
/*!
 * \brief Copy all the files and sub-directories from source to destination recursively.
*/
// ----------------------------------------------------------------------
static bool CopyDirectory(const std::string & theSrcPath, const std::string & theDestPath)
{
	if(::DirectoryExists(theSrcPath))
	{
		if(::CreateDirectory(theDestPath) == false)
			return false;

		std::string tempPath(theSrcPath);
		tempPath += "\\*"; // Jostain syyst‰ MSVC:ss‰ pit‰‰ lis‰t‰ file-filtteri hakemiston per‰‰n.
		struct _finddata_t fileinfo;
		long handle;
		if((handle = static_cast<long>(_findfirst(const_cast<char*>(tempPath.c_str()), &fileinfo))) != -1)
			::WinCopyItem(theSrcPath, theDestPath, fileinfo);
		else // jos find_first ei lˆyt‰nyt mit‰‰n ja mentiin silti find_next:iin, k‰‰tui XP:ss‰ rakennettu juttu NT4:ssa
			return true;
		while(!_findnext(handle,&fileinfo))
			::WinCopyItem(theSrcPath, theDestPath, fileinfo);
		_findclose(handle);
		return true;
	}
	return false;
}
#pragma warning( push )
#pragma warning( disable : 4996 )

static std::string WStringToString(const std::wstring& s)
{
	std::locale loc;
	std::string ns;
	ns.resize(s.size());
	std::use_facet<std::ctype<wchar_t> >(loc).narrow(&s[0], &s[0]+s.size(), '?', &ns[0]);
	return ns;
}

// Undo the above warning disabling.
#pragma warning( pop )

// Kopioidaan hakemisto rekursiivisesti.
// fDoSHFileOperation -parametri on sit‰ varten, ett‰ Win 7:ssa toimii SHFileOperation -funktio ok kun verkko levylt‰ kopsataa tavaraa
// esim. lokaali levylle, mutta ainakin XP:ssa t‰m‰ ei toimikaan, eli funktio toimii muuten, mutta ei verkkolevyjen kanssa.
// Siksi jouduin tekem‰‰n oman versio rekursiivisesta tiedosto kopioinnista.
// Siksi SmartMet tarkastaa onko Win 7 tai myˆhempi k‰ytˆss‰ ja k‰ytt‰‰ sitten haluttua operaatiota.
int CFmiFileSync::BruteDirCopy(CStringW &strFolderSrc, CStringW &strFolderDest, bool fDoSHFileOperation)
{
	if(fDoSHFileOperation == false)
	{
		std::wstring tmpSrcWstr = strFolderSrc;
		std::wstring tmpDestWstr = strFolderDest;
		std::string tmpSrcPath = ::WStringToString(tmpSrcWstr);
		std::string tmpDestPath = ::WStringToString(tmpDestWstr);
		return ::CopyDirectory(tmpSrcPath, tmpDestPath);
	}
	else
	{
		strFolderSrc += '\0'; // n‰m‰ toiset nollat pit‰‰ lis‰t‰ loppuun, muuten ei toimi
		strFolderDest += '\0'; // n‰m‰ toiset nollat pit‰‰ lis‰t‰ loppuun, muuten ei toimi
		//Directory Copying 
		SHFILEOPSTRUCTW SHDirOp;
		ZeroMemory(&SHDirOp, sizeof(SHFILEOPSTRUCT));
		SHDirOp.hwnd = NULL;
		SHDirOp.wFunc = FO_COPY;
		SHDirOp.pFrom = strFolderSrc;
		SHDirOp.pTo = strFolderDest;
		SHDirOp.fFlags = FOF_MULTIDESTFILES|FOF_NOCONFIRMMKDIR|FOF_NOCONFIRMATION;

		//The Copying Function
		return SHFileOperationW(&SHDirOp);
	}
}

bool CFmiFileSync::DirDelete(CStringW &strFolder)
{
	strFolder += '\0'; // n‰m‰ toiset nollat pit‰‰ lis‰t‰ loppuun, muuten ei toimi
	//Directory Copying 
	SHFILEOPSTRUCTW SHDirOp;
	ZeroMemory(&SHDirOp, sizeof(SHFILEOPSTRUCT));
	SHDirOp.hwnd = NULL;
	SHDirOp.wFunc = FO_DELETE;
	SHDirOp.pFrom = strFolder;
	SHDirOp.pTo = NULL;
	SHDirOp.fFlags = FOF_MULTIDESTFILES|FOF_ALLOWUNDO|FOF_NOCONFIRMATION;

	//The Copying Function
	int result = SHFileOperationW(&SHDirOp);

	return result == 0;
}

static bool FolderExists(CStringW &strFolderName)
{   
    return GetFileAttributes(strFolderName) != INVALID_FILE_ATTRIBUTES;   
}

HRESULT CFmiFileSync::DoSynchronize(const GUID &guidReplicaSrc, CStringW &strFolderSrc, CStringW &strMetaFileSrc, 
								    const GUID &guidReplicaDest, CStringW &strFolderDest, CStringW &strMetaFileDest)
{
	static ComInitializer gComInitializer; // pakko hoitaa COM-ssyteemin initialisointi n‰in, en tied‰ miksi tuntuu toimivan tyˆkoneessa ilman, mutta ei kannettavassa.

	CStringW m_strFilenameExc;
	CStringW m_strDirExc;
	CStringW m_strAttrExc;
	CStringW m_strFilenameInc;

	HRESULT hr = E_UNEXPECTED;

	// Create the source and destination providers.
	IFileSyncProvider* pProvSrc = NULL;
	hr = CoCreateInstance(CLSID_FileSyncProvider, NULL, CLSCTX_INPROC_SERVER, __uuidof(pProvSrc), (void**)&pProvSrc);
	if (SUCCEEDED(hr))
	{
		IFileSyncProvider* pProvDest = NULL;
		hr = CoCreateInstance(CLSID_FileSyncProvider, NULL, CLSCTX_INPROC_SERVER, __uuidof(pProvDest), (void**)&pProvDest);
		if (SUCCEEDED(hr))
		{
			// Create a scope filter and fill it (some strings may be empty).
			IFileSyncScopeFilter* pFilter = NULL;
			hr = pProvSrc->CreateNewScopeFilter(&pFilter);
			if (SUCCEEDED(hr))
			{
				hr = pFilter->SetFilenameExcludes(m_strFilenameExc.GetString());

				if (SUCCEEDED(hr))
				{
					hr = pFilter->SetSubDirectoryExcludes(m_strDirExc.GetString());
				}

				if (SUCCEEDED(hr))
				{
					DWORD dwMask = wcstoul(m_strAttrExc.GetString(), NULL, 16);
					hr = pFilter->SetFileAttributeExcludeMask(dwMask);
				}

				if (SUCCEEDED(hr))
				{
					// Only set the include list if we have something in it, because
					// setting the include list to empty effectively excludes all files.
					if (!m_strFilenameInc.IsEmpty())
					{
						hr = pFilter->SetFilenameIncludes(m_strFilenameInc.GetString());
					}
				}

				if (SUCCEEDED(hr))
				{
					// Initialize the providers.
					if(::MakeSureDirectoryExist(strMetaFileSrc, true))
					{
						hr = pProvSrc->Initialize(guidReplicaSrc, strFolderSrc.GetString(), strMetaFileSrc.GetString(), NULL, FILESYNC_INIT_FLAGS_NONE, pFilter, NULL, NULL);
						if (SUCCEEDED(hr))
						{
							if(::MakeSureDirectoryExist(strMetaFileDest, true))
							{
								if(::MakeSureDirectoryExist(strFolderDest, false))
									hr = pProvDest->Initialize(guidReplicaDest, strFolderDest.GetString(), strMetaFileDest.GetString(), NULL, FILESYNC_INIT_FLAGS_NONE, pFilter, NULL, NULL);
							}
						}
					}
				}

				pFilter->Release();
			}

			if (SUCCEEDED(hr))
			{
				// Synchronize!
				IApplicationSyncServices* pSvc = NULL;
				hr = CoCreateInstance(CLSID_SyncServices, NULL, CLSCTX_INPROC_SERVER, IID_IApplicationSyncServices, (void**)&pSvc);
				if (SUCCEEDED(hr))
				{
					ISyncSession* pSession = NULL;
					hr = pSvc->CreateSyncSession(pProvDest, pProvSrc, &pSession);
					if (SUCCEEDED(hr))
					{
						::SetSyncSession(pSession);
						try
						{
							SYNC_SESSION_STATISTICS syncStats;
							hr = pSession->Start(CRP_NONE, &syncStats);
						}
						catch(...)
						{
						}
						::ReleaseSyncSession();
						pSession->Release();
					}

					pSvc->Release();
				}
			}

			pProvDest->Release();
		}

		pProvSrc->Release();
	}

	return hr;
}

void CFmiFileSync::CancelSynchronization(void)
{
	::CancelSyncSession();
}

