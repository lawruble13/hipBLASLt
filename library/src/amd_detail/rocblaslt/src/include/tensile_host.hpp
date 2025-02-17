/*! \file */
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

/*********************************************************
 * Declaration of the rocBLAS<->Tensile interface layer. *
 *********************************************************/

#pragma once

/*****************************************************************************
 * WARNING: Tensile-specific data types, functions and macros should only be *
 * referenced from tensile_host.cpp. This header file defines the interface  *
 * that the rest of rocBLAS uses to access Tensile. If another Tensile       *
 * feature needs to be accessed, the API for accessing it should be defined  *
 * in this file, without referencing any Tensile-specific identifiers here.  *
 *****************************************************************************/

#include "handle.h"
//#include "tuple_helper.hpp"
#include "utility.hpp"
#include <atomic>
#include <Tensile/DataTypes.hpp>

// Return the value category for a value, as a double precision value, such
// such as whether it's 0, 1, -1 or some other value. Tensile uses a double
// precision value to express the category of beta. This function is to
// convert complex or other types to a double representing the category.
template <typename T>
constexpr double value_category(const T& beta)
{
    return beta == T(0) ? 0.0 : beta == T(1) ? 1.0 : beta == T(-1) ? -1.0 : 2.0;
}

/********************************************************************
 * RocblasltContractionProblem captures the arguments for a GEMM-like *
 * contraction problem, to be passed to runContractionProblem.      *
 ********************************************************************/
template <typename TiA, typename TiB = TiA, typename To = TiB, typename Tc = To>
struct RocblasltContractionProblem
{
    hipblasOperation_t trans_a;
    hipblasOperation_t trans_b;

    // The RocblasltContractionProblem data members should exactly match
    // Tensile's parameter types, even if rocBLAS uses differently
    // sized or signed types. The constructors should convert rocBLAS
    // types into the corresponding Tensile types stored in this class.
    size_t m;
    size_t n;
    size_t k;

    const Tc* alpha;

    const TiA*        A;
    const TiA* const* batch_A;
    size_t            row_stride_a;
    size_t            col_stride_a;
    size_t            batch_stride_a;

    const TiB*        B;
    const TiB* const* batch_B;
    size_t            row_stride_b;
    size_t            col_stride_b;
    size_t            batch_stride_b;

    const Tc* beta;

    const To*        C;
    const To* const* batch_C;
    size_t           row_stride_c;
    size_t           col_stride_c;
    size_t           batch_stride_c;

    To*        D;
    To* const* batch_D;
    size_t     row_stride_d;
    size_t     col_stride_d;
    size_t     batch_stride_d;

    Tc*        E;
    Tc* const* batch_E;
    size_t     row_stride_e;
    size_t     col_stride_e;
    size_t     batch_stride_e;

    size_t batch_count;
    bool   strided_batch;
    bool   grouped_gemm;
    bool   gradient;

    rocblaslt_compute_type compute_type;

    const void*         bias;
    const Tc*           scaleA;
    const Tc*           scaleB;
    const Tc*           scaleC;
    const Tc*           scaleD;
    const Tc*           scaleE;
    const Tc*           scaleAlphaVec;
    hipblasltDatatype_t bias_type;
    rocblaslt_epilogue  epilogue;
    void*               workspace;
    size_t              workspaceSize;

    hipStream_t stream;

    // gemm_ex
    // gemm_strided_batched_ex
    RocblasltContractionProblem(hipblasOperation_t     trans_a,
                                hipblasOperation_t     trans_b,
                                int64_t                m,
                                int64_t                n,
                                int64_t                k,
                                const Tc*              alpha,
                                const TiA*             A,
                                const TiA* const*      batch_A,
                                int64_t                ld_a,
                                int64_t                batch_stride_a,
                                const TiB*             B,
                                const TiB* const*      batch_B,
                                int64_t                ld_b,
                                int64_t                batch_stride_b,
                                const Tc*              beta,
                                const To*              C,
                                const To* const*       batch_C,
                                int64_t                ld_c,
                                int64_t                batch_stride_c,
                                To*                    D,
                                To* const*             batch_D,
                                int64_t                ld_d,
                                int64_t                batch_stride_d,
                                Tc*                    E,
                                Tc* const*             batch_E,
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
                                void*                  workspace,
                                size_t                 workspaceSize,
                                hipStream_t            stream)
        : trans_a(trans_a)
        , trans_b(trans_b)
        , m(m)
        , n(n)
        , k(k)
        , alpha(alpha)
        , A(A)
        , batch_A(batch_A)
        , row_stride_a(1)
        , col_stride_a(ld_a)
        , batch_stride_a(batch_stride_a)
        , B(B)
        , batch_B(batch_B)
        , row_stride_b(1)
        , col_stride_b(ld_b)
        , batch_stride_b(batch_stride_b)
        , beta(beta)
        , C(C)
        , batch_C(batch_C)
        , row_stride_c(1)
        , col_stride_c(ld_c)
        , batch_stride_c(batch_stride_c)
        , D(D)
        , batch_D(batch_D)
        , row_stride_d(1)
        , col_stride_d(ld_d)
        , batch_stride_d(batch_stride_d)
        , E(E)
        , batch_E(batch_E)
        , row_stride_e(1)
        , col_stride_e(ld_e)
        , batch_stride_e(batch_stride_e)
        , batch_count(batch_count)
        , strided_batch(strided_batch)
        , grouped_gemm(grouped_gemm)
        , gradient(gradient)
        , compute_type(compute_type)
        , bias(bias)
        , scaleA(scaleA)
        , scaleB(scaleB)
        , scaleC(scaleC)
        , scaleD(scaleD)
        , scaleE(scaleE)
        , scaleAlphaVec(scaleAlphaVec)
        , bias_type(bias_type)
        , epilogue(epilogue)
        , workspace(workspace)
        , workspaceSize(workspaceSize)
        , stream(stream)
    {
    }
};

void initTensileGemmData(rocblaslt_handle       handle,
                         rocblaslt::RocGemmType gemmType,
                         hipblasOperation_t     opA,
                         hipblasOperation_t     opB,
                         hipblasltDatatype_t    typeA,
                         hipblasltDatatype_t    typeB,
                         hipblasltDatatype_t    typeC,
                         hipblasltDatatype_t    typeD,
                         rocblaslt_compute_type typeCompute,
                         size_t                 maxWorkspaceBytes,
                         std::shared_ptr<void>& gemmData);

/*******************************************************************************
 * runContractionProblem() solves a RocblasltContractionProblem *
 *******************************************************************************/
template <typename TiA, typename TiB, typename To, typename Tc>
rocblaslt_status runContractionProblem(rocblaslt_handle                                     handle,
                                       const rocblaslt_matmul_algo*                         algo,
                                       RocblasltContractionProblem<TiA, TiB, To, Tc> const& problem,
                                       std::shared_ptr<void> gemmData);

template <typename TiA, typename TiB, typename To, typename Tc>
rocblaslt_status gemmCreate(RocblasltContractionProblem<TiA, TiB, To, Tc> const& problem,
                            std::shared_ptr<void>&                               gemmData,
                            size_t&                                              gemmCount);

template <typename TiA, typename TiB, typename To, typename Tc>
rocblaslt_status
    groupedGemmCreate(std::vector<RocblasltContractionProblem<TiA, TiB, To, Tc>>& probs,
                      std::shared_ptr<void>&                                      gemmData,
                      size_t&                                                     gemmCount);

rocblaslt_status makeArgument(rocblaslt_handle             handle,
                              const rocblaslt::RocGemmType gemmType,
                              const rocblaslt_matmul_algo& algo,
                              void*                        workspace,
                              bool                         useUserArgs,
                              hipStream_t                  stream,
                              std::shared_ptr<void>        gemmData);

// Run gemm only, without creating args, problems,...
rocblaslt_status runKernelFromInvocation(rocblaslt_handle       handle,
                                         rocblaslt::RocGemmType gemmType,
                                         std::shared_ptr<void>  gemmData,
                                         hipStream_t            stream);

rocblaslt_status getDeviceUserArgumentsValuesFromContractionProblem(rocblaslt_handle       handle,
                                                                    rocblaslt::RocGemmType gemmType,
                                                                    std::shared_ptr<void>  gemmData,
                                                                    void* hostDeviceUserArgs);

rocblaslt_status runKernelFromNewDeviceUserArguments(rocblaslt_handle       handle,
                                                     rocblaslt::RocGemmType gemmType,
                                                     std::shared_ptr<void>  gemmData,
                                                     void*                  deviceUserArgs,
                                                     hipStream_t            stream);

rocblaslt_status runKernelFromDeviceUserArguments(rocblaslt_handle             handle,
                                                  rocblaslt::RocGemmType       gemmType,
                                                  size_t                       gemmCount,
                                                  std::shared_ptr<void>        gemmData,
                                                  const rocblaslt_matmul_algo& algo,
                                                  void*                        deviceUserArgs,
                                                  void*                        workspace,
                                                  hipStream_t                  stream);

/***********************************************************************************
 * Whether Tensile has been initialized for at least one device (used for
 *testing) *
 ***********************************************************************************/
std::atomic_bool& rocblaslt_internal_tensile_is_initialized();

/**********************************************
 * Whether to suppress Tensile error messages *
 **********************************************/
inline bool& rocblaslt_suppress_tensile_error_messages()
{
    thread_local bool t_suppress = false;
    return t_suppress;
}

template <typename TiA, typename TiB = TiA, typename To = TiB, typename Tc = To>
rocblaslt_status getAllSolutions(RocblasltContractionProblem<TiA, TiB, To, Tc>&  prob,
                                 rocblaslt_handle                                handle,
                                 std::vector<rocblaslt_matmul_heuristic_result>& heuristicResults,
                                 size_t                                          maxWorkSpaceBytes);

template <typename TiA, typename TiB = TiA, typename To = TiB, typename Tc = To>
rocblaslt_status getAllSolutions(std::vector<RocblasltContractionProblem<TiA, TiB, To, Tc>>& probs,
                                 rocblaslt_handle                                            handle,
                                 std::vector<rocblaslt_matmul_heuristic_result>& heuristicResults,
                                 size_t                                          maxWorkSpaceBytes);

rocblaslt_status
    getSolutionsFromIndex(rocblaslt_handle                                handle,
                          std::vector<int>&                               solutionIndex,
                          std::vector<rocblaslt_matmul_heuristic_result>& heuristicResults,
                          size_t                                          maxWorkSpaceBytes);

template <typename TiA, typename TiB = TiA, typename To = TiB, typename Tc = To>
rocblaslt_status isSolutionSupported(rocblaslt_handle                               handle,
                                     RocblasltContractionProblem<TiA, TiB, To, Tc>& prob,
                                     std::shared_ptr<void>                          gemmData,
                                     rocblaslt_matmul_algo*                         algo,
                                     size_t* workspaceSizeInBytes);

rocblaslt_status isSolutionSupported(rocblaslt_handle              handle,
                                     const rocblaslt::RocGemmType& gemmType,
                                     std::shared_ptr<void>         gemmData,
                                     rocblaslt_matmul_algo&        algo,
                                     size_t&                       workspaceSizeInBytes);

/*******************************************************************************
 * getBestSolutions() calls finTopSolutions from Tensile and converts to       *
 * rocblaslt_matmul_heuristic_result                                           *
 *******************************************************************************/
template <typename TiA, typename TiB = TiA, typename To = TiB, typename Tc = To>
rocblaslt_status getBestSolutions(RocblasltContractionProblem<TiA, TiB, To, Tc> prob,
                                  rocblaslt_handle                              handle,
                                  std::shared_ptr<void>                         gemmData,
                                  int                                           requestedAlgoCount,
                                  rocblaslt_matmul_heuristic_result heuristicResultsArray[],
                                  int*                              returnAlgoCount,
                                  size_t                            maxWorkSpaceBytes);

rocblaslt_status getBestSolutions(rocblaslt_handle       handle,
                                  rocblaslt::RocGemmType gemmType,
                                  std::shared_ptr<void>  gemmData,
                                  const int              workspaceBytes,
                                  const int              requestedAlgoCount,
                                  std::vector<rocblaslt_matmul_heuristic_result>& heuristicResults);

/******************************************************
 * Map a hipblaslt data type to a corresponding Tensile type *
 ******************************************************/
inline Tensile::DataType hipblasltDatatype_to_tensile_type(hipblasltDatatype_t type)
{
    switch(type)
    {
    case HIPBLASLT_R_16F:
        return Tensile::DataType::Half;
    case HIPBLASLT_R_32F:
        return Tensile::DataType::Float;
    case HIPBLASLT_R_64F:
        return Tensile::DataType::Double;
    case HIPBLASLT_R_16B:
        return Tensile::DataType::BFloat16;
    case HIPBLASLT_R_8F_E4M3:
        return Tensile::DataType::Float8;
    case HIPBLASLT_R_8F_E5M2:
        return Tensile::DataType::BFloat8;
    case HIPBLASLT_R_8I:
        return Tensile::DataType::Int8;
    case HIPBLASLT_R_32I:
        return Tensile::DataType::Int32;
    default:
        assert(!"hipblasltDatatype_to_tensile_type: non-supported type");
        return Tensile::DataType::None;
    }
}
