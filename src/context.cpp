#include "context.hpp"

#include "pass.hpp"
#include "mesh.hpp"
#include "queue.hpp"
#include "types.hpp"
#include "buffer.hpp"
#include "texture.hpp"
#include "material.hpp"

#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"
#include "spirv_reflect.h"

namespace vfx {
    // todo: fill table with valid aspect
    inline const auto image_aspect_flags_table = std::unordered_map<vk::Format, vk::ImageAspectFlags> {
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

    inline constexpr auto get_buffer_usage_from_target(BufferUsage target) -> vk::BufferUsageFlags {
        using Type = std::underlying_type_t<BufferUsage>;

        auto flags = vk::BufferUsageFlags{};
        if (static_cast<Type>(target) & static_cast<Type>(BufferUsage::Vertex)) {
            flags |= vk::BufferUsageFlagBits::eVertexBuffer;
        }
        if (static_cast<Type>(target) & static_cast<Type>(BufferUsage::Index)) {
            flags |= vk::BufferUsageFlagBits::eIndexBuffer;
        }
        if (static_cast<Type>(target) & static_cast<Type>(BufferUsage::CopySrc)) {
            flags |= vk::BufferUsageFlagBits::eTransferSrc;
        }
        if (static_cast<Type>(target) & static_cast<Type>(BufferUsage::CopyDst)) {
            flags |= vk::BufferUsageFlagBits::eTransferDst;
        }
        if (static_cast<Type>(target) & static_cast<Type>(BufferUsage::Constant)) {
            flags |= vk::BufferUsageFlagBits::eUniformBuffer;
        }
        return flags;
    }

    inline constexpr auto get_memory_usage_from_target(BufferUsage target) -> VmaMemoryUsage {
        using Type = std::underlying_type_t<BufferUsage>;

        auto flags = VMA_MEMORY_USAGE_CPU_TO_GPU;
        if (static_cast<Type>(target) & static_cast<Type>(BufferUsage::CopySrc)) {
            flags = VMA_MEMORY_USAGE_CPU_ONLY;
        }
        if (static_cast<Type>(target) & static_cast<Type>(BufferUsage::CopyDst)) {
            flags = VMA_MEMORY_USAGE_GPU_ONLY;
        }
        return flags;
    }

    inline auto get_supported_format(vk::PhysicalDevice device, std::span<const vk::Format> formats, vk::FormatFeatureFlags flags) -> vk::Format {
        for (auto format : formats) {
            const auto properties = device.getFormatProperties(format);
            if ((properties.optimalTilingFeatures & flags) == flags) {
                return format;
            }
        }
        for (auto format : formats) {
            const auto properties = device.getFormatProperties(format);
            if ((properties.linearTilingFeatures & flags) == flags) {
                return format;
            }
        }
        return vk::Format::eUndefined;
    }

    inline auto select_depth_format(vk::PhysicalDevice device) -> vk::Format {
        static constexpr auto formats = std::array{
            vk::Format::eD32SfloatS8Uint,
            vk::Format::eD24UnormS8Uint,
            vk::Format::eD32Sfloat
        };
        return get_supported_format(
            device,
            formats,
            vk::FormatFeatureFlagBits::eDepthStencilAttachment
        );
    }

    inline auto find_queue_families(vk::Instance instance, vk::PhysicalDevice device) -> std::optional<std::pair<u32, u32>> {
        const auto properties = device.getQueueFamilyProperties();

        u32 graphics_family = -1;
        u32 present_family = -1;
        for (u32 i = 0; i < u32(properties.size()); i++) {
            if (properties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                graphics_family = i;
            }

            if (glfwGetPhysicalDevicePresentationSupport(instance, device, i)) {
                present_family = i;
            }

            if ((graphics_family != -1) && (present_family != -1)) {
                return std::pair{ graphics_family, present_family };
            }
        }
        return std::nullopt;
    }

    inline VKAPI_ATTR auto VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData
    ) -> VkBool32 {
        static constexpr const char* skip[] = {
            "UNASSIGNED-BestPractices-NonSuccess-Result",
            "VUID-vkCmdPushConstants-offset-01796",
            "VUID-vkCmdBindPipeline-pipeline-06197"
        };

        for (const char* msg : skip) {
            if (strstr(pCallbackData->pMessage, msg) != nullptr) {
                return VK_FALSE;
            }
        }

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
            spdlog::debug("{}", pCallbackData->pMessage);
        } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            spdlog::info("{}", pCallbackData->pMessage);
        } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            spdlog::warn("{}", pCallbackData->pMessage);
        } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            spdlog::error("{}", pCallbackData->pMessage);
        } else {
            spdlog::info("{}", pCallbackData->pMessage);
        }
        return VK_FALSE;
    }
}

auto vfx::createSystemDefaultContext() -> Arc<Context> {
    return Arc<vfx::Context>::alloc(vfx::ContextDescription{
        .app_name = "Demo",
        .flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
        .extensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            "VK_EXT_metal_surface",
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        },
#ifndef NDEBUG
        .enableDebugUtils = true
#endif
    });
}

vfx::Context::Context(const vfx::ContextDescription& description) {
    create_instance(description);
    select_physical_device();
    create_logical_device();
    create_memory_allocator();
}

vfx::Context::~Context() {
    vmaDestroyAllocator(allocator);
    logical_device.destroy();

    if (debug_utils) {
        instance.destroyDebugUtilsMessengerEXT(debug_utils);
    }
    instance.destroy();
}

void vfx::Context::create_instance(const vfx::ContextDescription& description) {
    vk::defaultDispatchLoaderDynamic.init(dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

    auto application_info = vk::ApplicationInfo{
        .pApplicationName = description.app_name.c_str(),
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "Vulkan",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_2
    };

    auto enableApiValidation = [s = getenv("VFX_ENABLE_API_VALIDATION")] {
        return s && strcmp(s, "1") == 0;
    }();

    if (enableApiValidation) {
        auto layers = std::vector<const char*>{
            "VK_LAYER_KHRONOS_synchronization2",
            "VK_LAYER_KHRONOS_validation"
        };

        auto enabled_validation_features = std::array{
//            vk::ValidationFeatureEnableEXT::eGpuAssisted,
//            vk::ValidationFeatureEnableEXT::eBestPractices,
            vk::ValidationFeatureEnableEXT::eSynchronizationValidation,
        };
        auto validation_features = vk::ValidationFeaturesEXT{};
        validation_features.setEnabledValidationFeatures(enabled_validation_features);

        auto instance_create_info = vk::InstanceCreateInfo {};
        instance_create_info.setPNext(&validation_features);
        instance_create_info.setFlags(description.flags);
        instance_create_info.setPApplicationInfo(&application_info);
        instance_create_info.setPEnabledLayerNames(layers);
        instance_create_info.setPEnabledExtensionNames(description.extensions);
        instance = vk::createInstance(instance_create_info);
    } else {
        auto layers = std::vector<const char*>{
            "VK_LAYER_KHRONOS_synchronization2"
        };

        auto instance_create_info = vk::InstanceCreateInfo {};
        instance_create_info.setFlags(description.flags);
        instance_create_info.setPApplicationInfo(&application_info);
        instance_create_info.setPEnabledLayerNames(layers);
        instance_create_info.setPEnabledExtensionNames(description.extensions);
        instance = vk::createInstance(instance_create_info);
    }

    vk::defaultDispatchLoaderDynamic.init(instance);

    if (description.enableDebugUtils) {
        vk::DebugUtilsMessageSeverityFlagsEXT message_severity_flags{};
        message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
        message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
        message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
        message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

        vk::DebugUtilsMessageTypeFlagsEXT message_type_flags{};
        message_type_flags |= vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral;
        message_type_flags |= vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
        message_type_flags |= vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

        const auto debug_create_info = vk::DebugUtilsMessengerCreateInfoEXT{
            .messageSeverity = message_severity_flags,
            .messageType = message_type_flags,
            .pfnUserCallback = debug_callback
        };
        debug_utils = instance.createDebugUtilsMessengerEXT(debug_create_info);
    }
}

void vfx::Context::select_physical_device() {
    for (auto device : instance.enumeratePhysicalDevices()) {
        const auto families = find_queue_families(instance, device);
        if (!families) {
            continue;
        }

        physical_device = device;
        graphics_family = families->first;
        present_family = families->second;
        break;
    }
//    depthStencilFormat = select_depth_format(physical_device);
}

void vfx::Context::create_logical_device() {
    f32 queue_priority = 1.0f;
    auto queue_create_infos = std::vector<vk::DeviceQueueCreateInfo>{};

    const auto graphics_queue_create_info = vk::DeviceQueueCreateInfo {
        .queueFamilyIndex = graphics_family,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority
    };
    queue_create_infos.emplace_back(graphics_queue_create_info);

    if (graphics_family != present_family) {
        const auto present_queue_create_info = vk::DeviceQueueCreateInfo {
            .queueFamilyIndex = present_family,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority
        };

        queue_create_infos.emplace_back(present_queue_create_info);
    }

    static constexpr auto device_extensions = std::array{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef __APPLE__
        VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
#endif
        VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME,
        VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME,
        VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
        VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME
    };

    auto synchronization_2_features = vk::PhysicalDeviceSynchronization2Features{};
    synchronization_2_features.setSynchronization2(VK_TRUE);

    auto portability_subset_features = vk::PhysicalDevicePortabilitySubsetFeaturesKHR{};
    portability_subset_features.setPNext(&synchronization_2_features);
    portability_subset_features.setImageViewFormatSwizzle(VK_TRUE);

    auto timeline_semaphore_features = vk::PhysicalDeviceTimelineSemaphoreFeatures{};
    timeline_semaphore_features.setPNext(&portability_subset_features);
    timeline_semaphore_features.setTimelineSemaphore(VK_TRUE);

    auto dynamic_rendering_features = vk::PhysicalDeviceDynamicRenderingFeatures{};
    dynamic_rendering_features.setPNext(&timeline_semaphore_features);
    dynamic_rendering_features.setDynamicRendering(VK_TRUE);

    auto features = vk::PhysicalDeviceFeatures{};
    features.setFillModeNonSolid(VK_TRUE);
    features.setSamplerAnisotropy(VK_TRUE);
    features.setMultiViewport(VK_TRUE);

    const auto features_2 = vk::PhysicalDeviceFeatures2{
        .pNext = &dynamic_rendering_features,
        .features = features
    };

    auto device_create_info = vk::DeviceCreateInfo{};
    device_create_info.setPNext(&features_2);
    device_create_info.setQueueCreateInfos(queue_create_infos);
    device_create_info.setPEnabledExtensionNames(device_extensions);

    logical_device = physical_device.createDevice(device_create_info, nullptr);
    vk::defaultDispatchLoaderDynamic.init(logical_device);

    present_queue = logical_device.getQueue(present_family, 0);
    graphics_queue = logical_device.getQueue(graphics_family, 0);
}

void vfx::Context::create_memory_allocator() {
    const auto functions = VmaVulkanFunctions{
        .vkGetInstanceProcAddr = vk::defaultDispatchLoaderDynamic.vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vk::defaultDispatchLoaderDynamic.vkGetDeviceProcAddr,
        .vkGetPhysicalDeviceProperties = vk::defaultDispatchLoaderDynamic.vkGetPhysicalDeviceProperties,
        .vkGetPhysicalDeviceMemoryProperties = vk::defaultDispatchLoaderDynamic.vkGetPhysicalDeviceMemoryProperties,
        .vkAllocateMemory = vk::defaultDispatchLoaderDynamic.vkAllocateMemory,
        .vkFreeMemory = vk::defaultDispatchLoaderDynamic.vkFreeMemory,
        .vkMapMemory = vk::defaultDispatchLoaderDynamic.vkMapMemory,
        .vkUnmapMemory = vk::defaultDispatchLoaderDynamic.vkUnmapMemory,
        .vkFlushMappedMemoryRanges = vk::defaultDispatchLoaderDynamic.vkFlushMappedMemoryRanges,
        .vkInvalidateMappedMemoryRanges = vk::defaultDispatchLoaderDynamic.vkInvalidateMappedMemoryRanges,
        .vkBindBufferMemory = vk::defaultDispatchLoaderDynamic.vkBindBufferMemory,
        .vkBindImageMemory = vk::defaultDispatchLoaderDynamic.vkBindImageMemory,
        .vkGetBufferMemoryRequirements = vk::defaultDispatchLoaderDynamic.vkGetBufferMemoryRequirements,
        .vkGetImageMemoryRequirements = vk::defaultDispatchLoaderDynamic.vkGetImageMemoryRequirements,
        .vkCreateBuffer = vk::defaultDispatchLoaderDynamic.vkCreateBuffer,
        .vkDestroyBuffer = vk::defaultDispatchLoaderDynamic.vkDestroyBuffer,
        .vkCreateImage = vk::defaultDispatchLoaderDynamic.vkCreateImage,
        .vkDestroyImage = vk::defaultDispatchLoaderDynamic.vkDestroyImage,
        .vkCmdCopyBuffer = vk::defaultDispatchLoaderDynamic.vkCmdCopyBuffer,
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
        /// Fetch "vkGetBufferMemoryRequirements2" on Vulkan >= 1.1, fetch "vkGetBufferMemoryRequirements2KHR" when using VK_KHR_dedicated_allocation extension.
        .vkGetBufferMemoryRequirements2KHR = vk::defaultDispatchLoaderDynamic.vkGetBufferMemoryRequirements2KHR,
        /// Fetch "vkGetImageMemoryRequirements 2" on Vulkan >= 1.1, fetch "vkGetImageMemoryRequirements2KHR" when using VK_KHR_dedicated_allocation extension.
        .vkGetImageMemoryRequirements2KHR = vk::defaultDispatchLoaderDynamic.vkGetImageMemoryRequirements2KHR,
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
        /// Fetch "vkBindBufferMemory2" on Vulkan >= 1.1, fetch "vkBindBufferMemory2KHR" when using VK_KHR_bind_memory2 extension.
        .vkBindBufferMemory2KHR = vk::defaultDispatchLoaderDynamic.vkBindBufferMemory2KHR,
        /// Fetch "vkBindImageMemory2" on Vulkan >= 1.1, fetch "vkBindImageMemory2KHR" when using VK_KHR_bind_memory2 extension.
        .vkBindImageMemory2KHR = vk::defaultDispatchLoaderDynamic.vkBindImageMemory2KHR,
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
        .vkGetPhysicalDeviceMemoryProperties2KHR = vk::defaultDispatchLoaderDynamic.vkGetPhysicalDeviceMemoryProperties2KHR,
#endif
    };

    const auto allocatorCreateInfo = VmaAllocatorCreateInfo{
        .physicalDevice = physical_device,
        .device = logical_device,
        .pVulkanFunctions = &functions,
        .instance = instance,
        .vulkanApiVersion = VK_API_VERSION_1_2
    };

    vmaCreateAllocator(&allocatorCreateInfo, &allocator);
}

auto vfx::Context::makeRenderPass(const vfx::RenderPassDescription& description) -> Arc<RenderPass> {
    std::vector<vk::SubpassDescription> subpasses{};
    subpasses.resize(description.definitions.size());

    for (u64 i = 0; i < description.definitions.size(); ++i) {
        subpasses[i].flags = {};
        subpasses[i].pipelineBindPoint = description.definitions[i].pipelineBindPoint;
        if (!description.definitions[i].inputAttachments.empty()) {
            subpasses[i].setInputAttachments(description.definitions[i].inputAttachments);
        }
        if (!description.definitions[i].colorAttachments.empty()) {
            subpasses[i].setColorAttachments(description.definitions[i].colorAttachments);
        }
        if (!description.definitions[i].resolveAttachments.empty()) {
            subpasses[i].setResolveAttachments(description.definitions[i].resolveAttachments);
        }
        if (description.definitions[i].depthStencilAttachment.has_value()) {
            subpasses[i].setPDepthStencilAttachment(&*description.definitions[i].depthStencilAttachment);
        }
        if (!description.definitions[i].preserveAttachments.empty()) {
            subpasses[i].setPreserveAttachments(description.definitions[i].preserveAttachments);
        }
    }

    auto create_info = vk::RenderPassCreateInfo{};
    create_info.setSubpasses(subpasses);
    create_info.setAttachments(description.attachments.elements);
    create_info.setDependencies(description.dependencies);

    auto out = Box<RenderPass>::alloc();
    out->context = this;
    out->handle = logical_device.createRenderPass(create_info);
    return out;
}

void vfx::Context::freeRenderPass(RenderPass* pass) {
    logical_device.destroyRenderPass(pass->handle);
}

auto vfx::Context::makeTexture(const vfx::TextureDescription& description) -> Arc<Texture> {
    const auto image_create_info = static_cast<VkImageCreateInfo>(vk::ImageCreateInfo{
        .imageType = vk::ImageType::e2D,
        .format = description.format,
        .extent = {
            .width = description.width,
            .height = description.height,
            .depth = 1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .usage = description.usage
    });

    VkImage image;
    VmaAllocation allocation;

    const auto allocation_create_info = VmaAllocationCreateInfo{.usage = VMA_MEMORY_USAGE_AUTO};
    vmaCreateImage(allocator, &image_create_info, &allocation_create_info, &image, &allocation, nullptr);

    const auto image_aspect_flags = image_aspect_flags_table.at(description.format);

    const auto view_create_info = vk::ImageViewCreateInfo{
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = description.format,
        .components = description.components,
        .subresourceRange = vk::ImageSubresourceRange{
            .aspectMask     = image_aspect_flags,
            .levelCount     = 1,
            .layerCount     = 1
        }
    };
    auto view = logical_device.createImageView(view_create_info);

    auto out = Box<Texture>::alloc();
    out->context = this;
    out->size.width = description.width;
    out->size.height = description.height;
    out->format = description.format;
    out->image = image;
    out->view = view;
    out->allocation = allocation;
    out->aspect = image_aspect_flags;
    return out;
}

void vfx::Context::freeTexture(Texture* texture) {
    logical_device.destroyImageView(texture->view);
    if (texture->allocation != nullptr) {
        vmaDestroyImage(allocator, texture->image, texture->allocation);
    }
}

auto vfx::Context::makeSampler(const vk::SamplerCreateInfo& info) -> Arc<Sampler> {
    auto out = Arc<Sampler>::alloc();
    out->context = this;
    out->handle = logical_device.createSampler(info);
    return out;
}

void vfx::Context::freeSampler(Sampler* sampler) {
    logical_device.destroySampler(sampler->handle);
}

auto vfx::Context::makeBuffer(vfx::BufferUsage target, u64 size) -> Arc<Buffer> {
    const auto buffer_create_info = static_cast<VkBufferCreateInfo>(vk::BufferCreateInfo {
        .size = static_cast<vk::DeviceSize>(size),
        .usage = get_buffer_usage_from_target(target)
    });

    const auto allocation_create_info = VmaAllocationCreateInfo {
        .usage = get_memory_usage_from_target(target)
    };

    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocation_info;

    vmaCreateBuffer(
        allocator,
        &buffer_create_info,
        &allocation_create_info,
        &buffer,
        &allocation,
        &allocation_info
    );
    auto out = Box<Buffer>::alloc();
    out->context = this;
    out->handle = buffer;
    out->allocation = allocation;
    out->allocationInfo = allocation_info;
    out->allocationSize = size;
    return out;
}

void vfx::Context::freeBuffer(Buffer* buffer) {
    vmaDestroyBuffer(allocator, buffer->handle, buffer->allocation);
}

auto vfx::Context::makeLibrary(const std::vector<char>& bytes) -> Arc<Library> {
    auto module_create_info = vk::ShaderModuleCreateInfo{
        .codeSize = bytes.size(),
        .pCode    = reinterpret_cast<const u32 *>(bytes.data())
    };
    auto module = logical_device.createShaderModule(module_create_info);

    auto out = Arc<Library>::alloc();
    out->context = this;
    out->module = module;

    spvReflectCreateShaderModule(
        bytes.size(),
        bytes.data(),
        &out->reflect
    );

    return out;
}

void vfx::Context::freeLibrary(Library* library) {
    logical_device.destroyShaderModule(library->module);
    spvReflectDestroyShaderModule(&library->reflect);
}

auto vfx::Context::makePipelineState(const vfx::PipelineStateDescription& description) -> Arc<PipelineState> {
    auto out = Box<PipelineState>::alloc();
    out->context = this;
//    out->description = description;

    struct DescriptorSetDescription {
        std::vector<vk::DescriptorSetLayoutBinding> bindings{};
    };

    std::vector<vk::PushConstantRange> constant_ranges{};
    std::vector<DescriptorSetDescription> descriptor_set_descriptions{};

    auto addShaderModule = [&](const Arc<Function>& function) {
        auto stage_flags = vk::ShaderStageFlagBits(function->library->reflect.shader_stage);

        auto refl_constant_blocks = std::span(
            function->library->reflect.push_constant_blocks,
            function->library->reflect.push_constant_block_count
        );

        constant_ranges.reserve(refl_constant_blocks.size());
        for (auto& refl_block : refl_constant_blocks) {
            auto& constant_range = constant_ranges.emplace_back();

            constant_range.setSize(refl_block.size);
            constant_range.setOffset(refl_block.offset);
            constant_range.setStageFlags(stage_flags);
        }

        auto refl_descriptor_sets = std::span(
            function->library->reflect.descriptor_sets,
            function->library->reflect.descriptor_set_count
        );

        for (auto& refl_set : refl_descriptor_sets) {
            if (refl_set.set >= descriptor_set_descriptions.size()) {
                descriptor_set_descriptions.resize(refl_set.set + 1);
            }

            auto refl_descriptor_bindings = std::span(
                refl_set.bindings,
                refl_set.binding_count
            );

            for (auto& refl_binding : refl_descriptor_bindings) {
                auto binding = vk::DescriptorSetLayoutBinding{
                    .binding = refl_binding->binding,
                    .descriptorType = vk::DescriptorType(refl_binding->descriptor_type),
                    .descriptorCount = refl_binding->count,
                    .stageFlags = stage_flags,
                    .pImmutableSamplers = nullptr
                };
                descriptor_set_descriptions.at(refl_set.set).bindings.emplace_back(binding);
            }
        }
    };

    if (description.vertexFunction) {
        addShaderModule(description.vertexFunction);
    }

    if (description.fragmentFunction) {
        addShaderModule(description.fragmentFunction);
    }

    out->descriptorSetLayouts.resize(descriptor_set_descriptions.size());
    for (u32 i = 0; i < out->descriptorSetLayouts.size(); ++i) {
        auto dsl_create_info = vk::DescriptorSetLayoutCreateInfo{};
        dsl_create_info.setBindings(descriptor_set_descriptions[i].bindings);
        out->descriptorSetLayouts[i] = logical_device.createDescriptorSetLayout(dsl_create_info);
    }

    auto pipeline_layout_create_info = vk::PipelineLayoutCreateInfo{};
    pipeline_layout_create_info.setSetLayouts(out->descriptorSetLayouts);
    pipeline_layout_create_info.setPushConstantRanges(constant_ranges);
    out->pipelineLayout = logical_device.createPipelineLayout(pipeline_layout_create_info);

    {
        vk::PipelineViewportStateCreateInfo viewportState = {};
        viewportState.setViewportCount(1);
        viewportState.setScissorCount(1);

        std::array dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };

        vk::PipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.setDynamicStates(dynamicStates);

        std::vector<vk::PipelineShaderStageCreateInfo> stages = {};

        if (description.vertexFunction) {
            vk::PipelineShaderStageCreateInfo info{};
            info.setStage(vk::ShaderStageFlagBits::eVertex);
            info.setModule(description.vertexFunction->library->module);
            info.setPName(description.vertexFunction->name.c_str());
            stages.emplace_back(info);
        }

        if (description.fragmentFunction) {
            vk::PipelineShaderStageCreateInfo info{};
            info.setStage(vk::ShaderStageFlagBits::eFragment);
            info.setModule(description.fragmentFunction->library->module);
            info.setPName(description.fragmentFunction->name.c_str());
            stages.emplace_back(info);
        }

        vk::PipelineVertexInputStateCreateInfo vertexInputState = {};
        if (description.vertexDescription.has_value()) {
            vertexInputState.setVertexBindingDescriptions(description.vertexDescription->layouts.elements);
            vertexInputState.setVertexAttributeDescriptions(description.vertexDescription->attributes.elements);
        }

        vk::PipelineColorBlendStateCreateInfo colorBlendState = {};
        colorBlendState.setAttachments(description.attachments.elements);

        vk::PipelineRenderingCreateInfo rendering = {};
        rendering.setViewMask(description.viewMask);
        rendering.setColorAttachmentFormats(description.colorAttachmentFormats.elements);
        rendering.setDepthAttachmentFormat(description.depthAttachmentFormat);
        rendering.setStencilAttachmentFormat(description.stencilAttachmentFormat);

        auto pipeline_create_info = vk::GraphicsPipelineCreateInfo{};
        pipeline_create_info.setPNext(&rendering);
        pipeline_create_info.setStages(stages);
        pipeline_create_info.setPVertexInputState(&vertexInputState);
        pipeline_create_info.setPInputAssemblyState(&description.inputAssemblyState);
        pipeline_create_info.setPViewportState(&viewportState);
        pipeline_create_info.setPRasterizationState(&description.rasterizationState);
        pipeline_create_info.setPMultisampleState(&description.multisampleState);
        pipeline_create_info.setPDepthStencilState(&description.depthStencilState);
        pipeline_create_info.setPColorBlendState(&colorBlendState);
        pipeline_create_info.setPDynamicState(&dynamicState);
        pipeline_create_info.setLayout(out->pipelineLayout);
        pipeline_create_info.setRenderPass(VK_NULL_HANDLE);
        pipeline_create_info.setSubpass(0);
        pipeline_create_info.setBasePipelineHandle(nullptr);
        pipeline_create_info.setBasePipelineIndex(0);

        vk::Pipeline pipeline{};
        std::ignore = logical_device.createGraphicsPipelines(
            {},
            1,
            &pipeline_create_info,
            nullptr,
            &pipeline
        );

        out->pipeline = pipeline;
    }

    return out;
}

void vfx::Context::freePipelineState(PipelineState* pipelineState) {
    for (auto& layout : pipelineState->descriptorSetLayouts) {
        logical_device.destroyDescriptorSetLayout(layout);
    }

    logical_device.destroyPipelineLayout(pipelineState->pipelineLayout);
    logical_device.destroyPipeline(pipelineState->pipeline);
}

auto vfx::Context::makeCommandQueue(u32 count) -> Arc<CommandQueue> {
    auto out = Box<CommandQueue>::alloc();
    out->context = this;
    out->handle = logical_device.getQueue(graphics_family, 0);

    const auto pool_create_info = vk::CommandPoolCreateInfo {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = graphics_family
    };
    out->pool = logical_device.createCommandPool(pool_create_info);

    const auto command_buffers_allocate_info = vk::CommandBufferAllocateInfo{
        .commandPool = out->pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = count
    };
    out->rawCommandBuffers = logical_device.allocateCommandBuffers(command_buffers_allocate_info);

    out->fences.resize(count);
    out->semaphores.resize(count);
    out->commandBuffers.resize(count);
    for (size_t i = 0; i < count; ++i) {
        vk::FenceCreateInfo fence_create_info{};
        fence_create_info.setFlags(vk::FenceCreateFlagBits::eSignaled);
        out->fences[i] = logical_device.createFence(fence_create_info);

        vk::SemaphoreCreateInfo semaphore_create_info{};
        out->semaphores[i] = logical_device.createSemaphore(semaphore_create_info);

        out->commandBuffers[i].context = this;
        out->commandBuffers[i].commandQueue = &*out;
        out->commandBuffers[i].fence = out->fences[i];
        out->commandBuffers[i].handle = out->rawCommandBuffers[i];
        out->commandBuffers[i].semaphore = out->semaphores[i];
    }
    return out;
}

void vfx::Context::freeCommandQueue(CommandQueue* queue) {
    for (auto& commandBuffer : queue->commandBuffers) {
        commandBuffer.reset();
    }

    for (auto& semaphore : queue->semaphores) {
        logical_device.destroySemaphore(semaphore);
    }

    for (auto& fence : queue->fences) {
        logical_device.destroyFence(fence);
    }

    logical_device.freeCommandBuffers(queue->pool, queue->rawCommandBuffers);
    logical_device.destroyCommandPool(queue->pool);
}