#include "reflifc/TemplateDeclaration.h"

#include <ifc/Declaration.h>

namespace reflifc
{
    Name TemplateDeclaration::name() const
    {
        return { ifc_, template_.name };
    }
}
