//-------------------------------------------------------------------------------------
// DirectXMath - SIMD C++ Math library
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// Copyright(C) 2018 Victor Coda.
//
// http://go.microsoft.com/fwlink/?LinkID=615560
//-------------------------------------------------------------------------------------
#include "dxmath.h"

XMVECTOR XM_CALLCONV XMVectorZero()
{
    return _mm_setzero_ps();
}

XMVECTOR XM_CALLCONV XMVectorSet
(
    float x, 
    float y, 
    float z, 
    float w
)
{
    return _mm_set_ps( w, z, y, x );
}

XMVECTOR XM_CALLCONV XMVectorReplicate
(
    float Value
)
{
    return _mm_set_ps1( Value );
}

float XM_CALLCONV XMVectorGetX(FXMVECTOR V)
{
    return _mm_cvtss_f32(V);
}

float XM_CALLCONV XMVectorGetY(FXMVECTOR V)
{
    XMVECTOR vTemp = XM_PERMUTE_PS(V,_MM_SHUFFLE(1,1,1,1));
    return _mm_cvtss_f32(vTemp);
}

float XM_CALLCONV XMVectorGetZ(FXMVECTOR V)
{
    XMVECTOR vTemp = XM_PERMUTE_PS(V,_MM_SHUFFLE(2,2,2,2));
    return _mm_cvtss_f32(vTemp);
}

float XM_CALLCONV XMVectorGetW(FXMVECTOR V)
{
    XMVECTOR vTemp = XM_PERMUTE_PS(V,_MM_SHUFFLE(3,3,3,3));
    return _mm_cvtss_f32(vTemp);
}

XMVECTOR XM_CALLCONV XMVectorSetX(FXMVECTOR V, float x)
{
    XMVECTOR vResult = _mm_set_ss(x);
    vResult = _mm_move_ss(V,vResult);
    return vResult;
}

XMVECTOR XM_CALLCONV XMVectorSetY(FXMVECTOR V, float y)
{
    XMVECTOR vResult = _mm_set_ss(y);
    vResult = _mm_insert_ps( V, vResult, 0x10 );
    return vResult;
}

XMVECTOR XM_CALLCONV XMVectorSetZ(FXMVECTOR V, float z)
{
    XMVECTOR vResult = _mm_set_ss(z);
    vResult = _mm_insert_ps( V, vResult, 0x20 );
    return vResult;
}

XMVECTOR XM_CALLCONV XMVectorSetW(FXMVECTOR V, float w)
{
    XMVECTOR vResult = _mm_set_ss(w);
    vResult = _mm_insert_ps( V, vResult, 0x30 );
    return vResult;
}

XMVECTOR XM_CALLCONV XMVectorSelect
(
    FXMVECTOR V1, 
    FXMVECTOR V2, 
    FXMVECTOR Control
)
{
    XMVECTOR vTemp1 = _mm_andnot_ps(Control,V1);
    XMVECTOR vTemp2 = _mm_and_ps(V2,Control);
    return _mm_or_ps(vTemp1,vTemp2);
}

XMVECTOR XM_CALLCONV XMVectorNegate
(
    FXMVECTOR V
)
{
    XMVECTOR Z;

    Z = _mm_setzero_ps();

    return _mm_sub_ps( Z, V );
}

XMVECTOR XM_CALLCONV XMVectorAdd
(
    FXMVECTOR V1, 
    FXMVECTOR V2
)
{
    return _mm_add_ps( V1, V2 );
}

XMVECTOR XM_CALLCONV XMVectorSubtract
(
    FXMVECTOR V1, 
    FXMVECTOR V2
)
{
    return _mm_sub_ps( V1, V2 );
}

XMVECTOR XM_CALLCONV XMVectorMultiply
(
    FXMVECTOR V1, 
    FXMVECTOR V2
)
{
    return _mm_mul_ps( V1, V2 );
}

XMVECTOR XM_CALLCONV XMVectorMultiplyAdd
(
    FXMVECTOR V1, 
    FXMVECTOR V2, 
    FXMVECTOR V3
)
{
    return _mm_fmadd_ps( V1, V2, V3 );
}

XMVECTOR XM_CALLCONV XMVectorDivide
(
    FXMVECTOR V1, 
    FXMVECTOR V2
)
{
    return _mm_div_ps( V1, V2 );
}

XMVECTOR XM_CALLCONV XMVectorScale
(
    FXMVECTOR V, 
    float    ScaleFactor
)
{
   XMVECTOR vResult = _mm_set_ps1(ScaleFactor);
   return _mm_mul_ps(vResult,V);
}

XMVECTOR XM_CALLCONV XMVector3Dot
(
    FXMVECTOR V1, 
    FXMVECTOR V2
)
{
    return _mm_dp_ps( V1, V2, 0x7f );
}

XMVECTOR XM_CALLCONV XMVector3Cross
(
    FXMVECTOR V1, 
    FXMVECTOR V2
)
{
    // y1,z1,x1,w1
    XMVECTOR vTemp1 = XM_PERMUTE_PS(V1,_MM_SHUFFLE(3,0,2,1));
    // z2,x2,y2,w2
    XMVECTOR vTemp2 = XM_PERMUTE_PS(V2,_MM_SHUFFLE(3,1,0,2));
    // Perform the left operation
    XMVECTOR vResult = _mm_mul_ps(vTemp1,vTemp2);
    // z1,x1,y1,w1
    vTemp1 = XM_PERMUTE_PS(vTemp1,_MM_SHUFFLE(3,0,2,1));
    // y2,z2,x2,w2
    vTemp2 = XM_PERMUTE_PS(vTemp2,_MM_SHUFFLE(3,1,0,2));
    // Perform the right operation
    vTemp1 = _mm_mul_ps(vTemp1,vTemp2);
    // Subract the right from left, and return answer
    vResult = _mm_sub_ps(vResult,vTemp1);
    // Set w to zero
    return _mm_and_ps(vResult,g_XMMask3.v);
}

XMVECTOR XM_CALLCONV XMVector3Length
(
    FXMVECTOR V
)
{
    XMVECTOR vTemp = _mm_dp_ps( V, V, 0x7f );
    return _mm_sqrt_ps( vTemp );
}

XMVECTOR XM_CALLCONV XMVector3Normalize
(
    FXMVECTOR V
)
{
    XMVECTOR vLengthSq = _mm_dp_ps( V, V, 0x7f );
    // Prepare for the division
    XMVECTOR vResult = _mm_sqrt_ps(vLengthSq);
    // Create zero with a single instruction
    XMVECTOR vZeroMask = _mm_setzero_ps();
    // Test for a divide by zero (Must be FP to detect -0.0)
    vZeroMask = _mm_cmpneq_ps(vZeroMask,vResult);
    // Failsafe on zero (Or epsilon) length planes
    // If the length is infinity, set the elements to zero
    vLengthSq = _mm_cmpneq_ps(vLengthSq,g_XMInfinity.v);
    // Divide to perform the normalization
    vResult = _mm_div_ps(V,vResult);
    // Any that are infinity, set to zero
    vResult = _mm_and_ps(vResult,vZeroMask);
    // Select qnan or result based on infinite length
    XMVECTOR vTemp1 = _mm_andnot_ps(vLengthSq,g_XMQNaN.v);
    XMVECTOR vTemp2 = _mm_and_ps(vResult,vLengthSq);
    vResult = _mm_or_ps(vTemp1,vTemp2);
    return vResult;
}

XMVECTOR XM_CALLCONV XMVector4Dot
(
    FXMVECTOR V1, 
    FXMVECTOR V2
)
{
    return _mm_dp_ps( V1, V2, 0xff );
}