#pragma once

#include "Declaration.h"
#include "RangeOf.h"

#include <ifc/File.h>
#include <ifc/Declaration.h>

namespace reflifc
{
    struct Struct
    {
        Struct(ifc::File const& ifc, ifc::ScopeDeclaration const& scope)
            : ifc_(ifc)
            , scope_(scope)
        {
        }

        bool is_complete() const;

        RangeOf<Declaration> auto members() const
        {
            assert(is_complete());
            return get_declarations(ifc_, ifc_.scope_descriptors()[scope_.initializer])
                | std::views::transform([&ifc = ifc_] (ifc::Declaration decl) { return Declaration(ifc, decl.index); });
        }

        std::vector<Struct> bases() const;

    private:
        ifc::File const & ifc_;
        ifc::ScopeDeclaration const& scope_;
    };
}
