#ifndef UNIX
#include "stdafx.h"
#endif // UNIX
#include "WaitCursorHelper.h"


// Tiimalasi halutaan laittaa päälle vain jos esim. ei ole animaatiota karttanäytössä käynnissä
WaitCursorHelper::WaitCursorHelper(bool showWaitCursor)
    :fShowWaitCursor(showWaitCursor)
{
    if(fShowWaitCursor)
    {
        AfxGetApp()->BeginWaitCursor();
    }
}

WaitCursorHelper::~WaitCursorHelper()
{
    if(fShowWaitCursor)
    {
        AfxGetApp()->EndWaitCursor();
    }
}
