#pragma once

#include "Object.hpp"
#include "glm/glm.hpp"

#include <string>
#include <vector>

struct AnimationChannel {
private:
    std::string target_path = {};

    int32_t sampler = 0;
    int32_t target_node = 0;

public:
    explicit AnimationChannel(std::string target_path, int32_t sampler, int32_t target_node)
        : target_path(std::move(target_path)), sampler(sampler), target_node(target_node) {}
};

struct AnimationSampler {
private:
    std::string interpolation = {};
    std::vector<float> inputs = {};
    std::vector<glm::f32vec4> outputs = {};

public:
    AnimationSampler(std::string interpolation, std::vector<float_t> inputs, std::vector<glm::f32vec4> outputs)
        : interpolation(std::move(interpolation)), inputs(std::move(inputs)), outputs(std::move(outputs)) {}
};

struct Animation : ManagedObject<Animation> {
private:
    std::string name;
    std::vector<AnimationSampler> samplers;
    std::vector<AnimationChannel> channels;
    float_t start = std::numeric_limits<float_t>::max();
    float_t end = std::numeric_limits<float_t>::min();

public:
    Animation(std::string name, std::vector<AnimationSampler> samplers, std::vector<AnimationChannel> channels, float_t start, float_t end)
        : name(std::move(name)), samplers(std::move(samplers)), channels(std::move(channels)), start(start), end(end) {}
};