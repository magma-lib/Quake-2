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

inline XMMATRIX XM_CALLCONV XMMatrixMultiply
(
    FXMMATRIX *M1, 
    FXMMATRIX *M2
)
{
    XMMATRIX mResult;
    // Splat the component X,Y,Z then W
#if defined(_XM_AVX_INTRINSICS_) && (!defined(_MSC_VER) || (_MSC_VER >= 1800))
    XMVECTOR vX = _mm_broadcast_ss((const float *)(&M1->r[0]) + 0);
    XMVECTOR vY = _mm_broadcast_ss((const float *)(&M1->r[0]) + 1);
    XMVECTOR vZ = _mm_broadcast_ss((const float *)(&M1->r[0]) + 2);
    XMVECTOR vW = _mm_broadcast_ss((const float *)(&M1->r[0]) + 3);
#else
    // Use vW to hold the original row
    XMVECTOR vW = M1->r[0];
    XMVECTOR vX = XM_PERMUTE_PS(vW,_MM_SHUFFLE(0,0,0,0));
    XMVECTOR vY = XM_PERMUTE_PS(vW,_MM_SHUFFLE(1,1,1,1));
    XMVECTOR vZ = XM_PERMUTE_PS(vW,_MM_SHUFFLE(2,2,2,2));
    vW = XM_PERMUTE_PS(vW,_MM_SHUFFLE(3,3,3,3));
#endif
    // Perform the operation on the first row
    vX = _mm_mul_ps(vX,M2->r[0]);
    vY = _mm_mul_ps(vY,M2->r[1]);
    vZ = _mm_mul_ps(vZ,M2->r[2]);
    vW = _mm_mul_ps(vW,M2->r[3]);
    // Perform a binary add to reduce cumulative errors
    vX = _mm_add_ps(vX,vZ);
    vY = _mm_add_ps(vY,vW);
    vX = _mm_add_ps(vX,vY);
    mResult.r[0] = vX;
    // Repeat for the other 3 rows
#if defined(_XM_AVX_INTRINSICS_) && (!defined(_MSC_VER) || (_MSC_VER >= 1800))
    vX = _mm_broadcast_ss((const float *)(&M1->r[1]) + 0);
    vY = _mm_broadcast_ss((const float *)(&M1->r[1]) + 1);
    vZ = _mm_broadcast_ss((const float *)(&M1->r[1]) + 2);
    vW = _mm_broadcast_ss((const float *)(&M1->r[1]) + 3);
#else
    vW = M1->r[1];
    vX = XM_PERMUTE_PS(vW,_MM_SHUFFLE(0,0,0,0));
    vY = XM_PERMUTE_PS(vW,_MM_SHUFFLE(1,1,1,1));
    vZ = XM_PERMUTE_PS(vW,_MM_SHUFFLE(2,2,2,2));
    vW = XM_PERMUTE_PS(vW,_MM_SHUFFLE(3,3,3,3));
#endif
    vX = _mm_mul_ps(vX,M2->r[0]);
    vY = _mm_mul_ps(vY,M2->r[1]);
    vZ = _mm_mul_ps(vZ,M2->r[2]);
    vW = _mm_mul_ps(vW,M2->r[3]);
    vX = _mm_add_ps(vX,vZ);
    vY = _mm_add_ps(vY,vW);
    vX = _mm_add_ps(vX,vY);
    mResult.r[1] = vX;
#if defined(_XM_AVX_INTRINSICS_) && (!defined(_MSC_VER) || (_MSC_VER >= 1800))
    vX = _mm_broadcast_ss((const float *)(&M1->r[2]) + 0);
    vY = _mm_broadcast_ss((const float *)(&M1->r[2]) + 1);
    vZ = _mm_broadcast_ss((const float *)(&M1->r[2]) + 2);
    vW = _mm_broadcast_ss((const float *)(&M1->r[2]) + 3);
#else
    vW = M1->r[2];
    vX = XM_PERMUTE_PS(vW,_MM_SHUFFLE(0,0,0,0));
    vY = XM_PERMUTE_PS(vW,_MM_SHUFFLE(1,1,1,1));
    vZ = XM_PERMUTE_PS(vW,_MM_SHUFFLE(2,2,2,2));
    vW = XM_PERMUTE_PS(vW,_MM_SHUFFLE(3,3,3,3));
#endif
    vX = _mm_mul_ps(vX,M2->r[0]);
    vY = _mm_mul_ps(vY,M2->r[1]);
    vZ = _mm_mul_ps(vZ,M2->r[2]);
    vW = _mm_mul_ps(vW,M2->r[3]);
    vX = _mm_add_ps(vX,vZ);
    vY = _mm_add_ps(vY,vW);
    vX = _mm_add_ps(vX,vY);
    mResult.r[2] = vX;
#if defined(_XM_AVX_INTRINSICS_) && (!defined(_MSC_VER) || (_MSC_VER >= 1800))
    vX = _mm_broadcast_ss((const float *)(&M1->r[3]) + 0);
    vY = _mm_broadcast_ss((const float *)(&M1->r[3]) + 1);
    vZ = _mm_broadcast_ss((const float *)(&M1->r[3]) + 2);
    vW = _mm_broadcast_ss((const float *)(&M1->r[3]) + 3);
#else
    vW = M1->r[3];
    vX = XM_PERMUTE_PS(vW,_MM_SHUFFLE(0,0,0,0));
    vY = XM_PERMUTE_PS(vW,_MM_SHUFFLE(1,1,1,1));
    vZ = XM_PERMUTE_PS(vW,_MM_SHUFFLE(2,2,2,2));
    vW = XM_PERMUTE_PS(vW,_MM_SHUFFLE(3,3,3,3));
#endif
    vX = _mm_mul_ps(vX,M2->r[0]);
    vY = _mm_mul_ps(vY,M2->r[1]);
    vZ = _mm_mul_ps(vZ,M2->r[2]);
    vW = _mm_mul_ps(vW,M2->r[3]);
    vX = _mm_add_ps(vX,vZ);
    vY = _mm_add_ps(vY,vW);
    vX = _mm_add_ps(vX,vY);
    mResult.r[3] = vX;
    return mResult;
}

inline XMMATRIX XM_CALLCONV XMMatrixMultiplyTranspose
(
    FXMMATRIX *M1, 
    FXMMATRIX *M2
)
{
    // Splat the component X,Y,Z then W
#if defined(_XM_AVX_INTRINSICS_) && (!defined(_MSC_VER) || (_MSC_VER >= 1800))
    XMVECTOR vX = _mm_broadcast_ss((const float *)(&M1->r[0]) + 0);
    XMVECTOR vY = _mm_broadcast_ss((const float *)(&M1->r[0]) + 1);
    XMVECTOR vZ = _mm_broadcast_ss((const float *)(&M1->r[0]) + 2);
    XMVECTOR vW = _mm_broadcast_ss((const float *)(&M1->r[0]) + 3);
#else
    // Use vW to hold the original row
    XMVECTOR vW = M1->r[0];
    XMVECTOR vX = XM_PERMUTE_PS(vW,_MM_SHUFFLE(0,0,0,0));
    XMVECTOR vY = XM_PERMUTE_PS(vW,_MM_SHUFFLE(1,1,1,1));
    XMVECTOR vZ = XM_PERMUTE_PS(vW,_MM_SHUFFLE(2,2,2,2));
    vW = XM_PERMUTE_PS(vW,_MM_SHUFFLE(3,3,3,3));
#endif
    // Perform the operation on the first row
    vX = _mm_mul_ps(vX,M2->r[0]);
    vY = _mm_mul_ps(vY,M2->r[1]);
    vZ = _mm_mul_ps(vZ,M2->r[2]);
    vW = _mm_mul_ps(vW,M2->r[3]);
    // Perform a binary add to reduce cumulative errors
    vX = _mm_add_ps(vX,vZ);
    vY = _mm_add_ps(vY,vW);
    vX = _mm_add_ps(vX,vY);
    XMVECTOR r0 = vX;
    // Repeat for the other 3 rows
#if defined(_XM_AVX_INTRINSICS_) && (!defined(_MSC_VER) || (_MSC_VER >= 1800))
    vX = _mm_broadcast_ss((const float *)(&M1->r[1]) + 0);
    vY = _mm_broadcast_ss((const float *)(&M1->r[1]) + 1);
    vZ = _mm_broadcast_ss((const float *)(&M1->r[1]) + 2);
    vW = _mm_broadcast_ss((const float *)(&M1->r[1]) + 3);
#else
    vW = M1->r[1];
    vX = XM_PERMUTE_PS(vW,_MM_SHUFFLE(0,0,0,0));
    vY = XM_PERMUTE_PS(vW,_MM_SHUFFLE(1,1,1,1));
    vZ = XM_PERMUTE_PS(vW,_MM_SHUFFLE(2,2,2,2));
    vW = XM_PERMUTE_PS(vW,_MM_SHUFFLE(3,3,3,3));
#endif
    vX = _mm_mul_ps(vX,M2->r[0]);
    vY = _mm_mul_ps(vY,M2->r[1]);
    vZ = _mm_mul_ps(vZ,M2->r[2]);
    vW = _mm_mul_ps(vW,M2->r[3]);
    vX = _mm_add_ps(vX,vZ);
    vY = _mm_add_ps(vY,vW);
    vX = _mm_add_ps(vX,vY);
    XMVECTOR r1 = vX;
#if defined(_XM_AVX_INTRINSICS_) && (!defined(_MSC_VER) || (_MSC_VER >= 1800))
    vX = _mm_broadcast_ss((const float *)(&M1->r[2]) + 0);
    vY = _mm_broadcast_ss((const float *)(&M1->r[2]) + 1);
    vZ = _mm_broadcast_ss((const float *)(&M1->r[2]) + 2);
    vW = _mm_broadcast_ss((const float *)(&M1->r[2]) + 3);
#else
    vW = M1->r[2];
    vX = XM_PERMUTE_PS(vW,_MM_SHUFFLE(0,0,0,0));
    vY = XM_PERMUTE_PS(vW,_MM_SHUFFLE(1,1,1,1));
    vZ = XM_PERMUTE_PS(vW,_MM_SHUFFLE(2,2,2,2));
    vW = XM_PERMUTE_PS(vW,_MM_SHUFFLE(3,3,3,3));
#endif
    vX = _mm_mul_ps(vX,M2->r[0]);
    vY = _mm_mul_ps(vY,M2->r[1]);
    vZ = _mm_mul_ps(vZ,M2->r[2]);
    vW = _mm_mul_ps(vW,M2->r[3]);
    vX = _mm_add_ps(vX,vZ);
    vY = _mm_add_ps(vY,vW);
    vX = _mm_add_ps(vX,vY);
    XMVECTOR r2 = vX;
#if defined(_XM_AVX_INTRINSICS_) && (!defined(_MSC_VER) || (_MSC_VER >= 1800))
    vX = _mm_broadcast_ss((const float *)(&M1->r[3]) + 0);
    vY = _mm_broadcast_ss((const float *)(&M1->r[3]) + 1);
    vZ = _mm_broadcast_ss((const float *)(&M1->r[3]) + 2);
    vW = _mm_broadcast_ss((const float *)(&M1->r[3]) + 3);
#else
    vW = M1->r[3];
    vX = XM_PERMUTE_PS(vW,_MM_SHUFFLE(0,0,0,0));
    vY = XM_PERMUTE_PS(vW,_MM_SHUFFLE(1,1,1,1));
    vZ = XM_PERMUTE_PS(vW,_MM_SHUFFLE(2,2,2,2));
    vW = XM_PERMUTE_PS(vW,_MM_SHUFFLE(3,3,3,3));
#endif
    vX = _mm_mul_ps(vX,M2->r[0]);
    vY = _mm_mul_ps(vY,M2->r[1]);
    vZ = _mm_mul_ps(vZ,M2->r[2]);
    vW = _mm_mul_ps(vW,M2->r[3]);
    vX = _mm_add_ps(vX,vZ);
    vY = _mm_add_ps(vY,vW);
    vX = _mm_add_ps(vX,vY);
    XMVECTOR r3 = vX;

    // x.x,x.y,y.x,y.y
    XMVECTOR vTemp1 = _mm_shuffle_ps(r0,r1,_MM_SHUFFLE(1,0,1,0));
    // x.z,x.w,y.z,y.w
    XMVECTOR vTemp3 = _mm_shuffle_ps(r0,r1,_MM_SHUFFLE(3,2,3,2));
    // z.x,z.y,w.x,w.y
    XMVECTOR vTemp2 = _mm_shuffle_ps(r2,r3,_MM_SHUFFLE(1,0,1,0));
    // z.z,z.w,w.z,w.w
    XMVECTOR vTemp4 = _mm_shuffle_ps(r2,r3,_MM_SHUFFLE(3,2,3,2));

    XMMATRIX mResult;
    // x.x,y.x,z.x,w.x
    mResult.r[0] = _mm_shuffle_ps(vTemp1, vTemp2,_MM_SHUFFLE(2,0,2,0));
    // x.y,y.y,z.y,w.y
    mResult.r[1] = _mm_shuffle_ps(vTemp1, vTemp2,_MM_SHUFFLE(3,1,3,1));
    // x.z,y.z,z.z,w.z
    mResult.r[2] = _mm_shuffle_ps(vTemp3, vTemp4,_MM_SHUFFLE(2,0,2,0));
    // x.w,y.w,z.w,w.w
    mResult.r[3] = _mm_shuffle_ps(vTemp3, vTemp4,_MM_SHUFFLE(3,1,3,1));
    return mResult;
}

inline XMMATRIX XM_CALLCONV XMMatrixTranspose
(
    FXMMATRIX *M
)
{
    // x.x,x.y,y.x,y.y
    XMVECTOR vTemp1 = _mm_shuffle_ps(M->r[0],M->r[1],_MM_SHUFFLE(1,0,1,0));
    // x.z,x.w,y.z,y.w
    XMVECTOR vTemp3 = _mm_shuffle_ps(M->r[0],M->r[1],_MM_SHUFFLE(3,2,3,2));
    // z.x,z.y,w.x,w.y
    XMVECTOR vTemp2 = _mm_shuffle_ps(M->r[2],M->r[3],_MM_SHUFFLE(1,0,1,0));
    // z.z,z.w,w.z,w.w
    XMVECTOR vTemp4 = _mm_shuffle_ps(M->r[2],M->r[3],_MM_SHUFFLE(3,2,3,2));
    XMMATRIX mResult;

    // x.x,y.x,z.x,w.x
    mResult.r[0] = _mm_shuffle_ps(vTemp1, vTemp2,_MM_SHUFFLE(2,0,2,0));
    // x.y,y.y,z.y,w.y
    mResult.r[1] = _mm_shuffle_ps(vTemp1, vTemp2,_MM_SHUFFLE(3,1,3,1));
    // x.z,y.z,z.z,w.z
    mResult.r[2] = _mm_shuffle_ps(vTemp3, vTemp4,_MM_SHUFFLE(2,0,2,0));
    // x.w,y.w,z.w,w.w
    mResult.r[3] = _mm_shuffle_ps(vTemp3, vTemp4,_MM_SHUFFLE(3,1,3,1));
    return mResult;
}

// Return the inverse and the determinant of a 4x4 matrix
inline XMMATRIX XM_CALLCONV XMMatrixInverse
(
    XMVECTOR *pDeterminant, 
    FXMMATRIX *M
)
{
    XMMATRIX MT = XMMatrixTranspose(M);
    XMVECTOR V00 = XM_PERMUTE_PS(MT.r[2],_MM_SHUFFLE(1,1,0,0));
    XMVECTOR V10 = XM_PERMUTE_PS(MT.r[3],_MM_SHUFFLE(3,2,3,2));
    XMVECTOR V01 = XM_PERMUTE_PS(MT.r[0],_MM_SHUFFLE(1,1,0,0));
    XMVECTOR V11 = XM_PERMUTE_PS(MT.r[1],_MM_SHUFFLE(3,2,3,2));
    XMVECTOR V02 = _mm_shuffle_ps(MT.r[2], MT.r[0],_MM_SHUFFLE(2,0,2,0));
    XMVECTOR V12 = _mm_shuffle_ps(MT.r[3], MT.r[1],_MM_SHUFFLE(3,1,3,1));

    XMVECTOR D0 = _mm_mul_ps(V00,V10);
    XMVECTOR D1 = _mm_mul_ps(V01,V11);
    XMVECTOR D2 = _mm_mul_ps(V02,V12);

    V00 = XM_PERMUTE_PS(MT.r[2],_MM_SHUFFLE(3,2,3,2));
    V10 = XM_PERMUTE_PS(MT.r[3],_MM_SHUFFLE(1,1,0,0));
    V01 = XM_PERMUTE_PS(MT.r[0],_MM_SHUFFLE(3,2,3,2));
    V11 = XM_PERMUTE_PS(MT.r[1],_MM_SHUFFLE(1,1,0,0));
    V02 = _mm_shuffle_ps(MT.r[2],MT.r[0],_MM_SHUFFLE(3,1,3,1));
    V12 = _mm_shuffle_ps(MT.r[3],MT.r[1],_MM_SHUFFLE(2,0,2,0));

    V00 = _mm_mul_ps(V00,V10);
    V01 = _mm_mul_ps(V01,V11);
    V02 = _mm_mul_ps(V02,V12);
    D0 = _mm_sub_ps(D0,V00);
    D1 = _mm_sub_ps(D1,V01);
    D2 = _mm_sub_ps(D2,V02);
    // V11 = D0Y,D0W,D2Y,D2Y
    V11 = _mm_shuffle_ps(D0,D2,_MM_SHUFFLE(1,1,3,1));
    V00 = XM_PERMUTE_PS(MT.r[1], _MM_SHUFFLE(1,0,2,1));
    V10 = _mm_shuffle_ps(V11,D0,_MM_SHUFFLE(0,3,0,2));
    V01 = XM_PERMUTE_PS(MT.r[0], _MM_SHUFFLE(0,1,0,2));
    V11 = _mm_shuffle_ps(V11,D0,_MM_SHUFFLE(2,1,2,1));
    // V13 = D1Y,D1W,D2W,D2W
    XMVECTOR V13 = _mm_shuffle_ps(D1,D2,_MM_SHUFFLE(3,3,3,1));
    V02 = XM_PERMUTE_PS(MT.r[3], _MM_SHUFFLE(1,0,2,1));
    V12 = _mm_shuffle_ps(V13,D1,_MM_SHUFFLE(0,3,0,2));
    XMVECTOR V03 = XM_PERMUTE_PS(MT.r[2],_MM_SHUFFLE(0,1,0,2));
    V13 = _mm_shuffle_ps(V13,D1,_MM_SHUFFLE(2,1,2,1));

    XMVECTOR C0 = _mm_mul_ps(V00,V10);
    XMVECTOR C2 = _mm_mul_ps(V01,V11);
    XMVECTOR C4 = _mm_mul_ps(V02,V12);
    XMVECTOR C6 = _mm_mul_ps(V03,V13);

    // V11 = D0X,D0Y,D2X,D2X
    V11 = _mm_shuffle_ps(D0,D2,_MM_SHUFFLE(0,0,1,0));
    V00 = XM_PERMUTE_PS(MT.r[1], _MM_SHUFFLE(2,1,3,2));
    V10 = _mm_shuffle_ps(D0,V11,_MM_SHUFFLE(2,1,0,3));
    V01 = XM_PERMUTE_PS(MT.r[0], _MM_SHUFFLE(1,3,2,3));
    V11 = _mm_shuffle_ps(D0,V11,_MM_SHUFFLE(0,2,1,2));
    // V13 = D1X,D1Y,D2Z,D2Z
    V13 = _mm_shuffle_ps(D1,D2,_MM_SHUFFLE(2,2,1,0));
    V02 = XM_PERMUTE_PS(MT.r[3], _MM_SHUFFLE(2,1,3,2));
    V12 = _mm_shuffle_ps(D1,V13,_MM_SHUFFLE(2,1,0,3));
    V03 = XM_PERMUTE_PS(MT.r[2],_MM_SHUFFLE(1,3,2,3));
    V13 = _mm_shuffle_ps(D1,V13,_MM_SHUFFLE(0,2,1,2));

    V00 = _mm_mul_ps(V00,V10);
    V01 = _mm_mul_ps(V01,V11);
    V02 = _mm_mul_ps(V02,V12);
    V03 = _mm_mul_ps(V03,V13);
    C0 = _mm_sub_ps(C0,V00);
    C2 = _mm_sub_ps(C2,V01);
    C4 = _mm_sub_ps(C4,V02);
    C6 = _mm_sub_ps(C6,V03);

    V00 = XM_PERMUTE_PS(MT.r[1],_MM_SHUFFLE(0,3,0,3));
    // V10 = D0Z,D0Z,D2X,D2Y
    V10 = _mm_shuffle_ps(D0,D2,_MM_SHUFFLE(1,0,2,2));
    V10 = XM_PERMUTE_PS(V10,_MM_SHUFFLE(0,2,3,0));
    V01 = XM_PERMUTE_PS(MT.r[0],_MM_SHUFFLE(2,0,3,1));
    // V11 = D0X,D0W,D2X,D2Y
    V11 = _mm_shuffle_ps(D0,D2,_MM_SHUFFLE(1,0,3,0));
    V11 = XM_PERMUTE_PS(V11,_MM_SHUFFLE(2,1,0,3));
    V02 = XM_PERMUTE_PS(MT.r[3],_MM_SHUFFLE(0,3,0,3));
    // V12 = D1Z,D1Z,D2Z,D2W
    V12 = _mm_shuffle_ps(D1,D2,_MM_SHUFFLE(3,2,2,2));
    V12 = XM_PERMUTE_PS(V12,_MM_SHUFFLE(0,2,3,0));
    V03 = XM_PERMUTE_PS(MT.r[2],_MM_SHUFFLE(2,0,3,1));
    // V13 = D1X,D1W,D2Z,D2W
    V13 = _mm_shuffle_ps(D1,D2,_MM_SHUFFLE(3,2,3,0));
    V13 = XM_PERMUTE_PS(V13,_MM_SHUFFLE(2,1,0,3));

    V00 = _mm_mul_ps(V00,V10);
    V01 = _mm_mul_ps(V01,V11);
    V02 = _mm_mul_ps(V02,V12);
    V03 = _mm_mul_ps(V03,V13);
    XMVECTOR C1 = _mm_sub_ps(C0,V00);
    C0 = _mm_add_ps(C0,V00);
    XMVECTOR C3 = _mm_add_ps(C2,V01);
    C2 = _mm_sub_ps(C2,V01);
    XMVECTOR C5 = _mm_sub_ps(C4,V02);
    C4 = _mm_add_ps(C4,V02);
    XMVECTOR C7 = _mm_add_ps(C6,V03);
    C6 = _mm_sub_ps(C6,V03);

    C0 = _mm_shuffle_ps(C0,C1,_MM_SHUFFLE(3,1,2,0));
    C2 = _mm_shuffle_ps(C2,C3,_MM_SHUFFLE(3,1,2,0));
    C4 = _mm_shuffle_ps(C4,C5,_MM_SHUFFLE(3,1,2,0));
    C6 = _mm_shuffle_ps(C6,C7,_MM_SHUFFLE(3,1,2,0));
    C0 = XM_PERMUTE_PS(C0,_MM_SHUFFLE(3,1,2,0));
    C2 = XM_PERMUTE_PS(C2,_MM_SHUFFLE(3,1,2,0));
    C4 = XM_PERMUTE_PS(C4,_MM_SHUFFLE(3,1,2,0));
    C6 = XM_PERMUTE_PS(C6,_MM_SHUFFLE(3,1,2,0));
    // Get the determinate
    XMVECTOR vTemp = XMVector4Dot(C0,MT.r[0]);
    if (pDeterminant)
        *pDeterminant = vTemp;
    vTemp = _mm_div_ps(g_XMOne.v,vTemp);
    XMMATRIX mResult;
    mResult.r[0] = _mm_mul_ps(C0,vTemp);
    mResult.r[1] = _mm_mul_ps(C2,vTemp);
    mResult.r[2] = _mm_mul_ps(C4,vTemp);
    mResult.r[3] = _mm_mul_ps(C6,vTemp);
    return mResult;
}

inline XMMATRIX XM_CALLCONV XMMatrixIdentity()
{
    XMMATRIX M;
    M.r[0] = g_XMIdentityR0.v;
    M.r[1] = g_XMIdentityR1.v;
    M.r[2] = g_XMIdentityR2.v;
    M.r[3] = g_XMIdentityR3.v;
    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixSet
(
    float m00, float m01, float m02, float m03,
    float m10, float m11, float m12, float m13,
    float m20, float m21, float m22, float m23,
    float m30, float m31, float m32, float m33
)
{
    XMMATRIX M;
    M.r[0] = XMVectorSet(m00, m01, m02, m03);
    M.r[1] = XMVectorSet(m10, m11, m12, m13);
    M.r[2] = XMVectorSet(m20, m21, m22, m23);
    M.r[3] = XMVectorSet(m30, m31, m32, m33);
    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixTranslation
(
    float OffsetX, 
    float OffsetY, 
    float OffsetZ
)
{
    XMMATRIX M;
    M.r[0] = g_XMIdentityR0.v;
    M.r[1] = g_XMIdentityR1.v;
    M.r[2] = g_XMIdentityR2.v;
    M.r[3] = XMVectorSet(OffsetX, OffsetY, OffsetZ, 1.f);
    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixTranslationFromVector
(
    FXMVECTOR Offset
)
{
    XMMATRIX M;
    M.r[0] = g_XMIdentityR0.v;
    M.r[1] = g_XMIdentityR1.v;
    M.r[2] = g_XMIdentityR2.v;
    M.r[3] = XMVectorSelect(g_XMIdentityR3.v, Offset, g_XMSelect1110.v);
    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixScaling
(
    float ScaleX, 
    float ScaleY, 
    float ScaleZ
)
{
    XMMATRIX M;
    M.r[0] = _mm_set_ps(0, 0, 0, ScaleX);
    M.r[1] = _mm_set_ps(0, 0, ScaleY, 0);
    M.r[2] = _mm_set_ps(0, ScaleZ, 0, 0);
    M.r[3] = g_XMIdentityR3.v;
    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixScalingFromVector
(
    FXMVECTOR Scale
)
{
    XMMATRIX M;
    M.r[0] = _mm_and_ps(Scale,g_XMMaskX.v);
    M.r[1] = _mm_and_ps(Scale,g_XMMaskY.v);
    M.r[2] = _mm_and_ps(Scale,g_XMMaskZ.v);
    M.r[3] = g_XMIdentityR3.v;
    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixRotationX
(
    float Angle
)
{
    float    SinAngle;
    float    CosAngle;

    SinAngle = sin(Angle);
    CosAngle = cos(Angle);

    XMVECTOR vSin = _mm_set_ss(SinAngle);
    XMVECTOR vCos = _mm_set_ss(CosAngle);
    // x = 0,y = cos,z = sin, w = 0
    vCos = _mm_shuffle_ps(vCos,vSin,_MM_SHUFFLE(3,0,0,3));
    XMMATRIX M;
    M.r[0] = g_XMIdentityR0.v;
    M.r[1] = vCos;
    // x = 0,y = sin,z = cos, w = 0
    vCos = XM_PERMUTE_PS(vCos,_MM_SHUFFLE(3,1,2,0));
    // x = 0,y = -sin,z = cos, w = 0
    vCos = _mm_mul_ps(vCos,g_XMNegateY.v);
    M.r[2] = vCos;
    M.r[3] = g_XMIdentityR3.v;
    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixRotationY
(
    float Angle
)
{
    float    SinAngle;
    float    CosAngle;
    
    SinAngle = sin(Angle);
    CosAngle = cos(Angle);

    XMVECTOR vSin = _mm_set_ss(SinAngle);
    XMVECTOR vCos = _mm_set_ss(CosAngle);
    // x = sin,y = 0,z = cos, w = 0
    vSin = _mm_shuffle_ps(vSin,vCos,_MM_SHUFFLE(3,0,3,0));
    XMMATRIX M;
    M.r[2] = vSin;
    M.r[1] = g_XMIdentityR1.v;
    // x = cos,y = 0,z = sin, w = 0
    vSin = XM_PERMUTE_PS(vSin,_MM_SHUFFLE(3,0,1,2));
    // x = cos,y = 0,z = -sin, w = 0
    vSin = _mm_mul_ps(vSin,g_XMNegateZ.v);
    M.r[0] = vSin;
    M.r[3] = g_XMIdentityR3.v;
    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixRotationZ
(
    float Angle
)
{
    float    SinAngle;
    float    CosAngle;
    
    SinAngle = sin(Angle);
    CosAngle = cos(Angle);

    XMVECTOR vSin = _mm_set_ss(SinAngle);
    XMVECTOR vCos = _mm_set_ss(CosAngle);
    // x = cos,y = sin,z = 0, w = 0
    vCos = _mm_unpacklo_ps(vCos,vSin);
    XMMATRIX M;
    M.r[0] = vCos;
    // x = sin,y = cos,z = 0, w = 0
    vCos = XM_PERMUTE_PS(vCos,_MM_SHUFFLE(3,2,0,1));
    // x = cos,y = -sin,z = 0, w = 0
    vCos = _mm_mul_ps(vCos,g_XMNegateX.v);
    M.r[1] = vCos;
    M.r[2] = g_XMIdentityR2.v;
    M.r[3] = g_XMIdentityR3.v;
    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixRotationNormal
(
    FXMVECTOR NormalAxis, 
    float     Angle
)
{
    float    SinAngle;
    float    CosAngle;
    
    SinAngle = sin(Angle);
    CosAngle = cos(Angle);

    XMVECTOR C2 = _mm_set_ps1(1.0f - CosAngle);
    XMVECTOR C1 = _mm_set_ps1(CosAngle);
    XMVECTOR C0 = _mm_set_ps1(SinAngle);

    XMVECTOR N0 = XM_PERMUTE_PS(NormalAxis,_MM_SHUFFLE(3,0,2,1));
    XMVECTOR N1 = XM_PERMUTE_PS(NormalAxis,_MM_SHUFFLE(3,1,0,2));

    XMVECTOR V0 = _mm_mul_ps(C2, N0);
    V0 = _mm_mul_ps(V0, N1);

    XMVECTOR R0 = _mm_mul_ps(C2, NormalAxis);
    R0 = _mm_mul_ps(R0, NormalAxis);
    R0 = _mm_add_ps(R0, C1);

    XMVECTOR R1 = _mm_mul_ps(C0, NormalAxis);
    R1 = _mm_add_ps(R1, V0);
    XMVECTOR R2 = _mm_mul_ps(C0, NormalAxis);
    R2 = _mm_sub_ps(V0,R2);

    V0 = _mm_and_ps(R0,g_XMMask3.v);
    XMVECTOR V1 = _mm_shuffle_ps(R1,R2,_MM_SHUFFLE(2,1,2,0));
    V1 = XM_PERMUTE_PS(V1,_MM_SHUFFLE(0,3,2,1));
    XMVECTOR V2 = _mm_shuffle_ps(R1,R2,_MM_SHUFFLE(0,0,1,1));
    V2 = XM_PERMUTE_PS(V2,_MM_SHUFFLE(2,0,2,0));

    R2 = _mm_shuffle_ps(V0,V1,_MM_SHUFFLE(1,0,3,0));
    R2 = XM_PERMUTE_PS(R2,_MM_SHUFFLE(1,3,2,0));

    XMMATRIX M;
    M.r[0] = R2;

    R2 = _mm_shuffle_ps(V0,V1,_MM_SHUFFLE(3,2,3,1));
    R2 = XM_PERMUTE_PS(R2,_MM_SHUFFLE(1,3,0,2));
    M.r[1] = R2;

    V2 = _mm_shuffle_ps(V2,V0,_MM_SHUFFLE(3,2,1,0));
    M.r[2] = V2;
    M.r[3] = g_XMIdentityR3.v;
    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixRotationAxis
(
    FXMVECTOR Axis, 
    float     Angle
)
{
    XMVECTOR Normal = XMVector3Normalize(Axis);
    return XMMatrixRotationNormal(Normal, Angle);
}

inline XMMATRIX XM_CALLCONV XMMatrixLookToLH
(
    FXMVECTOR EyePosition,
    FXMVECTOR EyeDirection,
    FXMVECTOR UpDirection
)
{
    XMVECTOR R2 = XMVector3Normalize(EyeDirection);

    XMVECTOR R0 = XMVector3Cross(UpDirection, R2);
    R0 = XMVector3Normalize(R0);

    XMVECTOR R1 = XMVector3Cross(R2, R0);

    XMVECTOR NegEyePosition = XMVectorNegate(EyePosition);

    XMVECTOR D0 = XMVector3Dot(R0, NegEyePosition);
    XMVECTOR D1 = XMVector3Dot(R1, NegEyePosition);
    XMVECTOR D2 = XMVector3Dot(R2, NegEyePosition);

    XMMATRIX M;
    M.r[0] = XMVectorSelect(D0, R0, g_XMSelect1110.v);
    M.r[1] = XMVectorSelect(D1, R1, g_XMSelect1110.v);
    M.r[2] = XMVectorSelect(D2, R2, g_XMSelect1110.v);
    M.r[3] = g_XMIdentityR3.v;

    M = XMMatrixTranspose(&M);

    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixLookToRH
(
    FXMVECTOR EyePosition, 
    FXMVECTOR EyeDirection, 
    FXMVECTOR UpDirection
)
{
    XMVECTOR NegEyeDirection = XMVectorNegate(EyeDirection);
    return XMMatrixLookToLH(EyePosition, NegEyeDirection, UpDirection);
}

inline XMMATRIX XM_CALLCONV XMMatrixLookAtLH
(
    FXMVECTOR EyePosition,
    FXMVECTOR FocusPosition,
    FXMVECTOR UpDirection
)
{
    XMVECTOR EyeDirection = XMVectorSubtract(FocusPosition, EyePosition);
    return XMMatrixLookToLH(EyePosition, EyeDirection, UpDirection);
}

inline XMMATRIX XM_CALLCONV XMMatrixLookAtRH
(
    FXMVECTOR EyePosition,
    FXMVECTOR FocusPosition,
    FXMVECTOR UpDirection
)
{
    XMVECTOR NegEyeDirection = XMVectorSubtract(EyePosition, FocusPosition);
    return XMMatrixLookToLH(EyePosition, NegEyeDirection, UpDirection);
}

#ifdef _PREFAST_
#pragma prefast(push)
#pragma prefast(disable:28931, "PREfast noise: Esp:1266")
#endif

inline XMMATRIX XM_CALLCONV XMMatrixPerspectiveLH
(
    float ViewWidth, 
    float ViewHeight, 
    float NearZ, 
    float FarZ
)
{
    XMMATRIX M;
    float TwoNearZ = NearZ + NearZ;
    float fRange = FarZ / (FarZ - NearZ);
    // Note: This is recorded on the stack
    XMVECTOR rMem = {
        TwoNearZ / ViewWidth,
        TwoNearZ / ViewHeight,
        fRange,
        -fRange * NearZ
    };
    // Copy from memory to SSE register
    XMVECTOR vValues = rMem;
    XMVECTOR vTemp = _mm_setzero_ps(); 
    // Copy x only
    vTemp = _mm_move_ss(vTemp,vValues);
    // TwoNearZ / ViewWidth,0,0,0
    M.r[0] = vTemp;
    // 0,TwoNearZ / ViewHeight,0,0
    vTemp = vValues;
    vTemp = _mm_and_ps(vTemp,g_XMMaskY.v);
    M.r[1] = vTemp;
    // x=fRange,y=-fRange * NearZ,0,1.0f
    vValues = _mm_shuffle_ps(vValues,g_XMIdentityR3.v,_MM_SHUFFLE(3,2,3,2));
    // 0,0,fRange,1.0f
    vTemp = _mm_setzero_ps();
    vTemp = _mm_shuffle_ps(vTemp,vValues,_MM_SHUFFLE(3,0,0,0));
    M.r[2] = vTemp;
    // 0,0,-fRange * NearZ,0
    vTemp = _mm_shuffle_ps(vTemp,vValues,_MM_SHUFFLE(2,1,0,0));
    M.r[3] = vTemp;
    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixPerspectiveRH
(
    float ViewWidth, 
    float ViewHeight, 
    float NearZ, 
    float FarZ
)
{
    XMMATRIX M;
    float TwoNearZ = NearZ + NearZ;
    float fRange = FarZ / (NearZ-FarZ);
    // Note: This is recorded on the stack
    XMVECTOR rMem = {
        TwoNearZ / ViewWidth,
        TwoNearZ / ViewHeight,
        fRange,
        fRange * NearZ
    };
    // Copy from memory to SSE register
    XMVECTOR vValues = rMem;
    XMVECTOR vTemp = _mm_setzero_ps(); 
    // Copy x only
    vTemp = _mm_move_ss(vTemp,vValues);
    // TwoNearZ / ViewWidth,0,0,0
    M.r[0] = vTemp;
    // 0,TwoNearZ / ViewHeight,0,0
    vTemp = vValues;
    vTemp = _mm_and_ps(vTemp,g_XMMaskY.v);
    M.r[1] = vTemp;
    // x=fRange,y=-fRange * NearZ,0,-1.0f
    vValues = _mm_shuffle_ps(vValues,g_XMNegIdentityR3.v,_MM_SHUFFLE(3,2,3,2));
    // 0,0,fRange,-1.0f
    vTemp = _mm_setzero_ps();
    vTemp = _mm_shuffle_ps(vTemp,vValues,_MM_SHUFFLE(3,0,0,0));
    M.r[2] = vTemp;
    // 0,0,-fRange * NearZ,0
    vTemp = _mm_shuffle_ps(vTemp,vValues,_MM_SHUFFLE(2,1,0,0));
    M.r[3] = vTemp;
    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixPerspectiveFovLH
(
    float FovAngleY, 
    float AspectRatio, 
    float NearZ, 
    float FarZ
)
{
    float    SinFov;
    float    CosFov;
    
    SinFov = sin(0.5f * FovAngleY);
    CosFov = cos(0.5f * FovAngleY);

    float fRange = FarZ / (FarZ-NearZ);
    // Note: This is recorded on the stack
    float Height = CosFov / SinFov;
    XMVECTOR rMem = {
        Height / AspectRatio,
        Height,
        fRange,
        -fRange * NearZ
    };
    // Copy from memory to SSE register
    XMVECTOR vValues = rMem;
    XMVECTOR vTemp = _mm_setzero_ps(); 
    // Copy x only
    vTemp = _mm_move_ss(vTemp,vValues);
    // CosFov / SinFov,0,0,0
    XMMATRIX M;
    M.r[0] = vTemp;
    // 0,Height / AspectRatio,0,0
    vTemp = vValues;
    vTemp = _mm_and_ps(vTemp,g_XMMaskY.v);
    M.r[1] = vTemp;
    // x=fRange,y=-fRange * NearZ,0,1.0f
    vTemp = _mm_setzero_ps();
    vValues = _mm_shuffle_ps(vValues,g_XMIdentityR3.v,_MM_SHUFFLE(3,2,3,2));
    // 0,0,fRange,1.0f
    vTemp = _mm_shuffle_ps(vTemp,vValues,_MM_SHUFFLE(3,0,0,0));
    M.r[2] = vTemp;
    // 0,0,-fRange * NearZ,0.0f
    vTemp = _mm_shuffle_ps(vTemp,vValues,_MM_SHUFFLE(2,1,0,0));
    M.r[3] = vTemp;
    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixPerspectiveFovRH
(
    float FovAngleY, 
    float AspectRatio, 
    float NearZ, 
    float FarZ
)
{
    float    SinFov;
    float    CosFov;
    
    SinFov = sin(0.5f * FovAngleY);
    CosFov = cos(0.5f * FovAngleY);
    float fRange = FarZ / (NearZ-FarZ);
    // Note: This is recorded on the stack
    float Height = CosFov / SinFov;
    XMVECTOR rMem = {
        Height / AspectRatio,
        Height,
        fRange,
        fRange * NearZ
    };
    // Copy from memory to SSE register
    XMVECTOR vValues = rMem;
    XMVECTOR vTemp = _mm_setzero_ps(); 
    // Copy x only
    vTemp = _mm_move_ss(vTemp,vValues);
    // CosFov / SinFov,0,0,0
    XMMATRIX M;
    M.r[0] = vTemp;
    // 0,Height / AspectRatio,0,0
    vTemp = vValues;
    vTemp = _mm_and_ps(vTemp,g_XMMaskY.v);
    M.r[1] = vTemp;
    // x=fRange,y=-fRange * NearZ,0,-1.0f
    vTemp = _mm_setzero_ps();
    vValues = _mm_shuffle_ps(vValues,g_XMNegIdentityR3.v,_MM_SHUFFLE(3,2,3,2));
    // 0,0,fRange,-1.0f
    vTemp = _mm_shuffle_ps(vTemp,vValues,_MM_SHUFFLE(3,0,0,0));
    M.r[2] = vTemp;
    // 0,0,fRange * NearZ,0.0f
    vTemp = _mm_shuffle_ps(vTemp,vValues,_MM_SHUFFLE(2,1,0,0));
    M.r[3] = vTemp;
    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixPerspectiveOffCenterLH
(
    float ViewLeft, 
    float ViewRight, 
    float ViewBottom, 
    float ViewTop, 
    float NearZ, 
    float FarZ
)
{
    XMMATRIX M;
    float TwoNearZ = NearZ+NearZ;
    float ReciprocalWidth = 1.0f / (ViewRight - ViewLeft);
    float ReciprocalHeight = 1.0f / (ViewTop - ViewBottom);
    float fRange = FarZ / (FarZ-NearZ);
    // Note: This is recorded on the stack
    XMVECTOR rMem = {
        TwoNearZ*ReciprocalWidth,
        TwoNearZ*ReciprocalHeight,
        -fRange * NearZ,
        0
    };
    // Copy from memory to SSE register
    XMVECTOR vValues = rMem;
    XMVECTOR vTemp = _mm_setzero_ps(); 
    // Copy x only
    vTemp = _mm_move_ss(vTemp,vValues);
    // TwoNearZ*ReciprocalWidth,0,0,0
    M.r[0] = vTemp;
    // 0,TwoNearZ*ReciprocalHeight,0,0
    vTemp = vValues;
    vTemp = _mm_and_ps(vTemp,g_XMMaskY.v);
    M.r[1] = vTemp;
    // 0,0,fRange,1.0f
    M.r[2] = XMVectorSet( -(ViewLeft + ViewRight) * ReciprocalWidth,
                          -(ViewTop + ViewBottom) * ReciprocalHeight,
                          fRange,
                          1.0f );
    // 0,0,-fRange * NearZ,0.0f
    vValues = _mm_and_ps(vValues,g_XMMaskZ.v);
    M.r[3] = vValues;
    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixPerspectiveOffCenterRH
(
    float ViewLeft, 
    float ViewRight, 
    float ViewBottom, 
    float ViewTop, 
    float NearZ, 
    float FarZ
)
{
    XMMATRIX M;
    float TwoNearZ = NearZ+NearZ;
    float ReciprocalWidth = 1.0f / (ViewRight - ViewLeft);
    float ReciprocalHeight = 1.0f / (ViewTop - ViewBottom);
    float fRange = FarZ / (NearZ-FarZ);
    // Note: This is recorded on the stack
    XMVECTOR rMem = {
        TwoNearZ*ReciprocalWidth,
        TwoNearZ*ReciprocalHeight,
        fRange * NearZ,
        0
    };
    // Copy from memory to SSE register
    XMVECTOR vValues = rMem;
    XMVECTOR vTemp = _mm_setzero_ps(); 
    // Copy x only
    vTemp = _mm_move_ss(vTemp,vValues);
    // TwoNearZ*ReciprocalWidth,0,0,0
    M.r[0] = vTemp;
    // 0,TwoNearZ*ReciprocalHeight,0,0
    vTemp = vValues;
    vTemp = _mm_and_ps(vTemp,g_XMMaskY.v);
    M.r[1] = vTemp;
    // 0,0,fRange,1.0f
    M.r[2] = XMVectorSet( (ViewLeft + ViewRight) * ReciprocalWidth,
                          (ViewTop + ViewBottom) * ReciprocalHeight,
                          fRange,
                          -1.0f );
    // 0,0,-fRange * NearZ,0.0f
    vValues = _mm_and_ps(vValues,g_XMMaskZ.v);
    M.r[3] = vValues;
    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixOrthographicLH
(
    float ViewWidth, 
    float ViewHeight, 
    float NearZ, 
    float FarZ
)
{
    XMMATRIX M;
    float fRange = 1.0f / (FarZ-NearZ);
    // Note: This is recorded on the stack
    XMVECTOR rMem = {
        2.0f / ViewWidth,
        2.0f / ViewHeight,
        fRange,
        -fRange * NearZ
    };
    // Copy from memory to SSE register
    XMVECTOR vValues = rMem;
    XMVECTOR vTemp = _mm_setzero_ps(); 
    // Copy x only
    vTemp = _mm_move_ss(vTemp,vValues);
    // 2.0f / ViewWidth,0,0,0
    M.r[0] = vTemp;
    // 0,2.0f / ViewHeight,0,0
    vTemp = vValues;
    vTemp = _mm_and_ps(vTemp,g_XMMaskY.v);
    M.r[1] = vTemp;
    // x=fRange,y=-fRange * NearZ,0,1.0f
    vTemp = _mm_setzero_ps();
    vValues = _mm_shuffle_ps(vValues,g_XMIdentityR3.v,_MM_SHUFFLE(3,2,3,2));
    // 0,0,fRange,0.0f
    vTemp = _mm_shuffle_ps(vTemp,vValues,_MM_SHUFFLE(2,0,0,0));
    M.r[2] = vTemp;
    // 0,0,-fRange * NearZ,1.0f
    vTemp = _mm_shuffle_ps(vTemp,vValues,_MM_SHUFFLE(3,1,0,0));
    M.r[3] = vTemp;
    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixOrthographicRH
(
    float ViewWidth, 
    float ViewHeight, 
    float NearZ, 
    float FarZ
)
{
    XMMATRIX M;
    float fRange = 1.0f / (NearZ-FarZ);
    // Note: This is recorded on the stack
    XMVECTOR rMem = {
        2.0f / ViewWidth,
        2.0f / ViewHeight,
        fRange,
        fRange * NearZ
    };
    // Copy from memory to SSE register
    XMVECTOR vValues = rMem;
    XMVECTOR vTemp = _mm_setzero_ps(); 
    // Copy x only
    vTemp = _mm_move_ss(vTemp,vValues);
    // 2.0f / ViewWidth,0,0,0
    M.r[0] = vTemp;
    // 0,2.0f / ViewHeight,0,0
    vTemp = vValues;
    vTemp = _mm_and_ps(vTemp,g_XMMaskY.v);
    M.r[1] = vTemp;
    // x=fRange,y=fRange * NearZ,0,1.0f
    vTemp = _mm_setzero_ps();
    vValues = _mm_shuffle_ps(vValues,g_XMIdentityR3.v,_MM_SHUFFLE(3,2,3,2));
    // 0,0,fRange,0.0f
    vTemp = _mm_shuffle_ps(vTemp,vValues,_MM_SHUFFLE(2,0,0,0));
    M.r[2] = vTemp;
    // 0,0,fRange * NearZ,1.0f
    vTemp = _mm_shuffle_ps(vTemp,vValues,_MM_SHUFFLE(3,1,0,0));
    M.r[3] = vTemp;
    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixOrthographicOffCenterLH
(
    float ViewLeft, 
    float ViewRight, 
    float ViewBottom, 
    float ViewTop, 
    float NearZ, 
    float FarZ
)
{
    XMMATRIX M;
    float fReciprocalWidth = 1.0f / (ViewRight - ViewLeft);
    float fReciprocalHeight = 1.0f / (ViewTop - ViewBottom);
    float fRange = 1.0f / (FarZ-NearZ);
    // Note: This is recorded on the stack
    XMVECTOR rMem = {
        fReciprocalWidth,
        fReciprocalHeight,
        fRange,
        1.0f
    };
    XMVECTOR rMem2 = {
        -(ViewLeft + ViewRight),
        -(ViewTop + ViewBottom),
        -NearZ,
        1.0f
    };
    // Copy from memory to SSE register
    XMVECTOR vValues = rMem;
    XMVECTOR vTemp = _mm_setzero_ps(); 
    // Copy x only
    vTemp = _mm_move_ss(vTemp,vValues);
    // fReciprocalWidth*2,0,0,0
    vTemp = _mm_add_ss(vTemp,vTemp);
    M.r[0] = vTemp;
    // 0,fReciprocalHeight*2,0,0
    vTemp = vValues;
    vTemp = _mm_and_ps(vTemp,g_XMMaskY.v);
    vTemp = _mm_add_ps(vTemp,vTemp);
    M.r[1] = vTemp;
    // 0,0,fRange,0.0f
    vTemp = vValues;
    vTemp = _mm_and_ps(vTemp,g_XMMaskZ.v);
    M.r[2] = vTemp;
    // -(ViewLeft + ViewRight)*fReciprocalWidth,-(ViewTop + ViewBottom)*fReciprocalHeight,fRange*-NearZ,1.0f
    vValues = _mm_mul_ps(vValues,rMem2);
    M.r[3] = vValues;
    return M;
}

inline XMMATRIX XM_CALLCONV XMMatrixOrthographicOffCenterRH
(
    float ViewLeft, 
    float ViewRight, 
    float ViewBottom, 
    float ViewTop, 
    float NearZ, 
    float FarZ
)
{
    XMMATRIX M;
    float fReciprocalWidth = 1.0f / (ViewRight - ViewLeft);
    float fReciprocalHeight = 1.0f / (ViewTop - ViewBottom);
    float fRange = 1.0f / (NearZ-FarZ);
    // Note: This is recorded on the stack
    XMVECTOR rMem = {
        fReciprocalWidth,
        fReciprocalHeight,
        fRange,
        1.0f
    };
    XMVECTOR rMem2 = {
        -(ViewLeft + ViewRight),
        -(ViewTop + ViewBottom),
        NearZ,
        1.0f
    };
    // Copy from memory to SSE register
    XMVECTOR vValues = rMem;
    XMVECTOR vTemp = _mm_setzero_ps(); 
    // Copy x only
    vTemp = _mm_move_ss(vTemp,vValues);
    // fReciprocalWidth*2,0,0,0
    vTemp = _mm_add_ss(vTemp,vTemp);
    M.r[0] = vTemp;
    // 0,fReciprocalHeight*2,0,0
    vTemp = vValues;
    vTemp = _mm_and_ps(vTemp,g_XMMaskY.v);
    vTemp = _mm_add_ps(vTemp,vTemp);
    M.r[1] = vTemp;
    // 0,0,fRange,0.0f
    vTemp = vValues;
    vTemp = _mm_and_ps(vTemp,g_XMMaskZ.v);
    M.r[2] = vTemp;
    // -(ViewLeft + ViewRight)*fReciprocalWidth,-(ViewTop + ViewBottom)*fReciprocalHeight,fRange*-NearZ,1.0f
    vValues = _mm_mul_ps(vValues,rMem2);
    M.r[3] = vValues;
    return M;
}

#ifdef _PREFAST_
#pragma prefast(pop)
#endif
