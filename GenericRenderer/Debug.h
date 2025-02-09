#pragma once

namespace Engine::Debug
{
    inline void Log(sstr log)
    {
#if defined(_DEBUG)
        OutputDebugStringA(log);
#endif
    }
    inline void Log(ustr log)
    {
#if defined(_DEBUG)
        OutputDebugStringW(reinterpret_cast<wstr>(log));
#endif
    }
}
