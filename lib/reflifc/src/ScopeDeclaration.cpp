#include "reflifc/ScopeDeclaration.h"

#include "reflifc/Namespace.h"
#include "reflifc/Struct.h"

#include <ifc/Type.h>

namespace reflifc
{
    Name ScopeDeclaration::name() const
    {
        return { ifc_, scope_.name };
    }

    bool ScopeDeclaration::is_namespace() const
    {
        return get_kind(scope_, ifc_) == ifc::TypeBasis::Namespace;
    }

    Namespace ScopeDeclaration::as_namespace() const
    {
        return { ifc_, scope_ };
    }

    bool ScopeDeclaration::is_struct() const
    {
        return get_kind(scope_, ifc_) == ifc::TypeBasis::Struct;
    }

    Struct ScopeDeclaration::as_struct() const
    {
        assert(is_struct());
        return { ifc_, scope_ };
    }
}
