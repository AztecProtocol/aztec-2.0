#include "transcript.hpp"
#include <common/net.hpp>
#include <common/assert.hpp>
#include <crypto/blake2s/blake2s.hpp>
#include <crypto/keccak/keccak.hpp>
#include <iomanip>
#include <iostream>

namespace transcript {

std::array<uint8_t, Keccak256Hasher::PRNG_OUTPUT_SIZE> Keccak256Hasher::hash(std::vector<uint8_t> const& buffer)
{
    keccak256 hash_result = ethash_keccak256(&buffer[0], buffer.size());
    for (auto& word : hash_result.word64s) {
        if (is_little_endian()) {
            word = __builtin_bswap64(word);
        }
    }
    std::array<uint8_t, PRNG_OUTPUT_SIZE> result;
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 8; ++j) {
            uint8_t byte = static_cast<uint8_t>(hash_result.word64s[i] >> (56 - (j * 8)));
            result[i * 8 + j] = byte;
        }
    }
    return result;
}

std::array<uint8_t, Blake2sHasher::PRNG_OUTPUT_SIZE> Blake2sHasher::hash(std::vector<uint8_t> const& buffer)
{
    std::vector<uint8_t> hash_result = blake2::blake2s(buffer);
    std::array<uint8_t, PRNG_OUTPUT_SIZE> result;
    for (size_t i = 0; i < PRNG_OUTPUT_SIZE; ++i) {
        result[i] = hash_result[i];
    }
    return result;
}

Transcript::Transcript(const std::vector<uint8_t>& input_transcript,
                       const Manifest input_manifest,
                       const HashType hash_type,
                       const size_t challenge_bytes)
    : num_challenge_bytes(challenge_bytes)
    , hasher(hash_type)
    , manifest(input_manifest)
{
    const size_t num_rounds = input_manifest.get_num_rounds();
    const uint8_t* buffer = &input_transcript[0];
    size_t count = 0;
    for (size_t i = 0; i < num_rounds; ++i) {
        for (auto manifest_element : input_manifest.get_round_manifest(i).elements) {
            if (!manifest_element.derived_by_verifier) {
                // printf("reading element %s ", manifest_element.name.c_str());
                // for (size_t j = 0; j < manifest_element.num_bytes; ++j) {
                //     printf("%x", buffer[count + j]);
                // }
                // printf("\n");
                elements.insert({ manifest_element.name,
                                  std::vector<uint8_t>(buffer + count, buffer + count + manifest_element.num_bytes) });
                count += manifest_element.num_bytes;
            }
        }
    }
    // printf("input buffer size = %lu \n", count);
}

void Transcript::add_element(const std::string& element_name, const std::vector<uint8_t>& buffer)
{
    ASSERT(manifest.get_round_manifest(current_round).includes_element(element_name));
    // printf("adding element %s . size = %lu \n [", element_name.c_str(), buffer.size());
    // for (size_t i = 0;i < buffer.size(); ++i)
    // {
    //     printf("%x", buffer[i]);
    // }
    // printf("]\n");
    elements.insert({ element_name, buffer });
}

void Transcript::apply_fiat_shamir(const std::string& challenge_name)
{
    ASSERT(current_round <= manifest.get_num_rounds());
    ASSERT(challenge_name == manifest.get_round_manifest(current_round).challenge);

    std::vector<uint8_t> buffer;
    if (current_round > 0) {
        buffer.insert(buffer.end(), current_challenge.data.begin(), current_challenge.data.end());
    }
    for (auto manifest_element : manifest.get_round_manifest(current_round).elements) {
        ASSERT(elements.count(manifest_element.name) == 1);
        std::vector<uint8_t>& element_data = elements.at(manifest_element.name);
        ASSERT(manifest_element.num_bytes == element_data.size());
        buffer.insert(buffer.end(), element_data.begin(), element_data.end());
    }

    std::vector<challenge> round_challenges;
    std::array<uint8_t, PRNG_OUTPUT_SIZE> base_hash{};

    switch (hasher) {
    case HashType::Keccak256: {
        base_hash = Keccak256Hasher::hash(buffer);
        break;
    }
    case HashType::Blake2s: {
        base_hash = Blake2sHasher::hash(buffer);
        break;
    }
    default: {
        base_hash = Keccak256Hasher::hash(buffer);
        break;
    }
    }

    const size_t num_challenges = manifest.get_round_manifest(current_round).num_challenges;
    const size_t challenges_per_hash = PRNG_OUTPUT_SIZE / num_challenge_bytes;

    for (size_t j = 0; j < challenges_per_hash; ++j) {
        std::array<uint8_t, PRNG_OUTPUT_SIZE> challenge{};
        std::copy(base_hash.begin() + (j * num_challenge_bytes),
                  base_hash.begin() + (j + 1) * num_challenge_bytes,
                  challenge.begin() + (PRNG_OUTPUT_SIZE - num_challenge_bytes));
        round_challenges.push_back({ challenge });
    }

    std::vector<uint8_t> rolling_buffer(base_hash.begin(), base_hash.end());
    rolling_buffer.push_back(0);

    size_t num_hashes = (num_challenges / challenges_per_hash);
    if (num_hashes * challenges_per_hash != num_challenges) {
        ++num_hashes;
    }
    for (size_t i = 1; i < num_hashes; ++i) {
        rolling_buffer[rolling_buffer.size() - 1] = static_cast<uint8_t>(i);
        std::array<uint8_t, PRNG_OUTPUT_SIZE> hash_output{};
        switch (hasher) {
        case HashType::Keccak256: {
            hash_output = Keccak256Hasher::hash(rolling_buffer);
            break;
        }
        case HashType::Blake2s: {
            hash_output = Blake2sHasher::hash(rolling_buffer);
            break;
        }
        default: {
        }
        }
        for (size_t j = 0; j < challenges_per_hash; ++j) {
            std::array<uint8_t, PRNG_OUTPUT_SIZE> challenge{};
            std::copy(hash_output.begin() + (j * num_challenge_bytes),
                      hash_output.begin() + (j + 1) * num_challenge_bytes,
                      challenge.begin() + (PRNG_OUTPUT_SIZE - num_challenge_bytes));
            round_challenges.push_back({ challenge });
        }
    }

    current_challenge = round_challenges[round_challenges.size() - 1];

    challenges.insert({ challenge_name, round_challenges });
    ++current_round;
}

std::array<uint8_t, Transcript::PRNG_OUTPUT_SIZE> Transcript::get_challenge(const std::string& challenge_name,
                                                                            const size_t idx) const
{
    // printf("getting challenge %s \n", challenge_name.c_str());
    ASSERT(challenges.count(challenge_name) == 1);
    return challenges.at(challenge_name)[idx].data;
}

size_t Transcript::get_num_challenges(const std::string& challenge_name) const
{
    // printf("getting challenge count for %s \n", challenge_name.c_str());
    ASSERT(challenges.count(challenge_name) == 1);

    return challenges.at(challenge_name).size();
}

std::vector<uint8_t> Transcript::get_element(const std::string& element_name) const
{
    // printf("getting element %s \n", element_name.c_str());
    ASSERT(elements.count(element_name) == 1);
    return elements.at(element_name);
}

std::vector<uint8_t> Transcript::export_transcript() const
{
    std::vector<uint8_t> buffer;

    for (size_t i = 0; i < manifest.get_num_rounds(); ++i) {
        for (auto manifest_element : manifest.get_round_manifest(i).elements) {
            ASSERT(elements.count(manifest_element.name) == 1);
            const std::vector<uint8_t>& element_data = elements.at(manifest_element.name);
            ASSERT(manifest_element.num_bytes == element_data.size());
            if (!manifest_element.derived_by_verifier) {
                // printf("writing element %s ", manifest_element.name.c_str());
                // for (size_t j = 0; j < element_data.size(); ++j) {
                //     printf("%x", element_data[j]);
                // }
                // printf("\n");
                buffer.insert(buffer.end(), element_data.begin(), element_data.end());
            }
        }
    }
    // printf("output buffer size = %lu \n", buffer.size());
    return buffer;
}

} // namespace transcript