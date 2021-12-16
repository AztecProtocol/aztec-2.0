#include "encrypt_note.hpp"
#include "../constants.hpp"
#include <crypto/pedersen/pedersen.hpp>

using namespace barretenberg;

namespace rollup {
namespace proofs {
namespace notes {
namespace native {

grumpkin::g1::affine_element encrypt_note(value_note const& note)
{
    grumpkin::g1::element p_1 = crypto::pedersen::fixed_base_scalar_mul<NOTE_VALUE_BIT_LENGTH>(note.value, 0);
    grumpkin::g1::element p_2 = crypto::pedersen::fixed_base_scalar_mul<254>(note.secret, 1);
    grumpkin::g1::element p_4 = crypto::pedersen::fixed_base_scalar_mul<32>((uint64_t)note.asset_id, 2);
    grumpkin::g1::element sum;
    if (note.value > 0) {
        sum = p_1 + p_2;
    } else {
        sum = p_2;
    }
    if (note.asset_id > 0) {
        sum += p_4;
    }
    grumpkin::g1::affine_element p_3 = crypto::pedersen::compress_to_point_native(note.owner.x, note.owner.y, 3);

    sum += p_3;
    grumpkin::g1::element p_5 = crypto::pedersen::fixed_base_scalar_mul<32>((uint64_t)note.nonce, 5);
    if (note.nonce > 0) {
        sum += p_5;
    }
    sum = sum.normalize();

    return { sum.x, sum.y };
}

} // namespace native
} // namespace notes
} // namespace proofs
} // namespace rollup