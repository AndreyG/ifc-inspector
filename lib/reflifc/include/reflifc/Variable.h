#pragma once

#include "Name.h"

#include <ifc/DeclarationFwd.h>
#include <ifc/FileFwd.h>

namespace reflifc
{
    struct Expression;

    struct Variable
    {
        Variable(ifc::File const& ifc, ifc::VariableDeclaration const& var)
            : ifc_(ifc)
            , var_(var)
        {
        }

        Name name() const;

        Expression ct_value() const;

    private:
        ifc::File const & ifc_;
        ifc::VariableDeclaration const& var_;
    };
}
