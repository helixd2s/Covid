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
#include <glm/gtc/quaternion.hpp>
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
  enum class FilterType : uint32_t {
    eEverything = 0u,
    eOpaque = 1u,
    eTranslucent = 2u
  };

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
    std::vector<uint32_t> imageIndices = {};
    std::vector<uint32_t> samplerIndices = {};
    std::vector<CTexture> textures = {};
    std::vector<WrapShared<GeometryLevelObj>> meshes = {};
    WrapShared<InstanceLevelObj> instanced = {};

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
    virtual tType load(std::string const& filename = "./BoomBox.gltf", FilterType const& filter = FilterType::eOpaque) {
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

      //
      decltype(auto) handleAccessor = [=,this](intptr_t const& accessorIndex, bool const& isIndice = false) {
        auto bresult = ZNAMED::BufferViewInfo{ .region = ZNAMED::BufferViewRegion{.deviceAddress = 0ull, .stride = 0ull, .size = 0ull}, .format = ZNAMED::BufferViewFormat::eNone };;

        if (accessorIndex >= 0) {
          //decltype(auto) bufferView = ZNAMED::BufferViewInfo{ .region = ZNAMED::BufferViewRegion{} };
          auto accessor = model.accessors[accessorIndex];
          auto bv = this->regions[accessor.bufferView];
          auto bufferView = model.bufferViews[accessor.bufferView];
          auto bufferObj = deviceObj->get<ResourceObj>(bv.buffer);
          auto address = bufferObj->getDeviceAddress();

          //
          if (isIndice) {
            auto defFormat = ZNAMED::BufferViewFormat::eUint3;
            if (accessor.componentType == 5123) { defFormat = ZNAMED::BufferViewFormat::eShort3; };
            return (bresult = ZNAMED::BufferViewInfo{ .region = ZNAMED::BufferViewRegion{.deviceAddress = address + bv.region.offset + accessor.byteOffset, .stride = defFormat == ZNAMED::BufferViewFormat::eUint3 ? 4u : 2u, .size = uint32_t(bv.region.size - accessor.byteOffset)}, .format = defFormat });
          };

          //
          auto defFormat = ZNAMED::BufferViewFormat::eFloat3;
          if (accessor.componentType == 5126) {
            if (accessor.type == TINYGLTF_TYPE_VEC4) { defFormat = ZNAMED::BufferViewFormat::eFloat4; };
            if (accessor.type == TINYGLTF_TYPE_VEC3) { defFormat = ZNAMED::BufferViewFormat::eFloat3; };
            if (accessor.type == TINYGLTF_TYPE_VEC2) { defFormat = ZNAMED::BufferViewFormat::eFloat2; };
            if (accessor.type == TINYGLTF_TYPE_SCALAR) { defFormat = ZNAMED::BufferViewFormat::eFloat; };
          };

          //
          auto decomposeFormat = BufferViewFormatBitSet(defFormat);
          auto virtualStride = bufferView.byteStride > 0 ? bufferView.byteStride : (decomposeFormat.countMinusOne + 1u) * (decomposeFormat.is16bit ? 2u : 4u);
          auto realStride = std::max(uint32_t(bufferView.byteStride), (decomposeFormat.countMinusOne + 1u) * (decomposeFormat.is16bit ? 2u : 4u));

          //
          return (bresult = ZNAMED::BufferViewInfo{ .region = ZNAMED::BufferViewRegion{.deviceAddress = address + bv.region.offset + accessor.byteOffset, .stride = uint32_t(virtualStride), .size = std::min(uint32_t(bv.region.size - accessor.byteOffset), uint32_t(virtualStride * accessor.count))}, .format = defFormat });
        };

        return bresult;
      };

      //
      decltype(auto) handleFactor = [=](auto const& factor) {
        return glm::vec4(factor[0], factor[1], factor[2], factor[3]);
      };

      // 
      for (decltype(auto) buffer : model.buffers) {
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
      for (decltype(auto) bufferView : model.bufferViews) {
        regions.push_back(BufferRegion{ .buffer = buffers[bufferView.buffer].as<vk::Buffer>(), .region = DataRegion{bufferView.byteOffset, bufferView.byteStride, bufferView.byteLength}});
      };

      //
      decltype(auto) materialBuffer = ZNAMED::ResourceObj::make(handle, ZNAMED::ResourceCreateInfo{
        .descriptors = cInfo->descriptors,
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
      for (decltype(auto) image : model.images) {
        //
        decltype(auto) imageObj = ZNAMED::ResourceObj::make(deviceObj, ZNAMED::ResourceCreateInfo{
          .descriptors = cInfo->descriptors,
          .imageInfo = ZNAMED::ImageCreateInfo{
            .format = vk::Format::eR8G8B8A8Unorm,
            .extent = vk::Extent3D{uint32_t(image.width), uint32_t(image.height), 1u},
            .type = ZNAMED::ImageType::eTexture
          }
        }.use(ZNAMED::ExtensionName::eMemoryAllocatorVma));

        //
        uploaderObj->executeUploadToResourceOnce(ZNAMED::UploadExecutionOnce{
          .host = cpp21::data_view<char8_t>((char8_t*)image.image.data(), 0ull, cpp21::bytesize(image.image)),
          .writeInfo = ZNAMED::UploadCommandWriteInfo{
            .dstImage = ZNAMED::ImageRegion{.image = imageObj.as<vk::Image>(), .region = ZNAMED::ImageDataRegion{.extent = vk::Extent3D{uint32_t(image.width), uint32_t(image.height), 1u}}},
          }
        });

        //
        decltype(auto) imgImageView = imageObj->createImageView(ZNAMED::ImageViewCreateInfo{ .viewType = vk::ImageViewType::e2D });

        //
        imageIndices.push_back(std::get<1u>(imgImageView));
      };


      //
      for (decltype(auto) sampler : model.samplers) {
        decltype(auto) samplerObj = ZNAMED::SamplerObj::make(deviceObj, ZNAMED::SamplerCreateInfo{
          .descriptors = cInfo->descriptors,
          .native = vk::SamplerCreateInfo {
            .magFilter = vk::Filter::eLinear,
            .minFilter = vk::Filter::eLinear,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat
          }
        });
        samplerIndices.push_back(samplerObj->getId());
      };


      //
      for (decltype(auto) texture : model.textures) {
        textures.push_back(CTexture{ imageIndices[texture.source], texture.sampler >= 0 ? samplerIndices[texture.sampler] : 0u});
      };


      //
      for (auto& material : model.materials) {
        //
        decltype(auto) materialInf = ZNAMED::MaterialInfo{};

        materialInf.texCol[std::to_underlying(ZNAMED::TextureBind::eAlbedo)] = ZNAMED::TexOrDef{ .texture = material.pbrMetallicRoughness.baseColorTexture.index >= 0 ? textures[material.pbrMetallicRoughness.baseColorTexture.index] : CTexture{}, .defValue = handleFactor(material.pbrMetallicRoughness.baseColorFactor)};

        materials.push_back(materialInf);
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
          decltype(auto) vertices = handleAccessor(primitive.attributes.at("POSITION"));
          decltype(auto) indices = handleAccessor(primitive.indices, true);

          //
          geometries.push_back(ZNAMED::GeometryInfo{
            .vertices = vertices,
            .indices = indices,
            .extensionRef = extensionAddress + pId * sizeof(ZNAMED::GeometryExtension),
            .materialRef = materialAddress + primitive.material * sizeof(ZNAMED::MaterialInfo),
            .primitiveCount = cpp21::tiled(uint32_t(model.accessors[primitive.indices >= 0 ? primitive.indices : primitive.attributes.at("POSITION")].count), 3u),
          });

          //
          extensions.push_back(ZNAMED::GeometryExtension{});

          //
          for (decltype(auto) attrib : primitive.attributes) {
            if (attrib.first == "TEXCOORD_0") {
              extensions.back().bufferViews[std::to_underlying(ZNAMED::BufferBind::eExtTexcoord)] = handleAccessor(attrib.second);
            };
            if (attrib.first == "NORMAL") {
              extensions.back().bufferViews[std::to_underlying(ZNAMED::BufferBind::eExtNormals)] = handleAccessor(attrib.second);
            };
            if (attrib.first == "TANGENT") {
              extensions.back().bufferViews[std::to_underlying(ZNAMED::BufferBind::eExtTangent)] = handleAccessor(attrib.second);
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
        meshes.push_back(ZNAMED::GeometryLevelObj::make(handle, ZNAMED::GeometryLevelCreateInfo{
          .geometries = geometries,
          .uploader = uploaderObj.as<uintptr_t>(),
        }));
      };

      // CURRENTLY, OPAQUE INSTANCES
      decltype(auto) instances = std::vector<InstanceDevInfo>{};
      decltype(auto) useMesh = [=, &instances, this](tinygltf::Model& model, intptr_t const& meshId, glm::mat4x4 transform = glm::mat4x4()) {
        decltype(auto) mesh = model.meshes[meshId];
        decltype(auto) transposed = glm::transpose(transform);
        instances.push_back(InstanceDevInfo{
          .transform = reinterpret_cast<vk::TransformMatrixKHR&>(transposed),
          .instanceCustomIndex = 0u,
          .mask = 0xFFu,
          .instanceShaderBindingTableRecordOffset = 0u,
          .flags = uint8_t(vk::GeometryInstanceFlagBitsKHR::eForceOpaque),
          .accelerationStructureReference = this->meshes[meshId]->getDeviceAddress()
        });
      };

      //
      std::function<void(tinygltf::Model&, tinygltf::Node&, glm::mat4x4 parentTransform)> handleNodes = {};
      handleNodes = [=, &handleNodes, this](tinygltf::Model& model, tinygltf::Node& node, glm::mat4x4 parentTransform = glm::mat4x4(1.f)) {
        //parentTransform;
        if (node.matrix.size() == 16) {
          parentTransform = parentTransform * glm::mat4x4(reinterpret_cast<glm::dmat4x4&>(*node.matrix.data()));
        };

        // 
        if (node.translation.size() == 3) {
          parentTransform = parentTransform * glm::translate(glm::mat4x4(1.f), glm::vec3(reinterpret_cast<glm::dvec3&>(*node.translation.data())));
        };

        // 
        if (node.rotation.size() == 4) {
          parentTransform = parentTransform * glm::mat4_cast(glm::quat(reinterpret_cast<glm::dquat&>(*node.rotation.data())));
        };

        // 
        if (node.scale.size() == 3) {
          parentTransform = parentTransform * glm::scale(glm::mat4x4(1.f), glm::vec3(reinterpret_cast<glm::dvec3&>(*node.scale.data())));
        };

        //
        if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
          useMesh(model, node.mesh, parentTransform);
        };

        //
        for (size_t i = 0; i < node.children.size(); i++) {
          assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
          handleNodes(model, model.nodes[node.children[i]], parentTransform);
        };
      };

      //
      decltype(auto) scene = model.scenes[model.defaultScene];
      for (decltype(auto) node : scene.nodes) { handleNodes(model, model.nodes[node], glm::mat4x4(1.f)); };

      //
      instanced = ZNAMED::InstanceLevelObj::make(handle, ZNAMED::InstanceLevelCreateInfo{
        .instances = instances,
        .uploader = this->cInfo->uploader,
      });

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