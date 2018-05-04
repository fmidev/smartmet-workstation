#include "CtrlViewKeyboardFunctions.h"


namespace CtrlView
{
    bool IsKeyboardKeyDown(int theKey)
    {
        return (::GetAsyncKeyState(theKey) & 0x8000) ? 1 : 0;
    }

    bool IsKeyboardKeyUp(int theKey)
    {
        return (::GetAsyncKeyState(theKey) & 0x8000) ? 0 : 1;
    }

    // Simulate a key release
    void ReleaseKeyboardKey(BYTE theKey, BYTE theScanCode)
    {
        ::keybd_event(theKey, theScanCode, KEYEVENTF_KEYUP, 0);
    }

    void ReleaseCtrlKeyIfStuck(UINT &nFlags)
    {
        if((nFlags & 8) == 8) // 8 = CTRL
        {
            if(!IsKeyboardKeyDown(VK_CONTROL))
            {
                ReleaseKeyboardKey(VK_CONTROL, 0x9D);
                nFlags -= 8;
            }
        }
        if((nFlags & 4) == 4) // 4 = SHIFT
        {
            if(!IsKeyboardKeyDown(VK_SHIFT))
            {
                ReleaseKeyboardKey(VK_SHIFT, 0xAA);
                nFlags -= 4;
            }
        }
    }

}
