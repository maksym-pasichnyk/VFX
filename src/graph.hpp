#pragma once

#include <map>
#include <vector>
#include <types.hpp>
#include <context.hpp>
#include <glm/vec4.hpp>
#include <tl/optional.hpp>

struct RenderGraph {
    struct SubpassDefinition {
        vk::PipelineBindPoint pipelineBindPoint{};
        std::vector<vk::AttachmentReference> pInputAttachments{};
        std::vector<vk::AttachmentReference> pColorAttachments{};
        std::vector<vk::AttachmentReference> pResolveAttachments{};
        tl::optional<vk::AttachmentReference> pDepthStencilAttachment{};
        std::vector<u32> pPreserveAttachments{};
    };

    Context& context;

    vk::RenderPass pass;
    std::vector<vk::ClearValue> clear_values{};
    std::vector<SubpassDefinition> subpass_definitions{};
    std::vector<vk::AttachmentDescription> attachment_descriptions{};

    std::map<std::string, u32> attachmant_table{};

    explicit RenderGraph(Context& context) : context(context) {}

    ~RenderGraph() {
        context.logical_device.destroyRenderPass(pass);
    }

    auto add_pass(const std::string& name) -> u64 {
        auto subpass = subpass_definitions.size();
        subpass_definitions.emplace_back();
        return subpass;
    }

    void add_present_output(u64 subpass, const std::string& name, vk::Format format) {
        auto attachment = u32(attachment_descriptions.size());
        clear_values.emplace_back(vk::ClearValue{});
        attachmant_table.emplace(name, attachment);
        attachment_descriptions.emplace_back(vk::AttachmentDescription{
            .flags = {},
            .format = format,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::ePresentSrcKHR
        });
        subpass_definitions[subpass].pColorAttachments.emplace_back(vk::AttachmentReference{
            .attachment = attachment,
            .layout = vk::ImageLayout::eColorAttachmentOptimal
        });
    }

    void add_color_output(u64 subpass, const std::string& name, vk::Format format) {
        auto attachment = u32(attachment_descriptions.size());
        clear_values.emplace_back(vk::ClearValue{});
        attachmant_table.emplace(name, attachment);
        attachment_descriptions.emplace_back(vk::AttachmentDescription{
            .flags = {},
            .format = format,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::eColorAttachmentOptimal
        });
        subpass_definitions[subpass].pColorAttachments.emplace_back(vk::AttachmentReference{
            .attachment = attachment,
            .layout = vk::ImageLayout::eColorAttachmentOptimal
        });
    }

    void set_depth_output(u64 subpass, const std::string& name, vk::Format format) {
        auto attachment = u32(attachment_descriptions.size());
        clear_values.emplace_back(vk::ClearValue{});
        attachmant_table.emplace(name, attachment);
        attachment_descriptions.emplace_back(vk::AttachmentDescription{
            .flags = {},
            .format = format,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal
        });
        subpass_definitions[subpass].pDepthStencilAttachment = vk::AttachmentReference{
            .attachment = attachment,
            .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal
        };
    }

    void add_color_input(u64 subpass, const std::string& name) {
        subpass_definitions[subpass].pInputAttachments.emplace_back(vk::AttachmentReference{
            .attachment = attachmant_table.at(name),
            .layout = vk::ImageLayout::eShaderReadOnlyOptimal
        });
    }

    void add_depth_input(u64 subpass, const std::string& name) {

    }

    void create_render_pass() {
        std::vector<vk::SubpassDescription> subpass_descriptions{};
        subpass_descriptions.resize(subpass_definitions.size());
        for (u64 i = 0; i < subpass_definitions.size(); ++i) {
            subpass_descriptions[i].flags = {};
            subpass_descriptions[i].pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
            if (!subpass_definitions[i].pInputAttachments.empty()) {
                subpass_descriptions[i].setInputAttachments(subpass_definitions[i].pInputAttachments);
            }
            if (!subpass_definitions[i].pColorAttachments.empty()) {
                subpass_descriptions[i].setColorAttachments(subpass_definitions[i].pColorAttachments);
            }
            if (!subpass_definitions[i].pResolveAttachments.empty()) {
                subpass_descriptions[i].setResolveAttachments(subpass_definitions[i].pResolveAttachments);
            }
            if (subpass_definitions[i].pDepthStencilAttachment.has_value()) {
                subpass_descriptions[i].setPDepthStencilAttachment(&*subpass_definitions[i].pDepthStencilAttachment);
            }
            if (!subpass_definitions[i].pPreserveAttachments.empty()) {
                subpass_descriptions[i].setPreserveAttachments(subpass_definitions[i].pPreserveAttachments);
            }
        }

        auto dependencies = std::vector<vk::SubpassDependency>{};
//        dependencies.emplace_back(vk::SubpassDependency{
//            .srcSubpass = VK_SUBPASS_EXTERNAL,
//            .dstSubpass = 0,
//            .srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe,
//            .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
//            .srcAccessMask = vk::AccessFlagBits::eMemoryRead,
//            .dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
//            .dependencyFlags = vk::DependencyFlagBits::eByRegion
//        });
//        dependencies.emplace_back(vk::SubpassDependency{
//            .srcSubpass = 0,
//            .dstSubpass = 1,
//            .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
//            .dstStageMask = vk::PipelineStageFlagBits::eFragmentShader,
//            .srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
//            .dstAccessMask = vk::AccessFlagBits::eShaderRead,
//            .dependencyFlags = vk::DependencyFlagBits::eByRegion
//        });
//        dependencies.emplace_back(vk::SubpassDependency{
//            .srcSubpass = 1,
//            .dstSubpass = VK_SUBPASS_EXTERNAL,
//            .srcStageMask = vk::PipelineStageFlagBits::eFragmentShader,
//            .dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe,
//            .srcAccessMask = vk::AccessFlagBits::eShaderRead,
//            .dstAccessMask = vk::AccessFlagBits::eMemoryWrite,
//            .dependencyFlags = vk::DependencyFlagBits::eByRegion
//        });
        auto create_info = vk::RenderPassCreateInfo{};
        create_info.setSubpasses(subpass_descriptions);
        create_info.setAttachments(attachment_descriptions);
        create_info.setDependencies(dependencies);
        pass = context.logical_device.createRenderPass(create_info);
    }

    void set_clear_color(const std::string& name, const glm::vec4& color) {
        auto [r, g, b, a] = color;
        auto value = vk::ClearColorValue{std::array{r, g, b, a}};
        clear_values.at(attachmant_table.at(name)).setColor(value);
    }

    void set_clear_depth(const std::string& name, f32 depth, u32 stencil) {
        auto value = vk::ClearDepthStencilValue{depth, stencil};
        clear_values.at(attachmant_table.at(name)).setDepthStencil(value);
    }
};
