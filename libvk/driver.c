#include "CSCIx239Vk.h"

VkPhysicalDevice findPhysicalDevice(GLFWwindow* window) {

    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    // uint32_t presentFamily;
    // uint32_t graphicsFamily;
    VkSurfaceCapabilitiesKHR capabilities;
    uint32_t formatCount;
    VkSurfaceFormatKHR* formats=NULL;
    uint32_t presentModeCount;
    VkPresentModeKHR* presentModes=NULL;

   //  Create Vulkan instance
   VkApplicationInfo appInfo =
   {
      .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName   = "Vulkan Textured Cube",
      .applicationVersion = VK_MAKE_VERSION(1,0,0),
      .pEngineName        = "No Engine",
      .engineVersion      = VK_MAKE_VERSION(1,0,0),
      .apiVersion         = VK_API_VERSION_1_0,
   };

   uint32_t glfwExtensionCount = 0;
   const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

   VkInstanceCreateInfo createInfo =
   {
      .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo        = &appInfo,
      .enabledExtensionCount   = glfwExtensionCount,
      .ppEnabledExtensionNames = glfwExtensions,
   };
   int ios = vkCreateInstance(&createInfo,NULL,&instance);
   if (ios) Fatal("Failed to create instance: %s\n",ErrString(ios));

   //  Create window surface
   ios = glfwCreateWindowSurface(instance,window,NULL,&surface);
   if (ios) Fatal("Failed to create window surface: %s\n",ErrString(ios));

   //
   //  Get available devices
   //
   uint32_t deviceCount=0;
   vkEnumeratePhysicalDevices(instance,&deviceCount,NULL);
   if (deviceCount==0) Fatal("Failed to find GPUs with Vulkan support\n");
   VkPhysicalDevice* devices = (VkPhysicalDevice*)malloc(deviceCount*sizeof(VkPhysicalDevice));
   if (!devices) Fatal("Failed to malloc %d VkPhysicalDevice\n",deviceCount);
   vkEnumeratePhysicalDevices(instance,&deviceCount,devices);

   //  Required device extensions
   const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

   //
   //  Select phyical device
   //
   physicalDevice = VK_NULL_HANDLE;
   for (int k=0;k<deviceCount && physicalDevice==VK_NULL_HANDLE;k++)
   {
      uint32_t queueFamilyCount = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(devices[k],&queueFamilyCount,NULL);
      VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*)malloc(queueFamilyCount*sizeof(VkQueueFamilyProperties));
      if (!queueFamilies) Fatal("Failed to malloc %d VkQueueFamilyProperties\n",queueFamilyCount);
      vkGetPhysicalDeviceQueueFamilyProperties(devices[k],&queueFamilyCount,queueFamilies);
      VkBool32 suitable = VK_FALSE;
      for (int i=0;i<queueFamilyCount && !suitable;i++)
      {
         VkBool32 presentSupport = VK_FALSE;
         vkGetPhysicalDeviceSurfaceSupportKHR(devices[k],i,surface,&presentSupport);
         VkPhysicalDeviceFeatures supportedFeatures;
         vkGetPhysicalDeviceFeatures(devices[k],&supportedFeatures);
         if (presentSupport && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && supportedFeatures.samplerAnisotropy)
         {
            suitable = VK_TRUE;
            // presentFamily  = i;
            // graphicsFamily = i;
         }
      }
      free(queueFamilies);

      //  Check that all required device extentions are supported
      if (suitable)
      {
         //  Get device extensions supported
         uint32_t extensionCount;
         vkEnumerateDeviceExtensionProperties(devices[k],NULL,&extensionCount,NULL);
         VkExtensionProperties* availableExtensions = (VkExtensionProperties*)malloc(extensionCount*sizeof(VkExtensionProperties));
         if (!availableExtensions) Fatal("Failed to malloc %d VkExtensionProperties\n",extensionCount);
         vkEnumerateDeviceExtensionProperties(devices[k],NULL,&extensionCount,availableExtensions);

         //  Check if device extentions match
         for (int k=0;k<sizeof(deviceExtensions)/sizeof(char*) && suitable;k++)
         {
            VkBool32 match = VK_FALSE;
            for (int i=0;i<extensionCount && !match;i++)
               if (!strcmp(deviceExtensions[k],availableExtensions[i].extensionName)) match = VK_TRUE;
            if (!match) suitable = VK_FALSE;
         }
         free(availableExtensions);
      }

      //  Get capabilities if device is suitable
      if (suitable)
      {
         //  Get device capabilities
         vkGetPhysicalDeviceSurfaceCapabilitiesKHR(devices[k],surface,&capabilities);
         //  Get formats
         if (formats) free(formats);
         vkGetPhysicalDeviceSurfaceFormatsKHR(devices[k],surface,&formatCount,NULL);
         if (formatCount)
         {
            formats = (VkSurfaceFormatKHR*)malloc(formatCount*sizeof(VkSurfaceFormatKHR));
            if (!formats) Fatal("Failed to malloc %d VkSurfaceFormatKHR\n",formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(devices[k],surface,&formatCount,formats);
         }
         else
            formats = NULL;
         //  Get presentation modes
         if (presentModes) free(presentModes);
         vkGetPhysicalDeviceSurfacePresentModesKHR(devices[k],surface,&presentModeCount,NULL);
         if (presentModeCount)
         {
            presentModes = (VkPresentModeKHR*)malloc(presentModeCount*sizeof(VkPresentModeKHR));
            if (!presentModes) Fatal("Failed to malloc %d VkPresentModeKHR\n",presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(devices[k],surface,&presentModeCount,presentModes);
         }
         else
            presentModes = NULL;
         //  Select device if formats and presentation modes are acceptable
         if (formats && presentModes)
            physicalDevice = devices[k];
      }
   }
   return physicalDevice;
}

int main() {
    
    //  Initialize GLFW, GLEW and launch window
    GLFWwindow* window = InitWindow("Drew Smith", 0, 800, 600, NULL, NULL);
    VkPhysicalDevice physicalDevice = findPhysicalDevice(window);
    struct Device device = CreateDevice(physicalDevice);
    printf("Found physical device %s\n", device.properties.deviceName);

    return 0;
}