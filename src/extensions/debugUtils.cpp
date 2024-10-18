//
// Created by radue on 10/14/2024.
//

#include "debugUtils.h"

namespace Ext {
    void DebugUtils::importFunctions(const vk::Instance instance) {
        vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
        vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
    }

    VkResult DebugUtils::createDebugUtilsMessengerEXT(
        const vk::Instance instance,
        const vk::DebugUtilsMessengerCreateInfoEXT &createInfo,
        const vk::AllocationCallbacks *allocator,
        vk::DebugUtilsMessengerEXT *debugMessenger) {
        if (vkCreateDebugUtilsMessengerEXT == nullptr) {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
        return vkCreateDebugUtilsMessengerEXT(
            instance,
            reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&createInfo),
            reinterpret_cast<const VkAllocationCallbacks*>(allocator),
            reinterpret_cast<VkDebugUtilsMessengerEXT*>(debugMessenger));
    }

    void DebugUtils::destroyDebugUtilsMessengerEXT(
        const vk::Instance instance,
        const vk::DebugUtilsMessengerEXT debugMessenger,
        const vk::AllocationCallbacks *allocator) {
        if (vkDestroyDebugUtilsMessengerEXT == nullptr) {
            return;
        }
        vkDestroyDebugUtilsMessengerEXT(
            instance,
            debugMessenger,
            reinterpret_cast<const VkAllocationCallbacks*>(allocator));
    }

    PFN_vkCreateDebugUtilsMessengerEXT DebugUtils::vkCreateDebugUtilsMessengerEXT = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT DebugUtils::vkDestroyDebugUtilsMessengerEXT = nullptr;

} // Ext