/*

SDG                                                                                               JJ

                                     Orc Horde

							       Vulkan Renderer
*/

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <vector>

//#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION

#define TINYOBJLOADER_IMPLEMENTATION
#include "vendor/tiny_obj_loader.h"

#include "renderer.hh"

/* ================== Pure functions that don't return any class specific data ================== */

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
									  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
									  const VkAllocationCallbacks* pAllocator,
									  VkDebugUtilsMessengerEXT* pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
	vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

  if (func != nullptr) {
	return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
	return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
								   VkDebugUtilsMessengerEXT debugMessenger,
								   const VkAllocationCallbacks* pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
	vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

  if (func != nullptr) {
	func(instance, debugMessenger, pAllocator);
  }
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
  
  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
  
  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
  
  for (const auto &extension : availableExtensions) {
	requiredExtensions.erase(extension.extensionName);
  }
  
  return requiredExtensions.empty();
}

/* ======================================== Render State ======================================== */

std::vector<RenderOp> RenderState::getRenderOps(Renderer &renderer) {
  std::vector<RenderOp> renderOps;

  for (auto& [asset, renderable] : assets) {
	auto slice = renderer.writeInstanceBuffer(renderable.instances);
	RenderOp op {
	  .type = DrawMeshInstanced,
	  .vertexBuffer = renderable.vertexBuffer,
	  .indexBuffer = renderable.indexBuffer,
	  .numIndices = renderable.numIndices,
	  .instanceBuffer = slice.buffer,
	  .numInstances = static_cast<uint32_t>(renderable.instances.size()),
	  .instanceOffset = slice.offset,
	};
	renderOps.push_back(op);
  }
  assert(renderOps[0].type == DrawMeshInstanced);
  //std::printf("renderOps0 %d, %d vs renderOps1 %d, %d", op0.numInstances, op0.instanceOffset, op1.numInstances, op1.instanceOffset);
  return renderOps;
}

/* ============================ Renderer Class Vulkan Implementation ============================ */

void Renderer::initWindow() {
  std::printf("\n /* ------- INITIALIZING WINDOW ------- */ \n\n");
  glfwInit();
  
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  
  window = glfwCreateWindow(INIT_WIN_W, INIT_WIN_H, "Vulkan", nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Renderer::initGraphics() {
  std::printf("\n /* ------- INITIALIZING GRAPHICS CONTEXT ------- */ \n\n");
  createInstance();
  setupDebugMessenger();
  createSurface();
  initVulkan();
}

void Renderer::initVulkan() {
  selectPhysicalDevice();
  createLogicalDevice();
  createSwapChain();
  createImageViews();
  createRenderPass();
  createDescriptorSetLayout();
  createGraphicsPipeline();
  createCommandPool();
  createColorResources();
  createDepthResources();
  createFramebuffers();
  createTextureSampler();
  createUniformBuffers();
  createDescriptorPool();
  createDescriptorSets();
  createCommandBuffers();
  createSyncObjects();
  createInstanceBuffers();
}

void Renderer::getInput() {
  glfwPollEvents();
}

bool Renderer::shouldClose() {
  return glfwWindowShouldClose(window);
}

void Renderer::createInstance() {
  if (enableValidationLayers && !checkValidationLayerSupport()) {
	throw std::runtime_error("validation layers requested, but not available!");
  }
  
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Hello Triangle";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;
  
  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  
  auto extensions = getRequiredExtensions();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();
  
  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
  if (enableValidationLayers) {
	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();
	
	populateDebugMessengerCreateInfo(debugCreateInfo);
	createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
  } else {
	createInfo.enabledLayerCount = 0;
	
	createInfo.pNext = nullptr;
  }
  
  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
	throw std::runtime_error("failed to create instance!");
  }
}

void Renderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
  createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
	VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
	VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
	VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
	VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
}

void Renderer::setupDebugMessenger() {
  if (!enableValidationLayers) return;
  
  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);
  
  if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
	throw std::runtime_error("failed to set up debug messenger!");
  }
}

std::vector<const char*> Renderer::getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  
  std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
  if (enableValidationLayers) {
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  
  return extensions;
}

bool Renderer::checkValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  
  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
  
  for (const char* layerName : validationLayers) {
	bool layerFound = false;
	
	for (const auto& layerProperties : availableLayers) {
	  if (strcmp(layerName, layerProperties.layerName) == 0) {
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

VkBool32 Renderer::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
								 VkDebugUtilsMessageTypeFlagsEXT messageType,
								 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
								 void* pUserData) {
  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
  
  return VK_FALSE;
}

void Renderer::selectPhysicalDevice() {
  physicalDevice = VK_NULL_HANDLE;
  
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  
  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPU with Vulkan support!");
  }
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
  
  int maxDeviceSuitability = 0;
  int newDeviceSuitability;
  
  for (const auto& device : devices) {
    if ((newDeviceSuitability = rateDeviceSuitability(device)) >
        maxDeviceSuitability) {
	  maxDeviceSuitability = newDeviceSuitability;
	  physicalDevice = device;
	  msaaSamples = getMaxUsableSampleCount();
	  break;
    }
  }
  
  if (physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
}

void Renderer::createLogicalDevice() {
  float queuePriority = 1.0f;
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
  
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                            indices.presentFamily.value()};
  
  for (auto queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }
  
  VkPhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.samplerAnisotropy = VK_TRUE;
  deviceFeatures.sampleRateShading = VK_TRUE;
  
  VkDeviceCreateInfo createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pEnabledFeatures = &deviceFeatures;
  
  createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();
  
  // deprecated fields
  if (enableValidationLayers) {
    createInfo.enabledLayerCount =
	  static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }
  
  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }
  
  vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
  vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void Renderer::createSurface() {
  if (glfwCreateWindowSurface(instance, window, nullptr, &surface) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }
}

// Probably these don't need to be on the class
VkSurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto& availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
		availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
	  return availableFormat;
    }
  }
  return availableFormats[0];
}
VkPresentModeKHR Renderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
	if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) return availablePresentMode;
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Renderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
	return capabilities.currentExtent;
  } else {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	VkExtent2D actualExtent = {static_cast<uint32_t>(width),
							   static_cast<uint32_t>(height)};
	actualExtent.width =
	  std::clamp(actualExtent.width, capabilities.minImageExtent.width,
				 capabilities.maxImageExtent.width);
	actualExtent.height =
	  std::clamp(actualExtent.height, capabilities.minImageExtent.height,
				 capabilities.maxImageExtent.height);
	
	return actualExtent;
  }
}

void Renderer::createSwapChain() {
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
  
  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
  
  uint32_t imageCount =  swapChainSupport.capabilities.minImageCount + 1;
  
  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }
  
  VkSwapchainCreateInfoKHR createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
  uint32_t queueFamilyIndices[] = {
    indices.graphicsFamily.value(),
    indices.presentFamily.value()
  };
  
  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
  }
  
  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE; // necessary for handling resizes
  
  
  if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }
  
  vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
  swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
}

VkImageView Renderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
  VkImageViewCreateInfo createInfo {};
  createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  createInfo.image = image;
  createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  createInfo.format = format;
  
  /* unnecesary but kept for clarity */
  createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  
  createInfo.subresourceRange.aspectMask = aspectFlags;
  createInfo.subresourceRange.baseMipLevel = 0;
  createInfo.subresourceRange.levelCount = mipLevels;
  createInfo.subresourceRange.baseArrayLayer = 0;
  createInfo.subresourceRange.layerCount = 1;
  
  VkImageView imageView;
  if (vkCreateImageView(device, &createInfo, nullptr, &imageView) != VK_SUCCESS) {
	throw std::runtime_error("failed to create image view!");
  }
  
  return imageView;
}

void Renderer::createImageViews() {
  swapChainImageViews.resize(swapChainImages.size());
  
  for (size_t i = 0; i < swapChainImages.size(); i++) {
	swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
  }
}

void Renderer::createRenderPass() {
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = swapChainImageFormat;
  colorAttachment.samples = msaaSamples;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  
  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  
  VkAttachmentDescription colorAttachmentResolve{};
  colorAttachmentResolve.format = swapChainImageFormat;
  colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  
  VkAttachmentReference colorAttachmentResolveRef{};
  colorAttachmentResolveRef.attachment = 2;
  colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  
  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = findDepthFormat(); // why are we doing this again?
  depthAttachment.samples = msaaSamples;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  
  VkAttachmentDescription depthAttachmentResolve{};
  
  
  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  
  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;
  subpass.pResolveAttachments = &colorAttachmentResolveRef;
  
  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  
  std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;
  
  if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
	throw std::runtime_error("failed to create render pass!");
  }
}

void Renderer:: createDescriptorSetLayout() {
  VkDescriptorSetLayoutBinding uboLayoutBinding {
	.binding = 				0,
	.descriptorType = 		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	.descriptorCount = 		1,
	.stageFlags = 			VK_SHADER_STAGE_VERTEX_BIT,
	.pImmutableSamplers = 	nullptr,
  };
  
  VkDescriptorSetLayoutBinding samplerLayoutBinding {
	.binding = 				1,
	.descriptorType = 		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	.descriptorCount = 		MAX_TEXTURES_LOADED,
	.stageFlags = 			VK_SHADER_STAGE_FRAGMENT_BIT,
	.pImmutableSamplers = 	nullptr,
  };
  
  std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
  
  VkDescriptorSetLayoutCreateInfo layoutInfo {
	.sType = 				VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
	.bindingCount = 		static_cast<uint32_t>(bindings.size()),
	.pBindings = 			bindings.data(),
  };
  
  if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
	throw std::runtime_error("failed to create descriptor set layout!");
  }
  
}

void Renderer::createGraphicsPipeline() {

  std::vector<VkVertexInputBindingDescription> simpleBinding{
	Vertex::getBindingDescription(),
	//Instance::getBindingDescription()
  };

  std::vector<VkVertexInputAttributeDescription> simpleAttribute;
  for (const auto& description : Vertex::getAttributeDescriptions()) {
	simpleAttribute.push_back(description);
  }

  createGraphicsPipeline("shaders/triangle_vert.spv", "shaders/triangle_frag.spv",
						 simpleBinding, simpleAttribute,
						 pipelineLayout, graphicsPipeline);

  std::vector<VkVertexInputBindingDescription> instanceBinding {
	Vertex::getBindingDescription(),
	Instance::getBindingDescription()
  };

  std::vector<VkVertexInputAttributeDescription> instanceAttribute;
  for (const auto& description : Vertex::getAttributeDescriptions()) {
	instanceAttribute.push_back(description);
  }
  for (const auto& description : Instance::getAttributeDescriptions()) {
  	instanceAttribute.push_back(description);
  }

  assert(instanceAttribute.size() == (Vertex::getAttributeDescriptions().size() + Instance::getAttributeDescriptions().size()));

  createGraphicsPipeline("shaders/base_vert.spv", "shaders/base_frag.spv",
						 instanceBinding, instanceAttribute,
						 instancedPipelineLayout, instancedGraphicsPipeline);
}

void Renderer::createGraphicsPipeline(const std::string &vertShader,
									  const std::string &fragShader,
									  const std::vector<VkVertexInputBindingDescription> bindingDescriptions,
									  const std::vector<VkVertexInputAttributeDescription> attributeDescriptions,
									  
									  VkPipelineLayout &pipelineLayout,
									  VkPipeline &graphicsPipeline) {
  auto vertShaderByteCode = readBinAsset(vertShader);
  auto fragShaderByteCode = readBinAsset(fragShader);
  
  VkShaderModule vertShaderModule = createShaderModule(vertShaderByteCode);
  VkShaderModule fragShaderModule = createShaderModule(fragShaderByteCode);
  
  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";
  
  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";
  
  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
  
  std::vector<VkDynamicState> dynamicStates = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_SCISSOR
  };
  
  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicState.pDynamicStates = dynamicStates.data();
  

  
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{
	.sType = 							VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	.vertexBindingDescriptionCount = 	static_cast<uint32_t>(bindingDescriptions.size()),
	.pVertexBindingDescriptions = 		bindingDescriptions.data(),
	.vertexAttributeDescriptionCount = 	static_cast<uint32_t>(attributeDescriptions.size()),
	.pVertexAttributeDescriptions = 	attributeDescriptions.data(),
  };
  
  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;
  
  /* for static view port and scissor
	 
	 VkViewport viewport{};
	 viewport.x = 0.0f;
	 viewport.y = 0.0f;
	 viewport.width = (float) swapChainExtent.width;
	 viewport.height = (float) swapChainExtent.height;
	 viewport.minDepth = 0.0f;
	 viewport.maxDepth = 1.0f;
	 
	 VkRect2D scissor{};
	 scissor.offset = { 0, 0 };
	 scissor.extent = swapChainExtent;
	 
  */
  
  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  // viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  // viewportState.pScissors = &scissor;
  
  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f;
  rasterizer.depthBiasClamp = 0.0f;
  rasterizer.depthBiasSlopeFactor = 0.0f;;
  
  VkPipelineMultisampleStateCreateInfo multisampling {};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_TRUE;
  multisampling.rasterizationSamples = msaaSamples;
  multisampling.minSampleShading = 0.2f;
  multisampling.pSampleMask = nullptr;
  multisampling.alphaToCoverageEnable = VK_FALSE;
  multisampling.alphaToOneEnable = VK_FALSE;
  
  VkPipelineDepthStencilStateCreateInfo depthStencil{};
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = VK_TRUE;
  depthStencil.depthWriteEnable = VK_TRUE;
  depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.minDepthBounds = 0.0f;
  depthStencil.maxDepthBounds = 1.0f;
  depthStencil.stencilTestEnable = VK_FALSE;
  depthStencil.front = {};
  depthStencil.back = {};
  
  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
  
  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY; // used for bitwise
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;  // used for bitwise
  colorBlending.blendConstants[1] = 0.0f;  // used for bitwise
  colorBlending.blendConstants[2] = 0.0f;  // used for bitwise
  colorBlending.blendConstants[3] = 0.0f;  // used for bitwise
  
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges = nullptr;
  
  if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
	throw std::runtime_error("failed to create pipeline layout");
  }
  
  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = pipelineLayout;
  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.basePipelineIndex = -1;
  
  if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
	throw std::runtime_error("failed to create graphics pipeline!");
  }
  
  vkDestroyShaderModule(device, vertShaderModule, nullptr);
  vkDestroyShaderModule(device, fragShaderModule, nullptr);
}

VkShaderModule Renderer::createShaderModule(const std::vector<char> &byteCode) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = byteCode.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(byteCode.data());
  
  VkShaderModule shaderModule;
  if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
	throw std::runtime_error("failed to create shader module");
  }
  return shaderModule;
}

void Renderer::createFramebuffers() {
  swapChainFramebuffers.resize(swapChainImageViews.size());
  for (size_t i = 0; i < swapChainImageViews.size(); i++) {
	std::array<VkImageView, 3> attachments = {
	  colorImageView,
	  depthImageView,
	  swapChainImageViews[i]
	};
	
	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = swapChainExtent.width;
	framebufferInfo.height = swapChainExtent.height;
	framebufferInfo.layers = 1;
	
	if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
	  throw std::runtime_error("failed to create framebuffer!");
	}
  }
}

void Renderer::createCommandPool() {
  QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
  
  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
  
  if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
	throw std::runtime_error("failed to create command pool!");
  }
}

void Renderer::createColorResources() {
  VkFormat colorFormat = swapChainImageFormat;
  
  createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples,
			  colorFormat, VK_IMAGE_TILING_OPTIMAL,
			  VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			  colorImage, colorImageMemory);
  colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void Renderer::createDepthResources() {
  VkFormat depthFormat = findDepthFormat();
  createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples,
			  depthFormat, VK_IMAGE_TILING_OPTIMAL,
			  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			  depthImage, depthImageMemory);
  depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

VkFormat Renderer::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
	
	if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
	  return format;
	} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
	  return format;
	}
  }
  
  throw std::runtime_error("failed to find supported format!");
}

VkFormat Renderer::findDepthFormat() {
  return findSupportedFormat({ VK_FORMAT_D32_SFLOAT,
	  VK_FORMAT_D32_SFLOAT_S8_UINT,
	  VK_FORMAT_D24_UNORM_S8_UINT },
	VK_IMAGE_TILING_OPTIMAL,
	VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool Renderer::hasStencilComponent(VkFormat format) {
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}


void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, std::vector<RenderOp> renderOps) {
  VkCommandBufferBeginInfo beginInfo{
	.sType = 				VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	.flags = 				0,
	.pInheritanceInfo = 	nullptr,
  };
  
  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
	throw std::runtime_error("failed to begin recording command buffer!");
  }
  
  std::array<VkClearValue, 2> clearValues;
  clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
  clearValues[1].depthStencil = {1.0f, 0};
  
  
  VkRenderPassBeginInfo renderPassInfo {
	.sType = 			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
	.renderPass = 		renderPass,
	.framebuffer = 		swapChainFramebuffers[imageIndex],
	.renderArea 		{ .offset = {0,0},
				 		  .extent = swapChainExtent },
	.clearValueCount = 	static_cast<uint32_t>(clearValues.size()),
	.pClearValues = 	clearValues.data(),
  };
  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  
  // TODO(caleb): allow shader objects or pipelines specified from the op here
  // we could replace this with a dynamic call to any number of shader objects
  // see: https://www.khronos.org/blog/you-can-use-vulkan-without-pipelines-today
  
  // need to set these because we're using a dynamic viewport
  VkViewport viewport{
	.x = 			0.0f,
	.y = 			0.0f,
	.width = 		static_cast<float>(swapChainExtent.width),
	.height = 		static_cast<float>(swapChainExtent.height),
	.minDepth = 	0.0f,
	.maxDepth = 	1.0f,
  };
  
  VkRect2D scissor{
	.offset = 		{0, 0},
	.extent = 		swapChainExtent,
  };
  
  for (const auto& op : renderOps) {
	
	//Vkcmddraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
	switch (op.type) {
	case DrawMeshSimple: {
	  std::printf("\n\n\nDRAWING SIMPLE MESH\n\n\n");
	  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	  VkBuffer vertexBuffers[] = {op.vertexBuffer};
	  VkDeviceSize offsets[] = {0};
	  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	  
	  vkCmdBindIndexBuffer(commandBuffer, op.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	  
	  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
							  pipelineLayout, 0, 1,
							  &descriptorSets[currentFrame], 0, nullptr);
	  
	  vkCmdDrawIndexed(commandBuffer, op.numIndices, 1, 0, 0, 0);
	} break;
	case DrawMeshInstanced: {	//
	  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, instancedGraphicsPipeline);

	  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	  VkBuffer vertexBuffers[] = {op.vertexBuffer, op.instanceBuffer};
	  VkDeviceSize offsets[] = {0, op.instanceOffset};
	  vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers, offsets);
	  
	  vkCmdBindIndexBuffer(commandBuffer, op.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	  
	  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
							  pipelineLayout, 0, 1,
							  &descriptorSets[currentFrame], 0, nullptr);
	  
	  vkCmdDrawIndexed(commandBuffer, op.numIndices, op.numInstances, 0, 0, 0);
	} break;
	}
  }
  
  vkCmdEndRenderPass(commandBuffer);
  
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
	throw std::runtime_error("failed to record command buffer!");
  }
}

void Renderer::createCommandBuffers() {
  commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  
  VkCommandBufferAllocateInfo allocInfo{
	.sType = 				VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	.commandPool = 			commandPool,
	.level = 				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	.commandBufferCount = 	(uint32_t) commandBuffers.size(),
  };
  
  if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
	throw std::runtime_error("failed to allocate command buffers!");
  }
}

void Renderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
							VkMemoryPropertyFlags properties,
							VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
  VkBufferCreateInfo bufferInfo {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  
  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create buffer!");
  }
  
  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
  
  VkMemoryAllocateInfo allocInfo {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
  
  if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate buffer memory!");
  }
  vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void Renderer::createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
						   VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
						   VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
						   VkImage& image, VkDeviceMemory& imageMemory) {
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = static_cast<uint32_t>(width);
  imageInfo.extent.height = static_cast<uint32_t>(height);
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = mipLevels;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = numSamples;
  imageInfo.flags =  0;
  
  if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
	throw std::runtime_error("failed to create image!");
  }
  
  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device, image, &memRequirements);
  
  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
  
  if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
	throw std::runtime_error("failed to allocate image memory");
  }
  vkBindImageMemory(device, image, imageMemory, 0);
}

void Renderer::createTextureImage(stbi_uc *pixels, int texWidth, int texHeight, int texChannels,
								  VkImage &textureImage, VkDeviceMemory &textureImageMemory,
								  VkImageView &textureImageView) {
  VkDeviceSize imageSize = texWidth * texHeight * 4;
 
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  
  createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			   stagingBuffer, stagingBufferMemory);
  
  void *data;
  vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(device, stagingBufferMemory);
  
  
  auto mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
  
  createImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT,
			  VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			  VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			  VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			  VK_IMAGE_USAGE_SAMPLED_BIT,
			  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			  textureImage, textureImageMemory);

  transitionImageLayout(textureImage,
						VK_FORMAT_R8G8B8A8_SRGB,
						VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						mipLevels);
  copyBufferToImage(stagingBuffer, textureImage,
					static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
  
  // this is now done automatically due to generating mipmaps
  //transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
  //VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);
  generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
  
  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);

  textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
									 VK_IMAGE_ASPECT_COLOR_BIT,
									 mipLevels);

}

// This should be done somewhere before we ever get here.
void Renderer::generateMipmaps(VkImage image, VkFormat imageFormat,
							   int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
  VkFormatProperties formatProperties;
  vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);
  
  if (!(formatProperties.optimalTilingFeatures &
		VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
	throw std::runtime_error("texture image format does not support linear blitting!");
  }
  
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();
  
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = image;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;
  
  int32_t mipWidth = texWidth;
  int32_t mipHeight = texHeight;
  
  for (uint32_t i = 1; i < mipLevels; i++) {
	barrier.subresourceRange.baseMipLevel = i -1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	
	vkCmdPipelineBarrier(commandBuffer,
						 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
						 0, nullptr,
						 0, nullptr,
						 1, &barrier);
	VkImageBlit blit{};
	blit.srcOffsets[0] = { 0, 0, 0 };
	blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
	blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit.srcSubresource.mipLevel = i - 1;
	blit.srcSubresource.baseArrayLayer = 0;
	blit.srcSubresource.layerCount = 1;
	blit.dstOffsets[0] = { 0, 0, 0 };
	blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
	blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit.dstSubresource.mipLevel = i;
	blit.dstSubresource.baseArrayLayer = 0;
	blit.dstSubresource.layerCount = 1;
	
	vkCmdBlitImage(commandBuffer,
				   image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				   image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				   1, &blit,
				   VK_FILTER_LINEAR);
	
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	
	vkCmdPipelineBarrier(commandBuffer,
						 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
						 0, nullptr,
						 0, nullptr,
						 1, &barrier);
	
	if (mipWidth > 1) { mipWidth /= 2; } 
	if (mipHeight > 1) { mipHeight /= 2; }
  }
  
  barrier.subresourceRange.baseMipLevel = mipLevels -1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  
  vkCmdPipelineBarrier(commandBuffer,
					   VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
					   0, nullptr,
					   0, nullptr,
					   1, &barrier);
  
  endSingleTimeCommands(commandBuffer);
}

void Renderer::createTextureSampler() {
  
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(physicalDevice, &properties);
  
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
  
  if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
	throw std::runtime_error("failed to create texture sampler!");
  }
}

void Renderer::createVertexBuffer(std::vector<Vertex> vertices,
								  VkBuffer &vertexBuffer, VkDeviceMemory &vertexBufferMemory) {
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
  
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			   stagingBuffer, stagingBufferMemory);
  
  void* data;
  vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices.data(), (size_t) bufferSize);
  vkUnmapMemory(device, stagingBufferMemory);
  
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			   vertexBuffer, vertexBufferMemory);
  copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
  
  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Renderer::createIndexBuffer(std::vector<uint32_t> indices,
								 VkBuffer &indexBuffer, VkDeviceMemory &indexBufferMemory) {
  VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
  
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			   stagingBuffer, stagingBufferMemory);
  
  void* data;
  vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, indices.data(), (size_t) bufferSize);
  vkUnmapMemory(device, stagingBufferMemory);
  
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			   indexBuffer, indexBufferMemory);
  copyBuffer(stagingBuffer, indexBuffer, bufferSize);
  
  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
}

BufferSlice Renderer::writeInstanceBuffer(std::vector<Instance> instances) {
  VkDeviceSize bufferSize = sizeof(instances[0]) * instances.size();
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			   stagingBuffer, stagingBufferMemory);
  // TODO(caleb): This MAY not be how we wan this to work.
  // we MAY want to be able to update this data later, and so instead just maintain
  // a large sparse buffer of instances that we can then modifiy one piece at a time
  // when specific elements move
  void* data;
  vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, instances.data(), (size_t) bufferSize);
  vkUnmapMemory(device, stagingBufferMemory);
  
  // TODO(caleb): we may also want to consider adding some kind of synchronization here
  // so that the vertex buffer isn't read until the copy buffer is finished, but this may
  // be a complete non-issue
  VkBuffer instanceBuffer = instanceBufferPool[currentFrame].buffer;
  uint32_t &currentOffset = instanceBufferPool[currentFrame].offset;

  copyBuffer(stagingBuffer, instanceBuffer, bufferSize, currentOffset);

  BufferSlice slice {
	.offset = currentOffset,
	.buffer = instanceBuffer
  };
  
  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);

  std::printf("Writing buffer at offset %d, with size: %zd and first position %f, %f, %fi\n", currentOffset, instances.size(), instances[0].position.x, instances[0].position.y, instances[0].position.z);

  currentOffset += bufferSize; // TODO(caleb): Handle case where we can overflow this
  assert(currentOffset <= (MAX_GAME_OBJECTS * sizeof(Instance)));

  return slice;
}

void Renderer::createDescriptorPool() {
  VkDescriptorPoolSize uboDescriptorPoolSize{};
  uboDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboDescriptorPoolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
  
  VkDescriptorPoolSize textureDescriptorPoolSize{};
  textureDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  textureDescriptorPoolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
  
  std::array<VkDescriptorPoolSize, 2> poolSizes = {uboDescriptorPoolSize, textureDescriptorPoolSize};
  
  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
  
  if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
	throw std::runtime_error("failed to create descriptor pool!");
  }
}

void Renderer::createDescriptorSets() {
  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
  VkDescriptorSetAllocateInfo allocInfo{
	.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
	.descriptorPool = descriptorPool,
	.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
	.pSetLayouts = layouts.data(),
  };
  
  descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
  if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
	throw std::runtime_error("failed to allocate descriptor sets!");
  }
  
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
	VkDescriptorBufferInfo bufferInfo{
	  .buffer = uniformBuffers[i],
	  .offset = 0,
	  .range = sizeof(UniformBufferObject),
	};

	VkWriteDescriptorSet uboDescriptorWrite{
	  .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	  .dstSet = descriptorSets[i],
	  .dstBinding = 0,
	  .dstArrayElement = 0,
	  .descriptorCount = 1,
	  .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	  .pBufferInfo = &bufferInfo,
	};

	VkDescriptorImageInfo imageInfo[MAX_TEXTURES_LOADED] {};
	for (size_t i = 0; i < MAX_TEXTURES_LOADED; i++) {
	  imageInfo[i] = VkDescriptorImageInfo {
		.sampler = textureSampler,
		.imageView = VK_NULL_HANDLE,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	  };
	}
	
	VkWriteDescriptorSet textureDescriptorWrite{
	  .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	  .dstSet = descriptorSets[i],
	  .dstBinding = 1,
	  .dstArrayElement = 0,
	  .descriptorCount = MAX_TEXTURES_LOADED,
	  .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	  .pImageInfo = imageInfo,
	};
	
	std::array<VkWriteDescriptorSet, 2> descriptorWrites { uboDescriptorWrite,
														   textureDescriptorWrite, };
	
	vkUpdateDescriptorSets(device,
						   static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
						   0, nullptr);
  }
}

void Renderer::addTextureImageToDescriptorSet(VkImageView &imageView, uint32_t &offset) {
  assert(numTextures + 1 < MAX_TEXTURES_LOADED);
	
  VkDescriptorImageInfo imageInfo{
	.sampler = textureSampler,
	.imageView = imageView,
	.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
  
  
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
	// TODO(caleb): We may want to check a freelist before assigning an image view
	// to a texture image in the texture array
	VkWriteDescriptorSet textureDescriptorWrite{
	  .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	  .dstSet = descriptorSets[i],
	  .dstBinding = 1,
	  .dstArrayElement = numTextures,
	  .descriptorCount = 1,
	  .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	  .pImageInfo = &imageInfo,
	};

	vkUpdateDescriptorSets(device, 1, &textureDescriptorWrite, 0, nullptr);
  }

  offset = numTextures++;
}


void Renderer::createUniformBuffers() {
  VkDeviceSize bufferSize = sizeof(UniformBufferObject);
  
  uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
  uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
  
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
	createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				 uniformBuffers[i], uniformBuffersMemory[i]);
	
	vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
  }
}

void Renderer::createSyncObjects() {
  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  
  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  
  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
		vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
		vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
	  throw std::runtime_error("failed to create semaphores!");
	}
  }
}

void Renderer::createInstanceBuffers() {
  VkDeviceSize bufferSize = MAX_GAME_OBJECTS * sizeof(instance);
  for (auto& instanceAlloc : instanceBufferPool) {
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				 instanceAlloc.buffer, instanceAlloc.memory);
	instanceAlloc.offset = 0;
  }
}
  

VkCommandBuffer Renderer::beginSingleTimeCommands(){
  VkCommandBufferAllocateInfo allocInfo {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = 1;
  
  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
  
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  
  vkBeginCommandBuffer(commandBuffer, &beginInfo);
  
  return commandBuffer;
}

void Renderer::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
  vkEndCommandBuffer(commandBuffer);
  
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
  
  vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphicsQueue);
  
  vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void Renderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, uint32_t dstOffset) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();
  
  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0;
  copyRegion.dstOffset = dstOffset;
  copyRegion.size = size;
  
  vkCmdCopyBuffer(commandBuffer,srcBuffer, dstBuffer, 1, &copyRegion);
  
  endSingleTimeCommands(commandBuffer);
}

void Renderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();
  
  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  
  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};
  
  vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
  
  endSingleTimeCommands(commandBuffer);
}


void Renderer::updateUniformBuffer(uint32_t currentImage) {
  static auto startTime = std::chrono::high_resolution_clock::now();
  
  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
  
  UniformBufferObject ubo{};
  //  ubo.view = glm::lookAt(glm::vec3(2.0f + time, 2.0f + time, 2.0f + time),
  //						 glm::vec3(0.0f, 0.0f, 0.0f),
  //						 glm::vec3(0.0f, 0.0f, 1.0f));
  //  
  //  ubo.proj = glm::perspective(glm::radians(45.0f),
  //							  swapChainExtent.width / (float) swapChainExtent.height,
  //							  0.1f,
  //							  100.0f);

  // NOTE(caleb): this is the top down view
  ubo.view = glm::lookAt(glm::vec3(0.0f, 0.1f, 10.0f), // TODO(caleb): WRITE YOUR OWN MATH
  						 glm::vec3(0.0f, 0.0f, 0.0f),
  						 glm::vec3(0.0f, 0.0f, -1.0f));
    
  ubo.proj = glm::perspective(glm::radians(45.0f),
  							  swapChainExtent.width / (float) swapChainExtent.height,
							  0.1f,
							  100.0f);

  ubo.proj[1][1] *= -1;
  
  
  memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void Renderer::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();
  
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = mipLevels;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  
  VkPipelineStageFlags sourceStage, destinationStage;
  
  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	
	sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	
	sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
	throw std::invalid_argument("unimplemented layout transition!");
  }
  
  vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
  
  endSingleTimeCommands(commandBuffer);
}

void Renderer::drawFrame(std::vector<RenderOp> renderOps) {
  vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
  
  uint32_t imageIndex;
  
  // get swapchain image
  switch (vkAcquireNextImageKHR(device, swapChain,
								UINT64_MAX, imageAvailableSemaphores[currentFrame],
								VK_NULL_HANDLE,
								&imageIndex)) {
  case VK_SUCCESS:
    break;
  case VK_SUBOPTIMAL_KHR:
    break;
  case VK_ERROR_OUT_OF_DATE_KHR:
    recreateSwapChain();
    return;
  default:
    throw std::runtime_error("failed to acquire swap chain image!");
  }
  
  vkResetFences(device, 1, &inFlightFences[currentFrame]);
  
  vkResetCommandBuffer(commandBuffers[currentFrame], 0);
  recordCommandBuffer(commandBuffers[currentFrame], imageIndex, renderOps);
  updateUniformBuffer(currentFrame);
  
  VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
  VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
  
  VkSubmitInfo submitInfo {
	.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
	.waitSemaphoreCount = 1,
	.pWaitSemaphores = waitSemaphores,
	.pWaitDstStageMask = waitStages,
	.commandBufferCount = 1,
	.pCommandBuffers = &commandBuffers[currentFrame],
	.signalSemaphoreCount = 1,
	.pSignalSemaphores = signalSemaphores,
  };
  
  if (auto res = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);
	  res != VK_SUCCESS) {
	throw std::runtime_error("failed to submit draw command buffer!");
  }
  
  VkSwapchainKHR swapChains[] = {swapChain};
  
  VkPresentInfoKHR presentInfo {
	.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
	.waitSemaphoreCount = 1,
	.pWaitSemaphores = signalSemaphores,
	.swapchainCount = 1,
	.pSwapchains = swapChains,
	.pImageIndices = &imageIndex,
	.pResults = nullptr,
  };
  
  // present image
  switch (vkQueuePresentKHR(presentQueue, &presentInfo)) {
  case VK_SUCCESS:
    break;
  case VK_ERROR_OUT_OF_DATE_KHR:
    recreateSwapChain();
  case VK_SUBOPTIMAL_KHR:
    recreateSwapChain();
  default:
    throw std::runtime_error("failed to present swap chain image!");
  }
  
  if (framebufferResized) {
    framebufferResized = false;
    recreateSwapChain();
  }

  instanceBufferPool[currentFrame].offset = 0;
  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::recreateSwapChain() {
  int width, height = 0;
  glfwGetFramebufferSize(window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(window, &width, &height);
    glfwWaitEvents();
  }
  
  vkDeviceWaitIdle(device);
  
  cleanupSwapChain();
  
  createSwapChain();
  createImageViews();
  createColorResources();
  createDepthResources();
  createFramebuffers();
}

void Renderer::cleanupSwapChain() {
  
  vkDestroyImageView(device, colorImageView, nullptr);
  vkDestroyImage(device, colorImage, nullptr);
  vkFreeMemory(device, colorImageMemory, nullptr);
  
  vkDestroyImageView(device, depthImageView, nullptr);
  vkDestroyImage(device, depthImage, nullptr);
  vkFreeMemory(device, depthImageMemory, nullptr);
  
  for (auto framebuffer : swapChainFramebuffers) {
    vkDestroyFramebuffer(device, framebuffer, nullptr);
  }
  
  for (auto imageView : swapChainImageViews) {
    vkDestroyImageView(device, imageView, nullptr);
  }
  vkDestroySwapchainKHR(device, swapChain, nullptr);
}

// WARNING(caleb): Assets should be unloaded before we get here!!!
void Renderer::cleanup() {
  std::printf("\n /* ------- SHUTTING DOWN ------- */ \n\n");
  
  vkDeviceWaitIdle(device);
  
  cleanupSwapChain();
  
  vkDestroySampler(device, textureSampler, nullptr);
  
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
	vkDestroyBuffer(device, uniformBuffers[i], nullptr);
	vkFreeMemory(device, uniformBuffersMemory[i], nullptr);

	vkDestroyBuffer(device, instanceBufferPool[i].buffer, nullptr);
	vkFreeMemory(device, instanceBufferPool[i].memory, nullptr);
  }
  
  vkDestroyDescriptorPool(device, descriptorPool, nullptr);
  
  vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
  
  vkDestroyPipeline(device, graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

  vkDestroyPipeline(device, instancedGraphicsPipeline, nullptr);
  vkDestroyPipelineLayout(device, instancedPipelineLayout, nullptr);

  
  vkDestroyRenderPass(device, renderPass, nullptr);
  
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
	vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
	vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
	vkDestroyFence(device, inFlightFences[i], nullptr);
  }
  
  vkDestroyCommandPool(device, commandPool, nullptr);
  
  vkDestroyDevice(device, nullptr);
  
  if (enableValidationLayers) {
	DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
  }
  
  vkDestroySurfaceKHR(instance, surface, nullptr);
  vkDestroyInstance(instance, nullptr);
  
  glfwDestroyWindow(window);
  
  glfwTerminate();
}

/* --- Utility Functions use above --- */

SwapChainSupportDetails Renderer::querySwapChainSupport(VkPhysicalDevice device) {
  SwapChainSupportDetails details;
  
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
  
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
  
  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                         details.formats.data());
  }
  
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
  
  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
											  device, surface, &presentModeCount, details.presentModes.data());
  }
  
  return details;
}

QueueFamilyIndices Renderer::findQueueFamilies(VkPhysicalDevice device) {
  QueueFamilyIndices indices;
  
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
  
  for (int i = 0;  !indices.isComplete() &&i < size(queueFamilies); i++) {
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
    }
	
	VkBool32 presentSupport = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
    if (presentSupport) {
      indices.presentFamily = i;
    }
  }
  
  return indices;
}

int Renderer::rateDeviceSuitability(VkPhysicalDevice device) {
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);
  
  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
  
  int score = 0;
  
  // If the GPU does not have a graphics queue family, the GPU is unsuitable (return 0)
  if (QueueFamilyIndices indices = findQueueFamilies(device); !indices.isComplete()) return 0;
  
  // if the GPU does not have the required extensions, the GPU is unsuitable (return 0)
  if (!checkDeviceExtensionSupport(device)) return 0;
  
  // if the GPU does not have the required features, the GPU is unsuitable (return 0)
  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
  if (!(supportedFeatures.samplerAnisotropy &&
		supportedFeatures.sampleRateShading)) return 0;
  
  // if the GPU does not have swap chain support, the GPU is unsuitable (return 0)
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
  if ( swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty()) return 0;
  
  // Discrete GPUs have a significant performance advantage
  if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
  
  
  // Maximum possible size of textures affects graphics quality
  score += deviceProperties.limits.maxImageDimension2D;
  
  // Application can't function without geometry shaders
  if (!deviceFeatures.geometryShader) return 0;
  
  return score;
}

void Renderer::framebufferResizeCallback(GLFWwindow *window, int width,
										 int height) {
  auto renderer = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
  renderer->framebufferResized = true;
}

uint32_t Renderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
  
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
		(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  
  throw std::runtime_error("failed to find suitable gpu memory type!");
}

VkSampleCountFlagBits Renderer::getMaxUsableSampleCount() {
  VkPhysicalDeviceProperties physicalDeviceProperties;
  vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
  
  VkSampleCountFlags counts =
	physicalDeviceProperties.limits.framebufferColorSampleCounts &
	physicalDeviceProperties.limits.framebufferDepthSampleCounts;

  if (counts & VK_SAMPLE_COUNT_64_BIT)  return VK_SAMPLE_COUNT_64_BIT;
  if (counts & VK_SAMPLE_COUNT_32_BIT)  return VK_SAMPLE_COUNT_32_BIT;
  if (counts & VK_SAMPLE_COUNT_16_BIT)  return VK_SAMPLE_COUNT_16_BIT;
  if (counts & VK_SAMPLE_COUNT_8_BIT)   return VK_SAMPLE_COUNT_8_BIT;
  if (counts & VK_SAMPLE_COUNT_4_BIT)   return VK_SAMPLE_COUNT_4_BIT;
  if (counts & VK_SAMPLE_COUNT_2_BIT)   return VK_SAMPLE_COUNT_2_BIT;
  
  return VK_SAMPLE_COUNT_1_BIT;
}

void Renderer::destroyBuffer(VkBuffer buffer) {
  vkDestroyBuffer(device, buffer, nullptr);
}

void Renderer::freeMemory(VkDeviceMemory memory) {
  vkFreeMemory(device, memory, nullptr);
}

void Renderer::destroyImage(VkImage image) {
  vkDestroyImage(device, image, nullptr);
}

void Renderer::destroyImageView(VkImageView imageView) {
    vkDestroyImageView(device, imageView, nullptr);
}


void Renderer::setCursorMovementCallback(GLFWcursor *cursor,
										  CursorPositionCallback cursorPositionCallback){
  if (cursor != nullptr) {
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetCursor(window, cursor);
  } else {
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (glfwRawMouseMotionSupported()) { // TODO(caleb): handle case where raw mouse motion is not supported
	  glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}
  }
  glfwSetCursorPosCallback(window, reinterpret_cast<GLFWcursorposfun>(cursorPositionCallback));
}


void Renderer::setMouseButtonCallback(MouseButtonCallback mouseButtonCallback){
  glfwSetMouseButtonCallback(window, mouseButtonCallback);
}

GLFWcursor *Renderer::createCursor(unsigned char pixels[16*16*4]) {
  GLFWimage image;
  image.width = 16;
  image.height = 16;
  image.pixels = pixels;

  return glfwCreateCursor(&image, 0, 0);
}
