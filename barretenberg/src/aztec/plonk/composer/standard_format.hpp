#pragma once
#include "composer_base.hpp"
#include <plonk/reference_string/file_reference_string.hpp>
#include <plonk/transcript/manifest.hpp>
#include "standard_composer.hpp"
#include <common/serialize.hpp>
#include <plonk/proof_system/proving_key/serialize.hpp>
#include <plonk/proof_system/verification_key/verification_key.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

using boost::property_tree::ptree;
using boost::property_tree::read_json;
namespace waffle {

typedef size_t variable_index;
// This file tries to make a UI for creating standard PLONK circuits
struct standard_format {
    uint32_t varnum;
    uint32_t pub_varnum;
    uint32_t constraint_num;
    // A standard plonk artihmetic constraint, as defined in the poly_triple struct, consists of selector values
    // for q_M,q_L,q_R,q_O,q_C and indices of three variables taking the role of left, right and output wire
    std::vector<poly_triple> constraints;
};

waffle::StandardComposer create_circuit(const standard_format& constraint_system)
{
    if (constraint_system.pub_varnum > constraint_system.varnum)
        std::cout << "too many public inputs!" << std::endl;
    waffle::StandardComposer composer = waffle::StandardComposer();
    std::vector<variable_index> var_indices;
    for (size_t i = 0; i < constraint_system.pub_varnum; i++) {
        var_indices.emplace_back(composer.add_public_variable(0));
    }

    for (size_t i = 0; i < constraint_system.varnum - constraint_system.pub_varnum; i++)
        var_indices.emplace_back(composer.add_variable(0));

    for (const auto& constraint : constraint_system.constraints) {
        composer.create_poly_gate(constraint);
    }

    return composer;
}
void write_proving_and_verifying_key(const StandardComposer& composer)
{
    std::ofstream os("pk.data");
    write(os, *composer.circuit_proving_key);
    os.close();

    std::ofstream os2("vk.data");
    write(os2, *composer.circuit_verification_key);
    os2.close();
}
void write_proof_to_file(const plonk_proof& proof)
{
    std::ofstream os("proof.data");
    write(os, proof.proof_data);
    os.close();
}
void read_proof_from_file(plonk_proof& proof)
{
    std::ifstream is("proof.data");
    read(is, proof.proof_data);
}
template <typename B> inline void read(B& buf, poly_triple& constraint)
{
    ::read(buf, constraint.a);
    ::read(buf, constraint.b);
    ::read(buf, constraint.c);
    read(buf, constraint.q_l);
    read(buf, constraint.q_r);
    read(buf, constraint.q_o);
    read(buf, constraint.q_m);
    read(buf, constraint.q_c);
}

template <typename B> inline void write(B& buf, poly_triple const& constraint)
{
    ::write(buf, constraint.a);
    ::write(buf, constraint.b);
    ::write(buf, constraint.c);
    write(buf, constraint.q_l);
    write(buf, constraint.q_r);
    write(buf, constraint.q_o);
    write(buf, constraint.q_m);
    write(buf, constraint.q_c);
}

template <typename B> inline void read(B& buf, standard_format& data)
{
    ::read(buf, data.varnum);
    ::read(buf, data.pub_varnum);
    ::read(buf, data.constraint_num);
    read(buf, data.constraints);
}

template <typename B> inline void write(B& buf, standard_format const& data)
{
    ::write(buf, data.varnum);
    ::write(buf, data.pub_varnum);
    ::write(buf, data.constraint_num);
    write(buf, data.constraints);
}

std::vector<fr> read_witness_from_file(const std::string filename, StandardComposer& composer)
{
    std::ifstream json(filename);
    ptree pt2;
    std::stringstream ss;
    ss << json.rdbuf();
    std::vector<fr> res;

    boost::property_tree::ptree pt;
    boost::property_tree::read_json(ss, pt);

    composer.variables.clear();
    BOOST_FOREACH (boost::property_tree::ptree::value_type& v, pt.get_child("witness")) {
        assert(v.first.empty()); // array elements have no names
        composer.variables.emplace_back(fr(std::stoi(v.second.data())));
    }
    return res;
}
Verifier make_verifier_with_public_input_from_file(const std::string filename,
                                                          const std::shared_ptr<verification_key> vk)
{
    std::ifstream json(filename);
    ptree pt2;
    std::stringstream ss;
    ss << json.rdbuf();
    std::vector<fr> res;

    boost::property_tree::ptree pt;
    boost::property_tree::read_json(ss, pt);

    std::string pubvarnum = pt.get<std::string>("pubvarnum");
    auto pubvarnumi = (uint32_t)std::stoi(pubvarnum);
    std::vector<fr> PI;
    BOOST_FOREACH (boost::property_tree::ptree::value_type& v, pt.get_child("PI")) {
        assert(v.first.empty()); // array elements have no names
        PI.emplace_back(fr(std::stoi(v.second.data())));
    }
    
    return Verifier(vk,waffle::StandardComposer::create_manifest(pubvarnumi), true, PI);
}
standard_format read_constraint_system_from_file(const std::string filename)
{
    std::ifstream json(filename);
    ptree pt;
    std::stringstream ss;
    ss << json.rdbuf();
    boost::property_tree::read_json(ss, pt);

    std::string varnum = pt.get<std::string>("varnum");
    int varnumi = std::stoi(varnum);

    std::string pubvarnum = pt.get<std::string>("pubvarnum");
    auto pubvarnumi = std::stoi(pubvarnum);
    std::string constraintnum = pt.get<std::string>("constraintnum");
    auto constraintnumi = std::stoi(constraintnum);
    waffle::standard_format res{ (uint32_t)varnumi, (uint32_t)pubvarnumi, (uint32_t)constraintnumi, {} };
    BOOST_FOREACH (boost::property_tree::ptree::value_type& v, pt.get_child("constraints")) {

        std::string a = v.second.get<std::string>("a");
        uint32_t ai = (uint32_t)std::stoi(a);
        std::string b = v.second.get<std::string>("b");
        auto bi = (uint32_t)std::stoi(b);
        std::string c = v.second.get<std::string>("c");
        auto ci = (uint32_t)std::stoi(c);
        std::string ql = v.second.get<std::string>("ql");
        fr qli(std::stoi(ql));
        std::string qr = v.second.get<std::string>("qr");
        fr qri(std::stoi(qr));
        std::string qo = v.second.get<std::string>("qo");
        fr qoi(std::stoi(qo));
        std::string qm = v.second.get<std::string>("qm");
        fr qmi(std::stoi(qm));
        std::string qc = v.second.get<std::string>("qc");
        fr qci(std::stoi(qc));
        res.constraints.emplace_back(poly_triple{ ai, bi, ci, qmi, qli, qri, qoi, qci });
    }
    return res;
}
void read_witness(std::vector<barretenberg::fr> witness, StandardComposer& composer)
{
    for (size_t i = 0; i < witness.size(); i++) {
        composer.variables[i] = witness[i];
    }
}
std::shared_ptr<proving_key> read_proving_key_from_file(size_t total_num_gates)
{
    auto crs_factory = std::make_unique<FileReferenceStringFactory>("../srs_db");

    size_t log2_n = static_cast<size_t>(numeric::get_msb(total_num_gates + 1));
    if ((1UL << log2_n) != (total_num_gates + 1)) {
        ++log2_n;
    }
    size_t new_n = 1UL << log2_n;
    // const size_t subgroup_size = ComposerBase::get_circuit_subgroup_size(total_num_gates +
    // StandardComposer::NUM_RESERVED_GATES);
    auto crs = crs_factory->get_prover_crs(new_n);
    proving_key_data data;
    std::ifstream is("pk.data");
    read(static_cast<std::istream&>(is), data);
    return std::make_shared<proving_key>(std::move(data), crs);
}
std::shared_ptr<verification_key> read_verification_key_from_file()
{
    auto crs_factory = std::make_unique<FileReferenceStringFactory>("../srs_db");
    auto ver_crs = crs_factory->get_verifier_crs();
    verification_key_data data;
    std::ifstream is("vk.data");
    read(static_cast<std::istream&>(is), data);
    return std::make_shared<verification_key>(std::move(data), ver_crs);
}
void read_witness(std::istream& is, StandardComposer& composer)
{
    read(is, composer.variables);
}
} // namespace waffle