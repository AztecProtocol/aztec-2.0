#include "standard_composer.hpp"
#include <ecc/curves/bn254/scalar_multiplication/scalar_multiplication.hpp>
#include <numeric/bitop/get_msb.hpp>
#include <plonk/proof_system/widgets/arithmetic_widget.hpp>
#include <plonk/proof_system/widgets/permutation_widget.hpp>
#include <plonk/composer/standard/compute_verification_key.hpp>

using namespace barretenberg;

namespace waffle {
void StandardComposer::create_add_gate(const add_triple& in)
{
    gate_flags.push_back(0);
    w_l.emplace_back(in.a);
    w_r.emplace_back(in.b);
    w_o.emplace_back(in.c);
    q_m.emplace_back(fr::zero());
    q_1.emplace_back(in.a_scaling);
    q_2.emplace_back(in.b_scaling);
    q_3.emplace_back(in.c_scaling);
    q_c.emplace_back(in.const_scaling);

    cycle_node left{ static_cast<uint32_t>(n), WireType::LEFT };
    cycle_node right{ static_cast<uint32_t>(n), WireType::RIGHT };
    cycle_node out{ static_cast<uint32_t>(n), WireType::OUTPUT };
    ASSERT(wire_copy_cycles.size() > in.a);
    ASSERT(wire_copy_cycles.size() > in.b);
    ASSERT(wire_copy_cycles.size() > in.c);
    wire_copy_cycles[static_cast<size_t>(in.a)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(in.b)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(in.c)].emplace_back(out);
    ++n;
}

void StandardComposer::create_big_add_gate(const add_quad& in)
{

    // (a terms + b terms = temp)
    // (c terms + d  terms + temp = 0 )
    fr t0 = variables[in.a] * in.a_scaling;
    fr t1 = variables[in.b] * in.b_scaling;
    fr temp = t0 + t1;
    uint32_t temp_idx = add_variable(temp);

    create_add_gate(add_triple{ in.a, in.b, temp_idx, in.a_scaling, in.b_scaling, fr::neg_one(), fr::zero() });

    create_add_gate(add_triple{ in.c, in.d, temp_idx, in.c_scaling, in.d_scaling, fr::one(), in.const_scaling });
}

void StandardComposer::create_balanced_add_gate(const add_quad& in)
{

    // (a terms + b terms = temp)
    // (c terms + d  terms + temp = 0 )
    fr t0 = variables[in.a] * in.a_scaling;
    fr t1 = variables[in.b] * in.b_scaling;
    fr temp = t0 + t1;
    uint32_t temp_idx = add_variable(temp);

    w_l.emplace_back(in.a);
    w_r.emplace_back(in.b);
    w_o.emplace_back(temp_idx);
    q_m.emplace_back(fr::zero());
    q_1.emplace_back(in.a_scaling);
    q_2.emplace_back(in.b_scaling);
    q_3.emplace_back(fr::neg_one());
    q_c.emplace_back(fr::zero());

    cycle_node left{ static_cast<uint32_t>(n), WireType::LEFT };
    cycle_node right{ static_cast<uint32_t>(n), WireType::RIGHT };
    cycle_node out{ static_cast<uint32_t>(n), WireType::OUTPUT };
    wire_copy_cycles[static_cast<size_t>(in.a)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(in.b)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(temp_idx)].emplace_back(out);

    ++n;

    w_l.emplace_back(temp_idx);
    w_r.emplace_back(in.c);
    w_o.emplace_back(in.d);
    q_m.emplace_back(fr::zero());
    q_1.emplace_back(fr::one());
    q_2.emplace_back(in.c_scaling);
    q_3.emplace_back(in.d_scaling);
    q_c.emplace_back(in.const_scaling);

    left = { static_cast<uint32_t>(n), WireType::LEFT };
    right = { static_cast<uint32_t>(n), WireType::RIGHT };
    out = { static_cast<uint32_t>(n), WireType::OUTPUT };
    wire_copy_cycles[static_cast<size_t>(temp_idx)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(in.c)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(in.d)].emplace_back(out);

    ++n;

    // in.d must be between 0 and 3
    // i.e. in.d * (in.d - 1) * (in.d - 2) = 0
    fr temp_2 = variables[in.d].sqr() - variables[in.d];
    uint32_t temp_2_idx = add_variable(temp_2);
    w_l.emplace_back(in.d);
    w_r.emplace_back(in.d);
    w_o.emplace_back(temp_2_idx);
    q_m.emplace_back(fr::one());
    q_1.emplace_back(fr::neg_one());
    q_2.emplace_back(fr::zero());
    q_3.emplace_back(fr::neg_one());
    q_c.emplace_back(fr::zero());

    left = { static_cast<uint32_t>(n), WireType::LEFT };
    right = { static_cast<uint32_t>(n), WireType::RIGHT };
    out = { static_cast<uint32_t>(n), WireType::OUTPUT };
    wire_copy_cycles[static_cast<size_t>(in.d)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(in.d)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(temp_2_idx)].emplace_back(out);

    ++n;

    constexpr fr neg_two = -fr(2);
    w_l.emplace_back(temp_2_idx);
    w_r.emplace_back(in.d);
    w_o.emplace_back(zero_idx);
    q_m.emplace_back(fr::one());
    q_1.emplace_back(neg_two);
    q_2.emplace_back(fr::zero());
    q_3.emplace_back(fr::zero());
    q_c.emplace_back(fr::zero());

    left = { static_cast<uint32_t>(n), WireType::LEFT };
    right = { static_cast<uint32_t>(n), WireType::RIGHT };
    wire_copy_cycles[static_cast<size_t>(temp_2_idx)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(in.d)].emplace_back(right);

    ++n;
}

void StandardComposer::create_big_add_gate_with_bit_extraction(const add_quad& in)
{
    // blah.
    // delta = (c - 4d)
    // delta^2 = c^2 + 16d^2 - 8dc
    // r = (-2*delta*delta + 9*delta - 7)*delta
    // r =

    fr delta = variables[in.d];
    delta += delta;
    delta += delta;
    delta = variables[in.c] - delta;

    uint32_t delta_idx = add_variable(delta);
    constexpr fr neg_four = -(fr(4));
    create_add_gate(add_triple{ in.c, in.d, delta_idx, fr::one(), neg_four, fr::neg_one(), fr::zero() });

    constexpr fr two = fr(2);
    constexpr fr seven = fr(7);
    constexpr fr nine = fr(9);
    const fr r_0 = (delta * nine) - ((delta.sqr() * two) + seven);
    uint32_t r_0_idx = add_variable(r_0);
    create_poly_gate(poly_triple{ delta_idx, delta_idx, r_0_idx, -two, nine, fr::zero(), fr::neg_one(), -seven });

    fr r_1 = r_0 * delta;
    uint32_t r_1_idx = add_variable(r_1);
    create_mul_gate(mul_triple{
        r_0_idx,
        delta_idx,
        r_1_idx,
        fr::one(),
        fr::neg_one(),
        fr::zero(),
    });

    // ain.a1 + bin.b2 + cin.c3 + din.c4 + r_1 = 0

    fr r_2 = (r_1 + (variables[in.d] * in.d_scaling));
    uint32_t r_2_idx = add_variable(r_2);
    create_add_gate(add_triple{ in.d, r_1_idx, r_2_idx, in.d_scaling, fr::one(), fr::neg_one(), fr::zero() });

    create_big_add_gate(
        add_quad{ in.a, in.b, in.c, r_2_idx, in.a_scaling, in.b_scaling, in.c_scaling, fr::one(), in.const_scaling });
}

void StandardComposer::create_big_mul_gate(const mul_quad& in)
{
    fr temp = ((variables[in.c] * in.c_scaling) + (variables[in.d] * in.d_scaling));
    uint32_t temp_idx = add_variable(temp);
    create_add_gate(add_triple{ in.c, in.d, temp_idx, in.c_scaling, in.d_scaling, fr::neg_one(), fr::zero() });

    create_poly_gate(
        poly_triple{ in.a, in.b, temp_idx, in.mul_scaling, in.a_scaling, in.b_scaling, fr::one(), in.const_scaling });
}

void StandardComposer::create_mul_gate(const mul_triple& in)
{
    gate_flags.push_back(0);
    add_gate_flag(gate_flags.size() - 1, GateFlags::FIXED_LEFT_WIRE);
    add_gate_flag(gate_flags.size() - 1, GateFlags::FIXED_RIGHT_WIRE);
    w_l.emplace_back(in.a);
    w_r.emplace_back(in.b);
    w_o.emplace_back(in.c);
    q_m.emplace_back(in.mul_scaling);
    q_1.emplace_back(fr::zero());
    q_2.emplace_back(fr::zero());
    q_3.emplace_back(in.c_scaling);
    q_c.emplace_back(in.const_scaling);

    cycle_node left{ static_cast<uint32_t>(n), WireType::LEFT };
    cycle_node right{ static_cast<uint32_t>(n), WireType::RIGHT };
    cycle_node out{ static_cast<uint32_t>(n), WireType::OUTPUT };
    ASSERT(wire_copy_cycles.size() > in.a);
    ASSERT(wire_copy_cycles.size() > in.b);
    ASSERT(wire_copy_cycles.size() > in.c);
    wire_copy_cycles[static_cast<size_t>(in.a)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(in.b)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(in.c)].emplace_back(out);
    ++n;
}

void StandardComposer::create_bool_gate(const uint32_t variable_index)
{
    gate_flags.push_back(0);
    add_gate_flag(gate_flags.size() - 1, GateFlags::FIXED_LEFT_WIRE);
    add_gate_flag(gate_flags.size() - 1, GateFlags::FIXED_RIGHT_WIRE);
    w_l.emplace_back(variable_index);
    w_r.emplace_back(variable_index);
    w_o.emplace_back(variable_index);

    q_m.emplace_back(fr::one());
    q_1.emplace_back(fr::zero());
    q_2.emplace_back(fr::zero());
    q_3.emplace_back(fr::neg_one());
    q_c.emplace_back(fr::zero());

    cycle_node left{ static_cast<uint32_t>(n), WireType::LEFT };
    cycle_node right{ static_cast<uint32_t>(n), WireType::RIGHT };
    cycle_node out{ static_cast<uint32_t>(n), WireType::OUTPUT };
    ASSERT(wire_copy_cycles.size() > variable_index);
    wire_copy_cycles[static_cast<size_t>(variable_index)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(variable_index)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(variable_index)].emplace_back(out);
    ++n;
}

void StandardComposer::create_poly_gate(const poly_triple& in)
{
    gate_flags.push_back(0);
    add_gate_flag(gate_flags.size() - 1, GateFlags::FIXED_LEFT_WIRE);
    add_gate_flag(gate_flags.size() - 1, GateFlags::FIXED_RIGHT_WIRE);
    w_l.emplace_back(in.a);
    w_r.emplace_back(in.b);
    w_o.emplace_back(in.c);
    q_m.emplace_back(in.q_m);
    q_1.emplace_back(in.q_l);
    q_2.emplace_back(in.q_r);
    q_3.emplace_back(in.q_o);
    q_c.emplace_back(in.q_c);

    cycle_node left{ static_cast<uint32_t>(n), WireType::LEFT };
    cycle_node right{ static_cast<uint32_t>(n), WireType::RIGHT };
    cycle_node out{ static_cast<uint32_t>(n), WireType::OUTPUT };
    ASSERT(wire_copy_cycles.size() > in.a);
    ASSERT(wire_copy_cycles.size() > in.b);
    ASSERT(wire_copy_cycles.size() > in.c);
    wire_copy_cycles[static_cast<size_t>(in.a)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(in.b)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(in.c)].emplace_back(out);
    ++n;
}

std::vector<uint32_t> StandardComposer::create_range_constraint(const uint32_t witness_index, const size_t num_bits)
{
    fr target = variables[witness_index].from_montgomery_form();

    std::vector<uint32_t> accumulators;

    constexpr fr four = fr{ 4, 0, 0, 0 }.to_montgomery_form();
    fr accumulator = fr::zero();
    uint32_t accumulator_idx = 0;
    for (size_t i = num_bits - 1; i < num_bits; i -= 2) {
        bool hi = target.get_bit(i);
        bool lo = target.get_bit(i - 1);

        uint32_t hi_idx = add_variable(hi ? fr::one() : fr::zero());
        uint32_t lo_idx = add_variable(lo ? fr::one() : fr::zero());
        create_bool_gate(hi_idx);
        create_bool_gate(lo_idx);

        uint64_t quad = (lo ? 1U : 0U) + (hi ? 2U : 0U);
        uint32_t quad_idx = add_variable(fr{ quad, 0, 0, 0 }.to_montgomery_form());

        create_add_gate(
            add_triple{ lo_idx, hi_idx, quad_idx, fr::one(), fr::one() + fr::one(), fr::neg_one(), fr::zero() });

        if (i == num_bits - 1) {
            accumulators.push_back(quad_idx);
            accumulator = variables[quad_idx];
            accumulator_idx = quad_idx;
        } else {
            fr new_accumulator = accumulator + accumulator;
            new_accumulator = new_accumulator + new_accumulator;
            new_accumulator = new_accumulator + variables[quad_idx];
            uint32_t new_accumulator_idx = add_variable(new_accumulator);
            create_add_gate(add_triple{
                accumulator_idx, quad_idx, new_accumulator_idx, four, fr::one(), fr::neg_one(), fr::zero() });
            accumulators.push_back(new_accumulator_idx);
            accumulator = new_accumulator;
            accumulator_idx = new_accumulator_idx;
        }
    }
    return accumulators;
}

waffle::accumulator_triple StandardComposer::create_logic_constraint(const uint32_t a,
                                                                     const uint32_t b,
                                                                     const size_t num_bits,
                                                                     const bool is_xor_gate)
{
    waffle::accumulator_triple accumulators;

    const fr left_witness_value = variables[a].from_montgomery_form();
    const fr right_witness_value = variables[b].from_montgomery_form();

    fr left_accumulator = fr::zero();
    fr right_accumulator = fr::zero();
    fr out_accumulator = fr::zero();

    uint32_t left_accumulator_idx = zero_idx;
    uint32_t right_accumulator_idx = zero_idx;
    uint32_t out_accumulator_idx = zero_idx;
    constexpr fr four = fr(4);
    constexpr fr neg_two = -fr(2);
    for (size_t i = num_bits - 1; i < num_bits; i -= 2) {
        bool left_hi_val = left_witness_value.get_bit(i);
        bool left_lo_val = left_witness_value.get_bit(i - 1);
        bool right_hi_val = right_witness_value.get_bit((i));
        bool right_lo_val = right_witness_value.get_bit(i - 1);

        uint32_t left_hi_idx = add_variable(left_hi_val ? fr::one() : fr::zero());
        uint32_t left_lo_idx = add_variable(left_lo_val ? fr::one() : fr::zero());
        uint32_t right_hi_idx = add_variable(right_hi_val ? fr::one() : fr::zero());
        uint32_t right_lo_idx = add_variable(right_lo_val ? fr::one() : fr::zero());

        bool out_hi_val = is_xor_gate ? left_hi_val ^ right_hi_val : left_hi_val & right_hi_val;
        bool out_lo_val = is_xor_gate ? left_lo_val ^ right_lo_val : left_lo_val & right_lo_val;

        uint32_t out_hi_idx = add_variable(out_hi_val ? fr::one() : fr::zero());
        uint32_t out_lo_idx = add_variable(out_lo_val ? fr::one() : fr::zero());

        create_bool_gate(left_hi_idx);
        create_bool_gate(right_hi_idx);
        create_bool_gate(out_hi_idx);

        create_bool_gate(left_lo_idx);
        create_bool_gate(right_lo_idx);
        create_bool_gate(out_lo_idx);

        // a & b = ab
        // a ^ b = a + b - ab
        create_poly_gate(poly_triple{ left_hi_idx,
                                      right_hi_idx,
                                      out_hi_idx,
                                      is_xor_gate ? neg_two : fr::one(),
                                      is_xor_gate ? fr::one() : fr::zero(),
                                      is_xor_gate ? fr::one() : fr::zero(),
                                      fr::neg_one(),
                                      fr::zero() });

        create_poly_gate(poly_triple{ left_lo_idx,
                                      right_lo_idx,
                                      out_lo_idx,
                                      is_xor_gate ? neg_two : fr::one(),
                                      is_xor_gate ? fr::one() : fr::zero(),
                                      is_xor_gate ? fr::one() : fr::zero(),
                                      fr::neg_one(),
                                      fr::zero() });

        fr left_quad = variables[left_lo_idx] + variables[left_hi_idx] + variables[left_hi_idx];
        fr right_quad = variables[right_lo_idx] + variables[right_hi_idx] + variables[right_hi_idx];
        fr out_quad = variables[out_lo_idx] + variables[out_hi_idx] + variables[out_hi_idx];

        uint32_t left_quad_idx = add_variable(left_quad);
        uint32_t right_quad_idx = add_variable(right_quad);
        uint32_t out_quad_idx = add_variable(out_quad);

        fr new_left_accumulator = left_accumulator + left_accumulator;
        new_left_accumulator = new_left_accumulator + new_left_accumulator;
        new_left_accumulator = new_left_accumulator + left_quad;
        uint32_t new_left_accumulator_idx = add_variable(new_left_accumulator);

        create_add_gate(add_triple{ left_accumulator_idx,
                                    left_quad_idx,
                                    new_left_accumulator_idx,
                                    four,
                                    fr::one(),
                                    fr::neg_one(),
                                    fr::zero() });

        fr new_right_accumulator = right_accumulator + right_accumulator;
        new_right_accumulator = new_right_accumulator + new_right_accumulator;
        new_right_accumulator = new_right_accumulator + right_quad;
        uint32_t new_right_accumulator_idx = add_variable(new_right_accumulator);

        create_add_gate(add_triple{ right_accumulator_idx,
                                    right_quad_idx,
                                    new_right_accumulator_idx,
                                    four,
                                    fr::one(),
                                    fr::neg_one(),
                                    fr::zero() });

        fr new_out_accumulator = out_accumulator + out_accumulator;
        new_out_accumulator = new_out_accumulator + new_out_accumulator;
        new_out_accumulator = new_out_accumulator + out_quad;
        uint32_t new_out_accumulator_idx = add_variable(new_out_accumulator);

        create_add_gate(add_triple{
            out_accumulator_idx, out_quad_idx, new_out_accumulator_idx, four, fr::one(), fr::neg_one(), fr::zero() });

        accumulators.left.emplace_back(new_left_accumulator_idx);
        accumulators.right.emplace_back(new_right_accumulator_idx);
        accumulators.out.emplace_back(new_out_accumulator_idx);

        left_accumulator = new_left_accumulator;
        left_accumulator_idx = new_left_accumulator_idx;

        right_accumulator = new_right_accumulator;
        right_accumulator_idx = new_right_accumulator_idx;

        out_accumulator = new_out_accumulator;
        out_accumulator_idx = new_out_accumulator_idx;
    }
    return accumulators;
}

void StandardComposer::fix_witness(const uint32_t witness_index, const barretenberg::fr& witness_value)
{
    gate_flags.push_back(0);

    w_l.emplace_back(witness_index);
    w_r.emplace_back(zero_idx);
    w_o.emplace_back(zero_idx);
    q_m.emplace_back(fr::zero());
    q_1.emplace_back(fr::one());
    q_2.emplace_back(fr::zero());
    q_3.emplace_back(fr::zero());
    q_c.emplace_back(-witness_value);

    cycle_node left{ static_cast<uint32_t>(n), WireType::LEFT };

    ASSERT(wire_copy_cycles.size() > witness_index);
    ASSERT(wire_copy_cycles.size() > zero_idx);
    ASSERT(wire_copy_cycles.size() > zero_idx);
    wire_copy_cycles[static_cast<size_t>(witness_index)].emplace_back(left);

    ++n;
}

uint32_t StandardComposer::put_constant_variable(const barretenberg::fr& variable)
{
    if (constant_variables.count(variable) == 1) {
        return constant_variables.at(variable);
    } else {
        uint32_t variable_index = add_variable(variable);
        fix_witness(variable_index, variable);
        constant_variables.insert({ variable, variable_index });
        return variable_index;
    }
}

waffle::accumulator_triple StandardComposer::create_and_constraint(const uint32_t a,
                                                                   const uint32_t b,
                                                                   const size_t num_bits)
{
    return create_logic_constraint(a, b, num_bits, false);
}

waffle::accumulator_triple StandardComposer::create_xor_constraint(const uint32_t a,
                                                                   const uint32_t b,
                                                                   const size_t num_bits)
{
    return create_logic_constraint(a, b, num_bits, true);
}

void StandardComposer::create_dummy_gates()
{
    gate_flags.push_back(0);
    // add in a dummy gate to ensure that all of our polynomials are not zero and not identical
    constexpr fr one = fr(1);
    constexpr fr two = fr(2);
    constexpr fr three = fr(3);
    constexpr fr four = fr(4);
    constexpr fr five = fr(5);
    constexpr fr six = fr(6);
    constexpr fr seven = fr(7);
    constexpr fr minus_twenty = -(fr(20));

    q_m.emplace_back(one);
    q_1.emplace_back(two);
    q_2.emplace_back(three);
    q_3.emplace_back(four);
    q_c.emplace_back(five);

    uint32_t a_idx = add_variable(six);
    uint32_t b_idx = add_variable(seven);
    uint32_t c_idx = add_variable(minus_twenty);

    w_l.emplace_back(a_idx);
    w_r.emplace_back(b_idx);
    w_o.emplace_back(c_idx);

    cycle_node left{ static_cast<uint32_t>(n), WireType::LEFT };
    cycle_node right{ static_cast<uint32_t>(n), WireType::RIGHT };
    cycle_node out{ static_cast<uint32_t>(n), WireType::OUTPUT };
    ASSERT(wire_copy_cycles.size() > a_idx);
    ASSERT(wire_copy_cycles.size() > b_idx);
    ASSERT(wire_copy_cycles.size() > c_idx);
    wire_copy_cycles[static_cast<size_t>(a_idx)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(b_idx)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(c_idx)].emplace_back(out);
    ++n;

    // add a second dummy gate the ensure our permutation polynomials are also
    // distinct from the identity permutation
    q_m.emplace_back(one);
    q_1.emplace_back(one);
    q_2.emplace_back(one);
    q_3.emplace_back(one);
    q_c.emplace_back(fr{ 127, 0, 0, 0 }.to_montgomery_form());

    w_l.emplace_back(c_idx);
    w_r.emplace_back(a_idx);
    w_o.emplace_back(b_idx);

    left = { static_cast<uint32_t>(n), WireType::LEFT };
    right = { static_cast<uint32_t>(n), WireType::RIGHT };
    out = { static_cast<uint32_t>(n), WireType::OUTPUT };
    ASSERT(wire_copy_cycles.size() > c_idx);
    ASSERT(wire_copy_cycles.size() > a_idx);
    ASSERT(wire_copy_cycles.size() > b_idx);
    wire_copy_cycles[static_cast<size_t>(c_idx)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(a_idx)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(b_idx)].emplace_back(out);
    ++n;
}

std::shared_ptr<proving_key> StandardComposer::compute_proving_key()
{
    if (circuit_proving_key) {
        return circuit_proving_key;
    }
    ASSERT(wire_copy_cycles.size() == variables.size());
    ASSERT(n == q_m.size());
    ASSERT(n == q_1.size());
    ASSERT(n == q_2.size());
    ASSERT(n == q_3.size());

    const size_t total_num_gates = n + public_inputs.size();
    size_t log2_n = static_cast<size_t>(numeric::get_msb(total_num_gates + 1));
    if ((1UL << log2_n) != (total_num_gates + 1)) {
        ++log2_n;
    }
    size_t new_n = 1UL << log2_n;
    for (size_t i = total_num_gates; i < new_n; ++i) {
        q_m.emplace_back(fr::zero());
        q_1.emplace_back(fr::zero());
        q_2.emplace_back(fr::zero());
        q_3.emplace_back(fr::zero());
        q_c.emplace_back(fr::zero());
    }

    for (size_t i = 0; i < public_inputs.size(); ++i) {
        cycle_node left{ static_cast<uint32_t>(i - public_inputs.size()), WireType::LEFT };
        cycle_node right{ static_cast<uint32_t>(i - public_inputs.size()), WireType::RIGHT };

        std::vector<cycle_node>& old_cycle_nodes = wire_copy_cycles[static_cast<size_t>(public_inputs[i])];

        std::vector<cycle_node> new_cycle_nodes;

        new_cycle_nodes.emplace_back(left);
        new_cycle_nodes.emplace_back(right);
        for (size_t i = 0; i < old_cycle_nodes.size(); ++i) {
            new_cycle_nodes.emplace_back(old_cycle_nodes[i]);
        }
        old_cycle_nodes = new_cycle_nodes;
    }
    auto crs = crs_factory_->get_prover_crs(new_n);
    circuit_proving_key = std::make_shared<proving_key>(new_n, public_inputs.size(), crs);
    polynomial poly_q_m(new_n);
    polynomial poly_q_c(new_n);
    polynomial poly_q_1(new_n);
    polynomial poly_q_2(new_n);
    polynomial poly_q_3(new_n);

    for (size_t i = 0; i < public_inputs.size(); ++i) {
        poly_q_m[i] = fr::zero();
        poly_q_1[i] = fr::one();
        poly_q_2[i] = fr::zero();
        poly_q_3[i] = fr::zero();
        poly_q_c[i] = fr::zero();
    }
    for (size_t i = public_inputs.size(); i < new_n; ++i) {
        poly_q_m[i] = q_m[i - public_inputs.size()];
        poly_q_1[i] = q_1[i - public_inputs.size()];
        poly_q_2[i] = q_2[i - public_inputs.size()];
        poly_q_3[i] = q_3[i - public_inputs.size()];
        poly_q_c[i] = q_c[i - public_inputs.size()];
    }

    poly_q_1.ifft(circuit_proving_key->small_domain);
    poly_q_2.ifft(circuit_proving_key->small_domain);
    poly_q_3.ifft(circuit_proving_key->small_domain);
    poly_q_m.ifft(circuit_proving_key->small_domain);
    poly_q_c.ifft(circuit_proving_key->small_domain);

    polynomial poly_q_1_fft(poly_q_1, new_n * 2);
    polynomial poly_q_2_fft(poly_q_2, new_n * 2);
    polynomial poly_q_3_fft(poly_q_3, new_n * 2);
    polynomial poly_q_m_fft(poly_q_m, new_n * 2);
    polynomial poly_q_c_fft(poly_q_c, new_n * 2);

    poly_q_1_fft.coset_fft(circuit_proving_key->mid_domain);
    poly_q_2_fft.coset_fft(circuit_proving_key->mid_domain);
    poly_q_3_fft.coset_fft(circuit_proving_key->mid_domain);
    poly_q_m_fft.coset_fft(circuit_proving_key->mid_domain);
    poly_q_c_fft.coset_fft(circuit_proving_key->mid_domain);

    circuit_proving_key->constraint_selectors.insert({ "q_m", std::move(poly_q_m) });
    circuit_proving_key->constraint_selectors.insert({ "q_c", std::move(poly_q_c) });
    circuit_proving_key->constraint_selectors.insert({ "q_1", std::move(poly_q_1) });
    circuit_proving_key->constraint_selectors.insert({ "q_2", std::move(poly_q_2) });
    circuit_proving_key->constraint_selectors.insert({ "q_3", std::move(poly_q_3) });

    circuit_proving_key->constraint_selector_ffts.insert({ "q_m_fft", std::move(poly_q_m_fft) });
    circuit_proving_key->constraint_selector_ffts.insert({ "q_c_fft", std::move(poly_q_c_fft) });
    circuit_proving_key->constraint_selector_ffts.insert({ "q_1_fft", std::move(poly_q_1_fft) });
    circuit_proving_key->constraint_selector_ffts.insert({ "q_2_fft", std::move(poly_q_2_fft) });
    circuit_proving_key->constraint_selector_ffts.insert({ "q_3_fft", std::move(poly_q_3_fft) });

    compute_sigma_permutations<3>(circuit_proving_key.get());
    return circuit_proving_key;
}

std::shared_ptr<verification_key> StandardComposer::compute_verification_key()
{
    if (circuit_verification_key) {
        return circuit_verification_key;
    }
    if (!circuit_proving_key) {
        compute_proving_key();
    }

    circuit_verification_key =
        waffle::standard_composer::compute_verification_key(circuit_proving_key, crs_factory_->get_verifier_crs());

    return circuit_verification_key;
}

std::shared_ptr<program_witness> StandardComposer::compute_witness()
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
    polynomial poly_w_1 = polynomial(new_n);
    polynomial poly_w_2 = polynomial(new_n);
    polynomial poly_w_3 = polynomial(new_n);
    for (size_t i = 0; i < public_inputs.size(); ++i) {
        fr::__copy(fr::zero(), poly_w_1[i]);
        fr::__copy(variables[public_inputs[i]], poly_w_2[i]);
        fr::__copy(fr::zero(), poly_w_3[i]);
    }
    for (size_t i = public_inputs.size(); i < new_n; ++i) {
        fr::__copy(variables[w_l[i - public_inputs.size()]], poly_w_1.at(i));
        fr::__copy(variables[w_r[i - public_inputs.size()]], poly_w_2.at(i));
        fr::__copy(variables[w_o[i - public_inputs.size()]], poly_w_3.at(i));
    }
    witness->wires.insert({ "w_1", std::move(poly_w_1) });
    witness->wires.insert({ "w_2", std::move(poly_w_2) });
    witness->wires.insert({ "w_3", std::move(poly_w_3) });
    computed_witness = true;
    return witness;
}

Verifier StandardComposer::create_verifier()
{
    compute_verification_key();
    Verifier output_state(circuit_verification_key, create_manifest(public_inputs.size()));
    return output_state;
}

UnrolledVerifier StandardComposer::create_unrolled_verifier()
{
    compute_verification_key();
    UnrolledVerifier output_state(circuit_verification_key, create_unrolled_manifest(public_inputs.size()));
    return output_state;
}

UnrolledProver StandardComposer::create_unrolled_prover()
{
    compute_proving_key();
    compute_witness();
    UnrolledProver output_state(circuit_proving_key, witness, create_unrolled_manifest(public_inputs.size()));

    std::unique_ptr<ProverPermutationWidget<3>> permutation_widget =
        std::make_unique<ProverPermutationWidget<3>>(circuit_proving_key.get(), witness.get());
    std::unique_ptr<ProverArithmeticWidget> widget =
        std::make_unique<ProverArithmeticWidget>(circuit_proving_key.get(), witness.get());

    output_state.widgets.emplace_back(std::move(permutation_widget));
    output_state.widgets.emplace_back(std::move(widget));

    return output_state;
}

Prover StandardComposer::create_prover()
{
    compute_proving_key();
    compute_witness();
    Prover output_state(circuit_proving_key, witness, create_manifest(public_inputs.size()));

    std::unique_ptr<ProverPermutationWidget<3>> permutation_widget =
        std::make_unique<ProverPermutationWidget<3>>(circuit_proving_key.get(), witness.get());
    std::unique_ptr<ProverArithmeticWidget> widget =
        std::make_unique<ProverArithmeticWidget>(circuit_proving_key.get(), witness.get());

    output_state.widgets.emplace_back(std::move(permutation_widget));
    output_state.widgets.emplace_back(std::move(widget));
    return output_state;
}

void StandardComposer::assert_equal_constant(uint32_t const a_idx, fr const& b)
{
    const add_triple gate_coefficients{
        a_idx, a_idx, a_idx, fr::one(), fr::zero(), fr::zero(), -b,
    };
    create_add_gate(gate_coefficients);
}

} // namespace waffle