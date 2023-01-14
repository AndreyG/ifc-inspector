#pragma once

#include <ifc/FileFwd.h>
#include <ifc/ExpressionFwd.h>

namespace reflifc
{
    struct TemplateDeclaration;
    struct Expression;

    struct SyntaticType
    {
        SyntaticType(ifc::File const& ifc, ifc::TemplateId const& template_id)
            : ifc_(ifc)
            , template_id_(template_id)
        {
        }

        TemplateDeclaration primary() const;

        std::vector<Expression> arguments() const;

    private:
        ifc::File const& ifc_;
        ifc::TemplateId const& template_id_;
    };
}