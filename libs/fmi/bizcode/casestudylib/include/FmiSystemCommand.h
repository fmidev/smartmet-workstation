#pragma once

#include <string>
#include "stdafx.h"

namespace CFmiSystemCommand
{
	bool DoCommand(std::string &theCommand, WORD theShowWindow = SW_SHOW, bool waitExecutionToStop = false, DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS);
}
