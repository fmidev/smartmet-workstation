#include "stdafx.h"
#include "WaitCursorHelper.h"


// Tiimalasi halutaan laittaa p��lle vain jos esim. ei ole animaatiota karttan�yt�ss� k�ynniss�
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
