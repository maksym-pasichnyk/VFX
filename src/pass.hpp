#pragma once

#include <map>
#include <vector>
#include <types.hpp>
#include <context.hpp>
#include <glm/vec4.hpp>
#include <tl/optional.hpp>

namespace vfx {
    struct SubpassDescription {
        vk::PipelineBindPoint pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        std::vector<vk::AttachmentReference> inputAttachments{};
        std::vector<vk::AttachmentReference> colorAttachments{};
        std::vector<vk::AttachmentReference> resolveAttachments{};
        tl::optional<vk::AttachmentReference> depthStencilAttachment{};
        std::vector<u32> preserveAttachments{};
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
                subpasses[i].pipelineBindPoint = definitions[i].pipelineBindPoint;
                if (!definitions[i].inputAttachments.empty()) {
                    subpasses[i].setInputAttachments(definitions[i].inputAttachments);
                }
                if (!definitions[i].colorAttachments.empty()) {
                    subpasses[i].setColorAttachments(definitions[i].colorAttachments);
                }
                if (!definitions[i].resolveAttachments.empty()) {
                    subpasses[i].setResolveAttachments(definitions[i].resolveAttachments);
                }
                if (definitions[i].depthStencilAttachment.has_value()) {
                    subpasses[i].setPDepthStencilAttachment(&*definitions[i].depthStencilAttachment);
                }
                if (!definitions[i].preserveAttachments.empty()) {
                    subpasses[i].setPreserveAttachments(definitions[i].preserveAttachments);
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