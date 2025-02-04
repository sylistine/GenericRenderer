#pragma once

#include <array>
#include <vector>
#include <string>

#if defined(_WINDOWS)
#include <Windows.h>
#endif

/*
* Modern integer type naming (c.2025), for brevity and legibility.
*/
using u16 = unsigned short;
using i16 = short;
using u32 = unsigned int;
using i32 = int;
using u64 = unsigned long;
using i64 = long;

/*
* Convenient types for brevity, legibility, and to ascertain unicode-16-ness.
*/

// Simple character string.
using sstr = const char*;

// Wide character string - same size as char16_t, but not guaranteed unicode.
using wstr = const wchar_t*;

// Unicode-16 character string - inclusive international format.
using ustr = const char16_t*;

/*
* Common standard class renaming, also for brevity and legibility.
*/

// List instead of Vec to avoid confusion with the glm type.
template<class T>
using List = std::vector<T>;

// Simple character string class.
using SString = std::string;

// Unicode-16 string class. The preferred class for strings in this project.
using String = std::u16string;

enum Result
{
    Success,
    Failed
};

// Built for fun, but also because stdlib might be overkill.
SString itoa(u64, u16 = 10);
SString itoa(u32, u16 = 10);
SString itoa(u16, u16 = 10);
SString itoa(i32, u16 = 10);
SString itoa(i16, u16 = 10);

String atou(SString);

// crazy that the standard lib doesn't have functions to count char16_t strings.
u64 strlen(ustr);

enum Alignment
{
    Left,
    Center,
    Right
};
String pad(String, u64, Alignment);

void Log(sstr log);
void Log(ustr log);
