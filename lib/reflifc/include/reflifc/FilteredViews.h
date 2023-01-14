#pragma once

#include "Module.h"
#include "Namespace.h"
#include "Struct.h"

namespace reflifc
{
    inline RangeOf<Namespace> auto get_namespaces(Module module)
    {
        return module.scope_declarations()
            | std::views::filter(&ScopeDeclaration::is_namespace)
            | std::views::transform(&ScopeDeclaration::as_namespace);
    }

    inline RangeOf<ScopeDeclaration> auto get_scope_declarations(Namespace ns)
    {
        return ns.get_declarations()
            | std::views::filter(&Declaration::is_scope)
            | std::views::transform(&Declaration::as_scope);
    }

    inline RangeOf<Struct> auto get_structs(Namespace ns)
    {
        return get_scope_declarations(ns)
            | std::views::filter(&ScopeDeclaration::is_struct)
            | std::views::transform(&ScopeDeclaration::as_struct);
    }
}
