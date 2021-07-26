#include "FmiHakeWarningMessages.h"
#include "SmartMetThreads_resource.h"

// TODO Mitä jos käyttäjä säätää sanomien lukua kesken ajon, pitääkö ohjelma käynnistää uudestaan, vai tehdäänkö muutokset lennossa?

static UINT PostMessageThread(LPVOID pParam)
{
    return ::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ID_MESSAGE_NEW_HAKE_WARNING_AVAILABLE, 0, 0);
}

void CFmiHakeWarningMessages::UpdateApplicationAfterChanges()
{
    // HakeWarning systeemi pyörii c++11 std::thread:issa. Sille annetaan tämä updatefunktio callbackinä.
    // Jostain syystä std::thread:ista lähetetyt PostMessage:t joko kaatuvat tai eivät mene perille CMainFrm:een.
    // Siksi on luotava erillinen CWinThread, joka tekee PostMessage kutsun, koska siitä sanomat menevät perille.
    CWinThread *postMessageThread = AfxBeginThread(::PostMessageThread, nullptr, THREAD_PRIORITY_NORMAL);
}

