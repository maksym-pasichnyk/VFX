#include "Device.hpp"
#include "Buffer.hpp"
#include "Sampler.hpp"
#include "Texture.hpp"
#include "Surface.hpp"
#include "Library.hpp"
#include "Drawable.hpp"
#include "Instance.hpp"
#include "Swapchain.hpp"
#include "CommandQueue.hpp"
#include "DescriptorSet.hpp"
#include "RenderPipelineState.hpp"
#include "ComputePipelineState.hpp"

#include "spdlog/spdlog.h"

#include <unordered_map>
#include <spirv_reflect.h>

// todo: fill table with valid aspect
const auto image_aspect_flags_table = std::unordered_map<vk::Format, vk::ImageAspectFlags> {
    { vk::Format::eUndefined, vk::ImageAspectFlagBits::eNone },
    { vk::Format::eR4G4UnormPack8, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR4G4B4A4UnormPack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB4G4R4A4UnormPack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR5G6B5UnormPack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB5G6R5UnormPack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR5G5B5A1UnormPack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB5G5R5A1UnormPack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA1R5G5B5UnormPack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8Snorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8Uscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8Sscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8Uint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8Sint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8Srgb, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8Snorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8Uscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8Sscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8Uint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8Sint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8Srgb, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8B8Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8B8Snorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8B8Uscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8B8Sscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8B8Uint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8B8Sint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8B8Srgb, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB8G8R8Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB8G8R8Snorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB8G8R8Uscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB8G8R8Sscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB8G8R8Uint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB8G8R8Sint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB8G8R8Srgb, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8B8A8Snorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8B8A8Uscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8B8A8Sscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8B8A8Uint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8B8A8Sint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB8G8R8A8Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB8G8R8A8Snorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB8G8R8A8Uscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB8G8R8A8Sscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB8G8R8A8Uint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB8G8R8A8Sint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB8G8R8A8Srgb, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA8B8G8R8UnormPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA8B8G8R8SnormPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA8B8G8R8UscaledPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA8B8G8R8SscaledPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA8B8G8R8UintPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA8B8G8R8SintPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA8B8G8R8SrgbPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA2R10G10B10UnormPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA2R10G10B10SnormPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA2R10G10B10UscaledPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA2R10G10B10SscaledPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA2R10G10B10UintPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA2R10G10B10SintPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA2B10G10R10UnormPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA2B10G10R10SnormPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA2B10G10R10UscaledPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA2B10G10R10SscaledPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA2B10G10R10UintPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA2B10G10R10SintPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16Snorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16Uscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16Sscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16Uint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16Sint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16Sfloat, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16Snorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16Uscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16Sscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16Uint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16Sint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16Sfloat, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16B16Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16B16Snorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16B16Uscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16B16Sscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16B16Uint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16B16Sint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16B16Sfloat, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16B16A16Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16B16A16Snorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16B16A16Uscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16B16A16Sscaled, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16B16A16Uint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16B16A16Sint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR16G16B16A16Sfloat, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR32Uint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR32Sint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR32Sfloat, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR32G32Uint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR32G32Sint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR32G32Sfloat, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR32G32B32Uint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR32G32B32Sint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR32G32B32Sfloat, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR32G32B32A32Uint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR32G32B32A32Sint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR32G32B32A32Sfloat, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR64Uint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR64Sint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR64Sfloat, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR64G64Uint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR64G64Sint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR64G64Sfloat, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR64G64B64Uint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR64G64B64Sint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR64G64B64Sfloat, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR64G64B64A64Uint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR64G64B64A64Sint, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR64G64B64A64Sfloat, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB10G11R11UfloatPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eE5B9G9R9UfloatPack32, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eD16Unorm, vk::ImageAspectFlagBits::eDepth },
    { vk::Format::eX8D24UnormPack32, vk::ImageAspectFlagBits::eDepth },
    { vk::Format::eD32Sfloat, vk::ImageAspectFlagBits::eDepth },
    { vk::Format::eS8Uint, vk::ImageAspectFlagBits::eStencil },
    { vk::Format::eD16UnormS8Uint, vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil },
    { vk::Format::eD24UnormS8Uint, vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil },
    { vk::Format::eD32SfloatS8Uint, vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil },
    { vk::Format::eBc1RgbUnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eBc1RgbSrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eBc1RgbaUnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eBc1RgbaSrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eBc2UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eBc2SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eBc3UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eBc3SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eBc4UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eBc4SnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eBc5UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eBc5SnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eBc6HUfloatBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eBc6HSfloatBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eBc7UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eBc7SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eEtc2R8G8B8UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eEtc2R8G8B8SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eEtc2R8G8B8A1UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eEtc2R8G8B8A1SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eEtc2R8G8B8A8UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eEtc2R8G8B8A8SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eEacR11UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eEacR11SnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eEacR11G11UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eEacR11G11SnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc4x4UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc4x4SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc5x4UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc5x4SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc5x5UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc5x5SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc6x5UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc6x5SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc6x6UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc6x6SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc8x5UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc8x5SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc8x6UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc8x6SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc8x8UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc8x8SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc10x5UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc10x5SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc10x6UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc10x6SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc10x8UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc10x8SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc10x10UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc10x10SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc12x10UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc12x10SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc12x12UnormBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc12x12SrgbBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG8B8G8R8422Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB8G8R8G8422Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG8B8R83Plane420Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG8B8R82Plane420Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG8B8R83Plane422Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG8B8R82Plane422Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG8B8R83Plane444Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR10X6UnormPack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR10X6G10X6Unorm2Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR10X6G10X6B10X6A10X6Unorm4Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG10X6B10X6G10X6R10X6422Unorm4Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB10X6G10X6R10X6G10X6422Unorm4Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG10X6B10X6R10X63Plane420Unorm3Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG10X6B10X6R10X62Plane420Unorm3Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG10X6B10X6R10X63Plane422Unorm3Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG10X6B10X6R10X62Plane422Unorm3Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG10X6B10X6R10X63Plane444Unorm3Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR12X4UnormPack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR12X4G12X4Unorm2Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR12X4G12X4B12X4A12X4Unorm4Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG12X4B12X4G12X4R12X4422Unorm4Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB12X4G12X4R12X4G12X4422Unorm4Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG12X4B12X4R12X43Plane420Unorm3Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG12X4B12X4R12X42Plane420Unorm3Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG12X4B12X4R12X43Plane422Unorm3Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG12X4B12X4R12X42Plane422Unorm3Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG12X4B12X4R12X43Plane444Unorm3Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG16B16G16R16422Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB16G16R16G16422Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG16B16R163Plane420Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG16B16R162Plane420Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG16B16R163Plane422Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG16B16R162Plane422Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG16B16R163Plane444Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG8B8R82Plane444Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG10X6B10X6R10X62Plane444Unorm3Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG12X4B12X4R12X42Plane444Unorm3Pack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG16B16R162Plane444Unorm, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA4R4G4B4UnormPack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA4B4G4R4UnormPack16, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc4x4SfloatBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc5x4SfloatBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc5x5SfloatBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc6x5SfloatBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc6x6SfloatBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc8x5SfloatBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc8x6SfloatBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc8x8SfloatBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc10x5SfloatBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc10x6SfloatBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc10x8SfloatBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc10x10SfloatBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc12x10SfloatBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc12x12SfloatBlock, vk::ImageAspectFlagBits::eColor },
    { vk::Format::ePvrtc12BppUnormBlockIMG, vk::ImageAspectFlagBits::eColor },
    { vk::Format::ePvrtc14BppUnormBlockIMG, vk::ImageAspectFlagBits::eColor },
    { vk::Format::ePvrtc22BppUnormBlockIMG, vk::ImageAspectFlagBits::eColor },
    { vk::Format::ePvrtc24BppUnormBlockIMG, vk::ImageAspectFlagBits::eColor },
    { vk::Format::ePvrtc12BppSrgbBlockIMG, vk::ImageAspectFlagBits::eColor },
    { vk::Format::ePvrtc14BppSrgbBlockIMG, vk::ImageAspectFlagBits::eColor },
    { vk::Format::ePvrtc22BppSrgbBlockIMG, vk::ImageAspectFlagBits::eColor },
    { vk::Format::ePvrtc24BppSrgbBlockIMG, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA4B4G4R4UnormPack16EXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eA4R4G4B4UnormPack16EXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc10x10SfloatBlockEXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc10x5SfloatBlockEXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc10x6SfloatBlockEXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc10x8SfloatBlockEXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc12x10SfloatBlockEXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc12x12SfloatBlockEXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc4x4SfloatBlockEXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc5x4SfloatBlockEXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc5x5SfloatBlockEXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc6x5SfloatBlockEXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc6x6SfloatBlockEXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc8x5SfloatBlockEXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc8x6SfloatBlockEXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eAstc8x8SfloatBlockEXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB10X6G10X6R10X6G10X6422Unorm4Pack16KHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB12X4G12X4R12X4G12X4422Unorm4Pack16KHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB16G16R16G16422UnormKHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eB8G8R8G8422UnormKHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG10X6B10X6G10X6R10X6422Unorm4Pack16KHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG10X6B10X6R10X62Plane420Unorm3Pack16KHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG10X6B10X6R10X62Plane422Unorm3Pack16KHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG10X6B10X6R10X62Plane444Unorm3Pack16EXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG10X6B10X6R10X63Plane420Unorm3Pack16KHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG10X6B10X6R10X63Plane422Unorm3Pack16KHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG10X6B10X6R10X63Plane444Unorm3Pack16KHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG12X4B12X4G12X4R12X4422Unorm4Pack16KHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG12X4B12X4R12X42Plane420Unorm3Pack16KHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG12X4B12X4R12X42Plane422Unorm3Pack16KHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG12X4B12X4R12X42Plane444Unorm3Pack16EXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG12X4B12X4R12X43Plane420Unorm3Pack16KHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG12X4B12X4R12X43Plane422Unorm3Pack16KHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG12X4B12X4R12X43Plane444Unorm3Pack16KHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG16B16G16R16422UnormKHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG16B16R162Plane420UnormKHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG16B16R162Plane422UnormKHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG16B16R162Plane444UnormEXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG16B16R163Plane420UnormKHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG16B16R163Plane422UnormKHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG16B16R163Plane444UnormKHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG8B8G8R8422UnormKHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG8B8R82Plane420UnormKHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG8B8R82Plane422UnormKHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG8B8R82Plane444UnormEXT, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG8B8R83Plane420UnormKHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG8B8R83Plane422UnormKHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eG8B8R83Plane444UnormKHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR10X6G10X6B10X6A10X6Unorm4Pack16KHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR10X6G10X6Unorm2Pack16KHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR10X6UnormPack16KHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR12X4G12X4B12X4A12X4Unorm4Pack16KHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR12X4G12X4Unorm2Pack16KHR, vk::ImageAspectFlagBits::eColor },
    { vk::Format::eR12X4UnormPack16KHR, vk::ImageAspectFlagBits::eColor },
};

struct DescriptorSetLayoutCreateInfo {
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {};

    void emplace(const vk::DescriptorSetLayoutBinding& other) {
        for (auto& binding : bindings) {
            if (binding.binding != other.binding) {
                continue;
            }
            if (binding.descriptorType != other.descriptorType) {
                continue;
            }
            if (binding.descriptorCount != other.descriptorCount) {
                continue;
            }
            binding.stageFlags |= other.stageFlags;
            return;
        }
        bindings.emplace_back(other);
    }
};

gfx::DeviceShared::DeviceShared(gfx::Instance instance, gfx::raii::Device raii, vk::PhysicalDevice adapter, uint32_t family_index, uint32_t queue_index, vk::Queue raw_queue, VmaAllocator allocator)
    : instance(std::move(instance)), raii(raii), adapter(adapter), family_index(family_index), queue_index(queue_index), raw_queue(raw_queue), allocator(allocator) {}

gfx::DeviceShared::~DeviceShared() {
    vmaDestroyAllocator(allocator);
    raii.raw.destroy(nullptr, raii.dispatcher);
}

gfx::Device::Device() : shared(nullptr) {}
gfx::Device::Device(std::shared_ptr<DeviceShared> shared) : shared(std::move(shared)) {}

auto gfx::Device::handle() -> vk::Device {
    return shared->raii.raw;
}

auto gfx::Device::dispatcher() -> const vk::raii::DeviceDispatcher& {
    return shared->raii.dispatcher;
}

auto gfx::Device::allocator() -> VmaAllocator {
    return shared->allocator;
}

void gfx::Device::waitIdle() {
    shared->raii.raw.waitIdle(shared->raii.dispatcher);
}

auto gfx::Device::newTexture(const TextureSettings& description) -> Texture {
    auto aspect = image_aspect_flags_table.at(description.format);
    auto texture = std::make_shared<TextureShared>(*this);

    texture->format = description.format;
    texture->extent.setWidth(description.width);
    texture->extent.setHeight(description.height);
    texture->extent.setDepth(1);
    texture->subresource.setAspectMask(aspect);
    texture->subresource.setLevelCount(1);
    texture->subresource.setLayerCount(1);

    vk::ImageCreateInfo image_create_info = {};
    image_create_info.setImageType(vk::ImageType::e2D);
    image_create_info.setFormat(description.format);
    image_create_info.extent.setWidth(description.width);
    image_create_info.extent.setHeight(description.height);
    image_create_info.extent.setDepth(1);
    image_create_info.setMipLevels(1);
    image_create_info.setArrayLayers(1);
    image_create_info.setUsage(description.usage);

    VmaAllocationCreateInfo allocation_create_info = {};
    allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
    vmaCreateImage(shared->allocator, reinterpret_cast<const VkImageCreateInfo*>(&image_create_info), &allocation_create_info, reinterpret_cast<VkImage*>(&texture->image), &texture->allocation, nullptr);

    vk::ImageViewCreateInfo view_create_info = {};
    view_create_info.setImage(texture->image);
    view_create_info.setViewType(vk::ImageViewType::e2D),
    view_create_info.setFormat(description.format);
    view_create_info.setComponents(description.mapping),
    view_create_info.setSubresourceRange(texture->subresource);

    texture->image_view = handle().createImageView(view_create_info, VK_NULL_HANDLE, dispatcher());

    return Texture(std::move(texture));
}

auto gfx::Device::newSampler(const vk::SamplerCreateInfo& info) -> Sampler {
    return Sampler(std::make_shared<SamplerShared>(*this, handle().createSampler(info, VK_NULL_HANDLE, dispatcher())));
}

auto gfx::Device::newBuffer(vk::BufferUsageFlags usage, uint64_t size, VmaAllocationCreateFlags options) -> Buffer {
    vk::BufferCreateInfo buffer_create_info = {};
    buffer_create_info.setSize(static_cast<vk::DeviceSize>(size));
    buffer_create_info.setUsage(usage);

    VmaAllocationCreateInfo allocation_create_info = {};
    allocation_create_info.flags = options;
    allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;

    auto buffer = std::make_shared<BufferShared>(*this);

    vmaCreateBuffer(allocator(), reinterpret_cast<const VkBufferCreateInfo*>(&buffer_create_info), &allocation_create_info, reinterpret_cast<VkBuffer*>(&buffer->raw), &buffer->allocation, nullptr);

    return Buffer(buffer);
}

auto gfx::Device::newBuffer(vk::BufferUsageFlags usage, const void* pointer, uint64_t size, VmaAllocationCreateFlags options) -> Buffer {
    // todo: use transfer operation if buffer is not mappable
    auto id = newBuffer(usage, size, options);
    std::memcpy(id.contents(), pointer, size);
    id.didModifyRange(0, size);
    return id;
}

auto gfx::Device::newLibrary(const std::vector<char>& bytes) -> Library {
    vk::ShaderModuleCreateInfo module_create_info = {};
    module_create_info.setCodeSize(bytes.size());
    module_create_info.setPCode(reinterpret_cast<const uint32_t *>(bytes.data()));

    auto library = std::make_shared<LibraryShared>(*this);
    library->raw = handle().createShaderModule(module_create_info, VK_NULL_HANDLE, dispatcher());

    spvReflectCreateShaderModule(module_create_info.codeSize, module_create_info.pCode, &library->spvReflectShaderModule);

    return Library(library);
}

auto gfx::Device::newRenderPipelineState(const gfx::RenderPipelineStateDescription& description) -> RenderPipelineState {
    std::vector<vk::PushConstantRange> push_constant_ranges = {};
    std::vector<DescriptorSetLayoutCreateInfo> descriptor_sets = {};

    for (auto& function : {description.vertexFunction, description.fragmentFunction}) {
        // todo: merge if overlaps
        for (auto& pcb : std::span(function.library.shared->spvReflectShaderModule.push_constant_blocks, function.library.shared->spvReflectShaderModule.push_constant_block_count)) {
            vk::PushConstantRange push_constant_range = {};
            push_constant_range.setSize(pcb.size);
            push_constant_range.setOffset(pcb.offset);
            push_constant_range.setStageFlags(vk::ShaderStageFlags(function.entry_point->shader_stage));
            push_constant_ranges.emplace_back(push_constant_range);
        }

        for (auto& sds : std::span(function.library.shared->spvReflectShaderModule.descriptor_sets, function.library.shared->spvReflectShaderModule.descriptor_set_count)) {
            if (sds.set >= descriptor_sets.size()) {
                descriptor_sets.resize(sds.set + 1);
            }

            for (auto& sb : std::span(sds.bindings, sds.binding_count)) {
                vk::DescriptorSetLayoutBinding binding = {};
                binding.setBinding(sb->binding);
                binding.setDescriptorType(vk::DescriptorType(sb->descriptor_type));
                binding.setDescriptorCount(sb->count);
                binding.setStageFlags(vk::ShaderStageFlags(function.entry_point->shader_stage));
                binding.setPImmutableSamplers(nullptr);

                descriptor_sets[sds.set].emplace(binding);
            }
        }
    }
    
    auto state = std::make_shared<RenderPipelineStateShared>(*this);

    state->bind_group_layouts.resize(descriptor_sets.size());
    for (uint32_t i = 0; i < state->bind_group_layouts.size(); ++i) {
        vk::DescriptorSetLayoutCreateInfo layout_create_info = {};
        layout_create_info.setBindings(descriptor_sets[i].bindings);
        state->bind_group_layouts[i] = handle().createDescriptorSetLayout(layout_create_info, nullptr, dispatcher());
    }

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info = {};
    pipeline_layout_create_info.setSetLayouts(state->bind_group_layouts);
    pipeline_layout_create_info.setPushConstantRanges(push_constant_ranges);
    state->pipeline_layout = handle().createPipelineLayout(pipeline_layout_create_info, nullptr, dispatcher());

    vk::PipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.setViewportCount(1);
    viewport_state.setScissorCount(1);

    auto dynamicStates = std::array{
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamic_state = {};
    dynamic_state.setDynamicStates(dynamicStates);

    std::vector<vk::PipelineShaderStageCreateInfo> stages = {};

    vk::PipelineShaderStageCreateInfo vertex_stage = {};
    vertex_stage.setStage(vk::ShaderStageFlagBits::eVertex);
    vertex_stage.setModule(description.vertexFunction.library.shared->raw);
    vertex_stage.setPName(description.vertexFunction.name.c_str());
    stages.emplace_back(vertex_stage);

    vk::PipelineShaderStageCreateInfo fragment_stage = {};
    fragment_stage.setStage(vk::ShaderStageFlagBits::eFragment);
    fragment_stage.setModule(description.fragmentFunction.library.shared->raw);
    fragment_stage.setPName(description.fragmentFunction.name.c_str());
    stages.emplace_back(fragment_stage);

    vk::PipelineVertexInputStateCreateInfo vertex_input_state = {};
    if (description.vertexDescription.has_value()) {
        vertex_input_state.setVertexBindingDescriptions(description.vertexDescription->layouts.elements);
        vertex_input_state.setVertexAttributeDescriptions(description.vertexDescription->attributes.elements);
    }

    vk::PipelineColorBlendStateCreateInfo color_blend_state = {};
    color_blend_state.setAttachments(description.attachments.elements);

    vk::PipelineRenderingCreateInfo rendering = {};
    rendering.setViewMask(description.viewMask);
    rendering.setColorAttachmentFormats(description.colorAttachmentFormats.elements);
    rendering.setDepthAttachmentFormat(description.depthAttachmentFormat);
    rendering.setStencilAttachmentFormat(description.stencilAttachmentFormat);

    vk::GraphicsPipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.setPNext(&rendering);
    pipeline_create_info.setStages(stages);
    pipeline_create_info.setPVertexInputState(&vertex_input_state);
    pipeline_create_info.setPInputAssemblyState(&description.inputAssemblyState);
    pipeline_create_info.setPViewportState(&viewport_state);
    pipeline_create_info.setPRasterizationState(&description.rasterizationState);
    pipeline_create_info.setPMultisampleState(&description.multisampleState);
    pipeline_create_info.setPDepthStencilState(&description.depthStencilState);
    pipeline_create_info.setPColorBlendState(&color_blend_state);
    pipeline_create_info.setPDynamicState(&dynamic_state);
    pipeline_create_info.setLayout(state->pipeline_layout);
    pipeline_create_info.setRenderPass(nullptr);
    pipeline_create_info.setSubpass(0);
    pipeline_create_info.setBasePipelineHandle(nullptr);
    pipeline_create_info.setBasePipelineIndex(0);

    std::ignore = handle().createGraphicsPipelines({}, 1, &pipeline_create_info, nullptr, &state->pipeline, dispatcher());
    
    return RenderPipelineState(state);
}

auto gfx::Device::newComputePipelineState(const Function& function) -> ComputePipelineState {
    std::vector<vk::PushConstantRange> push_constant_ranges = {};
    std::vector<DescriptorSetLayoutCreateInfo> descriptor_sets = {};

    for (auto& pcb : std::span(function.library.shared->spvReflectShaderModule.push_constant_blocks, function.library.shared->spvReflectShaderModule.push_constant_block_count)) {
        vk::PushConstantRange push_constant_range = {};
        push_constant_range.setSize(pcb.size);
        push_constant_range.setOffset(pcb.offset);
        push_constant_range.setStageFlags(vk::ShaderStageFlags(function.entry_point->shader_stage));
        push_constant_ranges.emplace_back(push_constant_range);
    }

    for (auto& sds : std::span(function.library.shared->spvReflectShaderModule.descriptor_sets, function.library.shared->spvReflectShaderModule.descriptor_set_count)) {
        if (sds.set >= descriptor_sets.size()) {
            descriptor_sets.resize(sds.set + 1);
        }

        for (auto& sb : std::span(sds.bindings, sds.binding_count)) {
            vk::DescriptorSetLayoutBinding binding = {};
            binding.setBinding(sb->binding);
            binding.setDescriptorType(vk::DescriptorType(sb->descriptor_type));
            binding.setDescriptorCount(sb->count);
            binding.setStageFlags(vk::ShaderStageFlags(function.entry_point->shader_stage));
            binding.setPImmutableSamplers(nullptr);

            descriptor_sets[sds.set].emplace(binding);
        }
    }

    auto state = std::make_shared<ComputePipelineStateShared>(*this);
    state->bind_group_layouts.resize(descriptor_sets.size());
    for (uint32_t i = 0; i < descriptor_sets.size(); ++i) {
        vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};
        descriptor_set_layout_create_info.setBindings(descriptor_sets[i].bindings);

        state->bind_group_layouts[i] = handle().createDescriptorSetLayout(descriptor_set_layout_create_info, nullptr, dispatcher());
    }

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info = {};
    pipeline_layout_create_info.setSetLayouts(state->bind_group_layouts);
    pipeline_layout_create_info.setPushConstantRanges(push_constant_ranges);
    state->pipeline_layout = handle().createPipelineLayout(pipeline_layout_create_info, nullptr, dispatcher());

    vk::PipelineShaderStageCreateInfo shader_stage_create_info{};
    shader_stage_create_info.setStage(vk::ShaderStageFlagBits::eCompute);
    shader_stage_create_info.setModule(function.library.shared->raw);
    shader_stage_create_info.setPName(function.name.c_str());

    vk::ComputePipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.setStage(shader_stage_create_info);
    pipeline_create_info.setLayout(state->pipeline_layout);
    pipeline_create_info.setBasePipelineHandle(nullptr);
    pipeline_create_info.setBasePipelineIndex(0);

    std::ignore = handle().createComputePipelines({}, 1, &pipeline_create_info, nullptr, &state->pipeline, dispatcher());
    return ComputePipelineState(state);
}

auto gfx::Device::newCommandQueue() -> CommandQueue {
    vk::CommandPoolCreateInfo command_pool_create_info = {};
    command_pool_create_info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    command_pool_create_info.setQueueFamilyIndex(shared->family_index);

    auto pool = handle().createCommandPool(command_pool_create_info, nullptr, dispatcher());

    return CommandQueue(std::make_shared<CommandQueueShared>(*this, pool));
}

// todo: get sizes from layout
auto gfx::Device::newDescriptorSet(vk::DescriptorSetLayout layout, const std::vector<vk::DescriptorPoolSize>& sizes) -> DescriptorSet {
    vk::DescriptorPoolCreateInfo pool_create_info = {};
    pool_create_info.setMaxSets(1);
    pool_create_info.setPoolSizes(sizes);
    auto pool = shared->raii.raw.createDescriptorPool(pool_create_info, VK_NULL_HANDLE, shared->raii.dispatcher);

    vk::DescriptorSetAllocateInfo ds_allocate_info = {};
    ds_allocate_info.setDescriptorPool(pool);
    ds_allocate_info.setDescriptorSetCount(1);
    ds_allocate_info.setPSetLayouts(&layout);
    auto set = shared->raii.raw.allocateDescriptorSets(ds_allocate_info, shared->raii.dispatcher)[0];

    return DescriptorSet(std::make_shared<DescriptorSetShared>(*this, set, pool));
}

auto gfx::Device::createSwapchain(Surface const& surface) -> Swapchain {
    return Swapchain(std::make_shared<SwapchainShared>(*this, surface));
}