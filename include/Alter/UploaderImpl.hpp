#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Device.hpp"
#include "./ResourceBuffer.hpp"
#include "./ResourceImage.hpp"

//
namespace ANAMED {
  //
    inline WrapShared<UploaderObj> UploaderObj::writeUploadToResourceCmd(cpp21::optional_ref<UploadCommandWriteInfo> copyRegionInfo) {
        //decltype(auto) submission = CommandOnceSubmission{ .info = this->cInfo->info };
        decltype(auto) mappedBuffer = copyRegionInfo->bunchBuffer ? copyRegionInfo->bunchBuffer : this->mappedBuffer;
        decltype(auto) device = this->base.as<vk::Device>();
        decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
        decltype(auto) size = copyRegionInfo->dstBuffer ? copyRegionInfo->dstBuffer->region.size : VK_WHOLE_SIZE;
        decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
        decltype(auto) hostMapOffset = /*hostMapOffset_ ? (*hostMapOffset_) :*/ copyRegionInfo->bunchBuffer ? 0ull : copyRegionInfo->hostMapOffset;

        //
        decltype(auto) subresourceRange = vk::ImageSubresourceRange{};
        decltype(auto) subresourceLayers = vk::ImageSubresourceLayers{};

        // 
        decltype(auto) BtI = vk::CopyBufferToImageInfo2{};
        decltype(auto) BtB = vk::CopyBufferInfo2{};
        decltype(auto) BtBRegions = std::vector<vk::BufferCopy2>{  };
        decltype(auto) BtIRegions = std::vector<vk::BufferImageCopy2>{  };

        //
        decltype(auto) imageBarriersBegin = std::vector<vk::ImageMemoryBarrier2>{};
        decltype(auto) imageBarriersEnd = std::vector<vk::ImageMemoryBarrier2>{};

        //
        decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
          vk::BufferMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapWrite),
            .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapWrite),
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapRead),
            .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapRead),
            .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .buffer = mappedBuffer,
            .offset = hostMapOffset,
            .size = size
          }
        };

        //
        decltype(auto) bufferBarriersEnd = std::vector<vk::BufferMemoryBarrier2>{
          vk::BufferMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapRead),
            .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapRead),
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapWrite),
            .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapWrite),
            .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .buffer = mappedBuffer,
            .offset = hostMapOffset,
            .size = size
          }
        };

        // 
        if (copyRegionInfo->dstImage) {
            auto& imageRegion = copyRegionInfo->dstImage.value();

            //
            decltype(auto) imageObj = deviceObj->get<ResourceImageObj>(imageRegion.image);
            decltype(auto) imageInfo = imageObj->infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);

            // 
            subresourceRange = imageObj->subresourceRange(imageRegion.region.baseLayer, imageRegion.region.layerCount, imageRegion.region.baseMipLevel, 1u);
            subresourceLayers = vk::ImageSubresourceLayers{
              .aspectMask = subresourceRange.aspectMask,
              .mipLevel = subresourceRange.baseMipLevel,
              .baseArrayLayer = subresourceRange.baseArrayLayer,
              .layerCount = subresourceRange.layerCount
            };

            BtIRegions.push_back(vk::BufferImageCopy2{
              .bufferOffset = hostMapOffset, .imageSubresource = subresourceLayers, .imageOffset = imageRegion.region.offset, .imageExtent = imageRegion.region.extent
                });
            BtI = vk::CopyBufferToImageInfo2{ .srcBuffer = mappedBuffer, .dstImage = imageRegion.image, .dstImageLayout = vk::ImageLayout::eTransferDstOptimal };

            //
            decltype(auto) accessMask = vk::AccessFlagBits2(vku::getAccessMaskByImageUsage(imageObj->getImageUsage()));

            //
            imageBarriersBegin.push_back(vk::ImageMemoryBarrier2{
                .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask) | (accessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
                .srcAccessMask = vk::AccessFlagBits2(accessMask),
                .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
                .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
                .oldLayout = imageObj->cInfo->layout,
                .newLayout = vk::ImageLayout::eTransferDstOptimal,
                .srcQueueFamilyIndex = imageRegion.queueFamilyIndex,
                .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
                .image = imageRegion.image,
                .subresourceRange = subresourceRange
                });

            //
            imageBarriersEnd.push_back(vk::ImageMemoryBarrier2{
                .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
                .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
                .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask) | (accessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
                .dstAccessMask = vk::AccessFlagBits2(accessMask),
                .oldLayout = vk::ImageLayout::eTransferDstOptimal,
                .newLayout = imageObj->cInfo->layout,
                .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
                .dstQueueFamilyIndex = imageRegion.queueFamilyIndex,
                .image = imageRegion.image,
                .subresourceRange = subresourceRange
                });
        };

        // 
        if (copyRegionInfo->dstBuffer) {
            auto& bufferRegion = copyRegionInfo->dstBuffer.value();

            //
            decltype(auto) bufferObj = deviceObj->get<ResourceBufferObj>(bufferRegion.buffer);
            decltype(auto) accessMask = vk::AccessFlagBits2(vku::getAccessMaskByBufferUsage(bufferObj->getBufferUsage()));

            //
            BtBRegions.push_back(vk::BufferCopy2{ .srcOffset = hostMapOffset, .dstOffset = bufferRegion.region.offset, .size = size });
            BtB = vk::CopyBufferInfo2{ .srcBuffer = mappedBuffer, .dstBuffer = bufferRegion.buffer };

            //
            bufferBarriersBegin.push_back(vk::BufferMemoryBarrier2{
              .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask) | (accessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
              .srcAccessMask = accessMask,
              .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
              .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
              .srcQueueFamilyIndex = bufferRegion.queueFamilyIndex,
              .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
              .buffer = bufferRegion.buffer,
              .offset = bufferRegion.region.offset,
              .size = size
                });

            //
            bufferBarriersEnd.push_back(vk::BufferMemoryBarrier2{
              .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
              .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
              .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask) | (accessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
              .dstAccessMask = accessMask,
              .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
              .dstQueueFamilyIndex = bufferRegion.queueFamilyIndex,
              .buffer = bufferRegion.buffer,
              .offset = bufferRegion.region.offset,
              .size = size
                });
        };

        // 
        copyRegionInfo->cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersBegin).setImageMemoryBarriers(imageBarriersBegin));
        if (copyRegionInfo->dstImage) copyRegionInfo->cmdBuf.copyBufferToImage2(BtI.setRegions(BtIRegions));
        if (copyRegionInfo->dstBuffer) copyRegionInfo->cmdBuf.copyBuffer2(BtB.setRegions(BtBRegions));
        copyRegionInfo->cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersEnd).setImageMemoryBarriers(imageBarriersEnd));

        //
        return SFT();
    };

    //
    inline WrapShared<UploaderObj> UploaderObj::writeDownloadToResourceCmd(cpp21::optional_ref<DownloadCommandWriteInfo> info) {
        decltype(auto) mappedBuffer = info->bunchBuffer ? info->bunchBuffer : this->mappedBuffer;
        decltype(auto) device = this->base.as<vk::Device>();
        decltype(auto) regions = std::vector<vk::BufferCopy2>{  };
        decltype(auto) copyInfo = vk::CopyBufferInfo2{};
        decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
        decltype(auto) size = info->srcBuffer ? info->srcBuffer->region.size : VK_WHOLE_SIZE;
        decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
        decltype(auto) hostMapOffset = /*hostMapOffset_ ? (*hostMapOffset_) :*/ info->bunchBuffer ? 0ull : info->hostMapOffset;

        //
        decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
          vk::BufferMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapRead),
            .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapRead),
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapWrite),
            .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapWrite),
            .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .buffer = mappedBuffer,
            .offset = hostMapOffset,
            .size = size
          },
        };

        //
        decltype(auto) bufferBarriersEnd = std::vector<vk::BufferMemoryBarrier2>{
          vk::BufferMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapWrite),
            .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapWrite),
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapRead),
            .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapRead),
            .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .buffer = mappedBuffer,
            .offset = hostMapOffset,
            .size = size
          }
        };

        // 
        if (info->srcBuffer) {
            //
            decltype(auto) bufferObj = deviceObj->get<ResourceBufferObj>(info->srcBuffer->buffer);
            decltype(auto) accessMask = vk::AccessFlagBits2(vku::getAccessMaskByBufferUsage(bufferObj->getBufferUsage()));

            //
            copyInfo = vk::CopyBufferInfo2{ .srcBuffer = info->srcBuffer->buffer, .dstBuffer = mappedBuffer };
            regions.push_back(vk::BufferCopy2{ .srcOffset = info->srcBuffer->region.offset, .dstOffset = hostMapOffset, .size = size });

            //
            bufferBarriersBegin.push_back(vk::BufferMemoryBarrier2{
              .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask),
              .srcAccessMask = accessMask,
              .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
              .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
              .srcQueueFamilyIndex = info->srcBuffer->queueFamilyIndex,
              .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
              .buffer = info->srcBuffer->buffer,
              .offset = info->srcBuffer->region.offset,
              .size = size
                });

            //
            bufferBarriersEnd.push_back(vk::BufferMemoryBarrier2{
              .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
              .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
              .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask),
              .dstAccessMask = accessMask,
              .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
              .dstQueueFamilyIndex = info->srcBuffer->queueFamilyIndex,
              .buffer = info->srcBuffer->buffer,
              .offset = info->srcBuffer->region.offset,
              .size = size
                });
        };

        if (info->srcBuffer) {
            info->cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersBegin));
            info->cmdBuf.copyBuffer2(copyInfo.setRegions(regions));
            info->cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersEnd));
        };

        return SFT();
    };

    // 
    inline WrapShared<DeviceObj> DeviceObj::writeCopyBuffersCommand(CopyBufferWriteInfo const& copyInfoRaw) {
        //decltype(auto) submission = CommandOnceSubmission{ .info = QueueGetInfo {.queueFamilyIndex = copyInfoRaw.dst->queueFamilyIndex } };
        decltype(auto) device = this->base.as<vk::Device>();
        decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
        decltype(auto) size = std::min(copyInfoRaw.src->region.size, copyInfoRaw.dst->region.size);
        decltype(auto) copyInfo = vk::CopyBufferInfo2{ .srcBuffer = copyInfoRaw.src->buffer, .dstBuffer = copyInfoRaw.dst->buffer };
        decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
        decltype(auto) regions = std::vector<vk::BufferCopy2>{ vk::BufferCopy2{.srcOffset = copyInfoRaw.src->region.offset, .dstOffset = copyInfoRaw.dst->region.offset, .size = size} };

        //
        decltype(auto) srcAccessMask = vk::AccessFlagBits2(vku::getAccessMaskByBufferUsage(deviceObj->get<ResourceBufferObj>(copyInfoRaw.src->buffer)->getBufferUsage()));
        decltype(auto) dstAccessMask = vk::AccessFlagBits2(vku::getAccessMaskByBufferUsage(deviceObj->get<ResourceBufferObj>(copyInfoRaw.dst->buffer)->getBufferUsage()));

        //
        decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
          vk::BufferMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(srcAccessMask),
            .srcAccessMask = srcAccessMask,
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
            .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
            .srcQueueFamilyIndex = copyInfoRaw.src->queueFamilyIndex,
            .dstQueueFamilyIndex = copyInfoRaw.dst->queueFamilyIndex,
            .buffer = copyInfoRaw.src->buffer,
            .offset = copyInfoRaw.src->region.offset,
            .size = size
          },
          vk::BufferMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(dstAccessMask),
            .srcAccessMask = dstAccessMask,
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
            .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
            .srcQueueFamilyIndex = copyInfoRaw.src->queueFamilyIndex,
            .dstQueueFamilyIndex = copyInfoRaw.dst->queueFamilyIndex,
            .buffer = copyInfoRaw.dst->buffer,
            .offset = copyInfoRaw.dst->region.offset,
            .size = size
          }
        };

        //
        decltype(auto) bufferBarriersEnd = std::vector<vk::BufferMemoryBarrier2>{
          vk::BufferMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
            .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(srcAccessMask),
            .dstAccessMask = srcAccessMask,
            .srcQueueFamilyIndex = copyInfoRaw.dst->queueFamilyIndex,
            .dstQueueFamilyIndex = copyInfoRaw.src->queueFamilyIndex,
            .buffer = copyInfoRaw.src->buffer,
            .offset = copyInfoRaw.src->region.offset,
            .size = size
          },
          vk::BufferMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
            .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(dstAccessMask),
            .dstAccessMask = dstAccessMask,
            .srcQueueFamilyIndex = copyInfoRaw.dst->queueFamilyIndex,
            .dstQueueFamilyIndex = copyInfoRaw.src->queueFamilyIndex,
            .buffer = copyInfoRaw.dst->buffer,
            .offset = copyInfoRaw.dst->region.offset,
            .size = size
          }
        };

        // 
        //submission.commandInits.push_back([=](vk::CommandBuffer const& cmdBuf) {
        auto _copyInfo = copyInfo;
        auto _depInfo = depInfo;
        copyInfoRaw.cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersBegin));
        copyInfoRaw.cmdBuf.copyBuffer2(_copyInfo.setRegions(regions));
        copyInfoRaw.cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersEnd));
        //});

        //
        //return this->executeCommandOnce(submission);
        return this->SFT();
    };


};

#endif
