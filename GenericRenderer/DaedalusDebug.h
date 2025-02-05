#pragma once

#include <vulkan/vulkan.hpp>

namespace Engine::Daedalus::Debug
{
    inline vk::DebugUtilsMessengerCreateInfoEXT getMessengerCreateInfo();
    void setup(vk::Instance);
    void cleanup();
}
