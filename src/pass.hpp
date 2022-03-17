#pragma once

#include <map>
#include <vector>
#include <types.hpp>
#include <context.hpp>
#include <glm/vec4.hpp>
#include <tl/optional.hpp>

namespace vfx {
    struct SubpassDescription {
        vk::PipelineBindPoint pipelineBindPoint{};
        std::vector<vk::AttachmentReference> pInputAttachments{};
        std::vector<vk::AttachmentReference> pColorAttachments{};
        std::vector<vk::AttachmentReference> pResolveAttachments{};
        tl::optional<vk::AttachmentReference> pDepthStencilAttachment{};
        std::vector<u32> pPreserveAttachments{};
    };

    struct RenderPass {
        vfx::Context& context;
        vk::RenderPass handle;

        explicit RenderPass(vfx::Context& context) : context(context) {}

        void init(const std::vector<vk::AttachmentDescription>& attachments, const std::vector<SubpassDescription>& definitions, const std::vector<vk::SubpassDependency>& dependencies) {
            std::vector<vk::SubpassDescription> subpasses{};
            subpasses.resize(definitions.size());

            for (u64 i = 0; i < definitions.size(); ++i) {
                subpasses[i].flags = {};
                subpasses[i].pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
                if (!definitions[i].pInputAttachments.empty()) {
                    subpasses[i].setInputAttachments(definitions[i].pInputAttachments);
                }
                if (!definitions[i].pColorAttachments.empty()) {
                    subpasses[i].setColorAttachments(definitions[i].pColorAttachments);
                }
                if (!definitions[i].pResolveAttachments.empty()) {
                    subpasses[i].setResolveAttachments(definitions[i].pResolveAttachments);
                }
                if (definitions[i].pDepthStencilAttachment.has_value()) {
                    subpasses[i].setPDepthStencilAttachment(&*definitions[i].pDepthStencilAttachment);
                }
                if (!definitions[i].pPreserveAttachments.empty()) {
                    subpasses[i].setPreserveAttachments(definitions[i].pPreserveAttachments);
                }
            }

            auto create_info = vk::RenderPassCreateInfo{};
            create_info.setSubpasses(subpasses);
            create_info.setAttachments(attachments);
            create_info.setDependencies(dependencies);
            handle = context.logical_device.createRenderPass(create_info);
        }

        ~RenderPass() {
            context.logical_device.destroyRenderPass(handle);
        }
    };
}