#include "group.hpp"
#include "device.hpp"
#include "buffer.hpp"
#include "texture.hpp"

vfx::ResourceGroup::ResourceGroup() {}

vfx::ResourceGroup::~ResourceGroup() {
    device->freeResourceGroup(this);
}

void vfx::ResourceGroup::setBuffer(const Arc<Buffer>& buffer, u64 offset, u32 binding) {
    auto descriptor_buffer_info = vk::DescriptorBufferInfo {
        .buffer = buffer->handle,
        .offset = offset,
        .range = VK_WHOLE_SIZE
    };
    auto write_descriptor_set = vk::WriteDescriptorSet{
        .dstSet = set,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo = &descriptor_buffer_info
    };
    device->handle->updateDescriptorSets(1, &write_descriptor_set, 0, nullptr);
}

void vfx::ResourceGroup::setTexture(const Arc<Texture>& texture, u32 binding) {
    auto descriptor_image_info = vk::DescriptorImageInfo{
        .imageView = texture->view,
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
    };
    auto write_descriptor_set = vk::WriteDescriptorSet{
        .dstSet = set,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eSampledImage,
        .pImageInfo = &descriptor_image_info
    };
    device->handle->updateDescriptorSets(1, &write_descriptor_set, 0, nullptr);
}

void vfx::ResourceGroup::setSampler(const Arc<Sampler>& sampler, u32 binding) {
    auto descriptor_image_info = vk::DescriptorImageInfo{
        .sampler = sampler->handle
    };
    auto write_descriptor_set = vk::WriteDescriptorSet{
        .dstSet = set,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eSampler,
        .pImageInfo = &descriptor_image_info
    };
    device->handle->updateDescriptorSets(1, &write_descriptor_set, 0, nullptr);
}

void vfx::ResourceGroup::setLabel(const std::string& name) {
    vk::DebugMarkerObjectNameInfoEXT pool_info = {};
    pool_info.setObjectType(vk::DebugReportObjectTypeEXT::eDescriptorPool);
    pool_info.setObject(u64(VkDescriptorPool(pool)));
    pool_info.setPObjectName(name.c_str());

    device->handle->debugMarkerSetObjectNameEXT(pool_info);

    vk::DebugMarkerObjectNameInfoEXT set_info = {};
    set_info.setObjectType(vk::DebugReportObjectTypeEXT::eDescriptorSet);
    set_info.setObject(u64(VkDescriptorSet(set)));
    set_info.setPObjectName(name.c_str());

    device->handle->debugMarkerSetObjectNameEXT(pool_info);
}
