#pragma once

#include "reflifc/Module.h"

#include "reflifc/decl/Enumeration.h"
#include "reflifc/decl/Field.h"
#include "reflifc/decl/ClassOrStruct.h"

struct PartitionDescription
{
    std::vector<reflifc::Field> fields;
};

class Commander
{
    ifc::File const& file_;
    std::unordered_map<std::string_view, ifc::PartitionSummary const*> partition_summary_;
    std::unordered_map<std::string_view, PartitionDescription> partition_description_;

    using ArgumentList = std::span<const std::string_view>;

    std::unordered_map<std::string_view, void(Commander::*)(ArgumentList)> commands_ = {
        { "partition", &Commander::partition },
        { "element",   &Commander::element },
    };

private:
    void fill_partitions(reflifc::Namespace);

    void present_partitions();
    size_t calculate_size_of(PartitionDescription const&) const;
    size_t size_of(reflifc::Type) const;
    size_t size_of(ifc::FundamentalType) const;
    size_t size_of(reflifc::ClassOrStruct) const;
    void present_partition(std::string_view partition);
    void partition(ArgumentList arguments);
    void element(ArgumentList arguments);
    void present_element(std::string_view partition, uint32_t index, ifc::PartitionSummary const&);
    void present_element(std::byte const* data_ptr, PartitionDescription const&);
    void present_value(std::byte const*& data_ptr, reflifc::Type);
    void present_object_value(const std::byte*& data_ptr, reflifc::ClassOrStruct);
    void present_enumerator(std::byte const*& data_ptr, reflifc::Enumeration);
    void present_abstract_reference(uint32_t storage, reflifc::Expression first_templarg, reflifc::Expression second_templarg);

    void present_partition_description(PartitionDescription const& partition);
    void present_type(reflifc::Type);
    void present_fundamental_type(ifc::FundamentalType);

public:
    Commander(reflifc::Module schema, ifc::File const& file);

    void operator() (std::string_view command);
};
