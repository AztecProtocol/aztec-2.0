#include "standard_example.hpp"
#include <common/log.hpp>

namespace rollup {
namespace client_proofs {
namespace standard_example {

using namespace plonk;

static std::shared_ptr<waffle::proving_key> proving_key;
static std::shared_ptr<waffle::verification_key> verification_key;

void build_circuit(Composer& composer)
{
    uint32_ct a(witness_ct(&composer, 123));
    uint32_ct b(public_witness_ct(&composer, 456));
    bool_ct r = (a + b) == 579;
    composer.assert_equal_constant(r.witness_index, barretenberg::fr(1));
}

void init_proving_key(std::unique_ptr<waffle::ReferenceStringFactory>&& crs_factory)
{
    Composer composer(std::move(crs_factory));
    build_circuit(composer);
    proving_key = composer.compute_proving_key();
}

void init_keys(std::unique_ptr<waffle::ReferenceStringFactory>&& crs_factory)
{
    Composer composer(std::move(crs_factory));
    build_circuit(composer);
    proving_key = composer.compute_proving_key();
    verification_key = composer.compute_verification_key();
}

Prover new_prover()
{
    Composer composer(proving_key, nullptr);
    build_circuit(composer);

    info("composer gates: ", composer.get_num_gates());

    Prover prover = composer.create_prover();

    return prover;
}

bool verify_proof(waffle::plonk_proof const& proof)
{
    Verifier verifier(verification_key, Composer::create_manifest(1));
    return verifier.verify_proof(proof);
}

} // namespace create
} // namespace client_proofs
} // namespace rollup