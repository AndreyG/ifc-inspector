#pragma once

#include <ifc/File.h>

struct PartitionDescription
{
    struct Field
    {
        const char* name;
        ifc::TypeIndex type;
    };

    std::vector<Field> fields;
};

class Schema : public ifc::File
{
    bool name_is(ifc::NameIndex name, std::string_view pattern) const;

    std::string_view string_literal_value(ifc::StringLiteral literal) const;

    ifc::TypeBasis get_kind(ifc::ScopeDeclaration const& scope) const;

    auto get_declarations(ifc::ScopeIndex scope) const;

public:
    std::unordered_map<std::string_view, PartitionDescription> partition_description;

    Schema(BlobView raw_data);

    const char* get_identifier(ifc::NameIndex name) const;

private:
    ifc::ScopeDeclaration const& find_ifc_namespace() const;

    std::pair<std::string_view, PartitionDescription> create_partition_description(ifc::ScopeDeclaration const&) const;

    PartitionDescription::Field create_field(ifc::DeclIndex decl) const;
};
