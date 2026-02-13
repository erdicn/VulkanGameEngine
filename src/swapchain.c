#include "swapchain.h"
#include "graphics_pipeline.h"
#include "defines.h"
#ifdef USE_OVERLAY
#include "overlay.h"
#endif
QueueFamilyIndices_t findQueueFamilies(VkPhysicalDevice device, AppVariables_t* app) {
    QueueFamilyIndices_t indices = {};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
    
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    for (int i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            indices.hasGraphicsFamily = true;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, app->surface, &presentSupport);  
        if (presentSupport) {
            indices.presentFamily = i;
            indices.hasPresentFamily = true;
        }
        if (indices.hasGraphicsFamily && indices.hasPresentFamily) {
            break;
        }
    }

    return indices;
}

SwapChainSupportDetails_t querySwapChainSupport(VkPhysicalDevice device, AppVariables_t* app){
    SwapChainSupportDetails_t details = {};
    
    testVK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, app->surface, &details.capabilities)); 

    testVK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, app->surface, &details.formats_len, NULL));

    if (details.formats_len != 0) {
        details.formats = (VkSurfaceFormatKHR*)realloc(details.formats, sizeof(VkSurfaceFormatKHR)*details.formats_len);
        testVK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, app->surface, &details.formats_len, details.formats));
    }

    testVK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, app->surface, &details.present_modes_len, NULL));

    if (details.present_modes_len != 0) {
        details.presentModes = (VkPresentModeKHR*)realloc(details.presentModes, sizeof(VkPresentModeKHR)*details.present_modes_len);
        testVK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, app->surface, &details.present_modes_len, details.presentModes));
    }

    return details;
}

void cleanupSwapChainSupport(SwapChainSupportDetails_t* details) {
    free(details->formats);
    free(details->presentModes);
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* available_formats, uint32_t available_formats_len) {
    for (uint32_t i = 0; i < available_formats_len; i++){
        VkSurfaceFormatKHR availableFormat = available_formats[i]; 
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return available_formats[0];
}

/*
VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application are transferred to the screen right away, which may result in tearing.
VK_PRESENT_MODE_FIFO_KHR: The swap chain is a queue where the display takes an image from the front of the queue when the display is refreshed and the program inserts rendered images at the back of the queue. If the queue is full then the program has to wait. This is most similar to vertical sync as found in modern games. The moment that the display is refreshed is known as "vertical blank".
VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs from the previous one if the application is late and the queue was empty at the last vertical blank. Instead of waiting for the next vertical blank, the image is transferred right away when it finally arrives. This may result in visible tearing.
VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the second mode. Instead of blocking the application when the queue is full, the images that are already queued are simply replaced with the newer ones. This mode can be used to render frames as fast as possible while still avoiding tearing, resulting in fewer latency issues than standard vertical sync. This is commonly known as "triple buffering", although the existence of three buffers alone does not necessarily mean that the framerate is unlocked
*/
VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR* available_present_modes, uint32_t available_present_len) {
    for (uint32_t i = 0; i < available_present_len; i++) {
        VkPresentModeKHR availablePresentMode = available_present_modes[i];
        if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR){//VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}


VkExtent2D chooseSwapExtent(AppVariables_t* app, const VkSurfaceCapabilitiesKHR* capabilities) {
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(app->window, &width, &height);

        VkExtent2D actualExtent = {
            (uint32_t)(width),
            (uint32_t)(height)
        };

        actualExtent.width  = CLAMP(actualExtent.width , capabilities->minImageExtent.width , capabilities->maxImageExtent.width );
        actualExtent.height = CLAMP(actualExtent.height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);

        return actualExtent;
    }
}

void createSwapChain(AppVariables_t* app) {
    SwapChainSupportDetails_t swapChainSupport = querySwapChainSupport(app->physical_device, app);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, swapChainSupport.formats_len);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, swapChainSupport.present_modes_len);
    // printf("The present mode choosen for app is %d\nVK_PRESENT_MODE_IMMEDIATE_KHR(0) VK_PRESENT_MODE_FIFO_KHR(2) VK_PRESENT_MODE_FIFO_RELAXED_KHR(3) VK_PRESENT_MODE_MAILBOX_KHR(1)\n", presentMode);
    VkExtent2D extent = chooseSwapExtent(app, &swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo={};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = app->surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    QueueFamilyIndices_t indices = findQueueFamilies(app->physical_device, app);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    testVK(vkCreateSwapchainKHR(app->device, &createInfo, NULL, &app->swap_chain.swap_chain));
    testVK(vkGetSwapchainImagesKHR(app->device, app->swap_chain.swap_chain, &imageCount, NULL));
    app->swap_chain.images = (VkImage*)realloc(app->swap_chain.images, imageCount*sizeof(VkImage)); 
    app->swap_chain.images_len = imageCount;
    testVK(vkGetSwapchainImagesKHR(app->device, app->swap_chain.swap_chain, &imageCount, app->swap_chain.images));

    app->swap_chain.image_format = surfaceFormat.format;
    app->swap_chain.extend = extent;
    cleanupSwapChainSupport(&swapChainSupport);
}

void freeSwapchainVars(SwapChain_t* swap_chain){
    free(swap_chain->images);
    free(swap_chain->image_views);
    free(swap_chain->frame_buffers);
}

void createImageViews(AppVariables_t* app) {
    app->swap_chain.image_views_len = app->swap_chain.images_len;
    app->swap_chain.image_views = (VkImageView*)realloc(app->swap_chain.image_views, sizeof(VkImageView)*app->swap_chain.image_views_len);

    for (size_t i = 0; i < app->swap_chain.images_len; i++) {
        VkImageViewCreateInfo createInfo ={};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = app->swap_chain.images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = app->swap_chain.image_format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        testVK(vkCreateImageView(app->device, &createInfo, NULL, &app->swap_chain.image_views[i]));
    }
}

void createFramebuffers(AppVariables_t* app){
    app->swap_chain.frame_buffers_len = app->swap_chain.image_views_len;
    app->swap_chain.frame_buffers = (VkFramebuffer*)realloc(app->swap_chain.frame_buffers, sizeof(VkFramebuffer)*app->swap_chain.frame_buffers_len);
    // collectGarbage(app->swap_chain.frame_buffers, app);

    for(size_t i = 0; i < app->swap_chain.image_views_len; i++){
         VkImageView attachments[] = {
            app->swap_chain.image_views[i]
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = app->render_pass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width  = app->swap_chain.extend.width;
        framebufferInfo.height = app->swap_chain.extend.height;
        framebufferInfo.layers = 1;

        testVK(vkCreateFramebuffer(app->device, &framebufferInfo, NULL, &app->swap_chain.frame_buffers[i]) != VK_SUCCESS);
    }
}

void cleanupSwapChain(AppVariables_t* app){
    for(size_t i = 0; i < app->swap_chain.frame_buffers_len; i++){
        vkDestroyFramebuffer(app->device, app->swap_chain.frame_buffers[i], NULL);
    }
    #ifdef USE_OVERLAY    
        vkFreeCommandBuffers(app->device, app->command_pool,
                            (uint32_t)(MAX_FRAMES_IN_FLIGHT), app->command_buffers);

        vkDestroyPipeline(app->device, app->overlay.triangle_graphics_pipeline, NULL);
        vkDestroyPipelineLayout(app->device, app->overlay.triangle_pipeline_layout, NULL);
        vkDestroyPipeline(app->device, app->overlay.graphics_pipeline, NULL);
        vkDestroyPipelineLayout(app->device, app->overlay.pipeline_layout, NULL);
        vkDestroyRenderPass(app->device, app->render_pass, NULL);

        for (size_t i = 0; i < app->overlay.image_views_len; i++) {
        vkDestroyImageView(app->device, app->overlay.image_views[i], NULL);
        }
        for (size_t i = 0; i < app->overlay.images_len; i++) {
        vkDestroyImage(app->device, app->overlay.images[i], NULL);
        }
        for (size_t i = 0; i < app->overlay.image_memories_len; i++) {
        vkFreeMemory(app->device, app->overlay.image_memories[i], NULL);
        }
    #endif

    for(size_t i = 0; i < app->swap_chain.image_views_len; i++){
        vkDestroyImageView(app->device, app->swap_chain.image_views[i], NULL);
    }
    // freeSwapchainVars(&app->swap_chain);

    vkDestroySwapchainKHR(app->device, app->swap_chain.swap_chain, NULL);
}

void createOverlayImages(AppVariables_t* app)
{
    app->overlay.images_len = app->swap_chain.images_len;
    app->overlay.images = (VkImage*)realloc(app->overlay.images, app->overlay.images_len*sizeof(VkImage));
    app->overlay.image_memories_len = app->swap_chain.images_len;
    app->overlay.image_memories = (VkDeviceMemory*)realloc(app->overlay.image_memories, app->overlay.image_memories_len*sizeof(VkDeviceMemory));
    for (size_t i = 0; i < app->overlay.images_len; i++)
    {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = app->swap_chain.extend.width;
        imageInfo.extent.height = app->swap_chain.extend.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = app->swap_chain.image_format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                          VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        testVK(vkCreateImage(app->device, &imageInfo, NULL, &app->overlay.images[i]));

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(app->device, app->overlay.images[i], &memRequirements);
        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;

        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(app->physical_device, &memProperties);
        VkMemoryPropertyFlags properties;
        bool found = false;
        uint32_t j;
        for (j = 0; j < memProperties.memoryTypeCount; j++)
        {
            if ((memRequirements.memoryTypeBits & (1 << j)) &&
                (memProperties.memoryTypes[j].propertyFlags &
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ==
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            testVK(VK_ERROR_UNKNOWN);
        }
        allocInfo.memoryTypeIndex = j;
        testVK(vkAllocateMemory(app->device, &allocInfo, NULL, &app->overlay.image_memories[i]));
        vkBindImageMemory(app->device, app->overlay.images[i], app->overlay.image_memories[i], 0);
    }
}

void createOverlayImageViews(AppVariables_t* app)
{
    // overlayImageViews.resize(swapChainImages.size());
    app->overlay.image_views_len = app->swap_chain.images_len;
    app->overlay.image_views = (VkImageView*)realloc(app->overlay.image_views, app->overlay.image_views_len*sizeof(VkImageView));

    for (size_t i = 0; i < app->overlay.image_views_len; i++)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = app->overlay.images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = app->swap_chain.image_format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        testVK(vkCreateImageView(app->device, &createInfo, NULL,
                              &app->overlay.image_views[i]));
    }
}

void recreateSwapChain(AppVariables_t* app){
    int width = 0, height = 0;
    glfwGetFramebufferSize(app->window, &width, &height);
    while(width == 0 || height == 0){
        glfwGetFramebufferSize(app->window, &width, &height);
        glfwWaitEvents();
    } // TODO PAUSES EVERYTHING while window is minimised 

    vkDeviceWaitIdle(app->device);

    #ifdef INCLUDE_OVERLAY
    shutdown_overlay();
    free(app->overlay.images);
        free(app->overlay.image_views);
        free(app->overlay.image_memories);
    #endif

    cleanupSwapChain(app);

    createSwapChain(app);
    createImageViews(app);
    #ifdef USE_OVERLAY
        createOverlayImages(app);
        createOverlayImageViews(app);
    #endif
    // TODO createRenderPass();
    // TODO createTriangleGraphicsPipeline();
    #ifdef USE_OVERLAY
        createOverlayGraphicsPipeline(app);    
    #endif
    createFramebuffers(app);

    // createDescriptorPool();
    // TODO createDescriptorSets();
    // TODO createCommandBuffers();
    #ifdef INCLUDE_OVERLAY
        // resize_overlay(app->swap_chain.extend.width, app->swap_chain.extend.height);
        init_overlay(app);
    #endif
}

