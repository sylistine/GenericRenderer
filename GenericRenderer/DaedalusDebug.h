#pragma once

#include "stdinc.h"

#include <vulkan/vulkan.hpp>

namespace Daedalus
{
namespace Debug
{
    VkBool32 debugLogger(
        VkDebugUtilsMessageSeverityFlagBitsEXT,
        VkDebugUtilsMessageTypeFlagsEXT,
        const VkDebugUtilsMessengerCallbackDataEXT*,
        void*);

    PFN_vkCreateDebugUtilsMessengerEXT createDebugMessenger;
    PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugMessenger;

    VkDebugUtilsMessengerEXT debugMessenger;


    inline VkDebugUtilsMessengerCreateInfoEXT getDebugMessengerCreateInfo()
    {
        auto createInfo = VkDebugUtilsMessengerCreateInfoEXT{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = 
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
        createInfo.pfnUserCallback = debugLogger;
        return createInfo;
    }
    inline cstr translateSeverityFlagBits(VkDebugUtilsMessageSeverityFlagBitsEXT bits)
    {
        switch (bits) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            return "V";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            return "I";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            return "W";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            return "E";
        default:
            return "?";
        }
    }
    inline cstr translateTypeFlags(VkDebugUtilsMessageTypeFlagsEXT flags)
    {
        switch (flags) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            return "General";
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            return "Validation";
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            return "Performance";
        case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
            return "Device Address Binding";
        default:
            return "Unknown Message Type";
        }
    }
    String itoa(u64 i, u16 b = 10)
    {
        auto str = String("");

        const int numstart = '0';
        const int majstart = 'A';
        const int minstart = 'a';
        while (i > 0) {
            auto v = i % b;
            auto out = v > 9 ? (char)(minstart + (v-10)) : (char)(numstart + v);
            str = String(1,out) + str;

            i /= b;
        }

        return str;
    }
    String itoa(u32 i, u16 b = 10)
    {
        return itoa(i, b);
    }
    String itoa(u16 i, u16 b = 10)
    {
        return itoa(i, b);
    }
    String itoa(i32 i, u16 b = 10)
    {
        if (i < 0) {
            return String(1, '-') + itoa((u64)(- 1 * i), b);
        } else {
            return itoa((u64)i, b);
        }
    }

    void setup(vk::Instance instance)
    {
        createDebugMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");
        destroyDebugMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT");

        auto createInfo = getDebugMessengerCreateInfo();
        createDebugMessenger(instance, &createInfo, nullptr, &debugMessenger);
    }

    void cleanup(vk::Instance instance)
    {
        destroyDebugMessenger(instance, debugMessenger, nullptr);
    }

    VkBool32 debugLogger(
        VkDebugUtilsMessageSeverityFlagBitsEXT severityFlag,
        VkDebugUtilsMessageTypeFlagsEXT typeFlags,
        const VkDebugUtilsMessengerCallbackDataEXT* data,
        void* userData)
    {
        auto tagPre = String("[");
        auto tagPost = String("]");

        auto prefix = String("Daedalus Vulkan Debug");

        auto severityLabel = String(translateSeverityFlagBits(severityFlag));
        auto typeLabel = String(translateTypeFlags(typeFlags));


        //auto labelPad = 16 - typeLabel.length();
        //if (labelPad > 0) typeLabel.append(labelPad, ' ');
        
        auto msg = prefix + " [" + severityLabel + "] ";
        
        if ((typeFlags & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0) {
            msg += data->pMessage;
            if (msg.back() != '\n') msg.push_back('\n');
        } else {
            msg += "[" + typeLabel + " | " + data->pMessageIdName + "] " +
                data->pMessage + "\n";
        }
        OutputDebugStringA((msg).c_str());

        return VK_TRUE;
    }

} // Debug
} // Daedalus
