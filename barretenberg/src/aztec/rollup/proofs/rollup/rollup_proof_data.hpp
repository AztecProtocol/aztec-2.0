#pragma once
#include <stdlib/types/turbo.hpp>

namespace rollup {
namespace proofs {
namespace rollup {

using namespace plonk::stdlib::types::turbo;

struct propagated_inner_proof_data {
    uint256_t proof_id;
    uint256_t public_input;
    uint256_t public_output;
    uint256_t asset_id;
    std::array<uint8_t, 64> new_note1;
    std::array<uint8_t, 64> new_note2;
    uint256_t nullifier1;
    uint256_t nullifier2;
    fr input_owner;
    fr output_owner;
};

struct rollup_proof_data {
    uint32_t rollup_id;
    uint32_t rollup_size;
    uint32_t data_start_index;
    fr old_data_root;
    fr new_data_root;
    fr old_null_root;
    fr new_null_root;
    fr old_data_roots_root;
    fr new_data_roots_root;
    std::vector<uint256_t> total_tx_fees;
    uint32_t num_txs;
    std::vector<propagated_inner_proof_data> inner_proofs;
    g1::affine_element recursion_output[2];

    rollup_proof_data(std::vector<uint8_t> const& proof_data);
};

} // namespace rollup
} // namespace proofs
} // namespace rollup
