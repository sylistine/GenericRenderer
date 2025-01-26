#pragma once

#include "stdinc.h"

#include <vulkan/vulkan.h>

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
    inline const char* translateSeverityFlagBits(VkDebugUtilsMessageSeverityFlagBitsEXT bits)
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
    inline const char* translateTypeFlags(VkDebugUtilsMessageTypeFlagsEXT flags)
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
    inline const char* translateObjectType(VkObjectType type)
    {
        switch (type) {

        }
    }
    std::string itoa(uint64_t i, uint16_t b = 10)
    {
        auto str = std::string("");

        const int numstart = '0';
        const int majstart = 'A';
        const int minstart = 'a';
        while (i > 0) {
            auto v = i % b;
            auto out = v > 9 ? (char)(minstart + (v-10)) : (char)(numstart + v);
            str = std::string(1,out) + str;

            i /= b;
        }

        return str;
    }
    std::string itoa(uint32_t i, uint16_t b = 10)
    {
        return itoa(i, b);
    }
    std::string itoa(uint16_t i, uint16_t b = 10)
    {
        return itoa(i, b);
    }
    std::string itoa(int32_t i, uint16_t b = 10)
    {
        if (i < 0) {
            return std::string(1, '-') + itoa((uint64_t)(- 1 * i), b);
        } else {
            return itoa((uint64_t)i, b);
        }
    }

    void setup(VkInstance instance)
    {
        createDebugMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");
        destroyDebugMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT");

        auto createInfo = getDebugMessengerCreateInfo();
        createDebugMessenger(instance, &createInfo, nullptr, &debugMessenger);
    }

    void cleanup(VkInstance instance)
    {
        destroyDebugMessenger(instance, debugMessenger, nullptr);
    }

    VkBool32 debugLogger(
        VkDebugUtilsMessageSeverityFlagBitsEXT severityFlag,
        VkDebugUtilsMessageTypeFlagsEXT typeFlags,
        const VkDebugUtilsMessengerCallbackDataEXT* data,
        void* userData)
    {
        auto tagPre = std::string("[");
        auto tagPost = std::string("]");

        auto prefix = std::string("Daedalus Vulkan Debug");

        auto severityLabel = std::string(translateSeverityFlagBits(severityFlag));
        auto typeLabel = std::string(translateTypeFlags(typeFlags));


        //auto labelPad = 16 - typeLabel.length();
        //if (labelPad > 0) typeLabel.append(labelPad, ' ');
        
        auto msg = prefix + " [" + severityLabel + "] ";
        
        if ((typeFlags & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0) {
            msg += data->pMessage;
        } else {
            msg += "[" + typeLabel + " | " + data->pMessageIdName + "] " +
                data->pMessage + "\n";
        }
        OutputDebugStringA((msg).c_str());

        return VK_TRUE;
    }

} // Debug
} // Daedalus
