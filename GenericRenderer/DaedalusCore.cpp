#include "Precompiled.h"
#include "DaedalusCore.h"

#if defined(_DEBUG)
#include "DaedalusDebug.h"
#endif

#include <vulkan/vulkan.hpp>
#include "VulkanUtils.h"

namespace Engine::Daedalus
{
    struct GPUProfile
    {
        vk::PhysicalDevice gpu;
        bool isDiscrete = false;
        u32 gfxFamilyIdx = UINT32_MAX;
        u32 cmpFamilyIdx = UINT32_MAX;
        u32 presentFamilyIdx = UINT32_MAX;
        u32 dedicatedTfrFamilyIdx = UINT32_MAX;
    };

    vk::Instance instance = VK_NULL_HANDLE;
    vk::SurfaceKHR surface = VK_NULL_HANDLE;
    List<GPUProfile> gpuProfiles;
    u32 activeGPUIdx = UINT32_MAX;
    vk::Device device = VK_NULL_HANDLE;
    vk::CommandPool gfxCmdPool = VK_NULL_HANDLE;
    vk::CommandPool presentCmdPool = VK_NULL_HANDLE;
    vk::CommandPool tfrCmdPool = VK_NULL_HANDLE;
    vk::DispatchLoaderDynamic loader;

    inline bool success(vk::Result res) { return res == vk::Result::eSuccess; }

    Result initialize()
    {
        if (instance != VK_NULL_HANDLE) {
            return Result::Failed;
        }

        // When checking against available layers and extensions,
        // unavailable items in the enabled* vectors are considered required,
        // and available items in the optional* vectors will be pushed onto the enabled* vectors.

        auto enabledLayers = List<sstr>();
#if defined(_DEBUG)
        auto validationLayer = SString("VK_LAYER_KHRONOS_validation");
        enabledLayers.push_back(validationLayer.c_str());
#endif

        auto enabledExts = List<sstr>();
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
        instanceCI.enabledLayerCount = (u32)enabledLayers.size();
        instanceCI.ppEnabledLayerNames = enabledLayers.data();
        instanceCI.enabledExtensionCount = (u32)enabledExts.size();
        instanceCI.ppEnabledExtensionNames = enabledExts.data();

#if defined(_DEBUG)
        auto debugUtilsMessengerCI = Debug::getMessengerCreateInfo();
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
        if (gfxCmdPool != VK_NULL_HANDLE) {
            device.destroyCommandPool(gfxCmdPool);
            if (presentCmdPool != gfxCmdPool) {
                device.destroyCommandPool(presentCmdPool);
            }
            if (tfrCmdPool != gfxCmdPool) {
                device.destroyCommandPool(tfrCmdPool);
            }
            gfxCmdPool = VK_NULL_HANDLE;
            presentCmdPool = VK_NULL_HANDLE;
            tfrCmdPool = VK_NULL_HANDLE;
        }

        if (device != VK_NULL_HANDLE) {
            device.destroy();
        }
        if (surface != VK_NULL_HANDLE) {
            instance.destroy(surface);
        }
        if (instance != VK_NULL_HANDLE) {
            Debug::cleanup();
            instance.destroy();
        }

        return Result::Success;
    }

    Result createDevice()
    {
        auto physicalDevices = instance.enumeratePhysicalDevices();

        // Acquire a GPU with both present and graphics capabilities.
        // If multiple GPUs support graphics and present, select:
        //physicalDeviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
        Engine::Debug::Log(u"============Physical Device Info============\n");
        for (auto gpu : physicalDevices) {
#if defined(_DEBUG)
            // A helpful visualization of queue family properties.
            Engine::Debug::Log(VkUtil::to_prettyString(gpu, surface).c_str());
#endif
            auto profile = GPUProfile();
            profile.gpu = gpu;

            auto queueFamilyProperties = gpu.getQueueFamilyProperties();
            auto properties = gpu.getProperties();

            profile.isDiscrete = properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;

            List<vk::Bool32> supportsPresent(queueFamilyProperties.size());
            for (auto i = 0; i < queueFamilyProperties.size(); i++) {
                supportsPresent[i] = gpu.getSurfaceSupportKHR(i, surface);
                if (profile.presentFamilyIdx == UINT32_MAX && supportsPresent[i]) {
                    profile.presentFamilyIdx = i;
                }
            }
            // For now, like with most Vulkan samples, we'll try to get a family
            // that supports both graphics and present.
            // Later, we might also pick a queue that can focus exclusively on
            // transfer operations for moving local<->device data.
            for (auto i = 0; i < queueFamilyProperties.size(); i++) {
                auto& flags = queueFamilyProperties[i].queueFlags;
                auto gfxFlag = vk::QueueFlagBits::eGraphics;
                auto cmpFlag = vk::QueueFlagBits::eCompute;
                auto tfrFlag = vk::QueueFlagBits::eTransfer;

                if (flags & gfxFlag) {
                    if (supportsPresent[i]) {
                        profile.gfxFamilyIdx = i;
                        profile.presentFamilyIdx = i;
                        break;
                    } else if (profile.gfxFamilyIdx == UINT32_MAX) {
                        profile.gfxFamilyIdx = i;
                    }
                }
                if ((flags & tfrFlag) && !(flags & gfxFlag) && !(flags & cmpFlag)) {
                    profile.dedicatedTfrFamilyIdx = i;
                }
            }
            if (profile.gfxFamilyIdx == UINT32_MAX ||
                profile.presentFamilyIdx == UINT32_MAX) {
                continue;
            }
            gpuProfiles.push_back(profile);
        }
        Engine::Debug::Log(u"============================================\n");

        if (gpuProfiles.size() < 1) {
            OutputDebugString(L"Unable to find a GPU with graphics and present capabilities.");
            return Result::Failed;
        }

        for (auto i = 0; i < gpuProfiles.size(); i++) {
            if (gpuProfiles[i].gfxFamilyIdx == gpuProfiles[i].presentFamilyIdx &&
                gpuProfiles[i].isDiscrete) {
                activeGPUIdx = i;
                break;
            }
        }
        if (activeGPUIdx == UINT32_MAX) {
            OutputDebugString(L"GPU is not ideal.");
        }

        // Select a favored GPU or let the user decide.
        auto profile = gpuProfiles[activeGPUIdx];
        auto supportedLayers = profile.gpu.enumerateDeviceLayerProperties();
        auto supportedExtensions = profile.gpu.enumerateDeviceExtensionProperties();

        auto queuePriorities = 0.0f;
        auto queueCreateInfos = List<vk::DeviceQueueCreateInfo>();
        auto graphicsQueueCI = vk::DeviceQueueCreateInfo();
        graphicsQueueCI.queueFamilyIndex = profile.gfxFamilyIdx;
        graphicsQueueCI.queueCount = 1;
        graphicsQueueCI.pQueuePriorities = &queuePriorities;
        queueCreateInfos.push_back(graphicsQueueCI);
        if (profile.gfxFamilyIdx != profile.presentFamilyIdx) {
            auto presentQueueCI = vk::DeviceQueueCreateInfo();
            presentQueueCI.queueFamilyIndex = profile.presentFamilyIdx;
            presentQueueCI.queueCount = 1;
            presentQueueCI.pQueuePriorities = &queuePriorities;
            queueCreateInfos.push_back(presentQueueCI);
        }

        auto deviceFeatures = vk::PhysicalDeviceFeatures();

        auto layers = List<sstr>();
        auto optLayers = List<sstr>();

        auto extensions = List<sstr>();
        auto optExtensions = List<sstr>();
        // Create Extension Lists
        {
            // Core rendering feature.
            extensions.push_back(vk::KHRSwapchainExtensionName);
            // Variable rate shading
            optExtensions.push_back(vk::KHRFragmentShadingRateExtensionName);
            // Intended for optimization of Pipeline Cache compilation during an app's runtime.
            optExtensions.push_back(vk::EXTPipelineCreationCacheControlExtensionName);
            // Speeds up sequences of draw commands by loading them all and obviating state checks.
            optExtensions.push_back(vk::EXTMultiDrawExtensionName);
            // [Obsolete] This extension enables GPU culling
            //optExtensions.push_back(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
            // This extension may help optimize framebuffer attachments that are also used as inputs.
            // Note: not available on 1 out of 1 nvidia gpus.
            optExtensions.push_back(vk::EXTRasterizationOrderAttachmentAccessExtensionName);
            // For fun
            optExtensions.push_back(vk::EXTMeshShaderExtensionName);
#if defined(_SPATIAL)
            // Core rendering feature in spatial applications.
            enabledExtensions.push_back(vk::KHRMultiviewExtensionName);
            // Enables foveated rendering.
            // Note: fragment density mmap extension not available on nvidia device.
            // Maybe only on tiled renderers?
            optExtensions.push_back(vk::EXTFragmentDensityMapExtensionName);
            // Enables more  performant foveated rendering.
            optExtensions.push_back(vk::EXTFragmentDensityMap2ExtensionName);
#if defined(_QCOM)
            // Enables high performance density map offsets, i.e. in gaze-based foveation.
            optExtensions.push_back(vk::QCOMFragmentDensityMapOffsetExtensionName);
#endif // _QCOM
#endif // _SPATIAL
#if defined(_MOBILE)
            // Might be necessary on mobile platforms. Maybe also on IOS?
            extensions.push_back(vk::KHRExternalMemoryCapabilitiesExtensionName);
            extensions.push_back(vk::KHRExternalMemoryExtensionName);
            // Enables programmable blending in tiled renderers. Maybe useful for color grading.
            // Depends on dynamic rendering.
            optExtensions.push_back(vk::EXTShaderTileImageExtensionName);
#if defined(_QCOM)
            // Provides tile information to the application. For... debugging?
            optExtensions.push_back(vk::QCOMTilePropertiesExtensionName);
            // Allows custom logic when writing tiles out to shared memory.
            optExtensions.push_back(vk::QCOMRenderPassShaderResolveExtensionName);
            // Allows driver-level handling of image transform changes for performance.
            optExtensions.push_back(vk::QCOMRenderPassTransformExtensionName);
            // Useful for attachments that are tested against but never written to,
            // such as a depth texture used after a depth pre-pass.
            optExtensions.push_back(vk::QCOMRenderPassStoreOpsExtensionName);
            // Enables some sampler filters for image processing.
            // Perhaps enabling some kind of bloom or bokeh in a custom resolve shader?
            optExtensions.push_back(vk::QCOMImageProcessingExtensionName);
            optExtensions.push_back(vk::QCOMImageProcessing2ExtensionName);
#endif //_QCOM
#endif // _MOBILE
#if defined(_RAYTRACING)
            extensions.push_back(vk::KHRRayTracingPositionFetchExtensionName);
            extensions.push_back(vk::KHRRayQueryExtensionName);
            extensions.push_back(vk::KHRAccelerationStructureExtensionName);
            extensions.push_back(vk::KHRRayTracingPositionFetchExtensionName);
            // Enables blending ray tracing pipelines with rasterization pipelines.
            extensions.push_back(vk::KHRRayTracingPipelineExtensionName);
#if defined(_DEBUG)
            extensions.push_back(vk::NVRayTracingValidationExtensionName);
#endif // _DEBUG
            // if  nvidia
            optExtensions.push_back(vk::NVRayTracingMotionBlurExtensionName);
#endif // _RAYTRACING
#if defined(_DEBUG)
            // Note: not available on 1 out of 1 nvidia gpus.
            optExtensions.push_back(vk::KHRPerformanceQueryExtensionName);
            // Note: not available on 1 out of 1 nvidia gpus.
            optExtensions.push_back(vk::EXTDeviceMemoryReportExtensionName);
#endif

            for (auto& eExt : extensions) {
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
        }
        
        // Create Device
        {
            auto info = vk::DeviceCreateInfo();
            info.queueCreateInfoCount = (u32)queueCreateInfos.size();
            info.pQueueCreateInfos = queueCreateInfos.data();
            info.enabledLayerCount = (u32)layers.size();
            info.ppEnabledLayerNames = layers.data();
            info.enabledExtensionCount = (u32)extensions.size();
            info.ppEnabledExtensionNames = extensions.data();
            info.pEnabledFeatures = &deviceFeatures;
            device = profile.gpu.createDevice(info);
        }
        
        // Create Command Pools
        {
            auto info = vk::CommandPoolCreateInfo();
            info.queueFamilyIndex = profile.gfxFamilyIdx;
            gfxCmdPool = device.createCommandPool(info);
            if (profile.presentFamilyIdx != profile.gfxFamilyIdx) {
                info.queueFamilyIndex = profile.presentFamilyIdx;
                presentCmdPool = device.createCommandPool(info);
            } else {
                presentCmdPool = gfxCmdPool;
            }
            if (profile.dedicatedTfrFamilyIdx != UINT32_MAX) {
                info.queueFamilyIndex = profile.dedicatedTfrFamilyIdx;
                tfrCmdPool = device.createCommandPool(info);
            } else {
                tfrCmdPool = gfxCmdPool;
            }
        }

        return Result::Success;
    }

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

} // namespace Engine::Daedalus