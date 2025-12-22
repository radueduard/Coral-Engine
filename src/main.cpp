

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif

#include "engine.h"
#include "gui/elements/popup.h"

int main()
{
#if ( VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1 )
	VULKAN_HPP_DEFAULT_DISPATCHER.init();
#endif

	const vk::detail::DynamicLoader dl;
	VULKAN_HPP_DEFAULT_DISPATCHER.init( dl );

	const PFN_vkGetInstanceProcAddr getInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>( "vkGetInstanceProcAddr" );
	VULKAN_HPP_DEFAULT_DISPATCHER.init( getInstanceProcAddr );

    Coral::Engine().Run();
    return 0;
}
