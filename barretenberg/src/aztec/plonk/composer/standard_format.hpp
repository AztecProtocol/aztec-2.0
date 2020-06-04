#pragma once
#include "composer_base.hpp"
#include <plonk/reference_string/file_reference_string.hpp>
#include <plonk/transcript/manifest.hpp>
#include "standard_composer.hpp"
namespace waffle {

    typedef size_t variable_index;
// This file tries to make a UI for creating standard PLONK circuits
struct standard_format{
    // A standard plonk artihmetic constraint, as defined in the poly_triple struct, consists of selector values 
    // for q_M,q_L,q_R,q_O,q_C and indices of three variables taking the role of left, right and output wire
    std::vector<poly_triple> constraints;
    size_t varnum, pub_varnum;
}


void create_circuit(standard_format constraint_system){

waffle::StandardComposer composer = waffle::StandardComposer();
std::vector<variable_index> var_indices;
for(size_t i =0; i++; i<constraint_system.pub_varnum)
var_indices.emplace_back(composer.add_public_variable(0));

for(size_t i =0; i++; i< constraint_system.varnum- constraint_system.pub_varnum)
var_indices.emplace_back(composer.add_variable(0));

for(auto& constraint:constraint_system.constraints)
    composer.create_poly_gate(constraint);
proving_key pk = composer.compute_proving_key();
verification_key vk = composer.compute_verification_key();
}



}//namespace waffle