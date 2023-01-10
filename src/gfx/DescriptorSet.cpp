#include "DescriptorSet.hpp"
#include "Texture.hpp"
#include "Sampler.hpp"
#include "Device.hpp"
#include "Buffer.hpp"

gfx::DescriptorSet::DescriptorSet(SharedPtr<Device> device) : mDevice(std::move(device)) {}

gfx::DescriptorSet::~DescriptorSet() {
    mDevice->mDevice.destroyDescriptorPool(mDescriptorPool, VK_NULL_HANDLE, mDevice->mDispatchLoaderDynamic);
}

void gfx::DescriptorSet::setBuffer(const SharedPtr<Buffer>& buffer, uint64_t offset, uint32_t binding) {
    vk::DescriptorBufferInfo descriptor_buffer_info = {};
    descriptor_buffer_info.setBuffer(buffer->mBuffer);
    descriptor_buffer_info.setOffset(offset);
    descriptor_buffer_info.setRange(VK_WHOLE_SIZE);

    vk::WriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.setDstSet(mDescriptorSet);
    write_descriptor_set.setDstBinding(binding);
    write_descriptor_set.setDstArrayElement(0);
    write_descriptor_set.setDescriptorCount(1);
    write_descriptor_set.setDescriptorType(vk::DescriptorType::eUniformBuffer);
    write_descriptor_set.setPBufferInfo(&descriptor_buffer_info);

    mDevice->mDevice.updateDescriptorSets(1, &write_descriptor_set, 0, nullptr, mDevice->mDispatchLoaderDynamic);
}

void gfx::DescriptorSet::setStorageBuffer(const SharedPtr<Buffer>& buffer, uint64_t offset, uint32_t binding) {
    vk::DescriptorBufferInfo descriptor_buffer_info = {};
    descriptor_buffer_info.setBuffer(buffer->mBuffer);
    descriptor_buffer_info.setOffset(offset);
    descriptor_buffer_info.setRange(VK_WHOLE_SIZE);

    vk::WriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.setDstSet(mDescriptorSet);
    write_descriptor_set.setDstBinding(binding);
    write_descriptor_set.setDstArrayElement(0);
    write_descriptor_set.setDescriptorCount(1);
    write_descriptor_set.setDescriptorType(vk::DescriptorType::eStorageBuffer);
    write_descriptor_set.setPBufferInfo(&descriptor_buffer_info);

    mDevice->mDevice.updateDescriptorSets(1, &write_descriptor_set, 0, nullptr, mDevice->mDispatchLoaderDynamic);
}

void gfx::DescriptorSet::setTexture(const SharedPtr<Texture>& texture, uint32_t binding) {
    vk::DescriptorImageInfo descriptor_image_info = {};
    descriptor_image_info.setImageView(texture->mImageView);
    descriptor_image_info.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    vk::WriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.setDstSet(mDescriptorSet);
    write_descriptor_set.setDstBinding(binding);
    write_descriptor_set.setDstArrayElement(0);
    write_descriptor_set.setDescriptorCount(1);
    write_descriptor_set.setDescriptorType(vk::DescriptorType::eSampledImage);
    write_descriptor_set.setPImageInfo(&descriptor_image_info);

    mDevice->mDevice.updateDescriptorSets(1, &write_descriptor_set, 0, nullptr, mDevice->mDispatchLoaderDynamic);
}

void gfx::DescriptorSet::setSampler(const SharedPtr<Sampler>& sampler, uint32_t binding) {
    vk::DescriptorImageInfo descriptor_image_info = {};
    descriptor_image_info.setSampler(sampler->mSampler);

    vk::WriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.setDstSet(mDescriptorSet);
    write_descriptor_set.setDstBinding(binding);
    write_descriptor_set.setDstArrayElement(0);
    write_descriptor_set.setDescriptorCount(1);
    write_descriptor_set.setDescriptorType(vk::DescriptorType::eSampler);
    write_descriptor_set.setPImageInfo(&descriptor_image_info);

    mDevice->mDevice.updateDescriptorSets(1, &write_descriptor_set, 0, nullptr, mDevice->mDispatchLoaderDynamic);
}

void gfx::DescriptorSet::setStorageImage(const SharedPtr<Texture>& texture, uint32_t binding) {
    vk::DescriptorImageInfo descriptor_image_info = {};
    descriptor_image_info.setImageView(texture->mImageView);
    descriptor_image_info.setImageLayout(vk::ImageLayout::eGeneral);

    vk::WriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.setDstSet(mDescriptorSet);
    write_descriptor_set.setDstBinding(binding);
    write_descriptor_set.setDstArrayElement(0);
    write_descriptor_set.setDescriptorCount(1);
    write_descriptor_set.setDescriptorType(vk::DescriptorType::eStorageImage);
    write_descriptor_set.setPImageInfo(&descriptor_image_info);

    mDevice->mDevice.updateDescriptorSets(1, &write_descriptor_set, 0, nullptr, mDevice->mDispatchLoaderDynamic);
}

void gfx::DescriptorSet::setLabel(const std::string& name) {
    vk::DebugMarkerObjectNameInfoEXT pool_info = {};
    pool_info.setObjectType(vk::DebugReportObjectTypeEXT::eDescriptorPool);
    pool_info.setObject(uint64_t(VkDescriptorPool(mDescriptorPool)));
    pool_info.setPObjectName(name.c_str());

    mDevice->mDevice.debugMarkerSetObjectNameEXT(pool_info, mDevice->mDispatchLoaderDynamic);

    vk::DebugMarkerObjectNameInfoEXT set_info = {};
    set_info.setObjectType(vk::DebugReportObjectTypeEXT::eDescriptorSet);
    set_info.setObject(uint64_t(VkDescriptorSet(mDescriptorSet)));
    set_info.setPObjectName(name.c_str());

    mDevice->mDevice.debugMarkerSetObjectNameEXT(pool_info, mDevice->mDispatchLoaderDynamic);
}
