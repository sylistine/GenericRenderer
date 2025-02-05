#pragma once

/// Includes and usings for standard library and other
/// external dependencies.

#include <array>
#include <vector>
#include <string>

#if defined(_WINDOWS)
#include <Windows.h>
#endif

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
