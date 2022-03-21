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
void error(int errnum, const char* errmsg)
{
  std::cerr << errnum << ": " << errmsg << std::endl;
};

//
struct Constants
{
  glm::mat4x4 perspective = glm::mat4x4(1.f);
  glm::mat4x4 perspectiveInverse = glm::mat4x4(1.f);
  glm::mat3x4 lookAt = glm::mat3x4(1.f);
  glm::mat3x4 lookAtInverse = glm::mat3x4(1.f);
};

//
struct UniformData {
  uint32_t framebufferAttachments[4] = { 0u,0u,0u,0u };
  glm::uvec2 extent = {}; uint32_t testTex, testSampler;
  Constants constants = {};
  //uint64_t verticesAddress = 0ull;
};



// 
int main() {

  // Be sure to enable "Yes with SEH Exceptions (/EHa)" in C++ / Code Generation;
  _set_se_translator([](unsigned int u, EXCEPTION_POINTERS* pExp) {
    std::string error = "SE Exception: ";
    switch (u) {
    case 0xC0000005:
      error += "Access Violation";
      break;
    default:
      char result[11];
      sprintf_s(result, 11, "0x%08X", u);
      error += result;
    };
    throw std::exception(error.c_str());
  });

#ifdef ENABLE_RENDERDOC
  //
  RENDERDOC_API_1_1_2* rdoc_api = NULL;

#ifdef _WIN32
  // At init, on windows
  if (HMODULE mod = GetModuleHandleA("renderdoc.dll"))
  {
    pRENDERDOC_GetAPI RENDERDOC_GetAPI =
      (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
    int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&rdoc_api);
    assert(ret == 1);
  }
#else
#ifdef __linux__
  // At init, on linux/android.
  // For android replace librenderdoc.so with libVkLayer_GLES_RenderDoc.so
  if (void* mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD))
  {
    pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
    int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&rdoc_api);
    assert(ret == 1);
  }
#endif
#endif

  //
  if (rdoc_api) rdoc_api->SetCaptureOptionU32(eRENDERDOC_Option_DebugOutputMute, true);
#endif

  //
  ZNAMED::initialize();




  //
  //std::cout << "We running experimental renderer... continue?" << std::endl;
  //system("PAUSE");


  // first cherep
  decltype(auto) instance = ZNAMED::InstanceObj::make(ZNAMED::context, ZNAMED::InstanceCreateInfo{

  });

  // second cherep
  decltype(auto) device = ZNAMED::DeviceObj::make(instance, ZNAMED::DeviceCreateInfo{
    .physicalDeviceGroupIndex = 0u,
    .physicalDeviceIndex = 0u
  });

  //
  decltype(auto) memoryAllocatorVma = ZNAMED::MemoryAllocatorVma::make(device, ZNAMED::MemoryAllocatorCreateInfo{

  });

  // final cherep for today
  decltype(auto) descriptors = ZNAMED::DescriptorsObj::make(device.with(0u), ZNAMED::DescriptorsCreateInfo{

  });

  //
  decltype(auto) uniformData = UniformData{};

  //
  //rcCInfo.extUsed = std::make_shared<ZNAMED::EXIP>(ZNAMED::EXIP{ {ZNAMED::ExtensionInfoName::eMemoryAllocator, ZNAMED::ExtensionName::eMemoryAllocatorVma} });

  
  //
  decltype(auto) uploader = ZNAMED::UploaderObj::make(device, ZNAMED::UploaderCreateInfo{

  });

  //
  decltype(auto) samplers = descriptors->getSamplerDescriptors();

  

  //
  std::vector<glm::vec4> vertices{ 
    glm::vec4{-1.f, -1.f, -0.8f, 1.0}, glm::vec4{1.f, -1.f, -0.8f, 1.0}, glm::vec4{-1.f, 1.f, -0.8f, 1.0},
    glm::vec4{1.f, 1.f, -0.8f, 1.0}, glm::vec4{-1.f, 1.f, -0.8f, 1.0}, glm::vec4{1.f, -1.f, -0.8f, 1.0},
    //glm::vec4{0.f, 0.f, 0.1f, 1.0}, glm::vec4{1.f, 0.f, 0.1f, 1.0}, glm::vec4{0.f, 1.f, 0.1f, 1.0},
    //glm::vec4{1.f, 1.f, 0.1f, 1.0}, glm::vec4{0.f, 1.f, 0.1f, 1.0}, glm::vec4{1.f, 0.f, 0.1f, 1.0},
  };
  std::vector<uint16_t> indices{0u,1u,2u,3u,4u,5u};
  std::vector<glm::vec2> texcoords{
    glm::vec2{0.f, 0.f}, glm::vec2{1.f, 0.f}, glm::vec2{0.f, 1.f},
    glm::vec2{1.f, 1.f}, glm::vec2{0.f, 1.f}, glm::vec2{1.f, 0.f},
  };


  // 
  uintptr_t voffset = 0ull;
  uintptr_t ioffset = cpp21::bytesize(vertices) + voffset;
  uintptr_t toffset = cpp21::bytesize(indices) + ioffset;

  //
  decltype(auto) buffer = ZNAMED::ResourceObj::make(device, ZNAMED::ResourceCreateInfo{
    .descriptors = descriptors.as<vk::PipelineLayout>(),
    .bufferInfo = ZNAMED::BufferCreateInfo{
      .size = cpp21::bytesize(indices) + cpp21::bytesize(vertices) + cpp21::bytesize(texcoords),
      .type = ZNAMED::BufferType::eUniversal,
    }
  }.use(ZNAMED::ExtensionName::eMemoryAllocatorVma));

  //
  //memcpy(uploader->getUploadMapped(voffset), vertices.data(), cpp21::bytesize(vertices));
  //memcpy(uploader->getUploadMapped(ioffset), indices.data(), cpp21::bytesize(indices));
  //memcpy(uploader->getUploadMapped(toffset), texcoords.data(), cpp21::bytesize(texcoords));

  // complete loader
  uploader->executeUploadToResourceOnce(ZNAMED::UploadExecutionOnce{
    .host = cpp21::data_view<char8_t>((char8_t*)vertices.data(), 0ull, cpp21::bytesize(vertices)),
    .writeInfo = ZNAMED::UploadCommandWriteInfo{
      .dstBuffer = ZNAMED::BufferRegion{buffer.as<vk::Buffer>(), ZNAMED::DataRegion{voffset, sizeof(glm::vec4), cpp21::bytesize(vertices)}},
    }
  });

  //
  uploader->executeUploadToResourceOnce(ZNAMED::UploadExecutionOnce{
    .host = cpp21::data_view<char8_t>((char8_t*)indices.data(), 0ull, cpp21::bytesize(indices)),
    .writeInfo = ZNAMED::UploadCommandWriteInfo{
      .dstBuffer = ZNAMED::BufferRegion{buffer.as<vk::Buffer>(), ZNAMED::DataRegion{ioffset, sizeof(glm::u16vec3), cpp21::bytesize(indices)}},
    }
  });

  //
  uploader->executeUploadToResourceOnce(ZNAMED::UploadExecutionOnce{
    .host = cpp21::data_view<char8_t>((char8_t*)texcoords.data(), 0ull, cpp21::bytesize(texcoords)),
    .writeInfo = ZNAMED::UploadCommandWriteInfo{
      .dstBuffer = ZNAMED::BufferRegion{buffer.as<vk::Buffer>(), ZNAMED::DataRegion{toffset, sizeof(glm::vec2), cpp21::bytesize(texcoords)}},
    }
  });

  //
  uint64_t verticesAddress = buffer->getDeviceAddress() + voffset;
  uint64_t indicesAddress = buffer->getDeviceAddress() + ioffset;
  uint64_t texcoordsAddress = buffer->getDeviceAddress() + toffset;



  //
  decltype(auto) geomExt = ZNAMED::GeometryExtension{};
  geomExt.bufferViews[std::to_underlying(ZNAMED::BufferBind::eExtTexcoord)] = ZNAMED::BufferViewInfo{ .region = ZNAMED::BufferViewRegion{.deviceAddress = texcoordsAddress, .stride = sizeof(glm::vec2), .size = uint32_t(cpp21::bytesize(texcoords))}, .format = ZNAMED::BufferViewFormat::eFloat2 };
  std::vector<ZNAMED::GeometryExtension> extensions = { geomExt };

  //
  decltype(auto) extensionBuffer = ZNAMED::ResourceObj::make(device, ZNAMED::ResourceCreateInfo{
    .descriptors = descriptors.as<vk::PipelineLayout>(),
    .bufferInfo = ZNAMED::BufferCreateInfo{
      .size = cpp21::bytesize(extensions),
      .type = ZNAMED::BufferType::eUniversal,
    }
  }.use(ZNAMED::ExtensionName::eMemoryAllocatorVma));

  //
  uint64_t extensionAddress = extensionBuffer->getDeviceAddress();

  //
  uploader->executeUploadToResourceOnce(ZNAMED::UploadExecutionOnce{
    .host = cpp21::data_view<char8_t>((char8_t*)extensions.data(), 0ull, cpp21::bytesize(extensions)),
    .writeInfo = ZNAMED::UploadCommandWriteInfo{
      .dstBuffer = ZNAMED::BufferRegion{extensionBuffer.as<vk::Buffer>(), ZNAMED::DataRegion{0ull, sizeof(ZNAMED::GeometryExtension), cpp21::bytesize(extensions)}},
    }
  });





  // 
  int w = 0, h = 0, comp = 0;
  unsigned char* image = stbi_load("./texture.png", &w, &h, &comp, STBI_rgb_alpha);

  // 
  if (image == nullptr) { throw(std::string("Failed to load texture")); };

  //
  decltype(auto) texture = ZNAMED::ResourceObj::make(device, ZNAMED::ResourceCreateInfo{
    .descriptors = descriptors.as<vk::PipelineLayout>(),
    .imageInfo = ZNAMED::ImageCreateInfo{
      .format = vk::Format::eR8G8B8A8Unorm,
      .extent = vk::Extent3D{uint32_t(w), uint32_t(h), 1u},
      .type = ZNAMED::ImageType::eTexture
    }
  }.use(ZNAMED::ExtensionName::eMemoryAllocatorVma));

  //
  uploader->executeUploadToResourceOnce(ZNAMED::UploadExecutionOnce{
    .host = cpp21::data_view<char8_t>((char8_t*)image, 0ull, uint32_t(w) * uint32_t(h) * 4u),
    .writeInfo = ZNAMED::UploadCommandWriteInfo{
      .dstImage = ZNAMED::ImageRegion{.image = texture.as<vk::Image>(), .region = ZNAMED::ImageDataRegion{.extent = vk::Extent3D{uint32_t(w), uint32_t(h), 1u}}},
    }
  });

  //
  decltype(auto) texImageView = texture->createImageView(ZNAMED::ImageViewCreateInfo{.viewType = vk::ImageViewType::e2D});

  //
  decltype(auto) samplerObj = ZNAMED::SamplerObj::make(device, ZNAMED::SamplerCreateInfo{
    .descriptors = descriptors.as<vk::PipelineLayout>(),
    .native = vk::SamplerCreateInfo {
      .magFilter = vk::Filter::eLinear,
      .minFilter = vk::Filter::eLinear,
      .addressModeU = vk::SamplerAddressMode::eRepeat,
      .addressModeV = vk::SamplerAddressMode::eRepeat
    }
  });

  //
  decltype(auto) material = ZNAMED::MaterialInfo{};
  material.texCol[std::to_underlying(ZNAMED::TextureBind::eAlbedo)] = ZNAMED::TexOrDef{ .texture = ZNAMED::CTexture{.textureId = std::get<1u>(texImageView), .samplerId = samplerObj->getId() }};
  std::vector<ZNAMED::MaterialInfo> materials = { material };

  //
  decltype(auto) materialBuffer = ZNAMED::ResourceObj::make(device, ZNAMED::ResourceCreateInfo{
    .descriptors = descriptors.as<vk::PipelineLayout>(),
    .bufferInfo = ZNAMED::BufferCreateInfo{
      .size = cpp21::bytesize(materials),
      .type = ZNAMED::BufferType::eUniversal,
    }
    }.use(ZNAMED::ExtensionName::eMemoryAllocatorVma));

  //
  uint64_t materialAddress = materialBuffer->getDeviceAddress();

  //
  uploader->executeUploadToResourceOnce(ZNAMED::UploadExecutionOnce{
    .host = cpp21::data_view<char8_t>((char8_t*)materials.data(), 0ull, cpp21::bytesize(materials)),
    .writeInfo = ZNAMED::UploadCommandWriteInfo{
      .dstBuffer = ZNAMED::BufferRegion{materialBuffer.as<vk::Buffer>(), ZNAMED::DataRegion{0ull, sizeof(ZNAMED::MaterialInfo), cpp21::bytesize(materials)}},
    }
    });



  //
  decltype(auto) geometryLevel = ZNAMED::GeometryLevelObj::make(device, ZNAMED::GeometryLevelCreateInfo{
    .geometries = std::vector<ZNAMED::GeometryInfo>{ZNAMED::GeometryInfo{
      .vertices = ZNAMED::BufferViewInfo{.region = ZNAMED::BufferViewRegion{.deviceAddress = verticesAddress, .stride = sizeof(glm::vec4), .size = uint32_t(cpp21::bytesize(vertices))}, .format = ZNAMED::BufferViewFormat::eFloat3},
      .indices = ZNAMED::BufferViewInfo{.region = ZNAMED::BufferViewRegion{.deviceAddress = indicesAddress, .stride = sizeof(uint16_t), .size = uint32_t(cpp21::bytesize(indices))}, .format = ZNAMED::BufferViewFormat::eShort3},
      .extensionRef = extensionAddress,
      .materialRef = materialAddress,
      .primitiveCount = 2u
    }},
    .uploader = uploader.as<uintptr_t>(),
    });

  //
  decltype(auto) instanceLevel = ZNAMED::InstanceLevelObj::make(device, ZNAMED::InstanceLevelCreateInfo{
    .instances = std::vector<ZNAMED::InstanceDevInfo>{ZNAMED::InstanceDevInfo{
      .transform = reinterpret_cast<vk::TransformMatrixKHR&&>(glm::mat3x4(1.f)),
      .instanceCustomIndex = 0u,
      .mask = 0xFFu,
      .instanceShaderBindingTableRecordOffset = 0u,
      .flags = 0u,
      .accelerationStructureReference = geometryLevel.as<uintptr_t>()
    }},
    .uploader = uploader.as<uintptr_t>(),
    });

  //
  decltype(auto) instanceAddressBlock = ZNAMED::InstanceAddressBlock{
    .opaqueAddressInfo = instanceLevel->getAddressInfo()
  };



  //
  decltype(auto) gltfLoader = ZNAMED::GltfLoaderObj::make(device, ZNAMED::GltfLoaderCreateInfo{
    .uploader = uploader.as<uintptr_t>(),
    .descriptors = descriptors.as<vk::PipelineLayout>()
  });

  // 
  gltfLoader->load("./BoomBox.gltf");




  //
  decltype(auto) compute = ZNAMED::PipelineObj::make(device.with(0u), ZNAMED::PipelineCreateInfo{
    .layout = descriptors.as<vk::PipelineLayout>(),
    .compute = ZNAMED::ComputePipelineCreateInfo{
      .code = cpp21::readBinaryU32("./test.comp.spv")
    }
  });

  //
  robin_hood::unordered_map<vk::ShaderStageFlagBits, cpp21::shared_vector<uint32_t>> stageMaps = {};
  stageMaps[vk::ShaderStageFlagBits::eVertex] = cpp21::readBinaryU32("./opaque.vert.spv");
  stageMaps[vk::ShaderStageFlagBits::eFragment] = cpp21::readBinaryU32("./opaque.frag.spv");

  //
  decltype(auto) graphics = ZNAMED::PipelineObj::make(device.with(0u), ZNAMED::PipelineCreateInfo{
    .layout = descriptors.as<vk::PipelineLayout>(),
    .graphics = ZNAMED::GraphicsPipelineCreateInfo{
      .stageCodes = stageMaps
    }
  });

  

  //
  decltype(auto) qfAndQueue = ZNAMED::QueueGetInfo{ 0u, 0u };
  std::array<ZNAMED::FenceType, 4> fences = {};

  //
  glfwSetErrorCallback(error);
  glfwInit();

  // 
  if (GLFW_FALSE == glfwVulkanSupported()) {
    glfwTerminate(); return -1;
  };

  // 
  glfwDefaultWindowHints();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  //
  uint32_t WIDTH = 640u, HEIGHT = 360u;
  float xscale = 1.f, yscale = 1.f;
  GLFWmonitor* primary = glfwGetPrimaryMonitor();
  glfwGetMonitorContentScale(primary, &xscale, &yscale);
  uint32_t SC_WIDTH = WIDTH * xscale, SC_HEIGHT = HEIGHT * yscale;

  //
  vk::SurfaceKHR surface = {};
  std::string title = "ZEON.TEON.A";
  decltype(auto) window = glfwCreateWindow(SC_WIDTH, SC_HEIGHT, title.c_str(), nullptr, nullptr);
  glfwCreateWindowSurface(instance.as<VkInstance>(), window, nullptr, (VkSurfaceKHR*)&surface);

  //
  decltype(auto) swapchain = ZNAMED::SwapchainObj::make(device, ZNAMED::SwapchainCreateInfo{
    .layout = descriptors.as<vk::PipelineLayout>(),
    .surface = surface,
    .info = qfAndQueue
  });

  //
  decltype(auto) renderArea = swapchain->getRenderArea();
  

  // set perspective
  auto persp = glm::perspective(60.f / 180 * glm::pi<float>(), float(renderArea.extent.width) / float(renderArea.extent.height), 0.001f, 10000.f);
  auto lkat = glm::lookAt(glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
  uniformData.constants.perspective = glm::transpose(persp);
  uniformData.constants.perspectiveInverse = glm::transpose(glm::inverse(persp));
  uniformData.constants.lookAt = glm::mat3x4(glm::transpose(lkat));
  uniformData.constants.lookAtInverse = glm::mat3x4(glm::transpose(glm::inverse(lkat)));
  uniformData.extent = glm::uvec2(renderArea.extent.width, renderArea.extent.height);

  //
  decltype(auto) framebuffer = ZNAMED::FramebufferObj::make(device.with(0u), ZNAMED::FramebufferCreateInfo{
    .layout = descriptors.as<vk::PipelineLayout>(),
    .extent = renderArea.extent,
    .info = qfAndQueue
  });

  //
  decltype(auto) readySemaphoreInfos = swapchain->getReadySemaphoreInfos();
  decltype(auto) presentSemaphoreInfos = swapchain->getPresentSemaphoreInfos();

  // 
  decltype(auto) framebufferAttachments = framebuffer->getImageViewIndices();
  memcpy(uniformData.framebufferAttachments, framebufferAttachments.data(), std::min(framebufferAttachments.size(), 4ull) * sizeof(uint32_t));



  // 
  while (!glfwWindowShouldClose(window)) { // 
    glfwPollEvents();
    _CrtDumpMemoryLeaks();

    //
#ifdef ENABLE_RENDERDOC
    if (rdoc_api) rdoc_api->StartFrameCapture(NULL, NULL);
#endif

    // 
    decltype(auto) acquired = swapchain->acquireImage(qfAndQueue);

    // 
    decltype(auto) uniformFence = descriptors->executeUniformUpdateOnce(ZNAMED::UniformDataSet{
      .writeInfo = ZNAMED::UniformDataWriteSet{
        .region = ZNAMED::DataRegion{0ull, 4ull, sizeof(UniformData)},
        .data = cpp21::data_view<char8_t>((char8_t*)&uniformData, 0ull, sizeof(UniformData)),
      },
      .submission = ZNAMED::SubmissionInfo{
        .info = qfAndQueue,
      }
    });

    //
    decltype(auto) graphicsFence = graphics->executePipelineOnce(ZNAMED::ExecutePipelineInfo{
      .graphics = ZNAMED::WriteGraphicsInfo{
        .layout = descriptors.as<vk::PipelineLayout>(),
        .framebuffer = framebuffer.as<uintptr_t>(),
        .swapchain = swapchain.as<vk::SwapchainKHR>(),
        .instanceDraws = instanceLevel->getDrawInfo(),
        .instanceAddressBlock = instanceAddressBlock
      },
      .submission = ZNAMED::SubmissionInfo{
        .info = qfAndQueue,
      }
    });

    //
    decltype(auto) computeFence = compute->executePipelineOnce(ZNAMED::ExecutePipelineInfo{
      .compute = ZNAMED::WriteComputeInfo{
        .dispatch = vk::Extent3D{cpp21::tiled(renderArea.extent.width, 256u), renderArea.extent.height, 1u},
        .layout = descriptors.as<vk::PipelineLayout>(),
        .swapchain = swapchain.as<vk::SwapchainKHR>(),
        .instanceAddressBlock = instanceAddressBlock
      },
      .submission = ZNAMED::SubmissionInfo{
        .info = qfAndQueue
      }
    });

    //
    auto& fence = fences[acquired];
    if (fence) { decltype(auto) unleak = std::get<0u>(*fence); }; device->tickProcessing();
    fence = std::get<0u>(swapchain->presentImage(qfAndQueue));

    // stop the capture
#ifdef ENABLE_RENDERDOC
    if (rdoc_api) rdoc_api->EndFrameCapture(NULL, NULL);
#endif
  };

  // 
  return 0;
};

