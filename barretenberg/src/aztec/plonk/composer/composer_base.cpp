#include "composer_base.hpp"
#include <plonk/proof_system/proving_key/proving_key.hpp>
#include <plonk/proof_system/utils/permutation.hpp>
#include <plonk/proof_system/verification_key/verification_key.hpp>

namespace waffle {

void ComposerBase::assert_equal(const uint32_t a_idx, const uint32_t b_idx)
{
    ASSERT((variables[a_idx] == variables[b_idx]));
    for (size_t i = 0; i < wire_copy_cycles[b_idx].size(); ++i) {
        wire_copy_cycles[a_idx].emplace_back(wire_copy_cycles[b_idx][i]);
        if (wire_copy_cycles[b_idx][i].wire_type == WireType::LEFT) {
            w_l[wire_copy_cycles[b_idx][i].gate_index] = a_idx;
        } else if (wire_copy_cycles[b_idx][i].wire_type == WireType::RIGHT) {
            w_r[wire_copy_cycles[b_idx][i].gate_index] = a_idx;
        } else if (wire_copy_cycles[b_idx][i].wire_type == WireType::OUTPUT) {
            w_o[wire_copy_cycles[b_idx][i].gate_index] = a_idx;
        } else if (wire_copy_cycles[b_idx][i].wire_type == WireType::FOURTH) {
            w_4[wire_copy_cycles[b_idx][i].gate_index] = a_idx;
        }
    }
    wire_copy_cycles[b_idx] = std::vector<cycle_node>();
}

template <size_t program_width> void ComposerBase::compute_sigma_permutations(proving_key* key)
{
    std::array<std::vector<uint32_t>, program_width> sigma_mappings;
    std::array<uint32_t, 4> wire_offsets{ 0U, 0x40000000, 0x80000000, 0xc0000000 };
    const uint32_t num_public_inputs = static_cast<uint32_t>(public_inputs.size());

    for (size_t i = 0; i < program_width; ++i) {
        sigma_mappings[i].reserve(key->n);
    }
    for (size_t i = 0; i < program_width; ++i) {
        for (size_t j = 0; j < key->n; ++j) {
            sigma_mappings[i].emplace_back(j + wire_offsets[i]);
        }
    }

    for (size_t i = 0; i < wire_copy_cycles.size(); ++i) {
        for (size_t j = 0; j < wire_copy_cycles[i].size(); ++j) {
            cycle_node current_cycle_node = wire_copy_cycles[i][j];
            size_t cycle_node_index = j == wire_copy_cycles[i].size() - 1 ? 0 : j + 1;
            cycle_node next_cycle_node = wire_copy_cycles[i][cycle_node_index];
            sigma_mappings[static_cast<uint32_t>(current_cycle_node.wire_type) >>
                           30U][current_cycle_node.gate_index + num_public_inputs] =
                next_cycle_node.gate_index + static_cast<uint32_t>(next_cycle_node.wire_type) + num_public_inputs;
        }
    }

    for (size_t i = 0; i < num_public_inputs; ++i) {
        sigma_mappings[0][i] = static_cast<uint32_t>(i + key->small_domain.size);
    }
    for (size_t i = 0; i < program_width; ++i) {
        std::string index = std::to_string(i + 1);
        barretenberg::polynomial sigma_polynomial(key->n);
        compute_permutation_lagrange_base_single<standard_settings>(
            sigma_polynomial, sigma_mappings[i], key->small_domain);

        barretenberg::polynomial sigma_polynomial_lagrange_base(sigma_polynomial);
        key->permutation_selectors_lagrange_base.insert(
            { "sigma_" + index, std::move(sigma_polynomial_lagrange_base) });
        sigma_polynomial.ifft(key->small_domain);
        barretenberg::polynomial sigma_fft(sigma_polynomial, key->large_domain.size);
        sigma_fft.coset_fft(key->large_domain);
        key->permutation_selectors.insert({ "sigma_" + index, std::move(sigma_polynomial) });
        key->permutation_selector_ffts.insert({ "sigma_" + index + "_fft", std::move(sigma_fft) });
    }
}

std::shared_ptr<proving_key> ComposerBase::compute_proving_key()
{
    const size_t total_num_gates = n + public_inputs.size();

    size_t log2_n = static_cast<size_t>(numeric::get_msb(total_num_gates + 1));
    if ((1UL << log2_n) != (total_num_gates + 1)) {
        ++log2_n;
    }
    size_t new_n = 1UL << log2_n;
    auto crs = crs_factory_->get_prover_crs(new_n);
    circuit_proving_key = std::make_shared<proving_key>(new_n, public_inputs.size(), crs);

    for (size_t i = 0; i < public_inputs.size(); ++i) {
        cycle_node left{ static_cast<uint32_t>(i - public_inputs.size()),
                         WireType::LEFT };
        cycle_node right{ static_cast<uint32_t>(i - public_inputs.size()), WireType::RIGHT };

        std::vector<cycle_node>& old_cycle = wire_copy_cycles[static_cast<size_t>(public_inputs[i])];

        std::vector<cycle_node> new_cycle;

        new_cycle.emplace_back(left);
        new_cycle.emplace_back(right);
        for (size_t i = 0; i < old_cycle.size(); ++i) {
            new_cycle.emplace_back(old_cycle[i]);
        }
        old_cycle = new_cycle;
    }

    for (size_t i = 0; i < selector_num; ++i) {

        std::vector<barretenberg::fr>& coeffs = selectors[i];
        ASSERT(n == coeffs.size());
        ASSERT(n == coeffs.size());
        for (size_t j = total_num_gates; j < new_n; ++j) {
            coeffs.emplace_back(fr::zero());
        }
        polynomial poly(new_n);

        for (size_t k = 0; k < public_inputs.size(); ++k) {
            poly[k] = fr::zero();
        }
        for (size_t k = public_inputs.size(); k < new_n; ++k) {
            poly[k] = coeffs[k - public_inputs.size()];
        }

        poly.ifft(circuit_proving_key->small_domain);
        polynomial poly_fft(poly, new_n * 4);


        if (use_mid_for_selectorfft[i]){

            poly_fft.coset_fft(circuit_proving_key->mid_domain);
        }
            
        else
            poly_fft.coset_fft(circuit_proving_key->large_domain);
        circuit_proving_key->constraint_selectors.insert({ selector_names[i], std::move(poly) });
        circuit_proving_key->constraint_selector_ffts.insert({ selector_names[i] + "_fft", std::move(poly_fft) });
    
    }


    return circuit_proving_key;
}


template <class program_settings>
std::shared_ptr<program_witness> ComposerBase::compute_witness_base()
{
    if (computed_witness) {
        return witness;
    }
    witness = std::make_shared<program_witness>();

    const size_t total_num_gates = n + public_inputs.size();
    size_t log2_n = static_cast<size_t>(numeric::get_msb(total_num_gates + 1));
    if ((1UL << log2_n) != (total_num_gates + 1)) {
        ++log2_n;
    }
    size_t new_n = 1UL << log2_n;
    for (size_t i = total_num_gates; i < new_n; ++i) {
        w_l.emplace_back(zero_idx);
        w_r.emplace_back(zero_idx);
        w_o.emplace_back(zero_idx);
    }
        if (program_settings::program_width>3){
    for (size_t i = total_num_gates; i < new_n; ++i) {
            w_4.emplace_back(zero_idx);
    }
        }
    polynomial poly_w_1 = polynomial(new_n);
    polynomial poly_w_2 = polynomial(new_n);
    polynomial poly_w_3 = polynomial(new_n);
    polynomial poly_w_4;
        
        if (program_settings::program_width>3)
            poly_w_4= polynomial(new_n);
    for (size_t i = 0; i < public_inputs.size(); ++i) {
        fr::__copy(variables[public_inputs[i]], poly_w_1[i]);
        fr::__copy(variables[public_inputs[i]], poly_w_2[i]);
        fr::__copy(fr::zero(), poly_w_3[i]);
        if (program_settings::program_width>3)
        fr::__copy(fr::zero(), poly_w_4[i]);
    }
    for (size_t i = public_inputs.size(); i < new_n; ++i) {
        fr::__copy(variables[w_l[i - public_inputs.size()]], poly_w_1.at(i));
        fr::__copy(variables[w_r[i - public_inputs.size()]], poly_w_2.at(i));
        fr::__copy(variables[w_o[i - public_inputs.size()]], poly_w_3.at(i));
        if (program_settings::program_width>3)
        fr::__copy(variables[w_4[i - public_inputs.size()]], poly_w_4.at(i));
    }
    witness->wires.insert({ "w_1", std::move(poly_w_1) });
    witness->wires.insert({ "w_2", std::move(poly_w_2) });
    witness->wires.insert({ "w_3", std::move(poly_w_3) });
        if (program_settings::program_width>3)
    witness->wires.insert({ "w_4", std::move(poly_w_4) });
    computed_witness = true;
    return witness;
}

template void ComposerBase::compute_sigma_permutations<3>(proving_key* key);
template void ComposerBase::compute_sigma_permutations<4>(proving_key* key);
template std::shared_ptr<program_witness> ComposerBase::compute_witness_base<standard_settings>();
template std::shared_ptr<program_witness> ComposerBase::compute_witness_base<turbo_settings>();

} // namespace waffle