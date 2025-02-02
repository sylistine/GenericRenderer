#pragma once

#include <array>
#include <vector>
#include <string>

using u16 = unsigned short;
using i16 = short;
using u32 = unsigned int;
using i32 = int;
using u64 = unsigned long;
using i64 = long;
// c = char, != clang
using cstr = const char*;
using str = const wchar_t*;
// List instead of Vec to avoid confusion with the glm type.
template<class T>
using List = std::vector<T>;
using String = std::string;

enum Result
{
    Success,
    Failed
};
