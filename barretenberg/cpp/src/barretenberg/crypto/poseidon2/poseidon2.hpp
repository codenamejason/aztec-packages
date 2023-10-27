#pragma once

#include "poseidon2_params.hpp"

#include "barretenberg/common/throw_or_abort.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace crypto {

template <typename Params> class Poseidon2 {
  public:
    // todo make parameters configurable?
    static constexpr size_t t = Params::t;
    static constexpr size_t d = Params::d;
    static constexpr size_t sbox_size = Params::sbox_size; // logp
    static constexpr size_t rounds_f = Params::rounds_f;
    static constexpr size_t rounds_p = Params::rounds_p;
    static constexpr size_t NUM_ROUNDS = Params::rounds_f + Params::rounds_p;

    using FF = typename Params::FF;
    using State = std::vector<FF>;
    using RoundConstants = std::array<FF, t>;
    using MatrixDiagonal = std::array<FF, t>;
    using RoundConstantsContainer = std::array<RoundConstants, NUM_ROUNDS>;

    static constexpr MatrixDiagonal internal_matrix_diagonal =
        Poseidon2Bn254ScalarFieldParams::internal_matrix_diagonal;
    static constexpr RoundConstantsContainer round_constants = Poseidon2Bn254ScalarFieldParams::round_constants;
    // derive_round_constants();

    static void matrix_multiplication_4x4(State& input)
    {
        /**
         * hardcoded algorithm that evaluates matrix multiplication using the following MDS matrix:
         * /         \
         * | 5 7 1 3 |
         * | 4 6 1 1 |
         * | 1 3 5 7 |
         * | 1 1 4 6 |
         * \         /
         */

        auto t0 = input[0] + input[1]; // A + B
        auto t1 = input[2] + input[3]; // C + D
        auto t2 = input[1] + input[1]; // 2B
        t2 += t1;                      // 2B + C + D
        auto t3 = input[3] + input[3]; // 2C
        t3 += t0;                      // 2C + A + B
        auto t4 = t1 + t1;
        t4 += t4;
        t4 += t3; // 4D + 6C + A + B
        auto t5 = t0 + t0;
        t5 += t5;
        t5 += t3;          // 4A + 6B + C + D
        auto t6 = t3 + t5; // 5A + 7B + 7C + 5D
        auto t7 = t2 + t4; // A + 3B + 5D + 7C
        input[0] = t6;
        input[1] = t5;
        input[2] = t7;
        input[3] = t4;
    }

    static void add_round_constants(State& input, const RoundConstants& rc)
    {
        for (size_t i = 0; i < t; ++i) {
            input[i] += rc[i];
        }
    }

    static void matrix_multiplication_internal(State& input)
    {
        // for t = 4
        auto sum = input[0];
        for (size_t i = 1; i < t; ++i) {
            sum += input[i];
        }
        for (size_t i = 0; i < t; ++i) {
            input[i] *= internal_matrix_diagonal[i];
            input[i] += sum;
        }
    }

    static void matrix_multiplication_external(State& input)
    {
        if constexpr (t == 4) {
            matrix_multiplication_4x4(input);
        } else {
            // erm panic
            throw_or_abort("not supported");
        }
    }

    static void apply_single_sbox(FF& input)
    {
        auto xx = input.sqr();
        auto xxxx = xx.sqr();
        input *= xxxx;
    }

    static void apply_sbox(State& input)
    {
        for (auto& in : input) {
            apply_single_sbox(in);
        }
    }

    static State permutation(State& input)
    {
        // deep copy
        State current_state(input);

        // Apply 1st linear layer
        matrix_multiplication_external(current_state);

        for (size_t i = 0; i < rounds_f; ++i) {
            add_round_constants(current_state, round_constants[i]);
            apply_sbox(current_state);
            matrix_multiplication_external(current_state);
        }

        const size_t p_end = rounds_f + rounds_p;
        for (size_t i = rounds_f; i < p_end; ++i) {
            current_state[0] += round_constants[i][0];
            apply_single_sbox(current_state[0]);
            matrix_multiplication_internal(current_state);
        }

        for (size_t i = p_end; i < NUM_ROUNDS; ++i) {
            add_round_constants(current_state, round_constants[i]);
            apply_sbox(current_state);
            matrix_multiplication_external(current_state);
        }
        return current_state;
    }
};
} // namespace crypto