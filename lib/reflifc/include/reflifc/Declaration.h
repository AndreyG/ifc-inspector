#pragma once

#include <ifc/FileFwd.h>
#include <ifc/DeclarationFwd.h>

namespace reflifc
{
    struct ScopeDeclaration;
    struct Enumeration;
    struct TemplateDeclaration;
    struct Variable;
    struct Field;

    struct Declaration
    {
        Declaration(ifc::File const& ifc, ifc::DeclIndex index)
            : ifc_(ifc)
            , index_(index)
        {
        }

        bool                is_scope() const;
        ScopeDeclaration    as_scope() const;

        bool                is_enumeration() const;
        Enumeration         as_enumeration() const;

        bool                is_template() const;
        TemplateDeclaration as_template() const;

        bool                is_variable() const;
        Variable            as_variable() const;

        bool                is_field() const;
        Field               as_field() const;

    private:
        ifc::File const &ifc_;
        ifc::DeclIndex index_;
    };
}
