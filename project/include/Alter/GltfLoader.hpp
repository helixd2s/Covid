#pragma once

//
#ifdef __cplusplus
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
#ifdef ALT_ENABLE_VMA
#include "./MemoryAllocatorVma.hpp"
#include "./ResourceVma.hpp"
#endif

//
namespace ANAMED {

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
  struct GltfInstanced : std::enable_shared_from_this<GltfInstanced> {
    std::vector<InstanceDataInfo> instances = {};
    WrapShared<InstanceLevelObj> instanced = {};
  };

  //
  struct GltfModel : std::enable_shared_from_this<GltfModel> {
    //
    std::string err = "";
    std::string warn = "";
    tinygltf::Model model;

    // data
    std::vector<cpp21::shared_vector<GeometryExtension>> extensions = {};
    std::vector<cpp21::shared_vector<GeometryInfo>> geometries = {};
    std::vector<MaterialInfo> materials = {};
    std::vector<BufferRegion> regions = {};

    //
    std::vector<WrapShared<ResourceObj>> images = {};
    std::vector<WrapShared<SamplerObj>> samplers = {};

    // indices cache
    std::vector<uint32_t> imageIndices = {};
    std::vector<uint32_t> samplerIndices = {};
    std::vector<CTexture> textures = {};

    // objects
    std::vector<WrapShared<ResourceObj>> buffers = {};
    std::vector<WrapShared<GeometryLevelObj>> meshes = {};

    // 
    std::vector<std::shared_ptr<GltfInstanced>> scenes = {};
    std::vector<WrapShared<ResourceObj>> extensionBuffers = {};
    
    //
    WrapShared<ResourceObj> materialBuffer = {};

    //
    uintptr_t defaultScene = 0ull;

    //
    virtual std::shared_ptr<GltfInstanced> getDefaultScene() {
      return this->scenes[this->defaultScene];
    };

    //
    virtual std::shared_ptr<GltfInstanced> getScene(uintptr_t const& scene) {
      return this->scenes[scene];
    };

    //
    virtual std::shared_ptr<GltfInstanced> getScene() {
      return this->getDefaultScene();
    };
  };

  //
  inline vk::Filter convertFilter(uint32_t filter) {
    switch (filter) {
      case TINYGLTF_TEXTURE_FILTER_NEAREST: return vk::Filter::eNearest;
      case TINYGLTF_TEXTURE_FILTER_LINEAR: return vk::Filter::eLinear;
    };
    return vk::Filter::eLinear;
  };

  //
  inline vk::SamplerAddressMode convertAddressMode(uint32_t mode) {
    switch (mode) {
    case TINYGLTF_TEXTURE_WRAP_REPEAT: return vk::SamplerAddressMode::eRepeat;
    case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE: return vk::SamplerAddressMode::eClampToEdge;
    case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: return vk::SamplerAddressMode::eMirroredRepeat;
    };
    return vk::SamplerAddressMode::eRepeat;
  };

  //
  inline vk::Format convertFormat(uint32_t comp, uint32_t bits, uint32_t type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
    if (bits == 8) {
      if (comp == 1) { return vk::Format::eR8Unorm; }; //break;
      if (comp == 2) { return vk::Format::eR8G8Unorm; }; //break;
      if (comp == 3) { return vk::Format::eR8G8B8Unorm; }; //break;
      if (comp == 4) { return vk::Format::eR8G8B8A8Unorm; }; //break;
    } else 
    if (bits == 16) {
      if (type == TINYGLTF_COMPONENT_TYPE_FLOAT) {
        if (comp == 1) { return vk::Format::eR16Sfloat; }; //break;
        if (comp == 2) { return vk::Format::eR16G16Sfloat; }; //break;
        if (comp == 3) { return vk::Format::eR16G16B16Sfloat; }; //break;
        if (comp == 4) { return vk::Format::eR16G16B16A16Sfloat; }; //break;
      } else {
        if (comp == 1) { return vk::Format::eR16Unorm; }; //break;
        if (comp == 2) { return vk::Format::eR16G16Unorm; }; //break;
        if (comp == 3) { return vk::Format::eR16G16B16Unorm; }; //break;
        if (comp == 4) { return vk::Format::eR16G16B16A16Unorm; }; //break;
      }
    } else
    if (bits == 32) {
      if (comp == 1) { return vk::Format::eR32Sfloat; }; //break;
      if (comp == 2) { return vk::Format::eR32G32Sfloat; }; //break;
      if (comp == 3) { return vk::Format::eR32G32B32Sfloat; }; //break;
      if (comp == 4) { return vk::Format::eR32G32B32A32Sfloat; }; //break;
    };
    return vk::Format::eR8G8B8A8Unorm;
  };

  //
  inline vk::ComponentMapping convertComponentMap(uint32_t comp, uint32_t bits, uint32_t type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
    switch (comp) {
      case 4: return vk::ComponentMapping{ .r = vk::ComponentSwizzle::eR, .g = vk::ComponentSwizzle::eG, .b = vk::ComponentSwizzle::eB, .a = vk::ComponentSwizzle::eA }; break;
      case 3: return vk::ComponentMapping{ .r = vk::ComponentSwizzle::eR, .g = vk::ComponentSwizzle::eG, .b = vk::ComponentSwizzle::eB, .a = vk::ComponentSwizzle::eOne }; break;
      case 2: return vk::ComponentMapping{ .r = vk::ComponentSwizzle::eR, .g = vk::ComponentSwizzle::eR, .b = vk::ComponentSwizzle::eR, .a = vk::ComponentSwizzle::eG }; break;
      case 1: return vk::ComponentMapping{ .r = vk::ComponentSwizzle::eR, .g = vk::ComponentSwizzle::eR, .b = vk::ComponentSwizzle::eR, .a = vk::ComponentSwizzle::eOne }; break;
    };
    return vk::ComponentMapping{ .r = vk::ComponentSwizzle::eR, .g = vk::ComponentSwizzle::eG, .b = vk::ComponentSwizzle::eB, .a = vk::ComponentSwizzle::eA };
  };

  static void QuatToAngleAxis(const std::vector<double> quaternion,
    double& outAngleDegrees,
    double* axis) {
    double qx = quaternion[0];
    double qy = quaternion[1];
    double qz = quaternion[2];
    double qw = quaternion[3];

    double angleRadians = 2 * acos(qw);
    if (angleRadians == 0.0) {
      outAngleDegrees = 0.0;
      axis[0] = 0.0;
      axis[1] = 0.0;
      axis[2] = 1.0;
      return;
    }

    double denom = sqrt(1 - qw * qw);
    outAngleDegrees = angleRadians * 180.0 / glm::pi<double>();
    axis[0] = qx / denom;
    axis[1] = qy / denom;
    axis[2] = qz / denom;
  };

  // 
  class GltfLoaderObj : public BaseObj {
  public:
    using BaseObj::BaseObj;
    using tType = WrapShared<GltfLoaderObj>;

    //
    std::optional<GltfLoaderCreateInfo> cInfo = GltfLoaderCreateInfo{};

  protected:

    //
    std::vector<std::shared_ptr<GltfModel>> gltfModels = {};
    tinygltf::TinyGLTF loader;

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };


  public: 

    //
    GltfLoaderObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<GltfLoaderCreateInfo> cInfo = GltfLoaderCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
    };

    // 
    virtual std::shared_ptr<GltfModel> load(std::string const& filename = "./BoomBox.gltf", float scale = 1.f, FilterType const& filter = FilterType::eOpaque) {
      decltype(auto) gltf = std::make_shared<GltfModel>();

      //decltype(auto) handle = Handle(cInfo->device, HandleType::eDevice);
      decltype(auto) handle = this->base;
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(handle);
      decltype(auto) uploaderObj = deviceObj->get<UploaderObj>(Handle(this->cInfo->uploader, HandleType::eUploader));
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(this->cInfo->descriptors);

      // 
      bool ret = loader.LoadASCIIFromFile(&gltf->model, &gltf->err, &gltf->warn, filename);
      //bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]); // for binary glTF(.glb)

      if (!gltf->warn.empty()) {
        printf("Warn: %s\n", gltf->warn.c_str());
      };

      if (!gltf->err.empty()) {
        printf("Err: %s\n", gltf->err.c_str());
      };

      if (!ret) {
        printf("Failed to parse glTF\n");
        return std::shared_ptr<GltfModel>{};
      };

      //
      for (decltype(auto) image : gltf->model.images) {
        //
        decltype(auto) imageObj = ANAMED::ResourceObj::make(deviceObj, ANAMED::ResourceCreateInfo{
          .descriptors = cInfo->descriptors,
          .imageInfo = ANAMED::ImageCreateInfo{
            .format = convertFormat(image.component, image.bits, image.pixel_type),
            .extent = vk::Extent3D{uint32_t(image.width), uint32_t(image.height), 1u},
            .type = ANAMED::ImageType::eTexture
          }
          });

        //
        decltype(auto) status = uploaderObj->executeUploadToResourceOnce(ANAMED::UploadExecutionOnce{
          .host = cpp21::data_view<char8_t>((char8_t*)image.image.data(), 0ull, cpp21::bytesize(image.image)),
          .writeInfo = ANAMED::UploadCommandWriteInfo{
            // # yet another std::optional problem (implicit)
            .dstImage = std::optional<ANAMED::ImageRegion>(ANAMED::ImageRegion{.image = imageObj.as<vk::Image>(), .region = ANAMED::ImageDataRegion{.extent = vk::Extent3D{uint32_t(image.width), uint32_t(image.height), 1u}}}),
          }
          });

        //vk::ComponentMapping componentMapping = vk::ComponentMapping{ .r = vk::ComponentSwizzle::eR, .r = vk::ComponentSwizzle::eG, .r = vk::ComponentSwizzle::eB, .r = vk::ComponentSwizzle::eA };
        decltype(auto) imgImageView = imageObj->createImageView(ANAMED::ImageViewCreateInfo{
          .viewType = vk::ImageViewType::e2D,
          .componentMapping = convertComponentMap(image.component, image.bits, image.pixel_type)
          });

        //
        device.waitIdle();
        deviceObj->tickProcessing();

        //
        while (!status->checkStatus()) { deviceObj->tickProcessing(); };
        deviceObj->tickProcessing();

        //
        gltf->images.push_back(imageObj);
        gltf->imageIndices.push_back(std::get<1u>(imgImageView));
      };

      //
      for (decltype(auto) sampler : gltf->model.samplers) {
        decltype(auto) samplerObj = ANAMED::SamplerObj::make(deviceObj, ANAMED::SamplerCreateInfo{
          .descriptors = cInfo->descriptors,
          .native = vk::SamplerCreateInfo {
            .magFilter = convertFilter(sampler.magFilter),
            .minFilter = convertFilter(sampler.minFilter),
            .addressModeU = convertAddressMode(sampler.wrapS),
            .addressModeV = convertAddressMode(sampler.wrapT)
          }
          });
        gltf->samplers.push_back(samplerObj);
        gltf->samplerIndices.push_back(samplerObj->getId());
      };

      //
      for (decltype(auto) texture : gltf->model.textures) {
        gltf->textures.push_back(CTexture{ gltf->imageIndices[texture.source], texture.sampler >= 0 ? gltf->samplerIndices[texture.sampler] : 0u });
      };

      //
      decltype(auto) handleAccessor = [=,this](intptr_t const& accessorIndex, bool const& isIndice = false) {
        auto bresult = ANAMED::BufferViewInfo{ .region = ANAMED::BufferViewRegion{.deviceAddress = 0ull, .stride = 0ull, .size = 0ull}, .format = ANAMED::BufferViewFormat::eNone };;

        if (accessorIndex >= 0) {
          //decltype(auto) bufferView = ANAMED::BufferViewInfo{ .region = ANAMED::BufferViewRegion{} };
          auto accessor = gltf->model.accessors[accessorIndex];
          auto bv = gltf->regions[accessor.bufferView];
          auto bufferView = gltf->model.bufferViews[accessor.bufferView];
          auto bufferObj = deviceObj->get<ResourceObj>(bv.buffer);
          auto address = bufferObj->getDeviceAddress();

          //
          auto defFormat = ANAMED::BufferViewFormat::eFloat4;
          if (accessor.componentType == 5126) {
            if (accessor.type == TINYGLTF_TYPE_VEC4) { defFormat = ANAMED::BufferViewFormat::eFloat4; };
            if (accessor.type == TINYGLTF_TYPE_VEC3) { defFormat = ANAMED::BufferViewFormat::eFloat3; };
            if (accessor.type == TINYGLTF_TYPE_VEC2) { defFormat = ANAMED::BufferViewFormat::eFloat2; };
            if (accessor.type == TINYGLTF_TYPE_SCALAR) { defFormat = ANAMED::BufferViewFormat::eFloat; };
          };

          //
          auto decomposeFormat = BufferViewFormatBitSet(defFormat);
          auto virtualStride = bufferView.byteStride > 0 ? bufferView.byteStride : (decomposeFormat.countMinusOne + 1u) * (decomposeFormat.is16bit ? 2u : 4u);
          auto realStride = std::max(uint32_t(bufferView.byteStride), (decomposeFormat.countMinusOne + 1u) * (decomposeFormat.is16bit ? 2u : 4u));
          auto realSize = std::max({uint32_t(std::max(intptr_t(bufferView.byteStride * accessor.count) /* - intptr_t(accessor.byteOffset)*/,0ll)), uint32_t((decomposeFormat.countMinusOne + 1u) * (decomposeFormat.is16bit ? 2u : 4u) * accessor.count), uint32_t(bv.region.size - accessor.byteOffset)});

          //
          if (isIndice) {
            auto defFormat = ANAMED::BufferViewFormat::eUint3;
            if (accessor.componentType == 5123) { defFormat = ANAMED::BufferViewFormat::eShort3; };

            //
            auto decomposeFormat = BufferViewFormatBitSet(defFormat);
            auto virtualStride = bufferView.byteStride > 0 ? bufferView.byteStride : ((decomposeFormat.countMinusOne + 1u) * (decomposeFormat.is16bit ? 2u : 4u) / 3u);
            auto realStride = std::max(uint32_t(bufferView.byteStride), ((decomposeFormat.countMinusOne + 1u) * (decomposeFormat.is16bit ? 2u : 4u)) / 3u);
            auto realSize = std::max({ uint32_t(std::max(intptr_t(bufferView.byteStride * accessor.count) /*- intptr_t(accessor.byteOffset)*/,0ll)), uint32_t((decomposeFormat.countMinusOne + 1u) * (decomposeFormat.is16bit ? 2u : 4u) * accessor.count / 3), uint32_t(bv.region.size - accessor.byteOffset)});

            //
            return (bresult = ANAMED::BufferViewInfo{ .region = ANAMED::BufferViewRegion{.deviceAddress = address + bv.region.offset + accessor.byteOffset, .stride = defFormat == ANAMED::BufferViewFormat::eUint3 ? 4u : 2u, .size = realSize}, .format = defFormat });
          };

          //
          return (bresult = ANAMED::BufferViewInfo{ .region = ANAMED::BufferViewRegion{.deviceAddress = address + bv.region.offset + accessor.byteOffset, .stride = uint32_t(virtualStride), .size = realSize}, .format = defFormat });
        };

        return bresult;
      };

      //
      decltype(auto) handleFactor = [=](auto const& factor) {
        return glm::vec4(factor[0], factor[1], factor[2], factor.size() > 3 ? factor[3] : 1.f);
      };

      // 
      for (decltype(auto) buffer : gltf->model.buffers) {
        //
        decltype(auto) bufferObj = ANAMED::ResourceObj::make(handle, ANAMED::ResourceCreateInfo{
          .descriptors = cInfo->descriptors,
          .bufferInfo = ANAMED::BufferCreateInfo{
            .size = cpp21::bytesize(buffer.data),
            .type = ANAMED::BufferType::eUniversal,
          }
        });

        // complete loader
        decltype(auto) status = uploaderObj->executeUploadToResourceOnce(ANAMED::UploadExecutionOnce{
          .host = cpp21::data_view<char8_t>((char8_t*)buffer.data.data(), 0ull, cpp21::bytesize(buffer.data)),
          .writeInfo = ANAMED::UploadCommandWriteInfo{
            .dstBuffer = ANAMED::BufferRegion{bufferObj.as<vk::Buffer>(), ANAMED::DataRegion{0ull, 1ull, cpp21::bytesize(buffer.data)}},
          }
        });

        //
        device.waitIdle();
        deviceObj->tickProcessing();

        //
        while (!status->checkStatus()) { deviceObj->tickProcessing(); };
        deviceObj->tickProcessing();

        // 
        gltf->buffers.push_back(bufferObj);
      };

      //
      for (decltype(auto) bufferView : gltf->model.bufferViews) {
        gltf->regions.push_back(BufferRegion{ .buffer = gltf->buffers[bufferView.buffer].as<vk::Buffer>(), .region = DataRegion{bufferView.byteOffset, bufferView.byteStride, bufferView.byteLength}});
      };

      //
      gltf->materialBuffer = ANAMED::ResourceObj::make(handle, ANAMED::ResourceCreateInfo{
        .descriptors = cInfo->descriptors,
        .bufferInfo = ANAMED::BufferCreateInfo{
          .size = gltf->model.materials.size() * sizeof(ANAMED::MaterialInfo),
          .type = ANAMED::BufferType::eUniversal,
        }
      });

      //
      uint64_t materialAddress = gltf->materialBuffer->getDeviceAddress();

      //
      for (decltype(auto) material : gltf->model.materials) {
        decltype(auto) materialInf = ANAMED::MaterialInfo{};
        materialInf.texCol[uint32_t(ANAMED::TextureBind::eAlbedo)] = ANAMED::TexOrDef{ .texture = material.pbrMetallicRoughness.baseColorTexture.index >= 0 ? gltf->textures[material.pbrMetallicRoughness.baseColorTexture.index] : CTexture{}, .defValue = handleFactor(material.pbrMetallicRoughness.baseColorFactor)};
        //materialInf.texCol[uint32_t(ANAMED::TextureBind::eAlbedo)] = ANAMED::TexOrDef{ .texture = CTexture{}, .defValue = /*handleFactor(material.pbrMetallicRoughness.baseColorFactor)*/ glm::vec4(1.f)};
        materialInf.texCol[uint32_t(ANAMED::TextureBind::eNormal)] = ANAMED::TexOrDef{ .texture = material.normalTexture.index >= 0 ? gltf->textures[material.normalTexture.index] : CTexture{}, .defValue = glm::vec4(0.5f, 0.5f, 1.f, 1.f) };
        materialInf.texCol[uint32_t(ANAMED::TextureBind::ePBR)] = ANAMED::TexOrDef{ .texture = material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0 ? gltf->textures[material.pbrMetallicRoughness.metallicRoughnessTexture.index] : CTexture{}, .defValue = glm::vec4(1.f, material.pbrMetallicRoughness.roughnessFactor, material.pbrMetallicRoughness.metallicFactor, 1.f) };
        materialInf.texCol[uint32_t(ANAMED::TextureBind::eEmissive)] = ANAMED::TexOrDef{ .texture = material.emissiveTexture.index >= 0 ? gltf->textures[material.emissiveTexture.index] : CTexture{}, .defValue = handleFactor(material.emissiveFactor) };
        gltf->materials.push_back(materialInf);
      };

      {
        //
        decltype(auto) status = uploaderObj->executeUploadToResourceOnce(ANAMED::UploadExecutionOnce{
          .host = cpp21::data_view<char8_t>((char8_t*)gltf->materials.data(), 0ull, cpp21::bytesize(gltf->materials)),
          .writeInfo = ANAMED::UploadCommandWriteInfo{
            .dstBuffer = ANAMED::BufferRegion{gltf->materialBuffer.as<vk::Buffer>(), ANAMED::DataRegion{0ull, sizeof(ANAMED::MaterialInfo), cpp21::bytesize(gltf->materials)}},
          }
        });

        //
        device.waitIdle();
        deviceObj->tickProcessing();

        //
        while (!status->checkStatus()) { deviceObj->tickProcessing(); };
        deviceObj->tickProcessing();
        
      };

      //
      for (decltype(auto) mesh : gltf->model.meshes) {
        //
        decltype(auto) extensionBuffer = ANAMED::ResourceObj::make(handle, ANAMED::ResourceCreateInfo{
          .descriptors = descriptorsObj.as<vk::PipelineLayout>(),
          .bufferInfo = ANAMED::BufferCreateInfo{
            .size = mesh.primitives.size() * sizeof(GeometryExtension),
            .type = ANAMED::BufferType::eUniversal,
          }
        });

        //
        uint64_t extensionAddress = extensionBuffer->getDeviceAddress();

        //
        gltf->extensions.push_back(std::vector<ANAMED::GeometryExtension>{});
        gltf->geometries.push_back(std::vector<ANAMED::GeometryInfo>{});
        gltf->extensionBuffers.push_back(extensionBuffer);

        // GEOMETRIES BUFFER BROKEN FOR NO "/fsanitize=address"
        auto extensions = gltf->extensions.back();
        auto geometries = gltf->geometries.back();

        //
        uintptr_t pCount = 0ull;
        for (decltype(auto) primitive : mesh.primitives) {
          uintptr_t pId = pCount++;
          //if (pId > 1) break; // for debug

          // 
          decltype(auto) vertices = handleAccessor(primitive.attributes.at("POSITION"));
          decltype(auto) indices = handleAccessor(primitive.indices, true);
          decltype(auto) nullView = ANAMED::BufferViewInfo{ .region = ANAMED::BufferViewRegion{.deviceAddress = 0ull, .stride = 0ull, .size = 0ull}, .format = ANAMED::BufferViewFormat::eNone };

          //
          decltype(auto) materialId = std::min(std::max(uintptr_t(primitive.material), 0ull), uintptr_t(gltf->materials.size() - 1u));
          geometries->push_back(ANAMED::GeometryInfo{
            .vertices = vertices,
            .indices = primitive.indices >= 0 ? indices : nullView,
            .extensionRef = extensionAddress + pId * sizeof(ANAMED::GeometryExtension),
            .materialRef = materialAddress + materialId * sizeof(ANAMED::MaterialInfo),
            .primitiveCount = uint32_t(gltf->model.accessors[primitive.indices >= 0 ? primitive.indices : primitive.attributes.at("POSITION")].count) / 3u
          });

          //
          extensions->push_back(ANAMED::GeometryExtension{});
          extensions->back().bufferViews[uint32_t(ANAMED::BufferBind::eExtTexcoord)] = nullView;
          extensions->back().bufferViews[uint32_t(ANAMED::BufferBind::eExtNormals)] = nullView;
          extensions->back().bufferViews[uint32_t(ANAMED::BufferBind::eExtTangent)] = nullView;

          //
          for (decltype(auto) attrib : primitive.attributes) {
            if (attrib.first == "TEXCOORD_0") {
              extensions->back().bufferViews[uint32_t(ANAMED::BufferBind::eExtTexcoord)] = handleAccessor(attrib.second);
            };
            if (attrib.first == "NORMAL") {
              extensions->back().bufferViews[uint32_t(ANAMED::BufferBind::eExtNormals)] = handleAccessor(attrib.second);
            };
            if (attrib.first == "TANGENT") {
              extensions->back().bufferViews[uint32_t(ANAMED::BufferBind::eExtTangent)] = handleAccessor(attrib.second);
            };
          };
        };

        {
          //
          decltype(auto) status = uploaderObj->executeUploadToResourceOnce(ANAMED::UploadExecutionOnce{
            .host = cpp21::data_view<char8_t>((char8_t*)extensions->data(), 0ull, cpp21::bytesize(*extensions)),
            .writeInfo = ANAMED::UploadCommandWriteInfo{
              .dstBuffer = ANAMED::BufferRegion{extensionBuffer.as<vk::Buffer>(), ANAMED::DataRegion{0ull, sizeof(ANAMED::GeometryExtension), cpp21::bytesize(*extensions)}},
            }
          });

          //
          device.waitIdle();
          deviceObj->tickProcessing();

          //
          while (!status->checkStatus()) { deviceObj->tickProcessing(); };
          deviceObj->tickProcessing();
          
          //
          // GEOMETRIES BUFFER BROKEN FOR NO "/fsanitize=address"
          gltf->meshes.push_back(ANAMED::GeometryLevelObj::make(handle, ANAMED::GeometryLevelCreateInfo{
            .geometries = geometries,
            .uploader = uploaderObj.as<uintptr_t>(),
          }));
        };
      };

      // 
      decltype(auto) useMesh = [=, this](std::shared_ptr<GltfInstanced> inst, tinygltf::Model& model, intptr_t const& meshId, glm::dmat4 transform = glm::dmat4()) {
        decltype(auto) mesh = model.meshes[meshId];
        decltype(auto) transposed = glm::mat4(glm::transpose(transform));
        inst->instances.push_back(InstanceDataInfo{
          .instanceDevInfo = InstanceDevInfo{
            .transform = reinterpret_cast<vk::TransformMatrixKHR&>(transposed),
            .instanceCustomIndex = 0u,
            .mask = 0xFFu,
            .instanceShaderBindingTableRecordOffset = 0u,
            .flags = uint8_t(vk::GeometryInstanceFlagBitsKHR{}/*::eTriangleFrontCounterclockwise*/),
            .accelerationStructureReference = gltf->meshes[meshId]->getDeviceAddress()
          },
          .instanceInfo {
            //.normalTransform = glm::mat3x3(glm::inverse(transform))
          }
        });
      };

      //
      std::function<void(std::shared_ptr<GltfInstanced>, tinygltf::Model&, tinygltf::Node&, glm::dmat4 parentTransform)> handleNodes = {};
      handleNodes = [=, &handleNodes, this](std::shared_ptr<GltfInstanced> inst, tinygltf::Model& model, tinygltf::Node& node, glm::dmat4 transform = glm::dmat4(1.f)) {
        // 
        auto localTransform = glm::dmat4(1.0);
        localTransform *= node.matrix.size() >= 16 ? glm::make_mat4(node.matrix.data()) : glm::dmat4(1.0);
        localTransform *= node.translation.size() >= 3 ? glm::translate(glm::dmat4(1.0), glm::make_vec3(node.translation.data())) : glm::dmat4(1.0);
        localTransform *= node.scale.size() >= 3 ? glm::scale(glm::dmat4(1.0), glm::make_vec3(node.scale.data())) : glm::dmat4(1.0);
        localTransform *= node.rotation.size() >= 4 ? glm::mat4_cast(glm::make_quat(node.rotation.data())) : glm::dmat4(1.0);

        //
        //if (node.rotation.size() >= 4) {
          //glm::dvec3 rot = glm::dvec3(0.0);
          //double angle = 0;
          //QuatToAngleAxis(node.rotation, angle, (double*)&rot);
          //localTransform *= glm::rotate(glm::dmat4(1.0), angle, rot);
        //};

        //
        if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
          useMesh(inst, model, node.mesh, transform * localTransform);
        };

        //
        for (size_t i = 0; i < node.children.size(); i++) {
          assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
          handleNodes(inst, model, model.nodes[node.children[i]], transform * localTransform);
        };
      };

      //
      for (decltype(auto) scene : gltf->model.scenes) {
        decltype(auto) inst = std::make_shared<GltfInstanced>();
        for (decltype(auto) node : scene.nodes) {
          handleNodes(inst, gltf->model, gltf->model.nodes[node], glm::dmat4(1.f) * glm::scale(glm::dmat4(1.0f), glm::dvec3(1.f * scale, 1.f * scale, 1.f * scale)));
        };

        //
        inst->instanced = ANAMED::InstanceLevelObj::make(handle, ANAMED::InstanceLevelCreateInfo{
          .instances = inst->instances,
          .uploader = this->cInfo->uploader,
          });

        //
        gltf->scenes.push_back(inst);
      };

      //
      gltf->defaultScene = gltf->model.defaultScene == -1 ? 0 : gltf->model.defaultScene;

      //
      device.waitIdle();
      deviceObj->tickProcessing();

      //
      this->gltfModels.push_back(gltf);
      return this->gltfModels.back();
    };

    //
    virtual std::shared_ptr<GltfInstanced> getDefaultScene(uintptr_t const& model = 0ull) {
      return this->gltfModels[model]->getDefaultScene();
    };

    //
    virtual std::shared_ptr<GltfInstanced> getScene(uintptr_t const& model, uintptr_t const& scene) {
      return this->gltfModels[model]->getScene(scene);
    };

    //
    virtual std::shared_ptr<GltfInstanced> getScene(uintptr_t const& model = 0ull) {
      return this->gltfModels[model]->getScene();
    };

    

    //
    virtual tType registerSelf() {
      ANAMED::context->registerObj(this->handle, shared_from_this());
      return SFT();
    };

    // 
     std::type_info const& type_info() const override {
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
#endif
