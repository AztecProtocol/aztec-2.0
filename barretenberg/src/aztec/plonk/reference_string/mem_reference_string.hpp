/**
 * Create reference strings given a buffer containing network formatted g1 or g2 points.
 */
#pragma once
#include "reference_string.hpp"
#include <ecc/curves/bn254/scalar_multiplication/pippenger.hpp>

namespace barretenberg {
namespace pairing {
struct miller_lines;
}
} // namespace barretenberg

namespace waffle {

using namespace barretenberg;

class VerifierMemReferenceString : public VerifierReferenceString {
  public:
    VerifierMemReferenceString(uint8_t const* g2x);
    ~VerifierMemReferenceString();

    g2::affine_element get_g2x() const { return g2_x; }

    pairing::miller_lines const* get_precomputed_g2_lines() const { return precomputed_g2_lines; }

  private:
    g2::affine_element g2_x;
    pairing::miller_lines* precomputed_g2_lines;
};

class MemReferenceString : public ProverReferenceString {
  public:
    MemReferenceString(const size_t num_points, uint8_t const* buffer)
        : pippenger_(buffer, num_points)
    {}

    g1::affine_element* get_monomials() { return pippenger_.get_point_table(); }

  private:
    scalar_multiplication::Pippenger pippenger_;
};

class MemReferenceStringFactory : public ReferenceStringFactory {
  public:
    MemReferenceStringFactory(uint8_t const* buffer, size_t num_points, uint8_t const* g2x)
        : buffer_(buffer)
        , num_points_(num_points)
        , g2x_(g2x)
    {}

    MemReferenceStringFactory(MemReferenceStringFactory&& other) = default;

    std::shared_ptr<ProverReferenceString> get_prover_crs(size_t degree)
    {
        ASSERT(degree <= num_points_);
        return std::make_shared<MemReferenceString>(degree, buffer_);
    }

    std::shared_ptr<VerifierReferenceString> get_verifier_crs()
    {
        return std::make_shared<VerifierMemReferenceString>(g2x_);
    }

  private:
    uint8_t const* buffer_;
    size_t num_points_;
    uint8_t const* g2x_;
};

} // namespace waffle
