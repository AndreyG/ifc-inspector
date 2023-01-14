#pragma once

#include <ifc/DeclarationFwd.h>
#include <ifc/FileFwd.h>

namespace reflifc
{
    struct Expression;

    struct Enumerator
    {
        Enumerator(ifc::File const& ifc, ifc::Enumerator const& enumerator)
            : ifc_(ifc)
            , enumerator_(enumerator)
        {
        }

        const char* name() const;

        Expression value() const;

    private:
        ifc::File const & ifc_;
        ifc::Enumerator const& enumerator_;
    };
}
