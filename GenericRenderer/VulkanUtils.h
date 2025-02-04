#pragma once

#include "stdinc.h"

#include <vulkan/vulkan.hpp>

namespace VkUtil
{
    String prettyString_QueueFamilies(vk::PhysicalDevice device)
    {
        auto str = String(u"");

        // 13 long, last idx 12
        List<ustr> columns;
        columns.push_back(u"     idx     ");
        columns.push_back(u"   Graphics  ");
        columns.push_back(u"   Compute   ");
        columns.push_back(u"   Transfer  ");
        columns.push_back(u"  SparseBind ");
        columns.push_back(u"  Protected  ");
        columns.push_back(u"   Vid-Dec   ");
        columns.push_back(u"   Vid-Enc   ");
        columns.push_back(u"  OpticalNV  ");
        
        auto tl = u'┌';
        auto tc = u'┬';
        auto tr = u'┐';
        auto lc = u'├';
        auto t = u'┼';
        auto rc = u'┤';
        auto bl = u'└';
        auto bc = u'┴';
        auto br = u'┘';
        auto v = u'│';
        auto h = u'─';
        auto b = u"─────────────";
        auto yea = u"      ●      ";
        auto nay = u"      ‐      ";

        // Top edge.
        str += String(1, tl);
        for (auto& column : columns) {
            str += String(b) + String(1, tc);
        }
        str[str.size() - 1] = tr;
        str.push_back(u'\n');

        // Header labels.
        for (auto& column : columns) {
            str.push_back(v);
            str += String(column);
        }
        str.push_back(v);
        str.push_back(u'\n');

        // Middle edge.
        str += String(1, lc);
        for (auto& column : columns) {
            str += String(b) + String(1, t);
        }
        str[str.size() - 1] = rc;
        str.push_back(u'\n');

        auto props = device.getQueueFamilyProperties();
        for (auto i = 0; i < props.size(); i++) {
            auto numstr = atou(itoa(i));

            str.push_back(v);
            str += String(6, u' ') + numstr + String(7 - numstr.length(), u' ');
            str.push_back(v);
            using flags = vk::QueueFlagBits;
            str += (props[i].queueFlags & flags::eGraphics) ? yea : nay;
            str.push_back(v);
            str += (props[i].queueFlags & flags::eCompute) ? yea : nay;
            str.push_back(v);
            str += (props[i].queueFlags & flags::eTransfer) ? yea : nay;
            str.push_back(v);
            str += (props[i].queueFlags & flags::eSparseBinding) ? yea : nay;
            str.push_back(v);
            str += (props[i].queueFlags & flags::eProtected) ? yea : nay;
            str.push_back(v);
            str += (props[i].queueFlags & flags::eVideoDecodeKHR) ? yea : nay;
            str.push_back(v);
            str += (props[i].queueFlags & flags::eVideoEncodeKHR) ? yea : nay;
            str.push_back(v);
            str += (props[i].queueFlags & flags::eOpticalFlowNV) ? yea : nay;
            str.push_back(v);
            str.push_back(u'\n');

            // Middle edge.
            auto l = i == props.size() - 1 ? bl : lc;
            auto c = i == props.size() - 1 ? bc : t;
            auto r = i == props.size() - 1 ? br : rc;
            str += String(1, l);
            for (auto& column : columns) {
                str += String(b) + String(1, c);
            }
            str[str.size() - 1] = r;
            str.push_back(u'\n');
        }

        return str;
    }
}
