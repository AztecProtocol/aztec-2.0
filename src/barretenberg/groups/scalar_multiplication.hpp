#ifndef SCALAR_MULTIPLICATION
#define SCALAR_MULTIPLICATION

#include "stddef.h"
#include "malloc.h"

#ifndef NO_MULTITHREADING
#include <omp.h>
#endif

#include "../assert.hpp"

#include "../types.hpp"

namespace barretenberg
{
namespace scalar_multiplication
{
struct wnaf_runtime_state
{
    uint64_t current_sign;
    uint64_t next_sign;
    size_t current_idx;
    size_t next_idx;
    size_t bits_per_wnaf;
    uint32_t *wnaf_iterator;
    uint32_t *wnaf_table;
    bool *skew_table;
};

struct multiplication_runtime_state
{
    size_t num_points;
    size_t num_rounds;
    size_t num_buckets;
    size_t switch_point;

    g1::element *buckets;

    g1::affine_element addition_temporary;
    g1::element accumulator;
    g1::element running_sum;
};

void compute_next_bucket_index(wnaf_runtime_state &state);
void generate_pippenger_point_table(g1::affine_element *points, g1::affine_element *table, size_t num_points);
g1::element pippenger_internal(fr::field_t *scalars, g1::affine_element *points, size_t num_initial_points, fr::field_t *endo_scalars);

g1::element pippenger_low_memory(fr::field_t *scalars, g1::affine_element *points, size_t num_initial_points);

g1::element pippenger(fr::field_t *scalars, g1::affine_element *points, size_t num_initial_points);
struct multiplication_state
{
    g1::affine_element *points;
    fr::field_t *scalars;
    size_t num_elements;
    g1::element output;
};

void batched_scalar_multiplications(multiplication_state *mul_state, size_t num_exponentiations);
} // namespace scalar_multiplication
} // namespace barretenberg

#endif