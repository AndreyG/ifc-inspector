#include "commander.h"

#include <ifc/Declaration.h>
#include <ifc/Expression.h>
#include <ifc/Type.h>

#include <charconv>
#include <iostream>

using namespace std::string_view_literals;

template<typename T>
T const* get_value(std::unordered_map<std::string_view, T> const & map, std::string_view key)
{
    auto it = map.find(key);
    if (it == map.end())
        return nullptr;
    return &it->second;
}

Commander::Commander(ifc::File::BlobView schema_raw_data, ifc::File const& file)
    : schema_(schema_raw_data)
    , file_(file)
{
    for (ifc::PartitionSummary const& partition : file_.table_of_contents())
    {
        std::string_view name = file_.get_string(partition.name);
        partition_summary_.emplace(name, &partition);
    }
}

void Commander::present_partitions()
{
    for (ifc::PartitionSummary const& partition : file_.table_of_contents())
    {
        std::cout << "name: '" << file_.get_string(partition.name) << "'"
            << ", elements count: " << raw_count(partition.cardinality)
            << "\n";
    }
}

static void present_summary(ifc::PartitionSummary const& summary)
{
    std::cout << "elements count: " << raw_count(summary.cardinality) << "\n";
}

void Commander::present_partition(std::string_view partition)
{
    auto summary = get_value(partition_summary_, partition);
    if (!summary)
    {
        std::cout << "file doesn't contain partition '" << partition << "'\n";
        return;
    }
    present_summary(**summary);

    auto descr = get_value(schema_.partition_description, partition);
    if (!descr)
    {
        std::cout << "unknown partition '" << partition << "'\n";
        return;
    }
    present_partition_description(*descr);

    const auto expected_entry_sizeof = static_cast<size_t>((*summary)->entry_size);
    const auto calculated_entry_sizeof = calculate_size_of(*descr);
    if (expected_entry_sizeof != calculated_entry_sizeof)
    {
        std::cout << "expected sizeof = " << expected_entry_sizeof
                  << ", but calculated sizeof " << calculated_entry_sizeof << "\n";
    }
}

static size_t align4(size_t size)
{
    return ((size - 1) / 4 + 1) * 4;
}

size_t Commander::calculate_size_of(PartitionDescription const& descr) const
{
    size_t size = 0;
    for (auto const& field : descr.fields)
        size += size_of(field.type);
    return align4(size);
}

size_t Commander::size_of(ifc::TypeIndex type) const
{
    using enum ifc::TypeSort;

    switch (const auto kind = type.sort())
    {
    case Fundamental:
        return size_of(schema_.fundamental_types()[type]);
    case Designated:
    {
        auto decl = schema_.designated_types()[type].decl;
        switch (const auto kind = decl.sort())
        {
        case ifc::DeclSort::Scope:
        {
            auto const& scope = schema_.scope_declarations()[decl];
            return size_of(scope);
        }
        case ifc::DeclSort::Enumeration:
        {
            auto const& enumeration = schema_.enumerations()[decl];
            return size_of(enumeration.base);
        }
        default:
            throw std::runtime_error("<unknown declaration>");
        }
    }
    case Syntactic:
    {
        ifc::TemplateId const& template_id = schema_.template_ids()[schema_.syntactic_types()[type].expr];
        auto primary_entity = schema_.decl_expressions()[template_id.primary].resolution;
        auto primary_entity_name = schema_.get_identifier(schema_.template_declarations()[primary_entity].name);
        assert(primary_entity_name == "AbstractReference"sv);
        return sizeof(std::uint32_t);
    }
    case Array:
    {
        auto array = schema_.array_types()[type];
        return sizeof array.element * get_int_literal_value(array.extent);
    }
    default:
        throw std::runtime_error("unexpected type");
    }
}

size_t Commander::size_of(ifc::FundamentalType type) const
{
    switch (const auto basis = type.basis)
    {
    case ifc::TypeBasis::Int:
        switch (type.precision)
        {
        case ifc::TypePrecision::Bit8:      return 8;
        case ifc::TypePrecision::Bit16:     return 16;
        case ifc::TypePrecision::Bit32:     return 32;
        case ifc::TypePrecision::Bit64:     return 64;
        case ifc::TypePrecision::Bit128:    return 128;
        case ifc::TypePrecision::Default:   return sizeof(int);
        case ifc::TypePrecision::Short:     return sizeof(short);
        case ifc::TypePrecision::Long:      return sizeof(long);
        default:
            throw std::runtime_error("unexpected precision");
        }
    case ifc::TypeBasis::Char:
        return sizeof(char);
    case ifc::TypeBasis::Bool:
        return sizeof(bool);
    default:
        throw std::runtime_error("unexpected type basis");
    }
}

size_t Commander::size_of(ifc::ScopeDeclaration const& scope) const
{
    size_t size = 0;

    for (auto member : get_declarations(schema_, schema_.scope_descriptors()[scope.initializer]))
    {
        if (member.index.sort() == ifc::DeclSort::Field)
            size += size_of(schema_.fields()[member.index].type);
    }

    return align4(size);
}

void Commander::present_partition_description(PartitionDescription const& partition)
{
    std::cout << "[Fields]\n";
    for (auto const & field : partition.fields)
    {
        std::cout << field.name << ": ";
        present_type(field.type);
        std::cout << '\n';
    }
}

void Commander::present_type(ifc::TypeIndex type)
{
    using enum ifc::TypeSort;

    switch (const auto kind = type.sort())
    {
    case Fundamental:
        return present_fundamental_type(schema_.fundamental_types()[type]);
    case Designated:
    {
        auto decl = schema_.designated_types()[type].decl;
        switch (const auto kind = decl.sort())
        {
        case ifc::DeclSort::Scope:
            std::cout << schema_.get_identifier(schema_.scope_declarations()[decl].name);
            break;
        case ifc::DeclSort::Enumeration:
            std::cout << schema_.get_string(schema_.enumerations()[decl].name);
            break;
        default:
            std::cout << "<unknown declaration>";
        }
        break;
    }
    case Syntactic:
    {
        ifc::TemplateId const& template_id = schema_.template_ids()[schema_.syntactic_types()[type].expr];
        auto primary_entity = schema_.decl_expressions()[template_id.primary].resolution;
        auto primary_entity_name = schema_.get_identifier(schema_.template_declarations()[primary_entity].name);
        assert(primary_entity_name == "AbstractReference"sv);
        auto arguments = get_tuple_expression_elements(schema_, schema_.tuple_expressions()[template_id.arguments]);
        assert(arguments.size() == 2);
        auto second_arg = schema_.type_expressions()[arguments[ifc::Index{1}]].denotation;
        std::string_view second_arg_name = schema_.get_string(schema_.enumerations()[schema_.designated_types()[second_arg].decl].name);
        assert(second_arg_name.ends_with("Sort"));
        second_arg_name.remove_suffix(4);
        std::cout << second_arg_name << "Index";
        break;
    }
    case Array:
    {
        auto array = schema_.array_types()[type];
        present_type(array.element);
        std::cout << "[" << get_int_literal_value(array.extent) << "]";
        break;
    }
    default:
        std::cout << "<unknown type>";
    }
}

void Commander::present_fundamental_type(ifc::FundamentalType type)
{
    switch (type.basis)
    {
    case ifc::TypeBasis::Bool:
        std::cout << "bool";
        break;
    default:
        std::cout << "<unknown fundamental type>";
    }
}

std::uint32_t Commander::get_int_literal_value(ifc::ExprIndex expr) const
{
    const auto literal = schema_.literal_expressions()[expr].value;
    assert(literal.sort() == ifc::LiteralSort::Immediate);
    return literal.index;
}

void Commander::partition(ArgumentList arguments)
{
    switch (arguments.size())
    {
    case 0:
        present_partitions();
        break;
    case 1:
        present_partition(arguments.front());
        break;
    default:
        std::cout << "command 'partition' expects 0 or 1 argument\n";
        break;
    }
}

static std::optional<std::uint32_t> parse_index(std::string_view str)
{
    std::uint32_t index;
    if (std::from_chars(str.data(), str.data() + str.size(), index).ec != std::errc{})
        return std::nullopt;

    return index;
}

void Commander::element(ArgumentList arguments)
{
    if (arguments.size() != 2)
    {
        std::cout << "command 'element' usage: 'partition name' 'element index'\n";
        return;
    }

    const auto partition = arguments[0];

    auto summary = get_value(partition_summary_, partition);
    if (!summary)
    {
        std::cout << "file doesn't contain partition '" << partition << "'\n";
        return;
    }

    auto index = parse_index(arguments[1]);
    if (!index)
    {
        std::cout << "second argument must be non-negative integer\n";
        return;
    }

    present_element(partition, *index, **summary);
}

void Commander::present_element(std::string_view partition, uint32_t index, ifc::PartitionSummary const& summary)
{
    const auto partition_size = raw_count(summary.cardinality);
    if (index >= partition_size)
    {
        std::cout << index << " is out of bounds, partition cardinality = " << partition_size << "\n";
        return;
    }

    auto descr = get_value(schema_.partition_description, partition);
    if (!descr)
    {
        std::cout << "unknown partition '" << partition << "'\n";
        return;
    }

    const auto entry_sizeof = static_cast<size_t>(summary.entry_size);
    const auto calculated_entry_sizeof = calculate_size_of(*descr);
    if (entry_sizeof != calculated_entry_sizeof)
    {
        std::cout << "expected sizeof = " << entry_sizeof
                  << ", but calculated sizeof " << calculated_entry_sizeof << "\n";
        return;
    }

    auto data_ptr = file_.get_data_pointer(summary) + index * entry_sizeof;
    present_element(data_ptr, *descr);
}

void Commander::present_element(std::byte const* data_ptr, PartitionDescription const& descr)
{
    for (auto const& field : descr.fields)
    {
        std::cout << field.name << ": ";
        present_value(data_ptr, field.type);
        std::cout << "\n";
    }
}

namespace
{
    template<typename Int>
    void present_int(const std::byte*& data_ptr)
    {
        std::cout << *reinterpret_cast<Int const*>(data_ptr);
        data_ptr += sizeof(Int);
    }

    void present_fundamental_value(const std::byte*& data_ptr, ifc::FundamentalType type)
    {
        if (type.basis == ifc::TypeBasis::Bool)
        {
            std::cout << std::boolalpha << *reinterpret_cast<bool const*>(data_ptr);
            data_ptr += sizeof(bool);
            return;
        }
        assert(type.basis == ifc::TypeBasis::Int);
        assert(type.sign == ifc::TypeSign::Unsigned);
        switch (type.precision)
        {
        case ifc::TypePrecision::Default:   return present_int<unsigned int>(data_ptr);
        case ifc::TypePrecision::Short:     return present_int<unsigned short>(data_ptr);
        case ifc::TypePrecision::Long:      return present_int<unsigned long>(data_ptr);
        case ifc::TypePrecision::Bit8:      return present_int<uint8_t>(data_ptr);
        case ifc::TypePrecision::Bit16:     return present_int<uint16_t>(data_ptr);
        case ifc::TypePrecision::Bit32:     return present_int<uint32_t>(data_ptr);
        case ifc::TypePrecision::Bit64:     return present_int<uint64_t>(data_ptr);
        }
    }
}

void Commander::present_value(std::byte const*& data_ptr, ifc::TypeIndex type)
{
    using enum ifc::TypeSort;

    switch (const auto kind = type.sort())
    {
    case Fundamental:
        return present_fundamental_value(data_ptr, schema_.fundamental_types()[type]);
    case Designated:
    {
        auto decl = schema_.designated_types()[type].decl;
        switch (const auto kind = decl.sort())
        {
        case ifc::DeclSort::Scope:
            return present_object_value(data_ptr, schema_.scope_declarations()[decl]);
        case ifc::DeclSort::Enumeration:
            return present_enumerator(data_ptr, schema_.enumerations()[decl]);
        default:
            throw std::runtime_error("<unknown declaration>");
        }
    }
    case Syntactic:
    {
        ifc::TemplateId const& template_id = schema_.template_ids()[schema_.syntactic_types()[type].expr];
        auto primary_entity = schema_.decl_expressions()[template_id.primary].resolution;
        auto primary_entity_name = schema_.get_identifier(schema_.template_declarations()[primary_entity].name);
        assert(primary_entity_name == "AbstractReference"sv);
        auto arguments = get_tuple_expression_elements(schema_, schema_.tuple_expressions()[template_id.arguments]);
        assert(arguments.size() == 2);
        present_abstract_reference(*reinterpret_cast<std::uint32_t const*>(data_ptr), arguments[ifc::Index{0}], arguments[ifc::Index{1}]);
        data_ptr += sizeof(std::uint32_t);
        break;
    }
    case Array:
    {
        const auto array = schema_.array_types()[type];
        std::cout << "[";
        bool first = true;
        for (std::uint32_t array_size = get_int_literal_value(array.extent), i = 0; i != array_size; ++i)
        {
            if (first)
                first = false;
            else
                std::cout << ", ";
            present_value(data_ptr, array.element);
        }
        std::cout << "]";
        break;
    }
    default:
        throw std::runtime_error("unexpected type");
    }
}

void Commander::present_object_value(const std::byte*& data_ptr, ifc::ScopeDeclaration const& scope)
{
    std::cout << "{ ";

    bool first = true;
    for (auto member : get_declarations(schema_, schema_.scope_descriptors()[scope.initializer]))
    {
        if (member.index.sort() == ifc::DeclSort::Field)
        {
            if (first)
                first = false;
            else
                std::cout << ", ";

            auto const& field = schema_.fields()[member.index];
            std::cout << schema_.get_string(field.name) << ": ";
            present_value(data_ptr, field.type);
        }
    }

    if (!first)
        std::cout << ' ';
    std::cout << "}";
}

void Commander::present_enumerator(std::byte const*& data_ptr, ifc::Enumeration const& enumeration)
{
    if (raw_count(enumeration.initializer.cardinality) == 0)
    {
        if (schema_.get_string(enumeration.name) == "TextOffset"sv)
        {
            std::cout << file_.get_string(*reinterpret_cast<ifc::TextOffset const*>(data_ptr));
            data_ptr += sizeof(ifc::TextOffset);
            return;
        }
        return present_value(data_ptr, enumeration.base);
    }

    const auto size = size_of(enumeration.base);
    for (ifc::Enumerator const & enumerator : schema_.enumerators().slice(enumeration.initializer))
    {
        const auto value = get_int_literal_value(enumerator.initializer);
        if (std::memcmp(data_ptr, &value, size) == 0)
        {
            std::cout << schema_.get_string(enumerator.name);
            goto exit;
        }
    }
    std::cout << "<unknown enumerator>";

exit:
    data_ptr += size;
}

void Commander::present_abstract_reference(uint32_t storage, ifc::ExprIndex first_templarg, ifc::ExprIndex second_templarg)
{
    if (storage == 0)
    {
        std::cout << "null";
        return;
    }

    auto const& enumeration = schema_.enumerations()[schema_.designated_types()[schema_.type_expressions()[second_templarg].denotation].decl];
    if (schema_.get_string(enumeration.name) == "NameSort"sv)
    {
        auto name = reinterpret_cast<ifc::NameIndex&>(storage);
        if (name.sort() == ifc::NameSort::Identifier)
        {
            std::cout << file_.get_string(ifc::TextOffset{name.index});
            return;
        }
    }

    const auto bits_count = get_int_literal_value(first_templarg);
    const auto mask = (1u << bits_count) - 1;
    const auto sort = storage & mask;

    std::cout << "(";
    for (ifc::Enumerator const & enumerator : schema_.enumerators().slice(enumeration.initializer))
    {
        const auto value = get_int_literal_value(enumerator.initializer);
        if (value == sort)
        {
            std::cout << schema_.get_string(enumerator.name);
            break;
        }
    }
    std::cout << ", " << ((storage ^ mask) >> bits_count) << ")";
}

static std::vector<std::string_view> split(std::string_view s)
{
    size_t pos_start = 0, pos_end;
    std::vector<std::string_view> res;

    while ((pos_end = s.find(' ', pos_start)) != std::string::npos) {
        res.push_back(s.substr(pos_start, pos_end - pos_start));
        pos_start = pos_end + 1;
    }

    res.push_back(s.substr(pos_start));
    return res;
}

void Commander::operator()(std::string_view command)
{
    auto parts = split(command);
    auto command_name = parts.front();
    auto it = commands_.find(command_name);
    if (it == commands_.end())
    {
        std::cout << "unknown command '" << command_name << "'\n";
    }
    else
    {
        auto fptr = it->second;
        (this->*fptr)(ArgumentList(parts).subspan(1));
    }
}
