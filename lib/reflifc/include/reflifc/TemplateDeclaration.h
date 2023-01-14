#pragma once

#include <ifc/DeclarationFwd.h>
#include <ifc/FileFwd.h>

#include "Name.h"

namespace reflifc
{
    struct TemplateDeclaration
    {
        TemplateDeclaration(ifc::File const& ifc, ifc::TemplateDeclaration const& template_)
            : ifc_(ifc)
            , template_(template_)
        {
        }

        Name name() const;

    private:
        ifc::File const & ifc_;
        ifc::TemplateDeclaration const& template_;
    };
}
