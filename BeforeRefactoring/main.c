#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #include <cglm/vec4.h>
// #include <cglm/mat4.h>  
#include <cglm/cglm.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "defines.h"
#include "app_structs.h"
#include "testing.h"
#include "debug_messenger.h" 
// #include "list.h"
#include "array.h"
#include "swapchain.h"
#include "graphics_pipeline.h"
#include "vertex2D.h"

#ifdef INCLUDE_OVERLAY
#include "overlay.h"
#endif


#define MAX_EXTENSIONS 128

static const uint32_t WIDTH  = 800;
static const uint32_t HEIGHT = 600;

// TODO might need to get nb extentions in len
#ifndef NDEBUG
static const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
static csarray_t validation_layers = {.str_arr = layers, .len = 1};
#else
static const char* layers[] = {};
static csarray_t validation_layers = {.str_arr = layers, .len = 0};
#endif
static const char* extensions[MAX_EXTENSIONS];
static const char* device_extension_names[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
static csarray_t device_extensions = {.str_arr = device_extension_names, .len = 1};




// static VkPhysicalDevice physical_device = VK_NULL_HANDLE;
// static VkDevice device; 

// static VkQueue graphics_queue;
// static VkQueue present_queue;

static void framebufferResizeCaalback(GLFWwindow* window, int width, int height){
    // this gets the app that we passed through the glfwSetWindowUserPointer(app->window, app); 
    AppVariables_t* app = (AppVariables_t*) glfwGetWindowUserPointer(window);
    app->frame_buffer_resized = true;
}

void initWindow(AppVariables_t* app){
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    app->window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", NULL, NULL);
    glfwSetWindowUserPointer(app->window, app); 
    glfwSetFramebufferSizeCallback(app->window, framebufferResizeCaalback);

}

const char** getRequiredExtensions(uint32_t* nb_extentions) {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // uint32_t extensionCount = 0;    
    // testVK(vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL));
    // printf("%d extensions supported\n", extensionCount);
    *nb_extentions = glfwExtensionCount + 1; 
    if (*nb_extentions > MAX_EXTENSIONS) {
        fprintf(stderr, "Too many extensions!\n");
        return NULL;
    }
    for(int i = 0; i < glfwExtensionCount; i++){
        extensions[i] = (char*) glfwExtensions[i];
    }
    extensions[glfwExtensionCount] = "VK_EXT_debug_utils";

    return extensions;
}

void createInstance(VkInstance* instance){
    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Hello Triangle";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 4, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 4, 0);
    app_info.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    // VkInstanceCreateInfo createInfo;
    // createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    // createInfo.pApplicationInfo = &app_info;

    uint32_t nb_max_extensions;
    getRequiredExtensions(&nb_max_extensions);
    create_info.enabledExtensionCount = nb_max_extensions;
    create_info.ppEnabledExtensionNames = extensions;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers) {
        create_info.enabledLayerCount = validation_layers.len;
        create_info.ppEnabledLayerNames = validation_layers.str_arr;

        populateDebugMessengerCreateInfo(&debugCreateInfo);
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        create_info.enabledLayerCount = 0;

        create_info.pNext = NULL;
    }

    testVK(vkCreateInstance(&create_info, NULL, instance));
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);

    VkExtensionProperties availableExtensions[extension_count];
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, availableExtensions);

    uint32_t extensions_checked = 0;
    for (uint32_t i = 0; i < device_extensions.len; i++) {
        for (uint32_t j = 0; j < extension_count; j++) {
             if(strcmp(availableExtensions[j].extensionName, device_extensions.str_arr[i]) == 0){
                extensions_checked++; 
                break;
             }
        }     
    }

    return (bool) (extensions_checked == device_extensions.len);
}

bool isDeviceSuitable(VkPhysicalDevice device, AppVariables_t* app) {
    QueueFamilyIndices_t indices = findQueueFamilies(device, app);

    bool extensions_supported = checkDeviceExtensionSupport(device);
    
    bool swapChainAdequate = false;
    if (extensions_supported) {
        SwapChainSupportDetails_t swapChainSupport = querySwapChainSupport(device, app);
        swapChainAdequate = (swapChainSupport.formats_len != 0) && (swapChainSupport.present_modes_len != 0);
        cleanupSwapChainSupport(&swapChainSupport);
    }


    return indices.hasGraphicsFamily && indices.hasPresentFamily && extensions_supported && swapChainAdequate;
}  

void pickPhysicalDevice(AppVariables_t* app){
    uint32_t deviceCount = 0;
    testVK(vkEnumeratePhysicalDevices(app->instance, &deviceCount, NULL));
    VkPhysicalDevice devices[deviceCount];
    testVK(vkEnumeratePhysicalDevices(app->instance, &deviceCount, devices));

    if (deviceCount == 0) {
        fprintf(stderr, "failed to find GPUs with Vulkan support!");
    }

    for (int i = 0; i < deviceCount; i++) {
        if (isDeviceSuitable(devices[i], app)) {
            app->physical_device = devices[i];
            VkPhysicalDeviceProperties device_properties;
            vkGetPhysicalDeviceProperties(app->physical_device, &device_properties);
            printf("Choosen device is: %s\n", device_properties.deviceName);
            break;
        }
    }

    if (app->physical_device == VK_NULL_HANDLE) {
        fprintf(stderr, "failed to find a suitable GPU!");
    }
}

void createLogicalDevice(AppVariables_t* app){
    QueueFamilyIndices_t indices = findQueueFamilies(app->physical_device, app);

    const uint32_t nb_queues = 2;
    VkDeviceQueueCreateInfo queue_create_infos[nb_queues];
    uint32_t unique_indices[nb_queues];
    uint32_t unique_count = 0;

    unique_indices[unique_count++] = indices.graphicsFamily;
    if (indices.graphicsFamily != indices.presentFamily){
        unique_indices[unique_count++] = indices.presentFamily;
    }

    float queue_priority = 1.0f; // influences the command buffer execution priority
    for(uint32_t i = 0; i < unique_count; i++){
        VkDeviceQueueCreateInfo queue_create_info = {0};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = unique_indices[i]; //indices.graphicsFamily;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos[i] = queue_create_info;
    }

    VkPhysicalDeviceFeatures device_features = {0};
    VkDeviceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = unique_count;
    create_info.pQueueCreateInfos = queue_create_infos;
    create_info.pEnabledFeatures  = &device_features;
    create_info.enabledExtensionCount = device_extensions.len;
    create_info.ppEnabledExtensionNames = device_extensions.str_arr;

    if (enableValidationLayers){
        create_info.enabledLayerCount = (uint32_t) validation_layers.len;
        create_info.ppEnabledLayerNames = validation_layers.str_arr;
    } else {
        create_info.enabledLayerCount = 0;
    }

    testVK(vkCreateDevice(app->physical_device, &create_info, NULL, &app->device));

    vkGetDeviceQueue(app->device, indices.graphicsFamily, 0, &app->graphics_queue);
    vkGetDeviceQueue(app->device, indices.presentFamily , 0, &app->present_queue);
}

void createSurface(AppVariables_t* app){
    testVK(glfwCreateWindowSurface(app->instance, app->window, NULL, &app->surface));
}

char* readFile(const char* filename, size_t* out_size){
    FILE* file = fopen(filename, "rb");

    if (!file) {
        fprintf(stderr, "failed to open file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file); 

    char* buffer = (char*)malloc(fileSize);
    if (!buffer) {
        fprintf(stderr, "failed to allocate memory for file: %s\n", filename);
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != fileSize) {
        fprintf(stderr, "failed to read file: %s\n", filename);
        free(buffer);
        fclose(file);
        return NULL;
    }

    fclose(file);
    *out_size = fileSize;
    return buffer;
}

VkShaderModule createShaderModule(AppVariables_t* app, Shader_t* code) {
    VkShaderModuleCreateInfo createInfo={0};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code->len;
    createInfo.pCode = (const uint32_t*)(code->code);

    VkShaderModule shaderModule;
    testVK(vkCreateShaderModule(app->device, &createInfo, NULL, &shaderModule));

    return shaderModule;
}

void createGraphicsPipeline(AppVariables_t* app) {
    Shader_t vertShaderCode , fragShaderCode ;
    vertShaderCode.code = readFile("shadervert.spv", &vertShaderCode.len);
    fragShaderCode.code = readFile("shaderfrag.spv", &fragShaderCode.len);

    VkShaderModule vertShaderModule = createShaderModule(app, &vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(app, &fragShaderCode);
    free(vertShaderCode.code);
    free(fragShaderCode.code);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {0};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {0};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    // vertexInputInfo.vertexBindingDescriptionCount = 0;
    // vertexInputInfo.vertexAttributeDescriptionCount = 0;
    VkVertexInputBindingDescription bindingDescription = vert2GetBindingDescription();
    uint32_t attribute_description_len = 2;
    VkVertexInputAttributeDescription attributeDescriptions[attribute_description_len];
    vert2GetAttributeDescriptions(attributeDescriptions, &attribute_description_len);
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)(attribute_description_len);
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState = {0};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount  = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth   = 1.0f;
    rasterizer.cullMode    = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace   = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {0};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable  = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = NULL; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending = {0};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp       = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    uint32_t dynamic_state_size = 2;
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {0};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = (uint32_t)(dynamic_state_size);
    dynamicState.pDynamicStates = dynamicStates;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    #ifdef USE_OVERLAY
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &app->descriptor_set_Layout;
    #endif

    testVK(vkCreatePipelineLayout(app->device, &pipelineLayoutInfo, NULL, &app->pipeline_layout));

    VkGraphicsPipelineCreateInfo pipelineInfo = {0};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    // pipelineInfo.pDepthStencilState = NULL; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = app->pipeline_layout;
    pipelineInfo.renderPass = app->render_pass;
    pipelineInfo.subpass = 0;
    // to create from existing pipeline
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    // pipelineInfo.basePipelineIndex = -1; // Optional

    testVK(vkCreateGraphicsPipelines(app->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &app->graphics_pipeline));

    vkDestroyShaderModule(app->device, fragShaderModule, NULL);
    vkDestroyShaderModule(app->device, vertShaderModule, NULL);
}


  void createDescriptorPool(AppVariables_t* app) {
    VkDescriptorPoolSize poolSize = {0};
    poolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    poolSize.descriptorCount =(uint32_t)(app->swap_chain.frame_buffers_len);;

    VkDescriptorPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = (uint32_t)(app->swap_chain.frame_buffers_len);
    testVK(vkCreateDescriptorPool(app->device, &poolInfo, NULL, &app->descriptor_pool));
  }

  void createDescriptorSetLayout(AppVariables_t* app) {
    VkDescriptorSetLayoutBinding overlayLayoutBinding = {0};
    overlayLayoutBinding.binding = 0;
    overlayLayoutBinding.descriptorCount = 1;
    overlayLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    overlayLayoutBinding.pImmutableSamplers = NULL;
    overlayLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {0};
    descriptorSetLayoutCreateInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = 1;
    descriptorSetLayoutCreateInfo.pBindings = &overlayLayoutBinding;

    testVK(vkCreateDescriptorSetLayout(app->device, &descriptorSetLayoutCreateInfo,
                                    NULL,
                                    &app->descriptor_set_Layout));
  }

  void createDescriptorSets(AppVariables_t* app) {
    uint32_t numFbs = app->swap_chain.frame_buffers_len;
    app->descriptor_sets = realloc(app->descriptor_sets, numFbs*sizeof(VkDescriptorSet));

    VkDescriptorSetLayout layouts[numFbs];
    for (uint32_t i = 0; i < numFbs; i++) {
        layouts[i] = app->descriptor_set_Layout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = app->descriptor_pool;
    allocInfo.descriptorSetCount = numFbs;
    allocInfo.pSetLayouts = layouts;
    testVK(vkAllocateDescriptorSets(app->device, &allocInfo, app->descriptor_sets));

    for (size_t i = 0; i < numFbs; i++) {
      VkDescriptorImageInfo descriptorImageInfo = {0};
      descriptorImageInfo.imageLayout =
          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      descriptorImageInfo.imageView = app->overlay.image_views[i];

      VkWriteDescriptorSet descriptorWrite = {0};
      descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrite.dstSet = app->descriptor_sets[i];
      descriptorWrite.dstBinding = 0;
      descriptorWrite.dstArrayElement = 0;
      descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
      descriptorWrite.descriptorCount = 1;
      descriptorWrite.pImageInfo = &descriptorImageInfo;

      vkUpdateDescriptorSets(app->device, 1, &descriptorWrite, 0, NULL);
    }
  }


void createRenderPass(AppVariables_t* app){
    VkAttachmentDescription colorAttachment = {0};
    colorAttachment.format = app->swap_chain.image_format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // 1 sampling 
    // VK_ATTACHMENT_LOAD_OP_LOAD: Preserve the existing contents of the attachment
    // VK_ATTACHMENT_LOAD_OP_CLEAR: Clear the values to a constant at the start
    // VK_ATTACHMENT_LOAD_OP_DONT_CARE: Existing contents are undefined; we don't care about them
    colorAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // VK_ATTACHMENT_STORE_OP_STORE: Rendered contents will be stored in memory and can be read later
    // VK_ATTACHMENT_STORE_OP_DONT_CARE: Contents of the framebuffer will be undefined after the rendering operation
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Images used as color attachment
    // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Images to be presented in the swap chain
    // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: Images to be used as destination for a memory copy operation
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {0};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // pInputAttachments: Attachments that are read from a shader
    // pResolveAttachments: Attachments used for multisampling color attachments
    // pDepthStencilAttachment: Attachment for depth and stencil data
    // pPreserveAttachments: Attachments that are not used by this subpass, but for which the data must be preserved
    VkSubpassDescription subpass= {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    testVK(vkCreateRenderPass(app->device, &renderPassInfo, NULL, &app->render_pass));
}

void createCommandPool(AppVariables_t* app){
    QueueFamilyIndices_t queue_family_indices = findQueueFamilies(app->physical_device, app);

    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
    // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // we are recording a command buffer every time
    poolInfo.queueFamilyIndex = queue_family_indices.graphicsFamily;

    app->graphics_queue_family_index = queue_family_indices.graphicsFamily;

    testVK(vkCreateCommandPool(app->device, &poolInfo, NULL, &app->command_pool));
}

void createCommandBuffers(AppVariables_t* app){
    // app->command_buffer_len = MAX_FRAMES_IN_FLIGHT;
    // app->command_buffers = malloc(app->command_buffer_len*sizeof(VkCommandBuffer));
    // collectGarbage((void *)&app->command_buffers, app);

    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = app->command_pool;
    // VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other command buffers.
    // VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from primary command buffers.
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) MAX_FRAMES_IN_FLIGHT;//app->command_buffer_len;

    testVK(vkAllocateCommandBuffers(app->device, &allocInfo, app->command_buffers));
}

void recordCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index, AppVariables_t* app){
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it once.
    // VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely within a single render pass.
    // VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be resubmitted while it is also already pending execution.
    // beginInfo.flags = 0; // Optional
    // beginInfo.pInheritanceInfo = NULL; // Optional

    testVK(vkBeginCommandBuffer(command_buffer, &beginInfo));

    VkRenderPassBeginInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = app->render_pass;
    renderPassInfo.framebuffer = app->swap_chain.frame_buffers[image_index];

    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = app->swap_chain.extend;
    //                         render area |  clear values for VK_ATTACHMENT_LOAD_OP_CLEAR
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    // VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
    // VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass commands will be executed from secondary command buffers.
    vkCmdBeginRenderPass(command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, app->graphics_pipeline);

    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width  = (float)(app->swap_chain.extend.width);
    viewport.height = (float)(app->swap_chain.extend.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor= {0};
    scissor.offset.x = 0;
    scissor.offset.y = 0;    
    scissor.extent = app->swap_chain.extend;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = {app->vertex_buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(command_buffer, app->index_buffer, 0, VK_INDEX_TYPE_UINT32);

    // The actual vkCmdDraw function is a bit anticlimactic, but it's so simple because of all the information we specified in advance. It has the following parameters, aside from the command buffer:
    // vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
    // instanceCount: Used for instanced rendering, use 1 if you're not doing that.
    // firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
    // firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
    // vkCmdDraw(command_buffer, 3, 1, 0, 0);
    // vkCmdDraw(command_buffer, (uint32_t)(app->vertices->len), 1, 0, 0);
    vkCmdDrawIndexed(command_buffer, (uint32_t)(app->indices->len), 1, 0, 0, 0);

    vkCmdEndRenderPass(command_buffer);
    testVK(vkEndCommandBuffer(command_buffer));

}

void createSyncObjects(AppVariables_t* app){
    // app->image_available_semaphores_len = MAX_FRAMES_IN_FLIGHT;
    // app->image_available_semaphores     = malloc(app->image_available_semaphores_len*sizeof(VkSemaphore));
    // app->render_finished_semaphores_len = MAX_FRAMES_IN_FLIGHT;
    // app->render_finished_semaphores     = malloc(app->render_finished_semaphores_len*sizeof(VkSemaphore));
    // collectGarbage((void *)&app->image_available_semaphores, app);
    // collectGarbage((void *)&app->render_finished_semaphores, app);

    VkSemaphoreCreateInfo semaphore_info = {0};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        testVK(vkCreateSemaphore(app->device, &semaphore_info, NULL, &app->image_available_semaphores[i])); 
        testVK(vkCreateSemaphore(app->device, &semaphore_info, NULL, &app->render_finished_semaphores[i])); 
        testVK(vkCreateFence(app->device, &fenceInfo, NULL, &app->in_flight_fences[i])); 
    }
}

uint32_t findMemoryType(AppVariables_t* app, uint32_t type_filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(app->physical_device, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    testVK(VK_ERROR_UNKNOWN); // TODO add my errors
    return 0; // will abort before comming here 
}

void createBuffer(AppVariables_t* app, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory) {
    VkBufferCreateInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    testVK(vkCreateBuffer(app->device, &bufferInfo, NULL, buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(app->device, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(app, memRequirements.memoryTypeBits, properties);

    testVK(vkAllocateMemory(app->device, &allocInfo, NULL, bufferMemory)); // TODO need to change this to a custom allocator 
    /*TODO It should be noted that in a real world application, you're not supposed to actually call vkAllocateMemory for every individual buffer. The maximum number of simultaneous memory allocations is limited by the maxMemoryAllocationCount physical device limit, which may be as low as 4096 even on high end hardware like an NVIDIA GTX 1080. The right way to allocate memory for a large number of objects at the same time is to create a custom allocator that splits up a single allocation among many different objects by using the offset parameters that we've seen in many functions.
    You can either implement such an allocator yourself, or use the VulkanMemoryAllocator library provided by the GPUOpen initiative. However, for this tutorial it's okay to use a separate allocation for every resource, because we won't come close to hitting any of these limits for now.*/

    vkBindBufferMemory(app->device, *buffer, *bufferMemory, 0);
}

void copyBuffer(AppVariables_t* app, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size){
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = app->command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(app->device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {0};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, src_buffer, dst_buffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(app->graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(app->graphics_queue);
    vkFreeCommandBuffers(app->device, app->command_pool, 1, &commandBuffer);

}

void createVertexBuffer(AppVariables_t* app){
    VkDeviceSize buffer_size = sizeof(app->vertices->vert[0]) * app->vertices->len;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    createBuffer(app, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);

    
    void* data;
    vkMapMemory(app->device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, app->vertices->vert, (size_t) buffer_size);
    vkUnmapMemory(app->device, staging_buffer_memory);
    
    // VK_BUFFER_USAGE_TRANSFER_SRC_BIT: Buffer can be used as source in a memory transfer operation.
    // VK_BUFFER_USAGE_TRANSFER_DST_BIT: Buffer can be used as destination in a memory transfer operation.
    createBuffer(app, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &app->vertex_buffer, &app->vertex_buffer_memory);
    
    copyBuffer(app, staging_buffer, app->vertex_buffer, buffer_size);

    vkDestroyBuffer(app->device, staging_buffer, NULL);
    vkFreeMemory(app->device, staging_buffer_memory, NULL);
}

void createPersistentVertexBuffer(AppVariables_t* app) {
    VkDeviceSize buffer_size = sizeof(Vertex2D_t) * app->vertices->len;

    // We use HOST_VISIBLE and HOST_COHERENT for direct CPU access
    createBuffer(app, buffer_size, 
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                 &app->vertex_buffer, 
                 &app->vertex_buffer_memory);

    // Map it ONCE and keep the pointer for the lifetime of the app
    vkMapMemory(app->device, app->vertex_buffer_memory, 0, buffer_size, 0, &app->vertex_persitent_buffer_mapped_ptr);
    
    // Initial data upload
    memcpy(app->vertex_persitent_buffer_mapped_ptr, app->vertices->vert, (size_t)buffer_size);
}

// same as the createVertexBufferBut only we dont recreate the vertex buffer 
void updateVertexBuffer(AppVariables_t* app){
    VkDeviceSize buffer_size = sizeof(app->vertices->vert[0]) * app->vertices->len;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    createBuffer(app, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);

    void* data;
    vkMapMemory(app->device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, app->vertices->vert, (size_t) buffer_size);
    vkUnmapMemory(app->device, staging_buffer_memory);
    
    copyBuffer(app, staging_buffer, app->vertex_buffer, buffer_size);

    vkDestroyBuffer(app->device, staging_buffer, NULL);
    vkFreeMemory(app->device, staging_buffer_memory, NULL);
}

void createIndexBuffer(AppVariables_t* app) {
    VkDeviceSize buffer_size = sizeof(app->indices->vals[0]) * app->indices->len;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    createBuffer(app, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);

    void* data;
    vkMapMemory(app->device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, app->indices->vals, (size_t) buffer_size);
    vkUnmapMemory(app->device, staging_buffer_memory);

    createBuffer(app, buffer_size,VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &app->index_buffer, &app->index_buffer_memory);

    copyBuffer(app, staging_buffer, app->index_buffer, buffer_size);

    vkDestroyBuffer(app->device, staging_buffer, NULL);
    vkFreeMemory(app->device, staging_buffer_memory, NULL);
}

void initVulkan(AppVariables_t* app){
    app->current_frame = 0;
    app->frame_buffer_resized = false; 
    createInstance(&app->instance);
    setupDebugMessenger(app->instance, &app->debug_messenger);   
    createSurface(app);
    pickPhysicalDevice(app);
    createLogicalDevice(app);
    createSwapChain(app);
    // createSwapChainImageViews(app);
    #ifdef USE_OVERLAY
        createOverlayImages(app);
        createOverlayImageViews(app);
    #endif
    createImageViews(app);
    createRenderPass(app);

    createFramebuffers(app); 
     #ifdef USE_OVERLAY
        createDescriptorSetLayout(app);
        createDescriptorPool(app);
        createDescriptorSets(app);
    #endif
    createGraphicsPipeline(app);

    #ifdef USE_OVERLAY  
        app->overlay.graphics_pipeline = app->graphics_pipeline;
        // createOverlayGraphicsPipeline(app);
    #endif
    

    createCommandPool(app);
    createVertexBuffer(app);
    createIndexBuffer(app);
    // createPersistentVertexBuffer(app);

    createCommandBuffers(app);
    createSyncObjects(app);

        
    #ifdef INCLUDE_OVERLAY
        init_overlay(app);
    #endif
}   

bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties availableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for (uint32_t i = 0; i < validation_layers.len; i++) {
        const char* layerName = validation_layers.str_arr[i];
        bool layerFound = false;

        for (uint32_t j = 0; j < layerCount; j++) {
            if (strcmp(layerName, availableLayers[j].layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

void drawFrame(AppVariables_t* app){
    // The vkWaitForFences function takes an array of fences and waits on the host for either any or all of the fences to be signaled before returning. The VK_TRUE we pass here indicates that we want to wait for all fences, but in the case of a single one it doesn't matter. This function also has a timeout parameter that we set to the maximum value of a 64 bit unsigned integer, UINT64_MAX, which effectively disables the timeout.
    vkWaitForFences(app->device, 1, &app->in_flight_fences[app->current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(app->device, app->swap_chain.swap_chain, UINT64_MAX, app->image_available_semaphores[app->current_frame], VK_NULL_HANDLE, &image_index);

    if(result == VK_ERROR_OUT_OF_DATE_KHR){
        recreateSwapChain(app);
        return;
    } else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
        testVK(result);
    }

    #ifdef INCLUDE_OVERLAY
        float old_bg_color[4] = {app->overlay.settings.bg_color[0], app->overlay.settings.bg_color[1],
                                 app->overlay.settings.bg_color[2], app->overlay.settings.bg_color[3]};
        VkSemaphore overlayFinished = submit_overlay(app, &app->overlay.settings, app->graphics_queue, image_index, NULL);
    #endif

    vkResetFences(app->device, 1, &app->in_flight_fences[app->current_frame]);
    vkResetCommandBuffer(app->command_buffers[app->current_frame], 0);
    recordCommandBuffer(app->command_buffers[app->current_frame], image_index, app);

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // TODO the cuda sync will be added here
    #ifdef INCLUDE_OVERLAY
        VkSemaphore waitSemaphores[] = {app->image_available_semaphores[app->current_frame], overlayFinished};
        VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 2;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
    #else
        VkSemaphore waitSemaphores[] = {app->image_available_semaphores[app->current_frame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
    #endif
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &app->command_buffers[app->current_frame];

    VkSemaphore signalSemaphores[] = {app->render_finished_semaphores[app->current_frame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkQueueSubmit(app->graphics_queue, 1, &submitInfo, app->in_flight_fences[app->current_frame]);

    #ifdef INCLUDE_OVERLAY
        if (old_bg_color[0] != app->overlay.settings.bg_color[0] ||
            old_bg_color[1] != app->overlay.settings.bg_color[1] ||
            old_bg_color[2] != app->overlay.settings.bg_color[2] ||
            old_bg_color[3] != app->overlay.settings.bg_color[3]) {
            // update clear color
            createCommandBuffers(app);
        }
    #endif


    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {app->swap_chain.swap_chain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &image_index;
    presentInfo.pResults = NULL;

    result = vkQueuePresentKHR(app->present_queue, &presentInfo);

    if(result == VK_ERROR_OUT_OF_DATE_KHR || VK_SUBOPTIMAL_KHR || 
       app->frame_buffer_resized){
        app->frame_buffer_resized = false;
        recreateSwapChain(app);
    } else if(result != VK_SUCCESS){
        testVK(result);
    }

    app->current_frame = (app->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

    #ifdef INCLUDE_OVERLAY
        vkQueueWaitIdle(app->present_queue);
    #endif
}

double lastTime = 0;
int frame_count = 0;
char title[16];

void updateFPS(AppVariables_t* app) {
    // Get current time in seconds
    
    double currentTime = (double)glfwGetTime();
    frame_count++;

    // If one second has passed
    if (currentTime - lastTime >= 10) {
        printf("\rFPS: %d", frame_count);
        fflush(stdout); 
        // sprintf(title, "%d", frame_count);
        glfwSetWindowTitle(app->window, title);
        frame_count = 0;
        lastTime = currentTime;
    }
}

void mainLoop(AppVariables_t* app){
    while(!glfwWindowShouldClose(app->window)) {
        glfwPollEvents();
        // nk_glfw3_new_frame();
        // // Define GUI
        // if (nk_begin(app->UI.nk_ctx, "My Window", nk_rect(50, 50, 200, 200),
        //     NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE)) {
        //     nk_layout_row_dynamic(app->UI.nk_ctx, 30, 1);
        //     if (nk_button_label(app->UI.nk_ctx, "Exit")) {
        //         glfwSetWindowShouldClose(app->window, 1);
        //     }
        // }
        // nk_end(app->UI.nk_ctx);

        for(size_t i = 0; i < app->vertices->len; i++){
            app->vertices->vert[i].pos[0] += 0.001;
        }
        
        // vkDestroyBuffer(app->device, app->vertex_buffer, NULL);
        // vkFreeMemory(app->device, app->vertex_buffer_memory, NULL);
        updateVertexBuffer(app);
        // memcpy(app->vertex_persitent_buffer_mapped_ptr, app->vertices->vert, 
        //        sizeof(Vertex2D_t) * app->vertices->len);
        
        // nk_glfw3_render(0, app->graphics_queue, app->current_frame, app->image_available_semaphores); // Draws the UI on top

        drawFrame(app);
        updateFPS(app);
    }

    vkDeviceWaitIdle(app->device);
} 

void cleanup(AppVariables_t* app){
    #ifdef INCLUDE_OVERLAY
        shutdown_overlay();
    #endif
    cleanupSwapChain(app);

    vkDestroyBuffer(app->device, app->index_buffer, NULL);
    vkFreeMemory(app->device, app->index_buffer_memory, NULL);
    
    // if (app->vertex_persitent_buffer_mapped_ptr != NULL) {
    //     vkUnmapMemory(app->device, app->vertex_buffer_memory);
    //     app->vertex_persitent_buffer_mapped_ptr = NULL; 
    // }    
    // vkDestroyBuffer(app->device, app->vertex_persitent_buffer_mapped_ptr, NULL);
    
    vkDestroyBuffer(app->device, app->vertex_buffer, NULL);
    vkFreeMemory(app->device, app->vertex_buffer_memory, NULL);
    

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        vkDestroySemaphore(app->device, app->image_available_semaphores[i], NULL);
        vkDestroySemaphore(app->device, app->render_finished_semaphores[i], NULL);
        vkDestroyFence(app->device, app->in_flight_fences[i], NULL);
    }

    vkDestroyCommandPool(app->device, app->command_pool, NULL);

    
    vkDestroyPipeline(app->device, app->graphics_pipeline, NULL);
    vkDestroyPipelineLayout(app->device, app->pipeline_layout, NULL);
    vkDestroyRenderPass(app->device, app->render_pass, NULL);
    
    // for(size_t i = 0; i < app->swap_chain.frame_buffers_len; i++){
    //     vkDestroyFramebuffer(app->device, app->swap_chain.frame_buffers[i], NULL);
    // }
    // for (size_t i = 0; i < app->swap_chain.image_views_len; i++) {
    //     vkDestroyImageView(app->device, app->swap_chain.image_views[i], NULL);
    // }
    // vkDestroySwapchainKHR(app->device, app->swap_chain.swap_chain, NULL);
    
    vkDestroyDevice(app->device, NULL);
    if (enableValidationLayers) {
            destroyDebugUtilsMessengerEXT(app->instance, app->debug_messenger, NULL);
    }

    vkDestroySurfaceKHR(app->instance, app->surface, NULL);
    vkDestroyInstance(app->instance, NULL);

    freeSwapchainVars(&app->swap_chain);

    glfwDestroyWindow(app->window);
    glfwTerminate();
}

uint32_t vertices_len = 4;
Vertex2D_t vertices[] = {
    // {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    // {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    // {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    // {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
    // {{0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
    // {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    // {{0.0f, -0.4f}, {1.0f, 1.0f/2, 1.0f}},
    // {{0.4f,  0.4f}, {0.0f, 1.0f/2, 0.0f}},
    // {{-0.4f, 0.4f}, {0.0f, 0.0f, 0.5f}}
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

uint32_t indice_vals[] = {0, 1, 2, 2, 3, 0};
ArrUint32_t indices = { .len = 6, .vals = indice_vals};


void initApp(AppVariables_t* app){
    if (app == NULL) return;
    app->vertices = malloc(sizeof(*app->vertices));
    app->vertices->vert = vertices;
    app->vertices->len  = vertices_len;
    app->indices = malloc(sizeof(*app->indices));
    app->indices->vals = indice_vals;
    app->indices->len = indices.len;

    // printf("%d %d %d %d %d\n", app->indices->vals[0], app->indices->vals[1], app->indices->vals[2], app->indices->vals[3], app->indices->len);
}

void freeApp(AppVariables_t* app){
    free(app->vertices);
    free(app->indices);
    // free(app);
}

int mainVulkan(){
    // testCGLM();
    
    AppVariables_t app_vars = {0};
    initApp(&app_vars);
    initWindow(&app_vars);
    app_vars.physical_device = VK_NULL_HANDLE;
    printf("Initialising vulkan\n");
    initVulkan(&app_vars);
    printf("Entering main loop\n");
    mainLoop(&app_vars); 
    printf("Cleanup started\n");
    // #ifdef INCLUDE_OVERLAY
    //     shutdown_overlay();
    // #endif
    cleanup(&app_vars);//window, instance, debug_messenger);
    freeApp(&app_vars);
    return 0;
}

int main(){
    // mainGUI();
    mainVulkan();
    return 0;
}