#pragma once

#include <vector>
#include <types.hpp>
#include <context.hpp>
#include <glm/vec4.hpp>

struct ShaderEffect {
    struct Resources {
        vk::DescriptorPool pool;
        vk::DescriptorSetLayout layout;
        std::vector<vk::DescriptorSet> descriptors{};
        std::vector<vk::PushConstantRange> constants{};
    };

    vk::Pipeline pipeline;
    vk::PipelineLayout layout;
    std::vector<vk::PipelineShaderStageCreateInfo> stages{};

    Resources resources;
};
