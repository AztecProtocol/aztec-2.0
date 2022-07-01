#include "c_bind.h"
#include <common/log.hpp>
#include <plonk/reference_string/pippenger_reference_string.hpp>
#include <plonk/composer/turbo_composer.hpp>
#include <plonk/proof_system/verification_key/sol_gen.hpp>
#include "dsl/standard_format/standard_format.hpp"
#include <plonk/composer/turbo/compute_verification_key.hpp>
#include <iostream>
#include <chrono>
#include <plonk/proof_system/types/polynomial_manifest.hpp>
#include <plonk/proof_system/commitment_scheme/kate_commitment_scheme.hpp>
#include <plonk/proof_system/proving_key/serialize.hpp>

typedef std::chrono::high_resolution_clock Clock;

#define WASM_EXPORT __attribute__((visibility("default")))

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

using namespace generic_constraint_system;

extern "C" {
WASM_EXPORT void composer__compute_proving_key()
{
    init_proving_key();
}

WASM_EXPORT bool composer__generic_verify_proof(uint8_t* proof, uint32_t length)
{
    waffle::plonk_proof pp = { std::vector<uint8_t>(proof, proof + length) };
    return verify_proof(pp);
}

WASM_EXPORT void composer__init_verification_key(void* pippenger, uint8_t const* g2x)
{
    auto crs_factory = std::make_unique<waffle::PippengerReferenceStringFactory>(
        reinterpret_cast<scalar_multiplication::Pippenger*>(pippenger), g2x);
    init_verification_key(std::move(crs_factory));
}

WASM_EXPORT void composer__init_circuit_def(uint8_t const* constraint_system_buf)
{
    auto cs = from_buffer<standard_format>(constraint_system_buf);

    init_circuit(cs);
}

WASM_EXPORT uint32_t composer__get_new_verification_key_data(uint8_t** output)
{
    auto buffer = to_buffer(*get_verification_key());
    auto raw_buf = (uint8_t*)malloc(buffer.size());
    memcpy(raw_buf, (void*)buffer.data(), buffer.size());
    *output = raw_buf;
    return static_cast<uint32_t>(buffer.size());
}

WASM_EXPORT void composer__init_verification_key_from_buffer(uint8_t const* vk_buf, uint8_t const* g2x)
{
    auto crs = std::make_shared<waffle::VerifierMemReferenceString>(g2x);
    waffle::verification_key_data vk_data;
    read(vk_buf, vk_data);
    init_verification_key(crs, std::move(vk_data));
}

WASM_EXPORT uint32_t composer__get_new_proving_key_data(uint8_t** output)
{
    // Copied from joint_split
    // Computing the size of the serialized key is non trivial. We know it's ~331mb.
    // Allocate a buffer large enough to hold it, and abort if we overflow.
    // This is to keep memory usage down.
    size_t total_buf_len = 350 * 1024 * 1024;
    auto raw_buf = (uint8_t*)malloc(total_buf_len);
    auto raw_buf_end = raw_buf;
    write(raw_buf_end, *get_proving_key());
    *output = raw_buf;
    auto len = static_cast<uint32_t>(raw_buf_end - raw_buf);
    if (len > total_buf_len) {
        info("Buffer overflow serializing proving key.");
        std::abort();
    }
    return len;
}

WASM_EXPORT uint32_t composer__get_circuit_size(uint8_t const* constraint_system_buf)
{
    auto constraint_system = from_buffer<standard_format>(constraint_system_buf);
    auto crs_factory = std::make_unique<waffle::ReferenceStringFactory>();
    auto composer = create_circuit(constraint_system, std::move(crs_factory));
    auto prover = composer.create_prover();
    auto circuit_size = prover.get_circuit_size();

    return static_cast<uint32_t>(circuit_size);
}

WASM_EXPORT uint32_t composer__compute_vk(void* pippenger,
                                          uint8_t const* g2x,
                                          uint8_t const* constraint_system_buf,
                                          uint8_t** output_buf)
{
    auto constraint_system = from_buffer<standard_format>(constraint_system_buf);
    auto crs_factory = std::make_unique<waffle::PippengerReferenceStringFactory>(
        reinterpret_cast<scalar_multiplication::Pippenger*>(pippenger), g2x);
    auto composer = create_circuit(constraint_system, std::move(crs_factory));

    auto verification_key = composer.compute_verification_key();

    auto buffer = to_buffer(*verification_key);

    *output_buf = buffer.data();
    return static_cast<uint32_t>(buffer.size());
}

WASM_EXPORT uint32_t composer__smart_contract(void* pippenger,
                                              uint8_t const* g2x,
                                              uint8_t const* constraint_system_buf,
                                              uint8_t** output_buf)
{
    auto constraint_system = from_buffer<standard_format>(constraint_system_buf);
    auto crs_factory = std::make_unique<waffle::PippengerReferenceStringFactory>(
        reinterpret_cast<scalar_multiplication::Pippenger*>(pippenger), g2x);
    auto composer = create_circuit(constraint_system, std::move(crs_factory));

    auto verification_key = composer.compute_verification_key();

    ostringstream stream;
    output_vk_sol_method(stream, verification_key);

    auto content_str = stream.str();
    std::vector<uint8_t> buffer(content_str.begin(), content_str.end());

    *output_buf = buffer.data();
    return static_cast<uint32_t>(buffer.size());
}

WASM_EXPORT void* composer__new_generic_prover(uint8_t const* witness_buf)
{
    auto witness = from_buffer<std::vector<fr>>(witness_buf);

    auto prover = new_generic_prover(witness);
    auto heapProver = new TurboProver(std::move(prover));

    return heapProver;
}
WASM_EXPORT void* composer__new_prover(void* pippenger,
                                       uint8_t const* g2x,
                                       uint8_t const* constraint_system_buf,
                                       uint8_t const* witness_buf)
{
    auto constraint_system = from_buffer<standard_format>(constraint_system_buf);

    auto t3 = Clock::now();
    auto crs_factory = std::make_unique<waffle::PippengerReferenceStringFactory>(
        reinterpret_cast<scalar_multiplication::Pippenger*>(pippenger), g2x);
    auto t4 = Clock::now();
    info(std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t3).count());

    auto witness = from_buffer<std::vector<fr>>(witness_buf);

    auto t5 = Clock::now();
    auto composer = create_circuit_with_witness(constraint_system, witness, std::move(crs_factory));
    auto t6 = Clock::now();
    logstr(format(std::chrono::duration_cast<std::chrono::nanoseconds>(t6 - t5).count()).c_str());

    auto prover = composer.create_prover();
    auto heapProver = new TurboProver(std::move(prover));

    return heapProver;
}

WASM_EXPORT size_t composer__new_proof(void* pippenger,
                                       uint8_t const* g2x,
                                       uint8_t const* constraint_system_buf,
                                       uint8_t const* witness_buf,
                                       uint8_t** proof_data_buf)
{

    auto t1 = Clock::now();
    auto constraint_system = from_buffer<standard_format>(constraint_system_buf);
    auto t2 = Clock::now();
    logstr(format("deserialising constraint system : ",
                  std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count(),
                  "ns",
                  " ~",
                  std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count(),
                  "seconds")
               .c_str());

    auto t3 = Clock::now();
    auto crs_factory = std::make_unique<waffle::PippengerReferenceStringFactory>(
        reinterpret_cast<scalar_multiplication::Pippenger*>(pippenger), g2x);
    auto t4 = Clock::now();
    logstr(format("creating crs factory : ",
                  std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t3).count(),
                  "ns",
                  " ~",
                  std::chrono::duration_cast<std::chrono::seconds>(t4 - t3).count(),
                  "seconds")
               .c_str());

    auto t5 = Clock::now();
    auto witness = from_buffer<std::vector<fr>>(witness_buf);
    auto composer = create_circuit_with_witness(constraint_system, witness, std::move(crs_factory));
    auto t6 = Clock::now();
    logstr(format("creating circuit with witness : ",
                  std::chrono::duration_cast<std::chrono::nanoseconds>(t6 - t5).count(),
                  "ns",
                  " ~",
                  std::chrono::duration_cast<std::chrono::seconds>(t6 - t5).count(),
                  "seconds")
               .c_str());

    auto t7 = Clock::now();
    aligned_free((void*)witness_buf);
    aligned_free((void*)g2x);
    aligned_free((void*)constraint_system_buf);
    auto t8 = Clock::now();
    logstr(format("aligned_free : ",
                  std::chrono::duration_cast<std::chrono::nanoseconds>(t8 - t7).count(),
                  "ns",
                  " ~",
                  std::chrono::duration_cast<std::chrono::seconds>(t8 - t7).count(),
                  "seconds")
               .c_str());

    auto t9 = Clock::now();
    auto prover = composer.create_prover();
    auto heapProver = new TurboProver(std::move(prover));
    auto t10 = Clock::now();
    logstr(format("creating prover : ",
                  std::chrono::duration_cast<std::chrono::nanoseconds>(t10 - t9).count(),
                  "ns",
                  " ~",
                  std::chrono::duration_cast<std::chrono::seconds>(t10 - t9).count(),
                  "seconds")
               .c_str());

    auto t11 = Clock::now();
    auto& proof_data = heapProver->construct_proof().proof_data;
    *proof_data_buf = proof_data.data();
    auto t12 = Clock::now();
    logstr(format("creating proof : ",
                  std::chrono::duration_cast<std::chrono::nanoseconds>(t12 - t11).count(),
                  "ns",
                  " ~",
                  std::chrono::duration_cast<std::chrono::seconds>(t12 - t11).count(),
                  "seconds")
               .c_str());

    logstr(format("total wasm time : ", std::chrono::duration_cast<std::chrono::seconds>(t12 - t1).count(), "seconds")
               .c_str());

    return proof_data.size();
}

WASM_EXPORT void composer__delete_prover(void* prover)
{
    delete reinterpret_cast<TurboProver*>(prover);
}

WASM_EXPORT bool composer__verify_proof(
    void* pippenger, uint8_t const* g2x, uint8_t const* constraint_system_buf, uint8_t* proof, uint32_t length)
{
    bool verified = false;
#ifndef __wasm__
    try {
#endif

        auto t1 = Clock::now();
        auto constraint_system = from_buffer<standard_format>(constraint_system_buf);
        auto crs_factory = std::make_unique<waffle::PippengerReferenceStringFactory>(
            reinterpret_cast<scalar_multiplication::Pippenger*>(pippenger), g2x);
        auto composer = create_circuit(constraint_system, std::move(crs_factory));
        auto t2 = Clock::now();

        logstr(format("verifier deserialise : ",
                      std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count(),
                      "ns",
                      " ~",
                      std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count(),
                      "seconds")
                   .c_str());

        waffle::plonk_proof pp = { std::vector<uint8_t>(proof, proof + length) };

        auto t3 = Clock::now();
        auto verifier = composer.create_verifier();
        auto t4 = Clock::now();

        logstr(format("creating verifier : ",
                      std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t3).count(),
                      "ns",
                      " ~",
                      std::chrono::duration_cast<std::chrono::seconds>(t4 - t3).count(),
                      "seconds")
                   .c_str());

        auto t5 = Clock::now();
        verified = verifier.verify_proof(pp);
        auto t6 = Clock::now();

        logstr(format("verifying proof : ",
                      std::chrono::duration_cast<std::chrono::nanoseconds>(t6 - t5).count(),
                      "ns",
                      " ~",
                      std::chrono::duration_cast<std::chrono::seconds>(t6 - t5).count(),
                      "seconds")
                   .c_str());
#ifndef __wasm__
    } catch (const std::exception& e) {
        verified = false;
        error(e.what());
    }
#endif
    return verified;
}

WASM_EXPORT bool composer__verify_proof_with_public_inputs(void* pippenger,
                                                           uint8_t const* g2x,
                                                           uint8_t const* constraint_system_buf,
                                                           uint8_t const* public_inputs_buf,
                                                           uint8_t* proof,
                                                           uint32_t length)
{
    bool verified = false;
#ifndef __wasm__
    try {
#endif
        auto t1 = Clock::now();
        auto constraint_system = from_buffer<standard_format>(constraint_system_buf);
        auto crs_factory = std::make_unique<waffle::PippengerReferenceStringFactory>(
            reinterpret_cast<scalar_multiplication::Pippenger*>(pippenger), g2x);
        auto composer = create_circuit(constraint_system, std::move(crs_factory));
        auto t2 = Clock::now();
        logstr(format("verifier deserialise : ",
                      std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count(),
                      "ns",
                      " ~",
                      std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count(),
                      "seconds")
                   .c_str());

        waffle::plonk_proof pp = { std::vector<uint8_t>(proof, proof + length) };

        auto t3 = Clock::now();
        // XXX: Seems we cannot pass the public inputs to the verifie
        // auto public_inputs = from_buffer<std::vector<fr>>(public_inputs_buf);

        auto verifier = composer.create_verifier();

        auto t4 = Clock::now();

        logstr(format("creating verifier : ",
                      std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t3).count(),
                      "ns",
                      " ~",
                      std::chrono::duration_cast<std::chrono::seconds>(t4 - t3).count(),
                      "seconds")
                   .c_str());

        auto t5 = Clock::now();
        verified = verifier.verify_proof(pp);
        auto t6 = Clock::now();

        logstr(format("verifying proof : ",
                      std::chrono::duration_cast<std::chrono::nanoseconds>(t6 - t5).count(),
                      "ns",
                      " ~",
                      std::chrono::duration_cast<std::chrono::seconds>(t6 - t5).count(),
                      "seconds")
                   .c_str());
#ifndef __wasm__
    } catch (const std::exception& e) {
        verified = false;
        error(e.what());
    }
#endif
    return verified;
}
}
