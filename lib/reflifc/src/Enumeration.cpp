#include "reflifc/Enumeration.h"

#include "reflifc/Type.h"

namespace reflifc
{
    char const* Enumeration::name() const
    {
        return ifc_.get_string(enum_.name);
    }

    Type Enumeration::underlying_type() const
    {
        return { ifc_, enum_.base };
    }
}
