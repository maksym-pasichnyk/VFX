#pragma once

#include "group.hpp"
#include "assets.hpp"
#include "buffer.hpp"
#include "device.hpp"
#include "context.hpp"
#include "delegate.hpp"
#include "glsl.hpp"
#include "material.hpp"
#include "pass.hpp"
#include "queue.hpp"
#include "signal.hpp"
#include "layer.hpp"
#include "texture.hpp"
#include "types.hpp"

struct SceneConstants {
    vfx::float4x4 ViewMatrix;
    vfx::float4x4 ProjectionMatrix;
    vfx::float4x4 ViewProjectionMatrix;
    vfx::float4x4 InverseViewProjectionMatrix;
    vfx::float3   CameraPosition;
    vfx::int2     Resolution;
    vfx::float1   Time;
};

struct ModelConstants {
    vfx::float4x4 ModelMatrix;
};