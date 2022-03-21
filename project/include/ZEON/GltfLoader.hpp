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
#ifdef Z_ENABLE_GLTF
#include <tinygltf/tiny_gltf.h>
#include <tinygltf/stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#endif

//
#include "./Core.hpp"
#include "./Context.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./MemoryAllocator.hpp"
#include "./QueueFamily.hpp"
#include "./Resource.hpp"
#include "./Sampler.hpp"
#include "./Descriptors.hpp"
#include "./Framebuffer.hpp"
#include "./Pipeline.hpp"
#include "./Uploader.hpp"
#include "./Semaphore.hpp"
#include "./Swapchain.hpp"
#include "./GeometryLevel.hpp"
#include "./InstanceLevel.hpp"

// 
#ifdef Z_ENABLE_VMA
#include "./MemoryAllocatorVma.hpp"
#include "./ResourceVma.hpp"
#endif

//
namespace ZNAMED {

  // 
  struct GltfLoaderCreateInfo {
    uintptr_t uploader = 0ull;
    vk::PipelineLayout descriptors = {};
  };

  // 
  class GltfLoaderObj : public BaseObj {
  public:
    using BaseObj::BaseObj;
    using tType = WrapShared<GltfLoaderObj>;
    using cType = const char const*;
    //using BaseObj;

  protected:

    //
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err = "";
    std::string warn = "";

    // 
    std::vector<WrapShared<ResourceObj>> buffers = {};
    std::vector<BufferRegion> regions = {};
    std::optional<GltfLoaderCreateInfo> cInfo = GltfLoaderCreateInfo{};
    std::vector<uint32_t> textureIndices = {};
    std::vector<uint32_t> samplerIndices = {};

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

  public: 

    //
    GltfLoaderObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<GltfLoaderCreateInfo> cInfo = GltfLoaderCreateInfo{}) {
      this->cInfo = cInfo;
      this->base = handle;
    };

    // 
    virtual tType load(std::string const& filename = "./BoomBox.gltf") {
      //decltype(auto) handle = Handle(cInfo->device, HandleType::eDevice);
      decltype(auto) handle = this->base;
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(handle);
      decltype(auto) uploaderObj = deviceObj->get<UploaderObj>(this->cInfo->uploader);
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(this->cInfo->descriptors);

      // 
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
        return SFT();
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
      decltype(auto) materialBuffer = ZNAMED::ResourceObj::make(handle, ZNAMED::ResourceCreateInfo{
        .descriptors = descriptorsObj.as<vk::PipelineLayout>(),
        .bufferInfo = ZNAMED::BufferCreateInfo{
          .size = model.materials.size() * sizeof(ZNAMED::MaterialInfo),
          .type = ZNAMED::BufferType::eUniversal,
        }
      }.use(ZNAMED::ExtensionName::eMemoryAllocatorVma));

      //
      uint64_t materialAddress = materialBuffer->getDeviceAddress();

      //
      std::vector<ZNAMED::MaterialInfo> materials = {};

      //
      for (auto& material : model.materials) {
        //
        decltype(auto) materialInf = ZNAMED::MaterialInfo{};

        //materialInf.texCol[std::to_underlying(ZNAMED::TextureBind::eAlbedo)] = ZNAMED::TexOrDef{ .texture = ZNAMED::CTexture{.textureId = textureIndices[material.pbrMetallicRoughness.baseColorTexture.index], .samplerId = 0u}};

        materials.push_back(materialInf);
      };

      //
      decltype(auto) handleAccessor = [=](uintptr_t const& accessorIndex) {
        decltype(auto) bufferView = ZNAMED::BufferViewInfo{ .region = ZNAMED::BufferViewRegion{} };

        return bufferView;
      };

      //
      for (decltype(auto) mesh : model.meshes) {
        std::vector<ZNAMED::GeometryExtension> extensions = {};
        std::vector<ZNAMED::GeometryInfo> geometries = {};

        //
        decltype(auto) extensionBuffer = ZNAMED::ResourceObj::make(handle, ZNAMED::ResourceCreateInfo{
          .descriptors = descriptorsObj.as<vk::PipelineLayout>(),
          .bufferInfo = ZNAMED::BufferCreateInfo{
            .size = mesh.primitives.size() * sizeof(ZNAMED::GeometryExtension),
            .type = ZNAMED::BufferType::eUniversal,
          }
        }.use(ZNAMED::ExtensionName::eMemoryAllocatorVma));

        //
        uint64_t extensionAddress = extensionBuffer->getDeviceAddress();
        
        //
        uintptr_t pCount = 0ull;
        for (decltype(auto) primitive : mesh.primitives) {
          uintptr_t pId = pCount++;

          //
          geometries.push_back(ZNAMED::GeometryInfo{
            .vertices = handleAccessor(primitive.attributes.at("POSITION")),
            .indices = handleAccessor(primitive.indices),
            .extensionRef = extensionAddress + pId * sizeof(ZNAMED::GeometryExtension),
            .materialRef = materialAddress + primitive.material * sizeof(ZNAMED::MaterialInfo),
            .primitiveCount = 2u
          });

          //
          extensions.push_back(ZNAMED::GeometryExtension{});

          //
          for (auto& attrib : primitive.attributes) {
            if (attrib.first == "TEXCOORD_0") {
              extensions.back().bufferViews[std::to_underlying(ZNAMED::BufferBind::eExtTexcoord)] = handleAccessor(attrib.second);
            };
          };
        };

        //
        uploaderObj->executeUploadToResourceOnce(ZNAMED::UploadExecutionOnce{
          .host = cpp21::data_view<char8_t>((char8_t*)extensions.data(), 0ull, cpp21::bytesize(extensions)),
          .writeInfo = ZNAMED::UploadCommandWriteInfo{
            .dstBuffer = ZNAMED::BufferRegion{extensionBuffer.as<vk::Buffer>(), ZNAMED::DataRegion{0ull, sizeof(ZNAMED::GeometryExtension), cpp21::bytesize(extensions)}},
          }
        });

        //
        //decltype(auto) geometryLevel = ZNAMED::GeometryLevelObj::make(handle, ZNAMED::GeometryLevelCreateInfo{
          //.geometries = geometries,
          //.uploader = uploaderObj.as<uintptr_t>(),
        //});
      };

      // 
      return SFT();
    };

    //
    virtual tType registerSelf() {
      ZNAMED::context->registerObj(this->handle, shared_from_this());
      return SFT();
    };

    // 
    virtual std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<GltfLoaderCreateInfo> cInfo = GltfLoaderCreateInfo{}) {
      auto shared = std::make_shared<GltfLoaderObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

  };
  
};