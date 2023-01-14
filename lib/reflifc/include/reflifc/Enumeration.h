#pragma once

#include "Enumerator.h"
#include "RangeOf.h"

#include <ifc/Declaration.h>
#include <ifc/File.h>

namespace reflifc
{
    struct Type;

    struct Enumeration
    {
        Enumeration(ifc::File const& ifc, ifc::Enumeration const& enum_)
            : ifc_(ifc)
            , enum_(enum_)
        {
        }

        char const* name() const;

        RangeOf<Enumerator> auto enumerators() const
        {
            return ifc_.enumerators().slice(enum_.initializer)
                | std::views::transform([&ifc = ifc_] (ifc::Enumerator const& enumerator) { return Enumerator(ifc, enumerator); });
        }

        Type underlying_type() const;

    private:
        ifc::File const & ifc_;
        ifc::Enumeration const& enum_;
    };
}
