#include "create_noop_join_split_proof.hpp"
#include "join_split_circuit.hpp"
#include <stdlib/merkle_tree/hash_path.hpp>
#include <stdlib/types/turbo.hpp>
#include <sys/stat.h>

namespace rollup {
namespace proofs {
namespace join_split {

using namespace barretenberg;
using namespace plonk::stdlib::types::turbo;
using namespace plonk::stdlib::merkle_tree;

std::vector<uint8_t> create_noop_join_split_proof(circuit_data const& circuit_data, fr const& merkle_root, bool valid)
{
    join_split_tx tx = noop_tx();
    tx.num_input_notes = valid ? 0 : 1;
    tx.old_data_root = merkle_root;

    Composer composer = Composer(circuit_data.proving_key, circuit_data.verification_key, circuit_data.num_gates);
    join_split_circuit(composer, tx);

    if (composer.failed) {
        error("join split logic failed: ", composer.err);
    }

    auto prover = composer.create_unrolled_prover();
    auto proof = prover.construct_proof();

    return proof.proof_data;
}

} // namespace join_split
} // namespace proofs
} // namespace rollup
