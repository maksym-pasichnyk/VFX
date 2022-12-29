#include "Device.hpp"
#include "Library.hpp"
#include "Function.hpp"

gfx::Library::Library(SharedPtr<Device> device, const vk::ShaderModuleCreateInfo& info) : mDevice(std::move(device)) {
    vkShaderModule = mDevice->vkDevice.createShaderModule(info, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);
    spvReflectCreateShaderModule(info.codeSize, info.pCode, &mSpvReflectShaderModule);
}

gfx::Library::~Library() {
    spvReflectDestroyShaderModule(&mSpvReflectShaderModule);
    mDevice->vkDevice.destroyShaderModule(vkShaderModule, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);
}

auto gfx::Library::newFunction(std::string name) -> SharedPtr<Function> {
    return TransferPtr(new Function(RetainPtr(this), std::move(name)));
}