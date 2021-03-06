#pragma once

//
#ifdef __cplusplus
#ifdef ALT_ENABLE_GLTF
#include <tinygltf/tiny_gltf.h>
#include <tinygltf/stb_image.h>
#endif

// 
#include "./Core.hpp"
#include "./Context.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./MemoryAllocator.hpp"
#include "./QueueFamily.hpp"
#include "./PipelineLayout.hpp"
#include "./Uploader.hpp"
#include "./ResourceBuffer.hpp"
#include "./ResourceImage.hpp"
#include "./ResourceSparse.hpp"
#include "./UploaderImpl.hpp"
#include "./Sampler.hpp"
#include "./Framebuffer.hpp"
#include "./Pipeline.hpp"
#include "./Semaphore.hpp"
#include "./Swapchain.hpp"
#include "./PingPong.hpp"
#include "./GeometryLevel.hpp"
#include "./InstanceLevel.hpp"
//#include "./Denoiser.hpp"

// 
#ifdef ALT_ENABLE_VMA
#include "./MemoryAllocatorVma.hpp"
#endif

//
#ifdef ALT_ENABLE_GLTF
#include "./GltfLoader.hpp"
#endif

// 
namespace ANAMED {

};
#endif
