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
#include "RenderPipelineState.hpp"
#include "ComputePipelineState.hpp"

#include "spdlog/spdlog.h"
#include "ManagedObject.hpp"

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

gfx::Device::Device(ManagedShared<Instance> instance, raii::Device raii, vk::PhysicalDevice adapter, uint32_t family_index, uint32_t queue_index, vk::Queue raw_queue, VmaAllocator allocator)
    : instance(std::move(instance)), raii(raii), adapter(adapter), family_index(family_index), queue_index(queue_index), queue(raw_queue), allocator(allocator) {}

gfx::Device::~Device() {
    vmaDestroyAllocator(allocator);
    raii.raw.destroy(nullptr, raii.dispatcher);
}

void gfx::Device::waitIdle() {
    raii.raw.waitIdle(raii.dispatcher);
}

auto gfx::Device::newTexture(const TextureSettings& description) -> ManagedShared<Texture> {
    auto aspect = image_aspect_flags_table.at(description.format);
    auto texture = MakeShared(new Texture(MakeShared(this->retain())));

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
    vmaCreateImage(allocator, reinterpret_cast<const VkImageCreateInfo*>(&image_create_info), &allocation_create_info, reinterpret_cast<VkImage*>(&texture->image), &texture->allocation, nullptr);

    vk::ImageViewCreateInfo view_create_info = {};
    view_create_info.setImage(texture->image);
    view_create_info.setViewType(vk::ImageViewType::e2D),
    view_create_info.setFormat(description.format);
    view_create_info.setComponents(description.mapping),
    view_create_info.setSubresourceRange(texture->subresource);

    texture->image_view = raii.raw.createImageView(view_create_info, VK_NULL_HANDLE, raii.dispatcher);

    return texture;
}

auto gfx::Device::newSampler(const vk::SamplerCreateInfo& info) -> ManagedShared<Sampler> {
    return MakeShared(new Sampler(MakeShared(this->retain()), raii.raw.createSampler(info, VK_NULL_HANDLE, raii.dispatcher)));
}

auto gfx::Device::newBuffer(vk::BufferUsageFlags usage, uint64_t size, VmaAllocationCreateFlags options) -> ManagedShared<Buffer> {
    vk::BufferCreateInfo buffer_create_info = {};
    buffer_create_info.setSize(static_cast<vk::DeviceSize>(size));
    buffer_create_info.setUsage(usage);

    VmaAllocationCreateInfo allocation_create_info = {};
    allocation_create_info.flags = options;
    allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;

    auto buffer = MakeShared(new Buffer(MakeShared(this->retain())));
    vmaCreateBuffer(allocator, reinterpret_cast<const VkBufferCreateInfo*>(&buffer_create_info), &allocation_create_info, reinterpret_cast<VkBuffer*>(&buffer->raw), &buffer->allocation, nullptr);
    return buffer;
}

auto gfx::Device::newBuffer(vk::BufferUsageFlags usage, const void* pointer, uint64_t size, VmaAllocationCreateFlags options) -> ManagedShared<Buffer> {
    // todo: use transfer operation if buffer is not mappable
    auto buffer = newBuffer(usage, size, options);
    std::memcpy(buffer->contents(), pointer, size);
    buffer->didModifyRange(0, size);
    return buffer;
}

auto gfx::Device::newLibrary(const std::vector<char>& bytes) -> ManagedShared<Library> {
    vk::ShaderModuleCreateInfo module_create_info = {};
    module_create_info.setCodeSize(bytes.size());
    module_create_info.setPCode(reinterpret_cast<const uint32_t *>(bytes.data()));

    auto library = MakeShared(new Library(MakeShared(this->retain())));
    library->raw = raii.raw.createShaderModule(module_create_info, VK_NULL_HANDLE, raii.dispatcher);

    spvReflectCreateShaderModule(module_create_info.codeSize, module_create_info.pCode, &library->spvReflectShaderModule);

    return library;
}

auto gfx::Device::newRenderPipelineState(const gfx::RenderPipelineStateDescription& description) -> ManagedShared<RenderPipelineState> {
    std::vector<vk::PushConstantRange> push_constant_ranges = {};
    std::vector<DescriptorSetLayoutCreateInfo> descriptor_sets = {};

    for (auto& function : {description.vertexFunction, description.fragmentFunction}) {
        // todo: merge if overlaps
        for (auto& pcb : std::span(function->library->spvReflectShaderModule.push_constant_blocks, function->library->spvReflectShaderModule.push_constant_block_count)) {
            vk::PushConstantRange push_constant_range = {};
            push_constant_range.setSize(pcb.size);
            push_constant_range.setOffset(pcb.offset);
            push_constant_range.setStageFlags(vk::ShaderStageFlags(function->entry_point->shader_stage));
            push_constant_ranges.emplace_back(push_constant_range);
        }

        for (auto& sds : std::span(function->library->spvReflectShaderModule.descriptor_sets, function->library->spvReflectShaderModule.descriptor_set_count)) {
            if (sds.set >= descriptor_sets.size()) {
                descriptor_sets.resize(sds.set + 1);
            }

            for (auto& sb : std::span(sds.bindings, sds.binding_count)) {
                vk::DescriptorSetLayoutBinding binding = {};
                binding.setBinding(sb->binding);
                binding.setDescriptorType(vk::DescriptorType(sb->descriptor_type));
                binding.setDescriptorCount(sb->count);
                binding.setStageFlags(vk::ShaderStageFlags(function->entry_point->shader_stage));
                binding.setPImmutableSamplers(nullptr);

                descriptor_sets[sds.set].emplace(binding);
            }
        }
    }
    
    auto state = MakeShared(new RenderPipelineState(MakeShared(this->retain())));

    state->bind_group_layouts.resize(descriptor_sets.size());
    for (uint32_t i = 0; i < state->bind_group_layouts.size(); ++i) {
        vk::DescriptorSetLayoutCreateInfo layout_create_info = {};
        layout_create_info.setBindings(descriptor_sets[i].bindings);
        state->bind_group_layouts[i] = raii.raw.createDescriptorSetLayout(layout_create_info, nullptr, raii.dispatcher);
    }

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info = {};
    pipeline_layout_create_info.setSetLayouts(state->bind_group_layouts);
    pipeline_layout_create_info.setPushConstantRanges(push_constant_ranges);
    state->pipeline_layout = raii.raw.createPipelineLayout(pipeline_layout_create_info, nullptr, raii.dispatcher);

    vk::PipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.setViewportCount(1);
    viewport_state.setScissorCount(1);

    vk::PipelineInputAssemblyStateCreateInfo input_assembly_state = {};
    input_assembly_state.setTopology(description.inputAssemblyState.topology);
    input_assembly_state.setPrimitiveRestartEnable(description.inputAssemblyState.primitive_restart_enable);

    vk::PipelineRasterizationStateCreateInfo rasterization_state = {};
    rasterization_state.setDepthClampEnable(description.rasterizationState.depth_clamp_enable);
    rasterization_state.setRasterizerDiscardEnable(description.rasterizationState.discard_enable);
    rasterization_state.setPolygonMode(description.rasterizationState.polygon_mode);
    rasterization_state.setLineWidth(description.rasterizationState.line_width);
    rasterization_state.setCullMode(description.rasterizationState.cull_mode);
    rasterization_state.setFrontFace(description.rasterizationState.front_face);
    rasterization_state.setDepthBiasEnable(description.rasterizationState.depth_bias_enable);
    rasterization_state.setDepthBiasConstantFactor(description.rasterizationState.depth_bias_constant_factor);
    rasterization_state.setDepthBiasClamp(description.rasterizationState.depth_bias_clamp);
    rasterization_state.setDepthBiasSlopeFactor(description.rasterizationState.depth_bias_slope_factor);

    vk::PipelineTessellationStateCreateInfo tessellation_state = {};
    tessellation_state.setPatchControlPoints(description.tessellationState.patch_control_points);

    vk::PipelineMultisampleStateCreateInfo multisample_state = {};
    multisample_state.setRasterizationSamples(description.multisampleState.rasterization_samples);
    multisample_state.setSampleShadingEnable(description.multisampleState.sample_shading_enable);
    multisample_state.setMinSampleShading(description.multisampleState.min_sample_shading);
    multisample_state.setPSampleMask(description.multisampleState.sample_mask);
    multisample_state.setAlphaToCoverageEnable(description.multisampleState.alpha_to_coverage_enable);
    multisample_state.setAlphaToOneEnable(description.multisampleState.alpha_to_one_enable);

    vk::PipelineDepthStencilStateCreateInfo depth_stencil_state = {};
    depth_stencil_state.setDepthTestEnable(description.depthStencilState.depth_test_enable);
    depth_stencil_state.setDepthWriteEnable(description.depthStencilState.depth_write_enable);
    depth_stencil_state.setDepthCompareOp(description.depthStencilState.depth_compare_op);
    depth_stencil_state.setDepthBoundsTestEnable(description.depthStencilState.depth_bounds_test_enable);
    depth_stencil_state.setMinDepthBounds(description.depthStencilState.min_depth_bounds);
    depth_stencil_state.setMaxDepthBounds(description.depthStencilState.max_depth_bounds);
    depth_stencil_state.setStencilTestEnable(description.depthStencilState.stencil_test_enable);
    depth_stencil_state.setFront(description.depthStencilState.front);
    depth_stencil_state.setBack(description.depthStencilState.back);

    auto dynamicStates = std::array{
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamic_state = {};
    dynamic_state.setDynamicStates(dynamicStates);

    std::vector<vk::PipelineShaderStageCreateInfo> stages = {};

    vk::PipelineShaderStageCreateInfo vertex_stage = {};
    vertex_stage.setStage(vk::ShaderStageFlagBits::eVertex);
    vertex_stage.setModule(description.vertexFunction->library->raw);
    vertex_stage.setPName(description.vertexFunction->name.c_str());
    stages.emplace_back(vertex_stage);

    vk::PipelineShaderStageCreateInfo fragment_stage = {};
    fragment_stage.setStage(vk::ShaderStageFlagBits::eFragment);
    fragment_stage.setModule(description.fragmentFunction->library->raw);
    fragment_stage.setPName(description.fragmentFunction->name.c_str());
    stages.emplace_back(fragment_stage);

    vk::PipelineVertexInputStateCreateInfo vertex_input_state = {};
    vertex_input_state.setVertexBindingDescriptions(description.vertexInputState.bindings);
    vertex_input_state.setVertexAttributeDescriptions(description.vertexInputState.attributes);

    vk::PipelineColorBlendStateCreateInfo color_blend_state = {};
    color_blend_state.setAttachments(description.colorBlendAttachments.elements);

    vk::PipelineRenderingCreateInfo rendering = {};
    rendering.setViewMask(description.viewMask);
    rendering.setColorAttachmentFormats(description.colorAttachmentFormats.elements);
    rendering.setDepthAttachmentFormat(description.depthAttachmentFormat);
    rendering.setStencilAttachmentFormat(description.stencilAttachmentFormat);

    vk::GraphicsPipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.setPNext(&rendering);
    pipeline_create_info.setStages(stages);
    pipeline_create_info.setPVertexInputState(&vertex_input_state);
    pipeline_create_info.setPInputAssemblyState(&input_assembly_state);
    pipeline_create_info.setPViewportState(&viewport_state);
    pipeline_create_info.setPRasterizationState(&rasterization_state);
    pipeline_create_info.setPTessellationState(&tessellation_state);
    pipeline_create_info.setPMultisampleState(&multisample_state);
    pipeline_create_info.setPDepthStencilState(&depth_stencil_state);
    pipeline_create_info.setPColorBlendState(&color_blend_state);
    pipeline_create_info.setPDynamicState(&dynamic_state);
    pipeline_create_info.setLayout(state->pipeline_layout);
    pipeline_create_info.setRenderPass(nullptr);
    pipeline_create_info.setSubpass(0);
    pipeline_create_info.setBasePipelineHandle(nullptr);
    pipeline_create_info.setBasePipelineIndex(0);

    std::ignore = raii.raw.createGraphicsPipelines({}, 1, &pipeline_create_info, nullptr, &state->pipeline, raii.dispatcher);
    
    return state;
}

auto gfx::Device::newComputePipelineState(const ManagedShared<Function>& function) -> ManagedShared<ComputePipelineState> {
    std::vector<vk::PushConstantRange> push_constant_ranges = {};
    std::vector<DescriptorSetLayoutCreateInfo> descriptor_sets = {};

    for (auto& pcb : std::span(function->library->spvReflectShaderModule.push_constant_blocks, function->library->spvReflectShaderModule.push_constant_block_count)) {
        vk::PushConstantRange push_constant_range = {};
        push_constant_range.setSize(pcb.size);
        push_constant_range.setOffset(pcb.offset);
        push_constant_range.setStageFlags(vk::ShaderStageFlags(function->entry_point->shader_stage));
        push_constant_ranges.emplace_back(push_constant_range);
    }

    for (auto& sds : std::span(function->library->spvReflectShaderModule.descriptor_sets, function->library->spvReflectShaderModule.descriptor_set_count)) {
        if (sds.set >= descriptor_sets.size()) {
            descriptor_sets.resize(sds.set + 1);
        }

        for (auto& sb : std::span(sds.bindings, sds.binding_count)) {
            vk::DescriptorSetLayoutBinding binding = {};
            binding.setBinding(sb->binding);
            binding.setDescriptorType(vk::DescriptorType(sb->descriptor_type));
            binding.setDescriptorCount(sb->count);
            binding.setStageFlags(vk::ShaderStageFlags(function->entry_point->shader_stage));
            binding.setPImmutableSamplers(nullptr);

            descriptor_sets[sds.set].emplace(binding);
        }
    }

    auto state = MakeShared(new ComputePipelineState(MakeShared(this->retain())));
    state->bind_group_layouts.resize(descriptor_sets.size());
    for (uint32_t i = 0; i < descriptor_sets.size(); ++i) {
        vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};
        descriptor_set_layout_create_info.setBindings(descriptor_sets[i].bindings);

        state->bind_group_layouts[i] = raii.raw.createDescriptorSetLayout(descriptor_set_layout_create_info, nullptr, raii.dispatcher);
    }

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info = {};
    pipeline_layout_create_info.setSetLayouts(state->bind_group_layouts);
    pipeline_layout_create_info.setPushConstantRanges(push_constant_ranges);
    state->pipeline_layout = raii.raw.createPipelineLayout(pipeline_layout_create_info, nullptr, raii.dispatcher);

    vk::PipelineShaderStageCreateInfo shader_stage_create_info{};
    shader_stage_create_info.setStage(vk::ShaderStageFlagBits::eCompute);
    shader_stage_create_info.setModule(function->library->raw);
    shader_stage_create_info.setPName(function->name.c_str());

    vk::ComputePipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.setStage(shader_stage_create_info);
    pipeline_create_info.setLayout(state->pipeline_layout);
    pipeline_create_info.setBasePipelineHandle(nullptr);
    pipeline_create_info.setBasePipelineIndex(0);

    std::ignore = raii.raw.createComputePipelines({}, 1, &pipeline_create_info, nullptr, &state->pipeline, raii.dispatcher);
    return state;
}

auto gfx::Device::newCommandQueue() -> ManagedShared<CommandQueue> {
    vk::CommandPoolCreateInfo command_pool_create_info = {};
    command_pool_create_info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    command_pool_create_info.setQueueFamilyIndex(family_index);

    auto pool = raii.raw.createCommandPool(command_pool_create_info, nullptr, raii.dispatcher);

    return MakeShared(new CommandQueue(MakeShared(this->retain()), pool));
}

auto gfx::Device::createSwapchain(const ManagedShared<Surface>& surface) -> ManagedShared<Swapchain> {
    return MakeShared(new Swapchain(MakeShared(this->retain()), surface));
}