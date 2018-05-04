#include "stdafx.h"
#include "PERSIST2.H"

#include "NFmiStringTools.h"
#include "FmiWin32Helpers.h"
#include "NFmiApplicationWinRegistry.h"

static CRect MakeRectFromString(const std::string &theRectStr)
{
    std::string rectStr(theRectStr);
	NFmiStringTools::Trim(rectStr); // trimmataan varmuudenvuoksi molemmat p‰‰t spaceista
	std::vector<std::string> valueStrVector = NFmiStringTools::Split(rectStr,",");
	if(valueStrVector.size() != 4)
		throw std::runtime_error("Error in MakeRectFromString: There were not 4 values for window position and size.");

	CRect rect;
	rect.left = NFmiStringTools::Convert<int>(valueStrVector[0]);
	rect.top = NFmiStringTools::Convert<int>(valueStrVector[1]);
	rect.right = NFmiStringTools::Convert<int>(valueStrVector[2]);
	rect.bottom = NFmiStringTools::Convert<int>(valueStrVector[3]);
    return rect;
}

namespace Persist2
{
    void DoWindowMovements(CWnd * win, bool justMoveWindow, const CRect &rect)
    {
        // t‰h‰n v‰liin tehd‰‰n tarkastelut, ett‰ jos ikkuna menee n‰yttˆjen ulkopuolelle, laitetaan se l‰himp‰‰n ikkunaan
        int startCornerCounter = 0;
        CRect fixedRect(CFmiWin32Helpers::FixWindowPosition(rect, startCornerCounter));
        if(justMoveWindow)
            win->SetWindowPos(NULL, fixedRect.left, fixedRect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        else
            win->MoveWindow(fixedRect);
    }

    CRect GetWindowRectFromWinRegistry(NFmiApplicationWinRegistry &theApplicationWinRegistry, const std::string &theRegKey)
    {
        return ::MakeRectFromString(theApplicationWinRegistry.WindowRectStr(theRegKey));
    }

    void ReadWindowRectFromWinRegistry(NFmiApplicationWinRegistry &theApplicationWinRegistry, const std::string &theRegKey, CWnd * win, bool justMoveWindow)
    {
        if(win)
        {
            if(theRegKey.empty() == false)
            {
                CRect rect = GetWindowRectFromWinRegistry(theApplicationWinRegistry, theRegKey);
                DoWindowMovements(win, justMoveWindow, rect);
            }
            else
                throw std::runtime_error("Error in ReadWindowRectFromWinRegistry: Given Windows registry key string was empty.");
        }
        else
            throw std::runtime_error("Error in ReadWindowRectFromWinRegistry: Given window was NULL.");
    }

    std::string MakeWindowRectString(const RECT &rect)
    {
        std::string rectStr;
        rectStr += NFmiStringTools::Convert<LONG>(rect.left);
        rectStr += ",";
        rectStr += NFmiStringTools::Convert<LONG>(rect.top);
        rectStr += ",";
        rectStr += NFmiStringTools::Convert<LONG>(rect.right);
        rectStr += ",";
        rectStr += NFmiStringTools::Convert<LONG>(rect.bottom);
        return rectStr;
    }

    std::string MakeWindowRectString(CWnd * win)
    {
        std::string rectStr;
        if(win)
        {
            WINDOWPLACEMENT wndpl;
            wndpl.length = sizeof(WINDOWPLACEMENT);
            // gets current window position
            BOOL bRet = win->GetWindowPlacement(&wndpl);
            rectStr = MakeWindowRectString(wndpl.rcNormalPosition);
        }
        return rectStr;
    }

    void WriteWindowRectToWinRegistry(NFmiApplicationWinRegistry &theApplicationWinRegistry, const std::string &theRegKey, CWnd * win)
    {
        if(win)
        {
            if(theRegKey.empty() == false)
            {
                std::string rectStr = MakeWindowRectString(win);
                if(!rectStr.empty())
                {
                    std::string currentRectStr = theApplicationWinRegistry.WindowRectStr(theRegKey);
                    if(rectStr != currentRectStr)
                        theApplicationWinRegistry.WindowRectStr(theRegKey, rectStr);
                }
            }
            else
                throw std::runtime_error("Error in WriteWindowRectToWinRegistry: Given Windows registry key string was empty.");
        }
    }

}
