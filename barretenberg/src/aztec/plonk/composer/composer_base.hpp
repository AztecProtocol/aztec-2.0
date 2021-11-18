#pragma once
#include <ecc/curves/bn254/fr.hpp>
#include <plonk/proof_system/prover/prover.hpp>
#include <plonk/proof_system/verifier/verifier.hpp>
#include <plonk/reference_string/file_reference_string.hpp>
#include <plonk/proof_system/types/prover_settings.hpp>

namespace waffle {
static constexpr uint32_t DUMMY_TAG = 0;

struct proving_key;
struct verification_key;
struct program_witness;

struct add_triple {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    barretenberg::fr a_scaling;
    barretenberg::fr b_scaling;
    barretenberg::fr c_scaling;
    barretenberg::fr const_scaling;
};

struct add_quad {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
    barretenberg::fr a_scaling;
    barretenberg::fr b_scaling;
    barretenberg::fr c_scaling;
    barretenberg::fr d_scaling;
    barretenberg::fr const_scaling;
};

struct mul_quad {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
    barretenberg::fr mul_scaling;
    barretenberg::fr a_scaling;
    barretenberg::fr b_scaling;
    barretenberg::fr c_scaling;
    barretenberg::fr d_scaling;
    barretenberg::fr const_scaling;
};

struct mul_triple {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    barretenberg::fr mul_scaling;
    barretenberg::fr c_scaling;
    barretenberg::fr const_scaling;
};

struct poly_triple {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    barretenberg::fr q_m;
    barretenberg::fr q_l;
    barretenberg::fr q_r;
    barretenberg::fr q_o;
    barretenberg::fr q_c;
};

inline bool operator==(poly_triple const& lhs, poly_triple const& rhs)
{
    // clang-format off
    return
        lhs.a == rhs.a &&
        lhs.b == rhs.b &&
        lhs.c == rhs.c &&
        lhs.q_m == rhs.q_m &&
        lhs.q_l == rhs.q_l &&
        lhs.q_r == rhs.q_r &&
        lhs.q_o == rhs.q_o &&
        lhs.q_c == rhs.q_c;
    // clang-format on
}

struct fixed_group_add_quad {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
    barretenberg::fr q_x_1;
    barretenberg::fr q_x_2;
    barretenberg::fr q_y_1;
    barretenberg::fr q_y_2;
};

struct fixed_group_init_quad {
    barretenberg::fr q_x_1;
    barretenberg::fr q_x_2;
    barretenberg::fr q_y_1;
    barretenberg::fr q_y_2;
};

struct accumulator_triple {
    std::vector<uint32_t> left;
    std::vector<uint32_t> right;
    std::vector<uint32_t> out;
};

struct ecc_add_gate {
    uint32_t x1;
    uint32_t y1;
    uint32_t x2;
    uint32_t y2;
    uint32_t x3;
    uint32_t y3;
    barretenberg::fr endomorphism_coefficient;
    barretenberg::fr sign_coefficient;
};

enum ComposerType {
    STANDARD,
    TURBO,
};

class ComposerBase {
  public:
    struct SelectorProperties {
        std::string name;
        bool use_mid_for_selectorfft = false;           // use middomain instead of large for selectorfft
        bool requires_lagrange_base_polynomial = false; // does the prover need the raw lagrange-base selector values?
    };

    static constexpr uint32_t IS_CONSTANT =
        UINT32_MAX; // indicates whether a witness index actually contains a constant
    static constexpr uint32_t REAL_VARIABLE = UINT32_MAX - 1;
    static constexpr uint32_t FIRST_VARIABLE_IN_CLASS = UINT32_MAX - 2;
    static constexpr size_t NUM_RESERVED_GATES = 4; // this must be >= num_roots_cut_out_of_vanishing_polynomial

    enum WireType { LEFT = 0U, RIGHT = (1U << 30U), OUTPUT = (1U << 31U), FOURTH = 0xc0000000, NULL_WIRE };

    struct cycle_node {
        uint32_t gate_index;
        WireType wire_type;

        cycle_node(const uint32_t a, const WireType b)
            : gate_index(a)
            , wire_type(b)
        {}
        cycle_node(const cycle_node& other)
            : gate_index(other.gate_index)
            , wire_type(other.wire_type)
        {}
        cycle_node(cycle_node&& other)
            : gate_index(other.gate_index)
            , wire_type(other.wire_type)
        {}
        cycle_node& operator=(const cycle_node& other)
        {
            gate_index = other.gate_index;
            wire_type = other.wire_type;
            return *this;
        }
        bool operator==(const cycle_node& other) const
        {
            return ((gate_index == other.gate_index) && (wire_type == other.wire_type));
        }
    };

    ComposerBase()
        : ComposerBase(std::shared_ptr<ReferenceStringFactory>(new FileReferenceStringFactory("../srs_db")))
    {}
    ComposerBase(std::shared_ptr<ReferenceStringFactory> const& crs_factory,
                 size_t selector_num = 0,
                 size_t size_hint = 0,
                 std::vector<SelectorProperties> selector_properties = {})
        : n(0)
        , crs_factory_(crs_factory)
        , selector_num(selector_num)
        , selectors(selector_num)
        , selector_properties(selector_properties)
        , rand_engine(nullptr)
    {
        for (auto& p : selectors) {
            p.reserve(size_hint);
        }
    }

    ComposerBase(size_t selector_num = 0,
                 size_t size_hint = 0,
                 std::vector<SelectorProperties> selector_properties = {})
        : n(0)
        , crs_factory_(std::make_unique<FileReferenceStringFactory>("../srs_db"))
        , selector_num(selector_num)
        , selectors(selector_num)
        , selector_properties(selector_properties)
        , rand_engine(nullptr)
    {
        for (auto& p : selectors) {
            p.reserve(size_hint);
        }
    }

    ComposerBase(std::shared_ptr<proving_key> const& p_key,
                 std::shared_ptr<verification_key> const& v_key,
                 size_t selector_num = 0,
                 size_t size_hint = 0,
                 std::vector<SelectorProperties> selector_properties = {})
        : n(0)
        , circuit_proving_key(p_key)
        , circuit_verification_key(v_key)
        , selector_num(p_key ? p_key->constraint_selectors.size() : selector_num)
        , selectors(selector_num)
        , selector_properties(selector_properties)
        , rand_engine(nullptr)
    {
        for (auto& p : selectors) {
            p.reserve(size_hint);
        }
    }

    ComposerBase(ComposerBase&& other) = default;
    ComposerBase& operator=(ComposerBase&& other) = default;
    virtual ~ComposerBase(){};

    virtual size_t get_num_gates() const { return n; }
    virtual size_t get_num_variables() const { return variables.size(); }
    virtual std::shared_ptr<proving_key> compute_proving_key_base(const size_t minimum_ciricut_size = 0,
                                                                  const size_t num_reserved_gates = NUM_RESERVED_GATES);
    virtual std::shared_ptr<proving_key> compute_proving_key() = 0;
    virtual std::shared_ptr<verification_key> compute_verification_key() = 0;
    virtual std::shared_ptr<program_witness> compute_witness() = 0;
    template <class program_settings> std::shared_ptr<program_witness> compute_witness_base();
    uint32_t zero_idx = 0;

    virtual void create_add_gate(const add_triple& in) = 0;
    virtual void create_mul_gate(const mul_triple& in) = 0;
    virtual void create_bool_gate(const uint32_t a) = 0;
    virtual void create_poly_gate(const poly_triple& in) = 0;
    virtual size_t get_num_constant_gates() const = 0;

    // update all variables in cycle of index to have real variable new_real_index
    uint32_t get_first_variable_in_class(uint32_t index) const
    {
        while (prev_var_index[index] != FIRST_VARIABLE_IN_CLASS) {
            index = prev_var_index[index];
        }
        return index;
    }
    // update all variables from index in equivalence class to have real variable new_real_index
    void update_real_variable_indices(uint32_t index, uint32_t new_real_index)
    {
        auto cur_index = index;
        do {
            real_variable_index[cur_index] = new_real_index;
            cur_index = next_var_index[cur_index];
        } while (cur_index != REAL_VARIABLE);
    }
    // uint32_t get_real_variable_index(uint32_t index) const
    // {
    //     if (index >= variable_index_map.size())
    //         std::cerr << "index:" << index << std::endl;
    //     ASSERT(index < variable_index_map.size());
    //     if (variable_index_map[index] != REAL_VARIABLE) {
    //         return get_real_variable_index(variable_index_map[index]);
    //     }
    //     return index;
    // }

    barretenberg::fr get_variable(const uint32_t index) const
    {
        ASSERT(variables.size() > index);
        return variables[real_variable_index[index]];
    }

    virtual uint32_t add_variable(const barretenberg::fr& in)
    {
        variables.emplace_back(in);
        const uint32_t index = static_cast<uint32_t>(variables.size()) - 1U;
        real_variable_index.emplace_back(index);
        next_var_index.emplace_back(REAL_VARIABLE);
        prev_var_index.emplace_back(FIRST_VARIABLE_IN_CLASS);
        variable_tags.emplace_back(DUMMY_TAG);
        wire_copy_cycles.push_back(std::vector<cycle_node>());
        return index;
    }

    virtual uint32_t add_public_variable(const barretenberg::fr& in)
    {
        variables.emplace_back(in);
        const uint32_t index = static_cast<uint32_t>(variables.size()) - 1U;
        real_variable_index.emplace_back(index);
        next_var_index.emplace_back(REAL_VARIABLE);
        prev_var_index.emplace_back(FIRST_VARIABLE_IN_CLASS);
        variable_tags.emplace_back(DUMMY_TAG);
        wire_copy_cycles.push_back(std::vector<cycle_node>());
        public_inputs.emplace_back(index);
        return index;
    }

    virtual void set_public_input(const uint32_t witness_index)
    {
        bool does_not_exist = true;
        for (size_t i = 0; i < public_inputs.size(); ++i) {
            does_not_exist = does_not_exist && (public_inputs[i] != witness_index);
        }
        if (does_not_exist) {
            public_inputs.emplace_back(witness_index);
        }
    }

    virtual void assert_equal(const uint32_t a_idx, const uint32_t b_idx, std::string const& msg = "assert_equal");
    virtual void copy_from_to(const uint32_t from_idx, const uint32_t to_idx);

    template <size_t program_width> void compute_wire_copy_cycles();
    template <size_t program_width, bool with_tags = false> void compute_sigma_permutations(proving_key* key);

    void add_selector(polynomial& small, const std::string& tag, bool preserve_lagrange_base = false)
    {
        if (preserve_lagrange_base) {
            polynomial lagrange_base(small, circuit_proving_key->n);
            circuit_proving_key->constraint_selectors_lagrange_base.insert({ tag, std::move(lagrange_base) });
        }
        small.ifft(circuit_proving_key->small_domain);
        polynomial large(small, circuit_proving_key->n * 4);
        large.coset_fft(circuit_proving_key->large_domain);
        circuit_proving_key->constraint_selectors.insert({ tag, std::move(small) });
        circuit_proving_key->constraint_selector_ffts.insert({ tag + "_fft", std::move(large) });
    }

    size_t get_circuit_subgroup_size(const size_t num_gates)
    {
        size_t log2_n = static_cast<size_t>(numeric::get_msb(num_gates));
        if ((1UL << log2_n) != (num_gates)) {
            ++log2_n;
        }
        return 1UL << log2_n;
    }

    size_t get_num_public_inputs() const { return public_inputs.size(); }

  public:
    size_t n;
    std::vector<uint32_t> w_l;
    std::vector<uint32_t> w_r;
    std::vector<uint32_t> w_o;
    std::vector<uint32_t> w_4;
    std::vector<uint32_t> public_inputs;
    std::vector<barretenberg::fr> variables;
    std::vector<uint32_t> next_var_index; // index of next variable in equivalence class (=REAL_VARIABLE if you're last)
    std::vector<uint32_t>
        prev_var_index; // index of  previous variable in equivalence class (=FIRST if you're in a cycle alone)
    std::vector<uint32_t> real_variable_index; // indices of corresponding real variables
    std::vector<uint32_t> variable_tags;
    uint32_t current_tag = DUMMY_TAG;
    std::map<uint32_t, uint32_t> tau; // the permutation on variable tags;
    std::vector<std::vector<cycle_node>> wire_copy_cycles;

    std::shared_ptr<proving_key> circuit_proving_key;
    std::shared_ptr<verification_key> circuit_verification_key;

    bool computed_witness = false;
    std::shared_ptr<program_witness> witness;

    std::shared_ptr<ReferenceStringFactory> crs_factory_;
    size_t selector_num;
    std::vector<std::vector<barretenberg::fr>> selectors;
    std::vector<SelectorProperties> selector_properties;
    bool failed = false;
    std::string err;
    numeric::random::Engine* rand_engine;
};

extern template void ComposerBase::compute_wire_copy_cycles<3>();
extern template void ComposerBase::compute_wire_copy_cycles<4>();
extern template void ComposerBase::compute_sigma_permutations<3, false>(proving_key* key);
extern template void ComposerBase::compute_sigma_permutations<4, false>(proving_key* key);
extern template std::shared_ptr<program_witness> ComposerBase::compute_witness_base<standard_settings>();
extern template std::shared_ptr<program_witness> ComposerBase::compute_witness_base<turbo_settings>();
extern template void ComposerBase::compute_sigma_permutations<4, true>(proving_key* key);

} // namespace waffle
