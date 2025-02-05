#include "Precompiled.h"
#include "Debug.h"

namespace Engine::Debug
{
    void Log(sstr log)
    {
        OutputDebugStringA(log);
    }
    void Log(ustr log)
    {
        OutputDebugStringW(reinterpret_cast<wstr>(log));
    }
}
