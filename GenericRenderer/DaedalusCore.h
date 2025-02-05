#pragma once

#include "stdinc.h"

#if defined (_WINDOWS)
#include <Windows.h>
#endif

namespace Engine::Daedalus
{
    Result initialize();
    Result terminate();

#if defined(_WINDOWS)
    Result createSurface(HINSTANCE, HWND);
#endif
}