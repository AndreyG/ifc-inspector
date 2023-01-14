#pragma once

#include "Declaration.h"
#include "Name.h"
#include "RangeOf.h"

#include <ifc/File.h>
#include <ifc/Declaration.h>

namespace reflifc
{
    struct Namespace
    {
        Namespace(ifc::File const& ifc, ifc::ScopeDeclaration const& scope)
            : ifc_(ifc)
            , scope_(scope)
        {
        }

        Name name() const { return { ifc_, scope_.name }; }

        RangeOf<Declaration> auto get_declarations() const
        {
            const auto scope_members = ifc_.scope_descriptors()[scope_.initializer];
            return ifc::get_declarations(ifc_, scope_members)
                | std::views::transform([&ifc = ifc_] (ifc::Declaration decl) { return Declaration(ifc, decl.index); });
        }

    private:
        ifc::File const & ifc_;
        ifc::ScopeDeclaration const& scope_;
    };
}
