#include "stdinc.h"

#include "DaedalusVkClasses.h"

namespace Daedalus
{
    Instance::Instance(VkInstance instance) :
        instance(instance)
    {
        uint32_t count;
        vkEnumeratePhysicalDevices(instance, &count, nullptr);
        auto vkgpus = std::vector<VkPhysicalDevice>(count);
        gpus = std::vector<GPU>(count);
        vkEnumeratePhysicalDevices(instance, &count, vkgpus.data());
        for (auto vkgpu : vkgpus) {
            gpus.push_back(GPU(vkgpu));
        }
    }
    Instance::~Instance() {}
    Instance::operator VkInstance()
    {
        return instance;
    }
    void Instance::Destroy()
    {
        vkDestroyInstance(instance, nullptr);
    }

#if defined(_WINDOWS)
    void Instance::CreateWin32SurfaceKHR(HINSTANCE hInstance, HWND hWnd)
    {
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hinstance = hInstance;
        createInfo.hwnd = hWnd;

        VkSurfaceKHR surface;
        if (VK_SUCCESS != vkCreateWin32SurfaceKHR(
            instance, &createInfo, nullptr, &surface)) {
            OutputDebugString(L"Daedalus: failed to create a Win32 Surface.\n");
            return;
        }
    }
#endif
    Device Instance::CreateDevice(VkDeviceCreateInfo info)
    {
        auto enabledLayers = std::vector<const char*>();
        auto optionalLayers = std::vector<const char*>();

        auto enabledExtensions = std::vector<const char*>();
        auto optionalExtensions = std::vector<const char*>();
        // Core rendering feature.
        enabledExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        // Variable rate shading
        optionalExtensions.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
        // Intended for optimization of Pipeline Cache compilation during an app's runtime.
        optionalExtensions.push_back(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME);
        // Speeds up sequences of draw commands by loading them all and obviating state checks.
        optionalExtensions.push_back(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
        // [Obsolete] This extension enables GPU culling
        //optionalExtensions.push_back(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
        // This extension may help optimize framebuffer attachments that are also used as inputs.
        // Note: not available on 1 out of 1 nvidia gpus.
        optionalExtensions.push_back(VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME);
        // For fun
        optionalExtensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
#if defined(_SPATIAL)
        // Core rendering feature in spatial applications.
        enabledExtensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        // Enables foveated rendering.
        // Note: fragment density mmap extension not available on nvidia device.
        // Maybe only on tiled renderers?
        optionalExtensions.push_back(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
        // Enables more  performant foveated rendering.
        optionalExtensions.push_back(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);
#if defined(_QCOM)
        // Enables high performance density map offsets, i.e. in gaze-based foveation.
        optionalExtensions.push_back(VK_QCOM_FRAGMENT_DENSITY_MAP_OFFSET_EXTENSION_NAME);
#endif // _QCOM
#endif // _SPATIAL
#if defined(_MOBILE)
        // Might be necessary on mobile platforms. Maybe also on IOS?
        enabledExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        // Enables programmable blending in tiled renderers. Maybe useful for color grading.
        // Depends on dynamic rendering.
        optionalExtensions.push_back(VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME);
#if defined(_QCOM)
        // Provides tile information to the application. For... debugging?
        optionalExtensions.push_back(VK_QCOM_TILE_PROPERTIES_EXTENSION_NAME);
#endif //_QCOM
#endif // _MOBILE
#if defined(_QCOM)
        // Allows custom logic when writing tiles out to shared memory.
        optionalExtensions.push_back(VK_QCOM_RENDER_PASS_SHADER_RESOLVE_EXTENSION_NAME);
        // Allows driver-level handling of image transform changes for performance.
        optionalExtensions.push_back(VK_QCOM_RENDER_PASS_TRANSFORM_EXTENSION_NAME);
        // Useful for attachments that are tested against but never written to,
        // such as a depth texture used after a depth pre-pass.
        optionalExtensions.push_back(VK_QCOM_RENDER_PASS_STORE_OPS_EXTENSION_NAME);
        // Enables some sampler filters for image processing.
        // Perhaps enabling some kind of bloom or bokeh in a custom resolve shader?
        optionalExtensions.push_back(VK_QCOM_IMAGE_PROCESSING_EXTENSION_NAME);
        optionalExtensions.push_back(VK_QCOM_IMAGE_PROCESSING_2_EXTENSION_NAME);
#endif // _QCOM
#if defined(_RAYTRACING)
        enabledExtensions.push_back(VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME);
        // Enables blending ray tracing pipelines with rasterization pipelines.
        enabledExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
#if defined(_DEBUG)
        enabledExtensions.push_back(VK_NV_RAY_TRACING_VALIDATION_EXTENSION_NAME);
#endif // _DEBUG
        // if  nvidia
        optionalExtensions.push_back(VK_NV_RAY_TRACING_MOTION_BLUR_EXTENSION_NAME);
#endif // _RAYTRACING
#if defined(_DEBUG)
        // Note: not available on 1 out of 1 nvidia gpus.
        optionalExtensions.push_back(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
        // Note: not available on 1 out of 1 nvidia gpus.
        optionalExtensions.push_back(VK_EXT_DEVICE_MEMORY_REPORT_EXTENSION_NAME);
#endif

        // basaed on layer and extension requirements, pick the best physical device.
        auto supportedGPUs = std::vector<GPU>();
        for (auto& gpu : gpus) {
            auto supportedExtensions = gpu.SupportedExtensions();
            auto gpuIsSupported = true;
            for (auto& eExt : enabledExtensions) {
                auto found = false;
                for (auto& sExt : supportedExtensions) {
                    found |= strcmp(eExt, sExt.extensionName) == 0;
                }
                if (!found) {
                    OutputDebugString(L"Extension missing: ");
                    OutputDebugStringA(eExt);
                    OutputDebugString(L"\n");
                    gpuIsSupported = false;
                    break;
                }
            }
            if (gpuIsSupported) {
                supportedGPUs.push_back(gpu);
            }
        }

        if (supportedGPUs.size() < 1) {
            OutputDebugString(L"No supported GPUs found.");
            // break with exception.
        }

        // maybe pick the best gpu. but in the mean time, we'll pick any gpu.
        // we're skipping the critical queue create  info step here.
        VkPhysicalDevice gpu = supportedGPUs[0];
        auto queueCreateInfos = std::vector<VkDeviceQueueCreateInfo>();
        queueCreateInfos.push_back(VkStruct::DeviceQueueCreateInfo());
        auto deviceFeatures = VkPhysicalDeviceFeatures{};

        auto deviceCI = VkStruct::DeviceCreateInfo();
        deviceCI.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
        deviceCI.pQueueCreateInfos = queueCreateInfos.data();
        deviceCI.enabledLayerCount = (uint32_t)enabledLayers.size();
        deviceCI.ppEnabledLayerNames = enabledLayers.data();
        deviceCI.enabledExtensionCount = (uint32_t)enabledExtensions.size();
        deviceCI.ppEnabledExtensionNames = enabledExtensions.data();
        deviceCI.pEnabledFeatures = &deviceFeatures;

        VkDevice device;
        if (VK_SUCCESS != vkCreateDevice(
            gpu, &deviceCI, nullptr, &device)) {
            OutputDebugString(L"Failed to create a device.\n");
        }
        return Device(device);
    }
    
    GPU::GPU(VkPhysicalDevice physicalDevice) : gpu(physicalDevice)
    {
    
    }

    std::vector<VkLayerProperties> GPU::SupportedLayers()
    {
        uint32_t propertyCount;
        vkEnumerateDeviceLayerProperties(gpu, &propertyCount, nullptr);
        auto properties = std::vector<VkLayerProperties>(propertyCount);
        vkEnumerateDeviceLayerProperties(gpu, &propertyCount, properties.data());
        return properties;
    }

    std::vector<VkExtensionProperties> GPU::SupportedExtensions(const char* layer)
    {
        uint32_t propertyCount;
        vkEnumerateDeviceExtensionProperties(gpu, layer, &propertyCount, nullptr);
        auto properties = std::vector<VkExtensionProperties>(propertyCount);
        vkEnumerateDeviceExtensionProperties(gpu, layer, &propertyCount, properties.data());
        return properties;
    }
}
