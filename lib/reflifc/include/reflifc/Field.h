#pragma once

#include <ifc/DeclarationFwd.h>
#include <ifc/FileFwd.h>

namespace reflifc
{
    struct Type;

    struct Field
    {
        Field(ifc::File const& ifc, ifc::FieldDeclaration const& field)
            : ifc_(ifc)
            , field_(field)
        {
        }

        const char* name() const;
        Type        type() const;

    private:
        ifc::File const & ifc_;
        ifc::FieldDeclaration const& field_;
    };
}
