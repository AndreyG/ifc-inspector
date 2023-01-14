#include "reflifc/Declaration.h"

#include "reflifc/Enumeration.h"
#include "reflifc/Field.h"
#include "reflifc/ScopeDeclaration.h"
#include "reflifc/TemplateDeclaration.h"
#include "reflifc/Variable.h"

#include <ifc/Declaration.h>
#include <ifc/File.h>

namespace reflifc
{
    bool Declaration::is_scope() const
    {
        return index_.sort() == ifc::DeclSort::Scope;
    }

    ScopeDeclaration Declaration::as_scope() const
    {
        return { ifc_, ifc_.scope_declarations()[index_] };
    }

    bool Declaration::is_enumeration() const
    {
        return index_.sort() == ifc::DeclSort::Enumeration;
    }

    Enumeration Declaration::as_enumeration() const
    {
        return { ifc_, ifc_.enumerations()[index_] };
    }

    bool Declaration::is_template() const
    {
        return index_.sort() == ifc::DeclSort::Template;
    }

    TemplateDeclaration Declaration::as_template() const
    {
        return { ifc_, ifc_.template_declarations()[index_] };
    }

    bool Declaration::is_variable() const
    {
        return index_.sort() == ifc::DeclSort::Variable;
    }

    Variable Declaration::as_variable() const
    {
        return { ifc_, ifc_.variables()[index_] };
    }

    bool Declaration::is_field() const
    {
        return index_.sort() == ifc::DeclSort::Field;
    }

    Field Declaration::as_field() const
    {
        return { ifc_, ifc_.fields()[index_] };
    }
}
