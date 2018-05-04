#pragma once

#include <string>

class CWnd;
class NFmiApplicationWinRegistry;

namespace Persist2
{
    void DoWindowMovements(CWnd * win, bool justMoveWindow, const CRect &rect);
    CRect GetWindowRectFromWinRegistry(NFmiApplicationWinRegistry &theApplicationWinRegistry, const std::string &theRegKey);
    void ReadWindowRectFromWinRegistry(NFmiApplicationWinRegistry &theApplicationWinRegistry, const std::string &theRegKey, CWnd * win, bool justMoveWindow);
    void WriteWindowRectToWinRegistry(NFmiApplicationWinRegistry &theApplicationWinRegistry, const std::string &theRegKey, CWnd * win);
    std::string MakeWindowRectString(CWnd * win);
    std::string MakeWindowRectString(const RECT &rect);
}
