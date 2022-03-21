#pragma once

//
#ifdef _WIN32
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#endif
#else
#ifdef __linux__ 
//FD defaultly
#endif
#endif

//
#include <tinygltf/tiny_gltf.h>
#include <tinygltf/stb_image.h>
#include <glm/gtc/matrix_transform.hpp>

// 
#include <ZEON/ZEON.hpp>
#include <GLFW/glfw3.h>
#ifdef ENABLE_RENDERDOC
#include "renderdoc_app.h"
#include <eh.h>
#endif

//
namespace ZNAMED {

  // 
  struct GltfLoaderCreateInfo {
    vk::Device device = {};
    uintptr_t uploader = 0ull;
    vk::PipelineLayout descriptors = {};
  };
  
  // 
  class GltfLoader {
  protected:

    //
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err = "";
    std::string warn = "";

    std::vector<WrapShared<ResourceObj>> buffers = {};
    std::vector<BufferRegion> regions = {};
    std::optional<GltfLoaderCreateInfo> cInfo = GltfLoaderCreateInfo{};

  public: 
    GltfLoader(std::optional<GltfLoaderCreateInfo> const& cInfo = GltfLoaderCreateInfo{}) {
      this->cInfo = cInfo;
    };

    // 
    virtual int loadGLTF(std::string const& filename = "./BoomBox.gltf") {
      decltype(auto) handle = Handle(cInfo->device, HandleType::eDevice);
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(handle);
      decltype(auto) uploaderObj = deviceObj->get<UploaderObj>(this->cInfo->uploader);

      bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
      //bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]); // for binary glTF(.glb)

      if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
      }

      if (!err.empty()) {
        printf("Err: %s\n", err.c_str());
      }

      if (!ret) {
        printf("Failed to parse glTF\n");
        return -1;
      }

      for (auto& buffer : model.buffers) {
        //
        decltype(auto) bufferObj = ZNAMED::ResourceObj::make(handle, ZNAMED::ResourceCreateInfo{
          .descriptors = cInfo->descriptors,
          .bufferInfo = ZNAMED::BufferCreateInfo{
            .size = cpp21::bytesize(buffer.data),
            .type = ZNAMED::BufferType::eUniversal,
          }
          }.use(ZNAMED::ExtensionName::eMemoryAllocatorVma));

        // complete loader
        uploaderObj->executeUploadToResourceOnce(ZNAMED::UploadExecutionOnce{
          .host = cpp21::data_view<char8_t>((char8_t*)buffer.data.data(), 0ull, cpp21::bytesize(buffer.data)),
          .writeInfo = ZNAMED::UploadCommandWriteInfo{
            .dstBuffer = ZNAMED::BufferRegion{bufferObj.as<vk::Buffer>(), ZNAMED::DataRegion{0ull, 1ull, cpp21::bytesize(buffer.data)}},
          }
        });

        // 
        buffers.push_back(bufferObj);
      };

      //
      for (auto& bufferView : model.bufferViews) {
        regions.push_back(BufferRegion{ .buffer = buffers[bufferView.buffer].as<vk::Buffer>(), .region = DataRegion{bufferView.byteOffset, bufferView.byteStride, bufferView.byteLength}});
      };

      // 
      return 0;
    };



  };
  
};