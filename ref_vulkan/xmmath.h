//-------------------------------------------------------------------------------------
// SIMD C++ Math library
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// Copyright (C) 2018 Victor Coda.
//-------------------------------------------------------------------------------------

#pragma once

#if defined(_MSC_VER) && !defined(_M_ARM) && !defined(_M_ARM64) && !defined(_M_HYBRID_X86_ARM64) && (!_MANAGED) && (!_M_CEE) && (!defined(_M_IX86_FP) || (_M_IX86_FP > 1)) && !defined(_XM_NO_INTRINSICS_) && !defined(_XM_VECTORCALL_)
#if ((_MSC_FULL_VER >= 170065501) && (_MSC_VER < 1800)) || (_MSC_FULL_VER >= 180020418)
#define _XM_VECTORCALL_ 1
#endif
#endif

#if _XM_VECTORCALL_
#define XM_CALLCONV __vectorcall
#else
#define XM_CALLCONV __fastcall
#endif

#define XM_CONST const

#pragma warning(push)
#pragma warning(disable:4514 4820)
// C4514/4820: Off by default noise
#include <math.h>
#include <float.h>
#include <malloc.h>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 4987)
// C4987: Off by default noise
#include <intrin.h>
#pragma warning(pop)

#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>

#include <assert.h>

#pragma warning(push)
#pragma warning(disable : 4005 4668)
// C4005/4668: Old header issue
#include <stdint.h>
#pragma warning(pop)

/****************************************************************************
*
* Conditional intrinsics
*
****************************************************************************/

#if defined(_XM_NO_MOVNT_)
#define XM_STREAM_PS( p, a ) _mm_store_ps( p, a )
#define XM_SFENCE()
#else
#define XM_STREAM_PS( p, a ) _mm_stream_ps( p, a )
#define XM_SFENCE() _mm_sfence()
#endif

#if defined(_XM_AVX_INTRINSICS_)
#define XM_PERMUTE_PS( v, c ) _mm_permute_ps( v, c )
#else
#define XM_PERMUTE_PS( v, c ) _mm_shuffle_ps( v, v, c )
#endif

typedef int bool;

typedef __m128 XMVECTOR;
typedef const XMVECTOR FXMVECTOR;
typedef const XMVECTOR GXMVECTOR;
typedef const XMVECTOR HXMVECTOR;

// Conversion types for constants
__declspec(align(16)) typedef struct 
{
    union
    {
        float f[4];
        XMVECTOR v;
    };
} XMVECTORF32;

__declspec(align(16)) typedef struct 
{
    union
    {
        int32_t i[4];
        XMVECTOR v;
    };
} XMVECTORI32;

__declspec(align(16)) typedef struct 
{
    union
    {
        uint32_t u[4];
        XMVECTOR v;
    };
} XMVECTORU32;

// Matrix type: Sixteen 32 bit floating point components aligned on a
// 16 byte boundary and mapped to four hardware vector registers

__declspec(align(16)) typedef struct
{
    XMVECTOR r[4];
} XMMATRIX;

typedef const XMMATRIX FXMMATRIX;

/****************************************************************************
*
* Constant definitions
*
****************************************************************************/

#define XM_PI 3.141592654f
#define XM_2PI 6.283185307f
#define XM_1DIVPI 0.318309886f
#define XM_1DIV2PI 0.159154943f
#define XM_PIDIV2 1.570796327f
#define XM_PIDIV4 0.785398163f

#define XM_SELECT_0 0x00000000
#define XM_SELECT_1 0xFFFFFFFF

#define XM_PERMUTE_0X 0
#define XM_PERMUTE_0Y 1
#define XM_PERMUTE_0Z 2
#define XM_PERMUTE_0W 3
#define XM_PERMUTE_1X 4
#define XM_PERMUTE_1Y 5
#define XM_PERMUTE_1Z 6
#define XM_PERMUTE_1W 7

#define XM_SWIZZLE_X 0
#define XM_SWIZZLE_Y 1
#define XM_SWIZZLE_Z 2
#define XM_SWIZZLE_W 3

#define XM_CRMASK_CR6 0x000000F0
#define XM_CRMASK_CR6TRUE 0x00000080
#define XM_CRMASK_CR6FALSE 0x00000020
#define XM_CRMASK_CR6BOUNDS XM_CRMASK_CR6FALSE

#define XM_CACHE_LINE_SIZE 64

/****************************************************************************
*
* Globals
*
****************************************************************************/

// The purpose of the following global constants is to prevent redundant
// reloading of the constants when they are referenced by more than one
// separate inline math routine called within the same function.  Declaring
// a constant locally within a routine is sufficient to prevent redundant
// reloads of that constant when that single routine is called multiple
// times in a function, but if the constant is used (and declared) in a
// separate math routine it would be reloaded.

#ifndef XMGLOBALCONST
#define XMGLOBALCONST extern const __declspec(selectany)
#endif

XMGLOBALCONST XMVECTORF32 g_XMSinCoefficients0 = { { { -0.16666667f, +0.0083333310f, -0.00019840874f, +2.7525562e-06f } } };
XMGLOBALCONST XMVECTORF32 g_XMSinCoefficients1 = { { { -2.3889859e-08f, -0.16665852f /*Est1*/, +0.0083139502f /*Est2*/, -0.00018524670f /*Est3*/ } } };
XMGLOBALCONST XMVECTORF32 g_XMCosCoefficients0 = { { { -0.5f, +0.041666638f, -0.0013888378f, +2.4760495e-05f } } };
XMGLOBALCONST XMVECTORF32 g_XMCosCoefficients1 = { { { -2.6051615e-07f, -0.49992746f /*Est1*/, +0.041493919f /*Est2*/, -0.0012712436f /*Est3*/ } } };
XMGLOBALCONST XMVECTORF32 g_XMTanCoefficients0 = { { { 1.0f, 0.333333333f, 0.133333333f, 5.396825397e-2f } } };
XMGLOBALCONST XMVECTORF32 g_XMTanCoefficients1 = { { { 2.186948854e-2f, 8.863235530e-3f, 3.592128167e-3f, 1.455834485e-3f } } };
XMGLOBALCONST XMVECTORF32 g_XMTanCoefficients2 = { { { 5.900274264e-4f, 2.391290764e-4f, 9.691537707e-5f, 3.927832950e-5f } } };
XMGLOBALCONST XMVECTORF32 g_XMArcCoefficients0 = { { { +1.5707963050f, -0.2145988016f, +0.0889789874f, -0.0501743046f } } };
XMGLOBALCONST XMVECTORF32 g_XMArcCoefficients1 = { { { +0.0308918810f, -0.0170881256f, +0.0066700901f, -0.0012624911f } } };
XMGLOBALCONST XMVECTORF32 g_XMATanCoefficients0 = { { { -0.3333314528f, +0.1999355085f, -0.1420889944f, +0.1065626393f } } };
XMGLOBALCONST XMVECTORF32 g_XMATanCoefficients1 = { { { -0.0752896400f, +0.0429096138f, -0.0161657367f, +0.0028662257f } } };
XMGLOBALCONST XMVECTORF32 g_XMATanEstCoefficients0 = { { { +0.999866f, +0.999866f, +0.999866f, +0.999866f } } };
XMGLOBALCONST XMVECTORF32 g_XMATanEstCoefficients1 = { { { -0.3302995f, +0.180141f, -0.085133f, +0.0208351f } } };
XMGLOBALCONST XMVECTORF32 g_XMTanEstCoefficients = { { { 2.484f, -1.954923183e-1f, 2.467401101f, XM_1DIVPI } } };
XMGLOBALCONST XMVECTORF32 g_XMArcEstCoefficients = { { { +1.5707288f, -0.2121144f, +0.0742610f, -0.0187293f } } };
XMGLOBALCONST XMVECTORF32 g_XMPiConstants0 = { { { XM_PI, XM_2PI, XM_1DIVPI, XM_1DIV2PI } } };
XMGLOBALCONST XMVECTORF32 g_XMIdentityR0 = { { { 1.0f, 0.0f, 0.0f, 0.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMIdentityR1 = { { { 0.0f, 1.0f, 0.0f, 0.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMIdentityR2 = { { { 0.0f, 0.0f, 1.0f, 0.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMIdentityR3 = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMNegIdentityR0 = { { { -1.0f, 0.0f, 0.0f, 0.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMNegIdentityR1 = { { { 0.0f, -1.0f, 0.0f, 0.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMNegIdentityR2 = { { { 0.0f, 0.0f, -1.0f, 0.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMNegIdentityR3 = { { { 0.0f, 0.0f, 0.0f, -1.0f } } };
XMGLOBALCONST XMVECTORU32 g_XMNegativeZero = { { { 0x80000000, 0x80000000, 0x80000000, 0x80000000 } } };
XMGLOBALCONST XMVECTORU32 g_XMNegate3 = { { { 0x80000000, 0x80000000, 0x80000000, 0x00000000 } } };
XMGLOBALCONST XMVECTORU32 g_XMMaskXY = { { { 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000 } } };
XMGLOBALCONST XMVECTORU32 g_XMMask3 = { { { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 } } };
XMGLOBALCONST XMVECTORU32 g_XMMaskX = { { { 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000 } } };
XMGLOBALCONST XMVECTORU32 g_XMMaskY = { { { 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000 } } };
XMGLOBALCONST XMVECTORU32 g_XMMaskZ = { { { 0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000 } } };
XMGLOBALCONST XMVECTORU32 g_XMMaskW = { { { 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF } } };
XMGLOBALCONST XMVECTORF32 g_XMOne = { { { 1.0f, 1.0f, 1.0f, 1.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMOne3 = { { { 1.0f, 1.0f, 1.0f, 0.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMZero = { { { 0.0f, 0.0f, 0.0f, 0.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMTwo = { { { 2.f, 2.f, 2.f, 2.f } } };
XMGLOBALCONST XMVECTORF32 g_XMFour = { { { 4.f, 4.f, 4.f, 4.f } } };
XMGLOBALCONST XMVECTORF32 g_XMSix = { { { 6.f, 6.f, 6.f, 6.f } } };
XMGLOBALCONST XMVECTORF32 g_XMNegativeOne = { { { -1.0f, -1.0f, -1.0f, -1.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMOneHalf = { { { 0.5f, 0.5f, 0.5f, 0.5f } } };
XMGLOBALCONST XMVECTORF32 g_XMNegativeOneHalf = { { { -0.5f, -0.5f, -0.5f, -0.5f } } };
XMGLOBALCONST XMVECTORF32 g_XMNegativeTwoPi = { { { -XM_2PI, -XM_2PI, -XM_2PI, -XM_2PI } } };
XMGLOBALCONST XMVECTORF32 g_XMNegativePi = { { { -XM_PI, -XM_PI, -XM_PI, -XM_PI } } };
XMGLOBALCONST XMVECTORF32 g_XMHalfPi = { { { XM_PIDIV2, XM_PIDIV2, XM_PIDIV2, XM_PIDIV2 } } };
XMGLOBALCONST XMVECTORF32 g_XMPi = { { { XM_PI, XM_PI, XM_PI, XM_PI } } };
XMGLOBALCONST XMVECTORF32 g_XMReciprocalPi = { { { XM_1DIVPI, XM_1DIVPI, XM_1DIVPI, XM_1DIVPI } } };
XMGLOBALCONST XMVECTORF32 g_XMTwoPi = { { { XM_2PI, XM_2PI, XM_2PI, XM_2PI } } };
XMGLOBALCONST XMVECTORF32 g_XMReciprocalTwoPi = { { { XM_1DIV2PI, XM_1DIV2PI, XM_1DIV2PI, XM_1DIV2PI } } };
XMGLOBALCONST XMVECTORF32 g_XMEpsilon = { { { 1.192092896e-7f, 1.192092896e-7f, 1.192092896e-7f, 1.192092896e-7f } } };
XMGLOBALCONST XMVECTORI32 g_XMInfinity = { { { 0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000 } } };
XMGLOBALCONST XMVECTORI32 g_XMQNaN = { { { 0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000 } } };
XMGLOBALCONST XMVECTORI32 g_XMQNaNTest = { { { 0x007FFFFF, 0x007FFFFF, 0x007FFFFF, 0x007FFFFF } } };
XMGLOBALCONST XMVECTORI32 g_XMAbsMask = { { { 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF } } };
XMGLOBALCONST XMVECTORI32 g_XMFltMin = { { { 0x00800000, 0x00800000, 0x00800000, 0x00800000 } } };
XMGLOBALCONST XMVECTORI32 g_XMFltMax = { { { 0x7F7FFFFF, 0x7F7FFFFF, 0x7F7FFFFF, 0x7F7FFFFF } } };
XMGLOBALCONST XMVECTORU32 g_XMNegOneMask = { { { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } } };
XMGLOBALCONST XMVECTORU32 g_XMMaskA8R8G8B8 = { { { 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000 } } };
XMGLOBALCONST XMVECTORU32 g_XMFlipA8R8G8B8 = { { { 0x00000000, 0x00000000, 0x00000000, 0x80000000 } } };
XMGLOBALCONST XMVECTORF32 g_XMFixAA8R8G8B8 = { { { 0.0f, 0.0f, 0.0f, (float)(0x80000000U) } } };
XMGLOBALCONST XMVECTORF32 g_XMNormalizeA8R8G8B8 = { { { 1.0f / (255.0f*(float)(0x10000)), 1.0f / (255.0f*(float)(0x100)), 1.0f / 255.0f, 1.0f / (255.0f*(float)(0x1000000)) } } };
XMGLOBALCONST XMVECTORU32 g_XMMaskA2B10G10R10 = { { { 0x000003FF, 0x000FFC00, 0x3FF00000, 0xC0000000 } } };
XMGLOBALCONST XMVECTORU32 g_XMFlipA2B10G10R10 = { { { 0x00000200, 0x00080000, 0x20000000, 0x80000000 } } };
XMGLOBALCONST XMVECTORF32 g_XMFixAA2B10G10R10 = { { { -512.0f, -512.0f*(float)(0x400), -512.0f*(float)(0x100000), (float)(0x80000000U) } } };
XMGLOBALCONST XMVECTORF32 g_XMNormalizeA2B10G10R10 = { { { 1.0f / 511.0f, 1.0f / (511.0f*(float)(0x400)), 1.0f / (511.0f*(float)(0x100000)), 1.0f / (3.0f*(float)(0x40000000)) } } };
XMGLOBALCONST XMVECTORU32 g_XMMaskX16Y16 = { { { 0x0000FFFF, 0xFFFF0000, 0x00000000, 0x00000000 } } };
XMGLOBALCONST XMVECTORI32 g_XMFlipX16Y16 = { { { 0x00008000, 0x00000000, 0x00000000, 0x00000000 } } };
XMGLOBALCONST XMVECTORF32 g_XMFixX16Y16 = { { { -32768.0f, 0.0f, 0.0f, 0.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMNormalizeX16Y16 = { { { 1.0f / 32767.0f, 1.0f / (32767.0f*65536.0f), 0.0f, 0.0f } } };
XMGLOBALCONST XMVECTORU32 g_XMMaskX16Y16Z16W16 = { { { 0x0000FFFF, 0x0000FFFF, 0xFFFF0000, 0xFFFF0000 } } };
XMGLOBALCONST XMVECTORI32 g_XMFlipX16Y16Z16W16 = { { { 0x00008000, 0x00008000, 0x00000000, 0x00000000 } } };
XMGLOBALCONST XMVECTORF32 g_XMFixX16Y16Z16W16 = { { { -32768.0f, -32768.0f, 0.0f, 0.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMNormalizeX16Y16Z16W16 = { { { 1.0f / 32767.0f, 1.0f / 32767.0f, 1.0f / (32767.0f*65536.0f), 1.0f / (32767.0f*65536.0f) } } };
XMGLOBALCONST XMVECTORF32 g_XMNoFraction = { { { 8388608.0f, 8388608.0f, 8388608.0f, 8388608.0f } } };
XMGLOBALCONST XMVECTORI32 g_XMMaskByte = { { { 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF } } };
XMGLOBALCONST XMVECTORF32 g_XMNegateX = { { { -1.0f, 1.0f, 1.0f, 1.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMNegateY = { { { 1.0f, -1.0f, 1.0f, 1.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMNegateZ = { { { 1.0f, 1.0f, -1.0f, 1.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMNegateW = { { { 1.0f, 1.0f, 1.0f, -1.0f } } };
XMGLOBALCONST XMVECTORU32 g_XMSelect0101 = { { { XM_SELECT_0, XM_SELECT_1, XM_SELECT_0, XM_SELECT_1 } } };
XMGLOBALCONST XMVECTORU32 g_XMSelect1010 = { { { XM_SELECT_1, XM_SELECT_0, XM_SELECT_1, XM_SELECT_0 } } };
XMGLOBALCONST XMVECTORI32 g_XMOneHalfMinusEpsilon = { { { 0x3EFFFFFD, 0x3EFFFFFD, 0x3EFFFFFD, 0x3EFFFFFD } } };
XMGLOBALCONST XMVECTORU32 g_XMSelect1000 = { { { XM_SELECT_1, XM_SELECT_0, XM_SELECT_0, XM_SELECT_0 } } };
XMGLOBALCONST XMVECTORU32 g_XMSelect1100 = { { { XM_SELECT_1, XM_SELECT_1, XM_SELECT_0, XM_SELECT_0 } } };
XMGLOBALCONST XMVECTORU32 g_XMSelect1110 = { { { XM_SELECT_1, XM_SELECT_1, XM_SELECT_1, XM_SELECT_0 } } };
XMGLOBALCONST XMVECTORU32 g_XMSelect1011 = { { { XM_SELECT_1, XM_SELECT_0, XM_SELECT_1, XM_SELECT_1 } } };
XMGLOBALCONST XMVECTORF32 g_XMFixupY16 = { { { 1.0f, 1.0f / 65536.0f, 0.0f, 0.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMFixupY16W16 = { { { 1.0f, 1.0f, 1.0f / 65536.0f, 1.0f / 65536.0f } } };
XMGLOBALCONST XMVECTORU32 g_XMFlipY = { { { 0, 0x80000000, 0, 0 } } };
XMGLOBALCONST XMVECTORU32 g_XMFlipZ = { { { 0, 0, 0x80000000, 0 } } };
XMGLOBALCONST XMVECTORU32 g_XMFlipW = { { { 0, 0, 0, 0x80000000 } } };
XMGLOBALCONST XMVECTORU32 g_XMFlipYZ = { { { 0, 0x80000000, 0x80000000, 0 } } };
XMGLOBALCONST XMVECTORU32 g_XMFlipZW = { { { 0, 0, 0x80000000, 0x80000000 } } };
XMGLOBALCONST XMVECTORU32 g_XMFlipYW = { { { 0, 0x80000000, 0, 0x80000000 } } };
XMGLOBALCONST XMVECTORI32 g_XMMaskDec4 = { { { 0x3FF, 0x3FF << 10, 0x3FF << 20, 0x3 << 30 } } };
XMGLOBALCONST XMVECTORI32 g_XMXorDec4 = { { { 0x200, 0x200 << 10, 0x200 << 20, 0 } } };
XMGLOBALCONST XMVECTORF32 g_XMAddUDec4 = { { { 0, 0, 0, 32768.0f*65536.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMAddDec4 = { { { -512.0f, -512.0f*1024.0f, -512.0f*1024.0f*1024.0f, 0 } } };
XMGLOBALCONST XMVECTORF32 g_XMMulDec4 = { { { 1.0f, 1.0f / 1024.0f, 1.0f / (1024.0f*1024.0f), 1.0f / (1024.0f*1024.0f*1024.0f) } } };
XMGLOBALCONST XMVECTORU32 g_XMMaskByte4 = { { { 0xFF, 0xFF00, 0xFF0000, 0xFF000000 } } };
XMGLOBALCONST XMVECTORI32 g_XMXorByte4 = { { { 0x80, 0x8000, 0x800000, 0x00000000 } } };
XMGLOBALCONST XMVECTORF32 g_XMAddByte4 = { { { -128.0f, -128.0f*256.0f, -128.0f*65536.0f, 0 } } };
XMGLOBALCONST XMVECTORF32 g_XMFixUnsigned = { { { 32768.0f*65536.0f, 32768.0f*65536.0f, 32768.0f*65536.0f, 32768.0f*65536.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMMaxInt = { { { 65536.0f*32768.0f - 128.0f, 65536.0f*32768.0f - 128.0f, 65536.0f*32768.0f - 128.0f, 65536.0f*32768.0f - 128.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMMaxUInt = { { { 65536.0f*65536.0f - 256.0f, 65536.0f*65536.0f - 256.0f, 65536.0f*65536.0f - 256.0f, 65536.0f*65536.0f - 256.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMUnsignedFix = { { { 32768.0f*65536.0f, 32768.0f*65536.0f, 32768.0f*65536.0f, 32768.0f*65536.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMsrgbScale = { { { 12.92f, 12.92f, 12.92f, 1.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMsrgbA = { { { 0.055f, 0.055f, 0.055f, 0.0f } } };
XMGLOBALCONST XMVECTORF32 g_XMsrgbA1 = { { { 1.055f, 1.055f, 1.055f, 1.0f } } };
XMGLOBALCONST XMVECTORI32 g_XMExponentBias = { { { 127, 127, 127, 127 } } };
XMGLOBALCONST XMVECTORI32 g_XMSubnormalExponent = { { { -126, -126, -126, -126 } } };
XMGLOBALCONST XMVECTORI32 g_XMNumTrailing = { { { 23, 23, 23, 23 } } };
XMGLOBALCONST XMVECTORI32 g_XMMinNormal = { { { 0x00800000, 0x00800000, 0x00800000, 0x00800000 } } };
XMGLOBALCONST XMVECTORU32 g_XMNegInfinity = { { { 0xFF800000, 0xFF800000, 0xFF800000, 0xFF800000 } } };
XMGLOBALCONST XMVECTORU32 g_XMNegQNaN = { { { 0xFFC00000, 0xFFC00000, 0xFFC00000, 0xFFC00000 } } };
XMGLOBALCONST XMVECTORI32 g_XMBin128 = { { { 0x43000000, 0x43000000, 0x43000000, 0x43000000 } } };
XMGLOBALCONST XMVECTORU32 g_XMBinNeg150 = { { { 0xC3160000, 0xC3160000, 0xC3160000, 0xC3160000 } } };
XMGLOBALCONST XMVECTORI32 g_XM253 = { { { 253, 253, 253, 253 } } };
XMGLOBALCONST XMVECTORF32 g_XMExpEst1 = { { { -6.93147182e-1f, -6.93147182e-1f, -6.93147182e-1f, -6.93147182e-1f } } };
XMGLOBALCONST XMVECTORF32 g_XMExpEst2 = { { { +2.40226462e-1f, +2.40226462e-1f, +2.40226462e-1f, +2.40226462e-1f } } };
XMGLOBALCONST XMVECTORF32 g_XMExpEst3 = { { { -5.55036440e-2f, -5.55036440e-2f, -5.55036440e-2f, -5.55036440e-2f } } };
XMGLOBALCONST XMVECTORF32 g_XMExpEst4 = { { { +9.61597636e-3f, +9.61597636e-3f, +9.61597636e-3f, +9.61597636e-3f } } };
XMGLOBALCONST XMVECTORF32 g_XMExpEst5 = { { { -1.32823968e-3f, -1.32823968e-3f, -1.32823968e-3f, -1.32823968e-3f } } };
XMGLOBALCONST XMVECTORF32 g_XMExpEst6 = { { { +1.47491097e-4f, +1.47491097e-4f, +1.47491097e-4f, +1.47491097e-4f } } };
XMGLOBALCONST XMVECTORF32 g_XMExpEst7 = { { { -1.08635004e-5f, -1.08635004e-5f, -1.08635004e-5f, -1.08635004e-5f } } };
XMGLOBALCONST XMVECTORF32 g_XMLogEst0 = { { { +1.442693f, +1.442693f, +1.442693f, +1.442693f } } };
XMGLOBALCONST XMVECTORF32 g_XMLogEst1 = { { { -0.721242f, -0.721242f, -0.721242f, -0.721242f } } };
XMGLOBALCONST XMVECTORF32 g_XMLogEst2 = { { { +0.479384f, +0.479384f, +0.479384f, +0.479384f } } };
XMGLOBALCONST XMVECTORF32 g_XMLogEst3 = { { { -0.350295f, -0.350295f, -0.350295f, -0.350295f } } };
XMGLOBALCONST XMVECTORF32 g_XMLogEst4 = { { { +0.248590f, +0.248590f, +0.248590f, +0.248590f } } };
XMGLOBALCONST XMVECTORF32 g_XMLogEst5 = { { { -0.145700f, -0.145700f, -0.145700f, -0.145700f } } };
XMGLOBALCONST XMVECTORF32 g_XMLogEst6 = { { { +0.057148f, +0.057148f, +0.057148f, +0.057148f } } };
XMGLOBALCONST XMVECTORF32 g_XMLogEst7 = { { { -0.010578f, -0.010578f, -0.010578f, -0.010578f } } };
XMGLOBALCONST XMVECTORF32 g_XMLgE = { { { +1.442695f, +1.442695f, +1.442695f, +1.442695f } } };
XMGLOBALCONST XMVECTORF32 g_XMInvLgE = { { { +6.93147182e-1f, +6.93147182e-1f, +6.93147182e-1f, +6.93147182e-1f } } };
XMGLOBALCONST XMVECTORF32 g_UByteMax = { { { 255.0f, 255.0f, 255.0f, 255.0f } } };
XMGLOBALCONST XMVECTORF32 g_ByteMin = { { { -127.0f, -127.0f, -127.0f, -127.0f } } };
XMGLOBALCONST XMVECTORF32 g_ByteMax = { { { 127.0f, 127.0f, 127.0f, 127.0f } } };
XMGLOBALCONST XMVECTORF32 g_ShortMin = { { { -32767.0f, -32767.0f, -32767.0f, -32767.0f } } };
XMGLOBALCONST XMVECTORF32 g_ShortMax = { { { 32767.0f, 32767.0f, 32767.0f, 32767.0f } } };
XMGLOBALCONST XMVECTORF32 g_UShortMax = { { { 65535.0f, 65535.0f, 65535.0f, 65535.0f } } };

#define XMConvertToRadians(Degrees) ((Degrees) * (XM_PI / 180.0f))
#define XMConvertToDegrees(Radians) ((Radians) * (180.0f / XM_PI))

XMVECTOR XM_CALLCONV XMVectorZero();
XMVECTOR XM_CALLCONV XMVectorSet(float x, float y, float z, float w);
XMVECTOR XM_CALLCONV XMVectorReplicate(float Value);
float XM_CALLCONV XMVectorGetX(FXMVECTOR V);
float XM_CALLCONV XMVectorGetY(FXMVECTOR V);
float XM_CALLCONV XMVectorGetZ(FXMVECTOR V);
float XM_CALLCONV XMVectorGetW(FXMVECTOR V);
XMVECTOR XM_CALLCONV XMVectorSetX(FXMVECTOR V, float x);
XMVECTOR XM_CALLCONV XMVectorSetY(FXMVECTOR V, float y);
XMVECTOR XM_CALLCONV XMVectorSetZ(FXMVECTOR V, float z);
XMVECTOR XM_CALLCONV XMVectorSetW(FXMVECTOR V, float w);
XMVECTOR XM_CALLCONV XMVectorSelect(FXMVECTOR V1, FXMVECTOR V2, FXMVECTOR Control);
XMVECTOR XM_CALLCONV XMVectorNegate(FXMVECTOR V);
XMVECTOR XM_CALLCONV XMVectorAdd(FXMVECTOR V1, FXMVECTOR V2);
XMVECTOR XM_CALLCONV XMVectorSubtract(FXMVECTOR V1, FXMVECTOR V2);
XMVECTOR XM_CALLCONV XMVectorMultiply(FXMVECTOR V1, FXMVECTOR V2);
XMVECTOR XM_CALLCONV XMVectorMultiplyAdd(FXMVECTOR V1, FXMVECTOR V2, FXMVECTOR V3);
XMVECTOR XM_CALLCONV XMVectorDivide(FXMVECTOR V1, FXMVECTOR V2);
XMVECTOR XM_CALLCONV XMVectorScale(FXMVECTOR V, float ScaleFactor);
XMVECTOR XM_CALLCONV XMVector3Dot(FXMVECTOR V1, FXMVECTOR V2);
XMVECTOR XM_CALLCONV XMVector3Cross(FXMVECTOR V1, FXMVECTOR V2);
XMVECTOR XM_CALLCONV XMVector3Length(FXMVECTOR V);
XMVECTOR XM_CALLCONV XMVector3Normalize(FXMVECTOR V);
XMVECTOR XM_CALLCONV XMVector4Dot(FXMVECTOR V1, FXMVECTOR V2);

XMMATRIX XM_CALLCONV XMMatrixMultiply(FXMMATRIX *M1, FXMMATRIX *M2);
XMMATRIX XM_CALLCONV XMMatrixMultiplyTranspose(FXMMATRIX *M1, FXMMATRIX *M2);
XMMATRIX XM_CALLCONV XMMatrixTranspose(FXMMATRIX *M);
XMMATRIX XM_CALLCONV XMMatrixInverse(XMVECTOR *pDeterminant, FXMMATRIX *M);
XMMATRIX XM_CALLCONV XMMatrixIdentity();
XMMATRIX XM_CALLCONV XMMatrixSet(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33);
XMMATRIX XM_CALLCONV XMMatrixTranslation(float OffsetX, float OffsetY, float OffsetZ);
XMMATRIX XM_CALLCONV XMMatrixTranslationFromVector(FXMVECTOR Offset);
XMMATRIX XM_CALLCONV XMMatrixScaling(float ScaleX, float ScaleY, float ScaleZ);
XMMATRIX XM_CALLCONV XMMatrixScalingFromVector(FXMVECTOR Scale);
XMMATRIX XM_CALLCONV XMMatrixRotationX(float Angle);
XMMATRIX XM_CALLCONV XMMatrixRotationY(float Angle);
XMMATRIX XM_CALLCONV XMMatrixRotationZ(float Angle);
XMMATRIX XM_CALLCONV XMMatrixRotationNormal(FXMVECTOR NormalAxis, float Angle);
XMMATRIX XM_CALLCONV XMMatrixRotationAxis(FXMVECTOR Axis, float Angle);
XMMATRIX XM_CALLCONV XMMatrixLookToLH(FXMVECTOR EyePosition, FXMVECTOR EyeDirection, FXMVECTOR UpDirection);
XMMATRIX XM_CALLCONV XMMatrixLookToRH(FXMVECTOR EyePosition, FXMVECTOR EyeDirection, FXMVECTOR UpDirection);
XMMATRIX XM_CALLCONV XMMatrixLookAtLH(FXMVECTOR EyePosition, FXMVECTOR FocusPosition, FXMVECTOR UpDirection);
XMMATRIX XM_CALLCONV XMMatrixLookAtRH(FXMVECTOR EyePosition, FXMVECTOR FocusPosition, FXMVECTOR UpDirection);
XMMATRIX XM_CALLCONV XMMatrixPerspectiveLH(float ViewWidth, float ViewHeight, float NearZ, float FarZ);
XMMATRIX XM_CALLCONV XMMatrixPerspectiveRH(float ViewWidth, float ViewHeight, float NearZ, float FarZ);
XMMATRIX XM_CALLCONV XMMatrixPerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ, float FarZ);
XMMATRIX XM_CALLCONV XMMatrixPerspectiveFovRH(float FovAngleY, float AspectRatio, float NearZ, float FarZ);
XMMATRIX XM_CALLCONV XMMatrixPerspectiveOffCenterLH(float ViewLeft, float ViewRight, float ViewBottom, float ViewTop, float NearZ, float FarZ);
XMMATRIX XM_CALLCONV XMMatrixPerspectiveOffCenterRH(float ViewLeft, float ViewRight, float ViewBottom, float ViewTop, float NearZ, float FarZ);
XMMATRIX XM_CALLCONV XMMatrixOrthographicLH(float ViewWidth, float ViewHeight, float NearZ, float FarZ);
XMMATRIX XM_CALLCONV XMMatrixOrthographicRH(float ViewWidth, float ViewHeight, float NearZ, float FarZ);
XMMATRIX XM_CALLCONV XMMatrixOrthographicOffCenterLH(float ViewLeft, float ViewRight, float ViewBottom, float ViewTop, float NearZ, float FarZ);
XMMATRIX XM_CALLCONV XMMatrixOrthographicOffCenterRH(float ViewLeft, float ViewRight, float ViewBottom, float ViewTop, float NearZ, float FarZ);
