#include "Device.hpp"
#include "Library.hpp"
#include "Function.hpp"

gfx::Library::Library(SharedPtr<Device> device, const vk::ShaderModuleCreateInfo& info) : mDevice(std::move(device)) {
    mShaderModule = mDevice->mDevice.createShaderModule(info, VK_NULL_HANDLE, mDevice->mDispatchLoaderDynamic);
    spvReflectCreateShaderModule(info.codeSize, info.pCode, &mSpvReflectShaderModule);
}

gfx::Library::~Library() {
    spvReflectDestroyShaderModule(&mSpvReflectShaderModule);
    mDevice->mDevice.destroyShaderModule(mShaderModule, VK_NULL_HANDLE, mDevice->mDispatchLoaderDynamic);
}

auto gfx::Library::newFunction(std::string name) -> SharedPtr<Function> {
    return TransferPtr(new Function(RetainPtr(this), std::move(name)));
}