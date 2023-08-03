#pragma once

#include "../../append_only_tree_snapshot.hpp"
#include "../constant_rollup_data.hpp"

#include "aztec3/constants.hpp"

namespace aztec3::circuits::abis {


const uint32_t BASE_ROLLUP_TYPE = 0;
const uint32_t MERGE_ROLLUP_TYPE = 1;

template <typename NCT> struct BaseOrMergeRollupPublicInputs {
    using fr = typename NCT::fr;
    using AggregationObject = typename NCT::AggregationObject;

    uint32_t rollup_type;
    // subtree  height is always 0 for base.
    // so that we always pass-in two base/merge circuits of the same height into the next level of recursion
    fr rollup_subtree_height;

    AggregationObject end_aggregation_object;
    ConstantRollupData<NCT> constants;

    AppendOnlyTreeSnapshot<NCT> start_private_data_tree_snapshot;
    AppendOnlyTreeSnapshot<NCT> end_private_data_tree_snapshot;

    AppendOnlyTreeSnapshot<NCT> start_nullifier_tree_snapshot;
    AppendOnlyTreeSnapshot<NCT> end_nullifier_tree_snapshot;

    AppendOnlyTreeSnapshot<NCT> start_contract_tree_snapshot;
    AppendOnlyTreeSnapshot<NCT> end_contract_tree_snapshot;

    fr start_public_data_tree_root;
    fr end_public_data_tree_root;

    // We hash public inputs to make them constant-sized (to then be unpacked on-chain)
    std::array<fr, NUM_FIELDS_PER_SHA256> calldata_hash;

    // For serialization, update with new fields
    MSGPACK_FIELDS(TODO add all fields);
    bool operator==(BaseOrMergeRollupPublicInputs<NCT> const&) const = default;
};

template <typename NCT> std::ostream& operator<<(std::ostream& os, BaseOrMergeRollupPublicInputs<NCT> const& obj)
{
    return os << "rollup_type:\n"
              << obj.rollup_type << "\n"
              << "rollup_subtree_height:\n"
              << obj.rollup_subtree_height << "\n"
              << "end_aggregation_object:\n"
              << obj.end_aggregation_object
              << "\n"
                 "constants:\n"
              << obj.constants
              << "\n"
                 "start_private_data_tree_snapshot:\n"
              << obj.start_private_data_tree_snapshot
              << "\n"
                 "end_private_data_tree_snapshot:\n"
              << obj.start_private_data_tree_snapshot
              << "\n"
                 "start_nullifier_tree_snapshot:\n"
              << obj.start_nullifier_tree_snapshot
              << "\n"
                 "end_nullifier_tree_snapshot:\n"
              << obj.end_nullifier_tree_snapshot
              << "\n"
                 "start_contract_tree_snapshot:\n"
              << obj.start_contract_tree_snapshot
              << "\n"
                 "end_contract_tree_snapshot:\n"
              << obj.end_contract_tree_snapshot
              << "\n"
                 "start_public_data_tree_root:\n"
              << obj.start_public_data_tree_root
              << "\n"
                 "end_public_data_tree_root:\n"
              << obj.end_public_data_tree_root
              << "\n"
                 "calldata_hash: "
              << obj.calldata_hash << "\n";
}

}  // namespace aztec3::circuits::abis
