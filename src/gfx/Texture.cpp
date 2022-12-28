#include "Buffer.hpp"
#include "Device.hpp"
#include "Texture.hpp"
#include "CommandQueue.hpp"
#include "CommandBuffer.hpp"

#include <unordered_map>

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

gfx::Texture::Texture(SharedPtr<Device> device) : mDevice(std::move(device)) {}

gfx::Texture::Texture(SharedPtr<Device> device, const TextureDescription& description) : Texture(std::move(device)) {
    vkFormat = description.format;
    vkExtent.setWidth(description.width);
    vkExtent.setHeight(description.height);
    vkExtent.setDepth(1);
    vkImageSubresourceRange.setAspectMask(image_aspect_flags_table.at(description.format));
    vkImageSubresourceRange.setLevelCount(1);
    vkImageSubresourceRange.setLayerCount(1);

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
    vmaCreateImage(mDevice->vmaAllocator, reinterpret_cast<const VkImageCreateInfo*>(&image_create_info), &allocation_create_info, reinterpret_cast<VkImage*>(&vkImage), &vmaAllocation, nullptr);

    vk::ImageViewCreateInfo view_create_info = {};
    view_create_info.setImage(vkImage);
    view_create_info.setViewType(vk::ImageViewType::e2D),
    view_create_info.setFormat(description.format);
    view_create_info.setComponents(description.components),
    view_create_info.setSubresourceRange(vkImageSubresourceRange);

    vkImageView = mDevice->vkDevice.createImageView(view_create_info, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);
}

gfx::Texture::~Texture() {
    mDevice->vkDevice.destroyImageView(vkImageView, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);
    if (vmaAllocation) {
        vmaDestroyImage(mDevice->vmaAllocator, vkImage, vmaAllocation);
    }
}

void gfx::Texture::replaceRegion(const void* data, uint64_t size) {
    auto StorageBuffer = mDevice->newBuffer(
        vk::BufferUsageFlagBits::eTransferSrc,
        size, data,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
    );
    vk::BufferImageCopy buffer_image_copy = {};
    buffer_image_copy.setImageExtent(vkExtent);
    buffer_image_copy.imageSubresource.setAspectMask(vkImageSubresourceRange.aspectMask);
    buffer_image_copy.imageSubresource.setLayerCount(1);

    auto CommandQueue = mDevice->newCommandQueue();
    auto CommandBuffer = CommandQueue->commandBufferWithUnretainedReferences();

    CommandBuffer->begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    CommandBuffer->changeTextureLayout(RetainPtr(this), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, vk::PipelineStageFlagBits2::eHost, vk::PipelineStageFlagBits2::eTransfer, {}, vk::AccessFlagBits2::eTransferWrite);
    CommandBuffer->vkCommandBuffer.copyBufferToImage(StorageBuffer->vkBuffer, vkImage, vk::ImageLayout::eTransferDstOptimal, 1, &buffer_image_copy, mDevice->vkDispatchLoaderDynamic);
    CommandBuffer->changeTextureLayout(RetainPtr(this), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits2::eTransfer, vk::PipelineStageFlagBits2::eFragmentShader, vk::AccessFlagBits2::eTransferWrite, vk::AccessFlagBits2::eShaderRead);
    CommandBuffer->end();
    CommandBuffer->submit();
    CommandBuffer->waitUntilCompleted();
}

void gfx::Texture::setLabel(const std::string& name) {
    vk::DebugMarkerObjectNameInfoEXT image_info = {};
    image_info.setObjectType(vk::DebugReportObjectTypeEXT::eImage);
    image_info.setObject(uint64_t(static_cast<VkImage>(vkImage)));
    image_info.setPObjectName(name.c_str());

    mDevice->vkDevice.debugMarkerSetObjectNameEXT(image_info, mDevice->vkDispatchLoaderDynamic);

    vk::DebugMarkerObjectNameInfoEXT view_info = {};
    view_info.setObjectType(vk::DebugReportObjectTypeEXT::eImageView);
    view_info.setObject(uint64_t(static_cast<VkImageView>(vkImageView)));
    view_info.setPObjectName(name.c_str());

    mDevice->vkDevice.debugMarkerSetObjectNameEXT(view_info, mDevice->vkDispatchLoaderDynamic);
}
