// matmul_base.cpp
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <stdint.h>

/*
  BLAS 
    C(m x n) = alpha * op(A) * op(B) + beta * C
    A: (m x k), B: (k x n)
    transa/transb == 'N' or 'T'
 */
void matmul_proj5(char transa, char transb,
                  int64_t m, int64_t n, int64_t k,
                  float alpha, const float* a, int64_t lda,
                  const float* b, int64_t ldb,
                  float beta, float* c, int64_t ldc);

int main(int argc, char** argv) {
  if (argc < 5) {
    std::fprintf(stderr,
      "Usage: %s M N K NUM_ITERS\n"
      "  Example: %s 64 64 64 20\n",
      argv[0], argv[0]);
    return 1;
  }

  int64_t M = std::atoll(argv[1]);
  int64_t N = std::atoll(argv[2]);
  int64_t K = std::atoll(argv[3]);
  int64_t num_iters = std::atoll(argv[4]);

  int64_t sizeA = M * K;
  int64_t sizeB = K * N;
  int64_t sizeC = M * N;

  std::vector<float> A(sizeA), B(sizeB), C(sizeC);

  for (int64_t i = 0; i < sizeA; ++i) {
    A[i] = static_cast<float>((i % 7) + 1);  
  }
  for (int64_t i = 0; i < sizeB; ++i) {
    B[i] = static_cast<float>(((i * 3) % 13) + 1); 
  }
  for (int64_t i = 0; i < sizeC; ++i) {
    C[i] = 0.0f;
  }

  for (int64_t iter = 0; iter < num_iters; ++iter) {
    matmul_proj5('N', 'N',
                 M, N, K,
                 /*alpha=*/1.0f, A.data(), M,
                 B.data(), K,
                 /*beta=*/0.0f, C.data(), M);
  }

  // correctness check
  double checksum = 0.0;
  for (int64_t i = 0; i < sizeC; ++i) {
    checksum += C[i];
  }
  std::printf("Checksum: %.6f\n", checksum);
  return 0;
}

// COMMIT start
// code section to optimize
// void matmul_proj5(char transa, char transb,
//                   int64_t m, int64_t n, int64_t k,
//                   float alpha, const float* a, int64_t lda,
//                   const float* b, int64_t ldb,
//                   float beta, float* c, int64_t ldc) {
//   for (int64_t j = 0; j < n; ++j) {
//     for (int64_t i = 0; i < m; ++i) {
//       float sum = 0.0f;
//       for (int64_t l = 0; l < k; ++l) {
//         float elemA = (transa == 'T')
//                         ? a[l + i * lda] // lda = M
//                         : a[i + l * lda]; 
//         float elemB = (transb == 'T')
//                         ? b[j + l * ldb] // ldb = K
//                         : b[l + j * ldb];
//         sum += elemA * elemB;
//       }
//       // C(i,j) = alpha * sum + beta * C(i,j)
//       c[i + j * ldc] = alpha * sum + beta * c[i + j * ldc]; // ldc = M
//     }
//   }
// }
void matmul_proj5(char transa, char transb,
                  int64_t m, int64_t n, int64_t k,
                  float alpha, const float* a, int64_t lda,
                  const float* b, int64_t ldb,
                  float beta, float* c, int64_t ldc)
{
    const int transA = (transa == 'T' || transa == 't');
    const int transB = (transb == 'T' || transb == 't');

    if (!transA && !transB) {
        for (int64_t j = 0; j < n; ++j) {
            float *cj = c + j * ldc;         // C(:, j)
            const float *bj = b + j * ldb;   // B(:, j)

            // beta에 따라 C(:,j) 초기 스케일링
            if (beta == 0.0f) {
                // C(:,j) = 0
                for (int64_t i = 0; i < m; ++i) {
                    cj[i] = 0.0f;
                }
            } else if (beta != 1.0f) {
                // C(:,j) *= beta
                for (int64_t i = 0; i < m; ++i) {
                    cj[i] *= beta;
                }
            }
            // beta == 1.0f 이면 아무것도 안 함

            // Loop Interchange + Loop-Invariant Hoisting 
            // j -> i -> l 순서에서 j -> l -> i 순서로 변경했다 
            for (int64_t l = 0; l < k; ++l) {
                const float b_lj = bj[l];        // B(l, j)
                const float *a_l = a + l * lda;  // A(:, l) 

                // alpha * b_lj은 loop invariant이므로 미리 계산한다
                const float coeff = alpha * b_lj;

                // Loop Unrolling + Strength Reduction 
                int64_t i = 0;

                // 4-way unrolling - 현재 들어오는게 M = 16이고 L1 Cache가 모두 8-way이므로 
                for (; i + 3 < m; i += 4) {
                    cj[i    ] += coeff * a_l[i    ];
                    cj[i + 1] += coeff * a_l[i + 1];
                    cj[i + 2] += coeff * a_l[i + 2];
                    cj[i + 3] += coeff * a_l[i + 3];
                }
                // 나머지 처리
                for (; i < m; ++i) {
                    cj[i] += coeff * a_l[i];
                }
            }
        }
        return;
    }

    // Transpose Case에 대해서 
    for (int64_t j = 0; j < n; ++j) {
        for (int64_t i = 0; i < m; ++i) {
            float sum = 0.0f;

            if (!transA && !transB) {
                // 이 경우는 사실상 들어올 일이 없음 
                for (int64_t l = 0; l < k; ++l) {
                    float elemA = a[i + l * lda];
                    float elemB = b[l + j * ldb];
                    sum += elemA * elemB;
                }
            } else if (transA && !transB) {
                // A^T * B
                for (int64_t l = 0; l < k; ++l) {
                    float elemA = a[l + i * lda];     
                    float elemB = b[l + j * ldb];     
                    sum += elemA * elemB;
                }
            } else if (!transA && transB) {
                // A * B^T
                for (int64_t l = 0; l < k; ++l) {
                    float elemA = a[i + l * lda];     
                    float elemB = b[j + l * ldb];     
                    sum += elemA * elemB;
                }
            } else {
                // A^T * B^T
                for (int64_t l = 0; l < k; ++l) {
                    float elemA = a[l + i * lda];     
                    float elemB = b[j + l * ldb];     
                    sum += elemA * elemB;
                }
            }

            // C(i,j) = alpha * sum + beta * C(i,j)
            float cij = c[i + j * ldc];

            // 마찬가지로 beta에 따라서 계산량을 줄였다 - Algebraic Simplification 
            if (beta == 0.0f) {
                cij = alpha * sum;
            } else if (beta == 1.0f) {
                cij = alpha * sum + cij;
            } else {
                cij = alpha * sum + beta * cij;
            }

            c[i + j * ldc] = cij;
        }
    }
}
// COMMIT end
