#pragma once

#include "stdinc.h"

#include <vulkan/vulkan.hpp>

namespace VkUtil
{
    String prettyString(vk::QueueFamilyProperties props)
    {
        auto str = String(u"");
        // 12 long, last idx 11
        List<ustr> labels;
        labels.push_back(u"  Graphics  ");
        labels.push_back(u"  Compute   ");
        labels.push_back(u"  Transfer  ");
        labels.push_back(u" SparseBind ");
        labels.push_back(u" Protected  ");
        labels.push_back(u"  Vid-Dec   ");
        labels.push_back(u"  Vid-Enc   ");
        labels.push_back(u" OpticalNV  ");
        ustr yea = u"○";
        ustr nay = u"×";

        for (auto& label : labels) {
            str += String(u"|") + label;
        }
        return str;
    }
}
