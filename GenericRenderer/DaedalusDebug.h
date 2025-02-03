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

    vk::DebugUtilsMessengerEXT messenger;
    vk::DispatchLoaderDynamic loader;

    /// <summary>
    /// This is also used before instance creation for instance create debug logging.
    /// </summary>
    /// <returns></returns>
    inline vk::DebugUtilsMessengerCreateInfoEXT getMessengerCreateInfo()
    {
        using sevFlags = vk::DebugUtilsMessageSeverityFlagBitsEXT;
        using typeFlags = vk::DebugUtilsMessageTypeFlagBitsEXT;
        auto createInfo = vk::DebugUtilsMessengerCreateInfoEXT{};
        createInfo.messageSeverity =
            sevFlags::eVerbose |
            sevFlags::eInfo |
            sevFlags::eWarning |
            sevFlags::eError;
        createInfo.messageType =
            typeFlags::eGeneral |
            typeFlags::eValidation |
            typeFlags::ePerformance |
            typeFlags::eDeviceAddressBinding;
        createInfo.pfnUserCallback = debugLogger;
        return createInfo;
    }

    inline sstr translateSeverityFlagBits(VkDebugUtilsMessageSeverityFlagBitsEXT flags)
    {
        switch (flags) {
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
    inline sstr translateTypeFlags(VkDebugUtilsMessageTypeFlagsEXT flags)
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

    void setup(vk::Instance instance)
    {
        loader.init();
        loader.init(instance);

        messenger = instance.createDebugUtilsMessengerEXT(
            getMessengerCreateInfo(), nullptr, loader);
    }

    void cleanup(vk::Instance instance)
    {
        instance.destroyDebugUtilsMessengerEXT(messenger, nullptr, loader);
    }

    VkBool32 debugLogger(
        VkDebugUtilsMessageSeverityFlagBitsEXT severityFlag,
        VkDebugUtilsMessageTypeFlagsEXT typeFlags,
        const VkDebugUtilsMessengerCallbackDataEXT* data,
        void* userData)
    {
        auto prefix = SString("Daedalus Vulkan Debug");

        auto severityLabel = SString(translateSeverityFlagBits(severityFlag));
        auto typeLabel = SString(translateTypeFlags(typeFlags));

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
