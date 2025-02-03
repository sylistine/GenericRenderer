#include "stdinc.h"

// For fun, maybe, but also I didn't want to include another stdlib just for this.
SString itoa(u64 i, u16 b = 10)
{
    auto str = SString("");

    const int numstart = '0';
    const int majstart = 'A';
    const int minstart = 'a';
    while (i > 0) {
        auto v = i % b;
        auto out = v > 9 ? (char)(minstart + (v - 10)) : (char)(numstart + v);
        str = SString(1, out) + str;

        i /= b;
    }

    return str;
}
SString itoa(u32 i, u16 b = 10)
{
    return itoa((u64)i, b);
}
SString itoa(u16 i, u16 b = 10)
{
    return itoa((u64)i, b);
}
SString itoa(i32 i, u16 b = 10)
{
    if (i < 0) {
        return SString(1, '-') + itoa((u64)(-1 * i), b);
    } else {
        return itoa((u64)i, b);
    }
}

void Log(sstr log)
{
    OutputDebugStringA(log);
}
void Log(ustr log)
{
    OutputDebugStringW(reinterpret_cast<wstr>(log));
}