#pragma once

#include<string>

std::string fromLocaleStringToUtf8(const std::string &localeStr);
std::string fromUtf8toLocaleString(const std::string &utf8Str);
