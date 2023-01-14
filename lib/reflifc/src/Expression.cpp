#include "reflifc/Expression.h"
#include "reflifc/DyadExpression.h"
#include "reflifc/Literal.h"
#include "reflifc/StringLiteral.h"
#include "reflifc/Type.h"

#include <ifc/Expression.h>
#include <ifc/File.h>

namespace reflifc
{
    bool Expression::is_dyad() const
    {
        return index_.sort() == ifc::ExprSort::Dyad;
    }

    DyadExpression Expression::as_dyad() const
    {
        return { ifc_, ifc_.dyad_expressions()[index_] };
    }

    bool Expression::is_literal() const
    {
        return index_.sort() == ifc::ExprSort::Literal;
    }

    Literal Expression::as_literal() const
    {
        return { ifc_, ifc_.literal_expressions()[index_].value };
    }

    bool Expression::is_string_literal() const
    {
        return index_.sort() == ifc::ExprSort::String;
    }

    StringLiteral Expression::as_string_literal() const
    {
        return { ifc_, ifc_.string_literal_expressions()[ifc_.string_expressions()[index_].string_index] };
    }

    bool Expression::is_type() const
    {
        return index_.sort() == ifc::ExprSort::Type;
    }

    Type Expression::as_type() const
    {
        return { ifc_, ifc_.type_expressions()[index_].denotation };
    }
}
