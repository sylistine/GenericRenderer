#pragma once

#include "Precompiled.h"
namespace Engine
{
    namespace Convert
    {
        // Built for fun, but also because stdlib might be overkill.
        SString itoa(u64, u16 = 10);
        SString itoa(u32, u16 = 10);
        SString itoa(u16, u16 = 10);
        SString itoa(i32, u16 = 10);
        SString itoa(i16, u16 = 10);

        String atou(SString);
    } // namespace Convert

    namespace StrUtil
    {
        // crazy that the standard lib doesn't have functions to count char16_t strings.
        u64 len(ustr);

        enum Alignment
        {
            Left,
            Center,
            Right
        };
        String pad(String, u64, Alignment);
    } // namespace StrUtil
} // namespacec Engine
