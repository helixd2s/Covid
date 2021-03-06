#pragma once
#ifdef __cplusplus
#define GLM_FORCE_SWIZZLE


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
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#ifdef ENABLE_RENDERDOC
#include "renderdoc_app.h"
#include <eh.h>
#endif

//
#include <Covid/Covid.hpp>
#include "lib/controller.hpp"

//
struct Constants
{
    glm::mat4x4 perspective = glm::mat4x4(1.f);
    glm::mat4x4 perspectiveInverse = glm::mat4x4(1.f);

    // TODO: make array of
    glm::mat3x4 lookAt[2] = { glm::mat3x4(1.f) };
    glm::mat3x4 lookAtInverse[2] = { glm::mat3x4(1.f) };
};

//
#pragma pack(push, 1)
struct UniformData {
    ANAMED::SwapchainStateInfo swapchain;
    ANAMED::PingPongStateInfo deferredBuf;
    ANAMED::PingPongStateInfo rasterBuf;
    ANAMED::FramebufferStateInfo framebuffers[2];
    ANAMED::FramebufferStateInfo dynamicRaster;
    Constants constants = {};
    glm::uvec2 rayCount = glm::uvec2(640, 360);

    //glm::uvec2 extent = {}; 
    uint32_t r0 = 0u;
    uint32_t frameCounter = 0u;
    uint32_t backgroundObj = 0u;
    uint32_t blueNoiseObj = 0u;
    uint64_t pixelData = 0ull;
    uint64_t writeData = 0ull;
    uint64_t rasterData[2] = { 0ull };
    uint64_t surfaceData = 0ull;
    //uint64_t hitData = 0ull;
};
#pragma pack(pop)

//
struct CounterData {
    uint32_t counters[4];
    uint32_t previousCounters[4];
};

//
struct PixelHitInfo {
    glm::uvec4 indices[2];
    glm::vec4 origin;
    glm::u16vec3 direct;
    glm::u16vec3 normal;
};

//
struct RayHitInfo {
    glm::uvec4 indices[2];
    glm::vec4 origin;
    glm::u16vec3 direct;
    glm::u16vec3 normal;
    glm::u16vec4 color;
};

//
// TODO: replace by image buffers
struct PixelSurfaceInfo {
    glm::u16vec4 tex[4];
    glm::uvec4 accum[3];
    glm::uvec4 color[3];
    glm::uvec4 flags = glm::uvec4(0u);
    glm::uvec4 prevf = glm::uvec4(0u);
};

//
struct RasterInfo {
    glm::uvec4 indices = glm::uvec4(0u); // indlude .W are pNext
    glm::vec4 barycentric = glm::vec4(0.f);
    glm::uvec4 derivatives = glm::uvec4(0.f); // f16[dUx, dUy], f16[dVx, dVy], f16[dWx, dWy], f16[dDx, dDy]
};

// 
class App {
protected:
    ANAMED::WrapShared<ANAMED::InstanceObj> instanceObj = {};
    ANAMED::WrapShared<ANAMED::DeviceObj> deviceObj = {};
    ANAMED::WrapShared<ANAMED::MemoryAllocatorVma> memoryAllocatorVma = {};
    ANAMED::WrapShared<ANAMED::PipelineLayoutObj> descriptorsObj = {};
    ANAMED::WrapShared<ANAMED::UploaderObj> uploaderObj = {};
    ANAMED::WrapShared<ANAMED::GltfLoaderObj> gltfLoaderObj = {};

    ANAMED::WrapShared<ANAMED::SwapchainObj> swapchainObj = {};
    ANAMED::WrapShared<ANAMED::FramebufferObj> nullFBO = {};
    ANAMED::WrapShared<ANAMED::FramebufferObj> framebufferObj[2] = {};
    ANAMED::WrapShared<ANAMED::PingPongObj> rasterBufObj = {};
    ANAMED::WrapShared<ANAMED::PingPongObj> deferredBufObj = {};
    ANAMED::WrapShared<ANAMED::ResourceImageObj> backgroundObj = {};
    ANAMED::WrapShared<ANAMED::ResourceImageObj> blueNoiseObj = {};
    ANAMED::WrapShared<ANAMED::ResourceBufferObj> hitDataObj = {};
    ANAMED::WrapShared<ANAMED::ResourceBufferObj> pixelDataObj = {};
    ANAMED::WrapShared<ANAMED::ResourceBufferObj> writeDataObj = {};
    ANAMED::WrapShared<ANAMED::ResourceBufferObj> rasterDataObj = {};
    ANAMED::WrapShared<ANAMED::ResourceBufferObj> surfaceDataObj = {};

    ANAMED::WrapShared<ANAMED::DenoiserObj> denoiserObj = {};

    //
    ANAMED::WrapShared<ANAMED::PipelineObj> overestimatedOpaqueObj = {};
    ANAMED::WrapShared<ANAMED::PipelineObj> overestimatedTranslucentObj = {};
    ANAMED::WrapShared<ANAMED::PipelineObj> underestimatedOpaqueObj = {};
    ANAMED::WrapShared<ANAMED::PipelineObj> underestimatedTranslucentObj = {};

    //
    ANAMED::WrapShared<ANAMED::PipelineObj> pathTracingObj = {};
    ANAMED::WrapShared<ANAMED::PipelineObj> finalObj = {};
    ANAMED::WrapShared<ANAMED::PipelineObj> combineObj = {};
    ANAMED::WrapShared<ANAMED::PipelineObj> reprojectionObj = {};
    ANAMED::WrapShared<ANAMED::PipelineObj> recopyObj = {};
    ANAMED::WrapShared<ANAMED::PipelineObj> resortObj = {};
    ANAMED::WrapShared<ANAMED::PipelineObj> controlObj = {};

    //
    UniformData uniformData = {};
    CounterData counterData = {};

    //
    ANAMED::InstanceAddressBlock instanceAddressBlock = {};
    ANAMED::QueueGetInfo qfAndQueue = ANAMED::QueueGetInfo{ 0u, 0u };
    vk::Rect2D renderArea = {};

    //
    glm::uvec2 nativeRasterSize = glm::uvec2(1280, 720);
    glm::uvec2 preRasterSize = glm::uvec2(1280, 720);
    glm::uvec2 rayCount = glm::uvec2(1280, 720);

    // CRITICAL!!!
    // GENERAL QUALITY OF FINAL IMAGE!!!
    // NOT RESPONSE FOR RAY-TRACE ITSELF!
    // RESPONSE FOR REPROJECTION!
    glm::uvec2 reprojectSize = glm::uvec2(1280, 720);

    //
    std::shared_ptr<std::array<ANAMED::FenceType, 8>> fences = {};
    std::shared_ptr<Controller> controller = {};
    std::shared_ptr<ANAMED::GltfModel> modelObj = {};
    std::shared_ptr<GLFWListener> listener = {};

    //
    GLFWwindow* window = nullptr;

    //
    double previousTime = glfwGetTime();
    uint32_t frameCount = 0;
    float scale = 1.f;

    //
    
    float xscale = 1.f, yscale = 1.f;

    //
public:
    App() {
        this->construct();
    };

    //
    virtual void tickProcessing() {
        deviceObj->tickProcessing();
    };

    //
    virtual void displayFPS(uint32_t& frameCount) {
        glfwSetWindowTitle(window, (std::string("Covid.TEON.A; FPS: ") + std::to_string(frameCount)).c_str()); frameCount = 0;
    };

    // 
    virtual std::experimental::generator<bool> renderGen() {
        co_yield false;

        // set perspective
        controller->handleFrame();
        auto persp = glm::perspective(60.f / 180 * glm::pi<float>(), float(renderArea.extent.width) / float(renderArea.extent.height), 0.002f, 20000.f);
        auto lkat = glm::lookAt(controller->viewPos, controller->viewCnt, controller->viewUp);
        uniformData.constants.perspective = glm::transpose(persp);
        uniformData.constants.perspectiveInverse = glm::transpose(glm::inverse(persp));
        uniformData.constants.lookAt[1] = cpp21::exchange(uniformData.constants.lookAt[0], glm::mat3x4(glm::transpose(lkat)));
        uniformData.constants.lookAtInverse[1] = cpp21::exchange(uniformData.constants.lookAtInverse[0], glm::mat3x4(glm::transpose(glm::inverse(lkat))));
        uniformData.frameCounter++;

        // 
        for (uint32_t i = 0; i < 2; i++) {
            //if (i == 2) { descriptorsObj->getCInfo().attachments[0].depthClearValue.depthStencil.depth = 0.f; };
            framebufferObj[i]->acquireImage(qfAndQueue);
            framebufferObj[i]->clearAttachments(qfAndQueue);
            //if (i == 2) { descriptorsObj->getCInfo().attachments[0].depthClearValue.depthStencil.depth = 1.f; };
        };

        // 
        decltype(auto) acquired = swapchainObj->acquireImage(qfAndQueue);
        decltype(auto) raster = rasterBufObj->acquireImage(qfAndQueue);
        decltype(auto) deferred = deferredBufObj->acquireImage(qfAndQueue);
        
        //
        for (uint32_t i = 0; i < 2; i++) {
            uniformData.framebuffers[i] = framebufferObj[i]->getStateInfo();
        };

        // still needs extent
        uniformData.dynamicRaster = nullFBO->getStateInfo();

        //
        uniformData.swapchain = swapchainObj->getStateInfo();
        uniformData.deferredBuf = deferredBufObj->getStateInfo();
        uniformData.rasterBuf = rasterBufObj->getStateInfo();

        // 
        rasterBufObj->clearImage(qfAndQueue, 0u, glm::uintBitsToFloat(glm::uvec4(0u)));
        rasterBufObj->clearImage(qfAndQueue, 1u, glm::vec4(1.f));

        //
        double currentTime = glfwGetTime();
        frameCount++;

        // 
        if (currentTime - previousTime >= 1.0)
        {
            // Display the frame count here any way you want.
            displayFPS(frameCount);
            previousTime = currentTime;
        };

        // wait ready for filling
        auto& fence = (*fences)[acquired];
        decltype(auto) status = false;
        if (fence) { while (!(status = fence->checkStatus())) { co_yield status; }; };

        // 
        decltype(auto) uniformFence = descriptorsObj->executeUniformUpdateOnce(ANAMED::UniformDataSet{
            // # yet another std::optional problem (implicit)
            .writeInfo = std::optional<ANAMED::UniformDataWriteSet>(ANAMED::UniformDataWriteSet{
              .region = ANAMED::DataRegion{0ull, 4ull, sizeof(UniformData)},
              .data = cpp21::data_view<char8_t>((char8_t*)&uniformData, 0ull, sizeof(UniformData)),
            }),
            .submission = ANAMED::SubmissionInfo{
              .info = qfAndQueue,
            }
        });

        //
        decltype(auto) controlFence                   = controlObj                  ->executePipelineOnce(ANAMED::ExecutePipelineInfo{ .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{.dispatch = vk::Extent3D{cpp21::tiled(uniformData.swapchain.extent.x, 32u), cpp21::tiled(uniformData.swapchain.extent.y, 4u), 1u}, .layout = descriptorsObj.as<vk::PipelineLayout>(), .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock) }), .submission = ANAMED::SubmissionInfo{.info = qfAndQueue } });
        decltype(auto) underestimatedOpaqueFence      = underestimatedOpaqueObj     ->executePipelineOnce(ANAMED::ExecutePipelineInfo{ .graphics = std::optional<ANAMED::WriteGraphicsInfo>(ANAMED::WriteGraphicsInfo{ .layout = descriptorsObj.as<vk::PipelineLayout>(), .framebuffer = framebufferObj[0].as<uintptr_t>(),  .instanceDraws = modelObj->getDefaultScene()->opaque->instanced->getDrawInfo(), .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock) }),.submission = ANAMED::SubmissionInfo{ .info = qfAndQueue, } });
        decltype(auto) underestimatedTranslucentFence = underestimatedTranslucentObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{ .graphics = std::optional<ANAMED::WriteGraphicsInfo>(ANAMED::WriteGraphicsInfo{ .layout = descriptorsObj.as<vk::PipelineLayout>(), .framebuffer = framebufferObj[1].as<uintptr_t>(),  .instanceDraws = modelObj->getDefaultScene()->translucent->instanced->getDrawInfo(), .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock) }),.submission = ANAMED::SubmissionInfo{.info = qfAndQueue, } });
        decltype(auto) overestimatedOpaqueFence       = overestimatedOpaqueObj      ->executePipelineOnce(ANAMED::ExecutePipelineInfo{ .graphics = std::optional<ANAMED::WriteGraphicsInfo>(ANAMED::WriteGraphicsInfo{ .layout = descriptorsObj.as<vk::PipelineLayout>(), .framebuffer = nullFBO.as<uintptr_t>(),  .instanceDraws = modelObj->getDefaultScene()->opaque->instanced->getDrawInfo(), .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock) }),.submission = ANAMED::SubmissionInfo{.info = qfAndQueue, } });
        decltype(auto) overestimatedTranslucentFence  = overestimatedTranslucentObj ->executePipelineOnce(ANAMED::ExecutePipelineInfo{ .graphics = std::optional<ANAMED::WriteGraphicsInfo>(ANAMED::WriteGraphicsInfo{ .layout = descriptorsObj.as<vk::PipelineLayout>(), .framebuffer = nullFBO.as<uintptr_t>(),  .instanceDraws = modelObj->getDefaultScene()->translucent->instanced->getDrawInfo(), .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock) }),.submission = ANAMED::SubmissionInfo{.info = qfAndQueue, } });
        decltype(auto) resortFence                    = resortObj                   ->executePipelineOnce(ANAMED::ExecutePipelineInfo{ .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{.dispatch = vk::Extent3D{cpp21::tiled(preRasterSize.x, 32u), cpp21::tiled(preRasterSize.y, 4u), 1u}, .layout = descriptorsObj.as<vk::PipelineLayout>(), .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock) }), .submission = ANAMED::SubmissionInfo{.info = qfAndQueue } });

        // 
        decltype(auto) reprojectionFence = reprojectionObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{ .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{.dispatch = vk::Extent3D{cpp21::tiled(reprojectSize.x, 32u), cpp21::tiled(reprojectSize.y, 4u), 1u}, .layout = descriptorsObj.as<vk::PipelineLayout>(), .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock) }), .submission = ANAMED::SubmissionInfo{.info = qfAndQueue } });
        decltype(auto) pathTracingFence  = pathTracingObj ->executePipelineOnce(ANAMED::ExecutePipelineInfo{ .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{ .dispatch = vk::Extent3D{cpp21::tiled(rayCount.x, 32u), cpp21::tiled(rayCount.y, 4u), 1u}, .layout = descriptorsObj.as<vk::PipelineLayout>(), .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock) }), .submission = ANAMED::SubmissionInfo{ .info = qfAndQueue } });
        decltype(auto) recopyFence       = recopyObj      ->executePipelineOnce(ANAMED::ExecutePipelineInfo{ .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{.dispatch = vk::Extent3D{cpp21::tiled(reprojectSize.x, 32u), cpp21::tiled(reprojectSize.y, 4u), 1u}, .layout = descriptorsObj.as<vk::PipelineLayout>(), .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock) }), .submission = ANAMED::SubmissionInfo{.info = qfAndQueue } });
        decltype(auto) combineFence      = combineObj     ->executePipelineOnce(ANAMED::ExecutePipelineInfo{ .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{.dispatch = vk::Extent3D{cpp21::tiled(reprojectSize.x, 32u), cpp21::tiled(reprojectSize.y, 4u), 1u}, .layout = descriptorsObj.as<vk::PipelineLayout>(), .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock) }), .submission = ANAMED::SubmissionInfo{.info = qfAndQueue } });
        decltype(auto) finalFence        = finalObj       ->executePipelineOnce(ANAMED::ExecutePipelineInfo{ .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{.dispatch = vk::Extent3D{cpp21::tiled(uniformData.swapchain.extent.x, 32u), cpp21::tiled(uniformData.swapchain.extent.y, 4u), 1u}, .layout = descriptorsObj.as<vk::PipelineLayout>(), .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock) }), .submission = ANAMED::SubmissionInfo{.info = qfAndQueue } });

        //
        deferredBufObj->presentImage(qfAndQueue);
        rasterBufObj->presentImage(qfAndQueue);
        fence = std::get<0u>(swapchainObj->presentImage(qfAndQueue));

        //
        std::swap(uniformData.rasterData[0], uniformData.rasterData[1]);

        // 
        co_yield true;
    };

    //
    virtual void initSurface(GLFWwindow* window) {
        // 
        this->controller = std::make_shared<Controller>(this->listener = std::make_shared<GLFWListener>(this->window = window));

        //
        vk::SurfaceKHR surface = {};
        glfwCreateWindowSurface(instanceObj.as<VkInstance>(), window, nullptr, (VkSurfaceKHR*)&surface);

        //
        swapchainObj = ANAMED::SwapchainObj::make(deviceObj, ANAMED::SwapchainCreateInfo{
          .layout = descriptorsObj.as<vk::PipelineLayout>(),
          .surface = surface,
          .info = qfAndQueue
            });

        //
        uniformData.swapchain = swapchainObj->getStateInfo();

        //
        glfwGetWindowContentScale(window, &xscale, &yscale);

        //
        renderArea = swapchainObj->getRenderArea();

        // two important spells
        //nativeRasterSize = glm::uvec2(float(renderArea.extent.width) / xscale * 1.f, float(renderArea.extent.height) / yscale * 1.f);
        //preRasterSize = glm::uvec2(float(renderArea.extent.width) / xscale * 1.f, float(renderArea.extent.height) / yscale * 1.f);
        reprojectSize = glm::uvec2(float(renderArea.extent.width) / xscale * 2.f, float(renderArea.extent.height) / yscale * 2.f);
        rayCount = glm::uvec2(float(renderArea.extent.width) / xscale * 2.f, float(renderArea.extent.height) / yscale * 2.f);

        // 
        surfaceDataObj = ANAMED::ResourceBufferObj::make(deviceObj, ANAMED::BufferCreateInfo{
            .descriptors = descriptorsObj.as<vk::PipelineLayout>(),
            .size = sizeof(PixelSurfaceInfo) * reprojectSize.x * reprojectSize.y,
            .type = ANAMED::BufferType::eStorage,
        });

        // 
        pixelDataObj = ANAMED::ResourceBufferObj::make(deviceObj, ANAMED::BufferCreateInfo{
            .descriptors = descriptorsObj.as<vk::PipelineLayout>(),
            .size = sizeof(PixelHitInfo) * reprojectSize.x * reprojectSize.y * 3u,
            .type = ANAMED::BufferType::eStorage
        });

        // 
        writeDataObj = ANAMED::ResourceBufferObj::make(deviceObj, ANAMED::BufferCreateInfo{
            .descriptors = descriptorsObj.as<vk::PipelineLayout>(),
            .size = sizeof(PixelHitInfo) * reprojectSize.x * reprojectSize.y * 3u,
            .type = ANAMED::BufferType::eStorage
         });

        //
        uniformData.rayCount = rayCount;
        uniformData.surfaceData = surfaceDataObj->getDeviceAddress();
        uniformData.pixelData = pixelDataObj->getDeviceAddress();
        uniformData.writeData = writeDataObj->getDeviceAddress();
        //uniformData.hitData = hitDataObj->getDeviceAddress();

        //
        for (uint32_t i = 0; i < 2; i++) {
            framebufferObj[i] = ANAMED::FramebufferObj::make(deviceObj.with(0u), ANAMED::FramebufferCreateInfo{
              .layout = descriptorsObj.as<vk::PipelineLayout>(),
              .extent = vk::Extent2D{nativeRasterSize.x, nativeRasterSize.y},
              .info = qfAndQueue
                });
            uniformData.framebuffers[i] = framebufferObj[i]->getStateInfo();
        };

        //
        nullFBO = ANAMED::FramebufferObj::make(deviceObj.with(0u), ANAMED::FramebufferCreateInfo{
            .layout = descriptorsObj.as<vk::PipelineLayout>(),
            .extent = vk::Extent2D{preRasterSize.x, preRasterSize.y},
            .info = qfAndQueue,
            .attachmentLayout = ANAMED::nullAttachmentLayout
        });

        // 
        rasterDataObj = ANAMED::ResourceBufferObj::make(deviceObj, ANAMED::BufferCreateInfo{
            .descriptors = descriptorsObj.as<vk::PipelineLayout>(),
            .size = sizeof(RasterInfo) * preRasterSize.x * preRasterSize.y * 32u,
            .type = ANAMED::BufferType::eStorage
        });

        //
        uniformData.rasterData[0] = rasterDataObj->getDeviceAddress();
        uniformData.rasterData[1] = uniformData.rasterData[0] + sizeof(RasterInfo) * preRasterSize.x * preRasterSize.y * 16u;

        // now, you understand why?
        rasterBufObj = ANAMED::PingPongObj::make(deviceObj.with(0u), ANAMED::PingPongCreateInfo{
          .layout = descriptorsObj.as<vk::PipelineLayout>(),
          .extent = vk::Extent2D{preRasterSize.x, preRasterSize.y},
          .minImageCount = 2u,
          .split = std::vector<uint32_t>{ 1, 1 },
          .formats = std::vector<vk::Format>{ vk::Format::eR32Uint, vk::Format::eR32Sfloat },
          .info = qfAndQueue
            });

        //
        deferredBufObj = ANAMED::PingPongObj::make(deviceObj.with(0u), ANAMED::PingPongCreateInfo{
          .layout = descriptorsObj.as<vk::PipelineLayout>(),
          .extent = vk::Extent2D{reprojectSize.x, reprojectSize.y},
          .minImageCount = 2u,
          .split = std::vector<uint32_t>{ 1, 1, 1, 1, 1, 1 },
          .formats = std::vector<vk::Format>{ vk::Format::eR16G16B16A16Sfloat, vk::Format::eR16G16B16A16Sfloat, vk::Format::eR16G16B16A16Sfloat, vk::Format::eR16G16B16A16Sfloat, vk::Format::eR16G16B16A16Sfloat, vk::Format::eR32G32B32A32Sfloat },
          .info = qfAndQueue
            });

        //
        uniformData.frameCounter = 0u;
        uniformData.deferredBuf = deferredBufObj->getStateInfo();
        uniformData.rasterBuf = rasterBufObj->getStateInfo();

        //
        //decltype(auto) swch = deferredBufObj->getCurrentSet();

        

    };

    //
    void loadModel(std::string model, float scale = 1.f) {
        instanceAddressBlock = (modelObj = gltfLoaderObj->load(model, this->scale = scale))->getDefaultScene()->addressBlock;
    };

protected:

    // 
    virtual void construct() {
        uniformData = UniformData{};

        // 
        instanceObj = ANAMED::InstanceObj::make(ANAMED::context, ANAMED::InstanceCreateInfo{

            });

        // 
        deviceObj = ANAMED::DeviceObj::make(instanceObj, ANAMED::DeviceCreateInfo{
          .physicalDeviceGroupIndex = 0u,
          .physicalDeviceIndex = 0u
            });

        //
        memoryAllocatorVma = ANAMED::MemoryAllocatorVma::make(deviceObj, ANAMED::MemoryAllocatorCreateInfo{

            });

        // 
        descriptorsObj = ANAMED::PipelineLayoutObj::make(deviceObj.with(0u), ANAMED::PipelineLayoutCreateInfo{

            });

        //
        uploaderObj = ANAMED::UploaderObj::make(deviceObj, ANAMED::UploaderCreateInfo{

            }.use(ANAMED::ExtensionName::eMemoryAllocatorVma));



        //
        std::unordered_map<vk::ShaderStageFlagBits, cpp21::shared_vector<uint32_t>> overestimatedOpaqueStageMaps = {};
        overestimatedOpaqueStageMaps[vk::ShaderStageFlagBits::eVertex] = cpp21::readBinaryU32("./shaders/triangles.vert.spv");
        overestimatedOpaqueStageMaps[vk::ShaderStageFlagBits::eGeometry] = cpp21::readBinaryU32("./shaders/triangles-opaque.geom.spv");
        overestimatedOpaqueStageMaps[vk::ShaderStageFlagBits::eFragment] = cpp21::readBinaryU32("./shaders/overestimate-opaque.frag.spv");
        overestimatedOpaqueObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
          .layout = descriptorsObj.as<vk::PipelineLayout>(),
          .graphics = ANAMED::GraphicsPipelineCreateInfo{
            .stageCodes = overestimatedOpaqueStageMaps,
            .attachmentLayout = ANAMED::nullAttachmentLayout,
            .hasConservativeRaster = true,
            .dynamicState = std::make_shared<ANAMED::GraphicsDynamicState>(ANAMED::GraphicsDynamicState{
              .hasDepthTest = false,
              .hasDepthWrite = false,
              .reversalDepth = false
            })
          }
        });

        //
        std::unordered_map<vk::ShaderStageFlagBits, cpp21::shared_vector<uint32_t>> overestimatedTranslucentStageMaps = {};
        overestimatedTranslucentStageMaps[vk::ShaderStageFlagBits::eVertex] = cpp21::readBinaryU32("./shaders/triangles.vert.spv");
        overestimatedTranslucentStageMaps[vk::ShaderStageFlagBits::eGeometry] = cpp21::readBinaryU32("./shaders/triangles-translucent.geom.spv");
        overestimatedTranslucentStageMaps[vk::ShaderStageFlagBits::eFragment] = cpp21::readBinaryU32("./shaders/overestimate-translucent.frag.spv");
        overestimatedTranslucentObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
          .layout = descriptorsObj.as<vk::PipelineLayout>(),
          .graphics = ANAMED::GraphicsPipelineCreateInfo{
            .stageCodes = overestimatedTranslucentStageMaps,
            .attachmentLayout = ANAMED::nullAttachmentLayout,
            .hasConservativeRaster = true,
            .dynamicState = std::make_shared<ANAMED::GraphicsDynamicState>(ANAMED::GraphicsDynamicState{
              .hasDepthTest = false,
              .hasDepthWrite = false,
              .reversalDepth = false
            })
          }
        });


        //
        std::unordered_map<vk::ShaderStageFlagBits, cpp21::shared_vector<uint32_t>> underestimatedOpaqueStageMaps = {};
        underestimatedOpaqueStageMaps[vk::ShaderStageFlagBits::eVertex] = cpp21::readBinaryU32("./shaders/triangles.vert.spv");
        underestimatedOpaqueStageMaps[vk::ShaderStageFlagBits::eGeometry] = cpp21::readBinaryU32("./shaders/triangles-opaque.geom.spv");
        underestimatedOpaqueStageMaps[vk::ShaderStageFlagBits::eFragment] = cpp21::readBinaryU32("./shaders/underestimate-opaque.frag.spv");
        underestimatedOpaqueObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
          .layout = descriptorsObj.as<vk::PipelineLayout>(),
          .graphics = ANAMED::GraphicsPipelineCreateInfo{
            .stageCodes = underestimatedOpaqueStageMaps,
            .hasConservativeRaster = true,
            .underestimated = true,
            .dynamicState = std::make_shared<ANAMED::GraphicsDynamicState>(ANAMED::GraphicsDynamicState{
              .hasDepthTest = true,
              .hasDepthWrite = true,
              .reversalDepth = false
            })
          }
        });

        //
        std::unordered_map<vk::ShaderStageFlagBits, cpp21::shared_vector<uint32_t>> underestimatedTranslucentStageMaps = {};
        underestimatedTranslucentStageMaps[vk::ShaderStageFlagBits::eVertex] = cpp21::readBinaryU32("./shaders/triangles.vert.spv");
        underestimatedTranslucentStageMaps[vk::ShaderStageFlagBits::eGeometry] = cpp21::readBinaryU32("./shaders/triangles-translucent.geom.spv");
        underestimatedTranslucentStageMaps[vk::ShaderStageFlagBits::eFragment] = cpp21::readBinaryU32("./shaders/underestimate-translucent.frag.spv");
        underestimatedTranslucentObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
          .layout = descriptorsObj.as<vk::PipelineLayout>(),
          .graphics = ANAMED::GraphicsPipelineCreateInfo{
            .stageCodes = underestimatedTranslucentStageMaps,
            .hasConservativeRaster = true,
            .underestimated = true,
            .dynamicState = std::make_shared<ANAMED::GraphicsDynamicState>(ANAMED::GraphicsDynamicState{
              .hasDepthTest = true,
              .hasDepthWrite = true,
              .reversalDepth = false
            })
          }
        });

        //
        controlObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
          .layout = descriptorsObj.as<vk::PipelineLayout>(),
          .compute = ANAMED::ComputePipelineCreateInfo{
            .code = cpp21::readBinaryU32("./shaders/control.comp.spv")
          }
        });

        //
        pathTracingObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
          .layout = descriptorsObj.as<vk::PipelineLayout>(),
          .compute = ANAMED::ComputePipelineCreateInfo{
            .code = cpp21::readBinaryU32("./shaders/path-tracing.comp.spv")
          }
        });

        //
        finalObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
          .layout = descriptorsObj.as<vk::PipelineLayout>(),
          .compute = ANAMED::ComputePipelineCreateInfo{
            .code = cpp21::readBinaryU32("./shaders/final.comp.spv")
          }
        });

        //
        combineObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
          .layout = descriptorsObj.as<vk::PipelineLayout>(),
          .compute = ANAMED::ComputePipelineCreateInfo{
            .code = cpp21::readBinaryU32("./shaders/combine.comp.spv")
          }
        });

        //
        reprojectionObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
          .layout = descriptorsObj.as<vk::PipelineLayout>(),
          .compute = ANAMED::ComputePipelineCreateInfo{
            .code = cpp21::readBinaryU32("./shaders/reprojection.comp.spv")
          }
        });

        //
        recopyObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
          .layout = descriptorsObj.as<vk::PipelineLayout>(),
          .compute = ANAMED::ComputePipelineCreateInfo{
            .code = cpp21::readBinaryU32("./shaders/recopy.comp.spv")
          }
        });

        //
        resortObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
          .layout = descriptorsObj.as<vk::PipelineLayout>(),
          .compute = ANAMED::ComputePipelineCreateInfo{
            .code = cpp21::readBinaryU32("./shaders/resort.comp.spv")
          }
        });

        //
        gltfLoaderObj = ANAMED::GltfLoaderObj::make(deviceObj, ANAMED::GltfLoaderCreateInfo{
          .uploader = uploaderObj.as<uintptr_t>(),
          .descriptors = descriptorsObj.as<vk::PipelineLayout>()
            });

        //
        qfAndQueue = ANAMED::QueueGetInfo{ 0u, 0u };
        fences = std::make_shared<std::array<ANAMED::FenceType, 8>>();

        //
        previousTime = glfwGetTime();
        frameCount = 0;

        //
        {
            stbi_ldr_to_hdr_scale(1.0f);
            stbi_ldr_to_hdr_gamma(2.2f);

            //
            int w = 0, h = 0, c = 0;
            float* data = (float*)stbi_loadf("./background.hdr", &w, &h, &c, STBI_rgb_alpha);

            //
            backgroundObj = ANAMED::ResourceImageObj::make(deviceObj, ANAMED::ImageCreateInfo{
                .descriptors = descriptorsObj.as<vk::PipelineLayout>(),
                .format = vk::Format::eR32G32B32A32Sfloat,
                .extent = vk::Extent3D{uint32_t(w), uint32_t(h), 1u},
                .type = ANAMED::ImageType::eTexture
            });

            //
            decltype(auto) pair = backgroundObj->createImageView(ANAMED::ImageViewCreateInfo{
              .viewType = vk::ImageViewType::e2D
                });


            // complete loader
            //
            decltype(auto) status = uploaderObj->executeUploadToResourceOnce(ANAMED::UploadExecutionOnce{
              .host = cpp21::data_view<char8_t>((char8_t*)data, 0ull, h * w * 16ull),
              .writeInfo = ANAMED::UploadCommandWriteInfo{
                    // # yet another std::optional problem (implicit)
                    .dstImage = std::optional<ANAMED::ImageRegion>(ANAMED::ImageRegion{.image = backgroundObj.as<vk::Image>(), .region = ANAMED::ImageDataRegion{.extent = vk::Extent3D{uint32_t(w), uint32_t(h), 1u}}}),
                  }
                });

            //
            descriptorsObj->updateDescriptors();

            //
            uniformData.backgroundObj = pair.indice;
        }

        //
        {
            //
            int w = 0, h = 0, c = 0;
            float* data = (float*)stbi_loadf("./BlueNoise470.png", &w, &h, &c, STBI_rgb_alpha);

            //
            blueNoiseObj = ANAMED::ResourceImageObj::make(deviceObj, ANAMED::ImageCreateInfo{
                .descriptors = descriptorsObj.as<vk::PipelineLayout>(),
                .format = vk::Format::eR8G8B8A8Uint,
                .extent = vk::Extent3D{uint32_t(w), uint32_t(h), 1u},
                .type = ANAMED::ImageType::eTexture
            });

            //
            decltype(auto) pair = blueNoiseObj->createImageView(ANAMED::ImageViewCreateInfo{
              .viewType = vk::ImageViewType::e2D
                });


            // complete loader
            //
            decltype(auto) status = uploaderObj->executeUploadToResourceOnce(ANAMED::UploadExecutionOnce{
              .host = cpp21::data_view<char8_t>((char8_t*)data, 0ull, h * w * 4ull),
              .writeInfo = ANAMED::UploadCommandWriteInfo{
                    // # yet another std::optional problem (implicit)
                    .dstImage = std::optional<ANAMED::ImageRegion>(ANAMED::ImageRegion{.image = blueNoiseObj.as<vk::Image>(), .region = ANAMED::ImageDataRegion{.extent = vk::Extent3D{uint32_t(w), uint32_t(h), 1u}}}),
                  }
                });

            //
            descriptorsObj->updateDescriptors();

            //
            uniformData.blueNoiseObj = pair.indice;
        };
    };

};

#endif
