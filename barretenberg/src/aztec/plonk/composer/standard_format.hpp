#pragma once
#include "composer_base.hpp"
#include <plonk/reference_string/file_reference_string.hpp>
#include <plonk/transcript/manifest.hpp>
#include "standard_composer.hpp"
#include <common/serialize.hpp>
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


waffle::StandardComposer create_circuit(standard_format& constraint_system)
{
std::cout << "in create" << constraint_system.pub_varnum << " " << constraint_system.varnum <<  std::endl;
if(constraint_system.pub_varnum> constraint_system.varnum)
std::cout << "too many public inputs!" << std::endl;
    waffle::StandardComposer composer = waffle::StandardComposer();
    std::vector<variable_index> var_indices;
    for (size_t i = 0; i < constraint_system.pub_varnum; i++ ){
        var_indices.emplace_back(composer.add_public_variable(0));
    }

    for (size_t i = 0; i < constraint_system.varnum - constraint_system.pub_varnum; i++)
        var_indices.emplace_back(composer.add_variable(0));

    for (const auto& constraint : constraint_system.constraints){
        std::cout<< "a:" << constraint.a << "b:" << (uint32_t)constraint.b << std::endl; 
        std::cout<< "c:" << constraint.c << "ql:" << constraint.q_l << std::endl; 
        composer.create_poly_gate(constraint);
    }
        
    
    return composer;
    // std::shared_ptr<proving_key> pk = composer.compute_proving_key();
    // std::shared_ptr<verification_key> vk = composer.compute_verification_key();
}
// void create_circuit_and_write_to_file(standard_format& constraint_system, std::ostream& os){
// auto composer = create_circuit(constraint_system);
// }
// void read(std::istream& is, poly_triple& constraint){
// read(is, constraint.a);
// read(is, constraint.b);
// read(is, constraint.c);
// read(is, constraint.q_l);
// read(is, constraint.q_r);
// read(is, constraint.q_o);
// read(is, constraint.q_m);
// read(is, constraint.q_c);

// }


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

// standard_format read_constraint_system_from_file(std::ifstream& is)
// {
//     standard_format constraint_system;
//     read(is, constraint_system);
//     std::cout << "in read:" << constraint.b << std::endl;
//     return constraint_system;
// }

// void write_constraint_system_to_file(std::ofstream& os,standard_format constraint_system)
// {

//     write(os, constraint_system.varnum);
//     write(os, constraint_system.pub_varnum);
//     write(os, constraint_system.constraint_num);
//     for (size_t i = 0; i < constraint_system.constraint_num; i++) {
//         poly_triple constraint;
//         write(os, constraint);
//         constraint_system.constraints.emplace_back(constraint);
        
//     }
// }
void read_witness(std::istream& is, StandardComposer& composer, size_t witness_length){
   for(size_t i =0; i < witness_length ; i++) {
       barretenberg::fr val;
       read(is, val);
       composer.variables[i]=val;
   }
}
void read_witness(std::vector<barretenberg::fr> witness, StandardComposer& composer){
   for(size_t i =0; i < witness.size() ; i++) {
       composer.variables[i]=witness[i];
   }
}
} // namespace waffle