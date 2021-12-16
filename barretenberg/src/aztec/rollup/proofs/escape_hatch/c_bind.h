#include <cstdint>

#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

WASM_EXPORT void escape_hatch__init_proving_key(void* pippenger, const uint8_t* g2x);

WASM_EXPORT void escape_hatch__init_verification_key(void* pippenger, uint8_t const* g2x);

WASM_EXPORT void* escape_hatch__new_prover(uint8_t const* escape_hatch_buf);

WASM_EXPORT void escape_hatch__delete_prover(void* prover);

WASM_EXPORT bool escape_hatch__verify_proof(uint8_t* proof, uint32_t length);
}
