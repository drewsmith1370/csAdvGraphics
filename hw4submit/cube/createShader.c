#include "CSCIx239Vk.h"

//
//  Create shader module
//
VkShaderModule CreateShaderModule(VkDevice device, const char* file)
{
   //  Open file
   FILE* f = fopen(file,"rb");
   if (!f) Fatal("Cannot open shader file %s\n",file);
   //  Seek to end to determine size, then rewind
   fseek(f,0,SEEK_END);
   int n = ftell(f);
   rewind(f);
   //  Allocate memory for the whole file
   uint32_t* code = (uint32_t*)malloc(n);
   if (!code) Fatal("Failed to malloc %d bytes for shader file %s\n",n,file);
   //  Snarf and close the file
   if (fread(code,n,1,f)!=1) Fatal("Cannot read %d bytes from shader file %s\n",n,file);
   fclose(f);
   //  Fill in shader struct
   VkShaderModuleCreateInfo shaderInfo =
   {
      .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = n,
      .pCode    = code,
   };
   //  Create shader
   VkShaderModule shaderModule;
   if (vkCreateShaderModule(device,&shaderInfo,NULL,&shaderModule))
      Fatal("Failed to create shader module\n");
   //  Free code and return
   free(code);
   return shaderModule;
}
