#include "stdinc.h"
#include "DaedalusCore.h"

#if defined(_DEBUG)
#include "DaedalusDebug.h"
#endif

//#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace Daedalus
{
    vk::Instance instance = VK_NULL_HANDLE;
    vk::SurfaceKHR surface = VK_NULL_HANDLE;
    vk::Device device = VK_NULL_HANDLE;

    inline bool success(vk::Result res) { return res == vk::Result::eSuccess; }

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
        auto validationLayer = String("VK_LAYER_KHRONOS_validation");
        enabledLayers.push_back(validationLayer.c_str());
#endif

        auto enabledExts = List<cstr>();
#if defined(_DEBUG)
        enabledExts.push_back(vk::EXTDebugUtilsExtensionName);
#endif
#if defined(_WINDOWS)
        enabledExts.push_back(vk::KHRWin32SurfaceExtensionName);
#endif
#if defined(_SPATIAL)
        extensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
#endif
        enabledExts.push_back(vk::KHRSurfaceExtensionName);
        enabledExts.push_back(vk::KHRGetSurfaceCapabilities2ExtensionName);
        // Should be obsoleted with vulkan api version 1.1
        enabledExts.push_back(vk::KHRGetPhysicalDeviceProperties2ExtensionName);
        //enabledExtensions.push_back(VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME);
        //enabledExtensions.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

        auto supportedLayers = vk::enumerateInstanceLayerProperties();
        auto supportedExtensions = vk::enumerateInstanceExtensionProperties();
        auto supportedKhronosExts = vk::enumerateInstanceExtensionProperties(validationLayer);

        auto appInfo = vk::ApplicationInfo();
        appInfo.pEngineName = "Generic Renderer";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pApplicationName = "Generic Renderer";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        auto instanceCI = vk::InstanceCreateInfo();
        instanceCI.pApplicationInfo = &appInfo;
        instanceCI.enabledLayerCount = enabledLayers.size();
        instanceCI.ppEnabledLayerNames = enabledLayers.data();
        instanceCI.enabledExtensionCount = enabledExts.size();
        instanceCI.ppEnabledExtensionNames = enabledExts.data();

#if defined(_DEBUG)
        auto debugUtilsMessengerCI = Debug::getDebugMessengerCreateInfo();
        debugUtilsMessengerCI.pNext = instanceCI.pNext;
        instanceCI.pNext = &debugUtilsMessengerCI;
#endif

        instance = vk::createInstance(instanceCI);

#if defined(_DEBUG)
        Debug::setup(instance);
#endif

        return Result::Success;
    }

    Result terminate()
    {
        if (device != VK_NULL_HANDLE) {
            device.destroy();
        }
        if (surface != VK_NULL_HANDLE) {
            instance.destroy(surface);
        }
        if (instance != VK_NULL_HANDLE) {
            Debug::cleanup(instance);
            instance.destroy();
        }

        return Result::Success;
    }

    Result createDevice();

#if defined(_WINDOWS)
    Result createSurface(HINSTANCE hInstance, HWND hWnd)
    {
        auto createInfo = vk::Win32SurfaceCreateInfoKHR();
        createInfo.hinstance = hInstance;
        createInfo.hwnd = hWnd;
        surface = instance.createWin32SurfaceKHR(createInfo);

        return createDevice();
    }
#endif

    struct GPUInfo
    {
        vk::PhysicalDevice gpu;
        bool isDiscrete = false;
        u32 graphicsFamilyIdx = UINT32_MAX;
        u32 presentFamilyIdx = UINT32_MAX;
        u32 transferFamilyIdx = UINT32_MAX;
    };

    Result createDevice()
    {
        auto gpus = instance.enumeratePhysicalDevices();

        auto gpuInfos = List<GPUInfo>();
        // Acquire a GPU with both present and graphics capabilities.
        // If multiple GPUs support graphics and present, select:
        //physicalDeviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
        for (auto gpu : gpus) {
            auto info = GPUInfo();
            info.gpu = gpu;

            auto queueFamilyProperties = gpu.getQueueFamilyProperties();
            auto properties = gpu.getProperties();

            info.isDiscrete = properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;

            List<vk::Bool32> supportsPresent(queueFamilyProperties.size());
            for (auto i = 0; i < queueFamilyProperties.size(); i++) {
                supportsPresent[i] = gpu.getSurfaceSupportKHR(i, surface);
                if (info.presentFamilyIdx == UINT32_MAX && supportsPresent[i]) {
                    info.presentFamilyIdx = i;
                }
            }
            // For now, like with most Vulkan samples, we'll try to get a family
            // that supports both graphics and present.
            // Later, we might also pick a queue that can focus exclusively on
            // transfer operations for moving local<->device data.
            for (auto i = 0; i < queueFamilyProperties.size(); i++) {
                auto& family = queueFamilyProperties[i];
                if (family.queueFlags & vk::QueueFlagBits::eGraphics) {
                    if (supportsPresent[i]) {
                        info.graphicsFamilyIdx = i;
                        info.presentFamilyIdx = i;
                        break;
                    } else if (info.graphicsFamilyIdx == UINT32_MAX) {
                        info.graphicsFamilyIdx = i;
                    }
                }
            }
            if (info.graphicsFamilyIdx == UINT32_MAX ||
                info.presentFamilyIdx == UINT32_MAX) {
                continue;
            }
            gpuInfos.push_back(info);
        }
        if (gpuInfos.size() < 1) {
            OutputDebugString(L"Unable to find a GPU with graphics and present capabilities.");
            return Result::Failed;
        }

        // We could use this list to present a set of options to the user.
        auto gpuIdx = 0;
        auto idealGPUFound = false;
        for (auto i = 0; i < gpuInfos.size(); i++) {
            if (gpuInfos[i].graphicsFamilyIdx == gpuInfos[i].presentFamilyIdx &&
                gpuInfos[i].isDiscrete) {
                gpuIdx = i;
                idealGPUFound = true;
                break;
            }
        }
        if (!idealGPUFound) {
            OutputDebugString(L"GPU is not ideal.");
        }

        // Select a favored GPU or let the user decide.
        auto supportedLayers = gpuInfos[gpuIdx].gpu.enumerateDeviceLayerProperties();
        auto supportedExtensions = gpuInfos[gpuIdx].gpu.enumerateDeviceExtensionProperties();

        auto queuePriorities = 0.0f;
        auto queueCreateInfos = List<vk::DeviceQueueCreateInfo>();
        auto graphicsQueueCI = vk::DeviceQueueCreateInfo();
        graphicsQueueCI.queueFamilyIndex = gpuInfos[gpuIdx].graphicsFamilyIdx;
        graphicsQueueCI.queueCount = 1;
        graphicsQueueCI.pQueuePriorities = &queuePriorities;
        queueCreateInfos.push_back(graphicsQueueCI);
        if (gpuInfos[gpuIdx].graphicsFamilyIdx != gpuInfos[gpuIdx].presentFamilyIdx) {
            auto presentQueueCI = vk::DeviceQueueCreateInfo();
            presentQueueCI.queueFamilyIndex = gpuInfos[gpuIdx].presentFamilyIdx;
            presentQueueCI.queueCount = 1;
            presentQueueCI.pQueuePriorities = &queuePriorities;
            queueCreateInfos.push_back(presentQueueCI);
        }

        auto deviceFeatures = vk::PhysicalDeviceFeatures();

        auto enabledLayers = List<cstr>();
        auto optionalLayers = List<cstr>();

        auto enabledExtensions = List<cstr>();
        auto optionalExtensions = List<cstr>();
        // Core rendering feature.
        enabledExtensions.push_back(vk::KHRSwapchainExtensionName);
        // Variable rate shading
        optionalExtensions.push_back(vk::KHRFragmentShadingRateExtensionName);
        // Intended for optimization of Pipeline Cache compilation during an app's runtime.
        optionalExtensions.push_back(vk::EXTPipelineCreationCacheControlExtensionName);
        // Speeds up sequences of draw commands by loading them all and obviating state checks.
        optionalExtensions.push_back(vk::EXTMultiDrawExtensionName);
        // [Obsolete] This extension enables GPU culling
        //optionalExtensions.push_back(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
        // This extension may help optimize framebuffer attachments that are also used as inputs.
        // Note: not available on 1 out of 1 nvidia gpus.
        optionalExtensions.push_back(vk::EXTRasterizationOrderAttachmentAccessExtensionName);
        // For fun
        optionalExtensions.push_back(vk::EXTMeshShaderExtensionName);
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
#endif //_QCOM
#endif // _MOBILE
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
        optionalExtensions.push_back(vk::KHRPerformanceQueryExtensionName);
        // Note: not available on 1 out of 1 nvidia gpus.
        optionalExtensions.push_back(vk::EXTDeviceMemoryReportExtensionName);
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

        auto deviceCI = vk::DeviceCreateInfo();
        deviceCI.queueCreateInfoCount = queueCreateInfos.size();
        deviceCI.pQueueCreateInfos = queueCreateInfos.data();
        deviceCI.enabledLayerCount = enabledLayers.size();
        deviceCI.ppEnabledLayerNames = enabledLayers.data();
        deviceCI.enabledExtensionCount = enabledExtensions.size();
        deviceCI.ppEnabledExtensionNames = enabledExtensions.data();
        deviceCI.pEnabledFeatures = &deviceFeatures;

        device = gpuInfos[gpuIdx].gpu.createDevice(deviceCI);

        return Result::Success;
    }
} // namespace Daedalus