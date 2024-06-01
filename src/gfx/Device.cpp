#include "Device.hpp"
#include "Buffer.hpp"
#include "Adapter.hpp"
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
gfx::Device::Device(rc<Adapter> adapter, vk::DeviceCreateInfo const& create_info)
: adapter(std::move(adapter))
, handle(this->adapter->handle.createDevice(create_info, nullptr, this->adapter->instance->dispatcher))
, dispatcher(this->adapter->instance->dispatcher.vkGetDeviceProcAddr, this->handle)
, allocator() {
    VmaVulkanFunctions functions = {};
    functions.vkGetDeviceProcAddr   = this->adapter->instance->dispatcher.vkGetDeviceProcAddr;
    functions.vkGetInstanceProcAddr = this->adapter->instance->dispatcher.vkGetInstanceProcAddr;

    VmaAllocatorCreateInfo allocator_create_info = {};
    allocator_create_info.physicalDevice = this->adapter->handle;
    allocator_create_info.device = this->handle;
    allocator_create_info.pVulkanFunctions = &functions;
    allocator_create_info.instance = this->adapter->instance->handle;
    allocator_create_info.vulkanApiVersion = VK_API_VERSION_1_2;
    vk::resultCheck(static_cast<vk::Result>(vmaCreateAllocator(&allocator_create_info, &allocator)), "Failed to create allocator");
}

gfx::Device::~Device() {
    vmaDestroyAllocator(allocator);
    this->handle.destroy(nullptr, this->dispatcher);
}

void gfx::Device::waitIdle(this Device& self) {
    self.handle.waitIdle(self.dispatcher);
}

auto gfx::Device::newTexture(this Device& self, TextureDescription const& description) -> rc<Texture> {
    auto aspect = image_aspect_flags_table.at(description.format);

    vk::ImageCreateInfo image_create_info = {};
    image_create_info.setImageType(vk::ImageType::e2D);
    image_create_info.setFormat(description.format);
    image_create_info.setExtent(vk::Extent3D(description.width, description.height, 1));
    image_create_info.setMipLevels(1);
    image_create_info.setArrayLayers(1);
    image_create_info.setUsage(description.usage);

    VmaAllocationCreateInfo allocation_create_info = {};
    allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;

    VkImage image;
    VmaAllocation allocation;
    vmaCreateImage(self.allocator, reinterpret_cast<const VkImageCreateInfo*>(&image_create_info), &allocation_create_info, reinterpret_cast<VkImage*>(&image), &allocation, nullptr);

    vk::ImageViewCreateInfo view_create_info = {};
    view_create_info.setImage(image);
    view_create_info.setViewType(vk::ImageViewType::e2D),
    view_create_info.setFormat(description.format);
    view_create_info.setComponents(description.mapping),
    view_create_info.setSubresourceRange(vk::ImageSubresourceRange(aspect, 0, 1, 0, 1));

    return rc<Texture>(new Texture(
        self.shared_from_this(),
        image,
        description.format,
        vk::Extent3D(
            description.width,
            description.height,
            1
        ),
        self.handle.createImageView(view_create_info, VK_NULL_HANDLE, self.dispatcher),
        vk::ImageSubresourceRange(aspect, 0, 1, 0, 1),
        allocation
    ));
}

auto gfx::Device::newSampler(this Device& self, vk::SamplerCreateInfo const& info) -> rc<Sampler> {
    return rc<Sampler>(new Sampler(self.shared_from_this(), info));
}

auto gfx::Device::newBuffer(this Device& self, vk::BufferUsageFlags usage, uint64_t size, StorageMode storage, VmaAllocationCreateFlags options) -> rc<Buffer> {
    vk::BufferCreateInfo buffer_create_info = {};
    buffer_create_info.setSize(static_cast<vk::DeviceSize>(size));
    buffer_create_info.setUsage(usage);

    VmaAllocationCreateInfo allocation_create_info = {};
    allocation_create_info.flags = options;
    allocation_create_info.usage = VMA_MEMORY_USAGE_UNKNOWN;
    switch (storage) {
        case StorageMode::ePrivate: {
            allocation_create_info.requiredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            allocation_create_info.preferredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            break;
        }
        case StorageMode::eManaged: {
            allocation_create_info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
            allocation_create_info.requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            allocation_create_info.preferredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            allocation_create_info.preferredFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
            break;
        }
        case StorageMode::eShared: {
            allocation_create_info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
            allocation_create_info.requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            allocation_create_info.requiredFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            allocation_create_info.preferredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            allocation_create_info.preferredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            allocation_create_info.preferredFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            break;
        }
        case StorageMode::eLazy: {
            allocation_create_info.requiredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            allocation_create_info.requiredFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
            allocation_create_info.preferredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            allocation_create_info.preferredFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
            break;
        }
    }

    VkBuffer buffer;
    VmaAllocation allocation;
    vmaCreateBuffer(self.allocator, reinterpret_cast<const VkBufferCreateInfo*>(&buffer_create_info), &allocation_create_info, &buffer, &allocation, nullptr);
    return rc<Buffer>(new Buffer(self.shared_from_this(), buffer, allocation));
}

auto gfx::Device::newBuffer(this Device& self, vk::BufferUsageFlags usage, const void* pointer, uint64_t size, StorageMode storage, VmaAllocationCreateFlags options) -> rc<Buffer> {
    // todo: use transfer operation if buffer is not mappable
    auto buffer = self.newBuffer(usage, size, storage, options);
    std::memcpy(buffer->contents(), pointer, size);
    buffer->didModifyRange(0, size);
    return buffer;
}

auto gfx::Device::newLibrary(this Device& self, std::span<char const> bytes) -> rc<Library> {
    vk::ShaderModuleCreateInfo create_info = {};
    create_info.setCodeSize(bytes.size());
    create_info.setPCode(reinterpret_cast<const uint32_t *>(bytes.data()));
    return rc<Library>(new Library(self.shared_from_this(), create_info));
}

auto gfx::Device::newDepthStencilState(this Device& self, DepthStencilStateDescription const& description) -> rc<DepthStencilState> {
    auto depth_stencil_state = rc<DepthStencilState>(new DepthStencilState());
    depth_stencil_state->isDepthTestEnabled = description.isDepthTestEnabled;
    depth_stencil_state->isDepthWriteEnabled = description.isDepthWriteEnabled;
    depth_stencil_state->depthCompareFunction = description.depthCompareFunction;
    depth_stencil_state->isDepthBoundsTestEnabled = description.depth_bounds_test_enable;
    depth_stencil_state->isStencilTestEnabled = description.stencil_test_enable;
    depth_stencil_state->frontFaceStencil = description.frontFaceStencil;
    depth_stencil_state->backFaceStencil = description.backFaceStencil;
    depth_stencil_state->minDepthBounds = description.min_depth_bounds;
    depth_stencil_state->maxDepthBounds = description.max_depth_bounds;
    return depth_stencil_state;
}

auto gfx::Device::newRenderPipelineState(this Device& self, rc<RenderPipelineStateDescription> const& description) -> rc<RenderPipelineState> {
    std::vector<vk::PushConstantRange> push_constant_ranges = {};
    std::vector<DescriptorSetLayoutCreateInfo> descriptor_sets = {};

    auto func = [&](gfx::Function* impl) {
        for (auto& pcb : std::span(impl->library->spvReflectShaderModule.push_constant_blocks, impl->library->spvReflectShaderModule.push_constant_block_count)) {
            vk::PushConstantRange push_constant_range = {};
            push_constant_range.setSize(pcb.size);
            push_constant_range.setOffset(pcb.offset);
            push_constant_range.setStageFlags(vk::ShaderStageFlags(impl->entry_point->shader_stage));
            push_constant_ranges.emplace_back(push_constant_range);
        }

        for (auto& sds : std::span(impl->library->spvReflectShaderModule.descriptor_sets, impl->library->spvReflectShaderModule.descriptor_set_count)) {
            if (sds.set >= descriptor_sets.size()) {
                descriptor_sets.resize(sds.set + 1);
            }

            for (auto& sb : std::span(sds.bindings, sds.binding_count)) {
                vk::DescriptorSetLayoutBinding binding = {};
                binding.setBinding(sb->binding);
                binding.setDescriptorType(vk::DescriptorType(sb->descriptor_type));
                binding.setDescriptorCount(sb->count);
                binding.setStageFlags(vk::ShaderStageFlags(impl->entry_point->shader_stage));
                binding.setPImmutableSamplers(nullptr);

                descriptor_sets[sds.set].emplace(binding);
            }
        }
    };

    func(&*description->getVertexFunction());
    func(&*description->getFragmentFunction());

    auto state = rc<RenderPipelineState>(new RenderPipelineState(self.shared_from_this(), description));
    vk::PipelineCacheCreateInfo pipeline_cache_info = {};
    vk::resultCheck(self.handle.createPipelineCache(&pipeline_cache_info, nullptr, &state->pipelineCache, self.dispatcher), "Failed to create pipeline cache");

    state->descriptorSetLayouts.resize(descriptor_sets.size());
    for (uint32_t i = 0; i < state->descriptorSetLayouts.size(); ++i) {
        vk::DescriptorSetLayoutCreateInfo layout_create_info = {};
        layout_create_info.setBindings(descriptor_sets[i].bindings);
        state->descriptorSetLayouts[i] = self.handle.createDescriptorSetLayout(layout_create_info, nullptr, self.dispatcher);
    }

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info = {};
    pipeline_layout_create_info.setSetLayouts(state->descriptorSetLayouts);
    pipeline_layout_create_info.setPushConstantRanges(push_constant_ranges);
    state->pipelineLayout = self.handle.createPipelineLayout(pipeline_layout_create_info, nullptr, self.dispatcher);

    return state;
}

auto gfx::Device::newComputePipelineState(this Device& self, rc<Function> const& function) -> rc<ComputePipelineState> {
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

    auto state = rc<ComputePipelineState>(new ComputePipelineState(self.shared_from_this()));
    state->descriptor_set_layouts.resize(descriptor_sets.size());
    for (uint32_t i = 0; i < descriptor_sets.size(); ++i) {
        vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};
        descriptor_set_layout_create_info.setBindings(descriptor_sets[i].bindings);

        state->descriptor_set_layouts[i] = self.handle.createDescriptorSetLayout(descriptor_set_layout_create_info, nullptr, self.dispatcher);
    }

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info = {};
    pipeline_layout_create_info.setSetLayouts(state->descriptor_set_layouts);
    pipeline_layout_create_info.setPushConstantRanges(push_constant_ranges);
    state->pipeline_layout = self.handle.createPipelineLayout(pipeline_layout_create_info, nullptr, self.dispatcher);

    vk::PipelineShaderStageCreateInfo shader_stage_create_info = {};
    shader_stage_create_info.setStage(vk::ShaderStageFlagBits::eCompute);
    shader_stage_create_info.setModule(function->library->handle);
    shader_stage_create_info.setPName(function->name.c_str());

    vk::ComputePipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.setStage(shader_stage_create_info);
    pipeline_create_info.setLayout(state->pipeline_layout);
    pipeline_create_info.setBasePipelineHandle(nullptr);
    pipeline_create_info.setBasePipelineIndex(0);

    vk::resultCheck(self.handle.createComputePipelines({}, 1, &pipeline_create_info, nullptr, &state->pipeline, self.dispatcher), "Failed to create compute pipeline");
    return state;
}

auto gfx::Device::newCommandQueue(this Device& self) -> rc<CommandQueue> {
    vk::CommandPoolCreateInfo create_info = {};
    create_info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    create_info.setQueueFamilyIndex(0);

    vk::CommandPool command_pool = self.handle.createCommandPool(create_info, nullptr, self.dispatcher);
    return rc<CommandQueue>(new CommandQueue(self.shared_from_this(), command_pool));
}

auto gfx::Device::createSwapchain(this Device& self, rc<Surface> const& surface) -> rc<Swapchain> {
    return rc<Swapchain>(new Swapchain(self.shared_from_this(), surface));
}