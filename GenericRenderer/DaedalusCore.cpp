#include "stdinc.h"
#include "DaedalusCore.h"

#if defined(_DEBUG)
#include "DaedalusDebug.h"
#endif

#include <vulkan/vulkan.h>
#include "VulkanUtils.h"

namespace Daedalus
{
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice gpu = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

    inline bool success(VkResult res) { return res == VK_SUCCESS; }

    Result initialize()
    {
        if (instance != VK_NULL_HANDLE) {
            return Result::Failed;
        }

        // When checking against available layers and extensions,
        // unavailable items in the enabled* vectors are considered required,
        // and available items in the optional* vectors will be pushed onto the enabled* vectors.

        auto enabledLayers = List<cstr>();
#if defined(_DEBUG)
        enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

        auto enabledExtensions = List<cstr>();
#if defined(_DEBUG)
        enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
#if defined(_WINDOWS)
        enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
#if defined(_SPATIAL)
        extensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
#endif
        enabledExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
        // Should be obsoleted with vulkan api version 1.1
        enabledExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        //enabledExtensions.push_back(VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME);
        //enabledExtensions.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

        auto supportedLayers = VkUtil::GetSuppportedInstanceLayers();
        auto supportedExtensions = VkUtil::GetSupportedInstanceExtensions();
        auto supportedKHRONOSVALIDATIONExts = VkUtil::GetSupportedInstanceExtensions("VK_LAYER_KHRONOS_validation");

        auto appInfo = VkStruct::ApplicationInfo();
        appInfo.pEngineName = "Generic Renderer";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pApplicationName = "Generic Renderer";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        auto instanceCI = VkStruct::InstanceCreateInfo();
        instanceCI.pApplicationInfo = &appInfo;
        instanceCI.enabledLayerCount = enabledLayers.size();
        instanceCI.ppEnabledLayerNames = enabledLayers.data();
        instanceCI.enabledExtensionCount = enabledExtensions.size();
        instanceCI.ppEnabledExtensionNames = enabledExtensions.data();

#if defined(_DEBUG)
        auto debugUtilsMessengerCI = Debug::getDebugMessengerCreateInfo();
        debugUtilsMessengerCI.pNext = instanceCI.pNext;
        instanceCI.pNext = &debugUtilsMessengerCI;
#endif

        if (!success(vkCreateInstance(&instanceCI, nullptr, &instance))) {
            return Result::Failed;
        }

#if defined(_DEBUG)
        Debug::setup(instance);
#endif

        return Result::Success;
    }

    Result terminate()
    {
        if (device != VK_NULL_HANDLE) {
            vkDestroyDevice(device, nullptr);
        }
        if (surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(instance, surface, nullptr);
        }
        if (instance != VK_NULL_HANDLE) {
            Debug::cleanup(instance);
            vkDestroyInstance(instance, nullptr);
        }

        return Result::Success;
    }

    Result createDevice();

#if defined(_WINDOWS)
    Result createSurface(HINSTANCE hInstance, HWND hWnd)
    {
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hinstance = hInstance;
        createInfo.hwnd = hWnd;

        if (!success(vkCreateWin32SurfaceKHR(
            instance, &createInfo, nullptr, &surface))) {
            OutputDebugString(L"Daedalus: failed to create a Win32 Surface.\n");
            return Result::Failed;
        }

        return createDevice();
    }
#endif

    Result createDevice()
    {
        auto gpus = VkUtil::GetPhysicalDevices(instance);
        auto surfaceSupportedGPUs = List<VkPhysicalDevice>();

        for (auto& gpu : gpus) {
            auto capabilities = VkSurfaceCapabilitiesKHR{};
            if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &capabilities) != VK_SUCCESS) {
                // physical device and surface are not sympatico
                continue;
            }
            auto physicalDeviceProperties = VkPhysicalDeviceProperties{};
            vkGetPhysicalDeviceProperties(gpu, &physicalDeviceProperties);

            auto supported = true;
            // test against gpu properties and surface capabilities with this gpu.
            // requirements: ?
            if (!supported) {
                continue;
            }

            auto queueFamilyProperties = VkUtil::GetPhysicalDeviceQueueFamilyProperties(gpu);

            surfaceSupportedGPUs.push_back(gpu);
        }

        if (surfaceSupportedGPUs.size() == 0) {
            OutputDebugString(L"Unable to find a supported GPU.");
            return Result::Failed;
        }

        // Select a favored GPU or let the user decide.
        gpu = surfaceSupportedGPUs[0];

        auto supportedLayers = VkUtil::GetSupportedDeviceLayers(gpu);
        auto supportedExtensions = VkUtil::GetSupportedDeviceExtensions(gpu);

        auto queueCreateInfos = List<VkDeviceQueueCreateInfo>();
        queueCreateInfos.push_back(VkStruct::DeviceQueueCreateInfo());

        auto deviceFeatures = VkPhysicalDeviceFeatures{};

        auto enabledLayers = List<cstr>();
        auto optionalLayers = List<cstr>();

        auto enabledExtensions = List<cstr>();
        auto optionalExtensions = List<cstr>();
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

        for (auto& eExt : enabledExtensions) {
            auto found = false;
            for (auto& sExt : supportedExtensions) {
                found |= strcmp(eExt, sExt.extensionName) == 0;
            }
            if (!found) {
                OutputDebugString(L"Extension missing: ");
                OutputDebugStringA(eExt);
                OutputDebugString(L"\n");
            }
        }

        auto deviceCI = VkStruct::DeviceCreateInfo();
        deviceCI.queueCreateInfoCount = queueCreateInfos.size();
        deviceCI.pQueueCreateInfos = queueCreateInfos.data();
        deviceCI.enabledLayerCount = enabledLayers.size();
        deviceCI.ppEnabledLayerNames = enabledLayers.data();
        deviceCI.enabledExtensionCount = enabledExtensions.size();
        deviceCI.ppEnabledExtensionNames = enabledExtensions.data();
        deviceCI.pEnabledFeatures = &deviceFeatures;

        if (!success(vkCreateDevice(gpu, &deviceCI, nullptr, &device)))
        {
            OutputDebugString(L"Failed to create a device.\n");
            return Result::Failed;
        }
        return Result::Success;
    }
} // namespace Daedalus