#pragma once

template<typename T, size_t N>
using simd_vector_type = __attribute__((__ext_vector_type__(N))) T;

template<typename T, size_t C, size_t R>
using simd_matrix_type = __attribute__((__matrix_type__(C, R))) T;

using simd_int1 = simd_vector_type<int, 1>;
using simd_int2 = simd_vector_type<int, 2>;
using simd_int3 = simd_vector_type<int, 3>;
using simd_int4 = simd_vector_type<int, 4>;

using simd_int2x2 = simd_matrix_type<int, 2, 2>;
using simd_int3x2 = simd_matrix_type<int, 3, 2>;
using simd_int4x2 = simd_matrix_type<int, 4, 2>;
using simd_int2x3 = simd_matrix_type<int, 2, 3>;
using simd_int3x3 = simd_matrix_type<int, 3, 3>;
using simd_int4x3 = simd_matrix_type<int, 4, 3>;
using simd_int2x4 = simd_matrix_type<int, 2, 4>;
using simd_int3x4 = simd_matrix_type<int, 3, 4>;
using simd_int4x4 = simd_matrix_type<int, 4, 4>;

using simd_float1 = simd_vector_type<float, 1>;
using simd_float2 = simd_vector_type<float, 2>;
using simd_float3 = simd_vector_type<float, 3>;
using simd_float4 = simd_vector_type<float, 4>;

using simd_float2x2 = simd_matrix_type<float, 2, 2>;
using simd_float3x2 = simd_matrix_type<float, 3, 2>;
using simd_float4x2 = simd_matrix_type<float, 4, 2>;
using simd_float2x3 = simd_matrix_type<float, 2, 3>;
using simd_float3x3 = simd_matrix_type<float, 3, 3>;
using simd_float4x3 = simd_matrix_type<float, 4, 3>;
using simd_float2x4 = simd_matrix_type<float, 2, 4>;
using simd_float3x4 = simd_matrix_type<float, 3, 4>;
using simd_float4x4 = simd_matrix_type<float, 4, 4>;

namespace simd {
    using int1 = ::simd_int1;
    using int2 = ::simd_int2;
    using int3 = ::simd_int3;
    using int4 = ::simd_int4;

    using int2x2 = ::simd_int2x2;
    using int3x2 = ::simd_int3x2;
    using int4x2 = ::simd_int4x2;
    using int2x3 = ::simd_int2x3;
    using int3x3 = ::simd_int3x3;
    using int4x3 = ::simd_int4x3;
    using int2x4 = ::simd_int2x4;
    using int3x4 = ::simd_int3x4;
    using int4x4 = ::simd_int4x4;

    using float1 = ::simd_float1;
    using float2 = ::simd_float2;
    using float3 = ::simd_float3;
    using float4 = ::simd_float4;

    using float2x2 = ::simd_float2x2;
    using float3x2 = ::simd_float3x2;
    using float4x2 = ::simd_float4x2;
    using float2x3 = ::simd_float2x3;
    using float3x3 = ::simd_float3x3;
    using float4x3 = ::simd_float4x3;
    using float2x4 = ::simd_float2x4;
    using float3x4 = ::simd_float3x4;
    using float4x4 = ::simd_float4x4;
}