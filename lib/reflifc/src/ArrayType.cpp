#include "reflifc/ArrayType.h"

#include "reflifc/Expression.h"
#include "reflifc/Type.h"

#include <ifc/Type.h>

namespace reflifc
{
    Type ArrayType::element() const
    {
        return { ifc_, array_.element };
    }

    Expression ArrayType::extent() const
    {
        return { ifc_, array_.extent };
    }
}
