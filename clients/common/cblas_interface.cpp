/*******************************************************************************
 *
 * MIT License
 *
 * Copyright (C) 2022-2023 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *******************************************************************************/
#include "cblas_interface.hpp"
#include "hipblaslt_vector.hpp"
#include "utility.hpp"
#include <bitset>
#include <iostream>
#include <omp.h>

CBLAS_TRANSPOSE HIPOperationToCBLASTanspose(hipblasOperation_t trans)
{
    switch(trans)
    {
    case HIPBLAS_OP_N:
        return CblasNoTrans;
    case HIPBLAS_OP_T:
        return CblasTrans;
    case HIPBLAS_OP_C:
        return CblasConjTrans;
    }
}

// gemm
template <>
void cblas_gemm<hip_bfloat16, hip_bfloat16, hip_bfloat16, float>(hipblasOperation_t  transA,
                                                                 hipblasOperation_t  transB,
                                                                 int64_t             m,
                                                                 int64_t             n,
                                                                 int64_t             k,
                                                                 float               alpha,
                                                                 const hip_bfloat16* A,
                                                                 int64_t             lda,
                                                                 const hip_bfloat16* B,
                                                                 int64_t             ldb,
                                                                 float               beta,
                                                                 hip_bfloat16*       C,
                                                                 int64_t             ldc,
                                                                 float               scaleD,
                                                                 bool                alt)
{
    // cblas does not support hip_bfloat16, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
        A_float[i] = static_cast<float>(A[i]);
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);
    for(size_t i = 0; i < sizeC; i++)
        C_float[i] = static_cast<float>(C[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C_float,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hip_bfloat16>(C_float[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hip_bfloat16>(C_float[i]);
    }
}

template <>
void cblas_gemm<hip_bfloat16, hip_bfloat16, float, float>(hipblasOperation_t  transA,
                                                          hipblasOperation_t  transB,
                                                          int64_t             m,
                                                          int64_t             n,
                                                          int64_t             k,
                                                          float               alpha,
                                                          const hip_bfloat16* A,
                                                          int64_t             lda,
                                                          const hip_bfloat16* B,
                                                          int64_t             ldb,
                                                          float               beta,
                                                          float*              C,
                                                          int64_t             ldc,
                                                          float               scaleD,
                                                          bool                alt)
{
    // cblas does not support hip_bfloat16, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
        A_float[i] = static_cast<float>(A[i]);
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<float>(C[i] * scaleD);
    }
}

template <>
void cblas_gemm<hipblaslt_f8, hipblaslt_f8, float, float>(hipblasOperation_t  transA,
                                                          hipblasOperation_t  transB,
                                                          int64_t             m,
                                                          int64_t             n,
                                                          int64_t             k,
                                                          float               alpha,
                                                          const hipblaslt_f8* A,
                                                          int64_t             lda,
                                                          const hipblaslt_f8* B,
                                                          int64_t             ldb,
                                                          float               beta,
                                                          float*              C,
                                                          int64_t             ldc,
                                                          float               scaleD,
                                                          bool                alt)
{
    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB);

    for(size_t i = 0; i < sizeA; i++)
        A_float[i] = static_cast<float>(A[i]);
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C,
                ldc);
    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<float>(C[i] * scaleD);
    }
}

template <>
void cblas_gemm<hipblaslt_bf8, hipblaslt_f8, float, float>(hipblasOperation_t   transA,
                                                           hipblasOperation_t   transB,
                                                           int64_t              m,
                                                           int64_t              n,
                                                           int64_t              k,
                                                           float                alpha,
                                                           const hipblaslt_bf8* A,
                                                           int64_t              lda,
                                                           const hipblaslt_f8*  B,
                                                           int64_t              ldb,
                                                           float                beta,
                                                           float*               C,
                                                           int64_t              ldc,
                                                           float                scaleD,
                                                           bool                 alt)
{
    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB);

    for(size_t i = 0; i < sizeA; i++)
        A_float[i] = static_cast<float>(A[i]);
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C,
                ldc);
    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<float>(C[i] * scaleD);
    }
}

template <>
void cblas_gemm<hipblaslt_f8, hipblaslt_bf8, float, float>(hipblasOperation_t   transA,
                                                           hipblasOperation_t   transB,
                                                           int64_t              m,
                                                           int64_t              n,
                                                           int64_t              k,
                                                           float                alpha,
                                                           const hipblaslt_f8*  A,
                                                           int64_t              lda,
                                                           const hipblaslt_bf8* B,
                                                           int64_t              ldb,
                                                           float                beta,
                                                           float*               C,
                                                           int64_t              ldc,
                                                           float                scaleD,
                                                           bool                 alt)
{
    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB);

    for(size_t i = 0; i < sizeA; i++)
        A_float[i] = static_cast<float>(A[i]);
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<float>(C[i] * scaleD);
    }
}

template <>
void cblas_gemm<hipblaslt_f8, hipblaslt_f8, hipblasLtHalf, float>(hipblasOperation_t  transA,
                                                                  hipblasOperation_t  transB,
                                                                  int64_t             m,
                                                                  int64_t             n,
                                                                  int64_t             k,
                                                                  float               alpha,
                                                                  const hipblaslt_f8* A,
                                                                  int64_t             lda,
                                                                  const hipblaslt_f8* B,
                                                                  int64_t             ldb,
                                                                  float               beta,
                                                                  hipblasLtHalf*      C,
                                                                  int64_t             ldc,
                                                                  float               scaleD,
                                                                  bool                alt)
{
    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
        A_float[i] = static_cast<float>(A[i]);
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);
    for(size_t i = 0; i < sizeC; i++)
        C_float[i] = static_cast<float>(C[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C_float,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblasLtHalf>(C_float[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblasLtHalf>(C_float[i]);
    }
}

template <>
void cblas_gemm<hipblaslt_bf8, hipblaslt_f8, hipblasLtHalf, float>(hipblasOperation_t   transA,
                                                                   hipblasOperation_t   transB,
                                                                   int64_t              m,
                                                                   int64_t              n,
                                                                   int64_t              k,
                                                                   float                alpha,
                                                                   const hipblaslt_bf8* A,
                                                                   int64_t              lda,
                                                                   const hipblaslt_f8*  B,
                                                                   int64_t              ldb,
                                                                   float                beta,
                                                                   hipblasLtHalf*       C,
                                                                   int64_t              ldc,
                                                                   float                scaleD,
                                                                   bool                 alt)
{
    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
        A_float[i] = static_cast<float>(A[i]);
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);
    for(size_t i = 0; i < sizeC; i++)
        C_float[i] = static_cast<float>(C[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C_float,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblasLtHalf>(C_float[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblasLtHalf>(C_float[i]);
    }
}

template <>
void cblas_gemm<hipblaslt_f8, hipblaslt_bf8, hipblasLtHalf, float>(hipblasOperation_t   transA,
                                                                   hipblasOperation_t   transB,
                                                                   int64_t              m,
                                                                   int64_t              n,
                                                                   int64_t              k,
                                                                   float                alpha,
                                                                   const hipblaslt_f8*  A,
                                                                   int64_t              lda,
                                                                   const hipblaslt_bf8* B,
                                                                   int64_t              ldb,
                                                                   float                beta,
                                                                   hipblasLtHalf*       C,
                                                                   int64_t              ldc,
                                                                   float                scaleD,
                                                                   bool                 alt)
{
    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
        A_float[i] = static_cast<float>(A[i]);
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);
    for(size_t i = 0; i < sizeC; i++)
        C_float[i] = static_cast<float>(C[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C_float,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblasLtHalf>(C_float[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblasLtHalf>(C_float[i]);
    }
}

template <>
void cblas_gemm<hipblaslt_f8, hipblasLtHalf, hipblaslt_f8, float>(hipblasOperation_t   transA,
                                                                  hipblasOperation_t   transB,
                                                                  int64_t              m,
                                                                  int64_t              n,
                                                                  int64_t              k,
                                                                  float                alpha,
                                                                  const hipblaslt_f8*  A,
                                                                  int64_t              lda,
                                                                  const hipblasLtHalf* B,
                                                                  int64_t              ldb,
                                                                  float                beta,
                                                                  hipblaslt_f8*        C,
                                                                  int64_t              ldc,
                                                                  float                scaleD,
                                                                  bool                 alt)
{
    // cblas does not support hip_bfloat16, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
        A_float[i] = static_cast<float>(A[i]);
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);
    for(size_t i = 0; i < sizeC; i++)
        C_float[i] = static_cast<float>(C[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C_float,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblaslt_f8>(C_float[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblaslt_f8>(C_float[i]);
    }
}

template <>
void cblas_gemm<hipblasLtHalf, hipblaslt_f8, hipblaslt_f8, float>(hipblasOperation_t   transA,
                                                                  hipblasOperation_t   transB,
                                                                  int64_t              m,
                                                                  int64_t              n,
                                                                  int64_t              k,
                                                                  float                alpha,
                                                                  const hipblasLtHalf* A,
                                                                  int64_t              lda,
                                                                  const hipblaslt_f8*  B,
                                                                  int64_t              ldb,
                                                                  float                beta,
                                                                  hipblaslt_f8*        C,
                                                                  int64_t              ldc,
                                                                  float                scaleD,
                                                                  bool                 alt)
{
    // cblas does not support hip_bfloat16, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
        A_float[i] = static_cast<float>(A[i]);
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);
    for(size_t i = 0; i < sizeC; i++)
        C_float[i] = static_cast<float>(C[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C_float,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblaslt_f8>(C_float[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblaslt_f8>(C_float[i]);
    }
}

template <>
void cblas_gemm<hipblaslt_f8, hipblasLtHalf, hipblasLtHalf, float>(hipblasOperation_t   transA,
                                                                   hipblasOperation_t   transB,
                                                                   int64_t              m,
                                                                   int64_t              n,
                                                                   int64_t              k,
                                                                   float                alpha,
                                                                   const hipblaslt_f8*  A,
                                                                   int64_t              lda,
                                                                   const hipblasLtHalf* B,
                                                                   int64_t              ldb,
                                                                   float                beta,
                                                                   hipblasLtHalf*       C,
                                                                   int64_t              ldc,
                                                                   float                scaleD,
                                                                   bool                 alt)
{
    // cblas does not support hip_bfloat16, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
        A_float[i] = static_cast<float>(A[i]);
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);
    for(size_t i = 0; i < sizeC; i++)
        C_float[i] = static_cast<float>(C[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C_float,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblasLtHalf>(C_float[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblasLtHalf>(C_float[i]);
    }
}

template <>
void cblas_gemm<hipblasLtHalf, hipblaslt_f8, hipblasLtHalf, float>(hipblasOperation_t   transA,
                                                                   hipblasOperation_t   transB,
                                                                   int64_t              m,
                                                                   int64_t              n,
                                                                   int64_t              k,
                                                                   float                alpha,
                                                                   const hipblasLtHalf* A,
                                                                   int64_t              lda,
                                                                   const hipblaslt_f8*  B,
                                                                   int64_t              ldb,
                                                                   float                beta,
                                                                   hipblasLtHalf*       C,
                                                                   int64_t              ldc,
                                                                   float                scaleD,
                                                                   bool                 alt)
{
    // cblas does not support hip_bfloat16, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
        A_float[i] = static_cast<float>(A[i]);
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);
    for(size_t i = 0; i < sizeC; i++)
        C_float[i] = static_cast<float>(C[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C_float,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblasLtHalf>(C_float[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblasLtHalf>(C_float[i]);
    }
}

template <>
void cblas_gemm<hipblaslt_f8, hipblasLtHalf, float, float>(hipblasOperation_t   transA,
                                                           hipblasOperation_t   transB,
                                                           int64_t              m,
                                                           int64_t              n,
                                                           int64_t              k,
                                                           float                alpha,
                                                           const hipblaslt_f8*  A,
                                                           int64_t              lda,
                                                           const hipblasLtHalf* B,
                                                           int64_t              ldb,
                                                           float                beta,
                                                           float*               C,
                                                           int64_t              ldc,
                                                           float                scaleD,
                                                           bool                 alt)
{
    // cblas does not support hip_bfloat16, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB);

    for(size_t i = 0; i < sizeA; i++)
        A_float[i] = static_cast<float>(A[i]);
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<float>(C[i] * scaleD);
    }
}

template <>
void cblas_gemm<hipblasLtHalf, hipblaslt_f8, float, float>(hipblasOperation_t   transA,
                                                           hipblasOperation_t   transB,
                                                           int64_t              m,
                                                           int64_t              n,
                                                           int64_t              k,
                                                           float                alpha,
                                                           const hipblasLtHalf* A,
                                                           int64_t              lda,
                                                           const hipblaslt_f8*  B,
                                                           int64_t              ldb,
                                                           float                beta,
                                                           float*               C,
                                                           int64_t              ldc,
                                                           float                scaleD,
                                                           bool                 alt)
{
    // cblas does not support hip_bfloat16, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB);

    for(size_t i = 0; i < sizeA; i++)
        A_float[i] = static_cast<float>(A[i]);
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<float>(C[i] * scaleD);
    }
}

template <>
void cblas_gemm<hipblasLtHalf, hipblasLtHalf, hipblasLtHalf, float>(hipblasOperation_t   transA,
                                                                    hipblasOperation_t   transB,
                                                                    int64_t              m,
                                                                    int64_t              n,
                                                                    int64_t              k,
                                                                    float                alpha,
                                                                    const hipblasLtHalf* A,
                                                                    int64_t              lda,
                                                                    const hipblasLtHalf* B,
                                                                    int64_t              ldb,
                                                                    float                beta,
                                                                    hipblasLtHalf*       C,
                                                                    int64_t              ldc,
                                                                    float                scaleD,
                                                                    bool                 alt)
{
    // cblas does not support hipblasLtHalf, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    if(alt)
    {
        for(size_t i = 0; i < sizeA; i++)
            A_float[i] = float_to_bfloat16_truncate(float(A[i]));
        for(size_t i = 0; i < sizeB; i++)
            B_float[i] = float_to_bfloat16_truncate(float(B[i]));
        for(size_t i = 0; i < sizeC; i++)
            C_float[i] = float_to_bfloat16_truncate(float(C[i]));
    }
    else
    {
        for(size_t i = 0; i < sizeA; i++)
            A_float[i] = A[i];
        for(size_t i = 0; i < sizeB; i++)
            B_float[i] = B[i];
        for(size_t i = 0; i < sizeC; i++)
            C_float[i] = C[i];
    }

    // just directly cast, since transA, transB are integers in the enum
    //printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C_float,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblasLtHalf>(C_float[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = hipblasLtHalf(C_float[i]);
    }
}

template <>
void cblas_gemm<hipblasLtHalf, hipblasLtHalf, float, float>(hipblasOperation_t   transA,
                                                            hipblasOperation_t   transB,
                                                            int64_t              m,
                                                            int64_t              n,
                                                            int64_t              k,
                                                            float                alpha,
                                                            const hipblasLtHalf* A,
                                                            int64_t              lda,
                                                            const hipblasLtHalf* B,
                                                            int64_t              ldb,
                                                            float                beta,
                                                            float*               C,
                                                            int64_t              ldc,
                                                            float                scaleD,
                                                            bool                 alt)
{
    // cblas does not support hipblasLtHalf, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB);

    if(alt)
    {
        for(size_t i = 0; i < sizeA; i++)
            A_float[i] = float_to_bfloat16_truncate(float(A[i]));
        for(size_t i = 0; i < sizeB; i++)
            B_float[i] = float_to_bfloat16_truncate(float(B[i]));
    }
    else
    {
        for(size_t i = 0; i < sizeA; i++)
            A_float[i] = A[i];
        for(size_t i = 0; i < sizeB; i++)
            B_float[i] = B[i];
    }

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<float>(C[i] * scaleD);
    }
}

template <>
void cblas_gemm<float, float, float, float>(hipblasOperation_t transA,
                                            hipblasOperation_t transB,
                                            int64_t            m,
                                            int64_t            n,
                                            int64_t            k,
                                            float              alpha,
                                            const float*       A,
                                            int64_t            lda,
                                            const float*       B,
                                            int64_t            ldb,
                                            float              beta,
                                            float*             C,
                                            int64_t            ldc,
                                            float              scaleD,
                                            bool               alt)
{
    size_t sizeC = n * size_t(ldc);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A,
                lda,
                B,
                ldb,
                beta,
                C,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<float>(C[i] * scaleD);
    }
}

template <>
void cblas_gemm<double, double, double, double>(hipblasOperation_t transA,
                                                hipblasOperation_t transB,
                                                int64_t            m,
                                                int64_t            n,
                                                int64_t            k,
                                                double             alpha,
                                                const double*      A,
                                                int64_t            lda,
                                                const double*      B,
                                                int64_t            ldb,
                                                double             beta,
                                                double*            C,
                                                int64_t            ldc,
                                                double             scaleD,
                                                bool               alt)
{
    size_t sizeC = n * size_t(ldc);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_dgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A,
                lda,
                B,
                ldb,
                beta,
                C,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<double>(C[i] * scaleD);
    }
}

template <>
void cblas_gemm<int8_t, int8_t, int32_t, int32_t>(hipblasOperation_t transA,
                                                  hipblasOperation_t transB,
                                                  int64_t            m,
                                                  int64_t            n,
                                                  int64_t            k,
                                                  int32_t            alpha,
                                                  const int8_t*      A,
                                                  int64_t            lda,
                                                  const int8_t*      B,
                                                  int64_t            ldb,
                                                  int32_t            beta,
                                                  int32_t*           C,
                                                  int64_t            ldc,
                                                  int32_t            scaleD,
                                                  bool               alt)
{

    // cblas does not support int8_t input / int32_t output, however non-overflowing
    // 32-bit integer operations can be represented accurately with double-precision
    // floats, so convert to doubles and downcast result down to int32_t.
    // NOTE: This will not properly account for 32-bit integer overflow, however
    //       the result should be acceptable for testing.

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<double> A_double(sizeA);
    host_vector<double> B_double(sizeB);
    host_vector<double> C_double(sizeC);

    for(size_t i = 0; i < sizeA; i++)
        A_double[i] = static_cast<double>(A[i]);
    for(size_t i = 0; i < sizeB; i++)
        B_double[i] = static_cast<double>(B[i]);
    for(size_t i = 0; i < sizeC; i++)
        C_double[i] = static_cast<double>(C[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_dgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_double,
                lda,
                B_double,
                ldb,
                beta,
                C_double,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<int32_t>(C_double[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<int32_t>(C_double[i]);
    }
}

// AlphaVec gemm
template <>
void cblas_gemm_alphascale<hip_bfloat16, hip_bfloat16, hip_bfloat16, float>(
    hipblasOperation_t  transA,
    hipblasOperation_t  transB,
    int64_t             m,
    int64_t             n,
    int64_t             k,
    float               alpha,
    const hip_bfloat16* A,
    int64_t             lda,
    const hip_bfloat16* B,
    int64_t             ldb,
    float               beta,
    hip_bfloat16*       C,
    int64_t             ldc,
    const float*        AlphaVec,
    float               scaleD,
    bool                alt)
{
    // cblas does not support hip_bfloat16, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
    {
        A_float[i] = static_cast<float>(A[i]);
        A_float[i] *= AlphaVec[i % m];
    }
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);
    for(size_t i = 0; i < sizeC; i++)
        C_float[i] = static_cast<float>(C[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C_float,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hip_bfloat16>(C_float[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hip_bfloat16>(C_float[i]);
    }
}

template <>
void cblas_gemm_alphascale<hip_bfloat16, hip_bfloat16, float, float>(hipblasOperation_t  transA,
                                                                     hipblasOperation_t  transB,
                                                                     int64_t             m,
                                                                     int64_t             n,
                                                                     int64_t             k,
                                                                     float               alpha,
                                                                     const hip_bfloat16* A,
                                                                     int64_t             lda,
                                                                     const hip_bfloat16* B,
                                                                     int64_t             ldb,
                                                                     float               beta,
                                                                     float*              C,
                                                                     int64_t             ldc,
                                                                     const float*        AlphaVec,
                                                                     float               scaleD,
                                                                     bool                alt)
{
    // cblas does not support hip_bfloat16, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB);

    for(size_t i = 0; i < sizeA; i++)
    {
        A_float[i] = static_cast<float>(A[i]);
        A_float[i] *= AlphaVec[i % m];
    }
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<float>(C[i] * scaleD);
    }
}

template <>
void cblas_gemm_alphascale<hipblaslt_f8, hipblaslt_f8, float, float>(hipblasOperation_t  transA,
                                                                     hipblasOperation_t  transB,
                                                                     int64_t             m,
                                                                     int64_t             n,
                                                                     int64_t             k,
                                                                     float               alpha,
                                                                     const hipblaslt_f8* A,
                                                                     int64_t             lda,
                                                                     const hipblaslt_f8* B,
                                                                     int64_t             ldb,
                                                                     float               beta,
                                                                     float*              C,
                                                                     int64_t             ldc,
                                                                     const float*        AlphaVec,
                                                                     float               scaleD,
                                                                     bool                alt)
{
    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
    {
        A_float[i] = static_cast<float>(A[i]);
        A_float[i] *= AlphaVec[i % m];
    }
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<float>(C[i] * scaleD);
    }
}

template <>
void cblas_gemm_alphascale<hipblaslt_bf8, hipblaslt_f8, float, float>(hipblasOperation_t   transA,
                                                                      hipblasOperation_t   transB,
                                                                      int64_t              m,
                                                                      int64_t              n,
                                                                      int64_t              k,
                                                                      float                alpha,
                                                                      const hipblaslt_bf8* A,
                                                                      int64_t              lda,
                                                                      const hipblaslt_f8*  B,
                                                                      int64_t              ldb,
                                                                      float                beta,
                                                                      float*               C,
                                                                      int64_t              ldc,
                                                                      const float*         AlphaVec,
                                                                      float                scaleD,
                                                                      bool                 alt)
{
    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
    {
        A_float[i] = static_cast<float>(A[i]);
        A_float[i] *= AlphaVec[i % m];
    }
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<float>(C[i] * scaleD);
    }
}

template <>
void cblas_gemm_alphascale<hipblaslt_f8, hipblaslt_bf8, float, float>(hipblasOperation_t   transA,
                                                                      hipblasOperation_t   transB,
                                                                      int64_t              m,
                                                                      int64_t              n,
                                                                      int64_t              k,
                                                                      float                alpha,
                                                                      const hipblaslt_f8*  A,
                                                                      int64_t              lda,
                                                                      const hipblaslt_bf8* B,
                                                                      int64_t              ldb,
                                                                      float                beta,
                                                                      float*               C,
                                                                      int64_t              ldc,
                                                                      const float*         AlphaVec,
                                                                      float                scaleD,
                                                                      bool                 alt)
{
    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
    {
        A_float[i] = static_cast<float>(A[i]);
        A_float[i] *= AlphaVec[i % m];
    }
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<float>(C[i] * scaleD);
    }
}
template <>
void cblas_gemm_alphascale<hipblaslt_f8, hipblaslt_f8, hipblasLtHalf, float>(
    hipblasOperation_t  transA,
    hipblasOperation_t  transB,
    int64_t             m,
    int64_t             n,
    int64_t             k,
    float               alpha,
    const hipblaslt_f8* A,
    int64_t             lda,
    const hipblaslt_f8* B,
    int64_t             ldb,
    float               beta,
    hipblasLtHalf*      C,
    int64_t             ldc,
    const float*        AlphaVec,
    float               scaleD,
    bool                alt)
{
    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
    {
        A_float[i] = static_cast<float>(A[i]);
        A_float[i] *= AlphaVec[i % m];
    }
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);
    for(size_t i = 0; i < sizeC; i++)
        C_float[i] = static_cast<float>(C[i]);

    // just directly cast, since transA, transB are integers in the enum
    //printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C_float,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblasLtHalf>(C_float[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = hipblasLtHalf(C_float[i]);
    }
}

template <>
void cblas_gemm_alphascale<hipblaslt_bf8, hipblaslt_f8, hipblasLtHalf, float>(
    hipblasOperation_t   transA,
    hipblasOperation_t   transB,
    int64_t              m,
    int64_t              n,
    int64_t              k,
    float                alpha,
    const hipblaslt_bf8* A,
    int64_t              lda,
    const hipblaslt_f8*  B,
    int64_t              ldb,
    float                beta,
    hipblasLtHalf*       C,
    int64_t              ldc,
    const float*         AlphaVec,
    float                scaleD,
    bool                 alt)
{
    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
    {
        A_float[i] = static_cast<float>(A[i]);
        A_float[i] *= AlphaVec[i % m];
    }
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);
    for(size_t i = 0; i < sizeC; i++)
        C_float[i] = static_cast<float>(C[i]);

    // just directly cast, since transA, transB are integers in the enum
    //printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C_float,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblasLtHalf>(C_float[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = hipblasLtHalf(C_float[i]);
    }
}

template <>
void cblas_gemm_alphascale<hipblaslt_f8, hipblaslt_bf8, hipblasLtHalf, float>(
    hipblasOperation_t   transA,
    hipblasOperation_t   transB,
    int64_t              m,
    int64_t              n,
    int64_t              k,
    float                alpha,
    const hipblaslt_f8*  A,
    int64_t              lda,
    const hipblaslt_bf8* B,
    int64_t              ldb,
    float                beta,
    hipblasLtHalf*       C,
    int64_t              ldc,
    const float*         AlphaVec,
    float                scaleD,
    bool                 alt)
{
    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
    {
        A_float[i] = static_cast<float>(A[i]);
        A_float[i] *= AlphaVec[i % m];
    }
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);
    for(size_t i = 0; i < sizeC; i++)
        C_float[i] = static_cast<float>(C[i]);

    // just directly cast, since transA, transB are integers in the enum
    //printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C_float,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblasLtHalf>(C_float[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = hipblasLtHalf(C_float[i]);
    }
}

template <>
void cblas_gemm_alphascale<hipblasLtHalf, hipblaslt_f8, hipblaslt_f8, float>(
    hipblasOperation_t   transA,
    hipblasOperation_t   transB,
    int64_t              m,
    int64_t              n,
    int64_t              k,
    float                alpha,
    const hipblasLtHalf* A,
    int64_t              lda,
    const hipblaslt_f8*  B,
    int64_t              ldb,
    float                beta,
    hipblaslt_f8*        C,
    int64_t              ldc,
    const float*         AlphaVec,
    float                scaleD,
    bool                 alt)
{
    // cblas does not support hipblasLtHalf, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
    {
        A_float[i] = static_cast<float>(A[i]);
        A_float[i] *= AlphaVec[i % m];
    }
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);
    for(size_t i = 0; i < sizeC; i++)
        C_float[i] = static_cast<float>(C[i]);

    // just directly cast, since transA, transB are integers in the enum
    //printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C_float,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblaslt_f8>(C_float[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblaslt_f8>(C_float[i]);
    }
}

template <>
void cblas_gemm_alphascale<hipblaslt_f8, hipblasLtHalf, hipblaslt_f8, float>(
    hipblasOperation_t   transA,
    hipblasOperation_t   transB,
    int64_t              m,
    int64_t              n,
    int64_t              k,
    float                alpha,
    const hipblaslt_f8*  A,
    int64_t              lda,
    const hipblasLtHalf* B,
    int64_t              ldb,
    float                beta,
    hipblaslt_f8*        C,
    int64_t              ldc,
    const float*         AlphaVec,
    float                scaleD,
    bool                 alt)
{
    // cblas does not support hipblasLtHalf, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
    {
        A_float[i] = static_cast<float>(A[i]);
        A_float[i] *= AlphaVec[i % m];
    }
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);
    for(size_t i = 0; i < sizeC; i++)
        C_float[i] = static_cast<float>(C[i]);

    // just directly cast, since transA, transB are integers in the enum
    //printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C_float,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblaslt_f8>(C_float[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblaslt_f8>(C_float[i]);
    }
}

template <>
void cblas_gemm_alphascale<hipblasLtHalf, hipblaslt_f8, hipblasLtHalf, float>(
    hipblasOperation_t   transA,
    hipblasOperation_t   transB,
    int64_t              m,
    int64_t              n,
    int64_t              k,
    float                alpha,
    const hipblasLtHalf* A,
    int64_t              lda,
    const hipblaslt_f8*  B,
    int64_t              ldb,
    float                beta,
    hipblasLtHalf*       C,
    int64_t              ldc,
    const float*         AlphaVec,
    float                scaleD,
    bool                 alt)
{
    // cblas does not support hipblasLtHalf, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
    {
        A_float[i] = static_cast<float>(A[i]);
        A_float[i] *= AlphaVec[i % m];
    }
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);
    for(size_t i = 0; i < sizeC; i++)
        C_float[i] = static_cast<float>(C[i]);

    // just directly cast, since transA, transB are integers in the enum
    //printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C_float,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblasLtHalf>(C_float[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = hipblasLtHalf(C_float[i]);
    }
}

template <>
void cblas_gemm_alphascale<hipblaslt_f8, hipblasLtHalf, hipblasLtHalf, float>(
    hipblasOperation_t   transA,
    hipblasOperation_t   transB,
    int64_t              m,
    int64_t              n,
    int64_t              k,
    float                alpha,
    const hipblaslt_f8*  A,
    int64_t              lda,
    const hipblasLtHalf* B,
    int64_t              ldb,
    float                beta,
    hipblasLtHalf*       C,
    int64_t              ldc,
    const float*         AlphaVec,
    float                scaleD,
    bool                 alt)
{
    // cblas does not support hipblasLtHalf, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    for(size_t i = 0; i < sizeA; i++)
    {
        A_float[i] = static_cast<float>(A[i]);
        A_float[i] *= AlphaVec[i % m];
    }
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);
    for(size_t i = 0; i < sizeC; i++)
        C_float[i] = static_cast<float>(C[i]);

    // just directly cast, since transA, transB are integers in the enum
    //printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C_float,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblasLtHalf>(C_float[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = hipblasLtHalf(C_float[i]);
    }
}

template <>
void cblas_gemm_alphascale<hipblasLtHalf, hipblaslt_f8, float, float>(hipblasOperation_t   transA,
                                                                      hipblasOperation_t   transB,
                                                                      int64_t              m,
                                                                      int64_t              n,
                                                                      int64_t              k,
                                                                      float                alpha,
                                                                      const hipblasLtHalf* A,
                                                                      int64_t              lda,
                                                                      const hipblaslt_f8*  B,
                                                                      int64_t              ldb,
                                                                      float                beta,
                                                                      float*               C,
                                                                      int64_t              ldc,
                                                                      const float*         AlphaVec,
                                                                      float                scaleD,
                                                                      bool                 alt)
{
    // cblas does not support hipblasLtHalf, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB);

    for(size_t i = 0; i < sizeA; i++)
    {
        A_float[i] = static_cast<float>(A[i]);
        A_float[i] *= AlphaVec[i % m];
    }
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);

    // just directly cast, since transA, transB are integers in the enum
    //printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<float>(C[i] * scaleD);
    }
}

template <>
void cblas_gemm_alphascale<hipblaslt_f8, hipblasLtHalf, float, float>(hipblasOperation_t   transA,
                                                                      hipblasOperation_t   transB,
                                                                      int64_t              m,
                                                                      int64_t              n,
                                                                      int64_t              k,
                                                                      float                alpha,
                                                                      const hipblaslt_f8*  A,
                                                                      int64_t              lda,
                                                                      const hipblasLtHalf* B,
                                                                      int64_t              ldb,
                                                                      float                beta,
                                                                      float*               C,
                                                                      int64_t              ldc,
                                                                      const float*         AlphaVec,
                                                                      float                scaleD,
                                                                      bool                 alt)
{
    // cblas does not support hipblasLtHalf, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB);

    for(size_t i = 0; i < sizeA; i++)
    {
        A_float[i] = static_cast<float>(A[i]);
        A_float[i] *= AlphaVec[i % m];
    }
    for(size_t i = 0; i < sizeB; i++)
        B_float[i] = static_cast<float>(B[i]);

    // just directly cast, since transA, transB are integers in the enum
    //printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<float>(C[i] * scaleD);
    }
}

template <>
void cblas_gemm_alphascale<hipblasLtHalf, hipblasLtHalf, hipblasLtHalf, float>(
    hipblasOperation_t   transA,
    hipblasOperation_t   transB,
    int64_t              m,
    int64_t              n,
    int64_t              k,
    float                alpha,
    const hipblasLtHalf* A,
    int64_t              lda,
    const hipblasLtHalf* B,
    int64_t              ldb,
    float                beta,
    hipblasLtHalf*       C,
    int64_t              ldc,
    const float*         AlphaVec,
    float                scaleD,
    bool                 alt)
{
    // cblas does not support hipblasLtHalf, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB), C_float(sizeC);

    if(alt)
    {
        for(size_t i = 0; i < sizeA; i++)
            A_float[i] = float_to_bfloat16_truncate(float(A[i]));
        for(size_t i = 0; i < sizeB; i++)
            B_float[i] = float_to_bfloat16_truncate(float(B[i]));
        for(size_t i = 0; i < sizeC; i++)
            C_float[i] = float_to_bfloat16_truncate(float(C[i]));
    }
    else
    {
        for(size_t i = 0; i < sizeA; i++)
        {
            A_float[i] = A[i];
            A_float[i] *= AlphaVec[i % m];
        }
        for(size_t i = 0; i < sizeB; i++)
            B_float[i] = B[i];
        for(size_t i = 0; i < sizeC; i++)
            C_float[i] = C[i];
    }

    // just directly cast, since transA, transB are integers in the enum
    //printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C_float,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<hipblasLtHalf>(C_float[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = hipblasLtHalf(C_float[i]);
    }
}

template <>
void cblas_gemm_alphascale<hipblasLtHalf, hipblasLtHalf, float, float>(hipblasOperation_t   transA,
                                                                       hipblasOperation_t   transB,
                                                                       int64_t              m,
                                                                       int64_t              n,
                                                                       int64_t              k,
                                                                       float                alpha,
                                                                       const hipblasLtHalf* A,
                                                                       int64_t              lda,
                                                                       const hipblasLtHalf* B,
                                                                       int64_t              ldb,
                                                                       float                beta,
                                                                       float*               C,
                                                                       int64_t              ldc,
                                                                       const float* AlphaVec,
                                                                       float        scaleD,
                                                                       bool         alt)
{
    // cblas does not support hipblasLtHalf, so convert to higher precision float
    // This will give more precise result which is acceptable for testing

    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA), B_float(sizeB);

    if(alt)
    {
        for(size_t i = 0; i < sizeA; i++)
            A_float[i] = float_to_bfloat16_truncate(float(A[i]));
        for(size_t i = 0; i < sizeB; i++)
            B_float[i] = float_to_bfloat16_truncate(float(B[i]));
    }
    else
    {
        for(size_t i = 0; i < sizeA; i++)
        {
            A_float[i] = A[i];
            A_float[i] *= AlphaVec[i % m];
        }
        for(size_t i = 0; i < sizeB; i++)
            B_float[i] = B[i];
    }

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_float,
                lda,
                B_float,
                ldb,
                beta,
                C,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<float>(C[i] * scaleD);
    }
}

template <>
void cblas_gemm_alphascale<float, float, float, float>(hipblasOperation_t transA,
                                                       hipblasOperation_t transB,
                                                       int64_t            m,
                                                       int64_t            n,
                                                       int64_t            k,
                                                       float              alpha,
                                                       const float*       A,
                                                       int64_t            lda,
                                                       const float*       B,
                                                       int64_t            ldb,
                                                       float              beta,
                                                       float*             C,
                                                       int64_t            ldc,
                                                       const float*       AlphaVec,
                                                       float              scaleD,
                                                       bool               alt)
{
    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<float> A_float(sizeA);

    for(size_t i = 0; i < sizeA; i++)
    {
        A_float[i] = A[i];
        A_float[i] *= AlphaVec[i % m];
    }

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_sgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A,
                lda,
                B,
                ldb,
                beta,
                C,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<float>(C[i] * scaleD);
    }
}

template <>
void cblas_gemm_alphascale<double, double, double, double>(hipblasOperation_t transA,
                                                           hipblasOperation_t transB,
                                                           int64_t            m,
                                                           int64_t            n,
                                                           int64_t            k,
                                                           double             alpha,
                                                           const double*      A,
                                                           int64_t            lda,
                                                           const double*      B,
                                                           int64_t            ldb,
                                                           double             beta,
                                                           double*            C,
                                                           int64_t            ldc,
                                                           const double*      AlphaVec,
                                                           double             scaleD,
                                                           bool               alt)
{
    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<double> A_double(sizeA);

    for(size_t i = 0; i < sizeA; i++)
    {
        A_double[i] = A[i];
        A_double[i] *= AlphaVec[i % m];
    }

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_dgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A,
                lda,
                B,
                ldb,
                beta,
                C,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<double>(C[i] * scaleD);
    }
}

template <>
void cblas_gemm_alphascale<int8_t, int8_t, int32_t, int32_t>(hipblasOperation_t transA,
                                                             hipblasOperation_t transB,
                                                             int64_t            m,
                                                             int64_t            n,
                                                             int64_t            k,
                                                             int32_t            alpha,
                                                             const int8_t*      A,
                                                             int64_t            lda,
                                                             const int8_t*      B,
                                                             int64_t            ldb,
                                                             int32_t            beta,
                                                             int32_t*           C,
                                                             int64_t            ldc,
                                                             const int32_t* AlphaVec, //cm review
                                                             int32_t        scaleD,
                                                             bool           alt)
{
    size_t sizeA = (transA == HIPBLAS_OP_N ? k : m) * size_t(lda);
    size_t sizeB = (transB == HIPBLAS_OP_N ? n : k) * size_t(ldb);
    size_t sizeC = n * size_t(ldc);

    host_vector<double> A_double(sizeA);
    host_vector<double> B_double(sizeB);
    host_vector<double> C_double(sizeC);

    for(size_t i = 0; i < sizeA; i++)
    {
        A_double[i] = static_cast<double>(A[i]);
        A_double[i] *= static_cast<double>(AlphaVec[i % m]);
    }
    for(size_t i = 0; i < sizeB; i++)
        B_double[i] = static_cast<double>(B[i]);
    for(size_t i = 0; i < sizeC; i++)
        C_double[i] = static_cast<double>(C[i]);

    // just directly cast, since transA, transB are integers in the enum
    // printf("transA: hipblaslt =%d, cblas=%d\n", transA, HIPOperationToCBLASTanspose(transA) );
    cblas_dgemm(CblasColMajor,
                HIPOperationToCBLASTanspose(transA),
                HIPOperationToCBLASTanspose(transB),
                m,
                n,
                k,
                alpha,
                A_double,
                lda,
                B_double,
                ldb,
                beta,
                C_double,
                ldc);

    if(scaleD != 1)
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<int32_t>(C_double[i] * scaleD);
    }
    else
    {
        for(size_t i = 0; i < sizeC; i++)
            C[i] = static_cast<int32_t>(C_double[i]);
    }
}
