#ifndef UNIX
#include "FmiHakeWarningMessages.h"
#include "SmartMetThreads_resource.h"

// TODO Mit� jos k�ytt�j� s��t�� sanomien lukua kesken ajon, pit��k� ohjelma k�ynnist�� uudestaan, vai tehd��nk� muutokset lennossa?

static UINT PostMessageThread(LPVOID pParam)
{
    return ::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ID_MESSAGE_NEW_HAKE_WARNING_AVAILABLE, 0, 0);
}

void CFmiHakeWarningMessages::UpdateApplicationAfterChanges()
{
    // HakeWarning systeemi py�rii c++11 std::thread:issa. Sille annetaan t�m� updatefunktio callbackin�.
    // Jostain syyst� std::thread:ista l�hetetyt PostMessage:t joko kaatuvat tai eiv�t mene perille CMainFrm:een.
    // Siksi on luotava erillinen CWinThread, joka tekee PostMessage kutsun, koska siit� sanomat menev�t perille.
    CWinThread *postMessageThread = AfxBeginThread(::PostMessageThread, nullptr, THREAD_PRIORITY_NORMAL);
}
#endif // UNIX

