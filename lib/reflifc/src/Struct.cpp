#include "reflifc/Struct.h"

#include <ifc/Type.h>

namespace reflifc
{
    bool Struct::is_complete() const
    {
        return !is_null(scope_.initializer);
    }

    static Struct get_base(ifc::File const & file_, ifc::TypeIndex type)
    {
        assert(type.sort() == ifc::TypeSort::Base);
        const auto base_type = file_.base_types()[type].type;
        auto const& base_decl = file_.scope_declarations()[file_.designated_types()[base_type].decl];
        return Struct(file_, base_decl);
    }

    std::vector<Struct> Struct::bases() const
    {
        auto base = scope_.base;
        if (base.is_null())
            return {};

        if (base.sort() == ifc::TypeSort::Base)
            return { get_base(ifc_, base) };

        auto tuple = ifc_.tuple_types()[base];
        std::vector<Struct> result;
        result.reserve(raw_count(tuple.cardinality));
        for (auto type : ifc_.type_heap().slice(tuple))
            result.push_back(get_base(ifc_, type));
        return result;
    }
}
