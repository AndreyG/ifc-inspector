#pragma once

#include "Name.h"

#include <ifc/FileFwd.h>
#include <ifc/Declaration.h>

namespace reflifc
{
    struct Namespace;
    struct Struct;

    struct ScopeDeclaration
    {
        ScopeDeclaration(ifc::File const& ifc, ifc::ScopeDeclaration const& scope)
            : ifc_(ifc)
            , scope_(scope)
        {
        }

        Name name() const;

        bool        is_namespace() const;
        Namespace   as_namespace() const;

        bool        is_struct() const;
        Struct      as_struct() const;

    private:
        ifc::File const & ifc_;
        ifc::ScopeDeclaration const& scope_;
    };
}
