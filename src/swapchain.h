#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H
#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <stdint.h>
#include "debug_messenger.h"
#include "app_structs.h"

typedef struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    uint32_t formats_len;
    VkPresentModeKHR* presentModes;
    uint32_t present_modes_len;
} SwapChainSupportDetails_t;

#define CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

QueueFamilyIndices_t findQueueFamilies(VkPhysicalDevice device, AppVariables_t* app);
SwapChainSupportDetails_t querySwapChainSupport(VkPhysicalDevice device, AppVariables_t* app);
void cleanupSwapChainSupport(SwapChainSupportDetails_t* details);
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* available_formats, uint32_t available_formats_len);
VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR* available_present_modes, uint32_t available_present_len);
VkExtent2D chooseSwapExtent(AppVariables_t* app, const VkSurfaceCapabilitiesKHR* capabilities);
void createSwapChain(AppVariables_t* app);
void freeSwapchainVars(SwapChain_t* swap_chain);
void createImageViews(AppVariables_t* app);
void createOverlayImages(AppVariables_t* app);
void createOverlayImageViews(AppVariables_t* app);
void createFramebuffers(AppVariables_t* app);
void cleanupSwapChain(AppVariables_t* app);
void recreateSwapChain(AppVariables_t* app);

#endif // SWAPCHAIN_H