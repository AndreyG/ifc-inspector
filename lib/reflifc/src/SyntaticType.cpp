#include "reflifc/SyntaticType.h"

#include "reflifc/Declaration.h"
#include "reflifc/Expression.h"
#include "reflifc/TemplateDeclaration.h"

#include <ifc/Expression.h>
#include <ifc/File.h>

namespace reflifc
{
    TemplateDeclaration SyntaticType::primary() const
    {
        return Declaration(ifc_, ifc_.decl_expressions()[template_id_.primary].resolution).as_template();
    }

    std::vector<Expression> SyntaticType::arguments() const
    {
        if (template_id_.arguments.sort() == ifc::ExprSort::Tuple)
        {
            auto elements = get_tuple_expression_elements(ifc_, ifc_.tuple_expressions()[template_id_.arguments]);
            std::vector<Expression> result;
            result.reserve(elements.size());
            for (auto arg : elements)
                result.emplace_back(ifc_, arg);
            return result;
        }

        return { { ifc_, template_id_.arguments } };
    }
}
