#pragma once

#include "aztec3/utils/types/circuit_types.hpp"
#include "aztec3/utils/types/convert.hpp"
#include "aztec3/utils/types/native_types.hpp"

#include <barretenberg/barretenberg.hpp>

namespace aztec3::circuits::apps::notes {

using aztec3::utils::types::CircuitTypes;
using aztec3::utils::types::NativeTypes;

template <typename NCT> struct DefaultPrivateNoteNullifierPreimage {
    using fr = typename NCT::fr;
    using boolean = typename NCT::boolean;

    fr commitment;
    fr owner_private_key;
    boolean is_dummy = false;
    // For serialization, update with new fields
    MSGPACK_FIELDS(TODO list all fields here);

    bool operator==(DefaultPrivateNoteNullifierPreimage<NCT> const&) const = default;

    template <typename Builder>
    DefaultPrivateNoteNullifierPreimage<CircuitTypes<Builder>> to_circuit_type(Builder& builder) const
    {
        static_assert((std::is_same<NativeTypes, NCT>::value));

        // Capture the circuit builder:
        auto to_ct = [&](auto& e) { return aztec3::utils::types::to_ct(builder, e); };

        DefaultPrivateNoteNullifierPreimage<CircuitTypes<Builder>> preimage = {
            to_ct(commitment),
            to_ct(owner_private_key),
            to_ct(is_dummy),
        };

        return preimage;
    };
};

}  // namespace aztec3::circuits::apps::notes
