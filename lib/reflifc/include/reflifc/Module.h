#pragma once

#include "RangeOf.h"
#include "ScopeDeclaration.h"

#include <ifc/File.h>

namespace reflifc
{
    struct Module
    {
        explicit Module(ifc::File const& ifc)
            : ifc_(ifc)
        {
        }

        RangeOf<ScopeDeclaration> auto scope_declarations() const
        {
            return ifc_.scope_declarations()
                | std::views::transform([&ifc = ifc_] (ifc::ScopeDeclaration const & scope) {
                    return ScopeDeclaration(ifc, scope);
                });
        }

    private:
        ifc::File const & ifc_;
    };
}
