#include "reflifc/Type.h"

#include "reflifc/ArrayType.h"
#include "reflifc/Declaration.h"
#include "reflifc/SyntaticType.h"

#include <ifc/File.h>
#include <ifc/Type.h>
#include <ifc/Expression.h>

namespace reflifc
{
    bool Type::is_fundamental() const
    {
        return index_.sort() == ifc::TypeSort::Fundamental;
    }

    ifc::FundamentalType Type::as_fundamental() const
    {
        return ifc_.fundamental_types()[index_];
    }

    bool Type::is_array() const
    {
        return index_.sort() == ifc::TypeSort::Array;
    }

    ArrayType Type::as_array() const
    {
        return { ifc_, ifc_.array_types()[index_] };
    }

    bool Type::is_designated() const
    {
        return index_.sort() == ifc::TypeSort::Designated;
    }

    Declaration Type::designation() const
    {
        return { ifc_, ifc_.designated_types()[index_].decl };
    }

    bool Type::is_syntactic() const
    {
        return index_.sort() == ifc::TypeSort::Syntactic;
    }

    SyntaticType Type::as_syntatic() const
    {
        return { ifc_, ifc_.template_ids()[ifc_.syntactic_types()[index_].expr] };
    }
}
