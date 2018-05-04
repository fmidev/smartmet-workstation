#pragma once

#include "stdafx.h"

namespace CtrlView
{
	//keyboard helpers
	bool IsKeyboardKeyDown(int theKey);
	bool IsKeyboardKeyUp(int theKey);
	void ReleaseKeyboardKey(BYTE theKey, BYTE theScanCode);
	void ReleaseCtrlKeyIfStuck(UINT &nFlags);
}
