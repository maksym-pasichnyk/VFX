#pragma once

using simd_int1 = __attribute__((__ext_vector_type__(1))) int;
using simd_int2 = __attribute__((__ext_vector_type__(2))) int;
using simd_int3 = __attribute__((__ext_vector_type__(3))) int;
using simd_int4 = __attribute__((__ext_vector_type__(4))) int;

using simd_int2x2 = __attribute__((__matrix_type__(2, 2))) int;
using simd_int3x2 = __attribute__((__matrix_type__(3, 2))) int;
using simd_int4x2 = __attribute__((__matrix_type__(4, 2))) int;
using simd_int2x3 = __attribute__((__matrix_type__(2, 3))) int;
using simd_int3x3 = __attribute__((__matrix_type__(3, 3))) int;
using simd_int4x3 = __attribute__((__matrix_type__(4, 3))) int;
using simd_int2x4 = __attribute__((__matrix_type__(2, 4))) int;
using simd_int3x4 = __attribute__((__matrix_type__(3, 4))) int;
using simd_int4x4 = __attribute__((__matrix_type__(4, 4))) int;

using simd_float1 = __attribute__((__ext_vector_type__(1))) float;
using simd_float2 = __attribute__((__ext_vector_type__(2))) float;
using simd_float3 = __attribute__((__ext_vector_type__(3))) float;
using simd_float4 = __attribute__((__ext_vector_type__(4))) float;

using simd_float2x2 = __attribute__((__matrix_type__(2, 2))) float;
using simd_float3x2 = __attribute__((__matrix_type__(3, 2))) float;
using simd_float4x2 = __attribute__((__matrix_type__(4, 2))) float;
using simd_float2x3 = __attribute__((__matrix_type__(2, 3))) float;
using simd_float3x3 = __attribute__((__matrix_type__(3, 3))) float;
using simd_float4x3 = __attribute__((__matrix_type__(4, 3))) float;
using simd_float2x4 = __attribute__((__matrix_type__(2, 4))) float;
using simd_float3x4 = __attribute__((__matrix_type__(3, 4))) float;
using simd_float4x4 = __attribute__((__matrix_type__(4, 4))) float;

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