#pragma once

//
#ifdef __cplusplus
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
#include "./ResourceSparse.hpp"
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
#ifdef Z_ENABLE_GLTF
#include "./GltfLoader.hpp"
#endif

// 
namespace ZNAMED {
  
};
#endif
