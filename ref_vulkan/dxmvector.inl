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

inline XMVECTOR XM_CALLCONV XMVectorZero()
{
    return _mm_setzero_ps();
}

inline XMVECTOR XM_CALLCONV XMVectorSet
(
    float x,
    float y,
    float z,
    float w
)
{
    return _mm_set_ps(w, z, y, x);
}

inline XMVECTOR XM_CALLCONV XMVectorSetX(FXMVECTOR V, float x)
{
    XMVECTOR vResult = _mm_set_ss(x);
    vResult = _mm_move_ss(V, vResult);
    return vResult;
}

inline XMVECTOR XM_CALLCONV XMVectorSetY(FXMVECTOR V, float y)
{
    // Swap y and x
    XMVECTOR vResult = XM_PERMUTE_PS(V, _MM_SHUFFLE(3, 2, 0, 1));
    // Convert input to vector
    XMVECTOR vTemp = _mm_set_ss(y);
    // Replace the x component
    vResult = _mm_move_ss(vResult, vTemp);
    // Swap y and x again
    vResult = XM_PERMUTE_PS(vResult, _MM_SHUFFLE(3, 2, 0, 1));
    return vResult;
}

inline XMVECTOR XM_CALLCONV XMVectorSetZ(FXMVECTOR V, float z)
{
    // Swap z and x
    XMVECTOR vResult = XM_PERMUTE_PS(V, _MM_SHUFFLE(3, 0, 1, 2));
    // Convert input to vector
    XMVECTOR vTemp = _mm_set_ss(z);
    // Replace the x component
    vResult = _mm_move_ss(vResult, vTemp);
    // Swap z and x again
    vResult = XM_PERMUTE_PS(vResult, _MM_SHUFFLE(3, 0, 1, 2));
    return vResult;
}

inline XMVECTOR XM_CALLCONV XMVectorSetW(FXMVECTOR V, float w)
{
    // Swap w and x
    XMVECTOR vResult = XM_PERMUTE_PS(V, _MM_SHUFFLE(0, 2, 1, 3));
    // Convert input to vector
    XMVECTOR vTemp = _mm_set_ss(w);
    // Replace the x component
    vResult = _mm_move_ss(vResult, vTemp);
    // Swap w and x again
    vResult = XM_PERMUTE_PS(vResult, _MM_SHUFFLE(0, 2, 1, 3));
    return vResult;
}

inline XMVECTOR XM_CALLCONV XMVectorSplatX
(
    FXMVECTOR V
)
{
    return XM_PERMUTE_PS(V, _MM_SHUFFLE(0, 0, 0, 0));
}

inline XMVECTOR XM_CALLCONV XMVectorSplatY
(
    FXMVECTOR V
)
{
    return XM_PERMUTE_PS(V, _MM_SHUFFLE(1, 1, 1, 1));
}

inline XMVECTOR XM_CALLCONV XMVectorSplatZ
(
    FXMVECTOR V
)
{
    return XM_PERMUTE_PS(V, _MM_SHUFFLE(2, 2, 2, 2));
}

inline XMVECTOR XM_CALLCONV XMVectorSplatW
(
    FXMVECTOR V
)
{
    return XM_PERMUTE_PS(V, _MM_SHUFFLE(3, 3, 3, 3));
}

inline XMVECTOR XM_CALLCONV XMVectorSelect
(
    FXMVECTOR V1,
    FXMVECTOR V2,
    FXMVECTOR Control
)
{
    XMVECTOR vTemp1 = _mm_andnot_ps(Control, V1);
    XMVECTOR vTemp2 = _mm_and_ps(V2, Control);
    return _mm_or_ps(vTemp1, vTemp2);
}

inline XMVECTOR XM_CALLCONV XMVectorNegate
(
    FXMVECTOR V
)
{
    XMVECTOR Z;
    Z = _mm_setzero_ps();
    return _mm_sub_ps(Z, V);
}

inline XMVECTOR XM_CALLCONV XMVectorAdd
(
    FXMVECTOR V1,
    FXMVECTOR V2
)
{
    return _mm_add_ps(V1, V2);
}

inline XMVECTOR XM_CALLCONV XMVectorSubtract
(
    FXMVECTOR V1,
    FXMVECTOR V2
)
{
    return _mm_sub_ps(V1, V2);
}

inline XMVECTOR XM_CALLCONV XMVectorMultiply
(
    FXMVECTOR V1,
    FXMVECTOR V2
)
{
    return _mm_mul_ps(V1, V2);
}

inline XMVECTOR XM_CALLCONV XMVectorMultiplyAdd
(
    FXMVECTOR V1,
    FXMVECTOR V2,
    FXMVECTOR V3
)
{
    return _mm_fmadd_ps(V1, V2, V3);
}

inline XMVECTOR XM_CALLCONV XMVector3Dot
(
    FXMVECTOR V1,
    FXMVECTOR V2
)
{
    // Perform the dot product
    XMVECTOR vDot = _mm_mul_ps(V1, V2);
    // x=Dot.vector4_f32[1], y=Dot.vector4_f32[2]
    XMVECTOR vTemp = XM_PERMUTE_PS(vDot, _MM_SHUFFLE(2, 1, 2, 1));
    // Result.vector4_f32[0] = x+y
    vDot = _mm_add_ss(vDot, vTemp);
    // x=Dot.vector4_f32[2]
    vTemp = XM_PERMUTE_PS(vTemp, _MM_SHUFFLE(1, 1, 1, 1));
    // Result.vector4_f32[0] = (x+y)+z
    vDot = _mm_add_ss(vDot, vTemp);
    // Splat x
    return XM_PERMUTE_PS(vDot, _MM_SHUFFLE(0, 0, 0, 0));
}

inline XMVECTOR XM_CALLCONV XMVector3Cross
(
    FXMVECTOR V1,
    FXMVECTOR V2
)
{
    // y1,z1,x1,w1
    XMVECTOR vTemp1 = XM_PERMUTE_PS(V1, _MM_SHUFFLE(3, 0, 2, 1));
    // z2,x2,y2,w2
    XMVECTOR vTemp2 = XM_PERMUTE_PS(V2, _MM_SHUFFLE(3, 1, 0, 2));
    // Perform the left operation
    XMVECTOR vResult = _mm_mul_ps(vTemp1, vTemp2);
    // z1,x1,y1,w1
    vTemp1 = XM_PERMUTE_PS(vTemp1, _MM_SHUFFLE(3, 0, 2, 1));
    // y2,z2,x2,w2
    vTemp2 = XM_PERMUTE_PS(vTemp2, _MM_SHUFFLE(3, 1, 0, 2));
    // Perform the right operation
    vTemp1 = _mm_mul_ps(vTemp1, vTemp2);
    // Subract the right from left, and return answer
    vResult = _mm_sub_ps(vResult, vTemp1);
    // Set w to zero
    return _mm_and_ps(vResult, g_XMMask3.v);
}

inline XMVECTOR XM_CALLCONV XMVector3Normalize
(
    FXMVECTOR V
)
{
    // Perform the dot product on x,y and z only
    XMVECTOR vLengthSq = _mm_mul_ps(V, V);
    XMVECTOR vTemp = XM_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(2, 1, 2, 1));
    vLengthSq = _mm_add_ss(vLengthSq, vTemp);
    vTemp = XM_PERMUTE_PS(vTemp, _MM_SHUFFLE(1, 1, 1, 1));
    vLengthSq = _mm_add_ss(vLengthSq, vTemp);
    vLengthSq = XM_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(0, 0, 0, 0));
    // Prepare for the division
    XMVECTOR vResult = _mm_sqrt_ps(vLengthSq);
    // Create zero with a single instruction
    XMVECTOR vZeroMask = _mm_setzero_ps();
    // Test for a divide by zero (Must be FP to detect -0.0)
    vZeroMask = _mm_cmpneq_ps(vZeroMask, vResult);
    // Failsafe on zero (Or epsilon) length planes
    // If the length is infinity, set the elements to zero
    vLengthSq = _mm_cmpneq_ps(vLengthSq, g_XMInfinity.v);
    // Divide to perform the normalization
    vResult = _mm_div_ps(V, vResult);
    // Any that are infinity, set to zero
    vResult = _mm_and_ps(vResult, vZeroMask);
    // Select qnan or result based on infinite length
    XMVECTOR vTemp1 = _mm_andnot_ps(vLengthSq, g_XMQNaN.v);
    XMVECTOR vTemp2 = _mm_and_ps(vResult, vLengthSq);
    vResult = _mm_or_ps(vTemp1, vTemp2);
    return vResult;
}

inline XMVECTOR XM_CALLCONV XMVector4Dot
(
    FXMVECTOR V1,
    FXMVECTOR V2
)
{
    XMVECTOR vTemp2 = V2;
    XMVECTOR vTemp = _mm_mul_ps(V1, vTemp2);
    vTemp2 = _mm_shuffle_ps(vTemp2, vTemp, _MM_SHUFFLE(1, 0, 0, 0)); // Copy X to the Z position and Y to the W position
    vTemp2 = _mm_add_ps(vTemp2, vTemp);          // Add Z = X+Z; W = Y+W;
    vTemp = _mm_shuffle_ps(vTemp, vTemp2, _MM_SHUFFLE(0, 3, 0, 0));  // Copy W to the Z position
    vTemp = _mm_add_ps(vTemp, vTemp2);           // Add Z and W together
    return XM_PERMUTE_PS(vTemp, _MM_SHUFFLE(2, 2, 2, 2));    // Splat Z and return
}
