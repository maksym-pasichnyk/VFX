#pragma once

#include "types.hpp"

#include <vector>
#include <optional>
#include <vulkan/vulkan.hpp>

namespace vfx {
    struct SubpassDescription {
        vk::PipelineBindPoint pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        std::vector<vk::AttachmentReference> inputAttachments{};
        std::vector<vk::AttachmentReference> colorAttachments{};
        std::vector<vk::AttachmentReference> resolveAttachments{};
        std::optional<vk::AttachmentReference> depthStencilAttachment{};
        std::vector<u32> preserveAttachments{};
    };

    struct AttachmentDescriptionArray {
        std::vector<vk::AttachmentDescription> elements{};

        auto operator[](size_t i) -> vk::AttachmentDescription& {
            if (elements.size() >= i) {
                elements.resize(i + 1, vk::AttachmentDescription{});
            }
            return elements[i];
        }
    };

    struct RenderPassDescription {
        std::vector<SubpassDescription> definitions;
        std::vector<vk::SubpassDependency> dependencies;

        AttachmentDescriptionArray attachments{};
    };

    struct Device;
    struct RenderPass {
    public:
        Device* device{};
        vk::RenderPass handle{};

    public:
        RenderPass();
        ~RenderPass();
    };
}