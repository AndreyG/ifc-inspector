#include "commander.h"

#include "reflifc/Expression.h"
#include "reflifc/Type.h"
#include "reflifc/Literal.h"
#include "reflifc/StringLiteral.h"
#include "reflifc/Query.h"
#include "reflifc/TemplateId.h"
#include "reflifc/TupleExpressionView.h"
#include "reflifc/decl/Enumeration.h"
#include "reflifc/decl/Enumerator.h"
#include "reflifc/decl/TemplateDeclaration.h"
#include "reflifc/decl/Variable.h"
#include "reflifc/decl/ScopeDeclaration.h"
#include "reflifc/expr/Dyad.h"
#include "reflifc/type/Array.h"
#include "reflifc/type/Base.h"

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

static std::pair<std::string_view, PartitionDescription> create_partition_description(reflifc::ClassOrStruct partition)
{
    std::string_view name;
    PartitionDescription descr;

    for (auto base : partition.bases())
        std::ranges::copy(fields(base.type.designation().as_class_or_struct()), std::back_inserter(descr.fields));

    for (auto member : partition.members())
    {
        if (member.is_field())
        {
            descr.fields.push_back(member.as_field());
        }
        else if (member.is_variable())
        {
            auto var = member.as_variable();
            if (has_name(var, "PartitionName"))
            {
                auto value = var.initializer().as_dyad();
                assert(value.op() == ifc::DyadicOperator::Plus);
                assert(value.right().as_literal().is_null());
                name = value.left().as_string_literal().value();
            }
        }
    }
    return { name, std::move(descr) };
}

void Commander::fill_partitions(reflifc::Namespace ifc_namespace)
{
    for (auto strct : get_classes_and_structs(ifc_namespace.scope()))
    {
        if (!strct.is_complete())
            continue;

        for (auto var : static_variables(strct))
        {
            if (has_name(var, "PartitionName"))
            {
                partition_description_.insert(create_partition_description(strct));
                break;
            }
        }
    }
}

Commander::Commander(reflifc::Module schema, ifc::File const& file)
    : file_(file)
{
    for (ifc::PartitionSummary const& partition : file_.table_of_contents())
    {
        std::string_view name = file_.get_string(partition.name);
        partition_summary_.emplace(name, &partition);
    }
    fill_partitions(find_namespace_by_name(schema.global_namespace(), "ifc").value());
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

    auto descr = get_value(partition_description_, partition);
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
        size += size_of(field.type());
    return align4(size);
}

size_t Commander::size_of(reflifc::Type type) const
{
    if (type.is_fundamental())
        return size_of(type.as_fundamental());

    if (type.is_designated())
    {
        auto decl = type.designation();

        if (decl.is_enumeration())
            return size_of(decl.as_enumeration().underlying_type());

        return size_of(decl.as_class_or_struct());
    }

    if (type.is_array())
    {
        auto array = type.as_array();
        return size_of(array.element()) * extent_value(array);
    }

    auto syntatic_type = type.as_syntactic();
    assert(has_name(syntatic_type.primary().as_template(), "AbstractReference"));
    return sizeof(std::uint32_t);
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

size_t Commander::size_of(reflifc::ClassOrStruct strct) const
{
    size_t size = 0;

    for (auto field : fields(strct))
        size += size_of(field.type());

    return align4(size);
}

void Commander::present_partition_description(PartitionDescription const& partition)
{
    std::cout << "[Fields]\n";
    for (auto const & field : partition.fields)
    {
        std::cout << field.name() << ": ";
        present_type(field.type());
        std::cout << '\n';
    }
}

void Commander::present_type(reflifc::Type type)
{
    if (type.is_fundamental())
    {
        present_fundamental_type(type.as_fundamental());
    }
    else if (type.is_designated())
    {
        auto decl = type.designation();

        if (decl.is_enumeration())
            std::cout << decl.as_enumeration().name();
        else
            std::cout << decl.as_scope().name().as_identifier();
    }
    else if (type.is_array())
    {
        auto array = type.as_array();
        present_type(array.element());
        std::cout << "[" << extent_value(array) << "]";
    }
    else
    {
        auto syntatic_type = type.as_syntactic();
        assert(has_name(syntatic_type.primary().as_template(), "AbstractReference"));
        auto arguments = syntatic_type.arguments();
        assert(arguments.size() == 2);
        std::string_view second_arg_name = arguments[1].as_type().designation().as_enumeration().name();
        assert(second_arg_name.ends_with("Sort"));
        second_arg_name.remove_suffix(4);
        std::cout << second_arg_name << "Index";
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

    auto descr = get_value(partition_description_, partition);
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
        std::cout << field.name() << ": ";
        present_value(data_ptr, field.type());
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

void Commander::present_value(std::byte const*& data_ptr, reflifc::Type type)
{
    if (type.is_fundamental())
    {
        present_fundamental_value(data_ptr, type.as_fundamental());
    }
    else if (type.is_designated())
    {
        auto decl = type.designation();
        if (decl.is_enumeration())
            present_enumerator(data_ptr, decl.as_enumeration());
        else
            present_object_value(data_ptr, decl.as_class_or_struct());
    }
    else if (type.is_array())
    {
        const auto array = type.as_array();
        std::cout << "[";
        bool first = true;
        for (std::uint32_t array_size = extent_value(array), i = 0; i != array_size; ++i)
        {
            if (first)
                first = false;
            else
                std::cout << ", ";
            present_value(data_ptr, array.element());
        }
        std::cout << "]";
    }
    else
    {
        auto syntatic_type = type.as_syntactic();
        assert(has_name(syntatic_type.primary().as_template(), "AbstractReference"));
        auto arguments = syntatic_type.arguments();
        assert(arguments.size() == 2);
        present_abstract_reference(*reinterpret_cast<std::uint32_t const*>(data_ptr), arguments[0], arguments[1]);
        data_ptr += sizeof(std::uint32_t);
    }
}

void Commander::present_object_value(const std::byte*& data_ptr, reflifc::ClassOrStruct strct)
{
    std::cout << "{ ";

    bool first = true;
    for (auto field : fields(strct))
    {
        if (first)
            first = false;
        else
            std::cout << ", ";

        std::cout << field.name() << ": ";
        present_value(data_ptr, field.type());
    }

    if (!first)
        std::cout << ' ';
    std::cout << "}";
}

void Commander::present_enumerator(std::byte const*& data_ptr, reflifc::Enumeration enumeration)
{
    if (enumeration.enumerators().empty())
    {
        if (enumeration.name() == "TextOffset"sv)
        {
            std::cout << file_.get_string(*reinterpret_cast<ifc::TextOffset const*>(data_ptr));
            data_ptr += sizeof(ifc::TextOffset);
            return;
        }
        return present_value(data_ptr, enumeration.underlying_type());
    }

    const auto size = size_of(enumeration.underlying_type());
    if (auto enumerator = find_enumerator_by_value(enumeration, std::span(data_ptr, size)))
        std::cout << enumerator->name();
    else
        std::cout << "<unknown enumerator>";

    data_ptr += size;
}

void Commander::present_abstract_reference(uint32_t storage, reflifc::Expression first_templarg, reflifc::Expression second_templarg)
{
    if (storage == 0)
    {
        std::cout << "null";
        return;
    }

    auto enumeration = second_templarg.as_type().designation().as_enumeration();
    if (enumeration.name() == "NameSort"sv)
    {
        auto name = reinterpret_cast<ifc::NameIndex&>(storage);
        if (name.sort() == ifc::NameSort::Identifier)
        {
            std::cout << file_.get_string(ifc::TextOffset{name.index});
            return;
        }
    }

    const auto bits_count = first_templarg.as_literal().int_value();
    const auto mask = (1u << bits_count) - 1;
    const auto sort = storage & mask;

    std::cout << "("
              << find_enumerator_by_value(enumeration, sort)->name()
              << ", "
              << ((storage ^ mask) >> bits_count)
              << ")";
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
