#pragma once

// T�nne on tarkoitus laittaa win32 riippuvaisia string konversioita

#include<string>

std::wstring convertPossibleUtf8StringToWideString(const std::string& utf8str);
