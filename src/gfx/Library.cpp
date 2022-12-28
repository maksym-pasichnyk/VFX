#include "Device.hpp"
#include "Library.hpp"
#include "Function.hpp"

#include <spirv_reflect.h>

gfx::Library::Library(SharedPtr<Device> device, const vk::ShaderModuleCreateInfo& info) : mDevice(std::move(device)) {
    spvReflectShaderModule = static_cast<SpvReflectShaderModule*>(::operator new(sizeof(SpvReflectShaderModule), static_cast<std::align_val_t>(alignof(SpvReflectShaderModule))));

    spvReflectCreateShaderModule(info.codeSize, info.pCode, static_cast<SpvReflectShaderModule*>(spvReflectShaderModule));
    vkShaderModule = mDevice->vkDevice.createShaderModule(info, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);
}

gfx::Library::~Library() {
    spvReflectDestroyShaderModule(static_cast<SpvReflectShaderModule*>(spvReflectShaderModule));
    ::operator delete(spvReflectShaderModule, sizeof(SpvReflectShaderModule), static_cast<std::align_val_t>(alignof(SpvReflectShaderModule)));

    mDevice->vkDevice.destroyShaderModule(vkShaderModule, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);
}

auto gfx::Library::makeFunction(std::string name) -> SharedPtr<Function> {
    return TransferPtr(new Function(RetainPtr(this), std::move(name)));
}