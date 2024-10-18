//
// Created by radue on 10/14/2024.
//

#pragma once

#include <vulkan/vulkan.hpp>

namespace Ext {
    struct DebugUtils {
        static void importFunctions(vk::Instance instance);

        static VkResult createDebugUtilsMessengerEXT(
            vk::Instance instance,
            const vk::DebugUtilsMessengerCreateInfoEXT &createInfo,
            const vk::AllocationCallbacks *allocator,
            vk::DebugUtilsMessengerEXT *debugMessenger);

        static void destroyDebugUtilsMessengerEXT(
            vk::Instance instance,
            vk::DebugUtilsMessengerEXT debugMessenger,
            const vk::AllocationCallbacks *allocator);

    private:
        static PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
        static PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
    };
} // Ext
