#pragma once

#include <ifc/FileFwd.h>
#include <ifc/NameFwd.h>

#include <string_view>

namespace reflifc
{
    struct Name
    {
        Name(ifc::File const& ifc, ifc::NameIndex index)
            : ifc_(ifc)
            , index_(index)
        {
        }

        bool        is_identifier() const;
        char const* as_identifier() const;

    private:
        ifc::File const & ifc_;
        ifc::NameIndex index_;
    };

    inline bool is_identifier(Name name, std::string_view s)
    {
        return name.is_identifier() && name.as_identifier() == s;
    }

    template<typename Declaration>
    bool has_name(Declaration declaration, std::string_view s)
    {
        return is_identifier(declaration.name(), s);
    }
}