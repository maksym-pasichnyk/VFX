#include "Device.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "Sampler.hpp"
#include "Instance.hpp"
#include "DescriptorSet.hpp"

gfx::DescriptorSet::DescriptorSet(ManagedShared<Device> device, vk::DescriptorSet raw, vk::DescriptorPool pool) : device(std::move(device)), raw(raw), pool(pool) {}
gfx::DescriptorSet::~DescriptorSet() {
    device->raii.raw.destroyDescriptorPool(pool, nullptr, device->raii.dispatcher);
}

void gfx::DescriptorSet::setBuffer(const Buffer& buffer, uint64_t offset, uint32_t binding) {
    vk::DescriptorBufferInfo descriptor_buffer_info = {};
    descriptor_buffer_info.setBuffer(buffer.raw);
    descriptor_buffer_info.setOffset(offset);
    descriptor_buffer_info.setRange(VK_WHOLE_SIZE);

    vk::WriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.setDstSet(raw);
    write_descriptor_set.setDstBinding(binding);
    write_descriptor_set.setDstArrayElement(0);
    write_descriptor_set.setDescriptorCount(1);
    write_descriptor_set.setDescriptorType(vk::DescriptorType::eUniformBuffer);
    write_descriptor_set.setPBufferInfo(&descriptor_buffer_info);

    device->raii.raw.updateDescriptorSets(1, &write_descriptor_set, 0, nullptr, device->raii.dispatcher);
}

void gfx::DescriptorSet::setStorageBuffer(const Buffer& buffer, uint64_t offset, uint32_t binding) {
    vk::DescriptorBufferInfo descriptor_buffer_info = {};
    descriptor_buffer_info.setBuffer(buffer.raw);
    descriptor_buffer_info.setOffset(offset);
    descriptor_buffer_info.setRange(VK_WHOLE_SIZE);

    vk::WriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.setDstSet(raw);
    write_descriptor_set.setDstBinding(binding);
    write_descriptor_set.setDstArrayElement(0);
    write_descriptor_set.setDescriptorCount(1);
    write_descriptor_set.setDescriptorType(vk::DescriptorType::eStorageBuffer);
    write_descriptor_set.setPBufferInfo(&descriptor_buffer_info);

    device->raii.raw.updateDescriptorSets(1, &write_descriptor_set, 0, nullptr, device->raii.dispatcher);
}

void gfx::DescriptorSet::setTexture(const Texture& texture, uint32_t binding) {
    vk::DescriptorImageInfo descriptor_image_info = {};
    descriptor_image_info.setImageView(texture.image_view);
    descriptor_image_info.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    vk::WriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.setDstSet(raw);
    write_descriptor_set.setDstBinding(binding);
    write_descriptor_set.setDstArrayElement(0);
    write_descriptor_set.setDescriptorCount(1);
    write_descriptor_set.setDescriptorType(vk::DescriptorType::eSampledImage);
    write_descriptor_set.setPImageInfo(&descriptor_image_info);

    device->raii.raw.updateDescriptorSets(1, &write_descriptor_set, 0, nullptr, device->raii.dispatcher);
}

void gfx::DescriptorSet::setSampler(Sampler const& sampler, uint32_t binding) {
    vk::DescriptorImageInfo descriptor_image_info = {};
    descriptor_image_info.setSampler(sampler.raw);

    vk::WriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.setDstSet(raw);
    write_descriptor_set.setDstBinding(binding);
    write_descriptor_set.setDstArrayElement(0);
    write_descriptor_set.setDescriptorCount(1);
    write_descriptor_set.setDescriptorType(vk::DescriptorType::eSampler);
    write_descriptor_set.setPImageInfo(&descriptor_image_info);

    device->raii.raw.updateDescriptorSets(1, &write_descriptor_set, 0, nullptr, device->raii.dispatcher);
}

void gfx::DescriptorSet::setStorageImage(const Texture& texture, uint32_t binding) {
    vk::DescriptorImageInfo descriptor_image_info = {};
    descriptor_image_info.setImageView(texture.image_view);
    descriptor_image_info.setImageLayout(vk::ImageLayout::eGeneral);

    vk::WriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.setDstSet(raw);
    write_descriptor_set.setDstBinding(binding);
    write_descriptor_set.setDstArrayElement(0);
    write_descriptor_set.setDescriptorCount(1);
    write_descriptor_set.setDescriptorType(vk::DescriptorType::eStorageImage);
    write_descriptor_set.setPImageInfo(&descriptor_image_info);

    device->raii.raw.updateDescriptorSets(1, &write_descriptor_set, 0, nullptr, device->raii.dispatcher);
}

void gfx::DescriptorSet::setLabel(const std::string& name) {
    vk::DebugMarkerObjectNameInfoEXT pool_info = {};
    pool_info.setObjectType(vk::DebugReportObjectTypeEXT::eDescriptorPool);
    pool_info.setObject(uint64_t(VkDescriptorPool(pool)));
    pool_info.setPObjectName(name.c_str());

    device->raii.raw.debugMarkerSetObjectNameEXT(pool_info, device->raii.dispatcher);

    vk::DebugMarkerObjectNameInfoEXT set_info = {};
    set_info.setObjectType(vk::DebugReportObjectTypeEXT::eDescriptorSet);
    set_info.setObject(uint64_t(VkDescriptorSet(raw)));
    set_info.setPObjectName(name.c_str());

    device->raii.raw.debugMarkerSetObjectNameEXT(pool_info, device->raii.dispatcher);
}