#pragma once

#include <ifc/ExpressionFwd.h>
#include <ifc/FileFwd.h>

namespace reflifc
{
    struct DyadExpression;
    struct Literal;
    struct StringLiteral;
    struct Type;

    struct Expression
    {
        Expression(ifc::File const& ifc, ifc::ExprIndex index)
            : ifc_(ifc)
            , index_(index)
        {
        }

        bool            is_dyad() const;
        DyadExpression  as_dyad() const;

        bool            is_literal() const;
        Literal         as_literal() const;

        bool            is_string_literal() const;
        StringLiteral   as_string_literal() const;

        bool            is_type() const;
        Type            as_type() const;

    private:
        ifc::File const& ifc_;
        ifc::ExprIndex index_;
    };
}
