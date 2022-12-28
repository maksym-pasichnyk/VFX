#pragma once

typedef                                         int simd_int1;
typedef __attribute__((__ext_vector_type__(2))) int simd_int2;
typedef __attribute__((__ext_vector_type__(3))) int simd_int3;
typedef __attribute__((__ext_vector_type__(4))) int simd_int4;

typedef                                         float simd_float1;
typedef __attribute__((__ext_vector_type__(2))) float simd_float2;
typedef __attribute__((__ext_vector_type__(3))) float simd_float3;
typedef __attribute__((__ext_vector_type__(4))) float simd_float4;

namespace simd {
    typedef ::simd_int1 int1;
    typedef ::simd_int2 int2;
    typedef ::simd_int3 int3;
    typedef ::simd_int4 int4;

    typedef ::simd_float1 float1;
    typedef ::simd_float2 float2;
    typedef ::simd_float3 float3;
    typedef ::simd_float4 float4;
}