#include "./pedersen.hpp"
#include "barretenberg/common/serialize.hpp"
#include "barretenberg/common/throw_or_abort.hpp"
#include <iostream>
#ifndef NO_OMP_MULTITHREADING
#include <omp.h>
#endif

namespace crypto {

/**
 * @brief Given a vector of fields, generate a pedersen commitment using the indexed generators.
 *
 * @details This method uses `Curve::BaseField` members as inputs. This aligns with what we expect when creating
 * grumpkin commitments to field elements inside a BN254 SNARK circuit.
 * @param inputs
 * @param context
 * @return Curve::AffineElement
 */
template <typename Curve>
typename Curve::AffineElement pedersen_commitment_base<Curve>::commit_native(const std::vector<Fq>& inputs,
                                                                             const GeneratorContext context)
{
    const auto generators = context.generators->get(inputs.size(), context.offset, context.domain_separator);
    Element result = Group::point_at_infinity;

    for (size_t i = 0; i < inputs.size(); ++i) {
        result += Element(generators[i]) * static_cast<uint256_t>(inputs[i]);
    }
    return result.normalize();
}

/**
 * @brief Given a vector of fields, generate a pedersen commitment using the indexed generators.
 *
 * @details This method uses `ScalarField` members as inputs. This aligns with what we expect for a "canonical"
 * elliptic curve commitment function. However, when creating grumpkin commitments inside a BN254 SNARK crcuit it is not
 * efficient to pack data into grumpkin::fr elements, as grumpkin::fq is the native field of BN254 circuits.
 *
 * @note This method is used currently for tests. If we find no downstream use for it by Jan 2024, delete!
 * @param inputs
 * @param context
 * @return Curve::AffineElement
 */
template <typename Curve>
typename Curve::BaseField pedersen_commitment_base<Curve>::compress_native(const std::vector<Fq>& inputs,
                                                                           const GeneratorContext context)
{
    return commit_native(inputs, context).x;
}
template class pedersen_commitment_base<curve::Grumpkin>;
} // namespace crypto
