#pragma once

#include "stdafx.h"

namespace CtrlView
{
	//keyboard helpers
	bool IsKeyboardKeyDown(int theKey);
	bool IsKeyboardKeyUp(int theKey);
#ifndef UNIX
	void ReleaseKeyboardKey(BYTE theKey, BYTE theScanCode);
	void ReleaseCtrlKeyIfStuck(UINT &nFlags);
#else
	inline void ReleaseKeyboardKey(int, int) {}
	inline void ReleaseCtrlKeyIfStuck(unsigned int&) {}
#endif // UNIX
}
