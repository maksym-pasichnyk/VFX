#pragma once

#include "group.hpp"
#include "assets.hpp"
#include "buffer.hpp"
#include "device.hpp"
#include "context.hpp"
#include "delegate.hpp"
#include "material.hpp"
#include "pass.hpp"
#include "queue.hpp"
#include "signal.hpp"
#include "layer.hpp"
#include "texture.hpp"
#include "types.hpp"

#include "glm/gtx/compatibility.hpp"

using int1 [[gnu::aligned(4)]]  = glm::i32;
using int2 [[gnu::aligned(16)]] = glm::i32vec2;
using int3 [[gnu::aligned(16)]] = glm::i32vec3;
using int4 [[gnu::aligned(16)]] = glm::i32vec4;

using int2x2 [[gnu::aligned(16)]] = glm::i32mat2x2;
using int2x3 [[gnu::aligned(16)]] = glm::i32mat2x3;
using int2x4 [[gnu::aligned(16)]] = glm::i32mat2x4;
using int3x2 [[gnu::aligned(16)]] = glm::i32mat3x2;
using int3x3 [[gnu::aligned(16)]] = glm::i32mat3x3;
using int3x4 [[gnu::aligned(16)]] = glm::i32mat3x4;
using int4x2 [[gnu::aligned(16)]] = glm::i32mat4x2;
using int4x3 [[gnu::aligned(16)]] = glm::i32mat4x3;
using int4x4 [[gnu::aligned(16)]] = glm::i32mat4x4;

using float1 [[gnu::aligned(4)]]  = glm::f32;
using float2 [[gnu::aligned(16)]] = glm::f32vec2;
using float3 [[gnu::aligned(16)]] = glm::f32vec3;
using float4 [[gnu::aligned(16)]] = glm::f32vec4;

using float2x2 [[gnu::aligned(16)]] = glm::f32mat2x2;
using float2x3 [[gnu::aligned(16)]] = glm::f32mat2x3;
using float2x4 [[gnu::aligned(16)]] = glm::f32mat2x4;
using float3x2 [[gnu::aligned(16)]] = glm::f32mat3x2;
using float3x3 [[gnu::aligned(16)]] = glm::f32mat3x3;
using float3x4 [[gnu::aligned(16)]] = glm::f32mat3x4;
using float4x2 [[gnu::aligned(16)]] = glm::f32mat4x2;
using float4x3 [[gnu::aligned(16)]] = glm::f32mat4x3;
using float4x4 [[gnu::aligned(16)]] = glm::f32mat4x4;

using double1 [[gnu::aligned(8)]]  = glm::f64;
using double2 [[gnu::aligned(16)]] = glm::f64vec2;
using double3 [[gnu::aligned(16)]] = glm::f64vec3;
using double4 [[gnu::aligned(16)]] = glm::f64vec4;

using double2x2 [[gnu::aligned(16)]] = glm::f64mat2x2;
using double2x3 [[gnu::aligned(16)]] = glm::f64mat2x3;
using double2x4 [[gnu::aligned(16)]] = glm::f64mat2x4;
using double3x2 [[gnu::aligned(16)]] = glm::f64mat3x2;
using double3x3 [[gnu::aligned(16)]] = glm::f64mat3x3;
using double3x4 [[gnu::aligned(16)]] = glm::f64mat3x4;
using double4x2 [[gnu::aligned(16)]] = glm::f64mat4x2;
using double4x3 [[gnu::aligned(16)]] = glm::f64mat4x3;
using double4x4 [[gnu::aligned(16)]] = glm::f64mat4x4;

using uint1 [[gnu::aligned(4)]]  = glm::u32;
using uint2 [[gnu::aligned(16)]] = glm::u32vec2;
using uint3 [[gnu::aligned(16)]] = glm::u32vec3;
using uint4 [[gnu::aligned(16)]] = glm::u32vec4;

using uint2x2 [[gnu::aligned(16)]] = glm::u32mat2x2;
using uint2x3 [[gnu::aligned(16)]] = glm::u32mat2x3;
using uint2x4 [[gnu::aligned(16)]] = glm::u32mat2x4;
using uint3x2 [[gnu::aligned(16)]] = glm::u32mat3x2;
using uint3x3 [[gnu::aligned(16)]] = glm::u32mat3x3;
using uint3x4 [[gnu::aligned(16)]] = glm::u32mat3x4;
using uint4x2 [[gnu::aligned(16)]] = glm::u32mat4x2;
using uint4x3 [[gnu::aligned(16)]] = glm::u32mat4x3;
using uint4x4 [[gnu::aligned(16)]] = glm::u32mat4x4;

struct SceneConstants {
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;
    float4x4 ViewProjectionMatrix;
    float4x4 InverseViewProjectionMatrix;
    float3   CameraPosition;
    int2     Resolution;
    float1   Time;
};

struct ObjectConstants {
    float4x4 transform;
};

struct Material {
    float4 color;
    float3 ambient;
};

struct Light {
    float3 position;
    float3 color;

    float brightness;
    float intensity;
};