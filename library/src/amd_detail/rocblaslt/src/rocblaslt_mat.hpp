/* ************************************************************************
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
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * ************************************************************************ */

#pragma once
#ifndef ROCBLASLT_MAT_HPP
#define ROCBLASLT_MAT_HPP

#include "handle.h"

#include "tensile_host.hpp"

template <typename TiA, typename TiB, typename To, typename Tc>
rocblaslt_status rocblaslt_batched_template(rocblaslt_handle             handle,
                                            hipblasOperation_t           trans_a,
                                            hipblasOperation_t           trans_b,
                                            int64_t                      m,
                                            int64_t                      n,
                                            int64_t                      k,
                                            const Tc*                    alpha,
                                            const TiA*                   a,
                                            int64_t                      ld_a,
                                            int64_t                      batch_stride_a,
                                            const TiB*                   b,
                                            int64_t                      ld_b,
                                            int64_t                      batch_stride_b,
                                            const Tc*                    beta,
                                            const To*                    c,
                                            int64_t                      ld_c,
                                            int64_t                      batch_stride_c,
                                            To*                          d,
                                            int64_t                      ld_d,
                                            int64_t                      batch_stride_d,
                                            Tc*                          e,
                                            int64_t                      ld_e,
                                            int64_t                      batch_stride_e,
                                            int64_t                      batch_count,
                                            bool                         strided_batch,
                                            bool                         grouped_gemm,
                                            bool                         gradient,
                                            rocblaslt_compute_type       compute_type,
                                            const rocblaslt_matmul_algo* algo,
                                            void*                        workspace,
                                            size_t                       workspaceSizeInBytes,
                                            const void*                  bias,
                                            const Tc*                    scaleA,
                                            const Tc*                    scaleB,
                                            const Tc*                    scaleC,
                                            const Tc*                    scaleD,
                                            const Tc*                    scaleE,
                                            const Tc*                    scaleAlphaVec,
                                            hipblasltDatatype_t          bias_type,
                                            rocblaslt_epilogue           epilogue,
                                            std::shared_ptr<void>        gemmData,
                                            hipStream_t                  stream)
{
    workspaceSizeInBytes = min(workspaceSizeInBytes, algo->max_workspace_bytes);
    RocblasltContractionProblem<TiA, TiB, To, Tc> problem{trans_a,
                                                          trans_b,
                                                          m,
                                                          n,
                                                          k,
                                                          alpha,
                                                          a,
                                                          nullptr,
                                                          ld_a,
                                                          batch_stride_a,
                                                          b,
                                                          nullptr,
                                                          ld_b,
                                                          batch_stride_b,
                                                          beta,
                                                          c,
                                                          nullptr,
                                                          ld_c,
                                                          batch_stride_c,
                                                          d,
                                                          nullptr,
                                                          ld_d,
                                                          batch_stride_d,
                                                          e,
                                                          nullptr,
                                                          ld_e,
                                                          batch_stride_e,
                                                          batch_count,
                                                          strided_batch,
                                                          grouped_gemm,
                                                          gradient,
                                                          compute_type,
                                                          bias,
                                                          scaleA,
                                                          scaleB,
                                                          scaleC,
                                                          scaleD,
                                                          scaleE,
                                                          scaleAlphaVec,
                                                          bias_type,
                                                          epilogue,
                                                          workspace,
                                                          workspaceSizeInBytes,
                                                          stream};
    return runContractionProblem(handle, algo, problem, gemmData);
}

template <typename TiA, typename TiB, typename To, typename Tc>
rocblaslt_status rocblaslt_gemm_create_batched_template(hipblasOperation_t     trans_a,
                                                        hipblasOperation_t     trans_b,
                                                        int64_t                m,
                                                        int64_t                n,
                                                        int64_t                k,
                                                        const Tc*              alpha,
                                                        const TiA*             a,
                                                        int64_t                ld_a,
                                                        int64_t                batch_stride_a,
                                                        const TiB*             b,
                                                        int64_t                ld_b,
                                                        int64_t                batch_stride_b,
                                                        const Tc*              beta,
                                                        const To*              c,
                                                        int64_t                ld_c,
                                                        int64_t                batch_stride_c,
                                                        To*                    d,
                                                        int64_t                ld_d,
                                                        int64_t                batch_stride_d,
                                                        Tc*                    e,
                                                        int64_t                ld_e,
                                                        int64_t                batch_stride_e,
                                                        int64_t                batch_count,
                                                        bool                   strided_batch,
                                                        bool                   grouped_gemm,
                                                        bool                   gradient,
                                                        rocblaslt_compute_type compute_type,
                                                        const void*            bias,
                                                        const Tc*              scaleA,
                                                        const Tc*              scaleB,
                                                        const Tc*              scaleC,
                                                        const Tc*              scaleD,
                                                        const Tc*              scaleE,
                                                        const Tc*              scaleAlphaVec,
                                                        hipblasltDatatype_t    bias_type,
                                                        rocblaslt_epilogue     epilogue,
                                                        std::shared_ptr<void>& gemmData,
                                                        size_t&                gemmCount)
{
    Tc alpha_1 = 1.0; // use dScaleAlphaVec instead, original alpha => 1.0
    if(scaleAlphaVec)
        alpha = &alpha_1;
    RocblasltContractionProblem<TiA, TiB, To, Tc> problem{trans_a,
                                                          trans_b,
                                                          m,
                                                          n,
                                                          k,
                                                          alpha,
                                                          a,
                                                          nullptr,
                                                          ld_a,
                                                          batch_stride_a,
                                                          b,
                                                          nullptr,
                                                          ld_b,
                                                          batch_stride_b,
                                                          beta,
                                                          c,
                                                          nullptr,
                                                          ld_c,
                                                          batch_stride_c,
                                                          d,
                                                          nullptr,
                                                          ld_d,
                                                          batch_stride_d,
                                                          e,
                                                          nullptr,
                                                          ld_e,
                                                          batch_stride_e,
                                                          batch_count,
                                                          strided_batch,
                                                          grouped_gemm,
                                                          gradient,
                                                          compute_type,
                                                          bias,
                                                          scaleA,
                                                          scaleB,
                                                          scaleC,
                                                          scaleD,
                                                          scaleE,
                                                          scaleAlphaVec,
                                                          bias_type,
                                                          epilogue,
                                                          nullptr,
                                                          0,
                                                          0};
    return gemmCreate(problem, gemmData, gemmCount);
}

template <typename TiA, typename TiB, typename To, typename Tc>
rocblaslt_status
    rocblaslt_groupedgemm_create_batched_template(hipblasOperation_t                trans_a,
                                                  hipblasOperation_t                trans_b,
                                                  std::vector<int64_t>&             m,
                                                  std::vector<int64_t>&             n,
                                                  std::vector<int64_t>&             k,
                                                  std::vector<const Tc*>&           alpha,
                                                  std::vector<const TiA*>&          a,
                                                  std::vector<int64_t>&             ld_a,
                                                  std::vector<int64_t>&             batch_stride_a,
                                                  std::vector<const TiB*>&          b,
                                                  std::vector<int64_t>&             ld_b,
                                                  std::vector<int64_t>&             batch_stride_b,
                                                  std::vector<const Tc*>&           beta,
                                                  std::vector<const To*>&           c,
                                                  std::vector<int64_t>&             ld_c,
                                                  std::vector<int64_t>&             batch_stride_c,
                                                  std::vector<To*>&                 d,
                                                  std::vector<int64_t>&             ld_d,
                                                  std::vector<int64_t>&             batch_stride_d,
                                                  std::vector<void*>&               e,
                                                  std::vector<int64_t>&             ld_e,
                                                  std::vector<int64_t>&             batch_stride_e,
                                                  std::vector<int64_t>&             batch_count,
                                                  bool                              strided_batch,
                                                  bool                              grouped_gemm,
                                                  rocblaslt_compute_type            compute_type,
                                                  std::vector<bool>&                gradient,
                                                  std::vector<const void*>&         bias,
                                                  std::vector<const Tc*>&           scaleAVec,
                                                  std::vector<const Tc*>&           scaleBVec,
                                                  std::vector<const Tc*>&           scaleCVec,
                                                  std::vector<const Tc*>&           scaleDVec,
                                                  std::vector<const Tc*>&           scaleEVec,
                                                  std::vector<const Tc*>&           scaleAlphaVec,
                                                  std::vector<hipblasltDatatype_t>& bias_type,
                                                  std::vector<rocblaslt_epilogue>&  epilogue,
                                                  std::shared_ptr<void>&            gemmData,
                                                  size_t&                           gemmCount)
{
    std::vector<RocblasltContractionProblem<TiA, TiB, To, Tc>> problems;

    for(int i = 0; i < m.size(); i++)
    {
        problems.push_back(RocblasltContractionProblem<TiA, TiB, To, Tc>{trans_a,
                                                                         trans_b,
                                                                         m[i],
                                                                         n[i],
                                                                         k[i],
                                                                         alpha[i],
                                                                         a[i],
                                                                         nullptr,
                                                                         ld_a[i],
                                                                         batch_stride_a[i],
                                                                         b[i],
                                                                         nullptr,
                                                                         ld_b[i],
                                                                         batch_stride_b[i],
                                                                         beta[i],
                                                                         c[i],
                                                                         nullptr,
                                                                         ld_c[i],
                                                                         batch_stride_c[i],
                                                                         d[i],
                                                                         nullptr,
                                                                         ld_d[i],
                                                                         batch_stride_d[i],
                                                                         (Tc*)e[i],
                                                                         nullptr,
                                                                         ld_e[i],
                                                                         batch_stride_e[i],
                                                                         batch_count[i],
                                                                         strided_batch,
                                                                         grouped_gemm,
                                                                         gradient[i],
                                                                         compute_type,
                                                                         bias[i],
                                                                         scaleAVec[i],
                                                                         scaleBVec[i],
                                                                         scaleCVec[i],
                                                                         scaleDVec[i],
                                                                         scaleEVec[i],
                                                                         scaleAlphaVec[i],
                                                                         bias_type[i],
                                                                         epilogue[i],
                                                                         nullptr,
                                                                         0,
                                                                         0});
    }
    return groupedGemmCreate(problems, gemmData, gemmCount);
}

template <typename TiA, typename TiB = TiA, typename To = TiB, typename Tc = To>
rocblaslt_status rocblaslt_matmul_typecasting(rocblaslt_handle             handle,
                                              hipblasOperation_t           trans_a,
                                              hipblasOperation_t           trans_b,
                                              int64_t                      m,
                                              int64_t                      n,
                                              int64_t                      k,
                                              const void*                  alpha,
                                              const void*                  a,
                                              int64_t                      ld_a,
                                              int64_t                      batch_stride_a,
                                              const void*                  b,
                                              int64_t                      ld_b,
                                              int64_t                      batch_stride_b,
                                              const void*                  beta,
                                              const void*                  c,
                                              int64_t                      ld_c,
                                              int64_t                      batch_stride_c,
                                              void*                        d,
                                              int64_t                      ld_d,
                                              int64_t                      batch_stride_d,
                                              void*                        e,
                                              int64_t                      ld_e,
                                              int64_t                      batch_stride_e,
                                              int64_t                      batch_count,
                                              bool                         strided_batch,
                                              bool                         grouped_gemm,
                                              bool                         gradient,
                                              rocblaslt_compute_type       compute_type,
                                              const rocblaslt_matmul_algo* algo,
                                              void*                        workspace,
                                              size_t                       workspaceSizeInBytes,
                                              const void*                  bias,
                                              const void*                  scaleA,
                                              const void*                  scaleB,
                                              const void*                  scaleC,
                                              const void*                  scaleD,
                                              const void*                  scaleE,
                                              const void*                  scaleAlphaVec,
                                              hipblasltDatatype_t          bias_type,
                                              rocblaslt_epilogue           epilogue,
                                              std::shared_ptr<void>        gemmData,
                                              hipStream_t                  stream)
{
    // check alignment of pointers before casting
    if(!isAligned(a, sizeof(TiA)) || !isAligned(b, sizeof(TiB)) || !isAligned(c, sizeof(To))
       || !isAligned(d, sizeof(To)))
    {
        std::cerr << "memmory is not aligned" << std::endl;
        return rocblaslt_status_invalid_size;
    }
    return rocblaslt_batched_template(handle,
                                      trans_a,
                                      trans_b,
                                      m,
                                      n,
                                      k,
                                      reinterpret_cast<const Tc*>(alpha),
                                      reinterpret_cast<const TiA*>(a),
                                      ld_a,
                                      batch_stride_a,
                                      reinterpret_cast<const TiB*>(b),
                                      ld_b,
                                      batch_stride_b,
                                      reinterpret_cast<const Tc*>(beta),
                                      reinterpret_cast<const To*>(c),
                                      ld_c,
                                      batch_stride_c,
                                      (To*)d,
                                      ld_d,
                                      batch_stride_d,
                                      (Tc*)e,
                                      ld_e,
                                      batch_stride_e,
                                      batch_count,
                                      strided_batch,
                                      grouped_gemm,
                                      gradient,
                                      compute_type,
                                      algo,
                                      workspace,
                                      workspaceSizeInBytes,
                                      reinterpret_cast<const void*>(bias),
                                      reinterpret_cast<const Tc*>(scaleA),
                                      reinterpret_cast<const Tc*>(scaleB),
                                      reinterpret_cast<const Tc*>(scaleC),
                                      reinterpret_cast<const Tc*>(scaleD),
                                      reinterpret_cast<const Tc*>(scaleE),
                                      reinterpret_cast<const Tc*>(scaleAlphaVec),
                                      bias_type,
                                      epilogue,
                                      gemmData,
                                      stream);
}

template <typename TiA, typename TiB = TiA, typename To = TiB, typename Tc = To>
rocblaslt_status rocblaslt_gemm_create_typecasting(hipblasOperation_t     trans_a,
                                                   hipblasOperation_t     trans_b,
                                                   int64_t                m,
                                                   int64_t                n,
                                                   int64_t                k,
                                                   const void*            alpha,
                                                   const void*            a,
                                                   int64_t                ld_a,
                                                   int64_t                batch_stride_a,
                                                   const void*            b,
                                                   int64_t                ld_b,
                                                   int64_t                batch_stride_b,
                                                   const void*            beta,
                                                   const void*            c,
                                                   int64_t                ld_c,
                                                   int64_t                batch_stride_c,
                                                   void*                  d,
                                                   int64_t                ld_d,
                                                   int64_t                batch_stride_d,
                                                   void*                  e,
                                                   int64_t                ld_e,
                                                   int64_t                batch_stride_e,
                                                   int64_t                batch_count,
                                                   bool                   strided_batch,
                                                   bool                   grouped_gemm,
                                                   bool                   gradient,
                                                   rocblaslt_compute_type compute_type,
                                                   const void*            bias,
                                                   const void*            scaleA,
                                                   const void*            scaleB,
                                                   const void*            scaleC,
                                                   const void*            scaleD,
                                                   const void*            scaleE,
                                                   const void*            scaleAlphaVec,
                                                   hipblasltDatatype_t    bias_type,
                                                   rocblaslt_epilogue     epilogue,
                                                   std::shared_ptr<void>& gemmData,
                                                   size_t&                gemmCount)
{
    // check alignment of pointers before casting
    if(!isAligned(a, sizeof(TiA)) || !isAligned(b, sizeof(TiB)) || !isAligned(c, sizeof(To))
       || !isAligned(d, sizeof(To)))
    {
        std::cerr << "memmory is not aligned" << std::endl;
        return rocblaslt_status_invalid_size;
    }
    return rocblaslt_gemm_create_batched_template(trans_a,
                                                  trans_b,
                                                  m,
                                                  n,
                                                  k,
                                                  reinterpret_cast<const Tc*>(alpha),
                                                  reinterpret_cast<const TiA*>(a),
                                                  ld_a,
                                                  batch_stride_a,
                                                  reinterpret_cast<const TiB*>(b),
                                                  ld_b,
                                                  batch_stride_b,
                                                  reinterpret_cast<const Tc*>(beta),
                                                  reinterpret_cast<const To*>(c),
                                                  ld_c,
                                                  batch_stride_c,
                                                  (To*)d,
                                                  ld_d,
                                                  batch_stride_d,
                                                  (Tc*)e,
                                                  ld_e,
                                                  batch_stride_e,
                                                  batch_count,
                                                  strided_batch,
                                                  grouped_gemm,
                                                  gradient,
                                                  compute_type,
                                                  reinterpret_cast<const void*>(bias),
                                                  reinterpret_cast<const Tc*>(scaleA),
                                                  reinterpret_cast<const Tc*>(scaleB),
                                                  reinterpret_cast<const Tc*>(scaleC),
                                                  reinterpret_cast<const Tc*>(scaleD),
                                                  reinterpret_cast<const Tc*>(scaleE),
                                                  reinterpret_cast<const Tc*>(scaleAlphaVec),
                                                  bias_type,
                                                  epilogue,
                                                  gemmData,
                                                  gemmCount);
}

template <typename TiA, typename TiB = TiA, typename To = TiB, typename Tc = To>
rocblaslt_status
    rocblaslt_groupedgemm_create_typecasting(hipblasOperation_t                trans_a,
                                             hipblasOperation_t                trans_b,
                                             std::vector<int64_t>&             m,
                                             std::vector<int64_t>&             n,
                                             std::vector<int64_t>&             k,
                                             std::vector<const void*>&         alpha,
                                             std::vector<const void*>&         a,
                                             std::vector<int64_t>&             ld_a,
                                             std::vector<int64_t>&             batch_stride_a,
                                             std::vector<const void*>&         b,
                                             std::vector<int64_t>&             ld_b,
                                             std::vector<int64_t>&             batch_stride_b,
                                             std::vector<const void*>&         beta,
                                             std::vector<const void*>&         c,
                                             std::vector<int64_t>&             ld_c,
                                             std::vector<int64_t>&             batch_stride_c,
                                             std::vector<void*>&               d,
                                             std::vector<int64_t>&             ld_d,
                                             std::vector<int64_t>&             batch_stride_d,
                                             std::vector<void*>&               e,
                                             std::vector<int64_t>&             ld_e,
                                             std::vector<int64_t>&             batch_stride_e,
                                             std::vector<int64_t>&             batch_count,
                                             bool                              strided_batch,
                                             bool                              grouped_gemm,
                                             rocblaslt_compute_type            compute_type,
                                             std::vector<bool>&                gradient,
                                             std::vector<const void*>&         bias,
                                             std::vector<const void*>&         scaleA,
                                             std::vector<const void*>&         scaleB,
                                             std::vector<const void*>&         scaleC,
                                             std::vector<const void*>&         scaleD,
                                             std::vector<const void*>&         scaleE,
                                             std::vector<const void*>&         scaleAlphaVec,
                                             std::vector<hipblasltDatatype_t>& bias_type,
                                             std::vector<rocblaslt_epilogue>&  epilogue,
                                             std::shared_ptr<void>&            gemmData,
                                             size_t&                           gemmCount)
{
    std::vector<const Tc*>  groupedAlpha, groupedBeta;
    std::vector<const TiA*> groupedA;
    std::vector<const TiB*> groupedB;
    std::vector<const To*>  groupedC;
    std::vector<To*>        groupedD;
    std::vector<const Tc*>  groupedScaleA;
    std::vector<const Tc*>  groupedScaleB;
    std::vector<const Tc*>  groupedScaleC;
    std::vector<const Tc*>  groupedScaleD;
    std::vector<const Tc*>  groupedScaleE;
    std::vector<const Tc*>  groupedScaleAlphaVec;

    for(int i = 0; i < alpha.size(); i++)
    {
        groupedAlpha.push_back(reinterpret_cast<const Tc*>(alpha[i]));
        groupedBeta.push_back(reinterpret_cast<const Tc*>(beta[i]));
        groupedA.push_back(reinterpret_cast<const TiA*>(a[i]));
        groupedB.push_back(reinterpret_cast<const TiB*>(b[i]));
        groupedC.push_back(reinterpret_cast<const To*>(c[i]));
        groupedD.push_back(reinterpret_cast<To*>(d[i]));
        groupedScaleA.push_back(reinterpret_cast<const Tc*>(scaleA[i]));
        groupedScaleB.push_back(reinterpret_cast<const Tc*>(scaleB[i]));
        groupedScaleC.push_back(reinterpret_cast<const Tc*>(scaleC[i]));
        groupedScaleD.push_back(reinterpret_cast<const Tc*>(scaleD[i]));
        groupedScaleE.push_back(reinterpret_cast<const Tc*>(scaleE[i]));
        groupedScaleAlphaVec.push_back(reinterpret_cast<const Tc*>(scaleAlphaVec[i]));
    }

    return rocblaslt_groupedgemm_create_batched_template(trans_a,
                                                         trans_b,
                                                         m,
                                                         n,
                                                         k,
                                                         groupedAlpha,
                                                         groupedA,
                                                         ld_a,
                                                         batch_stride_a,
                                                         groupedB,
                                                         ld_b,
                                                         batch_stride_b,
                                                         groupedBeta,
                                                         groupedC,
                                                         ld_c,
                                                         batch_stride_c,
                                                         groupedD,
                                                         ld_d,
                                                         batch_stride_d,
                                                         e,
                                                         ld_e,
                                                         batch_stride_e,
                                                         batch_count,
                                                         strided_batch,
                                                         grouped_gemm,
                                                         compute_type,
                                                         gradient,
                                                         bias,
                                                         groupedScaleA,
                                                         groupedScaleB,
                                                         groupedScaleC,
                                                         groupedScaleD,
                                                         groupedScaleE,
                                                         groupedScaleAlphaVec,
                                                         bias_type,
                                                         epilogue,
                                                         gemmData,
                                                         gemmCount);
}

inline rocblaslt_status rocblaslt_matmul_template(rocblaslt_handle             handle,
                                                  hipblasOperation_t           trans_a,
                                                  hipblasOperation_t           trans_b,
                                                  int64_t                      m,
                                                  int64_t                      n,
                                                  int64_t                      k,
                                                  const void*                  alpha,
                                                  const void*                  a,
                                                  hipblasltDatatype_t          a_type,
                                                  int64_t                      ld_a,
                                                  int64_t                      batch_stride_a,
                                                  const void*                  b,
                                                  hipblasltDatatype_t          b_type,
                                                  int64_t                      ld_b,
                                                  int64_t                      batch_stride_b,
                                                  const void*                  beta,
                                                  const void*                  c,
                                                  hipblasltDatatype_t          c_type,
                                                  int64_t                      ld_c,
                                                  int64_t                      batch_stride_c,
                                                  void*                        d,
                                                  hipblasltDatatype_t          d_type,
                                                  int64_t                      ld_d,
                                                  int64_t                      batch_stride_d,
                                                  void*                        e,
                                                  int64_t                      ld_e,
                                                  int64_t                      batch_stride_e,
                                                  int64_t                      batch_count,
                                                  bool                         strided_batch,
                                                  bool                         grouped_gemm,
                                                  bool                         gradient,
                                                  rocblaslt_compute_type       compute_type,
                                                  const rocblaslt_matmul_algo* algo,
                                                  void*                        workspace,
                                                  size_t                       workspaceSizeInBytes,
                                                  const void*                  bias,
                                                  const void*                  scaleA,
                                                  const void*                  scaleB,
                                                  const void*                  scaleC,
                                                  const void*                  scaleD,
                                                  const void*                  scaleE,
                                                  const void*                  scaleAlphaVec,
                                                  hipblasltDatatype_t          bias_type,
                                                  rocblaslt_epilogue           epilogue,
                                                  std::shared_ptr<void>        gemmData,
                                                  hipStream_t                  stream)
{
    rocblaslt_status rs_status = rocblaslt_status_not_implemented;

#define EX_TYPECASTING_PARM                                                                       \
    handle, trans_a, trans_b, m, n, k, alpha, a, ld_a, batch_stride_a, b, ld_b, batch_stride_b,   \
        beta, c, ld_c, batch_stride_c, d, ld_d, batch_stride_d, e, ld_e, batch_stride_e,          \
        batch_count, strided_batch, grouped_gemm, gradient, compute_type, algo, workspace,        \
        workspaceSizeInBytes, bias, scaleA, scaleB, scaleC, scaleD, scaleE, scaleAlphaVec,        \
        bias_type, epilogue, gemmData, stream

    if(a_type == HIPBLASLT_R_32F && b_type == HIPBLASLT_R_32F)
    {
        if(c_type == HIPBLASLT_R_32F && d_type == HIPBLASLT_R_32F)
        {
            if(compute_type == rocblaslt_compute_f32
               || compute_type == rocblaslt_compute_f32_fast_xf32)
            {
                rs_status
                    = rocblaslt_matmul_typecasting<float, float, float, float>(EX_TYPECASTING_PARM);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_16F && b_type == HIPBLASLT_R_16F)
    {
        if(c_type == HIPBLASLT_R_16F && d_type == HIPBLASLT_R_16F)
        {
            if(compute_type == rocblaslt_compute_f32)
            {
                rs_status = rocblaslt_matmul_typecasting<rocblaslt_half,
                                                         rocblaslt_half,
                                                         rocblaslt_half,
                                                         float>(EX_TYPECASTING_PARM);
            }
        }
        else if(c_type == HIPBLASLT_R_32F && d_type == HIPBLASLT_R_32F)
        {
            if(compute_type == rocblaslt_compute_f32)
            {
                rs_status
                    = rocblaslt_matmul_typecasting<rocblaslt_half, rocblaslt_half, float, float>(
                        EX_TYPECASTING_PARM);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_16B && b_type == HIPBLASLT_R_16B)
    {
        if(c_type == HIPBLASLT_R_16B && d_type == HIPBLASLT_R_16B)
        {
            if(compute_type == rocblaslt_compute_f32)
            {
                rs_status = rocblaslt_matmul_typecasting<rocblaslt_bfloat16,
                                                         rocblaslt_bfloat16,
                                                         rocblaslt_bfloat16,
                                                         float>(EX_TYPECASTING_PARM);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_8F_E4M3 && b_type == HIPBLASLT_R_8F_E4M3)
    {
        if(c_type == HIPBLASLT_R_32F && d_type == HIPBLASLT_R_32F)
        {
            if(compute_type == rocblaslt_compute_f32)
            {
                rs_status = rocblaslt_matmul_typecasting<rocblaslt_f8, rocblaslt_f8, float, float>(
                    EX_TYPECASTING_PARM);
            }
        }
        else if(c_type == HIPBLASLT_R_16F && d_type == HIPBLASLT_R_16F)
        {
            if(compute_type == rocblaslt_compute_f32)
            {
                rs_status = rocblaslt_matmul_typecasting<rocblaslt_f8,
                                                         rocblaslt_f8,
                                                         rocblaslt_half,
                                                         float>(EX_TYPECASTING_PARM);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_8F_E4M3 && b_type == HIPBLASLT_R_8F_E5M2)
    {
        if(c_type == HIPBLASLT_R_32F && d_type == HIPBLASLT_R_32F)
        {
            if(compute_type == rocblaslt_compute_f32)
            {
                rs_status = rocblaslt_matmul_typecasting<rocblaslt_f8, rocblaslt_bf8, float, float>(
                    EX_TYPECASTING_PARM);
            }
        }
        else if(c_type == HIPBLASLT_R_16F && d_type == HIPBLASLT_R_16F)
        {
            if(compute_type == rocblaslt_compute_f32)
            {
                rs_status = rocblaslt_matmul_typecasting<rocblaslt_f8,
                                                         rocblaslt_bf8,
                                                         rocblaslt_half,
                                                         float>(EX_TYPECASTING_PARM);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_64F && b_type == HIPBLASLT_R_64F)
    {
        if(c_type == HIPBLASLT_R_64F && d_type == HIPBLASLT_R_64F)
        {
            if(compute_type == rocblaslt_compute_f64)
            {
                rs_status = rocblaslt_matmul_typecasting<double, double, double, double>(
                    EX_TYPECASTING_PARM);
            }
        }
        else if(c_type == HIPBLASLT_R_16F && d_type == HIPBLASLT_R_16F)
        {
            if(compute_type == rocblaslt_compute_f32)
            {
                rs_status = rocblaslt_matmul_typecasting<rocblaslt_f8,
                                                         rocblaslt_bf8,
                                                         rocblaslt_half,
                                                         float>(EX_TYPECASTING_PARM);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_8F_E5M2 && b_type == HIPBLASLT_R_8F_E4M3)
    {
        if(c_type == HIPBLASLT_R_32F && d_type == HIPBLASLT_R_32F)
        {
            if(compute_type == rocblaslt_compute_f32)
            {
                rs_status = rocblaslt_matmul_typecasting<rocblaslt_bf8, rocblaslt_f8, float, float>(
                    EX_TYPECASTING_PARM);
            }
        }
        else if(c_type == HIPBLASLT_R_16F && d_type == HIPBLASLT_R_16F)
        {
            if(compute_type == rocblaslt_compute_f32)
            {
                rs_status = rocblaslt_matmul_typecasting<rocblaslt_bf8,
                                                         rocblaslt_f8,
                                                         rocblaslt_half,
                                                         float>(EX_TYPECASTING_PARM);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_8I && b_type == HIPBLASLT_R_8I) //int8
    {
        if(c_type == HIPBLASLT_R_8I && d_type == HIPBLASLT_R_8I)
        {
            if(compute_type == rocblaslt_compute_i32)
            {
                rs_status = rocblaslt_matmul_typecasting<hipblasLtInt8,
                                                         hipblasLtInt8,
                                                         hipblasLtInt8,
                                                         int32_t>(EX_TYPECASTING_PARM);
            }
        }
        else if(c_type == HIPBLASLT_R_32I && d_type == HIPBLASLT_R_32I)
        {
            if(compute_type == rocblaslt_compute_i32)
            {
                rs_status
                    = rocblaslt_matmul_typecasting<hipblasLtInt8, hipblasLtInt8, int32_t, int32_t>(
                        EX_TYPECASTING_PARM);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_16F && b_type == HIPBLASLT_R_8F_E4M3) // mix types
    {
        if(c_type == HIPBLASLT_R_8F_E4M3 && d_type == HIPBLASLT_R_8F_E4M3)
        {
            if(compute_type == rocblaslt_compute_f32_fast_f16)
            {
                rs_status = rocblaslt_matmul_typecasting<rocblaslt_half,
                                                         rocblaslt_f8,
                                                         rocblaslt_f8,
                                                         float>(EX_TYPECASTING_PARM);
            }
        }
        else if(c_type == HIPBLASLT_R_16F && d_type == HIPBLASLT_R_16F)
        {
            if(compute_type == rocblaslt_compute_f32_fast_f16)
            {
                rs_status = rocblaslt_matmul_typecasting<rocblaslt_half,
                                                         rocblaslt_f8,
                                                         rocblaslt_half,
                                                         float>(EX_TYPECASTING_PARM);
            }
        }
        else if(c_type == HIPBLASLT_R_32F && d_type == HIPBLASLT_R_32F)
        {
            if(compute_type == rocblaslt_compute_f32_fast_f16)
            {
                rs_status = rocblaslt_matmul_typecasting<rocblaslt_half,
                                                         rocblaslt_f8,
                                                         float,
                                                         float>(EX_TYPECASTING_PARM);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_8F_E4M3 && b_type == HIPBLASLT_R_16F) // mix types
    {
        if(c_type == HIPBLASLT_R_8F_E4M3 && d_type == HIPBLASLT_R_8F_E4M3)
        {
            if(compute_type == rocblaslt_compute_f32_fast_f16)
            {
                rs_status = rocblaslt_matmul_typecasting<rocblaslt_f8,
                                                         rocblaslt_half,
                                                         rocblaslt_f8,
                                                         float>(EX_TYPECASTING_PARM);
            }
        }
        else if(c_type == HIPBLASLT_R_16F && d_type == HIPBLASLT_R_16F)
        {
            if(compute_type == rocblaslt_compute_f32_fast_f16)
            {
                rs_status = rocblaslt_matmul_typecasting<rocblaslt_f8,
                                                         rocblaslt_half,
                                                         rocblaslt_half,
                                                         float>(EX_TYPECASTING_PARM);
            }
        }
        else if(c_type == HIPBLASLT_R_32F && d_type == HIPBLASLT_R_32F)
        {
            if(compute_type == rocblaslt_compute_f32_fast_f16)
            {
                rs_status = rocblaslt_matmul_typecasting<rocblaslt_f8,
                                                         rocblaslt_half,
                                                         float,
                                                         float>(EX_TYPECASTING_PARM);
            }
        }
    }
    else
    {
        log_error(__func__, "No such template.");
        rs_status = rocblaslt_status_not_implemented;
    }

    return rs_status;
}

inline rocblaslt_status rocblaslt_gemm_create_template_cpp(hipblasOperation_t     trans_a,
                                                           hipblasOperation_t     trans_b,
                                                           int64_t                m,
                                                           int64_t                n,
                                                           int64_t                k,
                                                           const void*            alpha,
                                                           const void*            a,
                                                           hipblasltDatatype_t    a_type,
                                                           int64_t                ld_a,
                                                           int64_t                batch_stride_a,
                                                           const void*            b,
                                                           hipblasltDatatype_t    b_type,
                                                           int64_t                ld_b,
                                                           int64_t                batch_stride_b,
                                                           const void*            beta,
                                                           const void*            c,
                                                           hipblasltDatatype_t    c_type,
                                                           int64_t                ld_c,
                                                           int64_t                batch_stride_c,
                                                           void*                  d,
                                                           hipblasltDatatype_t    d_type,
                                                           int64_t                ld_d,
                                                           int64_t                batch_stride_d,
                                                           void*                  e,
                                                           int64_t                ld_e,
                                                           int64_t                batch_stride_e,
                                                           int64_t                batch_count,
                                                           bool                   strided_batch,
                                                           bool                   grouped_gemm,
                                                           bool                   gradient,
                                                           rocblaslt_compute_type compute_type,
                                                           const void*            bias,
                                                           const void*            scaleA,
                                                           const void*            scaleB,
                                                           const void*            scaleC,
                                                           const void*            scaleD,
                                                           const void*            scaleE,
                                                           const void*            scaleAlphaVec,
                                                           hipblasltDatatype_t    bias_type,
                                                           rocblaslt_epilogue     epilogue,
                                                           std::shared_ptr<void>& gemmData,
                                                           size_t&                gemmCount)
{
    rocblaslt_status rs_status = rocblaslt_status_not_implemented;

#define EX_TYPECASTING_PARM_GEMM_CPP                                                              \
    trans_a, trans_b, m, n, k, alpha, a, ld_a, batch_stride_a, b, ld_b, batch_stride_b, beta, c,  \
        ld_c, batch_stride_c, d, ld_d, batch_stride_d, e, ld_e, batch_stride_e, batch_count,      \
        strided_batch, grouped_gemm, gradient, compute_type, bias, scaleA, scaleB, scaleC, scaleD,\
        scaleE, scaleAlphaVec, bias_type, epilogue, gemmData, gemmCount

    if(a_type == HIPBLASLT_R_32F && b_type == HIPBLASLT_R_32F)
    {
        if(c_type == HIPBLASLT_R_32F && d_type == HIPBLASLT_R_32F)
        {
            if(compute_type == rocblaslt_compute_f32
               || compute_type == rocblaslt_compute_f32_fast_xf32)
            {
                rs_status = rocblaslt_gemm_create_typecasting<float, float, float, float>(
                    EX_TYPECASTING_PARM_GEMM_CPP);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_16F && b_type == HIPBLASLT_R_16F)
    {
        if(c_type == HIPBLASLT_R_16F && d_type == HIPBLASLT_R_16F)
        {
            if(compute_type == rocblaslt_compute_f32)
            {
                rs_status = rocblaslt_gemm_create_typecasting<rocblaslt_half,
                                                              rocblaslt_half,
                                                              rocblaslt_half,
                                                              float>(EX_TYPECASTING_PARM_GEMM_CPP);
            }
        }
        else if(c_type == HIPBLASLT_R_32F && d_type == HIPBLASLT_R_32F)
        {
            if(compute_type == rocblaslt_compute_f32)
            {
                rs_status = rocblaslt_gemm_create_typecasting<rocblaslt_half,
                                                              rocblaslt_half,
                                                              float,
                                                              float>(EX_TYPECASTING_PARM_GEMM_CPP);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_16B && b_type == HIPBLASLT_R_16B)
    {
        if(c_type == HIPBLASLT_R_16B && d_type == HIPBLASLT_R_16B)
        {
            if(compute_type == rocblaslt_compute_f32)
            {
                rs_status = rocblaslt_gemm_create_typecasting<rocblaslt_bfloat16,
                                                              rocblaslt_bfloat16,
                                                              rocblaslt_bfloat16,
                                                              float>(EX_TYPECASTING_PARM_GEMM_CPP);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_64F && b_type == HIPBLASLT_R_64F)
    {
        if(c_type == HIPBLASLT_R_64F && d_type == HIPBLASLT_R_64F)
        {
            if(compute_type == rocblaslt_compute_f64)
            {
                rs_status = rocblaslt_gemm_create_typecasting<double, double, double, double>(
                    EX_TYPECASTING_PARM_GEMM_CPP);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_8I && b_type == HIPBLASLT_R_8I) //int8
    {
        if(c_type == HIPBLASLT_R_8I && d_type == HIPBLASLT_R_8I)
        {
            if(compute_type == rocblaslt_compute_i32)
            {
                rs_status
                    = rocblaslt_gemm_create_typecasting<hipblasLtInt8,
                                                        hipblasLtInt8,
                                                        hipblasLtInt8,
                                                        int32_t>(EX_TYPECASTING_PARM_GEMM_CPP);
            }
        }
        else if(c_type == HIPBLASLT_R_32I && d_type == HIPBLASLT_R_32I)
        {
            if(compute_type == rocblaslt_compute_i32)
            {
                rs_status
                    = rocblaslt_gemm_create_typecasting<hipblasLtInt8,
                                                        hipblasLtInt8,
                                                        int32_t,
                                                        int32_t>(EX_TYPECASTING_PARM_GEMM_CPP);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_16F && b_type == HIPBLASLT_R_8F_E4M3) // mix types
    {
        if(c_type == HIPBLASLT_R_8F_E4M3 && d_type == HIPBLASLT_R_8F_E4M3)
        {
            if(compute_type == rocblaslt_compute_f32_fast_f16)
            {
                rs_status = rocblaslt_gemm_create_typecasting<rocblaslt_half,
                                                              rocblaslt_f8,
                                                              rocblaslt_f8,
                                                              float>(EX_TYPECASTING_PARM_GEMM_CPP);
            }
        }
        else if(c_type == HIPBLASLT_R_16F && d_type == HIPBLASLT_R_16F)
        {
            if(compute_type == rocblaslt_compute_f32_fast_f16)
            {
                rs_status = rocblaslt_gemm_create_typecasting<rocblaslt_half,
                                                              rocblaslt_f8,
                                                              rocblaslt_half,
                                                              float>(EX_TYPECASTING_PARM_GEMM_CPP);
            }
        }
        else if(c_type == HIPBLASLT_R_32F && d_type == HIPBLASLT_R_32F)
        {
            if(compute_type == rocblaslt_compute_f32_fast_f16)
            {
                rs_status = rocblaslt_gemm_create_typecasting<rocblaslt_half,
                                                              rocblaslt_f8,
                                                              float,
                                                              float>(EX_TYPECASTING_PARM_GEMM_CPP);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_8F_E4M3 && b_type == HIPBLASLT_R_16F) // mix types
    {
        if(c_type == HIPBLASLT_R_8F_E4M3 && d_type == HIPBLASLT_R_8F_E4M3)
        {
            if(compute_type == rocblaslt_compute_f32_fast_f16)
            {
                rs_status = rocblaslt_gemm_create_typecasting<rocblaslt_f8,
                                                              rocblaslt_half,
                                                              rocblaslt_f8,
                                                              float>(EX_TYPECASTING_PARM_GEMM_CPP);
            }
        }
        else if(c_type == HIPBLASLT_R_16F && d_type == HIPBLASLT_R_16F)
        {
            if(compute_type == rocblaslt_compute_f32_fast_f16)
            {
                rs_status = rocblaslt_gemm_create_typecasting<rocblaslt_f8,
                                                              rocblaslt_half,
                                                              rocblaslt_half,
                                                              float>(EX_TYPECASTING_PARM_GEMM_CPP);
            }
        }
        else if(c_type == HIPBLASLT_R_32F && d_type == HIPBLASLT_R_32F)
        {
            if(compute_type == rocblaslt_compute_f32_fast_f16)
            {
                rs_status = rocblaslt_gemm_create_typecasting<rocblaslt_f8,
                                                              rocblaslt_half,
                                                              float,
                                                              float>(EX_TYPECASTING_PARM_GEMM_CPP);
            }
        }
    }
    else
    {
        log_error(__func__, "No such template.");
        rs_status = rocblaslt_status_not_implemented;
    }

    return rs_status;
}

inline rocblaslt_status
    rocblaslt_groupedgemm_create_template_cpp(hipblasOperation_t                trans_a,
                                              hipblasOperation_t                trans_b,
                                              std::vector<int64_t>&             m,
                                              std::vector<int64_t>&             n,
                                              std::vector<int64_t>&             k,
                                              std::vector<const void*>&         alpha,
                                              std::vector<const void*>&         a,
                                              hipblasltDatatype_t               a_type,
                                              std::vector<int64_t>&             ld_a,
                                              std::vector<int64_t>&             batch_stride_a,
                                              std::vector<const void*>&         b,
                                              hipblasltDatatype_t               b_type,
                                              std::vector<int64_t>&             ld_b,
                                              std::vector<int64_t>&             batch_stride_b,
                                              std::vector<const void*>&         beta,
                                              std::vector<const void*>&         c,
                                              hipblasltDatatype_t               c_type,
                                              std::vector<int64_t>&             ld_c,
                                              std::vector<int64_t>&             batch_stride_c,
                                              std::vector<void*>&               d,
                                              hipblasltDatatype_t               d_type,
                                              std::vector<int64_t>&             ld_d,
                                              std::vector<int64_t>&             batch_stride_d,
                                              std::vector<void*>&               e,
                                              std::vector<int64_t>&             ld_e,
                                              std::vector<int64_t>&             batch_stride_e,
                                              std::vector<int64_t>&             batch_count,
                                              bool                              strided_batch,
                                              bool                              grouped_gemm,
                                              std::vector<bool>&                gradient,
                                              rocblaslt_compute_type            compute_type,
                                              std::vector<const void*>&         bias,
                                              std::vector<const void*>&         scaleA,
                                              std::vector<const void*>&         scaleB,
                                              std::vector<const void*>&         scaleC,
                                              std::vector<const void*>&         scaleD,
                                              std::vector<const void*>&         scaleE,
                                              std::vector<const void*>&         scaleAlphaVec,
                                              std::vector<hipblasltDatatype_t>& bias_type,
                                              std::vector<rocblaslt_epilogue>&  epilogue,
                                              std::shared_ptr<void>&            gemmData,
                                              size_t&                           gemmCount)
{
    rocblaslt_status rs_status = rocblaslt_status_not_implemented;

#define EX_TYPECASTING_PARM_GroupedGemm_CPP                                                       \
    trans_a, trans_b, m, n, k, alpha, a, ld_a, batch_stride_a, b, ld_b, batch_stride_b, beta, c,  \
        ld_c, batch_stride_c, d, ld_d, batch_stride_d, e, ld_e, batch_stride_e, batch_count,      \
        strided_batch, grouped_gemm, compute_type, gradient, bias, scaleA, scaleB, scaleC, scaleD,\
        scaleE, scaleAlphaVec, bias_type, epilogue, gemmData, gemmCount

    if(a_type == HIPBLASLT_R_32F && b_type == HIPBLASLT_R_32F)
    {
        if(c_type == HIPBLASLT_R_32F && d_type == HIPBLASLT_R_32F)
        {
            if(compute_type == rocblaslt_compute_f32
               || compute_type == rocblaslt_compute_f32_fast_xf32)
            {
                rs_status = rocblaslt_groupedgemm_create_typecasting<float, float, float, float>(
                    EX_TYPECASTING_PARM_GroupedGemm_CPP);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_16F && b_type == HIPBLASLT_R_16F)
    {
        if(c_type == HIPBLASLT_R_16F && d_type == HIPBLASLT_R_16F)
        {
            if(compute_type == rocblaslt_compute_f32)
            {
                rs_status = rocblaslt_groupedgemm_create_typecasting<rocblaslt_half,
                                                                     rocblaslt_half,
                                                                     rocblaslt_half,
                                                                     float>(
                    EX_TYPECASTING_PARM_GroupedGemm_CPP);
            }
        }
        else if(c_type == HIPBLASLT_R_32F && d_type == HIPBLASLT_R_32F)
        {
            if(compute_type == rocblaslt_compute_f32)
            {
                rs_status = rocblaslt_groupedgemm_create_typecasting<rocblaslt_half,
                                                                     rocblaslt_half,
                                                                     float,
                                                                     float>(
                    EX_TYPECASTING_PARM_GroupedGemm_CPP);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_16B && b_type == HIPBLASLT_R_16B)
    {
        if(c_type == HIPBLASLT_R_16B && d_type == HIPBLASLT_R_16B)
        {
            if(compute_type == rocblaslt_compute_f32)
            {
                rs_status = rocblaslt_groupedgemm_create_typecasting<rocblaslt_bfloat16,
                                                                     rocblaslt_bfloat16,
                                                                     rocblaslt_bfloat16,
                                                                     float>(
                    EX_TYPECASTING_PARM_GroupedGemm_CPP);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_64F && b_type == HIPBLASLT_R_64F)
    {
        if(c_type == HIPBLASLT_R_64F && d_type == HIPBLASLT_R_64F)
        {
            if(compute_type == rocblaslt_compute_f64)
            {
                rs_status = rocblaslt_groupedgemm_create_typecasting<rocblaslt_bfloat16,
                                                                     rocblaslt_bfloat16,
                                                                     rocblaslt_bfloat16,
                                                                     float>(
                    EX_TYPECASTING_PARM_GroupedGemm_CPP);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_8I && b_type == HIPBLASLT_R_8I) //int8
    {
        if(c_type == HIPBLASLT_R_8I && d_type == HIPBLASLT_R_8I)
        {
            if(compute_type == rocblaslt_compute_i32)
            {
                rs_status = rocblaslt_groupedgemm_create_typecasting<hipblasLtInt8,
                                                                     hipblasLtInt8,
                                                                     hipblasLtInt8,
                                                                     int32_t>(
                    EX_TYPECASTING_PARM_GroupedGemm_CPP);
            }
        }
        else if(c_type == HIPBLASLT_R_32I && d_type == HIPBLASLT_R_32I)
        {
            if(compute_type == rocblaslt_compute_i32)
            {
                rs_status = rocblaslt_groupedgemm_create_typecasting<hipblasLtInt8,
                                                                     hipblasLtInt8,
                                                                     int32_t,
                                                                     int32_t>(
                    EX_TYPECASTING_PARM_GroupedGemm_CPP);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_16F && b_type == HIPBLASLT_R_8F_E4M3) // mix types
    {
        if(c_type == HIPBLASLT_R_8F_E4M3 && d_type == HIPBLASLT_R_8F_E4M3)
        {
            if(compute_type == rocblaslt_compute_f32_fast_f16)
            {
                rs_status = rocblaslt_groupedgemm_create_typecasting<rocblaslt_half,
                                                                     rocblaslt_f8,
                                                                     rocblaslt_f8,
                                                                     float>(
                    EX_TYPECASTING_PARM_GroupedGemm_CPP);
            }
        }
        else if(c_type == HIPBLASLT_R_16F && d_type == HIPBLASLT_R_16F)
        {
            if(compute_type == rocblaslt_compute_f32_fast_f16)
            {
                rs_status = rocblaslt_groupedgemm_create_typecasting<rocblaslt_half,
                                                                     rocblaslt_f8,
                                                                     rocblaslt_half,
                                                                     float>(
                    EX_TYPECASTING_PARM_GroupedGemm_CPP);
            }
        }
        else if(c_type == HIPBLASLT_R_32F && d_type == HIPBLASLT_R_32F)
        {
            if(compute_type == rocblaslt_compute_f32_fast_f16)
            {
                rs_status = rocblaslt_groupedgemm_create_typecasting<rocblaslt_half,
                                                                     rocblaslt_f8,
                                                                     float,
                                                                     float>(
                    EX_TYPECASTING_PARM_GroupedGemm_CPP);
            }
        }
    }
    else if(a_type == HIPBLASLT_R_8F_E4M3 && b_type == HIPBLASLT_R_16F) // mix types
    {
        if(c_type == HIPBLASLT_R_8F_E4M3 && d_type == HIPBLASLT_R_8F_E4M3)
        {
            if(compute_type == rocblaslt_compute_f32_fast_f16)
            {
                rs_status = rocblaslt_groupedgemm_create_typecasting<rocblaslt_f8,
                                                                     rocblaslt_half,
                                                                     rocblaslt_f8,
                                                                     float>(
                    EX_TYPECASTING_PARM_GroupedGemm_CPP);
            }
        }
        else if(c_type == HIPBLASLT_R_16F && d_type == HIPBLASLT_R_16F)
        {
            if(compute_type == rocblaslt_compute_f32_fast_f16)
            {
                rs_status = rocblaslt_groupedgemm_create_typecasting<rocblaslt_f8,
                                                                     rocblaslt_half,
                                                                     rocblaslt_half,
                                                                     float>(
                    EX_TYPECASTING_PARM_GroupedGemm_CPP);
            }
        }
        else if(c_type == HIPBLASLT_R_32F && d_type == HIPBLASLT_R_32F)
        {
            if(compute_type == rocblaslt_compute_f32_fast_f16)
            {
                rs_status = rocblaslt_groupedgemm_create_typecasting<rocblaslt_f8,
                                                                     rocblaslt_half,
                                                                     float,
                                                                     float>(
                    EX_TYPECASTING_PARM_GroupedGemm_CPP);
            }
        }
    }
    else
    {
        log_error(__func__, "No such template.");
        rs_status = rocblaslt_status_not_implemented;
    }

    return rs_status;
}
#endif
