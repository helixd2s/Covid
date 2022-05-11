#pragma once

//
#ifdef __cplusplus
#ifdef ALT_ENABLE_GLTF
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
#include "./Uploader.hpp"
#include "./Resource.hpp"
#include "./ResourceSparse.hpp"
#include "./Sampler.hpp"
#include "./PipelineLayout.hpp"
#include "./Framebuffer.hpp"
#include "./Pipeline.hpp"
#include "./Semaphore.hpp"
#include "./Swapchain.hpp"
#include "./PingPong.hpp"
#include "./GeometryLevel.hpp"
#include "./InstanceLevel.hpp"


// 
#ifdef ALT_ENABLE_VMA
#include "./MemoryAllocatorVma.hpp"
#include "./ResourceVma.hpp"
#endif

//
#ifdef ALT_ENABLE_GLTF
#include "./GltfLoader.hpp"
#endif

// 
namespace ANAMED {
  
};
#endif
