/*

SDG                                                                          JJ

                                     Orc Horde

									 Renderer
*/

#pragma once

#include <array>
#include <fstream>
#include <optional>
#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

const uint32_t INIT_WIN_W = 800;
const uint32_t INIT_WIN_H = 600;

//const std::string MODEL_PATH = "../models/maneki_neko/source/neko.obj";
//const std::string TEXTURE_PATH = "../models/maneki_neko/textures/neko.png";

const std::string TEXTURE_PATH = "../models/viking_room/viking_room.png";

const int MAX_FRAMES_IN_FLIGHT = 2;

class Asset; // defined in asset.hh

enum RenderOpType {
  DrawMeshSimple,
  DrawMeshInstanced,
};

struct RenderOp {
  RenderOpType type;
  VkBuffer vertexBuffer;
  VkBuffer indexBuffer;
  VkBuffer instanceBuffer;
};

struct Vertex {
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec2 texCoord;
  
  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
  }
  
  static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions {};
	// positions
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, pos);
	
	// colors
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);
	
	// textures
	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
	
	return attributeDescriptions;
  }
  
  
  bool operator==(const Vertex& other) const {
    return pos == other.pos &&
	  color == other.color &&
	  texCoord == other.texCoord;
  }
};

namespace std {
  template<> struct hash<Vertex> {
	size_t operator()(Vertex const& vertex) const {
	  return ((hash<glm::vec3>()(vertex.pos) ^
			   (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
		(hash<glm::vec2>()(vertex.texCoord) << 1);
	}
  };
}

typedef uint32_t Index;

struct Instance {
  glm::vec3 position;
  glm::vec3 rotation;
  float			scale;
  uint32_t		textureIndex;
  
  static VkVertexInputBindingDescription getBindingDescription() {
	return VkVertexInputBindingDescription {
	  .binding = 	1,
	  .stride = 	sizeof(Instance),
	  .inputRate = 	VK_VERTEX_INPUT_RATE_INSTANCE,
	};
  }
  
  static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
	std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions {
  	  VkVertexInputAttributeDescription {  // position
  	    .location = 3,
		.binding = 1,
  	    .format = VK_FORMAT_R32G32B32_SFLOAT,
  	    .offset = offsetof(Instance, position) },
	  
  	  VkVertexInputAttributeDescription {  // rotation
  	    .location = 4,
		.binding = 1,
  	    .format = VK_FORMAT_R32G32B32_SFLOAT,
  	    .offset = offsetof(Instance, rotation) },
	  
  	  VkVertexInputAttributeDescription {  // scale
  	    .location = 5,
		.binding = 1,
  	    .format = VK_FORMAT_R32_SFLOAT,
  	    .offset = offsetof(Instance, scale) },
	  
  	  VkVertexInputAttributeDescription{  // textureIndex
  	    .location = 6,
		.binding = 1,
  	    .format = VK_FORMAT_R32_SINT,
  	    .offset = offsetof(Instance, textureIndex) },
  	};
	return attributeDescriptions;
  }
};

struct UniformBufferObject {
  glm::mat4 view;
  glm::mat4 proj;
};

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;
  
  bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

const std::vector<const char*> deviceExtensions = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

class Renderer {
public:
  /* lifetime procedures */
  void initGraphics();
  void initWindow();
  bool shouldClose();
  void getInput();
  void cleanup();
  
  /* game procedures */
  void loadModel(std::string modelPath);
  void createVertexBuffer(std::vector<Vertex> vertices,
						  VkBuffer &vertexBuffer, VkDeviceMemory &vertexBufferMemory);
  void createIndexBuffer(std::vector<Index> indices,
						 VkBuffer &indexBuffer, VkDeviceMemory &indexBufferMemory);
  void createInstanceBuffer(std::vector<Instance> instances,
							VkBuffer &instanceBuffer, VkDeviceMemory &instanceBufferMemory);
  void drawFrame(std::vector<RenderOp> renderOps);
  void destroyBuffer(VkBuffer buffer);
  void freeMemory(VkDeviceMemory memory);
  
private:
  /* props */
  GLFWwindow *window;
  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkPhysicalDevice physicalDevice;
  VkDevice device;
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
				VkDebugUtilsMessageTypeFlagsEXT messageType,
				const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
				void *pUserData);
  VkQueue graphicsQueue;
  VkQueue presentQueue;
  VkSurfaceKHR surface;
  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages; // resized only on swapchain creation
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  std::vector<VkImageView>
  swapChainImageViews; // resized only on swapchain createinog
  VkRenderPass renderPass;
  VkDescriptorSetLayout descriptorSetLayout;
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;
  std::vector<VkFramebuffer> swapChainFramebuffers;
  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;
  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  uint32_t currentFrame = 0;
  bool framebufferResized = false;
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  VkBuffer indexBuffer;
  VkDeviceMemory indexBufferMemory;
  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;
  std::vector<void*> uniformBuffersMapped;
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;
  uint32_t mipLevels;
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  VkSampler textureSampler;
  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;
  VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
  VkImage colorImage;
  VkDeviceMemory colorImageMemory;
  VkImageView colorImageView;
  
  /* initialization functions */
  void createInstance();
  void setupDebugMessenger();
  void selectPhysicalDevice();
  void createLogicalDevice();
  void createSurface();
  void createSwapChain();
  void createImageViews();
  void createRenderPass();
  void createDescriptorSetLayout();
  void createGraphicsPipeline();
  void createCommandPool();
  void createColorResources();
  void createDepthResources();
  void createFramebuffers();
  void createTextureImage();
  void createTextureImageView();
  void createTextureSampler();
  void createUniformBuffers();
  void createDescriptorPool();
  void createDescriptorSets();
  void createCommandBuffers();
  void createSyncObjects();
  void initVulkan();
  
  /* handling things like resizes */
  void recreateSwapChain();
  static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
  
  /* de-initialization functions */
  void cleanupSwapChain();
  //  void destroyDebugMessenger();

  /* utility functions */
  bool checkValidationLayerSupport();
  void populateDebugMessengerCreateInfo(
										VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
  int rateDeviceSuitability(VkPhysicalDevice device);
  std::vector<const char *> getRequiredExtensions();
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes); 
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
  VkShaderModule createShaderModule(const std::vector<char> &byteCode);
  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, std::vector<RenderOp> renderOps);
  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  void updateUniformBuffer(uint32_t currentImage);
  void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling , VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer commandBuffer);
  void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
  void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
  VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
  VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
  VkFormat findDepthFormat();
  bool hasStencilComponent(VkFormat format);
  void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
  VkSampleCountFlagBits getMaxUsableSampleCount();
};

const std::vector<const char *> validationLayers = {
  "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

static std::vector<char> readBinAsset(const std::string& filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  
  if (!file.is_open()) {
    throw std::runtime_error("failed to open file!");
  }
  
  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);
  
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();
  
  return buffer;
}

