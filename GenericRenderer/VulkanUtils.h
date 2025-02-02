#pragma once

#include "stdinc.h"

#include <vulkan/vulkan.h>

namespace VkUtil
{
    inline List<VkLayerProperties> GetSuppportedInstanceLayers()
    {
        u32 propCount;
        vkEnumerateInstanceLayerProperties(&propCount, nullptr);
        auto props = List<VkLayerProperties>(propCount);
        vkEnumerateInstanceLayerProperties(&propCount, props.data());
        return props;
    }

    inline List<VkExtensionProperties> GetSupportedInstanceExtensions(
        cstr layer = nullptr)
    {
        u32 propCount;
        vkEnumerateInstanceExtensionProperties(layer, &propCount, nullptr);
        auto props = List<VkExtensionProperties>(propCount);
        vkEnumerateInstanceExtensionProperties(layer, &propCount, props.data());
        return props;
    }

    inline List<VkLayerProperties> GetSupportedDeviceLayers(
        VkPhysicalDevice device)
    {
        u32 propCount;
        vkEnumerateDeviceLayerProperties(device, &propCount, nullptr);
        auto props = List<VkLayerProperties>(propCount);
        vkEnumerateDeviceLayerProperties(device, &propCount, props.data());
        return props;
    }

    inline List<VkExtensionProperties> GetSupportedDeviceExtensions(
        VkPhysicalDevice device,
        const char* layer = nullptr)
    {
        u32 propCount;
        vkEnumerateDeviceExtensionProperties(device, layer, &propCount, nullptr);
        auto props = List<VkExtensionProperties>(propCount);
        vkEnumerateDeviceExtensionProperties(device, layer, &propCount, props.data());
        return props;
    }

    inline List<VkPhysicalDevice> GetPhysicalDevices(VkInstance instance)
    {
        u32 gpuCount;
        vkEnumeratePhysicalDevices(
            instance, &gpuCount, nullptr);
        auto gpus = List<VkPhysicalDevice>(gpuCount);
        vkEnumeratePhysicalDevices(
            instance, &gpuCount, gpus.data());
        return gpus;
    }

    inline List<VkQueueFamilyProperties> GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice gpu)
    {
        u32 propertyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(
            gpu, &propertyCount, nullptr);
        List<VkQueueFamilyProperties> properties(propertyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(
            gpu, &propertyCount, properties.data());
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