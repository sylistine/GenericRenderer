#include "Precompiled.h"

#include "Utils.h"

namespace Engine::Convert
{
    SString itoa(u64 i, u16 b)
    {
        if (i == 0) {
            return SString("0");
        }

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
    SString itoa(u32 i, u16 b)
    {
        return itoa((u64)i, b);
    }
    SString itoa(u16 i, u16 b)
    {
        return itoa((u64)i, b);
    }
    SString itoa(i64 i, u16 b)
    {
        if (i < 0) {
            return SString(1, '-') + itoa((u64)(-1 * i), b);
        } else {
            return itoa((u64)i, b);
        }
    }
    SString itoa(i32 i, u16 b)
    {
        return itoa((i64)i, b);
    }
    SString itoa(i16 i, u16 b)
    {
        return itoa((i64)i, b);
    }

    String atou(SString in)
    {
        String out;
        out.reserve(in.length());
        for (auto c : in) {
            out.push_back((char16_t)c);
        }
        return out;
    }
} // namespace Engine::Convert

namespace Engine::StrUtil
{
    u64 len(ustr str)
    {
        u64 len = 0;
        while (str[len]) {
            len++;
        }
        return len;
    }

    String pad(String str, u64 len, Alignment align)
    {
        if (len < str.length()) return str;

        auto padCount = len - str.length();
        if (align == Alignment::Left) {
            str = str + String(padCount, u' ');
        } else if (align == Alignment::Center) {
            auto halfPadCount = padCount / 2;
            auto padLeft = String(padCount / 2, u' ');
            auto padRight = String(padCount % 2 ? halfPadCount + 1 : halfPadCount, u' ');
            str = padLeft + str + padRight;
        } else {
            str = String(padCount, u' ') + str;
        }
        return str;
    }
} // namespace Engine::StrUtil
