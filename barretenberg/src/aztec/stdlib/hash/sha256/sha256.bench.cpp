#include "sha256.hpp"
#include <benchmark/benchmark.h>
#include <ecc/curves/bn254/fr.hpp>
#include "../../../plonk/composer/turbo_composer.hpp"
#include "../../../stdlib/types/turbo.hpp"

using namespace benchmark;
using namespace plonk::stdlib::types::turbo;

constexpr size_t NUM_HASHES = 10;

char get_random_char()
{
    return static_cast<char>(barretenberg::fr::random_element().data[0] % 8);
}

void generate_test_plonk_circuit(Composer& composer, size_t num_hashes)
{
    for (size_t j = 0; j < num_hashes; ++j) {

        std::string in;
        in.resize(53);
        // NIST standard reserves 9 bytes for message metadata, leaving 53 bytes for the message
        for (size_t i = 0; i < 53; ++i) {
            in[i] = get_random_char();
        }
        bit_array_ct input(&composer, in);
        plonk::stdlib::sha256(input);
    }
}

Composer composers[NUM_HASHES];
Prover provers[NUM_HASHES];
Verifier verifiers[NUM_HASHES];
waffle::plonk_proof proofs[NUM_HASHES];

void construct_witnesses_bench(State& state) noexcept
{
    for (auto _ : state) {
        size_t idx = (static_cast<size_t>((state.range(0))) - 1);
        composers[idx] = Composer();
        generate_test_plonk_circuit(composers[idx], static_cast<size_t>(state.range(0)));
    }
}
BENCHMARK(construct_witnesses_bench)->DenseRange(1, NUM_HASHES, 1);

void compute_proving_key(State& state) noexcept
{
    for (auto _ : state) {
        size_t idx = (static_cast<size_t>((state.range(0))) - 1);
        provers[idx] = composers[idx].create_prover();
    }
}
BENCHMARK(compute_proving_key)->DenseRange(1, NUM_HASHES, 1);

void compute_verification_key(State& state) noexcept
{
    for (auto _ : state) {
        size_t idx = (static_cast<size_t>((state.range(0))) - 1);
        verifiers[idx] = composers[idx].create_verifier();
    }
}
BENCHMARK(compute_verification_key)->DenseRange(1, NUM_HASHES, 1);

void construct_proofs_bench(State& state) noexcept
{
    for (auto _ : state) {
        state.PauseTiming();
        size_t idx = (static_cast<size_t>((state.range(0))) - 1);
        std::cout << "constructing proof for " << state.range(0) << " 64-byte SHA256 hashes with "
                  << provers[idx].key->small_domain.size << " constraints" << std::endl;
        state.ResumeTiming();

        proofs[idx] = provers[idx].construct_proof();
        state.PauseTiming();
        provers[idx].reset();
        state.ResumeTiming();
    }
}
BENCHMARK(construct_proofs_bench)->DenseRange(1, NUM_HASHES, 1);

void verify_proofs_bench(State& state) noexcept
{
    for (auto _ : state) {
        size_t idx = (static_cast<size_t>((state.range(0))) - 1);
        verifiers[idx].verify_proof(proofs[idx]);
    }
}
BENCHMARK(verify_proofs_bench)->DenseRange(1, NUM_HASHES, 1);

BENCHMARK_MAIN();
