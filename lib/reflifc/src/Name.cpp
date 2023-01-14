#include "reflifc/Name.h"

#include <ifc/File.h>
#include <ifc/Name.h>

bool reflifc::Name::is_identifier() const
{
    return index_.sort() == ifc::NameSort::Identifier;
}

char const* reflifc::Name::as_identifier() const
{
    return ifc_.get_string(ifc::TextOffset{index_.index});
}
