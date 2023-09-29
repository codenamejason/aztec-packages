#include <gtest/gtest.h>

#include "barretenberg/crypto/pedersen_commitment/pedersen.hpp"
#include "barretenberg/numeric/bitop/get_msb.hpp"
#include "barretenberg/numeric/uintx/uintx.hpp"
#include "barretenberg/plonk/composer/ultra_composer.hpp"
#include "barretenberg/plonk/proof_system/widgets/random_widgets/plookup_widget.hpp"
#include "barretenberg/proof_system/circuit_builder/ultra_circuit_builder.hpp"
#include "barretenberg/proof_system/plookup_tables/sha256.hpp"
#include "barretenberg/stdlib/primitives/plookup/plookup.hpp"

using namespace barretenberg;
using namespace proof_system;
using namespace proof_system::plonk;

namespace proof_system::plonk::test_ultra_plonk_composer {

namespace {
auto& engine = numeric::random::get_debug_engine();
}

using plookup::ColumnIdx;
using plookup::MultiTableId;

std::vector<uint32_t> add_variables(UltraCircuitBuilder& builder, std::vector<fr> variables)
{
    std::vector<uint32_t> res;
    for (size_t i = 0; i < variables.size(); i++) {
        res.emplace_back(builder.add_variable(variables[i]));
    }
    return res;
}

template <typename T> class ultra_plonk_composer : public ::testing::Test {
  public:
    void prove_and_verify(UltraCircuitBuilder& builder, UltraComposer& composer, bool expected_result)
    {
        if constexpr (T::use_keccak) {
            auto prover = composer.create_ultra_with_keccak_prover(builder);
            auto verifier = composer.create_ultra_with_keccak_verifier(builder);
            auto proof = prover.construct_proof();
            bool verified = verifier.verify_proof(proof);
            EXPECT_EQ(verified, expected_result);
        } else {
            auto prover = composer.create_prover(builder);
            auto verifier = composer.create_verifier(builder);
            auto proof = prover.construct_proof();
            bool verified = verifier.verify_proof(proof);
            EXPECT_EQ(verified, expected_result);
        }
    };
};

struct UseKeccak32Bytes {
    static constexpr bool use_keccak = true;
};

struct UsePlookupPedersen16Bytes {
    static constexpr bool use_keccak = false;
};

using BooleanTypes = ::testing::Types<UseKeccak32Bytes, UsePlookupPedersen16Bytes>;
TYPED_TEST_SUITE(ultra_plonk_composer, BooleanTypes);

// TODO fix
// TYPED_TEST(ultra_plonk_composer, create_gates_from_plookup_accumulators)
// {
//     auto builder = UltraCircuitBuilder();
//     auto composer = UltraComposer();

//     barretenberg::fr input_value = fr::random_element();
//     const fr input_hi = uint256_t(input_value).slice(126, 256);
//     const fr input_lo = uint256_t(input_value).slice(0, 126);
//     const auto input_hi_index = builder.add_variable(input_hi);
//     const auto input_lo_index = builder.add_variable(input_lo);

//     const auto sequence_data_hi = plookup::get_lookup_accumulators(MultiTableId::FIXED_BASE_LEFT_HI, input_hi);
//     const auto sequence_data_lo = plookup::get_lookup_accumulators(MultiTableId::FIXED_BASE_LEFT_LO, input_lo);

//     const auto lookup_witnesses_hi = builder.create_gates_from_plookup_accumulators(
//         MultiTableId::FIXED_BASE_LEFT_HI, sequence_data_hi, input_hi_index);
//     const auto lookup_witnesses_lo = builder.create_gates_from_plookup_accumulators(
//         MultiTableId::FIXED_BASE_LEFT_LO, sequence_data_lo, input_lo_index);

//     std::vector<barretenberg::fr> expected_x;
//     std::vector<barretenberg::fr> expected_y;

//     const size_t num_lookups_hi =
//         (128 + plookup::FixedBaseParams::BITS_PER_TABLE) / plookup::FixedBaseParams::BITS_PER_TABLE;
//     const size_t num_lookups_lo = 126 / plookup::FixedBaseParams::BITS_PER_TABLE;
//     const size_t num_lookups = num_lookups_hi + num_lookups_lo;

//     EXPECT_EQ(num_lookups_hi, lookup_witnesses_hi[ColumnIdx::C1].size());
//     EXPECT_EQ(num_lookups_lo, lookup_witnesses_lo[ColumnIdx::C1].size());

//     std::vector<barretenberg::fr> expected_scalars;
//     expected_x.resize(num_lookups);
//     expected_y.resize(num_lookups);
//     expected_scalars.resize(num_lookups);

//     {
//         const size_t num_rounds = (num_lookups + 1) / 2;
//         uint256_t bits(input_value);

//         const auto mask = plookup::FixedBaseParams::MAX_TABLE_SIZE - 1;

//         for (size_t i = 0; i < num_rounds; ++i) {
//             const auto& table = crypto::pedersen_hash::lookup::get_table(i);
//             const size_t index = i * 2;

//             uint64_t slice_a = ((bits >> (index * 9)) & mask).data[0];
//             expected_x[index] = (table[(size_t)slice_a].x);
//             expected_y[index] = (table[(size_t)slice_a].y);
//             expected_scalars[index] = slice_a;

//             if (i < 14) {
//                 uint64_t slice_b = ((bits >> ((index + 1) * 9)) & mask).data[0];
//                 expected_x[index + 1] = (table[(size_t)slice_b].x);
//                 expected_y[index + 1] = (table[(size_t)slice_b].y);
//                 expected_scalars[index + 1] = slice_b;
//             }
//         }
//     }

//     for (size_t i = num_lookups - 2; i < num_lookups; --i) {
//         expected_scalars[i] += (expected_scalars[i + 1] * plookup::FixedBaseParams::MAX_TABLE_SIZE);
//     }

//     size_t hi_shift = 126;
//     const fr hi_cumulative = builder.get_variable(lookup_witnesses_hi[ColumnIdx::C1][0]);
//     for (size_t i = 0; i < num_lookups_lo; ++i) {
//         const fr hi_mult = fr(uint256_t(1) << hi_shift);
//         EXPECT_EQ(builder.get_variable(lookup_witnesses_lo[ColumnIdx::C1][i]) + (hi_cumulative * hi_mult),
//                   expected_scalars[i]);
//         EXPECT_EQ(builder.get_variable(lookup_witnesses_lo[ColumnIdx::C2][i]), expected_x[i]);
//         EXPECT_EQ(builder.get_variable(lookup_witnesses_lo[ColumnIdx::C3][i]), expected_y[i]);
//         hi_shift -= plookup::FixedBaseParams::BITS_PER_TABLE;
//     }

//     for (size_t i = 0; i < num_lookups_hi; ++i) {
//         EXPECT_EQ(builder.get_variable(lookup_witnesses_hi[ColumnIdx::C1][i]), expected_scalars[i + num_lookups_lo]);
//         EXPECT_EQ(builder.get_variable(lookup_witnesses_hi[ColumnIdx::C2][i]), expected_x[i + num_lookups_lo]);
//         EXPECT_EQ(builder.get_variable(lookup_witnesses_hi[ColumnIdx::C3][i]), expected_y[i + num_lookups_lo]);
//     }

//     TestFixture::prove_and_verify(builder, composer, /*expected_result=*/true);
//}

TYPED_TEST(ultra_plonk_composer, test_no_lookup_proof)
{
    auto builder = UltraCircuitBuilder();
    auto composer = UltraComposer();

    for (size_t i = 0; i < 16; ++i) {
        for (size_t j = 0; j < 16; ++j) {
            uint64_t left = static_cast<uint64_t>(j);
            uint64_t right = static_cast<uint64_t>(i);
            uint32_t left_idx = builder.add_variable(fr(left));
            uint32_t right_idx = builder.add_variable(fr(right));
            uint32_t result_idx = builder.add_variable(fr(left ^ right));

            uint32_t add_idx = builder.add_variable(fr(left) + fr(right) + builder.get_variable(result_idx));
            builder.create_big_add_gate(
                { left_idx, right_idx, result_idx, add_idx, fr(1), fr(1), fr(1), fr(-1), fr(0) });
        }
    }

    TestFixture::prove_and_verify(builder, composer, /*expected_result=*/true);
}

TYPED_TEST(ultra_plonk_composer, test_elliptic_gate)
{
    typedef grumpkin::g1::affine_element affine_element;
    typedef grumpkin::g1::element element;
    auto builder = UltraCircuitBuilder();
    auto composer = UltraComposer();

    affine_element p1 = crypto::pedersen_commitment::commit_native({ barretenberg::fr(1) }, 0);

    affine_element p2 = crypto::pedersen_commitment::commit_native({ barretenberg::fr(1) }, 1);
    ;
    affine_element p3(element(p1) + element(p2));

    uint32_t x1 = builder.add_variable(p1.x);
    uint32_t y1 = builder.add_variable(p1.y);
    uint32_t x2 = builder.add_variable(p2.x);
    uint32_t y2 = builder.add_variable(p2.y);
    uint32_t x3 = builder.add_variable(p3.x);
    uint32_t y3 = builder.add_variable(p3.y);

    builder.create_ecc_add_gate({ x1, y1, x2, y2, x3, y3, 1, 1 });

    grumpkin::fq beta = grumpkin::fq::cube_root_of_unity();
    affine_element p2_endo = p2;
    p2_endo.x *= beta;
    p3 = affine_element(element(p1) + element(p2_endo));
    x3 = builder.add_variable(p3.x);
    y3 = builder.add_variable(p3.y);
    builder.create_ecc_add_gate({ x1, y1, x2, y2, x3, y3, beta, 1 });

    p2_endo.x *= beta;
    p3 = affine_element(element(p1) - element(p2_endo));
    x3 = builder.add_variable(p3.x);
    y3 = builder.add_variable(p3.y);
    builder.create_ecc_add_gate({ x1, y1, x2, y2, x3, y3, beta.sqr(), -1 });

    TestFixture::prove_and_verify(builder, composer, /*expected_result=*/true);
}

TYPED_TEST(ultra_plonk_composer, non_trivial_tag_permutation)
{
    auto builder = UltraCircuitBuilder();
    auto composer = UltraComposer();
    fr a = fr::random_element();
    fr b = -a;

    auto a_idx = builder.add_variable(a);
    auto b_idx = builder.add_variable(b);
    auto c_idx = builder.add_variable(b);
    auto d_idx = builder.add_variable(a);

    builder.create_add_gate({ a_idx, b_idx, builder.zero_idx, fr::one(), fr::one(), fr::zero(), fr::zero() });
    builder.create_add_gate({ c_idx, d_idx, builder.zero_idx, fr::one(), fr::one(), fr::zero(), fr::zero() });

    builder.create_tag(1, 2);
    builder.create_tag(2, 1);

    builder.assign_tag(a_idx, 1);
    builder.assign_tag(b_idx, 1);
    builder.assign_tag(c_idx, 2);
    builder.assign_tag(d_idx, 2);

    // builder.create_add_gate({ a_idx, b_idx, builder.zero_idx, fr::one(), fr::neg_one(), fr::zero(), fr::zero()
    // }); builder.create_add_gate({ a_idx, b_idx, builder.zero_idx, fr::one(), fr::neg_one(), fr::zero(),
    // fr::zero() }); builder.create_add_gate({ a_idx, b_idx, builder.zero_idx, fr::one(), fr::neg_one(),
    // fr::zero(), fr::zero() });

    TestFixture::prove_and_verify(builder, composer, /*expected_result=*/true);
}

TYPED_TEST(ultra_plonk_composer, non_trivial_tag_permutation_and_cycles)
{
    auto builder = UltraCircuitBuilder();
    auto composer = UltraComposer();
    fr a = fr::random_element();
    fr c = -a;

    auto a_idx = builder.add_variable(a);
    auto b_idx = builder.add_variable(a);
    builder.assert_equal(a_idx, b_idx);
    auto c_idx = builder.add_variable(c);
    auto d_idx = builder.add_variable(c);
    builder.assert_equal(c_idx, d_idx);
    auto e_idx = builder.add_variable(a);
    auto f_idx = builder.add_variable(a);
    builder.assert_equal(e_idx, f_idx);
    auto g_idx = builder.add_variable(c);
    auto h_idx = builder.add_variable(c);
    builder.assert_equal(g_idx, h_idx);

    builder.create_tag(1, 2);
    builder.create_tag(2, 1);

    builder.assign_tag(a_idx, 1);
    builder.assign_tag(c_idx, 1);
    builder.assign_tag(e_idx, 2);
    builder.assign_tag(g_idx, 2);

    builder.create_add_gate({ b_idx, a_idx, builder.zero_idx, fr::one(), fr::neg_one(), fr::zero(), fr::zero() });
    builder.create_add_gate({ c_idx, g_idx, builder.zero_idx, fr::one(), -fr::one(), fr::zero(), fr::zero() });
    builder.create_add_gate({ e_idx, f_idx, builder.zero_idx, fr::one(), -fr::one(), fr::zero(), fr::zero() });

    // builder.create_add_gate({ a_idx, b_idx, builder.zero_idx, fr::one(), fr::neg_one(), fr::zero(), fr::zero()
    // }); builder.create_add_gate({ a_idx, b_idx, builder.zero_idx, fr::one(), fr::neg_one(), fr::zero(),
    // fr::zero() }); builder.create_add_gate({ a_idx, b_idx, builder.zero_idx, fr::one(), fr::neg_one(),
    // fr::zero(), fr::zero() });

    TestFixture::prove_and_verify(builder, composer, /*expected_result=*/true);
}

TYPED_TEST(ultra_plonk_composer, bad_tag_permutation)
{
    {
        auto builder = UltraCircuitBuilder();
        auto composer = UltraComposer();
        fr a = fr::random_element();
        fr b = -a;

        auto a_idx = builder.add_variable(a);
        auto b_idx = builder.add_variable(b);
        auto c_idx = builder.add_variable(b);
        auto d_idx = builder.add_variable(a + 1);

        builder.create_add_gate({ a_idx, b_idx, builder.zero_idx, 1, 1, 0, 0 });
        builder.create_add_gate({ c_idx, d_idx, builder.zero_idx, 1, 1, 0, -1 });

        builder.create_tag(1, 2);
        builder.create_tag(2, 1);

        builder.assign_tag(a_idx, 1);
        builder.assign_tag(b_idx, 1);
        builder.assign_tag(c_idx, 2);
        builder.assign_tag(d_idx, 2);

        TestFixture::prove_and_verify(builder, composer, /*expected_result=*/false);
    }
    // Same as above but without tag creation to check reason of failure is really tag mismatch
    {
        auto builder = UltraCircuitBuilder();
        auto composer = UltraComposer();
        fr a = fr::random_element();
        fr b = -a;

        auto a_idx = builder.add_variable(a);
        auto b_idx = builder.add_variable(b);
        auto c_idx = builder.add_variable(b);
        auto d_idx = builder.add_variable(a + 1);

        builder.create_add_gate({ a_idx, b_idx, builder.zero_idx, 1, 1, 0, 0 });
        builder.create_add_gate({ c_idx, d_idx, builder.zero_idx, 1, 1, 0, -1 });

        auto prover = composer.create_prover(builder);
        auto verifier = composer.create_verifier(builder);

        TestFixture::prove_and_verify(builder, composer, /*expected_result=*/true);
    }
}

TYPED_TEST(ultra_plonk_composer, sort_widget)
{
    auto builder = UltraCircuitBuilder();
    auto composer = UltraComposer();
    fr a = fr::one();
    fr b = fr(2);
    fr c = fr(3);
    fr d = fr(4);

    auto a_idx = builder.add_variable(a);
    auto b_idx = builder.add_variable(b);
    auto c_idx = builder.add_variable(c);
    auto d_idx = builder.add_variable(d);
    builder.create_sort_constraint({ a_idx, b_idx, c_idx, d_idx });

    TestFixture::prove_and_verify(builder, composer, /*expected_result=*/true);
}

TYPED_TEST(ultra_plonk_composer, sort_with_edges_gate)
{
    fr a = fr::one();
    fr b = fr(2);
    fr c = fr(3);
    fr d = fr(4);
    fr e = fr(5);
    fr f = fr(6);
    fr g = fr(7);
    fr h = fr(8);

    {
        auto builder = UltraCircuitBuilder();
        auto composer = UltraComposer();
        auto a_idx = builder.add_variable(a);
        auto b_idx = builder.add_variable(b);
        auto c_idx = builder.add_variable(c);
        auto d_idx = builder.add_variable(d);
        auto e_idx = builder.add_variable(e);
        auto f_idx = builder.add_variable(f);
        auto g_idx = builder.add_variable(g);
        auto h_idx = builder.add_variable(h);
        builder.create_sort_constraint_with_edges({ a_idx, b_idx, c_idx, d_idx, e_idx, f_idx, g_idx, h_idx }, a, h);

        TestFixture::prove_and_verify(builder, composer, /*expected_result=*/true);
    }

    {
        auto builder = UltraCircuitBuilder();
        auto composer = UltraComposer();
        auto a_idx = builder.add_variable(a);
        auto b_idx = builder.add_variable(b);
        auto c_idx = builder.add_variable(c);
        auto d_idx = builder.add_variable(d);
        auto e_idx = builder.add_variable(e);
        auto f_idx = builder.add_variable(f);
        auto g_idx = builder.add_variable(g);
        auto h_idx = builder.add_variable(h);
        builder.create_sort_constraint_with_edges({ a_idx, b_idx, c_idx, d_idx, e_idx, f_idx, g_idx, h_idx }, a, g);
        auto prover = composer.create_prover(builder);
        auto verifier = composer.create_verifier(builder);

        proof proof = prover.construct_proof();

        bool result = verifier.verify_proof(proof);
        EXPECT_EQ(result, false);
    }
    {
        auto builder = UltraCircuitBuilder();
        auto composer = UltraComposer();
        auto a_idx = builder.add_variable(a);
        auto b_idx = builder.add_variable(b);
        auto c_idx = builder.add_variable(c);
        auto d_idx = builder.add_variable(d);
        auto e_idx = builder.add_variable(e);
        auto f_idx = builder.add_variable(f);
        auto g_idx = builder.add_variable(g);
        auto h_idx = builder.add_variable(h);
        builder.create_sort_constraint_with_edges({ a_idx, b_idx, c_idx, d_idx, e_idx, f_idx, g_idx, h_idx }, b, h);

        TestFixture::prove_and_verify(builder, composer, /*expected_result=*/false);
    }
    {
        auto builder = UltraCircuitBuilder();
        auto composer = UltraComposer();
        auto a_idx = builder.add_variable(a);
        auto c_idx = builder.add_variable(c);
        auto d_idx = builder.add_variable(d);
        auto e_idx = builder.add_variable(e);
        auto f_idx = builder.add_variable(f);
        auto g_idx = builder.add_variable(g);
        auto h_idx = builder.add_variable(h);
        auto b2_idx = builder.add_variable(fr(15));
        builder.create_sort_constraint_with_edges({ a_idx, b2_idx, c_idx, d_idx, e_idx, f_idx, g_idx, h_idx }, b, h);

        TestFixture::prove_and_verify(builder, composer, /*expected_result=*/false);
    }
    {
        auto builder = UltraCircuitBuilder();
        auto composer = UltraComposer();
        auto idx = add_variables(builder, { 1,  2,  5,  6,  7,  10, 11, 13, 16, 17, 20, 22, 22, 25,
                                            26, 29, 29, 32, 32, 33, 35, 38, 39, 39, 42, 42, 43, 45 });
        builder.create_sort_constraint_with_edges(idx, 1, 45);

        TestFixture::prove_and_verify(builder, composer, /*expected_result=*/true);
    }
    {
        auto builder = UltraCircuitBuilder();
        auto composer = UltraComposer();
        auto idx = add_variables(builder, { 1,  2,  5,  6,  7,  10, 11, 13, 16, 17, 20, 22, 22, 25,
                                            26, 29, 29, 32, 32, 33, 35, 38, 39, 39, 42, 42, 43, 45 });

        builder.create_sort_constraint_with_edges(idx, 1, 29);

        TestFixture::prove_and_verify(builder, composer, /*expected_result=*/false);
    }
}

TYPED_TEST(ultra_plonk_composer, range_constraint)
{
    {
        auto builder = UltraCircuitBuilder();
        auto composer = UltraComposer();
        auto indices = add_variables(builder, { 1, 2, 3, 4, 5, 6, 7, 8 });
        for (size_t i = 0; i < indices.size(); i++) {
            builder.create_new_range_constraint(indices[i], 8);
        }
        // auto ind = {a_idx,b_idx,c_idx,d_idx,e_idx,f_idx,g_idx,h_idx};
        builder.create_sort_constraint(indices);
        auto prover = composer.create_prover(builder);
        auto verifier = composer.create_verifier(builder);

        proof proof = prover.construct_proof();

        bool result = verifier.verify_proof(proof);
        EXPECT_EQ(result, true);
    }
    {
        auto builder = UltraCircuitBuilder();
        auto composer = UltraComposer();
        auto indices = add_variables(builder, { 3 });
        for (size_t i = 0; i < indices.size(); i++) {
            builder.create_new_range_constraint(indices[i], 3);
        }
        // auto ind = {a_idx,b_idx,c_idx,d_idx,e_idx,f_idx,g_idx,h_idx};
        builder.create_dummy_constraints(indices);

        TestFixture::prove_and_verify(builder, composer, /*expected_result=*/true);
    }
    {
        auto builder = UltraCircuitBuilder();
        auto composer = UltraComposer();
        auto indices = add_variables(builder, { 1, 2, 3, 4, 5, 6, 8, 25 });
        for (size_t i = 0; i < indices.size(); i++) {
            builder.create_new_range_constraint(indices[i], 8);
        }
        builder.create_sort_constraint(indices);

        TestFixture::prove_and_verify(builder, composer, /*expected_result=*/false);
    }
    {
        auto builder = UltraCircuitBuilder();
        auto composer = UltraComposer();
        auto indices =
            add_variables(builder, { 1, 2, 3, 4, 5, 6, 10, 8, 15, 11, 32, 21, 42, 79, 16, 10, 3, 26, 19, 51 });
        for (size_t i = 0; i < indices.size(); i++) {
            builder.create_new_range_constraint(indices[i], 128);
        }
        builder.create_dummy_constraints(indices);

        TestFixture::prove_and_verify(builder, composer, /*expected_result=*/true);
    }
    {
        auto builder = UltraCircuitBuilder();
        auto composer = UltraComposer();
        auto indices =
            add_variables(builder, { 1, 2, 3, 80, 5, 6, 29, 8, 15, 11, 32, 21, 42, 79, 16, 10, 3, 26, 13, 14 });
        for (size_t i = 0; i < indices.size(); i++) {
            builder.create_new_range_constraint(indices[i], 79);
        }
        builder.create_dummy_constraints(indices);
        auto prover = composer.create_prover(builder);
        auto verifier = composer.create_verifier(builder);

        proof proof = prover.construct_proof();

        bool result = verifier.verify_proof(proof);
        EXPECT_EQ(result, false);
    }
    {
        auto builder = UltraCircuitBuilder();
        auto composer = UltraComposer();
        auto indices =
            add_variables(builder, { 1, 0, 3, 80, 5, 6, 29, 8, 15, 11, 32, 21, 42, 79, 16, 10, 3, 26, 13, 14 });
        for (size_t i = 0; i < indices.size(); i++) {
            builder.create_new_range_constraint(indices[i], 79);
        }
        builder.create_dummy_constraints(indices);

        TestFixture::prove_and_verify(builder, composer, /*expected_result=*/false);
    }
}

TYPED_TEST(ultra_plonk_composer, range_with_gates)
{
    auto builder = UltraCircuitBuilder();
    auto composer = UltraComposer();
    auto idx = add_variables(builder, { 1, 2, 3, 4, 5, 6, 7, 8 });
    for (size_t i = 0; i < idx.size(); i++) {
        builder.create_new_range_constraint(idx[i], 8);
    }

    builder.create_add_gate({ idx[0], idx[1], builder.zero_idx, fr::one(), fr::one(), fr::zero(), -3 });
    builder.create_add_gate({ idx[2], idx[3], builder.zero_idx, fr::one(), fr::one(), fr::zero(), -7 });
    builder.create_add_gate({ idx[4], idx[5], builder.zero_idx, fr::one(), fr::one(), fr::zero(), -11 });
    builder.create_add_gate({ idx[6], idx[7], builder.zero_idx, fr::one(), fr::one(), fr::zero(), -15 });

    TestFixture::prove_and_verify(builder, composer, /*expected_result=*/true);
}

TYPED_TEST(ultra_plonk_composer, range_with_gates_where_range_is_not_a_power_of_two)
{
    auto builder = UltraCircuitBuilder();
    auto composer = UltraComposer();
    auto idx = add_variables(builder, { 1, 2, 3, 4, 5, 6, 7, 8 });
    for (size_t i = 0; i < idx.size(); i++) {
        builder.create_new_range_constraint(idx[i], 12);
    }

    builder.create_add_gate({ idx[0], idx[1], builder.zero_idx, fr::one(), fr::one(), fr::zero(), -3 });
    builder.create_add_gate({ idx[2], idx[3], builder.zero_idx, fr::one(), fr::one(), fr::zero(), -7 });
    builder.create_add_gate({ idx[4], idx[5], builder.zero_idx, fr::one(), fr::one(), fr::zero(), -11 });
    builder.create_add_gate({ idx[6], idx[7], builder.zero_idx, fr::one(), fr::one(), fr::zero(), -15 });

    TestFixture::prove_and_verify(builder, composer, /*expected_result=*/true);
}

TYPED_TEST(ultra_plonk_composer, sort_widget_complex)
{
    {

        auto builder = UltraCircuitBuilder();
        auto composer = UltraComposer();
        std::vector<fr> a = { 1, 3, 4, 7, 7, 8, 11, 14, 15, 15, 18, 19, 21, 21, 24, 25, 26, 27, 30, 32 };
        std::vector<uint32_t> ind;
        for (size_t i = 0; i < a.size(); i++)
            ind.emplace_back(builder.add_variable(a[i]));
        builder.create_sort_constraint(ind);
        auto prover = composer.create_prover(builder);
        auto verifier = composer.create_verifier(builder);

        proof proof = prover.construct_proof();

        bool result = verifier.verify_proof(proof);
        EXPECT_EQ(result, true);
    }
    {

        auto builder = UltraCircuitBuilder();
        auto composer = UltraComposer();
        std::vector<fr> a = { 1, 3, 4, 7, 7, 8, 16, 14, 15, 15, 18, 19, 21, 21, 24, 25, 26, 27, 30, 32 };
        std::vector<uint32_t> ind;
        for (size_t i = 0; i < a.size(); i++)
            ind.emplace_back(builder.add_variable(a[i]));
        builder.create_sort_constraint(ind);

        TestFixture::prove_and_verify(builder, composer, /*expected_result=*/false);
    }
}
TYPED_TEST(ultra_plonk_composer, sort_widget_neg)
{
    auto builder = UltraCircuitBuilder();
    auto composer = UltraComposer();
    fr a = fr::one();
    fr b = fr(2);
    fr c = fr(3);
    fr d = fr(8);

    auto a_idx = builder.add_variable(a);
    auto b_idx = builder.add_variable(b);
    auto c_idx = builder.add_variable(c);
    auto d_idx = builder.add_variable(d);
    builder.create_sort_constraint({ a_idx, b_idx, c_idx, d_idx });

    TestFixture::prove_and_verify(builder, composer, /*expected_result=*/false);
}

TYPED_TEST(ultra_plonk_composer, composed_range_constraint)
{
    auto builder = UltraCircuitBuilder();
    auto composer = UltraComposer();
    auto c = fr::random_element();
    auto d = uint256_t(c).slice(0, 133);
    auto e = fr(d);
    auto a_idx = builder.add_variable(fr(e));
    builder.create_add_gate({ a_idx, builder.zero_idx, builder.zero_idx, 1, 0, 0, -fr(e) });
    builder.decompose_into_default_range(a_idx, 134);

    TestFixture::prove_and_verify(builder, composer, /*expected_result=*/true);
}

TYPED_TEST(ultra_plonk_composer, non_native_field_multiplication)
{
    auto builder = UltraCircuitBuilder();
    auto composer = UltraComposer();

    fq a = fq::random_element();
    fq b = fq::random_element();
    uint256_t modulus = fq::modulus;

    uint1024_t a_big = uint512_t(uint256_t(a));
    uint1024_t b_big = uint512_t(uint256_t(b));
    uint1024_t p_big = uint512_t(uint256_t(modulus));

    uint1024_t q_big = (a_big * b_big) / p_big;
    uint1024_t r_big = (a_big * b_big) % p_big;

    uint256_t q(q_big.lo.lo);
    uint256_t r(r_big.lo.lo);

    const auto split_into_limbs = [&](const uint512_t& input) {
        constexpr size_t NUM_BITS = 68;
        std::array<fr, 5> limbs;
        limbs[0] = input.slice(0, NUM_BITS).lo;
        limbs[1] = input.slice(NUM_BITS * 1, NUM_BITS * 2).lo;
        limbs[2] = input.slice(NUM_BITS * 2, NUM_BITS * 3).lo;
        limbs[3] = input.slice(NUM_BITS * 3, NUM_BITS * 4).lo;
        limbs[4] = fr(input.lo);
        return limbs;
    };

    const auto get_limb_witness_indices = [&](const std::array<fr, 5>& limbs) {
        std::array<uint32_t, 5> limb_indices;
        limb_indices[0] = builder.add_variable(limbs[0]);
        limb_indices[1] = builder.add_variable(limbs[1]);
        limb_indices[2] = builder.add_variable(limbs[2]);
        limb_indices[3] = builder.add_variable(limbs[3]);
        limb_indices[4] = builder.add_variable(limbs[4]);
        return limb_indices;
    };
    const uint512_t BINARY_BASIS_MODULUS = uint512_t(1) << (68 * 4);
    auto modulus_limbs = split_into_limbs(BINARY_BASIS_MODULUS - uint512_t(modulus));

    const auto a_indices = get_limb_witness_indices(split_into_limbs(uint256_t(a)));
    const auto b_indices = get_limb_witness_indices(split_into_limbs(uint256_t(b)));
    const auto q_indices = get_limb_witness_indices(split_into_limbs(uint256_t(q)));
    const auto r_indices = get_limb_witness_indices(split_into_limbs(uint256_t(r)));

    proof_system::UltraCircuitBuilder::non_native_field_witnesses inputs{
        a_indices, b_indices, q_indices, r_indices, modulus_limbs, fr(uint256_t(modulus)),
    };
    const auto [lo_1_idx, hi_1_idx] = builder.evaluate_non_native_field_multiplication(inputs);
    builder.range_constrain_two_limbs(lo_1_idx, hi_1_idx, 70, 70);

    TestFixture::prove_and_verify(builder, composer, /*expected_result=*/true);
}

TYPED_TEST(ultra_plonk_composer, rom)
{
    auto builder = UltraCircuitBuilder();
    auto composer = UltraComposer();

    uint32_t rom_values[8]{
        builder.add_variable(fr::random_element()), builder.add_variable(fr::random_element()),
        builder.add_variable(fr::random_element()), builder.add_variable(fr::random_element()),
        builder.add_variable(fr::random_element()), builder.add_variable(fr::random_element()),
        builder.add_variable(fr::random_element()), builder.add_variable(fr::random_element()),
    };

    size_t rom_id = builder.create_ROM_array(8);

    for (size_t i = 0; i < 8; ++i) {
        builder.set_ROM_element(rom_id, i, rom_values[i]);
    }

    uint32_t a_idx = builder.read_ROM_array(rom_id, builder.add_variable(5));
    EXPECT_EQ(a_idx != rom_values[5], true);
    uint32_t b_idx = builder.read_ROM_array(rom_id, builder.add_variable(4));
    uint32_t c_idx = builder.read_ROM_array(rom_id, builder.add_variable(1));

    const auto d_value = builder.get_variable(a_idx) + builder.get_variable(b_idx) + builder.get_variable(c_idx);
    uint32_t d_idx = builder.add_variable(d_value);

    builder.create_big_add_gate({
        a_idx,
        b_idx,
        c_idx,
        d_idx,
        1,
        1,
        1,
        -1,
        0,
    });

    TestFixture::prove_and_verify(builder, composer, /*expected_result=*/true);
}

TYPED_TEST(ultra_plonk_composer, ram)
{
    auto builder = UltraCircuitBuilder();
    auto composer = UltraComposer();

    uint32_t ram_values[8]{
        builder.add_variable(fr::random_element()), builder.add_variable(fr::random_element()),
        builder.add_variable(fr::random_element()), builder.add_variable(fr::random_element()),
        builder.add_variable(fr::random_element()), builder.add_variable(fr::random_element()),
        builder.add_variable(fr::random_element()), builder.add_variable(fr::random_element()),
    };

    size_t ram_id = builder.create_RAM_array(8);

    for (size_t i = 0; i < 8; ++i) {
        builder.init_RAM_element(ram_id, i, ram_values[i]);
    }

    uint32_t a_idx = builder.read_RAM_array(ram_id, builder.add_variable(5));
    EXPECT_EQ(a_idx != ram_values[5], true);

    uint32_t b_idx = builder.read_RAM_array(ram_id, builder.add_variable(4));
    uint32_t c_idx = builder.read_RAM_array(ram_id, builder.add_variable(1));

    builder.write_RAM_array(ram_id, builder.add_variable(4), builder.add_variable(500));
    uint32_t d_idx = builder.read_RAM_array(ram_id, builder.add_variable(4));

    EXPECT_EQ(builder.get_variable(d_idx), 500);

    // ensure these vars get used in another arithmetic gate
    const auto e_value = builder.get_variable(a_idx) + builder.get_variable(b_idx) + builder.get_variable(c_idx) +
                         builder.get_variable(d_idx);
    uint32_t e_idx = builder.add_variable(e_value);

    builder.create_big_add_gate(
        {
            a_idx,
            b_idx,
            c_idx,
            d_idx,
            -1,
            -1,
            -1,
            -1,
            0,
        },
        true);
    builder.create_big_add_gate(
        {
            builder.zero_idx,
            builder.zero_idx,
            builder.zero_idx,
            e_idx,
            0,
            0,
            0,
            0,
            0,
        },
        false);

    TestFixture::prove_and_verify(builder, composer, /*expected_result=*/true);
}

TYPED_TEST(ultra_plonk_composer, range_checks_on_duplicates)
{
    auto builder = UltraCircuitBuilder();
    auto composer = UltraComposer();

    uint32_t a = builder.add_variable(100);
    uint32_t b = builder.add_variable(100);
    uint32_t c = builder.add_variable(100);
    uint32_t d = builder.add_variable(100);

    builder.assert_equal(a, b);
    builder.assert_equal(a, c);
    builder.assert_equal(a, d);

    builder.create_new_range_constraint(a, 1000);
    builder.create_new_range_constraint(b, 1001);
    builder.create_new_range_constraint(c, 999);
    builder.create_new_range_constraint(d, 1000);

    builder.create_big_add_gate(
        {
            a,
            b,
            c,
            d,
            0,
            0,
            0,
            0,
            0,
        },
        false);

    TestFixture::prove_and_verify(builder, composer, /*expected_result=*/true);
}

// Ensure copy constraints added on variables smaller than 2^14, which have been previously
// range constrained, do not break the set equivalence checks because of indices mismatch.
// 2^14 is DEFAULT_PLOOKUP_RANGE_BITNUM i.e. the maximum size before a variable gets sliced
// before range constraints are applied to it.
TEST(ultra_plonk_composer, range_constraint_small_variable)
{
    auto builder = UltraCircuitBuilder();
    auto composer = UltraComposer();
    uint16_t mask = (1 << 8) - 1;
    int a = engine.get_random_uint16() & mask;
    uint32_t a_idx = builder.add_variable(fr(a));
    uint32_t b_idx = builder.add_variable(fr(a));
    ASSERT_NE(a_idx, b_idx);
    uint32_t c_idx = builder.add_variable(fr(a));
    ASSERT_NE(c_idx, b_idx);
    builder.create_range_constraint(b_idx, 8, "bad range");
    builder.assert_equal(a_idx, b_idx);
    builder.create_range_constraint(c_idx, 8, "bad range");
    builder.assert_equal(a_idx, c_idx);

    auto prover = composer.create_prover(builder);
    auto proof = prover.construct_proof();
    auto verifier = composer.create_verifier(builder);
    bool result = verifier.verify_proof(proof);
    EXPECT_EQ(result, true);
}

} // namespace proof_system::plonk::test_ultra_plonk_composer
