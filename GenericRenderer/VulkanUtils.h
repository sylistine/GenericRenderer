#pragma once

#include "Utils.h"

#include <vulkan/vulkan.hpp>

namespace Engine::Daedalus::VkUtil
{
    inline String horizontalLine(
        char16_t lc, // left cross
        char16_t mc, // middle cross
        char16_t rc, // right cross
        char16_t span, // span char
        u64 cwidth,
        u64 ccount)
    {
        String str = String(1, lc);

        for (auto i = 0u; i < ccount; i++) {
            str += String(cwidth, span) + String(1, mc);
        }
        str[str.size() - 1] = rc;
        str.push_back(u'\n');
        return str;
    }

    String getQueueFamilyPropertiesPrettyString(
        vk::PhysicalDevice gpu,
        vk::SurfaceKHR surface = VK_NULL_HANDLE)
    {
        using namespace StrUtil;

        String str;
        // Begin queue family table.
        auto props = gpu.getQueueFamilyProperties();

        // Setup header labels.
        List<ustr> labelLiterals;
        labelLiterals.push_back(u"idx");
        labelLiterals.push_back(u"Graphics");
        labelLiterals.push_back(u"Compute");
        labelLiterals.push_back(u"Transfer");
        labelLiterals.push_back(u"SparseBind");
        labelLiterals.push_back(u"Protected");
        labelLiterals.push_back(u"Vid-Dec");
        labelLiterals.push_back(u"Vid-Enc");
        labelLiterals.push_back(u"OpticalNV");
        if (surface != VK_NULL_HANDLE) {
            labelLiterals.push_back(u"Present");
        }
        auto maxlen = 0u;
        for (auto& literal : labelLiterals) {
            auto cur = len(literal);
            if (cur > maxlen) {
                maxlen = cur;
            }
        }
        List<String> labelStrings;
        auto spanWidth = maxlen + 2;
        for(auto& literal : labelLiterals) {
            labelStrings.push_back(pad(String(literal), spanWidth, Alignment::Center));
        }
        
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
        auto y = u'●';
        auto n = u'‐';

        auto yea = pad(String(1, y), spanWidth, Alignment::Center);
        auto nay = pad(String(1, n), spanWidth, Alignment::Center);

        // Top edge.
        auto colWidth = (u64)labelStrings[0].length();
        auto colCount = (u64)labelStrings.size();
        str += horizontalLine(tl, tc, tr, h, colWidth, colCount);

        // Header labels.
        for (auto& column : labelStrings) {
            str.push_back(v);
            str += column;
        }
        str.push_back(v);
        str.push_back(u'\n');

        // Middle edge.
        str += horizontalLine(lc, t, rc, h, colWidth, colCount);

        // Add a row for each queue family on the GPU.
        for (auto i = 0; i < props.size(); i++) {
            using flags = vk::QueueFlagBits;
            auto& prop = props[i];
            auto idxStr = Convert::atou(Convert::itoa(i));

            str += String(1, v) + pad(String(idxStr), spanWidth, Alignment::Center);
            str += String(1, v) + ((prop.queueFlags & flags::eGraphics) ? yea : nay);
            str += String(1, v) + ((prop.queueFlags & flags::eCompute) ? yea : nay);
            str += String(1, v) + ((prop.queueFlags & flags::eTransfer) ? yea : nay);
            str += String(1, v) + ((prop.queueFlags & flags::eSparseBinding) ? yea : nay);
            str += String(1, v) + ((prop.queueFlags & flags::eProtected) ? yea : nay);
            str += String(1, v) + ((prop.queueFlags & flags::eVideoDecodeKHR) ? yea : nay);
            str += String(1, v) + ((prop.queueFlags & flags::eVideoEncodeKHR) ? yea : nay);
            str += String(1, v) + ((prop.queueFlags & flags::eOpticalFlowNV) ? yea : nay);
            if (surface != VK_NULL_HANDLE) {
                auto surfaceSupport = gpu.getSurfaceSupportKHR(i, surface);
                str += String(1, v) + ((surfaceSupport) ? yea : nay);
            }
            str.push_back(v);
            str.push_back(u'\n');

            // Middle or bottom edge.
            auto l = i == props.size() - 1 ? bl : lc;
            auto c = i == props.size() - 1 ? bc : t;
            auto r = i == props.size() - 1 ? br : rc;
            str += horizontalLine(l, c, r, h, colWidth, colCount);
        }

        return str;
    }

    /// <summary>
    /// Builds a pretty string describing physical device properties.
    /// 
    /// QueueFamily flags are displayed in a neat table.
    /// 
    /// If a Surface is provided, the returned string will also display whether or not
    /// the queue families also support present.
    /// </summary>
    /// <param name="gpu">A vk::PhysicalDevice object.</param>
    /// <param name="surface">(Optional) A vk::SurfaceKHR to test for Queue support.</param>
    /// <returns></returns>
    String to_prettyString(vk::PhysicalDevice gpu, vk::SurfaceKHR surface = VK_NULL_HANDLE)
    {
            auto props = gpu.getProperties();

            auto name = String(Convert::atou(props.deviceName));
            auto type = String(Convert::atou(vk::to_string(props.deviceType)));

            auto str = String(u"");
            str += String(u"Physical Device: ") + name + String(u"\n");
            str += String(u"Device Type: ") + type + String(u"\n");

            str += getQueueFamilyPropertiesPrettyString(gpu, surface);
            return str;
    }
}
