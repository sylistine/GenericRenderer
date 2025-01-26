#pragma once

#include "stdinc.h"

#include <vulkan/vulkan.h>

#include "VulkanUtils.h"

#if defined(_WINDOWS)
#include <Windows.h>
#endif

namespace Daedalus
{
    class Instance;
    class GPU;
    class Device;

    class Instance
    {
        static VkInstanceCreateInfo CreateInfo()
        {
            VkInstanceCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            return info;
        }
        static std::vector<VkLayerProperties> SupportedLayers()
        {
                uint32_t count;
                vkEnumerateInstanceLayerProperties(&count, nullptr);
                
                auto layers = std::vector<VkLayerProperties>(count);
                vkEnumerateInstanceLayerProperties(&count, layers.data());
                return layers;
        }
        static std::vector<VkExtensionProperties> SupportedExtensions(const char* layer = nullptr)
        {
            uint32_t count;
            vkEnumerateInstanceExtensionProperties(layer, &count, nullptr);
            
            auto extensions = std::vector<VkExtensionProperties>(count);
            vkEnumerateInstanceExtensionProperties(layer, &count, extensions.data());
            return extensions;
        }
        static Instance Create(VkInstanceCreateInfo* info)
        {
            VkInstance instance;
            vkCreateInstance(info, nullptr, &instance);
            return Instance(instance);
        }

        VkInstance instance = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        std::vector<GPU> gpus;
    public:
        Instance(VkInstance instance);
        ~Instance();
        operator VkInstance();
        void Destroy();
#if defined(_WINDOWS)
        void CreateWin32SurfaceKHR(HINSTANCE, HWND);
#endif
        Device CreateDevice(VkDeviceCreateInfo);
    };

    class GPU
    {
        VkPhysicalDevice gpu;
    public:
        GPU()
        {
            // do nothing;
        }
        GPU(VkPhysicalDevice physicalDevice);
        operator VkPhysicalDevice() { return gpu; }

        std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties() {
            uint32_t propertyCount;
            vkGetPhysicalDeviceQueueFamilyProperties(gpu, &propertyCount, nullptr);
            std::vector<VkQueueFamilyProperties> properties(propertyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(gpu, &propertyCount, properties.data());
            return properties;
        }
        std::vector<VkLayerProperties> SupportedLayers();
        std::vector<VkExtensionProperties> SupportedExtensions(const char* layer = nullptr);
    };

    class Device
    {
        VkDevice device;
    public:
        Device(VkDevice device) : device(device) {}
        void  Destroy()
        {
            if (device != VK_NULL_HANDLE) {
                vkDestroyDevice(device, nullptr);
            }
        }
    };
}
