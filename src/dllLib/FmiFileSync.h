#pragma once

#include "stdafx.h"

#ifdef SMARTMETDLLLIB_BUILD
#define SMARTMETDLLLIB_USAGE __declspec(dllexport)
#else
#define SMARTMETDLLLIB_USAGE __declspec(dllimport)
#endif

// T‰ss‰ on tiedosto synkronointi funktioita.
// Se k‰ytt‰‰ MicroSoftin Sync framework:ia.
// T‰ss‰ on unicode k‰‰nt‰j‰ optio p‰‰ll‰, koska Sync FrameWork vaatii sit‰

namespace CFmiFileSync
{
	HRESULT SMARTMETDLLLIB_USAGE DoSynchronize(const GUID &guidReplicaSrc, CStringW &strFolderSrc, CStringW &strMetaFileSrc, 
											   const GUID &guidReplicaDest, CStringW &strFolderDest, CStringW &strMetaFileDest);
	void SMARTMETDLLLIB_USAGE CancelSynchronization(void);
	int SMARTMETDLLLIB_USAGE BruteDirCopy(CStringW &strFolderSrc, CStringW &strFolderDest, bool fDoSHFileOperation);
	bool SMARTMETDLLLIB_USAGE DirDelete(CStringW &strFolder);

}
