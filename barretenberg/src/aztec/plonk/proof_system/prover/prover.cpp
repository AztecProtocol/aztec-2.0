#include "prover.hpp"
#include "../public_inputs/public_inputs.hpp"
#include "../utils/linearizer.hpp"
#include <ecc/curves/bn254/scalar_multiplication/scalar_multiplication.hpp>
#include <polynomials/iterate_over_domain.hpp>
#include <polynomials/polynomial_arithmetic.hpp>

#include <common/profiler.hpp>

using namespace barretenberg;

namespace waffle {

template <typename settings>
ProverBase<settings>::ProverBase(std::shared_ptr<proving_key> input_key,
                                 std::shared_ptr<program_witness> input_witness,
                                 const transcript::Manifest& input_manifest)
    : n(input_key == nullptr ? 0 : input_key->n)
    , transcript(input_manifest, settings::hash_type, settings::num_challenge_bytes)
    , key(input_key)
    , witness(input_witness)
    , queue(key.get(), witness.get(), &transcript)
{
    if (witness->wires.count("z") == 0) {
        witness->wires.insert({ "z", polynomial(n, n) });
    }
}

template <typename settings>
ProverBase<settings>::ProverBase(ProverBase<settings>&& other)
    : n(other.n)
    , transcript(other.transcript)
    , key(std::move(other.key))
    , witness(std::move(other.witness))
    , queue(key.get(), witness.get(), &transcript)
{
    for (size_t i = 0; i < other.widgets.size(); ++i) {
        widgets.emplace_back(std::move(other.widgets[i]));
    }
}

template <typename settings> ProverBase<settings>& ProverBase<settings>::operator=(ProverBase<settings>&& other)
{
    n = other.n;

    widgets.resize(0);
    for (size_t i = 0; i < other.widgets.size(); ++i) {
        widgets.emplace_back(std::move(other.widgets[i]));
    }

    transcript = other.transcript;
    key = std::move(other.key);
    witness = std::move(other.witness);
    queue = work_queue(key.get(), witness.get(), &transcript);
    return *this;
}

template <typename settings> void ProverBase<settings>::compute_wire_pre_commitments()
{
    for (size_t i = 0; i < settings::program_width; ++i) {
        std::string wire_tag = "w_" + std::to_string(i + 1);
        queue.add_to_queue({
            work_queue::WorkType::SCALAR_MULTIPLICATION,
            witness->wires.at(wire_tag).get_coefficients(),
            "W_" + std::to_string(i + 1),
        });
    }

    // add public inputs
    const polynomial& public_wires_source = key->wire_ffts.at("w_2_fft");
    std::vector<fr> public_wires;
    for (size_t i = 0; i < key->num_public_inputs; ++i) {
        public_wires.push_back(public_wires_source[i]);
    }
    transcript.add_element("public_inputs", fr::to_buffer(public_wires));
}

template <typename settings> void ProverBase<settings>::compute_quotient_pre_commitment()
{
    for (size_t i = 0; i < settings::program_width; ++i) {
        const size_t offset = n * i;
        queue.add_to_queue({
            work_queue::WorkType::SCALAR_MULTIPLICATION,
            &key->quotient_large.get_coefficients()[offset],
            "T_" + std::to_string(i + 1),
        });
    }
}

template <typename settings> void ProverBase<settings>::execute_preamble_round()
{
    queue.flush_queue();
    transcript.add_element("circuit_size",
                           { static_cast<uint8_t>(n >> 24),
                             static_cast<uint8_t>(n >> 16),
                             static_cast<uint8_t>(n >> 8),
                             static_cast<uint8_t>(n) });
    transcript.add_element("public_input_size",
                           { static_cast<uint8_t>(key->num_public_inputs >> 24),
                             static_cast<uint8_t>(key->num_public_inputs >> 16),
                             static_cast<uint8_t>(key->num_public_inputs >> 8),
                             static_cast<uint8_t>(key->num_public_inputs) });
    transcript.apply_fiat_shamir("init");

    for (size_t i = 0; i < settings::program_width; ++i) {
        std::string wire_tag = "w_" + std::to_string(i + 1);
        barretenberg::polynomial& wire = witness->wires.at(wire_tag);
        barretenberg::polynomial& wire_fft = key->wire_ffts.at(wire_tag + "_fft");
        barretenberg::polynomial_arithmetic::copy_polynomial(&wire[0], &wire_fft[0], n, n);
        queue.add_to_queue({ work_queue::WorkType::IFFT, nullptr, wire_tag });
    }
}

template <typename settings> void ProverBase<settings>::execute_first_round()
{
    queue.flush_queue();
    compute_wire_pre_commitments();
    for (auto& widget : widgets) {
        widget->compute_round_commitments(transcript, 1, queue);
    }
}

template <typename settings> void ProverBase<settings>::execute_second_round()
{
    queue.flush_queue();
    transcript.apply_fiat_shamir("beta");
    profiler::start_measurement();
    for (auto& widget : widgets) {
        widget->compute_round_commitments(transcript, 2, queue);
    }
    profiler::end_measurement("compute 2nd round commitments");
    for (size_t i = 0; i < settings::program_width; ++i) {
        std::string wire_tag = "w_" + std::to_string(i + 1);
        queue.add_to_queue({ work_queue::WorkType::FFT, nullptr, wire_tag });
    }
}

template <typename settings> void ProverBase<settings>::execute_third_round()
{
    queue.flush_queue();
    transcript.apply_fiat_shamir("alpha");
    fr alpha_base = fr::serialize_from_buffer(transcript.get_challenge("alpha").begin());

    for (size_t i = 0; i < widgets.size(); ++i) {
        alpha_base = widgets[i]->compute_quotient_contribution(alpha_base, transcript);
    }

    fr* q_mid = &key->quotient_mid[0];
    fr* q_large = &key->quotient_large[0];

    if constexpr (settings::uses_quotient_mid) {
        barretenberg::polynomial_arithmetic::divide_by_pseudo_vanishing_polynomial(
            key->quotient_mid.get_coefficients(), key->small_domain, key->mid_domain);
    }
    barretenberg::polynomial_arithmetic::divide_by_pseudo_vanishing_polynomial(
        key->quotient_large.get_coefficients(), key->small_domain, key->large_domain);

    if (settings::uses_quotient_mid) {
        key->quotient_mid.coset_ifft(key->mid_domain);
    }
    // key->quotient_large.coset_ifft(key->large_domain);
    key->quotient_large.coset_ifft(key->small_domain, key->large_domain, 4);

    if (settings::uses_quotient_mid) {
        ITERATE_OVER_DOMAIN_START(key->mid_domain);
        q_large[i] += q_mid[i];
        ITERATE_OVER_DOMAIN_END;
    }
    compute_quotient_pre_commitment();
}

template <typename settings> void ProverBase<settings>::execute_fourth_round()
{
    queue.flush_queue();
    transcript.apply_fiat_shamir("z"); // end of 3rd round
    compute_linearisation_coefficients();
    profiler::end_measurement("compute linearisation coefficients");
}

template <typename settings> void ProverBase<settings>::execute_fifth_round()
{
    queue.flush_queue();
    transcript.apply_fiat_shamir("nu");
    std::vector<fr> nu_challenges;
    std::vector<fr> shifted_nu_challenges;

    if constexpr (settings::use_linearisation) {
        nu_challenges.emplace_back(fr::serialize_from_buffer(transcript.get_challenge_from_map("nu", "r").begin()));
    }
    for (size_t i = 0; i < settings::program_width; ++i) {
        if (settings::requires_shifted_wire(settings::wire_shift_settings, i)) {
            shifted_nu_challenges.emplace_back(fr::serialize_from_buffer(
                transcript.get_challenge_from_map("nu", "w_" + std::to_string(i + 1)).begin()));
        }
        nu_challenges.emplace_back(
            fr::serialize_from_buffer(transcript.get_challenge_from_map("nu", "w_" + std::to_string(i + 1)).begin()));
    }

    fr z_challenge = fr::serialize_from_buffer(transcript.get_challenge("z").begin());
    fr* r = key->linear_poly.get_coefficients();

    std::array<fr*, settings::program_width> wires;
    for (size_t i = 0; i < settings::program_width; ++i) {
        wires[i] = &witness->wires.at("w_" + std::to_string(i + 1))[0];
    }

    // Next step: compute the two Kate polynomial commitments, and associated opening proofs
    // We have two evaluation points: z and z.omega
    // We need to create random linear combinations of each individual polynomial and combine them

    polynomial& opening_poly = key->opening_poly;
    polynomial& shifted_opening_poly = key->shifted_opening_poly;

    std::array<fr, settings::program_width> z_powers;
    z_powers[0] = z_challenge;
    for (size_t i = 1; i < settings::program_width; ++i) {
        z_powers[i] = z_challenge.pow(static_cast<uint64_t>(n * i));
    }

    polynomial& quotient_large = key->quotient_large;

    ITERATE_OVER_DOMAIN_START(key->small_domain);

    fr T0;
    fr quotient_temp = fr::zero();
    if constexpr (settings::use_linearisation) {
        quotient_temp = r[i] * nu_challenges[0];
    }
    for (size_t k = 1; k < settings::program_width; ++k) {
        T0 = quotient_large[i + (k * n)] * z_powers[k];
        quotient_temp += T0;
    }
    for (size_t k = 0; k < settings::program_width; ++k) {
        T0 = wires[k][i] * nu_challenges[k + settings::use_linearisation];
        quotient_temp += T0;
    }
    shifted_opening_poly[i] = 0;

    opening_poly[i] = quotient_large[i] + quotient_temp;

    ITERATE_OVER_DOMAIN_END;

    if constexpr (settings::wire_shift_settings > 0) {
        ITERATE_OVER_DOMAIN_START(key->small_domain);
        size_t nu_ptr = 0;
        if constexpr (settings::requires_shifted_wire(settings::wire_shift_settings, 0)) {
            fr T0;
            T0 = shifted_nu_challenges[nu_ptr++] * wires[0][i];
            shifted_opening_poly[i] += T0;
        }
        if constexpr (settings::requires_shifted_wire(settings::wire_shift_settings, 1)) {
            fr T0;
            T0 = shifted_nu_challenges[nu_ptr++] * wires[1][i];
            shifted_opening_poly[i] += T0;
        }
        if constexpr (settings::requires_shifted_wire(settings::wire_shift_settings, 2)) {
            fr T0;
            T0 = shifted_nu_challenges[nu_ptr++] * wires[2][i];
            shifted_opening_poly[i] += T0;
        }
        if constexpr (settings::requires_shifted_wire(settings::wire_shift_settings, 3)) {
            fr T0;
            T0 = shifted_nu_challenges[nu_ptr++] * wires[3][i];
            shifted_opening_poly[i] += T0;
        }
        for (size_t k = 4; k < settings::program_width; ++k) {
            if (settings::requires_shifted_wire(settings::wire_shift_settings, k)) {
                fr T0;
                T0 = shifted_nu_challenges[nu_ptr++] * wires[k][i];
                shifted_opening_poly[i] += T0;
            }
        }
        ITERATE_OVER_DOMAIN_END;
    }

    // Currently code assumes permutation_widget is first.
    for (size_t i = 0; i < widgets.size(); ++i) {
        widgets[i]->compute_opening_poly_contribution(transcript, settings::use_linearisation);
    }

    fr shifted_z;
    shifted_z = z_challenge * key->small_domain.root;

    opening_poly.compute_kate_opening_coefficients(z_challenge);

    shifted_opening_poly.compute_kate_opening_coefficients(shifted_z);

    queue.add_to_queue({
        work_queue::WorkType::SCALAR_MULTIPLICATION,
        opening_poly.get_coefficients(),
        "PI_Z",
    });
    queue.add_to_queue({
        work_queue::WorkType::SCALAR_MULTIPLICATION,
        shifted_opening_poly.get_coefficients(),
        "PI_Z_OMEGA",
    });
}

template <typename settings> barretenberg::fr ProverBase<settings>::compute_linearisation_coefficients()
{

    fr z_challenge = fr::serialize_from_buffer(transcript.get_challenge("z").begin());
    fr shifted_z = z_challenge * key->small_domain.root;

    polynomial& r = key->linear_poly;
    // ok... now we need to evaluate polynomials. Jeepers

    // evaluate the prover and instance polynomials.
    // (we don't need to evaluate the quotient polynomial, that can be derived by the verifier)
    for (size_t i = 0; i < settings::program_width; ++i) {
        std::string wire_key = "w_" + std::to_string(i + 1);
        const polynomial& wire = witness->wires.at(wire_key);
        fr wire_eval;
        wire_eval = wire.evaluate(z_challenge, n);
        transcript.add_element(wire_key, wire_eval.to_buffer());

        if (settings::requires_shifted_wire(settings::wire_shift_settings, i)) {
            fr shifted_wire_eval;
            shifted_wire_eval = wire.evaluate(shifted_z, n);
            transcript.add_element(wire_key + "_omega", shifted_wire_eval.to_buffer());
        }
    }

    for (size_t i = 0; i < widgets.size(); ++i) {
        widgets[i]->compute_transcript_elements(transcript, settings::use_linearisation);
    }

    fr t_eval = key->quotient_large.evaluate(z_challenge, 4 * n);

    if constexpr (settings::use_linearisation) {
        fr alpha_base = fr::serialize_from_buffer(transcript.get_challenge("alpha").begin());
        for (size_t i = 0; i < widgets.size(); ++i) {
            alpha_base = widgets[i]->compute_linear_contribution(alpha_base, transcript, r);
        }

        fr linear_eval = r.evaluate(z_challenge, n);
        transcript.add_element("r", linear_eval.to_buffer());
    }
    transcript.add_element("t", t_eval.to_buffer());
    return t_eval;
}

template <typename settings> waffle::plonk_proof& ProverBase<settings>::export_proof()
{
    proof.proof_data = transcript.export_transcript();
    return proof;
}

template <typename settings> waffle::plonk_proof& ProverBase<settings>::construct_proof()
{
    profiler::start_measurement();
    execute_preamble_round();
    profiler::end_measurement("preamble round");

    profiler::start_measurement();
    queue.process_queue();
    profiler::end_measurement("preamble queue");

    profiler::start_measurement();
    execute_first_round();
    profiler::end_measurement("first round");

    profiler::start_measurement();
    queue.process_queue();
    profiler::end_measurement("first round queue");

    profiler::start_measurement();
    execute_second_round();
    profiler::end_measurement("second round");

    profiler::start_measurement();
    queue.process_queue();
    profiler::end_measurement("second round queue");

    profiler::start_measurement();
    execute_third_round();
    profiler::end_measurement("third round");

    profiler::start_measurement();
    queue.process_queue();
    profiler::end_measurement("third round queue");

    profiler::start_measurement();
    execute_fourth_round();
    profiler::end_measurement("fourth round");

    profiler::start_measurement();
    execute_fifth_round();
    profiler::end_measurement("fifth round");

    profiler::start_measurement();
    queue.process_queue();
    profiler::end_measurement("fifth round queue");

    return export_proof();
}

template <typename settings> void ProverBase<settings>::reset()
{
    transcript::Manifest manifest = transcript.get_manifest();
    transcript = transcript::StandardTranscript(manifest, settings::hash_type, settings::num_challenge_bytes);
}

template class ProverBase<unrolled_standard_settings>;
template class ProverBase<unrolled_turbo_settings>;
template class ProverBase<standard_settings>;
template class ProverBase<turbo_settings>;

} // namespace waffle
