#include "schema.h"

#include <ifc/Declaration.h>
#include <ifc/Expression.h>
#include <ifc/Name.h>
#include <ifc/Type.h>

#include <ranges>

const char* Schema::get_identifier(ifc::NameIndex name) const
{
    assert(name.sort() == ifc::NameSort::Identifier);
    return get_string(ifc::TextOffset{name.index});
}

bool Schema::name_is(ifc::NameIndex name, std::string_view pattern) const
{
    return name.sort() == ifc::NameSort::Identifier && get_identifier(name) == pattern;
}

std::string_view Schema::string_literal_value(ifc::StringLiteral literal) const
{
    return { get_string(literal.start), raw_count(literal.length) - 1 };
}

ifc::TypeBasis Schema::get_kind(ifc::ScopeDeclaration const& scope) const
{
    return ifc::get_kind(scope, *this);
}

auto Schema::get_declarations(ifc::ScopeIndex scope) const
{
    return ifc::get_declarations(*this, scope_descriptors()[scope]) | std::views::transform(&ifc::Declaration::index);
}

Schema::Schema(BlobView raw_data): File(raw_data)
{
    auto const& ifc_namespace = find_ifc_namespace();
    auto partitions = get_declarations(ifc_namespace.initializer)
        | std::views::filter([] (ifc::DeclIndex decl) { return decl.sort() == ifc::DeclSort::Scope; })
        | std::views::transform([this] (ifc::DeclIndex decl) {
            return scope_declarations()[decl];
        })
        | std::views::filter([this] (ifc::ScopeDeclaration const & scope) {
            auto members = scope.initializer;
            if (is_null(members))
                return false;

            for (auto member : get_declarations(members))
            {
                if (member.sort() == ifc::DeclSort::Variable)
                {
                    auto const& var = variables()[member];
                    if (name_is(var.name, "PartitionName"))
                    {
                        assert(get_kind(scope) == ifc::TypeBasis::Struct);
                        return true;
                    }
                }
            }

            return false;
        })
        ;

    for (auto const& partition : partitions)
        partition_description.insert(create_partition_description(partition));
}

std::pair<std::string_view, PartitionDescription> Schema::create_partition_description(ifc::ScopeDeclaration const& partition) const
{
    std::string_view name;
    PartitionDescription descr;

    if (auto base = partition.base; !base.is_null())
    {
        auto base_type = base_types()[base].type;
        auto const& base_decl = scope_declarations()[designated_types()[base_type].decl];
        for (auto member : get_declarations(base_decl.initializer))
        {
            if (member.sort() == ifc::DeclSort::Field)
                descr.fields.push_back(create_field(member));
        }
    }
    for (auto member : get_declarations(partition.initializer))
    {
        switch (member.sort())
        {
        case ifc::DeclSort::Variable:
        {
            auto const& var = variables()[member];
            if (name_is(var.name, "PartitionName"))
            {
                auto const& initializer = dyad_expressions()[var.initializer];
                assert(initializer.op == ifc::DyadicOperator::Plus);
                assert(literal_expressions()[initializer.arguments[1]].value.is_null());
                const auto first_arg = string_expressions()[initializer.arguments[0]];
                name = string_literal_value(string_literal_expressions()[first_arg.string_index]);
            }
            break;
        }
        case ifc::DeclSort::Field:
        {
            descr.fields.push_back(create_field(member));
            break;
        }
        default:
            break;
        }
    }
    return { name, std::move(descr) };
}

PartitionDescription::Field Schema::create_field(ifc::DeclIndex decl) const
{
    auto const& field = fields()[decl];
    return { get_string(field.name), field.type };
}

ifc::ScopeDeclaration const& Schema::find_ifc_namespace() const
{
    auto scopes = scope_declarations();
    const auto it = std::ranges::find_if(scopes, [this] (ifc::ScopeDeclaration const& scope)
    {
        return get_kind(scope) == ifc::TypeBasis::Namespace && name_is(scope.name, "ifc");
    });
    assert(it != scopes.end());
    return *it;
}
