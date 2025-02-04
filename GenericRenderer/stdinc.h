#pragma once

#include <array>
#include <vector>
#include <string>

#if defined(_WINDOWS)
#include <Windows.h>
#endif

using u16 = unsigned short;
using i16 = short;
using u32 = unsigned int;
using i32 = int;
using u64 = unsigned long;
using i64 = long;
using sstr = const char*; // simple 8-bit char string type.
using wstr = const wchar_t*; // 16-bit, maybe unicode, string type.
using ustr = const char16_t*; // 16-bit unicode string type.
// List instead of Vec to avoid confusion with the glm type.
template<class T>
using List = std::vector<T>;
using SString = std::string; // simple 8-bit char string object.
using String = std::u16string; // 16-bit unicode string object.

enum Result
{
    Success,
    Failed
};

SString itoa(u64, u16 = 10);
SString itoa(u32, u16 = 10);
SString itoa(u16, u16 = 10);
SString itoa(i32, u16 = 10);
SString itoa(i16, u16 = 10);

String atou(SString);

void Log(sstr log);
void Log(ustr log);
