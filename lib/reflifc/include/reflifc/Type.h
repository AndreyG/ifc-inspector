#pragma once

#include <ifc/FileFwd.h>
#include <ifc/TypeFwd.h>

namespace reflifc
{
    struct ArrayType;
    struct Declaration;
    struct SyntaticType;

    struct Type
    {
        Type(ifc::File const& ifc, ifc::TypeIndex index)
            : ifc_(ifc)
            , index_(index)
        {
        }

        bool                    is_fundamental() const;
        ifc::FundamentalType    as_fundamental() const;

        bool        is_array() const;
        ArrayType   as_array() const;

        bool        is_designated() const;
        Declaration designation()   const;

        bool            is_syntactic() const;
        SyntaticType    as_syntatic()  const;

    private:
        ifc::File const& ifc_;
        ifc::TypeIndex index_;
    };
}
