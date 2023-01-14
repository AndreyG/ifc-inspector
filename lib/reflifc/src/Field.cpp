#include "reflifc/Field.h"

#include "reflifc/Type.h"

#include <ifc/Declaration.h>
#include <ifc/File.h>

namespace reflifc
{
    const char* Field::name() const
    {
        return ifc_.get_string(field_.name);
    }

    Type Field::type() const
    {
        return { ifc_, field_.type };
    }
}
