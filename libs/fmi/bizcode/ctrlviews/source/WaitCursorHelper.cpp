#ifndef UNIX
#include "stdafx.h"
#endif // UNIX
#include "WaitCursorHelper.h"


// Tiimalasi halutaan laittaa p��lle vain jos esim. ei ole animaatiota karttan�yt�ss� k�ynniss�
WaitCursorHelper::WaitCursorHelper(bool showWaitCursor)
    :fShowWaitCursor(showWaitCursor)
{
#ifndef UNIX
    if(fShowWaitCursor)
    {
        AfxGetApp()->BeginWaitCursor();
    }
#endif
}

WaitCursorHelper::~WaitCursorHelper()
{
#ifndef UNIX
    if(fShowWaitCursor)
    {
        AfxGetApp()->EndWaitCursor();
    }
#endif
}
