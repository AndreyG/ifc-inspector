#include "reflifc/Variable.h"

#include "reflifc/Expression.h"

#include <ifc/Declaration.h>

namespace reflifc
{
    Name Variable::name() const
    {
        return { ifc_, var_.name };
    }

    Expression Variable::ct_value() const
    {
        return { ifc_, var_.initializer };
    }
}
