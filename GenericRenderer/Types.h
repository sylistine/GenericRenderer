#pragma once

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