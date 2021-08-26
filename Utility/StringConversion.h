#pragma once
#include <string>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <windows.h>

std::string Utf8Encode(const std::wstring& wstr);
std::wstring Utf8Decode(const std::string& str);
void StrLower(std::string& str);

template< typename T >
std::string IntegerToHexStr(T i);

template<typename T>
inline std::string IntegerToHexStr(T i)
{
    std::stringstream stream;
    stream << std::hex << i;
    return stream.str();
}
