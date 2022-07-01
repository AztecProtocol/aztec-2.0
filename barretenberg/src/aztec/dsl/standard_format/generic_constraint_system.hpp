
#include <plonk/reference_string/pippenger_reference_string.hpp>
#include <plonk/composer/turbo_composer.hpp>
#include "dsl/standard_format/standard_format.hpp"
#include <plonk/composer/turbo/compute_verification_key.hpp>
#include <plonk/proof_system/commitment_scheme/kate_commitment_scheme.hpp>

typedef std::chrono::high_resolution_clock Clock;

using namespace waffle;
using namespace std;

#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

// This is a generic construction of the wasm bindings for joint_split and
// account. We take in a constraint_system, as the circuit is not known
// at compile time
namespace generic_constraint_system {

static std::shared_ptr<waffle::proving_key> proving_key;
static std::shared_ptr<waffle::verification_key> verification_key;
static std::shared_ptr<standard_format> constraint_system;

void init_circuit(standard_format cs)
{
    constraint_system = std::make_shared<standard_format>(cs);
}

void init_proving_key()
{
    auto crs_factory = std::make_unique<waffle::ReferenceStringFactory>();
    auto composer = create_circuit(*constraint_system, std::move(crs_factory));

    proving_key = composer.compute_proving_key();
}

void init_verification_key(std::unique_ptr<waffle::ReferenceStringFactory>&& crs_factory)
{
    // This is a problem with barretenberg. We should not need the proving key to compute the
    // verification key. The proving key also includes the evaluations over a domain of size 4n, where n is the circuit
    // size, which is not needed.
    if (!proving_key) {
        std::abort();
    }
    // Patch the 'nothing' reference string fed to init_proving_key.
    proving_key->reference_string = crs_factory->get_prover_crs(proving_key->n);

    verification_key = waffle::turbo_composer::compute_verification_key(proving_key, crs_factory->get_verifier_crs());
}

bool verify_proof(waffle::plonk_proof const& proof)
{
    TurboVerifier verifier(verification_key, Composer::create_manifest(verification_key->num_public_inputs));

    std::unique_ptr<waffle::KateCommitmentScheme<waffle::turbo_settings>> kate_commitment_scheme =
        std::make_unique<waffle::KateCommitmentScheme<waffle::turbo_settings>>();
    verifier.commitment_scheme = std::move(kate_commitment_scheme);

    return verifier.verify_proof(proof);
}

void init_verification_key(std::shared_ptr<waffle::VerifierMemReferenceString> const& crs,
                           waffle::verification_key_data&& vk_data)
{
    verification_key = std::make_shared<waffle::verification_key>(std::move(vk_data), crs);
}

TurboProver new_generic_prover(std::vector<fr> witness)
{
    TurboComposer composer(proving_key, nullptr);

    create_circuit_with_witness(composer, *constraint_system, witness);

    if (composer.failed) {
        error("composer logic failed: ", composer.err);
    }
    return composer.create_prover();
}

std::shared_ptr<waffle::proving_key> get_proving_key()
{
    return proving_key;
}

std::shared_ptr<waffle::verification_key> get_verification_key()
{
    return verification_key;
}

} // namespace generic_constraint_system