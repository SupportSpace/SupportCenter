#pragma once 

#include "AidLib\Strings\tstring.h"

std::wstring FromUtf8ToUtf16(const std::string& utf8string);
std::string  ToUtf8FromUtf16(const std::wstring& widestring);
