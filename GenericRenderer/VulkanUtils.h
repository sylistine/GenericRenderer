#pragma once

#include "stdinc.h"

#include <vulkan/vulkan.h>

namespace VkUtil
{
    inline std::vector<VkLayerProperties> GetSuppportedInstanceLayers()
    {
        uint32_t propertyCount;
        vkEnumerateInstanceLayerProperties(&propertyCount, nullptr);
        auto properties = std::vector<VkLayerProperties>(propertyCount);
        vkEnumerateInstanceLayerProperties(&propertyCount, properties.data());
        return properties;
    }

    inline std::vector<VkExtensionProperties> GetSupportedInstanceExtensions(const char* layer = nullptr)
    {
        uint32_t propertyCount;
        vkEnumerateInstanceExtensionProperties(layer, &propertyCount, nullptr);
        auto properties = std::vector<VkExtensionProperties>(propertyCount);
        vkEnumerateInstanceExtensionProperties(layer, &propertyCount, properties.data());
        return properties;
    }

    inline std::vector<VkLayerProperties> GetSupportedDeviceLayers(VkPhysicalDevice device)
    {
        uint32_t propertyCount;
        vkEnumerateDeviceLayerProperties(device, &propertyCount, nullptr);
        auto properties = std::vector<VkLayerProperties>(propertyCount);
        vkEnumerateDeviceLayerProperties(device, &propertyCount, properties.data());
        return properties;
    }

    inline std::vector<VkExtensionProperties> GetSupportedDeviceExtensions(VkPhysicalDevice device, const char* layer = nullptr)
    {
        uint32_t propertyCount;
        vkEnumerateDeviceExtensionProperties(device, layer, &propertyCount, nullptr);
        auto properties = std::vector<VkExtensionProperties>(propertyCount);
        vkEnumerateDeviceExtensionProperties(device, layer, &propertyCount, properties.data());
        return properties;
    }
}

namespace VkStruct
{
    inline VkApplicationInfo ApplicationInfo()
    {
        VkApplicationInfo info{};
        info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        return info;
    }

    inline VkInstanceCreateInfo InstanceCreateInfo()
    {
        VkInstanceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        return info;
    }

    inline VkDeviceQueueCreateInfo DeviceQueueCreateInfo()
    {
        VkDeviceQueueCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        return info;
    }

    inline VkDeviceCreateInfo DeviceCreateInfo()
    {
        VkDeviceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        return info;
    }
}